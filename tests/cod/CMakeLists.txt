cmake_minimum_required(VERSION 3.20)

# Canonicalize the project root directory
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${COD_PROJECT_SOURCE_DIR}/tests/cmake/CodTestToolchain.cmake")

project(cod_tests C CXX)

find_package(LLVM REQUIRED CONFIG PATHS "${COD_TOOLCHAIN_DIR}/lib/cmake/llvm" NO_DEFAULT_PATH)
find_package(Clang REQUIRED CONFIG PATHS "${COD_TOOLCHAIN_DIR}/lib/cmake/clang" NO_DEFAULT_PATH)
include_directories(BEFORE ${LLVM_INCLUDE_DIRS})
link_directories(BEFORE ${LLVM_LIBRARY_DIRS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} at ${LLVM_INSTALL_PREFIX}")

# Function to create and configure a test executable
function(configure_cod_target target_name)
    # Get source files from remaining arguments
    set(source_files ${ARGN})
    
    # Create the executable
    add_executable(${target_name} ${source_files})
    
    cod_link_shilos(${target_name})

    # Link with Clang libraries for cache functionality
    target_link_libraries(${target_name} PRIVATE 
        clangAST clangBasic clangDriver clangFrontend 
        clangLex clangParse clangSema clangSerialization
        clangAnalysis clangStaticAnalyzerCore clangStaticAnalyzerFrontend
        clangSupport clangEdit clangRewrite clangCodeGen
        clangASTMatchers clangAPINotes)
    
    # Compile flags
    if(CMAKE_HOST_WIN32)
        target_compile_options(${target_name} PRIVATE /W4 /WX /Zi /wd4100 /wd4996)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Werror -g -Wno-unused-parameter)
    endif()
endfunction()

# Create and configure C++ test executables

configure_cod_target(test_cod_workspace
    src/test_cod_workspace.cc
    ${COD_PROJECT_SOURCE_DIR}/cod/cod_workspace.cc
    ${COD_PROJECT_SOURCE_DIR}/cod/cod_cache.cc
    )

configure_cod_target(test_cod_cache
    src/test_cod_cache.cc
    ${COD_PROJECT_SOURCE_DIR}/cod/cod_cache.cc
    ${COD_PROJECT_SOURCE_DIR}/cod/cod_workspace.cc
    )
