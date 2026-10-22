# Evaluate runtime target expressions inside the runtime project, not in the
# outer LLVM ExternalProject command. LLD implements whole-library re-exports.
# Restrict the flag to shared-library builds and omit it from installed interfaces.
set(CMAKE_INSTALL_RPATH "@loader_path")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# Keep runtime bindings compatible with the system libc++ loaded by macOS.
# https://github.com/llvm/llvm-project/issues/111570
add_link_options("LINKER:-no_fixup_chains")

set(LIBCXX_ADDITIONAL_LIBRARIES
    "$<BUILD_INTERFACE:$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:-Wl,-reexport_library,$<TARGET_FILE:cxxabi_shared>>>"
    CACHE STRING "Additional libraries for the macOS libc++ build" FORCE)
