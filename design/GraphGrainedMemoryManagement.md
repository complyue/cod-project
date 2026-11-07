# CoD Regional Memory Vision and Implementation Tracker

Design revision 2 · 2026-11-06

## Design authority

- **Registry and scope** [D-13](DesignAuthorityRegistry.md#d-13), revision 2, all sections. The local graph boundary is confirmed. The full design and candidate value paths are `pending-human`. [K-293](DesignAuthorityRegistry.md#k-293) records the holding/transfer review requested by H-165; its exclusive-handoff lifecycle is a proposal.
- **Value lineage** P-138/P-139 propose that region-grained storage serves inspectable regional graphs (C-01) and efficient structured semantic units (C-05). P-140 records the confirmed language constraint on C-40. P-141/P-142 record dependencies on the regional representation and language contracts. [Charter](RegionalSemanticPlatformCharter.md) · [Regional types](MemoryRegionAndRegionalTypes.md) · [Language design](SymbolicDynamicScope.md)
- **Human baseline** H-99/H-100 govern user storage and captures. H-156 defines one direct-pointer graph per region. H-157 restricts inter-region relationships to the business level. H-151/H-152 supply the Actor model direction and make region holding and transfer a design priority. [Graph and language decisions](DesignAuthorityRegistry.md#h-156) · [Actor decisions](DesignAuthorityRegistry.md#h-151)
- **Invariants and acceptance** Keep every CoD pointer edge within its region and preserve whole-region lifetime and relocation. Track native evidence separately from the open construction, holding and transfer rules. [K-280](DesignAuthorityRegistry.md#k-280)

## One region per pointer graph

**Graph-Grained Memory Management** is CoD’s vision for managing memory at the granularity of a graph of objects connected by direct pointers. Each such graph occupies one region. Programs allocate and update individual objects inside it. The region is the unit of reclamation and whole-image relocation, and the candidate unit of ownership transfer. [Regional lifetime rules](MemoryRegionAndRegionalTypes.md#3-lifetime-rules)

A direct pointer is a traversable edge between objects in the same region. The regional representation can encode it as a relative offset, so moving the whole image preserves those edges without per-edge pointer repair. Remapping and transporting the bytes have their own costs. [Regional pointer representation](MemoryRegionAndRegionalTypes.md#regional_ptrt-offset-based-storage)

## Relationships between regions

Cross-region relationships exist in the business model. They are represented by values such as typed IDs or keys and interpreted by the program in its business context. CoD’s physical pointer types are confined to a single region. A procedure can work with multiple live regions through separate inputs or bindings, each with its own lifetime. [Language boundary](SymbolicDynamicScope.md#logical-relationships-between-regions)

For example, a `WorkPkg` for `Mainbox<WorkPkg>` could contain a task node, input buffers linked by direct pointers and a `CustomerId` value. The package region contains the task’s pointer graph. The receiving process can use the ID to look up a customer through its own business protocol. The customer’s region keeps its independent lifetime. [Actor design](SymbolicDynamicScope.md#actor-concurrency-direction)

## Implementation and evidence

| Area | Status and evidence |
| --- | --- |
| Allocation and lifetime | The native prototype allocates regional records and arrays and rejects references that would escape their owner’s scope. [Regional records](../tests/cod/language/region-records.cod) · [Lifetime rejection](../tests/cod/language/region-escape.cod) |
| Pointer boundary | The prototype rejects a field initialized with a pointer into another region. Independent inputs from two live regions retain their original owners, including references returned through pure calls. [Cross-region field rejection](../tests/cod/language/region-cross-pointer.cod) · [Independent owners](../tests/cod/language/region-capture.cod) |
| Whole-region relocation | The native harness copies the region image, releases the original, then checks sharing, a cycle and mutation through relocated references. [Relocation evidence](../tests/cod/language/regional-validation.json) |

The current encoding has an accepted temporary limitation. Assigning an object to its first pointer field can encode a displacement of zero that reads back as null. [Self-reference limitation](SymbolicDynamicScope.md#accepted-temporary-self-reference-limitation)

## Open design work

| Area | Acceptance target |
| --- | --- |
| Graph construction | Define when the one-graph invariant is checked during construction and mutation, including the treatment of temporarily disconnected allocations. Preserve supported sharing and cycles. |
| Logical relationships | Resolve a business key after independent region reload or relocation under an explicit identity and version policy. Define missing-target behavior in the business protocol. |
| Region holding and transfer | Define who can access and release a region during construction, send, queueing, receipt and disposal. For exclusive handoff, check that the sender’s aliases cannot be used after transfer, the receiver can consume the graph after the sender’s scope ends, and failed delivery has one release responsibility. |

The [dynamic bindings tracker](GlobalVariablesDoneRight.md) follows the execution-context side of the same actor example. The two trackers meet at the mailbox, where the receiver supplies the bindings and the region contract governs the package’s storage.

## Actor holding and transfer review

**Holding keeps storage alive; access permission determines who may read or mutate it.** A captured reference is not a holder. The current CoD compiler associates each reference with a scoped region owner and rejects use after that owner’s lifetime; its captures do not extend lifetime. The host `auto_region` is movable, but that alone supplies neither CoD alias invalidation nor a successful/rejected-send rule. Moving the host owner is distinct from moving or copying any regional object.

For the exclusive-handoff candidate, one region has one release responsibility at each stage. The table describes obligations, not a selected public API or an added runtime state field.

| Stage | Holder and access under the candidate |
| --- | --- |
| Constructing | Sender holds the region and may access its graph through live references. The declared payload root must belong to this region. |
| Admission rejected | Proposed rule: sender remains holder; its references remain usable. The mailbox has acquired no release responsibility. |
| Admission accepted / queued | Mailbox/runtime holds the region; sender loses access through ordinary values, dynamic bindings and captures. Sender scope exit must not free it. |
| Receiving | The receiver acquires holding and access. Queue removal must not release the payload before its handler finishes. |
| Completed or abandoned | The current holder releases the region exactly once or explicitly transfers it again. Queue closure and rejected delivery are different stages and must not share an ambiguous cleanup rule. |

Invalidation should be reasoned about for the region as a whole, since all direct pointer edges stay inside it under H-156/H-157. Revoking only the root variable leaves aliases to interior nodes usable. `perform` must not restore an old captured alias into validity; `effect` restoration must not resurrect sender access. If the same allocation later returns to the sender, address equality alone is insufficient to revalidate earlier references. A fresh receipt/access contract is needed; no generation counter, borrow syntax or runtime reference table is selected here.

Conditional admission is a compiler-design case, not just a queue return code. If a send succeeds only on one branch, the later join cannot unconditionally treat the sender as still owning the graph. Rejection that preserves the owner and rejection that returns a new owner have different source and checking consequences. Loops and forwarding must also account for which region remains held. These examples belong in the ownership contract before changing `Compiler::Region`, cleanup emission or accepted reference programs.

Physical relocation remains independent of holding transfer. Same-address-space handoff can pass a system owner and the root’s region-relative location without repairing internal edges. Copying an entire supported image can preserve its offsets but costs bytes and creates another lifetime; read-only sharing requires a shared lifetime and a stable-content rule. Neither copying nor sharing is implicitly included in exclusive transfer. A queue’s system envelope is not a user regional object and does not create a language-level cross-region pointer permission.

The proposed first milestone and decision table are in [Actor implementation readiness](GlobalVariablesDoneRight.md#actor-implementation-readiness). Existing allocator symmetry, region-only allocation, pointer encoding and H-146’s temporary first-field self-reference limitation remain unchanged. No actor ownership extension has been implemented or validated by the synchronous relocation evidence.
