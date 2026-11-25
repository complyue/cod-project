# CoD Project Rules for AI Agents

Respond in English unless the user explicitly asks otherwise.

## Design discussion and development mainline

The design discussion maintains the language contracts, reviews proposed implementation architecture before coding, and records its decisions. It may inspect source and existing evidence, challenge a delay or a gap in coverage, and ask mainline for the failing command, input, or missing evidence. It brings unresolved design trade-offs to the project user. The design discussion owns the external review backlog. When a design leaves an observable choice open, record the concrete question, the governing source and the next discussion or decision trigger there. Check existing authority first, bring genuine choices to the user promptly, and remove settled items after updating their contract. Track missing implementation evidence separately. Long-term topics may point to their existing repository entry.

Development mainline owns implementation, regression fixes, the compatibility baseline and its execution, and the decision to integrate, switch production and push the implementation. Mainline publishes when the cleared acceptance conditions are met. If a delayed cutover needs a compatibility run, code patch or implementation push, the design discussion asks mainline to perform it and follows up on the result. [Responsibilities and example](design/ImplementationArchitecture.md#design-review-and-delivery-responsibilities)

## Before coding

1. Identify the component, intended behavior and acceptance workload. Ask which component to change if the request leaves that unclear.
2. Read the relevant qualified language contract and its registry entry. Before writing any new project code, submit its structure to the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) and obtain explicit clearance. Include tests, scripts, tooling, native glue and additions to existing files. Describe responsibilities, caller and interface, state owners and lifetimes, failure and cleanup, and acceptance. Review coherent batches once within their stated scope. Return before expanding it. The [architecture review](design/ImplementationArchitecture.md) records existing dispositions and batch clearances.
3. Start from how the actual workload should read and work. If that source needs a confirmed CoD capability the seed lacks, include the smallest coherent seed increment in the reviewed work. Defer convenience only where the remaining source stays clear and retains the design's guarantees. Keep a temporary substitute isolated and use the intended source for delivery acceptance. See the [booter delivery rule](applications/cod-booter/README.md#initial-seed-delivery) and the [AST work-method counterexample](CHANGELOG.md#h-booter-work-method-20261122).
4. Before concluding, verify the real path against its acceptance conditions, including applicable ownership, failure and cleanup. Report supported coverage and limitations accurately. Follow the build and test authorization below.

## Semantic fidelity and implementation obstacles

- Correct code and fixtures against the latest confirmed contract. Use the pre-code architecture review for each code batch. Human reconfirmation of unchanged semantics is unnecessary for routine corrections unless the expected engineering cost is clearly disproportionate.
- When a technical obstacle would weaken a guarantee or force a material architectural compromise, explain the obstacle, affected behavior, alternatives and costs to the design discussion before proceeding on that scope. Obtain human confirmation before changing a language contract or the user's design principles. Continue independent qualified work.
- When a new confirmation conflicts with existing code, disclose sunk cost and expected rework, migrate faithfully, and remove superseded paths, assumptions and documentation. Preserve meaningful verification.
- An authorized stage may support a sound subset with an honest unsupported-feature diagnostic. Every accepted program must keep its confirmed meaning. Changes in scope or acceptance go through review.

## When to measure an improvement

Proceed with a well-founded, maintainable improvement once its implementation scope is cleared. Verify the behavior that matters for delivery. Collect precise benefit figures only when the user requests them, a concrete decision depends on them, or an explicit acceptance requirement needs them. Use evidence already available from normal work first. Reserve additional instrumentation and benchmark runs for questions that evidence cannot answer.

## Implementation boundaries

- Model compiler state and algorithms in C++ for the seed and CoD for the booter. Use sum types for alternatives with distinct payloads in idiomatic CoD. CoD owns the booter module graph, typed workspace and scoped symbolic-proc orchestration under the [module graph contract](applications/cod-booter/docs/ModuleGraph.md).
- Use C for narrow OS/native services, generic raw storage and LLVM, with CoD-defined C interfaces. Follow the [native component roadmap](applications/cod-booter/README.md#next-stage-repositories-and-native-runtime) and the relevant operation contract.
- Use C++20, match existing code and header style, and reuse repository libraries. Add external dependencies only on user request.
- Apply the [diagnostics policy](applications/cod-booter/README.md#diagnostics-by-compiler-stage) in every phase. Both bootstrap compilers in the planned `e-wrks/cod-seed` enforce required checks, report failures and preserve required runtime behavior. The booter fills the seed's exception-handling gaps and verifies propagation, handling and cleanup. Use available source locations and check facts. The future compiler in `e-wrks/cod` owns diagnostic-only passes and detailed explanations.
- Check relevant interactions with [planned fractions](design/FixedWidthFractions.md), including syntax, promotion, dispatch, layout, borrowing, transfer and formatting. Bring conflicts to design review. Fraction implementation remains deferred. H-256 covers reserving `//`, `#` comments and the representation-independent UoM prerequisites.

## Design Authority and Value Lineage

- The [charter](design/RegionalSemanticPlatformCharter.md) governs design. The [registry](design/DesignAuthorityRegistry.md) owns current scope and qualification and links to [changelog evidence](CHANGELOG.md). Qualification requires a human-confirmed value-derivation path, identified propositions and relationships, covered revision and scope, explicit invariants, non-goals and acceptance, and valid continuity. Technical dependencies, shipped code, agent summaries, silence and general approval do not establish it.
- For work inside a qualified contract, read its scope, targeted registry entry and necessary dependency contracts. This substitutes for rereading the charter, while repository, authorization and safety rules still apply. Review missing or stale evidence, upstream invalidation, conflicts and uncovered requirements. Treat changes to behavior, tests, defaults or interfaces that alter a contract as design changes.
- Before changing a design, read the relevant charter clauses, registry, original human evidence and parent contracts. Check the cumulative change from the confirmed baseline and the current revision. Examine value, goals, assumptions, scope, invariants, trade-offs, interfaces, acceptance and downstream effects.
- Record contract-preserving revisions, including editorial changes, as `continued` with rationale and old/new coverage. An agent continuity assessment carries the established qualification forward. New propositions, value relationships and substantive changes require human confirmation. Mark uncertainty `pending-human` or `needs-reconfirmation`, explain the alternatives and decision needed, and await a recorded decision on that scope. Changes to charter goals, principles or approval rules require explicit human confirmation.
- Put a compact **Design authority** block near the top of every new or revised design. State its registry link, revision and sections, parent concepts and relationships, value rationale, invariants, non-goals, acceptance, confirmed baseline and continuity. Update the block, registry and brief changelog record together. Include affected descendants, dependencies and reverse links. Invalidate only affected scope.
- Use one current registry entry per independently decidable scope. Record concepts and edges separately with endpoints, kind (`derives-from`, `depends-on`, `constrains`), rationale, scope/version and status. Keep exclusions, pending questions and supersession links current. Value edges explain charter value. Dependency edges explain requirements.
- Record the actual human, explicit statement, source/time, accepted or rejected scope, revision and exclusions. Label excerpts honestly when a durable source is unavailable. Preserve append-only decision history in Git with stable changelog IDs and immutable commit/path locators. Correct, withdraw or supersede through new events. Search for evidence before requesting confirmation. Register unqualified legacy documents as `pending-human` with the missing evidence and candidate value path.

## Build and Command Execution

- Use the seed's default specialized mode on the smallest real input that exercises a booter change. When a linked booter is needed, build it once in that mode and verify the affected commands and workloads. This applies to routine integration, self-compilation and bootstrap releases. Record which mode was exercised.
- Verify seed specialization with a small program that exercises several type contexts. Compare the expected differences in emitted method bodies and the expected agreement in observable behavior. Run a second full booter build with `--no-specialize` only for a concrete mode-dependent question that needs the whole application. Run any such comparison serially on the same inputs.
- For incremental CoD work, use `cmake --build ./build` or select the CoD target needed for the change. For seed-only work, use `cmake --build ./build --target cod` when the default target would also run unrelated work. Use the current worktree's `build/` for development tools and a separate worktree for another host, container or OS. Keep build symlinks and directories in place.
- Proceed without separate approval when the plan for the **target you will run**, including any nested builds it invokes, rules out a large-scale LLVM rebuild. For example, if `ninja -C build -n -v cod` shows only compilation of `cod/main.cc` and linking of `bin/cod` against existing LLVM libraries, proceed with the seed build. Incremental builtins and runtimes reached by a routine build follow the same rule.
- Dry-run an unfamiliar plan and the first build after client-profile, system-header or toolchain changes. Use `ninja -C build -n -d explain` with the selected target to investigate unexpected invalidation. Check package/content evidence when timestamps suggest broad invalidation. Seek approval before running a plan that would rebuild LLVM on a large scale or whose impact remains uncertain after inspection. An already authorized repair does not need repeated approval.
- Inspect top-level scripts with unclear effects and run them within the current authorization. Read build scripts, logs, caches and LLVM sources as needed. When asked about LLVM internals, verify and cite file/line evidence in the workspace's `llvm-project/`. If that tree is absent, state the limitation.

### Bootstrap and Build Configuration Safety

Require explicit human authorization before any of the following:

- Bootstrap or configuration work that can fully or partly rebootstrap LLVM, Clang or CoD. This includes CMake configure/generate, presets, fresh caches, new `-D` options and regeneration of the main build files.
- Editing toolchain/bootstrap definitions, including `do-build-cod.*`, `prepare-*`, CMake files and presets, LLVM patches and files in `llvm-project/`. The same rule applies to installing, updating, removing or switching toolchains and SDKs.
- Cleaning, deleting, moving, renaming, recreating or bulk-editing build trees, their metadata or install trees. Direct install, clean, LLVM toolchain (such as `llvm`, `clang` or `lld`), runtime and bootstrap stage targets also require it. The CoD seed target `cod` follows the incremental build rule above.

## Tests

Run test scripts only when asked. Confirm the component if unclear. Set `COD_TEST_TOOLCHAIN=build` by default and use `built` for release/pre-release context. Use the appropriate script under `tests/` or CTest/CMake test target.

## Memory Regions and Regional Types

- Preserve zero-cost relocation, ownership and publication in the C region service. Consult [the region protocols](design/MemoryRegionAndRegionalTypes.md) before changing regional pointers or allocation. Ask when a change could affect region invariants.
- In shilos, construct regional objects only in a region, with `memory_region&`, and preserve their no-copy/no-move model. Use `regional_ptr<T>` for intra-region references to regional or bits types, and `global_ptr<T, RT>` across regions. Prefer `auto_region<RT>` and match the allocator and release operation. Release memory regions through their protocol and never through C++ `delete`.

## Writing CoD source and diagnostics

In CoD source and examples, omit a semicolon when the boundary is clear. Write one when it disambiguates the parse or materially helps the reader see the intended grouping. When designing syntax, minimize the cases that require a semicolon. Between tokens, a newline has exactly the effect of horizontal whitespace. Use token grammar to delimit constructs and allow them to span lines. Preserve the grammar of each supported form. [Semicolon omission and syntax UX](design/SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux)

Every project parser recovers from syntax errors through the end of its input. Represent damaged syntax with located error-report nodes in the expected roles, retain later independent errors and expose them all through normal diagnostics. A recovered file with errors fails compilation. [Whole-file recovery](applications/cod-booter/README.md#whole-file-parser-recovery)

When a diagnostic refers to a resolved name/identity alias, show the name used at the reported site **and** the canonical fully qualified name of its referent. Carry this through renamed re-exports and runtime reports tied to a named source use. [Names in diagnostics](applications/cod-booter/README.md#names-in-diagnostics)

## Terminology and component conventions

Use **function value** for `func …` and **functor value** for receiver-bound `func … in O`. Apply the terms consistently in designs, code comments, diagnostics and examples. The two types cannot be assigned to each other. [Callable terminology](design/CallableStorageAndHereAccess.md#function-value-and-functor-value)

- Follow [CoD operation terminology](design/CallableViewsAndCaptures.md#copying-moving-and-graph-transfer) for bits transfer, reference binding, ownership transfer, callable construction/capture and graph transfer. Native byte copying and C++ moves grant no CoD permission. Callable views require lexical storage, including when part of a compound value.
- Use the [three qualified alias concepts](design/CallableViewsAndCaptures.md#alias-terminology) in discussions, documentation, comments, diagnostics and interfaces. Distinguish declared place aliasing, runtime target aliasing and name/identity aliasing. Preserve access, owners, lifetimes and permitted overlap. A declaration can select storage at runtime, and independent references can share a target.
- Keep YAML optional and separate from allocation. Use colocated `*_yaml.hh` conversions and reuse the helpers in `include/shilos/`.
- For codp, Git/VCS supplies package identity. Use `CodManifest.yaml` for deterministic builds and `path:` for local overrides. Preserve mmap sharing, zero-copy and zero-relocation assumptions.

## Documentation and repository hygiene

- Outside changelogs, describe current contracts, implementation, evidence, limits and open questions. Keep current alternatives and relevant evidence where useful. Use brief changelog records with stable IDs and immutable commit/path locators for history. Keep journals and obsolete snapshots out of the repository.
- After an authorized hygiene inspection, correct and self-review the inspected scope, then stage precise hunks, commit and push the maintenance work. Preserve concurrent edits and keep source and test changes out of documentation-only work.
