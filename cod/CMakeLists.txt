
set( LLVM_LINK_COMPONENTS
  Native CACHE STRING
  Core
  LineEditor
  Option
  OrcJIT
  Support
  )

include_directories(
  "${CLANG_BINARY_DIR}/include"
  "${CLANG_SOURCE_DIR}/include"
  )
add_clang_tool( cod
  cod.cc
  )
clang_target_link_libraries( cod PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangInterpreter
  )
