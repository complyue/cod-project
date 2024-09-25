
#
# Note:
#   we (cod) are being built as an external project of llvm, so:
#     ${CMAKE_SOURCE_DIR} is llvm-project/llvm
#     clang src lives at llvm-project/clang
#   clang is to be built before us, and its intermediate artifacts (.inc e.g.)
#   will be built under ${CMAKE_BINARY_DIR}/tools/clang/
#

#
# compile us (cod) with the fresh built clang, rather than the system compiler
#
set(CMAKE_C_COMPILER "${CMAKE_BINARY_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${CMAKE_BINARY_DIR}/bin/clang++")

#
# we depends on clang within the llvm-project repo checked out, not system one
#
include_directories(
  "${CMAKE_BINARY_DIR}/tools/clang/include"
  "${CMAKE_SOURCE_DIR}/../clang/include"
  )

add_subdirectory(cod)
