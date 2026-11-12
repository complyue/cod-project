# CoD regional ownership: syntax, semantics and use

Design revision: 73 · Recorded: 2026-11-12 · Status: design draft with scoped confirmed requirements.

## Design authority

- **Current regional captures:** [D-18](RegionalClosures.md)/[K-459](DesignAuthorityRegistry.md#k-459) is the bounded contract for structural region requirements, aliases, captures and owner projection under H-327–H-335. This revision updates only that navigation and removes superseded research text under H-336; C-65/C-73/C-75 and P-294–P-298 retain their confirmed scope. Owner/access/relocation, dynamic effects and D-16 remain constraints. Acceptance and unresolved limits stay explicit in D-18; unrelated pending contracts are not qualified.

- **Loop scoping rules:** [H-365](DesignAuthorityRegistry.md#h-365)/[K-522](DesignAuthorityRegistry.md#k-522), C-63/P-286/P-321/P-325, confirms the dedicated [loop contract](ExceptionControl.md#loop-scoping-rules): body/zero-iteration else declarations require a nested explicit scope; effects and fresh effect resources end at each iteration/else exit; direct deferred work registers to the nearest enclosing full scope. Delayed blocks cannot capture loop-control bindings, and deferred borrows cannot outlive iteration resources. This serves explicit resource responsibility, inspectable effects and intentional dynamic cleanup accumulation under charter §§2–3/§6/§8/§12. H-365 supersedes H-319's direct-loop-defer prohibition and resolves H-364; no general new scope abstraction, lifetime extension or loop form is selected. Acceptance covers all exits, hidden-owner retention, snapshots, global LIFO, exact release and accumulation costs. Implementation evidence remains separately bounded.

- **Nullable calls and uniform annotations:** [H-317/H-318](DesignAuthorityRegistry.md#h-317)/[K-443](DesignAuthorityRegistry.md#k-443) accepts explicit nullable ptr call boundaries, retains ptr edges for cycles and uses single-colon annotations with arrow results. C-72/P-284–P-285 preserves C-65 owners/access and C-63 checked failures; charter §§1–3 and D-16 compatibility were reviewed. Acceptance requires nullable forwarding without payload access, checked dereference, owner/result rejection and migrated grammar. H-319 establishes the revised scope boundary; native colon/nullable acceptance is recorded by K-444, while scope/defer implementation remains separate.


- **Declaration cleanup:** [H-316](DesignAuthorityRegistry.md#h-316)/[K-442](DesignAuthorityRegistry.md#k-442) applies C-72/P-283 to the current ref/ptr/val surface. Ordinary views name their target type; allocation and element selection preserve owner/access information directly. Let/var declarations and angle-bracket reference types are rejected. [Native evidence](../tests/cod/language/declaration-cleanup-validation.json) covers migrated examples, copied bits versus aliases, checked NPE, lifetime/relocation and multi-module composition. H-317 covers nullable call boundaries; automatic reference-target allocation and staged required-reference cycles remain outside that acceptance. Existing pending qualifications remain pending.

- **Native nullable-storage acceptance:** [K-441](DesignAuthorityRegistry.md#k-441) continues H-309–H-315 for the bounded field/val/checked-NPE implementation. [Evidence](../tests/cod/language/nullable-storage-validation.json) records the actual compiler, language cases, LLVM verification, relocated scalar/object reference fields and exceptional region cleanup. Nullability does not weaken definite initialization, fixed owners or readonly access. Automatic reference-target allocation and general cyclic ref publication remain outside this acceptance; H-316/K-442 covers declaration cleanup.

- **Checked NPE and explicit bits storage:** [H-313–H-315](DesignAuthorityRegistry.md#h-313)/[K-440](DesignAuthorityRegistry.md#k-440) accepts checked NullPointerException, mandatory module-top-level handling and val locals/parameters; null is a reserved literal and isnull inspects a pointer without dereferencing it. C-72/P-280–P-282 preserves storage/access/lifetime rules, with a null-initialized local acquiring its fixed owner at the first non-null assignment. D-17 owns exception/signature obligations and D-14 owns ordinary storage/owner checks. No checked OOM, payload RTTI, implicit regional-object copy or per-object owner is introduced. Acceptance requires native execution plus rejection at uncovered calls, unsafe reads and inconsistent owner paths; evidence is recorded separately from this confirmed contract.

- **Field storage:** [H-311](DesignAuthorityRegistry.md#h-311)/[K-438](DesignAuthorityRegistry.md#k-438) distinguishes unmodified/val fields storing bits directly from ref/ptr fields storing references, even to scalar targets. C-72/P-277 clarifies P-276: the reference rule applies only to reference-marked fields. Direct bits payloads need no separate target allocation; constructor allocation for reference fields remains unselected. Existing owners, access, relocation and fixed numeric payload constraints remain. Acceptance must distinguish all three field forms and reference rebinding from payload transfer; K-441 records bounded native implementation; automatic reference-target allocation remains unselected.

- **Ref/ptr declaration direction:** [H-309](DesignAuthorityRegistry.md#h-309)/[K-436](DesignAuthorityRegistry.md#k-436) confirms ptr-marked stored reference fields and directs ordinary variables toward ref/ptr declarations without let/var; H-311 permits unmodified/val bits fields. C-72/P-274–P-275 separates compile-time initialization checks from nullable pointer dereference while retaining owner/access constraints. [D-14's review](RegionalOwnership.md#ref-and-ptr-declaration-direction) records H-310/H-311's field storage distinction and keeps parameter, conversion and construction details pending. Acceptance must distinguish premature ref reads, nullable ptr failures, initialized payloads, same-owner edges and relocation. K-441 records the bounded source/runtime implementation without changing regional encoding or adding RTTI or fractions.

- **Region declarations and transfer results:** [H-307/H-308](DesignAuthorityRegistry.md#h-307)/[K-435](DesignAuthorityRegistry.md#k-435) covers region-keyword locals/parameters/results/signatures and void-or-readonly-view transfer overloads, including `([]<-)`. C-70/P-270–P-271 preserves visible resource responsibility; C-71/P-272–P-273 permits logical container writes without inventing addressable elements. Bits-like chaining guidance applies only to library overloads intended or recommended for chaining. Acceptance covers source rejection/migration, owner pairing and cleanup, packed and addressable writers, result/access checks and LLVM output. No runtime tags, fraction execution, regional encoding change or wider pending design qualification is implied.

- **Pointer invariants, fixed bindings and swaps:** [H-305](DesignAuthorityRegistry.md#h-305)/[H-306](DesignAuthorityRegistry.md#h-306)/[K-434](DesignAuthorityRegistry.md#k-434) covers initialized same-owner loop rebinding, `const ptr` groups and `<=>` / `<->`. C-65/P-266 separates runtime targets from invariant owner/type/access; P-267 distinguishes non-rebindable bindings from readonly referents. C-69/P-268–P-269 records explicit reference/bits exchange with checked destinations and void results. Covered sections are ordinary pointers, initialization/control and swap operations; wider pending design paths remain pending. Acceptance checks repeated/nested A/B exchange, const-binding versus referent access, ordered operands, function/procedure overloads, invalid initialization/access/owner rejection and LLVM output. No per-iteration alias-history solver, runtime ownership tag, new regional encoding, fraction implementation or RTTI is introduced.

- **Checked builtin delegation and zero-iteration fallback:** [H-304](DesignAuthorityRegistry.md#h-304)/[K-433](DesignAuthorityRegistry.md#k-433) confirms `intrinsic (operation)(...)` and optional range `for ... else`. C-67/P-262 makes same-signature delegation explicit while preserving all builtin safety and result rules; C-68/P-264 gives empty ranges an explicit initialization path. P-263/P-265 retain C-64/C-65 access, lifetime and initialization constraints. Covered sections are explicit builtin invocation, range control and their prototype acceptance; full-document candidate value paths remain pending. Acceptance includes overload delegation, cross-module body reuse, empty/nonempty ranges, once-only bounds, exceptional exits and invalid access/initialization rejection. No recursion support, loop-carried alias solver, fraction execution, RTTI, storage encoding or new ownership policy is selected.

- **Assignment-result safeguard:** [H-303](DesignAuthorityRegistry.md#h-303)/[K-432](DesignAuthorityRegistry.md#k-432) confirms that an ordinary `(=)` overload must finish with its destination parameter. C-64/P-261 makes the explicit destination return a semantic safeguard, rather than an alias-equivalence proof limitation. Acceptance rejects returning another alias even when it denotes the same storage; owner/access rules and the compiler representation are unchanged. This scoped confirmation does not qualify wider pending designs.

- **Const-qualified methods and reference assignment:** [H-302](DesignAuthorityRegistry.md#h-302)/[K-431](DesignAuthorityRegistry.md#k-431) confirms parameter qualifiers as method distinctions, exact destination qualification, safe input-view weakening, uniquely best dispatch and checked method bodies. C-65/P-260 constrains C-64/D-10/D-14 so reference assignment preserves owner/access provenance without hidden value conversions. Builtin `=` preserves destination access; selected overloads declare their result access. H-299 still forbids directly rebinding a writable ptr from a read-only view, but does not preclude selecting a method whose body finds an independently writable target. Acceptance covers qualifier selection, writable chains, actual caller rebinding and unsafe/ambiguous/result rejection. D-14r58 records the semantic safeguard, H-304 range-else contract and separate implementation limits. Full-document pending qualification remains separate.

- **Ptr/local-storage implementation:** [H-298](DesignAuthorityRegistry.md#h-298)/[K-429](DesignAuthorityRegistry.md#k-429) records the requested integration and bounded implementation of H-270–H-273/H-292–H-297. C-65/P-244–P-246/P-255/P-257/P-258 and C-64/P-256 connect explicit storage identity, lifetime and access to checked business operations. Acceptance covers local initialization, aliasing, external-owner calls, branch/exception joins, readonly destination views and declared result owners; [native evidence](../tests/cod/language/ptr-storage-validation.json) records the actual compiler. Regional layout, TLS contracts, fractions, transport and the wider qualifier/region-declaration grammar are unchanged. Full-document qualification remains pending; the proof limits are listed in D-14's bounded implementation section.

- **Local bits and assignment owners:** [H-292/H-293](DesignAuthorityRegistry.md#h-292)/[K-427](DesignAuthorityRegistry.md#k-427) confirms implicit enclosing-scope bits storage via `make local T { expression }`, preservation of read-only access through aliases, and mandatory explicit external owners on assignment-overload destinations. C-65/P-244/P-255 and C-64/P-256 retains lifetime and destination-view dependencies; acceptance must distinguish aliasing, independent bits and caller-visible assignment. [H-296/H-297](DesignAuthorityRegistry.md#h-296)/[K-428](DesignAuthorityRegistry.md#k-428) distinguishes individual value initialization from a comma-introduced declaration-group initializer and permits later initialization while rejecting reads before definite initialization. typeof remains compile-time only. K-429 records bounded native acceptance and the remaining analysis/source limits.

- **Symbolic implementation scope:** [H-287–H-291](DesignAuthorityRegistry.md#h-287)/[K-426](DesignAuthorityRegistry.md#k-426) records the requested implementation and callable/result decisions: every effect yields void; procedure rebinding uses `=`, while inline `effect proc` introduces or signature-checks its symbol and captures at definition. C-36/C-54/C-65/C-66 and P-248–P-254 preserve typed dynamic context, actual-region lifetime and explicit capture. [Native evidence](../tests/cod/language/symbolic-context-validation.json) covers typed TLS payloads, staged groups, touch/replay checks and overload-backed object transfer. This bounded scope does not qualify whole-document drafts or implement ordinary ptr migration, transport, runtime closures or fractions.

- **Symbol declaration spelling:** [H-276](DesignAuthorityRegistry.md#h-276)/[K-421](DesignAuthorityRegistry.md#k-421) records the preferred `region symbol` order and the accepted restriction to region symbols within one declaration. This refines the symbol draft without changing typed identity, effect installation or capture/shadowing; the wider result/qualifier contracts retain their status.

- **Region declaration direction:** [H-274](DesignAuthorityRegistry.md#h-274) requires root-constrained owner access with an actual borrowed region reference. [H-275](DesignAuthorityRegistry.md#h-275) confirms `where {region r:RootType}` and extends the direction to all user-facing region references: `region` is a declaration keyword rather than a type constructor. Generic `owner` remains the unconstrained region-or-scope form. [K-420](DesignAuthorityRegistry.md#k-420) coordinates C-65/C-55/P-247 and the wider declaration inventory below. H-307 completes the declaration surface for locals, parameters, results and symbolic signatures; K-435 records its bounded implementation.

- **Pointer-binding baseline:** [H-271](DesignAuthorityRegistry.md#h-271) makes ordinary user variables `ptr` bindings with an initializer-fixed owner, same-owner rebinding, implicit dereference on reads and no pointer arithmetic. It resolves RO-Q2 by rejecting cross-owner local rebinding. [H-272](DesignAuthorityRegistry.md#h-272) makes default function/procedure parameter targets callee-local: bits are copied, and modifying the parameter leaves the caller unchanged. [H-273](DesignAuthorityRegistry.md#h-273) confirms `in O`, `where {owner O}` and borrowed-result owner annotations, and permits omitting `ptr` in parameter declarations. [K-418](DesignAuthorityRegistry.md#k-418) records this syntax decision under C-65/P-244–P-246; wider qualifier/resource interfaces remain open; K-429 records the implemented ordinary-pointer subset.

- **Current decision / coverage:** [H-270](DesignAuthorityRegistry.md#h-270) confirms RO-Q1: assignment-family results retain destination views, including bits destinations, and ordinary-variable `=` assigns references. H-302 refines reference-result access while retaining readonly transfer results. A rejected cross-owner reference assignment must suggest `<-` for bits transfer. [K-416](DesignAuthorityRegistry.md#k-416) records the scoped reconciliation of C-64/P-241–P-243 and the acceptance cases below. This confirms the result contract, not the full qualifier/place implementation. H-271 separately resolves RO-Q2 below. [K-415](DesignAuthorityRegistry.md#k-415) retains the [owned-pool factory evidence](../tests/cod/language/owned-pool-validation.json) under D-17 E4/E5.

- **Lifecycle baseline:** [K-414](DesignAuthorityRegistry.md#k-414) records completed H-267/H-269 E1–E5 acceptance and [current native evidence](../tests/cod/language/exception-ownership-validation.json). Wider unconfirmed scopes retain their status.

- **Bounded exception/lifecycle contract:** [H-267](DesignAuthorityRegistry.md#h-267)/[K-408](DesignAuthorityRegistry.md#k-408) accepts [D-17r3 E1–E5](ExceptionControl.md#accepted-first-implementation-contract), including explicit throw-from only, cleanup, fresh owners and call/result transfer. This overrides pending labels for that enumerated scope; wider qualifier, assignment, mailbox and user exception graph choices remain open. [K-414](DesignAuthorityRegistry.md#k-414) records completed H-267/H-269 E1–E5 acceptance and [current native evidence](../tests/cod/language/exception-ownership-validation.json). Wider unconfirmed scopes retain their status.

- **Registry / coverage:** [D-14](DesignAuthorityRegistry.md#d-14), revision 73, all sections. [K-411](DesignAuthorityRegistry.md#k-411) records bounded D-17 lifecycle evidence; wider unconfirmed choices retain their status. [H-265](DesignAuthorityRegistry.md#h-265)/[K-406](DesignAuthorityRegistry.md#k-406) supplies unchecked OOM and exception-control planning; [D-17](ExceptionControl.md) owns the accepted bounded propagation/cleanup contract. [H-263](DesignAuthorityRegistry.md#h-263) preserves permitted local views across ownership transfer; [H-264](DesignAuthorityRegistry.md#h-264) selects the first typed let/make/root-initializer form. [K-405](DesignAuthorityRegistry.md#k-405) records their scope; H-265 supplies the subsequent unchecked-exception direction. [H-262](DesignAuthorityRegistry.md#h-262) confirms compiler-tracked consumption of ordinary named owners; [K-404](DesignAuthorityRegistry.md#k-404) propagates that scoped decision. [K-403](DesignAuthorityRegistry.md#k-403) records the lifecycle decision sequence and current implementation boundary below; qualifier and extended construction choices outside H-267/E1–E5 remain `pending-human`. [K-396](DesignAuthorityRegistry.md#k-396) coordinates module-world selection and current lifetime evidence; it selects no new ownership mechanism. [K-382](DesignAuthorityRegistry.md#k-382) records typed factory forwarding with preserved captures, implementation context and result owners. [K-377](DesignAuthorityRegistry.md#k-377) records direct adapted regional factories; [K-374](DesignAuthorityRegistry.md#k-374) records algorithms across distinct pool layouts/owners. The complete draft and candidate value path remain `pending-human`. K-379/K-367 reconciles evidence navigation and lifetime descriptions. K-346/K-338 retains the explicit-memory function and pending assignment-place/result reviews. These earlier records select no owning construction or assignment policy; H-262 supplies the named-source post-state without qualifying the full lifecycle. [K-401](DesignAuthorityRegistry.md#k-401) records H-255/H-256 comment reservation and H-260/H-261 quantity/widening decisions; full fraction execution remains deferred.
- **Value / dependencies:** C-54/P-143 and D-14/P-155 relate explicit lifetime responsibility to C-10's typed business intent; C-39/P-94 preserves regional storage. P-156–P-158 links effects/capture (D-10), graph lifetimes (D-13) and regional invariants (D-01). These candidate value paths and technical dependencies retain their registry status.
- **Human baseline:** H-166/H-180/H-181 establishes module lifetime, owner-preserving capture/shadow and non-extractable dynamic resources. H-190/H-193/H-196–H-203 distinguishes whole-region ownership from user storage policy, permits explicit-memory functions, and supplies borrowing, const and same-owner constraints. H-204–H-210/H-222–H-224 supplies transfer/assignment requirements; H-270 confirms destination-view results and reference-only assignment; H-211–H-220 governs interface and method context. H-262 permits non-optional named owning variables and makes them statically unavailable after transfer. H-263 separates that consumed name from still-valid views under a live accessible local owner; H-264 selects the first declaration/construction syntax.
- **Invariants / non-goals:** Regional objects remain region-only and relocatable without copy/move. Borrowing cannot extend lifetime or duplicate ownership. Functions cannot access dynamic bindings or call procedures. No per-object owner, runtime borrow registry, immutable snapshot or SNN policy is introduced.
- **Native evidence:** [Borrowed-region validation](../tests/cod/language/region-borrow-validation.json) covers caller-region allocation, forwarding, escape/extraction rejection, pool reuse/relocation and captured/hidden owners. The bounded implementation is described below; [Owning-lifecycle evidence](../tests/cod/language/ownership-lifecycle-validation.json) separately covers typed make, fresh owners, consuming calls/results and exceptional joins; general const/assignment/mailbox forms remain unimplemented.
- **Open scope / acceptance:** Owner reinitialization, extended construction beyond E5, optional outcomes, occupied destinations, const propagation, reference-assignment permissions and graph-transfer adaptation need coordinated decisions. H-221 authorizes continuing independent confirmed work. Ownership acceptance requires complete construction/edit/transfer/cleanup workflows with exact release and invalid-access checks, not only an allocation sample.

[Module worlds](ModuleWorlds.md) governs selection of regional-policy types and methods: information merges only in the receiver, and publication is explicit. Import-adapted dependencies retain conflicting implementations for typed rejection under H-253. Shared type identity or compatible TLS binding does not transfer ownership, extend a borrow or edit a source world. [The architecture audit](../tests/cod/language/world-architecture-validation.json) checks captured/hidden resources, source-world isolation, scope escape and dynamic-owner extraction; broader ordinary-owner/packet acceptance below remains separate.

## Ref and ptr declaration direction

H-309 confirms `ptr next:Node` as the declaration shape for a stored pointer field. It also directs ordinary variables toward ref or ptr declarations, eliminating let/var-style variable declarations. H-311 separately permits direct bits fields with val or no modifier. For reference declarations, ref must reject reads before initialization at compile time; dereferencing ptr may produce a null-pointer exception. H-313 accepts the coordinated checked-NPE and val increment; native evidence and remaining source boundaries are recorded separately.

Ordinary declarations use ref/ptr for reference bindings and val for independent bits storage. An object view is annotated with its target type, such as Node, with owner annotations where needed. Allocation and ref_at return such views directly. Dedicated region declarations and symbolic effects retain their separate resource/binding contracts. Let/var declarations and reference type constructors have no compatibility parser path.

### Separate initialization, absence and access

Three facts must remain distinct:

1. Whether a binding or stored edge has an established target.
2. Whether the target's payload has been initialized sufficiently for the requested read.
3. Whether that target's owner is still live and this access has the required permissions.

H-309/H-313 requires compile-time rejection before initialization. A readable ref denotes a non-null valid target; a potentially null ptr cannot silently acquire that guarantee. A ptr may represent absence and raise NPE when an actual dereference needs a target. This is not permission to read an arbitrary uninitialized address or bits payload. H-297's payload-read checks, fixed owners, no pointer arithmetic, same-owner stored edges and const access remain independent constraints.

No target absence policy makes a dangling reference acceptable. A null check cannot establish that an already-consumed or expired owner is live. NPE delivery should integrate with the established exception/cleanup mechanism rather than rely on a host segmentation fault. Its checked status is specified separately in [D-17](ExceptionControl.md#checked-nullpointerexception); the accepted allocation failures remain unchecked. NullPointerException is the accepted intrinsic checked category; broader pointer-conversion syntax remains separate.

The keyword distinction does not imply that ref bindings are immutable or ptr bindings own their targets. The existing const-binding versus readonly-target distinction should remain orthogonal. Ref rebinding requires a proven non-null target with the same owner and admissible access. Declaration groups retain definite payload initialization; parameter omission retains the accepted callee-local default.

### Field storage: val, ref and ptr

H-311 distinguishes direct value storage from stored references. H-310 applies to fields explicitly declared ref or ptr, including scalar targets; it does not require every field to be reference-marked.

| Field declaration | Storage in the containing object | Access contract |
| --- | --- | --- |
| `count:i64` | The i64 bits directly; equivalent to val for field storage. | Read/write the field payload, subject to initialization and access checks. |
| `val count:i64` | The i64 bits directly. | Same storage contract as an unmodified bits field. |
| `ref shared:i64` | A reference to i64 storage in the same owner. | The ref initialization-check direction applies. |
| `ptr next:Node` | A potentially null reference to Node in the same owner. | Dereference may produce checked NPE under D-17. |

```text
data Node {
  count:i64;       # Direct bits; equivalent to val count:i64.
  val flags:u32;   # Direct bits.
  ref shared:i64;  # A stored reference, including for a scalar target.
  ptr next:Node;   # A stored, potentially null reference.
}
```

A val/unmodified field occupies payload storage within the containing object. `node.count <- 3` writes those bits; it needs no separate scalar target allocation. The field does not contain a pointer slot that = may rebind. Val describes value storage, not immutability; access qualifiers remain independent. The confirmed decision covers bits fields and does not authorize copying/moving non-bits regional objects inline. H-313 accepts the local/parameter extension below.

For a reference field, its target occupies distinct storage in the same owner. `node.shared` reads the referenced bits, `node.shared <- amount` writes those bits, and `node.shared = other` rebinds the field edge to an admissible same-owner target. Two reference fields may share one target; a payload write is then observable through both, while rebinding one field leaves the other unchanged. Separate target storage does not require a separate host-heap allocation, per-object owner or destructor.

A regional reference field cannot point to a caller/callee stack scalar even when it is alive. Initializing that field must bind valid same-region storage, potentially through an explicitly selected constructor-allocation shorthand. Direct bits-field initialization simply transfers the bits and retains no source reference. No constructor or field assignment may hide a cross-owner edge by copying its native pointer bits.

Intrinsic numeric payloads remain fixed bits. A val/unmodified fraction field may embed those bits; a ref/ptr fraction field stores an edge to a same-owner payload. [D-16](FixedWidthFractions.md#storage-access-and-regional-integration) preserves this distinction. Dense scalar array-element layout is not implicitly changed by the field declaration rule.

Memory accounting distinguishes direct payload fields from reference slots and their targets, counting shared targets once. Stored references remain relocation-safe; neither ref nor ptr introduces pointer arithmetic or cross-owner stored edges. The native prototype implements the field keywords with the existing relative-reference representation; K-441 records scalar/object edge relocation after releasing the original image.

### Val locals and parameters

H-313 accepts explicit bits storage in lexical scopes and function/procedure parameters. A local val owns its bits storage in its lexical scope; a val parameter receives independent callee-local bits. Neither owns a region or copies a regional object. Val does not imply const. `<-` updates writable payloads; `=` has no builtin reference binding to replace on an existing val. Ref/ptr variables may refer to val storage under the same owner, initialization and access rules. LLVM may keep unaliased bits in registers.

```text
val n:i64 <- 3;
ref alias = n;
alias <- 4;
function copied(val x:i64) -> const i64 { x <- 9; x }
```

Fresh val declaration initialization materializes independent lexical bits; `make local` can explicitly supply the new storage, including delayed initialization. Reads still require definite initialization. H-272/H-273's omitted-parameter behavior remains: default bits parameters target callee-local storage; externally borrowed parameters require their owner annotation. Val parameters cannot name an external owner. Val is not a new overload-selection dimension or permission to allocate non-bits regional objects on the stack. Ordinary const results retain their separate value/view contract.

A reference to local val storage cannot escape that scope or be stored in a regional object. Future fraction val storage uses D-16's fixed bits with its nonzero-denominator validity requirement; no implicit zero initialization, layout, widening, canonicalization or fraction execution follows. Typed local storage and initialized reference targets remain separate from the nullable-pointer contract.

A val initializer requests a bits copy before its result-producing lexical block, conditional branch or handler ends. This permits copying an intrinsic exception payload out of an ending region. Ref/ptr initialization requests a view and still rejects escaping storage. Const ref may bind a statically known procedure as a fixed lexical callable alias; this is compile-time metadata and introduces no runtime closure allocation or mutable callable-pointer model.

### Null literals, tests and first owner acquisition

H-313–H-315 requires literal `null` and `isnull(p)`. `ptr p:T = null` has an explicit target type but no target owner yet. An optional `in r` fixes the owner from a known region; otherwise the first non-null assignment establishes it. An absent target creates no artificial lexical owner. Once established, clearing to null does not forget that owner, and subsequent non-null rebinding must retain it. Branches cannot establish different owners for one pointer. Owner/lifetime checks still reject expired targets; null testing never repairs invalid lifetime.

A pointer field inherits its containing region owner and can be initialized/rebound with `=`. Bare/val bits fields use `<-` to initialize their direct payloads. Ref fields reject null or unproven nullable sources; their target must be initialized and belong to the containing owner. Reference initialization does not invent a scalar target allocation. `isnull` checks the pointer slot, whereas a payload read/write can raise checked NPE. Generic callable argument evaluation cannot stand in for this intrinsic, because it may already dereference the argument.

Mutable ptr bindings rebound in a loop are conservatively nullable at the loop header; ref bindings retain their non-null constraint. A pointer whose owner has not yet been established cannot be dereferenced on a repeated loop path in this bounded analysis. Establish its owner before the loop, provide in r, or assign before reading. An isnull test itself does not dereference it. typeof(isnull(p)) and typeof(object.reference_field) inspect declarations without loading the target or raising NPE.

### Construction and conversion review

For `count:i64` or `val count:i64`, `make Packet(capacity = bytes) { count <- 3 }` initializes the directly stored bits. It does not allocate a separate target. The construction-allocation question applies only when count is explicitly ref or ptr. For such a field, automatic same-region target allocation during initialization versus explicit target allocation remains unselected; H-311's correction is not an answer choosing either policy. Ordinary later <- writes an existing target and = rebinds an edge; neither gains implicit allocation from this discussion.

The first source rule must handle both a delayed reference target and an allocated-but-uninitialized bits destination. An allocated slot can be a valid destination before it is readable. A ref-backed graph may likewise need its nodes allocated before all cyclic edges can be filled. Merely sharing one region cannot prove those edges initialized. Construction must establish reference fields before publishing the object; general delayed/cyclic ref publication remains outside the bounded constructor subset. No general runtime initialization registry is introduced.

Pointer declarations require an initializer; explicit null represents absence without reading indeterminate storage. H-314 specifies optional owner annotation and first non-null owner acquisition. Stored fields inherit the containing owner.

A potentially nullable ptr assigned to ref needs either a compiler-proven valid target or an explicit checked operation. A check of a mutable pointer field cannot establish validity after another alias or call changes that field; a retained checked view and reloading the field are different observations. The current checker refines a stable local binding on the non-null branch of isnull; reloading a mutable field is not covered by that proof. A separate pointer-conversion operation remains unselected. Parameter omission was accepted by H-273; introducing ref/ptr must explicitly specify the omitted kind and preserve callee-local bits versus external-owner arguments rather than silently changing that contract.

H-146 retains the current encoding limitation: a first regional pointer field aimed at its own object has displacement zero and reads as null. A promised non-null ref cannot silently inherit that behavior. The native ref-field store rejects that unrepresentable edge with a runtime representation guard, preserving its non-null promise. Ptr fields retain H-146 behavior. Intrinsic object fields or another representation remain separate decisions; no shilos layout changes.

### Pending decisions and acceptance

H-310–H-315 establishes direct/reference field storage, readable-ref guarantees, explicit null, first-owner acquisition and checked NPE. H-317 covers explicit ptr/ref result modes and retains nullable edges for cycles; staged required-reference publication and alternative relative encodings remain separate design work.

K-441 records the bounded acceptance: initialized ref aliases and rejected nullable sources, val initialization/access checks, checked null dereference with cleanup, fixed-owner joins, and mixed direct-bits/reference records with scalar sharing, rebinding, transfer and relocation. General delayed cyclic reference publication still needs its own design and acceptance; current constructors must supply every field.

### Nullable call boundaries and cyclic construction

H-317 confirms RO-Q3: explicit ptr parameters and results forward nullable views without reading their targets. For example, `function find(table:Table in O, key:i64) -> ptr Node in O where {owner O};`. An explicit `ptr node:Node in O` parameter may inspect isnull without throwing; actual target access requires checked NPE unless the compiler proves non-nullness. Ref and default object results promise non-null targets. Forwarded views retain an explicit, live external owner even when null. Null does not erase lifetime, same-owner or access checks; this is separate from optional owned-region carriers.

H-317 accepts the RO-Q4 recommendation: use nullable ptr edges for cyclic construction in this increment. Staged publication of mutually required ref fields is deferred until a real application requires it. Current constructors establish required fields before publication; no runtime initialization registry or relative-reference encoding change follows.

## Unified callable captures and region owners

[RegionalClosures.md](RegionalClosures.md) specifies ordinary regional aliases, inferred/explicit body captures, named owner associations and projection. The storage of a captured val/ref/ptr slot must belong to a live region; a lexical pointer into that region is not a regional binding slot. Captures borrow access and do not extend ownership or freeze contents. Captured slots observe later bits writes or reference rebinding, subject to initialization, access and checked-NPE rules.

Owner lifetime does not promise a historical business identity for a reused library slot. [Item validity](#reuse-is-not-whole-region-release) remains a library policy. Lexical callable captures do not change perform/behave's separate dynamic binding semantics. General stored closures and the scope/defer increment remain outside the implemented contract.

## Pointer variables and owner-generic parameters

H-271 establishes the following source model for user-program variables. `ptr` declares a reference-semantic variable; it is not a raw-address type exposed to arithmetic. A variable's initializer determines the identity of the owner of its target storage: a particular region instance or lexical-scope instance. That owner identity remains fixed for the binding's lifetime. The target may change within that owner, subject to its type and access permissions. Rebinding to a different owner is a compile error, even if both owners are alive and both targets have identical types.

The scope containing a pointer binding need not own its referent. A local pointer can refer into a region, or into an enclosing live lexical scope; its fixed owner is that referent's owner. This does not create a stored cross-owner graph edge. A pointer field within an object remains subject to H-203: its containing object and target must share the same owner.

Variable names in read positions always denote their dereferenced contents. For a numeric target, `p + 1` therefore computes on the number, not on the address. `p.field` accesses the target object. `=` uses the source referent's identity to rebind the destination pointer; it must not erase that identity by eagerly reducing every expression to loaded bits. `<-` transfers payload bits or admitted whole-region responsibility under its existing typed contract. No source-level address arithmetic, hidden dereference syntax or arbitrary native-address conversion follows from `ptr`.

Schematic declarations, with the established `ptr` keyword and the existing assignment distinction:

```text
ptr current = root(first_region);
ptr next = another_node_in_first_region;
current = next;                      # Same owner: rebind, without copying Node.
current = root(second_region);       # Different owner: compile error.
current.count <- root(second_region).count; # Bits transfer can cross owners.
```

The two regions may both have the same root type. Type identity does not establish allocation-owner identity. Repeated calls to one allocation site likewise create distinct owner instances. A control-flow join cannot forget the initializer's fixed owner or accept a cross-owner rebind simply because both paths are individually live. New `ptr` shadowing declarations have their own initialization; rebinding an existing pointer does not redeclare it.

### Dynamic symbols have a separate owner rule

H-277–H-281 distinguishes ordinary initialization-fixed `ptr` owners from symbolic bindings. Bits payloads reside directly in TLS and require `effect @bits <- rhs`. A non-bits object symbol uses `effect @object = rhs` to reference an object inside a region supplied by a region symbol; it may rebind across such region owners if its type and lifetime remain valid. The TLS reference does not become a cross-owner edge inside a regional graph, and it does not consume the supporting resource. Shadowing and capture preserve the actual referenced region and hidden owners. A symbol or capture must not outlive that resource. [Symbolic storage](SymbolicDynamicScope.md#symbolic-storage-and-regional-provenance) owns the detailed contract and acceptance cases.

### Default parameters target callee-local storage

H-272 confirms that a default function/procedure parameter has a target owned by the callee's lexical scope. Passing a bits argument evaluates and converts its value into independent local storage. The callee's parameter remains a pointer variable, but edits through it do not modify the caller's bits slot. Thus all variables have reference semantics without making every call borrow the caller's storage.

For `f(ptr x:i64)`, a caller supplying n does not let `x <- 9` modify n. This applies to both functions and procedures. It replaces H-201's unqualified-bits-parameter default borrowing; const is no longer required solely to isolate such an argument's bits. It does not by itself settle ordinary result annotations or every const permission rule. Evaluation/conversion must still finish for each argument before the next argument is evaluated.

Non-bits regional objects cannot be implicitly copied into callee-local storage: regional-only allocation and no object copy/move remain constraints. Such arguments need an explicit external-owner parameter, or a separately admitted whole-region consuming signature. A local reference to a whole-region ownership carrier must not duplicate its cleanup responsibility. The carrier's storage owner and the region whose lifetime it manages are distinct facts; the exact `ptr` spelling for that resource case still needs to preserve the established consumed-state and cleanup contract.

### Confirmed owner syntax

H-273 confirms the owner-generic spelling required by H-271 and optional parameter modifiers. H-317 gives an explicit ptr with an external owner the additional nullable-forwarding contract. An omitted modifier retains the ordinary argument-read boundary; default bits targets remain callee-local. Ref/ptr do not introduce distinct referent type identities or nullability-based overload selection. Local bindings use ref/ptr; val selects independent bits storage.

```text
# Both external targets must have the same owner instance.
function combine(dst:i64 in O, src:i64 in O)
  -> void where {owner O};

# Independent owner variables: equal or different instances are permitted.
function copy_bits(dst:i64 in D, src:i64 in S)
  -> void where {owner D, owner S};

# A returned view retains the external owner; the bool parameter is local.
function choose(a:Node in O, b:Node in O, second:bool)
  -> Node in O where {owner O};
```

For example, `a:Node in O` and `ptr a:Node in O` are equivalent parameters. Likewise, `second:bool` and `ptr second:bool` both target a callee-local copy. This applies to function and procedure signatures, including signature-only declarations. These forms are confirmed source syntax. The prototype accepts both regional and lexical storage for generic owner constraints.

`in O` associates a parameter or borrowed result with an owner variable. `where {owner O}` introduces that variable alongside, but distinct from, type parameters. One O constrains all of its occurrences to one actual owner; distinct variables D and S do not assert inequality. A caller need not pass an owner-ID value: the compiler checks the arguments' tracked provenance. O is neither a type identity, a TLS symbol, nor a requirement for RTTI. The same source method can be specialized for different owner instances without erasing their distinction within a call.

An annotation describes the referent's owner, not where the callee happens to store its pointer metadata. This permits deep forwarding and same-owner graph operations while preserving the original lifetime. Borrowed results must name an owner that survives the call; returning a view into default callee-local bits is invalid. A separate owner parameter can also represent an external lexical scope, so the mechanism is not limited to regions. A borrowed pointer's owner annotation never grants whole-region ownership or extraction rights.

### Region owners with root-policy access

H-274 distinguishes two signature requirements:

| Owner declaration | Accepted owner | What the body receives |
| --- | --- | --- |
| `owner O` | Region or lexical scope, without a root-type constraint | Provenance/lifetime constraints; the declaration alone supplies no region reference or root-policy operations. |
| Region-specific owner constrained by RootType | A region whose concrete root satisfies the selected type constraint | The same owner constraint plus a usable borrowed reference to that actual region, permitting authorized calls to its root's memory-management methods. |

H-275 confirms the region-specific declaration spelling:

```text
function extend(node:Node in r) -> Node in r
  where {region r:NodeStore} {
  allocate_neighbour(r, root(r), node)
}
```

`NodeStore` names a concrete root type or an abstract root-policy interface. `region` declares the kind of owner and `:NodeStore` constrains its root; r is not the root object itself. The body can use r as a borrowed region reference and `root(r)` to access its root. `allocate_neighbour` is an ordinary user-library method in this example, not a new intrinsic. The actual region is inferred from node's tracked owner; no lookup of an ambient effect or separate user-supplied owner ID is needed. The generic `owner O` spelling remains unchanged.

An abstract root constraint follows the existing explicit-conformance and module-visibility rules. It retains the actual concrete root type and region instance for demanded method selection; it introduces no structural-field test, base-layout cast, copied root or general RTTI. Two arguments marked `in r` must belong to the same region instance even if two different regions both satisfy NodeStore. A lexical-scope-owned argument cannot satisfy the region-specific declaration.

The region reference conveys borrowed access rather than cleanup responsibility. It cannot be consumed, sent away or installed as a new owning effect. The method may call permitted user allocation/reuse operations while the region remains live. Its root-policy reference does not bypass const restrictions or manufacture write permission from read-only input; the exact const/permission spelling remains part of the wider qualifier work. A function receives the reference explicitly through its signature and therefore needs no dynamic-context access. Returning `Node in r` retains the same lifetime dependency.

This connects the owner annotation to the actual-region requirement of H-206/H-208: graph-transfer methods need the source and destination regions to invoke their respective storage policies. Distinct region binders permit different policies; sharing one binder asserts identical ownership. A method with no input carrying the region cannot infer its instance merely from a desired root type; allocation-only factories still need an admitted explicit region input or fresh owning construction. No global default allocator is implied.

Acceptance must use a root-policy method that allocates or reuses storage in the correct actual region, including a result constrained to that region. It must reject a lexical owner, an unavailable root conformance, and different-region operands constrained to one binder. A second concrete root implementation should use the same abstractly constrained helper through selected methods. Existing explicit-region pool examples motivate this requirement but do not validate the inferred region binder.

### Shared owner constraints across symbolic references

H-284 proposes `symbol @o1:T1 in r, @o2:T2 in r where {region r:RT};`, applying actual-region identity and root constraints to symbolic object bindings. This reuses owner relations without imposing ordinary ptr's permanently fixed target owner: a whole group may be shadowed into another suitable live region. The object references still require backing by a region symbol, and inferring r grants no ownership or new ambient region binding. [D-10](SymbolicDynamicScope.md#shared-region-symbol-declarations) records H-285's left-to-right RHS evaluation followed by joint installation, preservation of the effective same-owner relation, and H-286's rejection of incompatible partial replay. Explicit `touch @o2` includes the companion in the procedure's capture dependencies without accessing its object contents or extending its region lifetime. Existing region owner parameters and ordinary whole-region transfer rules are unchanged.

### Region as a declaration keyword throughout the language

H-275 selects this language-wide direction. A region declaration identifies the storage resource and its root type; a ptr declaration identifies an object or bits view. The actual region is not its root object: `root(r)` supplies that object while r supplies the region reference needed by storage-policy methods. Generic `owner O` remains a provenance constraint accepting regions and lexical scopes; it introduces no resource value by itself.

The root annotation must not be reinterpreted as allocating a RootType object in the surrounding lexical scope. The resource remains intrinsic; a borrowed region reference does not gain cleanup responsibility, and an owned region still has exactly one transferable cleanup obligation. Removing a user-facing type-constructor spelling does not erase the compiler's distinction between a whole-region reference, a region owner and an interior object view.

The declaration shape covers locals, explicit parameters, results and symbolic procedure signatures. The root annotation is optional for an untyped region resource. The compiler retains the actual root where known and rejects incompatible declared roots:

| Role | Declaration form | Meaning and remaining boundary |
| --- | --- | --- |
| Inferred owner and actual region reference | `where {region r:NodeStore}` | Confirmed by H-275; infer r from arguments marked `in r`, check its root constraint and provide borrowed access. |
| Explicit borrowed region parameter | `function edit(region r:NodeStore, node:Node in r) -> void` | The caller supplies r explicitly; the node must belong to that same instance. This also serves calls with no interior argument from which to infer a region. |
| Borrowed local region binding | `region view:Pool = existing_region;` | No ownership acquisition; retain the actual region and its lifetime dependency. |
| Owning local region binding | `owned region packet:Packet <- make Packet(capacity = bytes) { sequence <- initial };` | Retain the owned qualifier; construction and single-owner transfer/cleanup remain the accepted E4/E5 contract. |
| Consuming region parameter | `function process(owned region packet:Packet) -> void` | Preserve consumption at callee entry after arguments/conversions succeed. |
| Borrowed or owning region result | `-> region :Pool` / `-> owned region :Pool` | Unnamed result declaration; avoids adding a named-result scope solely to remove the type-constructor spelling. Borrowed provenance and owning-result transfer remain distinct. |
| Typed symbolic region resource | `region symbol @state:Pool;` | Preferred symbol declaration form under H-276. It does not install a value; installation still uses `effect @state <- rhs`, and lookup cannot extract ownership. |

The prefix `region symbol` applies to the entire comma-separated declaration. Every item declares a region-reference symbol, with its own identity and root annotation; different root types are allowed. Non-region symbols use a separate declaration:

```text
region symbol @state:Pool, @scratch:Scratch;
symbol @step:u64;
```

Grouping shares only the declaration kind. It does not require the symbols to bind the same region, allocate a region or install a dynamic value. Their values still come from effect bindings with the established capture, shadow and restoration semantics. A line break is formatting; the declaration ends at its terminator.

For explicit parameters, `region` identifies a whole-region reference rather than a root-object parameter. `function edit(region r:Pool, node:Node in r)` requires both arguments to denote the same actual region; it also makes r available for root-policy operations. `const region r:Pool` restricts writes through that borrowed parameter. Bodyless signatures, callable selections and generic root aliases preserve this distinction. A plain generic value parameter cannot hide a whole-region argument. No user-data field can store a whole-region handle.

A borrowed result uses `-> region :Pool in r`; an owning result uses `-> owned region :Pool`. A symbolic procedure signature uses forms such as `proc(owned region :Pool) -> owned region :Pool`. Locals use `region view = existing` or `owned region value <- fresh`; omitting the root annotation infers it without omitting the declaration keyword. Region declarations yield void. Whole-region values cannot be held by ordinary ref/ptr/val declarations or expressed by the former angle-bracket type constructor. There is no compatibility parser path. Resource installation, captured shadows, no-extraction, single cleanup responsibility and zero-cost relocation retain their established contracts. General owner-bearing optional/results remain separate design work.

### Integration and acceptance boundaries

The compiler needs to retain a pointer's target type, target storage, fixed owner and access permission separately from a bits value loaded for computation. This is semantic information, not a requirement to allocate a pointer object or runtime owner record for every scalar. LLVM may eliminate storage when the program cannot observe its address or aliases. Same-owner proof must survive nested calls, rebindings and branch joins; inlining cannot extend a source lifetime.

The following details remain to be coordinated with the confirmed principle: general owner-bearing optional/result carrier storage; and parameter/result conversions that create new storage. An external-owner annotation cannot be treated as mere documentation while a conversion silently substitutes an unrelated local object. These are affected interfaces, not permission to silently introduce copyable regional objects or runtime owner dispatch. Symbolic effects retain their separate shadow/restore and no-extraction contracts; this pointer declaration rule does not authorize bare `@x = rhs` or `@x <- rhs`.

Acceptance should demonstrate same-owner local rebinding and rejection across owners of identical types; implicit numeric reads without address arithmetic; a default bits parameter whose mutation leaves the caller unchanged; an owner-generic parameter whose permitted payload write is visible to the caller; same-owner checks through a deep helper call; and rejection of a borrowed result from callee-local storage. Including or omitting ptr in a parameter declaration must preserve its binding, dispatch and owner checks. H-270's destination-view/const-result cases remain necessary. K-429 records native checks for this bounded storage and call contract; the limits below remain explicit. Fixed-width fractions remain bits for storage/transfer purposes; this rule introduces no fraction implementation, layout or promotion policy.

### Bounded pointer and initialization implementation

Ordinary `ptr` bindings retain target storage independently of a loaded numeric value. Local scalar/quantity/bool targets lower to LLVM storage places; default bits parameters receive independent callee-local targets. `where {owner O}` accepts a region or lexical owner, while `where {region r:Root}` accepts only a qualifying region and exposes its borrowed reference. Non-bits parameters require an explicit external owner. A const parameter remains read-only after rebinding within that owner. Signature-only result-owner promises are checked against the selected method's actual returned storage.

`make local`, individual value blocks, final comma group blocks, inferred fresh-bits initializers and `ptr n <- rhs` are implemented. A plain alias initializer retains its source target, owner and permissions. Bits `<-` writes the target; ordinary-pointer `=` rebinds within its fixed owner. Both associate rightward; builtin `=` preserves the destination access and `<-` yields a read-only target view. A fresh ordinary const bits result is copied; an explicitly owner-qualified result remains a view. A group block's result is discarded. Initializer names resolve before the new declaration enters scope, and each runtime RHS is evaluated once.

A builtin writable ptr rebind rejects a read-only source view, even when owner and type match (H-299). A new alias may retain a read-only view, and `<-` may copy bits into independently writable storage. H-302 makes builtin `=` retain the destination's access: writable `a = b = c` aliases the same live owner/storage, while read-only destinations remain read-only. Overload selection precedes this builtin check. A `(writable lhs, const rhs)` method is permitted; its body must establish any new writable target through independently valid access. Returning a borrowed view does not expose a rebindable pointer slot.

Exact initialization facts belong to allocations, so aliases and inlined external-owner calls share them. A repeating pointer can also carry a proven-initialized target without an exact allocation identity. Such a view does not let a later write mark an unrelated tracked allocation initialized. Loop backedges must preserve the entry initialization guarantee; branches and exceptional exits keep their own facts. Reaching branches and exception handlers intersect definite facts; a possibly empty loop cannot establish initialization solely from its body. Local slots are not zeroed, and no runtime initialization flags or owner tags are emitted. An explicit borrowed argument can supply an uninitialized destination, but every read inside its callee still checks that storage. A default copied argument must already be initialized at its own evaluation point, before later arguments run.

The bounded `typeof` query reads declaration/operand types without emitting the operand's evaluation. It supports known bindings, scalar literals/operations, fields/indices, selected intrinsic forms and calls with declared result types. It can inspect an uninitialized slot's type. A call whose result requires executing or analyzing an unannotated body is rejected with a diagnostic; this is not runtime reflection or general RTTI.

Current implementation limits:

- Repeated same-owner rebinding of initialized pointer targets is supported. A loop-carried pointer whose entry target is uninitialized still needs pre-loop initialization before rebinding in the bounded implementation; this is a remaining proof limit, not a language owner rule. Loop-local pointer declarations and writes to an uninitialized fixed target retain ordinary definite-initialization checks.
- A joined pointer may denote several allocations. Reads require every possible target to be initialized; a write through such a pointer does not yet prove which alternative became initialized. This can reject valid correlated branch programs without accepting an uninitialized read.
- Ref/ptr/val are the ordinary declaration forms. Whole-region locals, parameters, results and signatures use region declarations exclusively. Wider qualifier combinations and stored reference-field rebinding remain separate; user data remains region-only.
- General graph-transfer algorithms and fraction execution remain outside this increment. Existing regional transfer overloads and indexed operations require explicit destination owners; this is not a user-visible binding-cell API.

The regression suite checks the compiler's module, numerical, region, exception and symbolic contracts alongside [the ptr workflow](../tests/cod/language/ptr-storage.cod). [The validation record](../tests/cod/language/ptr-storage-validation.json) retains source/compiler hashes and actual results. LLVM promotion can remove unobservable local storage; source-level owner identity does not require a runtime registry.

### Fixed pointer bindings

`const ptr a = expression, b = expression, { ... };` declares a group of non-rebindable bindings. Each initializer establishes its target once; the optional final initialization block can write target contents but cannot rebind a declared name. `const ptr n = make local i64; n <- 3;` is valid: allocating a target and initializing its bits are separate events.

The prefix qualifies the pointer binding, not its referent. A writable referent stays writable; a readonly source view stays readonly. `ptr alias = fixed` creates a separate, rebindable alias with the same owner and target access. Ordinary function/procedure parameters likewise have their own local binding unless the operation explicitly denotes the caller's binding. `=` and `<=>` reject a const-ptr destination, including explicit intrinsic calls and overload invocation. No method can bypass this by receiving the actual caller binding. `<-` and `<->` may edit writable target storage. The qualifier is checked at compile time and introduces no runtime wrapper or const-based overload family.

### Runtime targets and loop invariants

A pointer's target address may vary while its declared type, initializer-fixed owner and target access remain unchanged. Runtime address slots represent that variation; compiler flow information describes the valid views and definite-initialization obligations. For initialized loop-carried targets, the loop header retains those invariants without claiming a particular allocation identity. Every normally continuing backedge must preserve initializedness. Reading a newly selected uninitialized target still rejects; merely selecting it does not initialize it.

The initialized-target abstraction cannot mark a particular allocation initialized after writing through a possibly different target. Conditional joins retain both tracked obligations and initialized alternatives. This keeps allocation-specific initialization proofs separate from the changing address and avoids a general alias-history solver. Exact access/owner checks apply to every rebind, and region consumption/lifetime checks still apply to loops and exceptional exits. Constant zero/single-iteration ranges retain their simpler flow facts. Known-good A/B exchanges need neither a runtime initialization flag nor compile-time parity tracking.

### Loop initialization and optional empty-range branch

Initialization before a possibly empty loop is an accepted workaround under H-299. The loop cannot supply a definite-initialization fact on a path that never executes its body. A writable-to-read-only rebind is independently invalid, rather than a permission change to model across iterations. Same-owner A/B swaps between initialized writable targets preserve the type, owner/lifetime, access and initialization invariants. Their validity does not require proving which allocation is selected on each iteration or tracking swap parity.

H-304 confirms an optional range `for ... else` branch that executes only when the body runs zero times:

```text
ptr result = make local i64;
for (i in first .. last) {
  result <- i;
} else {
  result <- fallback;
};
# Both normally completing alternatives initialize result.
```

For the existing finite range loop, this is a small bounded extension: evaluate the bounds once, branch on the initial range test, and reuse normal branch joins and lexical cleanup. The nonempty path contributes the body's initialization facts; the empty path contributes the else block's facts. A read after the loop requires initialization on every normally completing alternative. Neither bounds failure nor a body exception invokes the else block. No runtime initialization flag or iteration counter is needed. A read before the first body write still rejects.

Range `for` has a void result and an else branch without the loop index. [D-17’s loop scoping rules](ExceptionControl.md#loop-scoping-rules) govern declarations, effect resources and deferred work in both body and else. `fold` already has an explicit zero-iteration accumulator; `fold ... else` is rejected; a future extension would require a separate result contract. Break/continue and future loop forms are outside this bounded assessment. The parser, LLVM control-flow construction, initialization join and lexical dependency scan must preserve these distinct branch and lifetime responsibilities. Focused acceptance covers empty/nonempty dynamic ranges, partial initialization, exceptions and once-only bounds evaluation. This is independent of general loop-carried alias analysis.

The branch means **zero body executions only**; it is not an exhaustion callback. The construct yields void regardless of either block's final expression. Dynamic bindings installed by either block restore on that block's exit. Both source blocks are checked even when constant bounds make one unreachable; only feasible, normally completing paths contribute post-loop initialization facts.

## Bounded local lifecycle and remaining construction scope

The bounded local lifecycle constructs an initialized region, edits it through a borrow, returns or installs its ownership, and releases it exactly once. H-267 accepts this E1–E5 package; K-414 records its focused native acceptance. The candidate value path is C-10 → C-54/P-143 → D-14/P-155: explicit lifetime responsibility lets a business program prepare data safely for either scoped use or transport. H-267 qualifies D-17 E1–E5 for the bounded lifecycle; wider D-14 proposals retain their recorded status. H-262 confirms the named-source post-state, H-263 the local-view lifetime rule, and H-264 the initial declaration/construction spelling below. Mailbox delivery, generic graph conversion and full fraction arithmetic are not prerequisites for this local milestone.

### Confirmed named-source post-state

Consider an existing declaration `owned region packet:Packet` and the operation:

```text
effect @state <- packet;
```

H-262 makes `packet` unavailable in the compiler's control-flow state. A later `root(packet)` or second transfer is a compile error; the runtime value does not become an implicit nullable region. The installed resource remains alive under `@state`, whose reads provide non-extractable access. This chooses a source post-state, not an allocation layout or a new runtime ownership wrapper.

A transferable named region need not occupy optional storage. Explicit optional/result types express runtime absence or a returned send-error outcome; they are not inserted to represent a consumed ordinary variable. Provenance, capture and branch checks remain necessary.

Under this contract, every path reaching a use must retain a valid owner. If one path transfers and another retains it, an unconditional later access is invalid. Explicit optional/results remain useful when the business program needs a runtime outcome, including a returned packet after local send failure. Whether an unavailable binding can be reinitialized remains a separate choice. H-263 keeps pre-existing views usable while the new local owner remains alive and permits their access. Ending that ownership scope invalidates those views; sending the region out revokes sender access. A returned send-error owner does not automatically settle whether old revoked views become usable again.

### Local view validity after ownership transfer

The source name and the storage reached by an existing view have different validity rules:

```text
ref view = root(packet);
effect @state <- packet;
view.sequence <- 9u64; # Valid while @state's owner remains alive and accessible.
# root(packet) is invalid: the original owning declaration was consumed.
```

H-263 selects this behavior. Moving a whole-region handle changes cleanup responsibility, not the region instance or its internal references. The compiler must associate view validity with that actual region and the new owner's lifetime/access. It must not blindly attach every borrow to the original variable's availability. Conversely, consuming an outer owner into an inner scope can shorten the region's remaining lifetime: views may be used within the inner owning scope, but not after it ends. Dynamic shadowing alone does not end a hidden owner's lifetime; H-180's capture/shadow rule remains unchanged.

The analysis may reject uncertain paths but cannot accept use after the actual owner ends. Branches, argument evaluation and nested calls must preserve this distinction. This does not add a runtime borrow registry, reference count or per-pointer generation field, nor does it settle how revoked sender views behave if a send error later returns the same region.

### Construction-to-installation walkthrough

The H-264 construction contract, expressed with the H-307 declaration surface, is `owned region packet:Packet <- make Packet(capacity = bytes) { sequence <- initial };`: capacity may be a runtime expression, the root initializes directly inside the new region, and the result transfers ownership into the declaration. D-17 implements this form and unchecked allocation/capacity failures. The extended walkthrough below expresses the target qualifier/field-transfer surface; executable examples use both field `<-` and the explicit setfield intrinsic for bits edits. Wider const/resource combinations remain separate from the validated make/call/result lifecycle.

```text
data Packet { sequence:u64; }
region symbol state:Packet;

function make_packet(bytes:const u64, initial:const u64)
    -> owned region :Packet {
  make Packet(capacity = bytes) { sequence <- initial }
}

{
  owned region packet:Packet <- make_packet(4096u64, 7u64);
  root(packet).sequence <- 8u64;
  effect @state <- packet;
  proc @inspect() -> const u64 { root(@state).sequence }
  {
    effect @state <- make_packet(4096u64, 20u64);
    perform @inspect(); # 8; the captured resource retains its owner.
    behave @inspect();  # 20; the current binding supplies access.
  };
  behave @inspect();    # 8; the first binding is restored.
};
```

Construction returns responsibility for one initialized region; it does not create a stack Packet and copy it. The first effect takes that responsibility from the lexical source. The second effect temporarily hides the first resource without releasing it. The inner scope releases only its own resource; the enclosing scope then releases the first. Capturing or replaying `@inspect` creates no additional release responsibility. The name `packet` is consumed. A view obtained before the transfer can still access that region while its new owning effect remains live and accessible, under H-263.

The matching ordinary-use example returns an owner from the same factory and passes borrowed access through a helper:

```text
function update(region packet:Packet, value:const u64) -> void {
  root(packet).sequence <- value;
  {}
}

{
  owned region packet:Packet <- make_packet(4096u64, 7u64);
  update(packet, 9u64);
}; # The caller releases its region once; update never owns it.
```

D-17 E4 admits a borrowed argument without transferring responsibility. An owning parameter explicitly consumes at callee entry after argument evaluation/conversion; an owning result transfers after required finally work. H-271/H-272 requires an external-owner declaration for the borrowed path. The prototype still accepts bare region parameters as borrows while that source interface awaits migration.

### Remaining gates and implementation evidence

| Gate | Decision required | Existing implementation and acceptance boundary |
| --- | --- | --- |
| Construction | D-17 E5 fixes runtime payload capacity and every-field initialization in written order. | `make Root(capacity = bytes) { field <- value }` reserves root storage before evaluating fields and releases unfinished construction on exception. `root(region)` returns a borrowed product view. |
| Access and assignment | H-302 fixes ordinary reference-assignment qualifier selection; wider const/place propagation remains bounded. | LLVM storage places support explicit field/element edits. Fresh owning region declarations use `<-`; unqualified region parameters/results borrow. |
| Ownership flow | D-17 E4 fixes consuming entry/result boundaries; owner reassignment remains excluded. | Holder availability and region liveness are separate compiler facts. Normal/exceptional joins track consumed names and permit different fresh owning results; potentially ended borrows reject. |
| Failure and exit | D-17 fixes exact intrinsic catches, finally replacement and cleanup without graph rollback. H-269 retains explicit causes beyond catch exit; K-414 validates their fallible creation and storage. | Native checks cover failure before entry, failure inside the callee, failed root initialization and abandoned owning results. |
| Transport, subsequent increment | Intrinsic owner-bearing result form, local-error return, receiving ownership and sender-view invalidation. | No owning send/receive path exists. Local acceptance must continue to mean no delivery acknowledgment. |

The [native acceptance](../tests/cod/language/exception-ownership-validation.json) checks these local workflows through exact allocation/release accounting, escaped-borrow/duplicate-transfer/conditional-transfer rejection, and a valid factory branch returning distinct newly constructed regions. No RTTI, pointer-encoding change, per-object owner, runtime borrow registry or allocator-policy intrinsic is selected here.

## Exceptional construction and cleanup

H-265 requests `try`/`catch`/`finally` support and classifies ultimate out-of-memory failure as unchecked. Successful make returns its owner normally; OOM does not produce an implicit null or force every call into an optional/result type. The three intrinsic allocation/capacity failures propagate through the accepted catch/finally control; uncaught failure exits nonzero.

[Exception control](ExceptionControl.md) coordinates partial-root cleanup, effect restoration, consumed names, surviving views and handler-entry ownership facts. A failed initializer must retain cleanup responsibility until its unfinished region is released. Exception handling does not reverse already completed transfers or caller-region edits. The ordinary C++ allocation path releases its block when root construction throws; the focused shilos check validates both overloads and provider failure.

H-267 confirms the initial catch payload/selection and finally-failure rule, and K-414 records their direct LLVM implementation with H-269 retained explicit causes. General user exception graphs, RTTI, automatic growth/retry and transport results are not prerequisites inferred from unchecked OOM.

## Agreed behavior and undefined vocabulary

The current constraints are stronger than the source-level ownership vocabulary:

- A region is the intrinsic lifetime/ownership unit. Regional objects do not copy/move; user code may recycle typed slots or manage reusable space inside a still-owned region. Such reuse does not return part of the region to the system or acquire a separate intrinsic owner.
- A fresh owned declaration and borrowed/owning calls/results have the D-17 contract and implementation. Owner reassignment, general writable-place rules and optional owner storage remain separate.
- Installing such a region as an effect resource consumes the source’s ownership. The resource cannot subsequently be taken out of the dynamic binding.
- A procedure captures used dynamic values. Perform temporarily shadows lookup with those values; it does not consume, replace or destroy either the captured or hidden resource owner. Behave uses current values.
- WPO rejects an unavailable required binding. An explicitly absent optional value is different from an unavailable binding.

Here **ownership responsibility** means responsibility for retaining, releasing or transferring a region. H-200 supplies the owned type qualifier and H-262 supplies static unavailability of a named source after transfer. D-17 fixes fresh declaration initialization, call/result transfer and the bounded local-view checks. General reassignment, qualifier and transfer-expression choices retain their separate status.

## Intrinsic region ownership versus intra-region library policy

H-190 makes the boundary explicit. CoD must know when a whole region is created, held, transferred or released, and whether a reference still has a live region. Inside that region, user programs choose graph membership, free-list links, pool capacity classes, slot selection and reuse policy. Those choices do not become language ownership primitives merely because a library calls an operation `allocate`, `release` or `free`.

| Layer | Responsibility | What it does not imply |
| --- | --- | --- |
| CoD region resource | One lifetime/cleanup responsibility, admitted ownership sources, effect installation/shadowing, consuming call/result/send paths and invalidation of access when the region leaves the caller. | Per-object owners, destructors, reference counting, GC or an allocator policy. |
| Ordinary typed CoD operations | Regional placement, intra-region pointer edges, bounded field/array access, initialization/type validity and the function/procedure boundary. | Knowledge of a pool's live/free membership or its business object identity. |
| User library in the region | Pool/free-list topology and metadata, reuse eligibility, logical item identity and any library-specific stale-handle/double-return checks. | Authority to free or transfer the enclosing region, invent an owning handle, or bypass typed storage rules. |

The user-storage boundary remains stack plus regions. Temporary local values/references can still be used while operating on regional data. The clarification concerns ownership and allocation policy; it does not require every compiler temporary or stack bits value to become a regional object.

### Reuse is not whole-region release

A minimal typed pool can allocate stable slots once, then update their fields or associated SoA arrays and maintain a free-slot list/index structure. Returning a slot changes library membership. Acquiring it again uses existing storage; native region occupation need not increase. No regional object is moved and no per-object system `free` occurs. Scalar/index typing, regional views, deref, store and setfield provide these primitives. The native example below validates this bounded use, including function-based allocation-policy updates.

There are two different validity questions:

- **Region validity:** after the whole region is released or successfully transferred out, its old references must not remain usable under the sender's access context. This is an intrinsic concern even when the reference was obtained from a pool.
- **Library item validity:** after slot A is returned and reused for B while the region remains live, an old reference may still reach valid typed slot storage. Whether it is allowed to act on B is a library contract. CoD's region-lifetime proof does not establish logical item identity. A library may prohibit retaining such references or use checked handles if that requirement exists; this review does not mandate generations, counters or a new per-slot table.

Updating a stable, initialized typed slot is sufficient for a first pool example. Reinterpreting overlapping storage as unrelated types, exposing uninitialized storage or ending/restarting typed object lifetimes is a separate lower-level contract. H-190 leaves the allocator policy to user code; it does not automatically authorize unchecked pointer fabrication or C++ object-lifetime behavior. Introduce a further storage primitive only if a concrete library cannot be expressed with existing typed storage.

### Owner-bearing types are intrinsic

H-196 requires any type whose values contain a whole-region owner to be a CoD intrinsic type. An `owned region packet:Packet` declaration has that intrinsic ownership kind; any admitted optional/result form containing ownership must also be intrinsic. An unqualified region view does not itself carry ownership. A user-defined product, sum or generic instantiation cannot acquire that capability by embedding an owner directly or through another field/alternative. This is a transitive containment restriction. Mentioning an owning type in a callable signature is distinct from storing an owner in a user-defined data object.

The compiler must check transfer, projections, arguments/results and cleanup for those intrinsic resource forms. Arbitrary library wrappers cannot convert a dynamic access reference into a transferable owner. The eventual syntax may share pattern machinery with general ADTs, but the owner-bearing type and its lifetime rules remain intrinsic. `Packet` is non-owning and can appear in permitted intra-region data structures while retaining its region provenance.

This does not admit owning handles as arbitrary fields inside another region. Such containment would need an explicit inter-region ownership contract; no per-object pool requirement implies it. Existing direct-pointer edges remain region-local. Packet transport and dynamic-resource installation remain separate uses of whole-region ownership.

## Types, expression categories and storage permissions

The following notation is for analysis, not proposed runtime wrapper types:

| Notion | Meaning |
| --- | --- |
| `T` | The full declared value type of a binding. |
| `R` | A typed whole-region resource. It is distinct from a regional object type allocated inside another region. |
| `Opt(T)` | Semantic optionality, independent of whichever CoD spelling is selected. `Maybe<T>` is an example, not the required type constructor. |
| Bits datum and bits storage | Bits can be copied without regional object copy/move. A ptr variable targets bits storage rather than being a fresh scalar datum; H-272 copies default parameter targets into the callee scope and H-201 retains ordinary const bits value results. General local materialization remains to be specified. Pointer-sized representation alone does not classify a resource or reference as independent bits. |
| Transferable temporary | A transient expression result with one available region-lifetime responsibility that has not been installed in a dynamic binding. Precisely which results qualify remains to be chosen. |
| Writable optional place | A storage location whose contained resource may be removed and whose value can be replaced with its defined absent alternative. This is a candidate permission, not merely a pointer to mutable bytes. |
| Resource access | Permission to read or edit the region’s contents under the existing owner’s lifetime, without taking ownership. |

A resource’s type alone therefore cannot decide whether an expression may initialize an owning dynamic binding. A dynamic `@y : R` provides access but no consuming capability. A dynamic `@y : Opt(R)` likewise cannot have its optional storage emptied by extraction. Procedure access to the region’s contents can still be mutable: the optional slot and the graph it refers to have different permissions.

Aliases, casts, wrappers and functions cannot upgrade access to consuming permission. Constructing a fresh wrapper around a reference is not construction of a fresh region or a new owner. Captures retain their existing lifetime dependencies; they do not acquire an independent destruction responsibility.

## Candidate rule for effect-resource initialization

H-182’s example is:

```text
effect @x : R <- rhs
```

The annotation syntax itself is proposed; the current parser only accepts the unannotated effect form. The following is a candidate contract specifically for new resource installation. It does not redefine packet send, every ordinary assignment, or implicit perform replay.

For a bits-valued binding, evaluate the RHS as a value and bind its checked result. There is no region-ownership extraction. For a direct regional resource, H-262 admits a live named `owned region :RT` source: installation consumes that source and makes subsequent use of its name a compile error. Construction and optional extraction need their own source forms:

1. A newly constructed transferable resource temporary of the required type.
2. A writable optional source place that owns a present resource of the required type and supports replacement by absence. This is an optional modeling facility, not a requirement for all named owners.

The second condition is stronger than “writable reference to an optional type.” The source must actually hold the resource, the contained reference must carry its lifetime responsibility, and the current path must establish presence or explicitly handle absence. An unknown runtime optional value does not become a direct `R` merely because the reference is writable. Requiring a present match arm is one candidate; a checked extraction with explicit failure is another. No implicit trap, default region or universal automatic unwrap is selected.

On successful installation from a present source place:

```text
before: source = Present(r); source is responsible for r
 after: source = Absent;     the new dynamic owning scope is responsible for r
```

The old binding of `@x`, if any, is only shadowed and retains its owner. Source clearing and destination acquisition form one semantic transfer with one remaining cleanup responsibility. This does not prescribe a multithreaded atomic primitive or runtime transaction structure.

| RHS situation | Candidate result for a direct `R` target |
| --- | --- |
| Verified fresh whole-region construction result | Transfer its temporary lifetime responsibility into the effect. |
| Writable optional place holding an owned `R`, with presence established | Remove the resource, leave absence and install it. |
| Writable optional place whose presence is unknown | Need an explicit present path or defined checked-failure behavior. |
| Read-only optional place | Reject as a consuming source. |
| Dynamic `@y : R` | Reject: access does not supply a transferable temporary. |
| Dynamic `@y : Opt(R)` | Reject: the symbolic slot is not writable/extractable. |
| A call, cast, projection or fresh wrapper returning a reference to an existing dynamic resource | Reject: expression shape does not create ownership. |
| A live named `owned region :RT` variable | H-262: transfer responsibility into the effect; mark the source name unavailable. No implicit absent value. Borrowed `region :RT` remains non-consuming. |

An owning constructor result and a writable optional place need to be compiler-distinguishable from an ordinary value read. Whether the effect initializer recognizes the place contextually, or source syntax explicitly requests its consuming use, remains a syntax decision. A pure conversion function cannot secretly perform this extraction; CoD’s existing pure UoM conversion semantics are not a suitable implementation shortcut.

### Perform is not an ownership-source initializer

Perform already has a different contract. It uses a procedure’s saved values to shadow the corresponding identities without creating new region owners. It does not need its captured resource to satisfy the fresh/writable-optional source alternatives. Applying the public new-resource initializer rule to replay would wrongly reject capture or consume ownership. Lowering can share ordinary lookup machinery only while preserving that semantic difference.

## Optionality must be specified by meaning

Any selected optional spelling, nullable-type shorthand, sum form, presence test, pattern, unwrap and optional projection must preserve the same resource permissions. Examples such as `Maybe<R>`, `R?` or an explicit absent/present sum are possible notations to evaluate; listing them does not admit them all into CoD grammar.

The source and target optionalities also differ:

- For a direct `R` target, a source optional must provide its present resource. Absence cannot establish the required value.
- For an explicitly optional `Opt(R)` target, an explicitly constructed absent value is valid and owns no region. A present initializer must still obtain its resource from a legitimate source. H-181’s optional domain modeling must not be excluded by a literal “every non-bits type is a bare R” formulation.
- Optional bits values can be classified by the ordinary semantic bits rules; an optional resource does not become bits because its representation fits in a scalar.
- Nested optionality need not be flattened: outer absence and a present inner absence can have different business meaning. A consuming source path must identify which place and which resource it selects.
- An arbitrary resource-bearing sum is not automatically an optional owner with a uniquely defined empty value. If it has several resource-free alternatives or requires data in those alternatives, explicit case handling must determine the post-transfer value; the compiler must not guess it.

A read-only pattern or projection from `@y : Opt(R)` cannot yield a writable owning source place. It may permit access to the selected resource under the dynamic binding’s lifetime. Shadowing a present dynamic value with explicit absence preserves the hidden owner and restores that value at scope exit. These checks apply to semantic operations, not just to a type named Maybe.

## Procedure-valued effects retain their own contract

The bare rule “bits or region only” does not describe all effect bindings. H-184 explicitly permits both ordinary lexical procedure definitions and symbolic dynamic procedure definitions, and installing an already defined lexical procedure as an effect. These carry lexical meaning and captured dynamic values. They cannot be declared independent bits merely because machine code has an address, nor assumed to be freshly allocated regions.

For this draft, the fresh/optional-source rule applies to resource-valued data effects. Procedure declarations, using/import and procedure-value bindings retain their own callable rules while their representation is designed. A unified type/category account may cover them later, but this draft does not silently remove them or invent a runtime closure allocation.

### Typed symbols and procedure installation

H-184 gives the symbol-declaration form `symbol a : T1, b : T2`; every symbol definition must declare its type, including procedure types. A symbol declaration introduces a compile-time identity/alias and its value type, not a runtime lexical variable or an initial value. Reads use `@a`; writes require `effect`, with `<-` for bits/ownership and `=` for object-reference rebinding. A symbol selected from another vocabulary retains that identity’s declared type.

An ordinary named procedure enters lexical scope. A symbol-named procedure supplies a dynamic binding. Naming does not determine purity: both are procedures, may use effects and retain H-180 definition-time captures. Assigning an existing lexical procedure to a symbol selects that existing procedure value; it does not redefine the body or recapture current values. Procedure `mixin` supplies a new receiving-context definition. `using` preserves the selected implementation context; `import` retains source body/non-callable bindings/captures while adapting ordinary callable implementations under H-230/H-232.

The implemented callable forms are:

```text
proc step(value:i64) -> i64 { value }
symbol step_effect : proc(i64) -> i64;
effect @step_effect <- step;
proc @direct(value:i64) -> i64 { value }
```

`step(...)` is a lexical call using current dynamic bindings under H-189. `perform @step_effect(...)` restores the existing procedure's captures; `behave @step_effect(...)` uses current bindings. H-186/H-187's `proc @direct` also declares its typed symbol if absent. Neither `perform step(...)` nor `behave step(...)` is admitted; functions cannot call either procedure form. Callable types and this syntax do not settle whole-region owning-result types or optional extraction.

This gives the ownership design a concrete boundary: procedure-valued effects are admitted separately from bits and region-resource data. A lexical procedure used as a factory still needs a defined owning-result contract; a lexical name alone does not grant freshness, purity or consuming authority. No arbitrary lexical capture lifetime or hidden heap closure is introduced.

## Whole-region construction needs a separate contract

A “newly constructed R value” must not mean an ordinary rvalue expression, a new view of existing storage, or a stack-allocated regional object. The construction must establish a region’s storage/lifetime responsibility and initialize its contents in regional storage. Returning an owning handle does not copy/move the graph.

Two notions of “fresh” need comparison:

| Interpretation | Consequence |
| --- | --- |
| Newly allocated region instance | Direct construction and a factory proven to allocate a new region can supply it. Merely forwarding an existing resource cannot. |
| Exclusively transferable temporary result | Also permits a result whose responsibility has been transferred out of a legitimate source. It needs a precise result/cleanup contract, not just proof that the syntax is a call. |

H-182 proposes the first wording; the second is an agent alternative, not a confirmed generalization. H-183 leaves the overall ownership language open. A factory or block result must not be deemed fresh merely because it has type R.

### Syntax candidates

| Form to explore | Semantic requirement | Practical trade-off |
| --- | --- | --- |
| Explicit resource expression, illustratively `make Packet(capacity = bytes) { ... }` | Create and initialize one whole region; yield a checked owning temporary. | Makes effectful construction visible at the use site, but requires syntax for construction inputs, internal access and root completion. |
| Procedure factory, illustratively `behave @make_R(...)` | Its checked result must satisfy the same temporary/ownership contract. | Reuses named, testable procedural construction and composition; owning return semantics must be defined first. |
| Contextual optional source, illustratively `effect @x : R <- source_place` | The initializer consumes a writable present optional place rather than reading/copying its value. | Concise and consistent with the transfer arrow, but diagnostics and source typing must expose the consumption. |
| Explicit extraction expression before effect installation | Extraction produces a valid transferable result with a specified absent-source behavior. | Makes removal visible outside the effect form, but introduces another operation and broadens the temporary-result contract. No such operation is selected without a concrete need. |

These are alternatives for discussion, not implemented grammar. A declaration of writable local optional storage is also missing: the ordinary declaration forms do not select optional-owner extraction syntax or its mutation contract. The design must settle that declaration together with references to its storage, presence tests and installation; a standalone ownership keyword would not resolve the rest.

### Required construction semantics

1. **Allocation capability:** H-197/H-198 permits construction and explicit-memory editing inside functions while excluding dynamic-variable access and procedure calls. The prototype can allocate payloads in an explicitly supplied region. Whole-region make still requires the construction and ownership type contracts below; there is no ordinary read-only snapshot baseline.
2. **Construction access:** the body needs access to the region while building it. It must not obtain that access by irreversibly installing the same owner into an ordinary user dynamic resource and later moving it back out; H-181 forbids that. The construction-local access syntax still needs design.
3. **Final regional placement:** create root and nested objects in their region, preserving sharing, cycles and self-relative references. No temporary regional object is copied from the stack into its final position.
4. **Completion:** decide what makes a construction result usable, including its typed root and required initialized fields. This draft selects neither a graph-wide validator nor an auxiliary runtime structure.
5. **Transfer and cleanup:** completion yields one responsibility. Successful effect installation takes it; abandonment or a supported construction-failure path must have a defined cleanup responsibility. How source failure/control flow is expressed remains a separate language choice.
6. **Scope order:** RHS evaluation precedes installation. A failed construction must not be treated as a successful new binding or as destruction of the previously visible resource. Owning-scope exit and removal of captured shadows remain distinct.
7. **Composition:** factories, branches and block results must preserve the source category on every reachable path. A branch returning a fresh region and another returning a dynamic reference cannot be unified into transferable ownership merely because both are spelled R.

## Practical acceptance and the next coherent slice

Three use cases keep the draft grounded:

- **Local effect resource:** construct it, install it, capture its value in a procedure, call that procedure under another resource binding, restore lookup, then release each region at its actual owning scope.
- **Resource prepared before installation:** retain one whole-region owner, edit its contents and transfer that responsibility into an effect binding. H-262 selects compiler-tracked consumption; H-263 preserves permitted views under the new local owner; H-264 supplies the first typed declaration/construction form. D-17 E1–E5 supplies the bounded failure and call/result contract; wider forms remain separate.
- **Regional packet:** create, edit, send or receive it under its own transfer protocol. This draft does not impose effect installation or its optional-source rule on that separate scenario.

D-17 E1–E5 supplies the coordinated local resource/construction/call/result/cleanup contract. Optional-source extraction remains separate and is not required for every named owner. Dynamic-source extraction and wrapper laundering reject. A factory returning an owned region is explicit in its signature; not every function result is a fresh owner.

The prototype supports typed ordinary owners through D-17 E4/E5: initialized make, root borrowing/editing, consuming function/procedure parameters/results, installation into non-extractable effects and scope cleanup. Fresh owner declarations use `owned region packet:Packet <- ...`; unqualified region parameters borrow. Compiler checks distinguish consumed source names from valid views under a surviving local owner. Full const/reference-assignment, owner reassignment, optional and mailbox outcomes remain outside this increment.

For this effect-owned subset, liveness follows the responsible resource lifetime. An explicit full scope or loop iteration closes its local acquisitions; ordinary conditional grouping introduces no independent lifetime. Captured and hidden enclosing resources remain protected under [D-17’s loop scoping rules](ExceptionControl.md#loop-scoping-rules). Module acquisitions remain live through process exit, and captured replay changes lookup without closing a holding. Reference joins still require one region, returned views cannot outlive their owner, and unscoped conditional effect rebinding is rejected. [Recorded lifetime evidence](../tests/cod/language/owner-liveness-validation.json) covers those checks and native capture/release and relocation probes. That report covers the earlier effect-owned subset. D-17 and the E1–E5 audit separately cover ordinary consuming owners and their normal/exceptional joins.

## Remaining syntax, semantics and idiom validation

The local construction/edit/call/transfer/cleanup lifecycle is implemented. Further work should serve concrete programs while keeping region ownership separate from allocation policy inside a region.

| Topic | Confirmed foundation | Remaining design and decisive check |
| --- | --- | --- |
| Pointer variables and parameters | H-271 fixes a ptr binding's owner at initialization; H-272 copies default bits arguments into callee-local targets. External borrowing requires owner generics. | H-273 confirms owner annotations; coordinate external const access, local temporary lifetimes and result provenance. |
| Assignment and transfer | `=` rebinds references within a fixed owner; `<-` transfers bits/ownership. Both associate rightward and expose destination views: `=` preserves proven access and `<-` is readonly; owning moves use separate steps. | H-270/H-271 fixes result views and owner identity; H-293 requires an external destination owner and preserves read-only aliases. Indexed transfer syntax and general qualifier composition remain separate. |
| Owner reassignment and optional outcomes | Fresh ordinary owners have a consumed state; local transfer preserves permitted views. | Occupied destinations, reinitialization and intrinsic owner-bearing optional/results need explicit contracts. They are not required to construct an ordinary region. |
| Mailbox handoff | Local send errors only, no delivery acknowledgment; failed send can return ownership. | Specify the intrinsic result/matching surface, receive ownership and revoked sender views. Validate one complete process-boundary path when implemented. |

The library-pool example below already demonstrates allocator policy in CoD. It requires no language-level free-list, per-object owner or generation registry. New mechanisms need a reproduced limitation or a confirmed business requirement.

### Native user-library example

[pool-interface.cod](../tests/cod/language/pool-interface.cod) defines the shared `Slot` UoM and nominal store interfaces. [reusable-pool.cod](../tests/cod/language/reusable-pool.cod) uses that identity for separate payload/link arrays and an initialized regional root containing a free-list head and references to those arrays. Functions read, initialize, acquire and release slots through borrowed Pool views; no dynamic binding is accessed inside those functions. The `create(region arena, capacity:u32)` overload allocates in a caller-supplied region and returns a borrowed Pool view. The `create(bytes:u64, capacity:u32) -> owned region :Pool` overload constructs its typed root with make, allocates the two arrays, initializes the free chain and returns ownership. The root is not published before initialization succeeds; partial construction retains one cleanup responsibility. The library has no host heap, per-object owner or new compiler intrinsic.

The capacity index is the library's exhaustion sentinel; an exhausted acquire leaves storage unchanged. A slot belongs to this pool only until its next release. Returning a still-acquired slot exactly once, and discarding references to its previous logical item, are explicit caller preconditions. The example does not claim double-return or stale-logical-item detection; no membership/generation array is allocated merely to anticipate those requirements. Language bounds checks remain effective for actual array access.

[region-pool.cod](../tests/cod/language/region-pool.cod) fills four slots, checks exhaustion, then performs 10,000 two-slot return/reacquire laps with unchanged region occupation. [Current native evidence](../tests/cod/language/owned-pool-validation.json) reports 144 occupied bytes for the typed-root image on this host, including region metadata. Four deliberately insufficient budgets fail at root reservation, first array header, second array header and second array payload; the harness observes each release exactly once. Selecting Pool through the adapter also brings its result-associated owning factory into the caller. The original caller-region overload remains available. [pool-algorithms.cod](../tests/cod/language/pool-algorithms.cod) defines replace/peek/locate once against the interfaces. [pool-adapter.cod](../tests/cod/language/pool-adapter.cod) imports the concrete Pool identity, declares ReusableSlotStore conformance and imports those helpers into the concrete implementation context before explicitly re-exporting them. Their bodies remain in the source algorithm module. The interface uses mandatory typed callable declarations without fallback bodies; their implementations are checked on demand. [Callable-model evidence](../tests/cod/language/callable-model-validation.json) covers the declaration contract, and [quantity/widening evidence](../tests/cod/language/quantity-widening-validation.json) includes the pool with representation-independent Slot quantities. A returned payload-cell view remains associated with the pool region; alias writes are observed, while scope escape and cross-region edges are rejected. The independent harness copies the used image, frees its original, follows the relocated arrays/free chain and reuses its two free slots without pointer repair or further allocation. The reuse after relocation is performed by the independent C++ checker; this is not evidence of resuming CoD code against a moved region. The native CoD reuse and image-layout checks together validate the intended library/storage boundary.

### Algorithms across existing regions

[Cross-pool native evidence](../tests/cod/language/cross-pool-validation.json) composes the existing Pool with the one-slot Single probe layout. Each resides in a separate effect-owned region and explicitly conforms to the shared store interface through its adapter. An integration module selects both implementations and imports one source algorithm:

```text
function copy_into(destination:SlotStore, to:Slot,
                   source:SlotStore, from:Slot) -> i64 {
  ref target = cell(destination, to);
  store(target, read(source, from));
  target
}
```

The algorithm's source module knows the interface and Slot identity. Its selected read/cell implementations dispatch on the two concrete arguments; neither pool needs the other's layout. The copied item is an i64 value, so no stored reference crosses the owner boundary. Returning target preserves the destination region association. Both directions run, and subsequent writes through the returned view affect the destination while leaving the other pool's payload unchanged. Occupation of both persistent regions remains constant.

A companion helper copies the value and returns the source cell instead. A procedure using a temporary destination may return that source view, which remains usable after the destination scope ends. Returning the temporary destination's cell is rejected as an escaping regional result. The two variants distinguish the result's actual owner from the fact that both owners participated in the call.

These are ordinary explicit-memory calls using existing primitives. They do not supply the proposed reference-assignment/transfer operators, const permissions, whole-region movement, graph conversion or mailbox lifecycle. Slot validity retains the existing library preconditions; no pool metadata, allocator policy or runtime borrow mechanism is added. The record preserves complete generated sources and actual results without expanding the permanent fixture suite.

### Imported factories and allocation provenance

[Adapted-factory evidence](../tests/cod/language/adapted-factory-validation.json) composes selected implementations with borrowed regional results. A source-private function allocates a Node in a supplied region and computes its value through score; an exported procedure supplies that region and a gain from dynamic bindings. The adapter imports the procedure and selects a more specific score implementation. The body, Node identity and original captures remain source-defined, even when the receiver declares an unrelated local Node.

The frozen procedure produces11 under captured gain10 and21 under current gain20; the imported procedure selects score7 and produces17/27. A captured-region result remains usable after an unrelated inner resource scope ends, supports later field writes and links to another allocation in that captured region. The hidden outer caller region retains its occupied size. A current inner-region result cannot escape; a stored edge from a captured-region object to a caller-region object also rejects, with both acquisition locations reported.

The record's K-382 supplement forwards those factory values through four modules to a router with a `proc(Metric) -> Output` parameter. The router explicitly installs the supplied value in a symbolic binding and invokes perform or behave. The selected factory context and its source captures survive forwarding; the returned reference retains the region actually used by that invocation. Under inner gain 40, current source/adapted calls produce 41/47, while captured adapted calls retain 17. The captured object remains writable after the inner region ends and can link to another object in its original region. Returning a current inner-region allocation through the same router is rejected at the enclosing scope. This exercises statically known procedure values and borrowed results, without adding direct calls on procedure parameters or runtime closures.

These results distinguish implementation selection from allocation ownership. Import adaptation does not allocate a replacement region, move a captured owner or change the source representation. This is placement within existing regions and borrowing of the resulting objects; whole-region factory results and typed make have separate D-17 evidence above.

## Resource types and extended construction

H-192 asks for the language-level meaning of holding a region before selecting its post-transfer policy. The design starts with actual allocation, editing, procedure results, effect installation and mailbox handoff. There is no proposed user operation named `consume`; transfer is a semantic property of an operation that needs the whole region. An application that merely edits a packet should not unexpectedly acquire responsibility for destroying it.

### Type vocabulary

The type categories below distinguish borrowed whole-region access from ownership responsibility under H-200. H-271/H-272 separately governs ptr declarations and parameter owners; the type category alone does not choose a parameter's target owner:

| Source type | What the program receives | Lifetime responsibility |
| --- | --- | --- |
| `Packet` declared by `data` | A typed object in regional storage, accessible through a valid region-associated reference. | No independently releasable object, stack construction, regional copy or move. |
| `Packet` in a variable/parameter/result | Borrowed access to a Packet object in a live region; it need not be the root. | No transfer/destruction authority; retain region provenance. The implemented prototype still uses `Packet`. |
| `region packet:Packet` | Borrowed access to the whole region with its designated Packet root. | No release/transfer responsibility and no lifetime extension. |
| `owned region packet:Packet` | Whole-region ownership tied to the holding declaration's scope. | One release-or-transfer responsibility; borrowing does not duplicate it. |

A `region packet:Packet` declaration denotes an intrinsic resource with the D-17 borrowed/owning call contract. It is not a library struct containing a pointer and destructor. It carries the root's nominal type identity; two unrelated root types do not become interchangeable because their layouts match. The physical image need not contain a new runtime owner object merely because the source type is explicit. Region storage and regional objects remain the same intrinsic lifetime unit.

H-200 selects the short unqualified spelling for borrowed access. An owning declaration uses `owned region packet:Packet`; the declaration always states region and ownership, while the root may be inferred. A single scope manages the region's lifetime at a time. If an authorized operation transfers ownership, its destination becomes responsible and the source cannot also release it. Returning ownership from a factory therefore cannot leave cleanup attached to the factory's expired scope.

In particular, `region symbol @state:Packet` can remain a direct resource type under H-181. Reading `@state` supplies access under its dynamic owner; it does not authorize removal. `root(@state)` may supply object access, while installation into a second owning binding or sending the resource remains invalid. This requires checking the expression's source permission as well as its declared type. A matching `region :Packet` annotation alone cannot convert a dynamic resource read into a transferable result. Making every named whole-region value have identical extraction permissions would contradict the confirmed dynamic-resource rule.

Functions operate on explicitly supplied regional data and may read or modify it without accessing dynamic context. Returned object references retain their region provenance; function return does not itself end a caller-owned region. H-197/H-198 supplies the mutable baseline, with the implementation boundary recorded below.

H-193 retains region-only user objects; H-209 uses `data` for concrete representations and `type` for abstractions. The prototype implements regional product data with positional `alloc` construction and borrowed views/results. It rejects representation bodies on type declarations, regional type modifiers and ordinary by-value data constructors. Composite results may use caller-provided regional storage; the layout/index example allocates one reusable result slot. Primitive bits and scalar UoMs retain scalar representations. No arbitrary user-defined record becomes a bits global, capture or stack temporary. Typed make initializes concrete product roots. Sum construction and implicit borrowed-field notation remain separate work.

### Reference provenance and escape checking

A Packet view must retain a compile-time association with its enclosing region. In analysis notation, `Packet@rho` means a reference whose access depends on region `rho`; this is not a selected source lifetime-parameter syntax or a stored region ID in every pointer.

- `root(packet)` inherits packet's region association. Field/index traversal and returned references preserve the association; following an intra-region reference stays in that same region.
- Ordinary aliases, branch results and demanded calls must retain enough provenance to check subsequent access. A join cannot discard region information merely because both alternatives have the same element type. Uncertain joins must be represented soundly or rejected.
- A reference may return from a function while the caller still owns a sufficiently long-lived region. Ending the call does not destroy that caller-owned region. A reference cannot escape the actual enclosing region lifetime or the sender's valid access after handoff.
- Captured references/resources retain their access dependencies. Capturing a value cannot extend ownership or make an expired reference usable through perform. Existing non-consuming captured shadows preserve still-live owners.
- WPO supplies demanded call paths, not advance knowledge of every runtime branch or allocation instance. Provenance is a symbolic relationship across executions; two allocations at the same source site are not automatically one region instance.

The compiler already carries a `region_id` for the supported reference subset and rejects cross-region stored references and some escaping results. That is limited implementation evidence, not a complete proof for future branch-returned owners, sums or mailbox transfer. These checks remain at region granularity and do not establish a pool slot's logical-item identity after reuse.

### Make allocation expression

H-264 selects the construction and initialization contract with allocation parameters separate from in-place root initialization and H-200's explicit ownership qualifier:

```text
data Packet { sequence:u64; }
{
  owned region packet:Packet <-
    make Packet(capacity = 65536u64) { sequence <- 7u64 };
  ref view = root(packet);
  setfield(view, sequence, 8u64);
  # Ordinary scope exit releases the still-held region.
};
```

H-264 confirms the named capacity argument and in-place root initializer; H-307 supplies the owning region declaration shown here. D-17 E5 implements this bounded construction. Typed root access must preserve the same region provenance as the existing object-access operations. `make Packet` chooses the root object type; the result carries whole-region ownership, while unqualified `region :Packet` supplies borrowed access. The initializer is an in-place root initializer in the newly allocated region, not evaluation of a stack Packet followed by copying it. `root` only obtains an object reference and transfers no ownership.

The parentheses supply region-allocation inputs, with capacity required. The braces initialize the root's fields. A root can therefore have its own field named `capacity` without colliding with the allocator parameter. Capacity can be a runtime expression; `make` is not compile-time enumeration of region instances. The suggested first interpretation is usable content capacity for the root and subsequent regional allocations, with system-header overhead accounted separately. Exact capacity units, rounding, minimum size and error behavior remain design work; this draft does not establish a new allocator ABI or automatic growth policy.

Useful semantic boundaries for the proposed expression:

- `make` allocates and initializes a whole region. Procedures/module execution can perform such operations; H-197 permits them in functions as well. Allocation does not by itself require reading a user dynamic variable.
- Root initialization and any supported nested allocation happen in the new region. Until successful completion, no usable owning result is exposed. A supported failed construction path releases its unfinished region and leaves a preceding dynamic binding unchanged.
- Its result is the new whole-region resource. Binding that result must not make a second copy of its lifetime responsibility.
- `alloc` remains the separate operation for object placement inside an existing region. The prototype accepts `alloc @arena ...` in a procedure context and `alloc arena ...` through a named lexical borrow, including inside functions. Neither placement form creates or transfers the enclosing region. Internal slot/free-list policy remains user code.
- Complex graph initialization must eventually define how initializer code addresses the new region, how root constructor procedures participate, and how errors leave it. These questions do not justify adding an artificial dynamic owner that must later be extracted. The simple field initializer does not decide a general constructor-body ABI.

H-196 requires the whole-region wrapper to remain intrinsic. The `region r:Root` declaration exposes the user-defined root identity without making the owning wrapper user-defined. A possible source alias would have to preserve the intrinsic type and permissions; it cannot be a user-defined nominal product/sum with an owner field. General type aliases and additional construction forms remain open; H-264 supplies the first typed owning declaration and make spelling.

### Actual operations and transfer boundaries

For editing, a procedure can accept an existing object reference without owning the region:

```text
proc set_sequence(packet:Packet, value:const u64) -> void {
  setfield(packet, sequence, value);
  {}
}
```

A factory would instead return the newly made resource:

```text
proc make_packet(sequence:const u64, bytes:const u64) -> owned region :Packet {
  make Packet(capacity = bytes) { sequence <- sequence }
}
```

Under the proposed result rule, returning that expression transfers the new region's lifetime responsibility to the caller. It copies no Packet and needs no `consume` primitive. A procedure intentionally accepting and forwarding an entire region must have a whole-region parameter/result contract; this is distinct from the reference-taking editing procedure above. Owning result/parameter syntax follows the current direction; E4/E5 fixes the bounded construction and source-consumption rules; broader forms remain pending.

| Real operation | Required semantic distinction |
| --- | --- |
| Obtain the root, read or edit fields | Access to contents; the existing region responsibility stays with its holder. |
| Return a newly made region | Pass that responsibility from the construction/invocation to the caller; no release of the returned region at callee exit. |
| Install `effect @state <- make Packet(...) { ... }` | Pass responsibility into the new dynamic owning scope. The previous binding remains alive but hidden. |
| Send an ordinarily held packet | Pass the whole packet into the local transport attempt; local failure returns its responsibility through the result. Local success provides no delivery acknowledgment. |
| Exit a scope still holding a region | Release the entire region once. Individual objects and returned pool slots have no intrinsic destructor. |

The mailbox result still needs explicit local-error and returned-region handling. The future design notation `Accepted | LocalError { cause; owned region packet:Packet }` expresses that responsibility; it selects no sum grammar, wire protocol or transport API. Root access does not make a packet sendable if its owner is a non-extractable dynamic resource.

Because user-defined type objects are regional and H-196 reserves owner-bearing types for the language, this notation cannot introduce a user-defined SendResult containing a region owner. Nor does it authorize arbitrary owning-region fields inside a payload region. An admitted resource outcome/optional form must be intrinsic; its alternatives can carry primitive error information and the returned region handle. Its exact type/form remains open. The necessary rule is that the result preserves the region's single responsibility while respecting the distinction between intrinsic resource values and user-defined region-internal data; a generic sum label alone does not settle storage or ownership.

### Function and procedure parameter contracts

H-194/H-197 requires argument/result semantics, readable types and escape checking to agree. Callable kind controls access to dynamic context; it does not independently decide whether passing a region borrows or transfers it. The owning/unqualified intrinsic region types below make that distinction in signatures for both functions and procedures.

An object parameter or interior reference carries the enclosing region association. Memory operations inside nested functions can change reachable contents, so there is no automatic read-only snapshot boundary. Returning a reference into a still-live caller region is valid; returning a reference into a region released at callee exit is not. A whole-region owning result would transfer cleanup responsibility rather than copy a regional object. User-defined compound values remain regional even inside function signatures; their construction must use valid regional storage.

Necessary call/escape checks are region-grained:

1. Actual argument provenance survives parameters, field traversal and returned references. Same-root-type arguments from distinct region instances are not conflated.
2. Non-owning access does not transfer the caller's cleanup responsibility. Its callee cannot turn access into an owning result solely because the root type matches.
3. A handoff changes the source's availability according to the selected owning-source contract. Returning ownership in a result supplies an explicit new source of access; the handling of earlier aliases remains part of that contract.
4. Captured resources/references retain the same lifetime obligations. A procedure value neither extends a captured region's life nor permits later access after handoff.
5. Argument evaluation must not hide a lifetime change: if a later argument hands off/releases a region used by an earlier argument, invocation still needs valid provenance and permissions. Duplicate transfers and unsupported alias correlations are rejected rather than repaired by implicit null checks.

WPO supplies demanded call paths, not complete runtime values. The implementation must infer sound relationships or reject unsupported cases. No source lifetime-parameter language, per-object owner, runtime reference counter or per-pointer generation field is required by this initial whole-region distinction.

### Function rule: no dynamic context, with explicit memory operations

H-197/H-198 establishes the function rule for implementation and validation: functions exclude dynamic effect-variable access, while explicit region construction/editing and explicit-argument mailbox send are permitted. This replaces the automatic read-only snapshot guarantee from H-124/H-127/H-129. H-198 establishes this as the ordinary-function baseline and removes the read-only fallback plan. A separate const-function design may be considered when a concrete business need requires read-only semantics; no such mode is implemented.

This rule uses region-grained ownership and compiler-checked lifetimes. A function can obtain regions through permitted explicit parameters, create a local region with make, edit accessible contents, and release still-owned local regions at scope exit. It must not acquire access to `@state` indirectly through a helper, captured dynamic context or an intrinsic that reads a dynamic binding. The call boundary retains function-to-function calls and procedure-to-function/procedure calls; dynamic procedure calls remain unavailable from functions. Selected memory intrinsic overloads need the same rule as their bodies, rather than a blanket assumption that store or allocation implies dynamic access.

H-198 uses Haskell ST as the model for ordinary memory operations: scoped mutable state with statically checked escape, supplied by the language rather than explicit monadic syntax. Haskell's [ST documentation](https://hackage.haskell.org/package/base/docs/Control-Monad-ST.html) describes destructive updates indexed by a state parameter; runST encapsulates that state. CoD uses its region provenance/lifetime rules for the corresponding concern. This analogy does not import a rank-2 runST API or revoke H-197's explicit-argument mailbox send. CoD does not promise that every function is observationally pure.

The function keyword guarantees independence from dynamic context, not mathematical referential transparency. A function that increments an input region changes what its caller observes; repeated calls need not return the same result. The conceptual state relation is `(arguments, reachable region state) -> (result, updated region state)`. The source need not expose a state tuple or runtime effect object to make that distinction clear.

| Concern | Consequence of the adopted function rule |
| --- | --- |
| Local allocation and scope exit | make-created regions are locally owned until legitimately returned/transferred. Still-owned regions release once at lexical exit; forwarded owners are excluded from that cleanup. Allocation/failure semantics still need their own explicit contract. |
| Caller-owned input region | An access parameter may allow content edits without transferring the owner's cleanup responsibility. Exiting the function must not free the caller's region. Returning an interior reference is allowed only while its caller-region provenance remains valid. |
| Local reference escape | Returning only `T` into a region destroyed at function exit is rejected. Return the intrinsic whole-region resource instead, or allocate the result inside a supplied live region. No hidden lifetime extension or nullable dangling-reference state. |
| Returned whole region | Under an owning-result contract, `function make_packet(...) -> owned region :Packet { make Packet(...) { ... } }` returns the intrinsic owner without copying the root. Named return-source rules remain the same open issue as for procedures. |
| Snapshot semantics | Nested functions may now change input-reachable data, so the earlier automatic read-only snapshot guarantee no longer holds. Keeping read-only inputs would require a separately specified parameter restriction; merely calling the code a function cannot supply it. |
| Sequencing and optimization | Preserve source evaluation order around observable memory effects. Do not infer LLVM readonly/readnone, memoization, repeated-call elimination or reorderability from the function keyword. Existing inlining can expose actual loads/stores to LLVM; no new optimizer is required. |
| Errors | Lexical cleanup releases owned storage; it does not undo earlier edits to a caller region. Failed effect initialization preserves the prior binding/owner, but does not promise rollback of graph edits made while evaluating its RHS. |
| Intrinsics with external consequences | H-197 explicitly permits mailbox send with entirely explicit arguments in a function. It still needs the mailbox, packet ownership and result contracts; permission is not evidence of a native transport implementation. |

Same-region pointer rules continue to reject storing a reference to a short-lived local region inside another region's graph. Allowing edits inside a function does not grant cross-region pointer storage, user-defined owner containers, per-object destruction or native heap allocation for user objects. Object-pool algorithms remain ordinary region-internal code.

The compiler separates function-context checks from memory operations. The implemented increment permits store, setfield and function overloads that mutate explicit regional arguments; it preserves rejection of symbolic reads/writes, perform/behave and procedure calls. Bare region locals/parameters/results borrow an already held region. Functions can allocate data and arrays into that explicit input using alloc followed by its lexical name. Returned regional views retain the same owner. Typed make and explicit owning results additionally create and transfer whole-region responsibility under D-17 E4/E5. Mailbox transport remains unavailable. Concrete data products use the regional path, and returned views remain dependent on their existing owners. Field writes accept an explicit record reference or an already dereferenced borrowed record view; both use the same checked native address and field encoding. This does not add per-object allocation, copy/move, lifetime extension or a new pointer representation.

### Whole-region access and ownership: current native subset

H-271/H-272 governs the target `ptr` variable and parameter model above. The source forms in this subsection describe the implemented E4/E5 subset; their bare-parameter borrowing default does not override the new requirement for explicit external-owner parameters. Resource ownership and cleanup are preserved during that source-interface migration.

H-200 assigns unqualified region declarations to borrowed whole-region access and reserves the `owned` qualifier for lifetime responsibility. No separate `borrowed` keyword or `region_ref` type constructor is planned. Borrowed access may pass through many function/procedure scopes while exactly one owning scope remains responsible. Borrowing does not retain, count or extend the region's lifetime, and it does not grant exclusive access.

The native prototype supports raw `region r` and rooted `region r:Root` borrows. A local `region view = @arena`, a parameter `region arena` or a result `-> region` carries access to the original owner; it creates no cleanup entry. Forwarding through functions, procedures, branches and same-owner folds retains that provenance. Installing a forwarded borrow with effect remains rejected, and a local resource cannot escape via a borrowed return. The [borrowed-region evidence](../tests/cod/language/region-borrow-validation.json) covers these cases, library allocation and a 96-module forwarding chain. The existing scalar-by-value subset is not evidence for H-201's unqualified scalar-slot borrowing.

The following signatures illustrate this direction. `Packet` as the object-view spelling follows the user's proposal; the full grammar and lowering are not implemented:

```text
function create(bytes:const u64) -> owned region :Packet
function edit(region packet:Packet, sequence:const u64) -> void
function entry(region packet:Packet) -> Packet
function forward(owned region packet:Packet) -> owned region :Packet
```

The same types carry the same storage contract in a procedure. A function excludes dynamic-context lookup, rather than imposing read-only memory. An unqualified region parameter preserves the caller's owner and may edit valid contents. It cannot be sent, installed as a new owning effect, or converted to an owning result. An owning parameter/result transfers responsibility when supplied by an authorized owning source; the declared owned region mode makes that transfer explicit. H-262 supplies the named-source consumed state; H-263 preserves permitted views under a live accessible local owner.

An `owned region` declaration may infer its root from make; a borrowed region declaration never acquires cleanup responsibility by inference. A region parameter explicitly declares whole-resource borrowing or consumption. Interior object parameters separately require external-owner annotations. An owned parameter consumes at callee entry after successful argument evaluation/conversion. UoM conversions cannot silently substitute a new storage owner.

A returned Packet or region view retains its actual storage provenance. A borrowed result from a caller-owned live region is admissible; a view into a local region freed at return is not. A borrow from an owning parameter is invalid after the callee releases that owner. Local owner forwarding and external send must be distinguished: access follows actual region lifetime/disposition, not only a variable name. Mere inlining cannot make an escaping local valid.

The declaration of a dynamic resource and reads of that resource serve different roles. `effect @state <- rhs` installs ownership in its dynamic scope under the confirmed no-extraction rule; reading `@state` only supplies access. Borrowed-by-default naming fits this boundary. Region symbols use the region-first declaration and every effect returns void; a matching type never permits extracting the owner. Captured perform shadows lookup without consuming either the captured or hidden resource.

### Object views without an explicit ref wrapper

The proposed `p:Packet` names access to an existing regional object. It does not construct a Packet on the stack, copy/move the regional object, or create an independently owned Packet. The compiler must retain the same enclosing-region association currently tracked for `Packet`. A local native address or register used to access that object is an execution artifact, not stack allocation of the object itself.

Source-level elision must not erase distinctions required by stored graphs. A stored reference still needs relocation-safe encoding; an arbitrary native pointer cannot become a payload field merely because source says Packet. Typed nullability remains a separate explicit optionality contract; the elision does not select implicit nullable Packet or a hidden NPE.

H-203 fixes the permitted stored edge independently of physical layout: a field storing a reference and its target must share one allocation owner. The lifetime carrier is a region instance or lexical-scope instance. H-310/H-311 distinguishes val/unmodified bits payload fields from ref/ptr stored edges, including edges to bits. Assignment must respect each field's declared storage role.

### Local bits storage and initialization

H-292 confirms that a fresh bits result used to initialize a ptr is materialized in the enclosing lexical scope. The explicit source form is `make local T { expression }`. For an expression of type i64, the following are equivalent:

```text
ptr n = 3 + b;
ptr n = make local i64 { 3 + b };
```

These are alternative declarations, not two declarations of n in one scope. The implicit form infers the computed result type; it does not force every scalar to i64 or select a new numeric conversion. The initializer is evaluated once. The resulting bits storage belongs to the scope enclosing make local, not merely to the initializer's braces. No source-visible owning-bits wrapper or native heap allocation is required.

Initializing from existing storage remains borrowing:

```text
ptr alias = existing;
```

The alias retains that storage's actual owner and access permissions; it does not copy the bits simply because their type is scalar. A copied const bits function result can be materialized in the caller's scope. Returning an inner-scope reference does not materialize a copy or extend that target's lifetime. Default parameter copies retain the separately confirmed H-272 callee-local rule.

Only bits storage may be constructed locally. A non-bits data object still requires regional allocation; local construction cannot stack-copy a regional graph or duplicate region ownership. LLVM may keep unobservable storage in registers while preserving source aliasing, evaluation and lifetime semantics.

#### Individual value initializers and declaration-group blocks

H-296 distinguishes two source positions. A block attached directly to make local supplies the initial bits value of that one allocation. A block introduced after the final comma belongs to the entire ptr declaration group; it executes after the individual declarations, and its result is ignored.

```text
# One allocation initialized from the block's bits result.
ptr n = make local typeof(b) { b };

# One declaration followed by an explicit group initialization block.
ptr n = make local typeof(b), { n <- b };

# A group may combine individual value initialization and explicit writes.
ptr a = make local i64 { 1 },
    b = make local i64,
    { b <- a + 1 };
```

The comma is semantic punctuation: it separates the group block from an individual initializer. There is no heuristic based on the last expression's type to decide which role a block plays. The individual value block supplies bits; it does not implicitly denote a method body or discard that result. The group block runs ordinary expressions over the declared variables, can initialize several targets, and does not implicitly assign its final value to one of them.

Individual declarations evaluate in source order, each RHS once; the group block follows all of them. An initializer resolves its references in the preceding lexical environment. A newly declared name is available to subsequent declarations and to the final group block. Consequently, an individual initializer in a nested shadowing declaration can read the outer name, while the final group block uses the newly declared name. The shorthand's source expression must retain that resolution when lowered; the compiler must not accidentally redirect an outer reference to the new uninitialized destination.

The allocated bits belong to the lexical scope enclosing the declaration, not to the final initialization block. Ordinary locals created inside that block retain the block's own lifetime and cannot escape through these pointers. The declarations neither change fixed-owner rebinding nor permit stored cross-owner edges. Initializer failure follows normal exception unwinding; it does not roll back unrelated earlier writes or effects.

#### Transfer shorthand and compile-time typeof

The H-294 transfer initializer uses the comma-separated group form:

```text
ptr n <- b;
# Equivalent allocation and transfer:
ptr n = make local typeof(b), { n <- b };
```

This constructs independent local bits storage, then applies the ordinary transfer contract. In contrast, `ptr n = b` borrows existing storage. A bits copy from read-only access may create independent writable storage; it does not make an alias of the original target writable. Non-bits data still requires an explicitly supplied regional allocation destination and cannot use make local as a graph-copy fallback.

H-292's arithmetic shorthand keeps its individual value initializer:

```text
ptr n = 3 + b;
# For an i64 expression result:
ptr n = make local i64 { 3 + b };
```

H-295 makes typeof a compile-time operation with no runtime execution. `typeof(b)` does not evaluate b, allocate its value or invoke its procedures. Runtime initialization evaluates the source once; this supplies no RTTI or general runtime reflection mechanism. A known type can be queried even when its storage has not yet been initialized: the query does not read that payload. Broader typeof applicability and type-query forms remain separate from this local-initializer contract.

#### Definite initialization may extend beyond the declaration

H-297 permits a group to finish with allocated but uninitialized slots:

```text
ptr a = make local i64,
    b = make local i64,
    { a <- 1 };

# Reading b here is a compile error.
b <- 2;
# Reading b now is valid.
```

Storage existence and initialized contents are separate compiler facts. The pointer can identify a valid initialization destination without authorizing a read of its old contents. There is no implicit zero initialization or runtime uninitialized-value exception. A read requires a successful prior initialization on every path reaching it. If an overload reads the destination before initializing it, that read must reject; choosing an overload does not bypass the rule.

Initialization follows actual storage identity through aliases and explicit-owner calls, not merely the spelling of a variable. Initializing through one writable alias permits subsequent reads through another alias of that storage. Scope exit, invalid owner lifetime or a read-only view retains its independent constraints. Any initializer read remains subject to definite initialization even if its eventual result is unused.

Acceptance must distinguish the two block positions, preserve source order and shadowing, permit deferred initialization and reject premature reads across branches/calls, while preserving alias identity and enclosing-scope lifetime. These are confirmed source obligations covered by the bounded implementation below; no new per-value runtime initialization flags or allocator protocol is selected.

Under H-272, default bits parameters target independent callee-local storage. H-201's const bits results still supply independent values; H-270's assignment-family results instead retain destination views. An explicit external-owner parameter is needed to retain a caller's slot. H-273 confirms the owner grammar above; external read-only bits-access spelling remains separate. Ownership transfer through owned region is a separate resource case; no bits argument rule grants a copyable owner or regional object.

Bits arguments must be obtained during that argument's evaluation/conversion, before evaluating the next argument under the established sequencing rule. A later argument's mutation cannot retroactively change an earlier copied value. An explicitly borrowed external argument preserves access to its owner's slot; all supplied views still need valid storage at the call boundary. This distinguishes a callee-local bits copy from a read-only view of external storage.

The resulting obligations are:

| Case | Required semantic distinction |
| --- | --- |
| A bits parameter and another alias may reach the caller's slot | A default parameter has its own callee-local copy. An explicit external-owner parameter retains the original slot and observes permitted intervening writes. |
| A literal or `a + b` initializes a ptr with a fresh scalar | Enclosing lexical-scope bits storage, equivalent to make local with the inferred type; no arbitrary extension of an existing borrow. |
| A function returns a scalar | An unqualified i64 result borrows; a const i64 result supplies independent bits. A callee-local borrow cannot survive return merely because its element type is i64. |

Const bits results can be lowered through the native value ABI without preserving callee stack addresses. H-292 gives fresh scalar ptr initializers enclosing-scope storage; existing borrowed results retain their source owner and cannot escape it. An owned i64 is not required merely to return a computed scalar.

For non-bits objects, H-202 defines `const Packet` as read-only borrowed access. It does not copy/freeze the object or promise unchanged contents; a writable alias may change what later reads observe. This preserves the region lifetime dependency and narrows H-201's independent-value rule to bits. `const owned region :RT` qualifier composition remains a separate question; no owner may be duplicated through a const view.

Semantic borrowing also does not mandate a physical pointer or stack slot for every scalar. Once source aliasing and lifetime behavior are fixed, LLVM can keep unobservable storage in registers and eliminate loads/stores as usual. This is ordinary lowering, not permission to erase observable aliases or rescue an invalid escaping reference through WPO.

Acceptance must distinguish caller-visible external borrows from default callee-local bits parameters and ordinary const bits results, reject local-storage escapes and non-bits make-local construction, preserve borrowed caller-object returns, and keep compact regional bits fields/arrays relocation-safe. These are design acceptance obligations, not newly implemented or tested features. Existing K-311 native evidence covers the earlier explicit-reference mutable-function subset only.

### Const access and allocation-owner validity

| Source form | Access and lifetime |
| --- | --- |
| ptr variable initialized from existing T storage | Borrow with the initializer-fixed owner and permitted access; no lifetime extension. |
| Default bits parameter | Pointer to an independent callee-local bits copy under H-272. |
| Explicit external-owner parameter | Borrow external storage under its declared owner constraint; H-273 confirms the owner grammar, while const annotation remains separate. |
| Ordinary const bits result | Independent evaluated bits value under H-201; H-270 separately specifies assignment destination views. |
| const non-bits T | Read-only borrow under H-202; writable aliases may still change the target. |
| owned region :RT | One whole-region release-or-transfer responsibility. |

A holder of const non-bits access cannot modify the target through that access; an independently writable alias may still modify it. Reading inline fields does not grant permission to overwrite those fields. H-293 requires a new alias of a read-only view to remain read-only. Passing it through another variable, a field or an assignment result cannot restore write access. An operation requiring a writable view must reject a read-only one; independent copies of bits are different storage. See the assignment discussion below. No whole-graph freezing or non-aliasing is assumed. LLVM must not infer invariant memory or eliminate a reload across a possible aliasing write solely from const. The qualifier does not itself settle concurrency or race freedom.

A field's storage and a reference's target have distinct provenance facts. For an inline field, storage belongs to its containing object. For a stored reference, the target belongs to its actual allocation owner. H-203 requires exact equality of those owners, not merely overlapping or sufficiently long lifetimes:

```text
owner(containing object) == owner(referenced target)
```

Two regions of one root type are still different owners. A region owner and a lexical owner differ; two lexical activations differ even if produced by one source block. Moving a region's ownership handle does not change that region instance or rewrite its internal edges. Same-owner checking can follow compiler provenance; it does not prescribe a new runtime owner ID in every reference.

Ordinary borrowed bindings and parameters are access paths, not newly allocated payload objects. Passing a Packet through nested calls retains its region owner; it does not create a cross-owner stored edge simply because the callee has its own scope. The existing dynamic-resource installation/capture contract is also distinct from a user object field. This distinction grants no permission to persist captured handles or native stack addresses inside a regional payload.

Stored cross-owner references are compilation errors even while both owners are live. Same-owner references still require valid access at every use: a lexical view cannot escape the end of its source scope, and sender access cannot survive a handoff merely because the receiving region remains allocated elsewhere. WPO must preserve provenance through parameters, returns, branches and stores; unprovable validity is rejected rather than converted into a hidden nullable reference. It need not accept every mathematically safe program or know every runtime branch in advance.

The rule reinforces relocation-safe regional edges and does not admit non-bits stack objects. Independent bits copied into another owner have destination-owned storage and no retained source-reference dependency.

### Reference assignment and the ownership/bits transfer operator

H-204/H-205 names `<-` the ownership/bits transfer operator. H-270 confirms that ordinary `lhs = rhs` always performs pointer/reference assignment. It never falls back to payload copying, including when the source is bits or the owners differ. Behavior depends on semantic type, not native representation size:

| Operation | Meaning | Source and destination obligations |
| --- | --- | --- |
| Rebind a ptr name with = | Make that binding refer to the RHS target; copy no object payload. | H-271 fixes the owner at initialization: only same-owner targets are permitted. Retain target type/access checks; no lifetime extension. |
| Store a reference in an object with = | Change the stored edge. | Containing object and target must have the same owner; the edge slot must be writable and have a reference representation. |
| Copy bits with <- | Evaluate typed source bits and write them into existing destination storage. | Destination must be writable. Cross-owner copies are allowed; source remains usable and later source writes do not alter the copied bits. |
| Transfer owned region with <- | Pass the region's single cleanup responsibility to an admitted owning destination. | Source loses ownership availability; no graph copy, duplicated owner or extraction from a borrowed/dynamic resource. |
| Apply <- to a pointer-bearing object | No default graph transfer; a selected user overload may reconstruct/convert its graph. | Use permitted source/destination storage interfaces and form only same-owner edges. Existing regional objects do not gain intrinsic copy/move semantics. |

Existing-destination source forms (stored reference-field rebinding remains outside the bounded ptr implementation):

```text
alias = packet;                 # Borrowed name refers to the same Packet.
packet.next = next_packet;      # Stored edge: require the same region owner.
packet.count <- local_count;    # Copy bits across storage owners.
local_count <- local_count + 1; # Read and update the local bits slot.
```

When an assignment is rejected because the reference would cross owners, the diagnostic must identify the incompatible owners and explain that `=` assigns a reference: “You may want `<-` for bits transfer.” This is guidance, not an automatic rewrite. A pointer-bearing graph still requires a suitable user-defined transfer; replacing `=` with `<-` does not make an arbitrary graph copy valid.

Assignment reads its source without modifying it under H-223/H-224. H-293 prevents an alias from obtaining write permission through a read-only source view; independently supplied writable access retains its own permission. The general overload-body protocol remains separate from ordinary ptr rebinding. Writes through an explicitly const destination are forbidden. A reference or region handle is not copyable bits because its native representation happens to be a machine word. Wrappers, scalar casts and byte-copy helpers must not bypass the semantic category or owner-bearing-container restriction.

A val/unmodified scalar field and a dense numeric array slot use <- for their directly stored bits; = cannot manufacture a pointer slot or change their layout. A ref/ptr scalar field uses <- to write its target bits and = to rebind its stored edge. The indexed payload counterpart is `([]<-)(container, index, value)` under H-307, alongside the distinct `([]=)` family. It supports logical writes with void results as well as destination views; see the logical indexed transfer contract below. Current `(=)`/`([]=)` methods can implement arbitrary memory edits and must be reviewed before reusing their evidence for the new distinction. Typed overloads remain subject to const, owner and lifetime checks; method bodies cannot opt out of intrinsic ownership rules.

Symbolic bindings are separate from ordinary pointer destinations: `effect @x <- rhs` transfers bits/ownership, while `effect @x = rhs` rebinds a regional object reference or procedure. Bits symbols reject the `=` form. Both forms restore the preceding binding on scope exit. Its bits case installs a value; its owned-region case consumes an admitted owner into the dynamic scope. A previous binding/owner remains hidden and alive, then becomes reachable on scope exit. Ordinary bits stores are not rolled back on scope exit. Captured perform is a non-consuming replay path. Bare symbolic binding assignment remains forbidden. The proposed `@object <- rhs` edits its regional referent through an overload; it provides no second way to rebind @x.

Procedure-valued effects use `effect @p = procedure_value`, or inline `effect proc`. They install the selected procedure and captured values without duplicating captured region ownership. They do not classify procedure descriptors as transferable owners or permit effect-arrow procedure binding. Typed effect installation, public owner transfer and captured replay retain separate checks.

The concept is coherent, but these details must be chosen before lowering ordinary owner assignment:

- **Occupied destination and self-transfer:** replacing a live owner needs a defined cleanup/borrow policy. For the first increment, rejecting occupied or self-owning destinations is a candidate; automatic release/replacement is another policy. Effect shadowing cannot be borrowed as ordinary replacement semantics.
- **Expression result:** H-206 selects a const reference to the destination value, not another owner. H-207 requires explicit steps for ownership moves; the following sections specify the chain boundary. H-288 makes every scoped-effect result void; it supplies no assignment-chain view.
- **Evaluation and failure:** select/validate the destination and evaluate the source once, preserve the established argument/conversion order, and ensure failure cannot lose or duplicate responsibility. The precise ownership-consumption point and local-error result must agree with the operation. Send can return ownership on local failure without reviving the consumed source implicitly.
- **Source post-state and branches:** H-262 makes a transferred ordinary source unavailable at compile time, without an implicit NPE. Optional/result storage has separately specified presence semantics. Cross-process send also revokes sender views; local forwarding can preserve views only while the actual region remains accessible under its new owner.

These are targeted language decisions, not reasons to add a second allocator, a runtime borrow registry or per-object ownership. Bits transfer, owner transfer and reference assignment need distinct checked paths even if they share parts of the native lowering.

### Reference and bits swap operators

`a <=> b` exchanges two ordinary pointer bindings' current targets. `a <-> b` exchanges the bits stored at two writable target locations. Both evaluate operands once, left-to-right, and yield void; they cannot be chained. Operand effects occur during evaluation and are not rolled back if the second operand throws. The swap itself starts only after both operands are available and checked. For reference swap, evaluating an operand selects its binding; the current two targets are obtained after operand evaluation. For bits swap, evaluating an operand selects its storage location; both stored values are read before either is written.

| Operation | Builtin requirements | What is preserved |
| --- | --- | --- |
| `<=>` | Two rebindable ordinary pointer bindings, identical types, the same fixed owner, and access valid in both directions. | Target contents, owner/lifetime and each binding's access contract. |
| `<->` | Two writable, definitely initialized storage locations of the same bits type; owners may differ. | Each binding's target and owner. |

Self-swap is valid and leaves the selected binding or bits unchanged. A non-bits graph has no builtin bits-swap implementation. Numerical widening and UoM conversion cannot manufacture a destination; builtin swaps do not insert them. Future fraction swaps must retain D-16's fixed bits and exact type requirements; fractions remain unimplemented.

Parenthesized `(<=>)` and `(<->)` are overloadable compiler operation names, with both function and procedure methods allowed. Each method has exactly two non-variadic parameters declaring explicit external owners and yields void. Both are destinations, so their const qualifiers match exactly and no automatic argument-value conversion is inserted. Reference-swap parameters denote the actual caller bindings; bits-swap parameters denote their target storage. Function overloads remain unable to access dynamic effects, and procedure overloads use current bindings. `intrinsic (<=>)(a, b)` and `intrinsic (<->)(a, b)` delegate to the checked builtins. The existing assignment-result destination safeguard remains specific to assignment operations; a swap returns no destination view.

### Explicit checked builtin invocation

`intrinsic (operation)(arguments...)` selects the compiler's existing builtin implementation without user-overload lookup. For example:

```text
function (=)(lhs:i64 in O, rhs:i64 in O) -> i64 in O where {owner O} {
  intrinsic (=)(lhs, rhs);
  lhs
}
```

The ordinary call can select this method; its explicit intrinsic call performs the checked builtin rebind rather than recursively selecting the same method. Argument evaluation remains ordered and occurs once. Types, fixed owners, access permissions, lifetime and definite initialization are checked as usual. The builtin result retains its normal contract: reference `=` preserves destination access, while bits `<-` yields a readonly destination view. A writable-lhs/const-rhs method can delegate after independently obtaining a writable target; it cannot delegate an invalid readonly-to-writable rebind.

The form also applies to existing eager arithmetic and named intrinsics, such as `intrinsic (+)(a, b)` or `intrinsic (len)(values)`. It does not create a callable identity, runtime dispatch facility, unchecked pointer operation or conversion method. A missing builtin implementation rejects rather than falling back to a user method. In particular there is no builtin graph-transfer `<-` or indexed `([]=)` contract. Existing numerical widening and UoM identity checks remain; D-16 fraction execution is deferred. Explicit builtin selection does not make dynamic-context access legal inside a function.

### Const-qualified reference-assignment methods

Parameter `const` is part of a method signature under H-302, while the underlying data/type identity is unchanged. Matching retains the actual owner and concrete data type. Qualifier information is compile-time metadata, not RTTI or a runtime wrapper. Return qualifiers describe the selected method's result and do not choose an overload.

For ordinary reference rebinding of compatible types within one owner:

| Destination access | Source access | Builtin behavior | Result access |
| --- | --- | --- | --- |
| Writable | Writable | Rebind | Writable |
| Writable | Read-only | Reject | — |
| Read-only | Writable | Rebind with restricted access | Read-only |
| Read-only | Read-only | Rebind | Read-only |

An applicable user method is selected before the builtin path. A `(writable lhs, const rhs)` method may read its source, obtain a target through another valid writable reference, and rebind the destination to that target. The qualifier pair does not cause rejection before method lookup. Its body cannot turn the readonly source itself into writable access, change the destination owner, or escape the target lifetime. A failing selected body produces a compile error; the compiler does not retry a different method or the builtin operation.

Exact type/qualifier matches take precedence when they uniquely dominate. Otherwise a writable input may be viewed as read-only without copying storage; readonly-to-writable borrowed adaptation is forbidden. The destination qualifier must match exactly and is never weakened to find a method. `=` does not insert numerical widening or UoM conversion: those produce values rather than aliases of the original object. Abstract-interface admission can preserve the concrete target. Default bits parameters keep their separately defined callee-local copy semantics; a private writable copy of readonly input is not writable borrowing of that input.

Specificity compares applicable type, owner and qualifier constraints together. A candidate must be no worse on every relevant dimension and better on at least one to dominate another. A concrete readonly method and a broader writable method can therefore remain ambiguous for a writable concrete argument. Declaration order and return types do not break ties. Missing applicable user methods leave the checked builtin operation available; its owner/access/type rules still apply.

`a = b = c` associates as `a = (b = c)`. A writable builtin inner result can feed the outer writable assignment. An overload may explicitly return a const destination view; that result remains readonly when used by an outer expression. Transfer `<-` retains its const destination result, and ownership moves still require separate steps. The result always denotes the destination's resulting referent, never an owning handle or a public binding-cell pointer.

The assignment method's destination parameter denotes the caller's pointer binding; ordinary parameters retain their usual local bindings. H-303 requires the method body to finish with that destination parameter as a semantic safeguard. Returning another parameter or alias rejects even if it denotes the same storage; a copied value or temporary also rejects. A body may perform branches and updates, then return its destination parameter. The parameter need not be spelled `lhs`: its declared role and binding identity determine the check. This keeps the result explicitly tied to the destination selected for the operation and is not a missing alias-equivalence optimization. Recursive overload calls retain the prototype's existing recursion rejection. Explicit builtin delegation uses `intrinsic (=)(lhs, target)`, followed by the required destination-parameter result; lookup does not depend on whether the caller happens to be an overload.

H-293 requires the assignment-overload destination to declare an external owner. An unannotated lhs is not silently upgraded from the default callee-local target into caller storage. Reject that declaration/use and explain the missing owner annotation. For example, a same-type reference-assignment contract can use:

```text
function (=)(lhs:T in O, rhs:const T in O)
  -> const T in O where {T, owner O};
```

A procedure overload has the same owner obligation. A root-constrained regional destination may use the existing region binder instead. The declared owner must resolve to the actual caller-selected destination and survive the call; a formal owner name alone is not proof. For reference assignment, source and destination must satisfy the fixed-owner rule. Copying an argument into callee-local storage cannot satisfy an external-owner destination contract.

The destination of reference assignment is the selected binding/reference field being rebound. It is not permission to overwrite the object formerly reached through that binding. Owner/provenance and destination access are compiler facts; this requirement introduces no public pointer-to-pointer wrapper, runtime owner argument or binding-cell API. Its acceptance must demonstrate a caller-visible rebind and diagnose a missing external owner with a suggested `in O` / `where {owner O}` declaration.

The following are semantic probes for the target design, not runnable prototype source. They assume the relevant storage remains live and any subsequent write has independently valid permission.

| Probe | Required distinction |
| --- | --- |
| Rebind a to b, then update b's bits payload through an existing writable access. | If a now aliases b's storage, reading a observes that update. Passing only an independent copy of b's bits cannot establish this storage identity. |
| Retain the result view of a <- b, then update a through another permitted alias. | H-206's destination view observes a's updated contents; it is not a frozen copy of the earlier value. |
| Pass that view to an ordinary x:const i64 parameter, then mutate a while the call remains active. | H-201's parameter observes its evaluated bits value, although the separate view still denotes mutable storage. |
| Chain destination conversion across different value types. | The outer overload receives the inner destination's type/result, not the unconverted original source or a new owning handle. |

H-201 makes ordinary const bits parameters/results independent values; H-202 makes const non-bits access read-only borrowing. H-206 separately requires a destination-reference result for transfer, and H-224 supplies the const destination-typed assignment overload shape. The implementation must distinguish the assignment expression's retained storage target from a normal const bits function result. Reading a const source does not discard the need to track where a borrowed result points, nor allow a reference to a callee-local bits copy to escape.

**Confirmed RO-Q1 — assignment-method result category.** H-270 requires destination-view results rather than ordinary copied const bits. H-302 refines `=` access: builtin reference assignment preserves the destination permission, and an overload's declared result may retain or restrict proven access. A writable borrowed result cannot return a readonly view. Transfer `<-` keeps its readonly destination result. A normal `-> const i64` result still copies bits; assignment-family results retain destination storage and lifetime.

Infix `a = b`, explicit `(=)(a, b)` and callable aliases of selected assignment methods enforce the same owner/access/result rules. Assignment methods require an explicit external destination owner. The caller's pointer binding changes through the lhs parameter, while independently declared pointer aliases retain their own bindings. A result is not a pointer-slot capability and cannot rebind that caller name. Existing indexed-operation fixtures remain bounded compatibility evidence; this increment does not redefine general indexed destinations or graph-transfer algorithms.

Lean acceptance distinguishes writable chaining, readonly transfer results, exact and weakened input matches, actual caller rebinding, readonly access preservation, cross-owner rejection, ambiguous type/qualifier tradeoffs, invalid selected bodies and invalid destination results. No overload can obtain missing access from its declared return type.

**Confirmed RO-Q2 — local pointer owner is fixed.** H-271 rejects `a = b` after `ptr a = root(regionA); ptr b = root(regionB);` when A and B are distinct owners, even if both remain live and have the same type. Rebinding within the initializer's owner remains permitted. H-270's owner-mismatch diagnostic applies. The pointer's own local declaration does not allow it to switch target owners; H-203 separately constrains stored graph edges. RO-Q1's destination identity/lifetime rule remains; H-302 defines reference-result access.

H-293 forbids a newly established alias from regaining write permission through a read-only source. In a chain, the inner assignment's read-only destination result remains read-only when borrowed by the outer assignment. Independently held writable aliases can still modify the target; the RHS is never required to be writable merely to read it. The existing ([]=) hook and scalar-by-value prototype do not prove the proposed general reference-assignment, qualifier or storage-result contract.

### Transfer results and chaining

Builtin bits transfer returns a const reference to the destination value. A user `(<-)` overload declares either void or a const destination view (H-307). The view form follows H-206/H-270: It observes the destination's type and stored result, including any selected conversion, and remains dependent on that destination's lifetime. The operation does not return the original source, a new region owner or a frozen snapshot. A later permitted write through another alias may change what a retained result view observes.

For bits transfer, right-nested chaining is useful:

```text
a <- (b <- c);
```

H-209 makes the unparenthesized `a <- b <- c` parse as `a <- (b <- c)`. The inner operation finishes writing/converting b; the outer one reads the resulting const view of b. The outer dispatch therefore uses b's value type and owner, not c's. A const bits parameter of a subsequently called function obtains an independent bits value when evaluated; the transfer expression's const reference is itself a view. The compiler must distinguish reference-result expression category from ordinary const bits call results. This distinction does not yet select an extra public ref wrapper or overload-result syntax.

H-207 requires ownership moves to use explicit steps:

```text
b <- c;
a <- b;
```

After the first move b is the owner; after the second a is the owner. `a <- (b <- c)` cannot move ownership through the const result of the inner operation. A const reference to an owning destination provides non-consuming access, not a second independently transferable owner. Left-nested `(a <- b) <- c` also cannot write through the read-only result. Right association is fixed by H-209; ownership chains fail because the intermediate result is const, without a separate chain-specific ban.

Association fixes grouping, not a new evaluation order. H-148's left-to-right operand/argument evaluation and per-argument conversion rule still applies: evaluate each destination access once, perform the inner transfer before the outer store, and retain/check the outer destination's validity across inner effects. Indexed accesses must not be reevaluated merely because the operator chains. Exact precedence against other new assignment forms remains part of the grammar review.

A nonvoid ordinary `(<-)` method must return the original evaluated destination as a const view; the compiler checks its owner and identity. A void method returns no view and cannot supply a value to a surrounding transfer. Explicit calls and renamed selections obey the same rules. Return type does not participate in overload selection: otherwise identical void and view methods are ambiguous. No invalid result is replaced by a manufactured destination view.

The expected everyday use of `a <- b <- c` is bits transfer. Under H-308, a third-party library intending or recommending such chains should keep their meaning aligned with bits transfer. A library that does not promise or recommend chaining has no additional copy-like semantic obligation. Its normal type, access, lifetime, effects and declared-result contracts still apply. Whole-region moves use explicit steps; user-defined object writes need not promise a chain.

### Logical indexed transfer

`container[index] <- source` selects `([]<-)(container, index, source)` before assuming any addressable element. The container and index retain their types for dispatch. A packed representation can implement the write and return void without exposing an element reference or getter. The native array fallback writes the actual element and returns its readonly view; `intrinsic ([]<-)(...)` explicitly selects that checked fallback.

A nonvoid indexed method declares a const view in the destination container's owner. The method supplies the logical destination view; the compiler checks its lifetime, owner and access without inventing a physical element for an arbitrary container. The ordinary `(<-)` safeguard continues to require its original destination. Function and procedure methods use the existing pure/current-dynamic-context rules. Operands are evaluated once, left-to-right, and selected input conversions finish before the next operand. Destination conversions cannot replace writable storage with a numeric temporary. A void write used as a chain's intermediate value fails ordinary type checking.

[indexed-transfer.cod](../tests/cod/language/indexed-transfer.cod) exercises a two-field packed payload, void writes and a live array-element result. [region-declarations.cod](../tests/cod/language/region-declarations.cod) exercises region forwarding, explicit and inferred owner pairing, typed procedure signatures and surviving local views. [Validation](../tests/cod/language/region-transfer-validation.json) records source checks, native cleanup/relocation and LLVM verification.

### Symbolic destinations in ordinary transfer expressions

H-282 proposes `@object <- rhs` as an overload-backed edit of the current regional referent. H-283 explicitly distinguishes it from the effect-prefixed binding operation `effect @object <- rhs`, which the proposal rejects for non-bits object symbols. No binding is changed by the bare transfer expression and no rollback is implied. [D-10](SymbolicDynamicScope.md#scoped-bindings-versus-object-transfer) reviews its syntax, permissions, single destination evaluation, actual-region arguments and destination-view result. This extends the graph-transfer surface without weakening same-owner stored edges or permitting extraction of the supporting region resource. The bounded implementation and [native evidence](../tests/cod/language/symbolic-context-validation.json) cover this form; general graph-conversion algorithms remain user-library work.

### Bits-field transfer and user-defined graph conversion

H-206 permits language-supported bits-field assignment across region/lexical storage. The built-in operation writes declared bits payloads into existing compatible destination storage, without retaining source references. It is field-value assignment, not an implicit regional copy constructor, object move or byte dump of padding/runtime bookkeeping. It does not reclassify a region-only user type as stack-allocatable merely because its fields are bits.

A whole object containing references has no default cross-owner transfer. The compiler rejects it unless a matching user-defined (<-) function/procedure implements the transformation; it must not silently copy only some fields and leave the reference topology unspecified. Individually selected bits fields remain assignable. The user algorithm may allocate/initialize destination objects through the destination region's policy, transfer their bits, and connect them using references owned by that destination. It cannot store source-owner pointers into the destination graph.

Graph transformation is distinct from moving a whole-region handle. A read-only source overload can preserve the source while creating a different destination representation. If a library algorithm intentionally edits source pool state, it must declare and receive the necessary writable source access; it cannot be selected from a const source/result. Source recycling remains user-level regional policy rather than implicit whole-region ownership consumption.

Sharing/cycles must be handled according to the algorithm's declared graph semantics. A conversion intended to preserve topology must map repeated/cyclic source references consistently; a naive recursive field copy is not sufficient. The language need not impose a universal deep-copy algorithm, map, scratch heap or graph traversal framework. Domain algorithms may instead intentionally transform the topology, with their own acceptance conditions.

The storage rules remain intact: stack-side operands can be permitted bits storage or transient views into a region; this feature does not allocate non-bits graph objects on the stack. Source/destination overlap, destination capacity failure and partially completed writes require explicit overload contracts. The notation alone promises neither rollback nor source immutability in the presence of other aliases.

### Root-policy dispatch and actual owner arguments

H-206 requires selection by both sides' region types, including abstract root-type matching in region :RT; RT supplies the region's internal memory-management strategy interface. H-208 requires the selected function/procedure to receive both actual region references so it can call those methods. Type information alone cannot allocate into the intended region instance.

H-274 extends this requirement to ordinary owner-constrained signatures. [The region-owner form](#region-owners-with-root-policy-access), with its where binder confirmed by H-275, supplies both the root constraint and the actual borrowed region; generic owner constraints remain available when a region-or-scope distinction and allocator access are unnecessary. The graph-overload expansion below must use that same owner/reference relationship rather than infer a region from root-type identity alone. Its older type-constructor notation illustrates logical inputs only; the target public form uses region declarations.

The proposed logical dispatch inputs are destination value/access type, destination owner/root type, source value/access type and source owner/root type. The concrete owner instances remain distinct facts: two region :PoolRoot operands can name different allocations, and matching their root types does not satisfy the same-owner edge check.

A proposed graph-overload parameter layout pairs each object with its region:

```text
function (<-)(
  region destination_region:WritableNodeStore, destination:Node,
  const region source_region:ReadableNodeStore, source:const Node
) { ... }
```

WritableNodeStore and ReadableNodeStore stand for abstract root interfaces in this example; declaration/conformance grammar and the exact overload header/result syntax are not selected. The two region parameters are borrowed. The destination supports the permitted allocation/access methods; source permissions are declared by the algorithm. Both function and procedure overloads may use explicit region references; only procedures may additionally use dynamic context.

For the two-operand source form destination <- source, the proposed compiler expansion supplies the matching owner arguments from tracked provenance. It must prove each region argument actually owns its paired object, including for explicit operator-call syntax; matching types alone is insufficient. This is not ambient @region lookup, user heap allocation or permission to recover a writable region from a const operand. Precise owner-argument adaptation, including how const access limits allocation methods, remains part of the source contract.

Region-owner arguments are real borrowed values, not just static tags. They do not consume ownership, rebind effects or create cross-region stored edges; they let the method traverse the source and use destination allocation policy while both owners remain live. If a method requires writable source access, it must receive that independently valid permission rather than obtaining it through a read-only operand.

Abstract-root matching is proposed as satisfying an interface constraint while retaining the actual concrete RT and owner instance. It is not unchecked covariance of mutable regions, a layout cast to a base root, or an automatic conversion of one allocator's representation into another. The root interface exposes only operations promised by its contract; whole-region release/transfer remains intrinsic, while internal pool/free-list strategy belongs to user code.

H-209 requires a unique most-specific applicable method independently of declaration order; no match or ambiguity is a compile error. For nominal argument constraints, a candidate narrower on every constrained argument dominates a broader fallback. Crossed source/destination specializations may be incomparable and require an explicit method for their intersection. No score based on declaration order or arbitrary inheritance distance is introduced. H-210/H-211 selects nominal abstract <: relations. Data applicability uses declared conformance supplied by the selected adapter context under H-218/H-220. Multiple unrelated declared interfaces do not gain relative priority from declaration order. H-258 supplies typed callable declarations and generic bounds for the existing argument types. Root constraints and inferred region access have bounded native support under K-426; K-429 adds lexical-owner arguments and explicit assignment-destination owner checks. The complete overload-body protocol remains migration work. Existing UoM conversion-route ambiguity policy remains distinct from method-selection ambiguity.

The declaration split is `type` for abstract roots and `data` for concrete product/sum representations. A relationship such as `data ArenaRoot <: NodeStore` is an explicit conformance declaration. It may be head-only and supplied by a module separate from the representation definition. The same root may declare other interfaces in other modules; the adapter must import the ADT before declaring conformance, while a consumer may obtain it using the adapter. Signatures/fields do not infer conformance. Abstract-to-abstract inheritance stays nominal, with multiple abstract parents deferred. Body completeness is checked only as demanded operations require implementations. H-216 makes abstract inheritance confer methods and data annotations interface claims; abstract methods may be distributed across modules, with no Interface body and no eager completeness check. H-217 makes implicit method names shadowable by explicit names. The proposed lookup stage must select a region-policy operation before dispatch and must not use a hidden policy as fallback. Interface and selection requirements are reviewed in [D-07](DomainLanguageDesign.md#abstract-types-and-data-declarations). Matching a generic root R constrained by R <: NodeStore must preserve R and the actual owner instance. Using/import of root types brings source-visible associated methods under H-211; import preserves their definitions and permits extension, while mixin redefines a body in the receiver. This does not cast one root layout into another or create new ownership permissions.

With concrete root types known on demanded WPO paths, selection and specialization can happen at compile time. Runtime alternatives need an explicit type/sum discrimination contract rather than a claim that WPO knows all values. No vtable, dynamic dispatch service or general inheritance framework is required merely to make this first use possible. Abstract identities must follow the eventual module/type identity contract; matching spelling, method signatures or memory layout alone is not conformance.

Bits transfers involving lexical storage do not require invented regions for the stack. Region-specific overloads apply where real regional operands exist; lexical-owner adaptation for more general overloads is a separate source form. The same-owner check continues to use the actual region/lexical instance, independently of abstract root matching.

Lean acceptance should cover a bits-field copy across owners, a read-only destination-result chain that observes the intermediate representation, rejected chained ownership moves, and one reference-bearing graph conversion with relevant sharing/cycle behavior. Add exact/abstract dispatch and a cross-specialization ambiguity case, wrong object/owner pairing rejection, and const-source permission rejection. Typed-owner parameters, implicit owner-argument adaptation and graph-transfer overloads are design targets without native implementation. Ordinary abstract argument dispatch is implemented separately; its evidence does not validate owner pairing or graph transfer.

### Named owner consumption

H-262 selects non-optional ordinary owners with a compiler-tracked consumed state. After an operation transfers ownership, the source declaration is unavailable on that path: reading, borrowing from or transferring it again is a compile error. Its type is not rewritten to an optional and no hidden null value is introduced. An optional/result is explicit when runtime absence or an operation outcome is part of the business model.

A local transfer into an effect keeps the region alive under the dynamic owning scope. H-180/H-181's captured shadows and non-extraction rule remain unchanged. Consumption of the lexical name and validity of already obtained views are separate facts. H-263 preserves local views while the new owner remains alive and accessible; ending that ownership scope or sending the region away revokes access. Owner reinitialization, occupied destinations and revival after a failed-send return remain separate choices.

The compiler must preserve the consumed fact across demanded calls and branch joins. An unconditional use requires valid ownership on every reaching path. H-264/H-267/E4–E5 fixes the first construction and call/result transfer points; more general path-correlation analysis remains separate. WPO does not imply that runtime branch choices or allocation identities are known in advance.

### Post-send state without implicit null dereferences

Under H-262, a completed ownership transfer removes the source declaration's availability in the compiler's control-flow state. It does not change the value's declared type into a nullable type or define a null payload that later fails at runtime. Both reads and a second send through that declaration are compile errors after handoff. References obtained from it also lose sender access.

The proposed send result owns the disposition on every outcome: local acceptance returns no packet; a local error returns the unsent region together with the error. The original source is unavailable on both paths, so caller code does not have to infer whether a hidden runtime mutation restored it. In schematic source only:

```text
ref view = root(packet);
ref outcome = send(mailbox, packet);
# root(packet), deref(view), or send(mailbox, packet): compile errors here.
match outcome {
  Accepted => { /* No packet remains with the sender. */ }
  LocalError(error, returned_packet) => {
    ref editable = root(returned_packet);
    # Edit, retry with returned_packet, or release it by ending this arm.
  }
}
```

`match`, alternatives and bindings above are explanatory, not selected grammar. The result must be handled so that a returned region has one responsible holder; its destruction/forwarding follows the same region rules. The agent proposes obtaining fresh views from the returned owner instead of reviving every old alias on the failure path. This is a static access rule, not a runtime pointer-generation table or a claim that the failed packet occupies different bytes. The treatment of pre-send aliases after returned ownership remains a design choice requiring confirmation.

Conditional handoff also needs a static postcondition. If one branch sends and another retains the region, unconditional later use is not valid on every path and must be rejected. Code can handle each path separately or explicitly return/store an optional/result value. More precise proven path correlations may be supported later; a runtime null check is not the fallback for missing proof.

Explicit optional sources, when supported, still require presence handling and a defined absent alternative after extraction. That facility does not replace H-262's static consumed state for ordinary owners, and supplies no implicit unwrap or null-dereference behavior.

Compile-time reference provenance is required for ordinary owners and explicit optional outcomes alike. A stale view cannot become usable merely because the owner handle was placed in an optional or returned from a failed send. No such post-send checking is currently implemented: native mailbox transfer remains unimplemented; ordinary owning variables have the bounded local-lifecycle evidence above.

### Scoped resource and capture walkthrough

The following proposed forms avoid a named transfer source; the resource shadow/capture behavior itself is already confirmed:

```text
region symbol state:Packet;
effect @state <- make_packet(1u64, 65536u64);
proc read_state() -> u64 { deref(root(@state)).sequence }
symbol probe : proc() -> u64;
effect @probe <- read_state;
{
  effect @state <- make_packet(2u64, 65536u64);
  behave @print(perform @probe()); # 1: definition-time captured access.
  behave @print(behave @probe());  # 2: current access.
}; # Release R2, restore lookup of still-owned R1.
```

`perform` temporarily restores access to R1 and hides R2; it acquires or destroys neither. The top-level owner of R1 lasts until program exit. `effect @other <- @state`, sending `@state`, and routing it through a purported owning identity function remain invalid. Effect installation yields void; an explicit subsequent symbol read gives non-extractable access under the installed owner.

The current unparameterized region-resource subset has direct native evidence for shadow/capture: [region-owner-capture.cod](../tests/cod/language/region-owner-capture.cod), instrumented by [regional_runtime_tests.py](../tests/cod/language/regional_runtime_tests.py), acquires one enclosing region and three inner regions in selected logical scopes. It records eighteen live reads, releases [1,2,3,0] exactly once each and acquires nothing from the skipped logical operand. [The native resources result](../tests/cod/language/regional-validation.json) preserves this evidence at its recorded compiler revision. Symbol declarations are typed. Root-parameterized ordinary construction and transfer have separate D-17 evidence above. Routing an existing dynamic region through a same-region conditional or a procedure result also retains the no-extraction rejection (K-308). These checks establish neither the proposed typed make expression nor ordinary packet transfer.

Acceptance for the new type/construction increment must demonstrate initialized root access, actual editing, factory return and resource installation with exact cleanup, plus rejection of copies, dynamic extraction and expired views/captures. Named transfer, branches and optional send-error recovery require their coordinated source contract. Capacity/allocation errors require distinct coverage: H-265 selects unchecked ultimate OOM, while D-17 retains invalid-capacity and fixed-region-exhaustion classification for review. The examples must validate the user's lifecycle, not introduce a no-op ownership-disposal API to stand in for it.
