#!/usr/bin/env bash

#
# This script does checkout the corresponding llvm-project branch, 2-stage
# build cod as an external project of LLVM, together with a toolset bundled,
# including clang, lld, libcxx and etc.
#
# for LLVM cmake config tweaks, cf.
#  https://github.com/Homebrew/homebrew-core/blob/da26dd20d93fea974312a0177989178f0a28d211/Formula/l/llvm.rb
#


# certain COD branch should correspond to a specific LLVM branche
LLVM_BRANCH=release/18.x


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

    OS_SPEC_CMAKE_OPTS=(
-DDEFAULT_SYSROOT="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
-DCOMPILER_RT_ENABLE_IOS=OFF
-DCOMPILER_RT_ENABLE_WATCHOS=OFF
-DCOMPILER_RT_ENABLE_TVOS=OFF
-DLLVM_CREATE_XCODE_TOOLCHAIN=OFF
	)
	OS_SPEC_EXE_LINKER_FLAGS="-Wl,-rpath,@loader_path/../lib"
	OS_SPEC_SHARED_LINKER_FLAGS="-Wl,-rpath,@loader_path"

	# some early stage2 tools (e.g. llvm-min-tblgen) need libc++ from stage1,
	# as they have to run before stage2 libc++ is built.
	export DYLD_LIBRARY_PATH="$BUILD_DIR/stage1/lib"

# TODO: support more OSes
else
    # assuming Ubuntu
    HOST_NTHREADS=$(nproc)

	OS_SPEC_CMAKE_OPTS=(
-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF
-DCMAKE_POSITION_INDEPENDENT_CODE=ON
-DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON
-DLIBCXX_STATICALLY_LINK_ABI_IN_SHARED_LIBRARY=OFF
-DLIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY=ON
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
	OS_SPEC_EXE_LINKER_FLAGS="-Wl,-rpath,\$ORIGIN/../lib"
	OS_SPEC_SHARED_LINKER_FLAGS="-Wl,-rpath,\$ORIGIN"

	# some early stage2 tools (e.g. llvm-min-tblgen) need libc++ from stage1,
	# as they have to run before stage2 libc++ is built.
	export LD_LIBRARY_PATH="$BUILD_DIR/stage1/lib"
fi


# pull llvm-project repo here
# not to put it in ../ so we work in a single host dir, in case of devcontainers
if [ -d "./llvm-project/.git" ]; then
	git -C "./llvm-project" pull
else
	git clone --depth 1 -b "$LLVM_BRANCH" https://github.com/llvm/llvm-project.git "./llvm-project" && (
		cd llvm-project
		for p in "../patches/llvm/$LLVM_BRANCH/*.patch"; do
			git apply "$p"
		done
	)
fi


STAGE_COMMON_CMAKE_OPTS=(
	-DLLVM_ENABLE_PROJECTS="clang;lld"
	-DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind"
	-DLLVM_BUILD_EXTERNAL_COMPILER_RT=ON
	-DLIBCXX_ENABLE_SHARED=ON
	-DLIBCXX_ENABLE_STATIC=OFF
	-DLIBCXX_INSTALL_MODULES=ON
	-DLLVM_LINK_LLVM_DYLIB=ON
	-DLLVM_ENABLE_EH=ON
	-DLLVM_ENABLE_FFI=ON
	-DLLVM_ENABLE_RTTI=ON
	-DLLVM_INCLUDE_DOCS=OFF
	-DLLVM_INCLUDE_TESTS=OFF
	-DLLVM_INSTALL_UTILS=OFF
	-DLLVM_OPTIMIZED_TABLEGEN=ON
	"${OS_SPEC_CMAKE_OPTS[@]}"
	-DLLVM_TARGETS_TO_BUILD="Native"
	-DLLVM_ENABLE_IDE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	-DCMAKE_BUILD_TYPE=Release -G Ninja
	-S "$COD_SOURCE_DIR/llvm-project/llvm"
)


# spare 2 out of all available hardware threads
NJOBS=$(( HOST_NTHREADS <= 3 ? 1 : (HOST_NTHREADS - 2) ))

# stage-1: build clang with system compiler toolchain, bundling lld, libcxx etc. with it
test -x "$BUILD_DIR/stage1/bin/clang++" || (
	mkdir -p "$BUILD_DIR/stage1"
	cd "$BUILD_DIR/stage1"
	cmake "${STAGE_COMMON_CMAKE_OPTS[@]}"
	ninja -j${NJOBS}
)

# stage-2: build cod with stage1 clang, bundling clang, lld, libcxx etc. with it
test -x "$BUILD_DIR/cod/bin/cod" || (
	mkdir -p "$BUILD_DIR/cod"
	cd "$BUILD_DIR/cod"
	cmake -DCMAKE_C_COMPILER="$BUILD_DIR/stage1/bin/clang" \
		-DCMAKE_CXX_COMPILER="$BUILD_DIR/stage1/bin/clang++" \
		-DCMAKE_CXX_FLAGS="-stdlib=libc++" \
		-DCMAKE_EXE_LINKER_FLAGS="-L$BUILD_DIR/stage1/lib $OS_SPEC_EXE_LINKER_FLAGS" \
		-DCMAKE_SHARED_LINKER_FLAGS="-L$BUILD_DIR/stage1/lib $OS_SPEC_SHARED_LINKER_FLAGS" \
		-DCLANG_DEFAULT_CXX_STDLIB="libc++" \
		-DLLVM_ENABLE_LLD=ON \
		-DLLVM_EXTERNAL_PROJECTS="cod" \
		-DLLVM_EXTERNAL_COD_SOURCE_DIR="$COD_SOURCE_DIR" \
		"${STAGE_COMMON_CMAKE_OPTS[@]}"
	ninja -j${NJOBS}
)
