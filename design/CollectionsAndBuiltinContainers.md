# Collections and builtin containers

Design revision 8 · 2026-11-26 · Source design draft

## Design authority

At r8 [D-30-TYPED-SUBSCRIPTS](DesignAuthorityRegistry.md#d-30-typed-subscripts) supplies each axis to the library overload in source order, including ordinary unitless indices and nominal coordinates. P-766/P-767 preserve library array policy, receiver access, actual owners and UoM identity. [Decision and acceptance](../CHANGELOG.md#h-ranges-and-typed-subscripts-20261126)

At r7 [D-30-ARRAY-POLICY](DesignAuthorityRegistry.md#d-30-array-policy) assigns public array behavior and names to libraries. P-758/P-759 preserve the r6 owner, access and transfer rules at library and native boundaries. [Human direction and acceptance](JuliaPackagePortability.md#library-defined-arrays-and-the-native-c-boundary)

[H-DRAIN-HANDOFF-20261123](../CHANGELOG.md#h-drain-handoff-20261123) confirms incremental removal and suffix custody in [consuming traversal](#removal-order-and-failure) under [D-30](DesignAuthorityRegistry.md#d-30), P-589/P-590. Each successful handoff identifies the member whose owner changes. D-29-HEADERS governs release on overwrite or loop exit. Acceptance observes the first member between handoffs and checks the remaining members on early exit. The operation names, general conformance and owning-yield signatures remain proposals.

[H-ORDINARY-NAMES-20261119](../CHANGELOG.md#h-ordinary-names-20261119) covers the library fold and builtin array operations at r5 under [D-10-ORDINARY-NAMES](DesignAuthorityRegistry.md#d-10-ordinary-names).

- **Labelled visitation.** [H-GENERATOR-RECEPTION-20261119](../CHANGELOG.md#h-generator-reception-20261119) applies D-29-GENERALIZED-FOR to the r4 array example under P-590. Compare both headers after exchanging the order of the index and item declarations. Collection qualification and per-member owner proofs retain their existing scope.

- **Traversal UX.** [K-GENERALIZED-FOR-20261118](../CHANGELOG.md#k-generalized-for-20261118) relates the D-30r3 examples under P-589/P-590 to D-18-HERE-BREAK and D-29-LENDING. Charter §§4.3/8.1 motivate visible custody and early exit at the call site. Acceptance distinguishes skip, stop and take while preserving each actual member owner. The general interface and per-call owner proof retain their draft status.

- [D-30](DesignAuthorityRegistry.md#d-30) covers the CoD builtin container inventory and the proposed collection operations for read-only visitation, indexed removal and consuming traversal. The user placed these operations at collection scope. The names, method contracts and general owning loop header are proposals. [Scope and evidence](../CHANGELOG.md#k-collection-interfaces-20261118)
- Candidate P-589 derives from C-10/C-54/C-65. A caller can tell when it is inspecting an element and when it takes responsibility for that element. P-590 depends on CoD’s existing index domains, target access, owner and generator rules. [Charter §§3.3, 4.1, 4.3 and 8.1](RegionalSemanticPlatformCharter.md)
- The confirmed instance is the exception aggregate’s independently owned members and read-only enumeration. Its confirmation covers that instance. General collection conformance, operation names, storage layout, native lowering and ABI remain unqualified. [H-DEFER-MEMBER-TAKE-20261118](../CHANGELOG.md#h-defer-member-take-20261118)
- The visitor mutation rule applies to indirect calls. Acceptance checks owner identity after compaction. The wider visitor contract remains pending. [Engineering review](../CHANGELOG.md#k-collection-visitor-proof-20261118)

## Builtin containers and related forms

The named regional array below describes the current bootstrap representation. The bundled native interop vocabulary is explicitly selected as `CArray`; `julia.cod` can export `Array`. Each library specifies its own axes, layout and slice/view behavior. [Array policy and migration](JuliaPackagePortability.md#library-defined-arrays-and-the-native-c-boundary) · [Stepped subscripts](JuliaPackagePortability.md#stepped-subscripts)

| Form | Element and owner model | Design and source status |
| --- | --- | --- |
| Named regional array, e.g. `array Rates[u32(Neuron)]:f32;` | The declaration fixes the index domain and element type. An allocated array has a runtime length and an actual region owner. A view of an addressable element depends on that owner. | The seed fixture exercises the scalar array, length, indexed bits and a borrowed element using its current source spellings. A runtime length alone says nothing about support for insert, removal or growth. [Parser](../cod/lang_syntax.hh) · [Regional array fixture](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/region-arrays.cod) |
| `UnhandledDeferError.exceptions` | Ordered builtin collection with one independently owning `exception` holder per residual error. The aggregate owns the collection. Each element owns its own region. | D-17 confirms the specialized collection, read-only enumeration and single-member transfer. Generic method spellings and native implementation remain open. [Exception contract](ExceptionControl.md#region-and-members-of-unhandleddefererror) |
| Variadic parameter `rest:T...` | A call supplies one logical pack. The element passing mode and owner constraints come from the parameter. | D-10 owns argument expansion and forwarding. An array can supply the pack through `values...`. Pack indexing or expansion grants no structural mutation of the source. [Runtime-array expansion](SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter) |

Concrete map, set, list, queue and general resizable-sequence types remain open for CoD. A collection needs defined element modes, ownership, access, order, operations and failure behavior. Prefer a CoD library for collection policy. [Regional storage and library policy](RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy)

## Fold and array access

An ordinary library `fold` takes a synchronous `here` step as its first parameter. A half-open range overload can be written entirely in CoD.

```cod
func fold(here step(acc:u64, item:u64) -> u64,
          first:u64, past:u64, seed:u64) -> u64 {
    val acc <- seed;
    for (i in first .. past) { acc <- step(acc, i); };
    return acc;
}

val total <- fold(0u64, 4u64, 10u64) (acc, i) => { acc + i };
val empty <- fold(4u64, 4u64, 10u64) (acc, i) => { acc + i };
```

The results are 16 and 10. The written bounds and seed are evaluated once in that order. An empty input performs no step and still type-checks the supplied body. The step result must satisfy the accumulator contract, including applicable lossless conversions and owner/access evidence for a borrowed accumulator. An overload accepting a compatible producer visits in that producer’s order and honors its feedback contract. A single-brace step can `continue value` to deliver the next accumulator, `break value` to complete the fold call or `return value` from its lexical supplier. A double-brace callback has its own return target. All exits retain the applicable cleanup obligations.

For a writable `array Counts[u32]:u64` in a live owner, the builtin selection in `ref cell = counts[i]` borrows that element. After `counts[i] <- 9u64`, `cell` reads 9. In contrast, `val saved <- counts[i]` retains the bits read at reception. A readonly array supplies at most a readonly view. Preserve bounds checks, nominal index domains and once-only evaluation. A custom getter returning a val or a packed collection with only an indexed writer supplies no borrow of its internal storage. A library needing a specifically named borrow can publish an ordinary function returning `ref T in O`. The standard array element count comes from the visible `len` family.

## Multidimensional subscript overloads

Commas inside brackets separate positional arguments to **one** indexing operation. Scalar indices keep their ordinary argument types. A bounded range is one range value, irrespective of its length. An axis-relative selector preserves its supplied fields and indicates which bound was omitted. The two range forms preserve the endpoint policy, the typed bounds and the step. A library can inspect those values at the call without receiving an enumerated list, byte offsets or fabricated first/last coordinates. No public tuple, allocation or physical selector layout is required.

| Written use | Selected call |
| --- | --- |
| `m[r, c]` | `([])(m, r, c)` |
| `m[rows, c] <- value` | `([]<-)(m, rows, c, value)` |
| `m[r, c] = target` | `([]=)(m, r, c, target)` when the receiver supports reference binding |

For `m[1 ..= 6 by 2, 3]` the getter receives a range with start 1, written inclusive bound 6 and step 2, followed by the scalar 3. Its last visited row is 5. For `m[.. by -1, 3]` the first selector has two absent bounds and an explicit step of −1. The overload can distinguish a scalar, a bounded progression and an axis-relative selector, and use the coordinate and step types in method selection. The actual numbers and endpoint policy are available to the selected body. This contract leaves the public names and representation of selector types open.

`m[r][c]` performs two lookups and may have a different meaning. Each family uses ordinary visible overloads, arity, parameter modes and specificity. For example, a two-scalar getter and a range-plus-scalar getter can return an element and a column selection respectively. A rank mismatch with no applicable method is a normal call error. One composite index explicitly passed in brackets remains one argument. Scalar indices, a full-axis selector (`..`), and omitted ends (`lo ..`, `.. hi`, `.. by -1`) stay distinguishable. The library supplies an omitted bound from the corresponding axis and direction. Copies, borrowed views, bounds policy, result rank and memory strides follow the selected operation.

Prepare the receiver first, then each selector left to right, then any written RHS. Complete each argument's evaluation and required conversions before the next. A failure during preparation skips subsequent operands and method entry. An indexed writer receives the original receiver and selectors directly. Any check performed inside a user-defined setter runs at its ordinary point after argument preparation. Preserve the established destination-before-RHS checks of the builtin indexed transfer. The result, mutation and allowed access follow the chosen operation, including its owner and cleanup obligations.

### Unitless and unit-bearing axes

Ordinary integer axes accept unitless indices and ranges. A numerical matrix can expose `m[1, 2]` and `m[1 ..= 5 by 2, 2]` directly. A library declaring row and column units can instead provide a method taking `u32(Row)` and `u32(Column)`. For that method `grid[Row(1u32), Column(2u32)]` is valid and reversing the arguments fails. Comparing primitive widths or stripping a unit during overload selection would erase this distinction. Existing same-unit whole-type widening and conversions enabled by the formal parameter keep their established rules. A domain-specific library may separately publish a unitless ordinal overload and specify how an ordinal maps to its axis. The ordinary numerical profile needs no unit annotations at the call site. Bounds and index origin remain properties of the receiving array.

Range endpoints belong to compatible coordinate domains. The step is an advance, which can have a different representation or type from a coordinate. A discrete row domain can define `by 2` as two row advances. A physical coordinate domain can require a typed displacement. For data sampled every five milliseconds, an overload taking `.. by 2` as every second position visits samples ten milliseconds apart. A step of `ms(2u32)` asks for a two-millisecond advance and needs its own applicable coordinate operation. A dimensionless step does not automatically acquire a physical unit. A range with both bounds supplied keeps the same progression when it is used with another array. For an open selector such as `grid[.. by 2, c]`, a library may interpret the unitless step as every second position on that row axis. A user-defined shift or conversion must respect the original coordinate identity and operand effects. [Scalar units and index domains](ScalarUnitsAndIndexTypes.md#the-requirement)

Acceptance compares the ordered values observed by a custom getter and writer for a unitless matrix and a nominal grid. Cover a scalar versus a range, swapped units, open and negative steps, and a selector producer that fails before a later selector or the RHS. A numeric `1 ..= 6 by 2` visits 1, 3 and 5. An inclusive singleton at the largest coordinate visits it exactly once. Check a write observed through a borrowed strided view and an empty selection with no backing dereference.

## Proposed common operations

The first profile is an ordered, indexed collection. A collection supplies the operations its representation and access contract support. A read-only sequence can provide visitation. A collection with an appropriate index domain and structural write access can also provide removal. This draft leaves map keys, set traversal order and the conformance mechanism open.

| Proposed call | Effect on membership | What the caller receives |
| --- | --- | --- |
| `visit(xs) (index, item) => { ... }` | Visits the current elements in collection order. Membership stays intact, including when the callback throws. | The collection’s index and its declared read access to the element. For an owning element this is a readonly target view covered for the callback invocation. |
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

For this draft, `visit` and each observer invocation finish void. Assume `is_noise`, `report` and `requires_attention` accept a readonly member. The caller can express filtering and short-circuiting without adding a stop flag to the collection API:

```cod
visit(batch.exceptions) (index, member) => {
    if (is_noise(member)) { continue; } else {};
    report(index, member);
    if (requires_attention(member)) { break; } else {};
};
```

A skipped member stays in the batch. The first member requiring attention is reported, then the visit ends and the caller resumes after it. Later members are untouched. An empty batch invokes no observer. A search written in an enclosing bool-returning procedure could instead `return true;` on a match, and return false after a visit that finds none. Double braces remain available when the author wants a local callback return and ordinary callback exception handling.

For independent member regions A and B, the same observer can borrow A on its first invocation and B on its second. Each invocation needs its own actual owner proof. The proposed `visit` needs this even if the callback never exits early. Expressing this as a single ref-yield component with owner O would require O to denote both A and B during one generator invocation, which violates the fixed-owner rule.

For a generator-compatible visiting overload on a regional array, suppose each handoff supplies the labels `index` and `item`, with a readonly u64 item in the array’s owner. Both call sites select these labels explicitly. Swapping their order in the declared schema leaves each receiving operation unchanged. The second spelling uses the same spec in its parenthesized header. [Shared receiving header](RangedForAndGenerators.md#one-receiving-header-for-both-call-forms)

```cod
for { val index <-; ref item:const u64 =; } := visit(xs) { report(index, item); }
visit(xs) ({ val index <-; ref item:const u64 =; }) => { report(index, item); };
```

A visit relies on three facts. The collection remains available for traversal, the selected entry retains its release responsibility, and the observer may read the selected target while its actual owner is live. Each callback entry binds the owner of that particular target, including when one compiled callback body visits several regions. An index selects the entry at that step. Compaction can change a surviving element’s index while preserving its owner.

The structural mutation exclusion follows the collection wherever the callback can reach it. For example, suppose `remove_first()` can call `take_at` on the same collection through a second handle:

```cod
visit(batch.exceptions) (index, member) => {{
    remove_first();          # Reject while this collection is being visited.
    report(index, member);
}};
```

The same rule applies to a helper or a forwarded here body that can drain this collection. Removing from a provably distinct collection is compatible with this visit, subject to its ordinary access rules.

A borrowed generator yield requires one fixed owner for that output over the invocation. Per-call binding in a visitor preserves that rule. The generator contract separately governs a consuming loop’s body, early return and required cleanup. [Borrowed yields](RangedForAndGenerators.md#one-fixed-owner-per-borrowed-yield-component) · [For headers](RangedForAndGenerators.md#for-headers)

### Removal, order and failure

`take_at` evaluates the collection and index once, in written order. The index must belong to the collection’s declared domain. With bounds checking enabled, an out-of-range index raises `IndexOutOfBoundsError` before removal. In a densely indexed sequence the length decreases by one and later indices shift down. The surviving elements keep their relative order. A successful owning handoff preserves the selected target, its region and its access permissions. Ordinary consumption and cleanup apply even when a subsequent consumer throws. [Index domains](ScalarUnitsAndIndexTypes.md#the-requirement) · [Bounds policy](DesignAuthorityRegistry.md#d-17-bounds)

A consuming traversal (proposed name `drain`) requires structural write access. Each successful handoff removes exactly one member. An owning member passes to the receiver with its release obligation. A bits member supplies its independent value. Until the next handoff, the collection owns the entire unvisited suffix in its original order. Early exit by `break`, outward `return` or `throw` preserves that suffix. Failure while preparing the next member leaves it in the collection. The matching single-brace call commits the same removals. The draft interface excludes separate structural mutations during traversal.

For a batch containing A, B and C, a body that inspects A and breaks leaves B and C in the batch. Loop cleanup releases A if the control still owns it. If the body instead throws A itself, the exception propagates with its owner. If the body continues, the producer can observe that A is still live while preparing B. Overwriting the control with B releases A at that assignment. [Owning loop controls](RangedForAndGenerators.md#owning-loop-controls)

## First owning instance

`UnhandledDeferError.exceptions` supplies an ordered collection of independently owning `exception` elements of static type `Error`. A visit borrows each selected object under its member owner. Taking one hands off that owner and leaves the other members and the separate `body_exception` in the batch. The proposed owning loop header receives one independent member owner at each handoff. D-17 governs FIFO publication, member identity, wrapper chains and aggregate disposal. [Exception membership and CoD examples](ExceptionControl.md#enumerating-and-taking-deferred-failures)

## Acceptance boundary

Trace releases while draining three distinct owners. After the first body continues, the producer must observe zero releases of A between handoffs. On entry to the second body, A has been released once and B is live. Check exhaustion after the last member and failure before the second overwrite. Check early exit after A by break, outward return and throwing an independent exception. In each early-exit case B and C stay in order and loop cleanup releases A once. Throwing A itself must deliver that same live owner to the handler.

For an empty sequence, visit and drain make zero handoffs. For a three-element indexed sequence, taking the middle element delivers it once and leaves the first and last in order. Cover an invalid checked index, a throwing observer, and early exit after one draining handoff. In the owning instance, verify that a readonly collection supports observation, that a removed member survives disposal of the batch, and that sibling regions and the body failure remain live while the batch retains them. For members A, B and C from three distinct owners, take B, then visit A and C with the same callback body. C is now at the former index of B and the callback must read C under its own owner. On batch disposal A and C are released exactly once, and the holder of B retains responsibility for its later release. The aggregate’s internal holder array is a dedicated builtin. Ordinary regional user data remains subject to its existing element and relocatability restrictions, including the exclusion of callable views. Existing subscript read/write methods continue to have their declared result and access modes. [Logical indexed transfer](RegionalOwnership.md#logical-indexed-transfer)
