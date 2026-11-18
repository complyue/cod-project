# Collections and builtin containers

Design revision 2 · 2026-11-18 · Source design draft

## Design authority

- [D-30](DesignAuthorityRegistry.md#d-30) covers the CoD builtin container inventory and the proposed collection operations for read-only visitation, indexed removal and consuming traversal. The user placed these operations at collection scope. The names, method contracts and general owning loop header are proposals. [Scope and evidence](../CHANGELOG.md#k-collection-interfaces-20261118)
- Candidate P-589 derives from C-10/C-54/C-65. A caller can tell when it is inspecting an element and when it takes responsibility for that element. P-590 depends on CoD’s existing index domains, target access, owner and generator rules. [Charter §§3.3, 4.1, 4.3 and 8.1](RegionalSemanticPlatformCharter.md)
- The confirmed instance is the exception aggregate’s independently owned members and read-only enumeration. Its confirmation covers that instance. General collection conformance, operation names, storage layout, native lowering and ABI remain unqualified. [H-DEFER-MEMBER-TAKE-20261118](../CHANGELOG.md#h-defer-member-take-20261118)
- Revision 2 makes the r1 visitor mutation rule explicit for indirect calls and gives acceptance cases for owner identity after compaction. The proposed operation contracts retain their pending status. [Engineering review](../CHANGELOG.md#k-collection-visitor-proof-20261118)

## Builtin containers and related forms

| Form | Element and owner model | Design and source status |
| --- | --- | --- |
| Named regional array, e.g. `array Rates[u32(Neuron)]:f32;` | The declaration fixes the index domain and element type. An allocated array has a runtime length and an actual region owner. A view of an addressable element depends on that owner. | The seed parser and source fixtures cover the illustrated scalar array, allocation, `len`, indexed access and `ref_at`. A runtime length alone says nothing about support for insert, removal or growth. [Parser](../cod/lang_syntax.hh) · [Regional array fixture](../tests/cod/language/region-arrays.cod) |
| `UnhandledDeferError.exceptions` | Ordered builtin collection with one independently owning `exception` holder per residual error. The aggregate owns the collection. Each element owns its own region. | D-17 confirms the specialized collection, read-only enumeration and single-member transfer. Generic method spellings and native implementation remain open. [Exception contract](ExceptionControl.md#region-and-members-of-unhandleddefererror) |
| Variadic parameter `rest:T...` | A call supplies one logical pack. The element passing mode and owner constraints come from the parameter. | D-10 owns argument expansion and forwarding. An array can supply the pack through `values...`. Pack indexing or expansion grants no structural mutation of the source. [Runtime-array expansion](SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter) |

Concrete map, set, list, queue and general resizable-sequence types remain open for CoD. The seed’s C++ support library already has regional containers with native allocation, iteration and serialization contracts. A CoD builtin needs its own element modes, owner rules, access, order, failure behavior and supported operations. [Native regional containers](MemoryRegionAndRegionalTypes.md#specialized-container-types) · [Native container exception safety](ExceptionSafety.md#container-exception-safety)

## Proposed common operations

The first profile is an ordered, indexed collection. A collection supplies the operations its representation and access contract support. A read-only sequence can provide visitation. A collection with an appropriate index domain and structural write access can also provide removal. This draft leaves map keys, set traversal order and the conformance mechanism open.

| Proposed call | Effect on membership | What the caller receives |
| --- | --- | --- |
| `visit(xs) {{=> index, item; ... }}` | Visits the current elements in collection order. Membership stays intact, including when the callback throws. | The collection’s index and its declared read access to the element. For an owning element this is a readonly target view covered for the callback invocation. |
| `take_at(xs, i)` | Removes the selected element from a writable collection. | An independent bits result for a bits element, or the original owning holder for an independently owned resource. |
| `drain(xs)` in a `for` header | Removes the next element at each handoff. Unvisited elements stay in the collection if execution leaves the loop early. | One result per iteration, under the same element-transfer contract as removal. |

For example, assume `samples` is a writable, resizable sequence of `u64` accepting the index `i`:

```cod
val sample:u64 <- take_at(samples, i);
for val remaining <- drain(samples) {
    record_sample(remaining);
}
```

### Observation and owner scope

A visit passes the callback as the first parameter under the ordinary trailing-callback rules. Each callback call that borrows an owning element binds the actual owner of that element for that call. This permits one visit to inspect objects from several regions. A reference derived from the callback argument requires a live, statically provable owner at every later use. Conflicting structural mutation during a visit rejects. A callback failure preserves completed user effects. [Trailing callback arguments](SymbolicDynamicScope.md#trailing-callback-arguments)

A visit relies on three facts. The collection remains available for traversal, the selected entry retains its release responsibility, and the observer may read the selected target while its actual owner is live. Each callback entry binds the owner of that particular target, including when one compiled callback body visits several regions. An index selects the entry at that step. Compaction can change a surviving element’s index while preserving its owner.

The structural mutation exclusion follows the collection wherever the callback can reach it. For example, suppose `remove_first()` can call `take_at` on the same collection through a second handle:

```cod
visit(batch.exceptions) {{=> index, member;
    remove_first();          # Reject while this collection is being visited.
    report(index, member);
}};
```

The same rule applies to a captured callable that can drain this collection. Removing from a provably distinct collection is compatible with this visit, subject to its ordinary access rules.

A borrowed generator yield requires one fixed owner for that output over the invocation. Per-call binding in a visitor preserves that rule. The generator contract separately governs a consuming loop’s body, early return and required cleanup. [Borrowed yields](RangedForAndGenerators.md#one-fixed-owner-per-borrowed-yield-component) · [For headers](RangedForAndGenerators.md#for-headers)

### Removal, order and failure

`take_at` evaluates the collection and index once, in written order. The index must belong to the collection’s declared domain. With bounds checking enabled, an out-of-range index raises `IndexOutOfBoundsError` before removal. In a densely indexed sequence the length decreases by one and later indices shift down. The surviving elements keep their relative order. A successful owning handoff preserves the selected target, its region and its access permissions. Ordinary consumption and cleanup apply even when a subsequent consumer throws. [Index domains](ScalarUnitsAndIndexTypes.md#the-requirement) · [Bounds policy](DesignAuthorityRegistry.md#d-17-bounds)

`drain` needs structural write access for the traversal. Each handoff commits one removal. The body can inspect, forward or release the received element. If the body returns or throws, the collection still holds the unvisited suffix and receives ordinary cleanup from its holder. Separate structural mutations during the drain reject. Ordered implementations can advance a head cursor. The contract specifies order and custody independently of compaction or complexity guarantees.

## First owning instance

`UnhandledDeferError.exceptions` supplies an ordered collection of independently owning `exception` elements of static type `Error`. A visit borrows each selected object under its member owner. Taking one hands off that owner and leaves the other members and the separate `body_exception` in the batch. A proposed owning loop header receives a new independent holder at each handoff. D-17 governs FIFO publication, member identity, wrapper chains and aggregate disposal. [Exception membership and CoD examples](ExceptionControl.md#enumerating-and-taking-deferred-failures)

## Acceptance boundary

For an empty sequence, visit and drain make zero handoffs. For a three-element indexed sequence, taking the middle element delivers it once and leaves the first and last in order. Cover an invalid checked index, a throwing observer, and early exit after one draining handoff. In the owning instance, verify that a readonly collection supports observation, that a removed member survives disposal of the batch, and that sibling regions and the body failure remain live while the batch retains them. For members A, B and C from three distinct owners, take B, then visit A and C with the same callback body. C is now at the former index of B and the callback must read C under its own owner. On batch disposal A and C are released exactly once, and the holder of B retains responsibility for its later release. The aggregate’s internal holder array is a dedicated builtin. Ordinary regional user data remains subject to its existing element and relocatability restrictions, including the exclusion of callable views. Existing subscript read/write methods continue to have their declared result and access modes. [Logical indexed transfer](RegionalOwnership.md#logical-indexed-transfer)
