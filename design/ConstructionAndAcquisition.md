# CoD construction, bits ADTs and region acquisition

Design revision: 12 · Recorded: 2026-11-14 · Status: discussion draft with confirmed requirements; consuming resize direction recorded; RHS normalization and capacity/backend details under review; whole-document approval pending; implementation incomplete.

## Design authority

- **Registry / coverage:** [D-22](DesignAuthorityRegistry.md#d-22), revision12. H-383–H-386 confirms the scoped construction requirements; H-425–H-427 develops RHS normalization and intentional lifetime separation without move semantics. H-428 confirms allocation effects without language checkpoints/rewind. [H-429](../CHANGELOG.md#h-429) establishes distinct regional/lexical storage directions; [D-28](RegionalAndLexicalStorageRefactor.md) owns that refactoring and the proposed C++ external_ptr naming. [H-430](../CHANGELOG.md#h-430) rules out automatic/in-place language growth in favor of consuming replacement; [H-431](../CHANGELOG.md#h-431) selects ordinary owned-parameter failure cleanup. H-432 proposes lossless resize(r, new_cap) for growth and shrinkage. Detailed RHS, capacity/error mapping and resize backend protocols remain under review; P-348–P-350 and whole-document qualification remain pending-human. H-433 sets lexical pointer-bearing ADTs as the target with lexical bits-only allocation as fallback; D-28 owns its feasibility/UX evaluation. H-435 selects dynamic lexical capacity and owner-base-relative lexical storage independently of the remaining ADT/ABI work. H-436 separates lexical slot location from reference contents and permits register/stack optimization. [K-LEXICAL-SLOT-PAYLOAD-20261114](../CHANGELOG.md#k-lexical-slot-payload-20261114) records the clarified scope.
- **Candidate value path:** C-39/C-65/C-72 → C-01/C-06 (P-348) makes storage, access and lifetime intent explicit and checkable in readable source, including distinct lexical/regional construction and explicit initialization effects; C-09/C-54 → C-05 (P-349) separates persistent data from acquisition responsibility and preserves inspectable preparation effects. D-22 organizes these responsibilities through P-350. Charter §§2–5/§7–8 governs those boundaries; §§6/12 requires practical native execution and measured costs. These scoped paths do not qualify the wider ownership or DBMR designs.
- **Dependencies:** P-351: D-01 physical allocation/relative references; D-14 owners, capabilities, fields and qualifiers; D-23 scalar value/view reception; D-17 initialization, failure and cleanup; D-10/D-15/D-18/D-21 dynamic context, values and native dispatch; D-16 bits validity, layout and numeric compatibility. Dependency edges grant no authority.
- **Invariants / non-goals:** no ordinary reference ownership, per-object cleanup, nonregional graph allocation, cross-owner stored edge, implicit graph copy/move, lifetime extension or native callable environment in a regional field. Lexical scopes are not backed by implicit regions; D-28 records H-435 owner-base lexical views and separately reviews their physical carrier and lexical non-bits layout. Regional C++ types remain region-only and noncopyable. Actual-owner unions, abstract-result RTTI, fraction execution, pure/TCO and the compiler pilot remain separate.
- **Human baseline:** current project user in [the construction side chat](codex://threads/01a1d051-f608-7c13-b98a-684f065884d7), recorded 2026-11-12 America/Los_Angeles; exact message timestamps/personal name unavailable. H-383 preserves the ownership/allocation corrections; H-384 confirms R1, explicit R2 owner syntax/mismatch rejection and autonomous R3/R4 comparison with final selection reserved; H-385 confirms result-only constness. The [active main chat](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6) subsequently accepts contextual inference and prefers one coordinated migration without compatibility staging (H-386). DBMR protocols, aggregate ABI and final whole-file approval remain outstanding.
- **Acceptance:** the matrix below requires composed positive/rejection cases, exact acquisition/release accounting, relocation and measured compilation/runtime costs. No implementation or new performance result is claimed by this revision.

## Confirmed requirements

Only designated intrinsic resource-reference kinds distinguish owned and borrowed; the current kind is the region reference. Ordinary ref/ptr is borrowed, with target provenance, lifetime and access, without independent release or transfer authority. Storage ownership, reference-slot storage and the holder of region cleanup responsibility are distinct. User ADTs cannot contain intrinsic ownership carriers, including through wrappers.

Every ADT instance/storage view has an actual owner. Direct/val bits fields occupy that owner's storage; ref and non-null ptr fields target the same actual owner. Nominal type identity does not identify an owner instance. A bits copy across owners copies neither owner identity nor cleanup responsibility. Ordinary lexical borrows of outer storage do not become stored graph edges.

R1 admits a concrete ADT as bits when its finite fixed representation recursively consists only of bits fields/alternatives/tags, without stored references, ownership carriers, managed environments or implicit cleanup. This includes finite products, sums and qualifying generic instantiations. A one-word reference is still non-bits. An unresolved/abstract type requires sufficient bits evidence; R1 does not choose the source spelling for that constraint, a sum encoding, ABI, packing or validity of arbitrary bit patterns.

Bits ADTs may occupy lexical or regional storage, including a region root. Fresh lexical bits construction belongs to the current full scope; an ordinary grouping block adds no owner. Non-bits ADTs remain region-only with relative stored references. Adding a reference field invalidates dependent bits uses rather than selecting a hidden allocator. H-433 makes lexical pointer-bearing ADTs the target and this bits-only lexical boundary the conservative fallback. D-28 evaluates automatic direct/relative addressing and destination-scope construction capability; no particular layout, lexical allocation interface or builtin graph-copy extension is selected here. The current compiler's product-only/scalar-bits subset is an implementation limit, not the language definition.

The ordinary construction direction is `make T [in o]`; intrinsic resource acquisition uses `make region`. Explicit `in o` selects the actual owner and a receiving-variable owner mismatch is a compile error. Equal root types or lifetimes do not establish owner equality. Aliasing preserves provenance and access. RHS normalization below proposes a separate bits-copy path for value/readonly inputs; its interaction with D-23 reception is under review. An inferred `where {owner o}` binding supplies provenance, not implicit region allocation capability.

`make const region` restricts the published result view. Explicit preparation writes, including selected create/initialize/expand/eligible-trim operations, may precede that result subject to permissions and the selected protocol. Constness does not freeze the file, imply a snapshot or remove the owning acquisition's release obligation. Final DBMR grammar, sharing/defaults and operation/failure protocols remain unselected.

Callable payloads retain D-18's complete intrinsic value category and scope-owned backing; they are not bits ADTs or ordinary independently owned references. Local ADTs do not authorize lexical-cell captures or lifetime extension. H-370's deferred owning-argument transfer at invocation and H-365/H-371's scope restrictions remain unchanged.

## Bits classification and representation obligations

The proposed compiler separation is nominal type identity → resolved representation facts → placement/view → native transport. Classification must not replace nominal identity, actual-owner facts or abstract dispatch evidence.

| Obligation | Proposed integration / unresolved boundary |
| --- | --- |
| Concrete product | Check each instantiated direct field recursively; reject a ref/ptr or other non-bits constituent. Cache by resolved type identity and arguments, not source alias or actual owner. |
| Finite sum | Check all payloads and tags for finite bits representation. Preserve active-alternative validity. Concrete source/tag/layout work remains required; no tagged-union ABI is selected here. |
| Generic or recursive type | Distinguish unresolved evidence from proved non-bits. Detect infinite by-value expansion; pointer recursion is non-bits rather than a recursion escape in the classifier. Source constraints and diagnostics need an explicit contract. |
| Storage and field access | Address lexical and regional payloads through a common place abstraction retaining owner, access and initialization facts. Relative stored references keep D-01 encoding; a temporary native address is not a persisted CoD edge. |
| Copy and borrow | A bits read/copy is independent payload; a ref/ptr view aliases its target. Whole-value copy requires initialized valid payload. Field writes/read proofs must distinguish individual fields from the complete object. |
| Layout and native calls | Separate aggregate payload layout from reference transport. Preserve existing regional offsets; choose local aggregate argument/result transport with recursive callable and AOT requirements visible. No ABI choice follows from adding a classifier predicate. |
| Numeric validity | Bits eligibility promises no zero-validity or numeric conversion. D-16 fractions retain fixed payload/nonzero denominator and representation-independent UoM; array SoA/layout policy remains separate. |

The implementation dependency order is representation facts, aggregate places/initialization, payload operations, construction, then parameter/result/capture/TLS integration. Each increment must name incomplete admitted forms rather than present them as forbidden language. Classifying bits ADTs is not permission to make C++ `regional_*` objects stack-copyable.

## Owner-directed construction

The proposed compiler check for the confirmed construction behavior carries the resolved target type, actual destination owner, allocation capability, ordered field initializers and result access. Source forms feed that check; ordinary storage construction and intrinsic acquisition have distinct cleanup responsibility. This is a compiler organization proposal, not a new runtime object or universal allocator interface.

| Source situation | Required behavior and implementation boundary |
| --- | --- |
| Explicit `make T in r` | Use r only if it supplies the required placement/capability. Reject a conflicting receiver owner before runtime evaluation. H-384 confirms explicit selection and mismatch rejection; generic capability acquisition is not inferred. |
| Omitted owner, receiver has one unique owner and suitable allocation access | Construct there. A field may supply its containing owner to an explicitly written nested make. H-386 accepts this contextual inference. |
| Omitted owner, no destination | Bits use the current full lexical scope. Non-bits diagnose the missing regional destination. No TLS fallback or hidden region. H-386 accepts this fallback boundary; missing capability never causes a hidden allocator or fallback. |
| Receiver names provenance without allocator capability | Do not manufacture capability from identity. Require a usable allocation path; do not silently select another owner. |
| Initializer retains a reference result | Check actual owner/access. A non-bits graph is never implicitly copied; value/readonly bits normalization is reviewed below. |
| Ref/ptr field initialized without explicit make | Use the proposed RHS classification below: value/readonly bits materialize a target; retained views alias. Regional materialization remains under review. |

Illustrative candidate source, not current parser acceptance:

```text
ref p:Point = make Point { x <- 1; y <- 2 };
ref n:Node in r = make Node in r { ... };
ref bad:Node in r = make Node in s { ... }; # Reject distinct actual owners.
owned region arena:Root <- make region Root(capacity = bytes) { ... };
```

Construction must preserve initializer order and effects and publish only a valid initialized result. Existing D-17 failed acquisition cleanup does not promise rollback of effects or arbitrary reservations in an already-live region. Nested constructor naming/access, partial construction and allocator-policy interaction require review before implementing an uncovered case. Explicit owner spelling cannot bypass readonly allocation checks or same-owner stored edges.

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

Construction can share an owner/access/initialization model across those domains without sharing their allocator or reference encoding. Current full-scope and ordinary-grouping boundaries retain D-17. Lexical bits construction remains available; reference-bearing lexical ADTs need D-28's separate layout/placement decision. No scope-region resource, lexical region-capacity hint, automatic capture permission or new outer-selector syntax is implied.

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

H-426 retains inline TLS payloads for bits ADTs and the separately defined callable-view category. Symbolic reference declarations still require `in <some-owner>` and valid owner/access/lifetime relations; TLS is not an inline non-bits object arena. D-10 currently also requires region-symbol backing for object bindings. The lexical owner-base representation broadens neither that backing permission nor target lifetime. Intrinsic region-resource bindings retain their existing ownership/restoration role; no generic ADT can carry an intrinsic owning resource.

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
