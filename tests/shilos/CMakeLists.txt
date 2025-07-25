cmake_minimum_required(VERSION 3.20)

# Canonicalize the project root directory
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

# Set the compilers to use the chosen toolchain
set(CMAKE_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang++")

project(cod_tests CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Configure build to use the chosen toolchain

# Include directories for project headers
include_directories(BEFORE "${COD_INCLUDE_DIR}")

# Add library search paths
link_directories(BEFORE "${COD_TOOLCHAIN_DIR}/lib")

# Find the shilos library
find_library(SHILOS_LIBRARY 
    NAMES shilos libshilos
    PATHS "${COD_TOOLCHAIN_DIR}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

# Find required libraries
find_library(LIBCXXABI_LIBRARY c++abi PATHS "${COD_TOOLCHAIN_DIR}/lib")
find_library(LIBCXX_LIBRARY c++ PATHS "${COD_TOOLCHAIN_DIR}/lib")

if(NOT SHILOS_LIBRARY)
    message(FATAL_ERROR "shilos library not found")
endif()

message(STATUS "Found shilos library: ${SHILOS_LIBRARY}")

# Function to create and configure a test executable
function(configure_cod_target target_name)
    # Get source files from remaining arguments
    set(source_files ${ARGN})
    
    # Create the executable
    add_executable(${target_name} ${source_files})
    
    # Link with shilos library
    target_link_libraries(${target_name} PRIVATE ${SHILOS_LIBRARY})
    
    # Link with C++ runtime
    target_link_libraries(${target_name} PRIVATE c++ c++abi)
    
    # Compile with debug information
    target_compile_options(${target_name} PRIVATE -g)
    
    # Set rpath to find shared libraries at runtime
    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH "${COD_TOOLCHAIN_DIR}/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    # On macOS, ensure we can find the dylib at runtime
    if(APPLE)
        set_target_properties(${target_name} PROPERTIES
            BUILD_RPATH "${COD_TOOLCHAIN_DIR}/lib"
            INSTALL_NAME_DIR "@rpath"
        )
    endif()
endfunction()

# Create and configure all test executables

configure_cod_target(showcase_basic_usage ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/showcase_basic_usage.cc)

configure_cod_target(test_memory_regions ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_memory_regions.cc)

configure_cod_target(test_regional_types ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_regional_types.cc)

configure_cod_target(test_deletion_methods ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_deletion_methods.cc)
