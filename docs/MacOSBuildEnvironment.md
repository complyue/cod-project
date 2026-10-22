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
Set `COD_RECONFIGURE_BOOTSTRAP=1` on the bootstrap command to configure and
incrementally build both intermediate stages even when their tools are already
installed. This retains completed build outputs. Keep `SDKROOT` consistent with
the existing caches: a different path spelling for the same SDK can trigger
unnecessary recompilation.
Reconfiguration clears the runtime/debugserver hook settings and additional
ABI re-export flags from retained caches, and rechecks linker color-diagnostic
support for the selected Apple runtime linker.

For source-only incremental work, use:

```sh
cmake --build ./build
```

## Runtime selection

The macOS stages bundle libc++, libc++abi and compiler-rt, using the system
unwinder supplied by libSystem. Later stages select the Mach-O linker
`ld64.lld`. Compiler-rt's iOS, watchOS, tvOS and visionOS builds are disabled.
Compiler-rt targets the native host architecture through `DARWIN_osx_ARCHS` and
`DARWIN_osx_BUILTIN_ARCHS`. This avoids extra slices such as x86_64h, for which
the pinned Mach-O LLD emits the generic x86_64 subtype.
Runtime builds use the Apple linker: Mach-O LLD 23 does not implement the symbol
re-export lists needed by libc++. This is
configured through upstream `RUNTIMES_CMAKE_ARGS`; the bundled Clang drivers
continue to use LLD for ordinary programs.

Bootstrap executables, shared libraries, modules, nested runtimes and bundled
driver link commands use `-no_fixup_chains`. This keeps classic dyld bindings
when bundled and system libc++ coexist. The Intel bootstrap encountered this
failure in TableGen; related reports appear in
[LLVM issue 111570](https://github.com/llvm/llvm-project/issues/111570).

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

The portable toolchain smoke checks run with:

```sh
COD_TEST_TOOLCHAIN=build bash tests/toolchain/run-all.sh
```

On macOS they cover C, C++20 formatting, exception rethrow, ABI demangling,
bundled runtime selection, AddressSanitizer, and loading a bundled-runtime dylib
after system libc++. They also check the driver's classic dyld binding policy.
These checks pass on the local arm64 host with the combined configuration.
All three retained bootstrap stages, installation, the final incremental build,
CoD evaluation and LLDB Python/debugging checks also pass. Installed ASan,
libc++ and libc++abi load from `built/` without development-build dylibs.

Exact local results, tool checksums and log locations are recorded in
[`toolchain/macos-validation.json`](../toolchain/macos-validation.json).
Full project suites, universal distributions, installation relocation and
cross-platform targets are outside this smoke coverage.

## Intel baseline evidence

The separate native x86_64 run at commit
`0d69926` completed three stages and installation on macOS 14.7.7 with
AppleClang 16.0.0 and SDK 15.2. Its original record is preserved in
[`toolchain/macos-x86_64-validation.json`](../toolchain/macos-x86_64-validation.json).
That run used LLD with a whole-library libc++abi re-export hook, and checked
tool startup, Mach-O dependencies and runtime loading. It did not run functional
smoke tests, LLDB Python scripts, or debugger launch/attach checks. Its evidence
applies to that commit.

The combined configuration at `a956a2173e53c5e9f1a123a606d083a6b1b4b005` also
passes the native Intel rerun on that host. All three retained stages and
installation complete with the Apple runtime linker and compiler-rt's relative
runtime search paths. The final `cmake --build ./build` check passes. With
`COD_TEST_TOOLCHAIN=build`, the toolchain smoke suite passes C, C++20 formatting,
exception rethrow, ABI demangling, bundled runtime loading, AddressSanitizer,
system-first libc++ coexistence and classic dyld binding checks. CoD evaluates
`6 * 7` to `42`, and LLDB runs Python 3.9.6.

LLDB debugging passes with the signed Xcode helper: breakpoint resolution,
launch, stop at `main`, frame inspection, continuation and exit zero. Initial
debugger attempts timed out, including with that helper; after the stalled
processes exited, Apple LLDB and the unified LLDB retry both launched the same
fixture successfully. The cause of the transient stall is unconfirmed. The
bundled ad-hoc helper was not revalidated after that stall. Installed binaries
were inspected, but functional checks in this rerun use `build/`; full suites
and relocation remain outside coverage.

The `integration` section of
[`toolchain/macos-x86_64-validation.json`](../toolchain/macos-x86_64-validation.json)
records the Intel rerun separately from its baseline, including failed attempts
and the successful debugger retry. Local logs use the
`build/validation/macos-unified-` prefix.
