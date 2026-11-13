# Compilation and runtime architecture exploration

Design revision: 5 · Recorded: 2026-11-13 America/Los_Angeles · Status: exploration deliverable ready for whole-file review; candidate production designs remain pending.

## Design authority

- **Record and scope:** [D-25](DesignAuthorityRegistry.md#d-25), all sections. [H-387](DesignAuthorityRegistry.md#h-387) directly authorizes multiple architecture designs and isolated prototype validation; [K-573](DesignAuthorityRegistry.md#k-573) records scope and affected qualifications; [K-574](DesignAuthorityRegistry.md#k-574)/[K-575](DesignAuthorityRegistry.md#k-575)/[K-576](DesignAuthorityRegistry.md#k-576)/[K-577](DesignAuthorityRegistry.md#k-577) record revisions2–5 experimental evidence without qualifying candidate mechanisms. It does not approve any candidate mechanism or final architecture.
- **Candidate value path:** C-85/P-359 → C-01/C-04 and D-25/P-360 → C-85/C-02: preserve inspectable typed business intent, separate program/interpretation/execution state, and find an affordable route to native execution. These detailed agent-proposed derivations remain pending-human. The governing charter is [§§1–3, 4.3, 5–6, 8 and 12](RegionalSemanticPlatformCharter.md).
- **Constraints/dependencies:** P-361 depends on D-01/D-10/D-14/D-15/D-17/D-18/D-21; D-16 additionally constrains numeric and semantic identity. P-362 records the user's fused-artifact, open-input and experimental RTTI boundaries. Preserve actual owners, independent complete callable values, zero-cost regional relocation, selected interpretations, perform/behave/touch, return/exception/defer and initialization operand effects.
- **Baseline:** the current project user explicitly redirected work in the [architecture side chat](codex://threads/01a1d360-c1b0-7ca0-bd02-ea7ef88d2e0f), through the authorized [work package](../../.codex-handoffs/cod-project/2026-11-12-architecture-exploration/WORK_PACKAGE.md). Exact message timestamps and personal identity are unavailable. H-387 records excerpts and exclusions; H-386 requires topic discussion followed by whole-file approval.
- **Acceptance/non-goals:** produce reproducible experiments, empirical selection criteria, recommendations and a bounded next stage. Workloads, metrics, thresholds and rankings are outputs of exploration, not preapproved policy. No final selection, production ABI/checksum/layout, external-input integration/evolution protocol, DBMR/mailbox/IO subsystem, owner union or pure/TCO/fraction/compiler-pilot implementation is authorized here. Existing build/toolchain restrictions continue.

## Closing baseline and direction

The user classifies continued development of the current direct-to-bitcode route as **expected engineering effort unaffordable**. This is a planning judgment, not a measured labor estimate. Existing implementation effort is excluded from candidate selection.

Commit `34cf632` preserves the smallest closing increment. [Its report](../tests/cod/language/callable-recomposed-factory-validation.json) records 654 language cases verified, 57 native callable cases, 23 defer cases and three AddressSanitizer programs. The language evidence retains the original 653/654 run and targeted verification of an updated diagnostic expectation. All 77 preceding resource comparisons match. Recursive depth through 1000 retains 19 native definitions for the new primary cases. Repeated recomposition at 1/2/4/8/16 stages emits 8/11/17/29/53 definitions while bitcode grows 21,224/36,824/76,816/194,096/584,376 bytes. These are bounded observations, not a universal growth law or measured engineering cost.

Reusable material includes the observable context/ownership/cleanup cases, diagnostics, native resource accounting and historical failed probes. Unresolved actual-owner selection, abstract result evidence, construction/DBMR, module defer and family-dispatch mechanisms remain unresolved. Completing that backlog is not a prerequisite for this exploration.

## Responsibilities and preserved boundaries

| Responsibility | Required meaning | What it does not establish |
| --- | --- | --- |
| Fused semantic artifact | Retain all reachable selected interpretations, types, callable contracts/methods and procedure-local effect information from parallel module worlds. | Entry-world-only flattening, one merged method set, or a runtime module composer. |
| Checking completeness | Preserve the checks and evidence required to execute admitted programs; represent open input boundaries explicitly. | Every possible runtime tuple observed in source, or native code for every tuple already emitted. |
| Native preparedness | State precisely which admitted operations have executable native implementations before execution. | Semantic completeness by renaming it AOT completeness. |
| Dispatch | Select a method using actual semantic argument types and the callable's retained interpretation. | Dispatch by layout checksum alone or by a process-wide latest world. |
| Specialization | Optimize a stable subset with explicit assumptions and a correct general path. | Permission to remove effects, change owners or reinterpret existing values. |

The [existing AOT contract](AheadOfTimeMultipleDispatch.md) commits to native targets/dispatch before covered execution. Shared generic native bodies can satisfy that commitment without separate machine code for every tuple if coverage is proved. Interpreter or lazy-native alternatives change the timing contract and need final approval. H-387 authorizes investigating that change; it does not silently approve a weaker meaning of AOT.

An entry extension still requires explicit entry installation. Only affected dependency **initialization TLS writes** are suppressed. Operand construction, effects, failures and cleanup remain; dependency procedure-local installations and explicit source views remain reachable when used. Fusion therefore starts at entry reachability but follows these local interpretations too. Runtime TLS values, captures, region instances and scope changes are execution state, not new copies of the fused type artifact. A candidate must preserve required static owner/access/initialization rejection; experimental runtime assertions do not authorize moving those checks to runtime or introducing owner unions.

Runtime DBMR content graphs, future mailbox packets and possible file/network graphs can supply actual type combinations invisible to source construction paths. A carrier graph's representation type is distinct from the business types it describes. This document does not decide whether such inputs can introduce new semantic definitions or how they would be integrated. Experiments must state their local assumptions and retain at least one externally supplied combination absent from source-observed pairs.

Exploratory layout evidence may use checksums over complete lower-level memory-layout trees, including sum tags. Experiments must keep semantic identity separate and expose same-layout/different-meaning cases. A checksum match is not proof of equality without collision handling. Recursive type canonicalization, target layout, padding, offsets, permissions, UoM, nominal identity and method interpretation cannot be silently collapsed. No checksum algorithm or persistent representation is selected.

## Candidate responsibility splits

| Candidate | Checking and fusion | Cold execution and native work | Reuse and main burden |
| --- | --- | --- | --- |
| Shared native implementations | Check one semantic graph with symbolic parameters and explicit context/owner evidence; retain dispatch descriptors. | Prepare generic native bodies and dispatch before execution; specialize only where profitable. | LLVM optimizer/code generator; CoD must design finite checking, value/evidence transport and generic native operations. No interpreter required, but native-readiness proof still matters. |
| Semantic execution engine with optional native regions | Check and retain the semantic graph; execute explicit state and context operations. | Interpret cold operations; compile selected hot regions/functions from retained semantics. | LLVM ORC can manage native materialization and resources. CoD still owns interpreter, lowering, boundary state and assumptions. A lazy backend alone does not repair frontend expansion. |
| Interpreter on a reusable optimizing runtime | Retain semantic nodes and implement their meaning using a framework's interpreter conventions. | Framework profiles and compiles interpreter execution; CoD exposes appropriate stable data and guards. | Truffle or RPython can reuse more optimization machinery. CoD must adapt regional storage, lifecycle, semantic evidence and host/guest boundaries; host object graphs cannot replace relocatable business graphs. |

These are architectural alternatives, not three settings of the existing frontend. A hosted WebAssembly/JavaScript experiment can test explicit state and native-call boundaries with available tools, but does not establish Truffle/RPython performance or select WebAssembly as the final target.

## Reusable infrastructure and evidence limits

- [LLVM ORC](https://llvm.org/docs/ORCv2.html) supplies JIT linking/materialization infrastructure. The local [ReOptimizeLayer](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/ReOptimizeLayer.h) exposes profiling and reoptimization callbacks; [CompileOnDemandLayer](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h) supports deferred emission. Those are useful components, not a CoD semantic checker, interpreter, OSR policy or complete execution-state protocol.
- [Truffle partial evaluation](https://www.graalvm.org/dev/graalvm-as-a-platform/language-implementation-framework/PartialEvaluation/) derives optimized code from an interpreter and stable guest structure. Its documentation also requires careful specialization, generalization and invalidation. Inference for CoD: preserving semantic nodes may avoid duplicated meaning in a separate compiler, but adapting regional memory and dynamic binding boundaries still needs a real prototype.
- [GraalVM LLVM Runtime](https://www.graalvm.org/latest/reference-manual/llvm/) interprets LLVM bitcode and compiles hot parts. It is a candidate execution engine, but feeding it the existing fully expanded frontend output retains that frontend cost. A shared or semantic lowering is still needed to address the principal architectural issue.
- [RPython's JIT generator](https://rpython.readthedocs.io/en/latest/jit/overview.html) builds a tracing JIT from an interpreter with hints. This may reuse optimizer engineering, but requires a suitable RPython implementation and a regional/native integration design. It is not a drop-in optimizer for the current C++ frontend.
- [Eclipse OMR](https://github.com/eclipse-omr/omr) offers runtime/compiler components. Its component scope warrants comparison, but no OMR integration, cost or performance result is claimed here.

Available local tools include the current build's LLVM execution tools, Node/V8 and a Java runtime. GraalVM, RPython, OMR and added dependencies have not been installed. Tool availability affects which assumptions can be tested immediately; it is not an architecture ranking criterion.

## Implemented boundary experiments

[Source, runner and reproduction notes](../experiments/architecture/README.md) · [Raw results](../experiments/architecture/results.json)

The finite model retains base/extended/source interpretations and a dependency-local installation in one artifact. Execution holds bindings, independent callable snapshots, actual owner IDs and pending completion. Both argument types affect method selection; the second argument uses fixture-specific disjoint semantic classes. The model compares a small kernel interpreter, three prepared shared WebAssembly kernels and interpretation followed by per-kernel compilation after eight calls. The trigger is an experimental choice, not a recommended threshold. It does not use the existing CoD frontend.

The regional model uses shared linear memory, self-relative payload references and a separate owner-to-base map. Remapping copies whole bytes to simulate a changed mapping, then changes one base record without patching payload fields. This is a test of the addressing distinction, not a production allocator, an actual mmap experiment or a claim that physical copying is free. Actual shilos protocols remain unchanged.

Fifteen checks cover entry installation, complete fusion, two-argument semantic dispatch despite identical layouts, current/captured calls with owner rebinding, independent copies, touch without invocation, source views, procedure-local installation/restoration, suppressed initialization writes with operand/failure/cleanup behavior, readonly/expired-owner rejection and return/exception/defer transitions. The last transitions share a host scaffold; typed catch matching, cause chains, native unwinding, prepared owning arguments and complete CoD borrow checking are not implemented. An entry-only artifact is rejected. An observed-pairs-only control misses the injected external tuple. The external tuple uses existing type definitions; admitting new definitions is deliberately outside the model.

A separate LLVM probe uses real TLS, dependency-local rebinding/restoration, preserved operand effects and captured/current inputs. The same independently generated IR runs through ORC with whole-module or function-level materialization. The function dumps record which cold definitions reach the native layer. This tests code-generation granularity, not a CoD checker, final-image dead-code elimination or a whole-language interpreter.

**Status distinction:** execution state, kernel bytecode, layout records, method descriptors and the native boundary are implemented in the experiments. Full program control/defer is host scaffolding in the first model; later experiments represent those operations as data and exercise actual regional/native integration. WebAssembly native kernels are real, but production native-readiness, OSR/deoptimization and general semantic checking remain unverified. The whole-program executor below covers its stated finite language only. Truffle, RPython, Sulong and OMR have research evidence only.

## Observations

Five repetitions with deterministic shuffled mode order produced **90 successful model runs and 30 successful LLVM runs**. Each run uses a fresh process. The report records tool/source hashes, commands, all raw timings, traces and limits. Node/V8 runs disable lazy WebAssembly compilation and automatic Wasm tier-up, so prepared mode compiles all three kernels before the model's operations. This is kernel preparedness, not whole-program AOT. [V8 compilation controls](https://v8.dev/docs/wasm-compilation-pipeline)

| Types / interpretations | Possible tuples | Serialized semantic artifact | Shared kernel modules after the repeated workload |
| --- | ---: | ---: | ---: |
| 3 / 3 | 27 | 3,949 B | 3; 213 B total Wasm |
| 8 / 4 | 256 | 11,233 B | 3; 213 B total Wasm |
| 32 / 16 | 16,384 | 116,411 B | 3; 213 B total Wasm |

This is constructive evidence that every tuple need not receive independent code **for this method structure**. Semantic metadata still grows. WebAssembly bytes are not native machine-code bytes. General predicates, generic bodies, unsatisfied constraints and owner-dependent operations could change both growth and sharing.

At32 types/16 interpretations and20,000 calls, measured loop times were:

| Kernel execution | Median | Min–max | Cold model setup/check actions, median |
| --- | ---: | ---: | ---: |
| Interpreted | 19.343 ms | 19.019–22.767 ms | 1.823 ms |
| Prepared native | 17.786 ms | 16.870–18.931 ms | 2.441 ms |
| Selectively native | 18.488 ms | 17.508–19.047 ms | 2.294 ms |

These include JS dispatch, owner checks, temporary argument objects and native crossings; they do not isolate arithmetic throughput. Cold timing includes assertion scenarios and excludes artifact checking measured separately. Host startup is separately recorded. The modest differences support investigating boundary and dispatch costs before choosing a JIT framework. They do not predict native CoD throughput or establish a universally best tier threshold. Some cold assertion sequences already trigger one kernel's compilation in selective mode.

| Uncalled IR definitions | Function-level materialization, median process time | Whole-module materialization, median process time | Cold functions materialized |
| --- | ---: | ---: | --- |
| 10 | 21.05 ms | 18.00 ms | 0 versus10 |
| 100 | 23.84 ms | 33.44 ms | 0 versus100 |
| 1000 | 47.10 ms | 181.81 ms | 0 versus1000 |

Both paths receive the same full IR:2,023/9,673/87,973 bytes. Laziness saves materialization in this probe and has overhead at the smallest size. It does not remove generation, parsing or checking of those definitions. ORC supports custom program representations, so a later semantic-level materializer can move the boundary earlier; that is a design/implementation task, not a benefit already obtained by switching the existing backend. [ORC representations and materialization](https://llvm.org/docs/ORCv2.html)

## Whole-program graph and completion experiment

[Program graph](../experiments/architecture/semantic-program-results.graph.json) · [Reference/checker/lowering source](../experiments/architecture/semantic_program.py) · [Generated LLVM](../experiments/architecture/semantic-program-results.ll) · [Results](../experiments/architecture/semantic-program-results.json)

The second experiment places the complete small checkout program, branches, calls, selected effects, deferred bodies/handlers/finally and explicit returns into a typed data graph. Inventory availability from a selected interpretation determines the business score. A dependency procedure temporarily installs either of two dependency-local interpretations, and the caller verifies its own binding after that procedure returns. Captured and current view selection produce independently observed results.

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

This experiment closes the first model's host-control gap for a small program. Its logical resource model is supplemented by the actual regional/native experiment below. Required static checks remain required; the finite checker and dynamic token assertions do not substitute for the full language's owner proof.

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

All candidates require a CoD checking/fusion boundary that preserves semantic identity, selected interpretations, required static owner/access/initialization rejection and the open-input distinction. None of the runtime libraries removes that work. The finite explicit-signature prototypes are evidence for a responsibility split, not an implementation of the full checker.

| Candidate | Existing machinery to reuse | CoD work exposed by the experiments | Principal unresolved cost |
| --- | --- | --- | --- |
| Shared-native preparation | LLVM optimization and machine-code generation; generic function bodies and runtime dispatch. | Finite checked representation, semantic-to-native lowering, complete value/owner transport and completion paths. Six fixed generated definitions handle the regional fixture across runtime depths. | Proving general coverage before execution, including admitted but source-unobserved tuples; avoiding expansion of generic/captured/result evidence. No interpreter/profiler is required for this path. |
| Semantic executor plus selective native functions | ORC materialization, linking and resource tracking; LLVM lowering for useful functions/blocks. | An interpreter plus lowering, an explicit execution/native boundary, compile triggers and lifetime of code/assumptions. Kernel tiering and whole-program lowering are demonstrated separately. | Keeping two execution implementations aligned and deciding code lifetime. ORC does not supply CoD checking or a cold semantic executor. Whole-function boundaries can be investigated before adding OSR/deoptimization machinery. |
| Framework-derived optimized interpreter | Truffle partial evaluation or RPython's JIT generation; host runtime compilation/collection facilities. | A framework-compatible interpreter, efficient regional access, explicit owner release, selected-view guards and framework assumptions/invalidation. The managed probe demonstrates the memory/lifetime bridge only. | Whether stable regional program structure can be exposed to the optimizer without becoming a second semantic authority, and whether framework deoptimization preserves CoD state/owners. No framework-specific performance or labor result exists yet. |

The local [ORC reoptimization interface](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/ReOptimizeLayer.h) explicitly requires retaining old code resources until old invocations finish; [compile-on-demand](../llvm-project/llvm/include/llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h) supplies deferred emission. This is direct infrastructure evidence, not a solved CoD code-lifetime policy.

[Truffle partial evaluation](https://www.graalvm.org/dev/graalvm-as-a-platform/language-implementation-framework/PartialEvaluation/) derives optimized execution from an interpreter and stable guest structure, while requiring correct specialization, generalization and invalidation. Its [native interface](https://www.graalvm.org/latest/graalvm-as-a-platform/language-implementation-framework/NFI/) supplies foreign calls and requires accurate native signatures; it does not provide regional ownership. Inference: Truffle may reduce separately maintained optimization semantics, but an opaque native getter for every record is an integration cost that must be measured or removed. The HotSpot result does not prove what Truffle will inline or partially evaluate.

[RPython JIT generation](https://rpython.readthedocs.io/en/latest/jit/overview.html) similarly reuses interpreter semantics, with hints and a tracing-based implementation; its [foreign-function interface](https://rpython.readthedocs.io/en/latest/rffi.html) still requires explicit low-level types and manual raw-memory management. Inference: this remains an alternative if translating the interpreter and maintaining its regional adapter is acceptable. [OMR](https://github.com/eclipse-omr/omr) provides C/C++ runtime/compiler components rather than a ready CoD semantic interpreter; selecting it would still leave the checking/execution split and language adapter to CoD. No unmeasured framework is rejected on performance grounds.

No reliable engineering-hours forecast follows from these probes. The defensible comparison is which independently maintained semantic implementations and lifecycle protocols CoD would own, supported by the adapter that was actually needed. Reusing a compiler library and reusing an optimizing interpreter framework are materially different commitments.

## Criteria derived from the evidence

These are provisional selection criteria produced by the exploration, not human-approved thresholds or language policies.

| Criterion | Evidence and consequence | Candidate implications |
| --- | --- | --- |
| Retain all reachable interpretations without merging them | Entry-only negative control loses a dependency-local view; explicit views preserve different results. | Every candidate needs fusion independent of current TLS state. An entry-only runtime artifact is eliminated. |
| Remain correct on admitted but unobserved input tuples | Observed-only control misses the injected pair; generic semantic dispatch executes it. | Source-observed specialization alone cannot be the executable contract. Preserve a coverage proof or a general path; new-definition admission remains open. |
| Bound native work separately from semantic metadata | Three kernels handle increasing tuple products, while metadata grows; earlier CoD recomposition artifacts also grow. | Compare checked records, code bodies and bytes independently. Shared native AOT remains a serious candidate; tuple count alone does not justify JIT. |
| Avoid mandatory full frontend lowering before a cold path can execute | ORC saves native work but receives identical complete IR in both modes. | Merely changing the current backend to lazy LLVM/Sulong fails this criterion. A semantic executor or demand-driven semantic-to-native boundary addresses the missing responsibility. |
| Preserve real binding/owner/completion boundaries | Whole-program engines match 66 logical traces; actual regional/native boundaries match 96 further cases, including 20 sanitized cases, remapping, owner expiry and construction failure. General static borrow proofs and cross-native unwinding remain untested. | Any candidate must pass integrated region/exception tests before promotion. Native throughput alone is insufficient. |
| Demonstrate useful work per native crossing | The managed fixture needs 391,530 accessor calls versus 2,002 direct-view calls at depth 1000, with equal traces; a coarse native execution crosses once. | Prefer direct validated record access or useful blocks. Do not assume a framework will erase opaque foreign calls; verify its generated execution. |
| Share program structure without sharing execution | A fixed45-statement graph executes7,000 events at recursion depth 1000 with independent cleanup; flattened signature-compatible interpretations change behavior. | Share definitions under retained semantic identity. Do not infer shared execution or erase selected views from code sharing. |
| Account for execution storage independently | A fixed 16,056-byte regional program uses 1,859,768 bytes of monotonic interpreter state at depth 1000. | Compare live state, retained allocation and reclamation obligations separately from code sharing. Do not infer bounded memory from fixed code size. |
| Make implementation burden explicit | The managed adapter needs explicit record access, lifetimes, frames and completion despite host memory/call services. Framework documentation separates optimizer reuse from language integration. | Compare independently maintained semantics and lifecycle obligations. No labor-hours ranking exists; installed tools and sunk costs cannot select the production runtime. |

No numerical elimination cutoff, weighted score or universal performance claim is established. The two eliminated controls are incomplete semantics; the backend-only change is insufficient to solve the stated frontend problem. None of the three architectural candidates is eliminated by these bounded experiments.

## Recommendation and next bounded stage

**Recommend a checked, retained semantic program as the common architectural boundary.** Fuse reachable module interpretations there; keep TLS values, actual owners and completion in execution state. Native generation consumes this representation. The evidence rules out entry-world flattening, observed-tuples-only coverage and backend laziness as a complete repair for mandatory frontend expansion.

**Prioritize a bounded framework-derived interpreter pilot, with shared-native preparation as the comparison baseline.** This prioritizes reducing independently maintained optimization semantics, supported by the actual regional/managed adapter and the framework's documented facilities. It is a recommendation for the next investment, not a claim that Truffle is already faster, cheaper or selected. Truffle is the first proposed framework because it directly targets interpreter-derived optimization; RPython remains a documented alternative. A custom tracing JIT is not the default fallback.

The proposed checking/execution split keeps required semantic and owner checks before admitted execution and allows cold semantic execution before native preparation of that operation. This changes the earlier native-before-execution promise and is explicitly pending whole-file approval. If the user retains that promise, use the same retained representation with shared-native preparation instead; native coverage still needs its own proof. Neither path turns semantic completeness into a native-code timing synonym.

The next stage has one fixture and one decision: can an optimizing framework consume the regional program efficiently while preserving selected interpretations, actual ownership and completion? Reuse the checked regional fixture and native baseline; add the initial model's source-unobserved admitted tuple and complete captured/current-value cases at that boundary. Expose immutable program structure to the framework as a disposable execution view derived from the authoritative regional graph. Its exact representation is a pilot hypothesis, not an approved second program format.

The pilot should provide compilation evidence that hot guest control loses interpreter/foreign-field dispatch, complete trace and owner-release agreement across context changes/failures, and separate cold setup, warm execution, native crossings, code growth and host/native state measurements. Reject the proposed integration if it needs authoritative host-owned business graphs, weakens required static checks, changes actual owners, or erases interpretations. If useful optimization remains unproved, retain the shared-native baseline and reassess the adapter rather than adding a general tracing/OSR subsystem. These are proposed pilot acceptance conditions, not retroactively approved language policy.

Framework-specific preparation is outside this completed isolated-tool experiment and remains subject to the repository's existing environment authorization. No full source migration, general new owner/inference design, external-input admission/world evolution, permanent ABI/checksum, pure/TCO/fractions or mailbox/network subsystem belongs to that bounded pilot. Complete captured values and static proofs remain mandatory before any production replacement is promoted.

## Exploration deliverable coverage

The [deliverable audit](../experiments/architecture/exploration-deliverable-audit.json) checks the work-package requirements, current evidence/source/tool hashes and preserved authority history. This package completes the requested comparative exploration and produces a reviewable next-stage recommendation. It does **not** qualify a production architecture. Whole-file approval and final engine/checking/native-timing decisions remain with the user.

| Work-package requirement | Reviewable evidence and scope |
| --- | --- |
| Close the smallest old increment, preserve reusable evidence and stop expanding its backlog | Commit `34cf632` and the closing validation report; the original diagnostic mismatch and targeted rerun remain visible. |
| Compare materially different architectures without sunk-cost preference | Shared-native, semantic-plus-native and framework-derived-interpreter responsibility splits; independent generated-native and semantic executors, with managed/native adapter evidence. |
| Preserve fused interpretations and effect/install boundaries | Initial 15-check model and real LLVM TLS probe; whole-program dependency-local selection/restoration and signature-compatible flattening control. Production fusion/dispatch is not claimed implemented. |
| Preserve actual ownership, relocation, independent values and cleanup | Initial complete-image model; actual two-address regional mappings, borrowed owners, real construction failure, native ASAN and managed-scope checks; explicit full-language gaps are retained. |
| Keep external inputs and RTTI distinctions explicit | Admitted source-unobserved tuple control; carrier/business-type distinction; same-layout distinct semantic types and experimental checksum. Admission/evolution and permanent identity remain unselected. |
| Separate checking, native preparedness, dispatch and optimization | Responsibility table, ORC cold-materialization results, finite checking/shared lowering, explicit proposed native-timing change and unchanged static-check obligation. |
| Assess practical reuse and engineering burden | Official framework/runtime sources, local ORC interfaces, implemented 584-line managed adapter, and subsystem/lifecycle responsibility comparison; no invented hours or unmeasured framework performance claim. |
| Derive empirical criteria and a manageable recommendation | Criteria follow failure controls, code/metadata/state growth and measured crossings; one bounded framework pilot against shared-native preparation, with explicit limits and alternatives. |
| Maintain authority, build and scope discipline | H-387/K-573–K-577, unchanged historical confirmations, exact successful main builds and reproducible isolated runners. Candidate mechanisms remain pending; no bootstrap/toolchain/configuration changes or excluded language/subsystem implementation. |
