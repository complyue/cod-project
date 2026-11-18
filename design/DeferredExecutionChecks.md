# Static checks for deferred execution

Design revision: 3 · Recorded: 2026-11-17 · Status: pending human whole-file/value-path review.

## Design authority

- **Current capture-review reference:** [K-REPLACEMENT-EFFECTS-20261117](../CHANGELOG.md#k-replacement-effects-20261117) keeps the separate D-18 value-input qualification current. This file changes only review references; its semantic revision, governing value paths, constraints and acceptance are unchanged. The registry owns the current review scope.

- **Current source decision and review:** [H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117)/[K-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#k-full-scope-callable-20261117) confirms full ordinary-callable equivalence everywhere, including ordinary captured values and local returns; explicit full scopes own independent defer sequences. This r3 remains pending whole-file/value-path review. The source decision does not adopt this complete checking mechanism or the separate D-18 value-input contract.
- **Registry and coverage:** [D-17-DEFER-CHECKS](DesignAuthorityRegistry.md#d-17-defer-checks), revision 3, this whole file. Candidate P-561–P-563; [K-798](../CHANGELOG.md#k-798) records the proposal. This document is not a delegated implementation contract before human confirmation.
- **Candidate value path:** C-63 → C-10 through H-267-confirmed P-235 preserves explicit typed failure and cleanup responsibility. P-236 retains the bounded D-17 preservation baseline. Candidate P-561 connects this bounded source check through that existing value path to charter §§2/3.3/4.3/6.3/8/12: visible registration has a checkable execution context, valid helpers retain their own cleanup, and source errors are diagnosed before execution without adding runtime state.
- **Human-confirmed baseline:** [H-552](../CHANGELOG.md#h-552) selects rejection of nested defer declarations; [H-356](../CHANGELOG.md#h-356) fixes eager call preparation versus delayed blocks; [H-371](../CHANGELOG.md#h-371) requires explicit scopes for deferred-block declarations/effects; [H-295](../CHANGELOG.md#h-295) makes typeof nonexecuting. These are source confirmations, not confirmation of this new bounded mechanism/value path.
- **Dependencies and invariants:** P-562 requires D-10’s lookup/call permissions, D-17’s source contexts, D-14’s query/access/owner rules, D-18’s independent callable bodies, D-26’s diagnostics and D-16 compatibility. C-65 storage/lifetime rules are dependencies, not a new value grant. Preserve ordinary lexical lookup, func/proc permissions, actual owners, explicit returns, successful registration and intrinsic cleanup. Dependencies grant no new source permission.
- **Acceptance and exclusions:** The boundary table, source-check contract and acceptance cases below govern qualification. P-563 excludes queue/FIFO/aggregate implementation, completed-pass checked coverage, return inspection, module scheduling, new capture/region storage and broader query admission. Existing qualifications and the separate D-18 value-input review remain unchanged. No prior human confirmation or continuity assessment qualifies this first proposal.

## Source rule and execution boundaries

A callable may register deferred work while executing its ordinary body. A defer declaration targeting the currently draining scope is a source error. Ordinary groups, conditionals and loops preserve that context. An explicit full scope starts its own registration and cleanup context, even when entered from deferred execution.

The check distinguishes the source declaration’s execution context from the storage/effect permissions of an explicit scope:

| Source position | Context for checking defer declarations |
| --- | --- |
| Module execution or an ordinary callable body | Ordinary context, until a deferred body/handler is entered. |
| Callable expression and arguments prepared for a plain deferred call | Their actual enclosing context. Preparing a top-level deferred call is ordinary execution; preparation inside deferred work retains that enclosing deferred context. |
| Explicit delayed block, its catches and attached finally | Deferred execution context anchored at that enclosing defer declaration. |
| Bodyless deferred catches and optional finally | Deferred execution context, independent of whether an error eventually matches. |
| Ordinary group, conditional or loop inside deferred execution | The same deferred execution context; no new cleanup owner. |
| Explicit full scope inside deferred execution | A new independent registration/cleanup context; its own deferred actions cannot append to its draining sequence. Restore the outer context on exit. |
| A separately checked named, local or anonymous callable body | A fresh ordinary callable context. Construction/capture evaluation and the declaration itself retain their enclosing checks. |
| A typeof operand | Structural source constraints retain its enclosing callable/deferred context. Inspection performs no runtime work; H-295 supplies no exemption for otherwise invalid nested source. Separately defined callable bodies still have their independent context. |

Only the final invocation of a plain deferred call is delayed. For example, `defer consume({ defer earlier(); 7u64 });` prepares the grouped argument during ordinary registration. Both declarations are ordinary registrations, subject to existing scope and lifetime rules. A failed outer preparation registers no outer action and retains already successful registrations and their ordinary cleanup obligations.

In `defer { consume({ defer earlier(); 7u64 }); }`, the grouped argument executes as part of deferred work. Its defer declaration is rejected. Explicit `{{ ... }}` around that argument instead gives its defer an independent scope, which drains before argument evaluation returns to the enclosing deferred action.

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

A read-only source-structure pass may reuse the compiler’s scope validation. Its information includes the current cleanup scope and, when applicable, its enclosing deferred execution site. Explicit full scopes and callable bodies start independent cleanup contexts; leaving one restores the surrounding context. H-371 declaration/effect permissions and loop restrictions retain their own scope rules. The full-scope body is an ordinary callable body under H-FULL-SCOPE-CALLABLE-20261117; its implicit construction/capture remains checked in the enclosing context. This context check does not by itself implement ordinary captures or local control flow.

At each defer declaration:

1. If the current cleanup scope is already draining, report the inner declaration and its enclosing deferred execution site.
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
| Give an explicit full scope its own registration/drain context | Selected cleanup boundary: preserves independent scope cleanup without appending to the outer draining sequence. |
| Carry the outer prohibition through every explicit full scope | Rejects independent scope cleanup that the user explicitly requires. |
| Propagate the prohibition into called helper bodies | Rejects independent helper cleanup and makes source validity depend on inlining or call-graph expansion. |

## Acceptance

Negative cases must identify the nested declaration and its enclosing deferred context:

- Plain-call and block-form declarations in a deferred body, catch or attached finally, including a bodyless catch/finally.
- Same-drain nesting through ordinary groups, conditionals and loops. Within an independent full scope, its own deferred body still cannot directly register another action into that scope.
- Nesting in an unused or generic callable body and an unreachable branch. No invocation or specialization is required to discover the structural error.
- A queried nested declaration retains structural checks; nonexecution alone must not hide it. This case does not authorize unsupported statement inspection.
- Nested deferred work in a helper’s own deferred body is rejected within that helper, even when the helper itself is permitted.

Positive cases preserve their source-order effects and ordinary ownership/coverage obligations:

- Several ordinary registrations, ordinary loops and independent full scopes before cleanup.
- A top-level deferred call whose eager callable/argument preparation registers another action. Preserve successful registration order and the failure case where only the earlier action was registered.
- A deferred call to a separately checked helper with its own cleanup; local/anonymous helpers have the same independent body rule where their declarations/construction are otherwise permitted.
- Explicit scopes inside deferred work with permitted variables/effects, independent user defer registration and intrinsic cleanup. Inner cleanup completes before enclosing execution resumes; the outer action list does not grow.
- Type inspection of a cleanup-capable helper under deferred execution: no helper invocation, action registration, capture allocation or input initialization.

Validate source rejection before execution and preserve legal native behavior in specialized and general modes. Active fixtures must enforce the selected restriction and retain meaningful legal helper/preparation counterparts. Their original observations remain available in immutable Git history; passing the older nested-registration prototype is not acceptance of this contract.

## Ordinary callable boundary

[D-17's confirmed full-scope contract](ExceptionControl.md#full-scopes-as-immediate-ordinary-calls) governs captures and control as well as cleanup. `{{ ... }}` has independent invocation-local value inputs, local returns/return inspection and its own owner. No outer loop/yield or catch context crosses that ordinary callable boundary. The implicit callable construction is checked in the enclosing execution context; its body starts a fresh ordinary context. A standalone invocation occurs immediately when the expression is reached; surrounding delayed-block syntax still determines when that happens.

Acceptance of this checker requires the correct cleanup-boundary distinction, not a new runtime callback representation. Broader control/capture lowering and its separately pending contracts must be reported honestly; a structural pass cannot claim to implement them. D-18's value-input contract retains its separately recorded qualification. D-10 separately selects trailing callable arguments. Their construction stays in the enclosing context and their bodies are independent ordinary callables; this checker alone does not establish their implementation.

## Qualification boundary

Whole-file review must cover the candidate value path, this structural source profile, its eager-preparation/callable/query boundaries, diagnostic obligations and acceptance. It does not qualify the independent queued-error/FIFO, checked-pass, return-inspection, aggregate, module-startup, generator or ordinary capture mechanisms. Compiler implementation of this bounded contract remains pending until that review is recorded in the registry.
