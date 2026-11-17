# Regional and lexical storage: reference-model refactoring

Design revision: 29 · Recorded: 2026-11-17 · Status: one-ADT placement and scoped storage semantics selected; complete layout, encoding, backend and implementation contracts remain under review.

## Design authority

- **Typed reservation and completed construction:** [H-562](../CHANGELOG.md#h-562)/[K-TYPED-RESERVATION-20261117](../CHANGELOG.md#k-typed-reservation-20261117), [D-22-RESERVATION](DesignAuthorityRegistry.md#d-22-reservation)/P-500–P-502 covers D-28r29's initialization boundary. Bare `make T [in o]` reserves typed storage; an attached initializer block requests completed construction. C-65/C-72/C-95 and charter §§3.3/4.3/6.3/8 preserve explicit allocation and flow-checked readability. D-22 owns examples/acceptance; preserve established identity versus payload readiness, actual owners, staged sound proofs, no rollback and D-16. The source distinction is confirmed; aggregate algorithms/native support and complete value-path qualification remain separate. Other qualified contracts continue.

- **Owner-parameter symbol integration:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499, covers D-28r28's reference-symbol/owner dependency wording. The source form and staged sound acceptance are confirmed: installation selects one actual lexical or ordinary regional owner from the RHS; an `in @arena` declaration retains its additional selector constraint. C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve explicit dynamic context and lifetime responsibility. D-10 owns examples and acceptance. Preserve saved/captured owner validity, access/null/init, module symbol identity, no lifetime extension and D-16; full proof/physical/value-path qualification and source implementation remain separate. Other qualified scopes continue.

- **Native-coverage reference continuity:** [K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) preserves this revision's D-26 dependency at its current AOT contract section. Only dependency links and this summary change; existing semantics, authority, acceptance and D-16 remain unchanged.

- **Callable-reference continuity:** [K-ALIAS-GLOSSARY-20261117](../CHANGELOG.md#k-alias-glossary-20261117) preserves this revision's existing D-18 dependency under the canonical CallableViewsAndCaptures.md path. Only dependency links and this continuity summary change; existing contracts, qualification, acceptance and D-16 remain unchanged.

- **Capture/lifetime alignment:** [H-553–H-557](../CHANGELOG.md#h-553)/[K-CAPTURE-LIFETIMES-20261117](../CHANGELOG.md#k-capture-lifetimes-20261117), [D-18-VALUE-CAPTURE](DesignAuthorityRegistry.md#d-18-value-capture)/P-485–P-490 and [D-12-LIFETIME](DesignAuthorityRegistry.md#d-12-lifetime)/P-491–P-493 cover this revision's capture/owner dependency wording. Preserve independent value locals, selected reference targets, explicit var places, unique actual owners and statically bounded borrows; reject implicit slot sharing, closure-cell promotion and lifetime extension. C-36/C-54/C-65/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve visible storage/lifetime responsibility. D-18 owns acceptance and H-327/H-329 alias continuity. D-16, unrelated qualified scopes and existing symbol-declaration restrictions continue. New proof/physical/value-path qualification and source support remain separate; no allocator or graph-copy mechanism follows.

- **Symbolic reference provenance:** [H-482](../CHANGELOG.md#h-482)/[K-REFERENCE-SYMBOLS-20261116](../CHANGELOG.md#k-reference-symbols-20261116) integrates [D-10-REF-SYMBOL](DesignAuthorityRegistry.md#d-10-ref-symbol), P-422–P-424, into r26's TLS/owner transport boundary. Explicit ref/ptr symbols retain their installation-selected actual region through shadowing, capture and restoration; scalar targets use the same rule. Candidate C-36/C-54/C-65/C-93 serves charter §§3.2/3.3/4.3/6.3/8 by preserving owner-plus-locator interpretation and sharing valid owner information. D-10 owns acceptance and syntax; preserve separate group constraints, lifetime/access/init and H-481 capture timing. No owner extension, lexical-symbol backing, concrete metadata layout or broader implementation qualification follows.

- **One ADT across storage domains:** [D-28-ADT](DesignAuthorityRegistry.md#d-28-adt), r25's lexical ADT and derived-operation sections, records [H-478](../CHANGELOG.md#h-478)/[K-SINGLE-ADT-20261115](../CHANGELOG.md#k-single-adt-20261115). The user selects one nominal ADT and delegates unambiguous consequences. Candidate P-416 through C-10/C-39/C-65/C-91 serves charter §§3.1/3.3/4.3/6.3/7.1/8/12 by preserving business type identity across storage strategies; P-417 records dependencies and P-418 constrains D-07/D-10/D-14/D-22. Preserve same-owner fields, initialized access, zero-cost regional relocation, explicit graph transfer and lexical lifetimes; no new capture/TLS access, owning fields, implicit copy/move or permanent ABI. Acceptance covers both placements, common methods, automatic addressing, identity/growth and rejection boundaries below. Local semantics are confirmed; formal value-path/full-layout qualification remains pending. [H-479/H-480](../CHANGELOG.md#h-480)/[K-QUIRK-WORKAROUND-20261115](../CHANGELOG.md#k-quirk-workaround-20261115) retains the known first-field quirk with a preceding-field workaround; no dedicated analysis, acceptance expansion or repair work is required.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-28r21→22 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Staged acceptance / soundness:** [H-446](../CHANGELOG.md#h-446)/[H-447](../CHANGELOG.md#h-447), with [K-INIT-CONFIRM-20261115](../CHANGELOG.md#k-init-confirm-20261115), D-28-INIT/P-400, confirms the two-stage plan: accept common provable cases with minimum effort first; expand recognition for concrete needs later; soundness is mandatory throughout. C-65/C-72/P-398 and charter §§6.3/8/12 preserve checked readable storage and practical incremental implementation. The stated common-case coverage and soundness policy are confirmed planning scope; H-562 separately settles aggregate reservation; modifier combinations and broader publication mechanisms remain separate. This is not a current implementation claim. Acceptance requires every supported path and call boundary to preserve initialization, owner/access, identity and cleanup; unknown proof cases reject. Existing qualifications and the documentation-only task scope remain unchanged.

- **Initialization-flow principle:** [H-445](../CHANGELOG.md#h-445)/[K-INIT-FLOW-20261115](../CHANGELOG.md#k-init-flow-20261115), D-28-INIT/P-398–P-399, covers r20's flow section, continued by [K-INIT-COST-20261115](../CHANGELOG.md#k-init-cost-20261115): initialization state and obligations follow storage through local control flow as well as calls; established reference identity is distinct from readable payload. C-65/C-72/C-95 serves charter §§3.3/4.3/6.3/8 by preserving explicit sharing, access and ordered execution. The principle is confirmed; init-parameter syntax, complete aggregate/publication rules and implementation remain under review. Acceptance distinguishes states before/after discharge, aliases, branches, identity-only use and exceptional completion. Other qualified scopes continue unchanged; no runtime representation, rollback, move, fraction or lifetime change follows.

- **Construction scope / assessment:** [H-444](../CHANGELOG.md#h-444)'s cost criterion and H-445's flow principle govern the [combined assessment](#combined-implementation-cost-and-debt). K-INIT-COST-20261115 recommends a unified bounded extension alongside aggregate construction: moderate compiler effort, low additional runtime machinery and controllable debt. This is engineering judgment, not approval of concrete syntax/publication mechanisms; H-317/P-284 remains the qualified ordinary-construction boundary. C-65/C-72 and charter §§3.3/6.3/8/12 preserve initialized-reference guarantees and practical native execution. Future pure knot-tying remains D-19-KNOT/P-395–P-397. Acceptance for this review is a bounded cost explanation and coherent scopes; no code, tests, timing or measured performance are claimed.

- **Registry / scope:** [D-28](DesignAuthorityRegistry.md#d-28), r25, scoped directions and the sections qualified by the registry. [H-429](../CHANGELOG.md#h-429) separates regional and lexical storage, requests this dedicated refactoring document and proposes considering the C++ `global_ptr` → `external_ptr` rename. H-430/P-385 selects explicit consuming replacement instead of automatic/in-place language growth; H-431 selects ordinary owned-parameter failure cleanup. H-432 proposes a single resize(r, new_cap) for lossless growth/shrink, with exceptions when it cannot succeed. H-433 requests automatic addressing adaptation across storage domains and sets lexical pointer-bearing ADTs as the design target, with bits-only lexical allocation as the conservative fallback. H-434/H-435 selects dynamically allocated lexical capacity with owner-base-relative lexical pointers and replaceable heap-fragment backing. H-436 distinguishes a lexical slot locator from its contents: same-scope ref/ptr payloads use that scope's offset semantics, while other-owner references retain their own interpretation. Register/stack optimizations remain permitted. H-437 requires compiler tracking of all owner relations and shared runtime actual-owner identity/address information sufficient for dereferencing, avoiding redundant per-pointer owner storage. H-438 selects owner-qualified construction: a writable target parameter permits owner O, all-readonly inputs permit only const owner O, and explicit const owner remains allowed with writable targets. Independently selected requirements are recorded as D-28-SCOPE/C-92/P-386, D-28-OWNERS/C-93/P-387–P-388, D-28-ALLOC/C-94/P-389–P-390 and D-28-IDENTITY/C-95/P-391–P-392. H-439 makes actual owner plus target locator the reference correctness criterion and selects is; H-440 selects identity-based null testing; H-441 adds is not, including is not null; H-442 makes unary not and ! synonymous. H-443/D-28-INTEROP/P-393–P-394 prioritizes CoD semantic completeness: foreign adapters conform to CoD semantics and receive no additional stability/pinning promise. Exact physical protocols remain open. These scoped directions do not approve every layout, source form, ABI, allocator policy or migration choice below.
- **Candidate value path:** C-91/P-381 derives from C-39/C-65 and C-01/C-06: distinguish relocatable regional graphs from lexical execution storage while keeping target identity, access and lifetime explicit. Charter §§2/3.3/4.3/6.3/7.1/8/12 supports that rationale. P-383 organizes this document; both paths remain pending-human beyond H-429's stated direction.
- **Dependencies:** P-384 links D-01 physical storage, D-07 nominal types, D-10/D-15 TLS/worlds, D-14 owner/access/initialization, D-16 numeric validity, D-17 completion, D-18 callable values/captures, D-21/D-26 native transport and D-22/D-23/D-24 construction/result contracts. These dependencies confer no new authority.
- **Invariants / non-goals:** statically unique target owners; no lifetime extension, access escalation, owner unions, CoD move semantics, implicit graph copy or allocation rollback. Regional storage preserves relative edges and zero per-edge relocation work. Intrinsic region cleanup and assignment/value semantics retain their separate rules. H-478 selects lexical placement of the same pointer-bearing ADT with inherited same-owner fields. No general heap lifetime, concrete binary layout, new capture permission or permanent ABI is selected; C++ regional types retain D-01's region-only construction rules.
- **Acceptance / baseline:** H-429's current project-user messages are the human baseline; exact timestamp/personal identity are unavailable. [K-FOREIGN-ADAPTERS-20261115](../CHANGELOG.md#k-foreign-adapters-20261115) records H-443 foreign adaptation limits and removes foreign pinning as a pending CoD requirement. Internal reference/operation semantics remain unchanged; broader storage and native transport implementation remains pending. Owner-qualified allocation and shared owner information continue unchanged. Remaining CoD key transport and layout work does not reopen the selected reference semantics. Foreign API adaptation is separate and adds no language guarantee. H-435/H-436 storage/content and optimization requirements, and K-RESIZE-20261114 capacity/cleanup boundaries, remain unchanged. Acceptance requires the composed cases below, a reconciled source/design inventory and measured native costs. The source capability reference records bounded implementation evidence; broader acceptance here remains outstanding.

- **Current implementation alignment:** [K-704](../CHANGELOG.md#k-704) reconciles the independently confirmed H-443–H-447 scopes with K-702/K-703 and the [source capability reference](../tests/cod/language/README.md). Stable lexical identity/null observation, unary spelling and prepared scalar caller-binding methods remain implemented. Moving backing, aggregate initialization/publication, mandatory-write syntax, owner allocation transport and pure knot-tying retain their stated design/implementation boundaries.

## Problem and current direction

A region's stored graph must survive uniform relocation without rewriting every edge. A lexical scope describes execution lifetime and may hold views into independently owned storage. Sharing owner/lifetime checks does not require sharing the allocation mechanism or stored reference encoding.

The current direction is therefore two storage domains with explicit boundaries. Lexical scopes are not backed by implicit language regions. Region allocation policy is independently selectable by libraries/frameworks; a lexical view's lifetime does not shorten the lifetime of its target. That separation is an intentional modeling capability.

This refactoring separates terminology, semantic facts, physical representation and implementation transport. Some current native paths already use direct addresses for lexical access and relative offsets for region fields. The impact review must preserve correct existing paths and identify actual gaps; a conceptual refactoring is not evidence that all native code needs replacement.

## Terms and representation boundaries

| Concept | Proposed precise meaning | Independent obligations |
| --- | --- | --- |
| Regional relative/local reference | An edge stored in a region's object representation; the current encoding is relative to the edge slot. | Target shares the regional owner; null/non-null, initialization and access remain checked. |
| Lexical storage locator | Locate a materialized variable/binding slot using its storage scope and offset; native optimization may replace or eliminate that physical location. | Slot owner and target owner are distinct; locating a ref/ptr slot is not dereferencing its contents. |
| Lexical ref/ptr contents | Use same-scope offsets only when the target owner equals the slot's lexical owner; otherwise interpret the payload under its actual target-owner contract. | Retain one statically provable target owner and valid access/lifetime, without global lifetime or ownership. |
| External regional handle | A host/runtime handle used outside a target region to access an object within it. | Carries the information needed to resolve that region's target; it is not a persistable regional edge or a general lexical pointer. |
| Target owner | The lifetime authority for the referenced payload. | Separate from the storage/lifetime of the reference binding itself and from intrinsic cleanup responsibility. |

Stored representation and target provenance remain separate. A regional object's edge stays relative to its field slot when accessed through a lexical variable. Under H-435/H-436, a lexical variable's storage is located independently of how its contents are interpreted. Same-scope reference contents use the same scope-base offset convention; other-owner contents cannot be decoded using the slot owner's base. Val bits do not become pointer objects. Physical encoding now depends on the selected slot/view contract and the target storage domain, not just where the variable name is declared.

H-437 fixes sharing of actual-owner identity/address information; physical width and transport layout remain open. Resolved native addresses remain temporary implementation values. A raw address, a native address plus separately retained owner/type evidence, and an external regional handle are not automatically interchangeable ABIs. Static evidence may erase some runtime metadata without erasing the language obligations.

## Shared owner information from checked declarations

H-437 requires the compiler to track every `in o` declaration and arrange shared runtime information about the actual owner's identity and address, sufficient for every admitted dereference without redundant per-pointer owner tracking/storage. This includes owner relations established by inference under H-417, formal/actual substitutions, inherited field owners, aliases, borrowed results and permitted captures; omission of surface `in` spelling cannot drop provenance. The rule applies to lexical and regional owners, with their respective addressing and lifetime contracts.

A logical owner context denotes the actual owner and supplies the address-resolution information required by admitted representations. References proven to share that owner reuse this information; each still supplies its own target/place information and access/initialization facts. This is a sharing requirement, not a mandatory heap descriptor, global registry, numeric owner-ID format, fat pointer or exact ABI. Static facts may erase owner data or pass it once for a group of parameters/captures. Metadata may be split across a frame/environment and direct hidden arguments when that preserves the same obligations. Same-owner ADT fields reuse their containing owner association; regional relative edges gain no per-edge owner record. The compiler need not emit a runtime owner check on every access when static evidence already establishes the required facts.

```text
proc add_into(dst:u64 in O, src:const u64 in O) where {owner O} {
    ref again:const u64 in O = src;
    dst <- dst + again;
}
```

All three bindings retain the same actual O. The runtime need not store O's identity and base separately beside each target locator. The callee must nevertheless interpret its local slots separately from the external-owner contents under H-436. A later permitted allocation in O must make all subsequent resolutions use current backing; grouping metadata is not permission to cache a stale base at entry.

The following consequences are settled by the existing owner rules plus H-437:

- Group by proven actual-owner relationships, not textual owner names. Recursive invocations may instantiate the same formal name with distinct actual owners; a forwarded owner may instead be the very same instance across many calls.
- Merge owner information only where the sharing is sound. Two formal owner roles may reuse one runtime context when supplied the same proved actual owner; distinct/unknown owners do not acquire static equality because their base addresses happen to match or memory was reused.
- Preserve owner identity independently of current backing address. Lexical backing growth changes resolution state while retaining owner/target identity; consuming region replacement ends the old owner and cannot retarget its borrowed views.
- Every dereference must have sufficient live owner/resolution, selected target, type/layout, access and initialization evidence, either retained at runtime or established statically. An owner context alone does not identify every object's concrete type or grant access.
- Null remains associated with its fixed owner. Null testing does not read a target and does not permit an ambiguous owner or expired reference.
- Passing, returning or storing an admitted view adapts its payload and shares/retains the corresponding owner information. Prepared references, aliases, deferred work and complete callable values must preserve the relevant relationship without adding capture permission or extending owner lifetime.
- Register/stack optimization may remove physical slots, offsets or owner metadata when all observable semantics remain. Conversely, a shared context must remain valid wherever the generated code still needs it; copying raw addresses cannot replace that obligation.

An `in O` relation tracks provenance and lifetime. H-438 separately makes allocation permission explicit in the owner qualifier described below. Sharing identity/resolution information does not merge differently qualified access paths, grant ownership or retain O after scope exit. H-439 separately provides language reference identity; native raw-address exposure and persistent layouts retain their own contracts.

Acceptance compares many references to one owner with references to several owners, explicit versus inferred relations, same-named recursive owner instances, forwarding, null, prepared calls/returns/defer and backing growth. Inspection must establish that repeated pointer fields/variables do not each retain redundant owner identity/base state where a shared or statically erased representation suffices, while all dereferences remain correct. No measured overhead or implementation claim is made here.

### C++ naming proposal

H-429 proposes considering `external_ptr<T, RT>` as the replacement name for today's `global_ptr<T, RT>`. **Recommendation: adopt that name during the coordinated refactoring**, because it describes a handle accessing a regional target from outside the region and avoids conflating an external regional handle with the lexical reference model.

The current [implementation](../include/shilos/region.hh) stores a C++ reference to `memory_region<RT>` plus a target offset; `get()` derives an address from that referenced header. It is neither an arbitrary raw address nor a stable relocation descriptor. Renaming it does not make existing handles survive a moved region header. The `regional_ptr` name can remain for the existing relative edge; this draft introduces no second rename or public CoD pointer-type keyword.

The rename inventory includes declarations in [prelude.hh](../include/shilos/prelude.hh), conversions/accessors in [region.hh](../include/shilos/region.hh), [string helpers](../include/shilos/str.hh), colocated vector/list/dictionary YAML helpers, shilos tests, experiments, documentation and repository instructions. External downstream API/ABI consumers need an inventory too. Current code keeps its existing name until the refactoring is qualified and implemented; the proposed rename is not a completed source change or a permanent compatibility-alias commitment.

## Owner-qualified construction

H-438 selects a common allocation-access rule for lexical and regional owners. In a callable signature, at least one parameter referencing a writable target in O permits `where {owner O}`. If all parameters referring into O have readonly target access, only `where {const owner O}` is admissible. A callable may deliberately use `const owner O` even with writable target parameters: target writes and allocation permission are independent. Both owner bindings have a fixed actual identity; const here restricts owner operations, not rebinding of that identity.

| Inputs associated with O | Admissible owner declaration | Construction through O |
| --- | --- | --- |
| At least one writable-target reference | `owner O` or deliberately weaker `const owner O` | Permitted only through `owner O`. |
| Only readonly-target references | `const owner O` | Forbidden, including construction of readonly results. |
| No input establishing O | Neither declaration invents an actual owner | Require an independently admitted owner input; a result annotation alone is insufficient. |

Target constness is decisive. `ref x:const T in O` is readonly; `const ref x:T in O` only forbids rebinding x and still supplies writable target access. The rule applies to ref/ptr and implicit external-borrow parameter forms. Independent val expressions and default by-value parameter copies do not establish an external owner. Borrowing an existing val variable instead refers to that storage and retains its owner under D-23. A nullable ptr retains its owner and access even when null, so an admissible writable-target ptr can carry the required association without a dereference. Creating an owner-qualified null cannot upgrade a const owner capability.

For example, this constructor helper has the required owner access; lexical Node placement still depends on the separately reviewed ADT layout contract:

```text
data Node { value:u64; ptr next:Node; }
proc prepend(ptr head:Node in O, n:u64)
    -> ref Node in O where {owner O} {
    return make Node in O { value <- n; next = head; };
}

proc update_only(x:u64 in O) where {const owner O} {
    x <- 9u64;                  # Allowed: existing target is writable.
    # make u64 in O             # Rejected: O has no allocation access.
}

proc inspect(x:const u64 in O) -> val u64 where {const owner O} {
    return x;
}
# Replacing inspect's const owner O with owner O rejects its signature.
```

The call boundary checks both target access and the selected owner requirement. A non-const owner can satisfy a const-owner requirement; const-owner access cannot satisfy a non-const-owner requirement. Thus update_only can forward x to another const-owner helper but cannot obtain allocation indirectly by calling a helper requiring owner O. Aliases, null values, returns and permitted captures cannot strengthen a retained owner capability. Where another independently available non-const owner path exists, it may allocate under its own contract; const owner does not globally freeze that actual owner or prevent backing growth.

Mixed inputs to one O permit allocation when the declared owner is non-const, but readonly parameters stay readonly. Distinct formal roles can have different permissions even when their actual owner is equal: `owner D` and `const owner S` may share runtime identity/resolution information without granting S allocation or making source views writable. The compiler tracks these role permissions independently of any shared runtime record; no per-pointer permission tag or owner registry is mandated.

`make T in O` uses the actual owner's construction path. The caller supplies the checked association through H-437 shared owner information, without an additional user-visible builder argument solely for allocation permission. A generic owner need not expose a regional root value or root-policy methods; region-specific root constraints remain necessary when the body directly uses those operations. The native allocator dispatch/state is still an engineering contract to establish. Allocation remains subject to admitted T placement, initialization, policy, capacity and failure behavior. This permission grants no release/resize responsibility, region growth, graph copy, allocation rollback or lifetime extension. Implicit target materialization into O, when otherwise admitted, has the same allocation-access check.

Acceptance covers mixed/all-readonly inputs, leading versus target const, deliberate const-owner restriction with writable targets, null forwarding without dereference, indirect capability escalation, equal actual owners with distinct qualified roles, omitted-owner construction and unchanged region capacity/cleanup. Explicit region-resource inputs keep their separately admitted contracts; a signature with no supporting input cannot infer an owner from its desired result.

## Reference identity and the is operator

H-439 establishes one correctness criterion for references: each interpretation uses the selected actual owner and target locator at that observation. The owner supplies the current resolution context. The locator denotes the selected target within that owner, independently of the reference binding's own slot. A stored regional displacement must be decoded relative to its edge slot; it is not itself a canonical target identity. Lexical backing growth preserves existing owner/locator pairs while changing their resolved native addresses. Optimizations may share/eliminate metadata, cache a proved-stable address or use registers/stack storage only when they preserve this criterion and observable evaluation.

CoD provides the intrinsic binary operators `is` and `is not`, returning independent val bool results. H-441 defines `a is not b` as the exact negation of `a is b`, equivalent to `!(a is b)` with the same once-only reference-preserving operand evaluation. Both are fixed reference operations; user payload-equality overloads do not redefine them. They use equality-operator precedence, above boolean conjunction/disjunction; chained identity expressions do not implicitly become pairwise comparisons. The two-word is not form is one comparison operation, not is applied to a negated payload. H-442 also admits unary not as the exact spelling synonym of !, with the same tight unary precedence and operation identity; [the unary contract](SymbolicDynamicScope.md#unary-logical-negation) governs. Thus not (a is b), !(a is b) and a is not b agree, while not a is b groups as (not a) is b. Ordinary value equality retains its own contract.

For non-null references, `a is b` is true exactly when their selected actual owners and normalized target locators are equal. Access qualifiers, alternative views of that same target and the slot/representation carrying the reference do not change this result. Distinct targets do not become identical because their payloads, types, layouts or transient addresses match. A target locator preserves semantic subobject selection: an enclosing object and a distinct field/subobject cannot collapse merely because their native starting addresses coincide. Identity comparison grants no cast, type equivalence or stronger target/owner access.

```text
val a:u64 <- 7u64;
val b:u64 <- 7u64;
ref p:u64 = a;
ref alias_of_a:u64 = a;
ptr q:u64 in here = b;

p is alias_of_a;       # true: same selected owner and target.
p is q;                # false: distinct storage, despite equal bits.
p is not q;            # true: exact identity negation.
q = p;
p is q;                # true: q now selects a.
q = null;
q is null;             # true; no target read.
p is not null;         # true for the valid non-null ref.
not (p is null);       # Same result as !(p is null).
# 7u64 is 7u64          # Reject: independent values supply no targets.
```

A storage-denoting expression retains its selected target, including a named val variable or direct bits field with real storage. An independent arithmetic/value-call result has no such identity, and is does not allocate temporaries to invent one. Existing explicit materialization can establish storage before comparison. The pending grouping/value-normalization rules determine operand categories normally; is does not silently reinterpret an already independent value as a reference. Callable-storage identity, where admitted, is identity of that storage, not equality of code, captured contents or semantic method identity.

Evaluate operands once, left to right, retaining each selected owner/locator when its operand completes. A later operand may rebind the source variable or grow backing without changing the earlier selection. There is no short circuit merely because owner inequality could determine the boolean result. The selected reference carriers must be initialized and their owners remain valid. Evaluating a compound operand can dereference intermediate targets or call throwing code; comparing the final references itself does not read their payloads or dereference a nullable target. Growth requires current resolution where an address is needed; ending an owner cannot be repaired by comparing a stale address or identity token.

### Null uses the same identity relation

H-440/H-441 uses `p is null` and `p is not null` for null/non-null testing. A bare null in an identity comparison obtains the expected owner and reference context from the other operand, so the test is well formed even for a null pointer and does not acquire an owner from a future target. `null is p` follows the same symmetric relation. Two unqualified null operands without an established owner context reject; identity comparison does not weaken H-418.

Qualified null retains the pair of its actual owner and the null locator. Same-owner null references compare true; references belonging to distinct owners compare false even when both are null. Thus an explicit `p is null in Other` tests that particular owner/null pair, whereas `p is null` tests absence under p's own fixed owner. A non-null reference never equals the null locator. Comparisons may inspect separately proven owners without creating a mixed-owner reference or permitting cross-owner rebinding.

Null testing preserves checked-NPE rules: only an actual target access creates that obligation. `p is not null` (equivalently `not (p is null)` or `!(p is null)`) can establish non-nullness for a stable selected reference; later rebinding or mutation invalidates the corresponding fact. Comparing a mutable field once does not make a later reload stable. `typeof(p is null)` and `typeof(p is not null)` determine bool without executing p or reading a target. There is no separate null-test intrinsic in the selected source design.

### Identity keys for graph libraries

The language supplies identity/key support matching is, so graph libraries need not add an application ID field to every ADT. Equal identities must produce equal keys/hashes; hash collisions must not be mistaken for identity. Keys remain stable through backing growth and payload writes for the same target. A key extracted from p denotes that selected target, not future rebindings of p. Runtime owner information may be shared with a group of keys; no repeated owner record or globally allocated numeric owner ID is mandated.

Key storage must support traversal memoization across source/destination owners without bypassing same-owner pointer-field rules. A non-dereferenceable key is not a reference capability, does not retain an owner, and cannot restore a target after its lifetime ends. Ordinary val bits still have no owner annotation. The exact library key surface, checked context/lifetime transport and native representation are implementation-contract work; this requirement does not select a universal persistent identity token, raw-address conversion or new cross-owner graph field. Escalate only a material source/lifetime trade-off, rather than asking again whether CoD should provide identity.

Identity denotes selected storage, not an application's enduring entity identity. Whole-fragment growth and cleanup-holder transfer preserve a continuing owner/target; consuming region replacement does not. Pool reuse, container compaction and object movement retain their own validity contracts. No generation table or automatic object-following reference is implied; an invalidated reference cannot be compared to establish continuity. Library graph transfer still controls topology, allocation and failure and receives no default compiler copier.

Acceptance covers aliases versus equal payloads, named val storage versus independent results, readonly views, different owners with equal offsets, regional-edge decoding, slot aliases versus targets, null contextual ownership, selected operands across rebinding/growth, distinct subobjects and optimized storage, key/hash stability and invalid lifetimes. Numeric/fraction value equality, validity, `#` comments and reserved fraction syntax remain unchanged.

## Operations across the boundary

Illustrative CoD, with root already referring to a live object in r:

```text
data Node { ptr next:Node; }

ptr cursor:Node in r = root.next;
# root.next is a relative edge in r.
# cursor is an independent lexical binding retaining a view into r.

cursor = null;          # Clear only the lexical binding; retain owner r.
root.next = cursor;     # Store null using the regional edge representation.
```

Loading a regional edge resolves a view; storing that view back checks actual owner/access and encodes an edge relative to the destination slot. Copying the raw offset bits to another slot is not reference rebinding. The same rule applies when a helper returns a view or a conditional selects it. Reading a ptr value must preserve null without dereferencing its target.

An alias of a regional field is another access path to the same field slot. It does not replace that slot's relative encoding with a lexical pointer cell. D-18's ordinary-place alias and capture rules therefore need to distinguish slot selection from retaining the slot's current referent.

The following semantic dimensions remain separate throughout checking and lowering:

- val bits versus retained reference results;
- the reference binding/field's storage domain and representation;
- the target's actual owner and lifetime;
- binding mutability, target access and nullability;
- target initialization and actual concrete-type evidence;
- intrinsic resource cleanup responsibility.

No representation choice weakens H-417 unique-owner inference. A lexical view still cannot choose between different target owners at runtime without an independently admitted value operation; a regional edge still cannot point outside its owner. H-428 allows allocation effects on initialization failure, with no automatic rewind. H-426/H-427 RHS normalization remains in D-22; selecting a carrier neither approves that pending copy policy nor introduces move semantics.

## Owner-base-relative lexical storage

H-435 confirms dynamic lexical capacity and owner-base offsets independently of the wider ADT/ABI refactoring. H-436 clarifies that locating a lexical variable's storage and interpreting its contents are separate operations. A lexical scope remains the semantic lifetime owner; it does not become a CoD owned region, and regional resize retains its consuming contract.

### Locating storage and interpreting contents

Let S be the lexical owner of a variable's storage slot, and O the statically established target owner if its contents are an ordinary ref/ptr. For storage materialized in S's growable fragment:

| Operation / contents | Required interpretation |
| --- | --- |
| Locate the variable/binding slot | Resolve the slot offset against S's current base. This establishes where the contents are stored, not what they mean. |
| Read bits/val contents | Read the typed payload at that location; no second pointer dereference is implied. |
| Dereference ref/ptr contents with O = S | Interpret the reference payload with S's same-scope offset convention and resolve the target using S's current base. Slot offset and target offset are distinct. |
| Dereference ref/ptr contents with O != S | Apply the actual target owner's reference interpretation. Never add S's base merely because the reference variable lives in S. |

For nullable ptr, null handling belongs to the selected reference representation and does not resolve a target. The slot location itself is not a nullable pointer payload. Owner equality is static under H-417; this distinction does not require a runtime owner tag on every pointer.

Another lexical scope is also an other-owner case. Its representation may refer to that owner's resolution context, but H-436 does not select the same-scope single-offset representation for it. A regional target retains its regional lifetime/access contract; an external regional handle or temporary resolved address is not automatically the chosen CoD ABI. Exact inter-owner carriers remain open. This concerns admitted ordinary reference bindings and does not relax same-owner stored fields inside ADTs.

Schematic CoD shows why a reference's contents may need different transport even though the referent is unchanged:

```text
proc use_outer(x:u64 in O) where {owner O} {
    ref q:u64 in O = x;
    q <- 2u64;
}
val n:u64 <- 1u64;
ref p:u64 = n;
use_outer(p);
```

In the caller, p's slot and n have the same lexical owner. The callee's x/q slots belong to its own scope, while their target owner remains the caller's O. Argument preparation and reference initialization must preserve that target and adapt representation where required; copying offset bytes and using the callee's base would be incorrect. Aliasing a slot preserves the original slot and its encoding. Creating a new binding to its referent is a different operation. Neither operation copies the target payload or extends its lifetime.

### Physical allocation and optimization

Register allocation, scalar replacement, fixed stack storage and elimination of unused storage/owner metadata remain permitted under H-436 and charter §§6.2–6.3. Dynamic lexical capacity is a language capability, not a mandatory heap allocation for every scope or variable. The compiler may eliminate an offset calculation or the slot itself when it preserves all observable owner, alias, access, initialization, evaluation and cleanup behavior.

Taking a reference does not by itself mandate heap allocation. All surviving aliases and calls must still share the correct target. If a chosen register/stack representation cannot satisfy those access paths, lowering must materialize compatible addressable storage or use another proved-correct representation. It must not encode a fixed stack address as an ordinary offset into a movable fragment and then assume fragment growth preserves that target. Exact materialization/ABI strategies remain open; no optimization grants new source placement or escape permission.

The fragment organization under review uses one active contiguous payload for the portion of an actual full-scope instance that needs growable backing. The owner context supplies its current base and allocation state. Same-scope payload offsets and variable-slot offsets can use that context; external references retain their independently required representation. H-437 requires sharing that owner information across the references that use it. No permanent fat-pointer ABI, mandatory all-variable heap frame or one global context per source scope is selected. Recursion has distinct owners. Plain grouping blocks do not create owners under D-17. Multiple concurrent fragments would require additional fragment identity or a different mapping contract.

The owner anchor/resolution metadata cannot depend on an untracked raw address into the fragment being replaced. Its exact storage/cleanup arrangement is open. Relocation must preserve object offsets, alignment, initialized bits and all admitted embedded representations. Callable environments, selected places, cleanup records and native interop cannot be assumed relocatable merely because ordinary data pointer fields use offsets. Ordinary CoD objects acquire no user-visible copy/move operation.

### Selected addresses across allocation

Changing pointer fields is insufficient if generated code retains stale native addresses. Consider this schematic body, where the RHS has independently admitted capability to allocate in p's owner:

```text
p <- value_that_allocates_in_the_same_scope();
```

The lhs is selected before the RHS. For a target in growable lexical backing, lowering retains its target-owner context and location information, evaluates the RHS once, then resolves that same target against the current backing before storing. An external target keeps its own target-owner representation; growth of the pointer variable's slot owner alone does not relocate that external target. It must not rerun an indexed/field selection or reload a rebound pointer variable and accidentally choose a different target. Reference assignment analogously retains the selected destination slot's locator and encodes its new contents under the fixed target-owner contract. Selected aliases, calls/returns, recursion, deferred work and all allowed captures need the same treatment. A generic helper cannot keep a raw pointer alive across an operation that may relocate its owner.

The compiler can retain a resolved address across operations proven unable to relocate that owner. Otherwise it must resolve again after possible growth. H-443 requires foreign adapters to respect this same correctness boundary. An adapter may use a raw address only while its validity is established; it cannot assume that CoD refreshes arbitrary foreign pointers. Foreign expectations do not introduce lexical pinning, suppress ordinary growth or add a new growth-failure condition. CoD-generated native code and its runtime remain fully responsible for preserving the language semantics.

### Growth, transfer and failure

A feasible growth sequence allocates a sufficiently aligned replacement, preserves the retained payload and offsets, updates the stable anchor, then releases the old block with the matching allocator. It avoids per-edge fixups, but copying a live payload may still cost linear time in its extent. Growth policy, capacity hints and multi-fragment alternatives are not selected. Scope identity and target identity remain unchanged across successful backing replacement, unlike consuming region resize.

Replacement allocation must not invalidate existing targets before replacement storage is ready. On allocation failure the previous fragment remains valid for as long as its scope survives; propagating cleanup may of course end that scope. Earlier allowed initialization effects are not undone. This is an internal lexical-storage operation, not an owned-region parameter transfer, so H-431's input-consumption rule does not apply to it. Detailed allocator/cleanup protocols remain part of scoped allocation integration; there is no language checkpoint or graph rollback.

Bits transfer still reads and writes typed payloads after resolving each side. A graph-transfer library still constructs destination targets and maps topology according to its contract. Encoding a mapped destination target as a scope-base offset or regional slot displacement can be automatic; copying a source offset into a different owner cannot stand in for graph transformation. A single nominal ADT remains the UX candidate, with distinct native layout/access evidence. The external_ptr rename does not implement a stable lexical owner anchor by itself.

Acceptance must compare optimized register/stack forms with materialized fragment storage. Force growth between operand selection and access, independently in the slot and target owners, through nested calls and caller-scope allocation, with same-scope versus other-owner references, aliased slots, null, recursive scope instances, sharing/cycles, abrupt completion and allocation failure. Verify no stale raw address or relocated-anchor dependency survives; also cover interval/range/alignment/null encoding. The selected model supports dynamic lexical capacity while retaining target identity; base resolution and the relocation-aware native-call boundary are implementation costs to measure. No benchmark or implementation support is claimed.

## Explicit resize consumes and replaces the region

H-430/H-431 establishes explicit consuming replacement and ordinary owned-parameter cleanup. H-432 proposes one lossless intrinsic for both growth and shrinkage. The current candidate signature is:

```text
func resize(r:owned region:R, new_cap:u64) -> owned region:R where {R}
```

The schematic signature makes the requested capacity explicit; exact generic/parameter spelling remains subject to source integration. Ordinary allocation cannot grow a region automatically, and resize does not preserve an old language region with live direct/external views. It consumes cleanup responsibility for the input and returns a replacement on success. This is a design contract under review, not current parser/compiler support.

The direct lifetime consequence of replacement is that the old region's lifetime ends and the output has a new actual owner identity. Root type R is preserved, but type equality does not equate the old and new owners. Existing references, aliases and captured/deferred views into the old region cannot be used after replacement or refreshed merely by receiving the output. A reference binding whose owner is fixed to the input cannot be rebound to the output; callers obtain fresh views instead.

```text
# r is an owned region with root type Root.
ref before = root(r);
# Reads/writes through before are valid while r remains live.
owned region resized:Root <- resize(r, 65536u64);
ref after = root(resized);  # Fresh view associated with the returned owner.
# root(r) rejects: r was consumed.
# Using before after resize rejects: its target owner ended.
```

This is distinct from D-14/H-263's ordinary local ownership transfer, which may keep the same region and existing views live under a different cleanup holder. An owned-input/owned-output signature alone does not imply replacement: resize's intrinsic contract supplies that additional fact. Nor does owned mean no borrows exist. The checker must reject surviving uses through aliases, captures, TLS, prepared deferred work or other admitted access paths. A local reference name need not vanish lexically if it is no longer used; no blanket ban on ever taking a borrow before resize follows. Unproved overlapping access is not made safe by a consuming parameter annotation.

The retained regional graph preserves payload, sharing and cycles with relative edges intact under uniform relocation. This is whole-region replacement, with no per-object CoD move/copy operation, implicit graph duplication or per-edge fixup. Runtime base-address equality is not owner identity; no exact physical address or allocator strategy is selected. Root/target access cannot become more permissive through resize.

### Requested capacity and lossless shrinkage

The recommended capacity convention reuses D-17 E5: new_cap is the exact usable payload byte budget, including the root, payload allocations and their alignment consumption, excluding the fixed region header. It is neither an increment nor a requested amount of free space. Physical page/allocation rounding may reserve more bytes without increasing that language-visible budget. Header addition, representability and alignment arithmetic must be checked.

Let C be the current payload capacity and E the end of the retained allocated payload extent, measured from the payload start. Under the current bump allocator, E corresponds to occupation minus the region-header size. It includes alignment gaps and previously allocated storage; it is not the sum of root-reachable live object sizes. A separately specified reclamation policy may establish a smaller retained extent, but resize does not infer one by tracing or inspecting payloads.

| Request | Recommended outcome |
| --- | --- |
| new_cap > C, representable and supported | Return a larger-capacity replacement, preserving retained contents and offsets. New free storage is not implicitly initialized. |
| E <= new_cap < C | Trim only unused tail capacity; return a smaller-capacity replacement. |
| new_cap == C | Retain the same consuming/result-owner boundary. Backend work may be elided if the contract is preserved; old views do not regain validity based on a runtime size comparison. |
| new_cap < E | Throw rather than truncate allocated storage, silently clamp capacity or compact objects. |
| Invalid/unrepresentable request or provider failure | Throw under the selected failure category; no partial or nullable successful region result. |

Losslessness preserves initialized payloads, validity, root/concrete-type evidence, object offsets, sharing and cycles. The region's capacity metadata changes; no guarantee of copying uninitialized bytes as readable values is implied. Shrinkage does not discard an allocated object merely because it is unreachable from the root. It does not pack holes, rewrite stored offsets or invoke object constructors/copy/move operations. Compaction and application-managed reclamation require their own explicit contracts. A zero request is not an alternate release operation and cannot succeed when the root/retained extent does not fit.

The proposed exception mapping reuses D-17 categories: InvalidCapacityError for invalid/unrepresentable requests or size arithmetic; RegionCapacityExceededError when a representable requested budget cannot contain the retained extent; OutOfMemoryError when the provider cannot supply required backing. Capacity checks inside resize occur after consuming entry. H-431 therefore applies even to a rejected shrink: input ownership is not restored and callee cleanup releases what it still owns. Successful resize is lossless; failure does not promise preservation of the input region. Argument computation/conversion that fails before entry retains the separate E4 rule. Backend/file failures still require a selected resource protocol.

This exact-capacity, offset-preserving shrink and equal-capacity rule is the agent's concrete integration recommendation under H-432 and H-423, with D-17's established capacity units retained. It is not an independently approved compaction algorithm, new exception taxonomy or storage backend.

### Entry, failure and returned ownership

H-431 chooses D-17 E4 exactly; there is no resize-specific ownership recovery exception:

| Boundary / outcome | Input ownership | Returned ownership |
| --- | --- | --- |
| Argument evaluation or conversion fails before entry | This call has not consumed an existing named input; earlier explicit argument effects remain. | None. |
| Callee entry | Caller binding is consumed; callee receives cleanup responsibility. | None yet. |
| Validation/allocation/processing fails after entry | Callee cleans up resources it still owns, including the input or a prepared replacement as applicable. Input ownership is not restored to the caller. | None. |
| Normal result selected, but required cleanup fails | The selected owning result is abandoned and released exactly once. | None. |
| Successful completion including required cleanup | Old input has been replaced/retired under resize's contract. | Caller receives exactly one owning result. |

A catch does not undo the transfer or make the consumed input name usable. Choosing an owning return inside the callee does not deliver it before finally/deferred cleanup completes. If caller-side receiving work later fails, normal temporary/local result cleanup applies; that is not restoration of input ownership. A successful ordinary consuming function may deliberately return the same region, but that is an explicit owning result rather than rollback; resize supplies the stronger replacement rule.

The contract is semantic. The current [source capability reference](../tests/cod/language/README.md) excludes regional values from implemented coverage; existing historical fixtures do not establish current resize support. Implementation acceptance must exercise the complete entry/result/cleanup boundaries with exact resource accounting.

## Integrated operations and lifetimes

The compiler must distinguish semantic type, selected storage location, stored-reference encoding, target owner, target layout/access and intrinsic cleanup responsibility. The following obligations derive from the existing operation contracts; they do not select an intermediate representation, descriptor layout or new source syntax.

| Operation / boundary | Information that must survive | Required behavior |
| --- | --- | --- |
| Declaration and `make` initialization | Destination owner/capability, selected storage, ordered initializer state | Growth during a later initializer cannot invalidate earlier selected locations. Establish completed bindings under existing initialization rules; preserve earlier permitted effects on failure. |
| Ordinary `=` | Selected destination reference slot; source target identity/owner/access | Resolve the source and encode it for the actual destination slot. Fixed-owner checks remain; copying encoded words between unlike slots is not the operation. |
| Bits `<-` | Selected destination payload target and converted source value | Preserve the original target across RHS growth or rebinding; store only after successful RHS completion. |
| Reference `<=>` | Both selected binding slots and their permitted target owners | After ordered operand preparation, decode both current references and encode each for its destination. A regional field and an external lexical binding may need different encodings even when both target the same region. |
| Bits `<->` | Both selected payload targets and both original bits values | Resolve live locations after operand preparation, read both payloads before writing either, preserving overlap and checked-null behavior. |
| Slot alias | Original selected slot and its representation/access | A lexical alias name does not create a new pointer cell or change a regional field into a lexical offset field. |
| Argument preparation and invocation | Prepared value or selected referent/place according to passing mode | Later arguments may grow an owner or rebind a source variable. Refresh locations without changing the already prepared referent or reevaluating arguments. Revalidate owner lifetime at entry. |
| Borrowed return | Selected target, target owner/access and concrete/layout evidence | Cleanup may grow backing or update payloads. Deliver the same selected live target, adapting transport to its receiver. Callee-local storage still cannot escape. |
| Scalar val return | Independent converted bits from the return expression | Cleanup growth or source mutation cannot reread/change saved bits. No observable pointer or owner association is added to val results. |
| Assignment-method result | Original destination role and resulting referent | Retain D-14's result safeguard and readonly/writable rules; representation adaptation does not substitute another destination. |
| Complete callable result | Selected caller destination and complete environment | A raw hidden destination address cannot remain stale across growth of its backing. Use a qualified stable-placement or relocation-aware construction protocol while preserving the complete-value result. |
| Deferred invocation and scope exit | Prepared callable/arguments, selected places and remaining cleanup | Relocation does not change registration order, prepared snapshots, capture permissions or completion replacement. Keep necessary owner metadata valid through its last admitted use. |

For an existing `p <- f()` destination, p's slot owner and its target owner can differ. Growth of the slot owner may move the reference cell without moving the selected target. Growth of the target owner may move the selected target without changing the cell. Neither event permits reloading a rebound p and selecting a different target. Conversely, `p = f()` retains the binding slot and writes a new admitted reference into it. This distinction must survive infix, explicit operator-call and aliased-method forms.

Readonly target access, function/procedure kind and register promotion are not by themselves proofs that an owner cannot grow during a call: another admitted capability may allocate there. A possible compiler organization tracks which owners an operation may relocate, or conservatively refreshes the needed contexts after uncertain calls. Such internal facts do not add a user-visible effect system or make physical addresses specialization keys.

### Backing growth, object movement and owner replacement

| Event | Semantic owner / object identity | What references require |
| --- | --- | --- |
| Lexical fragment growth preserving offsets | Same owner and existing targets | Resolve retained locations through current owner metadata; no per-edge target rewrite. |
| Ordinary region cleanup-holder transfer | Same region while its actual lifetime continues | Preserve existing admitted views under H-263, independent of the consumed holder name. |
| Consuming region `resize` | Replacement owner and retired input | Reject subsequent old-owner views; obtain fresh views from the owning result. |
| Container expansion, compaction or pool reuse that changes object locations inside an owner | Not established by lexical fragment growth | A separate container/identity/borrow contract must preserve or invalidate references appropriately. Offsets alone do not grant address-stable vector growth or arbitrary object relocation. |

[Reference identity](#reference-identity-and-the-is-operator) supplies the actual-owner/target-locator criterion and language key support for memoization. Domain entity IDs remain a library concern. Keys follow the same selected target across backing growth, without promising continuity across incompatible object reuse, compaction or owner replacement. No generation table, exposed raw-offset API or runtime borrow registry follows.

## Lexical ADTs as part of the complete model

The lexical pointer-bearing ADT goal is technically compatible with same-scope offsets: when an admitted lexical object's stored references have that object's owner, its fields can use offsets in that owner's coordinate system. Regional instances retain slot-relative edges. This supports a bounded lexical graph model without first allowing arbitrary cross-owner fields.

H-478 selects **one nominal ADT** across regional and lexical storage. An otherwise admissible pointer-bearing ADT may use either placement under the same owner/access/initialization rules, with the actual placement supplying layout and addressing evidence. Business declarations, conformance and semantic methods are shared. The compiler must adapt admitted field/view operations automatically. Concrete layout/ABI, complete proof coverage and implementation remain outstanding; bits-only lexical construction is an implementation fallback, not a competing nominal-type design. Layout equality is neither required nor sufficient for nominal equality; pointers encoded as integers remain non-bits.

Schematic CoD illustrates the selected common source-method contract; it does not claim current aggregate compiler support:

```text
data Node { value:u64; ptr next:Node; }
proc next_of(n:Node in O) -> ptr Node in O where {owner O} {
    return n.next;
}
```

The parameter binding may live in the callee while its Node lives in a different owner. Resolve that object through the admitted inter-owner view, then interpret its field using the object's layout and owner. A regional next field is slot-relative; a lexical same-owner next field uses its scope's offsets. The returned view preserves O and nullability without reading its target, and is adapted to the caller's receiving storage. One implementation strategy specializes known layouts; another shares code with retained layout evidence. Neither requires a semantic type or native body per owner instance.

| Construction / field form | Integrated requirement |
| --- | --- |
| Direct bits/val field | Initialize its typed payload in the containing owner's storage. Preserve numeric validity, including D-16 nonzero denominators; no blanket zero initialization. |
| Nullable ptr field | Establish its fixed owner even when null; later rebinding may connect initialized same-owner nodes. Define null and valid-offset encodings without conflating a slot locator with a null reference. |
| Required ref field | Prove an initialized non-null same-owner target before a readable completed object is published. A cycle of required refs does not become constructible merely because offsets support cycles. |
| Same nominal ADT in lexical or regional storage | Choose layout/access from actual placement evidence, retaining method/type identity and owner rules. Do not infer placement from the scope of the temporary variable holding a view. |
| Nested inline payload / product / sum / array | Direct bits constituents retain their category. Admit additional reference-bearing inline layouts only with explicit field, alignment, active-alternative and array policies; ordinary lexical ADTs do not imply arbitrary recursive by-value graphs. |
| Pointer to an outer scope or region | Ordinary lexical bindings retain their existing permission and inter-owner interpretation. A pointer field inside a new lexical ADT remains subject to H-203 same-owner rules; H-478 does not turn it into a cross-owner view record. |

The [known first-field quirk](#known-first-field-self-reference-quirk) has a user-side layout workaround and does not block this work.

[Owner-qualified construction](#owner-qualified-construction) supplies the common `make T in O` permission for caller-owned as well as current-scope storage. A helper with owner O may allocate there; const owner O cannot, even when the helper may edit existing writable targets. Callee-local allocation cannot stand in for caller-owned graph nodes. The underlying allocation/representation dispatch still needs to satisfy the actual owner's policy and admitted placement.

Storage without an observable address can remain in registers; bounded addressable storage can remain on the stack when all alias/call paths are implemented consistently. A generic operation that lacks those proofs needs a compatible materialized representation. One source ADT therefore need not imply one native allocation strategy.

### Consequences of one nominal ADT

The following source consequences follow H-478 and the existing owner, field, dispatch and operation contracts; they require no additional language mechanism or repeated choice of the storage model.

| Surface | Selected consequence | Limit retained |
| --- | --- | --- |
| Type identity and conformance | Node denotes the same nominal data type in both placements; generic T and nominal assertions retain that identity. | Complete view types still carry owner, access, mode and nullability. Same nominal identity does not equate distinct owners or storage layouts. |
| Semantic method selection | Use the same declarations and ordinary type/constraint rules for either placement. Layout-dependent native access is an implementation choice beneath the selected method. | Placement is not a new implicit overload dimension or public NodeLex/NodeRegion type. Existing explicit owner/capability constraints still apply. |
| Construction | `make Node in O` uses the actual selected owner and its allocation access/path. Both direct bits fields and same-owner reference slots belong to that owner. | Const owner cannot allocate. A callee's own scope is not a substitute for the requested caller owner; construction does not grant resource ownership. With no contextual destination, non-bits construction still requires an explicit owner, which may be `in here`. |
| Field access and slot aliases | Decode using the containing object's placement, even when the local name or alias lives elsewhere. Preserve a selected slot's encoding when forwarding it. | Never infer target layout from the scope of the receiving variable or reinterpret offset words as independent bits. |
| Reference assignment and view transport | Preserve logical target/owner and encode for the destination view/slot. The compiler retains or shares sufficient layout/owner evidence across calls and returns. | Same-owner checks remain; an identical native integer is not an interchangeable reference representation. |
| Bits transfer | Read/write admitted direct bits fields using their ordinary type/conversion rules. Their enclosing ADT may itself be non-bits. | No whole-object byte copy, padding copy, new numeric conversion or fraction validity assumption. |
| Lifetimes and completion | Lexical objects live until their actual full scope ends; region objects retain their region lifetime. Results, defers, failures and cleanup preserve the selected targets. | No escaping callee-local graph, implicit promotion to a region, extension of owner lifetime or rollback of published effects. |
| Storage optimization | Registers/stack may replace materialized lexical storage when the complete observable contract is preserved; growable storage resolves through its current owner information. | This does not permit placing a C++ regional_type on the stack or changing D-01 constructors/copy rules. |
| Captures, TLS and intrinsic resources | Existing D-10/D-15/D-18 eligibility and region-backed symbolic-object requirements continue. | Lexical ADT availability grants no lexical-cell capture, inline TLS graph, intrinsic owning field or runtime self-capture timing policy. |

A target view into a regional Node remains regional when passed through a lexical slot; a target view into a lexical Node remains lexical when called through an owner-generic signature. The semantic method needs each target's actual layout, not one global layout guessed from the nominal type. Known layouts may use direct code; genuinely varying admitted layouts carry sufficient shared evidence. Retain different actual type/layout facts within one owner as needed: owner-base sharing is not proof that every object in that owner has one type/layout. Neither one native body per owner nor a universal per-pointer descriptor is required.

H-478 changes placement eligibility, not the bits predicate. Adding ptr/ref fields still invalidates bits-only uses, ordinary independent-value copying, bits TLS and relevant generic constraints even when lexical allocation is admitted. Nested reference-bearing inline products, sum payload encodings and array strides need their own admitted layouts; no recursive by-value object or automatic non-bits return copying follows. Unsupported combinations receive diagnostics until their representation and proof coverage exist.

### Construction, growth and field provenance

```cod
data Node { value:u64; ptr next:Node; }
ref local = make Node in here { value <- 1u64; next = null; };
ref regional = make Node in r { value <- 2u64; next = null; };
# null receives its owner from the containing field's declared contract.
local.next = regional; # Reject: same nominal Node, different target owners.
```

Readonly access remains transitive through the selected fields; changing representation cannot upgrade it. A previously initialized nullable edge may be rebound to an initialized same-owner Node. Required ref fields retain the initialization-flow contract and cannot publish a readable completed object before its obligations are proved. Source ordering, exceptional completion and allowed allocation/publication effects remain unchanged. This does not introduce a group allocation checkpoint or forward data binding.

Lexical backing growth preserves all existing object/field locators and logical identities. Later resolution uses the current base; a nested call allocating in O cannot leave stale raw addresses for a pending read, write, return or deferred operand. A field's slot owner remains the containing object's owner. Offsets are not reinterpreted using an unrelated caller/callee scope merely because its variable holds the view. Region remapping retains zero per-edge repair; consuming resize remains a replacement-owner event that invalidates old views.

### Known first-field self-reference quirk

Under the current regional encoding, assigning an object's address to its first pointer field stores zero displacement, which reads as null (H-146/H-479). H-480 leaves this quirk to users: place a nonzero-sized unused field before that pointer when a self-link is needed, for example:

```cod
data Link { padding:u8; ptr next:Link; }
```

Keep this note and workaround; do not add special compiler analysis, a dedicated acceptance program or encoding-repair work for it. Existing implementation checks are unchanged. Other design and implementation proceed normally.

### Composed acceptance and engineering sequence

The source contract is settled for the scope above; readiness is checked by composed evidence, not by the existence of a local offset struct.

1. Check one nominal Node and its conformance through both placements, a common owner-generic reader/constructor, and calls where binding scope differs from target owner. Keep owner/access qualifiers and reject cross-owner fields and local escapes.
2. Cover direct bits, nullable edges, aliases and same-owner cycles within admitted initialization rules.
3. Exercise allocation/backing changes during argument preparation, field writes, returned views and cleanup, preserving selected places and shared actual-owner metadata. Preserve region relocation and consuming resize boundaries.
4. Exercise library transfers across all four storage pairs, including shared nodes/cycles, source/destination growth and exceptional partial effects, without byte-copying pointer encodings or adding automatic traversal.
5. Reject unauthorized captures/TLS placement, owning fields, unproved initialization and unsupported aggregate layouts. Verify pure bits/fraction constraints and existing source behavior remain unchanged.

Use a shared semantic place/view operation for regional and lexical access, then supply concrete owner/layout resolution at lowering. Integrate aggregate construction and initialization summaries with that operation instead of introducing a second nominal type system or parallel checker. Share evidence by proven owner/layout facts; specialize access paths only when useful. Exact descriptor/offset widths, allocation policy and native code sharing stay engineering choices constrained by the contract. Measure both memory/code costs and the complete acceptance cases before claiming overhead or coverage; no benchmark or compiler support is asserted by this document.

### Graph transfer with growing source or destination

Region-to-region, region-to-lexical, lexical-to-region and lexical-to-lexical library graph transfers therefore share one semantic interface. Each source/destination access uses its own layout evidence. Copy admitted bits, allocate admitted destination nodes, and construct destination edges under the library algorithm's existing identity/topology/publication policy. A returned view of the source cannot substitute for a new destination edge with a different owner. Mutable source/destination callbacks may grow either backing; retain logical selected places and memo identities, not native addresses. No traversal, automatic graph clone or transactional failure policy is added by the compiler's addressing adaptation.

H-206/H-208/H-351 continues to put graph policy in selected user/library `(<-)` methods. Automatic addressing means the algorithm's field operations, bits transfers and edge assignments use the correct representations; it does not provide a default graph copier.

For a topology-preserving library transfer, the conceptual work is to select source/destination policies and capabilities, establish a destination node for each admitted source identity, transfer typed bits, then connect destination references to the corresponding destination nodes. Memoization and publication order must handle sharing, cycles and required-ref initialization. An algorithm may use a different order or intentionally change topology when its contract says so. Source and destination overlap, reuse, callbacks and partial writes remain part of that contract.

A growing source invalidates address-based memo keys; a growing destination invalidates cached construction addresses. Both must retain the selected logical identities/locations across allocation. Source refs cannot be stored directly as destination edges merely because an offset is numerically representable. Existing valid allocations/publications need not roll back on later failure under H-428. A transfer to a fresh region and consuming region replacement retain their distinct cleanup rules.

The minimum coherent acceptance for lexical pointer-bearing ADTs includes construction capability, required initialization, field access, inter-owner call transport, graph transfer and final scope cleanup. An implementation may retain a diagnosed bits-only lexical subset until that coverage is delivered; it must not invent separate nominal types or accept unsound pointer-bearing cases to bridge the gap.

## Captures, TLS, native interfaces and persistence

Offset-backed lexical storage supplies no implicit capture permission. D-18 admits independent value snapshots, selected ordinary reference values and explicit var place aliases, with lexical borrows requiring a static non-escape proof. An ordinary reference snapshot does not alias its source binding slot; an explicit place alias does. A lexical ADT's fields do not become capturable merely because their storage can grow. No closure abstraction promotes cells, retains frames or keeps borrowed owners alive; an owner-resolution context supplies current address information, not ownership. See the [compiler alias rule](CallableViewsAndCaptures.md#compiler-rule-target-sharing-is-not-binding-slot-aliasing) and [vision](GlobalVariablesDoneRight.md#explicit-lifetimes-without-closures).

Complete callable values require representation-safe relocation/copying of their whole intrinsic environment, including nested values. Retained ordinary capture values and invocation-local copies have distinct semantic storage; explicit borrowed places/targets preserve their external owners. Their internal environment offsets and a lexical owner's storage offsets are different coordinate systems. Lowering must compose them correctly. The existing complete-value contract does not establish that current cleanup-list links, hidden output pointers or native environment addresses already survive scope-fragment growth.

TLS payload categories and effect restoration remain D-10/D-15 contracts. H-482's explicit ref/ptr symbols retain the region selected through their declared owner symbol at installation. H-561's owner-parameter form also admits lexical and ordinary regional targets without an owning-region symbol. Each installed/captured/restored reference retains its actual owner and locator, resolved against that owner's current backing; selector shadows or repeated binder spellings cannot retarget it or equate distinct owners. Shared resolution information grants no ownership or lifetime extension. Bits and admitted callable payloads retain their value behavior. TLS holds the reference, not an inline lexical graph; graph fields retain their same-owner constraints. Restoring a binding cannot revive a consumed region or ended lexical owner. D-10 owns the source acceptance and staged implementation boundary.

Scope exit must keep the state needed by remaining deferred work, prepared results and intrinsic releases valid until their last specified use. Leaving a full scope ends its owner even if backing happens to be retained or reused by an allocator. No new per-object destructor, persistent activation, detached heap lifetime or continuation-capture facility follows. Declaring storage relocatable does not by itself make a suspended execution serializable.

### Foreign interfaces adapt to CoD semantics

H-443 makes completeness and consistency of CoD’s own semantic system the priority. Foreign interfaces receive no additional language-level promise of address stability, layout compatibility, lifetime extension, pinning or allocation behavior. External language/runtime integrations adapt to CoD’s actual owner, locator, access, construction and completion rules. This boundary applies to foreign adapters; CoD-generated native calls and runtime mechanisms must implement the complete selected CoD semantics themselves.

A raw address is usable only for an interval in which the selected target and its backing remain valid. A callback that may grow a lexical owner prevents assuming that an earlier address still resolves its target. Const target or const-owner access is not a global stability guarantee: another admitted access path may allocate. Consuming replacement or scope exit also ends the relevant validity. A reference’s encoded bits need not be a C/C++ pointer, and neither a shared context nor the proposed external_ptr name establishes a foreign ABI.

An integration can use an owner/locator-aware bridge that resolves current addresses at each required access, a proved stable interval, or explicit data marshaling whose copy/alias behavior is part of that integration’s contract. These are adaptation strategies, not new guaranteed CoD APIs. Adapters must preserve actual owners, permissions and declared call/result/cleanup behavior; copied payloads cannot silently stand in for aliases, and stale raw pointers cannot be treated as valid references. An unsupported foreign usage must be identified as an integration limitation instead of changing otherwise valid CoD allocation or reference behavior.

When foreign code saves an address, invokes an allocating CoD callback and then accesses the target again, CoD may continue allocating in the callback’s non-const owner. The bridge must arrange valid access after the callback; C/C++ code cannot keep using an earlier address if backing moved. CoD does not promise to pin lexical storage, fail growth solely because foreign code saved an address, or automatically place objects in a region to satisfy that foreign API. An explicit region retains its ordinary capacity, replacement and lifetime rules and is not a blanket foreign-address guarantee.

Acceptance: describe each admitted adapter’s representation, access and lifetime limits; exercise callback growth/owner replacement where that adapter claims support, and reject or exclude unsupported shapes. Preserve the same CoD-visible reference, allocation and cleanup semantics across adapters. A general foreign ABI or pinning subsystem is not a prerequisite for completing this refactor.

### Persistence boundaries

Regional images and DBMR preserve their physical graph/storage contracts. A lexical slot locator, scope anchor or runtime callable address is not a persistent reference. A library may transfer graph data into a region using its selected schema, topology and failure rules. Optional YAML serialization, file publication, mapping permissions and durability remain separate. This review neither changes the region file layout nor introduces an automatic lexical-memory snapshot.

## Global contract and source impact

The table identifies required design integration and current source audit surfaces. It is an impact plan, not permission to activate pending mechanisms. Existing qualified behavior remains the acceptance baseline; only the named extensions remain under review.

| Contract / surface | Required integration | Boundary retained |
| --- | --- | --- |
| [D-01 / regional storage](MemoryRegionAndRegionalTypes.md), `include/shilos/region.hh` | Keep slot-relative edges distinct from lexical slot locators and inter-owner handles. Preserve range, alignment and existing regional encoding invariants. | Region-only C++ objects, no per-object copy/move, allocator symmetry and zero per-edge relocation work. |
| [D-07 / data and layouts](DomainLanguageDesign.md) | Apply H-478's one-ADT placement contract with sufficient concrete field/sum/array evidence. | Bits eligibility, nominal type/conformance identity and SoA-related constraints; unsupported lexical layouts retain explicit diagnostics. |
| [D-14 / owners and assignment](RegionalOwnership.md) | Retain slot owner versus target owner through rebind, swaps, aliases, initializers and borrowed results. | Fixed owner, access, nullability, initialization and destination-result identity. |
| [D-22 / construction](ConstructionAndAcquisition.md) | Implement owner-qualified construction, graph publication and representation-aware initialization. | Explicit actual owner; H-438 allocation access; allowed effects and no checkpoint/rewind. |
| [D-17 / completion](ExceptionControl.md) | Keep deferred selections/results and necessary owner contexts valid across growth and cleanup. | Existing order, replacement, exact release and consuming-call boundaries. |
| [D-18 / callable values](CallableViewsAndCaptures.md) | Adapt complete environments, prepared arguments and hidden caller destinations to selected storage; audit nested coordinate systems. | Complete values, borrowed captures, existing capture permissions and caller lifetime. |
| [D-23 / scalar reception](ScalarCallResults.md), [D-24 / inferred results](CallableResultInference.md) | Transport inferred or explicit borrowed results with target/layout evidence; retain value snapshots and receiver materialization. | Result mode and inference anchors do not depend on native pointer width or allocation strategy. |
| [D-21 / native preparation](AheadOfTimeExecutionPreparation.md), [D-26 / coverage](AheadOfTimeExecutionPreparation.md#generic-and-recursive-native-coverage) | Cover each admitted layout/access path, allowing shared code or prepared specialization. Keep layout facts separate from same-T evidence. | No semantic type per layout, implementation per owner instance, runtime method registration or loss of outside-catalog behavior. |
| [D-10 / symbols](SymbolicDynamicScope.md), [D-15 / worlds](ModuleWorlds.md) | Preserve current/captured symbolic reference interpretation and owner dependencies during adaptation/restoration. | No TLS category or backing expansion, foreign-world access or retained-owner authority. |
| [D-16 / fractions](FixedWidthFractions.md), numeric preparation | Apply typed payload transfer after resolving locations; preserve validity and representation-independent units. | No fraction implementation, promotion, zero-validity or pointer-as-bits classification. |
| Checked representation: `cod/lang_checked.hh`, `cod/lang_callable_flow.hh`, `cod/lang_completion.hh` | Review selected slots/targets, actual owners, alias selections, available/initialized state and completion effects. Add sufficient representation facts only after the corresponding contract is qualified. | Existing proofs must not be replaced by a raw address or a runtime tag that excuses ambiguous ownership. |
| Native path: `cod/lang_llvm.hh`, `cod/lang_native_runtime.hh` | Audit `binding_slot`, `binding_address`, local construction, rebind/transfer/swap, argument/result projection, deferred records and cleanup. Their current address-based paths are not evidence of moving-frame support. | Once-only selection, ordered evaluation and correct native behavior for the current admitted subset. |
| C++ naming, YAML and fixtures | Inventory `prelude.hh`, `region.hh`, `str.hh`, vector/list/dict YAML helpers, shilos fixtures, architecture/micro-dispatch experiments and current documentation/instructions for a coherent external_ptr migration. | A rename changes neither regional representation nor CoD lexical transport; optional serialization remains separate. |
| Foreign adapters, diagnostics and tooling | Document adapter representation/access/lifetime limits and unsupported shapes; display semantic owner/target identity across relocation. | H-443 requires adaptation to CoD semantics without extra stable-address/pinning promises or changed ordinary growth. No implicit raw-pointer cast, general foreign ABI or persistent stack snapshot. |

The C++ handle rename and lexical representation are separate migration surfaces: today's global_ptr retains a region-header reference plus an offset, not a refreshable lexical owner anchor. Shilos regional C++ objects must not be made stack-constructible or movable to implement CoD lexical ADTs. Any lexical native representation needs its own qualified lowering while preserving existing regional APIs until a coordinated migration is selected.

## Remaining engineering and escalation boundaries

| Work item | Engineering direction and escalation boundary | Selected source contract |
| --- | --- | --- |
| Observable format/capacity commitments | Escalate only a proposed binary-compatibility, exposed-layout or representable-range change that existing contracts do not settle. Internal layout/transport alternatives are engineering review. | H-478 fixes one nominal ADT and automatic addressing; same-owner fields and the bits predicate remain. No new format or capacity promise is selected. |
| Identity-key and allocation transport | Provide usable memo keys and the actual owner construction path while preserving target identity, field-owner constraints and key lifetime. | H-439 selects owner-plus-locator identity and is; H-440 selects is null. H-438 permits non-const owner construction without a separate builder. Library graph policy and allowed effects remain. |
| Remaining physical transport and encoding | Implement shared owner contexts with suitable locator transport, reference encoding, width/range/alignment and metadata lifetime. Escalate user-visible layout/compatibility trade-offs; routine lowering choices need not reopen the source rule. | H-437 owner-information sharing and sufficient dereference evidence are selected; H-436 slot/payload distinctions and optimization remain. |
| Regional resize backend and API migration | Complete exact capacity/error/backing-file review and coordinate the proposed C++ rename independently. | No automatic/in-place CoD region growth; consuming cleanup and fresh result ownership remain. |

The selected lexical-capacity/slot-content requirements and H-478 nominal contract stand independently. Resolve routine encoding, metadata and lowering choices within their existing constraints. Escalate a concrete new source/lifetime boundary or observable format/capacity trade-off; do not reopen nominal unity, require every lexical variable to allocate on the heap or treat offset integers as bits eligibility.

## Remaining CoD integration

Owner-information sharing, qualified construction, reference identity and null testing are selected under H-437–H-440. H-443 fixes foreign adaptation as an integration boundary with no extra language promise. Their direct consequences are resolved under the user’s explicit delegation; no further vote is needed on these mechanisms. The following source-level questions remain distinct from routine choices about how to store a context or pass a hidden argument. Examples are CoD design sketches, not claims of current compiler support. Complete routine integration directly and discuss only material unresolved source/lifetime choices; recommendations are not approvals.

### Selected ADT placement contract

```text
# Candidate ordinary construction forms; exact aggregate support remains pending.
ref local = make Node in here { value <- 1u64; next = null; };
ref regional = make Node in r { value <- 2u64; next = null; };
```

H-478 selects one nominal Node with storage-specific field interpretation supplied by placement/owner evidence. Same-owner fields give local.next owner here and regional.next owner r; cross-owner rebinding remains invalid. The [derived source contract and composed acceptance](#consequences-of-one-nominal-adt) governs further engineering. Physical layout/encoding and complete implementation coverage remain outstanding, without reopening nominal identity or adding public pointer types for the encoding.

### Required-reference cycles during construction

**Current contract: H-562 selects bare typed reservation, complementing H-445–H-447's staged initialization-flow direction for ordinary proc/func.** Under H-444's cost criterion and H-445's flow principle, the combined assessment below finds moderate compiler effort and controllable debt when shared with aggregate construction. General arbitrary-graph proof remains expensive. H-317's nullable-edge route remains available; the non-null staged route uses the distinct reservation/completion contract in D-22, with detailed proof/native support and full value-path qualification still separate. This is neither a permanent prohibition nor a prerequisite for lexical pointer-bearing ADTs. Future pure knot-tying has its own affirmative research goal in [D-19](ConstFunctionsAndTailCalls.md#knot-tying-as-a-pure-design-goal).

Nullable cycles can be assembled without exposing an incomplete non-null reference:

```text
# r is an existing live non-const owner; these are design examples.
data Link { ptr next:Link; }
ref a = make Link in r { next = null; };
ref b = make Link in r { next = a; };
a.next = b;
```

A separate source requirement arises if the graph type promises non-null edges:

```text
data RingNode { ref next:RingNode; }
# Desired completed graph: a.next is b && b.next is a.
# With no pre-existing RingNode, serial construction has no valid first target.
```

D-14 requires a ref to denote an initialized, non-null target. Merely reserving two addresses cannot establish that guarantee. An ordinary initialization group still uses preceding-environment lookup and source order; it does not introduce forward data bindings. Callable recursion support does not supply recursive object initialization. This applies to both ordinary callable kinds: func excludes dynamic effects, but that alone does not prove target initialization, non-escape or cleanup safety.

The runtime work for a finite, known graph is small in principle: reserve the nodes, encode the edges once their locators are known, then expose completed objects. Neither a general lazy evaluator nor per-reference runtime initialization tags are intrinsically necessary. Both target identities can be established before filling either node; cyclic construction need not require forward name lookup or recursive declaration groups. H-562 obtains typed incomplete aggregate destinations with bare `make T [in o]`; the initializer-block form instead requires completed construction. [D-22](ConstructionAndAcquisition.md#typed-reservation-and-completed-construction) owns the distinction and finite-cycle example. A separate literal-only cyclic-construction facility is not recommended; the flow model below should provide the common machinery.

Nullable construction preserves an observable limitation: the field type stays ptr after linking; constructing a complete cycle does not convert the nominal type to one with ref fields. Library invariants can restrict updates, and ordinary non-null checks/refinements still govern access. No implicit promotion, unsafe publication, runtime registry, graph copy or rollback is added. H-444's pure goal remains open without blocking unrelated storage refactoring.

### Initialization obligations follow control flow

H-445 establishes the underlying principle: initialization is tracked before and after operations throughout a code block, not only when an explicit init parameter passes responsibility across a call boundary. The proposed parameter qualifier exposes a callable's entry restrictions and successful-completion guarantee; it is not the origin or sole carrier of initialization state.

Track selected storage and the facts valid at each program point. Distinguish an established target identity, an initialized reference slot, and an initialized referent payload. In the proposed construction context, `a.next = b` initializes that reference slot and records its selected target B; it does not read or initialize B's payload. `a.next is b` can then compare identities, whereas a payload read through that edge still needs B's initialization evidence. Before the assignment, an uninitialized a.next slot cannot even supply an identity. Field-slot projection itself needs valid owner/layout evidence.

Aliases share the target relationship, not independent copies of a permanent init type qualifier. Discharging B's obligation makes the relevant evidence available through aliases still targeting B at subsequent program points; it does not retroactively legalize earlier reads. Rebinding an edge changes which target its later uses follow. Reference copying neither initializes the referent nor creates a separate obligation to write it through every alias.

```text
# Semantic flow example.
val n:u64 <- make local u64;
ptr p:u64 = n;                # Target identity exists; payload is unreadable.
val same <- p is n;           # Identity-only observation.
# val early <- p;             # Rejected before initialization.
p <- 7u64;                   # Establishes initialization of n's storage.
val later <- n;              # Reads are now valid through n or p.
```

At a branch join, retain only initialization facts guaranteed by every reachable incoming path. A loop body that might not execute supplies no unconditional guarantee. An init helper's successful completion can discharge the caller's corresponding obligation; failure supplies no such blanket fact and performs no rollback. A deferred write supplies evidence when it executes, not at registration. These are flow consequences, not new runtime flags or cleanup rules.

Uninitialized local storage does not by itself require a pointless write before being abandoned. Construction/result contracts and the proposed init parameters determine which obligations must be discharged at their completion boundaries; every actual payload read independently requires valid evidence. The initial proposal's mandatory write, including treatment of already initialized arguments, must be reflected explicitly in the callable contract rather than assumed from a caller's pre-existing readable state.

The bounded implementation direction reuses target relationships and definite-initialization summaries, adding aggregate field tracking where needed. Unknown alias/mutation cases may reject conservatively. Candidate details still needing review are partial versus whole-object completion, writes of incomplete identities into already published graphs, rebinding, and indirect-call contract preservation. Normal readable-ref interfaces cannot silently erase an outstanding restriction. No provisional-pointer runtime representation or change to nominal Node identity is implied. D-16 payload validity remains required when reading numeric fields.

### Combined implementation cost and debt

**Assessment: moderate compiler work, low additional runtime machinery, and controllable debt when implemented with the common aggregate initialization model.** This supports carrying bounded initialization flow into the construction design. It is not a small isolated keyword patch or a promise to prove arbitrary graph algorithms. Delivering complete source/native support from the current compiler is a larger aggregate/construction increment: the shared ADT/layout baseline is not already implemented. Engineering recommendations here do not qualify the pending language contract.

Committed source `8914d0f3914d4586789f0461f187bb428fa54446` supplies substantial reusable machinery in `cod/lang_checked.hh`: selected targets/owners/versions and per-path initialization (`:66–184`), read/write summaries and call substitution (`:1143–1200`), initialization/cleanup obligations (`:1361–1390`), and recursive summary fixed points (`:5319–5355`). `cod/lang_completion.hh:27–48` distinguishes current selections from entry targets. Ordinary aggregate field selection still reports an implementation gap at `cod/lang_checked.hh:4480–4484`; current argument modes in `cod/lang_syntax.hh:160` do not include init. These are bounded source-inspection facts, not benchmark results; the [capability reference](../tests/cod/language/README.md) owns current supported-source claims.

| Area | Incremental work / cost |
| --- | --- |
| Local writes, branches, aliases and cleanup | Low to moderate extension of existing facts and joins; preserve original selected targets and execution order. Do not build a second analyzer for init. |
| Aggregate fields and identity edges | Moderate core work: model field places, initialized reference slots, possible targets and per-field payload readiness. Much of this is required for ordinary ADT construction anyway. |
| Explicit init call contracts | Moderate: parse/check the qualifier, preserve it in callable types and indirect calls, and prove mandatory writes on every normal completion. Existing readable state cannot discharge a newly promised write. Extend summaries to fields/target relationships, rather than only top-level parameters. |
| Cyclic completion | Moderate for a finite tracked component: prove concrete field writes and valid closed required-reference dependencies before ordinary publication. A cycle alone cannot prove initialization; uninitialized bits or unresolved edges remain failures. |
| Owner/layout/backend integration | Reuse the selected owner-plus-locator model and existing construction stores. No extra pointer word, runtime initialization registry, lazy evaluator, graph copy or rollback is inherently needed. General ADT/layout support is a shared prerequisite, not all attributable to init. |
| Arbitrary graph mutation and unknown aliases | High if exact proofs are promised for dynamic containers, unbounded traversal, opaque helpers or publication through unknown aliases. Start with conservative diagnostics when evidence is insufficient. A merged abstract target representing several objects cannot be marked fully initialized merely because one object was written. |

The critical boundary is ordinary readable publication. After `a.value <- 1u64; a.next = b;`, a's own slots may be filled while b.value is still unreadable. An ordinary consumer able to traverse a.next must not acquire a stronger guarantee. Tracked identity-only uses can remain valid; calls, callbacks, writes to already published graphs and failure paths must preserve the outstanding restriction or reject. Complete private component construction is the simpler initial case. No blanket transaction or new exclusivity mechanism is selected.

Debt stays low by keeping readiness/mandatory-write facts separate from nominal types and runtime pointer encodings, using one field/target abstraction across local flow and call summaries, and sharing completed-object checks with ordinary constructors. Avoid special literal graphs, copied init flags requiring alias-wide clearing, and unsound optimistic facts after unknown mutation. Budget analysis precision explicitly: conservative joins can reject safe programs, which is an implementation limitation rather than permission to weaken the language rule.

Recommended integration order: field places and readiness in the common checker; local identity propagation and normal/exceptional completion; explicit init contracts using the same summaries, including callable-type preservation; then finite cyclic construction and its ordinary-publication proof. H-562 settles typed reservation through the presence or absence of an initializer block; no new allocation keyword is required. Focused future acceptance must cover early-read rejection, writes through aliases, branch/zero-iteration joins, delegated and recursive calls, failure/defer timing, a two-node required-ref cycle, and a pending target reachable from a would-be published object. Measure compiler convergence and memory as target/field sets grow; do not promise linear whole-program checking or universal proof precision. Pure knot-tying can reuse this foundation, while its evaluation model remains separate.

### Two-stage acceptance with invariant soundness

H-446/H-447 confirms minimum-effort acceptance of basic/common cases first, with later investment in recognizing specifically needed cases. Soundness is mandatory in both stages. Language meaning stays stable while proof precision improves: an unproved program receives an explicit diagnostic, and no accepted program may rely on a future analysis enhancement to justify its behavior. This confirms the staged plan and its bounded coverage, while detailed init/construction mechanisms outside that scope remain pending.

**Phase 1: one small, complete proof model.** Reuse the existing flow/call/cleanup machinery. Track definite initialization and required writes for known storage places, possible reference targets and their mutation versions; extend the same representation to supported fixed-layout product fields. Initial coverage targets are:

- Sequential writes, identity-only observations, exact local aliases and field-reference assignments with known target relationships.
- Ordinary if/else joins: retain facts guaranteed on every reachable incoming path. No symbolic arithmetic or cross-branch correlation solver is required.
- Statically selected init helpers with checked entry/exit contracts, composed using summaries. Straightforward delegation is in scope; recursive or indirect cases need sound available summaries or receive an unproved diagnostic. Callable-type conversion must never erase the qualifier, even when invocation support is deferred.
- A finite tracked construction component, including a small required-ref cycle, using H-562 typed reservation once the corresponding field/proof support is implemented. Establish all required payload writes and reference dependencies before ordinary readable publication. No recursive declaration group or arbitrary graph traversal proof is required.

The scalar/direct-place portion can be delivered before aggregate support; claiming the aggregate portion requires its actual field, representation and completion checks. This is implementation sequencing within phase 1, not permission to use unsound placeholder behavior. Existing qualified supported programs retain their ordinary rules.

Initially reject new cases whose safety depends on unresolved aliasing, runtime-indexed elements, opaque mutation, publication through untracked existing graphs, precise loop invariants or correlations beyond this model. A loop need not reject merely for being a loop: retain invariants that are proved, but do not claim a write from a body that may execute zero times. Unknown calls can preserve independently established facts only with a justified effect bound; invalidating a fact cannot undo an escape or read that already occurred.

The non-negotiable proof obligations are:

1. Payload reads require initialized evidence for every possible selected target; identity use still requires initialized reference-slot contents and live owners.
2. A write through an exact destination establishes that destination's fact. Writing one of several possible objects cannot establish that all were written; retain only facts justified by the tracked selection.
3. Every normal completion satisfies promised writes and readable-result/publication requirements, including dependencies through required-ref edges. Pre-existing readable state cannot substitute for the promised write.
4. Argument preparation, exceptions, catch, defer and final cleanup are included wherever they affect these claims. Unsupported combinations reject. Failure supplies no blanket success guarantee and no rollback.
5. Alias mutation, rebinding, calls and owner end/replacement cannot leave stale usable evidence. All selected implementations of an admitted call must meet the retained contract; recursive assumptions cannot excuse an actual early read or unwritten normal return.

**Phase 2: demand-driven improvements to proof precision.** Add bounded field/array relationships, useful loop invariants, recursive construction summaries, indirect-call precision or publication patterns when a concrete application needs them. Each increment must explain the abstract facts and their sound transfer/join rules, preserve earlier contracts, and exercise both the newly accepted case and nearby invalid cases. Analysis limits produce diagnostics rather than optimistic success. There is no promise of recognizing every safe program or a deadline for this phase.

Debt control: keep one analyzer and a visible distinction between a proven violation and insufficient proof. Diagnostics should identify the target/path or unsupported relationship. Do not turn temporary proof limits into permanent syntax bans, implement a competing cyclic-literal checker, add per-pointer runtime init flags, or erase qualifiers to broaden acceptance. Future acceptance evidence should cover both branches, zero-iteration loops, same/different aliases, caller-slot mutation, ordinary/cyclic publication and cleanup/failure paths. This document requests no test execution or source implementation.

Region resize's backing-file/capacity details and the coordinated C++ naming migration remain separately scoped work. They do not block recording the owner-sharing rule or require another vote on per-pointer owner overhead.

## Review sequence and acceptance

This task authorizes documentation only. The following are contract-review units and future acceptance requirements, not implementation or test execution instructions.

1. Keep the selected owner, slot/content and optimization baseline coherent across D-14/D-22/D-28. Specify inter-owner views and address invalidation without fixing an unnecessary universal pointer ABI.
2. Complete lexical ADT placement and integrate the selected owner-qualified construction and reference-identity contracts, including concrete key transport. Reconcile the exact affected D-01/D-07/D-14/D-22 scopes; do not infer qualification from the ancestor diagram or a compiler prototype.
3. Compose calls, results, complete callable environments and cleanup with D-17/D-18/D-21/D-23/D-24/D-26. Preserve TLS/capture and persistence exclusions.
4. Qualify CoD physical encoding, runtime transport and migration acceptance; document foreign adapter limits separately under H-443. A later separately authorized source task can then implement the relevant scoped contract and update current capability evidence.

| Acceptance case | Required observation |
| --- | --- |
| D28-A01: slot versus contents | Same-owner, other-lexical-owner and regional-target references resolve correctly; null is distinct from slot location. Moving only the slot owner never changes an external target. |
| D28-A02: optimization equivalence | Register/stack/scalar-replaced and materialized forms preserve alias identity, reads/writes, initialization, call behavior and scope cleanup. No mandatory heap frame appears. |
| D28-A03: assignment and swaps | Growth and rebinding between operand evaluations preserve original selections; reference swaps re-encode each destination and bits swaps preserve both old payloads. |
| D28-A04: prepared calls and results | Later-argument allocation, recursion and cleanup growth retain selected targets/places and value snapshots. Invalid owner escape still rejects. |
| D28-A05: construction and failure | Earlier valid initialized nodes survive later allocation failure while their owner lives; incomplete required refs do not become readable; no implicit allocation rollback occurs. |
| D28-A06: graph transfer | Exercise all four lexical/regional source/destination pairs with null, sharing and cycles, overlapping inputs where admitted, growing owners and exact library failure expectations. Stable identity keys survive permitted growth. |
| D28-A07: callable and deferred state | Nested complete callable values and prepared destination/target references survive permitted movement; deferred ordering and completion replacement remain exact. Accept D-18 value captures and proved explicit owner-bounded borrows; reject implicit cell promotion, source-slot sharing for ordinary reference copies and unproved escapes. |
| D28-A08: layout and type evidence | Generic/shared paths distinguish layout access from nominal identity and same-T; code sharing cannot erase required concrete evidence or owner proofs. |
| D28-A09: lifetime distinctions | Lexical backing replacement preserves its owner; consuming region resize ends old views; container movement/reuse receives no automatic exemption. |
| D28-A10: native and persisted boundaries | CoD-generated code preserves owner/locator correctness through growth. Foreign adapters state and meet their limits without extra pinning/stability promises or changed CoD growth semantics; lexical anchors/addresses do not enter regional persistent images by implication. |
| D28-A11: numeric and source compatibility | Preserve val/ref initialization operators, RHS pending boundaries, `#` comments, reserved fraction syntax, typed numeric conversion and nonzero-denominator validity. |
| D28-A12: physical and resource bounds | Cover offset range/overflow/alignment, allocator symmetry, OOM and cleanup paths; measure code size, access cost, allocation count, copied extent and peak backing. No performance claim follows from representation choice alone. |
| D28-A13: shared owner evidence | Many same-owner declarations reuse live identity/address information; inferred owners, recursive instances and forwarded owners remain distinct where required. Every dereference has sufficient owner, target and layout evidence, with no redundant per-pointer owner copy where sharing/elision suffices. |
| D28-A14: qualified owner allocation | Writable input permits owner or const owner; all-readonly input permits only const owner. Const owner allows declared target writes but rejects direct/indirect allocation; null and shared runtime contexts cannot upgrade permission. Preserve admitted placement, policy, capacity, initialization and cleanup. |
| D28-A15: identity and null | is follows the selected owner/locator pair; is not gives its exact boolean complement with identical operand evaluation, without payload equality or final-target dereference; is null uses contextual ownership. Preserve operand order, aliases, readonly views, distinct subobjects, named val storage, null/owner distinctions and key/hash stability through backing growth. |

Completion of the design requires recorded decisions for the remaining interfaces and an accepted value path for each delegated scope. Implementation completion additionally requires current source capability evidence. This integrated review supplies neither source changes nor execution/benchmark results.
