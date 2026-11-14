# Regional and lexical storage: reference-model refactoring

Design revision: 7 · Recorded: 2026-11-14 · Status: refactoring direction and dedicated tracking authorized; detailed contracts, naming and implementation plan pending-human.

## Design authority

- **Registry / scope:** [D-28](DesignAuthorityRegistry.md#d-28), r7, all sections. [H-429](../CHANGELOG.md#h-429) separates regional and lexical storage, requests this dedicated refactoring document and proposes considering the C++ `global_ptr` → `external_ptr` rename. H-430/P-385 selects explicit consuming replacement instead of automatic/in-place language growth; H-431 selects ordinary owned-parameter failure cleanup. H-432 proposes a single resize(r, new_cap) for lossless growth/shrink, with exceptions when it cannot succeed. H-433 requests automatic addressing adaptation across storage domains and sets lexical pointer-bearing ADTs as the design target, with bits-only lexical allocation as the conservative fallback. H-434/H-435 selects dynamically allocated lexical capacity with owner-base-relative lexical pointers and replaceable heap-fragment backing. H-436 distinguishes a lexical slot locator from its contents: same-scope ref/ptr payloads use that scope's offset semantics, while other-owner references retain their own interpretation. Register/stack optimizations remain permitted. The independently selected requirements are registered as D-28-SCOPE/C-92/P-386; physical anchor/allocator/native-call protocols remain open. These scoped directions do not approve every layout, source form, ABI, allocation capability or migration choice below.
- **Candidate value path:** C-91/P-381 derives from C-39/C-65 and C-01/C-06: distinguish relocatable regional graphs from lexical execution storage while keeping target identity, access and lifetime explicit. Charter §§2/3.3/4.3/6.3/7.1/8/12 supports that rationale. P-383 organizes this document; both paths remain pending-human beyond H-429's stated direction.
- **Dependencies:** P-384 links D-01 physical storage, D-07 nominal types, D-10/D-15 TLS/worlds, D-14 owner/access/initialization, D-16 numeric validity, D-17 completion, D-18 callable values/captures, D-21/D-26 native transport and D-22/D-23/D-24 construction/result contracts. These dependencies confer no new authority.
- **Invariants / non-goals:** statically unique target owners; no lifetime extension, access escalation, owner unions, CoD move semantics, implicit graph copy or allocation rollback. Regional storage preserves relative edges and zero per-edge relocation work. Intrinsic region cleanup and assignment/value semantics retain their separate rules. No general heap allocation, lexical non-bits layout, new capture permission or permanent ABI is selected.
- **Acceptance / baseline:** H-429's current project-user messages are the human baseline; exact timestamp/personal identity are unavailable. [K-LEXICAL-SLOT-PAYLOAD-20261114](../CHANGELOG.md#k-lexical-slot-payload-20261114) records the selected storage/content distinction and optimization boundary and preserves K-RESIZE-20261114 capacity/cleanup boundaries. Acceptance requires the composed cases below, a reconciled source/design inventory and measured native costs. No implementation, build or test evidence is claimed here.

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

The owner-base-offset model leaves physical width and metadata transport open. Resolved native addresses remain temporary implementation values. A raw address, a native address plus separately retained owner/type evidence, and an external regional handle are not automatically interchangeable ABIs. Static evidence may erase some runtime metadata without erasing the language obligations.

### C++ naming proposal

H-429 proposes considering `external_ptr<T, RT>` as the replacement name for today's `global_ptr<T, RT>`. **Recommendation: adopt that name during the coordinated refactoring**, because it describes a handle accessing a regional target from outside the region and avoids conflating an external regional handle with the lexical reference model.

The current [implementation](../include/shilos/region.hh) stores a C++ reference to `memory_region<RT>` plus a target offset; `get()` derives an address from that referenced header. It is neither an arbitrary raw address nor a stable relocation descriptor. Renaming it does not make existing handles survive a moved region header. The `regional_ptr` name can remain for the existing relative edge; this draft introduces no second rename or public CoD pointer-type keyword.

The rename inventory includes declarations in [prelude.hh](../include/shilos/prelude.hh), conversions/accessors in [region.hh](../include/shilos/region.hh), [string helpers](../include/shilos/str.hh), colocated vector/list/dictionary YAML helpers, shilos tests, experiments, documentation and repository instructions. External downstream API/ABI consumers need an inventory too. Current code keeps its existing name until the refactoring is qualified and implemented; the proposed rename is not a completed source change or a permanent compatibility-alias commitment.

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

## Lexical pointer-bearing ADTs: feasibility and transfer review

H-433 makes lexical ADTs containing pointers the design target; lexical bits-only allocation is the conservative fallback. The requested evaluation is one nominal ADT with storage-specific references and automatic addressing adaptation. H-435/H-436 selects lexical storage locators and same-local-owner offset contents; other-owner reference contents retain separate interpretation, while regional fields retain self-relative edges. One nominal ADT remains the evaluation target, without selecting a particular layout carrier or allocator. Current qualified placement remains the fallback until the affected contract is selected.

### One type, storage-specific layouts

The recommended candidate retains one nominal data type and its semantic methods. Placement supplies the representation evidence used for field offsets and reference encoding. A lexical handle into a regional instance still accesses that instance's regional layout. Taking or passing a view does not relocate, reconstruct or change the layout of its target.

Schematic CoD illustrates a method that could serve either storage domain under the candidate:

```text
data Node { value:u64; ptr next:Node; }
proc next_of(n:Node in O) -> ptr Node in O where {owner O} {
    return n.next;
}
```

The actual owner of n identifies the target storage domain; the storage of n's parameter binding does not. For a regional instance, the field operation resolves a relative edge; for a lexical instance, it resolves the offset against the actual lexical owner's current base. The returned nullable view retains O, including when null. This example preserves the existing same-owner field rule. Lexical objects containing references to a different owner need a separate decision; the pointer encoding alone does not grant that permission.

Checking/lowering must keep the following facts distinct:

| Fact | Why it is needed |
| --- | --- |
| Nominal target type and concrete evidence | Preserve method selection, generic identity and interpreted field shape. Layout is not a new business type or an owner identity. |
| Target object's storage domain/layout | Select correct field locations and the encoding of fields inside that object. |
| Selected reference slot's encoding | Reading/rebinding a lexical view follows its own carrier; an alias of a regional field retains that field's slot-relative encoding. Aliasing preserves the selected slot. |
| Actual owner, access, nullability and initialization | Validate the semantic operation independently of the address representation. |

Known storage domains can be specialized without a runtime representation branch. Shared native code receiving either domain must retain enough layout evidence or select an appropriate prepared implementation. This need not mean a tag on every pointer, one implementation per owner instance, or layout-based user overloads. The permanent carrier/dispatch strategy remains open under D-21/D-26 and needs measured code-size/access costs. A raw address alone cannot determine whether the target's reference fields use owner-base or slot-relative offsets.

### What automatic transfer handling means

H-206/H-208/H-351 already separates builtin bits transfer from library-defined graph transformation. H-433's addressing adaptation can preserve that boundary:

| Operation | Automatic compiler responsibility | Semantic boundary |
| --- | --- | --- |
| Bits `<-`, including a payload reached through a ref/ptr field | Resolve each operand using its own layout, apply the established typed conversion and write the selected destination payload. | No pointer-field byte copy, source alias retention or topology change. D-16 payload validity remains unchanged. |
| Reference `=` and reference-slot aliasing | Resolve the source referent; encode an admitted target using the destination slot's representation, preserving null. | Prove fixed owner/access. Different owners reject; addressing adaptation cannot relabel a source target. |
| Selected graph `(<-)` method | Make ordinary field reads, writes, rebinding and calls representation-aware for each source/destination domain. | The selected library algorithm controls construction, topology, overlap and failure behavior. No default graph copier is introduced. |
| Whole-region ownership transfer | Preserve the region image and transfer cleanup responsibility. | No conversion to lexical objects. |
| Consuming resize | Preserve regional offsets during whole-image replacement. | Its separate owner/cleanup contract applies; it is not object-graph conversion. |

All four graph-transfer domain pairs need acceptance: lexical→lexical, lexical→regional, regional→lexical and regional→regional. Existing region-specific methods remain applicable only where their region/root requirements hold; a generic owner does not fabricate a region argument. A shared traversal can use appropriate destination construction operations while the compiler hides pointer encoding. User code should not require direct/relative casts or pointer arithmetic.

For a topology-preserving transfer, if source a has two edges to b and b has an edge back to a, the destination must contain one corresponding b and the back-edge to the corresponding a. Merely decoding source addresses and re-encoding them in the destination would retain the wrong owner. A library can maintain a source-to-destination mapping and stage destination construction; it must also obey current ref initialization/publication rules. Algorithms intentionally changing topology retain their own contracts. No implicit object copy/move or universal scratch map follows.

### The substantive allocation boundary

Fixed-size lexical instances with already established same-scope targets present no fundamental addressing obstacle. A transfer that constructs an arbitrary number of nodes in a caller-owned lexical scope needs more than lifetime provenance: it needs usable construction capability for that actual scope. Allocating those nodes on the callee's ordinary stack would give them the wrong lifetime. This is the main unresolved interface beyond carrying layout evidence.

A candidate is caller-supplied scoped construction operations, with stable semantic targets and cleanup at the destination scope's exit. Physical addresses may change; H-435/H-436 requires surviving storage locators and reference payloads to retain their respective owner/representation facts. Region destinations continue to expose their selected root policy; lexical destinations need their own admissible interface without pretending to be a region. Traversal can be shared behind those operations. For source usability, the candidate should retain make T in o wherever o supplies the required construction capability. A dedicated scope binder, an explicit builder or another checked capability path could supply it; an ordinary owner binder remains provenance only. Exact capability syntax, backing/materialization strategy and permitted cleanup machinery remain pending-human; register/stack optimization permission is already selected under H-436. No implicit region backing, independent heap lifetime, owner extension or allocator capability inferred solely from owner identity is selected.

Allocation/transfer failure retains H-428: prior valid writes and allocations in an existing owner need not roll back. A failed fresh whole-region acquisition still has its separate exact cleanup rule. Lexical allocation must have a selected normal/exceptional scope cleanup contract before implementation. Adding pointer-bearing lexical storage does not relax callable capture or TLS payload categories.

### Feasibility, UX and current implementation evidence

The model is technically feasible: typed field operations can select a representation while preserving the same nominal data and methods. Its UX advantage is one source model and the same field/assignment syntax in both domains. Two distinct nominal types would reduce some representation uncertainty but force duplicated interfaces or explicit conversions into business code. Keeping bits-only lexical storage avoids the new allocation/layout work and remains the fallback, not the preferred product outcome.

The necessary compiler work is substantial and is not currently implemented. [CheckedStorage/CheckedBinding/ViewEvidence](../cod/lang_checked.hh) separates lexical scope, target ownership and selected aliases for the current subset, but supplies no general dual-layout ADT contract. [LLVMEmission](../cod/lang_llvm.hh) distinguishes value, target and slot projections; its current rebind path stores a native address directly and must not be reused unchanged for relative slots. [regional_ptr and global_ptr](../include/shilos/region.hh) supply concrete relative-load/store and external-handle examples; their raw C++ interfaces are not language lifetime proofs.

Acceptance must cover field/array access, nested direct payloads, nullable and cyclic references, aliases of fields, calls/results, generic shared code, permitted captures and all four transfer pairs. Relative offset range and the existing zero-null/self-reference encoding limitation require an explicit layout/encoding solution; automatic adaptation cannot treat null as an ordinary address delta. Whole-object byte transfer is not a substitute for these operations. Measure native code size, field access and transfer allocation costs; this review supplies reasoning and source inspection, not benchmarks or execution evidence.

The next contract decisions are the concrete lexical relocation/native-call protocol and destination-scope construction interface for graph algorithms. The same-type/dual-layout model is the recommended candidate for evaluation; physical ABI, arbitrary lexical graph allocation and any broader field-owner permission remain unselected.

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

The fragment organization under review uses one active contiguous payload for the portion of an actual full-scope instance that needs growable backing. The owner context supplies its current base and allocation state. Same-scope payload offsets and variable-slot offsets can use that context; external references retain their independently required representation. No permanent fat-pointer ABI, mandatory all-variable heap frame or one global context per source scope is selected. Recursion has distinct owners. Plain grouping blocks do not create owners under D-17. Multiple concurrent fragments would require additional fragment identity or a different mapping contract.

The owner anchor/resolution metadata cannot depend on an untracked raw address into the fragment being replaced. Its exact storage/cleanup arrangement is open. Relocation must preserve object offsets, alignment, initialized bits and all admitted embedded representations. Callable environments, selected places, cleanup records and native interop cannot be assumed relocatable merely because ordinary data pointer fields use offsets. Ordinary CoD objects acquire no user-visible copy/move operation.

### Selected addresses across allocation

Changing pointer fields is insufficient if generated code retains stale native addresses. Consider this schematic body, where the RHS has independently admitted capability to allocate in p's owner:

```text
p <- value_that_allocates_in_the_same_scope();
```

The lhs is selected before the RHS. For a target in growable lexical backing, lowering retains its target-owner context and location information, evaluates the RHS once, then resolves that same target against the current backing before storing. An external target keeps its own target-owner representation; growth of the pointer variable's slot owner alone does not relocate that external target. It must not rerun an indexed/field selection or reload a rebound pointer variable and accidentally choose a different target. Reference assignment analogously retains the selected destination slot's locator and encodes its new contents under the fixed target-owner contract. Selected aliases, calls/returns, recursion, deferred work and all allowed captures need the same treatment. A generic helper cannot keep a raw pointer alive across an operation that may relocate its owner.

The compiler can retain a resolved address across operations proven unable to relocate that owner. Otherwise it must resolve again after possible growth. Foreign/native code that retains a raw address needs a pinned/no-growth access interval or a separately admitted handle interface; silently refreshing arbitrary native pointers is impossible. Exact exposure and pinning policy is a material open decision, not a new FFI feature.

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
| Lexical ADTs with reference fields | Permit storage-dependent layouts for one nominal ADT, require distinct nominal types, or retain bits-only lexical placement as the fallback. Layout-polymorphic access would affect field offsets, generic calls and actual-type evidence. | H-429 withdraws scope-region backing; it does not by itself choose a lexical non-bits layout or graph-copy semantics. |
| Lexical carrier and growth | Implement H-435 owner-base offsets with a selected anchor transport, relocation boundary and native-address exposure protocol. | Dynamic capacity, distinct slot/payload interpretation and register/stack optimization are selected independently; inter-owner ABI, automatic regional growth and new lifetimes do not follow. |
| Nominal type and placement | A logical field shape may need separate representation facts. Decide whether placement is a type constraint, a view/layout property or an explicit distinct type. | Physical similarity is not nominal identity; existing bits and regional graph contracts remain. |
| Resize capacity/backing protocol | Review exact payload budget, retained-extent shrink bounds, equal-capacity handling and existing-error mapping above; specify backend and shared-mapping behavior. | H-430–H-432 retains consuming replacement, lossless success and ordinary failure cleanup. Old views never follow the replacement; no automatic growth or implicit compaction. |
| Cross-owner references in a future lexical aggregate | Direct representation can encode such addresses, but field-wise owner/lifetime relationships must be specified before admitting them. | Do not apply an owner union or lift regional same-owner edges by implication. |
| Naming and migration | `external_ptr` is the recommended C++ handle name; lexical reference terminology must not promise global lifetime or persistence. | Source/API migration remains planned; no blanket rename changes semantics. |

The lexical pointer-bearing ADT target and conservative fallback are recorded under H-433. The feasibility review above isolates layout evidence, H-435 owner-base relocation and destination-scope construction capability; neither automatically changes which owners fields may borrow. Until the corresponding scope is qualified, regional-only non-bits placement remains the implementation boundary.

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
7. Any selected lexical non-bits support has explicit layout, construction, lifetime, cross-owner-field and native-call acceptance. Cover all four lexical/regional transfer pairs, aliased pointer slots, sharing/cycles and source-to-destination identity mapping; reject unavailable construction capability and callee-local storage escaping as caller-owned nodes. No support claim follows merely from renaming a C++ handle.
8. Qualified source/API migration updates declarations, helpers, YAML, fixtures, instructions and current docs together. Old spellings remain only where they describe still-current external interfaces or immutable changelog evidence.
9. Measure reference representation, emitted native code, allocation and access costs for representative lexical/region paths. A new name or more explicit type model is not itself a performance result.

The document is complete as a refactoring contract only when the material decisions, accepted value path and selected acceptance scope are recorded. Implementation completion additionally requires current source capability evidence; this draft supplies neither blanket qualification nor test results.
