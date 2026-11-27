# Collections and builtin containers

Design revision 18 · 2026-11-27 · Source design draft

## Design authority

At r18 the [user’s decision](../CHANGELOG.md#h-hash-width-and-pop-20261127) sets the [public hash result width](#public-hash-result) and narrows the r17 copy-to-retain advice to [map lookup](#container-pointer-results). D-30, P-589/P-590, continues the r17 nullable result and owner rules. Detached pop is proposed below.

At r17 the [container pointer result decision](../CHANGELOG.md#h-container-pointer-results-20261127) qualifies bootstrap [pop and lookup results](#container-pointer-results) under [D-30](DesignAuthorityRegistry.md#d-30), P-590/P-162. It continues the r16 owner-only reference lifetime rule and leaves storage reuse policy to the regional library work.

At r16 the [reference lifetime clarification](../CHANGELOG.md#h-ref-outlives-owner-20261127) applies [D-14](RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy), P-162, to [deque node reuse](#bootstrap-linked-deque). The r15 public collection scope continues.

At r15 the [bootstrap collection decision](../CHANGELOG.md#h-booter-source-ergonomics-20261127) adds the [linked deque](#bootstrap-linked-deque) to D-30 for the real booter workloads under [D-20-BOOTSTRAP](DesignAuthorityRegistry.md#d-20-bootstrap), P-642/P-643. The r14 indexing rules and qualifications on other collection interfaces continue.

At r14 [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families), P-778/P-779, applies the [call-form decision](../CHANGELOG.md#h-dispatch-by-call-form-20261127) to indexed read, indexed value transfer and indexed reference binding. Fixed methods precede variadic methods across the eligible receiver and free families. The r13 A/B outcomes and explicit expansion continue.

At r13 the [user’s A/B decision](../CHANGELOG.md#h-fixed-arity-oo-indexed-reads-20261127) revises [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families) for [fixed-arity OO indexed reads](#fixed-arity-oo-indexed-reads). P-778/P-779 continue the receiver, access, module view and explicit pack rules.

At r12 the [OO overview](DomainLanguageDesign.md#how-receiver-dispatch-works) places r11’s indexing order alongside other receiver operations. The [indexing vocabulary](#indexing-terminology) distinguishes all three bracket operations from named field accessors. [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families) continues P-778/P-779 and the two-world acceptance below.

At r11 the [user’s priority decision](../CHANGELOG.md#h-index-defaults-and-free-methods-20261126) completes [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families). P-778/P-779 continue r10 receiver identity, call-form eligibility and AOT coverage. [Abstract defaults and free indexing operators](#abstract-defaults-and-free-indexing-operators) cover shapes missing from the concrete receiver, including explicit expansion, ambiguity and ordinary module visibility.

At r10 [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families), P-778/P-779, gives each nominal receiver its own family for each indexing operation in the selected world. [Dispatch through an abstract receiver](#receiver-families-for-indexing) preserves r9 argument order, access, fixed and variadic selection, and library control of the axes. The [user decision](../CHANGELOG.md#h-receiver-index-families-20261126) covers indexing. Ordinary free functions keep their established families.

At r9 [D-30-TYPED-SUBSCRIPTS](DesignAuthorityRegistry.md#d-30-typed-subscripts) puts the assigned value or target before the positional indices in the method signature. P-766/P-767 continue for bracket evaluation order, units and owner. [Literal ADTs](RangedForAndGenerators.md#concrete-types-of-range-and-slice-literals) and [explicit expansion](SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter) determine which typed operands the library receives.

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

## Bootstrap linked deque

The first public generic linked collection for the booter is a deque parameterized by its element type, implemented in CoD with private nodes. Insertion and removal at either end serve FIFO queues and LIFO stacks. The bootstrap plan omits a separate public list. A second interface would need a caller with a distinct operation, such as splicing at a known interior position. The [booter roadmap](../applications/cod-booter/README.md#booter-source-ergonomics) sets seed acceptance and the order of the regional storage manager and growable vector.

### Container pointer results

Deque pop and map lookup return a nullable pointer. Empty or missing gives null. On success the pointer selects typed storage with its actual owner and access. A caller can handle either outcome by matching the result once in a [`case`](DomainLanguageDesign.md#case-expressions-and-branch-local-variables). Both operations follow the [ordinary reference lifetime rule](RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy).

**Map lookup** borrows the selected value. Later map operations may change the contents observed through that pointer. To retain the value observed at lookup, copy the payload before an operation that can change it. Copying the pointer retains the same target. For a reference-bearing item, constructing an independent graph requires an admitted [graph transfer](CallableViewsAndCaptures.md#copying-moving-and-graph-transfer).

**Deque pop** removes the item. The proposed detachment guarantee preserves the removed payload across subsequent deque operations. Push 7, pop it into `p`, then push and pop 13. While the owner lives, `p` still reads 7 unless someone explicitly writes the selected payload. A writable alias retains its access. The first implementation can leave the removed node in its region. The later intra-region memory manager and library work will establish reuse criteria.

## Public hash result

A key type’s public `hash` overload returns `UInt64` on every target. A map may use fewer bits for its bucket index or cached fingerprint. Colliding hashes require key equality to distinguish entries. The width decision leaves the hash algorithm and any cross-version persistence guarantee to their own contracts.

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

For a writable `array Counts[u32]:u64` in a live owner, the builtin selection in `ref cell = counts[i]` borrows that element. After `counts[i] <- 9u64`, `cell` reads 9. In contrast, `val saved <- counts[i]` retains the bits read at reception. A readonly array supplies at most a readonly view. Preserve bounds checks, nominal index domains and once-only evaluation. An indexed read with a by-value result grants no borrow of the element’s storage. A packed collection can provide indexed value transfer without exposing a borrow of its internal storage. A library needing a specifically named borrow can publish an ordinary function returning `ref T in O`. The standard array element count comes from the visible `len` family.

## Multidimensional subscript overloads

Commas inside brackets separate positional arguments to **one** indexing operation. Scalar indices keep their ordinary argument types. A bounded range is one range value, irrespective of its length. An axis-relative selector preserves its supplied fields and indicates which bound was omitted. The two range forms preserve the endpoint policy, the typed bounds and the step. A library can inspect those values at the call without receiving an enumerated list, byte offsets or fabricated first/last coordinates. No public tuple, allocation or physical selector layout is required.

### Indexing terminology

**Indexing** (下标访问) covers the three bracket operations. Name the operation when discussing an overload, a diagnostic or a dispatch decision.

| Operation | Written use | Selected call |
| --- | --- | --- |
| Indexed read (下标读取) | `m[r, c]` | `([])(m, r, c)` |
| Indexed value transfer (下标传值) | `m[rows, c] <- value` | `([]<-)(m, value, rows, c)` |
| Indexed reference binding (下标引用绑定) | `m[r, c] = target` | `([]=)(m, target, r, c)` |

An **indexed read method** implements `([])`. Its declared result determines whether the caller receives a value or can borrow a place. Reserve *field getter* and *field setter* for named field accessors such as a computed `x.size`.

For `m[1 ..= 6 by 2, 3]` the indexed read method receives a range with start 1, written inclusive bound 6 and step 2, followed by the scalar 3. Its last visited row is 5. For `m[.. by -1, 3]` the first selector has two absent bounds and an explicit step of −1. The overload can distinguish a scalar, a bounded progression and an axis-relative selector, and use the coordinate and step types in method selection. The actual numbers and endpoint policy are available to the selected body. The [literal type contract](RangedForAndGenerators.md#concrete-types-of-range-and-slice-literals) gives both selectors concrete ADT identities before overload selection.

`m[r][c]` performs two lookups and may have a different meaning. Each lookup follows the [receiver, abstract-default and free-family order](#abstract-defaults-and-free-indexing-operators). Within the applicable level, ordinary arity, parameter modes and specificity govern selection. For example, indexed read methods taking two scalars or a range and a scalar can return an element and a column selection respectively. A rank mismatch with no applicable method is a normal call error. One composite index explicitly passed in brackets remains one argument. Scalar indices, a full-axis selector (`..`), and omitted ends (`lo ..`, `.. hi`, `.. by -1`) stay distinguishable. The library supplies an omitted bound from the corresponding axis and direction. Copies, borrowed views, bounds policy, result rank and memory strides follow the selected operation.

Prepare the receiver first, then each selector left to right, then any written RHS. Complete each argument's evaluation and required conversions before the next. A failure during preparation skips subsequent operands and method entry. The selected method binds the prepared RHS immediately after the receiver, leaving a final index pack available. Thus `m[next_row(), next_col()] <- produce()` evaluates both selectors before `produce()`. The explicitly written call `([]<-)(m, produce(), next_row(), next_col())` prepares its actuals in its own written order. An explicit expansion in `m[axes...] <- value` supplies the index pack, including at a length for which a fixed indexed value transfer method exists. Unitless integers still bind integer formals directly. A pack mixing integers and selectors needs an element contract that admits both. Any check performed inside a user-defined indexed value transfer runs at its ordinary point after argument preparation. Preserve the established destination-before-RHS checks of the builtin indexed transfer. The result, mutation and allowed access follow the chosen operation, including its owner and cleanup obligations.

### Receiver families for indexing

A nominal collection type carries separate families for `([])` (read), `([]<-)` (value transfer) and `([]=)` (reference binding) wherever it provides those operations. The family’s identity is the receiver type and operation. Its visible methods belong to the module interpretation selected at the call site. A type synonym retains the original family. The builtin array operations keep their existing representation, bounds and ownership contracts.

An OO head such as `func Dense.([]<-)(value:Int64, idxs:Int64...) -> void` adds an indexed value transfer method for writable `Dense`. The receiver binds the usual fixed `self` with its actual owner. A library that explicitly imports the receiver type can publish an adapter adding a two-axis method to that same family. Consumers that select the extended type view see both methods. Another module can keep the original view. Family membership requires a binding to the receiver and permission to contribute. A coincidentally visible free operator spelling by itself supplies neither. The OO head makes the receiver explicit, including in an adapter. Bracket expressions keep the argument preparation order above. [OO receivers](DomainLanguageDesign.md#how-receiver-dispatch-works) · [Module publication](ModuleWorlds.md#merging-type-owned-method-families)

Suppose `Dense` and `Sparse` both declare conformance to `Matrix`. Each defines its own variadic indexed value transfer for integer indices. A selected adapter adds a fixed `(value, row, col)` method for `Dense`. In a generic writer whose receiver is declared as writable `Matrix`, the same two source calls have these outcomes:

| Expression in the generic body | Actual `Dense` | Actual `Sparse` |
| --- | --- | --- |
| `m[row, col] <- value` | Fixed `Dense.([]<-)` from the adapter | Variadic `Sparse.([]<-)` |
| `m[axes...] <- value`, with the same two indices | Variadic `Dense.([]<-)` | Variadic `Sparse.([]<-)` |

One receiver's variadic method never competes with another receiver's variadic method. Within the chosen family, ordinary tuple specificity and the explicit pack rule apply. Its methods may specialize an index type, a selector pattern or the value. An ambiguous concrete choice needs resolution in that family. Within a given arity group, an applicable concrete method takes priority. A shape missing on the concrete receiver can use an [abstract default or a free operator](#abstract-defaults-and-free-indexing-operators). An invocation failure propagates through ordinary cleanup. A read-only receiver keeps its access when its concrete type becomes known.

A caller can pass or forward a `Matrix` view without a cast or a user-written type switch. Its declared conformance and the selected world determine which concrete types the call can admit. Ahead-of-time compilation checks coverage of each demanded call, including its result, access, ownership and effects. When the actual type is already known the compiler can emit a direct call. Otherwise it preserves the receiver evidence and prepares local dispatch that selects the actual ADT's indexing behavior. A shared generic body can execute both rows of the table. A call known to select `Dense.([]<-)` needs that family’s incoming effects. When several receiver types or call shapes remain reachable, cover every family the call can actually select, including an abstract or free fallback. Apply the pre-2.0 incoming-symbol rule to each selected family. Concrete type evidence can discharge an unreachable alternative. [Family-wide incoming effects](SymbolicDynamicScope.md#family-wide-effect-requirements-before-20) [AOT coverage](AheadOfTimeExecutionPreparation.md#actual-type-dispatch-and-wpo-coverage)

Verify the same actual through concrete and abstract formals, both receiver types through one call site, and two module views with and without the adapter. In the original view the direct two-axis `Dense` call reaches its variadic body. A reachable type with no permitted implementation for the demanded write must fail coverage, and an unused conformer outside the admitted call need not supply it.

#### Abstract defaults and free indexing operators

For each bracket operation, ordinary indexing first considers applicable fixed methods, including those using declared defaults. If that group is empty it considers variadic methods. Within each group, use the following family order:

1. The actual concrete receiver’s OO family.
2. Visible OO defaults on abstractions to which that receiver has declared conformance, including inherited abstractions. Conformance makes an applicable default available automatically. A concrete author can specialize a fixed default by supplying a matching fixed method.
3. The unprefixed free operator family for that operation, resolved by ordinary name lookup in the call’s selected module interpretation.

Within each group, family priority precedes comparison of refinements across levels. For example, a free method that explicitly names `Dense` and the exact index type still yields to an applicable `Matrix` default. Within a level use the established applicability and unique-most-specific rules, including nominal conformance, access, units and typed patterns. The same inherited definition reached twice counts once. If two unrelated abstractions offer equally good defaults for the same arguments, a call reaching those defaults is ambiguous even if the free family has an exact match. A concrete implementation for that shape can resolve it.

Suppose `Dense` declares conformance to `Matrix` and supplies a two-index value transfer method. `Matrix` supplies a one-index default that translates a linear index into a row and column and calls the two-index operation. A library can also author methods in an unprefixed family, for example with the head `func ([]<-)(m:Dense in O, value:Int64, i:Int64) -> void where {const owner O}`. It can define a local free callable or explicitly import a free family to extend it. This declaration contributes to that free family. An OO head such as `func Dense.([]<-)(…)` contributes to the receiver family under its own extension grant. The free family follows ordinary aliases, shadowing and visibility in the selected world. Another package’s private or unselected family is outside lookup.

| Write to an actual `Dense`, also when passed as `Matrix` | Selected implementation |
| --- | --- |
| `m[row, col] <- v` | The concrete two-index value transfer. |
| `m[i] <- v`, with both the `Matrix` default and the exact free method above | The one-index `Matrix` default. Its delegated two-index write reaches `Dense`. |
| `m[label] <- v`, with a nominal label accepted only by a visible free method | The free indexed value transfer for the label. Without that family the demanded call lacks coverage. |

The written call form determines eligibility at every level. If `Dense` supplies only a variadic indexed value transfer for the call, a fixed abstract or visible free method can win. For `m[axes...] <- v`, only variadic implementations participate. A fixed concrete method therefore leaves room for an applicable variadic abstract default, then a visible free variadic implementation if the OO levels lack one. The one-variadic limit remains per family. [Fixed and expanded calls](SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter)

This ordering selects one implementation. Ambiguity at the winning level, a missing required effect, a failed argument conversion, a bounds failure or a throw from the selected body do not trigger another attempt. Use the same prepared receiver and argument values. Both bracket notation and the corresponding explicit indexing operator call keep their established written evaluation orders. Explicit `intrinsic` indexing retains its existing builtin contract. Check the three calls above in two worlds with different free-family visibility, explicit pack selection through a missing OO shape, and a throwing default with a free indexed value transfer whose observable write must be skipped.

#### Fixed-arity OO indexed reads

For an ordinary indexed read, prefer an applicable fixed method over a variadic one. A declared default can fill an omitted fixed slot. This applies across the concrete receiver, its storage ancestors and its declared abstractions. A fixed method still needs compatible index types and receiver access. Explicit expansion `g[axes...]` selects a variadic method even for a single element.

Suppose `ChildGrid extends BaseGrid` and declares conformance to `Matrix`. The method `func const Matrix.([])(i:Int64) -> Int64 { 2 }` supplies the one-index default. Compare two alternative definitions of the pack method:

| Pack method in the example | `g[7]` on an actual `ChildGrid` | `g[7, 8]` |
| --- | --- | --- |
| A. `func const BaseGrid.([])(axes:Int64...) -> Int64 { 1 }` | 2 | 1 |
| B. `func const ChildGrid.([])(axes:Int64...) -> Int64 { 1 }` | 2 | 1 |

A visible free one-index method returning 3 leaves A and B at 2. In C, remove the `Matrix` method from B. The child still offers its pack, and the free method now handles `g[7]` and returns 3. A free fixed method taking `row:Int64, col:Int64 { 0 }` also returns 3 for this call when it replaces the free one-index method. The default fills the column after selection. Writing `g[axes...]` for an actual one-element pack reaches the child’s variadic method and returns 1 in all three cases.

### Unitless and unit-bearing axes

Ordinary integer axes accept unitless indices and ranges. A numerical matrix can expose `m[1, 2]` and `m[1 ..= 5 by 2, 2]` directly. A library declaring row and column units can instead provide a method taking `u32(Row)` and `u32(Column)`. For that method `grid[Row(1u32), Column(2u32)]` is valid and reversing the arguments fails. Comparing primitive widths or stripping a unit during overload selection would erase this distinction. Existing same-unit whole-type widening and conversions enabled by the formal parameter keep their established rules. A domain-specific library may separately publish a unitless ordinal overload and specify how an ordinal maps to its axis. The ordinary numerical profile needs no unit annotations at the call site. Bounds and index origin remain properties of the receiving array.

Range endpoints belong to compatible coordinate domains. The step is an advance, which can have a different representation or type from a coordinate. A discrete row domain can define `by 2` as two row advances. A physical coordinate domain can require a typed displacement. For data sampled every five milliseconds, an overload taking `.. by 2` as every second position visits samples ten milliseconds apart. A step of `ms(2u32)` asks for a two-millisecond advance and needs its own applicable coordinate operation. A dimensionless step does not automatically acquire a physical unit. A range with both bounds supplied keeps the same progression when it is used with another array. For an open selector such as `grid[.. by 2, c]`, a library may interpret the unitless step as every second position on that row axis. A user-defined shift or conversion must respect the original coordinate identity and operand effects. [Scalar units and index domains](ScalarUnitsAndIndexTypes.md#the-requirement)

Acceptance compares the ordered values observed by user-defined indexed read and indexed value transfer methods for a unitless matrix and a nominal grid. Cover a scalar versus a range, swapped units, open and negative steps, and a selector producer that fails before a later selector or the RHS. A numeric `1 ..= 6 by 2` visits 1, 3 and 5. An inclusive singleton at the largest coordinate visits it exactly once. Check a write observed through a borrowed strided view and an empty selection with no backing dereference.

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

A visit passes the callback as the first parameter under the ordinary trailing-callback rules. Each callback call that borrows an owning element binds the actual owner of that element for that call. This permits one visit to inspect objects from several regions. A reference derived from the callback argument requires a live, statically provable owner at every later use. The proposed visitation contract excludes conflicting structural mutation during the traversal. Retaining a reference to an item adds no outlives check at a later mutation. A callback failure preserves completed user effects. [Trailing callback arguments](SymbolicDynamicScope.md#trailing-callback-arguments)

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
