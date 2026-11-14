# Callable result-type inference

Discussion revision: 1 · Recorded: 2026-11-13 · Status: pending topic discussion and whole-file approval.

## Design authority

- **Registry / scope:** [D-24](DesignAuthorityRegistry.md#d-24), revision 1, all sections, `pending-human`. There is no human-confirmed baseline for this proposal and no delegated implementation contract.
- **Candidate value path:** C-84/P-356 relates inferred method results to C-01/C-06: source can omit redundant type spelling while the semantic graph retains a complete, inspectable contract. P-357 organizes that value in this document. Charter §§2–3/§5.3/§6/§8/§12 require checked meaning, finite cyclic representation and native behavior consistent with the selected program. These relationships require confirmation rather than borrowing authority from dependencies.
- **Confirmed constraints:** H-258 requires explicit parameter/result signatures on bodyless declarations. H-361 requires explicit value returns; H-362 permits void fallthrough. H-343 requires native recursion. H-201/H-273/H-342 preserve result storage, owners and complete callable values; H-196 excludes dynamic effects from functions. These constraints do not independently select omitted-result inference.
- **Dependencies / exclusions:** P-358 depends on D-10 method/context selection, D-14 owner/access/initialization, D-17 completion, D-18 callable/native transport, D-16 numeric validity, and D-21/D-22/D-23 unresolved dispatch/construction/result boundaries. No owner union, runtime type tag, fresh scalar storage, new conversion, effect permission, pure syntax implementation or TCO is selected.
- **Acceptance:** After topic resolution and whole-file approval, explicit and inferred forms must agree across named calls, callable values, recursive graphs, selected module contexts and deferred invocation. Invalid return types, owners, access and initialization remain rejected. Inference must terminate or report an identified unresolved constraint; finite native recursion must not depend on source expansion depth.

## Topic: when may a result annotation be omitted?

The proposal asks whether an implemented named function may omit its result annotation:

```text
func view(region r:Item, n:u64) {
  if (n == 0u64) { return root(r); } else {};
  return view(r, n - 1u64);
}
```

Its apparent result is an existing Item owned by r. The explicit form is `func view(region r:Item, n:u64) -> Item { ... }`. Its acceptance requires a separately qualified inference policy; implementation observations cannot choose that policy.

Two source policies are available:

| Policy | Business-source consequence | Compiler obligation |
| --- | --- | --- |
| Permit omitted results on implemented named functions | The method body supplies its result type; bodyless declarations retain explicit contracts. | Infer and validate the result before selecting native transport, including cyclic calls and runtime callable selection. |
| Require explicit result annotations on every definition | The programmer writes the signature; existing omitted-result examples migrate in one coordinated change. | Check the body against the declared result and diagnose omission directly. |

The proposed direction is to permit omission on implemented named functions. It reduces repeated source information while keeping complete checked signatures available to tools and native lowering. Its cost is a separate inference obligation and potentially less local diagnostics. Requiring annotations makes the signature locally readable and reduces inference complexity, at the cost of more source declarations. Neither policy is activated by this draft.

## Proposed meaning if omission is retained

An omitted annotation requests type inference from the callable's explicit returns. It is not a void declaration and does not enable implicit return expressions. The inferred result must satisfy the existing result-category, access, owner and lifetime rules; inference cannot turn a borrowed object into a copied value or extend its owner.

All reachable returning alternatives participate. Existing type compatibility and explicitly permitted conversions remain authoritative. A runtime-selected callable cannot use the first checked body as the result contract for other bodies. The result and its native representation must be established for every admitted call before executable output is published.

Recursive calls form constraints between signatures. A recursive edge is not evidence that the result is void, and copying the body repeatedly cannot establish its type. In the example, the explicit `return root(r)` supplies an Item constraint and the recursive edge forwards it. Reversing branch order or placing the base return after the recursive branch must preserve the result.

Inference uses the selected lexical/module implementation context and instantiated type/owner parameters. Results discovered for one world or argument instantiation cannot silently qualify another. Existing caller-provided callable destinations and regional/lexical result provenance remain separate from the inferred target type.

This scope proposes omission only for implemented named functions. Extending it to procedure definitions, anonymous definitions or bodyless declarations is not implied. The deferred pure-function proposal remains separate.

## Topics to finish before whole-file approval

- Decide between the two source policies above.
- If inference is retained, settle a recursive cycle with no type-defining returning path: whether it infers a non-returning result, requires an annotation, or follows another explicit rule. Ordinary void fallthrough and missing-return failure must remain distinguishable.
- Specify whether a separate explicit callable declaration may supply the method's result constraint, including when several declarations apply.
- Define where inference must stop at an abstract result boundary. Existing required family dispatch behavior remains, while unresolved differing-concrete evidence/ABI is not selected here.

Fresh arithmetic in an ordinary scalar result remains the separate [scalar-result topic](ScalarCallResults.md). Inferring `u64` alone does not decide where a fresh value lives or permit mixing an external borrow with a fresh result owner. Whole-file approval must retain that exclusion unless the scalar topic is explicitly settled too.

## Acceptance after approval

1. Explicit and inferred versions of the regional example return the same alias; later writes remain visible and expired/readonly access rejects.
2. Self and mutual recursion, both branch orders, generic instantiations and nested forwarding use finite native call graphs with equivalent results.
3. Named, copied, runtime-selected and deferred callable paths establish the same result contract before native transport. Operand evaluation, pending completion and cleanup are unchanged.
4. Unanchored cycles follow the selected policy. Incompatible returning alternatives and conflicting declarations receive clear diagnostics without unbounded expansion or an invented result type.
5. Captured/current effects, module-world selection, owner/initialization checks and exceptional release retain their existing contracts.
6. Measure inference work and emitted code growth on composed programs; do not claim general acceptance from one recursive example.

## Implementation boundary

Current compiler availability is recorded in the [source reference](../tests/cod/language/README.md). Existing parser acceptance or rejection does not qualify either proposed source policy. [K-632](../CHANGELOG.md#k-632) covers editorial continuity; D-24 remains pending-human.
