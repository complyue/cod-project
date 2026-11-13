# AOT micro-dispatch work log

Task authority: external work package revision 5, received from user-authorized [side chat](codex://threads/01a1d51f-3ef4-7101-a86c-94666675f5b6) on 2026-11-13 America/Los_Angeles. [D-21](../../design/AheadOfTimeExecutionPreparation.md), [D-25](../../design/CompilationRuntimeArchitectureExploration.md) and registry H-396/H-397/K-585 record design replacement, focused native implementation/evidence and conditional merge. Permanent production mechanisms remain outside this delivery's qualification.

## Branches and checkpoint

- Prior main: `8ff957610a455d0f242c45108777dd68a77561c6`.
- Previous exploration: `codex/local-dispatch-exploration`, `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8`. Preserved separately, not merged or selected as final architecture.
- Candidate: `codex/aot-micro-dispatch`, created directly from prior main. Only the selective reuse attributed in the README comes from the previous package.
- Carry forward 3,133 seconds from the preceding goal plus a conservative 300 seconds of branch/handoff work. The existing 48-active-hour checkpoint is shared; it is neither a fresh allowance nor a minimum. Current goal started at Unix timestamp 1794579693. At the final documentation audit it had used 2,734 seconds: 6,167 seconds combined, approximately 1 hour 43 minutes. The completion receipt below records the final estimate. Idle waiting between tasks is excluded; no token budget was requested.

## Instruction history

Revision 2 authorized the independent replacement experiment and preserved the earlier branch without selecting its recommendation. Revision 3 required an observable reason for concrete distinctions. Revision 4 selected concrete-type discriminators and call-site facts, superseding fingerprints as runtime RTTI; type checksums became separate root-associated DBMR compatibility planning. Revision 5 required comparable persisted type meaning across compatible fused worlds, including separately compiled writer/reader evidence. All refinements were acknowledged through the existing chat channel before merge.

The resulting fixture uses explicit stable 64-bit definition IDs and one small root catalog. It introduces no second per-value tag, build-local persisted ordinal, central registry service, graph rewriting or production DBMR format/loader change. The catalog and literal allocation scheme are experimental mechanisms, not permanent protocols.

## Validation completed

The final focused run completed with exit status 0:

```sh
python3 experiments/micro-dispatch/run.py --work /tmp/cod-micro-dispatch-v5 --output experiments/micro-dispatch/observations.json
```

The runner used the existing build toolchain, C++20, `-O0 -g -UNDEBUG -fno-inline` and existing shilos/LLVM libraries. No specialization pass or runtime compiler ran. [Observations](observations.json) records source hashes, commands, results and full traces; `generated.inc` and `type_ids.inc` are matching review snapshots. Temporary native binaries and the complete emitted artifacts remain in the reported work directory, outside the repository.

- All 13 selected native cases passed, including explicit zero return, independent fields, crossed method specificity, actual-type opcode selection, current/captured interpretations, shared actual owners, abstract result storage/forwarding, alias updates, failed-update preservation, acquisition/verifier failure and catch/finally completion.
- Native symbol inspection found one definition of each of 13 generated method bodies. Reversed declaration order and optional proven call-site selector elimination preserved the full results/traces without adding bodies.
- Missing intersections, unproved coverage, ambiguous IDs, unresolved type/generic references, missing entry installation and invalid/abstract/mismatched/out-of-range values rejected for the expected reasons.
- Separately compiled older writer and newer reader preserved the result/trace after unrelated type addition and declaration reordering. Full saved-file SHA-256 was unchanged before/after accepted and rejected readers. A conflicting Example/Other ID interpretation rejected with `REJECT 938` before payload access.
- Accepted readers mapped the readonly region twice at different bases; regional bytes remained unchanged. Mutable cases used a separate scratch region and actual owner-checked relative links.
- One emitted LLVM module was compiled separately and executed natively with result 313. Actual LLVM resources were acquired and released in the expected order.
- Separate root-checksum planning preserves recursive/shared named descriptions and demonstrates that equal checksums do not repair an invalid discriminator interpretation. It is not a runtime selector or production compatibility predicate.

The required repository build also completed with exit status 0:

```sh
cmake --build ./build
```

Before execution, `ninja -C build -n -d explain` showed 10 existing incremental steps and no full LLVM rebuild. The user's existing incremental runtime/configuration authorization covered this path. The build performed its existing runtime configure/build and staging work; builtins needed no work, with a small nsan dynamic-list object and existing libcxx-fuzzer install substep. Existing CMake policy/deprecation warnings did not fail the build. No main configure command, bootstrap/toolchain edit, build-tree cleanup or symlink change was made. No unrelated compiler suite ran.

## Merge-gate assessment

| Required condition | Evidence and result |
| --- | --- |
| Explained AOT route with general per-method bodies | D-21 and the README cover finite declarations, selection, local operations, complete values, owners, effects and completion. Native execution works with specialization disabled and without an assumed compiler/fallback. Passed for this authored-contract subset. |
| Discriminating native and persistence observations | The final run covers the central paths and intended negative cases, including separately prepared worlds over unchanged bytes. Generated operations and trusted primitives are identified. Passed. |
| No unresolved obstacle defeats the claimed scope | Finite coverage is bounded and rejects exhausted proofs; required runtime paths are implemented. Runtime checks, carrier/catalog costs and coverage fragmentation remain explicit. General generic closure and production interfaces are excluded rather than claimed solved. Passed for the bounded scope. |
| Coherent design and authority | D-21r7 replaces the execution architecture; D-25r11 and directly affected references agree. H-396/H-397 preserve the actual instructions, and K-585 records scoped continuity under the charter. Production qualification is not inferred from prototype success or merge. Passed. |
| Candidate build, evidence and safe ancestry | Focused checks and the exact repository build passed. Source/snapshot hashes, history preservation, branch ancestry, local links and whitespace audits passed. Main matched the recorded baseline at merge; the merge tree exactly matched the validated candidate. Passed. |

No observed major obstacle defeats this explicitly delivered scope. The authorized merge includes the replacement design, bounded prototype and evidence; it does not claim a full CoD compiler or permanent identity/ABI/compatibility protocol. There is no throughput or universal code-size claim. Full generic closure, inferred owner/effect checking, arbitrary exceptions/captures, module composition/final-image elimination and production DBMR admission remain follow-up work. ORC remains a future tracing-JIT option only.

## Completion receipt

- Candidate commit: `b49fa25df30670deef1442006164b5fd8e3be710` on `codex/aot-micro-dispatch`.
- Authorized merge into main: `3f19fdd36787c44bfff8e4c843a1795a8dbfa46b`. Its parents are prior main `8ff957610a455d0f242c45108777dd68a77561c6` and the candidate above. The merge tree exactly matches the validated candidate tree.
- Post-merge integrity audit passed for all seven implementation source hashes and both generated review snapshots. Earlier H/K evidence matches the prior-main and archive payloads; registry anchors are unique and replacement-package links resolve. The earlier branch remains at `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8`.
- At the post-merge audit the current goal had used 3,072 seconds. With 3,433 carried seconds, the shared active-work estimate is 6,505 seconds, approximately 1 hour 48 minutes; only final receipt/notification work follows. The 48-hour checkpoint was not approached or reset.
- This receipt is a documentation-only follow-up to the validated merge. No source/build changes or further test campaign follow from recording it. No remote push was requested or performed.
