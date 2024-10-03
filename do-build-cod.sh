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
COD_BUILD_TYPE=Release # or RelWithDebInfo, or Debug

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

		# use system's libc++ on macOS
		COD_RT_LIBS="compiler-rt"
		OS_SPEC_CMAKE_OPTS=(
			-DDEFAULT_SYSROOT="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
			-DCOMPILER_RT_ENABLE_IOS=OFF
			-DCOMPILER_RT_ENABLE_WATCHOS=OFF
			-DCOMPILER_RT_ENABLE_TVOS=OFF
			-DLLVM_CREATE_XCODE_TOOLCHAIN=OFF
		)

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
		-G
		Ninja
		-S
		"$COD_SOURCE_DIR/llvm-project/llvm"
	)

	# spare 2 out of all available hardware threads
	NJOBS=$((HOST_NTHREADS <= 3 ? 1 : (HOST_NTHREADS - 2)))

	# stage-1: build clang with system compiler (no clang assumed) toolchain,
	#          bundling lld, libc++ etc. with it
	#
	#   we install just rt libs into stage1rt, at stage1 so stage2 tools can
	#   dynamically link with them, this is crucial for llvm-min-tblgen and
	#   some others tools, those built and used before rt libs (e.g. libc++)
	#   are built at stage2
	#
	#   we don't install all stuff (which'll include libLLVM.so and etc.), for
	#   they'd possibly expose libstdc++ based APIs (in case stage1 used
	#   g++/libstdc++), rendering stage2 tools unable to rt link properly
	#
	test -x "$BUILD_DIR/stage1/bin/clang++" || (
		mkdir -p "$BUILD_DIR/stage1rt"
		mkdir -p "$BUILD_DIR/stage1"
		cd "$BUILD_DIR/stage1"
		cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/stage1rt" \
			-DLLVM_TARGETS_TO_BUILD=Native \
			-DCMAKE_BUILD_TYPE=Release \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		ninja -j${NJOBS}
		ninja -j${NJOBS} install-runtimes
	)

	# stage-2: build CoD toolset with stage1 clang and install it
	#
	#   we do HAVE_UNW_ADD_DYNAMIC_FDE=1 per:
	#     https://github.com/llvm/llvm-project/issues/43419
	#
	test -x "$BUILD_DIR/cod/bin/cod" || (
		# to be able to rt link with depended runtime libs built & installed
		# from llvm source, at the previous stage. we'll build these rt libs
		# at this stage again, but some tools are built and used even earlier
		if [ "$(uname)" == "Darwin" ]; then
			export DYLD_LIBRARY_PATH="$BUILD_DIR/stage1rt/lib"
		else # assuming Ubuntu
			export LD_LIBRARY_PATH="$BUILD_DIR/stage1rt/lib"
		fi

		mkdir -p "$BUILD_DIR/stage2"
		cd "$BUILD_DIR/stage2"
		cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/cod" \
			-DCMAKE_PREFIX_PATH="$BUILD_DIR/stage1rt;$BUILD_DIR/stage1" \
			-DCMAKE_C_COMPILER="$BUILD_DIR/stage1/bin/clang" \
			-DCMAKE_CXX_COMPILER="$BUILD_DIR/stage1/bin/clang++" \
			-DLLVM_ENABLE_LIBCXX=ON \
			-DLLVM_USE_LINKER=lld \
			-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
			-DLLVM_EXTERNAL_PROJECTS="cod" \
			-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
			-DLLVM_TARGETS_TO_BUILD=Native \
			-DCMAKE_BUILD_TYPE=Release \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		ninja -j${NJOBS}
		ninja -j${NJOBS} install
	)

	# stage-3: build CoD toolset again, with itself built & installed at
	#          stage2, replace its previous installation by stage2
	#
	#   we do HAVE_UNW_ADD_DYNAMIC_FDE=1 per:
	#     https://github.com/llvm/llvm-project/issues/43419
	#
	test -x "$BUILD_DIR/stage3/bin/cod" || (
		# to be able to rt link with depended runtime libs built & installed
		# from llvm source, at the previous stage. we'll build these rt libs
		# at this stage again, but some tools are built and used even earlier
		#
		# note all libs (including libLLVM.so and etc.) are eligible to rt link
		# by stage3 tools, as they are built with clang++/libc++ at stage2
		if [ "$(uname)" == "Darwin" ]; then
			export DYLD_LIBRARY_PATH="$BUILD_DIR/cod/lib"
		else # assuming Ubuntu
			export LD_LIBRARY_PATH="$BUILD_DIR/cod/lib"
		fi

		mkdir -p "$BUILD_DIR/stage3"
		cd "$BUILD_DIR/stage3"
		cmake -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/cod" \
			-DCMAKE_PREFIX_PATH="$BUILD_DIR/cod" \
			-DCMAKE_C_COMPILER="$BUILD_DIR/cod/bin/clang" \
			-DCMAKE_CXX_COMPILER="$BUILD_DIR/cod/bin/clang++" \
			-DLLVM_ENABLE_LIBCXX=ON \
			-DLLVM_USE_LINKER=lld \
			-DHAVE_UNW_ADD_DYNAMIC_FDE=1 \
			-DLLVM_EXTERNAL_PROJECTS="cod" \
			-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
			-DLLVM_TARGETS_TO_BUILD="$COD_TARGETS_TO_BUILD" \
			-DCMAKE_BUILD_TYPE="$COD_BUILD_TYPE" \
			"${STAGE_COMMON_CMAKE_OPTS[@]}"
		ninja -j${NJOBS}
		ninja -j${NJOBS} install
	)

	# now clangd can properly index CoD toolset stuff, in building itself

	exit
} # unreachable here and after
