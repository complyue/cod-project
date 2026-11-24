<a id="coarse-grained-ownership-for-relocatable-object-graphs"></a>

# When Can One Owner Account for a Graph of References?

## Abstract

A railway planner opens a read-only network snapshot containing shared stations and return routes.
Its queries keep several views into the same snapshot. Can a compiler share the evidence that identifies their
owner and still resolve every target correctly over the owner's lifetime? We investigate that question for CoD. We give an arithmetic argument for
relocating internal edges and a separate simulation argument for sharing owner information in a
restricted reference model. Two histories distinguish preserving a live owner across a backing
change from retiring an owner and reusing its address. We propose experiments that vary the
number of owners, the number of views and the duration for which each part of the graph is useful.
A region can remove redundant owner information while retaining many bytes past their last use.
The model gives sufficient conditions for correct sharing. A compiler proof must establish those
conditions for accepted programs. Measurements must determine which workloads benefit.

## Owner identity, target identity and representation

The planner uses a snapshot in three ways. It walks edges stored *inside* the graph, keeps query
references in a live activation, and eventually releases the snapshot. Each view designates a
target under one actual owner. A field-relative displacement can be sufficient to traverse an
internal edge because that edge and its target have the same owner. An external view must still
be interpretable at its point of use, including when another call has changed the current backing
of a live lexical owner. A persisted graph acquires a new live owner when a reader opens it.
Two readers can agree about every path in the graph and have separate lifetimes.

Consider a call with the following signature and body from the CoD storage design.

```cod
proc add_into(dst:u64 in O, src:const u64 in O) where {owner O} {
    ref again:const u64 in O = src;
    dst <- dst + again;
}
```

The caller supplies one actual owner for the two parameters. The callee can share the evidence for
resolving it. It still needs a selection for each target, read permission on the source, write
permission on the destination and initialized operands for the addition. The two selections can
coincide. A recursive invocation may bind the same formal name to another owner. The research
claim concerns sharing evidence wherever the static association is established. It implies
neither a particular pointer width nor one physical descriptor shared by every activation.
[Shared owner information](../design/RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations)
· [Reference identity](../design/RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator)

## Two relocation arguments

**Inside an image.** Let an edge field be at address $s$ and its target at $t$, in one validated
region with a representable non-null field-relative displacement $d=t-s$. After uniform
translation by $k$, the same bits in the translated field resolve to $(s+k)+d=t+k$.
The argument applies to each edge regardless of the traversal used to reach it. For example, put
a route at byte 24, a junction at 64, and two route fields at 32 and 40 that both point to the
junction. Their stored displacements are +32 and +24. A junction field at 72 can point back to
the route with displacement −48. Moving the whole image preserves both aliases and the cycle
without writing any of the three fields. Comparing displacement bits would incorrectly classify
the first two selections as different.

This argument assumes compatible layout, unchanged field and target offsets and a valid encoding
for each edge. A reserved null representation needs its own interpretation. Copying just the +32
field to byte 40 would select byte 72. An image translation and a graph edit therefore have
different preconditions. Validation of the incoming image, mapping pages, moving bytes and
reconstructing a graph in another owner each have their own cost.
[Storage operations](../design/RegionalAndLexicalStorageRefactor.md#operations-across-the-boundary)
· [Representation limits](../design/RegionalAndLexicalStorageRefactor.md#known-first-field-self-reference-quirk)

**Outside an image.** Consider a source model in which every live view carries an actual owner
$o$, a normalized target selection $l$, and the access facts needed for its permitted uses.
The selection can distinguish an enclosing object from a subobject at the same byte address.
For non-null views, the source identity observation compares $(o,l)$. The model of the lowering
stores or forwards a common owner context for views proven to share $o$, with each view retaining
its own selection and access facts. Expanding a lowered view recovers the source pair and those facts.
Resolution uses the owner's current backing. This relation permits erasure of evidence established
statically. It also permits another copy of an owner argument at a call boundary.
[Owner and locator identity](../design/RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator)

For the proof sketch, start from initialized views with matching owners, selectors, permissions
and contents in both stores. In the grouped store, each view refers to an owner context. Interpret
a view by substituting its context's actual owner for that reference. The proposed invariant is
that this substitution gives the source view at each observation. Here are the induction cases.

| Operation | What must agree before and after lowering |
| --- | --- |
| Bind or forward a view | Substitute the caller's actual owner, keep its target selection and propagate only the permitted access. Two owners with the same formal spelling in different activations remain distinct. |
| Select a destination, then evaluate an allocating right operand | Save the selected place once. If the right operand grows its owner's backing, resolve the saved selection through the current base before writing. Re-evaluating the selector could choose another target. |
| Compare two live views | Compare actual owners and normalized selections. Distinct stored displacements can designate the same target. A field and its enclosing object can have coincident addresses and distinct identities. |
| Read or write | Equal selections under the relation denote the same storage. Preserve initialization and access checks at that observation, including updates seen through other valid views. |
| End or replace an owner | Subsequent use of an old borrow is inadmissible. A fresh occupant of the address obtains its own identity. Ordinary local transfer of cleanup responsibility can preserve the original region and its eligible views. |

For any finite sequence of the operations above, if the implementation establishes each row's
preconditions, induction on the sequence preserves the source observations. The argument is
conditional on a sound static owner association and on the specified encoding and resolution
operations. Proving that a compiler actually establishes the relation across arbitrary control flow,
cleanup, captured views and native boundaries is the remaining soundness task. Nullable references
add an owner-qualified null case to the identity relation. Its separate observation should be
included before extending the proof to that fragment.
[Growth and replacement](../design/RegionalAndLexicalStorageRefactor.md#backing-growth-object-movement-and-owner-replacement)
· [Owner-qualified null](../design/RegionalAndLexicalStorageRefactor.md#null-uses-the-same-identity-relation)

Two small histories discriminate plausible incorrect implementations. In the first, select a
lexical destination, grow that *same* owner's backing during right-operand evaluation, and write.
The value must reach the initially selected target in the new backing. In the second, borrow from
a region, consume it in an operation that replaces the owner, and attempt to use the old borrow.
The checker must reject that use even on a path where an allocator would reuse the exact address.
Include the positive sibling in which an ordinary local handoff preserves the region and the
borrow remains permitted. A checker that rejects both programs has not demonstrated the intended
distinction. [Prepared places](../design/RegionalAndLexicalStorageRefactor.md#selected-addresses-across-allocation)
· [Consuming replacement](../design/RegionalAndLexicalStorageRefactor.md#explicit-resize-consumes-and-replaces-the-region)

## What is already known

Cyclone gives a region typing discipline for establishing lifetime safety. Lattner and Adve show
how a compiler can associate linked structures with pools, pass pool information and compress
pointers. Boost.Interprocess provides field-relative pointers for objects mapped at different
addresses. These are three independent baselines for lifetime, metadata and relocation. A
pool-aware implementation with the same checks may meet the model here in full. In that event the
contribution to investigate is which CoD source declarations establish the necessary evidence,
where it survives forwarding, recursion and backing changes, and how accurately the compiler uses it.
A smaller native pointer or a successful mapping alone cannot distinguish those possibilities.
[Cyclone region typing](https://doi.org/10.1145/512529.512563)
· [Transparent Pointer Compression for Linked Data Structures](https://llvm.org/pubs/2005-06-12-MSP-PointerComp.pdf)
· [Boost.Interprocess relative pointers](https://www.boost.org/doc/libs/1_81_0/doc/html/interprocess/offset_ptr.html)

## Evaluation

**Correctness oracle.** Generate legal small image graphs from symbolic node and field identities.
Include a diamond, a cycle, an isolated root and equal-valued distinct nodes. Encode them once,
map the same bytes read-only at two distinct addresses, and compare all reachable labeled edges
and identity pairs against the generator's graph. Comparing only a checksum of reachable values
would miss the replacement of a shared node with two equal copies. Use the two histories above
and recursive calls with distinct actual owners as compiler cases. Record the expected check or
exact target and value *before* running either lowering. A different alias relation, an accepted
stale borrow, a rejected permitted handoff or a write to old backing is a correctness failure,
regardless of timings. Attribute compiler observations to a revision and record the lowering it
actually used. A run that happened to reuse an address cannot establish that a borrow was valid.
A compiler that lacks the growth path supplies no measurement for that row.

**Separate two costs.** First hold the image encoding and topology fixed. Compare a checked
reference carrying its own owner information with grouped owner contexts and a per-view locator.
Use the same access checks, locator width, optimizer settings and workload in both. Let $V$
be the number of simultaneously live external views, $G$ the owner-context occurrences that
actually survive optimization in those activations, and $W$ the bytes per retained owner
argument. If the per-view baseline materializes $V$ equal-width owner arguments and the grouped version
materializes $G$, the difference attributable to those arguments is $(V-G)W$ bytes. Locators, padding, spills, duplicated call arguments and any lookup
tables must still be charged. Sweep views per owner and independently vary call depth. Inspect
optimized code and report peak live metadata, allocations, resolved-base loads and traversal time.
A compiler that eliminates the same fields from both variants eliminates the predicted advantage.

Next hold the source checks and view representation fixed. Compare a read-only field-relative
image, an owner-base-relative offset image of matched width, and an address-based image that
requires fixup when mapped elsewhere. Verify identical application graphs. Time image validation,
mapping, first touch and full traversal separately. Count bytes read, bytes copied and edge fields
rewritten. Report warm and cold conditions, image size, edge density, host, compiler revision,
options and repetition count. Alternate run order within each graph and cache condition and give
paired differences with uncertainty across independent runs. Base-relative encoding can also yield zero internal
repairs. A faster read-only mapping against a repairing baseline would support a representation
choice, while saying little about the benefit of grouping external owner information.

**Price the lifetime unit.** Run one workload that releases all routes together and another that
keeps a small selection useful across many subsequent revisions. For the second, compare retaining
one whole owner with a layout or explicit copy that gives the retained subset an independent
lifetime. Keep result values, identities required by the application and lookup work comparable.
If preserving identity rules out a copy, report that limitation. Track the integral of allocated
backing capacity over time alongside resident memory, copy costs and release work. For example, a
64 MiB region kept for 60 additional seconds to serve a 4 KiB selection accounts for 3,840 MiB·s
of retained capacity in that interval. This is arithmetic for a hypothetical workload, not a
measurement of resident pages. It is the actual holder that must keep the owner live.

The three comparisons answer different questions. A valid graph can need zero edge repairs yet
cost a full scan to validate. External owner sharing can save metadata yet increase traversal
time. A region that matches a snapshot's lifetime can become expensive when one small result must
survive the rest. Reporting those outcomes independently would show where this ownership discipline
pays for itself, including workloads in which a simpler representation performs equally well.
