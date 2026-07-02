cmake_minimum_required(VERSION 3.20)

# Canonicalize the project root directory
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${COD_PROJECT_SOURCE_DIR}/tests/cmake/CodTestToolchain.cmake")

project(cod_tests CXX)

# Function to create and configure a test executable
function(configure_cod_target target_name)
    # Get source files from remaining arguments
    set(source_files ${ARGN})
    
    # Create the executable
    add_executable(${target_name} ${source_files})
    
    cod_link_shilos(${target_name})
    
    # Compile with debug information
    if(CMAKE_HOST_WIN32)
        target_compile_options(${target_name} PRIVATE /Zi)
    else()
        target_compile_options(${target_name} PRIVATE -g)
    endif()
endfunction()

# Create and configure all test executables

configure_cod_target(showcase_basic_usage ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/showcase_basic_usage.cc)

configure_cod_target(test_memory_regions ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_memory_regions.cc)

configure_cod_target(test_regional_types ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_regional_types.cc)

configure_cod_target(test_deletion_methods ${COD_PROJECT_SOURCE_DIR}/tests/shilos/src/test_deletion_methods.cc)
