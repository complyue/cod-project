# Callable result-type inference

Discussion revision: 5 · Recorded: 2026-11-14 · Status: callable-wide result rules and independent recursive anchors confirmed; remaining topics and whole-file value path pending-human.

## Design authority

- **Registry / scope:** [D-24](DesignAuthorityRegistry.md#d-24), revision 5. [H-421](../CHANGELOG.md#h-421) confirms omitted result annotations with independent return-type evidence, requiring an explicit annotation for an unanchored recursive component. [H-422](../CHANGELOG.md#h-422) establishes this policy at callable scope: func/proc and named/anonymous definitions use the same result rules. These are scoped human baselines; separate-declaration/expected-type constraints, no-value-return bodies, abstract result joins and whole-file qualification remain open.
- **Value path:** C-84/P-356 relates inferred results to C-01/C-06: reduce redundant source spelling while retaining a complete, inspectable checked signature. H-421 accepts that source-policy direction and its local rationale, including the cost of less local signature information; H-422 applies it uniformly across callable definitions. P-357's complete document relationship and the broader candidate charter path remain pending-human. Charter §§2–3/§5.3/§6/§8/§12 requires checked meaning, finite cyclic representation and native behavior consistent with the selected program.
- **Governing constraints:** H-258 requires explicit parameter/result signatures on bodyless declarations. H-361 requires explicit value returns; H-362 permits void fallthrough. H-343 requires native recursion. H-413/H-414 makes a scalar result val by default, keeps explicit scalar `T in O` as a ref result and rejects `val T in O`. H-273/H-342 preserves explicit owners and complete callable values. H-417 requires one statically provable owner for every ref/ptr result. H-196 excludes dynamic effects from functions. H-421/H-422 permits source omission uniformly across callable definitions without weakening these contracts. Callable kind continues to govern effects and callable compatibility independently of result inference.
- **Dependencies / exclusions:** P-358 depends on D-10 method/context selection, D-14 owner/access/initialization, D-17 completion, D-18 callable/native transport, D-16 numeric validity, D-21 native coverage, D-22 construction and D-23 result modes. No owner union, runtime type tag, scalar materialization beyond D-23, new conversion, effect permission, implicit return, pure syntax implementation, TCO or permanent ABI is selected. D-16 fraction constructor/promotion/result-width choices and implementation remain deferred; operator typing cannot bootstrap a missing recursive result type.
- **Acceptance / continuity:** The acceptance cases below cover the confirmed source policy and its dependency boundaries. [K-RESULT-ANCHOR-20261114](../CHANGELOG.md#k-result-anchor-20261114) records the independent-anchor baseline under H-421. [K-CALLABLE-RESULT-20261114](../CHANGELOG.md#k-callable-result-20261114) records r4→5 under H-422, applying the same result policy across callable kinds and naming forms. Explicit and inferred forms must agree within the selected scope; invalid return types, owners, access and initialization reject. Inference must establish the required contract or diagnose an unresolved constraint, and finite native recursion must not depend on source expansion depth. No compiler implementation or test result is claimed.

## Result rules apply to callable definitions

H-421/H-422 permits a callable definition with a body to omit its result annotation under one common rule. The body supplies the checked result contract through explicit returns and independently established result-type evidence. This applies to func and proc, whether named or anonymous. Naming form and callable kind do not select different result defaults, inference permissions or recursive-anchor requirements.

| Callable form | Result contract |
| --- | --- |
| Named func or proc definition | Explicit result annotation, or inference from the body under the common rules. |
| Anonymous func or proc definition | The same explicit/inferred result rules; independent body evidence permits inference without requiring an expected callable type. |
| Bodyless callable declaration | Explicit parameter/result signatures under H-258; there is no body from which to infer a result. |

Callable kind still determines effect permissions and compatibility: a func body cannot gain procedure calls or dynamic effects through this result policy. The same result judgment applies to bodies otherwise legal in their respective kinds. No anonymous parameter inference, new capture permission, implicit return or deferred pure-function syntax is introduced. Whether an external declaration or expected callable type can supply missing result evidence is a separate contextual-typing question below.

This reduces repeated source information while keeping a complete signature available to tools and native lowering; understanding an inferred signature may require inspecting several bodies.

An omitted annotation requests inference. It does not declare void or enable implicit return expressions. An inferred scalar result has val semantics, including when its expression reads existing scalar storage. A borrowed scalar result requires an explicit reference-result contract under D-23. Inference cannot copy a non-bits regional object, change established result modes, extend an owner or invent an owner association for a value.

```text
func sum_to(n:u64) {
  if (n == 0u64) { return 0u64; } else {};
  return n + sum_to(n - 1u64);
}
```

`return 0u64;` independently establishes the u64 result type, with scalar val semantics. The recursive call then has that result contract, and the addition and its return are checked under ordinary operator/type compatibility. The completed signature is equivalent to `-> val u64`. Reversing source order does not change which return provides the type evidence. The same example declared as proc has the same result-type judgment; its callable kind remains proc.

All reachable returning alternatives must be checked against the established result contract. Existing type compatibility and explicitly permitted conversions remain authoritative; an anchor does not exempt other returns from checking. A runtime-selected callable cannot use the first checked body as the result contract for other admitted bodies. The required result and native transport must be established before executable output is published.

## Independent anchors for recursive inference

An anchor is a returning expression whose result type can be established independently of the unresolved result types in the recursive call component. A typed literal such as `0u64` qualifies. A known parameter or nonrecursive expression can also supply such evidence when its type is independently established; literal syntax is not itself required.

`return n + sum_to(n - 1u64);` is not an anchor. Knowing n's type, knowing the recursive call's argument types, or selecting a plausible `+` overload must not be used to assume the recursive call's result type and then justify that same assumption with the resulting addition. The expression is checked after an independent anchor or explicit result annotation has established the required type. Return-site evidence and subsequent return validation are distinct obligations; this does not prescribe a compiler pass order.

```text
func sum_to(n:u64) {
  return n + sum_to(n - 1u64);
}
# Reject: no independent result-type anchor; require an explicit result annotation.
```

With an explicit `-> u64` or `-> val u64`, the recursive result contract is available for checking the body. This says nothing about termination: execution might never reach a return. Missing result-type evidence must not be repaired by assigning void or introducing an inferred non-returning type.

Mutual recursion follows the same dependency rule. An independent return in one method may establish a result constraint propagated through the component; every participating method and returning expression still needs a uniquely established, compatible contract. If result constraints only refer back to unresolved results in that component, require an explicit result annotation to break the dependency. A recursive call edge alone supplies no new type evidence, and repeated body expansion is not inference.

```text
func first() { return second(); }
func second() { return first(); }
# Reject without an explicit result annotation that resolves the cycle.
```

Inference uses the selected lexical/module implementation context and instantiated type/owner parameters. Evidence for one world or argument instantiation cannot silently qualify another. Runtime call depth does not create new source return-type evidence. Caller-provided callable destinations and regional/lexical provenance remain separate from the inferred target type.

## Remaining decisions

- Whether a separate explicit callable declaration or expected callable type may supply or constrain an omitted result, including how several applicable contracts interact. This contextual-typing question applies across callable kinds and naming forms; H-422 does not select its precedence or allow circular evidence. Bodyless signatures remain mandatory.
- Omitted-result bodies with no value-producing return, including normal fallthrough and throw-only bodies. Preserve the distinction between an established void contract, missing required returns and unresolved result evidence; the recursive-cycle rule does not select a general void or non-returning inference policy.
- Where inference stops at abstract result joins and what declared boundary is required. Existing actual-concrete-type method selection remains mandatory; unresolved evidence transport, generic/aggregate result modes and ABI choices are not selected here.

The [scalar-result contract](ScalarCallResults.md) keeps bare scalar T as val and an explicitly owner-annotated scalar result T in O as ref T in O; val T in O is invalid. The receiving declaration or assignment determines copying, materialization or binding after the callee's mode is established. H-415 requires `<-` for scalar val declaration initialization. Receiving syntax cannot infer a borrow from an independent result or supply missing recursive type evidence. Every inferred reference still needs one statically provable owner and valid access/lifetime.

## Acceptance

1. Named and anonymous func/proc definitions with independently typed returns obey the same result policy. The anchored sum_to example infers a scalar val u64 result as either func or proc; corresponding explicit-result forms have the same observations while retaining their callable kinds and effect restrictions. An anonymous body with an independently typed return can infer its result without an expected callable type. A return reading known scalar storage supplies independent bits, not a source alias.
2. The recursive-expression-only example rejects despite the known u64 parameter, recursive arguments and arithmetic operation. A mutually recursive component without independent result evidence rejects; a valid explicit result annotation resolves the corresponding missing contract without making a termination claim.
3. Independent anchors work before or after recursive returns in source order and propagate through self/mutual recursion, forwarding and selected generic instantiations. Check every remaining return and reject incompatible alternatives; do not use operator candidates or caller receiving context as substitutes for the missing anchor.
4. Explicitly borrowed results retain existing alias, owner and access behavior. Copies, readonly restrictions, escaping owners, nullable views and initialization checks follow D-14/D-23; neither omission nor inference repairs an invalid reference result.
5. Named, anonymous, copied, runtime-selected and deferred callable paths establish the same complete result contract before native transport. Operand evaluation, value snapshots, retained targets, pending/replaced completion and exceptional cleanup remain unchanged.
6. Unresolved constraints and unsupported remaining scopes receive explicit diagnostics without unbounded body expansion or invented result modes. Existing void fallthrough and missing-return obligations remain distinct.
7. Preserve selected lexical/module worlds, captured/current effect constraints and exact resource cleanup. Measure inference work and emitted code growth on composed programs; one recursive example does not establish general coverage.
8. Numerical conversions retain named-quantity identity and established compatibility. Future fractions retain their own constructor/result-width/promotion, validity, fixed-bits and formatting decisions; result inference adds no numerical conversion or fraction implementation.

## Implementation boundary

Current compiler availability is recorded in the [source reference](../tests/cod/language/README.md). Parser acceptance or rejection is not evidence of design confirmation. H-421/H-422 confirms callable-wide omission and the independent-anchor rule; remaining decisions and complete document/value-path qualification retain their separate scope. D-26's native coverage obligations apply to the resulting checked contracts without selecting a concrete inference algorithm or permanent representation.
