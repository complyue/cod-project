# CoD Project Rules for AI Agents

- Respond in English unless the user explicitly asks otherwise.

## Design Authority and Value Lineage

### Sources of authority
- The design charter (the general principles, or 总则) is `design/RegionalSemanticPlatformCharter.md`.
- `design/DesignAuthorityRegistry.md` is the canonical record of design concepts, mechanisms, abstractions, value-derivation/dependency relationships, document qualification, and human confirmations. Use stable record IDs and read only the entries relevant to the task.
- Every design document, wherever located, must have a human-confirmed value-derivation path to the charter before it can serve as a delegated implementation contract. A technical dependency or a link to the charter alone does not establish that path.
- Confirmation applies to an identified proposition, relationship, document revision and scope, not automatically to an entire file or every descendant. Existing code, merged commits, agent summaries, silence, and general approval of a direction are not evidence of approval of all detailed mechanisms.

### Qualified design documents
- A design is qualified only for the scope recorded as `confirmed` or validly `continued` in the registry: a human-confirmed baseline and derivation path exist; the current revision is covered; its constraints, non-goals and acceptance criteria are explicit; and no relevant upstream change or unresolved conflict invalidates the path.
- Add a compact `Design authority` block near the top of each new or revised design document. Include its registry ID/link, design revision and covered sections, parent concept/relationship IDs and the value rationale, governing invariants/non-goals, acceptance criteria, human-confirmed baseline, and current continuity record when applicable. The registry is authoritative if a local summary is stale; uncertainty blocks delegation for the affected scope.
- New designs and newly chosen concepts, abstractions or derivation relationships require human confirmation of their value and scope before qualification. Agents may draft them as `pending-human`; they must not self-approve them by borrowing an ancestor's confirmation.
- A `continued` revision preserves an already qualified document's confirmed contract. It records an agent's continuity assessment, not another human confirmation. It cannot qualify a previously unconfirmed document or ratify an unresolved design choice.

### Implementing against a qualified design
- Read the applicable design contract, its targeted registry status/continuity entry, and directly needed dependency contracts. Identify the design IDs and covered scope for the task.
- When the task stays within that qualified scope, agents are accountable to the local design and need not load the charter, unrelated designs, or the entire authority registry. Do not repeatedly ask humans to reconfirm an unchanged qualified design.
- This context shortcut concerns the design charter only. It never waives this `AGENTS.md`, applicable repository instructions, task authorization, build/test restrictions, or operational safety requirements.
- Missing qualification, stale evidence, contradictory implementation, an uncovered requirement, or a suspected break in the value chain triggers design review for the affected scope. Reading the charter alone does not manufacture missing human confirmation.
- Do not disguise a design change as implementation-only work. If code, tests, defaults or interfaces would change the contract, follow the design-change procedure even if no Markdown edit was initially planned.

### Creating or modifying a design
1. Read the relevant charter clauses in their current context, the affected registry records and human confirmation evidence, and the necessary parent/dependency contracts. Unlike implementation under a qualified contract, design editing cannot skip the charter alignment check.
2. Compare the proposed content with the human-confirmed baseline and the current accepted revision. Explain the preserved value chain and examine goals, assumptions, scope, invariants, trade-offs, interfaces, acceptance criteria and downstream dependencies. Consider the cumulative change since human confirmation, not just the latest small diff.
3. If a change naturally continues an already qualified design without changing the confirmed contract or resolving an unconfirmed choice, record the rationale and old/new revision coverage as a continuity event and keep it `continued`. Editorial changes also need a brief continuity entry, but not a new human approval.
4. If continuity is doubtful, or the proposal changes a confirmed value, boundary, material trade-off, parent relationship or open design choice, mark the affected scope `pending-human` or `needs-reconfirmation`. Present the proposed chain, the point of doubt, alternatives/consequences and the exact decision needed to a human. Do not activate the disputed design or implement it as accepted until explicit confirmation is recorded; unrelated qualified work may continue.
5. Update the document's authority block and the registry together, preferably in one commit. Record affected descendants and dependencies; invalidate only scopes whose qualification can no longer be justified. An obsolete ancestor must not leave descendants silently qualified.
- Changing the charter's goals, principles or approval rules requires explicit human confirmation; agents cannot rewrite the root to make a preferred design appear compliant.

### Maintaining the authority registry
- Record concepts and relationships separately. For each relationship identify its endpoints, kind (`derives-from`, `depends-on`, or `constrains`), rationale, scope/version and confirmation state. Value chains must explain why a mechanism serves a charter value; dependency chains explain what it requires. Dependency edges never grant design authority.
- Human confirmation events must identify the actual human as known, an explicit statement with its source and timestamp, the accepted/rejected propositions and relationships, scope/revision and exclusions. A conversation excerpt can be recorded when no durable link exists; label its provenance and do not invent a URL, identity, signature or broader approval.
- Keep human decision history append-only. Corrections, rejection, withdrawal, replacement and renewed confirmation are new events referencing the earlier event. Update the current-status index separately; never erase inconvenient evidence or relabel an agent assessment as a human decision.
- Maintain reverse links to affected designs, supersession/invalidation records, and pending questions so the path from charter value to concept, mechanism and implementation contract remains traceable in both directions.
- Existing documents without sufficient recorded evidence are `pending-human`, not automatically grandfathered in. Register their current revision, candidate value chain and missing confirmation; add the authority block when revising them. They remain readable references, but cannot supply the no-charter implementation shortcut or authorize new design decisions. Search for existing evidence before requesting a new decision; do not fabricate retrospective approval.

## Build and Command Execution
- After code changes, compile exactly with: `cmake --build ./build`.
- Treat `./build` as the active build tree for the current checkout/worktree; use a separate worktree for another host/container/OS development environment.
- Top-level repository scripts may be executed when they are within the current human authorization. Inspect scripts first when their effects are unclear. Scripts that can bootstrap, reconfigure, clean, install, or mutate toolchains follow the Bootstrap and Build Configuration Safety rules below.
- Do not create or modify build symlinks.
- Do not re-create build directories, that's too time/resource costing.

## Bootstrap and Build Configuration Safety
- Unless the human explicitly authorizes a bootstrap/configuration task in the current conversation, agents must not run commands or make edits that can cause a full or partial rebootstrap of LLVM/Clang/CoD.
- The normal post-edit build command `cmake --build ./build` is allowed only after source-only changes that do not alter build configuration. If build configuration files were changed, ask for explicit authorization before building.
- Do not run CMake configure/generate commands for the main build without explicit authorization. This includes `cmake -S ... -B ...`, `cmake -B ./build`, `cmake --preset ...`, `cmake --fresh`, commands that pass new `-D...` cache options, or any command intended to regenerate `build/CMakeCache.txt`, `build/build.ninja`, or `build/CMakeFiles/`.
- Do not clean, delete, move, rename, recreate, or bulk-edit build outputs without explicit authorization. This includes `build/`, `built/`, `build-*`, `built-*`, `build/.bootstrap/`, `CMakeCache.txt`, `CMakeFiles/`, `build.ninja`, `.ninja_*`, and generated install trees.
- Do not invoke install, clean, runtime, LLVM, Clang, LLD, libc++, compiler-rt, or bootstrap stage targets without explicit authorization. Examples include `cmake --install`, `cmake --build ./build --target install`, `install-*`, `install-runtimes`, `clean`, `stage*`, `clang`, `lld`, `llvm`, `runtimes`, `libcxx`, and `compiler-rt`.
- Do not modify bootstrap or toolchain definition files without explicit authorization. This includes `do-build-cod.*`, `prepare-*`, CMake files (`CMakeLists.txt`, `*.cmake`, presets, toolchain files), LLVM patch files under `patches/llvm/`, and files under `llvm-project/`.
- Do not update, reinstall, remove, or switch host toolchains and SDKs without explicit authorization. This includes Visual Studio Build Tools/MSVC, Windows SDK, LLVM/Clang, CMake, Ninja, Git, system compilers, and package-manager commands such as `winget`, `choco`, `scoop`, `apt`, or `brew` when used for toolchain packages.
- Reading build scripts, CMake files, caches, generated logs, or LLVM sources is allowed. Reporting what would need to be reconfigured or bootstrapped is allowed. Executing or editing the rebootstrap path requires explicit human authorization.

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
- Exceptions: dedicated change logs (e.g., `CHANGELOG.md`), the historical evidence and decision/continuity records in `design/DesignAuthorityRegistry.md`, and places where old and new designs coexist.

## Memory Regions and Regional Types (Critical)
- Zero-cost relocation is mandatory for regional types.
- Allocate regional types in memory regions only (never on the stack).
- No copy/move semantics for regional types.
- Constructors that create regional objects must accept `memory_region&`.
- Use `regional_ptr<T>` for intra-region references; use `global_ptr<T, RT>` for multi-region references.
- `regional_ptr<T>` can only point to regional types or bits types - never standard C++ types like std::string or std::vector.
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
  - Keep development tools pointed at `build/` in this worktree.
  - Ask for explicit authorization before touching bootstrap/configuration state.
  - Follow existing patterns and C++20 best practices.
  - Prefer `auto_region`; preserve zero-cost-relocation invariants.
  - Keep YAML logic isolated; use existing YAML helpers.
  - Run top-level repository scripts only when their effects stay within the current human authorization.
- Don’t:
  - Execute top-level repository scripts that can bootstrap, reconfigure, clean, install, or mutate toolchains unless explicitly authorized.
  - Reconfigure, clean, install, regenerate, or modify the bootstrapped toolchain path unless explicitly authorized.
  - Allocate regional types on the stack or introduce copy/move.
  - Add external dependencies without explicit approval.
  - Break allocator matching or use `delete` on memory regions.

## When Unsure
- Ask which component to modify (shilos/cod/codp/tests).
- Ask when a change could impact memory-region invariants.
