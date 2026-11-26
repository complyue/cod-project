# CoD Project Rules for AI Agents

Respond in English unless the user explicitly asks otherwise.

## Compiler design target

Design the default `cod` invocation to compile and run a program with `go run` semantics, leaving no executable as a user output. The seed’s default bootstrap job produces the `cod-booter` executable. Favor Go-like compile speed and moderate runtime performance in the seed, booter and first production release. The separate optimizing mode builds a high-quality executable for later use by default. It is planned as experimental in the first release of `e-wrks/cod` and formally supported in 2.0. [Compiler modes and acceptance](applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds)

## Design discussion and development mainline

The design discussion maintains the language contracts, confirms major program structure and algorithm choices before implementation, and records its decisions. It may inspect source and existing evidence, challenge a delay or a gap in coverage, and ask mainline for the failing command, input, or missing evidence. It brings unresolved design trade-offs to the project user. The design discussion owns the external review backlog. When a design leaves an observable choice open, record the concrete question, the governing source and the next discussion or decision trigger there. Check existing authority first, bring genuine choices to the user promptly, and remove settled items after updating their contract. Track missing implementation evidence separately. Long-term topics may point to their existing repository entry.

Development mainline chooses implementation details, fixes regressions, verifies the real workload and publishes within the agreed design. Share material milestones, blockers and design questions as they arise. [Responsibilities and examples](design/ImplementationArchitecture.md#design-review-and-delivery-responsibilities)

Pull requests are a normal publication route. The project user may delegate a specific merge to another agent. Use authority already granted for the change, including an explicit decision in a PR or its originating discussion. Record the decision and its scope once and link it where needed. The merge commit identifies the revision delivered. [PR authorization and design evidence](design/ImplementationArchitecture.md#pull-request-authorization-and-design-evidence)

## Before coding

1. Identify the component, intended behavior and acceptance workload. Ask which component to change if the request leaves that unclear.
2. Read the relevant qualified contract and registry entry. Bring major decisions about program architecture or algorithms to the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) for confirmation before implementing it, including when the language behavior stays the same. Describe the decision, affected responsibilities and trade-off. Once agreed, mainline implements and verifies it independently. Routine edits, helpers, fixtures, scripts and increments within that design proceed directly. Reopen a review when the structural or algorithmic decision changes.
3. Start from how the actual workload should read and work. If that source needs a confirmed CoD capability the seed lacks, include the smallest coherent seed increment in the work. Defer convenience only where the remaining source stays clear and retains the design's guarantees. Keep a temporary substitute isolated and use the intended source for delivery acceptance. See the [booter delivery rule](applications/cod-booter/README.md#initial-seed-delivery) and the [AST work-method counterexample](CHANGELOG.md#h-booter-work-method-20261122).
4. Before concluding, verify the real path against its acceptance conditions, including applicable ownership, failure and cleanup. Report supported coverage and limitations accurately. Follow the build and test authorization below.

For each proposed compiler proof, snapshot, traversal, precision requirement or regression gate, name the decision or behavior that would change if it were omitted. Follow intermediate users through to a safety decision, emitted or runtime behavior, required diagnostic or other qualified obligation of this component. Consider the cheapest sound alternative, including omitting the work. Use stronger evidence only for the uses that need it. Omit work with no such consumer. [Applying the rule](design/ImplementationArchitecture.md#justifying-compiler-work-before-coding)

## Semantic fidelity and implementation obstacles

- Make routine implementation corrections directly against the latest confirmed contract. Bring a disproportionate engineering cost to the design discussion when it calls for a design or priority decision.
- When a technical obstacle would weaken a guarantee or force a material architectural compromise, explain the obstacle, affected behavior, alternatives and costs to the design discussion before proceeding on that scope. Obtain human confirmation before changing a language contract or the user's design principles. Continue independent qualified work.
- When a new confirmation conflicts with existing code, disclose sunk cost and expected rework, migrate faithfully, and remove superseded paths, assumptions and documentation. Preserve meaningful verification.
- An authorized stage may support a sound subset with an honest unsupported-feature diagnostic. Every accepted program must keep its confirmed meaning. Bring changes to contracted scope or required behavior to design review. Mainline chooses the implementation increments and evidence for the agreed outcome.

## Passes and dependency changes

**Give each component an interface that makes prohibited steps impossible to express.** Use a workqueue or another mechanism that can reschedule work only for a concrete project need. Identify the outcome the user depends on and why a simpler traversal or fixed sequence of passes cannot deliver it within the required constraints. Limit each work item to the operation justified by that need. [Workqueue design example](design/ImplementationArchitecture.md#when-a-workqueue-is-justified)

**A phase must not both collect and use the same kind of information.** State which completed facts each pass reads and which facts it produces. Collection may propagate a new finding to every pending entry that depends on it. Using that finding as evidence for a decision belongs to a later pass. A consumer reads the producer’s completed data and leaves it unchanged.

For example, a completed callee contract can tell the caller which places may change and which destinations an `init` parameter guarantees to initialize on successful completion. Caller checking applies the facts its safety decision needs to the actual arguments. **Restarting the same pass from entry on a dependency change is prohibited throughout CoD, including the bootstrap compilers and the future `e-wrks/cod`.** Keep the operation out of the phase interface. Across source revisions, an incremental request may recompute results whose recorded inputs changed and reuse the others. If a real workload appears to require a same-phase restart or using a fact kind during its collection, bring the input, affected output, alternatives and costs to the project user before proceeding on that design. [Seed checker counterexample](design/ImplementationArchitecture.md#restarting-a-pass-from-entry) · [Callee behavior](design/ImplementationArchitecture.md#consuming-completed-callee-behavior)

## Review scope and completion

When the user sets a rule for a whole component, carry its scope, objective and completion conditions through each implementation batch. Keep known uncovered paths in the active work. A scoped correctness result closes only that batch. Design review checks the remaining coverage before accepting completion, changing priority or authorizing a performance comparison. If a concrete case would require narrowing the user’s rule, bring it to the user. [Application to seed bootstrap time](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance)

## Order work by the decision it serves

Before scheduling an evaluation, identify what decision it could change or what requirement it could verify. **If planned, required work will remove or change something material to the result, finish that work before evaluating the target.** Defer an intermediate evaluation when its result would neither change the next action nor establish something that will still hold at delivery.

During the change, verify properties that must survive each step, such as correctness and compatibility. Use focused investigation when its result can resolve a current implementation choice. Apply the same ordering to a whole workstream and to its individual batches. The [seed cutover](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance) illustrates the distinction.

## When to measure an improvement

Proceed with a well-founded, maintainable improvement once its implementation scope is cleared. Verify the behavior that matters for delivery. Collect precise benefit figures only when the user requests them, a concrete decision depends on them, or an explicit acceptance requirement needs them. Use evidence already available from normal work first. Reserve additional instrumentation and benchmark runs for questions that evidence cannot answer.

## Implementation boundaries

- Model compiler state and algorithms in C++ for the seed and CoD for the booter. Use sum types for alternatives with distinct payloads in idiomatic CoD. CoD owns the booter module graph, typed workspace and scoped symbolic-proc orchestration under the [module graph contract](applications/cod-booter/docs/ModuleGraph.md).
- Implement new shared native support in C, including OS services, generic raw storage and LLVM, with CoD-defined C interfaces. Prefer CoD for high-level features and use C++ for high-level logic inside the seed. Remove remaining dependencies on the retired shilos library. Follow the operation contract for each caller. [Component boundary](design/ImplementationArchitecture.md#retiring-shilos) · [Native component roadmap](applications/cod-booter/README.md#next-stage-repositories-and-native-runtime)
- The seed accepts CoD source entry modules. Retire the C++ eval/interactive driver and its workspace/cache support. Future REPL work belongs in the production compiler, `e-wrks/cod`.
- For C++ work, use C++20 and match existing code and header style. Reuse current project libraries. Add external dependencies only on user request.
- The C++ seed’s delivery target is trusted, valid CoD to LLVM IR and bitcode for the booter, including the interfaces needed to link it. Keep the analysis necessary for faithful emission and required runtime behavior. Remove selected-path binding verification and its exclusive analysis from the default seed. Before 2.0 the CoD booter and `e-wrks/cod` check the [family-wide incoming effect-symbol requirement](design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20) after accounting for local installations at each use. Keep the seed focused on translation and remove verifier-only work from its normal path. Add a default-off check only for a concrete engineering need. The CoD booter owns static soundness checking, including checking its own source. The future compiler in `e-wrks/cod` adds diagnostic-only passes and detailed explanations. [Seed scope](applications/cod-booter/README.md#trusted-source-and-the-seed) · [Stage policy](applications/cod-booter/README.md#diagnostics-by-compiler-stage)
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

- Follow [CoD operation terminology](design/CallableViewsAndCaptures.md#copying-moving-and-graph-transfer) for bits transfer, reference binding, ownership transfer, callable construction/capture and graph transfer. Native byte copying and C++ moves grant no CoD permission. Callable views require lexical storage, including when part of a compound value.
- Use the [three qualified alias concepts](design/CallableViewsAndCaptures.md#alias-terminology) in discussions, documentation, comments, diagnostics and interfaces. Distinguish declared place aliasing, runtime target aliasing and name/identity aliasing. Preserve access, owners, lifetimes and permitted overlap. A declaration can select storage at runtime, and independent references can share a target.
- Keep YAML optional and convert it at the I/O boundary, separately from region allocation.
- For seed and booter inputs, prepare the [project-local source tree](design/CodProjectStructureAndPkgMgmt.md#directory-layout) by hand. The project and each immediate directory under `.cod/packages/` are module roots. Future package management belongs in `e-wrks/cod`.

## Documentation and repository hygiene

- Outside changelogs, describe current contracts, implementation, evidence, limits and open questions. Keep current alternatives and relevant evidence where useful. Use brief changelog records with stable IDs and immutable commit/path locators for history. Keep journals and obsolete snapshots out of the repository.
- After an authorized hygiene inspection, correct and self-review the inspected scope, then stage precise hunks, commit and publish the maintenance work through the authorized route. Preserve concurrent edits and keep source and test changes out of documentation-only work.
