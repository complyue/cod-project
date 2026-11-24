# Callable result-type inference

Design revision: 13 · Recorded: 2026-11-24 · Status: scoped result-inference contract with unique least common results.

## Design authority

- **Callable results and control destinations.** The [normal-completion decision](../CHANGELOG.md#h-normal-completion-20261124) qualifies [D-24-TAIL](DesignAuthorityRegistry.md#d-24-tail) r13 for void fallthrough and cleanup observation. [H-CALLABLE-TAIL-20261119](../CHANGELOG.md#h-callable-tail-20261119) governs value tails and lexical control destinations. P-631/P-632 connect independent result evidence and exact cleanup to C-84/C-41 and charter §§3.2–3.3/4.3/8.1. Acceptance pairs natural and explicit exits, including the [deferred observations](ExceptionControl.md#return-state-inspection-and-replacement), and retains the non-void, recursive and callback controls below.

- **Standalone throw-only scopes.** [H-STANDALONE-VOID-20261118](../CHANGELOG.md#h-standalone-void-20261118) confirms r10’s narrow default under D-17-FULL-SCOPE and D-24. C-84/P-356–P-357 links independent return evidence to an inspectable callable signature under charter §§3.3/6.3/8. P-358 supplies the completion dependency. The examples below distinguish value, normal void and throw-only results. Ordinary callable and recursive inference, access, checked failures and D-16 remain in force. [K-FULL-SCOPE-CHECKED-INTERFACE-20261118](../CHANGELOG.md#k-full-scope-checked-interface-20261118) records bounded source and native acceptance for throw-only results, queries and explicit value returns. The standalone throw-only default also applies under the current body rules.

- **Full-scope dependency continuity:** [K-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#k-full-scope-implementation-20261118) updates only the referenced r92 qualification. This document’s semantic revision, own qualification, acceptance and exclusions are unchanged.

- **Current capture-review reference:** [K-REPLACEMENT-EFFECTS-20261117](../CHANGELOG.md#k-replacement-effects-20261117) keeps the separate D-18 value-input qualification current. The registry owns the current review scope.

- **Full scopes are ordinary callable invocations:** [H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117)/[K-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#k-full-scope-callable-20261117), [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope)/P-564–P-566, covers r8→9 in the explicit-full-scope interactions below. The user confirms ordinary capture, local return and independent cleanup everywhere; D-17 owns the source acceptance. Reuse C-63/C-65/C-73/C-75/C-84 and charter §§2/3.2–3.3/4.3/6.3/8/12 for one inspectable callable/lifetime model. Preserve actual owners, permissions, declared failures and D-16; the ordinary full-scope result and owner checks apply. [H-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#h-full-scope-implementation-20261118) separately qualifies D-17’s bounded r92 full-scope profile, including immediate ref/ptr inputs; general escaping borrowed-view and declared-place capture mechanisms remain outside it. D-18's value-input profile retains its independent qualification. D-10 separately records the selected trailing callback surface; complete implementation/value-path qualification remains pending.

- **Scope / baseline:** [D-24](DesignAuthorityRegistry.md#d-24), r13. H-421/H-422 establishes callable-wide omission and independent recursive anchors; [H-423](../CHANGELOG.md#h-423) accepts reuse of explicit declarations and delegates routine consequences. [H-424](../CHANGELOG.md#h-424) confirms unique least common results through existing permitted conversions. Func/proc and named/anonymous definitions share this policy. The registry retains the precise qualification of each scope.
- **Value path:** C-84/P-356–P-357 relates reduced source repetition and complete inspectable signatures to C-01/C-06 and charter §§3.3/5.3/6.3/8/12. The local rationale is accepted; broader document/value-path qualification remains separate from the settled source requirements.
- **Dependencies / invariants:** P-358 links D-10/D-14/D-17/D-18/D-21/D-22/D-23 and D-16. Preserve callable effects, lexical return destinations, scalar value/view modes, unique owners, access, initialization, cleanup and numerical compatibility. Existing owner, capture, regional transfer and numeric contracts apply.
- **Acceptance / continuity:** The cases below define observable requirements. [K-COMMON-RESULT-20261114](../CHANGELOG.md#k-common-result-20261114) records r6→7 under H-424; H-423's routine resolutions retain their recorded scope. [K-723](../CHANGELOG.md#k-723) continues the confirmed declaration-context requirement from r7→8 under H-421–H-424; body-local shadows cannot redefine a recursive callable’s retained evidence. Whole-document/value-path qualification remains pending. Source implementation coverage remains in the [compiler reference](../tests/cod/language/README.md).

## Establishing a result contract

A callable definition obtains its result contract from an explicit annotation, an independently established applicable declaration/expected callable type, or independent evidence in its body. The body form selects the expressions that contribute result evidence. All callable kinds use the same compatibility, joining and independent-evidence rules. Bodyless declarations require explicit parameter/result signatures under H-258.

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

## Explicit full scopes

H-FULL-SCOPE-CALLABLE-20261117 makes `{{ ... }}` an ordinary implicit niladic callable followed by invocation. Establish its result independently using these same rules. An explicit return or value tail delivers the inner invocation’s result. That call expression can itself be used in an enclosing return or eligible tail. An outer `return {{ return 7u64; }};` may use the independently established inner result as ordinary call evidence. `{{ 7u64; }}` also yields u64, including inside a proc. With no expected result, a standalone throw-only full scope defaults to void. For a valid exception operand e, `typeof({{ throw e; }})` is void and `typeof({{ return 7u64; }})` is u64. An independently established expected result still applies. A throwing path produces no payload. These scope rules also govern full scopes used in caller-context defaults, subject to the parameter’s expected contract. No new capture, effect, owner or expected-type-driven overload selection permission follows. D-10 separately governs trailing callback arguments; the expected callable contract must be established independently before checking their bodies.

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

Evidence belongs to the selected lexical/module context and instantiated type/owner parameters. Recursive references retain the callable’s definition-site evidence even inside a body scope that shadows an enclosing name. A new nested definition uses its own declaration context, including the enclosing body’s visible shadows. Retained type-only evidence does not capture or read runtime payloads, initialize storage or extend an owner lifetime. It does not transfer silently to another world or instantiation. Every returning alternative must satisfy the established result, including permitted conversions and owner/access/initialization checks. These are dependency obligations, not a prescribed compiler pass order.

## Completion without a value

An explicit bare return supplies void evidence. A normally completing func or full-scope here with no result payload also supplies void evidence. Reaching the end of an ordinary proc supplies a normal no-value path. A named or anonymous proc with no value-return obligation can infer void from that path. When an annotation, applicable declaration or independent value return establishes a non-void result, the required-value rule applies. A throw supplies no payload. An unresolved value return cannot be discarded to infer void.

If a value result is established, a path that falls through without providing it retains H-361's missing-return behavior and diagnostics. If the body supplies neither a value result nor normal no-value completion evidence, as with an ordinary throw-only callable definition or an unanchored recursive cycle, require an explicit result contract. The standalone full-scope default above applies only to the throw-only case. An already declared result remains valid across exceptional/non-returning paths; those paths do not fabricate a payload. H-362 governs cleanup replacement. An explicit empty return and natural void completion are equally observable in the owning invocation’s defer.

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

## Results of proc, func and here bodies

Use the body form to determine which exits provide a result. Apply the same callable-wide type, category, owner and compatibility checks to those results. [Return-state inspection](ExceptionControl.md#return-state-inspection-and-replacement) is confined to deferred execution of the owning callable.

| Source body | Normal completion | Destination of a written return |
| --- | --- | --- |
| Ordinary named or anonymous proc | Execute the final item as a statement, including effects, failure and disposal of an unused result. A declared or inferred void proc may finish there or use an empty return. A non-void result requires a value return under the ordinary missing-result rule. | This procedure. |
| Ordinary named or anonymous func | A reached final value expression supplies the function result. Early explicit returns remain available. | This function. |
| Full-scope here, including standalone `{{ ... }}` | A reached final expression supplies its own invocation result, whether written in a proc or func. Locals, effect permissions and cleanup follow the existing full-scope rules. | This here invocation. |
| Single-brace supplied here group or generator loop body | Its normal tail supplies feedback according to the here/yield signature. The existing void feedback rule can discard it. | The lexical supplying callable under D-18/D-29. `continue value;` explicitly delivers feedback to the invocation. |

```cod
func sum_to(n:u64) {
    if (n == 0u64) { return 0u64; }
    n + sum_to(n - 1u64)
}

proc tick() {
    sum_to(3u64)
}
```

For a body with a value tail, use the normally completing final expression as written. A terminating semicolon delimits it. An explicit `: void` discards a known typed result. Never scan backward past a final declaration, cleanup registration or void expression to recover an earlier value. Explicit returns and the normal tail use the same least-common-result rules. In sum_to the explicit zero anchors the recursive contract. A body consisting solely of the recursive addition still needs independent result evidence. A final if, case or try contributes its ordinary expression result. Exceptional paths and preceding unconditional returns supply no invented tail evidence.

For an ordinary function or a full-scope here invocation, a successful value tail has the effect of `return E;` at the end of that body's own scope. Evaluate the whole E once, including its own try/finally, then prepare the retained result before draining the callable’s deferred work. Existing result mode, stable owner, successful-init, replacement and abandoned-result cleanup rules apply. A scalar val snapshots bits; a returned reference needs a live owner. A value-producing tail makes `if returning` in that callable’s deferred entries observe the prepared result just like an explicit value return. Natural void completion of a proc, func or full-scope here enters cleanup with the same observable normal state as an explicit empty return. A procedure ending in `<-` performs the transfer and discards its readonly destination view. A throw during preparation follows ordinary exceptional completion. A cleanup body’s final expression alone never replaces an enclosing callable result. It takes an explicit enclosing return to do so.

Standalone double braces denote a here invocation even inside a proc. `typeof({{ 7u64; }})` is u64 in either enclosing kind. A directly attached double-brace case arm uses the same local tail rule; an explicit arm fallthrough still runs its cleanup before scanning continues. A single-brace case arm keeps its own case result and break rules. A here parameter’s signature still controls admissible feedback. The checker does not infer a proc capability, yield contract or overload from a tail. A generator retains its final void contract and its independent feedback contract. A proc generator may naturally exhaust or end early with an empty return. Neither exit issues another yield. A named sum constructor initializes the implicit fields and has its own ban on return; a final field assignment cannot become a callable result. Module toplevels likewise have no ordinary caller result. The future purity and tail-call optimization contracts remain independent.

```cod
func plus_one(here body() -> u64) -> u64 { body() + 1u64 }
proc demo() -> u64 {
    val n <- plus_one() { return 7u64; };
    return n + 100u64;
}
```

| Body supplied at the plus_one call | Result of demo under the lexical control rule |
| --- | --- |
| `{ 7u64 }` or `{ continue 7u64; }` | 108 |
| `{{ 7u64 }}` or `{{ return 7u64; }}` | 108 |
| `{ break 7u64; }` | 107 |
| `{ return 7u64; }` | 7 |

This distinction preserves the [generalized for-loop](RangedForAndGenerators.md#generalized-for-loops). Making the last row return 108 would retarget an existing outward exit and would also change how a loop body returns from its supplier.

Check declared and inferred void procedures, including anonymous bodies, with a final transfer. Check a func and a full-scope here with the same final transfer followed by an empty return. Removing that return exposes the transfer’s value tail and can change the inferred result or make a declared void result invalid. The [formatter rule](../docs/IdiomaticsGuideline.md#canonical-code-style) preserves that difference. Compiler support is tracked in the [source reference](../tests/cod/language/README.md#normal-completion-coverage).

## Acceptance

1. Equivalent explicit, separately declared, independently contextual and body-inferred contracts produce the same result behavior across func/proc and named/anonymous definitions. Conflicting declarations reject; ordinary call receiving syntax does not change a signature.
2. Literal/parameter/nonrecursive anchors work in either source order and through self/mutual recursion. Recursive-expression-only inference rejects without an explicit contract; checking never uses an overload assumption to prove itself.
3. Numeric joins match the table in both branch orders; identical anchors preserve their contract. Verify downstream overload selection after widening, conversion at the return boundary and unchanged intermediate arithmetic. Reject absent/incomparable common results and invalid explicit conversions; recursive-only inference still rejects.
4. Normal no-value completion, a missing required value, throw-only bodies and unresolved recursive results remain distinct. Pending/replaced returns and exceptional cleanup preserve D-17's rules.
5. Scalar values, retained reference targets, unique owners, access, initialization and callable environments retain D-14/D-18/D-23's contracts. Copies cannot repair invalid escapes or grant reference covariance; ordinary numerical compatibility and D-16 fraction constraints remain intact.
6. Named, copied, selected and deferred calls establish complete checked result contracts before native transport. Validate generic/world substitutions and finite recursive code graphs; do not claim broad support from one example or this documentation review.
