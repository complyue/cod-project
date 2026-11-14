# Regional variables and callable captures

Design revision: 91 · Recorded: 2026-11-14 · Status: current bounded prototype contract.

## Design authority

- **Registry / current scope:** [D-18](DesignAuthorityRegistry.md#d-18); the semantic revision stated above and the independently qualified scopes in the current registry. **Confirmed/continued only for recorded capture/value/native-boundary scope.** H-327–H-335/H-337–H-344/H-349/H-350; C-65/C-73/C-75/P-293–P-303. H-342 replaces callable alias initialization; H-344 permits runtime-sized scope-owned backing. H-417 requires statically unique ref/ptr owners through selected calls and captures; K-UNIQUE-OWNER-20261114 integrates that scoped constraint while preserving the established complete-value/native contract.
- **Value path:** C-65/C-73/C-75 and P-294/P-295/P-297/P-298 make storage selection, access and lifetime visible in source, serving charter §§1–3/§8. P-299/P-300/P-301/P-302/P-303 connects the runtime callable boundary and caller placement to explicit storage lifetimes and relocatable regional data. C-65/P-258 requires payload reads to have definite prior initialization, independently of storage naming or invocation form. P-293 covers the bounded structural-region work; it does not approve every possible region or closure representation.
- **Contract boundary:** Preserve actual borrowed owners, complete nested values, recursion, caller destinations, access and cleanup; measure allocation/compilation. No regional callable payload, lexical-cell promotion, independent user heap lifetime or captured-region retention. H-417 rejects ref/ptr results without one statically provable owner; D18-Q-result-discrimination representation remains pending; native fixtures do not establish D-21’s wider stage/family contract.
- **Invariants / non-goals:** D-14 owns storage/access/lifetimes, D-10 distinguishes lexical and dynamic calls, and D-17 owns checked exceptions. D-01 relocation, D-15 module/TLS separation and D-16 numeric/fraction constraints remain unchanged. No lexical-cell promotion, independently managed user heap objects, regional object copy/move, ownership extension or RTTI. Lexical lifetime ownership does not require stack backing.
- **Acceptance:** Exercise both selection paths, captured-slot writes, owner adaptation, complete nested values, nullable invocation and deep recursion; reject invalid kind, owner, access and lifetime. Preserve caller-owned results, independent copies, cleanup and relocation, and measure construction/invocation cost and compilation scale. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-UNIQUE-OWNER-20261114](../CHANGELOG.md#k-unique-owner-20261114) covers r90→91: H-417 closes only reference-owner alternatives, preserving independently qualified complete values, capture/access, recursion, native coverage and cleanup. Abstract concrete-type evidence representation retains its own review scope.

## Structural regions

`region` declares a product of named fields. A consumer can require a compatible field subset; WPO retains the concrete region layout, including additional fields and their actual offsets. Field names, val/ref/ptr kinds and data/UoM identities remain significant. Structural compatibility neither creates an owner nor converts one actual region into another. A requirement is a lower bound, not an access view that hides extra concrete fields.

Unmodified/val fields store bits directly; ref/ptr fields store same-owner references. Capturing a val slot observes its current bits. Capturing a ref/ptr slot observes its current binding and later rebinding, with ordinary initialization and checked-NPE obligations. Region-level allocation/release remains intrinsic; free lists, pools and item reuse remain library policies. No physical allocator or relative-pointer encoding change follows.

## Callable completion

H-361/H-362 and [D-17](ExceptionControl.md) require explicit `return` for values from proc/func brace bodies. Return from deferred catch/finally targets the enclosing callable, replaces a pending completion and continues remaining cleanup. Missing a required return is unchecked, with static diagnostics where possible; void callables may fall through. H-363 records pure declarations with implicit body results while deferring purity implementation. Implementation availability for explicit returns, missing-return delivery and deferred execution is maintained in the source capability reference. These source changes preserve complete values, caller result storage and owner/access checks.

## Ordinary aliases

```text
[const] [val | ref | ptr] var name [: [const] Type] [in owner] -> lvalue
```

An alias selects existing storage once. It creates no holder, allocation, payload copy or ownership transfer. Omitted kind is inferred; an explicit prefix must match exactly. A ptr field remains ptr even when currently non-null. An optional type annotation checks the selected view without numeric conversion. `:const` is also valid without an explicit type.

`in owner` checks the storage's actual owner. Declaration-leading `const` restricts the selected binding/storage through the alias; type-leading `const` restricts its target and every derived access path. These dimensions are independent under D-14; other writable aliases can still change the storage. Existing `=`, `<-`, `<=>` and `<->` rules apply through either name. Selecting an array element or field path fixes that storage location, rather than re-evaluating the selection later.

Aliases can name ordinary variables, regional fields and addressable array elements. Whole-region holders, TLS bindings, temporaries and non-addressable logical container elements do not supply this alias form. Selecting a field through a dynamic region resource does not alias its TLS binding. A lexical pointer into a region still has lexical binding storage and is ineligible for closure capture.

## Callable owners and captures

| Syntax | Meaning |
| --- | --- |
| `where {region state:Counter}` | Declare a formal regional owner and its required field shape. |
| `func()->const u64 with {region state}` | Associate a callable with that actual owner. |
| `func [ticks] () -> const u64 { return ticks; }` | Declare the complete body capture list. |
| `func () -> const u64 { return ticks; }` | Infer required body captures. |

```text
data Stream { id:u64; }
region Counter { ticks:u64; }
function reader(ref stream:Stream in state)
    where {region state:Counter}
    -> func()->const u64 with {region state} {
    val var ticks:u64 in state -> state.ticks;
    return func [ticks] () -> const u64 { return ticks; };
}
```

The stream argument supplies its actual region owner; a separate region parameter is unnecessary. The alias selects a regional slot, so the callback can outlive the factory call while that region remains live. It does not retain the factory's lexical alias cell. Named and anonymous functions/procedures share these capture rules.

Omitted brackets infer required variables. An explicit list must include every required capture: missing entries error, unused entries warn. Every listed capture remains retained, even if unused. Its owner must appear in with before any unused warning is considered. Every with-listed owner must actually be captured; the list is not a conservative lifetime upper bound.

Captures may retain regional slots, borrowed regional access and statically known callables with valid dependencies. Ordinary lexical val/ref/ptr binding cells cannot be promoted or captured. Captures do not freeze regional contents or extend owner lifetimes. Functions remain independent of dynamic effects; procedure lexical captures remain separate from perform's captured symbolic values and behave/direct calls' current dynamic context.

Parentheses group callable types. `-> func()->T with {...} throws E` declares the returned callback's exception, while `-> (func()->T with {...}) throws E` declares the factory's exception. Unbound with names reject; with does not declare a new owner.

## Owner projection

`cb -> state` borrows the region named state in cb's visible with-type, retaining the captured permissions. A helper accepting `func()->const u64 with {region r}` projects `cb -> r`; the factory's former name state is not exposed through that parameter. Multiple names map explicitly to actual regions, independently of declaration or allocation order.

Projection permits ordinary region helpers and root-field access. It cannot consume ownership, obtain writable access from a readonly association or outlive the region. Each owner association is a logical environment slot; WPO may eliminate its physical storage when the actual region is statically fixed. No separate lookup service or RTTI is required.

## Runtime callable contract

A callable view preserves its selected family dispatcher, module interpretation, view contract and required runtime environment. Several methods are permitted; actual argument types choose the applicable method. Existing native body-selection adapters are implementation infrastructure, not proof of family dispatch across abstract argument/result types. Direct method selection is valid when equivalent for all admitted alternatives. The family architecture and WPO coverage are tracked in [D-21](AheadOfTimeExecutionPreparation.md).

Module method registration constructs semantic information and does not capture runtime TLS. Runtime callable materialization creates the complete environment; reinstallation retains the existing value/captures. Lexical slots, named retained owners, nullable views, copies/replacement and captured/current invocation retain their qualified constraints. Per-family capture construction and abstract evidence must satisfy those constraints rather than introducing implicit owner extension or regional callable storage.

Method calls and callable-value calls separate operand preparation from invocation. Preparation retains the selected code, declared passing modes and ordered conversions. Invocation revalidates the prepared views and supplies the current procedure environment, or the explicitly requested captured environment. Symbolic builtins use the same boundary; their observable action occurs only at invocation. Native callable target protection and prepared-value cleanup retain their existing failure behavior. Scope-owned deferred registration and dispatch remain implementation work.

Target access remains separate from holder binding permissions and pure behavior. Acceptance must cover independent alias permissions, readonly captures, const-holder use, copy-failure preservation and bounded invocation cost.

The ABI belongs to one statically linked CoD binary image. Compiler-generated typed adapters connect a callable view to its implementation and captured environment while preserving that implementation's module world. Native dependencies retain the system CRT contract. Cross-process code references, persisted callable views, dynamic code loading and general RTTI are outside this increment.

Callable view payloads and their environments have lexical lifetime owners. Their physical backing may be stack or heap; runtime-sized environments are supported. The owning scope must release its allocations on normal and exceptional exit. Allocation failure follows the existing unchecked OutOfMemory contract. They cannot appear in regional fields or arrays and are not bits payloads eligible for arbitrary transfer. Regions remain relocatable; a regional ADT and its methods provide the business-level equivalent of a functor without storing a native code address in regional data.

Non-null variables use func/proc and have val semantics; nullable references to callable payloads use ptr. Definitions establish callable identities/methods; callable variable declarations construct lexical values:

```text
func read = reader(state);
func copied = read;
copied <- another;
proc update = updater(state);
ptr optional:func()->const u64 with {region state} = null;
```

The initializer is evaluated once. A fresh callable result constructs its complete payload and environment into caller-provided lexical storage. At the internal ABI boundary, a callable-returning function/procedure receives a hidden destination pointer; a forwarding call may pass that destination onward. Merely returning a header pointing into an expired callee environment is invalid. Captured regions remain borrowed and retain their original lifetimes and access permissions.

Declaration and nested capture copy the complete selected callable value, including its environment. A wrapper retains an independent callable snapshot; replacing the original later does not change that snapshot. Regional slots and regions inside either environment remain borrowed, so their subsequent content changes stay observable. No regional object is copied and no lifetime is extended.

A default callable parameter has an independent callee-local target, following D-14's default parameter ownership. Its value is prepared at that argument's evaluation boundary, before later arguments can replace the source holder. An explicit `in O` parameter borrows external callable storage instead. A callable value result constructs an independent complete value; an explicitly borrowed result retains its declared external owner. Fresh unaliased images may transfer into their destination without copying. Allocation failure and later-argument failure must release prepared values without altering the source holder or extending captured-region lifetimes.

`destination <- source` replaces a compatible callable value in existing lexical storage, preserving type/owner checks. Ordinary `destination = source` cannot rebind a func/proc variable. A ptr instead refers to callable storage and uses ordinary nullable-reference rebinding/lifetime rules. Its invocation reads the currently stored callable value. typeof is compile-time only; isnull and checked NPE apply to nullable references.

The complete callable representation must remain valid after bits movement: it contains no pointers into its own prior storage or an expired factory frame. Nested callable environments are included by value. This is an intrinsic lexical value category, not a general bits type that may enter regional fields/arrays. The adapter computes environment access from the current payload address; a copied header pointing to a previous lexical environment does not satisfy this contract.

Direct/behave procedure calls use current dynamic bindings; perform installs the selected procedure's captured symbolic values for its execution scope. Native representation must not merge bodies, module contexts or captured snapshots merely because signatures match.

Recursive calls are required for both functions and procedures. A blanket recursion rejection is not an acceptable endpoint. Call checking must account for cycles in the demanded program; successful finite source expansion is not a substitute for native recursion. The deferred const-function/TCO plan in [D-19](ConstFunctionsAndTailCalls.md) does not restrict procedure recursion. Material conflicts with that plan require human review.

## LLVM compilation scale

Reasonable performance is a development requirement, including during the prototype stage. An MVP may narrow the implemented surface, but each implemented path must preserve the business contract and have defensible runtime and compilation costs. Measurements must distinguish construction and complete-value copying from invocation, expose allocation and cleanup behavior, and include shared/deep composition. Passing a convenient demonstration does not establish generality or scalability. No numeric performance budget or particular optimizer/allocator is selected by this requirement.

WPO determines the demanded program and its semantic contexts; it does not require one indivisible LLVM optimization unit. Native function boundaries must represent recursive cycles and shared dependencies without expanding the dynamic call tree into source copies. Reuse must preserve the selected module world, concrete signature, conformance information and captured context. A shared origin or matching signature alone is insufficient.

Code sharing must preserve selected semantic views, concrete types, actual owners, access, initialization, captures and completion. Reuse is an implementation optimization; it does not memoize runtime values or eliminate required effects. Current emitter organization and measured coverage belong in the source capability reference.

## Implementation status and review boundaries

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

H-417 closes the reference-owner alternative question: each ref/ptr needs one statically provable owner, inferred from initialization if omitted. A selected callable or returned-view join with unresolved distinct owners rejects; runtime selection of targets within one established owner remains permitted. This is per-reference uniqueness, not a requirement that every captured reference share one owner. D18-Q-result-discrimination remains a separate representation question. Complete values, checked captures, caller destinations, recursion, owner/access/lifetime proofs and cleanup are required within each qualified scope. Broader family, stage and coverage obligations belong to D-21/D-26; they do not approve a permanent callable ABI.

### D18-Q-result-discrimination — behavior confirmed; representation pending-human

An abstract result can hide different concrete types: callbacks returning `Entity in r` can return Node or Edge. H-375 requires subsequent calls to retain actual-type method selection; a one-concrete-result implementation limit cannot restrict the language. H-376 requires precompiled coverage of admitted alternatives.

Concrete evidence must survive parameters/results, stored and nullable references, TLS, callable environments and owner-preserving relocation, with valid result destinations and exceptional cleanup. H-396 requires concrete discriminators without selecting physical transport, a reflection catalog or a regional object-header tag. Returning only an address and an abstract declared type is insufficient. D-21/D-26 owns coverage and execution preparation; H-417 independently requires statically unique reference owners. Different concrete types may share that one owner, so the owner decision does not settle concrete-type evidence transport.

## Examples and validation

The [callable fixtures](../tests/cod/language/closures/) specify composed acceptance scenarios: independent nested values, captures and current contexts, recursion, selected bodies, caller destinations, nullable views and cleanup. Current runnable coverage is maintained in the [source reference](../tests/cod/language/README.md); fixture presence alone establishes no implementation support.

Measure construction, copy and invocation costs, compilation scale, region release and failure recovery on the supported paths. Reject incompatible kind, owner, access and lifetime; do not infer whole-document qualification from passing examples.
