# Source compiler micro-dispatch migration work log

Operational tracking for the user-authorized migration recorded in [H-398](DesignAuthorityRegistry.md#h-398). This log is not a design contract. The outcome-only goal is maintained separately; changing plans and evidence belong here.

## Baseline and authorization

- Date: 2026-11-13 America/Los_Angeles. Source handoff: [side chat](codex://threads/01a1d579-bd41-7bc0-a3de-219c64b37b45).
- Baseline: `2ab762d617b742f4fba7c2c617bacd303739ccef`, verified clean before mutation.
- Local annotated tag: `multi-dispatch`, targeting that exact commit. The annotation distinguishes the included micro-dispatch design/isolated experiment from the existing source compiler. No tag was moved or pushed.
- Working branch: `codex/source-micro-dispatch`, created from that baseline. The earlier isolated baseline and evidence remain available without serving as a hidden compiler fallback.
- User authorizes autonomous source migration and validation. Substantive generic acceptance, identity/ABI and other new language choices still require topic/whole-file review. No new time/token allowance, bootstrap/toolchain authorization or subagent authorization was supplied. Existing exact-build and focused-test rules continue.

## Removal-first stage decision

[H-399](DesignAuthorityRegistry.md#h-399) records the user’s explicit instruction: “造成技术债务负担的历史实现，应该先砍掉，从清爽的基础上补缺失的部分，当前整改阶段不要求功能完整、测试全过”. This supersedes incremental retention of old paths merely to keep feature coverage. Remove the entangled checking/emission/context-specialization implementation first; build supported paths on the clean foundation. Feature parity and a fully green legacy suite are not stage completion conditions. Retained/new paths still require correct behavior and regional/owner/effect/cleanup invariants. No weakened assertions, hidden fallback or new language prohibition is authorized.

## Current work

The closure contract is approved at D-26r8/H-407/H-409; D-21r10, D-15r22 and D-25r14 carry the reconciled outside-catalog boundary. The user requested a new autonomous foundation goal on 2026-11-13. H-399/H-408 governs practical stage acceptance. The earlier universal-unproved proposal is superseded; actual semantic errors, resource stops and identified implementation gaps are distinct.

The completed validation checkpoint is a usable scalar-method foundation: real CoD source with semantic methods and their general/demanded specialized implementations, proven direct calls, local operation selection, direct and mutual recursion, explicit returns, conditional control, printing and ordered prepared cleanup. Correct retained paths and useful rejection evidence are required. Product/abstract regional carriers, persistence admission, arbitrary captures/worlds and full generic analysis remain explicit capability gaps, not language restrictions. This checkpoint does not select their permanent ABI.

Implemented organization:

1. Retain an LLVM-independent checked method/operation record, with source locations and checked call targets. Checking may retain distinct finite scalar substitutions. One semantic body can supply multiple prepared implementations; user overload selection stays separate from choosing an implementation of the selected method.
2. Prepare general implementations and call-site-demanded specializations; use proven direct calls, retain independent local selections where facts remain unknown, and provide no legacy fallback. The execution-local scalar carrier is private to generated code and stores no persistent identity or regional graph.
3. Preserve explicit completion and cleanup on supported paths. Reuse native primitives; unsupported constructs receive specific implementation-gap diagnostics.
4. Validate representative source/native results, semantic-method versus implementation counts, reuse across recursion, generic type distinctions, proven direct calls, general-path controls and meaningful negative cases, then record supported/missing behavior and exact build results.

The 12-observation pre-removal baseline passed and the unchanged selection after removal failed; both are historical comparison evidence, not stage parity requirements. Reports remain at `/tmp/cod-source-micro-dispatch-baseline.json` and `/tmp/cod-source-micro-dispatch-removal.json`.

## Removal checkpoint

The previous 10,897-line frontend and 1,553-line LLVM emitter are replaced by a small entry pipeline and native artifact emitter. `NativeCallDemand`, caller-context equivalence/reuse, interleaved checking/emission, native value/place semantic state and the old flow/refinement/emission loop are absent. The syntax/parser and native library primitives remain reusable; no old compiler path is callable.

At this intermediate checkpoint, empty source parses and emits a native main; nonempty forms produce an explicit implementation-gap diagnostic. This is deliberate temporary loss, not the final foundation's claimed capability or a language restriction. Existing source fixtures remain unchanged. Checked method/value/operation paths are the next work after the closure/interface boundary is established.

The first exact build failed on a constructor mismatch in the new entry pipeline (`Parser` takes source and path); the call was corrected. The retry passed using the same required command and already-authorized incremental runtime steps. The unchanged 12-observation legacy selection then reported 0/12 passing, as expected for the documented missing paths; its report is `/tmp/cod-source-micro-dispatch-removal.json`. An empty-program native smoke check exposed a required `cod_lang_exception_site` link symbol in the existing runtime library. The emitter now supplies its established no-known-sites result; a further exact build and smoke rerun validate that fix. No legacy assertions were changed.


Final removal-checkpoint validation: `cmake --build ./build` passed after the runtime link-interface fix. Empty-source LLVM and bitcode emission, actual `--run` native execution and preserved malformed-source diagnostics passed; `/tmp/cod-source-foundation-smoke.json` records the observations. The expected legacy failures remain explicit. A source audit finds no `NativeCallDemand`, `same_native_context`, speculative native reuse or CodeExtractor path in the replacement frontend/emitter. Historical registry payloads, unique anchors, new document links and diff whitespace audits passed.


Removal checkpoint committed as `8048666` on `codex/source-micro-dispatch`; main and the `multi-dispatch` tag are unchanged. The validation index now describes current capability and directs historical source-form details to the immutable baseline, so old passing reports do not imply current support. The closure contract remains pending whole-file review; its revision 2 incorporates the confirmed removal-first stage boundary without reopening that priority.

## Goal restart

The previous broad goal was paused and cleared by the user. The replacement foundation goal was explicitly requested after D-26 approval; no new time/token allowance or toolchain permission was supplied. Source work proceeds within qualified behavior, and no unchanged design decision is being reopened. The five side-review document updates are retained intact.

## Hierarchy and initiator clarification

H-409 records the authorized 2026-11-13 handoff from side chat `01a1d612-0889-7fe2-87f3-f4f1ff0772f5`: document the clarified model first, then continue implementation. A family contains user methods; each semantic method can have several statically registered native specializations. Proven call-site facts demand/reuse appropriate AOT implementations. Micro-dispatch points are local operation selections, not whole-body versions. User definition/compile-time computation, compiler analysis/preparation and runtime calls/effect bindings have distinct initiators and phases. The earlier no-specialization-pass/one-emitted-body plan is superseded.

The general-implementation-as-zero-extra-assumptions representation is an engineering proposal, not a user-selected mechanism. No ABI, specialization key/budget algorithm, method loading or runtime compiler follows. Runtime type/exception, owner/capture and cleanup contracts are preserved. The app goal record was paused at handoff receipt; the new handoff explicitly directs documentation followed by implementation, while goal-state resumption remains controlled by the user/app.

## Scalar foundation checkpoint

The source pipeline now checks scalar method obligations independently of LLVM. Semantic family selection precedes implementation selection. Primitive identities and repeated type-parameter relations remain distinct; exact signature containment handles concrete-versus-generic and repeated-binder specificity. Declared interfaces publish before recursive checking; every queued obligation must finish before emission. A concrete 4,096-obligation resource stop identifies its active callable and does not classify the source as invalid.

Known source calls demand/reuse AOT implementations. Concrete methods share one native body; generic methods have a shared general body plus demanded specializations in default mode. `--no-specialize` emits only general/concrete bodies to validate correctness independently of specialization. Local arithmetic, return conversion, printing and genuinely varying semantic call targets select locally and rejoin control. A proven common method target calls its general implementation directly. Runtime values and activation depth do not enter checking-instance keys. No runtime compiler or fallback exists.

The admitted slice has no external input carrier: all reachable scalar tuples arise from checked source paths. Private generated scalar transport carries primitive identity and payload, while general entry checks protect the accepted tuples. `UnsoundRuntimeTyping` is the approved unchecked typing-breach category; this does not implement outside-catalog admission, checked `StaticallyUnknownType`, persistent type IDs, a general `<any>` carrier or a permanent ABI. The finite checking records and emitter are replaceable internal implementation, not qualification of the excluded production representation.

Plain scalar defer registration prepares arguments in source order and publishes only after successful preparation. Per-scope records drain in reverse registration order through a shared completion path, including conditional registration, nested scopes, recursive calls, callee failure and preparation failure. Subsequent action failures replace the pending failure while remaining actions continue. Missing returns retain the warning/error controls and unchecked runtime exception. Numeric range/overflow/zero-divisor diagnostics preserve the primitive boundaries and complete registered scalar cleanup before the program-boundary diagnostic.

Supported inventory: i64/u8/u32/u64/bool/f32/f64; literal range checks, arithmetic/comparisons and established lossless primitive widening; explicit const scalar or void method results; unbounded scalar type parameters and same-T; direct/mutual recursion; scalar conditionals and short circuit; std:io print; ordinary grouping and explicit scopes; plain prepared scalar deferred calls. Mutable/local storage, regional values and ownership transport, user operators, units/fractions, first-class callables and captures, general TLS bindings/module composition, loops, handlers/deferred blocks, throws declarations, outside-catalog ingress and omitted results remain diagnosed implementation gaps. No existing legacy fixture or assertion was weakened.

Validation: exact `cmake --build ./build` passed with the already-authorized incremental runtime steps. The first native check exposed misuse of this LLVM checkout's terminator query; `hasTerminator()` fixes the control-flow construction. Subsequent source/native/IR validation passed 41/41, then 54/54 after concrete-body reuse, common-target direct calls and numeric boundary completion. Final command: `COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-foundation-validation.json python3 tests/cod/language/foundation_tests.py`. The durable [validation report](../tests/cod/language/foundation-validation.json) records commands, actual outputs, diagnostics, executable/source hashes and structural observations. The suite checks 12 semantic bodies in general-only mode, independent local numeric paths, direct known-call specializations, specialization reuse, semantic overload choice inside a generic wrapper, recursion at increased runtime depth, definition-order independence, and analysis without executing effects. This establishes the stated foundation checkpoint, not legacy parity or full D-26 coverage. Build log: `/tmp/cod-foundation-build.log`.
