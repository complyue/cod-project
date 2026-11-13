# CoD source compiler validation

Reference revision: 219 · Recorded: 2026-11-13 America/Los_Angeles.

## Design authority

This validation index follows [D-21](../../../design/AheadOfTimeMultipleDispatch.md) and the source migration under [H-398](../../../design/DesignAuthorityRegistry.md#h-398)/[H-399](../../../design/DesignAuthorityRegistry.md#h-399). [K-588](../../../design/DesignAuthorityRegistry.md#k-588) records the current availability correction. Existing fixture expectations retain their governing language, owner, context and completion contracts; this document grants no new semantics or implementation qualification. Charter §§2–3/§6.3/§12 requires observable native evidence and an honest distinction between intended behavior and current support.

## Current implementation boundary

The source compiler is undergoing removal-first restructuring. The old coupled semantic checking, context specialization and LLVM emission path is absent. The current checkpoint supports parsing, malformed-source diagnostics, and native LLVM/bitcode emission and execution for empty source. Nonempty source execution receives an explicit implementation-gap diagnostic. There is no legacy fallback.

The [migration work log](../../../design/MicroDispatchMigrationWorkLog.md) records the supported/missing inventory, commands, failures and next work. Feature parity and a fully passing legacy suite are not acceptance conditions for this restructuring stage. Supported paths must still behave correctly; missing paths must not be reported as new language prohibitions.

The fixtures and their assertions remain intact. They provide recovery/comparison evidence and target behavior as source capabilities are implemented. A passing historical report does not establish current compiler support. The initial 12-observation baseline passed; the same selection after removal reported 0/12 passing. Empty-source native smoke and parser-diagnostic checks passed separately. These are different observations, not a reclassification of failed legacy checks as success.

## Historical baseline

The local annotated `multi-dispatch` tag points to `2ab762d617b742f4fba7c2c617bacd303739ccef`. It includes the earlier source compiler, fixtures, validation reports and reference revision 218 of this document, together with the newer micro-dispatch design and isolated experiment. It is a historical recovery point, not another compiler path linked into current programs.

The full prior source-form/implementation reference is recoverable from the immutable commit:

```sh
git show 2ab762d617b742f4fba7c2c617bacd303739ccef:tests/cod/language/README.md
```

Human decisions remain in the [authority registry](../../../design/DesignAuthorityRegistry.md). The [isolated micro-dispatch experiment](../../../experiments/micro-dispatch/README.md) supplies bounded authored-contract evidence; it is not source-compiler acceptance.

## Focused validation

Under the task's test authorization, select fixtures by file stem:

```sh
COD_TEST_TOOLCHAIN=build python3 tests/cod/language/run_tests.py recursive-calls callable-generic
```

`tests/cod/test_language.sh` delegates to the same runner. `COD_LANGUAGE_REPORT` optionally writes commands, outputs, diagnostics, source hashes and binary identity to JSON. The runner also performs its frontend-only `import-mapping.cod` emission check; include that observation when reporting totals. An omitted selection runs the full fixture suite and should only be used within the authorized test scope.

Native runtime scripts cover resource accounting, cleanup, image copies, failures and emitted LLVM execution. Their presence does not imply current support or permission to run all of them. Keep `COD_TEST_TOOLCHAIN=build` for this checkout; use the required `cmake --build ./build` after source changes. Configuration/bootstrap permissions remain separate.

As supported source paths return, validate their meaningful success and failure cases, emitted general method bodies and actual native behavior. Do not weaken assertions merely to regain green results. Record intentional implementation gaps separately from semantic errors and unproved native closure.
