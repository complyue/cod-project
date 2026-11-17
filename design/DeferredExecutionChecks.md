# Static checks for deferred execution

Design revision: 1 · Recorded: 2026-11-17 · Status: pending human whole-file/value-path review.

## Design authority

- **Registry and coverage:** [D-17-DEFER-CHECKS](DesignAuthorityRegistry.md#d-17-defer-checks), revision 1, this whole file. Candidate P-561–P-563; [K-798](../CHANGELOG.md#k-798) records the proposal. This document is not a delegated implementation contract before human confirmation.
- **Candidate value path:** C-63 → C-10 through H-267-confirmed P-235 preserves explicit typed failure and cleanup responsibility. P-236 retains the bounded D-17 preservation baseline. Candidate P-561 connects this bounded source check through that existing value path to charter §§2/3.3/4.3/6.3/8/12: visible registration has a checkable execution context, valid helpers retain their own cleanup, and source errors are diagnosed before execution without adding runtime state.
- **Human-confirmed baseline:** [H-552](../CHANGELOG.md#h-552) selects rejection of nested defer declarations; [H-356](../CHANGELOG.md#h-356) fixes eager call preparation versus delayed blocks; [H-371](../CHANGELOG.md#h-371) requires explicit scopes for deferred-block declarations/effects; [H-295](../CHANGELOG.md#h-295) makes typeof nonexecuting. These are source confirmations, not confirmation of this new bounded mechanism/value path.
- **Dependencies and invariants:** P-562 requires D-10’s lookup/call permissions, D-17’s source contexts, D-14’s query/access/owner rules, D-18’s independent callable bodies, D-26’s diagnostics and D-16 compatibility. C-65 storage/lifetime rules are dependencies, not a new value grant. Preserve ordinary lexical lookup, func/proc permissions, actual owners, explicit returns, successful registration and intrinsic cleanup. Dependencies grant no new source permission.
- **Acceptance and exclusions:** The boundary table, source-check contract and acceptance cases below govern qualification. P-563 excludes queue/FIFO/aggregate implementation, completed-pass checked coverage, return inspection, module scheduling, new capture/region storage and broader query admission. Existing qualifications and the separate D-18r113 review remain unchanged. No prior human confirmation or continuity assessment qualifies this first proposal.

## Source rule and execution boundaries

A callable may register deferred work while executing its ordinary body. A defer declaration in that callable’s deferred execution body, catch or attached finally is a source error. Ordinary groups, conditionals, loops and explicit full scopes preserve the enclosing deferred execution context.

The check distinguishes the source declaration’s execution context from the storage/effect permissions of an explicit scope:

| Source position | Context for checking defer declarations |
| --- | --- |
| Module execution or an ordinary callable body | Ordinary context, until a deferred body/handler is entered. |
| Callable expression and arguments prepared for a plain deferred call | Their actual enclosing context. Preparing a top-level deferred call is ordinary execution; preparation inside deferred work retains that enclosing deferred context. |
| Explicit delayed block, its catches and attached finally | Deferred execution context anchored at that enclosing defer declaration. |
| Bodyless deferred catches and optional finally | Deferred execution context, independent of whether an error eventually matches. |
| Ordinary group, conditional, loop or explicit full scope inside deferred execution | The same deferred execution context. An explicit scope may admit variables/effects under H-371; it does not admit another defer declaration. |
| A separately checked named, local or anonymous callable body | A fresh ordinary callable context. Construction/capture evaluation and the declaration itself retain their enclosing checks. |
| A typeof operand | Structural source constraints retain its enclosing callable/deferred context. Inspection performs no runtime work; H-295 supplies no exemption for otherwise invalid nested source. Separately defined callable bodies still have their independent context. |

Only the final invocation of a plain deferred call is delayed. For example, `defer consume({ defer earlier(); 7u64 });` prepares the grouped argument during ordinary registration. Both declarations are ordinary registrations, subject to existing scope and lifetime rules. A failed outer preparation registers no outer action and retains already successful registrations and their ordinary cleanup obligations.

In `defer { consume({ defer earlier(); 7u64 }); }`, the grouped argument executes as part of deferred work. Its defer declaration is rejected. Explicit `{{ ... }}` around that argument does not change this result.

A separately checked helper may have its own deferred work. Its defers belong to its invocation/full scopes; calling it during cleanup does not append them to the caller’s draining sequence. Inlining preserves those source scope and cleanup boundaries.

```cod
proc helper() {
    defer release_resource();
    work();
}

defer helper();
```

A local named helper declared during deferred execution still requires a permitted declaration scope under H-371. Its body starts an independent callable context. An anonymous callable body follows the same rule; this does not grant new capture or lifetime permissions.

Type inspection retains ordinary source, destination, storage, access and owner validity. Querying a cleanup-capable helper’s declared result must not invoke it, register its actions, create capture images or initialize its inputs. Unsupported statement/query forms remain separately diagnosed implementation gaps. This contract grants no broader typeof language.

## Bounded source-check contract

Validate every incorporated source body, including unused methods, generic bodies and anonymous definitions, in checking-only and emission paths before accepting an executable artifact. Source well-formedness does not depend on a particular call, specialization or runtime branch being reached. Bodyless declarations have no invented execution body.

A read-only source-structure pass may reuse the compiler’s scope validation. Its information consists of the enclosing callable boundary and, when applicable, the enclosing deferred execution site. H-371 declaration/effect permissions and loop declaration restrictions remain separate state: resetting those permissions at an explicit full scope must preserve the deferred execution site.

At each defer declaration:

1. If the current callable is already in deferred execution, report the inner declaration and its enclosing deferred execution site.
2. For a plain deferred call, check its eagerly prepared callable expression and operands in the current context. The separately checked callee body retains its own callable context.
3. For a delayed block or bodyless entry, check the delayed body, catches and attached finally in its deferred context.

Check ordinary child expressions in their enclosing context. Check a callable definition’s declaration and construction under that context, and its body independently. Type inspection does not execute queried expressions or erase their structural source constraints. No interprocedural cleanup prohibition, method demand, new flow proof or runtime registration guard is required for this check.

The diagnostic is a compile-time source error with the inner location and an enclosing-defer location or note. Exact wording and ordering against independent source errors are implementation details. No prefix runtime effects or partial LLVM/bitcode output may be published after rejection. Existing resource-limit and unsupported-query diagnostics retain their separate meanings.

The check preserves intrinsic owner release and effect restoration. It establishes the bounded source restriction; it does not establish the full deferred-error queue, global shutdown protocol or aggregate ownership mechanism.

## Design trade-offs

| Approach | Consequence |
| --- | --- |
| Proposed source-context check with eager-preparation and callable boundaries | Preserves H-356 timing and independent helper cleanup while rejecting source declarations in deferred execution. No runtime state is added. |
| Treat every syntactic descendant of a defer token as delayed | Rejects ordinary eager-preparation registrations and loses H-356’s execution distinction. |
| Reset the prohibition at an explicit full scope | Admits nested declarations during deferred execution, contrary to H-552. |
| Propagate the prohibition into called helper bodies | Rejects independent helper cleanup and makes source validity depend on inlining or call-graph expansion. |

## Acceptance

Negative cases must identify the nested declaration and its enclosing deferred context:

- Plain-call and block-form declarations in a deferred body, catch or attached finally, including a bodyless catch/finally.
- Nesting through ordinary groups, conditionals, loops and explicit full scopes, including an explicit-scope delayed action.
- Nesting in an unused or generic callable body and an unreachable branch. No invocation or specialization is required to discover the structural error.
- A queried nested declaration retains structural checks; nonexecution alone must not hide it. This case does not authorize unsupported statement inspection.
- Nested deferred work in a helper’s own deferred body is rejected within that helper, even when the helper itself is permitted.

Positive cases preserve their source-order effects and ordinary ownership/coverage obligations:

- Several ordinary registrations, ordinary loops and independent full scopes before cleanup.
- A top-level deferred call whose eager callable/argument preparation registers another action. Preserve successful registration order and the failure case where only the earlier action was registered.
- A deferred call to a separately checked helper with its own cleanup; local/anonymous helpers have the same independent body rule where their declarations/construction are otherwise permitted.
- Explicit scopes inside deferred work with permitted variables/effects and intrinsic cleanup, without another user defer declaration.
- Type inspection of a cleanup-capable helper under deferred execution: no helper invocation, action registration, capture allocation or input initialization.

Validate source rejection before execution and preserve legal native behavior in specialized and general modes. Active fixtures must enforce the selected restriction and retain meaningful legal helper/preparation counterparts. Their original observations remain available in immutable Git history; passing the older nested-registration prototype is not acceptance of this contract.

## Qualification boundary

Whole-file review must cover the candidate value path, this structural source profile, its eager-preparation/callable/query boundaries, diagnostic obligations and acceptance. It does not qualify the independent queued-error/FIFO, checked-pass, return-inspection, aggregate, module-startup, generator or ordinary capture mechanisms. Compiler implementation of this bounded contract remains pending until that review is recorded in the registry.
