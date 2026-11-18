# Scalar call results and receiving storage

Design revision: 27 · Recorded: 2026-11-18 · Status: scalar result and receiving-storage requirements confirmed within the registry scope.

## Design authority

- **Parenthesized result projection:** [H-PARENTHESIZED-VALUES-20261118](../CHANGELOG.md#h-parenthesized-values-20261118)/[K-PARENTHESIZED-VALUES-20261118](../CHANGELOG.md#k-parenthesized-values-20261118) covers r26→27’s selected scalar projection under [D-10-PARENS](DesignAuthorityRegistry.md#d-10-parens). Parentheses project a readable scalar target result to independent bits; unparenthesized operator-result views retain their contract. Preserve C-65/C-72/P-354–P-355 and charter §§3.3/4.3/6.3/8/12 through explicit receiving intent, target identity, readonly access, once-only effects and cleanup. The example/acceptance below supersedes the parenthesized-view outcome; general grammar/type-carrier/native qualification and D-16 remain separate. No whole-document value-path approval is inferred.

- **Readonly-reference correction:** [H-READONLY-REF-BINDING-20261117](../CHANGELOG.md#h-readonly-ref-binding-20261117)/[K-READONLY-REF-BINDING-20261117](../CHANGELOG.md#k-readonly-ref-binding-20261117) continues r25→26’s existing access-preserving receiving matrix. A writable `ref` cannot receive a readonly returned target through `=`; implicit fresh storage cannot repair this incompatibility. Explicit `<-` may initialize independent readable bits without changing the source view. Preserve C-65/C-72/P-354–P-355, charter §§3.3/4.3/6.3/8/12, target identity, owners, cleanup and D-16. Acceptance covers rejection, readonly target retention and independent snapshots. D-10-PARENS now supplies explicit scalar projection; conditional/type-carrier integration stays in D-22; no new representation, ABI or broader implementation qualification follows.

- **Discussion-index continuity:** [K-DISCUSSION-SCAN-20261117](../CHANGELOG.md#k-discussion-scan-20261117) continues r24→25 by reconciling the hidden-result summary with H-576/H-593 and D-22’s receiving scope. C-65/C-72/P-354–P-355, charter §§3.3/4.3/6.3/8 and the existing scalar matrices/acceptance remain unchanged; both cross-entry overlap cases are settled, while complete transport/diagnostics retain their qualification boundary. No new source choice, implementation permission or D-16 change.

- **Receiving-overlap boundary:** [H-576](../CHANGELOG.md#h-576)/[H-593](../CHANGELOG.md#h-593)/[K-RECEIVING-CORE-20261117](../CHANGELOG.md#k-receiving-core-20261117), [D-22-RECEIVE-OVERLAP](DesignAuthorityRegistry.md#d-22-receive-overlap)/P-518–P-520, continues r23→24’s dependency summary. Within one receiving spec, cross-entry write/borrowed-read and write/write overlap are author-responsibility UB, with planned warnings/opt-in errors and deferrable costly analysis. Preserve this document’s ordinary single-operation matrix, sequential writes outside the spec, distinct-slot reference sharing, owners/access/init and D-16 under charter §§3.3/4.3/6.3/8/12. D-22 owns acceptance; no general no-target-sharing rule, compiler support or full receiving qualification follows.

- **Receiving-specification review:** [H-574](../CHANGELOG.md#h-574)/[H-575](../CHANGELOG.md#h-575)/[K-RECEIVING-SPEC-20261117](../CHANGELOG.md#k-receiving-spec-20261117) continues r21→22’s dependency wording without changing the scalar matrices. [D-22-RECEIVE-SCOPE](DesignAuthorityRegistry.md#d-22-receive-scope)/P-515–P-517 assesses a declarative map with explicit receiving operations, optional same-name selectors and no arbitrary expressions. Existing destinations retain ordinary pre-RHS selection; new names are not inputs to other specification entries. Preserve C-65/C-72/P-354–P-355, charter §§3.3/4.3/6.3/8/12, private completion, actual owners and D-16. D-22 owns H-576/H-593 overlap rules, H-592 bounded grammar/scope and remaining transport/diagnostic acceptance; no new grammar, proof/ABI or compiler support is qualified here.

- **Output-model continuity:** [H-571](../CHANGELOG.md#h-571)/[K-OUTPUT-ACCEPT-20261117](../CHANGELOG.md#k-output-accept-20261117) covers r18→19 under [D-22-RESULT-DESTINATION](DesignAuthorityRegistry.md#d-22-result-destination)/P-512–P-514. The accepted caller-provided logical destination preserves this document’s independently qualified scalar modes, retained values/targets and post-cleanup receiving updates. Charter §§3.3/4.3/6.3/8 and D-16 remain; no physical output ABI, broader aggregate permission or new implementation qualification follows.

- **Controlled return inspection:** [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state)/[H-542–H-547](../CHANGELOG.md#h-542) and [H-548](../CHANGELOG.md#h-548)/[K-RETURNING-PROFILE-20261116](../CHANGELOG.md#k-returning-profile-20261116) covers r18's hidden-result section under candidate P-482–P-484 and existing P-354–P-355. Author-controlled result inspection/replacement serves C-54/C-63/C-65 and charter §§3.2–3.3/4.3/6.3/8/12 while preserving independent bits, complete callable values, actual borrowed targets, preparation and exact release. D-17 owns the bounded grammar, explicit-void marker, filter/cancellation and binding-lifetime acceptance selected under H-548's simplicity/coherence delegation. Raw slot aliasing, owning-result capture and general regional copying are excluded. Full value-path/physical qualification and source implementation remain separate; ordinary call-site receiving is unchanged.

- **Scope / baseline:** [D-23](DesignAuthorityRegistry.md#d-23), r27. H-413–H-419 confirms scalar defaults, declaration operators and unique/null-owner requirements; H-420 authorizes integration and implementation of those rules. Whole-document value-path qualification retains its registry status; the new return-inspection mechanism has its separate scope above.
- **Value path:** P-354 relates C-65 storage/lifetimes, C-72 bits and C-54 value/resource distinctions to readable result and receiving intent under charter §§3.3/5.3/6.3/8/12. Its broader document derivation remains pending-human; the source decisions below are settled.
- **Dependencies / invariants:** P-355 links D-14 storage/access/initialization, D-17 completion, D-18 complete callable values, D-16 numeric/fraction validity and D-22 construction. Preserve assignment-family destination views, parameter passing, unique owners, readonly restrictions and cleanup. No regional copy/allocation, lifetime extension, aggregate/generic result classification or permanent ABI is selected.
- **Acceptance / continuity:** The matrices, cleanup examples and final acceptance cases specify the established receiving baseline. [K-STORAGE-SPLIT-20261114](../CHANGELOG.md#k-storage-split-20261114) continues r9→10 with explicit review boundaries: H-PARENTHESIZED-VALUES-20261118 selects scalar projection and D-22 retains conditional/type-carrier integration; H-READONLY-REF-BINDING-20261117 retains ordinary readonly-reference binding and explicit bits transfer; H-429's carrier/placement refactoring belongs to D-28. These pending extensions do not silently change the implemented matrix or a callee's result mode. H-428 permits initialization allocation effects without rollback. [K-REFERENCE-IDENTITY-20261114](../CHANGELOG.md#k-reference-identity-20261114) integrates H-439/H-440 owner-plus-locator identity and is null, preserving established access, result and lifetime behavior. Current source/native coverage and proof boundaries are recorded in the [source capability reference](../tests/cod/language/README.md).
- **Current implementation alignment:** [K-714](../CHANGELOG.md#k-714) continues r14→15 under H-416/H-417/H-420/H-423 and P-255/P-257/P-258/P-354–P-355. The [source capability reference](../tests/cod/language/README.md) covers scalar receivers that choose an existing target or an independent value within one proven owner. Acceptance preserves selected identity versus value snapshots, once-only evaluation, readonly access, initialization and cleanup. General borrowed-argument materialization, external-owner allocation, numeric joins and regional representation retain their separate boundaries. [K-718](../CHANGELOG.md#k-718) continues r15→16 by aligning current coverage with D-18’s independently qualified complete callable values: callable ptr mixtures use the same receiving/cleanup path without changing this scalar matrix or classifying callables as bits.

## Result mode belongs to the callable

For an ordinary scalar type T, `-> T` returns independent evaluated bits. An explicit `-> val T` spells the same mode; this is the direct syntax consequence of the selected val default. `-> const T` also supplies independent bits. Const is not required to obtain a value result and does not make an independently created receiving variable const.

An explicit `-> ref T in O` returns a non-null view of existing live storage. `-> ref const T in O` restricts target access; it does not freeze the target. `-> ptr T in O` retains the separately confirmed nullable-view contract. `-> T in O` means `-> ref T in O`, and `-> const T in O` means `-> ref const T in O`. Only bare T, without an owner or reference mode, defaults to val. A val result carries no owner restriction or association: `-> val T in O` is invalid. The storage later holding those bits still has a lifetime, which constrains references to that storage; it is not an owner attached to the independent value. Under H-439/[D-28 identity contract](RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator), a named val variable or a borrow of it supplies an existing storage target for is; an independent arithmetic or val-returning call result does not. Identity comparison allocates no temporary. Thus val n followed by ref r = n preserves n is r, whereas a separate copied val has a different target. H-440 null tests retain the pointer’s owner without reading that target.

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

[D-10-PARENS](SymbolicDynamicScope.md#parenthesized-value-expressions-and-operator-idioms) selects explicit parenthesized scalar projection; [D-22](ConstructionAndAcquisition.md#rhs-categories-grouping-and-value-normalization) retains conditional/type-carrier integration work. [H-READONLY-REF-BINDING-20261117](../CHANGELOG.md#h-readonly-ref-binding-20261117) preserves this matrix’s access checks: a readonly returned reference cannot initialize a writable reference through `=`, and automatic fresh storage cannot make that declaration valid. [D-28](RegionalAndLexicalStorageRefactor.md) separates stored-reference representation from actual target owner and result mode. This projection changes the RHS category before reception; it does not authorize receiver-driven materialization of an unprojected readonly view or claim compiler support.

In these tables V() has an independent scalar value result and B() has an explicit non-null scalar view result. Both have compatible scalar types. Each RHS is evaluated once. Copies require readable initialized source bits and obey existing numerical compatibility rules; borrows preserve exact target type, access and owner. The tables describe ordinary builtin scalar behavior, not arbitrary operator overloads or regional field construction.

| New declaration | From V() | From B() | Status / basis |
| --- | --- | --- | --- |
| `val n:T <- ...` | Initialize n's independent lexical bits from the result. | Read the returned target and initialize an independent copy. | Settled: val storage and bits transfer. |
| `val n:T = ...` | Reject; use `<-` for val initialization. | Reject; val initialization copies bits with `<-`, never aliases with `=`. | Settled: H-415. |
| `ref r:T = ...` | Materialize the returned bits in the declaration's enclosing lexical scope; r refers to that new target. | Bind r to the returned existing target, preserving its owner/access. | Settled: fresh-initializer materialization versus existing-storage binding. |
| `ref r:T <- ...` | Reject: a ref declaration uses `=`. | Reject: request a separate value copy, then bind a ref to its storage. | Settled: H-416. |

A ref declaration uses `=`. An independent scalar result is materialized in caller-local storage; an existing storage expression, including a val variable or a borrowed call result, supplies its existing target. To request an independent copy of a borrowed result, write `val copy:T <- B(); ref r:T = copy;`. The compiler must not turn every ref initializer into a copy. A later rebind changes r's reference slot without converting it into val storage or giving it ownership of its target.

For a callable returning `ref const T in O`, `ref live:const T = B()` retains its readonly target and owner, while `ref writable:T = B()` rejects. `val snapshot:T <- B()` explicitly reads initialized bits into independent storage; a later `ref writable:T = snapshot` binds to that storage. This distinction is not a conversion of a readonly reference into a writable one. Omitted receiving type annotations preserve inferred target access; explicit owner mismatch also rejects rather than triggering allocation. Declaration-leading `const ref` only restricts rebinding and cannot remove target constness.

For an independent value, the conceptual expansion `ref r = make T; r <- expression;` describes construction plus initialization as one declaration operation. It is not two ordinary statements publishing an uninitialized readable ref. Preserve once-only evaluation, initializer name lookup in the preceding lexical environment, failure effects and enclosing-scope storage lifetime. This selects no new make grammar, zero initialization, regional allocator or physical storage ABI. The ref-declaration decision does not repeal the separately confirmed ptr transfer shorthand.

Materialized storage belongs to the enclosing full lexical scope, not merely to the initializer expression or the callee. Ordinary `{ ... }` does not add a scope; `{{ ... }}`, calls and module boundaries follow D-17. Ref requires initialized non-null storage before it is exposed as readable. This is lexical bits construction, not lifetime extension of a borrowed source, regional allocation or a promised physical stack slot. A regional reference field cannot retain a caller-local target; its distinct construction rules remain in D-22.

A scalar initializer may select between an existing target and an independent value at runtime. The target arm retains its selected storage; the value arm materializes its evaluated bits in the receiving declaration’s full lexical scope after successful initializer completion. Every possible target, including that materialized storage, must have the same statically provable owner. Cleanup may change the contents observed through the retained target, but does not change an already evaluated independent value. A failed or returning initializer publishes no receiving binding. The result grants only the access common to the admitted target alternatives; no owner union, implicit conversion of a borrowed target, or general materialization of borrowed call arguments follows.

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

[D-22’s accepted common result model](ConstructionAndAcquisition.md#caller-provided-result-destinations) permits the private result destination to be provided by the caller while preserving the completion boundary below. The logical destination may lower to registers or storage. It is not automatically the visible lhs of a receiving assignment, and a borrowed result’s carrier never becomes its target owner. D-22 assesses H-574/H-575’s declarative receiving specification for new/existing destinations, with explicit operations, optional field-name omission and no arbitrary expressions. It preserves the matrix and pre-RHS destination selection here; D-22’s H-576/H-593 makes cross-entry write/borrowed-read and write/write overlap UB, with planned warnings and opt-in errors. Its bounded receiving grammar and ordinary full-scope treatment are recorded there; complete proof/native transport and diagnostic coverage remain separate from these selected source rules.

A val-returning scalar callable may be understood as having a compiler-private, initially uninitialized result slot of the declared scalar type. No source name, address or ordinary alias exposes that slot. [D-17's return-state mechanism](ExceptionControl.md#return-state-inspection-and-replacement) supplies controlled inspection and branch bindings without making the slot writable by ordinary source assignment. It is a model of the retained result, not a required stack allocation or native ABI.

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

Cleanup writes to the source cannot change saved bits. A successful explicit return replaces the retained result and may replace the ordinary body exception. An escaping ordinary exception that cancels a return clears its result; an ordinary catch does not revive it. A failure handled entirely inside finally leaves the suspended surrounding result intact when that finally completes normally. An escaping deferred failure instead follows D-17's FIFO queue and does not itself clear the result. Final residual failures prevent delivery, and required cleanup still runs. The slot is protected from ordinary program mutation while explicit completion control can replace or clear it. Receiving val/ref storage retains its own independent access qualifiers.

Return preparation installs a new result only after evaluation and conversion succeed. If preparation fails, the applicable failure path above determines whether the old result remains or is cancelled; there is no partially installed replacement. `if returning (val rtn)` binds an independent scalar value on a match. Mutating that binding does not mutate the retained slot; `return rtn` performs an explicit replacement. Reference/pointer patterns retain the checked target/owner/access without copying its payload or granting ownership. Type filters inspect the already prepared return type and add no numerical conversion. D-17 records the bounded pattern grammar, mismatches, existing-owner filters, null evidence and binding failure. Explicit return without a payload matches (); normal void fallthrough does not. Ordinary branch bindings use their existing lifetimes across replacement: bits and complete callable copies stay independent, borrowed views keep their selected live target. Owned results permit non-capturing () inspection only; capturing or borrowing that private owning result rejects. No new ownership/lifetime transport follows from inspection.

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

Assignment-family calls remain special: builtin `=` returns the resulting destination view with its established access; builtin bits `<-` returns a readonly destination view. Those operator results do not become independent scalar bits merely because their target type is scalar or a result annotation uses const. H-PARENTHESIZED-VALUES-20261118 makes expression parentheses a separate explicit value projection: `ref live:const T = n <- V();` retains the view, whereas `ref snapshot:T = (n <- V());` receives independent bits and materializes a fresh target. `val snapshot:T <- n <- V();` explicitly transfers the written bits without needing parentheses. Infix and explicit calls, including name/identity aliases of the operator, retain the operator’s own contract; surrounding expression syntax determines how that result is consumed.

```cod
ref n:u64 = 1u64;
ref live:const u64 = n <- 2u64;
ref snapshot:u64 = (n <- 3u64);
n <- 4u64;
assert live == 4u64;
assert snapshot == 3u64;
assert live is n;
assert snapshot is not n;
```

Projection reads the completed result once, after the enclosed call/operator and its cleanup/effects finish. Later cleanup cannot change those independent bits. A grouped independent value remains independent without another target read. `(n) is n` rejects for scalar n because projection supplies no target; identity comparison does not allocate one. Parenthesized projected bits cannot name the original scalar destination. Ordinary call/signature delimiters and operator-name syntax are not expression projection. No new scope, numeric conversion, ownership transfer or source-access upgrade follows.

[D-28](RegionalAndLexicalStorageRefactor.md#integrated-operations-and-lifetimes) carries this receiving contract through lexical backing growth. For `=`, retain the selected binding slot and encode the delivered target for that slot; for `<-`, retain the selected payload target across the call. Slot and target owners may grow independently. Borrowed results retain their chosen live target through cleanup and adapt to the receiver's owner/representation relationship; val results retain their already converted independent bits. No growth-induced reevaluation, source reread or change to the matrices above is allowed. H-437/[D-28-OWNERS](RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations) requires borrowed results to retain the correct shared owner association through cleanup and reception, with current resolution information after permitted growth. Independent scalar val results require no owner metadata. Exact physical transport remains a separate review scope.

## Acceptance and implementation boundary

1. Bare and explicit val scalar results agree for literals, arithmetic, existing external storage and default callee-local parameters. Mixed existing/fresh return branches produce values in either order, without an owner union.
2. Owner-annotated scalar results are ref shorthand; explicit val-plus-owner rejects. Every declaration/assignment cell preserves the specified storage identity, once-only evaluation and initialized contents. Reject scalar val declaration `=` and ref declaration `<-`; preserve later `<-` writes through an existing ref. Fresh ref initialization materializes a value, while existing-target initialization binds to that target. A readonly returned target cannot initialize a writable ref; explicit `<-` into independent val storage remains allowed.
3. Retained aliases observe writes through ref destinations and survive rebinding of another reference without changing their own targets. Val copies and materialized value returns remain independent. Returning a view retains the selected target even if cleanup rebinds its source slot.
4. Value/view cleanup cases above agree through direct, selected, captured/current and recursive calls and deferred invocation. A replacement return uses the declared result mode; exceptional exits perform no final receiving update.
5. Reject unknown/multiple-owner initialization and returned-view joins, invalid escapes, owner changes, readonly escalation, premature reads and invalid nullable conversions. Accept runtime target choices within one proven owner; reject direct null initialization lacking a declaration-side or literal-side owner and reject explicit owner mismatches. Check here against full lexical scopes and owner propagation from an enclosing variable; no outer syntax is admitted. Definite initialization and destination lifetime remain valid across calls and cleanup.
6. Assignment-family destination results retain their contract; explicit parenthesized scalar consumption projects bits, while unparenthesized reference reception preserves the target. Cover snapshots, identity/destination rejection and delimiter distinction. Parameter rules, complete callable values and non-bits regional storage retain their separate contracts. No aggregate ABI, generic-result default, dynamic owner union or implicit region allocation is inferred.
7. Numerical conversions retain named-quantity identity and existing lossless compatibility. Future fractions retain fixed bits, validity, explicit canonicalization and their separate syntax/layout/promotion/formatting review; this decision does not implement them or choose aggregate result transport.

Current primitive scalar/owner support, its boundaries and validation commands are recorded in the [source capability reference](../tests/cod/language/README.md). This document states the result/receiving contract; a documentation update supplies no compiler validation evidence.
