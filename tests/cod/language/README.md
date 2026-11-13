# CoD source compiler validation

Reference revision: 223 · Recorded: 2026-11-13 America/Los_Angeles.

## Design authority

- **Structured completion continuity:** [K-599](../../../design/DesignAuthorityRegistry.md#k-599) covers revision223 under D-17/H-267/H-356–H-362/H-365/H-371. Finally and delayed scalar blocks preserve or replace pending completion through common exit routing while full scopes retain cleanup ownership. D-14 initialization/access and D-10 function/effect restrictions remain constraints. Acceptance includes nested return/failure paths, late reads, registration during cleanup, exact release and allocation failure. Typed catches, exception values and wider deferred-flow proofs remain outside this coverage.

- **Loop and cleanup continuity:** [K-598](../../../design/DesignAuthorityRegistry.md#k-598) covers revision222 under D-17/H-304/H-356/H-365 and D-14's initialization contract. The scalar subset preserves once-only bounds, zero-iteration else, explicit scopes, distinct prepared registrations and global LIFO. Acceptance includes native ordering, normal/failure exits, exact release and accumulation costs; iteration effects, regional borrowing and deferred handlers remain outside this implementation coverage. D-21/D-26 preparation/coverage and D-16 numeric boundaries are unchanged.

- **Storage and documentation continuity:** [K-597](../../../design/DesignAuthorityRegistry.md#k-597) covers revision221 under the existing D-14 scalar-storage/qualifier and D-10/D-17 scope contracts. Process tracking is external; this reference owns current capabilities and reproducible checks. No new language or persistent representation contract is introduced.

- **D-21 link continuity:** [K-596](../../../design/DesignAuthorityRegistry.md#k-596) updates the execution-preparation document locator; this document’s semantic revision, qualified scope and exclusions are unchanged.

This validation index follows [D-21](../../../design/AheadOfTimeExecutionPreparation.md) and the source migration under [H-398](../../../design/DesignAuthorityRegistry.md#h-398)/[H-399](../../../design/DesignAuthorityRegistry.md#h-399). [K-595](../../../design/DesignAuthorityRegistry.md#k-595) records the scalar foundation baseline under the approved D-26r8/H-407/H-409 coverage contract. The confirmed baseline, value path and exclusions remain in D-21/D-26; this revision changes the implementation inventory only. Existing fixture expectations retain their governing language, owner, context and completion contracts; this document grants no new semantics or implementation qualification. Charter §§2–3/§6.3/§12 requires observable native evidence and an honest distinction between intended behavior and current support.

## Current implementation boundary

The source compiler implements a bounded scalar foundation. LLVM-independent checking selects semantic methods and completes demanded scalar obligations before native emission. Proven calls select prepared specializations; general bodies retain independent local operation choices. Concrete methods reuse one native implementation. There is no legacy fallback.

Supported source includes explicit const scalar results, void callables, unbounded scalar type parameters, exact-argument overload selection with same-T relations, direct/mutual recursion, conditionals, short-circuit booleans, primitive numeric operations and widening, and selected `std:io` printing. Ordinary grouping shares the enclosing cleanup scope; explicit `{{ ... }}` ends its own scope. Plain deferred calls prepare scalar arguments before registration and run in reverse registration order on normal return or failure. Value-callable fallthrough warns and raises unchecked `MissingReturnException`.

Local scalar `val` declarations provide independent storage. Default and explicit scalar parameters receive callee-local copies; writable parameters and values accept `<-` updates. Declaration-leading const protects val payloads, type-leading const restricts target access, and transfer results are readonly destination views. Grouped declarations preserve source order and initializer-before-binding lookup. Explicit scopes restore outer names; ordinary groups share names and reject duplicates. `-Wshadow` has its own error control.

A `val` may use `make local T` without an initializer and receive a later write. Reads require definite availability and initialization across conservative branch and short-circuit joins. `make local typeof(name)` queries a known local type without reading its payload. `=` on existing val storage remains an error because val has no reference binding to replace. Unsupported reference, capture or owner operations do not fall back to bits copying.

Half-open integer `for` and scalar `fold` evaluate bounds once. Bounds currently have matching primitive integer types; fold bodies retain the initial scalar type. The index and accumulator are readonly loop bindings. A range's optional `else` runs only for zero iterations and cannot access its index. Body and else are both checked; feasible normal paths determine post-loop initialization, with literal integer bounds providing empty/nonempty facts. Direct body/else declarations require a nested explicit scope.

Ordinary loop bodies register plain deferred calls to the nearest full scope. Each successful registration owns independent prepared scalar arguments and joins one dynamic LIFO queue. Explicit scopes inside loops drain per iteration; scopes surrounding loops accumulate until exit. Failed preparation registers no action. Allocation failure preserves earlier actions; invocation failure replaces the pending failure while remaining actions run. Execution-local records use the native allocator and are released exactly once. Storage grows with live registrations and argument arity; no regional storage, persistent representation or owning-reference behavior follows from this scalar queue.

`try { ... } finally { ... }` preserves an expression's value or pending return/failure through unconditional cleanup. A finalizer completes with void or explicitly returns from the enclosing callable; a new return/failure replaces the pending completion. Ordinary grouping still creates no lifetime scope. Finally reads require initialization across the conservatively tracked normal, return and failure inputs.

`defer { ... }` resolves lexical bindings when registered and reads their contents when invoked. Its declarations require explicit nested scopes, and it cannot capture loop-control bindings. Delayed blocks may register more work and return from the enclosing callable while remaining actions continue. Attached `finally` and bodyless `defer finally { ... }` share ordinary finally execution. A pending failure skips an attached deferred body and enters its finalizer; a plain delayed block executes even during failure cleanup. Missing-return enforcement follows cleanup, so an explicit deferred return can supply the required result.

Delayed read checking currently requires initialization proved before registration. Queue-dependent later initialization, guarantees from deferred writes and complete deferred-return analysis remain proof gaps; missing-return warnings can be conservative when cleanup supplies a result. Registration never applies a delayed write immediately.

This is a closed source-program slice: there is no external abstract-value ingress. Execution-local scalar tags preserve the supported primitive identities but define no persistent ABI or CoD `<any>` representation. Generic checking retains finite demanded substitutions; runtime recursion and loop counts add no bodies. The implementation does not prove the full generic/recursive closure contract. Regional values, ref/ptr local aliasing and rebinding, external-owner parameters, module-local captures, runtime-selected destination views, target-access-qualified overloads, units/fractions, first-class callables, dynamic binding and iteration effects, general module composition, mixed-type range bounds, fold accumulator conversions, typed catches/deferred handlers, exception values and explicit causes, declared throws and omitted-result inference remain explicit implementation gaps. Voluntary throws contracts are rejected as unsupported, never silently waived.

Feature parity and a fully passing legacy suite are not acceptance conditions for this restructuring stage. Supported paths must still behave correctly; missing paths must not be reported as new language prohibitions.

The legacy fixtures and their assertions remain intact. They provide recovery/comparison evidence and target behavior as source capabilities are implemented. A passing historical report does not establish current compiler support. Focused source/native checks establish the supported foundation; historical legacy failures are not reclassified as passing.

## Historical baseline

The local annotated `multi-dispatch` tag points to `2ab762d617b742f4fba7c2c617bacd303739ccef`. It includes the earlier source compiler, fixtures, validation reports and reference revision 218 of this document, together with the newer micro-dispatch design and isolated experiment. It is a historical recovery point, not another compiler path linked into current programs.

The full prior source-form/implementation reference is recoverable from the immutable commit:

```sh
git show 2ab762d617b742f4fba7c2c617bacd303739ccef:tests/cod/language/README.md
```

Human decisions remain in the [authority registry](../../../design/DesignAuthorityRegistry.md). The [isolated micro-dispatch experiment](../../../experiments/micro-dispatch/README.md) supplies bounded authored-contract evidence; it is not source-compiler acceptance.

## Focused validation

Run the focused source checks:

```sh
COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-foundation-validation.json python3 tests/cod/language/foundation_tests.py
COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-storage-validation.json python3 tests/cod/language/storage_tests.py
COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-loop-validation.json python3 tests/cod/language/loop_tests.py
COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-completion-validation.json python3 tests/cod/language/completion_tests.py
COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-loop-runtime-validation.json python3 tests/cod/language/loop_runtime_tests.py
```

These suites exercise default AOT specialization and `--no-specialize` general-only execution, including recursion, overload specificity, independent local operations, preparation/cleanup failures, numeric boundaries and negative checking cases. The foundation suite verifies emitted method/implementation reuse and that analysis executes no runtime effects. The storage suite checks copies, mutation, scope lookup, access permissions and initialization joins. Loop checks cover once-only bounds, zero-iteration else, folds, initialization and composed return/cleanup paths. Completion checks exercise finally, delayed scalar blocks, pending/replacement returns and failures, late reads, nested scopes and conservative initialization boundaries. Native instrumentation verifies dynamic call/block queue order, release, allocation failure before and during cleanup, and live/peak record storage; increasing runtime registration counts must preserve the emitted loop body. Set `COD_LANGUAGE_REPORT` to a path outside the repository to retain commands, results and source hashes.

For separate legacy comparison, select fixtures by file stem:

```sh
COD_TEST_TOOLCHAIN=build python3 tests/cod/language/run_tests.py recursive-calls callable-generic
```

`tests/cod/test_language.sh` delegates to the same runner. `COD_LANGUAGE_REPORT` optionally writes commands, outputs, diagnostics, source hashes and binary identity to JSON. The runner also performs its frontend-only `import-mapping.cod` emission check; include that observation when reporting totals. An omitted selection runs the full fixture suite and should only be used within the authorized test scope.

Native runtime scripts cover resource accounting, cleanup, image copies, failures and emitted LLVM execution. Their presence does not imply current support or permission to run all of them. Keep `COD_TEST_TOOLCHAIN=build` for this checkout; use the required `cmake --build ./build` after source changes. Configuration/bootstrap permissions remain separate.

As supported source paths return, validate their meaningful success and failure cases, emitted general method bodies and actual native behavior. Do not weaken assertions merely to regain green results. Record identified implementation gaps separately from semantic errors and detected compiler resource exhaustion.
