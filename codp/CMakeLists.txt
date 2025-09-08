
add_clang_tool( codp
  main.cc
  codp_utils.cc
  codp_init.cc
  codp_add.cc
  codp_rm.cc
  codp_solve.cc
  )

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # Ensure LLVM dsymutil is used via proper PATH setup (only on APPLE platforms)
    if(APPLE)
            add_custom_command(TARGET codp POST_BUILD
                COMMAND ${CMAKE_BINARY_DIR}/bin/dsymutil $<TARGET_FILE:codp>
                COMMENT "Generating dSYM file for codp"
                VERBATIM
            )
    endif()
endif()

target_link_libraries( codp PRIVATE
  shilos
  )
