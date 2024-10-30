
add_clang_tool( codp
  main.cc
  )

clang_target_link_libraries( codp PRIVATE
  shilos
  )
