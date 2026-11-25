
set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  BitWriter
  Core
  Support
  TargetParser
  )

add_clang_tool( cod
  main.cc
  )

clang_target_link_libraries( cod PRIVATE
  clangBasic
  )
