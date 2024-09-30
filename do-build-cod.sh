#!/usr/bin/env bash

#
# This script does checkout the corresponding llvm-project branch, 2-stage
# build CoD as an external project of LLVM, to form a toolset bundled with
# llvm, clang, lld, libc++ and etc.
#

# certain CoD branch should persist to a specific LLVM branche
LLVM_BRANCH=release/18.x

# build type for CoD
COD_BUILD_TYPE=Release # or RelWithDebInfo, or Debug

{ # keep body of cmds in this single command group, so they execute as a whole
	# thus edit-during-(long)-execution and partial downloads are not problems
	set -e
	cd $(dirname "$0")
	COD_SOURCE_DIR=$(pwd)

	# suffix the build dir, so devcontainers/Docker and native build dirs can
	# coexist on macOS
	BUILD_DIR="build-$(uname -m)-$(uname -s)"
	# vscode-clangd expects build/compile_commands.json
	test -L build || ln -s "$BUILD_DIR/cod" build
	# use full path for build dir
	BUILD_DIR="$COD_SOURCE_DIR/$BUILD_DIR"

	if [ "$(uname)" == "Darwin" ]; then
		# macOS
		HOST_NTHREADS=$(sysctl -n hw.ncpu)

		# use system's libc++ on macOS
		COD_RT_LIBS="compiler-rt"
		OS_SPEC_CMAKE_OPTS=(
			-DDEFAULT_SYSROOT="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
			-DCOMPILER_RT_ENABLE_IOS=OFF
			-DCOMPILER_RT_ENABLE_WATCHOS=OFF
			-DCOMPILER_RT_ENABLE_TVOS=OFF
			-DLLVM_CREATE_XCODE_TOOLCHAIN=OFF
		)

		# at stage2, some tools built and used early (i.e. before stage2 rt libs)
		# need this, or they may fail to run due to broken rt linkage
		export DYLD_LIBRARY_PATH="$BUILD_DIR/stage1rt/lib"

	# TODO: support more OSes
	else
		# assuming Ubuntu
		HOST_NTHREADS=$(nproc)

		# build and bundle our own libc++ and etc.
		COD_RT_LIBS="libunwind;libcxxabi;libcxx;compiler-rt"
		OS_SPEC_CMAKE_OPTS=(
			-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF
			-DCMAKE_POSITION_INDEPENDENT_CODE=ON
			-DLIBCXX_USE_COMPILER_RT=ON
			-DLIBCXX_HAS_ATOMIC_LIB=OFF
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

		# at stage2, some tools built and used early (i.e. before stage2 rt libs)
		# need this, or they may fail to run due to broken rt linkage
		export LD_LIBRARY_PATH="$BUILD_DIR/stage1rt/lib"

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
		# we don't build and bundle libc, treating CRT and the system C
		# compiler, either GCC or (Apple)Clang, as part of the OS rather than
		# part of a toolset
		-DLLVM_ENABLE_PROJECTS="clang;lld"
		-DLLVM_ENABLE_RUNTIMES="$COD_RT_LIBS"

		# prefer libc++
		-DCLANG_DEFAULT_CXX_STDLIB="libc++"
		# embrace c++20 modules
		-DLIBCXX_INSTALL_MODULES=ON

		# various CoD languages/runtimes can share the LLVM lib at runtime
		-DLLVM_LINK_LLVM_DYLIB=ON

		-DLLVM_ENABLE_EH=ON
		-DLLVM_ENABLE_FFI=ON
		-DLLVM_ENABLE_RTTI=ON

		-DLLVM_INCLUDE_DOCS=OFF
		-DLLVM_INCLUDE_TESTS=OFF
		-DLLVM_INSTALL_UTILS=OFF
		-DLLVM_OPTIMIZED_TABLEGEN=ON

		${OS_SPEC_CMAKE_OPTS[@]}

		-DLLVM_ENABLE_IDE=ON
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1
		-DLLVM_TARGETS_TO_BUILD="Native"
		-G
		Ninja
		-S
		"$COD_SOURCE_DIR/llvm-project/llvm"
	)

	# spare 2 out of all available hardware threads
	NJOBS=$((HOST_NTHREADS <= 3 ? 1 : (HOST_NTHREADS - 2)))

	# stage-1: build clang with system compiler toolchain,
	#          bundling lld, libc++ etc. with it
	#
	#   we install stage1 built rt libs into stage1rt, so stage2 tools can
	#   dynamically link with them, this is crucial for llvm-min-tblgen and
	#   some others tools, those built and used before rt libs (e.g. libc++)
	#   are built at stage2
	#
	#   but we don't install libLLVM.so and other libs, they'd possibly expose
	#   libstdc++ based APIs (in case stage1 used g++/libstdc++), thus stage2
	#   tools won't rt link properly
	#
	test -x "$BUILD_DIR/stage1/bin/clang++" || (
		mkdir -p "$BUILD_DIR/stage1rt"
		mkdir -p "$BUILD_DIR/stage1"
		cd "$BUILD_DIR/stage1"
		cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/stage1rt" \
			-DCMAKE_BUILD_TYPE=Release \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		ninja -j${NJOBS}
		ninja -j${NJOBS} install-runtimes
	)

	# stage-2: build CoD toolset with stage1 clang,
	#          bundling clang, lld, libc++ etc. with it
	#
	#   we do HAVE_UNW_ADD_DYNAMIC_FDE=1 per:
	#     https://github.com/llvm/llvm-project/issues/43419
	#
	test -x "$BUILD_DIR/cod/bin/cod" || (
		mkdir -p "$BUILD_DIR/cod"
		cd "$BUILD_DIR/cod"
		cmake -DCMAKE_PREFIX_PATH="$BUILD_DIR/stage1rt;$BUILD_DIR/stage1" \
			-DCMAKE_C_COMPILER="clang" \
			-DCMAKE_CXX_COMPILER="clang++" \
			-DLLVM_USE_LINKER=lld \
			-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
			-DLLVM_EXTERNAL_PROJECTS="cod" \
			-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
			-DCMAKE_BUILD_TYPE="$COD_BUILD_TYPE" \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		ninja -j${NJOBS}
	)

	exit
} # unreachable here and after
