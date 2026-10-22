# Changelog

## 2026-10-22 — Patch-free LLVM 23 bootstrap

- Pin the Linux and native macOS three-stage bootstrap to unmodified LLVM
  `llvmorg-23.1.3` (`0d261d1ca552c95a8f007e061c787ac7132fbcbc`), with source
  revision and cleanliness checks. No LLVM patches are applied.
- Unify native macOS arm64 and x86_64 configuration: select the SDK through
  `xcrun`, build compiler-rt for the host architecture, and use Apple ld for
  runtime libraries while bundled Clang drivers use Mach-O LLD for programs.
- Select bundled C++ runtimes through Clang configuration files and
  loader-relative search paths. Use classic dyld bindings to support coexistence
  with system libc++. Scope the LLVM 23.1.3 debugserver `<memory>` workaround to
  its C++ and Objective-C++ targets without editing upstream sources.
- Support incremental refresh of retained bootstrap stages with
  `COD_RECONFIGURE_BOOTSTRAP=1`. Extend the portable toolchain smoke checks with
  macOS runtime selection, AddressSanitizer and system-first libc++ coexistence.

Validation and evidence:

- Native macOS arm64 and x86_64 completed all three stages, installation and
  `cmake --build ./build`. Toolchain smoke, CoD expression evaluation, LLDB Python
  and debugging with Xcode's signed debugserver passed. The unified Intel rerun
  validated source commit `a956a2173e53c5e9f1a123a606d083a6b1b4b005`.
  See the [macOS environment guide](docs/MacOSBuildEnvironment.md),
  [arm64 record](toolchain/macos-validation.json) and
  [Intel record](toolchain/macos-x86_64-validation.json).
- The Intel debugger check passed after initial timeouts; the transient stall's
  cause is unconfirmed. The bundled ad-hoc debugserver remains unvalidated on
  Intel and was denied attachment on arm64. Intel functional smoke used `build/`;
  installed binaries were inspected. Full macOS project suites, installation
  relocation and universal distributions were not validated.
- The inherited Linux baseline records a successful three-stage bootstrap,
  installation, eight test suites, relocation and IDE acceptance on Debian 13
  x86_64. Those results apply to the recorded Linux revision; Linux was not rerun
  for this macOS integration. See the
  [Linux environment guide](docs/LinuxBuildEnvironment.md) and
  [Linux validation record](toolchain/linux-validation.json).
