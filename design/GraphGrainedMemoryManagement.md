# CoD Regional Memory Vision and Implementation Tracker

Design revision 12 · 2026-11-07

## Design authority

- **Implemented callable/resource subset:** [H-188/H-189](DesignAuthorityRegistry.md#h-188) authorizes implementation/validation and selects current dynamic bindings for direct lexical procedure calls. [K-305](DesignAuthorityRegistry.md#k-305) records typed/grouped symbols, `proc(T, ...) -> RT` signatures, lexical procedures, combined symbolic declarations, captured-value-preserving installation and rejection of ownership extraction from dynamic resources. [Native evidence](../tests/cod/language/typed-procedure-validation.json) covers this subset. General optional sums, ordinary-owner syntax and packet transfer remain separate design work; full-contract qualification is unchanged.

- **Ownership source-language review:** [H-182/H-183/K-302](DesignAuthorityRegistry.md#h-182) require a coherent syntax/semantics/use design before treating “ordinary owner” as a source-language category. [D-14](RegionalOwnership.md) drafts resource construction, transferable temporaries, writable optional sources and all selected optionality forms. Those mechanisms remain pending-human. H-184 separately admits lexical/symbolic procedures and requires typed symbol declarations; procedure-valued effects must not be accidentally excluded by a data-resource rule.

- **Direct resource types and optional domain values:** [H-181/K-301](DesignAuthorityRegistry.md#h-181) confirms that region-valued symbolic bindings cannot be consumed or emptied by moving ownership out. They support shadowing with owner preservation. WPO requires a supplied binding at each demanded use, so a direct region-reference type needs no implicit `Maybe`. Explicit `symbol @x :: Maybe<SomeRegion>` remains allowed for business optionality; the declaration alone supplies no value. This supersedes H-169’s effect-to-effect consuming direction and settles H-178’s no-transfer-out question. Typed symbols and dynamic-resource no-extraction are implemented; general optional sums remain design work.

- **Resource installation:** H-177 permits ordinary owners and [H-179](DesignAuthorityRegistry.md#h-179) confirms consuming transfer from an ordinary variable into a dynamic resource owner. H-181 confirms the restriction on ownership transfer out of dynamic resource bindings. It does not apply to captured replay, which preserves owners under H-180.

- **Governing clarification:** [H-180/K-300](DesignAuthorityRegistry.md#h-180) separates regional packet send/receive from region-backed effect resources shared through dynamic context. Procedures capture used dynamic values at definition; perform shadows their identities temporarily without replacing, consuming or destroying either the captured resource or the hidden caller resource. This supersedes H-176’s identity-only interpretation. Revision9 covers the distinction and source-inspected implementation evidence.

- **Current syntax and analysis:** [H-172/K-296](DesignAuthorityRegistry.md#h-172) confirms the exclusive `effect @name <- rhs` spelling without changing transfer or restoration. [H-173](DesignAuthorityRegistry.md#h-173) proposes limiting dynamic values to owners, bits and statically selected procedures; the complete value-category restriction and root-access API remain `pending-human`; H-180 reaffirms value capture and owner-preserving replay independently of the pending allowed-type set.
- **Registry and scope** [D-13](DesignAuthorityRegistry.md#d-13), revision 12, all sections. The local graph boundary is confirmed. The full design and candidate value paths are `pending-human`. [K-293](DesignAuthorityRegistry.md#k-293) records the holding/transfer review requested by H-165; its exclusive-handoff lifecycle is a proposal.
- **Current review** [H-166–H-168/K-294](DesignAuthorityRegistry.md#h-166) cover process-lifetime module scopes, the effect-only holder premise and local-error-only send feedback. H-169’s effect-to-effect move direction is superseded by H-181; [K-295](DesignAuthorityRegistry.md#k-295) records the earlier synchronous implementation. The send/receipt proof remains design analysis. H-170 proposes returning the unsent region on local failure; sum syntax and binding-presence representation remain open.
- **Value lineage** P-138/P-139 propose that region-grained storage serves inspectable regional graphs (C-01) and efficient structured semantic units (C-05). P-140 records the confirmed language constraint on C-40. P-141/P-142 record dependencies on the regional representation and language contracts. [Charter](RegionalSemanticPlatformCharter.md) · [Regional types](MemoryRegionAndRegionalTypes.md) · [Language design](SymbolicDynamicScope.md)
- **Human baseline** H-99/H-100 govern user storage and captures. H-156 defines one direct-pointer graph per region. H-157 restricts inter-region relationships to the business level. H-151/H-152 supply the Actor model direction and make region holding and transfer a design priority. [Graph and language decisions](DesignAuthorityRegistry.md#h-156) · [Actor decisions](DesignAuthorityRegistry.md#h-151)
- **Invariants and acceptance** Keep every CoD pointer edge within its region and preserve whole-region lifetime and relocation. Track native evidence separately from the open construction, holding and transfer rules. [K-280](DesignAuthorityRegistry.md#k-280)

## One region per pointer graph

**Graph-Grained Memory Management** is CoD’s vision for managing memory at the granularity of a graph of objects connected by direct pointers. Each such graph occupies one region. Programs allocate and update individual objects inside it. The region is the unit of reclamation and whole-image relocation, and the candidate unit of ownership transfer. [Regional lifetime rules](MemoryRegionAndRegionalTypes.md#3-lifetime-rules)

A direct pointer is a traversable edge between objects in the same region. The regional representation can encode it as a relative offset, so moving the whole image preserves those edges without per-edge pointer repair. Remapping and transporting the bytes have their own costs. [Regional pointer representation](MemoryRegionAndRegionalTypes.md#regional_ptrt-offset-based-storage)

## Relationships between regions

Cross-region relationships exist in the business model. They are represented by values such as typed IDs or keys and interpreted by the program in its business context. CoD’s physical pointer types are confined to a single region. A procedure can work with multiple live regions through separate inputs or bindings, each with its own lifetime. [Language boundary](SymbolicDynamicScope.md#logical-relationships-between-regions)

For example, a `WorkPkg` for `Mainbox<WorkPkg>` could contain a task node, input buffers linked by direct pointers and a `CustomerId` value. The package region contains the task’s pointer graph. The receiving process can use the ID to look up a customer through its own business protocol. The customer’s region keeps its independent lifetime. [Actor design](SymbolicDynamicScope.md#actor-concurrency-direction)

## Packet transport and effect resources are distinct scenarios

H-180 separates these uses of the same regional storage substrate:

| Scenario | Purpose | Relevant ownership operation |
| --- | --- | --- |
| Cross-process regional packet | Construct a payload, send it, receive it and apply its contents. | An ordinary owning value participates in the packet’s transfer/error/result protocol. No dynamic-resource installation is required to build or send it. |
| Region-backed effect resource | Construct a local resource and make it available by symbol throughout the applicable dynamic context. | H-179 consumes the ordinary source when installing the dynamic owner. Definition-time capture and perform replay affect lookup, not ownership. |

A receiver can use its own effects to process a packet, but this does not turn the packet into an effect resource or make its transport inherit the sender’s dynamic context. Shared regional representation and relocation do not identify these lifecycle contracts. The effect-resource case must not be designed by asking how to move a captured resource through the packet send path.

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

The [dynamic bindings tracker](GlobalVariablesDoneRight.md) follows the execution-context side of the same actor example. Receiver interpretation can use dynamic bindings, while packet storage follows its own transfer contract. This integration does not identify effect-resource ownership with message ownership.

## Actor holding and transfer review

**Holding keeps storage alive; access permission determines who may read or mutate it.** A captured reference is not a holder. The current CoD compiler associates each reference with a scoped region owner and rejects use after that owner’s lifetime; its captures do not extend lifetime. The host `auto_region` is movable, but that alone supplies neither CoD alias invalidation nor a successful/rejected-send rule. Moving the host owner is distinct from moving or copying any regional object.

For the exclusive-handoff candidate, one region has one release responsibility at each stage. The table describes obligations, not a selected public API or an added runtime state field.

| Stage | Holder and access under the candidate |
| --- | --- |
| Constructing | Sender holds the region and may access its graph through live references. The declared payload root must belong to this region. |
| Local send error | Under H-170’s returned-region proposal, the original binding is empty and the error result holds the unsent region until an ordinary holder acquires it or it is disposed. This reports no remote delivery. |
| Locally accepted / queued | The intrinsic/runtime holds the region; sender loses access through ordinary values and dynamic bindings. Sender scope exit must not free it. No recipient delivery is promised. |
| Receiving | The receiver acquires holding and access. Queue removal must not release the payload before its handler finishes. |
| Completed or abandoned | The current holder releases the region exactly once or explicitly transfers it again. Local failure and later message disposal are different stages. Later non-delivery cannot restore an already-consumed sender reference. |

Invalidation should be reasoned about for the region as a whole, since all direct pointer edges stay inside it under H-156/H-157. Revoking only the root variable leaves aliases to interior nodes usable. Packet consumption must invalidate stale sender views; restoring an unrelated dynamic binding is not a way to recover a sent packet. Captured effect-resource replay follows the separate owner-preserving contract below. If the same allocation later returns to the sender, address equality alone is insufficient to revalidate earlier references. A fresh receipt/access contract is needed; no generation counter, borrow syntax or runtime reference table is selected here.

Conditional transfer is a compiler-design case, not just a local send return code. If a send succeeds only on one branch, the later join cannot unconditionally treat the sender as still owning the graph. Rejection that preserves the owner and rejection that returns a new owner have different source and checking consequences. Loops and forwarding must also account for which region remains held. The local transfer implementation checks branch joins and loop re-acquisition; send/receipt outcomes still need the result and access contract described below.

Physical relocation remains independent of holding transfer. Same-address-space handoff can pass a system owner and the root’s region-relative location without repairing internal edges. Copying an entire supported image can preserve its offsets but costs bytes and creates another lifetime; read-only sharing requires a shared lifetime and a stable-content rule. Neither copying nor sharing is implicitly included in exclusive transfer. A queue’s system envelope is not a user regional object and does not create a language-level cross-region pointer permission.

The proposed first milestone and decision table are in [Actor implementation readiness](GlobalVariablesDoneRight.md#actor-implementation-readiness). Existing allocator symmetry, region-only allocation, pointer encoding and H-146’s temporary first-field self-reference limitation remain unchanged. Synchronous local owner moves and module lifetime are validated; the relocation evidence does not establish Actor send, receipt or isolation.

<a id="effect-held-region-ownership"></a>
## Whole-region ownership

“Owner” below identifies lifetime responsibility and the existing compiler’s holder facts. H-183 does not treat it as an already defined ordinary-variable type or move syntax. The [ownership-language draft](RegionalOwnership.md) makes that distinction explicit and compares H-182’s initializer-source proposal against the confirmed shadowing/availability rules. Packet transfer remains separate from effect installation.

H-177 permits ordinary lexical variables to own regions, superseding H-167’s effect-only placement premise. H-169’s source-empty transfer remains: an owner is not freely copyable. Static tracking depends on exclusive holding and checked access transitions, not on every holder having an `@` name. WPO can check these transitions along demanded paths without enumerating runtime allocations. Repeated creation and receipt must preserve the same rules for each instance.

Three kinds of information must stay separate:

| Kind | Meaning and proposed checking |
| --- | --- |
| Whole-region owning variable or effect binding | Responsible for holding/releasing or transferring a region. The current H-169 implementation supports `effect @b <- @a` as a consuming move; H-181 prohibits taking ownership out of that dynamic source. H-179 confirms consuming ordinary-to-dynamic installation. Captured replay performs neither move. An `effect` creates a new binding frame even when its destination has the same symbol identity as its source. |
| Interior regional reference | Non-owning access to an object, associated with its region instance. It may be a local traversal view or pure function input, but using it requires that instance to remain accessible. H-175 proposes rejecting it as a procedure capture. It cannot prolong lifetime or manufacture a second whole-region owner. |
| Intrinsic runtime holder | Holds a message between consumption and receipt/disposal, when no user variable owns it. This belongs to system runtime machinery and does not expose a second user storage carrier. Receipt establishes an ordinary packet owner; constructing/installing an effect resource is not an implicit receive operation. |

A lexical owner handle is a stack/local resource value, while the objects it owns remain region allocated. Moving that handle transfers release responsibility without copying the graph. Procedure metadata records symbol identities and captured values; a captured resource does not acquire a second independent owner. The same rule must apply to owner-valued expression results, parameters and returns; exact lexical move syntax and aggregate/result forms remain design work. H-177 does not permit arbitrary owner fields inside regional graphs, extend module-global declaration categories or change function purity.

For ordinary transferable owners and the earlier executable, a compact analysis records holder state at each program point. It does not prescribe a moved-out state for a valid H-181 dynamic resource:

| Abstract state | Permitted operation |
| --- | --- |
| Owned(R) | Access R; consume it only through an ownership operation permitted for this scenario/holder. |
| Empty | Rebind to an acquired/received region; use as a live region is a compilation error. |
| Owned-or-empty | Refine through an explicit checked path before access, or reject an unproven use. This state arises when control-flow outcomes disagree. |

These are compiler facts, not a proposed region header or new user ADT. A straight-line consuming send changes `Owned(R)` to `Empty`. The compiler must also invalidate access through interior views associated with R when the region ceases to be locally accessible. Branch joins combine states; a loop body must be valid on its next iteration, rather than assuming the first iteration’s owner survives a send. Unknown runtime destinations or message order do not prevent checking the sender’s consumption and receiver’s typed acquisition boundaries.

Consumption is persistent across procedure calls and scope restoration. It is not equivalent to executing an ordinary nested `effect @r <- empty`, whose value would be restored on return. The analysis must preserve consumption when a callee finishes, under both `perform` and `behave`. The intrinsic must consume the actual owning operand, not merely receive a copied numeric address. The synchronous implementation keeps program-point owner/access facts separate from `Environment` binding snapshots. Closing a procedure restores lookup without restoring consumed ownership.

A dynamic-resource example illustrates the required precision: an outer `@r` owns R1 and a nested scope installs its own newly acquired R2 under that identity. R1 stays owned while hidden. Ending the inner owning scope releases R2 and restores R1. In contrast, replaying a previously captured R1 under `perform` acquires no new ownership and releases neither pre-existing resource at call exit. Module top-level owners keep H-166’s process lifetime. Packet send consumption follows the separate ordinary-owner path.

### Implemented synchronous foundation

The executable implements definition-time capture, caller-bound lexical calls and effect-only acquisition of fresh region resources. Consuming ownership out of those dynamic bindings is rejected under H-181. Ordinary owners, owning optional values and packet transfer remain separate unimplemented contracts. Capture/replay does not copy the graph or duplicate cleanup responsibility.

Native acquisition is an immediate effect initializer. Each acquired resource has one cleanup slot, retained to process exit for module ownership or released when a local owning scope ends. There is no dynamic-owner move path. Payload layout, pointer offsets and allocator pairing are unchanged. Ordinary owner parameters/results and packet transport remain separate work.

[Current native evidence](../tests/cod/language/typed-procedure-validation.json) covers owner-preserving replay and no-extraction rejection; the regional harness separately verifies relocation and exactly one release. Historical owner-move evidence retains its original scope. No runtime owner table or per-binding user heap allocation is introduced.

### Local errors and the empty postcondition

H-168’s no-delivery-feedback contract removes any need to wait for acknowledgement before changing sender ownership. After local acceptance, the runtime owns delivery or disposal even if the recipient never receives the message. This does not by itself decide what a *local send error* does to the owner.

H-170 proposes returning the unsent region in a sum on local failure. The useful ownership contract is: entering the intrinsic consumes the original binding on every returning path; local acceptance leaves the runtime holding the packet, while local failure returns its ownership in the result. The failed payload is not discarded and the original binding does not silently become live again. Operand evaluation that fails before entering the intrinsic has not yet consumed it. The packet result may be acquired by an ordinary owner under H-177 or disposed according to the packet-result rule; its precise syntax remains open.

A failed-send return is a new owning value for the same region allocation, not a copy of the graph. Moving it into a retry binding restores an explicit local owner. The proof must use this ownership path rather than a coincidental matching address; which pre-existing non-owning references may then be reused needs an explicit validity rule. H-169 establishes the source owner’s emptiness, not a blanket rule that every interior pointer becomes permanently unusable after a local owner-to-owner move.

The native `send` interface reports local errors without asserting delivery, and UDP does not guarantee delivery. These references support the feedback distinction, not a CoD ownership rule: [Linux send(2)](https://man7.org/linux/man-pages/man2/send.2.html), [RFC 768](https://www.rfc-editor.org/info/rfc768/). CoD is not required to use UDP packets, socket buffer semantics, retransmission or delivery-status tracking merely to adopt that distinction.

The implementation acceptance must distinguish successful-send emptiness, the chosen local-error rule, stale interior references and prohibited captures, consumption inside a nested procedure, restoration of an unrelated outer owner, branch joins, loop re-acquisition and source-module regions surviving initialization. The synchronous checks cover local moves, captures, restoration, joins, per-iteration acquisition and module lifetime. They do not establish send outcomes, receipt or returned-owner validity.

### Packet result sums

H-170’s `Maybe<WorkPkg>` is illustrative. `WorkPkg` is the regional payload root; a return must carry the *whole-region owner*, not a stack copy of that regional object or merely a borrowed `ref<WorkPkg>`. `Maybe<region<WorkPkg>>` makes that distinction in design notation. It can communicate local acceptance versus a returned region but has no field for the local error cause.

A more informative candidate is the following logical sum, with names and generic/type-definition syntax still provisional:

```text
SendResult<P> = Accepted | LocalError { cause: SendError, packet: region<P> }
```

`Accepted` means local acceptance only. The error arm supplies both the system error and the returned owner, allowing a retry decision without a hidden last-error effect. This sum directly serves H-168’s local-error feedback and H-170’s returned-region purpose; it is not a proposal for delivery receipts, retries inside the primitive or a general exception system. If error detail is intentionally unnecessary, the simpler `Maybe<region<P>>` remains a valid alternative to discuss.

For either candidate, `let outcome = send(mailbox, packet)` is illustrative: the ordinary source `packet` is empty after the intrinsic, and only the error variant can hold it in `outcome`. Matching that variant must move its owner into another ordinary holder; copying it into a second independent owner is invalid. This packet-result contract does not require making the result a dynamic effect resource. H-177 permits the ordinary carrier without settling exact owning-sum or matching syntax. Dropping a still-owning result must retain one defined cleanup responsibility. This does not permit owning region handles as ordinary fields inside user regional graphs.

### Direct dynamic resources and explicit Maybe values

H-181 makes binding availability a WPO obligation, separate from the binding’s value type. A direct resource-reference type is sufficient because ownership cannot be taken out of the dynamic variable. A missing enclosing/supplied binding is a compilation error, not an implicit `None`, null resource or default value. Perform’s valid captured shadows and behave’s caller context follow H-180/H-102’s existing path checks.

An application may deliberately declare `symbol @x :: Maybe<SomeRegion>`. The absent alternative is then a domain value established by an effect, not the absence of the symbol’s binding. A bare declaration does not initialize it to None. The present alternative retains the same prohibition on consuming the dynamic resource; unwrapping for access cannot mint a transferable owner. Shadowing the present alternative with absence keeps the hidden resource owned until its actual owning-scope end, and restores it when the shadow ends.

No universal Maybe transformation, runtime presence flag or new pointer representation is needed for this distinction. Typed symbol declarations and no-extraction checks are implemented; general sum construction/matching is not. The [language contract](SymbolicDynamicScope.md#binding-availability-and-optional-resource-values) records the separate binding/type obligations.

## Proposed dynamic-value boundary and soundness

H-173 proposes allowing only owning-region references, bits values and procedures whose selected definition is statically known in dynamic bindings. The complete allowed-type set remains a design candidate, not an enabled restriction. H-181 explicitly permits domain-selected Maybe-wrapped region values, so the three-item list must not be read as banning that form. The current prototype still accepts an interior `ref<T>` in an effect binding. H-174 asks how precisely the current implementation handles ownership after branches. The [syntax and lifetime evidence](../tests/cod/language/effect-arrow-validation.json) separates observed compiler behavior from proposals and proof obligations.

### What the restriction would simplify

A whole-region owner determines storage lifetime; an interior reference only identifies an object within that lifetime. If dynamic bindings may contain both, a procedure can acquire an indirect dependency on a region through an apparently ordinary `@target`, through an alias of that symbol, or through a retained environment in the current value-capturing implementation. Restoring the binding map does not prove that the restored referent is still accessible. Ordinary packet ownership can be consumed and interior views must respect its lifetime. Under H-181 an established dynamic resource cannot be consumed that way, but captured or local views must still stay within their actual owning scopes.

Keeping borrowed interior references out of the dynamic context would remove that ambient access path. Multiple references within one live region need not be prohibited: region-local sharing and cycles are intentional, individual objects are not reclaimed, procedure mutation is sequential, and pure calls cannot mutate their input graphs. The immediate proof obligation is live, correctly typed access, not universal non-aliasing or a new per-object exclusive-borrow rule. The context would contain explicit storage owners, independent values and statically selected behavior. Local graph traversal can still use lexical references and explicit pure function inputs; restrictions on explicit interior-reference procedure parameters need their own contract. This serves C-36’s inspectable context and C-54’s explicit ownership through C-10; it does not require a per-object runtime alias table or change regional offsets. P-144/P-145 remain dependencies; H-180 supplies value capture and owner-preserving replay, not a captured graph copy or a new owner.

The restriction is useful but not necessary for soundness in principle. Region typing can associate many references with one region and require that region to remain accessible at each use; it need not determine which exact object every pair of references aliases. Cyclone’s region system has a formal core soundness argument using region and effect information. Its separate unique-pointer facilities also illustrate the distinction between consuming ownership and bounded borrowing. These are feasibility references, not a CoD proof or a proposal to adopt Cyclone’s heap, annotations, runtime opening checks or reference counting. [Cyclone regional type system](https://www.cs.cornell.edu/Projects/cyclone/papers/cyclone-regions.pdf) · [Cyclone memory-management manual](https://www.cs.cornell.edu/projects/cyclone/online-manual/main-screen008.html)

### Boundaries that still need checking

**Lexical references can cross an effectful call.** Consider a lexical reference obtained from `alloc @arena`, followed by a procedure that moves `@arena` into its local owner and returns. That destination’s scope releases the region. A later dereference of the lexical reference is invalid even if every dynamic binding contains only an owner, bits or a statically selected procedure. The native probe `lexical-reference-after-owner-release` is rejected today. Its companion first reads a scalar through a pure function, releases the region, then prints the copied scalar; it succeeds. The proposed dynamic restriction does not remove this distinction.

**Capture values and ownership are different facts.** H-180 reaffirms that a procedure captures the then-current values of its used dynamic variables. Symbol identities determine which variables those are; they do not replace the value snapshots. For a region resource, capture retains the selected resource value and its lifetime dependency, without copying the graph or taking another independently movable owner. Perform temporarily shadows current lookup with those saved values; behave uses the current values. Resource mutations remain visible through the captured resource.

**Shadowed owners remain live.** If perform exposes captured R1 under an identity currently naming R2, R2 becomes unreachable through that identity during the call. Its owner is neither consumed nor replaced. Other valid references to R2 retain their validity. Leaving the call removes the shadow, restores R2 and does not release R1. The [binding/ownership trace below](#dynamic-resource-ownership-and-temporary-shadowing) makes these distinct facts explicit.

H-175’s proposed prohibition on captured interior pointers remains a separate allowed-type question. It does not prohibit capturing a whole effect-resource value for replay under H-180. Local interior references still name the region from which they were derived; changing dynamic lookup does not retarget them. They require a live accessible owner at use. No arbitrary graph copy, implicit heap closure or lifetime extension for an escaped procedure is selected.

**Bits is a semantic category.** An offset or address is physically representable as bits, but a typed `ref<T>` retains a region dependency. A product, newtype or future sum containing such a reference must not become eligible simply by wrapping it. Plain copied scalar/product values are independent. A copied view of a regional record containing references is not automatically an independent bits snapshot. User code must not reconstruct dereference authority from an arbitrary integer; any owner-plus-index accessor must validate its type, region-relative target and bounds. No new integer-to-pointer conversion or stored cross-region edge is implied.

**Pure traversal remains useful.** A pure function can traverse explicitly supplied input graphs and return an existing reference while the caller’s owner is live, preserving H-127/H-129/H-133. Its nested pure calls cannot transfer or release ownership because they cannot invoke procedures/effects. Argument evaluation happens before this interval: a later argument can invalidate an earlier regional argument, so the call boundary must still validate all supplied dependencies. Restricting dynamic bindings is not a reason to prohibit ordinary graph traversal or to introduce copied snapshots.

### Runtime creation, ordinary owners and editing

H-175 requires CoD user programs to create and edit regions at runtime before sending them to other processes. H-177 permits ordinary variables to own these regions. Current `region(capacity)` already emits a runtime allocation through `cod_lang_region_new`; the frontend’s region IDs describe provenance and are not compile-time allocations. The current restrictions are owner placement/escape and the missing typed owner/root and mailbox interfaces.

A movable packet can be constructed and sent while held by ordinary variables. Separately, a region intended for dynamic-context use can be transferred into an effect binding. H-179 makes that transfer consuming: the ordinary source is empty afterwards. The packet workflow does not require dynamic installation. H-181’s restriction on transfer out belongs to effect-resource ownership, not to the definition of packet transport. In design notation only:

```text
let packet = create_region(...);
... construct and edit the graph using this procedure-local owner ...
let result = send(mailbox, packet);
```

Construction and editing under a procedure-local ordinary owner do not capture it or an interior pointer in a procedure definition. Captured effect-resource access must be distinguished from access to a packet region created during the current invocation. A packet-building procedure can eventually return ownership to its caller rather than retaining it dynamically. Exact creation/root/return/send spellings and owner parameter modes remain unimplemented design work; the notation does not supply a new callable or borrowing API.

An owner returned from a block or procedure must transfer cleanup responsibility to the receiving holder before the source scope closes. A conditional result may select different region instances with the same owner type: joining their types alone cannot invent a shared region identity or declare both source holders live. The result must have one owner on each path, and each source’s empty/live state and existing views must be checked on that path. These are concrete extensions beyond today’s same-region-only join and effect-only acquisition. No runtime alias registry or general predicate theorem prover is implied.

Acceptance should cover one region created at runtime, populated through current-owner access, returned/moved into an ordinary holder, and eventually released exactly once. A branch-selected owner must remain usable while invalid source/view uses are rejected. Send/returned-error ownership additionally needs the pending mailbox and sum contracts; synchronous allocation evidence cannot establish cross-process delivery.

### Current branch-analysis precision

Dynamic-resource consumption is rejected at the attempted extraction, including inside branches and loops. It cannot produce a valid moved-out symbolic binding. Fresh branch-local resources can shadow a supplied binding and unwind without consuming it; captured replay obeys the same owner-preservation rule.

The checker tracks the owning region of a regional reference and rejects local-resource escape or unsupported joins of different region identities. General ordinary-owner branch results and optional extraction still require D-14's source-language contract. Historical consumption/correlated-guard probes describe the superseded move implementation and are not evidence of currently accepted programs.

### Application fit and the remaining design decisions

For the SNN, an owner for the CC plus a typed target-neuron index and frame selector can identify work without putting a borrowed target/frame view in the dynamic context. A helper obtains lexical views from the live CC and traverses them through explicit parameters. This fits the existing SoA and typed-index direction. If A and B are parts of one CC region, they must not become two independent owners of that same region; selectors distinguish them. Matching index brands alone do not establish the intended CC instance. Index selection must either deliberately mean the current CC or carry the required business/provenance check.

This requires an ergonomic, checked way to obtain a declared typed root or subview from an owner. The current language has an unparameterized `region` resource and `alloc`/`ref<T>` operations; it does not yet provide the complete typed-owner/root API needed for this replacement. The API should be designed from the actual access paths, without inventing an unrestricted offset-to-pointer primitive or a new global lookup service.

H-170’s owner-bearing send result belongs to the ordinary packet flow. It does not force an owning sum into the proposed dynamic-value categories. H-181 independently permits an optional effect resource when that optionality is a business requirement. It does not make the packet result an effect resource or permit owner extraction from a dynamic optional value.

The direction is definition-time capture of permitted dynamic values, owner-preserving replay, ordinary packet owners and scoped effect-resource owners. The complete allowed-type restriction and typed owner/root API remain open. Current branch precision and lexical-view validity need checking independently of these distinctions.

## Dynamic resource ownership and temporary shadowing

An ordinary variable can own a region before it is installed as an effect resource. H-179 makes that installation consuming: the ordinary source becomes empty and the resource’s owning scope takes responsibility. Capturing the resulting value and replaying it are different operations. They must not execute that ownership-transfer rule again.

Suppose an owning scope holds R1 and `@p` captures its value under `@work`. A nested caller scope then owns R2 under the same identity:

| Point | Visible value of `@work` | R1 owner | R2 owner |
| --- | --- | --- | --- |
| Caller before perform | R2 | Still holds R1 | Still holds R2 |
| During perform of `@p` | Captured R1 | Unchanged | Unchanged; temporarily hidden through this variable |
| After perform returns | R2 | Unchanged | Unchanged; visible again |
| During behave of `@p` from that caller | R2 | Unchanged | Unchanged |

Perform does not move ownership from either holder, construct a replacement owner or overwrite the hidden holder with an empty value. Removing its temporary shadow destroys neither region. Scope-based destruction happens only when the actual owning scope ends; a local region newly acquired during the call follows that own scope. A module-level owning scope lasts until process exit under H-166. Capturing a scalar saves its value; capturing a resource does not freeze its mutable contents.

H-181’s confirmed no-transfer-out rule prevents a callee from consuming a dynamic resource owner. It must still allow H-180’s temporary capture replay: the replay changes lookup, not the region holder. Explicit movement to another owner, scope shadowing with a newly acquired resource, and replay of an already captured resource must remain semantically distinct even if their lowered representations share native addresses.

H-111’s expression-result rule also cannot duplicate a region owner. Installing an ordinary owner with `effect` and evaluating its result does not create an independent owner. H-181 rules out a consuming use of the dynamic resource through that result; exact typed access forms remain part of the owner API design. Saved-value replay retains the H-180 non-consuming contract. No runtime reference count, new user wrapper type or binding-frame ABI follows merely from these semantic distinctions.

### Source inspection and bounded acceptance

`define_procedure` retains definition-time values in compiler metadata. Symbolic `perform` selects that environment; `behave` and direct lexical procedure calls use the caller context. Replay does not call `bind` to transfer a region. `bind` rejects an existing dynamic holder as a consuming source; fresh acquisition registers cleanup. `block`/`close_holdings` only close holders created inside the scope being exited.

LLVM effects for known procedure/region values remain compile-time metadata; native cleanup registers fresh acquisitions and restores only entries after its scope mark. The resource-capture fixture defines under R1, invokes perform under R2, checks each selected capacity, then uses R2 and the restored R1 again. The separate native regional harness verifies relocation and one release per allocation. This bounded evidence is not a proof of every lifetime path or packet-transfer behavior.
