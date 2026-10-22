# macOS build environment

`do-build-cod.sh` builds CoD with the pinned, unmodified LLVM 23.1.3 source
checkout. The native macOS bootstrap uses AppleClang for stage one, the resulting
Clang for stage two, and the stage-two installation for the final development
build. LLVM patches under `patches/llvm/` are not applied.

## Prerequisites and build

Xcode or Command Line Tools must provide AppleClang and a macOS SDK. The script
uses `xcrun` to resolve the selected SDK and supplies it to both CMake and Clang.
`SDKROOT` may select a different installed macOS SDK. CMake, Ninja, Git and Python
must be on `PATH`; SWIG and Python development files enable LLDB's Python
bindings. On a host with Homebrew, the prerequisite helper is:

```sh
bash ./prepare-macos.sh
```

Bootstrap and configuration require the authorization described in `AGENTS.md`.
Once authorized, run:

```sh
COD_BUILD_JOBS=6 bash ./do-build-cod.sh
```

The script uses a shallow, filtered sparse checkout at the exact LLVM commit.
It rejects a mismatched commit or local source changes. Each host environment
owns its checkout and build tree. The active build is `build/`, the stage-two
compiler installation is `build/cod/`, and the final installation is `built/`.
Intermediate stages remain under `build/.bootstrap/` for incremental repair.

For source-only incremental work, use:

```sh
cmake --build ./build
```

## Runtime selection

The macOS stages bundle libc++, libc++abi and compiler-rt, using the system
unwinder supplied by libSystem. Later stages select the Mach-O linker
`ld64.lld`. Compiler-rt's iOS, watchOS, tvOS and visionOS builds are disabled.
Runtime builds use the Apple linker: Mach-O LLD 23 does not implement the symbol
re-export lists needed by libc++. This is
configured through upstream `RUNTIMES_CMAKE_ARGS`; the bundled Clang drivers
continue to use LLD for ordinary programs.

Runtime dylibs search beside themselves and, for compiler-rt's nested library
directory, in the toolchain's `lib/` directory before compiler-injected paths.
These loader-relative paths apply to both build and installed libraries.

LLVM 23.1.3's debugserver headers require `<memory>` without including it. CoD's
CMake configuration supplies `-include memory` only to the debugserver C++ and
Objective-C++ targets for that release. LLVM source files remain unmodified.

`toolchain/macos/clang.cfg` and `clang++.cfg` are copied beside the stage-one
compiler and configured and installed with CoD in subsequent stages. They use
Clang's `<CFGDIR>` substitution to locate the bundled library directory and add
a runtime search path. The upstream libc++ build re-exports the in-tree
libc++abi symbols on macOS. Clang's `--no-default-config` option disables these
default configuration files.

## Local validation

The initial host is macOS 26.5.1 arm64, with 32 GiB memory and eight logical CPUs.
AppleClang 21.0.0 and the SDK selected from Xcode provide the host toolchain.
CMake 4.4.4, Ninja 1.13.2 and SWIG 4.5.1 are installed locally under
`build/.bootstrap/host-tools/`. CMake and Ninja archives are checked against the
SHA-256 digests published with their upstream releases; the SWIG wheel is
checked against its PyPI digest.

On this checkout, `build/validation/macos-env.sh` adds those tools to `PATH` and
selects the SWIG library directory. Source it before invoking build commands in
a new shell. Host tools and build outputs are local state, not tracked sources.

Native bootstrap, installation and incremental building pass. C++20 formatting,
exceptions, ABI demangling and runtime loading pass for both `build/` and
`built/`. The installed AddressSanitizer check passes and loads compiler-rt,
libc++ and libc++abi from `built/`. CoD compiles and evaluates an expression in a
minimal project. LLDB starts and runs its Python interpreter.

The bundled debugserver is ad-hoc signed; this host denies its attach request.
Debugging passes with the signed debugserver already supplied by Xcode:

```sh
export LLDB_DEBUGSERVER_PATH="$(dirname "$(xcrun lldb -P)")/debugserver"
built/bin/lldb
```

This host configuration does not install a signing certificate or change macOS
security settings. The LLDB check covers breakpoint resolution, launch, stopping
at `main`, frame inspection, and continuation to a successful exit.

Exact results, tool checksums and log locations are recorded in
[`toolchain/macos-validation.json`](../toolchain/macos-validation.json).
No project test scripts have been run. Intel macOS, older macOS releases,
universal distributions, installation relocation and cross-platform targets
are unvalidated.
