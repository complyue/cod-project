cmake_minimum_required(VERSION 3.20)

# Stand-alone CMake project for the `yaml-ux` utility.  This mirrors the
# structure used in tests/shilos and tests/yaml-cmp so that we can build against the *installed*
# shilos toolchain located under <project>/built/.

# Path constants -------------------------------------------------------------
set(COD_PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include("${COD_PROJECT_SOURCE_DIR}/tests/cmake/CodTestToolchain.cmake")

project(yaml_ux CXX)

# ---------------------------------------------------------------------------
# Build yaml-ux executable
# ---------------------------------------------------------------------------
add_executable(yaml-ux src/yaml_ux.cc)
cod_link_shilos(yaml-ux)
