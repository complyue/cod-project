cmake_minimum_required(VERSION 3.20)

# Stand-alone CMake project for the `yaml-exc-trace` utility.  This mirrors the
# structure used in tests/shilos and tests/yaml-cmp so that we can build against the *installed*
# shilos toolchain located under <project>/built/.

# Path constants -------------------------------------------------------------
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# Determine toolchain directory based on COD_TEST_TOOLCHAIN environment variable
# Default to "build" for development workflow, can be set to "built" for pre-release testing
if(DEFINED ENV{COD_TEST_TOOLCHAIN})
    set(COD_TEST_TOOLCHAIN $ENV{COD_TEST_TOOLCHAIN})
else()
    set(COD_TEST_TOOLCHAIN "build")
endif()

if(COD_TEST_TOOLCHAIN STREQUAL "build")
    set(COD_TOOLCHAIN_DIR "${COD_PROJECT_SOURCE_DIR}/build")
    set(COD_INCLUDE_DIR "${COD_PROJECT_SOURCE_DIR}/include")
    message(STATUS "Using development toolchain from build/ directory")
    message(STATUS "Using source headers from include/ directory")
elseif(COD_TEST_TOOLCHAIN STREQUAL "built")
    set(COD_TOOLCHAIN_DIR "${COD_PROJECT_SOURCE_DIR}/built")
    set(COD_INCLUDE_DIR "${COD_TOOLCHAIN_DIR}/include")
    message(STATUS "Using installed toolchain from built/ directory")
    message(STATUS "Using installed headers from built/include/ directory")
else()
    message(FATAL_ERROR "COD_TEST_TOOLCHAIN must be 'build' or 'built', got: ${COD_TEST_TOOLCHAIN}")
endif()

# Use the freshly built Clang/C++ toolchain from the project build tree
set(CMAKE_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang++")

project(yaml_exc_trace_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# ---------------------------------------------------------------------------
# Header & library search paths
# ---------------------------------------------------------------------------
include_directories(BEFORE "${COD_INCLUDE_DIR}")
link_directories(BEFORE "${COD_TOOLCHAIN_DIR}/lib")

# Locate shilos shared library built by main project
find_library(SHILOS_LIBRARY
    NAMES shilos libshilos
    PATHS "${COD_TOOLCHAIN_DIR}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

# libc++ and libc++abi from toolchain (may be static or dynamic depending on build)
find_library(LIBCXX_LIBRARY c++ PATHS "${COD_TOOLCHAIN_DIR}/lib")
find_library(LIBCXXABI_LIBRARY c++abi PATHS "${COD_TOOLCHAIN_DIR}/lib")

# ---------------------------------------------------------------------------
# Build yaml-exc-trace-test executable with Debug info for source line/col in stack traces
# ---------------------------------------------------------------------------
# Set build type to Debug for consistent debug info
set(CMAKE_BUILD_TYPE Debug)

# Use standard debug flags with explicit dSYM generation
add_executable(yaml-exc-trace-test src/yaml_exc_trace_test.cc)
target_compile_options(yaml-exc-trace-test PRIVATE -g -O0)

# Ensure LLVM dsymutil is used via proper PATH setup (only on APPLE platforms)
if(APPLE)
    add_custom_command(TARGET yaml-exc-trace-test POST_BUILD
        COMMAND ${COD_TOOLCHAIN_DIR}/bin/dsymutil $<TARGET_FILE:yaml-exc-trace-test>
        COMMENT "Generating dSYM file for yaml-exc-trace-test"
        VERBATIM
    )
endif()

target_link_libraries(yaml-exc-trace-test PRIVATE ${SHILOS_LIBRARY})
# Link C++ runtime explicitly when found
if(LIBCXX_LIBRARY)
  target_link_libraries(yaml-exc-trace-test PRIVATE ${LIBCXX_LIBRARY})
endif()
if(LIBCXXABI_LIBRARY)
  target_link_libraries(yaml-exc-trace-test PRIVATE ${LIBCXXABI_LIBRARY})
endif()

# Ensure the loader can locate shared libs at runtime
set_target_properties(yaml-exc-trace-test PROPERTIES
  INSTALL_RPATH "${COD_TOOLCHAIN_DIR}/lib"
  BUILD_WITH_INSTALL_RPATH TRUE
)
# ---------------------------------------------------------------------------
# Build dump_debug_info_test executable with Debug info for comprehensive debug info testing
# ---------------------------------------------------------------------------
add_executable(dump_debug_info_test src/dump_debug_info_test.cc)
target_compile_options(dump_debug_info_test PRIVATE -g -O0)
target_link_libraries(dump_debug_info_test PRIVATE ${SHILOS_LIBRARY})
if(LIBCXX_LIBRARY)
  target_link_libraries(dump_debug_info_test PRIVATE ${LIBCXX_LIBRARY})
endif()
if(LIBCXXABI_LIBRARY)
  target_link_libraries(dump_debug_info_test PRIVATE ${LIBCXXABI_LIBRARY})
endif()

# Ensure the loader can locate shared libs at runtime
set_target_properties(dump_debug_info_test PROPERTIES
  INSTALL_RPATH "${COD_TOOLCHAIN_DIR}/lib"
  BUILD_WITH_INSTALL_RPATH TRUE
)

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
