# CoD Regional Memory Vision and Implementation Tracker

Design revision 4 · 2026-11-06

## Design authority

- **Registry and scope** [D-13](DesignAuthorityRegistry.md#d-13), revision 4, all sections. The local graph boundary is confirmed. The full design and candidate value paths are `pending-human`. [K-293](DesignAuthorityRegistry.md#k-293) records the holding/transfer review requested by H-165; its exclusive-handoff lifecycle is a proposal.
- **Current review** [H-166–H-168/K-294](DesignAuthorityRegistry.md#h-166) cover process-lifetime module scopes, the effect-only holder premise and local-error-only send feedback. H-169 confirms ownership moves between effect bindings; [K-295](DesignAuthorityRegistry.md#k-295) records that synchronous implementation. The send/receipt proof remains design analysis. H-170 proposes returning the unsent region on local failure; sum syntax and binding-presence representation remain open.
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
| Allocation and lifetime | The native prototype allocates regional records and arrays, retains module-owned regions until normal process exit, moves ownership between effect bindings and rejects references that would escape their owner’s scope. [Regional records](../tests/cod/language/region-records.cod) · [Lifetime rejection](../tests/cod/language/region-escape.cod) |
| Pointer boundary | The prototype rejects a field initialized with a pointer into another region. Independent inputs from two live regions retain their original owners, including references returned through pure calls. [Cross-region field rejection](../tests/cod/language/region-cross-pointer.cod) · [Independent owners](../tests/cod/language/region-capture.cod) |
| Whole-region relocation | The native harness moves the effect-held owner, requires one non-null release, copies the region image, releases the original, then checks sharing, a cycle and mutation through relocated references. [Relocation evidence](../tests/cod/language/regional-validation.json) |

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
| Local send error | Under H-170’s returned-region proposal, the original binding is empty and the error result holds the unsent region until an effect binding acquires it or it is disposed. This reports no remote delivery. |
| Locally accepted / queued | The intrinsic/runtime holds the region; sender loses access through ordinary values, dynamic bindings and captures. Sender scope exit must not free it. No recipient delivery is promised. |
| Receiving | The receiver acquires holding and access. Queue removal must not release the payload before its handler finishes. |
| Completed or abandoned | The current holder releases the region exactly once or explicitly transfers it again. Local failure and later message disposal are different stages. Later non-delivery cannot restore an already-consumed sender reference. |

Invalidation should be reasoned about for the region as a whole, since all direct pointer edges stay inside it under H-156/H-157. Revoking only the root variable leaves aliases to interior nodes usable. `perform` must not restore an old captured alias into validity; `effect` restoration must not resurrect sender access. If the same allocation later returns to the sender, address equality alone is insufficient to revalidate earlier references. A fresh receipt/access contract is needed; no generation counter, borrow syntax or runtime reference table is selected here.

Conditional transfer is a compiler-design case, not just a local send return code. If a send succeeds only on one branch, the later join cannot unconditionally treat the sender as still owning the graph. Rejection that preserves the owner and rejection that returns a new owner have different source and checking consequences. Loops and forwarding must also account for which region remains held. The local transfer implementation checks branch joins and loop re-acquisition; send/receipt outcomes still need the result and access contract described below.

Physical relocation remains independent of holding transfer. Same-address-space handoff can pass a system owner and the root’s region-relative location without repairing internal edges. Copying an entire supported image can preserve its offsets but costs bytes and creates another lifetime; read-only sharing requires a shared lifetime and a stable-content rule. Neither copying nor sharing is implicitly included in exclusive transfer. A queue’s system envelope is not a user regional object and does not create a language-level cross-region pointer permission.

The proposed first milestone and decision table are in [Actor implementation readiness](GlobalVariablesDoneRight.md#actor-implementation-readiness). Existing allocator symmetry, region-only allocation, pointer encoding and H-146’s temporary first-field self-reference limitation remain unchanged. Synchronous local owner moves and module lifetime are validated; the relocation evidence does not establish Actor send, receipt or isolation.

## Effect-held region ownership

H-167 asks whether restricting all user-level whole-region holders to `effect @some_region = value` makes static ownership tracking practical. **Yes: H-169’s move rule makes the holder exclusive rather than freely copyable.** WPO can prove resource-state transitions along code paths; it need not enumerate every region allocated at runtime. Loops and receive operations can produce arbitrarily many instances while obeying the same checked transition rules.

Three kinds of information must stay separate:

| Kind | Meaning and proposed checking |
| --- | --- |
| Whole-region owning effect binding | Responsible for holding/releasing or transferring a region. Under H-169, `effect @b = @a` transfers the owner and empties `@a`. A local alias alone does not create another owner. An `effect` creates a new binding frame even when its destination has the same symbol identity as its source. |
| Interior regional reference | Non-owning access to an object, associated with its region instance. It may remain a function input or captured value, but using it requires that instance to remain accessible. It cannot prolong lifetime or manufacture a second whole-region owner. |
| Intrinsic runtime holder | Holds a message between consumption and receipt/disposal, when no user effect binding owns it. This belongs to system runtime machinery and does not expose a second user storage carrier. Receipt establishes an eligible effect binding. |

Effect-only placement alone is insufficient if two bindings can independently own the same region or if capturing a whole-region handle duplicates ownership. A capture may retain an interior reference/dependency, but cannot make an ended or transferred owner live again. The rule must also cover `effect` expression results: a temporary region operand is not permission to store an independent owner in a lexical `let`, ordinary record field, global or capture. Whole-region resource use and existing bits/reference value use need separate checking; this does not change scalar capture-by-value.

A compact candidate analysis records the state of an owning binding at each program point:

| Abstract state | Permitted operation |
| --- | --- |
| Owned(R) | Access or transfer the current symbolic region instance R. |
| Empty | Rebind to an acquired/received region; use as a live region is a compilation error. |
| Owned-or-empty | Refine through an explicit checked path before access, or reject an unproven use. This state arises when control-flow outcomes disagree. |

These are compiler facts, not a proposed region header or new user ADT. A straight-line consuming send changes `Owned(R)` to `Empty`. The compiler must also invalidate access through values/captures associated with R. Branch joins combine states; a loop body must be valid on its next iteration, rather than assuming the first iteration’s owner survives a send. Unknown runtime destinations or message order do not prevent checking the sender’s consumption and receiver’s typed acquisition boundaries.

Consumption is persistent across procedure calls and scope restoration. It is not equivalent to executing an ordinary nested `effect @r = empty`, whose value would be restored on return. The analysis must preserve consumption when a callee finishes, under both `perform` and `behave`. The intrinsic must identify the owning binding, not merely receive a copied numeric address. The synchronous implementation keeps program-point owner/access facts separate from `Environment` binding snapshots. Closing a procedure restores lookup without restoring consumed ownership.

A shadowing example illustrates the required precision: an outer `@r` owns R1, an inner `effect @r` owns a different R2, and the inner send consumes R2. Exiting that inner scope may restore the still-owned R1. It must never restore a consumed R2 or another alias of it. The lifetime of a module top-level frame until process exit does not require retaining regions it has already sent; process-exit cleanup handles only resources it still owns.

### Implemented synchronous foundation

Native region acquisition is an immediate effect initializer. A whole owner is rejected as a lexical local or ordinary callable parameter; regional fields and bits products retain their existing type restrictions. Each acquiring or destination effect frame has a private native owner slot. Moving nulls the source slot and installs the same region in the destination slot. Ordinary scope exit frees only its remaining owner; module slots are released at normal program exit. Payload layout, pointer offsets and allocator pairing are unchanged.

Owner reads require an owned binding and a live local region. Interior references keep access through a local owner-to-owner move; a captured whole owner stays tied to its consumed source frame. Branch and short-circuit joins reject access unless every path retains ownership. Unknown or repeated loops reject consumption of an outer owner, while statically zero/single iterations and per-iteration acquisition are supported. These are conservative prototype checks, not a general ownership-polymorphic type system or a selected mailbox protocol.

[The native evidence](../tests/cod/language/actor-foundation-validation.json) covers the module boundary, transfer through calls, non-resurrection, independent shadowing, conditional cleanup, repeated acquisition and exactly one release in the existing relocation harness. No runtime owner table, regional header field or heap allocation per binding is required.

### Local errors and the empty postcondition

H-168’s no-delivery-feedback contract removes any need to wait for acknowledgement before changing sender ownership. After local acceptance, the runtime owns delivery or disposal even if the recipient never receives the message. This does not by itself decide what a *local send error* does to the owner.

H-170 proposes returning the unsent region in a sum on local failure. The useful ownership contract is: entering the intrinsic consumes the original binding on every returning path; local acceptance leaves the runtime holding the packet, while local failure returns its ownership in the result. The failed payload is not discarded and the original binding does not silently become live again. Operand evaluation that fails before entering the intrinsic has not yet consumed it. The result must be acquired by an effect binding or disposed according to the resource-result rule; its precise syntax remains open.

A failed-send return is a new owning value for the same region allocation, not a copy of the graph. Moving it into a retry binding restores an explicit local owner. The proof must use this ownership path rather than a coincidental matching address; which pre-existing non-owning references may then be reused needs an explicit validity rule. H-169 establishes the source owner’s emptiness, not a blanket rule that every interior pointer becomes permanently unusable after a local owner-to-owner move.

The native `send` interface reports local errors without asserting delivery, and UDP does not guarantee delivery. These references support the feedback distinction, not a CoD ownership rule: [Linux send(2)](https://man7.org/linux/man-pages/man2/send.2.html), [RFC 768](https://www.rfc-editor.org/info/rfc768/). CoD is not required to use UDP packets, socket buffer semantics, retransmission or delivery-status tracking merely to adopt that distinction.

The implementation acceptance must distinguish successful-send emptiness, the chosen local-error rule, stale interior/captured references, consumption inside a nested procedure, restoration of an unrelated outer owner, branch joins, loop re-acquisition and source-module regions surviving initialization. The synchronous checks cover local moves, captures, restoration, joins, per-iteration acquisition and module lifetime. They do not establish send outcomes, receipt or returned-owner validity.

### Result sums and binding presence

H-170’s `Maybe<WorkPkg>` is illustrative. `WorkPkg` is the regional payload root; a return must carry the *whole-region owner*, not a stack copy of that regional object or merely a borrowed `ref<WorkPkg>`. `Maybe<region<WorkPkg>>` makes that distinction in design notation. It can communicate local acceptance versus a returned region but has no field for the local error cause.

A more informative candidate is the following logical sum, with names and generic/type-definition syntax still provisional:

```text
SendResult<P> = Accepted | LocalError { cause: SendError, packet: region<P> }
```

`Accepted` means local acceptance only. The error arm supplies both the system error and the returned owner, allowing a retry decision without a hidden last-error effect. This sum directly serves H-168’s local-error feedback and H-170’s returned-region purpose; it is not a proposal for delivery receipts, retries inside the primitive or a general exception system. If error detail is intentionally unnecessary, the simpler `Maybe<region<P>>` remains a valid alternative to discuss.

For either candidate, `effect @outcome = send(@mailbox, @packet)` is illustrative: `@packet` is empty after the intrinsic, and only the error variant can hold the region in `@outcome`. Matching that variant must move its owner into an effect binding such as `@retry`; extracting it into an ordinary copying local would violate the premise. An ownership-bearing sum therefore obeys the same carrier restriction as a direct region owner. Dropping a still-owning result must retain one defined cleanup responsibility. This does not permit owning region handles as ordinary fields inside user regional graphs.

It is useful for every dynamic binding to have a conceptual **absent/present** state. It is not necessary to make the value of every `@x` have the public type `Maybe<T>`. With the existing WPO rule, `effect @gain = 2; @gain + 1` can still use a statically known integer. An absent required binding remains a compilation error, and an emptied owner cannot be used until reacquired. An explicitly optional business value, such as a lookup result, can independently have a real `Maybe<T>` type; its `None` is a bound value, not the absence of the binding itself.

The recommended distinction is binding presence/ownership facts for all effects, explicit sum types for business or intrinsic results, and ordinary `@x` reads only where the checker proves the required value present. A runtime presence bit, sentinel or other slot layout is not chosen by this semantic distinction. Pattern matching and transfer out of an ownership-bearing sum remain the next bounded syntax/type design. The current branch/loop checks reject unproven ownership but provide no sum refinement or presence-test syntax.
