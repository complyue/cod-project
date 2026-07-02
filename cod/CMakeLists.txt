
set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Core
  LineEditor
  Option
  OrcJIT
  Support
  )

add_clang_tool( cod
  main.cc
  cod_cache.cc
  cod_workspace.cc
  )

target_link_libraries( cod PRIVATE
  shilos
  )

clang_target_link_libraries( cod PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangInterpreter
  clangLex
  clangParse
  clangSema
  clangSerialization
  )
