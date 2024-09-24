#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
    # macOS
    HOST_THREADS=$(sysctl -n hw.ncpu)

    OS_SPEC_FLAGS="\
-DDEFAULT_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/ \
-DCOMPILER_RT_ENABLE_IOS=OFF \
-DCOMPILER_RT_ENABLE_WATCHOS=OFF \
-DCOMPILER_RT_ENABLE_TVOS=OFF \
-DLLVM_CREATE_XCODE_TOOLCHAIN=OFF \
	"

else
    # Linux (assuming Ubuntu)
    HOST_THREADS=$(nproc)

	OS_SPEC_FLAGS=""

fi


# pull llvm-project repo aside us
if [ -d "./llvm-project/.git" ]; then
	git -C "./llvm-project" pull
else
	git clone --depth 1 -b release/18.x https://github.com/llvm/llvm-project.git "./llvm-project"
fi


# generate build tree
#   do favor to vscode-clangd by `-DCMAKE_EXPORT_COMPILE_COMMANDS=1``
BUILD_DIR="build-$(uname -m)-$(uname -s)"
rm -rf $BUILD_DIR; mkdir -p $BUILD_DIR
# cf.
#  https://github.com/Homebrew/homebrew-core/blob/da26dd20d93fea974312a0177989178f0a28d211/Formula/l/llvm.rb#L107-L160
#
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DLLVM_ENABLE_IDE=ON \
	-DLLVM_EXTERNAL_PROJECTS="cod" \
 	-DLLVM_EXTERNAL_COD_SOURCE_DIR="../cod-project" \
	-DLLVM_ENABLE_PROJECTS="clang" \
	-DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
	-DLLVM_LINK_LLVM_DYLIB=ON \
	-DLLVM_BUILD_EXTERNAL_COMPILER_RT=ON \
	-DLLVM_ENABLE_LIBCXX=ON \
	-DLIBCXX_INSTALL_MODULES=ON \
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
	-S "./llvm-project/llvm" -B $BUILD_DIR


# do build
cd $BUILD_DIR
# spare 2 out of all available hardware threads
NUM_JOBS=$(( HOST_THREADS <= 3 ? 1 : (HOST_THREADS - 2) ))
ninja -j${NUM_JOBS}
