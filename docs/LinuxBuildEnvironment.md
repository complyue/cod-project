# Linux build environment

The Linux bootstrap uses an unmodified LLVM release, Clang configuration files
for bundled runtime selection, and C++20 for CoD. Native macOS instructions and
validation are in [MacOSBuildEnvironment.md](MacOSBuildEnvironment.md). The
Windows bootstrap has its own version selection.

## Source and tools

`do-build-cod.sh` pins LLVM to a release tag and its exact commit. An existing
checkout must match that commit and have no local modifications. The sparse
checkout includes the required projects and omits disabled upstream test and
documentation trees. It does not apply files from `patches/llvm/`.
LLVM libc sources supply shared utility headers; the host C library remains
the distribution's glibc.

The validated environment is Debian 13 x86-64 with GCC 14.2.0 as the stage-one
compiler, CMake 4.4.4, Ninja 1.13.2, SWIG 4.5.1 and LLVM 23.1.3. Linux validation
covers all three bootstrap stages, installation, eight test suites, relocation,
LLDB debugging and Python integration, and the project IDE checks below.
Exact versions, upstream archive checksums and validation outcomes belong in
[`toolchain/linux-validation.json`](../toolchain/linux-validation.json).

Required host packages include `build-essential`, `git`, `pkg-config`, Python 3,
`libffi-dev`, `libedit-dev`, `libncurses-dev`, `zlib1g-dev`, `libzstd-dev`,
`libxml2-dev` and `python3-dev`. SWIG is also required; it and the Python
development files support the final LLDB build. `prepare-ubuntu.sh` installs distro prerequisites;
its CMake and Ninja package versions may differ from the validated upstream
versions. The latest stable CMake and Ninja archives come from their official
GitHub releases and must match the published SHA-256 digests.

The deployed LTSP environment exposes CMake and Ninja through `/opt/bin`, with
their installations under `/opt/cmake` and `/opt/ninja`. SWIG 4.5.1 is installed
in the system image at `/usr/local/bin/swig`, with its library under
`/usr/local/share/swig/4.5.1`. These deployed tools support a clean three-stage
bootstrap with no trial installation or `/usr/local/bin/ninja` compatibility
link. All bootstrap and test caches select `/opt/bin/ninja`.

## Build and validation

Use a separate checkout/worktree for each host environment. The active CoD build
tree is `build/`, the installed distribution is `built/`, and bootstrap stages
remain under `build/.bootstrap/`. `build/cod/` contains the compiler used for the
active build. Do not replace these paths with symlinks or discard bootstrap
outputs to repair an incremental failure.

The Linux validation keeps its bootstrap log in
`build/validation/linux-bootstrap.log`. The adjacent
`linux-bootstrap-process.json` records the process ID, host boot ID and exit-code
file, so monitoring can distinguish a running build from an interrupted run.
These records survive a diskless client reboot; files under `/tmp` do not.

Bootstrap/configuration requires the explicit authorization described in
`AGENTS.md`. Once authorized, run:

```sh
COD_BUILD_JOBS=8 bash ./do-build-cod.sh
```

The job limit applies to all bootstrap stages; LLVM links are serialized to
bound peak memory use. Choose a smaller job count on hosts with less available
memory. For source-only incremental changes, run exactly:

```sh
cmake --build ./build
```

When tests are authorized, use `COD_TEST_TOOLCHAIN=build`. Linux validation
covers CoD, shilos, codp and YAML integration; direct C/C++ compiler use; bundled
runtime loading; and installation relocation. Record failures and unrun checks
as such, without treating a successful configure as a successful build.

`tests/toolchain/run-all.sh` compiles and runs C and C++20 programs without
explicit library paths, exercises C++ formatting and exceptions, and checks
that the loader selects libc++, libc++abi and libunwind from the selected
toolchain without library or include path environment overrides.

Clang loads `bin/clang.cfg` or `bin/clang++.cfg`. `<CFGDIR>` resolves library
paths relative to the compiler configuration, including after relocation.
The C++ driver selects libc++; link invocations receive a runtime search path.
The bundled library directory precedes command-line `-L` directories because
Clang reads default configuration arguments first. `--no-default-config`
explicitly disables the bundled configuration when selecting another toolchain
or library search policy.

## VSCodium and clangd acceptance

Successful development bootstrap includes working C++ language support for this
checkout in VSCodium. Install `llvm-vs-code-extensions.vscode-clangd` and a stable
standalone clangd server on the local SSD. The Linux bootstrap enables Clang, LLD
and LLDB; it does not enable `clang-tools-extra` or produce clangd.

Workspace settings select the installed server with `clangd.path` and pass
`--compile-commands-dir=${workspaceFolder}/build`. Allow driver queries only for
`${workspaceFolder}/build/cod/bin/clang` and `clang++`, so clangd can discover the
active compiler's target and bundled libc++ headers. The repository's `.clangd`
supplies its C++20 parsing settings. Keep indexing concurrency bounded while the
bootstrap is running. Do not copy compilation databases from another checkout or
point the IDE at an intermediate bootstrap stage.

The validated IDE combination is VSCodium 1.126.04524, the clangd extension 0.6.0
and standalone clangd 23.1.0. Project checks cover diagnostics, completion, hover
and definitions on six source/header files, including navigation to bundled
libc++ headers and an unsaved diagnostic round trip. The deployed server is
`/opt/clangd/bin/clangd`. Refactoring actions such as `ExtractFunction` and
`SwapBinaryOperands` are outside the passed IDE feature checks.

IDE acceptance requires the installed extension to activate on this checkout,
start the configured server, and load `build/compile_commands.json`. Verify
diagnostics, code completion, hover information and go-to-definition on actual
CoD, shilos and codp sources and headers. Standard library, project and generated
headers must resolve using the active build configuration. Installation, server
startup or a standalone sample alone does not establish project IDE readiness.
Keep this check pending until the final compilation database and required build
artifacts exist. Record outcomes in `toolchain/linux-validation.json`, with local
evidence under `build/validation/`.

## Diskless development hosts

On LTSP clients, packages and tools installed into the running system can
disappear after reboot. CoD records the requirements and tested versions here;
the designated LTSP maintenance chat owns persistence in the `superuser`
profile. Profile changes, deployment and reboot validation are tracked by
`ltsp-diskless`, separately from the CoD build results. Do not reboot a client
while its bootstrap or validation is running.

Temporary trial paths and generated build caches are local development state.
They do not require permanent compatibility paths in the system image. Host
tool changes that invalidate a build configuration require a project-owned,
explicitly authorized configuration or bootstrap using the deployed tools.
