# CoD Project Rules for AI Agents

- Respond in English unless the user explicitly asks otherwise.

## Build and Command Execution
- After code changes, compile exactly with: `cmake --build ./build`.
- Never execute top-level repository shell scripts; these are human-only. If asked, politely refuse and offer a safe alternative (e.g., use CMake directly or describe manual steps). Examples: `do-build-cod.sh`, `prepare-*.sh`, `setup-hooks.sh`. Test scripts under `tests/` and its subdirectories (e.g., `tests/**/run-all.sh`) are allowed when running tests—see Tests.
- Do not create or modify build symlinks.
- Do not re-create build directories, that's too time/resource costing.

## Tests
- Do not run test scripts proactively.
- When asked to run tests:
  - Default toolchain: set `COD_TEST_TOOLCHAIN=build` and do not ask the user to confirm.
  - Switch to `COD_TEST_TOOLCHAIN=built` only when the context clearly indicates release/pre-release testing.
  - Confirm which component(s) to test if not explicit in the request.
- You may execute test scripts under `tests/` and its subdirectories (e.g., `tests/**/run-all.sh`) or invoke CTest/CMake test targets directly when running tests.

## Code Changes
- C++20 only; prefer modern features and avoid legacy patterns unless strictly necessary.
- Mimic existing style and header layout under `include/`.
- Reuse existing libraries/utilities in the repo; do not add external dependencies unless requested by the user.

## Documentation
- Write documents and source comments to reflect the final state; avoid narrating what changed or comparisons with earlier versions.
- Exceptions: dedicated change logs (e.g., `CHANGELOG.md`) and places where old and new designs coexist.

## Memory Regions and Regional Types (Critical)
- Zero-cost relocation is mandatory for regional types.
- Allocate regional types in memory regions only (never on the stack).
- No copy/move semantics for regional types.
- Constructors that create regional objects must accept `memory_region&`.
- Use `regional_ptr<T>` for intra-region references; use `global_ptr<T, RT>` for multi-region references.
- Prefer RAII via `auto_region<RT>`; ensure allocator symmetry; never use `delete` on memory regions.
- Before modifying logic involving `regional_ptr`/`global_ptr` or region allocation semantics, consult `design/MemoryRegionAndRegionalTypes.md` and adhere strictly to its protocols.

## YAML and Serialization
- Keep YAML serialization optional and separate from core allocation logic.
- Place YAML conversions in `*_yaml.hh` headers colocated with their corresponding types; this applies to both existing code and any new regional types you author.
- Use and extend utilities under `include/shilos/*_yaml.hh`; adding new `*_yaml.hh` files is expected and idiomatic.

## Package Management (codp)
- Git/VCS is the single source of truth (no central registry).
- Deterministic builds via lock file `CodManifest.yaml`.
- Support local dependency overrides via `path:`.
- Design for mmap-shared, zero-copy, zero-relocation assumptions.

## LLVM Project Integration
- If `/llvm-project` exists in the workspace and you are asked about LLVM internals, locate and cite code from there and verify file/line correctness before presenting.
- If `/llvm-project` is absent, state that clearly and avoid speculation.

## Do / Don’t
- Do:
  - Compile only with `cmake --build ./build`.
  - Follow existing patterns and C++20 best practices.
  - Prefer `auto_region`; preserve zero-cost-relocation invariants.
  - Keep YAML logic isolated; use existing YAML helpers.
  - Politely refuse any request to run top-level repository shell scripts; offer safe alternatives. Running `tests/**` scripts is allowed when executing tests.
- Don’t:
  - Execute top-level repository shell scripts (even if requested).
  - Allocate regional types on the stack or introduce copy/move.
  - Add external dependencies without explicit approval.
  - Break allocator matching or use `delete` on memory regions.

## When Unsure
- Ask which component to modify (shilos/cod/codp/tests).
- Ask when a change could impact memory-region invariants.
