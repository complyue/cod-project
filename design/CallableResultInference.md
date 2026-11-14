# Callable result-type inference

Design revision: 7 · Recorded: 2026-11-14 · Status: scoped result-inference contract with unique least common results.

## Design authority

- **Scope / baseline:** [D-24](DesignAuthorityRegistry.md#d-24), r7. H-421/H-422 establishes callable-wide omission and independent recursive anchors; [H-423](../CHANGELOG.md#h-423) accepts reuse of explicit declarations and delegates routine consequences. [H-424](../CHANGELOG.md#h-424) confirms unique least common results through existing permitted conversions. Func/proc and named/anonymous definitions share this policy. The registry retains the precise qualification of each scope.
- **Value path:** C-84/P-356–P-357 relates reduced source repetition and complete inspectable signatures to C-01/C-06 and charter §§3.3/5.3/6.3/8/12. The local rationale is accepted; broader document/value-path qualification remains separate from the settled source requirements.
- **Dependencies / invariants:** P-358 links D-10/D-14/D-17/D-18/D-21/D-22/D-23 and D-16. Preserve callable effects, explicit value returns, scalar value/view modes, unique owners, access, initialization, cleanup and numerical compatibility. No implicit value return, owner union, new capture permission, regional copy, fraction promotion/implementation or permanent ABI follows.
- **Acceptance / continuity:** The cases below define observable requirements. [K-COMMON-RESULT-20261114](../CHANGELOG.md#k-common-result-20261114) records r6→7 under H-424; H-423's routine resolutions retain their recorded scope. Source implementation coverage remains in the [compiler reference](../tests/cod/language/README.md).

## Establishing a result contract

A callable definition obtains its result contract from an explicit annotation, an independently established applicable declaration/expected callable type, or independent evidence in its body. Callable kind controls effects and callable compatibility; it does not select a different result-inference policy. Bodyless declarations require explicit parameter/result signatures under H-258.

A separate declaration already states the result; the definition need not repeat it:

```text
func sum_to(n:u64) -> u64;
func sum_to(n:u64) {
  return n + sum_to(n - 1u64);
}
```

The declaration establishes val u64 before the recursive body is checked. This is an explicit contract, not circular inference. The same principle applies when a callable definition has an independently known expected callable type. It supplies any missing result contract, including result mode, target access and owner constraints. An ordinary scalar result defaults to val when no explicit reference-result contract applies; an owner annotation never attaches an owner to val bits.

All applicable declarations remain obligations. Identical result requirements are consistent; incompatible requirements reject. If applicability or the result remains unresolved, require enough explicit information to establish the contract. An annotation cannot waive an incompatible declaration. A candidate overload's guessed result expectation cannot justify selecting that same candidate; ordinary dispatch remains independent of result-type tie breaking.

Context checks a definition or a callable view against an established contract. It does not retroactively change an existing callable's checked method signature for each consumer. A call's receiving val/ref variable and `=`/`<-` operation supply no result-type evidence to the callee; D-23 governs receiving storage after the call's result is established. Parameter inference, new anonymous syntax and capture permissions are separate contracts.

## Independent evidence in recursive bodies

```text
func sum_to(n:u64) {
  if (n == 0u64) { return 0u64; } else {};
  return n + sum_to(n - 1u64);
}
```

`return 0u64;` independently establishes u64. The recursive call, addition and remaining return are then checked against that result contract. A typed parameter or nonrecursive expression can also supply independent evidence; a literal is not required. Source order does not change which returns supply evidence.

`return n + sum_to(n - 1u64);` supplies no independent anchor. Knowing n's type, the recursive argument types or possible `+` overloads cannot establish the missing recursive result. Without a declaration, annotation or other independent anchor, this body rejects. An explicit result contract permits type checking without claiming that execution terminates.

Mutual recursion follows the same rule: establish contracts from independent evidence, then check every dependent return. A component whose result constraints only refer back to its unresolved results requires an explicit contract. Repeated body expansion, inferred void and an invented non-returning type cannot repair that cycle.

Evidence belongs to the selected lexical/module context and instantiated type/owner parameters. It does not transfer silently to another world or instantiation. Every returning alternative must satisfy the established result, including permitted conversions and owner/access/initialization checks. These are dependency obligations, not a prescribed compiler pass order.

## Completion without a value

When no explicit result contract applies, a body with no value-producing return and an independently established normal completion supplies void evidence. This infers a no-value contract; it does not add implicit value-return expressions. A normal no-value exit supplies a type constraint without requiring proof that execution reaches it. A body containing an unresolved `return expression` cannot discard that obligation and infer void from another fallthrough path.

If a value result is established, a path that falls through without providing it retains H-361's missing-return behavior and diagnostics. If the body supplies neither a value result nor normal no-value completion evidence, as with a throw-only body or an unanchored recursive cycle, require an explicit result contract. An already declared result remains valid across exceptional/non-returning paths; those paths do not fabricate a payload. H-362's cleanup replacement and void completion rules remain unchanged.

## Unique least common results

When no explicit or independently supplied expected result applies, combine the independently established returning contracts using existing permitted result conversions. Infer their unique least common result. This applies uniformly to func/proc and named/anonymous definitions under H-424.

A common result must accept every returning contract through those existing conversions. The least result is a common candidate that is compatible with every other common candidate under the same relation. Choose by semantic type compatibility, not spelling, source order, literal magnitude or shared machine representation. If there is no common candidate or no unique least candidate, require an explicit result contract and valid conversions; an annotation cannot authorize an otherwise invalid conversion.

```text
func choose(flag:bool) {
  if (flag) { return 1u32; } else { return 2u64; };
}
# Inferred result: val u64; the u32 branch widens at its return boundary.
```

| Independent numeric return types | Inferred result under established compatibility |
| --- | --- |
| u32 and u64 | val u64 |
| u32 and f32 | val f64 |
| i64 and u64 | No whole-type lossless numeric join; require an explicit contract and valid explicit conversions. |
| u32 and u64 representations of the same named quantity | That same quantity with u64 representation. |

Every admitted return is checked against the completed contract. A known expected result is checked directly rather than replaced by a synthesized one. The recursive-anchor requirement remains: an unresolved recursive result or operator candidate is not an independent input to the join, and common-result inference cannot make an unanchored recursive component valid. Reordering branches must preserve the result and the conversion obligations.

Conversion happens at the return boundary under D-17/D-23 sequencing and cleanup rules. Inferring a wider result does not change the types or overflow behavior of earlier intermediate computations. Adding a wider independent return can change the inferred signature and downstream overload selection, including calls reached through the original branch; this is an accepted consequence of inference. The receiving variable does not choose that signature, and ordinary method lookup still does not use an expected result as a tie-breaker.

The relation preserves complete result contracts: value/view mode, target access, owner and nominal identity remain constraints. Bits widening creates no reference covariance, owner union, mutable access or lifetime extension. Named quantities retain their units; the rule introduces no new numeric conversion, implicit unit conversion, synthetic union/intersection type or allocation. Distinct reference owners cannot acquire a common owner through type inference.

Nominal/abstract results may join only through already admitted compatibility, preserving actual-concrete-type method selection. Multiple incomparable common contracts require an explicit boundary. Physical type-evidence transport remains in D-18/D-21/D-26; generic/aggregate result classification remains in D-22. Future fraction joins depend on their separately established constructor/promotion/result-width rules; this policy neither selects those conversions nor implements fractions.

## Acceptance

1. Equivalent explicit, separately declared, independently contextual and body-inferred contracts produce the same result behavior across func/proc and named/anonymous definitions. Conflicting declarations reject; ordinary call receiving syntax does not change a signature.
2. Literal/parameter/nonrecursive anchors work in either source order and through self/mutual recursion. Recursive-expression-only inference rejects without an explicit contract; checking never uses an overload assumption to prove itself.
3. Numeric joins match the table in both branch orders; identical anchors preserve their contract. Verify downstream overload selection after widening, conversion at the return boundary and unchanged intermediate arithmetic. Reject absent/incomparable common results and invalid explicit conversions; recursive-only inference still rejects.
4. Normal no-value completion, a missing required value, throw-only bodies and unresolved recursive results remain distinct. Pending/replaced returns and exceptional cleanup preserve D-17's rules.
5. Scalar values, retained reference targets, unique owners, access, initialization and callable environments retain D-14/D-18/D-23's contracts. Copies cannot repair invalid escapes or grant reference covariance; ordinary numerical compatibility and D-16 fraction constraints remain intact.
6. Named, copied, selected and deferred calls establish complete checked result contracts before native transport. Validate generic/world substitutions and finite recursive code graphs; do not claim broad support from one example or this documentation review.
