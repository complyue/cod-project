add_clang_library( shilos SHARED
  shilos.cc
  )

# Install shilos headers into the toolchain
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/shilos"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT shilos
  FILES_MATCHING PATTERN "*.hh"
  )

# Install the main shilos.hh header
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../include/shilos.hh"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT shilos
  )
