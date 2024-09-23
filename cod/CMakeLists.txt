
set( LLVM_LINK_COMPONENTS
  Native CACHE STRING
  Core
  LineEditor
  Option
  OrcJIT
  Support
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
