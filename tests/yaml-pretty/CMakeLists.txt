cmake_minimum_required(VERSION 3.20)

# Stand-alone CMake project for the `yaml-pretty` utility.  This mirrors the
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

project(yaml_pretty CXX)

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
# Build yaml-pretty executable
# ---------------------------------------------------------------------------
add_executable(yaml-pretty src/yaml_pretty.cc)

target_link_libraries(yaml-pretty PRIVATE ${SHILOS_LIBRARY})
# Link C++ runtime explicitly when found
if(LIBCXX_LIBRARY)
  target_link_libraries(yaml-pretty PRIVATE ${LIBCXX_LIBRARY})
endif()
if(LIBCXXABI_LIBRARY)
  target_link_libraries(yaml-pretty PRIVATE ${LIBCXXABI_LIBRARY})
endif()

# Ensure the loader can locate shared libs at runtime
set_target_properties(yaml-pretty PROPERTIES
  INSTALL_RPATH "${COD_TOOLCHAIN_DIR}/lib"
  BUILD_WITH_INSTALL_RPATH TRUE
)

if(APPLE)
  set_target_properties(yaml-pretty PROPERTIES
    BUILD_RPATH "${COD_TOOLCHAIN_DIR}/lib"
    INSTALL_NAME_DIR "@rpath"
  )
endif() 
