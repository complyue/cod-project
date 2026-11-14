# Named quantities and lossless numeric compatibility

Design revision: 1 · Recorded: 2026-11-13 · Status: pending whole-file human review.

## Design authority

- **Record and scope:** [D-27](DesignAuthorityRegistry.md#d-27), revision 1, all sections below. This document consolidates the already selected named-unit, quantity-construction and lossless-compatibility requirements into a bounded implementation contract. It is not qualified until its proposed value path receives human confirmation.
- **Proposed value path:** C-49/P-377 → C-01: independent unit identities make domain distinctions checkable rather than relying on comments or numeric layout. C-61/P-378 → C-01: independent numerical representation and whole-type lossless compatibility preserve that meaning while allowing explicit precision/storage choices. P-379 organizes those responsibilities in this document. Charter §§1–3, 5.3–5.4, 6.3, 8 and 12 require inspectable typed intent, preserved interpretations and native evidence. These are proposed relationships, not new human confirmations.
- **Human baseline:** H-128 establishes independent compile-time unit identities and identity-preserving aliases; H-260 selects independent unit declarations, explicit quantity representations, representation-preserving construction and extraction; H-261 selects whole-type lossless widening, exact-before-widened method selection and least-safe-common builtin arithmetic. H-256 prioritizes this representation separation while keeping fractions deferred. The original records are locatable in the [registry](DesignAuthorityRegistry.md); historical implementations do not qualify this document.
- **Dependencies and invariants:** P-380 records dependencies on D-11/D-16's scoped numeric requirements, D-14's owner/access/initialization rules, D-17's completion and argument preparation, D-18's callable values and D-21/D-26's actual-type selection and checked native coverage. Dependency edges grant no additional authority. The scoped requirements below preserve unit identity, numerical values, access and source evaluation order.
- **Acceptance and exclusions:** the acceptance section defines the evidence required for claimed supported paths. Compound units, fractions, automatic unit-route selection, general module composition, permanent ABI and new generic inference rules are outside this contract. No current implementation result is claimed.

## Meaning and source forms

A named unit is a compile-time identity, distinct from symbols and runtime values. A quantity type combines one such identity with a numerical representation. Equal spelling or equal layout does not make independently declared units equal. Selecting an existing unit under an alias preserves its identity; this file does not define module-selection machinery.

```cod
unit metres;
unit centimetres;
func twice(x: f64(metres)) -> const f64(metres) {
    return metres(scalar(x) * 2.0f64);
}
```

`metres(3u64)` constructs a quantity with the metres identity and the supplied `u64` representation. `metres(3.0f64)` uses the same identity with `f64`. Construction evaluates its operand once. `scalar(value)` explicitly extracts the representation from a quantity. A bare unit is not a scalar value, and adding a unit does not convert an already quantified value to a different unit.

The initial numerical representations are `i64`, `u8`, `u32`, `u64`, `f32` and `f64`. Bool is not numeric. A quantity retains its complete type through local storage, calls, returns, generic substitution and callable contexts. Plain numerical compatibility never removes its unit or replaces it with another one.

Quantity arithmetic has declared typed meaning. The compiler must not infer that arbitrary addition, multiplication or comparison is valid merely because operands have the same underlying representation. Explicit ordinary functions can extract, compute and reconstruct quantities under their declared contracts. Parenthesized operator naming is an existing design direction; operator elaboration and control-flow overloading are outside this document.

## Whole-type lossless compatibility

An automatic numeric widening is allowed only when every value of the source representation is exactly representable in the destination representation. Identity conversion is always valid. The additional primitive destinations are:

| Source | Lossless destinations |
| --- | --- |
| `u8` | `u32`, `u64`, `i64`, `f32`, `f64` |
| `u32` | `u64`, `i64`, `f64` |
| `f32` | `f64` |
| `i64`, `u64`, `f64` | none |

The magnitude of one literal does not change this relation. In particular, `u32 → f32`, `i64/u64 → f64`, float-to-integer and narrowing are not implicit widenings. Floating widening preserves numerical values, signed zero and infinities; it does not promise preservation of NaN payload encoding.

Widening a quantity changes only its representation and preserves its unit identity. For example, `u32(metres) → f64(metres)` is compatible; `u32(centimetres) → f64(metres)` is not a numeric widening. Explicit conversion functions may state and implement a change of unit or representation; their bodies remain checked under ordinary function rules.

Local initialization, independent value assignment and const results use the established value-compatibility rule. Borrowing a target does not widen it: a writable or borrowed reference must preserve the target type, owner and access contract. Widening must not manufacture new aliases or permissions.

For ordinary calls, exact applicable methods take precedence. If none apply, there must be one most-specific widening-compatible method; incomparable best candidates are an error. Return types do not select ordinary methods. A call retains the selected method's declared result and checked exception contract. Argument evaluation and any required conversion finish in source order before evaluating the next argument; failure prevents later effects.

Builtin mixed primitive arithmetic uses a unique least common safe representation or requires an explicit conversion. For example, `u32` with `f32` uses `f64`; `i64` with `f64` has no whole-type lossless common representation. Overflow and other runtime failures retain the existing checked operation and cleanup behavior.

Generic matching remains subject to D-26's actual identity and repeated-type relations. This file does not select broader generic inference/coherence rules, callback variance, conversion of borrowed targets or a common-type inference rule for otherwise unconstrained repeated binders. An unsupported combination must retain a concrete implementation-gap diagnostic instead of silently dropping a relation.

## Implementation boundary

The shared semantic checker establishes the complete quantity type, selected method and numerical adaptation before native emission. Native arithmetic uses the numerical representation; the unit identity remains part of the checked interpretation. General native execution must retain sufficient actual-type evidence to distinguish quantities wherever selection or a same-type constraint needs it. This does not require a user-visible runtime unit object or a per-value wrapper allocation, and selects no permanent carrier encoding or ABI.

Optional specialization may erase evidence whose interpretation is already established. General execution with specialization disabled must produce the same meaning, effects, diagnostics and failures. Runtime values, owner addresses and recursion depth do not require new method definitions. No legacy checker/emitter fallback is part of this contract.

Automatic starred-parameter unit conversion, route search and ambiguity policy remain governed by the separately recorded D-11 decisions and require their own qualified implementation scope. Unit bundles, receiver/source identity composition, arrays/index domains, compound-unit algebra and broader generic quantity constructors are not newly settled here. Fraction construction, arithmetic, promotion, formatting and layout remain deferred under D-16. Ordinary/regional construction and pure/TCO planning retain their own boundaries.

## Acceptance

For each claimed supported source-to-native path, validate both default specialization and general-only execution:

- One unit identity works with every supported numerical representation; distinct units remain distinct despite equal representations. Construction and extraction preserve values and execute operands once.
- Quantities retain their identity through copied and borrowed storage, initialization, mutation, returns, generic calls and complete callable contexts. Wrong units, invalid borrows and access/lifetime violations are rejected.
- Whole-type widening accepts the table above and rejects lossy, value-dependent, cross-unit and reference-covariant alternatives. Exact, most-specific and ambiguous call cases have explicit coverage.
- Mixed primitive arithmetic uses the required least safe representation and rejects incompatible pairs. Boundaries, runtime failure and surrounding cleanup are exercised.
- Explicit functions can convert between named quantities using checked scalar formulas; unavailable automatic routes and module mechanisms are reported as implementation gaps rather than silently approximated.
- Native evidence distinguishes selected semantic methods from optional implementations, retains shared bodies through recursion and shows no allocation solely to attach a unit to primitive bits. Reports identify their source/compiler revisions and do not claim unsupported module, regional, abstract-input or fraction coverage.

Whole-file approval would confirm this bounded value path and contract. It would not approve all of D-11, the excluded mechanisms, a particular checker algorithm or a permanent representation.
