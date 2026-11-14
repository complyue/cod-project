# Regional and lexical storage: reference-model refactoring

Design revision: 3 · Recorded: 2026-11-14 · Status: refactoring direction and dedicated tracking authorized; detailed contracts, naming and implementation plan pending-human.

## Design authority

- **Registry / scope:** [D-28](DesignAuthorityRegistry.md#d-28), r3, all sections. [H-429](../CHANGELOG.md#h-429) withdraws region-backed lexical storage and directs regional references toward relative/local representation and lexical references toward direct/global representation. It requests this dedicated refactoring document and proposes considering the C++ `global_ptr` → `external_ptr` rename. H-430/P-385 selects explicit consuming replacement instead of automatic/in-place language growth; H-431 selects ordinary owned-parameter failure cleanup. H-432 proposes a single resize(r, new_cap) for lossless growth/shrink, with exceptions when it cannot succeed. These scoped directions do not approve every layout, source form, ABI or migration choice below.
- **Candidate value path:** C-91/P-381 derives from C-39/C-65 and C-01/C-06: distinguish relocatable regional graphs from lexical execution storage while keeping target identity, access and lifetime explicit. Charter §§2/3.3/4.3/6.3/7.1/8/12 supports that rationale. P-383 organizes this document; both paths remain pending-human beyond H-429's stated direction.
- **Dependencies:** P-384 links D-01 physical storage, D-07 nominal types, D-10/D-15 TLS/worlds, D-14 owner/access/initialization, D-16 numeric validity, D-17 completion, D-18 callable values/captures, D-21/D-26 native transport and D-22/D-23/D-24 construction/result contracts. These dependencies confer no new authority.
- **Invariants / non-goals:** statically unique target owners; no lifetime extension, access escalation, owner unions, CoD move semantics, implicit graph copy or allocation rollback. Regional storage preserves relative edges and zero per-edge relocation work. Intrinsic region cleanup and assignment/value semantics retain their separate rules. No general heap allocation, lexical non-bits layout, new capture permission or permanent ABI is selected.
- **Acceptance / baseline:** H-429's current project-user messages are the human baseline; exact timestamp/personal identity are unavailable. [K-RESIZE-20261114](../CHANGELOG.md#k-resize-20261114) records the resize/capacity review, preserving the existing charter/dependency boundaries. Acceptance requires the composed cases below, a reconciled source/design inventory and measured native costs. No implementation, build or test evidence is claimed here.

## Problem and current direction

A region's stored graph must survive uniform relocation without rewriting every edge. A lexical scope describes execution lifetime and may hold views into independently owned storage. Sharing owner/lifetime checks does not require sharing the allocation mechanism or stored reference encoding.

The current direction is therefore two storage domains with explicit boundaries. Lexical scopes are not backed by implicit language regions. Region allocation policy is independently selectable by libraries/frameworks; a lexical view's lifetime does not shorten the lifetime of its target. That separation is an intentional modeling capability.

This refactoring separates terminology, semantic facts, physical representation and implementation transport. Some current native paths already use direct addresses for lexical access and relative offsets for region fields. The impact review must preserve correct existing paths and identify actual gaps; a conceptual refactoring is not evidence that all native code needs replacement.

## Terms and representation boundaries

| Concept | Proposed precise meaning | Independent obligations |
| --- | --- | --- |
| Regional relative/local reference | An edge stored in a region's object representation; the current encoding is relative to the edge slot. | Target shares the regional owner; null/non-null, initialization and access remain checked. |
| Lexical direct/global view | An execution-time reference binding whose representation is independent of a regional edge slot's address. It may denote lexical storage or a live regional target. | Retain one statically provable target owner and valid access/lifetime. Global does not mean global lifetime or ownership. |
| External regional handle | A host/runtime handle used outside a target region to access an object within it. | Carries the information needed to resolve that region's target; it is not a persistable regional edge or a general lexical pointer. |
| Target owner | The lifetime authority for the referenced payload. | Separate from the storage/lifetime of the reference binding itself and from intrinsic cleanup responsibility. |

The first interpretation to review is that **the storage containing a reference selects its stored representation; the target selects provenance and lifetime**. A regional object's edge remains relative when accessed through a lexical variable. A lexical borrow of that object does not become a stored relative edge merely because its target is regional. “All lexical pointers are direct/global” does not turn inline val bits into pointer objects or force every reference to have a new runtime owner tag.

The label direct/global currently leaves physical width and metadata transport open. A raw address, a native address plus separately retained owner/type evidence, and an external regional handle are not automatically interchangeable ABIs. Static evidence may erase some runtime metadata without erasing the language obligations.

### C++ naming proposal

H-429 proposes considering `external_ptr<T, RT>` as the replacement name for today's `global_ptr<T, RT>`. **Recommendation: adopt that name during the coordinated refactoring**, because it describes a handle accessing a regional target from outside the region and avoids collision with the lexical direct/global concept.

The current [implementation](../include/shilos/region.hh) stores a C++ reference to `memory_region<RT>` plus a target offset; `get()` derives an address from that referenced header. It is neither an arbitrary raw address nor a stable relocation descriptor. Renaming it does not make existing handles survive a moved region header. The `regional_ptr` name can remain for the existing relative edge; this draft introduces no second rename or public CoD pointer-type keyword.

The rename inventory includes declarations in [prelude.hh](../include/shilos/prelude.hh), conversions/accessors in [region.hh](../include/shilos/region.hh), [string helpers](../include/shilos/str.hh), colocated vector/list/dictionary YAML helpers, shilos tests, experiments, documentation and repository instructions. External downstream API/ABI consumers need an inventory too. Current code keeps its existing name until the refactoring is qualified and implemented; the proposed rename is not a completed source change or a permanent compatibility-alias commitment.

## Operations across the boundary

Illustrative CoD, with root already referring to a live object in r:

```text
data Node { ptr next:Node; }

ptr cursor:Node in r = root.next;
# root.next is a relative edge in r.
# cursor is an independent lexical binding with a direct/global view into r.

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

No representation choice weakens H-417 unique-owner inference. A lexical direct view still cannot choose between different target owners at runtime without an independently admitted value operation; a regional edge still cannot point outside its owner. H-428 allows allocation effects on initialization failure, with no automatic rewind. H-426/H-427 RHS normalization remains in D-22; selecting a carrier neither approves that pending copy policy nor introduces move semantics.

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

The proposed exception mapping reuses D-17 categories: InvalidCapacity for invalid/unrepresentable requests or size arithmetic; RegionCapacityExceeded when a representable requested budget cannot contain the retained extent; OutOfMemory when the provider cannot supply required backing. Capacity checks inside resize occur after consuming entry. H-431 therefore applies even to a rejected shrink: input ownership is not restored and callee cleanup releases what it still owns. Successful resize is lossless; failure does not promise preservation of the input region. Argument computation/conversion that fails before entry retains the separate E4 rule. Backend/file failures still require a selected resource protocol.

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

## Material decisions still required

| Decision | Alternatives / consequence | Current boundary |
| --- | --- | --- |
| Lexical ADTs with reference fields | Permit a distinct direct-reference layout, provide representation-dependent layouts for one nominal ADT, or retain regional-only placement for such ADTs. Layout-polymorphic access would affect field offsets, generic calls and actual-type evidence. | H-429 withdraws scope-region backing; it does not by itself choose a lexical non-bits layout or graph-copy semantics. |
| Direct/global carrier | Define when a plain address is sufficient and where owner/base/type evidence must be retained. | No universal fat pointer, raw-pointer ABI or new intrinsic type is selected. |
| Nominal type and placement | A logical field shape may need separate representation facts. Decide whether placement is a type constraint, a view/layout property or an explicit distinct type. | Physical similarity is not nominal identity; existing bits and regional graph contracts remain. |
| Resize capacity/backing protocol | Review exact payload budget, retained-extent shrink bounds, equal-capacity handling and existing-error mapping above; specify backend and shared-mapping behavior. | H-430–H-432 retains consuming replacement, lossless success and ordinary failure cleanup. Old views never follow the replacement; no automatic growth or implicit compaction. |
| Cross-owner references in a future lexical aggregate | Direct representation can encode such addresses, but field-wise owner/lifetime relationships must be specified before admitting them. | Do not apply an owner union or lift regional same-owner edges by implication. |
| Naming and migration | `external_ptr` is the recommended C++ handle name; lexical direct/global terminology must not promise global lifetime or persistence. | Source/API migration remains planned; no blanket rename changes semantics. |

The first substantive choice is lexical reference-bearing ADTs: pointer representation alone does not settle where such an object may be created, how its layout is selected, or which owners its fields can borrow. Until that scope is qualified, the current regional-only non-bits placement remains the implementation boundary.

## Impact and work packages

| Area | Required review/change | Preserve / acceptance |
| --- | --- | --- |
| D-01 and shilos APIs | Reconcile relative edges, external handles, temporary native addresses and contradictory global-pointer storage examples; perform the qualified external_ptr rename coherently. | Region-only C++ objects, allocator symmetry, self-relative encoding and no per-object copy/move. |
| D-07/D-14/D-22 | Separate nominal type, representation eligibility, storage domain, actual owner, construction access and ref/ptr slot behavior. | Inline bits, readonly/const independence, definite initialization, unique owners and no automatic graph copying. |
| Checked compiler | Audit [CheckedBinding/ViewEvidence and owner flow](../cod/lang_checked.hh) for accidental coupling of carrier, slot storage and target owner. | Retain current initialized-target, alias, null, owner and cleanup proofs; unsupported representations diagnose explicitly. |
| Native lowering | Audit [lexical slots, selected places and calls](../cod/lang_llvm.hh) plus regional access paths for resolve/store boundaries and retained metadata. | Once-only address selection, correct relative encoding and native behavior across recursion/defer/failure. Existing LLVM alloca usage alone proves no complete model. |
| D-18/D-21/D-26 | Distinguish captured regional slots, borrowed targets, lexical binding cells and intrinsic complete callable environments. | No automatic lexical-cell capture, self-address-dependent environment or expired borrowed view; preserve concrete-type dispatch. |
| D-10/D-15 | Review TLS reference transport independently of TLS payload categories and symbol-backed owners. | Bits payloads/callable values retain their contracts; symbolic references retain explicit owner relations and restoration. No inline non-bits TLS arena. |
| D-23/D-24 | Carry result mode, target owner/access and any layout evidence through calls and inferred signatures. | Value snapshots, retained targets, independent recursive anchors and existing conversion limits. |
| D-16 / serialization / DBMR | Keep numeric bits validity independent of pointer representation; distinguish persisted graphs from native lexical views in YAML/snapshots. | Fixed valid fraction payloads, optional colocated YAML, no direct process address persisted as a regional edge or new file-layout promise. |

Proceed in reviewable packages: settle terminology and the lexical-ADT boundary; qualify the corresponding storage/view contract; refactor checked representation facts and native boundary operations; coordinate C++ API migration and downstream users; then update composed fixtures and current capability documentation. H-386's coordinated source migration remains the direction where applicable; internal work packages do not establish a permanent dual language or compatibility API.

This is planning authorization, not an implementation/build/test request. No bootstrap/configuration work is needed for this document. Implementation must follow repository source/build/test restrictions and identify any separately required toolchain authorization.

## Acceptance and completion criteria

1. A lexical view into a regional graph preserves target identity, owner and access while local rebinding leaves the original regional slot unchanged; an alias of that slot observes its actual mutations.
2. Loads/stores round-trip between relative edges and direct/external views, including null, readonly targets, self/cycle encoding limits and same-owner rejection. Raw offset copying is never used as slot-independent rebinding.
3. Same-owner runtime target selection succeeds; ambiguous owner joins, expired scopes, invalid source cleanup, cross-owner stored edges and readonly escalation reject.
4. Calls, returns, generic/abstract dispatch, closure captures, TLS shadow restoration and deferred argument preparation preserve the selected target/slot and required concrete/layout evidence.
5. Failure after a valid publication into an existing region leaves that target live; failed whole-region acquisition releases exactly once. No allocation checkpoint, CoD move or implicit graph copy appears.
6. Resize grows or trims an unused tail without changing retained payload/offsets/sharing/cycles; test E-1, E, C and C+1 capacity boundaries, alignment/header accounting, overflow and allocation failure. Equal-capacity requests retain consuming/result-owner semantics. Reject lossy truncation, implicit compaction and old-view use; fresh output views succeed. Cover pre-entry failure, post-entry rejection/failure and abandoned owning results with exact release and no implicit ownership restoration or automatic/in-place growth.
7. Any selected lexical non-bits support has explicit layout, construction, lifetime, cross-owner-field and native-call acceptance; no support claim follows merely from renaming a C++ handle.
8. Qualified source/API migration updates declarations, helpers, YAML, fixtures, instructions and current docs together. Old spellings remain only where they describe still-current external interfaces or immutable changelog evidence.
9. Measure reference representation, emitted native code, allocation and access costs for representative lexical/region paths. A new name or more explicit type model is not itself a performance result.

The document is complete as a refactoring contract only when the material decisions, accepted value path and selected acceptance scope are recorded. Implementation completion additionally requires current source capability evidence; this draft supplies neither blanket qualification nor test results.
