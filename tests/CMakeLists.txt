cmake_minimum_required(VERSION 3.20)

# Set path to the built toolchain
set(COD_TOOLCHAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../built")

# Set the compilers to use the built toolchain
set(CMAKE_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang++")

project(cod_tests CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Set paths to the in-tree build and include directories
set(COD_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build")
set(COD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

# Set the CXX flags to use the built libc++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -nostdinc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${COD_TOOLCHAIN_DIR}/include/c++/v1")

# Include directories for project headers (in-tree)
include_directories(BEFORE ${COD_INCLUDE_DIR})

# Add library search paths (prioritize in-tree build, then toolchain)
link_directories(BEFORE "${COD_BUILD_DIR}/lib")
link_directories(BEFORE "${COD_TOOLCHAIN_DIR}/lib")

# Find the shilos library from in-tree build
find_library(SHILOS_LIBRARY 
    NAMES shilos libshilos
    PATHS "${COD_BUILD_DIR}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

# Find required toolchain libraries
find_library(LIBCXXABI_LIBRARY c++abi PATHS "${COD_TOOLCHAIN_DIR}/lib")
find_library(LIBCXX_LIBRARY c++ PATHS "${COD_TOOLCHAIN_DIR}/lib")

if(NOT SHILOS_LIBRARY)
    message(FATAL_ERROR "shilos library not found in ${COD_BUILD_DIR}/lib")
endif()

message(STATUS "Found shilos library: ${SHILOS_LIBRARY}")

# Function to configure a target with our hybrid approach
function(configure_cod_target target_name)
    # Use in-tree project headers
    target_include_directories(${target_name} PRIVATE ${COD_INCLUDE_DIR})
    
    # Link with in-tree shilos library
    target_link_libraries(${target_name} PRIVATE ${SHILOS_LIBRARY})
    
    # Link with toolchain libc++
    target_link_libraries(${target_name} PRIVATE c++ c++abi)
    
    # Set library search paths
    target_link_directories(${target_name} PRIVATE "${COD_BUILD_DIR}/lib")
    target_link_directories(${target_name} PRIVATE "${COD_TOOLCHAIN_DIR}/lib")
    
    # Set rpath to find shared libraries at runtime (both locations)
    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH "${COD_BUILD_DIR}/lib:${COD_TOOLCHAIN_DIR}/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    # On macOS, ensure we can find the dylib at runtime
    if(APPLE)
        set_target_properties(${target_name} PROPERTIES
            BUILD_RPATH "${COD_BUILD_DIR}/lib:${COD_TOOLCHAIN_DIR}/lib"
        )
    endif()
endfunction()

# Simple working test
add_executable(simple_test src/simple_test.cc)
configure_cod_target(simple_test)


message(STATUS "Tests configured to use:")
message(STATUS "  - Toolchain from: ${COD_TOOLCHAIN_DIR}")
message(STATUS "  - Headers from: ${COD_INCLUDE_DIR}")
message(STATUS "  - Shilos library from: ${COD_BUILD_DIR}/lib")
message(STATUS "  - C++ runtime from: ${COD_TOOLCHAIN_DIR}/lib") 
