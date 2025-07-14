cmake_minimum_required(VERSION 3.20)

# Stand-alone CMake project for the `yaml-cmp` test utility.  This mirrors the
# structure used in tests/shilos so that we can build against the *installed*
# shilos toolchain located under <project>/built/.

# Path constants -------------------------------------------------------------
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(COD_TOOLCHAIN_DIR "${COD_PROJECT_SOURCE_DIR}/built")

# Use the freshly built Clang/C++ toolchain from the project build tree
set(CMAKE_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang++")

project(yaml_cmp_test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# ---------------------------------------------------------------------------
# Header & library search paths
# ---------------------------------------------------------------------------
include_directories(BEFORE "${COD_TOOLCHAIN_DIR}/include")
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
# Build yaml-cmp executable
# ---------------------------------------------------------------------------
add_executable(yaml-cmp yaml_cmp.cc)

target_link_libraries(yaml-cmp PRIVATE ${SHILOS_LIBRARY})
# Link C++ runtime explicitly when found
if(LIBCXX_LIBRARY)
  target_link_libraries(yaml-cmp PRIVATE ${LIBCXX_LIBRARY})
endif()
if(LIBCXXABI_LIBRARY)
  target_link_libraries(yaml-cmp PRIVATE ${LIBCXXABI_LIBRARY})
endif()

# Ensure the loader can locate shared libs at runtime
set_target_properties(yaml-cmp PROPERTIES
  INSTALL_RPATH "${COD_TOOLCHAIN_DIR}/lib"
  BUILD_WITH_INSTALL_RPATH TRUE
)

if(APPLE)
  set_target_properties(yaml-cmp PROPERTIES
    BUILD_RPATH "${COD_TOOLCHAIN_DIR}/lib"
    INSTALL_NAME_DIR "@rpath"
  )
endif() 
