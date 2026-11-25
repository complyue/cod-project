
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
set_target_properties(cod PROPERTIES OUTPUT_NAME cod-seed)

clang_target_link_libraries( cod PRIVATE
  clangBasic
  )
