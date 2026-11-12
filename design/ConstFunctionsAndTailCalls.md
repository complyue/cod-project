# Const functions and tail calls: deferred research plan

Design revision: 2 · Recorded: 2026-11-11 · Status: deferred pre-research; no implementation scheduled.

## Design authority

- **Pure declaration surface:** [H-363](DesignAuthorityRegistry.md#h-363)/[K-520](DesignAuthorityRegistry.md#k-520) selects `pure` to declare a const function whose final body expression provides its result without requiring `return`. C-76/P-305/P-325 preserves the readonly-external/private-mutation boundary under charter §§2–3/§6/§12. The user explicitly keeps purity implementation and TCO deferred. Acceptance and unresolved callable-type/checking details remain those below; this syntax choice does not qualify an optimizer purity attribute or authorize implementation.

- **Registry / covered scope:** [D-19](DesignAuthorityRegistry.md#d-19), with human evidence [H-345–H-348](DesignAuthorityRegistry.md#h-345) and review [K-464](DesignAuthorityRegistry.md#k-464). The confirmed scope is the direction, outer-call boundary, deferred priority and conflict-review requirement. H-363 selects the pure declaration keyword and implicit final-expression result. Callable-type details, static checking and TCO eligibility remain pending-human before implementation.
- **Value path:** C-76/P-304/P-305 derives from C-36's explicit function/effect boundary and C-65's checked storage/access/lifetimes, serving charter §§1–3/§6/§8: readable functional computation with explicit effects and safe native execution. P-306 records dependencies; P-307 requires human review of material conflicts. Dependencies do not confer authority.
- **Invariants / non-goals:** Preserve regional relocation, owner validity, ordinary function/procedure semantics and module-world/TLS separation. No implicit snapshot copy, permanent freezing, caller-state rollback, blanket optimizer purity attribute, persisted native callable or general continuation runtime is selected. Const-function/TCO implementation and persistent-data libraries are deferred.
- **Acceptance / baseline:** H-345 requires recursion in current work; H-346 restricts the future TCO exploration to const functions; H-347 supplies the immediate-return and private-mutation boundary; H-348 authorizes this plan and requires consultation over material conflicts. Future acceptance below is a proposed research checklist, not test evidence or an instruction to start implementation.

## Priority and scope

Recursive calls are a required part of the current callable/runtime work, for functions and procedures. Recursion does not require a tail-call guarantee, and success at finite source inlining is not a substitute for recursion support.

The optional functional-programming direction combines const functions with persistent data structures. Its initial TCO exploration is confined to const functions. TCO for ordinary functions or procedures is not planned here. Complexity or cost may justify narrowing or abandoning the optional direction after human review; the recursion requirement remains.

Current development needs to remain materially compatible with this direction. It need not implement a const qualifier, persistence, tail-call checking or a speculative optimization framework now.

## Declaration surface

`pure f(...) -> T { ...; expression }` declares a const function with its final body expression as the result. H-361/H-362 requires explicit value returns from ordinary proc/func brace bodies and permits void fallthrough. The pure form preserves concise functional definitions without changing the ordinary function/procedure result contract. This declaration spelling is recorded for future implementation; callable-type syntax, helper/callback admission and purity checking remain unimplemented.

## Boundary of a const function

The intended observation interval is the outermost const-function invocation, from entry through its immediate return. Over that interval:

1. Pre-existing external state reachable through parameters, captures, field references, aliases or further dereferences is readonly.
2. Computation may use mutable local variables and allocate, initialize, edit and release private regions. Private state must not become an externally observable mutation of pre-existing state.
3. No dynamic effects, actor mailbox send/receive or other external effects are permitted. Admission of observations such as ambient clocks/randomness and native services must respect the same boundary.
4. The entire nested call tree, including callbacks and selected overloads, must preserve these conditions. Passing callables internally does not itself violate the contract.
5. Results retain the existing owner/access/lifetime obligations. Internal mutation does not authorize returning a reference into a destroyed private region, extending a captured owner or introducing regional copy/move.

Conceptually, the invocation maps an input-state snapshot to an immediate result while leaving pre-existing external state unchanged. “Snapshot” denotes this observation boundary, not a copied graph or a new runtime storage mechanism. A proof requires the input state to remain stable during the invocation; shared-memory/concurrent mutation needs a separately established stability contract.

This boundary permits ST-like private mutable construction. It does not require all local bindings to be const or every private object to be readonly. The checking mechanism must distinguish private mutable state from external readonly state throughout nested calls. Whether this uses explicit const callable types, contextual checking of ordinary helpers, or a bounded combination remains open.

## Immediate results and later observations

The initial discussion covers non-callable results: val results and ref/ptr results. A returned reference selects a target at the return point; it does not freeze that target. Later mutation and dereferencing occur outside this invocation's purity boundary. A fresh result may also become mutable afterward without retroactively violating the invocation's contract.

For example, a const selection operation can return a readonly view of one node in an input graph. The caller may subsequently modify that node through another permitted alias. That later read/write is a separate observation, not evidence that the earlier selection mutated its input.

Consequently, this plan does not authorize memoization by pointer identity, moving calls across input mutations, or permanent immutability assumptions. It does not define the purity of a later invocation of an escaped callable; returned callable results remain outside this first research scope. Internal callable use must still preserve the outer call boundary.

A persistent-data library can retain earlier logical versions by leaving their published nodes unchanged. This stronger library property is distinct from readonly access during one call. Sharing nodes across owners remains subject to the regional-reference contract; no cross-region sharing mechanism is chosen here.

## Private construction and escape

Private region construction serves useful algorithms without exposing their temporary updates. The Haskell ST analogy concerns encapsulation of state; it does not import Haskell's exact type syntax or compiler machinery. An output must use an already legal surviving owner/result path.

Two construction cases remain distinct:

- A newly created private region can be mutated internally and released, or returned through an authorized owning-result contract if that result form is included in a later scope.
- Allocating nodes in a caller-provided region can change pre-existing allocator state and observable occupancy. Such allocation does not automatically qualify as private construction merely because its new nodes were previously unreachable. Any exception or explicit output-construction capability needs a separate decision.

The immediate non-callable result discussion does not settle whole-region result syntax, freezing, publication or ownership transfer. Existing valid borrow/transfer rules continue to apply; no new escape channel is inferred.

## TCO and the callable ABI

TCO concerns growth of execution frames, not growth of result data or captured environments. Repeatedly wrapping a previous callable by value can grow its environment even if the calls use bounded stack space.

The current callable direction is compatible in principle:

- Native callable entries/adapters make call boundaries representable independently of source inlining.
- A forwarding call can pass an incoming caller-owned result destination onward. A temporary destination allocated in the departing frame does not have the same lifetime.
- Complete callable-value construction avoids pointers into an expired factory environment. Borrowed regions retain their own lifetime obligations.
- Lexical lifetime ownership does not mandate physical stack backing. Scope-owned heap backing is not inherently incompatible with future TCO, but cleanup must remain correct.

Const-function eligibility still depends on the particular call. Pending catch/finally/defer behavior, private-region release or an argument borrowing the departing frame may require that frame or an equivalent obligation to remain. Readonly external access alone does not prove these obligations removable.

Research should start with self-tail recursion, then evaluate compatible mutual and indirect calls within the const-function subset. The exact guarantee, treatment of ineligible tail positions and supported ABI signatures are future decisions. Compiler adapters and exception plumbing must be included in bounded-stack evidence; optimizing only one visible call edge is insufficient.

## Compatibility review during current work

A material conflict must be explained to a human before the incompatible choice is activated. Identify the requirement, concrete conflicting behavior, alternatives and cost; continue unrelated qualified work. In particular, review choices that would:

- Permanently equate function calls with unbounded source expansion, or prevent representable recursive call cycles.
- Force callable/result environments to borrow a frame that must already have ended, or hide owner/cleanup obligations needed for later tail-call reasoning.
- Make readonly external access impossible to preserve through aliases, callback calls, imported methods or native services.
- Commit to observable writes to pre-existing state as an unavoidable part of every supposedly boundary-pure call.
- Require broad semantic changes merely to distinguish callable-body constness from const storage/access or a const result.

An adapter without TCO today, a conservative rejected optimization, or remaining implementation work is not itself a material conflict. No current ABI is required to satisfy LLVM musttail universally. No concrete irreconcilable conflict was established during this planning review; this is not a proof of compatibility for unfinished implementation choices.

D-16 review: no fraction syntax, representation, promotion, overflow behavior, formatting, UoM identity or transfer rule changes. A future const checker must examine the selected numeric/conversion method's behavior rather than assume purity from its operator spelling. Tail-call lowering must preserve evaluation/conversion order and numerical failure semantics.

## Decisions and evidence for the future increment

Before implementation, settle only what the selected increment needs:

- Callable-type syntax and its distinction from const bindings, access views and result qualifiers; the declaration keyword is pure.
- Checking of private state, helper calls, callbacks, overload dispatch and native services, including input stability assumptions.
- Supported output construction/escape forms and exception/resource-failure behavior; no implicit rollback or ignored cleanup.
- TCO eligibility, guarantee and diagnostics, including generic/mutual/indirect recursion and ABI restrictions.

Focused acceptance should demonstrate external-state preservation, allowed private mutation, immediate-result semantics and rejected dangling output. Tail recursion should execute far beyond ordinary call-stack capacity with measured bounded frame use, while preserving failures, cleanup and results. Add mutual/indirect cases only when they exercise supported mechanisms. Persistent-data experiments should assess version preservation and allocation cost separately from call-stack use.

Implementation remains deferred until explicitly scheduled. These are proposed acceptance observations, not a request to create a large test matrix now.

## Technical references

- [Control.Monad.ST](https://hackage-content.haskell.org/package/base-4.20.2.0/docs/Control-Monad-ST.html): private destructive updates with an encapsulated state parameter.
- [Data.Array.ST](https://hackage-content.haskell.org/package/array-0.5.8.0/docs/Data-Array-ST.html): mutable construction can yield an immutable array through a safe boundary without an obligatory array copy. CoD has not selected this particular publication mechanism.
- [LLVM call instruction](https://llvm.org/docs/LangRef.html#call-instruction): tail versus musttail, ABI restrictions and the departing-frame lifetime condition. Backend mechanisms are implementation references, not CoD semantic authority.
