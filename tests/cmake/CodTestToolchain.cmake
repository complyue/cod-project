include_guard(GLOBAL)

if(NOT DEFINED COD_PROJECT_SOURCE_DIR)
  message(FATAL_ERROR "COD_PROJECT_SOURCE_DIR must be set before including CodTestToolchain.cmake")
endif()

file(TO_CMAKE_PATH "${COD_PROJECT_SOURCE_DIR}" COD_PROJECT_SOURCE_DIR)

if(DEFINED ENV{COD_TEST_TOOLCHAIN})
  set(COD_TEST_TOOLCHAIN "$ENV{COD_TEST_TOOLCHAIN}")
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

if(CMAKE_HOST_WIN32)
  set(COD_TEST_EXE_SUFFIX ".exe")
  set(COD_TEST_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang-cl.exe")
  set(COD_TEST_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang-cl.exe")
  set(COD_TEST_LINKER "${COD_TOOLCHAIN_DIR}/bin/lld-link.exe")
  find_program(COD_WINDOWS_MANIFEST_TOOL mt.exe)
  if(DEFINED ENV{COD_TEST_BUILD_TYPE})
    set(COD_TEST_BUILD_TYPE "$ENV{COD_TEST_BUILD_TYPE}")
  else()
    set(COD_TEST_BUILD_TYPE "Release")
  endif()
else()
  set(COD_TEST_EXE_SUFFIX "")
  set(COD_TEST_C_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang")
  set(COD_TEST_CXX_COMPILER "${COD_TOOLCHAIN_DIR}/bin/clang++")
endif()

if(NOT EXISTS "${COD_TEST_CXX_COMPILER}")
  message(FATAL_ERROR "CoD test compiler was not found: ${COD_TEST_CXX_COMPILER}")
endif()

set(CMAKE_C_COMPILER "${COD_TEST_C_COMPILER}" CACHE FILEPATH "CoD test C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${COD_TEST_CXX_COMPILER}" CACHE FILEPATH "CoD test C++ compiler" FORCE)
if(CMAKE_HOST_WIN32)
  set(CMAKE_BUILD_TYPE "${COD_TEST_BUILD_TYPE}" CACHE STRING "CoD Windows test build type" FORCE)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "CoD Windows test runtime library" FORCE)
  set(CMAKE_LINKER "${COD_TEST_LINKER}" CACHE FILEPATH "CoD test linker" FORCE)
  if(COD_WINDOWS_MANIFEST_TOOL)
    set(CMAKE_MT "${COD_WINDOWS_MANIFEST_TOOL}" CACHE FILEPATH "Windows manifest tool" FORCE)
  endif()
  message(STATUS "Using Windows test build type: ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

function(cod_setup_test_target target_name)
  target_include_directories(${target_name} PRIVATE
    "${COD_INCLUDE_DIR}"
    "${COD_TOOLCHAIN_DIR}/include"
  )
  target_link_directories(${target_name} PRIVATE "${COD_TOOLCHAIN_DIR}/lib")
  set_target_properties(${target_name} PROPERTIES
    INSTALL_RPATH "${COD_TOOLCHAIN_DIR}/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
  )

  if(CMAKE_HOST_WIN32)
    target_compile_options(${target_name} PRIVATE
      /utf-8
      /D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH
    )
  elseif(APPLE)
    set_target_properties(${target_name} PROPERTIES
      BUILD_RPATH "${COD_TOOLCHAIN_DIR}/lib"
      INSTALL_NAME_DIR "@rpath"
    )
  endif()
endfunction()

function(cod_link_shilos target_name)
  find_library(SHILOS_LIBRARY
    NAMES shilos libshilos
    PATHS "${COD_TOOLCHAIN_DIR}/lib"
    NO_DEFAULT_PATH
    REQUIRED
  )

  target_link_libraries(${target_name} PRIVATE "${SHILOS_LIBRARY}")

  if(NOT CMAKE_HOST_WIN32)
    find_library(LIBCXX_LIBRARY c++ PATHS "${COD_TOOLCHAIN_DIR}/lib")
    find_library(LIBCXXABI_LIBRARY c++abi PATHS "${COD_TOOLCHAIN_DIR}/lib")
    if(LIBCXX_LIBRARY)
      target_link_libraries(${target_name} PRIVATE "${LIBCXX_LIBRARY}")
    endif()
    if(LIBCXXABI_LIBRARY)
      target_link_libraries(${target_name} PRIVATE "${LIBCXXABI_LIBRARY}")
    endif()
  endif()

  cod_setup_test_target(${target_name})
endfunction()
