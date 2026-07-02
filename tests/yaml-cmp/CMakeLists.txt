cmake_minimum_required(VERSION 3.20)

# Stand-alone CMake project for the `yaml-cmp` test utility.  This mirrors the
# structure used in tests/shilos so that we can build against the *installed*
# shilos toolchain located under <project>/built/.

# Path constants -------------------------------------------------------------
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${COD_PROJECT_SOURCE_DIR}/tests/cmake/CodTestToolchain.cmake")

project(yaml_cmp_test CXX)

# ---------------------------------------------------------------------------
# Build yaml-cmp executable
# ---------------------------------------------------------------------------
add_executable(yaml-cmp src/yaml_cmp.cc)
cod_link_shilos(yaml-cmp)

# ---------------------------------------------------------------------------
# Build yaml authoring test executable
# ---------------------------------------------------------------------------
add_executable(test_yaml_authoring src/test_yaml_authoring.cc)
cod_link_shilos(test_yaml_authoring)
