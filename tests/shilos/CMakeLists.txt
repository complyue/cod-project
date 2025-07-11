cmake_minimum_required(VERSION 3.20)

# Canonicalize the project root directory
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# Set path to the built toolchain
set(COD_TOOLCHAIN_DIR "${COD_PROJECT_SOURCE_DIR}/built")

# Set the compilers to use the built toolchain
set(CMAKE_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang++")

project(cod_tests CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Configure build to use the built toolchain

# Include directories for project headers
include_directories(BEFORE "${COD_TOOLCHAIN_DIR}/include")

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

configure_cod_target(test_yaml_serialization ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_yaml_serialization.cc)

configure_cod_target(test_yaml_containers ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_yaml_containers.cc)
