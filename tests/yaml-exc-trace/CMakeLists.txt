cmake_minimum_required(VERSION 3.20)

# Stand-alone CMake project for the `yaml-exc-trace` utility.  This mirrors the
# structure used in tests/shilos and tests/yaml-cmp so that we can build against the *installed*
# shilos toolchain located under <project>/built/.

# Path constants -------------------------------------------------------------
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${COD_PROJECT_SOURCE_DIR}/tests/cmake/CodTestToolchain.cmake")

project(yaml_exc_trace_tests CXX)

# ---------------------------------------------------------------------------
# Build yaml-exc-trace-test executable with Debug info for source line/col in stack traces
# ---------------------------------------------------------------------------
# Set build type to Debug for consistent debug info
set(CMAKE_BUILD_TYPE Debug)

# Use standard debug flags with explicit dSYM generation
add_executable(yaml-exc-trace-test src/yaml_exc_trace_test.cc)
if(CMAKE_HOST_WIN32)
  target_compile_options(yaml-exc-trace-test PRIVATE /Zi /Od)
else()
  target_compile_options(yaml-exc-trace-test PRIVATE -g -O0)
endif()

# Ensure LLVM dsymutil is used via proper PATH setup (only on APPLE platforms)
if(APPLE)
    add_custom_command(TARGET yaml-exc-trace-test POST_BUILD
        COMMAND ${COD_TOOLCHAIN_DIR}/bin/dsymutil $<TARGET_FILE:yaml-exc-trace-test>
        COMMENT "Generating dSYM file for yaml-exc-trace-test"
        VERBATIM
    )
endif()

cod_link_shilos(yaml-exc-trace-test)
# ---------------------------------------------------------------------------
# Build dump_debug_info_test executable with Debug info for comprehensive debug info testing
# ---------------------------------------------------------------------------
add_executable(dump_debug_info_test src/dump_debug_info_test.cc)
if(CMAKE_HOST_WIN32)
  target_compile_options(dump_debug_info_test PRIVATE /Zi /Od)
else()
  target_compile_options(dump_debug_info_test PRIVATE -g -O0)
endif()
cod_link_shilos(dump_debug_info_test)

if(APPLE)
  set_target_properties(dump_debug_info_test PROPERTIES
    BUILD_RPATH "${COD_TOOLCHAIN_DIR}/lib"
    INSTALL_NAME_DIR "@rpath"
  )
  add_custom_command(TARGET dump_debug_info_test POST_BUILD
    COMMAND ${COD_TOOLCHAIN_DIR}/bin/dsymutil $<TARGET_FILE:dump_debug_info_test>
    COMMENT "Generating dSYM file for dump_debug_info_test"
    VERBATIM
  )
endif()
# ---------------------------------------------------------------------------
if(APPLE)
  set_target_properties(yaml-exc-trace-test PROPERTIES
    BUILD_RPATH "${COD_TOOLCHAIN_DIR}/lib"
    INSTALL_NAME_DIR "@rpath"
  )
endif()
