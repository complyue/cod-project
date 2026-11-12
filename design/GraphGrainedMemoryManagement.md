# CoD Regional Memory Vision and Implementation Tracker

Design revision 31 · 2026-11-12

## Design authority

- **Callable-world and AOT scope:** [H-372–H-377](DesignAuthorityRegistry.md#h-372)/[K-534](DesignAuthorityRegistry.md#k-534), D-13r31, records runtime capture and installation boundaries while preserving graph ownership, regional relocation and resource restoration. C-35/C-36/C-65 and P-330/P-338 preserve explicit semantic construction, typed execution and inspectable business dispatch under charter §§2–3/§5.2/§6/§8/§12. [H-378](DesignAuthorityRegistry.md#h-378)/[K-535](DesignAuthorityRegistry.md#k-535) confirms [D-21](AheadOfTimeMultipleDispatch.md)'s P-333/P-334/P-337 value path for the stated behavioral scope. Detailed namespace, type-evidence/ABI, coverage and diagnostic mechanisms remain pending-human. H-379's entry-installation requirement is recorded separately; its operational cancellation boundary remains open. Existing qualified invariants continue only within their unchanged scopes. Acceptance requires the composed family/context, installation/capture and cleanup scenarios in D-21; current implementation evidence does not establish that new behavior. No root-charter change or whole-document qualification is inferred.

- **Current regional captures:** [D-18](RegionalClosures.md)/[K-459](DesignAuthorityRegistry.md#k-459) is the bounded contract for structural region requirements, aliases, captures and owner projection under H-327–H-335. This revision updates only that navigation and removes superseded research text under H-336; C-65/C-73/C-75 and P-294–P-298 retain their confirmed scope. Owner/access/relocation, dynamic effects and D-16 remain constraints. Acceptance and unresolved limits stay explicit in D-18; unrelated pending contracts are not qualified.

- **Declaration cleanup:** [H-316](DesignAuthorityRegistry.md#h-316)/[K-442](DesignAuthorityRegistry.md#k-442) applies C-72/P-283 to the current ref/ptr/val surface. Ordinary views name their target type; allocation and element selection preserve owner/access information directly. Let/var declarations and angle-bracket reference types are rejected. [Native evidence](../tests/cod/language/declaration-cleanup-validation.json) covers migrated examples, copied bits versus aliases, checked NPE, lifetime/relocation and multi-module composition. No nullable parameter/result grammar, reference-target allocation policy or general cyclic construction is selected. Existing pending qualifications remain pending.

- **Region-source continuity:** [H-307](DesignAuthorityRegistry.md#h-307)/[K-435](DesignAuthorityRegistry.md#k-435) updates region examples to the declaration-keyword surface in [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language). C-70/P-270–P-271 preserves the intrinsic borrowed/owning distinction and existing lifecycle constraints. This revision changes source notation and evidence navigation only; it does not qualify pending transport, optional-owner, numeric or general graph contracts. Acceptance is coherent examples plus the migrated compiler/lifecycle checks recorded by K-435.

- **Symbolic implementation scope:** [H-287–H-291](DesignAuthorityRegistry.md#h-287)/[K-426](DesignAuthorityRegistry.md#k-426) records the requested implementation and callable/result decisions: every effect yields void; procedure rebinding uses `=`, while inline `effect proc` introduces or signature-checks its symbol and captures at definition. C-36/C-54/C-65/C-66 and P-248–P-254 preserve typed dynamic context, actual-region lifetime and explicit capture. [Native evidence](../tests/cod/language/symbolic-context-validation.json) covers typed TLS payloads, staged groups, touch/replay checks and overload-backed object transfer. This bounded scope does not qualify whole-document drafts or implement ordinary ptr migration, transport, runtime closures or fractions.

- **Bounded exception/lifecycle contract:** [H-267](DesignAuthorityRegistry.md#h-267)/[K-408](DesignAuthorityRegistry.md#k-408) accepts [D-17r3 E1–E5](ExceptionControl.md#accepted-first-implementation-contract), including explicit throw-from only, cleanup, fresh owners and call/result transfer. This overrides pending labels for that enumerated scope; wider qualifier, assignment, mailbox and user exception graph choices remain open. [K-414](DesignAuthorityRegistry.md#k-414) records completed H-267/H-269 E1–E5 acceptance and [current native evidence](../tests/cod/language/exception-ownership-validation.json). Wider unconfirmed scopes retain their status.

- **Registry / scope:** [D-13](DesignAuthorityRegistry.md#d-13), revision 31, all sections. [K-411](DesignAuthorityRegistry.md#k-411) records D-17 local lifecycle implementation/evidence without adding transport or graph transactions. [H-265](DesignAuthorityRegistry.md#h-265)/[K-406](DesignAuthorityRegistry.md#k-406) links [exception control](ExceptionControl.md) and unchecked ultimate OOM to construction cleanup. [H-263/H-264](DesignAuthorityRegistry.md#h-263)/[K-405](DesignAuthorityRegistry.md#k-405) supplies permitted local-view survival and the initial typed construction form. [H-262](DesignAuthorityRegistry.md#h-262)/[K-404](DesignAuthorityRegistry.md#k-404) confirms compiler-tracked consumption of ordinary named owners; H-267/E1–E5 fixes the bounded exception and consuming call/result contract. [K-383](DesignAuthorityRegistry.md#k-383) reconciles the lexical-view example and named-source wording with non-extractable dynamic resources; H-262 now selects the ordinary named-source consumed state. K-379/K-367 maintains evidence navigation and graph/resource boundaries. H-156/H-157 confirms the local graph boundary; the full design and candidate value paths remain `pending-human`.
- **Value lineage:** P-138/P-139 relates region-grained storage to inspectable graphs (C-01) and structured semantic units (C-05); P-140 constrains C-40. C-54/P-143 and D-14/P-155 concerns explicit lifetime responsibility through C-10. P-141/P-142 supplies representation/language dependencies, not qualification. These paths serve charter §§2–4 while keeping storage, access and execution distinct.
- **Human baseline:** H-99/H-100 governs user storage and captures; H-156/H-157 sets the graph/inter-region boundary. H-166 keeps module owners alive through process exit. H-177/H-179 permits ordinary ownership and consuming installation; H-180/H-181 requires value capture, owner-preserving replay and no extraction from dynamic resources. H-184–H-189 governs typed lexical/symbolic procedures. H-190/H-193/H-196–H-203 and H-209 governs intrinsic ownership, regional data, mutable explicit-memory functions, borrowing/const and same-owner edges, with their recorded exclusions.
- **Invariants / non-goals:** preserve region-local stored pointers, regional allocation, zero-cost relocation, no regional object copy/move and one whole-region cleanup responsibility. Captures do not extend lifetime or duplicate ownership. Functions cannot access dynamic bindings or call procedures. No per-object owner, mandatory generation table, runtime borrow registry, copied graph snapshot or allocator policy is selected.
- **Acceptance / evidence:** native examples must preserve sharing/cycles, reject cross-owner stored edges and escaping views, keep hidden/captured owners alive and release each actual acquisition once. [Current owner/relocation evidence](../tests/cod/language/regional-validation.json) covers guarded scopes and independent relocated graph/pool inspection. [Symbolic reference evidence](../tests/cod/language/effect-conformance-validation.json) covers the supported reference-binding subset without settling its final allowed-type policy. Historical reports retain their own source revisions.
- **Pending boundary:** [Regional ownership](RegionalOwnership.md) keeps whole-region make/root construction, consuming call/result boundaries, owned/const syntax, optional outcomes and mailbox transfer separate from implemented dynamic resources and user-library reuse. H-277 admits region-symbol-backed interior references; their capture/escape lowering must prove resource lifetime. Native packet-transfer acceptance requires its own complete lifecycle and local-error proof; synchronous evidence does not supply it.

## One region per pointer graph

**Graph-Grained Memory Management** is CoD’s vision for managing memory at the granularity of a graph of objects connected by direct pointers. Each such graph occupies one region. Programs allocate and update individual objects inside it. The region is the intrinsic unit of reclamation, whole-image relocation and ownership transfer. User programs may recycle typed slots and manage free lists or pools inside a live region; returning a slot does not return regional storage to the system. [Ownership boundary and validation](RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy) [Regional lifetime rules](MemoryRegionAndRegionalTypes.md#3-lifetime-rules)

A direct pointer is a traversable edge between objects in the same region. The regional representation can encode it as a relative offset, so moving the whole image preserves those edges without per-edge pointer repair. Remapping and transporting the bytes have their own costs. [Regional pointer representation](MemoryRegionAndRegionalTypes.md#regional_ptrt-offset-based-storage)

## Structural region definitions and variable storage

[RegionalClosures.md](RegionalClosures.md#structural-regions) specifies the implemented field-subset requirement and its relation to captured regional slots. WPO retains concrete layouts and actual owners; structural compatibility does not authorize layout casts, cross-owner stored edges or nominal data-interface conformance.

Region acquisition/release, capacity, typed placement and reference provenance remain intrinsic services. Free lists, typed pools and item reuse remain CoD library policies under H-190. The source contract uses the existing native root shell and shilos allocator without changing D-01's physical representation. Overlapping/retyping storage, compaction with outstanding captures and custom physical allocators require separate design.

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
| Allocation and lifetime | The native prototype allocates regional records and arrays, retains module-owned regions until normal process exit, rejects extraction from effect-held owners and rejects references that would escape their owner’s scope. [Regional records](../tests/cod/language/region-records.cod) · [Lifetime rejection](../tests/cod/language/region-escape.cod) |
| Pointer boundary | The prototype rejects a field initialized with a pointer into another region. Independent inputs from two live regions retain their original owners, including references returned through functions. [Cross-region field rejection](../tests/cod/language/region-cross-pointer.cod) · [Independent owners](../tests/cod/language/region-capture.cod) |
| Whole-region relocation | Each native relocation program requires one non-null release, copies its region image, releases the original, then checks the relocated graph or pool. [Relocation evidence](../tests/cod/language/regional-validation.json) |

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
| Local send error | Under H-170's returned-region proposal, the original source no longer owns the packet and the error result holds the unsent region until an ordinary holder acquires it or it is disposed. H-262 selects compiler-tracked unavailability of the transferred source; the precise send/result consumption boundary remains pending. This reports no remote delivery. |
| Locally accepted / queued | The intrinsic/runtime holds the region; sender loses access through ordinary values and dynamic bindings. Sender scope exit must not free it. No recipient delivery is promised. |
| Receiving | The receiver acquires holding and access. Queue removal must not release the payload before its handler finishes. |
| Completed or abandoned | The current holder releases the region exactly once or explicitly transfers it again. Local failure and later message disposal are different stages. Later non-delivery cannot restore an already-consumed sender reference. |

Invalidation should be reasoned about for the region as a whole, since all direct pointer edges stay inside it under H-156/H-157. Revoking only the root variable leaves aliases to interior nodes usable. Packet consumption must invalidate stale sender views; restoring an unrelated dynamic binding is not a way to recover a sent packet. Captured effect-resource replay follows the separate owner-preserving contract below. If the same allocation later returns to the sender, address equality alone is insufficient to revalidate earlier references. A fresh receipt/access contract is needed; no generation counter, borrow syntax or runtime reference table is selected here.

Conditional transfer is a compiler-design case, not just a local send return code. If a send succeeds only on one branch, the later join cannot unconditionally treat the sender as still owning the graph. Rejection that preserves the owner and rejection that returns a new owner have different source and checking consequences. Loops and forwarding must also account for which region remains held. The synchronous resource implementation checks scoped acquisitions, same-owner reference joins and no dynamic extraction. Ordinary transfer and send/receipt outcomes still need their operational contract; H-262 supplies the named-source consumed state after transfer.

Physical relocation remains independent of holding transfer. Same-address-space handoff can pass a system owner and the root’s region-relative location without repairing internal edges. Copying an entire supported image can preserve its offsets but costs bytes and creates another lifetime; read-only sharing requires a shared lifetime and a stable-content rule. Neither copying nor sharing is implicitly included in exclusive transfer. A queue’s system envelope is not a user regional object and does not create a language-level cross-region pointer permission.

The proposed first milestone and decision table are in [Actor implementation readiness](GlobalVariablesDoneRight.md#actor-implementation-readiness). Existing allocator symmetry, region-only allocation, pointer encoding and H-146’s temporary first-field self-reference limitation remain unchanged. Synchronous resource shadowing, cleanup and module lifetime have native evidence; relocation does not establish ordinary owner moves, Actor send, receipt or isolation.

<a id="effect-held-region-ownership"></a>
## Whole-region ownership

“Owner” below identifies lifetime responsibility and the existing compiler’s holder facts. H-183 does not treat it as an already defined ordinary-variable type or move syntax. The [ownership-language draft](RegionalOwnership.md) makes that distinction explicit and compares H-182’s initializer-source proposal against the confirmed shadowing/availability rules. Packet transfer remains separate from effect installation.

H-177 permits ordinary lexical variables to own regions. A region owner is not freely copyable; H-262 makes the named source unavailable at compile time after transfer, without an implicit null value. Static tracking depends on exclusive holding and checked access transitions, not on every holder having an `@` name. WPO can check these transitions along demanded paths without enumerating runtime allocations. Repeated creation and receipt must preserve the same rules for each instance.

Three kinds of information must stay separate:

| Kind | Meaning and proposed checking |
| --- | --- |
| Whole-region owning variable or effect binding | Responsible for holding/releasing a region. Ordinary holders transfer it under D-17 E4/E5; dynamic resource bindings cannot be consumed under H-181. The prototype rejects `effect @b <- @a`. H-179 ordinary-to-dynamic installation is implemented by consuming the ordinary holder. Captured replay performs no move. An `effect` creates a new binding frame even when its destination has the same symbol identity as its source. |
| Interior regional reference | Non-owning access to an object, associated with its region instance. It may be a local traversal view or explicit function input, but using it requires that instance to remain accessible. The prototype supports owner-tracked interior-reference bindings/captures; H-175’s proposed restriction remains unselected. A reference cannot prolong lifetime or manufacture a second whole-region owner. |
| Intrinsic runtime holder | Holds a message between consumption and receipt/disposal, when no user variable owns it. This belongs to system runtime machinery and does not expose a second user storage carrier. Receipt establishes an ordinary packet owner; constructing/installing an effect resource is not an implicit receive operation. |

A lexical owner handle is a stack/local resource value, while the objects it owns remain region allocated. Moving that handle transfers release responsibility without copying the graph. Procedure metadata records symbol identities and captured values; a captured resource does not acquire a second independent owner. The same rule must apply to owner-valued expression results, parameters and returns; exact lexical move syntax and aggregate/result forms remain design work. H-177 does not permit arbitrary owner fields inside regional graphs, extend module-global declaration categories or change the independent function context rule.

One candidate for ordinary transferable owners records the following facts at each program point. D-14 also considers explicit optional source storage; this table does not select between them or describe an implemented ordinary-owner path. Valid H-181 dynamic resources have no consuming moved-out state:

| Abstract state | Permitted operation |
| --- | --- |
| Owned(R) | Access R; consume it only through an ownership operation permitted for this scenario/holder. |
| Empty | Rebind to an acquired/received region; use as a live region is a compilation error. |
| Owned-or-empty | Refine through an explicit checked path before access, or reject an unproven use. This state arises when control-flow outcomes disagree. |

These are compiler facts, not a proposed region header or new user ADT. A straight-line consuming send changes `Owned(R)` to `Empty`. The compiler must also invalidate access through interior views associated with R when the region ceases to be locally accessible. Branch joins combine states; a loop body must be valid on its next iteration, rather than assuming the first iteration’s owner survives a send. Unknown runtime destinations or message order do not prevent checking the sender’s consumption and receiver’s typed acquisition boundaries.

Consumption is persistent across procedure calls and scope restoration. It is not equivalent to executing an ordinary nested `effect @r <- empty`, whose value would be restored on return. The analysis must preserve consumption when a callee finishes, under both `perform` and `behave`. The intrinsic must consume the actual owning operand, not merely receive a copied numeric address. The synchronous implementation keeps region liveness separate from `Environment` binding snapshots and restores lookup without duplicating resource ownership. The D-17 ordinary-owner path preserves consumed-source facts across return and exceptional edges; local views remain valid only while their actual owner permits access.

A dynamic-resource example illustrates the required precision: an outer `@r` owns R1 and a nested scope installs its own newly acquired R2 under that identity. R1 stays owned while hidden. Ending the inner owning scope releases R2 and restores R1. In contrast, replaying a previously captured R1 under `perform` acquires no new ownership and releases neither pre-existing resource at call exit. Module top-level owners keep H-166’s process lifetime. Packet send consumption follows the separate ordinary-owner path.

### Implemented synchronous foundation

The bounded executable implements capture at callable-value construction, caller-bound lexical calls, typed make and ordinary owning locals/calls/results. Effect installation consumes an ordinary owner; extraction from the resulting dynamic resource is rejected under H-181. Capture/replay does not copy the graph or duplicate cleanup responsibility. Owning optional values and packet transport remain unimplemented.

Native acquisition registers cleanup before root initialization. Transfers leave the source cleanup holder empty and establish one destination responsibility. LLVM joins can return a selected owner from different allocation paths; borrowed views retain their provenance and cannot outlive the actual owner. [Focused lifecycle evidence](../tests/cod/language/ownership-lifecycle-validation.json) checks construction, callee-entry timing, abandoned results and exact cleanup. Payload pointer offsets and allocator pairing remain unchanged; no dynamic-resource extraction or packet transport is implemented.

[Current guarded-owner evidence](../tests/cod/language/regional-validation.json) records four acquisitions, four releases in order [1,2,3,0] and eighteen live reads. Three selected logical scopes release their local regions; a skipped operand acquires none. The independent graph/pool checks preserve 120-byte and 152-byte images after copying and freeing the originals. CoD does not resume against the relocated images. [Typed-procedure evidence](../tests/cod/language/typed-procedure-validation.json) retains its earlier capture/no-extraction scope. There is no runtime owner table or per-binding user heap allocation.

### Local errors and source availability

H-196 requires any returned-owner outcome/optional type to be intrinsic, including indirect containment. User-defined regional products/sums cannot embed a region owner. A payload root remains a user-defined type; the whole-region handle and its owning result forms are intrinsic. The notation below specifies an ownership outcome, not permission to declare a copyable SendResult in user code.

H-168’s no-delivery-feedback contract removes any need to wait for acknowledgement before changing sender ownership. After local acceptance, the runtime owns delivery or disposal even if the recipient never receives the message. This does not by itself decide what a *local send error* does to the owner.

H-170 proposes returning the unsent region in a sum on local failure. The useful ownership contract is: entering the intrinsic consumes the original binding on every returning path; local acceptance leaves the runtime holding the packet, while local failure returns its ownership in the result. The failed payload is not discarded and the original binding does not silently become live again. Operand evaluation that fails before entering the intrinsic has not yet consumed it. The packet result may be acquired by an ordinary owner under H-177 or disposed according to the packet-result rule; its precise syntax remains open.

A failed-send return is a new owning value for the same region allocation, not a copy of the graph. Moving it into a retry binding restores an explicit local owner. The proof must use this ownership path rather than a coincidental matching address; which pre-existing non-owning references may then be reused needs an explicit validity rule. H-262 fixes the ordinary source's consumed state. H-263 preserves views across local ownership transfer while the new owner permits access; revival of revoked sender views after error recovery remains a separate question. H-169's historical effect-to-effect move rule is superseded by H-181 and does not settle ordinary packet-source syntax.

Under H-262, a transferred packet declaration is unavailable at compile time; it is not a nullable value that fails on later dereference. The proposed send contract also revokes saved sender views, while a local-error arm supplies an explicit returned owner from which new views can be derived. The exact failure sum, consumption boundary and reuse of pre-existing views remain pending. Explicit optional storage may model runtime absence but is not required for every named owner. H-194 additionally requires access-only versus transferring procedure parameters to be readable and lifetime-checked.

The native `send` interface reports local errors without asserting delivery, and UDP does not guarantee delivery. These references support the feedback distinction, not a CoD ownership rule: [Linux send(2)](https://man7.org/linux/man-pages/man2/send.2.html), [RFC 768](https://www.rfc-editor.org/info/rfc768/). CoD is not required to use UDP packets, socket buffer semantics, retransmission or delivery-status tracking merely to adopt that distinction.

Packet-transfer acceptance must distinguish successful handoff, the selected source post-state and local-error rule, invalid old views, consumption inside nested calls and restoration of unrelated outer resources. Capture admissibility requires its own selected contract. The synchronous checks cover no dynamic extraction, captures, restoration, one-owner joins, per-iteration acquisition and module lifetime. They do not establish ordinary source moves, send outcomes, receipt or returned-owner validity.

### Packet result sums

H-170’s `Maybe<WorkPkg>` is illustrative. `WorkPkg` is the regional payload root; a return must carry the *whole-region owner*, not a stack copy of that regional object or merely a borrowed `WorkPkg`. An intrinsic optional whole-region owner expresses that alternative; a borrowed region declaration denotes access, not transferable ownership. Its optional syntax remains unselected. It can communicate local acceptance versus a returned region but has no field for the local error cause.

A more informative candidate is the following logical sum, with names and generic/type-definition syntax still provisional:

```text
SendResult<P> = Accepted | LocalError { cause::SendError; owned region packet::P }
```

`Accepted` means local acceptance only. The error arm supplies both the system error and the returned owner, allowing a retry decision without a hidden last-error effect. This sum directly serves H-168’s local-error feedback and H-170’s returned-region purpose; it is not a proposal for delivery receipts, retries inside the primitive or a general exception system. If error detail is intentionally unnecessary, a simpler intrinsic optional owner remains an alternative to discuss. Both owner-bearing outcomes must be intrinsic under H-196, not user-defined regional sums.

For either candidate, binding the result of `send(mailbox, packet)` is illustrative: the ordinary source `packet` relinquishes ownership at the intrinsic boundary, and only the error variant can return it in `outcome`. H-262 makes a transferred ordinary source statically unusable; explicit absence is modeled only where the source itself has an optional type. Matching that variant must move its owner into another ordinary holder; copying it into a second independent owner is invalid. This packet-result contract does not require making the result a dynamic effect resource. H-177 permits the ordinary carrier without settling exact owning-sum or matching syntax. Dropping a still-owning result must retain one defined cleanup responsibility. This does not permit owning region handles as ordinary fields inside user regional graphs.

### Direct dynamic resources and explicit Maybe values

H-181 makes binding availability a WPO obligation, separate from the binding’s value type. A direct resource-reference type is sufficient because ownership cannot be taken out of the dynamic variable. A missing enclosing/supplied binding is a compilation error, not an implicit `None`, null resource or default value. Perform’s valid captured shadows and behave’s caller context follow H-180/H-102’s existing path checks.

An application may deliberately declare `symbol @x :: Maybe<SomeRegion>`. The absent alternative is then a domain value established by an effect, not the absence of the symbol’s binding. A bare declaration does not initialize it to None. The present alternative retains the same prohibition on consuming the dynamic resource; unwrapping for access cannot mint a transferable owner. Shadowing the present alternative with absence keeps the hidden resource owned until its actual owning-scope end, and restores it when the shadow ends.

No universal Maybe transformation, runtime presence flag or new pointer representation is needed for this distinction. Typed symbol declarations and no-extraction checks are implemented; general sum construction/matching is not. The [language contract](SymbolicDynamicScope.md#binding-availability-and-optional-resource-values) records the separate binding/type obligations.

<a id="proposed-dynamic-value-boundary-and-soundness"></a>

## Symbolic references and soundness

Module method registration and bare symbol lookup create no runtime capture or TLS binding. Explicit materialization/installation and invocation retain the actual owners of captured or referenced resources. Family-dispatch evidence under H-375/H-376 must survive the applicable regional relocation and reference lifetime without making module definitions into resource owners. The physical evidence/ABI is a separate D-21 design task; no graph edge, allocator or owner protocol is changed by stage separation.

H-277–H-281 permits bits stored directly in TLS and non-bits references into regions supplied by region symbols. Bits transfer uses `effect @x <- rhs`; object-reference rebinding uses `effect @x = rhs`, with scoped restoration in both cases. Rebinding may select a different live region. Whole-region ownership and statically selected procedure values retain their separate contracts. The bounded native contract is recorded in [symbolic-context evidence](../tests/cod/language/symbolic-context-validation.json).

### What makes the reference valid

A whole-region resource determines storage lifetime; an interior symbolic reference identifies an object within that lifetime. The compiler must track the actual region behind the reference through branches, calls, captures and scope restoration. A matching root type or an ambient symbol's current spelling is insufficient. WPO checks required bindings but does not make unknown runtime allocation instances identical.

A non-bits binding must be backed by a region-symbol resource; an ordinary owner's liveness alone does not install that resource. Rebinding the TLS reference does not modify a stored edge inside either regional graph, transfer ownership from the supporting symbol or extend lifetime. Region-local sharing and cycles remain intentional. No per-object exclusive ownership or runtime alias table follows from this requirement.

### Boundaries that still need checking

**Lexical reference validity follows the actual owner.** A procedure can create its own local effect-owned region and use a derived reference while that scope is live. Returning that reference would outlive the acquisition and is rejected; returning independent scalar bits read before scope exit does not retain the reference's lifetime dependency. A supplied view into a still-live caller region may be returned instead. [Factory evidence](../tests/cod/language/adapted-factory-validation.json) checks this owner distinction through direct calls and four forwarding modules: a captured-region result survives an unrelated inner scope, while a result allocated in that inner region cannot escape. H-181 rejects extraction from an existing dynamic owner at the attempted transfer itself. Restricting dynamic value categories would not remove the need to check lexical view lifetimes.

**Capture values and ownership are different facts.** H-180 reaffirms that a procedure captures the then-current values of its used dynamic variables. Symbol identities determine which variables those are; they do not replace the value snapshots. For a region resource, capture retains the selected resource value and its lifetime dependency, without copying the graph or taking another independently movable owner. Perform temporarily shadows current lookup with those saved values; behave uses the current values. Resource mutations remain visible through the captured resource.

**Shadowed owners remain live.** If perform exposes captured R1 under an identity currently naming R2, R2 becomes unreachable through that identity during the call. Its owner is neither consumed nor replaced. Other valid references to R2 retain their validity. Leaving the call removes the shadow, restores R2 and does not release R1. The [binding/ownership trace below](#dynamic-resource-ownership-and-temporary-shadowing) makes these distinct facts explicit.

H-277 admits interior symbolic references; H-180 value capture requires preserving their actual region/lifetime dependency. An escaped procedure cannot use that capture to retain a dead resource. Whole effect-resource replay remains non-consuming. Local interior references still name the region from which they were derived; changing dynamic lookup does not retarget them. They require a live accessible owner at use. No arbitrary graph copy, implicit heap closure or lifetime extension for an escaped procedure is selected.

**Bits is a semantic category.** An offset or address is physically representable as bits, but a typed `T` retains a region dependency. A product, newtype or future sum containing such a reference must not become eligible simply by wrapping it. Independent bits values carry no regional-reference dependency; a regional record view retains its owner's lifetime even when its fields are bits. User code must not reconstruct dereference authority from an arbitrary integer; any owner-plus-index accessor must validate its type, region-relative target and bounds. No new integer-to-pointer conversion or stored cross-region edge is implied.

**Explicit graph access remains useful.** A function can traverse and edit explicitly supplied input graphs and return an existing reference while its region remains live. H-197/H-198 excludes dynamic-context access rather than memory writes; no automatic snapshot interval exists. A later argument or nested call must not leave an earlier view usable after its region is released or handed off. The current prototype checks the implemented reference subset; owning function arguments/results have D-17 evidence, while send-time invalidation remains unimplemented.

### Runtime creation, ordinary owners and editing

H-175 requires CoD user programs to create and edit regions at runtime before sending them to other processes. H-177 permits ordinary variables to own these regions. Current `region(capacity)` already emits a runtime allocation through `cod_lang_region_new`; the frontend’s region IDs describe provenance and are not compile-time allocations. The current restrictions are owner placement/escape and the missing typed owner/root and mailbox interfaces.

A movable packet can be constructed and sent while held by ordinary variables. Separately, a region intended for dynamic-context use can be transferred into an effect binding. H-179 makes that transfer consuming: the source relinquishes ownership. H-262 makes the ordinary source statically unavailable after that transfer, without implicit absence. The packet workflow does not require dynamic installation. H-181’s restriction on transfer out belongs to effect-resource ownership, not to the definition of packet transport. In design notation only:

```text
ref packet = create_region(...);
... construct and edit the graph using this procedure-local owner ...
ref result = send(mailbox, packet);
```

Construction and editing under a procedure-local ordinary owner do not capture it or an interior pointer in a procedure definition. Captured effect-resource access must be distinguished from access to a packet region created during the current invocation. A packet-building procedure can eventually return ownership to its caller rather than retaining it dynamically. D-17 fixes and implements bounded make/root/owning-call forms. Send and owner-bearing optional outcomes remain separate design work.

An owner returned from a block or procedure must transfer cleanup responsibility to the receiving holder before the source scope closes. A conditional result may select different region instances with the same owner type: joining their types alone cannot invent a shared region identity or declare both source holders live. The result must have one owner on each path, and each source's ownership availability and existing views must be checked on that path. These are concrete extensions beyond today's same-region-only join and effect-only acquisition. No runtime alias registry or general predicate theorem prover is implied.

Acceptance should cover one region created at runtime, populated through current-owner access, returned/moved into an ordinary holder, and eventually released exactly once. A branch-selected owner must remain usable while invalid source/view uses are rejected. Send/returned-error ownership additionally needs the pending mailbox and sum contracts; synchronous allocation evidence cannot establish cross-process delivery.

### Current branch-analysis precision

Dynamic-resource consumption is rejected at the attempted extraction, including inside branches and loops. It cannot produce a valid moved-out symbolic binding. Fresh branch-local resources can shadow a supplied binding and unwind without consuming it; captured replay obeys the same owner-preservation rule.

The checker tracks the owning region of a regional reference and rejects local-resource escape or unsupported joins of different region identities. D-17 owning branch results may select between different live regions; general optional extraction remains separate. Historical consumption/correlated-guard probes describe the superseded move implementation and are not evidence of currently accepted programs.

### Application fit and the remaining design decisions

For the SNN, an owner for the CC plus a typed target-neuron index and frame selector can identify work without putting a borrowed target/frame view in the dynamic context. A helper obtains lexical views from the live CC and traverses them through explicit parameters. This fits the existing SoA and typed-index direction. If A and B are parts of one CC region, they must not become two independent owners of that same region; selectors distinguish them. Matching index brands alone do not establish the intended CC instance. Index selection must either deliberately mean the current CC or carry the required business/provenance check.

This requires an ergonomic, checked way to obtain a declared typed root or subview from an owner. The native language provides typed region declarations, make/root and alloc views. These operations retain explicit owner provenance; the wider optional-owner transport API remains unselected. The API should be designed from the actual access paths, without inventing an unrestricted offset-to-pointer primitive or a new global lookup service.

H-170’s owner-bearing send result belongs to the ordinary packet flow. It does not force an owning sum into the proposed dynamic-value categories. H-181 independently permits an optional effect resource when that optionality is a business requirement. It does not make the packet result an effect resource or permit owner extraction from a dynamic optional value.

The direction is runtime callable-value capture of permitted dynamic values, owner-preserving replay, ordinary packet owners and scoped effect-resource owners. Complete reference-provenance lowering and the remaining typed owner/root interfaces still require implementation and validation. Current branch precision and lexical-view validity need checking independently of these distinctions.

## Dynamic resource ownership and temporary shadowing

An ordinary variable can own a region before it is installed as an effect resource. H-179 makes that installation consuming: the ordinary source relinquishes ownership and the resource's owning scope takes responsibility. H-262 makes the ordinary source statically unavailable, without an implicit null value. Capturing the resulting value and replaying it are different operations. They must not execute that ownership-transfer rule again.

Suppose an owning scope holds R1 and `@p` captures its value under `@work`. A nested caller scope then owns R2 under the same identity:

| Point | Visible value of `@work` | R1 owner | R2 owner |
| --- | --- | --- | --- |
| Caller before perform | R2 | Still holds R1 | Still holds R2 |
| During perform of `@p` | Captured R1 | Unchanged | Unchanged; temporarily hidden through this variable |
| After perform returns | R2 | Unchanged | Unchanged; visible again |
| During behave of `@p` from that caller | R2 | Unchanged | Unchanged |

Perform does not move ownership from either holder, construct a replacement owner or overwrite the hidden holder with an empty value. Removing its temporary shadow destroys neither region. Scope-based destruction happens only when the actual owning scope ends; a local region newly acquired during the call follows that own scope. A module-level owning scope lasts until process exit under H-166. Capturing a scalar saves its value; capturing a resource does not freeze its mutable contents.

H-181’s confirmed no-transfer-out rule prevents a callee from consuming a dynamic resource owner. It must still allow H-180’s temporary capture replay: the replay changes lookup, not the region holder. Explicit movement to another owner, scope shadowing with a newly acquired resource, and replay of an already captured resource must remain semantically distinct even if their lowered representations share native addresses.

Every effect returns void (H-288), so installation exposes no owner-bearing expression result. An explicit subsequent resource read remains non-extractable under H-181. Saved-value replay retains the H-180 non-consuming contract and never duplicates cleanup responsibility.

### Source inspection and bounded acceptance

`define_procedure` retains definition-time values in compiler metadata. Symbolic `perform` selects that environment; `behave` and direct lexical procedure calls use the caller context. Replay does not call `bind` to transfer a region. `bind` rejects an existing dynamic holder as a consuming source; fresh acquisition registers cleanup. `block`/`close_holdings` only close holders created inside the scope being exited.

LLVM effects for known procedure/region values remain compile-time metadata; native cleanup registers fresh acquisitions and restores only entries after its scope mark. The resource-capture fixture defines under R1, invokes perform under R2, checks each selected capacity, then uses R2 and the restored R1 again. The separate native regional harness verifies relocation and one release per allocation. This bounded evidence is not a proof of every lifetime path or packet-transfer behavior.
