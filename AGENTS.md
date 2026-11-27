# CoD Project Rules for AI Agents

Respond in English unless the user explicitly asks otherwise.

## Design discussion and development mainline

**Mainline proceeds independently through delivery within an agreed design.** Use the consequence of the decision to determine who owns it.

| Decision | Who acts |
| --- | --- |
| Apply an agreed design to the next workload, fix a regression, choose a helper or fixture, verify compatibility, or publish. | Mainline decides and proceeds. Routine results stay in the development worklog. |
| Make a major choice about program structure or algorithms, change contracted scope or acceptance, or address a guarantee the design cannot meet. | Bring the choice, consequences and alternatives to the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) **before implementing it**, even if language behavior stays the same. |
| Change a language contract or project principle, narrow a user-set rule, or resolve a trade-off beyond delegated authority. | Design brings the choice to the project user promptly and obtains an explicit decision. |

For example, forwarding another borrowed input through an established callback belongs to mainline. Changing which compiler phase owns capture information warrants design review. One review covers the choice through delivery. Report material milestones and blockers as they arise. A progress report requires no acknowledgment to continue. [Responsibilities and examples](design/ImplementationArchitecture.md#design-review-and-delivery-responsibilities)

Design maintains the contracts and records decisions. Before imposing a delivery gate, design itself must identify which confirmed requirement the simplest faithful implementation would miss without it. A proof needed only for an optimization belongs to that optimization. Design can inspect source, challenge delays or coverage gaps, and ask mainline for a failing input, command or missing evidence. In the external review backlog, keep each unresolved observable choice with its governing source and next trigger. Check existing authority first. Update the contract and remove settled items. Track implementation evidence separately and link existing entries for long-term topics.

PRs are a normal publication route. Honor the user’s scoped merge delegation, including authority in a PR or its originating discussion. Record each decision and scope once. Link that evidence and use the merge commit to identify delivery. [PR authorization](design/ImplementationArchitecture.md#pull-request-authorization-and-design-evidence)

## Compiler design target

The `cod` command (Compile on Demand) compiles for the host and runs with `go run` semantics and leaves no user executable. The seed’s default bootstrap produces `cod-booter`. Seed, booter and first-release `e-wrks/cod` target Go-like compile speed and moderate runtime performance. The booter ships one practical code-generation mode. User-selectable performance tuning belongs to `cfe` (Compile for Ever) in `e-wrks/cod`, which builds an optimized executable for later use and owns cross-builds. The production installation uses LLVM components managed by `codup`. It is experimental in the first release and formally supported in 2.0. [Compiler modes](applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds)

## Before coding

1. Identify the component, intended behavior and real acceptance workload. Clarify an ambiguous component. Read the relevant qualified contract and registry entry.
2. Start with how that source should read and work. Include the smallest coherent seed increment for any missing confirmed capability it needs. Defer convenience only when the source stays clear and faithful. Isolate temporary substitutes and use the intended source for delivery acceptance. [Booter delivery](applications/cod-booter/README.md#initial-seed-delivery) · [AST counterexample](CHANGELOG.md#h-booter-work-method-20261122)
3. Justify each compiler proof, snapshot, traversal, precision requirement and regression gate by tracing it to a concrete safety decision, emitted or runtime behavior, required diagnostic or other qualified obligation. Use the cheapest sound evidence sufficient for that consumer. Omit work with no such consumer. [Examples](design/ImplementationArchitecture.md#justifying-compiler-work-before-coding)

### Semantic fidelity and implementation obstacles

Correct routine defects against the latest confirmed contract. An authorized subset must diagnose unsupported features and preserve the meaning of every accepted program. If an obstacle threatens a guarantee, forces a material architectural compromise or makes cost disproportionate enough to change priorities, raise the concrete case, affected behavior, alternatives and cost. Continue independent qualified work. When a new decision supersedes code, disclose sunk cost and expected rework, remove superseded paths, assumptions and documentation, and preserve meaningful verification.

### Review scope and completion

Verify the real path, including ownership, failure and cleanup. Report actual coverage and limitations. Carry the user’s objective and completion conditions through every batch and keep uncovered paths active. A batch result closes only its own scope. Design checks remaining coverage before declaring completion, changing priority or authorizing a performance comparison. [Seed cutover](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance)

## Passes and dependency changes

**Make restarting a pass from entry inexpressible** in the seed, the CoD booter and `e-wrks/cod`. Keep operations that reset or restart the producer out of reach of consumers, dependency notifications, fallbacks and scheduled work. Review the operations reachable through the interfaces, including indirect calls. A run that happens to visit each body once cannot establish this property. [Rule and counterexamples](design/ImplementationArchitecture.md#restarting-a-pass-from-entry)

**Separate collection from use for each fact kind.** A producer records facts and dependencies for a revision. Any required propagation works on those records. A later decision reads the completed facts through a read-only interface. Across source edits, refresh affected results from their recorded dependencies and reuse valid results. If a real workload appears to require violating this boundary, bring the input, affected result, alternatives and costs to the project user and obtain an explicit decision before making an exception. [Completed callee behavior](design/ImplementationArchitecture.md#consuming-completed-callee-behavior)

Use a workqueue or other rescheduling mechanism only for a concrete user outcome that a direct traversal or ordered passes cannot deliver within the required constraints. Identify that outcome and the limitation. Restrict work items to the justified operation. [Workqueue example](design/ImplementationArchitecture.md#when-a-workqueue-is-justified)

## Order work by the decision it serves

Before an evaluation, name the decision it could change or the requirement it would verify. Finish planned work that would materially change the result before evaluating the target. Verify lasting correctness and compatibility during development. Investigate an intermediate result when it can resolve a current choice. Apply this to individual batches and the whole workstream. [Seed example](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance)

### When to measure an improvement

Implement a well-founded, maintainable improvement once its scope is cleared. Collect precise benefit figures only on user request, for a concrete decision, or to meet an acceptance requirement. Use existing evidence first. Add instrumentation or benchmarks only for remaining questions.

## Implementation boundaries

- Model compiler state and algorithms in C++ for the seed and CoD for the booter. Use sum types for alternatives with distinct payloads in idiomatic CoD. CoD owns the booter module graph, typed workspace and scoped symbolic-proc orchestration under the [module graph contract](applications/cod-booter/docs/ModuleGraph.md).
- Implement new shared native support in C, including OS services, generic raw storage and LLVM, with CoD-defined C interfaces. Prefer CoD for high-level features and use C++ for high-level logic inside the seed. Remove remaining dependencies on the retired shilos library. Follow the operation contract for each caller. [Component boundary](design/ImplementationArchitecture.md#retiring-shilos) · [Native component roadmap](applications/cod-booter/README.md#next-stage-repositories-and-native-runtime)
- The seed accepts CoD source entry modules. Retire the C++ eval/interactive driver and its workspace/cache support. Future REPL work belongs in the production compiler, `e-wrks/cod`.
- For C++ work, use C++20 and match existing code and header style. Reuse current project libraries. Add external dependencies only on user request.
- The C++ seed’s delivery target is trusted, valid CoD to LLVM IR and bitcode for the booter, including the interfaces needed to link it. Keep the analysis necessary for faithful emission and required runtime behavior. Remove selected-path binding verification and its exclusive analysis from the default seed. Before 2.0 the CoD booter and `e-wrks/cod` check the [family-wide incoming effect-symbol requirement](design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20) after accounting for local installations at each use. Keep the seed focused on translation and remove verifier-only work from its normal path. Add a default-off check only for a concrete engineering need. The CoD booter owns static soundness checking, including checking its own source. The future compiler in `e-wrks/cod` adds diagnostic-only passes and detailed explanations. [Seed scope](applications/cod-booter/README.md#trusted-source-and-the-seed) · [Stage policy](applications/cod-booter/README.md#diagnostics-by-compiler-stage)
- Check relevant interactions with [planned fractions](design/FixedWidthFractions.md), including syntax, promotion, dispatch, layout, borrowing, transfer and formatting. Bring conflicts to design review. Fraction implementation remains deferred. H-256 covers reserving `//`, `#` comments and the representation-independent UoM prerequisites.

## Design Authority and Value Lineage

The [charter](design/RegionalSemanticPlatformCharter.md) governs design. The [registry](design/DesignAuthorityRegistry.md) owns qualified scope and its [decision evidence](CHANGELOG.md). Within an existing contract, read its scope, registry entry and necessary dependencies. Check for stale evidence, invalidation, conflicts or uncovered requirements. Repository, authorization and safety rules still apply.

Before changing a design, read the governing charter clauses, original human evidence, registry and parent contracts. A change to behavior, tests, defaults or interfaces that alters the contract is a design change. New propositions, value relationships and substantive changes require human confirmation, as do changes to charter goals, principles or approval rules. Seek existing evidence first. Mark unresolved scope `pending-human` or `needs-reconfirmation` and wait for the decision on that scope.

For every new or revised design, follow the [required authority record](design/DesignAuthorityRegistry.md#recording-a-design-revision). Update the design’s authority block, registry and changelog together. Carry forward a contract-preserving revision, including an editorial revision, as `continued` with rationale and old/new coverage.

## Build and Command Execution

- Use the rewritten seed on the smallest real input that exercises a booter change. When a linked booter is needed, build it once and verify the affected commands and workloads. This applies to routine integration, self-compilation and bootstrap releases. The [shared-emission switch](CHANGELOG.md#h-seed-shared-switch-retirement-20261126) belongs to the earlier seed.
- For incremental CoD work, use `cmake --build ./build` or select the CoD target needed for the change. For seed-only work, use `cmake --build ./build --target cod` when the default target would also run unrelated work. Use the current worktree's `build/` for development tools and a separate worktree for another host, container or OS. Keep build symlinks and directories in place.
- Proceed without separate approval when the plan for the **target you will run**, including any nested builds it invokes, rules out a large-scale LLVM rebuild. For example, if `ninja -C build -n -v cod` shows only compilation of `cod/main.cc` and linking of `bin/cod-seed` against existing LLVM libraries, proceed with the seed build. Incremental builtins and runtimes reached by a routine build follow the same rule.
- Dry-run an unfamiliar plan and the first build after client-profile, system-header or toolchain changes. Use `ninja -C build -n -d explain` with the selected target to investigate unexpected invalidation. Check package/content evidence when timestamps suggest broad invalidation. Seek approval before running a plan that would rebuild LLVM on a large scale or whose impact remains uncertain after inspection. An already authorized repair does not need repeated approval.
- Inspect top-level scripts with unclear effects and run them within the current authorization. Read build scripts, logs, caches and LLVM sources as needed. When asked about LLVM internals, verify and cite file/line evidence in the workspace's `llvm-project/`. If that tree is absent, state the limitation.

### Concurrency on the build host

Run one memory-heavy workload at a time across chats and worktrees on the same host. A full booter build occupies that slot through its nested generation and link steps. Full-input profiling, verification and other jobs with a comparable footprint use the same slot. Treat a large job with uncertain memory needs as heavy until its plan or existing observations establish its footprint.

Small fixtures, routine compiler invocations, debugger probes and ordinary incremental jobs may run alongside each other and a heavy build when their combined expected memory use leaves the host reserve available. Classify the invocation by its input and nested work. A peer process name alone must never block a launch or cancel running work.

For a heavy launch, check available memory before starting and monitor it through completion. On the current Linux build host, preserve the existing 6 GiB `MemAvailable` reserve. Defer new work that would consume the reserve and retain the pressure guard for an actual shortfall. A comparison that needs isolation for valid results may request it for that run.

### Bootstrap and Build Configuration Safety

Routine CMake regeneration of an existing build tree follows the incremental build rule. Proceed without separate approval when the selected work, including nested steps, rules out a large-scale LLVM rebuild. If a Ninja dry run shows only regeneration, run just the generator target (for example, `ninja -C build build.ninja` in the existing Ninja tree). Then inspect the regenerated plan for the intended target with `ninja -C build -n -v TARGET`. Ask before executing a large-scale LLVM rebuild or a step whose impact is still uncertain after inspection.

Require explicit human authorization before any of the following:

- Bootstrap or configuration changes that can fully or partly rebootstrap LLVM, Clang or CoD. These include fresh configurations or caches, changes to presets and new `-D` options.
- Editing toolchain/bootstrap definitions, including `do-build-cod.*`, `prepare-*`, CMake files and presets, LLVM patches and files in `llvm-project/`. The same rule applies to installing, updating, removing or switching toolchains and SDKs.
- Cleaning, deleting, moving, renaming, recreating or bulk-editing build trees, their metadata or install trees. Direct install, clean, LLVM toolchain (such as `llvm`, `clang` or `lld`), runtime and bootstrap stage targets also require it. The CoD seed target `cod` follows the incremental build rule above.

## Tests

Existing test fixtures may all be deleted, including whole fixture groups and their test-only runner entries. Add or restore a fixture when a concrete development or delivery need arises. Check the requested behavior through the real workload where it supplies the evidence. [Fixture retirement decision](CHANGELOG.md#h-test-fixtures-on-demand-20261125)

Run test scripts only when asked. Confirm the component if unclear. Set `COD_TEST_TOOLCHAIN=build` by default and use `built` for release/pre-release context. Use the appropriate script under `tests/` or CTest/CMake test target.

## Memory Regions and Regional Types

- Preserve zero-cost relocation, owner lifetime, allocator pairing and publication in native region operations. Follow the [regional ownership rules](design/RegionalOwnership.md) and [DBMR publication contract](design/ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr). Bring changes to these invariants to design review.

## Writing CoD source and diagnostics

In CoD source and examples, omit a semicolon when the boundary is clear. Write one when it disambiguates the parse or materially helps the reader see the intended grouping. When designing syntax, minimize the cases that require a semicolon. Between tokens, a newline has exactly the effect of horizontal whitespace. Use token grammar to delimit constructs and allow them to span lines. Preserve the grammar of each supported form. [Semicolon omission and syntax UX](design/SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux)

Every project parser recovers from syntax errors through the end of its input. Represent damaged syntax with located error-report nodes in the expected roles, retain later independent errors and expose them all through normal diagnostics. A recovered file with errors fails compilation. [Whole-file recovery](applications/cod-booter/README.md#whole-file-parser-recovery)

When a diagnostic refers to a resolved name/identity alias, show the name used at the reported site **and** the canonical fully qualified name of its referent. Carry this through renamed re-exports and runtime reports tied to a named source use. [Names in diagnostics](applications/cod-booter/README.md#names-in-diagnostics)

## Terminology and component conventions

Use **function value** for `func …` and **functor value** for receiver-bound `func … in O`. Apply the terms consistently in designs, code comments, diagnostics and examples. The two types cannot be assigned to each other. [Callable terminology](design/CallableStorageAndHereAccess.md#function-value-and-functor-value)

- Use **indexed read**, **indexed value transfer** and **indexed reference binding** for the three bracket operations. Reserve field getter/setter terminology for field accessors. [Indexing terminology](design/CollectionsAndBuiltinContainers.md#indexing-terminology)
- Follow [CoD operation terminology](design/CallableViewsAndCaptures.md#copying-moving-and-graph-transfer) for bits transfer, reference binding, ownership transfer, callable construction/capture and graph transfer. Native byte copying and C++ moves grant no CoD permission. Callable views require lexical storage, including when part of a compound value.
- Use the [three qualified alias concepts](design/CallableViewsAndCaptures.md#alias-terminology) in discussions, documentation, comments, diagnostics and interfaces. Distinguish declared place aliasing, runtime target aliasing and name/identity aliasing. Preserve access, owners, lifetimes and permitted overlap. A declaration can select storage at runtime, and independent references can share a target.
- Keep YAML optional and convert it at the I/O boundary, separately from region allocation.
- For seed and booter inputs, prepare the [project-local source tree](design/CodProjectStructureAndPkgMgmt.md#directory-layout) by hand. The project and each immediate directory under `.cod/packages/` are module roots. Future package management belongs in `e-wrks/cod`.

## Documentation and repository hygiene

- Keep the primary worktree checked out on `main` at all times. Use named linked worktrees for other branches and Git revision queries for read-only inspection. Check the primary branch during worktree hygiene and handoff.
- Outside changelogs, describe current contracts, implementation, evidence, limits and open questions. Keep current alternatives and relevant evidence where useful. Use brief changelog records with stable IDs and immutable commit/path locators for history. Keep journals and obsolete snapshots out of the repository.
- After an authorized hygiene inspection, correct and self-review the inspected scope, then stage precise hunks, commit and publish the maintenance work through the authorized route. Preserve concurrent edits and keep source and test changes out of documentation-only work.

### Temporary side chats

- Plan for closure at any point. Use existing sources for read-only work and a linked checkout when edits need isolation. Before creating a worktree or output path that could outlive the chat, record its path, purpose, surviving owner, cleanup trigger and review point outside the chat. The parent or hygiene owner accepts custody in advance, including through a standing assignment. If custody is unavailable, use existing sources for read-only work and have an accepting owner arrange any needed edits.
- When pausing, keep meaningful work recoverable through a reachable commit or a scoped patch with required untracked inputs. Leave the surviving owner a current review point. At merge, cancellation, completion or accepted handoff, promptly remove verified disposable paths within existing authorization. Record a continuing need and next check for anything the owner takes over. Act without waiting for a user instruction.
- The surviving owner reviews open entries at their recorded triggers or during routine hygiene, including when closure goes unnoticed. A review date prompts inspection. Before removal, verify commit reachability, staged and working files, active consumers and any required build or install tree approval.
