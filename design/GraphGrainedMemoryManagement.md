# CoD Regional Memory Vision and Implementation Tracker

Design revision 39 · 2026-11-24

## Design authority

[D-10’s explicit reference symbols](SymbolicDynamicScope.md#owner-parameter-reference-symbols-and-implementation-cost) require valid target access through the actual owner during installation, capture and restoration. The [scope reconciliation](../CHANGELOG.md#k-design-scope-audit-20261124) applies this dependency to the whole-region holder comparison.

At r38 the [human decision](../CHANGELOG.md#h-actor-region-transport-and-chan-ends-20261124) makes complete validated reception the publication boundary for the AKCP path. P-711 applies to private assembly, disposal and the [whole-region acceptance scenarios](ActorChannels.md#acceptance-scenarios). The send consumption point and fault owner follow their existing review.

[D-10-ACTOR-CHAN](DesignAuthorityRegistry.md#d-10-actor-chan) constrains r37's [Actor transfer review](#actor-holding-and-transfer-review) and [systemic-fault recovery](#ownership-when-a-systemic-fault-is-reported). [H-ACTOR-CHAN-BOUNDARY-20261123](../CHANGELOG.md#h-actor-chan-boundary-20261123) selects fire-and-forget sending except for system faults likely to compromise subsequent sends. [K-ACTOR-CHAN-RESEARCH-20261123](../CHANGELOG.md#k-actor-chan-research-20261123) removes the earlier generic per-message result recommendation. H-262/H-263 and intrinsic owner carriers remain; no exact send-consumption point or fault owner outcome is selected. C-39/C-54 and existing candidate value paths retain their scope. [D-32](ActorChannels.md) supplies the research workloads and acceptance cases.

[D-10-OWN-QUALIFIER](DesignAuthorityRegistry.md#d-10-own-qualifier) and [H-CONTEXTUAL-OWN-20261121](../CHANGELOG.md#h-contextual-own-20261121) cover the qualifier in the illustrative region handoff at r36.

- **Current capture-review reference:** [K-REPLACEMENT-EFFECTS-20261117](../CHANGELOG.md#k-replacement-effects-20261117) keeps the separate D-18 value-input qualification current. This file changes only review references; its semantic revision, governing value paths, constraints and acceptance are unchanged. The registry owns the current review scope.

- **Parser migration continuity:** [K-795](../CHANGELOG.md#k-795) continues H-595/D-13-ROOT-ACCESS/P-546–P-548 from r34 to r35. The compiler treats root as an ordinary name and rejects forcing it through intrinsic syntax. C-74/C-65/C-72 → charter §§2/3.1–3.3/4.3/6.3/8/12 retains readable named storage and ordinary lookup. Acceptance distinguishes user callables/storage/imports from builtin forcing, preserving unrelated intrinsics and C++ physical root helpers. Complete named-root/receiving proof and native support remain separate; D-16 and D-18's existing qualification boundaries are unchanged.

- **Named root access:** [H-595](../CHANGELOG.md#h-595)/[K-NAMED-ROOTS-20261117](../CHANGELOG.md#k-named-roots-20261117), [D-13-ROOT-ACCESS](DesignAuthorityRegistry.md#d-13-root-access)/P-546–P-548, covers r34's structural-region access section and aligned dependent examples. The human confirms both direct named-root access and region receiving, including ordinary assignment destinations, and explicitly removes the source root intrinsic. Candidate C-74/C-65/C-72 → charter §§2/3.1–3.3/4.3/6.3/8/12 serves readable storage selection with unchanged field kinds, actual owners, access, initialization and release. Acceptance is below; complete structural matching/proof/value-path qualification and compiler migration remain separate. Preserve region lifetime and relocation, D-16, D-17 cleanup and D-18's existing qualification boundaries; no new whole-product value or capture authority follows.

- **Callable placement and transfer boundaries:** [H-563](../CHANGELOG.md#h-563)/[H-567–H-568](../CHANGELOG.md#h-567), [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/P-503–P-505 and [D-22-TRANSFER](DesignAuthorityRegistry.md#d-22-transfer)/P-506–P-508 cover D-13r33's placement/operation terminology. Callable-view values have lexical-scope storage, never regional storage; only pure bits admit whole-payload transfer. CoD has no C++ object copying/moving model. C-54/C-65/C-72/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve regional relocatability and explicit identity/lifetime responsibility. D-18/D-22 own acceptance; retain dedicated callable construction/capture, graph transfer, actual owners and D-16. [K-LEXICAL-PRODUCTS-20261117](../CHANGELOG.md#k-lexical-products-20261117) aligns dependencies. H-564–H-566's callable-bearing ADT/anonymous-data direction remains under review, including type identity/results; no new aggregate implementation or full proof/physical/value-path qualification follows.

- **Owner-parameter symbol integration:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499, covers D-13r32's reference-symbol/owner dependency wording. The source form and staged sound acceptance are confirmed: installation selects one actual lexical or ordinary regional owner from the RHS; an `in @arena` declaration retains its additional selector constraint. C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve explicit dynamic context and lifetime responsibility. D-10 owns examples and acceptance. Preserve saved/captured owner validity, access/null/init, module symbol identity, no lifetime extension and D-16; full proof/physical/value-path qualification and source implementation remain separate. Other qualified scopes continue.

- **Callable-reference continuity:** [K-ALIAS-GLOSSARY-20261117](../CHANGELOG.md#k-alias-glossary-20261117) preserves this revision's existing D-18 dependency under the canonical CallableViewsAndCaptures.md path. Only dependency links and this continuity summary change; existing contracts, qualification, acceptance and D-16 remain unchanged.

- **Registry / current scope:** [D-13](DesignAuthorityRegistry.md#d-13); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole file pending-human.** C-39/C-40/C-53–C-55/P-94/P-138–P-143/P-155; H-156/H-157 establishes the local graph boundary.
- **Value path:** P-138/P-139 relates region-grained storage to inspectable graphs (C-01) and structured semantic units (C-05); P-140 constrains C-40. C-54/P-143 and D-14/P-155 concerns explicit lifetime responsibility through C-10. P-141/P-142 supplies representation/language dependencies, not qualification. These paths serve charter §§2–4 while keeping storage, access and execution distinct.
- **Contract boundary:** D-14/D-17 owns scoped lifecycle behavior. Same-region stored edges, hidden-owner preservation and explicit transfer remain; D-22 refines only bits-eligible placement. No transport, per-object ownership, borrow table or allocator policy is qualified by tracker/relocation examples.
- **Invariants / non-goals:** preserve region-local stored pointers, regional allocation, zero-cost relocation, no regional object copy/move and one whole-region cleanup responsibility. Captures do not extend lifetime or duplicate ownership. Functions cannot access dynamic bindings or call procedures. No per-object owner, mandatory generation table, runtime borrow registry, copied graph snapshot or allocator policy is selected.
- **Acceptance:** Validate relocation, same-owner graph edges, preserved hidden owners and exact region release. Tracker examples do not qualify transport or allocator policy. Earlier compiler coverage is recorded in the [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## One region per pointer graph

**Graph-Grained Memory Management** is CoD’s vision for managing memory at the granularity of a graph of objects connected by direct pointers. Each such graph occupies one region. Programs allocate and update individual objects inside it. The region is the intrinsic unit of reclamation, whole-image relocation and ownership transfer. User programs may recycle typed slots and manage free lists or pools inside a live region; returning a slot does not return regional storage to the system. [Ownership boundary and validation](RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy)

A direct pointer is a traversable edge between objects in the same region. The regional representation can encode it as a relative offset, so moving the whole image preserves those edges without per-edge pointer repair. Remapping and transporting the bytes have their own costs. [Regional pointer representation](RegionalAndLexicalStorageRefactor.md)

## Structural region definitions and variable storage

[CallableViewsAndCaptures.md](CallableViewsAndCaptures.md#structural-regions) records the field-subset source requirement and its relation to captured regional slots. Current compiler coverage is separately bounded by the source capability reference. WPO retains concrete layouts and actual owners; structural compatibility does not authorize layout casts, cross-owner stored edges or nominal data-interface conformance.

Region acquisition/release, capacity, typed placement and reference provenance remain intrinsic services. Free lists, typed pools and item reuse remain CoD library policies under H-190. A native root shell may implement this source contract with relocatable regional storage; it is not a separately exposed CoD object. Overlapping/retyping storage, compaction with outstanding captures and custom physical allocators require separate design.

### Named root access and receiving specifications

H-595 removes the CoD `root(r)` intrinsic. A region is a structural product of named root fields, accessed either by `r.name` or by a receiving specification. Neither form exposes a separate unnamed whole-root object. A named region schema and an otherwise admitted structural field requirement use the same field semantics; this does not select new anonymous-annotation grammar.

```cod
data Node { value:u64; }
region State { count:u64; ptr next:Node; }

# state is a live region matching State; other belongs to the same region.
{ ref n = count; ptr p = next; val old <- count; } := state;
n <- 10u64;           # Write the region's count field.
p = other;            # Rebind only local p.
state.next = other;   # Rebind the region's next slot.
state.count <- 11u64; # Write the direct bits field.

ptr var link -> state.next;
link = other;         # Explicit declared place alias of that regional slot.
```

Receiving a reference creates an ordinary independent local binding; it does not declare a place alias of the source slot. The existing standalone `var name -> place` form supplies an explicit place alias when needed. Receiving items keep their existing `=`/`<-` operators; no additional `var ... -> ...` receiving-item form is introduced. Ordinary ADT field access remains unchanged.

| Declared root field | Selected meaning and permitted destination operation |
| --- | --- |
| Direct/unmodified or val bits field | An addressable bits place in r. Value reception reads independent bits; ref reception borrows that place with owner r. `r.count <- bits` writes it; `r.count = target` rejects because it has no reference slot. This does not create an owner-qualified independent val type. |
| ref field | Its non-null selected target has owner r and admitted target access. `r.head = target` rebinds its regional slot; `r.head <- bits` writes the selected referent when that payload operation is admitted. |
| ptr field | Its nullable target/absence retains owner r. `r.next = target_or_null` updates the slot; `<-` writes through the selected non-null target under ordinary access/readiness/null rules. Ref reception still needs valid non-null evidence. |

Selecting a known field place does not read its payload. Reading a ref/ptr field's selected target requires an initialized slot; reading through it independently requires readable payload and any necessary non-null proof. Binding const and target const remain independent: a fixed ref slot may still expose a writable target, while writable rebinding cannot strengthen target access. Selecting roots grants no allocation, release or ownership-transfer authority beyond the existing region contract.

A receiving RHS evaluates once. For an existing region expression it borrows that region, selects the named fields and performs the ordinary requested operations. Omitted roots are untouched: there is no implicit read, initialization, destruction, ownership transfer or cleanup of those fields. This differs from a callable structural result, whose omitted outputs still receive their ordinary cleanup. An owned region returned by a source expression remains subject to ordinary temporary-owner cleanup; any selected borrow must satisfy the resulting lifetime, without retaining that region implicitly. Independent bits may outlive it after a valid read. [D-22](ConstructionAndAcquisition.md#region-sources-for-receiving) owns the shared receiving rules and this source-category distinction.

A spec selects current target/value information at its receiving operation. Later rebinding of state.next does not retarget p. A direct later occurrence of state.next observes its then-current binding; an explicit place alias such as link continues to name the selected regional slot. Dynamic region-symbol shadowing likewise affects new selections, while already selected views retain their actual owner. Cleanup, ownership transfer, release and resize preserve those ordinary distinctions.

Structural requirements match names and full field contracts, not ordinal positions or nominal region-schema names. Retain actual concrete layout and owner when a consumer requires a compatible subset. Additional known concrete fields follow D-18's lower-bound rule; no missing field becomes available merely because an owner is live. A generic `owner O` without regional shape evidence cannot select named roots. Nominal ADTs held by fields retain their own identity and explicit conformance.

There is no implicit first-root selection or flattening of a referenced object's fields. A single-field region still uses that field's name or a corresponding receiving item. A helper needing a Node receives the selected root; one needing region shape or allocation access receives the region. A policy object is an ordinary named field, such as `r.pool`, whose own abstract interface retains nominal conformance. Structural region compatibility does not invent a product-level nominal object or a packed-prefix layout. A field named root or capacity has only its ordinary declared meaning.

**Implementation boundary:** no builtin or compatibility meaning remains for `root(r)`. The parser treats root as an ordinary name for user callables, lexical storage and imports; intrinsic syntax cannot force whole-root access. An undeclared root follows ordinary lookup failure. Native root-address helpers belong to the storage implementation. This naming rule establishes no regional native or receiving support; the [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md) remains authoritative for implementation coverage.

**Acceptance:** cover direct and receiving access to bits/ref/ptr roots, single/multiple roots, renamed/reordered/subset selection, actual layout differences, omitted roots left untouched, destination selection/effect order, valid declared place aliases, later rebinding, inherited access, unreadable slots/payloads, nullable-to-ref rejection, missing fields, same shape with different owners, symbolic shadowing and ownership transfer/release/resize. Apply receiving overlap rules to the actual selected reads/writes, not to harmless shared target identities. Reject builtin whole-root access, hidden owner retention and invalid temporary borrows. No regional callable field, owning user-data field, general structural ADT, automatic graph transfer, RTTI or physical allocator change follows; D-16 bits/UoM identities and validity remain unchanged.

## Relationships between regions

Cross-region relationships exist in the business model. They are represented by values such as typed IDs or keys and interpreted by the program in its business context. CoD’s physical pointer types are confined to a single region. A procedure can work with multiple live regions through separate inputs or bindings, each with its own lifetime. [Language boundary](SymbolicDynamicScope.md#logical-relationships-between-regions)

For example, a chan whose message type is `WorkPkg` could carry a task node, input buffers linked by direct pointers and a `CustomerId` value. The package region contains the task's pointer graph. The receiving Actor can use the ID to look up a customer through its own business protocol. The customer's region keeps its independent lifetime. [Actor design](SymbolicDynamicScope.md#actor-concurrency-direction)

## Packet transport and effect resources are distinct scenarios

H-180 separates these uses of the same regional storage substrate:

| Scenario | Purpose | Relevant ownership operation |
| --- | --- | --- |
| Inter-Actor regional message | Construct a payload, send it, receive it and apply its contents. | An ordinary owning value participates in the packet’s transfer/error/result protocol. No dynamic-resource installation is required to build or send it. |
| Region-backed effect resource | Construct a local resource and make it available by symbol throughout the applicable dynamic context. | H-179 consumes the ordinary source when installing the dynamic owner. Definition-time capture and perform replay affect lookup, not ownership. |

A receiver can use its own effects to process a packet, but this does not turn the packet into an effect resource or make its transport inherit the sender’s dynamic context. Shared regional representation and relocation do not identify these lifecycle contracts. The effect-resource case must not be designed by asking how to move a captured resource through the packet send path.

## Implementation and evidence

Current source-language regional support is tracked in the [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md). The storage contract requires same-owner stored edges, actual-owner lifetime checks and relocation without pointer repair; C++ storage facilities do not establish CoD language acceptance.

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
| Reported systemic communication fault | The exact consumption point and fault ownership postcondition remain pending: the source may still hold the region, an intrinsic carrier may return ownership, or the runtime may dispose of it under the eventual contract. A fault report concerns continued sending capability, not the delivery status of this message. |
| Completed handoff / runtime retention | The intrinsic/runtime holds the region for delivery or disposal; sender loses access through ordinary values and dynamic bindings. Sender scope exit must not free it. The sender receives no delivery receipt, and ordinary later loss does not return ownership. |
| Receiving | The receiver acquires holding and access. Queue removal must not release the payload before its handler finishes. |
| Completed or abandoned | The current holder releases the region exactly once or explicitly transfers it again. A reported systemic fault and ordinary later message disposal are different observations. Later non-delivery cannot restore an already-consumed sender reference. |

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
| Interior regional reference | Non-owning access to an object, associated with its region instance. It may be a local traversal view or explicit function input, but using it requires that instance to remain accessible. H-482 and H-561 admit explicit ref/ptr symbol bindings to an interior target. Installation, capture and restoration require its actual owner to stay live. A reference cannot prolong lifetime or manufacture a second whole-region owner. |
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

### Synchronous lifecycle requirements

Capture/replay preserves the graph and its single cleanup responsibility. Acquisition establishes cleanup before root initialization; transfer preserves one responsible owner. Borrowed views retain actual-owner provenance. Dynamic-resource extraction is rejected under H-181. Owning optional values and packet transport retain their separate design scope.

Earlier compiler support, implementation entry points and validation commands are recorded in the [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

### Local errors and source availability

The current [Actor/chan contract](SymbolicDynamicScope.md#actor-concurrency-direction) refines H-168: report a systemic communication fault when subsequent sends are likely to fail; otherwise provide fire-and-forget sending. Ordinary loss supplies no individual outcome, receipt or returned packet. The planned network path uses AKCP over UDP. The receiver holds incomplete input privately and publishes a region only after complete reception and validation. Abandonment releases private storage. Transport repair and backpressure preserve ordinary fire-and-forget sending. [Regional transport](ActorChannels.md#regional-transport-over-akcp)

A reported systemic fault needs an explicit ownership postcondition. Whether send has already consumed the source, leaves ownership with the sender, returns an intrinsic owning carrier, or disposes of an accepted payload is not selected. These alternatives differ in source availability and cleanup. Reporting a system condition cannot leave ownership implicit.

After a completed transfer, H-262 makes the original owning name statically unavailable, without a nullable runtime payload. H-263's valid views under a new local holder do not grant outbound sender access. A later return of the same allocation does not by itself revive revoked views. Conditional sending must preserve these facts at control-flow joins.

H-196 requires any owner-bearing result or optional carrier to be intrinsic, including indirect containment; user regional products/sums cannot embed a whole-region owner. The message root may remain a user-defined type. Message ownership also remains distinct from capturing or replaying a dynamic resource under H-180/H-181.

Acceptance must distinguish completed handoff, a qualifying systemic fault under each selected ownership path, ordinary loss, abandoned runtime-held messages and invalid old views. Every path needs one responsible holder through release or onward transfer. Synchronous resource, capture and restoration evidence does not establish these chan transitions.

<a id="packet-result-sums"></a>

### Ownership when a systemic fault is reported

H-170's returned-region example remains a proposal for fault recovery. The current fire-and-forget boundary does not make an Accepted/LocalError sum the normal observation of every transmission. A later dropped message cannot return the already-consumed source or produce an implicit delivery report.

Compare three possible fault postconditions before choosing an interface:

| Candidate | Consequence that must be specified |
| --- | --- |
| Reject before ownership transfer | The original holder remains responsible; prove which source/view facts still hold when the fault is reported. |
| Consume and return through an intrinsic owner-bearing carrier | The old name stays consumed; the carrier needs defined acquisition, disposal and view-validity rules. |
| Consume and dispose under runtime responsibility | The sender keeps no packet owner; the runtime must account for release even when reporting the system fault. |

None is selected here. The contract must separate failures in argument preparation, the exact send boundary and faults discovered after handoff. H-262's consumed state applies once transfer has completed. If the API admits returned ownership, H-196's intrinsic carrier rule and H-263's lifetime/access obligations still apply. A borrowed root cannot substitute for the whole-region owner.

[Actor/chan research](ActorChannels.md#decisions-to-discuss-next) uses a request/reply workload and a shared result collector to make these alternatives reviewable without introducing source grammar.

### Direct dynamic resources and explicit Maybe values

H-181 makes binding availability a WPO obligation, separate from the binding’s value type. A direct resource-reference type is sufficient because ownership cannot be taken out of the dynamic variable. A missing enclosing/supplied binding is a compilation error, not an implicit `None`, null resource or default value. Perform’s valid captured shadows and behave’s caller context follow H-180/H-102’s existing path checks.

An application may deliberately declare `symbol @x :: Maybe<SomeRegion>`. The absent alternative is then a domain value established by an effect, not the absence of the symbol’s binding. A bare declaration does not initialize it to None. The present alternative retains the same prohibition on consuming the dynamic resource; unwrapping for access cannot mint a transferable owner. Shadowing the present alternative with absence keeps the hidden resource owned until its actual owning-scope end, and restores it when the shadow ends.

No universal Maybe transformation, runtime presence flag or new pointer representation is needed for this distinction. Typed symbol declarations and no-extraction checks are implemented; general sum construction/matching is not. The [language contract](SymbolicDynamicScope.md#binding-availability-and-optional-resource-values) records the separate binding/type obligations.

<a id="proposed-dynamic-value-boundary-and-soundness"></a>

## Symbolic references and soundness

H-563 forbids regional storage of callable-view values, including through compound carriers. This concerns executable views/environments, not regional data describing programs or definitions. H-567/H-568 supplies the source model: CoD has no C++ object copying/moving semantics; only pure bits admit whole-payload transfer. Reference binding, owned-region responsibility transfer, dedicated callable construction and explicit graph transfer remain distinct. Native implementation operations cannot grant wider source permissions.

Module method registration and bare symbol lookup create no runtime capture or TLS binding. Explicit materialization/installation and invocation retain the actual owners of captured or referenced resources. Family-dispatch evidence under H-375/H-376 must survive the applicable regional relocation and reference lifetime without making module definitions into resource owners. The physical evidence/ABI is a separate D-21 design task; no graph edge, allocator or owner protocol is changed by stage separation.

H-277–H-281 permits bits stored directly in TLS and non-bits references into regions supplied by region symbols. Bits transfer uses `effect @x <- rhs`; object-reference rebinding uses `effect @x = rhs`, with scoped restoration in both cases. Rebinding may select a different live region. Whole-region ownership and scoped procedure handlers have their own contracts.

### What makes the reference valid

The selected lexical owner or whole-region resource determines storage lifetime; an interior symbolic reference identifies a target within that lifetime. The compiler must track its actual owner through branches, calls, captures and scope restoration. A matching root type or an ambient symbol's current spelling is insufficient. WPO checks required bindings but does not make unknown runtime allocation instances identical.

Region-selector and existing shared-region declarations require their declared resource. H-561's explicit owner-parameter references instead admit a proved lexical or ordinary regional owner, without installing an owning-region symbol. Active, saved and captured bindings must remain within that owner's lifetime. Rebinding the TLS reference does not modify a stored edge inside either regional graph, transfer ownership from the supporting symbol or extend lifetime. Region-local sharing and cycles remain intentional. No per-object exclusive ownership or runtime target-alias table follows from this requirement.

### Boundaries that still need checking

**Lexical reference validity follows the actual owner.** A procedure can create its own local effect-owned region and use a derived reference while that scope is live. Returning that reference would outlive the acquisition and is rejected; returning independent scalar bits read before scope exit does not retain the reference's lifetime dependency. A supplied view into a still-live caller region may be returned instead. H-181 rejects extraction from an existing dynamic owner at the attempted transfer itself. Restricting dynamic value categories would not remove the need to check lexical view lifetimes.

**Capture values and ownership are different facts.** H-180 reaffirms that a procedure captures the then-current values of its used dynamic variables. Symbol identities determine which variables those are; they do not replace the value snapshots. For a region resource, capture retains the selected resource value and its lifetime dependency, without copying the graph or taking another independently movable owner. Perform temporarily shadows current lookup with those saved values; behave uses the current values. Resource mutations remain visible through the captured resource.

**Shadowed owners remain live.** If perform exposes captured R1 under an identity currently naming R2, R2 becomes unreachable through that identity during the call. Its owner is neither consumed nor replaced. Other valid references to R2 retain their validity. Leaving the call removes the shadow, restores R2 and does not release R1. The [binding/ownership trace below](#dynamic-resource-ownership-and-temporary-shadowing) makes these distinct facts explicit.

H-277 admits interior symbolic references; H-180 value capture requires preserving their actual region/lifetime dependency. An escaped procedure cannot use that capture to retain a dead resource. Whole effect-resource replay remains non-consuming. Local interior references still name the region from which they were derived; changing dynamic lookup does not retarget them. They require a live accessible owner at use. No arbitrary graph copy, implicit heap closure or lifetime extension for an escaped procedure is selected.

**Bits is a semantic category.** An offset or address is physically representable as bits, but a typed `T` retains a region dependency. A product, newtype or future sum containing such a reference must not become eligible simply by wrapping it. Independent bits values carry no regional-reference dependency; a regional record view retains its owner's lifetime even when its fields are bits. User code must not reconstruct dereference authority from an arbitrary integer; any owner-plus-index accessor must validate its type, region-relative target and bounds. No new integer-to-pointer conversion or stored cross-region edge is implied.

**Explicit graph access remains useful.** A function can traverse and edit explicitly supplied input graphs and return an existing reference while its region remains live. H-197/H-198 excludes dynamic-context access rather than memory writes; no automatic snapshot interval exists. A later argument or nested call must not leave an earlier view usable after its region is released or handed off. The current prototype checks the implemented reference subset; owning function arguments/results have D-17 evidence, while send-time invalidation remains unimplemented.

### Runtime creation, ordinary owners and editing

H-175 requires runtime construction and editing of regional messages before sending them to other Actors. H-177 permits ordinary variables to own those regions. The [construction contract](ConstructionAndAcquisition.md) and D-17 E1–E5 govern initialized construction, borrowing and owning calls; the chan-specific consumption and fault paths remain separate.

A message can be built while held by an ordinary owner. Separately, a region intended for dynamic use can be consumed into an effect binding under H-179. H-262 makes the transferred ordinary source statically unavailable; H-181 prohibits extracting the installed dynamic owner. Message construction and sending need no such effect installation.

The intended workflow is to construct and initialize a message graph, edit it under its ordinary owner, and send it through a chan with the matching static message type. This description selects neither a declaration spelling nor a returned-packet result. The [systemic-fault recovery alternatives](#ownership-when-a-systemic-fault-is-reported) must determine each remaining ownership path.

Returning an owner must transfer cleanup responsibility before its previous scope closes. Branches cannot invent a common region identity or two live owners from an equal root type. Preserve each actual owner and all permitted views through the chosen paths. Runtime allocation evidence alone does not establish inter-Actor handoff or network delivery.

### Current branch-analysis precision

Dynamic-resource consumption is rejected at the attempted extraction, including inside branches and loops. It cannot produce a valid moved-out symbolic binding. Fresh branch-local resources can shadow a supplied binding and unwind without consuming it; captured replay obeys the same owner-preservation rule.

The checker tracks the owning region of a regional reference and rejects local-resource escape or unsupported joins of different region identities. D-17 owning branch results may select between different live regions; general optional extraction remains separate. Historical consumption/correlated-guard probes describe the superseded move implementation and are not evidence of currently accepted programs.

### Application fit and the remaining design decisions

For the SNN, an owner for the CC plus a typed target-neuron index and frame selector can identify work without putting a borrowed target/frame view in the dynamic context. A helper obtains lexical views from the live CC and traverses them through explicit parameters. This fits the existing SoA and typed-index direction. If A and B are parts of one CC region, they must not become two independent owners of that same region; selectors distinguish them. Matching index brands alone do not establish the intended CC instance. Index selection must either deliberately mean the current CC or carry the required business/provenance check.

Named root selection supplies the checked typed subview through `r.name`, without a whole-root intrinsic. Construction and field access retain explicit owner provenance; the wider optional-owner transport API remains unselected. The API should be designed from the actual access paths, without inventing an unrestricted offset-to-pointer primitive or a new global lookup service.

H-170's proposed owner-bearing send result, if admitted for systemic-fault recovery, belongs to the ordinary packet flow. It does not force an owning sum into the proposed dynamic-value categories. H-181 independently permits an optional effect resource when that optionality is a business requirement. It does not make the packet result an effect resource or permit owner extraction from a dynamic optional value.

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
