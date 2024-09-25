#!/usr/bin/env bash

#
# This script does checkout the corresponding llvm-project branch, 2-stage
# build cod as an external project of LLVM, together with a toolset bundled,
# including clang, lld, libcxx and etc.
#
# for LLVM cmake config tweaks, cf.
#  https://github.com/Homebrew/homebrew-core/blob/da26dd20d93fea974312a0177989178f0a28d211/Formula/l/llvm.rb
#

LLVM_BRANCH=release/18.x

if [ "$(uname)" == "Darwin" ]; then
    # macOS
    HOST_NTHREADS=$(sysctl -n hw.ncpu)

    OS_SPEC_FLAGS="\
-DDEFAULT_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk \
-DCOMPILER_RT_ENABLE_IOS=OFF \
-DCOMPILER_RT_ENABLE_WATCHOS=OFF \
-DCOMPILER_RT_ENABLE_TVOS=OFF \
-DLLVM_CREATE_XCODE_TOOLCHAIN=OFF \
	"

# TODO: support more OSes
else
    # assuming Ubuntu
    HOST_NTHREADS=$(nproc)

	OS_SPEC_FLAGS="\
-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF \
-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
-DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
-DLIBCXX_STATICALLY_LINK_ABI_IN_SHARED_LIBRARY=OFF \
-DLIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY=ON \
-DLIBCXX_USE_COMPILER_RT=ON \
-DLIBCXX_HAS_ATOMIC_LIB=OFF \
-DLIBCXXABI_ENABLE_STATIC_UNWINDER=ON \
-DLIBCXXABI_STATICALLY_LINK_UNWINDER_IN_SHARED_LIBRARY=OFF \
-DLIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY=ON \
-DLIBCXXABI_USE_COMPILER_RT=ON \
-DLIBCXXABI_USE_LLVM_UNWINDER=ON \
-DLIBUNWIND_USE_COMPILER_RT=ON \
-DCOMPILER_RT_USE_BUILTINS_LIBRARY=ON \
-DCOMPILER_RT_USE_LLVM_UNWINDER=ON \
-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
	"

fi


# pull llvm-project repo here
# not to put it in ../ so we work in a single host dir, in case of devcontainers
if [ -d "./llvm-project/.git" ]; then
	git -C "./llvm-project" pull
else
	git clone --depth 1 -b "$LLVM_BRANCH" https://github.com/llvm/llvm-project.git "./llvm-project"
fi


# spare 2 out of all available hardware threads
NJOBS=$(( HOST_NTHREADS <= 3 ? 1 : (HOST_NTHREADS - 2) ))

# suffix the build dir, so devcontainers/Docker and native build dirs can
# coexist on macOS
BUILD_DIR="build-$(uname -m)-$(uname -s)"
# (re)start with a fresh build dir
rm -rf "$BUILD_DIR"; mkdir -p "$BUILD_DIR"
# vscode-clangd expects build/compile_commands.json
rm build; ln -s "$BUILD_DIR" build
# do build in build dir
cd "$BUILD_DIR"

# 2-stage build for cod with clang/lld bundled
cmake -DCLANG_ENABLE_BOOTSTRAP=ON \
	-DBOOTSTRAP_LLVM_EXTERNAL_PROJECTS="cod" \
 	-DBOOTSTRAP_LLVM_EXTERNAL_COD_SOURCE_DIR=".." \
	-DLLVM_ENABLE_PROJECTS="clang;lld" \
	-DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
	-DLLVM_BUILD_EXTERNAL_COMPILER_RT=ON \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=0 \
	-DBOOTSTRAP_CMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DBOOTSTRAP_LLVM_USE_LINKER=lld \
	-DCLANG_DEFAULT_CXX_STDLIB=libc++ \
	-DLIBCXX_ENABLE_SHARED=ON \
	-DLIBCXX_ENABLE_STATIC=OFF \
	-DLIBCXX_INSTALL_MODULES=ON \
	-DLLVM_LINK_LLVM_DYLIB=ON \
	-DLLVM_ENABLE_IDE=ON \
	-DLLVM_ENABLE_EH=ON \
	-DLLVM_ENABLE_FFI=ON \
	-DLLVM_ENABLE_RTTI=ON \
	-DLLVM_INCLUDE_DOCS=OFF \
	-DLLVM_INCLUDE_TESTS=OFF \
	-DLLVM_INSTALL_UTILS=OFF \
	-DLLVM_OPTIMIZED_TABLEGEN=ON \
	-DCLANG_FORCE_MATCHING_LIBCLANG_SOVERSION=OFF \
	$OS_SPEC_FLAGS \
	-DLLVM_TARGETS_TO_BUILD="Native" \
	-DCMAKE_BUILD_TYPE=Release -G Ninja \
	-S "../llvm-project/llvm"

ninja -j${NJOBS} stage2
