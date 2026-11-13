# CoD source compiler validation

Reference revision: 220 · Recorded: 2026-11-13 America/Los_Angeles.

## Design authority

- **D-21 link continuity:** [K-596](../../../design/DesignAuthorityRegistry.md#k-596) updates the execution-preparation document locator; this document’s semantic revision, qualified scope and exclusions are unchanged.

This validation index follows [D-21](../../../design/AheadOfTimeExecutionPreparation.md) and the source migration under [H-398](../../../design/DesignAuthorityRegistry.md#h-398)/[H-399](../../../design/DesignAuthorityRegistry.md#h-399). [K-595](../../../design/DesignAuthorityRegistry.md#k-595) records revision220 availability under the approved D-26r8/H-407/H-409 coverage contract. The confirmed baseline, value path and exclusions remain in D-21/D-26; this revision changes the implementation inventory only. Existing fixture expectations retain their governing language, owner, context and completion contracts; this document grants no new semantics or implementation qualification. Charter §§2–3/§6.3/§12 requires observable native evidence and an honest distinction between intended behavior and current support.

## Current implementation boundary

The source compiler implements a bounded scalar foundation. LLVM-independent checking selects semantic methods and completes demanded scalar obligations before native emission. Proven calls select prepared specializations; general bodies retain independent local operation choices. Concrete methods reuse one native implementation. There is no legacy fallback.

Supported source includes explicit const scalar results, void callables, unbounded scalar type parameters, exact-argument overload selection with same-T relations, direct/mutual recursion, conditionals, short-circuit booleans, primitive numeric operations and widening, and selected `std:io` printing. Ordinary grouping shares the enclosing cleanup scope; explicit `{{ ... }}` ends its own scope. Plain deferred calls prepare scalar arguments before registration and run in reverse registration order on normal return or failure. Value-callable fallthrough warns and raises unchecked `MissingReturnException`.

This is a closed source-program slice: there is no external abstract-value ingress. Execution-local scalar tags preserve the supported primitive identities but define no persistent ABI or CoD `<any>` representation. Generic checking retains finite demanded substitutions; runtime recursion adds no bodies. The implementation does not prove the full generic/recursive closure contract. Regional values, ordinary local storage, units/fractions, first-class callables/captures, dynamic binding, general module composition, loops, deferred blocks, catch/finally, declared throws and omitted-result inference remain explicit implementation gaps. Voluntary throws contracts are rejected as unsupported, never silently waived.

The [migration work log](../../../design/MicroDispatchMigrationWorkLog.md) records the supported/missing inventory, commands, failures and next work. Feature parity and a fully passing legacy suite are not acceptance conditions for this restructuring stage. Supported paths must still behave correctly; missing paths must not be reported as new language prohibitions.

The legacy fixtures and their assertions remain intact. They provide recovery/comparison evidence and target behavior as source capabilities are implemented. A passing historical report does not establish current compiler support. The initial 12-observation baseline passed; the same selection after removal reported 0/12 passing. The scalar foundation has its own source/native and IR evidence; historical legacy failures are not reclassified as passing. These are different observations, not a reclassification of failed legacy checks as success.

## Historical baseline

The local annotated `multi-dispatch` tag points to `2ab762d617b742f4fba7c2c617bacd303739ccef`. It includes the earlier source compiler, fixtures, validation reports and reference revision 218 of this document, together with the newer micro-dispatch design and isolated experiment. It is a historical recovery point, not another compiler path linked into current programs.

The full prior source-form/implementation reference is recoverable from the immutable commit:

```sh
git show 2ab762d617b742f4fba7c2c617bacd303739ccef:tests/cod/language/README.md
```

Human decisions remain in the [authority registry](../../../design/DesignAuthorityRegistry.md). The [isolated micro-dispatch experiment](../../../experiments/micro-dispatch/README.md) supplies bounded authored-contract evidence; it is not source-compiler acceptance.

## Focused validation

Under the task's test authorization, run the scalar foundation checks:

```sh
COD_TEST_TOOLCHAIN=build COD_LANGUAGE_REPORT=/tmp/cod-foundation-validation.json python3 tests/cod/language/foundation_tests.py
```

The suite exercises default AOT specialization and `--no-specialize` general-only execution, including recursion, overload specificity, independent local operations, preparation/cleanup failures, numeric boundaries and negative checking cases. It verifies emitted method/implementation reuse and that analysis executes no runtime effects. The current recorded result is 54/54; detailed commands, evidence and exclusions are in the migration log.

For separate legacy comparison, select fixtures by file stem:

```sh
COD_TEST_TOOLCHAIN=build python3 tests/cod/language/run_tests.py recursive-calls callable-generic
```

`tests/cod/test_language.sh` delegates to the same runner. `COD_LANGUAGE_REPORT` optionally writes commands, outputs, diagnostics, source hashes and binary identity to JSON. The runner also performs its frontend-only `import-mapping.cod` emission check; include that observation when reporting totals. An omitted selection runs the full fixture suite and should only be used within the authorized test scope.

Native runtime scripts cover resource accounting, cleanup, image copies, failures and emitted LLVM execution. Their presence does not imply current support or permission to run all of them. Keep `COD_TEST_TOOLCHAIN=build` for this checkout; use the required `cmake --build ./build` after source changes. Configuration/bootstrap permissions remain separate.

As supported source paths return, validate their meaningful success and failure cases, emitted general method bodies and actual native behavior. Do not weaken assertions merely to regain green results. Record identified implementation gaps separately from semantic errors and detected compiler resource exhaustion.
