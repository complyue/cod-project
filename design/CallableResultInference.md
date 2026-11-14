# Callable result-type inference

Design revision: 6 · Recorded: 2026-11-14 · Status: scoped result-inference contract; common-result synthesis remains open.

## Design authority

- **Scope / baseline:** [D-24](DesignAuthorityRegistry.md#d-24), r6. H-421/H-422 establishes callable-wide omission and independent recursive anchors; [H-423](../CHANGELOG.md#h-423) accepts reuse of explicit declarations and delegates routine consequences. Func/proc and named/anonymous definitions share this policy. The registry retains the precise qualification of each scope.
- **Value path:** C-84/P-356–P-357 relates reduced source repetition and complete inspectable signatures to C-01/C-06 and charter §§3.3/5.3/6.3/8/12. The local rationale is accepted; broader document/value-path qualification remains separate from the settled source requirements.
- **Dependencies / invariants:** P-358 links D-10/D-14/D-17/D-18/D-21/D-22/D-23 and D-16. Preserve callable effects, explicit value returns, scalar value/view modes, unique owners, access, initialization, cleanup and numerical compatibility. No implicit value return, owner union, new capture permission, regional copy, fraction promotion/implementation or permanent ABI follows.
- **Acceptance / continuity:** The cases below define observable requirements. [K-INFERENCE-CLEANUP-20261114](../CHANGELOG.md#k-inference-cleanup-20261114) records r5→6 and distinguishes H-423's confirmation from the agent's routine resolutions. Source implementation coverage remains in the [compiler reference](../tests/cod/language/README.md).

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

## Open decision: combining different independent result types

An established expected result already determines how each return is checked. Returns with the same independently established result contract also require no new joining rule. The material unresolved case is different independent anchor types with no expected result:

```text
func choose(flag:bool) {
  if (flag) { return 1u32; } else { return 2u64; };
}
```

Existing numeric compatibility allows u32 to widen to u64 when u64 is required. It does not by itself decide whether an omitted result should choose that common type.

| Policy | Consequence |
| --- | --- |
| Require matching independent result types; otherwise require an explicit result | The example needs `-> u64`. Adding a differently typed branch forces the source to state its result boundary. |
| Infer a unique least common result under existing permitted conversions | The example infers val u64. Adding a wider branch can change the inferred signature; incomparable candidates require an explicit result. |

Neither alternative permits first-return wins, declaration-order choice, value-dependent numeric narrowing, a synthesized owner union or a new implicit conversion. The choice also needs a clear boundary for nominal/abstract result joins and future fraction representations. Abstract results must preserve actual concrete-type method selection under D-18/D-21/D-26; physical evidence transport remains their separate question. Generic/aggregate result classification remains in D-22. No option in this table is activated by this cleanup.

## Acceptance

1. Equivalent explicit, separately declared, independently contextual and body-inferred contracts produce the same result behavior across func/proc and named/anonymous definitions. Conflicting declarations reject; ordinary call receiving syntax does not change a signature.
2. Literal/parameter/nonrecursive anchors work in either source order and through self/mutual recursion. Recursive-expression-only inference rejects without an explicit contract; checking never uses an overload assumption to prove itself.
3. Normal no-value completion, a missing required value, throw-only bodies and unresolved recursive results remain distinct. Pending/replaced returns and exceptional cleanup preserve D-17's rules.
4. Scalar values, retained reference targets, unique owners, access, initialization and callable environments retain D-14/D-18/D-23's contracts. Copies cannot repair invalid escapes or grant reference covariance; ordinary numerical compatibility and D-16 fraction constraints remain intact.
5. Named, copied, selected and deferred calls establish complete checked result contracts before native transport. Validate generic/world substitutions and finite recursive code graphs; do not claim broad support from one example or this documentation review.
