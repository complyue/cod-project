# Scalar call results and receiving storage

Design revision: 10 · Recorded: 2026-11-14 · Status: scalar result and receiving-storage requirements confirmed within the registry scope.

## Design authority

- **Scope / baseline:** [D-23](DesignAuthorityRegistry.md#d-23), r10. H-413–H-419 confirms scalar defaults, declaration operators and unique/null-owner requirements; H-420 authorizes integration and implementation of those rules. Whole-document value-path qualification retains its registry status.
- **Value path:** P-354 relates C-65 storage/lifetimes, C-72 bits and C-54 value/resource distinctions to readable result and receiving intent under charter §§3.3/5.3/6.3/8/12. Its broader document derivation remains pending-human; the source decisions below are settled.
- **Dependencies / invariants:** P-355 links D-14 storage/access/initialization, D-17 completion, D-18 complete callable values, D-16 numeric/fraction validity and D-22 construction. Preserve assignment-family destination views, parameter passing, unique owners, readonly restrictions and cleanup. No regional copy/allocation, lifetime extension, aggregate/generic result classification or permanent ABI is selected.
- **Acceptance / continuity:** The matrices, cleanup examples and final acceptance cases specify the established receiving baseline. [K-STORAGE-SPLIT-20261114](../CHANGELOG.md#k-storage-split-20261114) continues r9→10 with explicit review boundaries: H-426/H-427's grouped-value/readonly-bits reception proposal belongs to D-22; H-429's carrier/placement refactoring belongs to D-28. These pending extensions do not silently change the implemented matrix or a callee's result mode. H-428 permits initialization allocation effects without rollback.

## Result mode belongs to the callable

For an ordinary scalar type T, `-> T` returns independent evaluated bits. An explicit `-> val T` spells the same mode; this is the direct syntax consequence of the selected val default. `-> const T` also supplies independent bits. Const is not required to obtain a value result and does not make an independently created receiving variable const.

An explicit `-> ref T in O` returns a non-null view of existing live storage. `-> ref const T in O` restricts target access; it does not freeze the target. `-> ptr T in O` retains the separately confirmed nullable-view contract. `-> T in O` means `-> ref T in O`, and `-> const T in O` means `-> ref const T in O`. Only bare T, without an owner or reference mode, defaults to val. A val result carries no owner restriction or association: `-> val T in O` is invalid. The storage later holding those bits still has a lifetime, which constrains references to that storage; it is not an owner attached to the independent value.

```text
proc value(x:u64 in o) -> u64 where {owner o} {
  return x;
}
proc increment(x:u64 in o) -> u64 where {owner o} {
  return x + 1u64;
}
proc view(x:u64 in o) -> ref u64 in o where {owner o} {
  return x;
}
```

The first two calls return bits. Even `return x` reads a value rather than exporting x's storage. An ordinary default scalar parameter may therefore be returned by value safely. The last call returns x's target with its actual owner and access; a default callee-local parameter cannot supply that escaping view.

The receiver does not change the callee's mode. A ref declaration receiving value(x) can create storage for the returned bits, but cannot turn that call into a borrow of x. A val declaration receiving view(x) reads and copies its referent after the call completes; it does not change view's borrowed-result contract.

Within a value-result callable, branches returning x and x + 1 both supply values, without a result-owner alternative. Within an explicit borrowed-result callable, each returning branch must supply admissible live storage; a bare fresh arithmetic value is not such a target. H-417 requires all admitted borrowed-result alternatives to have one statically provable owner. If owner inference is unknown or non-unique, reject; simultaneous liveness of different owners is insufficient. Callable selection must preserve each checked result mode; receiver syntax cannot silently adapt an arbitrary union of incompatible callable contracts.

## Call-site matrix: new declarations

[D-22](ConstructionAndAcquisition.md#rhs-categories-grouping-and-value-normalization) reviews grouped scalar values and copying readonly bits during initialization; its detailed alias/grouping integration remains pending. [D-28](RegionalAndLexicalStorageRefactor.md) separates stored-reference representation from actual target owner and result mode. The matrix below retains its independently confirmed baseline until those affected extensions are qualified; it does not veto the proposed changes or claim their implementation.

In these tables V() has an independent scalar value result and B() has an explicit non-null scalar view result. Both have compatible scalar types. Each RHS is evaluated once. Copies require readable initialized source bits and obey existing numerical compatibility rules; borrows preserve exact target type, access and owner. The tables describe ordinary builtin scalar behavior, not arbitrary operator overloads or regional field construction.

| New declaration | From V() | From B() | Status / basis |
| --- | --- | --- | --- |
| `val n:T <- ...` | Initialize n's independent lexical bits from the result. | Read the returned target and initialize an independent copy. | Settled: val storage and bits transfer. |
| `val n:T = ...` | Reject; use `<-` for val initialization. | Reject; val initialization copies bits with `<-`, never aliases with `=`. | Settled: H-415. |
| `ref r:T = ...` | Materialize the returned bits in the declaration's enclosing lexical scope; r refers to that new target. | Bind r to the returned existing target, preserving its owner/access. | Settled: fresh-initializer materialization versus existing-storage binding. |
| `ref r:T <- ...` | Reject: a ref declaration uses `=`. | Reject: request a separate value copy, then bind a ref to its storage. | Settled: H-416. |

A ref declaration uses `=`. An independent scalar result is materialized in caller-local storage; an existing storage expression, including a val variable or a borrowed call result, supplies its existing target. To request an independent copy of a borrowed result, write `val copy:T <- B(); ref r:T = copy;`. The compiler must not turn every ref initializer into a copy. A later rebind changes r's reference slot without converting it into val storage or giving it ownership of its target.

For an independent value, the conceptual expansion `ref r = make T; r <- expression;` describes construction plus initialization as one declaration operation. It is not two ordinary statements publishing an uninitialized readable ref. Preserve once-only evaluation, initializer name lookup in the preceding lexical environment, failure effects and enclosing-scope storage lifetime. This selects no new make grammar, zero initialization, regional allocator or physical storage ABI. The ref-declaration decision does not repeal the separately confirmed ptr transfer shorthand.

Materialized storage belongs to the enclosing full lexical scope, not merely to the initializer expression or the callee. Ordinary `{ ... }` does not add a scope; `{{ ... }}`, calls and module boundaries follow D-17. Ref requires initialized non-null storage before it is exposed as readable. This is lexical bits construction, not lifetime extension of a borrowed source, regional allocation or a promised physical stack slot. A regional reference field cannot retain a caller-local target; its distinct construction rules remain in D-22.

If the receiving reference omits `in O`, its initializer must establish one statically provable owner. An explicit external owner must agree with the returned target; neither omission nor annotation permits runtime owner alternatives. A fresh ordinary scalar return does not acquire that owner from the receiver: a declaration cannot turn `in r` alone into permission to allocate within r. Explicit make and its accepted contextual allocation rules remain separate.

## Call-site matrix: existing variables

Here n already denotes val storage and r already refers to a valid writable scalar target. Owner and access restrictions apply independently.

| Existing-variable operation | From V() | From B() |
| --- | --- | --- |
| `n <- ...` | Write the returned bits into n's existing payload. | Read the returned target and copy its bits into n. |
| `n = ...` | Reject: n has no rebindable reference slot. | Reject for the same reason; val cannot become an alias. |
| `r <- ...` | Write the returned bits into r's current target; retain its reference binding. | Copy the returned target's bits into r's current target; retain its reference binding. |
| `r = ...` | Reject: an independent value supplies no existing target for reference assignment. | Rebind r to the returned target only with the same fixed owner and admissible type/access. |

The new-declaration materialization rule does not allocate a fresh target for every later `r = V()`. To update the existing target use `r <- V()`; to refer to a separate value, first give that value valid storage, then rebind subject to the same-owner rule. A bits transfer may cross owners because it retains no source reference. Ref assignment may not switch owners merely because both are still live.

Declaration-leading const and target-leading const remain independent. `const ref r:T` may receive its first binding and write a writable target but cannot later rebind. `ref r:const T` may rebind to compatible same-owner targets but cannot write through its readonly view. `const val n:T` permits initialization and rejects later payload changes. Copying readable readonly bits into fresh writable storage is allowed; aliasing those bits never grants write access.

A direct null initializer requires a declaration-side owner (`ptr p:T in O = null`) or literal-side owner (`ptr p:T = null in O` or `null in here`); bare `ptr p:T = null` rejects. Both sides must agree when both are explicit. There is no outer selector: an outer-scope variable can supply its existing owner through `=` even when its pointer value is null. A null alternative alongside a view with owner O retains that one expected owner. An explicit nullable result follows ptr rules: `ptr p:T = P()` can forward null without a target read; ref reception requires the established non-null/initialization proof or admitted checked operation. `<-` copies target bits and therefore requires a valid initialized non-null source, with the existing checked-NPE rules where applicable. No implicit zero or null-to-scalar conversion follows.

## Hidden result as a semantic model

A val-returning scalar callable may be understood as having a compiler-private, initially uninitialized result slot of the declared scalar type. The slot is inaccessible to body and cleanup code: no source name, address, alias or capture exposes it. It is a model of the pending result, not a required stack allocation or native ABI.

```text
# Semantic pseudocode, not an ordinary source declaration or callable fallthrough.
hidden val __rtn__:u64;

# return expression:
__rtn__ <- evaluate_and_convert(expression);
pending_completion = Return;
# Leave the current control-flow path and unwind to the callable exit.
# Run the required scope cleanup; do not execute statements after return.
# Deliver __rtn__ only if the final completion is a successful value return.
```

The private store models checked return conversion and capture of independent bits; it does not expose a new assignment destination to user-defined methods. A failed expression/conversion follows exception completion without installing a new return value. No implicit zero value or ordinary required-value fallthrough is introduced. H-361's missing-return behavior remains.

Cleanup writes to the source cannot change saved bits. A later explicit return evaluates a new value and replaces the pending return; a final uncaught throw prevents delivery. Required cleanup still runs, including when a return is replaced. The slot is therefore protected from ordinary program mutation, not immutable against the compiler's explicit completion-replacement protocol. Receiving val/ref storage retains its own independent access qualifiers.

For a borrowed result, the corresponding private operation selects and retains the target, actual owner, access and required type evidence, without copying the target payload. Later rebinding of the source reference does not change this selected target. Writes to the target remain observable. A nullable ptr result also retains absence without a target read. The target owner must survive cleanup and subsequent caller use; a caller-side copy cannot repair an invalid escaping borrow.

```text
proc select(a:u64 in o, b:u64 in o) -> u64 in o where {owner o} {
  ref p = a;
  defer { p = b; a <- 9u64; };
  return p;
}

val a:u64 <- 1u64;
val b:u64 <- 2u64;
ref r = select(a, b);
# r still refers to a, whose payload is now 9; it does not follow p to b.
```

## Other result categories and remaining boundaries

The hidden scalar slot does not qualify other representations. D-14/D-17 already require borrowed non-bits objects to retain their live regional target rather than copy it. An owning region result carries unique cleanup responsibility, is delivered only after successful callee cleanup, and must be released exactly once if abandoned. That protocol is not an ordinary copyable scalar slot.

D-18's complete callable result is an independent intrinsic value, including its complete nested environment. It remains a distinct category from scalar bits; borrowed captures retain their original lifetimes and may observe later target writes. Copying the callable does not snapshot the contents of captured regions.

H-417 rejects ref/ptr initialization and borrowed-result joins without one statically provable owner, closing the different-owner alternative policy. Runtime target selection within one proven owner remains allowed. Static owner inference and its examples are specified in D-14. Abstract results must preserve actual-concrete-type method selection, while the permanent transport of that evidence remains under review in D-18/D-21/D-26. Generic/aggregate result classification and construction/partial-failure protocols belong to D-22/D-24. No conclusion about those choices follows merely from a hidden-result-slot model.

## Evaluation, cleanup and assignment results

A value result obtains its bits when its return expression is evaluated. Later cleanup writes do not change that pending value; a later explicit return may replace it, and all remaining cleanup still executes. An explicit borrowed result retains the selected target through cleanup, so subsequent reads observe completed writes. The target must survive the call.

For an external scalar initially containing 1, suppose the callee executes `return x` and its remaining cleanup writes 9 into x:

| Callable result and receiver | Delivered observation |
| --- | --- |
| Value result; `val n <- V()` | n contains 1. |
| Value result; `ref r = V()` | r refers to independent caller-local storage containing 1. |
| Borrowed result; `val n <- B()` | The independent receiving storage contains 9: the caller reads after callee cleanup. |
| Borrowed result; `ref r = B()` | r aliases x and observes 9, as well as subsequent permitted writes. |

Ordinary call return and block-expression consumption are distinct boundaries. D-14's copying out of an ending block/handler does not allow a borrowed callable result to expose dead callee storage. A caller-side copy cannot repair an invalid borrowed return.

For existing destinations, evaluate the destination access once before the RHS under the established left-to-right rule, retain that selected destination across RHS effects, and validate its lifetime/access. For `=`, the destination is the selected binding slot; for `<-`, it is the selected payload target. Complete the call including cleanup before applying the caller's store/rebind. An exceptional call delivers no scalar result and performs no final receiving store/rebind; its earlier effects and destination-selection effects retain ordinary semantics. This is not transactional rollback. Direct construction optimizations must preserve observations, including aliases that can read a destination during the call.

Assignment-family calls remain special: builtin `=` returns the resulting destination view with its established access; builtin bits `<-` returns a readonly destination view. They do not become independent scalar results merely because their target type is scalar or a method uses const in its result annotation. Thus `ref q = (n <- V())` retains a readonly view of n, while `val q <- (n <- V())` copies n's written bits. Infix, explicit and aliased calls obey the same result contract.

## Acceptance and implementation boundary

1. Bare and explicit val scalar results agree for literals, arithmetic, existing external storage and default callee-local parameters. Mixed existing/fresh return branches produce values in either order, without an owner union.
2. Owner-annotated scalar results are ref shorthand; explicit val-plus-owner rejects. Every declaration/assignment cell preserves the specified storage identity, once-only evaluation and initialized contents. Reject scalar val declaration `=` and ref declaration `<-`; preserve later `<-` writes through an existing ref. Fresh ref initialization materializes a value, while existing-target initialization aliases.
3. Retained aliases observe writes through ref destinations and survive rebinding of another reference without changing their own targets. Val copies and materialized value returns remain independent. Returning a view retains the selected target even if cleanup rebinds its source slot.
4. Value/view cleanup cases above agree through direct, selected, captured/current and recursive calls and deferred invocation. A replacement return uses the declared result mode; exceptional exits perform no final receiving update.
5. Reject unknown/multiple-owner initialization and returned-view joins, invalid escapes, owner changes, readonly escalation, premature reads and invalid nullable conversions. Accept runtime target choices within one proven owner; reject direct null initialization lacking a declaration-side or literal-side owner and reject explicit owner mismatches. Check here against full lexical scopes and owner propagation from an enclosing variable; no outer syntax is admitted. Definite initialization and destination lifetime remain valid across calls and cleanup.
6. Existing assignment-family destination results, parameter rules, complete callable values and non-bits regional storage retain their independently confirmed contracts. No aggregate ABI, generic-result default, dynamic owner union or implicit region allocation is inferred.
7. Numerical conversions retain named-quantity identity and existing lossless compatibility. Future fractions retain fixed bits, validity, explicit canonicalization and their separate syntax/layout/promotion/formatting review; this decision does not implement them or choose aggregate result transport.

Current primitive scalar/owner support, its boundaries and validation commands are recorded in the [source capability reference](../tests/cod/language/README.md). This document states the result/receiving contract; a documentation update supplies no compiler validation evidence.
