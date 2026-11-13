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

The first design activity is [the generic/recursive-closure contract](GenericAndRecursiveClosure.md), pending whole-file approval. It distinguishes fixed-instance recursion, finite instantiation/recursive descriptions and expanding semantic obligations. The proposal reports unsupported closure as unproved instead of treating a work cap as a proof or silently selecting a new language restriction.

Read-only source audit confirms that `Value`, `NativeCallDemand`, `compile()`, `record_field()` and `LLVMEmission::type_of()` couple semantic state to concrete LLVM transport and context-specific checking. Existing generic, recursive-capture and growing-callback fixtures provide important retained behavior. Their current implementation does not settle pending result-inference or ABI choices.

Next work:

1. Finish closure topic review and inventory retained source behaviors with their actual design authority.
2. Derive a checked method/value/operation boundary from the agreed closure contract and a concrete source-to-native acceptance workload. Keep unresolved carrier/result/identity decisions reviewable.
3. Remove the old entangled implementation first, retain reusable syntax/native primitives, and add coherent checked source-to-native paths on the new foundation. Track missing capabilities explicitly; no parallel legacy semantic path remains solely for compatibility.
4. Validate abstract arguments/results, independent polymorphic fields, selection, recursion, actual owners/context and cleanup with specialization disabled, then run the required exact repository build and applicable focused acceptance checks.

A focused baseline run finished before compiler changes: 12/12 observations passed using `COD_TEST_TOOLCHAIN=build`, including the runner’s frontend-only check. The report is `/tmp/cod-source-micro-dispatch-baseline.json`. Selected fixtures cover abstract forwarding/ambiguity/narrowing, direct and mutual recursion, generic calls/bounds, fresh captures, growing callbacks/expired owners and deferred recursion. This is historical comparison evidence, not a promise of stage feature parity. No source-migration completion or whole-language support is claimed.

## Removal checkpoint

The previous 10,897-line frontend and 1,553-line LLVM emitter are replaced by a small entry pipeline and native artifact emitter. `NativeCallDemand`, caller-context equivalence/reuse, interleaved checking/emission, native value/place semantic state and the old flow/refinement/emission loop are absent. The syntax/parser and native library primitives remain reusable; no old compiler path is callable.

At this intermediate checkpoint, empty source parses and emits a native main; nonempty forms produce an explicit implementation-gap diagnostic. This is deliberate temporary loss, not the final foundation's claimed capability or a language restriction. Existing source fixtures remain unchanged. Checked method/value/operation paths are the next work after the closure/interface boundary is established.

The first exact build failed on a constructor mismatch in the new entry pipeline (`Parser` takes source and path); the call was corrected. The retry passed using the same required command and already-authorized incremental runtime steps. The unchanged 12-observation legacy selection then reported 0/12 passing, as expected for the documented missing paths; its report is `/tmp/cod-source-micro-dispatch-removal.json`. An empty-program native smoke check exposed a required `cod_lang_exception_site` link symbol in the existing runtime library. The emitter now supplies its established no-known-sites result; a further exact build and smoke rerun validate that fix. No legacy assertions were changed.


Final removal-checkpoint validation: `cmake --build ./build` passed after the runtime link-interface fix. Empty-source LLVM and bitcode emission, actual `--run` native execution and preserved malformed-source diagnostics passed; `/tmp/cod-source-foundation-smoke.json` records the observations. The expected legacy failures remain explicit. A source audit finds no `NativeCallDemand`, `same_native_context`, speculative native reuse or CodeExtractor path in the replacement frontend/emitter. Historical registry payloads, unique anchors, new document links and diff whitespace audits passed.
