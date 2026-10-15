#!/usr/bin/env bash

#
# Build CoD with a pinned, unmodified LLVM release in three stages.
# Linux bundles Clang, LLD, libc++, libc++abi, libunwind and compiler-rt.
# See docs/LinuxBuildEnvironment.md for prerequisites and validation records.
#

LLVM_RELEASE=llvmorg-23.1.3
LLVM_COMMIT=0d261d1ca552c95a8f007e061c787ac7132fbcbc

# LLVM targets to be supported by CoD
: "${COD_TARGETS_TO_BUILD:=Native}"
# on Cuda ready OSes, include NVPTX target for Cuda support
# COD_TARGETS_TO_BUILD="Native;NVPTX"

# build type for CoD
: "${COD_BUILD_TYPE:=Release}" # or RelWithDebInfo, or Debug

{ # keep body of cmds in this single command group, so they execute as a whole
	# thus edit-during-(long)-execution and partial downloads are not problems
	set -e
	cd "$(dirname "$0")"
	COD_SOURCE_DIR=$(pwd)

	# One checkout/worktree owns one development environment.  Native, WSL,
	# Docker, and macOS builds should use separate worktrees instead of sharing
	# one source directory.  Keep the final development build tree at ./build so
	# clangd finds build/compile_commands.json by convention.
	BUILD_DIR="$COD_SOURCE_DIR/build"
	BOOTSTRAP_DIR="$BUILD_DIR/.bootstrap"
	STAGE1_DIR="$BOOTSTRAP_DIR/stage1"
	STAGE2_DIR="$BOOTSTRAP_DIR/stage2"
	RUNTIME_INSTALL_DIR="$BOOTSTRAP_DIR/cod-rt"
	STAGE2_INSTALL_DIR="$BUILD_DIR/cod"
	FINAL_INSTALL_DIR="$COD_SOURCE_DIR/built"
	for DIR in "$BUILD_DIR" "$FINAL_INSTALL_DIR" "$COD_SOURCE_DIR/llvm-project" \
		"$BOOTSTRAP_DIR" "$STAGE1_DIR" "$STAGE2_DIR" "$RUNTIME_INSTALL_DIR" "$STAGE2_INSTALL_DIR"; do
		if [ -L "$DIR" ]; then
			echo "Error: $DIR must be a real directory, not a symlink." >&2
			exit 1
		fi
		if [ -e "$DIR" ] && [ ! -d "$DIR" ]; then
			echo "Error: $DIR must be a directory." >&2
			exit 1
		fi
	done

	if [ "$(uname)" == "Darwin" ]; then
		# macOS
		HOST_NTHREADS=$(sysctl -n hw.ncpu)

		OS_SPEC_CMAKE_OPTS=(
			-DDEFAULT_SYSROOT="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
			-DCOMPILER_RT_ENABLE_IOS=OFF
			-DCOMPILER_RT_ENABLE_WATCHOS=OFF
			-DCOMPILER_RT_ENABLE_TVOS=OFF
			-DLLVM_CREATE_XCODE_TOOLCHAIN=OFF

			#
			# we want modern features of C++ (those beyond AppleClang) in CoD,
			# tho things would inevitablly become complicated on macOS, e.g.
			# CoD won't be able to load `.dylib`s built with Xcode.  cf.
			#
			#   https://discourse.llvm.org/t/can-different-versions-of-libc-coexist-in-a-program-at-runtime/69302/6
			#
			# the implicit libc++abi from macOS doesn't support our (newer)
			# libc++ well, we have to build our own, yet libunwind from macOS
			# (i.e. -lSystem) is better not overridden likely
			#
			# so far we use macOS' integrated libunwind, and build our own
			# libc++, libc++abi and compiler-rt
			#
			# The macOS runtime selection requires separate validation against
			# the pinned release; the Linux driver configuration is not used here.
			#
			-DLLVM_ENABLE_RUNTIMES="libcxxabi;libcxx;compiler-rt"
			-DLIBCXXABI_USE_LLVM_UNWINDER=OFF
			-DLIBCXXABI_ENABLE_STATIC_UNWINDER=OFF
			-DCOMPILER_RT_USE_LLVM_UNWINDER=OFF
		)

	elif [ "$(uname)" == "Linux" ]; then
		HOST_NTHREADS=$(nproc)

		OS_SPEC_CMAKE_OPTS=(
			-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF
			-DLIBCXX_HAS_ATOMIC_LIB=OFF

			#
			# build our own libc++ suite and compiler-rt on Linux
			#
			# Clang configuration files select the bundled library directory.
			#
			-DLLVM_ENABLE_RUNTIMES="libunwind;libcxxabi;libcxx;compiler-rt"
			-DLIBCXXABI_ENABLE_STATIC_UNWINDER=ON
			-DLIBCXXABI_STATICALLY_LINK_UNWINDER_IN_SHARED_LIBRARY=OFF
			-DLIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY=ON
			-DLIBCXXABI_USE_COMPILER_RT=ON
			-DLIBCXXABI_USE_LLVM_UNWINDER=ON
			-DLIBUNWIND_USE_COMPILER_RT=ON
			-DCOMPILER_RT_USE_BUILTINS_LIBRARY=ON
			-DCOMPILER_RT_USE_LLVM_UNWINDER=ON
			-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON
		)
	else
		echo "Error: this bootstrap supports Linux and macOS hosts only." >&2
		exit 1
	fi

	# Keep the source dependency in this worktree and never update it implicitly.
	if [ ! -e "./llvm-project" ]; then
		git clone --depth 1 --single-branch --no-checkout -b "$LLVM_RELEASE" \
			https://github.com/llvm/llvm-project.git "./llvm-project"
		git -C "./llvm-project" -c checkout.workers=8 sparse-checkout set --no-cone --stdin \
			< "$COD_SOURCE_DIR/toolchain/llvm-sparse-checkout"
		git -C "./llvm-project" -c checkout.workers=8 checkout --detach "$LLVM_COMMIT"
	fi
	ACTUAL_LLVM_COMMIT=$(git -C "./llvm-project" rev-parse HEAD)
	if [ "$ACTUAL_LLVM_COMMIT" != "$LLVM_COMMIT" ]; then
		echo "Error: llvm-project is at '$ACTUAL_LLVM_COMMIT', expected $LLVM_RELEASE ($LLVM_COMMIT)." >&2
		exit 1
	fi
	if [ -n "$(git -C "./llvm-project" status --porcelain)" ]; then
		echo "Error: the patch-free bootstrap requires an unmodified llvm-project checkout." >&2
		exit 1
	fi

	STAGE_COMMON_CMAKE_OPTS=(
		# build an LLVM/Clang based, self-contained C++ toolset
		# though we don't build and bundle libc, treating CRT and the system C
		# compiler, either GCC or (Apple)Clang, as part of the OS rather than
		# part of a toolset
		-DLLVM_ENABLE_PROJECTS="clang;lld"
		#
		# prefer lld - this affects later (except the first) stages
		#
		# system installed linker may not work properly for our clang, when
		# bootstraping CoD at later stages, cmake basic compiler checks may
		# fail due to linking issues without this specified, i.e. system
		# default linker (ld) will be used by cmake's checks, so let's tell
		# our clang to use our own lld by default
		#
		# anyway the final CoD toolset should prefer to use bundled lld
		#
		-DCLANG_DEFAULT_LINKER=lld
		# prefer libc++
		-DCLANG_DEFAULT_CXX_STDLIB="libc++"
		# embrace c++20 modules
		-DLIBCXX_INSTALL_MODULES=ON
		# use our own libclang_rt
		-DLIBCXX_USE_COMPILER_RT=ON

		# various CoD languages/runtimes can share the LLVM lib at runtime
		-DLLVM_LINK_LLVM_DYLIB=ON

		-DLLVM_ENABLE_EH=ON
		-DLLVM_ENABLE_FFI=ON
		-DLLVM_ENABLE_RTTI=ON

		-DLLVM_INCLUDE_DOCS=OFF
		-DLLVM_INCLUDE_TESTS=OFF
		-DLLVM_INCLUDE_BENCHMARKS=OFF
		-DLIBCXX_INCLUDE_DOCS=OFF
		-DLIBCXXABI_INCLUDE_DOCS=OFF
		-DLIBUNWIND_INCLUDE_DOCS=OFF
		-DLLVM_INSTALL_UTILS=OFF
		-DLLVM_OPTIMIZED_TABLEGEN=ON
		-DLLVM_PARALLEL_LINK_JOBS=1

		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DLLVM_ENABLE_IDE=ON
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1

		"${OS_SPEC_CMAKE_OPTS[@]}"

		-G
		Ninja
		-S
		"$COD_SOURCE_DIR/llvm-project/llvm"
	)

	# spare 2 out of all available hardware threads
	: "${COD_BUILD_JOBS:=$((HOST_NTHREADS <= 3 ? 1 : (HOST_NTHREADS - 2)))}"
	export CMAKE_BUILD_PARALLEL_LEVEL="$COD_BUILD_JOBS"

	#
	# stage-2: build CoD toolset with stage1 clang then install it
	#
	#   we always (even on Linux) do HAVE_UNW_ADD_DYNAMIC_FDE=1 per:
	#     https://github.com/llvm/llvm-project/issues/43419
	#
	{ test -x "$STAGE2_INSTALL_DIR/bin/cod" &&
		test -x "$STAGE2_INSTALL_DIR/bin/clang++" &&
		test -x "$STAGE2_INSTALL_DIR/bin/ld.lld"; } || (
		#
		# stage-1: build clang with system compiler (gcc or older clang) toolchain,
		#          bundling lld, libc++ etc. with it
		#
		#   we collect rt libs into cod-rt, and start out later stages with them,
		#   so intermediate tools can dynamically link with them, this is crucial
		#   for llvm-min-tblgen and some others tools, those built and used before
		#   rt libs (e.g. libc++) are built at its own stage
		#
		#   these libs (e.g. libc++) are preferably selected via relative (../lib/)
		#   rpath, this is crucial in case system installed (older) versions lack
		#   symbols from our built libs from llvm-project
		#
		{ test -x "$STAGE1_DIR/bin/clang++" &&
			test -x "$STAGE1_DIR/bin/ld.lld" &&
			test -d "$RUNTIME_INSTALL_DIR/lib"; } || (
			mkdir -p "$STAGE1_DIR" && cd "$STAGE1_DIR"
			cmake -DCMAKE_INSTALL_PREFIX="$RUNTIME_INSTALL_DIR" \
				-DLLVM_TARGETS_TO_BUILD=Native \
				-DCMAKE_BUILD_TYPE=Release \
				"${STAGE_COMMON_CMAKE_OPTS[@]}"
			cmake --build "$STAGE1_DIR"
			cmake --build "$STAGE1_DIR" --target install-runtimes
		)
		if [ "$(uname)" == "Linux" ]; then
			cp "$COD_SOURCE_DIR/toolchain/linux/clang.cfg" "$STAGE1_DIR/bin/clang.cfg"
			cp "$COD_SOURCE_DIR/toolchain/linux/clang++.cfg" "$STAGE1_DIR/bin/clang++.cfg"
		fi

		mkdir -p "$STAGE2_DIR" && cd "$STAGE2_DIR"
		cp -rf "$RUNTIME_INSTALL_DIR/lib" ./
		cmake -DCMAKE_INSTALL_PREFIX="$STAGE2_INSTALL_DIR" \
			-DCMAKE_SYSTEM_PREFIX_PATH="$STAGE1_DIR" \
			-DCMAKE_C_COMPILER="$STAGE1_DIR/bin/clang" \
			-DCMAKE_CXX_COMPILER="$STAGE1_DIR/bin/clang++" \
			-DCMAKE_LINKER="$STAGE1_DIR/bin/ld.lld" \
			-DLLVM_ENABLE_LIBCXX=ON \
			-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
			-DLLVM_EXTERNAL_PROJECTS="cod" \
			-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
			-DLLVM_TARGETS_TO_BUILD=Native \
			-DCMAKE_BUILD_TYPE=Release \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		cmake --build "$STAGE2_DIR"
		cmake --install "$STAGE2_DIR"
	)

	#
	# stage-3: build the final CoD toolset in ./build with itself (stage2
	#          installation), then overwrite ./built
	#
	#          we add lldb etc. only at this final stage
	#
	#   we always (even on Linux) do HAVE_UNW_ADD_DYNAMIC_FDE=1 per:
	#     https://github.com/llvm/llvm-project/issues/43419
	#
	#   this script also does incremental build after CoD cmake cfg changed:
	#     unconditionally update the final build tree to reflect the changes,
	#     then build & install again
	#
	mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"
	test -d ./lib || cp -rf "$STAGE2_INSTALL_DIR/lib" ./
	cmake -DCMAKE_INSTALL_PREFIX="$FINAL_INSTALL_DIR" \
		-DCMAKE_SYSTEM_PREFIX_PATH="$STAGE2_INSTALL_DIR" \
		-DCMAKE_C_COMPILER="$STAGE2_INSTALL_DIR/bin/clang" \
		-DCMAKE_CXX_COMPILER="$STAGE2_INSTALL_DIR/bin/clang++" \
		-DCMAKE_LINKER="$STAGE2_INSTALL_DIR/bin/ld.lld" \
		-DLLVM_ENABLE_LIBCXX=ON \
		-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
		-DLLVM_EXTERNAL_PROJECTS="cod" \
		-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
		-DLLVM_TARGETS_TO_BUILD="$COD_TARGETS_TO_BUILD" \
		-DCMAKE_BUILD_TYPE="$COD_BUILD_TYPE" \
		"${STAGE_COMMON_CMAKE_OPTS[@]}" \
		-DLLVM_ENABLE_PROJECTS="clang;lld;lldb"
	cd "$COD_SOURCE_DIR"
	cmake --build ./build
	cmake --install ./build
	# Retain bootstrap outputs for incremental repair and reproducible diagnostics.

	exit
} # unreachable here and after
