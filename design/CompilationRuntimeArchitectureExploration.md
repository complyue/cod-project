# Compilation and runtime architecture exploration

Design revision: 1 · Recorded: 2026-11-12 America/Los_Angeles · Status: authorized exploration; candidate production designs pending whole-file review.

## Design authority

- **Record and scope:** [D-25](DesignAuthorityRegistry.md#d-25), all sections. [H-387](DesignAuthorityRegistry.md#h-387) directly authorizes multiple architecture designs and isolated prototype validation; [K-573](DesignAuthorityRegistry.md#k-573) records scope and affected qualifications. It does not approve any candidate mechanism or final architecture.
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

## Experiment design under development

The first experiment isolates the semantic/native boundary: a complete artifact retains base and extended interpretations and a dependency-local installation, while execution holds explicit bindings, complete callable snapshots, owner identities and pending completion. It compares semantic interpretation, prepared shared native kernels and selective native kernels without using the existing CoD frontend's expansion. Shared harness behavior must be distinguished from candidate-specific code.

Checks include current versus captured calls, independent copies, region remapping without payload-pointer patching, failed operand preparation, suppressed initialization writes, cleanup order and recovery, same-layout distinct semantic types, and runtime-supplied unseen tuples. An entry-only artifact and an observed-tuples-only dispatcher serve as deliberately incomplete controls. These are experimental designs, not language policy or preapproved selection criteria.

A separate LLVM execution probe will distinguish retained IR/checking work from eager or lazy native materialization. It must not claim that ORC's ability to defer code generation eliminates frontend semantic expansion. Timings, object/code counts and semantic traces will be recorded with tool/source identity and repeated runs; limitations will be carried into the recommendation.

## Evidence, criteria and recommendation

Prototype results and the resulting selection criteria are pending. No winner is selected. The next deliverable is measured evidence with an explicit account of what was implemented, modelled and left unverified; the final recommendation will remain subject to whole-file review.
