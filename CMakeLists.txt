
set(CMAKE_C_COMPILER "${CMAKE_BINARY_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${CMAKE_BINARY_DIR}/bin/clang++")

include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}/../clang/include"
  "${CMAKE_SOURCE_DIR}/../clang/include"
  )

add_subdirectory(cod)
