
add_clang_tool( codp
  main.cc
  )

target_link_libraries( codp PRIVATE
  shilos
  )
