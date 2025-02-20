#!/usr/bin/env bash

#
# This script does checkout the corresponding llvm-project branch, 3-stage
# build CoD as an external project of LLVM, to form a toolset bundled with
# llvm, clang, lld, libc++ and etc.
#

# certain CoD branch should persist to a specific LLVM branche
LLVM_BRANCH=release/18.x

# LLVM targets to be supported by CoD
COD_TARGETS_TO_BUILD="Native"
# on Cuda ready OSes, include NVPTX target for Cuda support
# COD_TARGETS_TO_BUILD="Native;NVPTX"

# build type for CoD
: ${COD_BUILD_TYPE:=Release} # or RelWithDebInfo, or Debug

{ # keep body of cmds in this single command group, so they execute as a whole
	# thus edit-during-(long)-execution and partial downloads are not problems
	set -e
	cd $(dirname "$0")
	COD_SOURCE_DIR=$(pwd)

	# suffix the build dir, so devcontainers/Docker and native build dirs can
	# coexist on macOS
	BUILD_DIR="build-$(uname -m)-$(uname -s)"
	# vscode-clangd expects build/compile_commands.json, and stage3 build tree
	# is the final env for CoD development
	test -L build || ln -s "$BUILD_DIR/stage3" build
	# use full path for build dir
	BUILD_DIR="$COD_SOURCE_DIR/$BUILD_DIR"

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
			# note we patch
			#   clang/lib/Driver/ToolChains/Darwin.cpp
			# to have the bundled libs being the default
			#
			-DLLVM_ENABLE_RUNTIMES="libcxxabi;libcxx;compiler-rt"
			-DLIBCXXABI_USE_LLVM_UNWINDER=OFF
			-DLIBCXXABI_ENABLE_STATIC_UNWINDER=OFF
			-DCOMPILER_RT_USE_LLVM_UNWINDER=OFF
		)

	# TODO: support more OSes
	else
		# assuming Ubuntu
		HOST_NTHREADS=$(nproc)

		OS_SPEC_CMAKE_OPTS=(
			-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF
			-DLIBCXX_HAS_ATOMIC_LIB=OFF

			#
			# build our own libc++ suite and compiler-rt on Linux
			#
			# note we patch
			#   clang/lib/Driver/ToolChains/Gnu.cpp
			# to have the bundled libs being the default
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

	fi

	# pull llvm-project repo here
	# not to put it in ../ so we work in a single host dir, in case of devcontainers
	if [ -d "./llvm-project/.git" ]; then
		git -C "./llvm-project" pull
	else
		git clone --depth 1 -b "$LLVM_BRANCH" https://github.com/llvm/llvm-project.git "./llvm-project" && (
			cd llvm-project
			find "../patches/llvm/$LLVM_BRANCH" -type f -name "*.patch" -print0 |
				sort -z | xargs -0 -I{} git apply "{}"
		)
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
		-DLLVM_INSTALL_UTILS=OFF
		-DLLVM_OPTIMIZED_TABLEGEN=ON

		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DLLVM_ENABLE_IDE=ON
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1

		${OS_SPEC_CMAKE_OPTS[@]}

		-G
		Ninja
		-S
		"$COD_SOURCE_DIR/llvm-project/llvm"
	)

	# spare 2 out of all available hardware threads
	NJOBS=$((HOST_NTHREADS <= 3 ? 1 : (HOST_NTHREADS - 2)))

	#
	# stage-2: build CoD toolset with stage1 clang then install it
	#
	#   we always (even on Linux) do HAVE_UNW_ADD_DYNAMIC_FDE=1 per:
	#     https://github.com/llvm/llvm-project/issues/43419
	#
	test -x "$BUILD_DIR/cod/bin/cod" || (
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
		test -x "$BUILD_DIR/stage1/bin/clang++" || (
			mkdir -p "$BUILD_DIR/stage1" && cd "$BUILD_DIR/stage1"
			cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/cod-rt" \
				-DLLVM_TARGETS_TO_BUILD=Native \
				-DCMAKE_BUILD_TYPE=Release \
				"${STAGE_COMMON_CMAKE_OPTS[@]}"
			ninja -j${NJOBS}
			ninja -j${NJOBS} install-runtimes
		)

		mkdir -p "$BUILD_DIR/stage2" && cd "$BUILD_DIR/stage2"
		cp -rf ../cod-rt/lib ./
		cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/cod" \
			-DCMAKE_PREFIX_PATH="$BUILD_DIR/stage1" \
			-DCMAKE_C_COMPILER="$BUILD_DIR/stage1/bin/clang" \
			-DCMAKE_CXX_COMPILER="$BUILD_DIR/stage1/bin/clang++" \
			-DCMAKE_LINKER="$BUILD_DIR/stage1/bin/ld.lld" \
			-DLLVM_ENABLE_LIBCXX=ON \
			-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
			-DLLVM_EXTERNAL_PROJECTS="cod" \
			-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
			-DLLVM_TARGETS_TO_BUILD=Native \
			-DCMAKE_BUILD_TYPE=Release \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		ninja -j${NJOBS}
		ninja -j${NJOBS} install
	)

	#
	# stage-3: build the final CoD toolset, with itself (stage2 installation),
	#          then overwrite the installation
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
	mkdir -p "$BUILD_DIR/stage3" && cd "$BUILD_DIR/stage3"
	test -d ./lib || cp -rf ../cod-rt/lib ./
	cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/cod" \
		-DCMAKE_PREFIX_PATH="$BUILD_DIR/cod" \
		-DCMAKE_C_COMPILER="$BUILD_DIR/cod/bin/clang" \
		-DCMAKE_CXX_COMPILER="$BUILD_DIR/cod/bin/clang++" \
		-DCMAKE_LINKER="$BUILD_DIR/cod/bin/ld.lld" \
		-DLLVM_ENABLE_LIBCXX=ON \
		-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
		-DLLVM_EXTERNAL_PROJECTS="cod" \
		-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
		-DLLVM_TARGETS_TO_BUILD="$COD_TARGETS_TO_BUILD" \
		-DCMAKE_BUILD_TYPE="$COD_BUILD_TYPE" \
		"${STAGE_COMMON_CMAKE_OPTS[@]}" \
		-DLLVM_ENABLE_PROJECTS="clang;lld;lldb"
	ninja -j${NJOBS}
	ninja -j${NJOBS} install

	# cleanup artifacts not needed anymore
	rm -rf "$BUILD_DIR"/{cod-rt,stage1,stage2} || true

	exit
} # unreachable here and after
