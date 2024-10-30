
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

#
# Note:
#   we (cod) are being built as an external project of llvm, so:
#     ${CMAKE_SOURCE_DIR} is llvm-project/llvm
#     clang src lives at llvm-project/clang
#   clang is to be built before us, and its intermediate artifacts (.inc e.g.)
#   will be built under ${CMAKE_BINARY_DIR}/tools/clang/
#
# we depends on clang/libcxx/compiler-rt etc. from our local checkout of
# llvm-project repo, not from system installation
#
include_directories( BEFORE
  "${CMAKE_BINARY_DIR}/tools/clang/include"
  "${CMAKE_SOURCE_DIR}/../clang/include"
  )
# libcxx etc. should have been built there
link_directories( BEFORE
  "${CMAKE_BINARY_DIR}/lib"
)

include_directories( BEFORE
  include
  )

add_subdirectory(shilos)
add_subdirectory(cod)
add_subdirectory(codp)
