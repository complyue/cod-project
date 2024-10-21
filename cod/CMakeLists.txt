
set( LLVM_LINK_COMPONENTS
  Native
  Core
  LineEditor
  Option
  OrcJIT
  Support
  )

add_clang_tool( cod
  main.cc
  clang-repl.cc
  )

clang_target_link_libraries( cod PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangInterpreter
  )
