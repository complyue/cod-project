# CoD construction, bits ADTs and region acquisition

Design revision: 25 · Recorded: 2026-11-17 · Status: discussion draft with confirmed requirements; consuming resize direction recorded; RHS normalization and capacity/backend details under review; whole-document approval pending; implementation incomplete.

## Design authority

- **Structural-product UX review:** [H-569](../CHANGELOG.md#h-569), [D-22-ANON-DATA](DesignAuthorityRegistry.md#d-22-anon-data)/P-509–P-511, covers r25's partial named reception requirement and structural-anonymous-data candidate. Preserve named nominal identity, once-only evaluation, per-field operations, required cleanup, owner/access and H-563/H-567/H-568. Exact shape/variance, result/receiving syntax and full value-path qualification remain pending; partial receiving does not itself require general structural subtyping.

- **Callable placement and transfer boundaries:** [H-563](../CHANGELOG.md#h-563)/[H-567–H-568](../CHANGELOG.md#h-567), [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/P-503–P-505 and [D-22-TRANSFER](DesignAuthorityRegistry.md#d-22-transfer)/P-506–P-508 cover D-22r24's placement/operation terminology. Callable-view values have lexical-scope storage, never regional storage; only pure bits admit whole-payload transfer. CoD has no C++ object copying/moving model. C-54/C-65/C-72/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve regional relocatability and explicit identity/lifetime responsibility. D-18/D-22 own acceptance; retain dedicated callable construction/capture, graph transfer, actual owners and D-16. [K-LEXICAL-PRODUCTS-20261117](../CHANGELOG.md#k-lexical-products-20261117) aligns dependencies. H-564–H-566's callable-bearing ADT/anonymous-data direction remains under review, including type identity/results; no new aggregate implementation or full proof/physical/value-path qualification follows.

- **Typed reservation and completed construction:** [H-562](../CHANGELOG.md#h-562)/[K-TYPED-RESERVATION-20261117](../CHANGELOG.md#k-typed-reservation-20261117), [D-22-RESERVATION](DesignAuthorityRegistry.md#d-22-reservation)/P-500–P-502 covers D-22r23's initialization boundary. Bare `make T [in o]` reserves typed storage; an attached initializer block requests completed construction. C-65/C-72/C-95 and charter §§3.3/4.3/6.3/8 preserve explicit allocation and flow-checked readability. D-22 owns examples/acceptance; preserve established identity versus payload readiness, actual owners, staged sound proofs, no rollback and D-16. The source distinction is confirmed; aggregate algorithms/native support and complete value-path qualification remain separate. Other qualified contracts continue.

- **Owner-parameter symbol integration:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499, covers D-22r22's reference-symbol/owner dependency wording. The source form and staged sound acceptance are confirmed: installation selects one actual lexical or ordinary regional owner from the RHS; an `in @arena` declaration retains its additional selector constraint. C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve explicit dynamic context and lifetime responsibility. D-10 owns examples and acceptance. Preserve saved/captured owner validity, access/null/init, module symbol identity, no lifetime extension and D-16; full proof/physical/value-path qualification and source implementation remain separate. Other qualified scopes continue.

- **Capture/lifetime alignment:** [H-553–H-557](../CHANGELOG.md#h-553)/[K-CAPTURE-LIFETIMES-20261117](../CHANGELOG.md#k-capture-lifetimes-20261117), [D-18-VALUE-CAPTURE](DesignAuthorityRegistry.md#d-18-value-capture)/P-485–P-490 and [D-12-LIFETIME](DesignAuthorityRegistry.md#d-12-lifetime)/P-491–P-493 cover this revision's capture/owner dependency wording. Preserve independent value locals, selected reference targets, explicit var places, unique actual owners and statically bounded borrows; reject implicit slot sharing, closure-cell promotion and lifetime extension. C-36/C-54/C-65/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve visible storage/lifetime responsibility. D-18 owns acceptance and H-327/H-329 alias continuity. D-16, unrelated qualified scopes and existing symbol-declaration restrictions continue. New proof/physical/value-path qualification and source support remain separate; no allocator or graph-copy mechanism follows.

- **Nominal ADT placement:** [H-478](../CHANGELOG.md#h-478)/[K-SINGLE-ADT-20261115](../CHANGELOG.md#k-single-adt-20261115) covers r19→20's alignment with [D-28-ADT](DesignAuthorityRegistry.md#d-28-adt), P-416–P-418. One CoD ADT retains nominal identity across lexical/regional placement; actual owners provide layout/access evidence. The candidate C-10/C-39/C-65/C-91 path serves charter §§3.1/4.3/6.3/7.1/8 through shared business types and checked storage. Preserve same-owner edges, bits classification, initialization, lifetimes and explicit graph transfer; no C++ regional-type stack allocation, capture/TLS extension, graph copy or runtime support is implied. Acceptance and unresolved physical protocols belong to D-28's composed cases. This scoped human decision and direct integration do not qualify the whole document or settle H-146's encoding mechanism.

- **Group-scope continuity:** [K-KNOT-COST-20261115](../CHANGELOG.md#k-knot-cost-20261115) covers r16→17 under H-317/P-284, H-428 and H-444. C-65/C-72 and candidate P-348/P-350 preserve ordered, checked construction and visible effects under charter §§3.3/6.3/8; no value path is newly qualified. Ordinary groups gain no forward data bindings or provisional ref escape; required-ref cycles remain deferred after D-28's cost assessment. Future pure knot-tying belongs to D-19-KNOT. Acceptance and whole-document exclusions remain unchanged; no implementation claim follows. H-447/[K-INIT-CONFIRM-20261115](../CHANGELOG.md#k-init-confirm-20261115) continues r17→18 with the confirmed staged-acceptance plan; it introduces no forward names, rollback or automatic incomplete-object publication.

- **Registry / coverage:** [D-22](DesignAuthorityRegistry.md#d-22), revision25. H-383–H-386 confirms the scoped construction requirements; H-425–H-427 develops RHS normalization and intentional lifetime separation without move semantics. H-428 confirms allocation effects without language checkpoints/rewind. [H-429](../CHANGELOG.md#h-429) establishes distinct regional/lexical storage directions; [D-28](RegionalAndLexicalStorageRefactor.md) owns that refactoring and the proposed C++ external_ptr naming. [H-430](../CHANGELOG.md#h-430) rules out automatic/in-place language growth in favor of consuming replacement; [H-431](../CHANGELOG.md#h-431) selects ordinary owned-parameter failure cleanup. H-432 proposes lossless resize(r, new_cap) for growth and shrinkage. Detailed RHS, capacity/error mapping and resize backend protocols remain under review; P-348–P-350 and whole-document qualification remain pending-human. H-433 sets the lexical pointer-bearing goal; H-478/D-28-ADT selects one nominal ADT across both domains. Bits-only lexical construction remains an implementation fallback, with physical layout/encoding and complete coverage still outstanding. H-435 selects dynamic lexical capacity and owner-base-relative lexical storage independently of the remaining ADT/ABI work. H-436 separates lexical slot location from reference contents and permits register/stack optimization. [K-OWNER-ALLOCATION-20261114](../CHANGELOG.md#k-owner-allocation-20261114) records H-438 allocation through non-const owners and optional const-owner restriction with writable targets, using H-437 shared owner information. [K-REFERENCE-IDENTITY-20261114](../CHANGELOG.md#k-reference-identity-20261114) integrates H-439/H-440 owner-plus-locator identity and is null, preserving established access, result and lifetime behavior. Current bounded source/native coverage is recorded in the [source capability reference](../tests/cod/language/README.md). Placement, allocator transport and whole-document qualification remain separate.
- **Candidate value path:** C-39/C-65/C-72 → C-01/C-06 (P-348) makes storage, access and lifetime intent explicit and checkable in readable source, including distinct lexical/regional construction and explicit initialization effects; C-09/C-54 → C-05 (P-349) separates persistent data from acquisition responsibility and preserves inspectable preparation effects. D-22 organizes these responsibilities through P-350. Charter §§2–5/§7–8 governs those boundaries; §§6/12 requires practical native execution and measured costs. These scoped paths do not qualify the wider ownership or DBMR designs.
- **Dependencies:** P-351: D-01 physical allocation/relative references; D-14 owners, capabilities, fields and qualifiers; D-23 scalar value/view reception; D-17 initialization, failure and cleanup; D-10/D-15/D-18/D-21 dynamic context, values and native dispatch; D-16 bits validity, layout and numeric compatibility. Dependency edges grant no authority.
- **Invariants / non-goals:** no ordinary reference ownership, per-object cleanup, owner-free heap graphs, cross-owner stored edge, implicit graph copy/move, lifetime extension or native callable environment in a regional field. Lexical scopes are not backed by implicit regions; D-28 records H-435 owner-base lexical views and H-478 same-nominal lexical ADTs; physical carrier/layout protocols remain separate. Regional C++ types remain region-only and noncopyable. Actual-owner unions, abstract-result RTTI, fraction execution, pure/TCO and the compiler pilot remain separate.
- **Human baseline:** current project user in [the construction side chat](codex://threads/01a1d051-f608-7c13-b98a-684f065884d7), recorded 2026-11-12 America/Los_Angeles; exact message timestamps/personal name unavailable. H-383 preserves the ownership/allocation corrections; H-384 confirms R1, explicit R2 owner syntax/mismatch rejection and autonomous R3/R4 comparison with final selection reserved; H-385 confirms result-only constness. The [active main chat](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6) subsequently accepts contextual inference and prefers one coordinated migration without compatibility staging (H-386). DBMR protocols, aggregate ABI and final whole-file approval remain outstanding.
- **Acceptance:** the matrix below requires composed positive/rejection cases, exact acquisition/release accounting, relocation and measured compilation/runtime costs. No implementation or new performance result is claimed by this revision.

- **Current implementation alignment:** [K-704](../CHANGELOG.md#k-704) continues r18→19's capability pointer while retaining H-447's staged planning scope. Bounded lexical identity and scalar/caller-slot cleanup evidence does not establish aggregate reservation, publication, mandatory-write contracts or actual-owner allocation transport.

## Confirmed requirements

Only designated intrinsic resource-reference kinds distinguish owned and borrowed; the current kind is the region reference. Ordinary ref/ptr is borrowed, with target provenance, lifetime and access, without independent release or transfer authority. Storage ownership, reference-slot storage and the holder of region cleanup responsibility are distinct. User ADTs cannot contain intrinsic ownership carriers, including through wrappers.

Every ADT instance/storage view has an actual owner. Direct/val bits fields occupy that owner's storage; ref and non-null ptr fields target the same actual owner. Nominal type identity does not identify an owner instance. A bits copy across owners copies neither owner identity nor cleanup responsibility. Ordinary lexical borrows of outer storage do not become stored graph edges.

R1 admits a concrete ADT as bits when its finite fixed representation recursively consists only of bits fields/alternatives/tags, without stored references, ownership carriers, managed environments or implicit cleanup. This includes finite products, sums and qualifying generic instantiations. A one-word reference is still non-bits. An unresolved/abstract type requires sufficient bits evidence; R1 does not choose the source spelling for that constraint, a sum encoding, ABI, packing or validity of arbitrary bit patterns.

Bits ADTs may occupy lexical or regional storage, including a region root. Fresh lexical bits construction belongs to the current full scope; an ordinary grouping block adds no owner. H-478 selects the same nominal pointer-bearing ADT in either domain, with same-owner fields and automatic addressing for its actual placement. Adding a reference field still invalidates dependent bits uses; lexical eligibility does not make the object an independent bits value. D-28 supplies the selected contract and H-438 owner-qualified construction, while exact layout/allocation transport and complete native coverage remain outstanding. Unsupported lexical layouts retain a diagnosed bits-only implementation fallback. No builtin graph-copy extension follows. The current compiler's product-only/scalar-bits subset is an implementation limit, not the language definition.

The ordinary construction direction is `make T [in o]`; intrinsic resource acquisition uses `make region`. Explicit `in o` selects the actual owner and a receiving-variable owner mismatch is a compile error. Equal root types or lifetimes do not establish owner equality. Aliasing preserves provenance and access. RHS normalization below proposes a separate bits-copy path for value/readonly inputs; its interaction with D-23 reception is under review. Under H-438, `where {owner o}` supplies allocation access when supported by at least one writable-target input; `where {const owner o}` forbids allocation and is required for all-readonly inputs. A callable may deliberately select const owner with writable inputs. Generic owner construction does not expose a regional root-policy reference or grant resource ownership.

`make const region` restricts the published result view. Explicit preparation writes, including selected create/initialize/expand/eligible-trim operations, may precede that result subject to permissions and the selected protocol. Constness does not freeze the file, imply a snapshot or remove the owning acquisition's release obligation. Final DBMR grammar, sharing/defaults and operation/failure protocols remain unselected.

Callable payloads retain D-18's complete intrinsic value category and scope-owned backing; they are not bits ADTs or ordinary independently owned references. Local ADT placement alone authorizes no capture or lifetime extension. D-18 admits value snapshots and explicit non-escaping owner-qualified references under H-553–H-556, without promoting lexical cells or copying regional graphs. H-370's deferred owning-argument transfer at invocation and H-365/H-371's scope restrictions remain unchanged.

## Storage eligibility and payload transfer

H-563 prohibits regional storage of callable-view values. H-567/H-568 separately confines whole-payload transfer to pure bits and rejects a C++ object copy/move model. These are independent checks: an ordinary reference-bearing graph may be regional and zero-cost relocatable without being transferable as a whole value; a callable view may have an intrinsic lexical construction contract without being bits. Initialization and bare reservation do not override either check.

| Operation | Contract retained |
| --- | --- |
| Pure bits payload transfer | Require initialized valid bits and the ordinary destination/type/access rules. It copies no object identity, owner or cleanup responsibility. |
| Reference binding | Retain a selected target/actual owner; do not duplicate the referent. |
| Callable-view construction/capture | Use D-18's dedicated complete-environment and borrow rules. This supplies no generic aggregate-copy operation. |
| Whole-region ownership transfer | Consume/pass release responsibility under the explicit owned contract; regional objects remain the same graph. |
| Graph transfer | Explicit destination construction and graph mapping under the selected library contract; no implicit copy/move or automatically chosen policy. |

A regional field, array or generic compound may not contain callable-view values. Proving a field's bytes relocatable is insufficient. H-565/H-566's lexical-only callable-bearing ADT proposal below is still a separate source choice; the fixed regional restriction is not a blanket settled prohibition on every lexical ADT field. D-16 bits validity and existing no-owning-resource-in-user-ADT constraints continue.

## Anonymous data and lexical compound results — preferred direction under review

H-565 proposes allowing callable fields in lexical-only ADTs, using storage categories such as pure bits, relocatable-reference graphs and callable-bearing lexical data. H-566 prefers anonymous `data { ... }` as a named-product mechanism, with name-based unpacking, over introducing a separate positional-only tuple type if engineering cost is comparable. H-569 identifies convenient partial-field receiving as an important UX requirement and proposes structural typing for anonymous data. This is the preferred direction being assessed, not yet a complete source/ABI/identity contract.

The suggested classification is inferred from the instantiated fields, separate from nominal identity and operation eligibility:

| Candidate storage category | Eligible placement | Whole-payload transfer |
| --- | --- | --- |
| Pure bits product/sum | Lexical or regional under existing rules. | Ordinary bits transfer only, with valid initialized payload. |
| Reference-bearing graph without callable values | Lexical or regional when its same-owner/layout contracts permit. | Not admitted; use existing references or explicit graph transfer/construction. |
| Compound containing callable-view values | Lexical only if the compound-field extension is admitted. | Not admitted; use explicit construction and each constituent's existing contract. |

“Storage category” here is a checked property of an existing data type, not a new nominal type per placement or a choice to change the charter's kind/type hierarchy. A lexical-only compound may also contain ordinary graph references; calling it callable-bearing does not make the whole compound safe to copy. Nested products, sums, arrays and generic field types propagate the relevant restrictions. Same-owner reference edges must not lead to otherwise prohibited regional callable storage. A finite fixed-point classification can cover recursive type graphs; unknown/infinite-layout cases need their ordinary diagnostics, not a runtime type scan or owner union.

```cod
# Candidate named and anonymous shapes; aggregate support remains incomplete.
data Result { count:u64; action:proc()->u64; }
data { count:u64; action:proc()->u64; }
```

Named and anonymous products should reuse field layout, owner/initialization evidence, placement eligibility and field access. A product containing only bits need not become lexical-only merely because it is anonymous; any proposed such restriction would require its own rationale. H-478's one-nominal-ADT rule continues for each eligible placement. Anonymous syntax does not silently make existing named ADTs structurally identical.

### Structural anonymous data and partial named reception

H-569 makes partial named reception a central requirement rather than a later convenience. The current engineering recommendation is structural rules for explicitly anonymous products while keeping named ADTs nominal. Two named declarations do not become equal because their field layouts match. Anonymous comparison must use resolved field/type/mode/access and owner/capture contracts, not spelling alone or physical bytes; the exact identity and field-order policy remains under review.

Partial unpacking can be implemented independently of general structural subtyping. A receiver selects a subset of known field names and need not bind the rest. Evaluate the producer once; selecting two fields must not call it twice. An omitted field is not read or duplicated merely to unpack the selected fields. Producing the aggregate still performs its specified initialization/effects and retains required cleanup for all constructed fields, including omitted ones. Receiving order may be expressed by name without relying on positional tuple order; preserve the existing preparation/publication rules rather than sorting observable evaluation.

Selected bits are transferred as bits, reference receivers preserve the selected target and actual owner, explicit place aliases retain their declared place, and callable receivers use the dedicated intrinsic construction/binding contract. No selection converts a non-bits aggregate into a whole-value copy or silently prolongs a temporary/borrowed owner. Required names must exist and meet the receiving type/access/owner contract; omitted extra fields alone do not make a partial pattern fail.

This UX requirement does not silently introduce width/depth subtyping for arbitrary mutable records. A projected structural reference, if later admitted, needs correct field-layout evidence, owner/access preservation and writable-field type soundness. Fixed-name unpacking itself needs no runtime string map, open-row polymorphism or new subtype relation. These distinctions keep the initial implementation bounded while leaving the richer type policy explicit.

### Results and unpacking without object copying

A non-bits result must be constructed in receiver-owned storage or returned as a reference with a valid explicit owner contract. Returning an already allocated callee-local compound cannot be legalized by implicit whole-object copying, moving, graph transfer or lifetime promotion. Existing contextual make ownership and callable caller destinations supply integration precedents, not a complete aggregate result contract. Result typing, availability of the destination owner/capability, retained result cleanup and exceptional paths still need a coherent source contract.

Name-based unpacking should evaluate the producer once, select fields by their declared names and prepare each destination according to its own bits/reference/callable initialization contract. It must preserve initialization-group publication and allowed RHS effects, without inventing whole-compound copying, positional tuple semantics or hidden declared place aliases. A non-bits field can only be bound or constructed through an operation already valid for that field; unpacking does not manufacture one. Partial field selection is required by H-569; exact receiving syntax and its integration with result/owner contracts remain under review.

Callable-bearing compounds must retain the external owner dependencies of each view through construction, field access, return and unpacking. The containing lexical scope cannot extend those owners. Ordinary ref/ptr fields retain their same-owner rule; a named product is not automatically a cross-owner view record. Intrinsic owning resources do not become ordinary ADT fields through this proposal.

### Engineering assessment and unresolved identity contract

Anonymous data reusing the common aggregate machinery appears to add bounded grammar/type-registration/field-name work, rather than a separate backend. A positional tuple subsystem would still need constituent layout, initialization, result transport, cleanup and borrowed-owner proof. Thus avoiding a separate tuple category is a plausible lower-debt direction; total implementation is substantial because complete aggregate support is not already available. No numerical effort estimate or native acceptance claim is established.

Current source inspection at `7701bc7` finds named data field parsing in `cod/lang_syntax.hh`, while `cod/lang_checked.hh`'s `lexical_value_type` still admits only the supported bits and procedure categories. General non-exception field operations have implementation gaps. `cod/lang_llvm.hh::caller_destination` currently selects non-borrowed procedure results; it is not already a general compound-result destination. Reusing that architecture requires real type/place/owner and cleanup integration, not removing one rejection. The mainline developer independently reviewed the current source at main `2a385f2a1f4818ddebe964efb6c4303f3035c00a` and reached the same bounded conclusion: fixed-name anonymous products need no separate backend and have no identified dramatic marginal cost relative to tuples with equivalent field/result semantics. Both alternatives owe substantial aggregate construction, cleanup and destination/borrow proof. Name-based selection can resolve labels statically without a runtime string map. Structural versus per-site nominal identity remains a real semantic choice; initializer order cannot be changed by canonicalizing field keys. This engineering assessment selects no layout, identity policy, new source implementation or complete value path.

Anonymous structural typing is the preferred candidate under H-569; the material identity choice is the exact anonymous shape/compatibility rule rather than an implicit change to named nominal types. A per-site nominal alternative would reuse existing machinery more directly but would affect same-T and equal-looking product interchange. Structural identity needs rules for field names/order, declared modes, resolved child type identities and owner/capture contracts; physical layout equality is insufficient. Name-based unpacking alone does not decide this question. Exact construction/result/unpacking syntax, compound-field eligibility and full value-path/physical qualification also remain under review. These are source-visible choices, not reasons to revive C++ copy/move semantics.

## Bits classification and representation obligations

The proposed compiler separation is nominal type identity → resolved representation facts → placement/view → native transport. Classification must not replace nominal identity, actual-owner facts or abstract dispatch evidence.

| Obligation | Proposed integration / unresolved boundary |
| --- | --- |
| Concrete product | Check each instantiated direct field recursively; reject a ref/ptr or other non-bits constituent. Cache by resolved type identity and arguments, not source alias or actual owner. |
| Finite sum | Check all payloads and tags for finite bits representation. Preserve active-alternative validity. Concrete source/tag/layout work remains required; no tagged-union ABI is selected here. |
| Generic or recursive type | Distinguish unresolved evidence from proved non-bits. Detect infinite by-value expansion; pointer recursion is non-bits rather than a recursion escape in the classifier. Source constraints and diagnostics need an explicit contract. |
| Storage and field access | Address lexical and regional payloads through a common place abstraction retaining owner, access and initialization facts. Relative stored references keep D-01 encoding; a temporary native address is not a persisted CoD edge. |
| Payload transfer and borrowing | Only pure bits permit whole-payload transfer, requiring initialized valid contents. Reference binding retains a target/actual owner and does not duplicate the referent. Field readiness does not create generic object copy/move semantics. |
| Layout and native calls | Separate aggregate payload layout from reference transport. Preserve existing regional offsets; choose local aggregate argument/result transport with recursive callable and AOT requirements visible. No ABI choice follows from adding a classifier predicate. |
| Numeric validity | Bits eligibility promises no zero-validity or numeric conversion. D-16 fractions retain fixed payload/nonzero denominator and representation-independent UoM; array SoA/layout policy remains separate. |

The implementation dependency order is representation facts, aggregate places/initialization, payload operations, construction, then parameter/result/capture/TLS integration. Each increment must name incomplete admitted forms rather than present them as forbidden language. Classifying bits ADTs is not permission to make C++ `regional_*` objects stack-copyable.

## Owner-directed construction

The proposed compiler check for the confirmed construction behavior carries the resolved target type, actual destination owner, allocation capability, ordered field initializers and result access. Source forms feed that check; ordinary storage construction and intrinsic acquisition have distinct cleanup responsibility. This is a compiler organization proposal, not a new runtime object or universal allocator interface.

| Source situation | Required behavior and implementation boundary |
| --- | --- |
| Explicit `make T in r` | Use r only if it supplies the required placement/capability. Reject a conflicting receiver owner before runtime evaluation. H-384 confirms explicit selection and mismatch rejection; H-438 supplies the generic owner allocation-access rule. |
| Omitted owner, receiver has one unique owner and suitable allocation access | Construct there. A field may supply its containing owner to an explicitly written nested make. H-386 accepts this contextual inference. |
| Omitted owner, no destination | Bits use the current full lexical scope. Non-bits still require an explicit actual owner when none is supplied by context; H-478 admits `in here` as well as a regional destination. No TLS fallback, hidden region or automatic owner choice follows. H-386's explicitness boundary remains; missing capability never causes a hidden allocator or fallback. |
| Receiver selects a const owner | Reject construction in that owner, including a fresh readonly result or otherwise admitted implicit target materialization. Do not silently choose another owner. |
| Receiver selects a non-const owner | H-438 permits construction using that actual owner; retain placement, policy, capacity, initialization and failure checks. |
| Initializer retains a reference result | Check actual owner/access. A non-bits graph is never implicitly copied; value/readonly bits normalization is reviewed below. |
| Ref/ptr field initialized without explicit make | Use the proposed RHS classification below: value/readonly bits materialize a target; retained views alias. Regional materialization remains under review. |

Illustrative candidate source, not current parser acceptance:

```text
ref p:Point = make Point { x <- 1; y <- 2 };
ref n:Node in r = make Node in r { ... };
ref bad:Node in r = make Node in s { ... }; # Reject distinct actual owners.
owned region arena:Root <- make region Root(capacity = bytes) { ... };
```

Completed construction must preserve initializer order and effects and publish only a valid initialized result. Bare typed reservation has the separate identity-only contract below. Existing D-17 failed acquisition cleanup does not promise rollback of effects or arbitrary reservations in an already-live region. Nested constructor naming/access, partial construction and allocator-policy interaction require review before implementing an uncovered case. Explicit owner spelling cannot bypass readonly allocation checks or same-owner stored edges.

## Typed reservation and completed construction

H-562 fixes the distinction through the existing initializer-block syntax; no extra allocation keyword is needed:

| Form | Completion contract |
| --- | --- |
| Bare `make T [in o]` | Reserve suitably typed storage with a selected owner and a non-null target identity on successful allocation. Establish no payload initialization merely from allocation. |
| `make T [in o] { ... }` | Reserve and initialize storage, then provide a completed result. An empty block still requests completion and must satisfy the type's initialization requirements. |

This distinction concerns the bare ordinary allocation form. Existing constructor-argument calls and intrinsic resource/exception construction retain their own completion contracts; absence of a trailing block does not reinterpret `make region` or an exception constructor as incomplete raw storage. Type layout and owner allocation capability must be established before reservation. Physical zero-filled bytes are not evidence of initialized fields or valid typed values.

A ptr may retain a reserved target's identity under the existing initialization-flow rules. Fresh allocation supplies non-nullness, not readable payload. Projection of a known field destination, reference-identity assignment and `is` observations require their own owner/layout/reference-slot facts but need not read the target payload. Ordinary readable-ref uses, payload copies, captures or calls cannot erase outstanding initialization restrictions.

```cod
data Node { value:u64; ref next:Node; }
# r is an existing live writable owner; design acceptance, not compiler coverage.
ptr a:Node in r = make Node in r;
ptr b:Node in r = make Node in r;
a.value <- 1u64;
b.value <- 2u64;
a.next = b;
b.next = a;
ref ready:Node in r = a;  # Allowed once the complete component is proved ready.
```

The two reference-field assignments establish identities and dependency edges; they do not read the other node's payload. A finite required-reference cycle can become readable when every required field and edge in the tracked component is established. Merely closing a cycle proves no missing bits initialized. Before that point, initialization restrictions follow the targets through local flow and calls under D-28; ordinary readable publication rejects. This is the already selected staged proof model, not an implicit graph copier, runtime readiness tag or lazy evaluation mechanism.

Reservation is an allowed allocation side effect. Abandoning an unread incomplete target need not force a pointless write; its storage remains governed by its actual owner. Failure promises no allocation rewind in a live region, and lexical/whole-region cleanup retains its existing obligations. The distinction does not enlarge a type's storage eligibility: callable-view values remain excluded from regional storage, and D-16 fraction reads require a valid initialized payload, not merely allocated component bytes.

Acceptance covers bare reservation versus empty/nonempty initializer blocks, non-null identity before payload readiness, rejected early reads/ref publication, sequential and branched field initialization, the finite two-node component, and failure/abandonment without rollback. The same source distinction applies to eligible lexical and regional storage. Common proved cases may be implemented first; incomplete aggregate support or unknown lifetime/readiness facts must reject explicitly. Exact field/proof algorithms, qualifier combinations and broader publication precision remain separate.

## Unified lexical and regional initialization — proposal

The proposed common model is: select an owner with construction access, establish storage, initialize it, and establish the completed group bindings. Allocation is an allowed initialization side effect under H-428. P-348/P-350's candidate value is explicit allocation and lifetime policy in readable source, while charter §§3.3/4.3/6.3/7.2 requires preserved sharing, effects, access and failure behavior. H-426/H-427 proposes new RHS policies; affected D-14/D-23 reception and explicit D-01 region-resize extensions need the scoped review below. D-28 owns the distinct storage/representation refactoring. H-428 preserves D-17's separation of initialization effects from whole-region cleanup. Source examples illustrate design, not current compiler acceptance.

### RHS categories, grouping and value normalization

For copyable bits, the proposed initialization rule is determined by the checked RHS category, independently of whether source and destination owners happen to agree:

| RHS category | Ref/ptr initialization with `=` |
| --- | --- |
| Independent val bits, including a scalar value-result call | Allocate a fresh destination in the selected owner and initialize its payload. |
| Readonly target view of initialized bits | Read the bits into a fresh destination; do not grant write access to the source. This proposed copy default changes D-23's general existing-view initializer rule. |
| Mutable target view | Alias the target, preserving actual owner/type/access and checking destination compatibility. |
| Non-bits object view | Preserve the regional target and access; readonly access supplies no graph-copy permission. |
| Nullable/absent pointer view | Preserve pointer absence/provenance under D-14. Neither null nor an ordinary pointer view silently becomes a bits payload read. |

Here readonly describes target access (`ref r:const T`), not declaration-leading const (`const ref r:T`), which only restricts rebinding the reference slot. A const binding may still expose a writable target. Copy eligibility does not depend on a machine representation being one word: ordinary references, intrinsic ownership carriers and callable environments retain their separate categories.

H-427 permits grouping to yield a value. **Proposed precise scalar rule:** `(x)` groups the expression and projects readable bits to an independent val result. Nested grouping is idempotent with respect to value category; it performs no ownership transfer, invalidation or move. A bare addressable storage expression still supplies its place/view, including when the storage was declared val; the declaration's storage kind and the expression's result category are distinct. This last spelling rule is part of the proposal's concrete review scope, not inferred from C++ terminology.

```text
data Box { ref n:u64; }
# r supplies live allocation access.
val local:u64 <- 7u64;
ref a:Box in r = make Box in r { n = (local); }; # Fresh copied bits in r.
ref b:Box in r = make Box in r { n = a.n; };     # Mutable view: alias a.n.
ref c:Box in r = make Box in r { n = (a.n); };   # Fresh copied bits, even in r.
```

A `val T in O` remains invalid. Fresh target placement comes from the receiver/containing object or the full-scope default, with usable allocation access; the copied value carries no owner. No owner-mismatch heuristic selects copying. Independent targets remain distinct even when their bits compare equal. Direct/val fields use `<-` to initialize inline payloads, without a separate target. Ref declaration `<-` stays rejected; later `=` rebinding and `<-` payload writes retain their separate contracts. D-14's ptr transfer-declaration shorthand is not silently extended or removed.

Grouping now has observable category consequences: `consume(x)` versus `consume((x))` may select different admissible access/parameter behavior, and `(x) <- value` cannot write through an independent bits result. Assignment-result views must be checked through grouping without changing the operator's own result contract. Retaining a readonly scalar alias needs a clearly specified borrow/alias path; the existing ordinary-place alias can name an existing place but is not a general syntax for retaining an arbitrary returned view. Non-bits grouping must retain a view without copy/move; whether it narrows access remains a separate precise rule. Complete callable values retain D-18 rather than being reclassified as scalar bits. These are outstanding integration cases, not new operators or ABI choices.

### Conditional results

The proposed scalar conditional computes one result category before receiving initialization. For a mix of a view and val bits, normalize the selected branch to val bits; then materialize one receiving target. Do not branch between aliasing and allocating implicitly:

```text
ref chosen:u64 in r = if (reuse) { existing } else { 7u64 };
# Either path initializes a fresh target; writing chosen does not change existing.
```

Only the selected branch evaluates. Any required bits read occurs while that source is still live, before its producing scope/cleanup ends. A value join can combine readable bits from different owners because no reference escapes; a readonly-reference join still needs one statically provable owner. Same-owner compatible mutable-view alternatives may retain their reference category when no value normalization is required. Non-bits alternatives require the existing compatible view/owner rules, not implicit graph cloning. Numeric joins use existing permitted conversions and preserve units/fraction constraints; none of this changes an already established callable result signature.

### Initialization order and allowed allocation effects

H-427's joint initialization establishes the group bindings on success; H-428 explicitly permits allocation as an initialization side effect. There is no CoD allocation checkpoint, automatic cursor rewind or implied transaction over initializer effects. Initializers run in source order and evaluate once. For a fresh target, H-416's conceptual order remains reserve, evaluate/convert, initialize; failure to reserve may prevent RHS effects. Earlier bindings are available to later initializers and the final group block. Lookup in an individual initializer still uses the preceding lexical environment. Owner liveness and allocation access must survive every pending write.

Joint initialization is not recursive data binding. Ordinary proc/func groups do not expose later targets or incomplete objects as readable refs. H-317's nullable-edge route remains the current ordinary contract. H-447 confirms [bounded common-case acceptance followed by demand-driven proof precision](RegionalAndLexicalStorageRefactor.md#two-stage-acceptance-with-invariant-soundness), including finite cycles when aggregate and completion prerequisites are ready; this does not change ordinary group lookup. Future pure knot-tying is a separate [D-19 research goal](ConstFunctionsAndTailCalls.md#knot-tying-as-a-pure-design-goal), not a change to the ordering or effects rules here.

If a group fails, its incomplete bindings are not made available as successfully initialized results. Allocations already made in an existing live region remain governed by that region, and earlier valid writes/effects remain. An initializer can publish a fully initialized target into pre-existing same-owner state; a subsequent failure does not reclaim that target behind the surviving reference. Merely allocated or partly initialized targets still cannot be exposed as readable valid objects.

```text
data Holder { ptr saved:u64; }
proc publish(x:u64 in r, holder:Holder in r) -> u64 where {owner r} {
    holder.saved = x;
    return 0u64;
}
# holder predates this group; fail() throws; r remains live in the caller.
ref a:u64 in r = 7u64,
    b:u64 in r = publish(a, holder),
    c:u64 in r = fail();
# After catching the failure, holder.saved still targets valid initialized 7u64.
# Allocation and publication were effects; no group rewind invalidates them.
```

D-17's cleanup remains category-specific. A newly acquired whole region that fails before successful acquisition publication releases its resources exactly once. Existing-region construction does not gain that whole-region cleanup responsibility. Scope exit, deferred work and completion replacement retain their existing rules; grouping does not invent an additional defer scope. Neither earlier existing-state writes nor I/O are implicitly undone.

Applications and third-party libraries may provide allocation-undo frameworks with explicit ownership, lifetime and publication contracts, for example constructing in an independently owned temporary region. The language's ordinary validity rules still apply; the possibility of such a framework supplies no general permission to rewind storage behind live references, graph-copy between regions or detach a scope owner. No new standard rollback API or allocator mechanism is selected here.

## Regional and lexical storage boundaries

[Regional and lexical storage refactoring](RegionalAndLexicalStorageRefactor.md) owns the current direction: a regional stored graph uses relative/local references, while H-435/H-436 supports dynamic lexical capacity with separate storage locators and reference contents. A materialized lexical slot is located by its storage-scope offset; same-scope ref/ptr contents use that scope's offset convention, while other-owner contents retain their target-owner interpretation. Register/stack allocation may optimize this model when observable semantics are preserved. A lexical scope is a lifetime owner, not an implicit region. A view's binding lifetime and the referenced payload's owner remain distinct; libraries/frameworks may exploit that separation to decouple memory management from business processing.

Construction can share an owner/access/initialization model across those domains without sharing their allocator or reference encoding. Current full-scope and ordinary-grouping boundaries retain D-17. Lexical bits construction remains available; H-478 selects reference-bearing lexical placement under D-28's one-ADT contract, whose physical layout and complete native coverage remain outstanding. No scope-region resource, lexical region-capacity hint, automatic capture permission or new outer-selector syntax is implied.

### Lexical ADT construction integration

The [integrated lexical ADT review in D-28](RegionalAndLexicalStorageRefactor.md#lexical-adts-as-part-of-the-complete-model) records H-478's selected one nominal ADT with placement-specific layout/access evidence. Same-owner fields make a bounded lexical graph model possible without first admitting cross-owner view records. Pointer fields remain non-bits even when represented by integer offsets. Required ref initialization, nullable cycles, array/inline layout and partial publication need composed proof/representation coverage; bits-only lexical support remains an implementation fallback.

[Owner-qualified construction in D-28](RegionalAndLexicalStorageRefactor.md#owner-qualified-construction) selects the common permission for caller-owned and current-scope storage. A writable-target parameter permits owner O; const owner O remains available as an explicit no-allocation contract even with writable targets. All-readonly parameters permit only const owner O. Allocation uses the actual owner's path carried through shared runtime information, without an extra builder parameter merely to request permission. A callee's own storage cannot stand in for caller-owned graph nodes.

Traversal remains in selected library `(<-)` methods, using H-439/[D-28 identity contract](RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator) for owner-plus-locator identity and graph memoization support. Libraries retain sharing/cycle policy and failure expectations; ADTs need no mandatory application ID field merely to distinguish reference targets. H-437 metadata sharing preserves each H-438 qualified access path; it neither merges permissions nor supplies a default graph-copy algorithm. H-478 settles nominal identity and lexical placement; physical allocator transport and concrete layouts remain separately reviewed. H-428 effects, D-16 validity, register/stack optimization and existing capture/TLS boundaries remain unchanged.

### Explicit consuming region resize

H-430 selects explicit replacement rather than automatic or in-place capacity growth at the CoD language level. Ordinary allocation uses the region's established capacity and retains D-17's unchecked capacity-exhaustion behavior. Initial capacity is not permission to grow implicitly or a promise that failed initializers will be retried. Lexical scopes have no region-capacity policy.

H-432 proposes a unified intrinsic with an explicit capacity argument. The current candidate shape is:

```text
func resize(r:owned region:R, new_cap:u64) -> owned region:R where {R}
```

[The resize contract in D-28](RegionalAndLexicalStorageRefactor.md#explicit-resize-consumes-and-replaces-the-region) owns the replacement/lifetime boundary. The input's cleanup responsibility transfers at callee entry after argument evaluation/conversion. Success returns ownership of the resized replacement; old direct/external views do not follow it. H-431 explicitly retains ordinary cleanup if failure occurs after entry: release what the callee still owns, with no input ownership restored to the caller. Failure before entry has not transferred the input merely because a call expression began evaluating.

The result preserves root type and graph contents/sharing/cycles without exposing two independently owned copies or adding CoD object move/copy semantics. D-28 recommends new_cap as an exact payload byte budget under D-17 E5, lossless shrink only down to the retained allocated extent, unchanged offsets and a uniform consuming boundary even for equal capacity. Too-small requests throw instead of truncating or compacting; post-entry rejection retains H-431 cleanup, not input preservation. Exact source grammar, capacity/error integration and backend/mapping/file protocols remain under review. Physical relocation and allocator strategy must satisfy the replacement contract; no observable stable-address or automatic-borrow-refresh promise follows.

Acquisition-time file preparation and independently shared mappings retain separate DBMR protocols. An owned handle conveys cleanup responsibility, not proof that no other mapping or actor can observe the backing file. The explicit intrinsic does not silently authorize shared-file resize, flush/durability behavior, allocator switching or read-only access escalation.

### TLS and numeric boundaries

H-426 retains inline TLS payloads for bits ADTs and the separately defined callable-view category. Explicit symbolic references require an owner contract and valid access/lifetime evidence. D-10/H-561 permits an owner parameter instantiated from a proved lexical or ordinary regional target; the region-selector form keeps its separate constraint. TLS holds a selected reference rather than an inline non-bits object. The lexical owner-base representation supplies no lifetime extension. Intrinsic region-resource bindings retain their ownership/restoration role; no generic ADT can carry an intrinsic owning resource.

D-16 fractions remain fixed valid bits with nonzero denominators. Storage representation, grouping/copying and initialization effects introduce no all-zero default, hidden canonicalization, numeric widening, component-validity relaxation or fraction implementation. General bits-ADT copying and callable transport still need their own representation/ABI integration.

## Source migration

H-386 selects one coordinated cutover after this design and the required representation integration are qualified. Use one semantic construction model: ordinary `make T [in o]` is distinct from whole-region `make region Root(...)`; a receiving variable’s `owned` qualifier cannot silently select allocation.

Migrate live fixtures, generated programs and examples together, and diagnose unsupported spellings with an actionable replacement. There is no compatibility period or permanent parallel syntax. This direction does not settle aggregate ABI or DBMR protocols.

## DBMR alternatives and protocol work

These are comparison candidates. Both preserve confirmed result-only constness and keep acquisition preparation, explicit consuming resize, compaction and snapshot publication distinct.

| Criterion | A: clauses on acquisition | B: typed acquisition request |
| --- | --- | --- |
| Illustrative surface | `make const region Root from existing path expand to bytes` | `make const region Root from existing_file(path, expand_to = bytes)` |
| Source intent | Source and capacity effects visible at the acquisition site; diagnostics can underline conflicting clauses. | Named request operations compose source/options; reusable helpers can describe acquisition without acquiring ownership themselves. |
| Ordinary use | Few concepts for one acquisition; modifier combinations need a finite grammar with explicit defaults. | Familiar call arguments and named options; users must distinguish request construction from acquisition and result access. |
| Complexity | Dedicated syntax lowered once into a checked plan. | Request typing, helper evaluation and admissible dynamic choices need a contract; a user-defined record must not contain an owning region handle. |
| Native/runtime cost | Static options can choose one native acquisition path. | Equivalent static options can select the same path; runtime-selected options would require checking/branches, without inherently requiring another mapping or heap object. |
| Failure diagnosis | Acquisition errors point at the source/capacity clause. | Must retain origins through helper-produced requests to diagnose the failed acquisition. |

**Provisional recommendation: A** for the initial explicit acquisition surface. It introduces less request-value machinery and keeps persistent-file effects visible. B is useful if real applications need reusable runtime-selected acquisition descriptions. No latency/code-size benchmark supports a performance preference yet; both can lower to the same acquisition operations. This is not a final R3 selection or frozen syntax.

The operation matrix below applies to either candidate and is still proposed. A source-form preference alone cannot qualify these file protocols.

| Situation | Proposed outcome / contract still needed |
| --- | --- |
| New source, destination exists | Reject without overwriting; creation must be exclusive rather than an existence-check/open race. |
| Existing source missing, invalid or incompatible | Reject; no silent create, rebuild or clear. Specify root/layout/reference validation and failure types. |
| Const result with requested preparation writes | Obtain authorized preparation access, perform specified writes, then publish the restricted result. Decide physical mapping protection independently of the already-confirmed language access rule. |
| Expansion/trim request already satisfied | Specify exact-size versus minimum-capacity semantics, total/payload/free-byte units, limits and idempotence; do not inherit the C++ reserve-free API accidentally. |
| Trim intersects occupied storage | Reject. Tail trim preserves retained offsets and is not compaction. Validate under the selected coordination boundary. |
| Partial acquisition/preparation failure | Release every acquired handle/mapping exactly once; specify remaining file contents/extent. File rollback is not supplied by region cleanup. |
| Shared mapping and preparation/replacement | Define admissible access and coordination. H-430 excludes ordinary in-place language growth; acquisition preparation or a consuming replacement still needs a backing-file/shared-mapping protocol. Shared mapping supplies neither ownership exclusivity, locking nor transactions. |
| Release/flush failure | Define explicit observable failure and cleanup behavior; closing an acquisition does not delete data or guarantee durable publication. |

Before final R3 selection, compare a concrete capacity/failure/coordination contract and its costs against the existing primitives. Include a state-by-state failure table, access requirements and publication point. Existing snapshot save/replace (C-78) and compatibility-marker planning (C-77) remain separate inputs, not completed protocols.

## Implementation dependencies

The [source capability reference](../tests/cod/language/README.md) owns current compiler support. Construction integration must keep syntax, payload placement, field access, owner/access/initialization proofs, callable transport and cleanup consistent. A bits predicate alone cannot establish aggregate transport or lexical field-place support.

[DBMR](../include/shilos/dbmr.hh) acquisition must be assessed against the proposed exclusive creation, validation, capacity, access, failure and coordination contract. Existing storage primitives do not establish full graph/schema validation, failure atomicity or durable publication. Measurements must distinguish counted operations from observed latency and resource totals.

[K-RESIZE-20261114](../CHANGELOG.md#k-resize-20261114) records the current proposal review; whole-file and unresolved construction/acquisition qualification remain pending-human.

## Acceptance and decisions before implementation

| Scope | Required evidence |
| --- | --- |
| Bits and owners | Local/regional nested products, finite sums and generic instances; copies independent across owners; aliases retain owner/access; reference/owner/callable-bearing types fail bits classification; invalid local escape and cross-owner edge reject. |
| Owner allocation access | Mutable owner permits allocation; const owner forbids direct/indirect allocation while preserving declared target writes. Cover all-readonly, mixed, nullable and binding-const inputs; metadata sharing cannot upgrade access. |
| Construction | Explicit owner agrees with receiver; equal root type never hides distinct owners; selected inference requires usable capability; existing borrows retain provenance; once-only ordered initialization, premature-read rejection and partial-failure cleanup. |
| Unified initialization proposal | Compare bare places, grouped val results, readonly bits, retained mutable/non-bits views and null across declarations/fields. Check normalized conditional identity, live source reads before cleanup, grouping/dispatch effects, const-binding separation, explicit readonly aliases and unchanged later assignment. |
| Initialization effects | A later initializer failure preserves earlier valid publications and allocations in an existing live region; incomplete bindings/readable objects are not exposed. Preserve source order, exact cleanup for failed whole-region acquisition, nested effects and existing defer/completion rules. No implicit allocation or graph rollback. |
| Distinct storage and explicit resize | Use D-28 placement/representation acceptance. Cover grow/shrink/equal-capacity replacement, lossless retained extent, pre-entry failure, consuming entry, old-view invalidation, fresh result views, post-entry rejection/failure cleanup and abandoned owning results. No automatic growth, ownership restoration or scope-region backing; capacity/backend details need their own acceptance. |
| Composition | Aggregate arguments/results through native recursion, callable captures/dynamic bindings, deferred execution and loops preserve value/view semantics and existing ownership transfer timing. |
| File acquisition | Selected source/access/capacity operation matrix, invalid image rejection, exact handle/mapping release, relocation preserving sharing/cycles and observable preparation effects before const publication. |
| Costs | Compile time, generated text/bitcode, stack/backing bytes, region/mapping count and peak extent; no accidental per-object cleanup or graph copy. No numeric target is assumed. |

## Review process and remaining topics

Discuss unresolved design topics in terms of source examples, behavior, alternatives and consequences. Registry identifiers are internal traceability, not items the user must approve individually. Once the topics are resolved, present the complete revised document for approval. This revision records the discussion decisions; it does not claim final approval or ask the user to approve competing alternatives.

Contextual owner inference and the coordinated migration direction are recorded above and need no repeated preference question. The remaining topics are:

- **Aggregate values and native calls:** define concrete product/sum representation, sufficient generic bits evidence, initialization granularity and parameter/result transport while preserving nominal identity, relative graphs, complete callable values and fraction validity. Distinguish internal compiler choices from observable layout or validity commitments.
- **RHS normalization:** complete bare-place/grouped-value semantics, readonly alias reception, non-bits grouping and operator/parameter interactions. Conditional bits normalize to a value; no implicit move or graph copy follows.
- **Initialization integration:** preserve H-428 allowed allocation effects and D-17 cleanup while completing precise RHS/name/order checking. No checkpoint or publication-rollback choice remains open at the language level.
- **Storage refactoring and explicit-region resize:** D-28 owns lexical/regional reference representation, any lexical non-bits layout and external_ptr naming. H-430–H-432 supplies explicit consuming resize with lossless success and ordinary failure cleanup; review the capacity/error mapping and DBMR backing protocols.
- **Persistent acquisition:** complete the source/access/capacity/failure/coordination matrix and compare its implementations before choosing the DBMR surface. Result-only constness is already confirmed; mapping protection, capacity units, partial effects, validation and release/durability outcomes remain explicit topics.

The candidate value rationale, constraints, non-goals and acceptance remain part of the eventual whole-document review. No separate approval of internal relationship numbers is requested.
