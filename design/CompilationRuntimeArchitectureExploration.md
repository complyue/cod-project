# Compilation and runtime architecture exploration

Design revision: 7 · Recorded: 2026-11-13 America/Los_Angeles · Status: external-value boundary confirmed; comparative analysis ready for whole-file review; native performance and candidate production designs remain unqualified.

## Design authority

- **Record and scope:** [D-25](DesignAuthorityRegistry.md#d-25), all sections. [H-387](DesignAuthorityRegistry.md#h-387) authorizes comparative architecture exploration. [H-390](DesignAuthorityRegistry.md#h-390) fixes external inputs to values of statically known ADTs. [H-392](DesignAuthorityRegistry.md#h-392)/[K-581](DesignAuthorityRegistry.md#k-581) governs the retained evidence and revision7 assessment. The native materialization, whole-program, regional and managed-memory experiments retain their measured scope; no production engine, performance threshold or native-timing change is approved.
- **Candidate value path:** C-85/P-359 → C-01/C-04 and D-25/P-360 → C-85/C-02: preserve inspectable typed business intent, separate program/interpretation/execution state, and find an affordable route to native execution. These detailed agent-proposed derivations remain pending-human. The governing charter is [§§1–3, 4.3, 5–6, 8 and 12](RegionalSemanticPlatformCharter.md).
- **Constraints/dependencies:** P-361 depends on D-01/D-10/D-14/D-15/D-17/D-18/D-21; D-16 additionally constrains numeric and semantic identity. P-362 records the user's fused-artifact, external-value and experimental RTTI boundaries. Input values and their combinations can be unknown before execution; their ADTs and semantic interpretations belong to the statically fixed fused world. Preserve actual owners, independent complete callable values, zero-cost regional relocation, selected interpretations, perform/behave/touch, return/exception/defer and initialization operand effects.
- **Baseline:** the current project user explicitly redirected work in the [architecture side chat](codex://threads/01a1d360-c1b0-7ca0-bd02-ea7ef88d2e0f), through the authorized [work package](../../.codex-handoffs/cod-project/2026-11-12-architecture-exploration/WORK_PACKAGE.md). Exact message timestamps and personal identity are unavailable. H-387 records excerpts and exclusions; H-386 requires topic discussion followed by whole-file approval.
- **Acceptance/non-goals:** produce reproducible experiments, empirical selection criteria, recommendations and a bounded next stage. Workloads, metrics, thresholds and rankings are outputs of exploration, not preapproved policy. External data must not extend or reinterpret the fused semantic set. Concrete input-validation/transport mechanisms, a separate composition/publication protocol, final runtime selection, production ABI/checksum/layout, DBMR/mailbox/IO subsystems, owner unions and pure/TCO/fraction/compiler-pilot implementation are outside this scope. Existing build/toolchain restrictions continue.

## Closing baseline and direction

The user classifies continued development of the current direct-to-bitcode route as **expected engineering effort unaffordable**. This is a planning judgment, not a measured labor estimate. Existing implementation effort is excluded from candidate selection.

Commit `34cf632` preserves the smallest closing increment. [Its report](../tests/cod/language/callable-recomposed-factory-validation.json) records 654 language cases verified, 57 native callable cases, 23 defer cases and three AddressSanitizer programs. The language evidence retains the original 653/654 run and targeted verification of an updated diagnostic expectation. All 77 preceding resource comparisons match. Recursive depth through 1000 retains 19 native definitions for the new primary cases. Repeated recomposition at 1/2/4/8/16 stages emits 8/11/17/29/53 definitions while bitcode grows 21,224/36,824/76,816/194,096/584,376 bytes. These are bounded observations, not a universal growth law or measured engineering cost.

Reusable material includes the observable context/ownership/cleanup cases, diagnostics, native resource accounting and historical failed probes. Unresolved actual-owner selection, abstract result evidence, construction/DBMR, module defer and family-dispatch mechanisms remain unresolved. Completing that backlog is not a prerequisite for this exploration.

## Responsibilities and preserved boundaries

| Responsibility | Required meaning | What it does not establish |
| --- | --- | --- |
| Fused semantic artifact | Retain all reachable selected interpretations, types, callable contracts/methods and procedure-local effect information from parallel module worlds. | Entry-world-only flattening, one merged method set, or a runtime module composer. |
| Checking completeness | Preserve the checks and evidence required to execute admitted programs; cover external values within the fixed fused semantic set. | Every possible runtime tuple observed in source, permission to introduce new definitions, or native code for every tuple already emitted. |
| Native preparedness | State precisely which admitted operations have executable native implementations before execution. | Semantic completeness by renaming it AOT completeness. |
| Dispatch | Select a method using actual semantic argument types and the callable's retained interpretation. | Dispatch by layout checksum alone or by a process-wide latest world. |
| Specialization | Optimize a stable subset with explicit assumptions and a correct general path. | Permission to remove effects, change owners or reinterpret existing values. |

The [existing AOT contract](AheadOfTimeMultipleDispatch.md) commits to native targets/dispatch before covered execution. Shared generic native bodies can satisfy that commitment without separate machine code for every tuple if coverage is proved. Interpreter or lazy-native alternatives change the timing contract and need final approval. H-387 authorizes investigating that change; it does not silently approve a weaker meaning of AOT.

An entry extension still requires explicit entry installation. Only affected dependency **initialization TLS writes** are suppressed. Operand construction, effects, failures and cleanup remain; dependency procedure-local installations and explicit source views remain reachable when used. Fusion therefore starts at entry reachability but follows these local interpretations too. Runtime TLS values, captures, region instances and scope changes are execution state, not new copies of the fused type artifact. A candidate must preserve required static owner/access/initialization rejection; experimental runtime assertions do not authorize moving those checks to runtime or introducing owner unions.

Runtime DBMR content graphs, future mailbox packets and possible file/network graphs supply **values of statically known ADTs only**. A sum value can select any alternative admitted by its known type, and multiple values can form a combination absent from source construction paths. They cannot add or change type definitions, constructor definitions, methods, callable contracts or interpretations in the fused world's type-level semantic set. Runtime type evidence identifies an existing member of that set; it is not authority to register a new definition. A carrier graph's representation type remains distinct from the business types it describes, and matching a carrier layout or checksum does not establish that its described values are admissible.

For example, if `Entity` admits `Node` and `Edge`, external data may supply `Node/Edge` even when source construction exhibits only `Node/Node`. It cannot introduce a third alternative or a new dispatch method. Input outside the admitted semantic set must not execute as a valid business value; concrete validation, encoding and failure-reporting mechanisms remain unspecified. Experiments must retain an externally supplied, source-unobserved combination of known alternatives. The boundary is fixed for the selected fused composition, not a requirement to bake every domain into the compiler: separately checking and publishing another composition is a distinct activity under the charter's stage separation, with no protocol selected here.

Exploratory layout evidence may use checksums over complete lower-level memory-layout trees, including sum tags. Experiments must keep semantic identity separate and expose same-layout/different-meaning cases. A checksum match is not proof of equality without collision handling. Recursive type canonicalization, target layout, padding, offsets, permissions, UoM, nominal identity and method interpretation cannot be silently collapsed. No checksum algorithm or persistent representation is selected.

## Candidate responsibility splits

| Candidate | Checking and fusion | Cold execution and native work | Reuse and main burden |
| --- | --- | --- | --- |
| Shared native implementations | Check one semantic graph with symbolic parameters and explicit context/owner evidence; retain dispatch descriptors. | Prepare generic native bodies and dispatch before execution; specialize only where profitable. | LLVM optimizer/code generator; CoD must design finite checking, value/evidence transport and generic native operations. No interpreter required, but native-readiness proof still matters. |
| Semantic execution engine with optional native regions | Check and retain the semantic graph; execute explicit state and context operations. | Interpret cold operations; compile selected hot regions/functions from retained semantics. | LLVM ORC can manage native materialization and resources. CoD still owns interpreter, lowering, boundary state and assumptions. A lazy backend alone does not repair frontend expansion. |
| Interpreter on a reusable optimizing runtime | Retain semantic nodes and implement their meaning using a framework's interpreter conventions. | Framework profiles and compiles interpreter execution; CoD exposes appropriate stable data and guards. | Truffle or RPython can reuse more optimization machinery. CoD must adapt regional storage, lifecycle, semantic evidence and host/guest boundaries; host object graphs cannot replace relocatable business graphs. |

These alternatives assign different responsibilities to CoD. The retained experiments exercise independent LLVM lowering, regional interpretation and managed access to regional storage. They do not establish a production runtime ranking.

## Reusable infrastructure and evidence limits

- [LLVM ORC](https://llvm.org/docs/ORCv2.html) supplies JIT linking/materialization infrastructure. The local [ReOptimizeLayer](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/ReOptimizeLayer.h) exposes profiling and reoptimization callbacks; [CompileOnDemandLayer](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h) supports deferred emission. Those are useful components, not a CoD semantic checker, interpreter, OSR policy or complete execution-state protocol.
- [Truffle partial evaluation](https://www.graalvm.org/dev/graalvm-as-a-platform/language-implementation-framework/PartialEvaluation/) derives optimized code from an interpreter and stable guest structure. Its documentation also requires careful specialization, generalization and invalidation. Inference for CoD: preserving semantic nodes may avoid duplicated meaning in a separate compiler, but adapting regional memory and dynamic binding boundaries still needs a real prototype.
- [GraalVM LLVM Runtime](https://www.graalvm.org/latest/reference-manual/llvm/) interprets LLVM bitcode and compiles hot parts. It is a candidate execution engine, but feeding it the existing fully expanded frontend output retains that frontend cost. A shared or semantic lowering is still needed to address the principal architectural issue.
- [RPython's JIT generator](https://rpython.readthedocs.io/en/latest/jit/overview.html) builds a tracing JIT from an interpreter with hints. This may reuse optimizer engineering, but requires a suitable RPython implementation and a regional/native integration design. It is not a drop-in optimizer for the current C++ frontend.
- [Eclipse OMR](https://github.com/eclipse-omr/omr) offers runtime/compiler components. Its component scope warrants comparison, but no OMR integration, cost or performance result is claimed here.

The experiments use the current build's LLVM tools and the installed Java runtime. No optimizing framework is installed or measured by these probes. Tool availability does not establish suitability for the production architecture.

## Native materialization experiment

[Runner](../experiments/architecture/native_materialization.py) · [Results](../experiments/architecture/native-materialization-results.json) · [Reproduction notes](../experiments/architecture/README.md)

An independently generated LLVM program uses real TLS, dependency-local rebinding/restoration, scalar captured/current inputs and a suppressed installation's operand counter. The same complete IR runs through ORC with whole-module or function-level materialization. Function dumps identify the uncalled definitions reaching the native layer. This tests emission granularity and those scalar observations; it does not establish general fusion, complete captures, installation failure/cleanup, owner proofs or final-image elimination.

The report preserves **30 historical LLVM runs** without a new execution. Source provenance, generated IR hashes, commands, outputs and timings identify the original measurement. The standalone runner retains the identical IR generator.

| Uncalled IR definitions | Function-level materialization, median process time | Whole-module materialization, median process time | Cold functions materialized |
| --- | ---: | ---: | --- |
| 10 | 21.05 ms | 18.00 ms | 0 versus10 |
| 100 | 23.84 ms | 33.44 ms | 0 versus100 |
| 1000 | 47.10 ms | 181.81 ms | 0 versus1000 |

Both paths receive the same full IR:2,023/9,673/87,973 bytes. Laziness saves materialization in this probe and has overhead at the smallest size. It does not remove generation, parsing or checking of those definitions. ORC supports custom program representations, so a semantic-level materializer could move the boundary earlier; implementing and measuring that boundary remains work. These process durations include startup, code generation and output and do not rank steady-state execution throughput. [ORC representations and materialization](https://llvm.org/docs/ORCv2.html)

## Whole-program graph and completion experiment

[Program graph](../experiments/architecture/semantic-program-results.graph.json) · [Reference/checker/lowering source](../experiments/architecture/semantic_program.py) · [Generated LLVM](../experiments/architecture/semantic-program-results.ll) · [Results](../experiments/architecture/semantic-program-results.json)

The whole-program experiment places the complete small checkout program, branches, calls, selected effects, deferred bodies/handlers/finally and explicit returns into a typed data graph. Inventory availability from a selected interpretation determines the business score. A dependency procedure temporarily installs either of two dependency-local interpretations, and the caller verifies its own binding after that procedure returns. Captured and current view selection produce independently observed results.

A reference machine uses explicit execution frames and task lists; Python recursion is not used for guest calls. A separate lowering walks each reachable definition once and emits ordinary recursive LLVM functions with explicit pending completion and cleanup paths. The experiment's aggregate carrier and scalar view IDs are temporary choices, not a production ABI or complete captured-value representation. Both engines execute the same graph; neither uses the current CoD frontend.

**66 complete trace comparisons passed** across22 input scenarios and three repetitions. Every event, logical resource token and release order is compared before retaining trace hashes. A separate business recurrence supplies expected results beyond engine agreement. The graph and LLVM artifact remain unchanged across all executions.

| Runtime recursion depth | Reference peak frames | Events per execution | Checked graph / generated code |
| --- | ---: | ---: | --- |
| 4 | 6 | 28 | 5 definitions,45 statements;7 LLVM definitions |
| 20 | 22 | 140 | Same |
| 200 | 202 | 1,400 | Same |
| 1000 | 1002 | 7,000 | Same |

The graph is16,381 serialized bytes; emitted LLVM is19,976 bytes. These numbers are fixed across the inputs because execution values do not participate in definition generation. Every recursive execution still performs its own effects and cleanup. This establishes a finite shared lowering for this explicit-signature fixture; it does not solve generic recursion, growing captured schemas, abstract result evidence or owner inference in the complete language.

The failure cases distinguish body failure, deferred failure, normal recovery with an original return, explicit return from a catch, return replacement in finally, and missing return after recovery without a prior result. Later cleanup runs after replacement. All logical acquisitions release exactly once in reverse scope order and TLS restores to the entry binding. These are checked obligations and integer error codes, not actual memory regions, typed catch matching or native C++ exception unwinding.

Negative controls provide stronger distinctions than a successful execution alone:

- An entry-only artifact, a wrong family result and an unknown family are rejected by the finite checker.
- Flattening all selected interpretations to the same signature-compatible method still passes type checking but changes a result from82 to140 and changes the trace. Preserving selected interpretation is therefore an independent requirement.
- A type-correct wrong resource token is rejected by both machines; the native experiment exits97 on an invariant violation. That assertion code is not a language exception policy.

At depth 1000, local median reference execution is approximately49.95ms and native process time30.66ms. Reference timing includes interpretation/trace collection; native timing includes `lli` startup, code generation and complete trace output. At smaller depths native startup dominates. These are deliberately reported with different cost boundaries and are not a runtime performance ranking.

This experiment executes control and completion from data for a small program. Its logical resource model is supplemented by the actual regional/native experiment below. Required static checks remain required; the finite checker and dynamic token assertions do not substitute for the full language's owner proof.

## Regional program/state and native borrowed-value experiment

Artifacts: [generator and runner](../experiments/architecture/regional_program.py), [regional executor and native harness](../experiments/architecture/regional_program.cc), [results](../experiments/architecture/regional-program-results.json).

The same checkout graph is stored as 168 typed node records, including the null sentinel, and five procedures in existing shilos regional containers. Intra-region edges use `regional_ptr`; cross-region addresses are temporary host `global_ptr` views. Two simultaneous read-only mappings of a known fixture file have different base addresses, identical occupied bytes and identical execution. The existing DBMR mapper supplies this test; no loading/admission protocol, external business input integration or new persistent format is defined. Execution frames and task/defer links occupy a separate actual region. Active state is not remapped by this test.

Each availability interpretation reads a divisor borrowed from an actual policy region. The host image model copies its selected-view scalar independently while preserving borrowed owners; two views can intentionally share an owner. It does not implement the complete CoD captured-value schema. A host lifetime witness survives owner destruction without retaining the region. Expired access fails before dereferencing the stale address. This is a test guard, not permission to weaken required static owner checks or a concurrent lifetime protocol.

Two native interfaces execute the shared LLVM program: an accessor copies the scalar through an output parameter, or it supplies a checked address from which LLVM loads the scalar. Both preserve actual owner lifetime and explicit completion. Region acquisitions use `auto_region`; injected root-construction exceptions are caught inside the C++ callback and become experimental error 904. Expired policy access uses 905. Neither code selects a language exception ABI, and no C++ exception crosses generated LLVM frames.

**96 comparisons passed:** 38 scenarios for each native interface, plus 10 scenarios per interface with AddressSanitizer. Each case compares complete traces from the two regional mappings, native execution and an independent logical fault projection in the earlier reference machine. The sanitized runs cover both C++ and generated LLVM; object-symbol checks verify LLVM memory-access instrumentation. Exact root-entry/free counts include failed construction, and no observed region remains live. Body/defer failure, normal recovery, explicit handler return, finally replacement, missing return, TLS restoration and remaining cleanup retain their earlier meanings.

| Recursion depth, context input 1 | Program occupation | Interpreter state occupation | Peak live observed regions | Trace events |
| --- | ---: | ---: | ---: | ---: |
| 4 | 16,056 bytes | 21,944 bytes | 8 | 28 |
| 20 | 16,056 bytes | 46,152 bytes | 24 | 140 |
| 200 | 16,056 bytes | 376,264 bytes | 204 | 1,400 |
| 1000 | 16,056 bytes | 1,859,768 bytes | 1,004 | 7,000 |

At depth 1000 the three executions together enter and free 3,005 heap regions: three policy regions, two interpreter-state regions and 3,000 lease regions. Mapped program storage is accounted separately. Both native variants retain six generated definitions, including the entry wrapper; their LLVM sizes are 21,299 and21,218 bytes. Native harness and sanitizer sizes include support code and debug information and are not generated business-code sizes.

Owner identity affects observable business behavior: with policy 0 changed to 7, keeping policy 1 at 3 produces 12, while deliberately borrowing policy 0 for both views produces 16. An expired owner propagates 905 or is recovered by an explicit handler return 777. A normal catch after body failure yields missing-return 903, while normal recovery from a deferred failure preserves the original result 14. First-acquisition failure emits no business events and still frees its partially constructed region.

The experiment supports sharing immutable regional program structure across execution locations and using explicit borrowed native views. It also reveals a separate cost: the monotonic state region retains finished frames and tasks, so fixed program/native size does not imply bounded execution storage. No frame-reuse policy, arbitrary state migration, complete capture representation, general owner proof, hosted-runtime optimization or performance winner is established. Whole-process durations include file mapping, three executions, trace comparison and output; they cannot rank the individual engines.

## Managed-host regional-memory experiment

Artifacts: [managed interpreter](../experiments/architecture/ManagedRegionalProgram.java), [native bridge](../experiments/architecture/managed_adapter.cc), [runner](../experiments/architecture/managed_runtime.py), [results](../experiments/architecture/managed-runtime-results.json). The regional records, allocation/lifetime helpers and native harness share [one fixture header](../experiments/architecture/regional_fixture.hh); the earlier 96 regional comparisons, including 20 ASAN cases, pass after that extraction.

The installed OpenJDK 21 executes the same program by interpreting actual read-only regional node records. Java retains small offset directories and temporary execution frames; it does not copy business control into a second authoritative Java AST. Direct mode reads relative links through scoped foreign-memory views. Accessor mode calls the native bridge for every field/policy read. A third mode invokes the whole shared LLVM program in one foreign call. The latter is a coarse native baseline, not framework-generated optimization.

The Java 21 [memory-segment API](https://docs.oracle.com/en/java/javase/21/docs/api/java.base/java/lang/foreign/MemorySegment.html) provides spatial, temporal and read-only access constraints; its [linker](https://docs.oracle.com/en/java/javase/21/docs/api/java.base/java/lang/foreign/Linker.html) mediates native calls. The experiment explicitly coordinates these host scopes with actual shilos ownership. A shared borrowed owner has one scope; closing it releases the native region and rejects a retained Java view before memory access. Native code independently observes the owner as expired. These host facilities do not infer CoD owner relations, selected interpretations or cleanup semantics, and they are not a concurrent unload protocol.

**342 full-trace comparisons passed**, covering 38 cases, three modes and three repetitions, plus 30 validated warmups. Runs alternate the two program mappings and shuffle mode order. Body/defer failures, root-construction failures, owner expiry, shared owners, selected interpretations, normal/explicit recovery, finally replacement, exact region frees and context restoration agree with the independent logical reference. Read-only writes, out-of-bounds reads, closed program views and expired cached borrows are rejected. The JVM is not ASAN-instrumented; the separate native fixture supplies the stated ASAN evidence.

| Depth, context input 1 | Direct / accessor foreign calls | Direct / accessor local execution median | Whole-native median | Java heap allocation per interpreted execution, median |
| --- | ---: | ---: | ---: | ---: |
| 4 | 10 / 1,598 | 0.058 / 0.089ms | 0.020ms | 15,376 bytes |
| 20 | 42 / 7,850 | 0.181 / 0.319ms | 0.057ms | 74,544 bytes |
| 200 | 402 / 78,320 | 1.716 / 3.560ms | 0.923ms | 749,592 bytes |
| 1000 | 2,002 / 391,530 | 7.173 / 12.427ms | 2.883ms | 3,808,176 bytes |

At depth 1000 both Java paths read 386,528 fields and perform 3,000 policy reads. Direct reads eliminate those 389,528 crossings while preserving 2,000 lease acquire/release calls and two completion-counter queries. Each mode enters/frees 1,003 actual policy/lease regions. This supports choosing a boundary that exposes stable record access or useful blocks of work; it does not select an engine or establish a universal speed ratio.

Timing excludes setup/mapping, oracle comparison and native trace transfer. Managed event/frame allocation is included; the native path uses a reserved event vector. Region-count instrumentation, host compilation, allocation and scheduling affect the figures. Java bytes are cumulative thread allocations, not retained heap or native-region bytes, and cannot be compared as equivalent to the previous interpreter's 1.86 MB occupied state. No statistical performance guarantee follows from three repetitions in one process.

The concrete adapter comprises three new files / 584 source lines plus shared fixture and lowering code. That is a scope observation, not a labor estimate or a language-runtime size forecast. The implementation still owns relative-link access, native/host lifetime coordination, explicit interpretation state, guest frames and return/defer propagation. Neither FFM nor HotSpot supplied those meanings. Truffle partial evaluation, RPython tracing, complete CoD captures/static owner proofs, active-state migration and multi-threaded TLS remain unmeasured by this probe.

## Reuse and engineering responsibility

All candidates require a CoD checking/fusion boundary that preserves semantic identity, selected interpretations, required static owner/access/initialization rejection and the distinction between unknown input values and fixed type-level semantics. They need no external-data mechanism for semantic registration, runtime method-set growth or world evolution. Checking that incoming values belong to the known semantic set remains necessary. None of the runtime libraries removes that work. The finite explicit-signature prototypes are evidence for a responsibility split, not an implementation of the full checker.

| Candidate | Existing machinery to reuse | CoD work exposed by the experiments | Principal unresolved cost |
| --- | --- | --- | --- |
| Shared-native preparation | LLVM optimization and machine-code generation; generic function bodies and runtime dispatch. | Finite checked representation, semantic-to-native lowering, complete value/owner transport and completion paths. Six fixed generated definitions handle the regional fixture across runtime depths. | Proving general coverage before execution, including admitted but source-unobserved tuples; avoiding expansion of generic/captured/result evidence. No interpreter/profiler is required for this path. |
| Semantic executor plus selective native functions | ORC materialization, linking and resource tracking; LLVM lowering for useful functions/blocks. | An interpreter plus lowering, an explicit execution/native boundary, compile triggers and lifetime of code/assumptions. Whole-program lowering and ORC emission granularity have separate evidence; adaptive tiering is unverified. | Keeping two execution implementations aligned and deciding code lifetime. ORC does not supply CoD checking or a cold semantic executor. Whole-function boundaries can be investigated before adding OSR/deoptimization machinery. |
| Framework-derived optimized interpreter | Truffle partial evaluation or RPython's JIT generation; host runtime compilation/collection facilities. | A framework-compatible interpreter, efficient regional access, explicit owner release, selected-view guards and framework assumptions/invalidation. The managed probe demonstrates the memory/lifetime bridge only. | Whether stable regional program structure can be exposed to the optimizer without becoming a second semantic authority, and whether framework deoptimization preserves CoD state/owners. No framework-specific performance or labor result exists yet. |

The local [ORC reoptimization interface](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/ReOptimizeLayer.h) explicitly requires retaining old code resources until old invocations finish; [compile-on-demand](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h) supplies deferred emission. This is direct infrastructure evidence, not a solved CoD code-lifetime policy.

[Truffle partial evaluation](https://www.graalvm.org/dev/graalvm-as-a-platform/language-implementation-framework/PartialEvaluation/) derives optimized execution from an interpreter and stable guest structure, while requiring correct specialization, generalization and invalidation. Its [native interface](https://www.graalvm.org/latest/graalvm-as-a-platform/language-implementation-framework/NFI/) supplies foreign calls and requires accurate native signatures; it does not provide regional ownership. Inference: Truffle may reduce separately maintained optimization semantics, but an opaque native getter for every record is an integration cost that must be measured or removed. The HotSpot result does not prove what Truffle will inline or partially evaluate.

[RPython JIT generation](https://rpython.readthedocs.io/en/latest/jit/overview.html) similarly reuses interpreter semantics, with hints and a tracing-based implementation; its [foreign-function interface](https://rpython.readthedocs.io/en/latest/rffi.html) still requires explicit low-level types and manual raw-memory management. Inference: this remains an alternative if translating the interpreter and maintaining its regional adapter is acceptable. [OMR](https://github.com/eclipse-omr/omr) provides C/C++ runtime/compiler components rather than a ready CoD semantic interpreter; selecting it would still leave the checking/execution split and language adapter to CoD. No unmeasured framework is rejected on performance grounds.

No reliable engineering-hours forecast follows from these probes. The defensible comparison is which independently maintained semantic implementations and lifecycle protocols CoD would own, supported by the adapter that was actually needed. Reusing a compiler library and reusing an optimizing interpreter framework are materially different commitments.

## Selection criteria and evidence gaps

These criteria are agent recommendations for whole-file review. The appropriate performance comparator is an optimized native implementation of equivalent business behavior, with the same ownership, dispatch and completion obligations. The retained probes do not yet establish an acceptable throughput or latency ratio. No numerical threshold is selected.

| Criterion | Retained evidence and limit | Consequence |
| --- | --- | --- |
| Preserve selected interpretations | The whole-program checker rejects an entry-only graph. Signature-compatible flattening changes the result and trace. | Retain dependency-local views independently of current bindings in every candidate. |
| Cover external values of known ADTs | H-390 fixes the semantic set. The retained native fixtures do not validate multi-argument ADT dispatch on a source-unobserved external tuple or same-layout distinct semantic types. | These remain acceptance requirements and explicit validation gaps. Do not infer coverage from source-observed tuples or layout equality. |
| Keep native work independent of execution count | Five checked procedures lower to fixed code through recursion depth1000; the regional fixture retains six generated definitions. | Shared lowering works for these explicit signatures. General generics, changing capture schemas and type-combination growth remain unmeasured. |
| Distinguish frontend work from native emission | ORC changes cold materialization while receiving identical complete IR. | Backend laziness alone leaves frontend expansion intact. Retained semantics must permit shared lowering or a genuinely earlier demand boundary. |
| Preserve actual owners and completion | 66 logical, 96 regional/native and 342 managed/native comparisons cover their documented cases;20 regional cases use ASAN. | Dynamic fixture assertions do not establish general static ownership or complete captured-value transport. |
| Remove avoidable execution layers | The managed fixture records391,530 accessor crossings versus2,002 direct-view crossings at depth1000. | Direct access reduces crossings in this interpreter; it does not prove optimizer removal of dispatch or competitive native throughput. |
| Account for state and allocation | A fixed16,056-byte program uses1,859,768 bytes of monotonic interpreter state at depth1000. | Measure live state, retained allocations and reclamation separately from code size. |
| Establish native performance explicitly | Existing timings include different startup, tracing, allocation and boundary costs. No matched optimized-native performance study exists. | Measure generated code, warm throughput, first-call/tail latency, allocation and memory traffic against equivalent native execution. Name residual dynamic costs and justify them. |
| Demonstrate engineering savings | The managed adapter requires explicit record access, ownership, frames and completion. Framework optimization has no measured result. | Optimizer reuse is a hypothesis. Neither a JIT label nor host-memory integration establishes lower implementation burden. |

The evidence excludes entry-only flattening and shows why backend materialization alone cannot solve mandatory frontend expansion. It does not establish a performance winner among the three candidates. External-value coverage, semantic identity, complete callable values and suppressed-installation failure/cleanup remain required even where this experiment set supplies no integrated check.

## Recommendation and next bounded stage

**Retain a checked semantic program as the common architectural boundary.** The whole-program and regional fixtures support sharing definitions while retaining selected interpretations, runtime owners and completion. Native code remains a derived execution product. General checking, inference and capture representation remain design work; fixed fixture code size does not solve them automatically.

**Prioritize validation of shared-native lowering against an equivalent optimized native reference.** The fixed fused semantic set permits coverage over known alternatives without runtime type/method registration. Shared native preparation preserves the recorded AOT timing while allowing runtime values to select prepared targets. This is the next proposed investigation, not a production architecture selection or a return to expanding the old frontend's backlog. General coverage and engineering affordability still need evidence.

A bounded next stage should use one regional business fixture and:

1. Add dispatch over known sum alternatives, including a source-unobserved input pair and same-layout distinct semantic identities; reject unknown alternatives and attempted semantic extension without changing the fused artifact.
2. Exercise complete captured/current values, actual owners and initialization-write suppression with operand effects/failures/cleanup through both the semantic reference and shared-native path. Keep business control in the checked graph.
3. Compare the generated implementation with an independently written optimized native reference under matched semantics and instrumentation. Separate trace-based correctness runs from timing runs; report compilation cost, code size, cold/first-call latency, warm throughput, allocations and live/retained memory. Inspect generated code for interpreter dispatch, indirect calls, redundant checks and boundary crossings, preserving genuinely dynamic operations.

These are proposed validation tasks, not executed tests or an authorization to implement pending language mechanisms. The current native harness provides a starting point, but its instrumented timings do not meet this performance comparison requirement.

**Truffle/RPython remain unmeasured alternatives without priority over the native path.** A framework proposal must demonstrate that hot guest control and regional accesses compile into efficient native execution, preserve owners/interpretations/completion, and reduce the semantics CoD maintains independently. Java foreign-memory integration supplies no such optimizer evidence. Framework-specific experiments should answer a concrete limitation found in the shared-native study or demonstrate a measurable engineering advantage; no default custom tracing/OSR subsystem is proposed.

Native-before-execution remains the recorded baseline. Interpreting cold operations or preparing code during execution is a separate pending trade-off requiring review of latency and code/compilation costs. The fixed semantic set supplies no correctness reason to require such a timing change. Runtime selection, exact performance acceptance and final whole-file qualification remain with the user.

No production input-validation/transport protocol, separate composition/publication mechanism, permanent ABI/checksum, general new owner/inference design, source migration, pure/TCO/fractions or mailbox/network subsystem belongs to this bounded proposal. Framework installation and main-build configuration retain their independent authorization requirements.

## Retained evidence coverage

The reports below describe their recorded executions. This analysis supplies a scoped recommendation and explicit gaps; it does not certify full-language conformance, production performance or a selected engine.

| Requirement | Evidence and remaining scope |
| --- | --- |
| Preserve the old closing increment | Commit `34cf632` and its validation report retain the original diagnostic mismatch and targeted rerun; completing the old backlog is not the next stage. |
| Compare materially different responsibilities | Shared-native, semantic-plus-native and framework-derived execution are compared. Frameworks retain research-only optimizer status. |
| Preserve interpretations and bindings |30 LLVM materialization runs; whole-program dependency-local selection/restoration and entry-only/flattening controls. Complete production fusion and initialization failure/cleanup integration remain unverified. |
| Preserve ownership, relocation and completion |66 whole-program,96 regional/native including20 ASAN, and342 managed/native comparisons plus30 managed warmups. Complete callable values and general static proofs remain gaps. |
| Cover external values and retain semantic identity | Confirmed fixed-ADT boundary and explicit coverage requirements. Multi-argument external dispatch, same-layout identity discrimination and permanent RTTI/layout mechanisms have no qualification from these retained experiments. |
| Bound code and execution storage independently | Fixed whole-program/native definitions across recursion depths; regional state occupation and managed allocation/crossing observations. No general type-combination scaling law is established. |
| Establish performance and affordability | Existing measurements expose specific costs but do not provide a matched optimized-native benchmark or engineering-hours forecast. The proposed next stage addresses this gap. |
| Preserve authority and scope | H-387/H-390/H-392 and K-581 govern this analysis; reports preserve their original provenance. Production mechanisms, engine and timing changes remain pending. |
