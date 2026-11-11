# Regional variables and callable captures

Design revision: 13 · Recorded: 2026-11-11 · Status: current bounded prototype contract.

## Design authority

- **Registry and scope:** [D-18](DesignAuthorityRegistry.md#d-18), all sections. H-327–H-335 confirms ordinary aliases, inferred/checked captures, named actual owners and borrowed owner projection; H-330/H-333 authorizes integration and runtime-selection repair. [H-336](DesignAuthorityRegistry.md#h-336)/[K-459](DesignAuthorityRegistry.md#k-459) preserves these conclusions while retiring research material. Qualification remains scoped to these requirements.
- **Value path:** C-65/C-73/C-75 and P-294/P-295/P-297/P-298 make storage selection, access and lifetime visible in source, serving charter §§1–3/§8. P-293 covers the bounded structural-region work; it does not approve every possible region or closure representation.
- **Dependencies and invariants:** D-14 owns storage/access/lifetimes, D-10 distinguishes lexical and dynamic calls, and D-17 owns checked exceptions. D-01 relocation, D-15 module/TLS separation and D-16 numeric/fraction constraints remain unchanged. No lexical-cell promotion, hidden user heap, regional object copy/move, ownership extension or RTTI.
- **Acceptance:** Execute both selection paths, observable captured-slot writes/rebinding, named-owner adaptation and deep composition; reject invalid kind, owner, access, capture lists and lifetimes. Preserve native relocation and bounded region use. General stored/runtime-dispatched closure representations remain outside this contract.

## Structural regions

`region` declares a product of named fields. A consumer can require a compatible field subset; WPO retains the concrete region layout, including additional fields and their actual offsets. Field names, val/ref/ptr kinds and data/UoM identities remain significant. Structural compatibility neither creates an owner nor converts one actual region into another. A requirement is a lower bound, not an access view that hides extra concrete fields.

Unmodified/val fields store bits directly; ref/ptr fields store same-owner references. Capturing a val slot observes its current bits. Capturing a ref/ptr slot observes its current binding and later rebinding, with ordinary initialization and checked-NPE obligations. Region-level allocation/release remains intrinsic; free lists, pools and item reuse remain library policies. No physical allocator or relative-pointer encoding change follows.

## Ordinary aliases

```text
[val | ref | ptr] var name [: [const] Type] [in owner] -> lvalue
```

An alias selects existing storage once. It creates no holder, allocation, payload copy or ownership transfer. Omitted kind is inferred; an explicit prefix must match exactly. A ptr field remains ptr even when currently non-null. An optional type annotation checks the selected view without numeric conversion. `:const` is also valid without an explicit type.

`in owner` checks the storage's actual owner. `const` prevents both rebinding and target mutation through the alias; other writable aliases can still change the storage. Existing `=`, `<-`, `<=>` and `<->` rules apply through either name. Selecting an array element or field path fixes that storage location, rather than re-evaluating the selection later.

Aliases can name ordinary variables, regional fields and addressable array elements. Whole-region holders, TLS bindings, temporaries and non-addressable logical container elements do not supply this alias form. Selecting a field through a dynamic region resource does not alias its TLS binding. A lexical pointer into a region still has lexical binding storage and is ineligible for closure capture.

## Callable owners and captures

| Syntax | Meaning |
| --- | --- |
| `where {region state:Counter}` | Declare a formal regional owner and its required field shape. |
| `func()->const u64 with {region state}` | Associate a callable with that actual owner. |
| `func [ticks] () -> const u64 { ticks }` | Declare the complete body capture list. |
| `func () -> const u64 { ticks }` | Infer required body captures. |

```text
data Stream { id:u64; }
region Counter { ticks:u64; }
function reader(ref stream:Stream in state)
    where {region state:Counter}
    -> func()->const u64 with {region state} {
    val var ticks:u64 in state -> state.ticks;
    func [ticks] () -> const u64 { ticks }
}
```

The stream argument supplies its actual region owner; a separate region parameter is unnecessary. The alias selects a regional slot, so the callback can outlive the factory call while that region remains live. It does not retain the factory's lexical alias cell. Named and anonymous functions/procedures share these capture rules.

Omitted brackets infer required variables. An explicit list must include every required capture: missing entries error, unused entries warn. Every listed capture remains retained, even if unused. Its owner must appear in with before any unused warning is considered. Every with-listed owner must actually be captured; the list is not a conservative lifetime upper bound.

Captures may retain regional slots, borrowed regional access and statically known callables with valid dependencies. Ordinary lexical val/ref/ptr binding cells cannot be promoted or captured. Captures do not freeze regional contents or extend owner lifetimes. Functions remain independent of dynamic effects; procedure lexical captures remain separate from perform's captured symbolic values and behave/direct calls' current dynamic context.

Parentheses group callable types. `-> func()->T with {...} throws E` declares the returned callback's exception, while `-> (func()->T with {...}) throws E` declares the factory's exception. Unbound with names reject; with does not declare a new owner.

## Owner projection

`cb -> state` borrows the region named state in cb's visible with-type, retaining the captured permissions. A helper accepting `func()->const u64 with {region r}` projects `cb -> r`; the factory's former name state is not exposed through that parameter. Multiple names map explicitly to actual regions, independently of declaration or allocation order.

Projection permits ordinary region helpers and root-field access. It cannot consume ownership, obtain writable access from a readonly association or outlive the region. Each owner association is a logical environment slot; WPO may eliminate its physical storage when the actual region is statically fixed. No separate lookup service or RTTI is required.

## Runtime selection and limits

Callbacks with one statically known implementation, compatible capture kinds and the same actual owners/owner associations can select different captured slots at runtime. LLVM PHIs select regional slot addresses. Ref/ptr captures select the binding slot, not its current target; later rebinding and checked null reads remain observable. Nested callable captures compose through the same selection.

Different implementations, captured dynamic contexts and actual owner sets remain unsupported. General stored/persistent closures, standalone generic callable binders and contextual block callbacks require separate work. The double-brace scope/defer increment is not implemented by this contract.

## Examples and validation

- [Stream monitor](../tests/cod/language/closures/stream-monitor.cod) and [client](../tests/cod/language/closures/stream-monitor-use.cod): object-derived owners, counters, mean packet size, following versus pinned slots and null handling.
- [Runtime selection](../tests/cod/language/closures/closure-selection.cod): both branches, nested captures, pointer-slot rebinding, procedure calls and named owner projection.
- [Composition checks](../tests/cod/language/closures/composition_tests.py): 64 modules and 10,000 frame updates with bounded region occupation.

The implementation checkpoint passed 295/295 language checks and native relocation/cleanup/composition checks. Cleanup verification passes all 42 affected language checks and the composition checks. The active fixtures remain executable with `COD_TEST_TOOLCHAIN=build`; detailed research, superseded syntax and historical measurements are available at commit `f7d3b44`.
