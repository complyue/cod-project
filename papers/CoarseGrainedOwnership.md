# Coarse-Grained Ownership for Relocatable Object Graphs

## Abstract

A program can hold thousands of references into one graph even though releasing the graph is a single ownership decision. A persisted graph presents a second problem: a reader may map its bytes at a different virtual address. We study how these two facts interact in CoD. A checked reference designates a target in one actual owner. The compiler may share the information needed to locate that owner across references. Regional fields can encode same-region targets by relative displacement. We derive the relocation property for an internal edge and identify a separate safety obligation for references held outside the graph. We propose a lifetime proof and experiments that distinguish per-edge repair, address resolution and physical data movement. The full proof, compiler coverage and performance measurements remain open.

## Owner identity, target identity and representation

Consider a railway network. Many routes can reach the same station, and connections can form cycles. Opening a read-only snapshot at a new address should preserve those relationships without walking every route to repair its fields. Meanwhile, a program that closes one snapshot and opens tomorrow's snapshot must obtain fresh references. Reusing yesterday's mapping address tells the program nothing about which station an old reference would select.

CoD checks `in O` against one actual owner per reference. Many references can share that owner. A call such as the following establishes what information the compiler can share.

```cod
proc add_into(dst:u64 in O, src:const u64 in O) where {owner O} {
    ref again:const u64 in O = src;
    dst <- dst + again;
}
```

The three bindings need one account of the actual owner and enough information to select their respective targets. They still have different access permissions. A recursive call can instantiate the same formal name with a different owner. Conversely, several frames can forward one actual owner. Grouping evidence by the spelling `O`, or by an address that storage might later reuse, would fail one of those cases. The language specifies what a dereference must establish and leaves the placement of owner information to the implementation. [Shared owner information](../design/RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations)

## Two relocation arguments

Take a valid non-null regional edge. Let its field be at byte address *s* and its target at *t*. With a field-relative representation, the stored displacement is *d = t − s*. After a uniform displacement *k*, reading the same unmodified field yields *(s + k) + d = t + k*. This applies to each internal edge independently, including edges in a cycle and several edges reaching one target. It assumes a valid, layout-compatible image and representable displacements. A null sentinel is interpreted by its representation. Moving one node independently or copying one field's displacement to another slot falls outside the calculation. [Regional storage representations](../design/RegionalAndLexicalStorageRefactor.md)

For a reference outside the graph, let its semantic selection be the pair of actual owner and normalized target locator. A native address is one possible result of resolving that pair. A continuing lexical owner may acquire a new backing base. Resolving the same locator with its current base must reach the same target. By contrast, consuming region replacement retires the old owner. Even if the allocator returns the same address, access through an old borrow must be statically excluded. A reader opening a persisted graph similarly obtains its own live owner and selects targets from its roots. Processes may agree on graph topology without sharing a language-level owner identity. [Reference identity](../design/RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator) · [Regional ownership](../design/RegionalOwnership.md)

The proof obligation is to maintain owner liveness and a valid locator through each permitted borrow, call, transfer of ownership, backing change and release. An implementation that hoists the base across an operation that can change backing needs evidence that the cached address is still valid. An implementation that merely checks that the owner's *variable* is initialized after rebinding cannot justify an earlier borrow. Both mistakes can survive tests in which an allocator happens to reuse an address.

## What is already known

Cyclone gives regions a static typing discipline. Lattner and Adve connect pointers to pools statically, pass pool descriptors across calls and encode pointers as offsets. Boost.Interprocess documents field-relative offsets for data mapped at different addresses. These precedents cover important parts of the proposed representation. CoD's research question is which source-level lifetime and identity guarantees survive a lowering that shares owner evidence, and what those guarantees cost for durable, multiply referenced graphs. [Grossman et al., *Region-Based Memory Management in Cyclone*](https://doi.org/10.1145/512529.512563) · [Lattner and Adve, *Transparent Pointer Compression for Linked Data Structures*](https://llvm.org/pubs/2005-06-12-MSP-PointerComp.pdf) · [Boost.Interprocess offset pointers](https://www.boost.org/doc/libs/latest/doc/html/interprocess/offset_ptr.html)

## Evaluation

| Experiment | Evidence to collect |
| --- | --- |
| Map the same read-only image at two forced-distinct addresses. Include cycles and nodes with several incoming edges. | Compare root-relative reachability, values and shared-target relationships in each mapping. Count repair writes separately from image validation and page faults. |
| Force lexical backing growth between preparing and using a valid reference. Retire an owner and reuse its address in a separate negative program. | The first program must reach the original target. The type checker must exclude the stale use in the second, regardless of allocator behavior. |
| Vary references per owner independently of the number of owners. Include nested calls with the same and with distinct actual owners. | Inspect lowered owner arguments and retained metadata. Measure bytes per edge, bytes per active owner and traversal throughput against an owner descriptor stored with every reference and a pool-relative implementation. |

A raw-address implementation that repairs a writable image provides a baseline for relocation work. Time mapping, validation, first access and full traversal separately. Zero internal edge repairs is compatible with linear-time validation, copying the backing or page-in costs. External handles still require enough information to find their owners. Storage for target locators still grows with the number of edges. A publication claim would require the lifetime proof and evidence showing that these costs have been distinguished in practice.
