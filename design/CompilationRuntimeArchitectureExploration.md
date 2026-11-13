# Compilation and runtime architecture exploration

Design revision: 2 · Recorded: 2026-11-12 America/Los_Angeles · Status: authorized exploration; candidate production designs pending whole-file review.

## Design authority

- **Record and scope:** [D-25](DesignAuthorityRegistry.md#d-25), all sections. [H-387](DesignAuthorityRegistry.md#h-387) directly authorizes multiple architecture designs and isolated prototype validation; [K-573](DesignAuthorityRegistry.md#k-573) records scope and affected qualifications; [K-574](DesignAuthorityRegistry.md#k-574) records revision2 experimental evidence without qualifying candidate mechanisms. It does not approve any candidate mechanism or final architecture.
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

An entry extension still requires explicit entry installation. Only affected dependency **initialization TLS writes** are suppressed. Operand construction, effects, failures and cleanup remain; dependency procedure-local installations and explicit source views remain reachable when used. Fusion therefore starts at entry reachability but follows these local interpretations too. Runtime TLS values, captures, region instances and scope changes are execution state, not new copies of the fused type artifact.

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

**Status distinction:** execution state, kernel bytecode, layout records, method descriptors and the native boundary are implemented in the experiments. Full program control/defer is host scaffolding in the first model. WebAssembly native kernels are real, but a complete semantic-program executor, production native-readiness proof, regional integration, OSR/deoptimization and general semantic checking remain unverified. Truffle, RPython, Sulong and OMR have research evidence only.

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

## Criteria derived from the evidence

These are provisional selection criteria produced by the exploration, not human-approved thresholds or language policies.

| Criterion | Evidence and consequence | Candidate implications |
| --- | --- | --- |
| Retain all reachable interpretations without merging them | Entry-only negative control loses a dependency-local view; explicit views preserve different results. | Every candidate needs fusion independent of current TLS state. An entry-only runtime artifact is eliminated. |
| Remain correct on admitted but unobserved input tuples | Observed-only control misses the injected pair; generic semantic dispatch executes it. | Source-observed specialization alone cannot be the executable contract. Preserve a coverage proof or a general path; new-definition admission remains open. |
| Bound native work separately from semantic metadata | Three kernels handle increasing tuple products, while metadata grows; earlier CoD recomposition artifacts also grow. | Compare checked records, code bodies and bytes independently. Shared native AOT remains a serious candidate; tuple count alone does not justify JIT. |
| Avoid mandatory full frontend lowering before a cold path can execute | ORC saves native work but receives identical complete IR in both modes. | Merely changing the current backend to lazy LLVM/Sulong fails this criterion. A semantic executor or demand-driven semantic-to-native boundary addresses the missing responsibility. |
| Preserve real binding/owner/completion boundaries | Model modes agree under rebinding, remapping and failure/recovery checks. Production native unwinding/borrowing remain untested. | Any candidate must pass integrated region/exception tests before promotion. Native throughput alone is insufficient. |
| Demonstrate useful work per native crossing | Native kernel gains are modest in the host-controlled workload; cold costs differ. | Investigate whole semantic blocks/functions and state-transfer cost before a framework or tiering policy is chosen. |
| Make implementation burden explicit | ORC reuses linking/materialization; Truffle/RPython reuse interpreter optimization; all leave CoD semantic integration work. | Compare owned subsystems and measured adapter effort. No measured labor ranking exists, so installation availability and sunk costs cannot pick a winner. |

No numerical elimination cutoff, weighted score or universal performance claim is established. The two eliminated controls are incomplete semantics; the backend-only change is insufficient to solve the stated frontend problem. None of the three architectural candidates is eliminated by these bounded experiments.

## Recommendation and next bounded stage

**Recommendation:** retain a checked semantic program representation and make native generation a consumer of that representation. Keep execution values, TLS bindings, actual owners and pending completion separate. Preserve both shared-native readiness and cold semantic execution as alternatives until an integrated program demonstrates their engineering costs. This is an exploratory recommendation, not approval of a production IR, checker or runtime ABI.

The next decisive prototype should put the **whole small business program and completion flow into inspectable data**, not just method kernels. It should retain one finite recursive graph across changing runtime contexts, select at least two dependency-local interpretations, and execute an effect-result-dependent business branch. Run the same graph through a reference executor and a shared native lowering boundary; compare checking work, materialized bodies, state-transfer costs and trace/resource behavior. This addresses the largest evidence gap in the present common host scaffold.

For reusable optimizing runtimes, the discriminating follow-up is an actual region-memory and cleanup adapter in a candidate framework, not an arithmetic speed test. Truffle/RPython/OMR require separate environment preparation under the existing toolchain restrictions; no installation or production commitment follows from this recommendation. ORC can be probed further with the existing tree, but convenience alone does not make it the winner.

Whole-file review should eventually decide the production checking/execution boundary and native-before-execution contract, informed by those additional results. Layout/checksum/ABI choices, external graph admission and world evolution remain separate topics. Exploration remains active; this revision records the first evidence milestone, not a completed architecture selection.
