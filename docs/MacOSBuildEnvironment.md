# macOS build environment

`do-build-cod.sh` builds CoD with the pinned, unmodified LLVM 23.1.3 source.
The macOS toolchain bundles Clang, Mach-O LLD, libc++, libc++abi and compiler-rt;
the final stage also builds LLDB. The OS supplies the C library and unwinder.
Build and validation results are recorded in
[`toolchain/macos-validation.json`](../toolchain/macos-validation.json).

## Host tools and SDK

Select an Xcode or Command Line Tools installation with `xcode-select` before
bootstrapping. The script resolves the macOS SDK through `xcrun`, honoring an
explicit `SDKROOT`, and uses it for both CMake and the bundled Clang default
sysroot. SDK selection belongs to this worktree's build configuration.

Required host tools include Git, CMake, Ninja, Python 3 and SWIG 4 or newer.
The SDK supplies libffi, libedit and the system library headers. SWIG and Python
development headers enable LLDB's Python bindings. `prepare-macos.sh` installs
the Homebrew prerequisites, including zlib and pkg-config.

On Intel hosts without compatible Homebrew bottles, source builds of missing
tools can be installed under `build/.bootstrap/host-tools`. Add that prefix's
`bin` directory to `PATH` before bootstrapping. Keep the prefix available for
incremental builds; CMake records the discovered tools' absolute paths. The
validation record identifies any locally built tools and their source hashes.

## Bootstrap and incremental builds

Use a separate checkout/worktree for each host environment. `build/` is the
active development tree, `build/cod/` contains its compiler, and `built/` is
the installed distribution. Intermediate stages remain in `build/.bootstrap/`.
These directories must be real directories, not build-directory symlinks.

After bootstrap/configuration authorization as required by `AGENTS.md`, run:

```sh
COD_BUILD_JOBS=6 bash ./do-build-cod.sh
```

Choose a job count appropriate for available memory. LLVM links are serialized.
After changing intermediate-stage build configuration, set
`COD_RECONFIGURE_BOOTSTRAP=1` on the same command to configure and incrementally
build both bootstrap stages before the final stage. Existing outputs are retained.
For a worktree with locally built prerequisite tools, use:

```sh
PATH="$PWD/build/.bootstrap/host-tools/bin:$PATH" \
  COD_BUILD_JOBS=6 bash ./do-build-cod.sh
```

For source-only incremental changes, run exactly:

```sh
cmake --build ./build
```

The bootstrap validates the LLVM commit and rejects modified LLVM sources.
It uses `ld64.lld` for macOS and retains the intermediate runtime installations.
Compiler-rt libraries target the host architecture, selected through
`DARWIN_osx_ARCHS` and `DARWIN_osx_BUILTIN_ARCHS`. On Intel, this selects x86_64
without a separate x86_64h slice: the pinned Mach-O LLD emits the generic x86_64
CPU subtype, so those two linked slices cannot form a universal library.
Clang loads `bin/clang.cfg` or `bin/clang++.cfg`; `<CFGDIR>` resolves the bundled
library search path relative to those files. Link commands receive a runtime
search path, and the C++ driver selects libc++. The shared libc++ build uses
`LIBCXX_ADDITIONAL_LIBRARIES` to re-export the complete in-tree libc++abi dylib.
`toolchain/macos/runtimes.cmake` sets this option inside the runtime project so
its target expressions are evaluated in the correct CMake scope.
This supplies ABI symbols through libc++ with Mach-O LLD, whose symbol-list
re-export option is unimplemented. The flag is restricted to shared-library
builds and excluded from installed CMake interfaces. Runtime libraries use
`@loader_path` as their first runtime search path to prefer adjacent bundled
libraries in both build and install trees.
Bootstrap link options, the runtime hook and the bundled driver configuration
disable Mach-O chained fixups with `-no_fixup_chains`. This selects classic dyld
bindings when bundled and system libc++ coexist; related failures are described in
[LLVM issue 111570](https://github.com/llvm/llvm-project/issues/111570).
`--no-default-config` disables the bundled driver
configuration when explicitly selecting another library policy.

LLDB's `Debugserver` project loads `toolchain/macos/debugserver.cmake` through
`CMAKE_PROJECT_Debugserver_INCLUDE`. The hook pre-includes `<memory>` for that
subproject's C++ compilations because its pinned headers use `std::shared_ptr`
and `std::enable_shared_from_this` without including their defining header.
The option is limited to debugserver and does not modify the LLVM checkout.

## Validation evidence

Bootstrap logs live in `build/validation/macos-bootstrap.log`. The adjacent
`macos-bootstrap-process.json` records the PID, start time, command, status and
exit-code file. A successful configure is not a completed bootstrap; all three
stages and installation must finish successfully.

Inspect compiler versions, Mach-O library dependencies and runtime search paths
in the final tree before marking the build complete. Record unrun checks as
unrun. Run repository test scripts only when authorized, using
`COD_TEST_TOOLCHAIN=build` by default and the explicitly requested components.

### Validated host and outcome

On 2026-10-22, the native x86_64 bootstrap completed all three stages and
installation on macOS 14.7.7 using Apple Clang 16.0.0 and the Xcode macOS 15.2
SDK. The completed run exited with status 0 at 03:21:03 Asia/Shanghai.
The final `cmake --build ./build` verification also exited with status 0.
LLVM remains unmodified at `0d261d1ca552c95a8f007e061c787ac7132fbcbc`.

Both `build/bin/` and `built/bin/` report Clang, LLD and LLDB 23.1.3.
CoD help output and debugserver version output start successfully in both
trees. Mach-O inspection confirms relative runtime search paths first,
libc++ re-exporting the bundled libc++abi, and no chained fixups in the
inspected executables and shared libraries. Loader traces for Clang++ and LLDB
resolve bundled LLVM/C++ libraries from the corresponding tree; system libc++
and libc++abi are also loaded through system dependencies.

The binaries retain absolute build-path fallback search paths, and the LLDB
Python plugin retains a search path to the selected Xcode Python framework.
Relocation and Python scripting behavior have not been tested. Debugserver is
ad-hoc signed because this host has no `lldb_codesign` development identity;
debugger launch and attach behavior have not been tested. Repository test
suites were not requested and were not run.

Detailed evidence is in `build/validation/macos-final-versions.txt`,
`macos-final-macho.json`, `macos-final-runtime-loading.txt`,
`macos-final-cli-startup.txt` and `macos-debugserver-signature.txt` in the same
directory. The machine-readable validation record lists the covered checks
and their limits.
