add_clang_library( shilos SHARED
  shilos.cc
  di.cc
)

# LLVM DebugInfo dependencies for enhanced stack traces
llvm_map_components_to_libnames(llvm_debuginfo_libs
  DebugInfoDWARF
  Object
  Support
  )

target_link_libraries(shilos PUBLIC ${llvm_debuginfo_libs})

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
