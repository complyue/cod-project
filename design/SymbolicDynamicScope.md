# CoD symbols, dynamic scope and callable kinds

Design revision: 139 · Recorded: 2026-11-13 · Phase: language/application co-design.

## Design authority

- **Recursive contexts and capture demand:** [K-559](DesignAuthorityRegistry.md#k-559)–[K-561](DesignAuthorityRegistry.md#k-561) and [K-563](DesignAuthorityRegistry.md#k-563)–[K-567](DesignAuthorityRegistry.md#k-567), D-18r84/D-10r139, continue H-97/H-102/H-180/H-181/H-286/H-319/H-337–H-344/H-356–H-362/H-365/H-371. C-37/P-89, C-39/P-95, C-66/P-252/P-253, C-65/P-294/P-299/P-301–P-303 and C-63/P-321–P-324/P-326/P-329 preserve explicit contexts, actual owners and cleanup under charter §§2–3/§5.3/§6/§8/§12. Selected callable images, current TLS holders, inherited procedural snapshots and arguments use complete-schema and returned-owner proofs. Invocation owner correspondence preserves aliases, root layout, access, liveness and effective shared groups; retained capture owners remain fixed. Borrowed snapshot metadata and shared capture demand remain distinct from cleanup responsibility and compiler replay-object identity. Reused captured invocations still validate their effective groups. Delayed checking uses saved invocation bases, and distinct captured caller frames remain eligible for further helper checking. D-14/D-17 constrain ownership/completion; runtime payloads, lifetime and source policy are unchanged. [Fresh-image acceptance](../tests/cod/language/callable-fresh-image-validation.json) extends [captured recursion](../tests/cod/language/callable-captured-recursion-validation.json) and [replayed procedures](../tests/cod/language/callable-replayed-procedure-validation.json) with self/mutual recursion, returned copies, partial-replay/expired-owner rejection, native cleanup and measured compilation growth. Broader context/result proofs and D-16/D-22/D-23/D-24 boundaries remain; pure/TCO/compiler-pilot implementation stays deferred.

- **Deferred current context:** [K-554](DesignAuthorityRegistry.md#k-554), D-10r132/D-17r52/D-18r72, continues H-102/H-180/H-181/H-319/H-337–H-344/H-356–H-362/H-365/H-371 and C-63/P-235–P-236/P-321–P-324/P-326/P-329 with C-65/P-294/P-299/P-301–P-303. Current bindings remain available on proven registered-action paths through normal/failure joins; perform retains definition-time captures. This preserves explicit context, ordering and resource responsibility under charter §§2–3/§5.3/§6/§8/§12. D-14 owners/access, D-10 binding rules and D-17 cleanup remain constraints. [Acceptance evidence](../tests/cod/language/defer-context-validation.json) covers conditional registration, later rebinding, grouped regional references, loops, nested registration, recovery/finally, native recursion, failures and missing/expired-context rejection. No runtime presence flag, lifetime extension, owner union, ABI or allocator change follows. Wider relational proofs, open body-result staging, inferred-signature transport, module queues and family/AOT integration remain unfinished. D-16/D-22 and pure/TCO/compiler-pilot deferral remain unchanged; this is scoped continuity, not whole-file approval.

- **Implicit owner bindings:** [H-382](DesignAuthorityRegistry.md#h-382)/[K-538](DesignAuthorityRegistry.md#k-538), D-10r131, confirms P-343: every inferred owner is a const lexical binding in the callable body, including generic region-or-scope owners. C-65's provenance/root-capability distinction and H-366's binding/target constness remain; H-381's nearest lookup includes these names. This preserves semantic identity, explicit permissions and lifetime under charter §§2–3/§4.3/§5/§6/§8 without requiring a runtime owner object. Acceptance covers shadowing, immutable binding, permitted target writes and absence of implicit region capability. Broader first-class owner storage, actual-owner union and native dispatch representation remain outside this clarification.

- **Nearest symbol lookup and execution separation:** [H-381](DesignAuthorityRegistry.md#h-381)/[K-537](DesignAuthorityRegistry.md#k-537), D-10r130, records P-342: @ access follows the nearest lexical binding and rejects a non-symbol binding. Captured/current execution, cached identities and delayed bodies preserve that lookup. H-248/H-249/H-372 separates semantic ModuleWorld records from ModuleExecution values/progress; source behavior remains governed by the existing contracts. This continues explicit names/context under charter §§2–3/§5.2/§6/§8/§12 and the H-378 value path. Acceptance covers shadowing, capture/mixin lookup and preserved native ownership/context; full family/stage migration and physical dispatch mechanisms remain outstanding.

- **Initialization cancellation:** [H-380](DesignAuthorityRegistry.md#h-380)/[K-536](DesignAuthorityRegistry.md#k-536), D-10r129, confirms P-339–P-341: omit the affected dependency initialization TLS writes, preserve operand evaluation/effects/failures and procedure-local installations. The H-378 value path and existing owner/cleanup constraints continue under charter §§2–3/§5.2/§6.3/§8/§12. Acceptance distinguishes those observations and final-image removal of genuinely unused code; identity/conditional-coverage and detailed native mechanisms remain design work.

- **Callable-world and AOT scope:** [H-372–H-377](DesignAuthorityRegistry.md#h-372)/[K-534](DesignAuthorityRegistry.md#k-534), D-10r128, records definition/extension, callable materialization, explicit installation and family invocation as separate operations. C-35/C-36/C-37/C-57/C-59 and P-330–P-332/P-335/P-336 preserve explicit semantic construction, typed execution and inspectable business dispatch under charter §§2–3/§5.2/§6/§8/§12. [H-378](DesignAuthorityRegistry.md#h-378)/[K-535](DesignAuthorityRegistry.md#k-535) confirms [D-21](AheadOfTimeMultipleDispatch.md)'s P-333/P-334/P-337 value path for the stated behavioral scope. Detailed namespace, type-evidence/ABI, coverage and diagnostic mechanisms remain pending-human. H-379's entry-installation requirement is bounded by H-380: initialization-only suppression with operand evaluation preserved. Composition identity/conditional coverage and detailed native mechanisms remain open. Existing qualified invariants continue only within their unchanged scopes. Acceptance requires the composed family/context, installation/capture and cleanup scenarios in D-21; current implementation evidence does not establish that new behavior. No root-charter change or whole-document qualification is inferred.

- **Callable completion continuity:** [K-526](DesignAuthorityRegistry.md#k-526) continues H-361/H-362/C-63/P-324 and H-365/P-326 for revision 127's callable source and return/effect interaction. Proc/func values use explicit return; iteration and enclosing-scope effects restore on return, and one finally body can replace a pending return/error. D-17 owns completion semantics; D-14 ownership/access and D-18 typed complete values remain unchanged. [Return evidence](../tests/cod/language/return-validation.json) bounds acceptance. This serves readable business control and explicit lifetimes under charter §§2–3/§6/§8/§12; defer and unrelated pending designs remain unimplemented/unqualified in their recorded scopes.

- **Grouping continuity:** [K-525](DesignAuthorityRegistry.md#k-525) continues the H-149/H-277/H-319/H-365 declaration, binding and scope contract for this revision. Ordinary groups share names and effects with their enclosing scope; explicit scopes and calls restore their bindings, while iteration effects follow D-17. Conditional and exceptional paths join available typed bindings and declaration initialization; captured/current invocation retains its established interpretation. C-63/P-286/P-326 and the qualified symbolic-context relationships serve explicit lifetimes and readable effects under charter §§2–3/§6/§8/§12. D-14 owners/access, D-18 callable values and D-17 cleanup remain constraints. [Native grouping evidence](../tests/cod/language/grouping-validation.json) covers this bounded implementation; return/defer, broader pending designs and D-16 fraction execution are not qualified by it.

- **Current regional captures:** [D-18](RegionalClosures.md)/[K-459](DesignAuthorityRegistry.md#k-459) is the bounded contract for structural region requirements, aliases, captures and owner projection under H-327–H-335. This revision updates only that navigation and removes superseded research text under H-336; C-65/C-73/C-75 and P-294–P-298 retain their confirmed scope. Owner/access/relocation, dynamic effects and D-16 remain constraints. Acceptance and unresolved limits stay explicit in D-18; unrelated pending contracts are not qualified.

- **Loop scoping rules:** [H-365](DesignAuthorityRegistry.md#h-365)/[K-522](DesignAuthorityRegistry.md#k-522), C-63/P-286/P-321/P-326, confirms the dedicated [loop contract](ExceptionControl.md#loop-scoping-rules): body/zero-iteration else declarations require a nested explicit scope; effects and fresh effect resources end at each iteration/else exit; direct deferred work registers to the nearest enclosing full scope. Delayed blocks cannot capture loop-control bindings, and deferred borrows cannot outlive iteration resources. This serves explicit resource responsibility, inspectable effects and intentional dynamic cleanup accumulation under charter §§2–3/§6/§8/§12. H-365 supersedes H-319's direct-loop-defer prohibition and resolves H-364; no general new scope abstraction, lifetime extension or loop form is selected. Acceptance covers all exits, hidden-owner retention, snapshots, global LIFO, exact release and accumulation costs. Implementation evidence remains separately bounded. [K-524](DesignAuthorityRegistry.md#k-524) corrects this relationship identifier without changing the confirmed contract.

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

- **Symbolic implementation scope:** [H-287–H-291](DesignAuthorityRegistry.md#h-287)/[K-426](DesignAuthorityRegistry.md#k-426) records the requested implementation and callable/result decisions: every effect yields void; procedure rebinding uses `=`, while the recorded inline `effect proc` path constructs captured values and installs them. H-372/H-373 supersedes its implicit-registration/installation and no-extension interpretation; compiler migration remains outstanding. C-36/C-54/C-65/C-66 and P-248–P-254 preserve typed dynamic context, actual-region lifetime and explicit capture. [Native evidence](../tests/cod/language/symbolic-context-validation.json) covers typed TLS payloads, staged groups, touch/replay checks and overload-backed object transfer. This bounded scope does not qualify whole-document drafts or implement ordinary ptr migration, transport, runtime closures or fractions.

- **Region declaration direction:** [H-274/H-275](DesignAuthorityRegistry.md#h-274)/[K-420](DesignAuthorityRegistry.md#k-420) requires root-constrained access to the actual region, confirms `where {region r:RootType}` and makes region a declaration keyword throughout user-facing region references. [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language) specifies the complete local/parameter/result/symbol surface under H-307. Generic owner remains a region-or-scope constraint; effects, cleanup and function restrictions are unchanged.

- **Variable/parameter baseline:** [H-271/H-272](DesignAuthorityRegistry.md#h-271) requires `ptr` variables, initialization-fixed owners, same-owner-only rebinding, implicit dereference and callee-local default parameter targets. [H-273](DesignAuthorityRegistry.md#h-273)/[K-418](DesignAuthorityRegistry.md#k-418) confirms [D-14 owner annotations](RegionalOwnership.md#confirmed-owner-syntax): `in O`, `where {owner O}`, result owners, and optional `ptr` in parameter declarations. This replaces the older default borrowed-parameter source contract without changing effect capture/shadowing, function restrictions or region cleanup. Current native examples retain their prototype spelling and are not evidence of this migration.

- **Bounded exception/lifecycle contract:** [H-267](DesignAuthorityRegistry.md#h-267)/[K-408](DesignAuthorityRegistry.md#k-408) accepts [D-17r3 E1–E5](ExceptionControl.md#accepted-first-implementation-contract), including explicit throw-from only, cleanup, fresh owners and call/result transfer. This overrides pending labels for that enumerated scope; wider qualifier, assignment, mailbox and user exception graph choices remain open. [K-414](DesignAuthorityRegistry.md#k-414) records completed H-267/H-269 E1–E5 acceptance and [current native evidence](../tests/cod/language/exception-ownership-validation.json). Wider unconfirmed scopes retain their status.

- **Registry / scope:** [D-10](DesignAuthorityRegistry.md#d-10), revision138, within the registry's recorded scopes; [H-265](DesignAuthorityRegistry.md#h-265)/[K-406](DesignAuthorityRegistry.md#k-406) adds exception-control planning in [D-17](ExceptionControl.md); [H-263/H-264](DesignAuthorityRegistry.md#h-263)/[K-405](DesignAuthorityRegistry.md#k-405) records permitted local-view survival and the first typed construction form; [H-262](DesignAuthorityRegistry.md#h-262)/[K-404](DesignAuthorityRegistry.md#k-404) records compiler-tracked consumption of named ordinary owners without changing dynamic resource capture/shadowing; H-257/H-258 and [K-399](DesignAuthorityRegistry.md#k-399) cover lexical callee selection, typed callable declarations and the compiler value/place distinction; [K-400](DesignAuthorityRegistry.md#k-400) records successful build repair and28 focused compiler/native/IR checks. [K-396](DesignAuthorityRegistry.md#k-396) records the completed architecture audit on the K-395 compiler, including H-253 conflict rejection, H-252 warning controls and cross-world TLS. Earlier checkpoints retain their actual scopes. H-234/H-237–H-249 supplies independent module worlds, separate typed TLS, vocabulary identities, receiver-local composition and identity-based publication; H-245 supersedes cross-module editing. H-251 reserves RTTI for human discussion. Earlier decisions/evidence retain their scopes in the registry. Full contracts and candidate value paths remain pending-human; this revision does not qualify unresolved mechanisms. [K-401](DesignAuthorityRegistry.md#k-401) records H-255/H-256 comment reservation and H-260/H-261 quantity/widening decisions; full fraction execution remains deferred.
- **Value lineage:** C-10's inspectable typed execution motivates C-35/C-43/C-45/C-46 identity/composition through P-84/P-100/P-102/P-109–P-111; C-56/C-57/P-182–P-197 covers abstractions, dispatch and adapter visibility. C-39/C-54/P-94/P-143/P-155 retains regional lifetime boundaries; C-58/P-199–P-203 covers publication and selected contexts. These candidate paths serve charter §§2–4 without borrowing authority from technical dependencies.
- **Human baseline:** H-96–H-141 supplies symbols/modules/units, subject to supersession. H-143–H-146 selects LLVM, lean checks and the zero-offset limitation. H-149/H-150 bounds value shadowing/reserved names. H-166/H-180/H-181 governs module lifetime, capture/shadow and non-extraction; H-184–H-189 governs typed lexical/symbolic procedures. H-193/H-196–H-210 governs regional data, intrinsic owners, explicit-memory functions, borrowing/const and transfer requirements. H-211–H-233 governs source selection, mixin, explicit interfaces, concrete dispatch, aliases/implementation contexts and publication, with the scope/exclusions recorded in the registry.
- **Invariants / non-goals:** preserve region-only non-bits storage, zero-cost relocation, no regional copy/move, same-owner stored edges, preserved hidden/captured owners and no dynamic access or procedure calls from functions. No module namespace, runtime interface/borrow registry, immutable snapshot, allocator/toolchain change or SNN model is selected. Compound views, higher-order overload values, full qualifiers/ownership construction/transfer and mailbox remain bounded or pending.
- **Acceptance / evidence:** H-245 authorizes the corrected architecture goal, whose implementation audit is recorded by K-396; H-221 records the completed ten-hour period. Native checks must distinguish source/caller contexts, conformance visibility, using/import permission, unique/ambiguous dispatch, demanded/unused missing bodies and owner lifetimes. The [prototype contract table](../tests/cod/language/README.md#current-bounded-acceptance) and [evidence below](#runnable-prototype-evidence) identify implemented subsets and actual source revisions; qualifier, transfer and mailbox acceptance requires its own evidence.

## Ordinary storage and checked nullable access

H-309–H-315 distinguishes direct val bits, initialization-checked ref targets and nullable ptr targets. [D-14](RegionalOwnership.md#ref-and-ptr-declaration-direction) owns field storage, local lifetimes and first non-null owner acquisition. [D-17](ExceptionControl.md#checked-nullpointerexception) owns checked NPE: local handling or immediate func/proc throws, with mandatory handling at module top level. Symbolic procedure signatures retain this contract; neither perform captures nor behave's current bindings erase it. Null is a reserved literal and isnull does not dereference its pointer operand.

These ordinary storage declarations do not replace the flat typed TLS slots. Effect-only writes, capture, shadow restoration, region-symbol backing and non-extractable resources retain their contracts. Val parameters copy supported bits into callee-local storage, while external references retain explicit owners. New source acceptance is recorded with the current validation; earlier native evidence retains its source/compiler scope.

## Symbolic storage and regional provenance

Dynamic symbolic variables have their own storage and rebinding contract (H-277–H-281):

| Binding category | Storage and writes |
| --- | --- |
| Bits-valued symbol | Its bits payload is allocated directly in TLS. `effect @x <- rhs` transfers bits rather than rebinding a pointer to the RHS's lexical storage. The payload does not borrow that storage. |
| Non-bits object-valued symbol | It references an object inside a region supplied by a region symbol. `effect @x = rhs` may select another such region, subject to the declared type and actual region lifetime. No object is copied into TLS. |
| Region symbol | It provides the whole-region resource under the existing ownership contract. Shadowing preserves hidden owners; extraction remains forbidden. |
| Procedure-valued symbol | It retains the separately defined callable/capture contract. The object-reference rule does not select a new runtime closure representation. |

Symbolic binding writes require `effect`: `effect @x = rhs` performs rebinding, while `effect @x <- rhs` performs ownership/bits transfer (H-280). Bare `@x = rhs` and bare `@x <- rhs` cannot write a symbolic binding. The proposed object-content transfer form is distinguished below. Both forms have scoped validity: an effect saves the preceding binding and restores it when its scope ends. For bits, the saved state is the preceding bits value; for an object reference, it is the preceding reference and its provenance. Edits to referenced regional contents are not undone by restoration. Every effect expression has type `void` (H-288). A following explicit symbolic read obtains the current binding; an effect cannot supply a value argument or form an assignment chain.

The object-reference rebinding form does not take ownership away from the supporting region symbol. A fresh whole-region resource still uses consuming transfer under the existing no-extraction rule. H-281 requires `<-` for bits and `=` for object-reference rebinding. `effect @bits = rhs` is rejected; it cannot introduce a pointer to RHS lexical storage. An object-reference binding is not a bits-valued symbol merely because its native address occupies bits. Whole-region borrowing/replay and regional graph transfer retain separate interfaces. Neither syntax admits an implicit graph copy or extraction from an existing region symbol.

The same symbolic identity may refer to objects in different regions over time. This is an explicit exception to initialization-fixed owners for ordinary `ptr` variables. It changes a TLS reference, not a pointer field stored in either regional graph; same-owner stored-edge rules and zero-cost relocation remain intact. A region with an ordinary owner is not sufficient merely because it is live: the object binding must be backed by a region-symbol resource. This does not install such a resource implicitly or transfer its ownership.

The compiler must preserve the reference's actual region association through branches, capture and restoration, and reject a use that can outlive its supporting resource. Equal root types or equal symbol spellings do not prove equal region instances. Rebinding or shadowing a region symbol does not retarget existing references: a hidden owner remains alive under H-180, and an old reference still denotes its original region. TLS thread lifetime cannot extend that region's lifetime.

Procedure capture retains bits values or the selected regional reference with its lifetime dependency. Perform replays those captured values; behave uses current bindings. Capturing an object reference does not own the region or copy its graph. Detailed capture/escape lowering must prove this lifetime condition; the earlier proposal to exclude every interior reference is not an accepted restriction on all symbolic bindings. Functions still cannot read or write symbolic bindings.

Bounded acceptance must demonstrate independent TLS bits payloads, bits/reference restoration, one object symbol rebound between two live symbol-backed regions, and a captured reference retaining its original region. It must reject unsupported ordinary-owner backing and references that escape their resource lifetime, including through branches/captures. No runtime borrow registry, RTTI, extra per-object ownership or fraction representation is selected. Fixed-width fraction bits, when implemented, retain their declared representation; this rule chooses no new promotion or alignment policy.

### Shared-region symbol declarations

H-284/H-287 extends owner annotations to a set of symbolic object references:

```text
symbol @o1:T1 in r, @o2:T2 in r
  where {region r:RT};

effect @o1 = xxx, @o2 = yyy;
```

The shared r constrains the referenced objects to one actual region instance whose root satisfies RT. It does not merely require equal root types, nor require T1 and T2 to be equal. A concrete root constraint and an abstract root-policy constraint follow the same type identity, explicit visible conformance and concrete-dispatch rules as region owner parameters. Regional lifetime, access permission and region-symbol backing still apply independently. Inferring r does not install a new region resource or transfer ownership.

This form concerns non-bits object-reference symbols. It does not relocate bits-valued symbolic payloads from TLS into a region, and the declaration does not itself bind either symbol. The source examples retain the explicit @ marker; dropping it from effect targets is not inferred from the abbreviated discussion example.

The declaration-level r expresses a shared owner relation; it is not a new symbol identity or a program-wide region name. The relation instantiates the relation for each effective set of bindings, allowing a joint shadow from region A to region B rather than pinning both symbols permanently to the first region. Local aliases must retain the relation attached to the symbol identities; renaming/selecting a symbol cannot erase it. Separate declarations that happen to spell their binder r do not thereby name one region. The clause does not introduce an ambient lexical r value outside the declaration; obtaining region access in a callable still uses its declared owner/resource inputs.

The same-owner requirement must describe the effective bindings, not just the spelling of a comma-separated effect. When both members are active, rebinding only one to a different region would break the declared invariant and must not be accepted. Replacing one by another object in the same region does not break it. A joint rebind may change both owners together. Shadow exit must restore a coherent preceding set; captures and demanded calls must preserve the same relation. This adds a relational constraint to the existing flat TLS entries, not a runtime tuple object, copied regional graph or another module world.

#### Group evaluation and installation

H-285 confirms the following order for the reference-binding group:

1. Evaluate RHS expressions left-to-right, completing their ordinary evaluation and applicable checks. The group's pending bindings are not installed during this phase, so yyy does not see the proposed o1 binding.
2. Establish each symbol's type/access/lifetime requirements and the shared actual-region/root constraint.
3. Install the group together and retain the preceding bindings for scoped restoration. No source-program call observes this group's half-installed state.

If an RHS throws, none of this group's proposed bindings is installed. This is a binding-publication guarantee, not a transaction on RHS execution. Explicit effects and regional mutations performed by RHS code follow their own scope/exception semantics; no immutable snapshot, rollback of object edits, cross-thread atomicity or hardware atomic operation is implied. WPO must establish required owner relations along admitted paths; this proposal does not choose runtime owner tags or an RTTI fallback for unproven relationships.

#### Explicit capture dependencies with touch

A procedure using only @o1 can capture its value from A, while a later caller has both symbols bound into B. H-286 requires rejecting a perform call that would replay only captured o1 and create an invalid A/B pair. The compiler does not silently capture unused group members. A partial replay that preserves the owner constraint is not rejected merely for being partial.

`touch @o2` explicitly includes that symbol in the containing procedure's dynamic capture dependencies without using its referent for computation:

```text
proc p() -> void {
  touch @o2;
  work(@o1)
}
```

Here work denotes an illustrative operation with a void result. At value construction, the procedure captures the values of both the normally used @o1 and explicitly touched @o2, following ordinary capture rules. Under perform, their captured values are checked and replayed together, then the caller's preceding bindings are restored. Under behave, current caller bindings remain in force. Touch does not request captured replay when the call mode is behave.

Touch uses the existing symbol identity and alias-resolution rules. It declares a dynamic dependency; it does not rebind TLS, dereference object fields, copy a regional graph, invoke an object method or take ownership. It has no meaningful computation result. Ordinary demanded binding, type, shared-owner and capture-lifetime checks still apply, and functions cannot use it to introduce dynamic dependencies. Adding a touched reference adds its lifetime dependency; it does not keep the region alive beyond its owner.

After capture/binding analysis, touch needs no object-access instruction in the procedure body. Its dependency must not disappear merely because the referent is otherwise unused: capturing and replaying the value may still require storage or native instructions. This is a language capture operation, not a volatile read or an optimizer workaround. Only the shown symbol form is specified; arbitrary expression evaluation or member traversal is not introduced.

Acceptance must reject the incompatible partial perform, admit explicitly touching the companion when both captured references have a valid shared owner, preserve current bindings for behave, and retain the dependency through symbol aliases. Missing required bindings and expired captured regions still reject. Removing touch as a dead expression before capture analysis must not silently change those outcomes.

#### Remaining source details

Single, grouped and inline-procedure effects all return void. The implemented group form uses `=` entries and one shared region clause; mixed transfer/rebind groups remain outside this increment. There is no implicit tuple or transaction result.

Acceptance must distinguish two objects in one suitable region from two equally typed but distinct regions; reject an unsatisfied root constraint or invalid lifetime/backing; keep the same relation through aliases and scope restoration; show that the second RHS sees the preceding group, and that RHS failure installs no new members. A joint A-to-B shadow should pass where a mismatching single-member shadow rejects. H-286 supplies the capture acceptance above. Current native reports do not establish owner-generic symbolic groups, touch or batched installation.

### Scoped bindings versus object transfer

H-282 proposes separating scoped binding operations from persistent object mutation; H-283 explicitly requires the two source forms to be treated as different syntax, not an optional effect modifier. This review is pending implementation and does not qualify unspecified graph-overload interfaces.

| Source form | Operation and scope behavior |
| --- | --- |
| `effect @bits <- rhs` | Transfer bits into TLS, save the preceding payload, restore it on scope exit. |
| `effect @resource <- rhs` | Acquire an eligible whole-region owner for this scope, preserve any hidden owner and restore lookup on exit. Releasing the new acquisition does not revive its consumed ordinary source. |
| `effect @object = rhs` | Temporarily rebind the reference; restore the previous reference on exit. Mutations of either referent are not rolled back. |
| `effect @object <- rhs` | Proposed compile error for a non-bits object symbol. An object-transfer overload cannot make this binding form valid. |
| `@object <- rhs` | Proposed ordinary transfer expression: dynamically read the destination reference and invoke a matching user overload to edit that object. No scoped binding is created, and no implicit undo occurs. |

The grammar distinction is explicit: the effect-prefixed form is a binding operation checked against the symbol category. The bare form is a transfer expression with a symbolic-read operand; its overload receives the destination object access, not authority to rebind the TLS slot. A missing overload is a compilation error for this whole-object form, including a non-bits object whose fields happen to be bits. Existing bits-field transfers remain a separate operation. Bare `@bits <- rhs`, `@resource <- rhs` and `@object = rhs` are not authorized by this proposal.

A saved bits value or reference is enough to restore a binding. It is not enough to restore a mutated graph: aliases, allocations, pool changes and other edits may be visible independently of the symbol. No transaction log, graph snapshot or inverse overload is needed when object transfer is explicitly persistent. Normal scope exit, finally and caught exceptions restore bindings/perform cleanup without undoing regional writes. The overload defines its failure behavior; partial mutation is possible unless its own algorithm provides a stronger guarantee.

The established transfer contracts constrain the syntax:

- Resolve the destination once before evaluating the RHS under H-148. Retain its actual region and permissions across RHS effects; do not look up the destination symbol again when invoking the overload. Invalid lifetime across the operation must reject.
- Dispatch through the ordinary selected method context, using the object types and relevant root interfaces. H-206/H-208 requires actual source/destination regions for graph algorithms. Those regions come from operand provenance, not an assumed current `@region`, since captured or shadowed references may designate another still-live resource.
- Require writable destination access and preserve source permissions. A user overload cannot create cross-owner stored edges, extract a dynamic region owner or override lifetime checks. Its algorithm defines self/overlap and sharing/cycle behavior; there is no implicit graph-copy fallback.
- A transfer overload may return void or a read-only view of the original evaluated destination. The nonvoid form retains H-270's identity/lifetime safeguard: temporary rebinding inside the method cannot retarget that result. Void writes do not require an addressable result; their use as an intermediate chain value fails type checking. H-308 limits bits-like chaining guidance to libraries intending or recommending chains.
- The expression is available only where symbolic reads are permitted: a procedure or module execution scope. Its selected overload can be a function operating on explicit object/region arguments, or a procedure using current dynamic bindings. This does not authorize functions to read `@object` directly or force every memory-editing overload to be a procedure.

Acceptance covers one overload-backed update, persistent content after the enclosing effect binding restores, destination stability when the RHS shadows the same symbol, actual-owner access under captured replay, and rejection of effect-prefixed object transfer, missing overload, const destination or invalid lifetime. The focused [symbolic-context evidence](../tests/cod/language/symbolic-context-validation.json) records this bounded implementation; it does not establish general graph-cloning algorithms. Fixed-width fractions remain bits and use the bits-transfer rule; no fraction layout or promotion choice follows.

## Callable effects

A symbolic definition registers or extends a module-world family without installing a runtime value. The selected combined form performs definition/extension at compilation and compatible family-view installation at runtime:

```text
symbol @selected:proc() -> i64;
effect @selected() -> i64 { return @gain; };
effect @selected;
effect @selected = other;
effect @selected = @other;
```

Existing import/extension permissions, declaration promises, applicability and ambiguity checks govern method information. An existing symbol retains its identity and declared slot contract. That contract constrains the installed family view; it is not a blanket prohibition on adding methods. The combined form installs the compatible family selected in the current module context, including applicable methods, rather than only its added method.

`effect @selected;` adds no method. It materializes the current composed family, including admitted extensions/mixins, and installs it. Bare `other` denotes a registered symbol object whose selected module semantics supplies a family view. `@other` reads an existing TLS callable value; installing it retains its captures. An ordinary lexical callable value is also an installation source under its existing complete-value contract. Source/destination identities may differ without merging their methods or redirecting the source interpretation.

All installation forms yield void and restore the prior binding on normal/exceptional scope exit. Procedure rebinding uses `=`, not owning-region transfer with `<-`. Captures arise at runtime value construction; bare symbol lookup or module registration performs no capture. Reinstalling a value neither recaptures nor extends expired owners. `perform` replays captures and `behave` uses current bindings. Functions retain their dynamic-context exclusion.

The prototype currently accepts `effect proc`; treatment of that spelling, defaults and detailed grammar migration remains DD-AOT-07. The selected `effect @sym(...)` behavior is settled. A skipped runtime path installs nothing; repeated execution creates installations without repeatedly registering methods. Existing declaration/scope boundaries apply.

H-379 requires the entry module to explicitly install a dependency callable that it extends; omission is a compile error. Correct entry installation cancels the applicable dependency installations so they do not retain unused world code. [D-21](AheadOfTimeMultipleDispatch.md#entry-installation-and-unused-dependency-code) owns initialization-only cancellation with operand evaluation preserved and LLVM reachability acceptance; identity/coverage and native mechanisms remain under review.

## Bounded native typed bindings

LLVM TLS entries contain typed payloads directly. Bits bindings save/store/restore their bits, without an intervening lexical value-cell pointer. Regional object entries contain offsets with actual owner provenance tracked by WPO; procedure and whole-region associations remain specialized compiler information. Capture analysis follows demanded typed calls, retaining explicit touch dependencies even when they emit no body instruction. Shared-owner constraints are checked against the effective captured/current binding set, including aliases and nested calls.

[Symbolic-context evidence](../tests/cod/language/symbolic-context-validation.json) records the current native traces, rejected programs and TLS IR inspection. Earlier pointer-cell/world reports retain their actual compiler hashes; they remain historical architecture evidence. Module construction still uses independent identity-based worlds, as specified by [D-15](ModuleWorlds.md#identity-records-aliases-and-publication). No new runtime type registry, cross-module editor or ownership mechanism follows from this lowering.

## Module construction and runtime TLS editing

H-245 forbids cross-module module-world editing at compile time and runtime. Each module constructs its own information about types, callables and overloaded methods from explicit source exports; information about identical imported identities merges locally. The source world is not edited, and local integration does not implicitly re-export its contents. Compilation determines the flattened TLS shape and type obligations; runtime dynamically scoped effects change and restore its typed bindings: inline bits or permitted references. [D-15](ModuleWorlds.md) owns the architecture review.

H-238 requires different concrete entity inventories. H-239 specifies one flattened TLS collection of typed symbolic pointers: S identifies the symbol, T independently identifies its required type, and the pointer dynamically refers to a compatible value. That value may originate in another module-world copy. H-240 gives every module its own semantic runtime world; TLS does not duplicate those worlds or their method tables. Identity-compatible rebinding and module information merge are different operations. Capture-by-value, reference lifetime and ownership rules are not replaced by choosing pointers as the carrier. The complete inventory belongs in D-15 and the shared vision in [D-12](GlobalVariablesDoneRight.md), as required by H-241. H-235 requires ordinary CoD calls to avoid exposing the compiler's routing machinery. In particular, manually installing a procedure into a symbol is not a source-language remedy for an internal lookup defect. Existing perform/behave and deliberate effect binding retain their own language purposes.

H-236 requires architecture before local lookup fixes. H-247/H-248 makes that architecture concrete: source exports map public aliases to identities; receiver information is keyed by identity and can have multiple local aliases. Module construction reads source worlds and merges locally. H-246/H-249 directs removal of obsolete technical helpers and autonomous application of these principles within their confirmed scope. Callable-context machinery remains internal; no public world symbol, invocation protocol or special capture exception follows.

## Semantic core supplied by the user

| Construct | Meaning |
| --- | --- |
| `symbol tgt_neuron : T` | Introduce a first-class symbol with strict identity and an explicitly declared value type. The runtime use is `@tgt_neuron`. |
| Import a symbol, optionally under a local alias | Refer to the same exported symbol identity. An alias does not create another symbol. This is distinct from redefining a procedure body with mixin. |
| Vocabulary module | Define typed symbols, primarily abstract types with some supporting ADTs, and their method collections; document their meaning and publish intended shared identities explicitly. A symbol declaration does not supply a default TLS binding. |
| `using` an external procedure | Select source family information and interpretation without extension permission or implicit TLS installation. |
| `import` an external definition | Select an exported source definition with extension permission. An imported helper uses the receiving module's callable implementation context under H-232; source body/non-callable lexical bindings remain. |
| `mixin` an external procedure body | Construct a receiving-context definition with its symbol mapping; runtime capture requires value materialization. |
| `@var_name` | Read the value under the declared symbol identity; WPO requires a supplied dynamic binding. |
| `effect @s <- value` | Scoped ownership/bits transfer; bits symbols require this form. Restore the preceding binding on scope exit; result is void. |
| `effect @s = value` | Scoped non-consuming object-reference rebinding into a region-symbol-backed region; may change region owner while preserving type/lifetime checks. Rejected for bits symbols. |
| Materialize a procedure value | Retain the selected family/interpretation and required lexical/dynamic environment under existing owner rules. Module registration alone creates no runtime capture. |
| `perform @p` | Resolve the procedure, temporarily install its captured effect values in its execution scope, execute its body, and restore the caller's bindings. |
| `behave @p` | Resolve and execute the procedure with its definition's lexical environment and the current dynamic bindings. Its stored dynamic captures are not installed. |
| Function | Lexically defined; compute from explicit inputs to outputs, with no symbolic effect access, `perform` or `behave`. |
| Procedure | May have an ordinary lexical name or a symbolic dynamic binding; may access effects, invoke procedures and call functions in either case. |
| Module top level | Declare symbols, functions and bits globals initialized by literals or function calls; register procedure definitions. Top-level `effect`, `perform` and `behave` expressions are also permitted. Scanning runs none of their execution semantics. |
| Module execution composition | Scan/load every entry dependency first; then compile from the entry, incorporating dependency top-level execution semantics in encounter order. The compiled initialization supplies the module dynamic scope. |
| Required bindings | Checked on the effective caller paths during on-demand whole-program compilation; absence is a compilation error. |
| Expression / `void` | Evaluation yields a result; `void` explicitly indicates no meaningful returned value. The language direction uses expressions without a separate statement category. |

A procedure declaration/body can be registered before its definition event has execution semantics. Here capture at “definition” means the event that creates the procedure value in the resulting execution, not scanning the declaration. Compiling its code or binding an existing procedure value to another symbol with `effect` does not, by itself, recreate its captures. Named procedure declaration forms are described below; additional procedure-value expression syntax remains open.

The supplied `effect` spelling operates within the enclosing scope. It need not introduce a separate brace construct. Repeated/nested bindings restore in the corresponding unwind order. The procedure value used by a call is resolved in the caller's current context **before** its captures are installed; otherwise changing the dispatch symbol could change which procedure is being called.

## Typed symbol definitions

H-184 requires a type for every newly defined symbol, including those used to bind procedures:

```text
symbol a : T1, b : T2;
```

The names in this declaration introduce symbol identities in the module’s compile-time vocabulary. They do not declare runtime lexical values named `a` and `b`; symbolic reads remain `@a` and `@b`. Each identity has its declared value type before binding. A standalone `symbol` declaration does not supply a runtime value, even for an optional type. The compiler checks uses and installations against that type instead of choosing a permanent type from the first effect value.

A combined `proc @p(...) -> RT { ... }` declaration also satisfies the explicit-type requirement: when `@p` has no prior declaration or import, its signature declares the new symbol's procedure type. The identity and methods are established during compilation; a runtime value and binding require an explicit materialization/installation event. No separate `symbol p : ...` declaration is required.

Identity-preserving aliases and vocabulary selections also preserve the declared type. Procedure selection/installation must respect the destination symbol’s declared procedure type. Mixin body maps translate identities at compile time, with typed uses checked in the receiving context. The implemented procedure-type notation is `proc(T1, T2) -> RT`; parameters/results retain their nominal type identities. Aliasing does not silently retype an identity. Broader callable subtyping remains outside this subset.

The executable requires a type on each `symbol` definition. Both `symbol x : T` and `symbol @x : T` are accepted, including comma-separated definitions. A procedure signature supplies the type in a combined declaration. The compiler-provided `std:io` print family remains scalar-polymorphic; binding it to an explicitly typed procedure symbol checks and restricts its signature.

## Shared vocabulary identities

H-242 extends the vocabulary idiom beyond symbols: the vocabulary also defines types, mainly abstractions with a few supporting ADTs, and their method collections. Selecting the same resolved declarations gives independent module worlds shared symbol, type and callable identities. Local aliases preserve those identities; private declarations still require explicit export before selection.

The TLS entry uses the vocabulary symbol identity and its declared type identity. A compatible value supplied from another module-world copy does not require another symbol or a world-specific version of the TLS slot. This is shared identity provenance, not shared mutable module state. [World inventory](ModuleWorlds.md#vocabulary-modules-establish-shared-identities) · [Global Variables Done Right](GlobalVariablesDoneRight.md#shared-vocabularies-independent-worlds)

This idiom requires no new source module kind. A vocabulary may provide typed callable declarations, implemented methods or both under H-257/H-258; no default dynamic value is inferred merely from a symbol/type declaration.

## Symbol identity, variable identity and compiled location

Keep three identities distinct:

1. **Symbol declaration identity:** identifies the exported declaration and its resolved package/module origin. Re-exports and local aliases preserve it. Distinct package versions or independently selected origins remain distinguishable even when every textual name agrees.
2. **Thread binding:** associates that symbol with its current value for this program execution in a thread. Different threads do not share the binding merely because they import the same symbol.
3. **Compiled location:** an offset or other specialized access path chosen for a particular compiled program layout. It is an execution artifact, not the symbol's persistent identity.

A candidate durable identity combines a resolved package-definition identity with a declaration identity. A human-readable version string alone does not distinguish every source/configuration origin; a raw address cannot survive reload. The concrete identity encoding and equivalence of repeated instantiations of the same package remain design choices. Imported aliases must already resolve to the same declaration before layout is assigned.

Under H-114, “first-class” symbol values belong to compilation; aliases resolve before runtime accesses are lowered. The current prototype supports declared identities and their aliases, without runtime symbol objects or dynamically selected identities. Any future runtime symbol-selection or open-ended loading facility needs a separate contract and cannot be inferred from first-class compile-time values.

A use makes the declared symbol part of the demanded program’s binding requirements; it does not replace H-184’s explicit symbol type declaration. H-102 still checks value availability along demanded whole-program paths. No zero/default value, implicit Any, optional None or type chosen by the latest binding is introduced.

## Symbol origins, aliases and dynamic values

H-112 requests explicit origins and local aliases. H-113 excludes lexical symbolic variables and ordinary symbolic assignment. H-114 places symbol identities and module-local aliases at compile time; runtime values bind under symbol identities. H-115 permits an alias-mapping table when importing a procedure body, with matching local spellings needing no explicit entry. H-116 specifies `using/import "<modu>" @src_name as @local_name` and requires the destination symbol to exist beforehand. H-212 assigns the receiving-context redefinition and its body maps to mixin; import now preserves definitions and grants extension permission. These rules determine the relationships below. H-117 supplies `symbol from` and the receiving-symbol-first `with` direction. H-118 authorizes their bounded implementation. The prototype also implements the H-119/H-120 combined using and identity-checked abbreviation under H-121.

### Compilation and execution have different bindings

| Layer | Binding / value | Consequence |
| --- | --- | --- |
| Module resolution | Source selector resolves to a particular module/package origin | Same spelling from distinct selected origins need not denote the same symbol |
| Compile-time symbol value | `symbol @x` creates a nominal identity S | S has no default runtime value and is not a runtime string/name lookup |
| Compile-time alias environment | A module-local spelling such as `@target` denotes S | Aliases disappear from runtime access; multiple aliases of S denote the same dynamic variable |
| Runtime dynamic environment | S has a current runtime value in the thread's active binding context | Reads access that value; `effect` installs a scoped binding and restores its predecessor on exit |

Conceptually, compilation resolves `local spelling -> symbol identity S -> program-specific TLS location`. Runtime `@x` accesses the value at that location. WPO can lower a known identity directly to a slot or specialize away the access; this requires no runtime alias map, string interning or symbol dictionary. S's semantic identity is distinct from the physical offset chosen for one compiled program. No particular identity encoding is selected.

The alias environment is a compiler name-resolution relation, not a set of lexical symbolic variables. A module can call S `@x` and another module can call the same S `@target`; both access the same thread-local dynamic binding. `effect @x <- 2` binds S and yields void; `@x = 2` is invalid. Functions' ordinary lexical parameters/constants remain non-`@` values and do not gain permission to read effects.

In declaration/selection syntax, `@name` identifies which symbol is meant. In an ordinary expression, `@name` reads the runtime value bound under that symbol. Making identities compile-time values does not make those runtime reads compile-time constants. Any later syntax for passing symbol values to compile-time computations must preserve this distinction; no new runtime symbol-object type or user allocation carrier is implied.

### Identity introduction versus procedure installation

A symbol declaration registers an identity, declared TLS contract and local access name. Bare `p` names that symbol object; it is not a second ordinary callable alias. A symbolic method declaration constructs the family’s semantic information. Neither supplies a current TLS binding. `@p` in a read expression accesses the binding; effect-destination syntax identifies the slot without reading it.

| Operation | Semantic result | Runtime installation |
| --- | --- | --- |
| Declare/select a symbol | Introduce or alias its identity and semantics. | None from selection alone. |
| Define/extend a symbolic callable | Construct receiver-local method information under selection permissions. | None without effect. |
| `effect @p;` or combined effect definition | Select a compatible family from the current module interpretation. | Materialize and install that view. |
| `effect @p = source;` | Resolve source symbol semantics and destination compatibility. | Install a materialized source-family view. |
| `effect @p = @source;` | Resolve typed source/destination slots. | Install the existing value and captures. |

Two aliases of one identity address the same TLS slot. Different source/destination identities can intentionally participate in installation; that operation does not alias identities or extend destination methods. Alias selection and installation must be represented separately in the compiler.

<a id="combined-using-and-identity-checked-abbreviation"></a>
### Source selection and explicit installation

Using/import/mixin composes module information according to existing permissions and selected contexts. Exporting a symbol-identified callable publishes its entire method set. Selecting a bodyless contract does not fabricate an implementation. Selection alone installs no receiver value; a dependency can still perform explicit effects in its once-only incorporated initialization.

The prototype’s `using symbol` and `... @source as @destination` paths combine selection with installation. Their old implicit-installation behavior requires coordinated grammar/example migration under H-372/H-373. Preserve intentional cross-identity installation through explicit effects and preserve aliases/contexts under the source-selection contract. A same-spelled distinct identity must not be silently merged; an installation destination is not a compile-time alias. Namespace collisions, shadowing and the final selection grammar remain scoped design work, not an assumed independent `@` lookup escape.

A vocabulary supplies identities without a default binding. In the selected model an integration selects those identities and source semantics, then explicitly installs the intended compatible view. This separation preserves H-109/H-140 once-only incorporation and H-166 resource lifetime while making the receiver’s runtime mutation visible.

<a id="import-time-alias-translation"></a>
### Mixin-time alias translation

H-115 adds a compile-time correspondence between source-body aliases and receiving-module symbols. The following prototype spelling presents the supplied local symbol first and source-body role second. Its outer implicit installation requires H-372/H-373 migration; the inner mapping direction remains the contract:

```text
symbol @step : proc() -> i64;
symbol from "application-vocabulary.cod" @cell as @target;
symbol from "model-vocabulary.cod" @gain;

effect @target <- 2;
effect @gain <- 3;

mixin "model.cod" @update as @step
    with { @target as @tgt_neuron; };
```

Read `@target as @tgt_neuron` as “use my `@target` symbol as the mixed-in body’s `@tgt_neuron`.” The left name resolves in the receiving module; the right name identifies a source-body alias to be satisfied. The clause does not introduce a receiving-module alias called `@tgt_neuron` or read either symbol’s runtime value.

Conceptually, `resolve_mixed_in(@tgt_neuron) = resolve_local(@target)`. An implementation can index the correspondence by the right-hand source alias even though the written clause starts with the supplied local symbol. Resolve this correspondence before checking body requirements, computing captures and lowering runtime accesses. If the body also uses `@gain` and the receiver supplies that spelling, the implicit correspondence is `@gain as @gain`; no explicit entry is necessary.

The consistent reading of `A as B` is “use A in the role/name B.” For an outer `using/mixin`, A is a supplied source procedure and B is its receiving destination. Inside `with`, A is a supplied receiving symbol and B is the source-body role. The scopes differ explicitly; `as` need not mean a universal source-module-to-receiving-module arrow. The table is compile-time input under H-115; its supplier/role direction is implemented under H-118.

The coherent interpretation with H-105/H-115 is that an omitted entry uses the same receiving spelling. A same-spelling local alias may denote a different identity from the source alias: redefinition then uses the receiving identity; it does not equate the two original symbols. The prototype checks this case with independently declared source and receiving identities; spellings themselves do not determine identity. There is no fallback to a source binding when the intended receiving name or required value is unavailable; the missing requirement is diagnosed on the demanded whole-program path.

The mapping is compiler input and creates no runtime alias relation. `using` keeps source body/identities and interpretation; `mixin` translates the body into the receiving context. Runtime materialization, not either selection statement, constructs captures; reinstallation preserves an existing value’s captures. `perform` installs whichever values that resulting procedure captured; `behave` reads current dynamic bindings under that procedure's resolved identities. Capture values can be computed at runtime even though the set of identities and access locations is determined during compilation.

In that prototype form, outer `as @step` chooses the destination binding; the target language must expose installation as an effect, separately from alias selection. The inner `@target as @tgt_neuron` supplies an existing local symbol for a role in the mixed-in body. Neither should be treated as an implicit text-wide rename. Translation must cover symbolic reads, effect targets and procedure-call symbols consistently. Source initialization and other already-created procedures are not silently rewritten by this body's map; H-109's source execution and its own binding checks remain separate.

The prototype requires each explicit source role to occur in the selected body, rejects duplicate source-role entries, and diagnoses attempts to split aliases of one source identity across different receiving identities. Mapping multiple such aliases to one receiving identity works. These are bounded diagnostic policies; the complete language policy for broader mappings remains open. No general symbol-mapping framework or runtime registry is required.

### Syntax and practical use

The prototype `using/mixin "module" @source as @local` spelling exposes origin and destination but still conflates selection with installation. Coordinated migration must retain identity/context while making installation explicit. `symbol @local : T` introduces a new identity; `symbol from "module" @source as @local` names an existing identity. This keeps compile-time symbol declarations together and uses using/import for source-preserving selection and mixin for receiving-context definition. Neither declaration permits `@local = ...` assignment. H-214 excludes a module-prefix namespace and qualified callable names. Grouped selections may be considered without binding a module name; callable use still goes through selected local names.

Keep source provenance at declarations/selections and short names in procedure bodies. A local alias rename accompanied by the corresponding explicit mixin-map update should preserve the selected identities and runtime behavior. Auto-matching source-body spellings is convenient, but a change to one side alone can change or break the correspondence; navigation and diagnostics should show the resolved identities, not just matching text. Ambiguous aliases should require explicit disambiguation instead of depending on selection order.

Symbol selection still triggers source execution under H-109, after complete scanning and during entry-driven composition. The source top-level scope remains alive until process exit under H-166; identity-only selection still does not select a bound resource under H-110’s resource-selection rule. H-140 makes repeated encounters reuse the module incorporated at its first encounter. Grouped selection syntax remains open; direct module-qualified callable expressions are excluded by H-214. A local alias read uses an already selected identity rather than performing another module selection.

Rust's `use path as name` illustrates local aliases for explicit source paths. Racket's `only-in`, `rename-in` and `prefix-in` illustrate selection and spelling controls. These are useful naming comparisons; they do not supply CoD's dynamic-variable, destination-binding, capture or body-mixin semantics. [Rust Reference](https://doc.rust-lang.org/stable/reference/items/use-declarations.html), [Racket Guide](https://docs.racket-lang.org/guide/module-require.html).

### Module origins without local module names

H-214 requires source modules to remain origins of selections rather than local namespace bindings. Using/import/mixin names the source at the selection site; calls use local lexical names, selected operator/intrinsic names or symbolic perform/behave. A single alias preserves the selected source identity; H-230 separately creates identities for extension-module callables and same-alias unions. Diagnostics can display package/module provenance without admitting Module.f or Module.@p as expressions. There is no qualified-call or qualified-definition escape from extension permissions. H-215 proposes a separate explicit import-as declaration for selected method names. H-217 puts implicitly brought method names in a shadowable lower-priority lookup tier; this does not create a module namespace. H-226 resolves explicit versus implicit names before typed method dispatch, preserves operation identity through aliases and rejects a demanded unresolved implicit-name conflict. Detailed signature selectors and alias availability remain design work. [The method-selection review](DomainLanguageDesign.md#type-associated-method-visibility-and-extension) records H-225–H-227 and the remaining boundaries. Preserve this boundary unless an actual serious design obstacle is identified.

### Explicit method aliases and implicit method names

The form `import "module" operation as local_operation;` selects a source-preserving callable alias with extension permission. An alias names the selected callable in its import context. H-230/H-232 gives receiving-context callable adaptation and same-alias unions distinct identities while retaining source method definitions; this is not body redefinition as mixin does. Compile-time aliases must not be confused with an explicit effect that installs a value under a predeclared destination identity. The local spelling is not the signature's identity.

H-217 gives automatically brought method names lower priority than explicit bindings in the same receiving scope. Name lookup first resolves the local binding, then argument-type dispatch uses that operation's visible method set. An explicit binding with no applicable method does not fall back to a hidden implicit name. Ordinary nested-scope shadowing and duplicate explicit-declaration rules remain; no new method is authorized merely by a using selection. Separate import-as can both supply an explicit alias and grant extension permission.

Implicit collisions remain unresolved until a demanded name or imported-family extension needs one identity, so explicit aliases can resolve the conflict without encounter-order winners. H-226 selects local shadowing after using a type and unique-family extension after import; it does not authorize use of an explicit alias before its selection event or new initialization effects. Source bodies retain definition context under using/import; H-232 adapts an imported helper's callable implementations to the receiving module; this must not silently become general lexical rebinding. Methods may be distributed across modules and inherited through abstract <: without an Interface body; only demanded missing implementations are errors. The full signature and visibility rules are in D-07.

A private helper remains a source lexical definition even when its enclosing imported helper adapts public callable dependencies. That selected implementation context propagates through the private body; an unrelated same-spelled receiving definition does not replace it. Direct using/import/mixin of the private name still requires an export. [Native private-helper evidence](../tests/cod/language/private-helper-validation.json) covers these boundaries and preserves typed procedure captures through the resulting call chain.

### Captured callees and selected implementations

A symbolic call uses the selected procedure value in its dynamic binding. Importing the containing helper does not rewrite that stored value into a different implementation. `perform` restores the helper’s captured values; `behave` uses the current bindings. An explicit effect installation can supply an imported procedure whose own ordinary callable dependencies use an adapted implementation context. These are separate selections, with no recapture or runtime method dictionary.

[Native composition evidence](../tests/cod/language/captured-callee-validation.json) uses a source inner procedure computing score(x) + @bias. The source score returns1, the adapter score returns7, the definition-time bias is1 and the current bias is5. A routing procedure invokes the procedure-valued @task; the caller replaces that binding with the adapter-selected inner procedure.

| Call | Native result |
| --- | --- |
| Direct source-preserved helper | 6 |
| Direct import-adapted helper | 12 |
| Perform the source-preserved or import-adapted router | 2 for each; both restore the original captured @task value. |
| Behave through either router after explicit @task rebinding | 12 for each; both use the selected current procedure value. |
| Perform the explicitly adapted inner procedure | 8; adapted score7 with its retained captured bias1. |
| Read caller @bias after the calls | 5; temporary captures did not replace its binding. |

The complete generated sources and output are preserved in the record. This checks composition of the existing capture/context contracts; it does not select a policy for competing dependency views or establish lexical-call acceptance; H-257 and K-400 supply that separate contract and evidence.

### Callee selection and argument rebinding: native observation

[The argument-rebinding record](../tests/cod/language/call-argument-observations.json) checks the current lowering when an argument changes the symbolic callee binding. The prototype resolves the procedure value first, evaluates arguments in the caller environment, then invokes that selected value. Perform uses its stored captures; behave uses the environment after argument evaluation. Argument-side effects belong to the enclosing scope and remain after the call; restoring the procedure body does not undo them.

The example selects an exported read procedure with captured bias10 and defines an alternate with captured bias20. Passing an argument that binds bias31 gives41 under perform and leaves caller bias31; binding32 gives64 under behave and leaves32. Rebinding the callee symbol to the alternate inside an argument does not change the in-flight selected read: it returns43 under perform or68 under behave. Subsequent calls use the alternate, producing3301/3401 with current bindings or2001 with its capture.

This is actual prototype behavior, not a newly confirmed language-wide call-order rule. H-148 fixes each argument's conversion before the next argument; it does not expressly place symbolic callee resolution relative to effectful arguments. That timing distinction can be reviewed with the local value/call namespace question. The probe adds no callback-call syntax, runtime dispatch or changed capture mechanism.

### Explicit conformance: declaration permission and consumer visibility

H-218 permits `data D <: A;` to add conformance for an existing data identity, including declarations in different modules and more than one abstraction. Available signatures never infer conformance. H-220 requires an adapter to import the ADT before declaring such a relation. Using the ADT is insufficient to authorize the declaration; using the adapter is sufficient for consumers to obtain that declared relation. Visibility to use and permission to extend are separate facts.

Source-preserving functions/procedures retain their source body, non-callable lexical bindings and captured values. The native implementation retains source-family provenance while selecting ordinary methods under distinct module/context identities; it does not accumulate every caller-visible method. H-230 requires distinct extension/union identities; H-232 gives using the preserved selected context and import the receiving implementation context; compound alias/context conflicts remain to be specified. H-229 preserves established conformance of concrete arguments through broad constraints and nested traversal, without publishing unrelated caller type relations. Operator/intrinsic candidate lookup remains lexical. Mixin checks the redefined body in its receiving context: interface use needs the corresponding adapter view, and declaring new conformance needs import permission on the ADT. This is independent of perform/behave's dynamic-value selection. No new effect binding, body capture, region permission or runtime type field follows from conformance.

D-07 illustrates selecting the data type through an adapter with using. Explicit export/re-export and export import have a bounded implementation; compound publication choices remain pending. Neither requires module-qualified calls or a runtime interface registry. A source body may use its own declared relations without exporting those facts through every callable selection. Once-only incorporation and explicit alias availability retain their existing rules.

### Explicit publication and extension identities

H-231 makes module resources private unless declared with export. Using/import/mixin, symbol selection and implicit associated-method selection must respect that boundary; an alias or selected type cannot bypass it. Public source bodies can use their own private dependencies without making them directly selectable. H-233 requires a receiving alias to have its own explicit export before it can be selected again. The confirmed shorthand export import performs the normal import and publishes everything that statement selects, including its associated method selections, within source privacy constraints. The proposed export list and outstanding conformance/conversion bundle details are in [D-07](DomainLanguageDesign.md#explicit-exports-and-private-module-resources); the prototype supports single-name export declarations and export import, while grouped-list and broader bundle syntax remain outside it.

H-230 requires distinct callable identities for extension modules. Different local aliases preserve their selected method sets; same-alias composition of versions of the same semantic callable creates a virtual union identity in the selecting module. The required base/extended/indirect example is1,7,1. The [helper-context example](DomainLanguageDesign.md#extension-module-callable-identities-and-local-unions) has a confirmed result under H-232: using base twice gives2, while import base twice gives14 through the receiving score implementation. A later using preserves the selected exported context. This adaptation does not change symbol identity, dynamic bindings, capture mode, source globals or general lexical body resolution; mixin remains body redefinition. H-253 requires conflicting inherited and receiver-selected overloads to reject during import adaptation; no alias/identity priority silently selects a winner. Explicit arbitration syntax remains future design.

### Implementation boundary and acceptance

The native prototype selects existing definitions with using/import and redefines symbolic procedure bodies with mixin. Source-preserving selections retain types, lexical resolution and captures; symbolic destinations must be declared before selection. Omitting as requires matching source/destination symbol identities, while explicit as can install the source-defined procedure under a different predeclared destination. Combined identity introduction remains using symbol. Direct named-callable extension requires import under the extended local alias. The native implementation keeps semantic-family provenance separate from extension/local-union identities and checks exports for every cross-module selector. Original methods with no ordinary callable dependencies deduplicate across contexts; distinct observable contexts are not silently erased. Import stores a receiving implementation context, using preserves the selected context, and scope restoration prevents cross-alias leakage. Selecting a type also brings its source-visible associated methods at lower name priority, including result-only factories and inherited applicable signatures. Local ordinary declarations shadow using-only implicit names or extend a uniquely imported family. Original methods deduplicate across paths. Cross-alias grants and extension of unrelated merged callable families remain outside the prototype. These limits do not change the separate declaration rules for compiler operations.

Mixin translates source-body symbol roles to receiving identities before checking required bindings and collecting captures. Repeated mixing uses the current provider's aliases; it does not rewrite the source initialization or existing definitions. Body maps are rejected on using/import, and built-in procedures have no body to mix. Lexical procedures may also be mixed under ordinary local names and subsequently installed into symbolic bindings. Operation-overload mixin remains outside this subset; mixin does not copy type definitions, function definitions or globals. UoM selection retains its separate using-units contract.

[The module-selection checkpoint](../tests/cod/language/module-selection-validation.json) records source-preserving and receiver-context calls, captured/current values, type aliases and lexical procedure values in one native case; existing mapping and region-factory cases now use mixin. The full batch passed 153/154 checks, with the sole mismatch an obsolete expected diagnostic for using-with-map; updating that expectation and rerunning the affected case plus frontend-only validation passed 2/2 without another compiler change. All five composition cases passed, including the 96-module regional mixed-body chain. This is evidence for the selected subset, not abstract conformance, implicit method names or extension permissions.

## Vocabulary modules and procedure reuse

Vocabulary modules publish common symbol/type identities and callable contracts/methods. Primarily abstract types and supporting ADTs allow independent worlds to agree on meaning. Selecting a symbol supplies no default TLS value. Export of a symbol-identified callable includes its whole method family; view compatibility is separate from privacy.

| Semantic selection | Body and non-callable lexical context | Callable interpretation |
| --- | --- | --- |
| `using` | Preserve the source definition. | Preserve its selected interpretation; no extension permission. |
| `import` | Preserve the source body and non-callable bindings. | Admit receiver-local extension and the selected receiving interpretation. |
| `mixin` | Redefine the body in the receiver. | Resolve under that receiving definition, with explicit symbol mapping where supplied. |

These compile-time operations neither install a TLS value nor snapshot runtime bindings. A runtime operation that constructs a selected callable view supplies its environment under the capture contract; installing an already constructed value preserves that environment. Source lexical captures and actual owners cannot be replaced by same-spelled receiver variables. Family environment construction and grammar migration must reconcile these requirements rather than deriving captures from scanner state.

Invocation remains separate: `perform` temporarily replays the selected value’s captures, while `behave` and ordinary direct procedure calls use current dynamic bindings. For a value captured with `@x = 1`, reinstallation after `@x = 7` retains 1 for perform; a fresh value constructed with 7 can capture 7. Both use the value’s selected family/context, and behave uses the current caller binding. These examples identify the construction event explicitly and do not ascribe capture to import itself.

H-230’s base/extended/indirect `1,7,1` and H-232’s using/import interpretation remain acceptance constraints. Source body/non-callable bindings and independent identities persist. Equivalent definitions may share machine code, but merely removing the one-method guard or renaming an internal identity cannot implement family values.

## Functions and procedures

H-197/H-198 establishes functions as lexical callables that may operate on explicit memory and explicit-argument system intrinsics without reading or writing dynamic bindings. Procedures may additionally use dynamic context. The ordinary function contract is mutable; a future const-function form would be a separate design.

A function definition binds a lexical name and cannot access dynamic effect variables. A procedure may have an ordinary lexical name or belong to a symbol-identified module family. Explicit effects install family views into dynamic scope. Both procedure forms may use effects; lexical naming does not make a procedure a function. A function cannot call either procedure form, directly or indirectly.

A lexical procedure keeps its lexical definition context and H-180’s captured values of the dynamic variables it uses. Installing that existing procedure with `effect @operation = local_procedure` preserves the selected definition and captures; assignment is not another definition event. The destination identity must have an explicitly declared compatible procedure type. Procedure `mixin` is the distinct operation that redefines a source body in receiving context.

H-185 specifies the call forms: `p(...)` resolves a lexical procedure; `perform @p(...)` and `behave @p(...)` resolve a procedure through its symbolic binding. H-189 makes direct lexical calls use the caller's current dynamic bindings. `perform` temporarily installs the selected value’s captured values; `behave` uses current values. `perform p(...)`, `behave p(...)` and direct `@p(...)` calls are invalid. Pure functions cannot invoke procedures through any form.

| Call form | Callee lookup | Established invocation rule |
| --- | --- | --- |
| `p(...)` | Ordinary lexical procedure name | Use current dynamic bindings; no symbolic callee selection. |
| `perform @p(...)` | Procedure value currently bound to the symbol | Temporarily install that procedure’s captured dynamic values. |
| `behave @p(...)` | Procedure value currently bound to the symbol | Use current dynamic values. |

### Lexical calls and shadowing

H-382 includes inferred owner names: `where {owner o}` introduces a const lexical binding even without region-specific access. It shadows an outer symbol `o`, so `@o` rejects. Binding immutability neither freezes target contents nor grants a region capability; see [owner bindings](RegionalOwnership.md#region-owners-with-root-policy-access).

H-381 also applies nearest-binding lookup to symbolic access. If a parameter/local p shadows the module symbol object p, @p rejects because that nearest binding is not a symbol. Reads, effect destinations, touch, perform/behave, deferred uses and typeof inspection follow the same rule. Cached identities and capture pruning cannot bypass it. A separately defined callable still uses its own lexical context, not its caller's local p. Same-scope module-category collisions and general symbol-value storage remain outside this bounded rule.

H-257 requires ordinary call syntax to respect the nearest lexical value binding. A procedure parameter or local procedure value supplies the callee; a same-spelled module callable does not bypass it. A non-callable local value makes the call ill-typed. Direct calls retain H-189's current dynamic bindings. Symbol-only `perform`/`behave` and the function/procedure effect boundary are unchanged.

```text
proc selected(x:i64) -> i64 { return x + 20 }
proc callback(x:i64) -> i64 { return x + 1 }
proc apply(callback:proc(i64) -> i64, x:i64) -> i64 { return callback(x) }
# apply(selected, 3) must yield 23.
```

A parameter or local declaration that shadows an outer local, module callable or module value/type binding emits `[-Wshadow]`. `-Werror=shadow` or the general warnings-as-errors option makes it an error. Same-scope duplicate declarations remain errors. No user-visible call protocol, escaping closure representation, general function-value type or runtime callable dispatch is introduced.

### Callable declarations and methods

A callable identity can exist without any executable methods. Its signature declarations and its methods are separate module-world information. Using/import, explicit exports, aliases and receiver-local merging apply to both; selecting or exporting an unimplemented declaration does not execute or fabricate a body. A demanded call requires an applicable implementation. A function declaration promises a function implementation with a compatible result for its applicable inputs; a procedure declaration supplies the corresponding procedure contract. These promises are checked on demanded calls, alongside the selected method's own signature.

H-258 requires parameters and an explicit return type on a bodyless declaration; an untyped identity-only `function score;` is not supported. Generic declarations and methods use compile-time type parameters, including abstract upper bounds, and a final variadic parameter:

```text
function score(x:T) -> i64 where T <: Metric;
function combine(first:T, rest:T...) -> T where T;
proc dispatch(first:T, rest:T...) -> void where T;
function pair(x:T, y:U) -> i64 where {T, U};
```

Type parameters are inferred from concrete argument types at a demanded call. Repeated occurrences constrain those types to agree; bounds use the caller's selected conformance information. A type parameter that cannot be inferred from inputs is diagnosed. Unique-most-specific dispatch compares signature constraints, including concrete types, bounds, repeated-variable relationships and fixed versus variadic arity, independently of declaration order. Equally specific candidates remain ambiguous. This bounded prototype does not supply generic ADT constructors, explicit type-argument calls or a complete higher-rank type system.

Within a method, `len(rest)` gives the tail count, `rest[i]` reads an element and `other(first, rest...)` forwards it. The finite argument pack is compiler metadata with each argument's existing provenance; it is not a user heap object. A dynamic index into regional arguments requires a common live owner. A pack cannot escape its call scope or become a TLS value. Empty tails are valid when their element type can be inferred from another argument or is explicitly concrete. Automatic conversion still completes for each argument before the next argument's effects.

### Evaluated bits and storage places

H-257 directs the compiler model toward LLVM register optimization. An evaluated bits value is an SSA value; a storage place denotes a typed address in a tracked owner. Reading a place produces the value at that point. Writing the place does not retroactively change that SSA value. A retained regional reference permits a subsequent read to observe intervening writes. Const non-bits access therefore does not imply immutable underlying memory.

LLVM lowering uses typed places for referent, field and element access, with common read/write operations. Regional aggregate views retain their offsets and owners; relative reference fields keep their existing encoding. Bits operations and variadic argument selection do not require artificial memory cells. This internal separation does not introduce `owned`/`const`/`make` source syntax, change the assignment-result contract, extend owner lifetime or select RTTI. The complete ordinary-region lifecycle is the separately deferred fourth work package.

### Direct symbolic procedure declaration

```text
proc @p(arg:T) -> RT { ... }
effect @p;
perform @p(argument);
behave @p(argument);
```

The method declaration defines or extends the symbol’s module-world family without a runtime binding. If no symbol exists, its explicit signature can introduce the nominal identity and declared contract under the existing declaration rules; an existing/imported symbol retains its identity and permissions. Declaration promises and compatible view checks remain distinct from method addition.

Bare `p` denotes that symbol object, not an additional ordinary callable alias. Materializing a family view is a separate runtime operation; `effect @p;` performs that operation and installation without adding a method. Combined `effect @p(...) { ... }` requests both stages. Capture belongs to materialization, and installation of an existing callable value preserves its captures. Functions cannot gain dynamic access through either spelling.

The existing `proc(T1, T2) -> RT` view contract remains typed. Several methods do not make a callable value ambiguous: invocation dispatches by actual types within its selected interpretation. The compiler’s present one-method construction guard is an implementation gap, not a restriction of this contract. General view compatibility, concrete-type evidence and grammar migration are tracked in [D-21](AheadOfTimeMultipleDispatch.md).

### Callable context and memory access

The procedure’s name binding, its body’s lexical environment and its use of dynamic effects are separate facts. Registering a body during scanning does not perform captures; its runtime definition semantics still follow the existing scan/incorporation boundary. Nested-definition placement, arbitrary lexical captures and ordinary-name overload collision policy require their own bounded design. No heap closure or new capture lifetime follows merely from adding lexical procedure names.

A function uses explicit inputs and may read/edit their reachable memory. It may call functions and use permitted lexical definitions/constants, but cannot read or rebind an ambient @symbol, invoke a procedure through another spelling, or obtain hidden dynamic context. Explicit-argument intrinsics can have external consequences under H-197. No referential-transparency or read-only guarantee follows from the function keyword. Lexical captures must not smuggle dynamic access or an invalid storage lifetime into a function.

Procedure signatures may have explicit arguments and results as well as effect requirements. “Procedure” does not imply a void return. Function-valued arguments require functions; passing a procedure there cannot bypass the effect restriction. Methods need the same callable-kind distinction.

The checker should retain each procedure's required symbol identities and read/write/binding use through calls. This is a proposed implementation of the user's distinction, not a commitment to a general row-polymorphic effect calculus. A small finite requirement set is sufficient for the first examples. Foreign calls need a trusted declaration of their effects; an opaque native call is not presumed pure.

Constructing a description and executing its requested operation remain distinct. A function may construct/edit regional data through explicit resources once the source allocation contract exists. Looking up @region, or creating a procedure that captures current effects, accesses dynamic context and therefore cannot occur inside a function. Context exclusion applies transitively through helpers and foreign intrinsics.

The SNN support layer may use functions for numerical work and explicit array edits, with procedures supplying ambient target/frame resources. This callable boundary selects no neural transition or learning algorithm. H-200/H-201's borrowing, local-bits and const-value contract needs its own source implementation; K-311 validates the existing explicit-reference memory subset only.

## Captured values and call behavior

Let `D` be the caller's current symbolic bindings, and let a procedure value `p` retain its selected family/interpretation, lexical environment `L`, and captured values `C` indexed by its required symbols. These are semantic components, not a prescribed heap object layout.

```text
perform @p:
    save caller bindings for the identities in C
    install C in the procedure's execution scope
    select the method using actual arguments in the retained family/interpretation
    evaluate that method under L and those temporary bindings
    restore the saved caller bindings on scope exit

behave @p:
    select the method using actual arguments in the retained family/interpretation
    evaluate that method under L and the current dynamic bindings
```

Only the captured identities are temporarily installed. `behave` does not erase lexical captures or turn lexical names into dynamic names. The callee can establish further `effect` bindings normally in either call mode.

For a scalar example, evaluate `effect @x <- 1`, materialize/install a view under `@p` whose selected method reads `@x`, then evaluate `effect @x <- 2` in the caller scope:

| Observation | Result |
| --- | --- |
| `perform @p` | 1 |
| `@x` after that call | 2 |
| `behave @p` | 2 |

If the body evaluates `effect @x <- @x + 1` and then reads `@x`, its local binding is restored on exit in **both** call modes. With capture 1 and caller value 7, two `perform @p` calls each observe 2; two `behave @p` calls each observe 8. The caller remains 7 after every call. `behave` omits capture installation; it does not make the body's own scoped writes permanent. Installing a captured scalar does not mutate the stored capture itself.

Scope exit includes the selected language's structured returns and failure/unwind paths. Lowering must arrange restoration on each such path. It does not promise to run cleanup after process termination, or roll back effects performed through referenced objects.

### Regional resource capture preserves owners

Three different facts must remain distinct: the symbol identity used for lookup, the value captured at procedure definition, and the region’s ownership/lifetime. A procedure captures the then-current value of each required dynamic variable under its resolved identity. It does not reread an old variable’s later value as a substitute for that snapshot. For a region-backed resource, the captured value identifies the existing resource; it is not a copy of the object graph or a new independent owner.

Suppose `@work` denotes R1 when `@p` is defined. The caller subsequently shadows `@work` with R2 while both owning scopes remain alive:

| Point | Region reached through `@work` | Ownership and lifetime |
| --- | --- | --- |
| Before `perform @p` | R2 | R1 and R2 retain their respective owners. |
| Inside `perform @p` | Captured R1 | R2 is hidden through this variable only. Its ownership and cleanup responsibility are untouched. R1 also retains its existing owner. |
| After `perform @p` | R2 | The caller binding is restored. Neither region was released by capture replay or shadow removal. |
| Inside `behave @p` from that caller | R2 | The procedure uses current values; its saved R1 is not installed. |

A previous reference or another valid binding to R2 is not invalidated merely because `@work` is shadowed. Mutating R1 through the captured resource changes the live graph; leaving the call restores lookup, not graph contents. Removing the temporary shadow does not free R1 either. Only actual ownership/scope events release resources, and a region newly acquired within the call follows its own owning scope.

The supported typed-reference prototype also retains compile-time interface facts associated with the selected reference value. Captured/current lookup must not erase those facts, and rebinding must not accumulate the previous value's guarantees. [Native evidence](../tests/cod/language/effect-conformance-validation.json) distinguishes Weighted captured dispatch from Alternate current dispatch without publishing either adapter relation globally. This is compiler evidence propagation; it adds no runtime tag, reference permission, graph snapshot or lifetime extension.

Captured resources must be valid for their uses. H-180 supplies owner preservation during temporary shadowing; it does not choose a reference-counting system, an implicit lifetime extension for an escaped procedure, or a region-copying snapshot. Definition-time capture, temporary replay and explicit ownership transfer must have different checking/lowering paths. H-181’s restriction on moving a resource out of a dynamic owner must not reject legitimate captured replay.

### What “uses” includes

Capturing only the symbol spellings appearing directly in a body is insufficient. If `p` executes `behave @q` and `q` reads `@gain`, then `@gain` can be a dynamic dependency of `p` even when the name never appears in `p`'s source. Conversely, a `perform` call can supply a callee's dependencies from that callee's own captures.

The required-symbol analysis must account for dispatch, transitive calls, higher-order arguments, reads/writes and bindings that already supply a value inside the procedure. Native regional snapshot demand follows those same checked uses through copies and recursive results; an unused definition-time binding alone does not force a distinct native context. Final sharing retains every discovered read/touch dependency, including uses propagated through a reused native body. A current-context helper called under perform receives the active captured overlay and its actual invocation bases; equivalence must preserve owner aliases and the effective shared-region groups. Replayed procedure values follow the same native-input proof as callable arguments, including complete image schemas, retained owners and returned-image independence. Captures are taken before later control-flow branches are known; a conditional read is still a potential use unless checking proves it unreachable. A procedure that provides its own binding before every access need not capture an irrelevant outer value.

CoD checks the demanded whole-program execution with its caller paths available, rather than compiling a module against an unknowable future caller. A finite summary or fixed-point analysis is a candidate way to handle recursion and branches; the semantic requirement is not to enumerate an unbounded runtime stack. Missing requirements cannot be hidden behind an empty summary or deferred to an ordinary runtime lookup failure.

### Call-path binding checks

H-102 locates checking at compilation of the demanded call paths. The compiler follows module initialization, callers, scoped bindings, callable selection and invocation mode; declaration text in an isolated module is not the compilation unit.

- For `behave @p`, required effects must be available in the current effective dynamic scope along each reachable path to their use, accounting for bindings established inside the body.
- For `perform @p`, the values it must install must have been obtainable at the applicable definition event. Missing required capture values are compilation errors, with no fallback to the caller's values.
- Branches and possible callees are checked in their corresponding contexts. Knowing caller paths does not require knowing runtime branch values; a reachable path lacking a required binding cannot be excused by another path that supplies it.

Deferred current-context invocation uses the bindings present when its owning full scope drains. A conditional registration requires those bindings on every path reaching that action, including failures after registration; a path that never registered it supplies no invocation obligation. Binding and owner proofs share the queue's branch choices. Later rebinding remains observable, iteration effects restore at their own boundary, and temporary effects inside an explicit cleanup scope restore before later actions. These guarantees do not make a missing binding available to an independent callable or create a definition-time capture. [Deferred-context evidence](../tests/cod/language/defer-context-validation.json) records bounded native acceptance.

The reusable-operation case is therefore checked through its complete definition/call paths and chosen mode. This does not select a runtime missing-value sentinel, implicit zero value, or a separate-module rejection policy. The precise analysis must preserve by-value captures while avoiding requirements for values that the checked path does not use.

### Module scanning and execution composition

H-106 separates registration of the complete dependency set from compilation of execution semantics. The module top level admits `symbol`, `function` and **bits global-variable definitions whose initial values are bits literals or function calls**. It also permits `effect @name <- rhs`, `perform` and `behave` expressions under H-107. A top-level `procedure @proc_name {}` registers its definition body during scanning. Global initializers and executable top-level expressions belong to module execution; the scanner records them without invoking functions/procedures, installing effects or capturing runtime values.

| Phase | Required work | Meaning of a top-level procedure definition |
| --- | --- | --- |
| 1. Scan/load | Scan the entry and all its dependency modules, registering the permitted definitions and retaining initializers, executable expressions, procedure bodies and their origins. Finish this phase before compiling entry execution semantics. | Available to resolve and compile; no procedure invocation, dynamic binding installation or value capture occurs merely from scanning it. |
| 2. Compile execution | Start from the entry module's execution semantics. As this compilation encounters dependency modules, incorporate their top-level execution semantics into the resulting program in that encounter order. | Its methods remain registered semantic information. Explicit materialization/installation and other executable expressions join module initialization under their selected contexts. |

For a non-recursive example, suppose scanning loads dependencies A, B and C in that order, but compiling the entry encounters B and then A. The dependency execution-semantics incorporation order is **B, A**, independent of the earlier scan order. C's scan alone does not cause its execution semantics to be incorporated. Any cross-module reference qualifies as a trigger under H-109, including a vocabulary symbol, `using`, `import`, `mixin`, or a function/global reference. H-140 incorporates each module only once. These are compilation encounters after scanning, not immediate execution of user code by the compiler. A diamond dependency reuses the incorporated module; its initialization effects are not duplicated.

The native diamond fixture uses export import to publish a selected symbolic procedure and later selects it under another alias after changing the caller’s binding. Its shared initializer runs once and perform still sees the original capture. [Recorded order evidence](../tests/cod/language/module-order-validation.json) separates the native output from the compiler’s verbose scan/incorporation report; --check emits no user-program output. The same record permutes local method declarations while keeping imported/type/conformance visibility fixed, without changing encounter availability.

“Compile execution semantics” means constructing the resulting program, not running user initialization effects in the compiler. Capture values follow runtime callable materialization in that program’s execution; they are not snapshots of the scanner's state. Required bindings are still checked at compile time over the resulting caller/initialization paths. Declaration availability therefore does not prove that a required dynamic value is available.

The entry module and every incorporated dependency retain their top-level scopes until process exit under H-166. Finishing a dependency’s initialization body does not end that scope or release its still-held regions. Ordinary nested procedure/block scopes keep their own exit boundaries. A region explicitly transferred out of a top-level holder need not remain allocated until process exit.

Scope lifetime and active binding lookup must be distinguished. H-110’s resource-selection boundary remains: `using`/`import` selects the source-defined procedure/resources and `mixin` defines the selected body in the receiving context. Merely selecting a vocabulary identity does not select its bound resource. H-166 changes lifetime; it does not explicitly replace these selection rules with automatic ambient exposure of every dependency binding. The implementation must retain module contexts without treating initialization return as their destruction. The precise retained-context representation and shutdown ordering remain open.

A source-defined procedure can therefore refer to a region still held by its module top level after initialization completes. Its captured references remain subject to the owning region’s lifetime, and a capture itself does not retain an independently ended nested region. Source versus receiving contexts, capture-by-value and current-binding `behave` remain distinct.

**Implemented boundary:** module-owned regions remain live after initialization and are released at normal program exit. Returning to the incorporating module restores its active binding lookup; the retained source context and its owners remain available through explicitly selected resources. Ordinary nested blocks and calls still release their local owners on exit. The source-module capture fixture reads its region after initialization; the nested-region escape fixture remains rejected. [K-295](DesignAuthorityRegistry.md#k-295) records the implementation and native evidence. This synchronous prototype has no user shutdown hooks or Actor lifecycle policy.

The [guarded-owner fixture](../tests/cod/language/short-circuit-owner-validation.json) uses an explicit scoped right operand of && to acquire each inner region and a skipped right operand of || to verify that no extra acquisition occurs. The independent harness observes release order1,2,3,0 and eighteen reads against live owners. Unscoped conditional rebinding remains rejected; short-circuiting does not change captured/current lookup or extend a nested owner's lifetime. This is expression control flow within the existing module initialization, not a rule for conditional module incorporation.

Any kind of cross-module reference encountered during execution compilation triggers incorporation of that module's execution semantics, including `import` and `mixin`; definition reuse does not exempt source execution. H-140 requires once-only incorporation in first-encounter order after scanning. Repeated encounters reuse the selected resources and do not produce another initialization body or recapture under a different caller. Cyclic initialization and branch handling remain separate questions; the prototype diagnoses a demanded initialization cycle. This limit does not select permanent cyclic-initialization semantics.

For a bits global, a bits literal and a function call producing the required bits value are permitted initializer forms. A procedure invocation is not thereby a function-call initializer; top-level `perform`/`behave` remains a separate allowed expression form. Function initializers retain the no-dynamic-context contract, even though their evaluation belongs to module execution. This permits computed model constants without introducing scan-time user execution.

Top-level `effect` establishes bindings in the module's execution scope, while `perform` and `behave` follow their ordinary capture/current-binding semantics there. Compiling these expressions places their behavior into the resulting program; it does not run them during dependency discovery. Bare symbolic method declarations contribute no implicit binding; combined effect declarations retain both stages. The registration/execution distinction applies to expressions as well as procedure declarations.

The exact literal grammar, global mutability/sharing and physical realization remain to be specified alongside H-99's storage boundary and H-197/H-198's function boundary. Permitting bits globals or effectful module execution does not by itself introduce native-heap user objects or ambient mutable global reads in functions.

### Whole-region owning bindings

H-190 confines intrinsic ownership to whole regions. Intra-region free lists, object pools and slot reuse are user-library policies under ordinary typed access; they do not introduce per-object language owners or change region release/transfer. A still-live region does not by itself validate a reused slot's business identity. [D-14](RegionalOwnership.md#remaining-syntax-semantics-and-idiom-validation) records the missing whole-region lifecycle and library-sufficiency validation.

The following lifetime constraints are confirmed; the source-language meaning of an ordinary owner is still incomplete under H-183. [D-14](RegionalOwnership.md) designs the required declaration, access, construction, optionality and transfer forms together. H-262 confirms that an ordinary named owning source becomes statically unavailable after transfer; it need not use optional storage. H-182’s optional-source extraction remains a separate proposal. H-263 preserves borrowed views while the new local owner remains alive and permits access; ending that scope or sending the region out revokes access. H-264 selects the first typed let/make form. H-265 selects unchecked ultimate OOM and requests try/catch/finally planning. D-17 fixes consuming call/result boundaries and exception control; K-414 records the completed bounded local lifecycle. General qualifiers, owner reassignment and transport remain separate.

H-177 permits ordinary owners, and H-179 requires an ordinary variable to relinquish ownership when it installs a region-backed dynamic resource. H-181 prohibits consuming ownership out of a region-valued symbolic binding. Such a binding cannot be moved into an empty state; permitted shadowing and H-180 captured replay preserve its existing owner. The prohibition also applies when an explicitly optional dynamic value contains a region: matching or accessing the present case must not become a hidden ownership extraction.

This is distinct from ordinary packet ownership. An ordinary owner may be consumed by its separate move/send/result protocol. A dynamic resource remains held for its owning scope, including while hidden by another value. Ending an actual owning scope releases its own resource; removing a perform shadow does not release either pre-existing resource.

**Implemented boundary:** `effect @b <- @a` and `effect @a <- @a` reject whole-region values already held by a dynamic binding. Captured replay uses its own non-consuming path and preserves both captured and shadowed owners. The compiler no longer clears source owner slots during effect installation.

The executable supports typed make, ordinary owning locals/calls/results and consuming installation into a dynamic resource under D-17. Its checks reject escaped resources and incompatible borrowed-owner joins. The language-wide ptr/region-declaration migration, optional extraction and packet send/result paths remain separate work.

### Exceptional control and scoped effects

[Exception control](ExceptionControl.md) supplies H-267/H-269's accepted try/catch/finally, unchecked failures and retained explicit causes. Leaving an effect scope exceptionally must restore its preceding binding and release only resources owned by that exiting scope. Perform replay must preserve pre-existing captured and hidden owners. Neither catching an exception nor restoring an environment snapshot can resurrect a consumed ordinary owner. Functions retain their exclusion from dynamic bindings/procedure calls; explicit-memory failure propagation does not grant new effect access. K-414 supplies focused exceptional-path evidence, including captured/hidden resource preservation; it does not qualify wider closure or concurrency contracts.

### Binding availability and optional resource values

The declaration’s value type and the availability of its dynamic binding are separate facts. H-181 permits a direct region-reference declaration and an explicitly optional one:

```text
symbol @required : ResRegion;
symbol @optional : Maybe<SomeRegion>;
```

These show the user’s desired typed-symbol form; typed symbol annotations are implemented, while general `Maybe` construction/matching and domain-specific resource types remain design work. The names denote domain resource types, not a selected generic region layout.

| State at a demanded read | Meaning |
| --- | --- |
| No supplying binding for `@required` or `@optional` | Compilation error. A symbol declaration, even with a Maybe type, is not an initial value. |
| `@required` has a valid supplied region | Read the direct region-reference value. No implicit optional wrapper or moved-out empty state is needed. |
| `@optional` is explicitly bound to the absent alternative | A valid bound business value. The program handles that alternative under its sum-type rules. |
| `@optional` is explicitly bound to the present alternative | A valid bound resource value; accessing that alternative does not permit taking ownership out of the dynamic binding. |

WPO checks the effective definition/call paths. Behave uses the active caller bindings; perform can supply captured values by its temporary shadows, whose origin and resource lifetime must also be valid. Required bindings cannot appear merely because their identities/types were declared, or because some different runtime path supplies them. The type system need not encode this lookup availability by rewriting every `T` to `Maybe<T>`, and no runtime presence bit or null sentinel follows from the rule.

For an explicitly optional resource, shadowing a present value with the absent alternative does not destroy the hidden region. Leaving that shadow restores the preceding value and its preserved ownership. This is the same H-180 resource-lifetime rule, not optionality-driven mutation or extraction of the underlying owner. Exact sum syntax/layout remains independent implementation work.

### Expressions and `void`

A symbolic read is `@var_name`; a symbolic binding write requires `effect @var_name = rhs` for rebinding or `effect @var_name <- rhs` for ownership/bits transfer. Plain assignment, compound assignment and increment syntax must not provide alternate symbolic binding writes. H-282/H-283 separately proposes a transfer expression `@object <- rhs` that mutates the referent rather than the binding. Reads and writes participate in expression evaluation; procedures may also return meaningful results. `void` explicitly indicates no meaningful result, rather than a missing required binding, an implicit nullable value or another allocation carrier.

H-280 admits `=` for scoped rebinding alongside `<-` for scoped transfer; bare symbolic binding assignment remains invalid. H-281 requires `<-` for bits and `=` for object references, as specified in [the storage contract](#symbolic-storage-and-regional-provenance). H-204/H-205 additionally develops an ordinary ownership/bits transfer operator with the same glyph. Prefix effect retains scoped installation/restoration and its typed procedure-binding case; ordinary bits stores persist, and ordinary owner moves require a separate admitted owning destination. Captured replay remains non-consuming. The prototype supports `<-` in effect bindings, fresh owning declarations and make root initializers; parameter conversion-route arrows retain their independent grammar. General transfer overloads, owner reassignment and reference-assignment migration remain unimplemented.

Every effect expression evaluates to void. Read `@x` explicitly after `effect @x <- 3` when the value is needed. This applies equally to one binding, a grouped binding and combined callable installation; it supplies no implicit tuple, last-value result or chain of transfers. Binding lifetime follows the enclosing scope, not merely evaluation of the effect expression. Ordinary assignment/transfer overloads retain their separate destination-view result contract.

H-148 specifies callable argument order: evaluate an argument exactly once and finish its selected automatic conversions before evaluating the next argument. A failed conversion prevents later argument effects. Functions, procedures and overloaded operator spellings share this rule; compile-time method/route selection does not execute those effects. The callable body begins after all arguments and conversions finish. [D-11](ScalarUnitsAndIndexTypes.md#argument-evaluation-and-automatic-conversions) records the conversion-specific scope. Other open expression conventions are not settled by this decision.

### Local value shadowing

H-149 permits a nested lexical scope to shadow an outer local value, while duplicate declarations within one scope are compilation errors. The initializer is evaluated before the new binding becomes visible. Under H-271, a `ptr` declaration fixes its own target owner at initialization; rebinding that variable cannot select another owner. A nested declaration introduces a distinct binding, so this owner restriction is not a prohibition on shadowing. Leaving the scope restores lexical lookup of the outer name. Local names remain distinct from symbol variables written through `effect`. The native prototype uses ref/ptr/val declarations with initializer-before-binding lookup and fixed-owner checks.

Function parameters belong to the callable's inherent scope. Ordinary braces and conditional grouping do not create lexical scopes; explicit `{{ ... }}` does. A loop's index and fold accumulator belong to the loop construct. Body and zero-iteration else declarations require a nested explicit scope, while iteration effects and enclosing-scope deferred work follow [D-17's loop scoping rules](ExceptionControl.md#loop-scoping-rules). Scope-local name tracking is compiler metadata; no runtime lexical binding table or new regional representation is implied.

Acceptance covers an initializer reading the outer value, restoration after explicit scopes and loop iterations, parameter and loop binding uniqueness, and rejection of repeated declarations in one scope before program output. H-149 resolves K-253's naming alternatives; global declaration, function/type namespaces, dynamic symbols and broader scope-result rules retain their existing boundaries.

### Pointer reads and parameter owners

H-292 makes a fresh bits ptr initializer shorthand for `make local T { expression }` in the enclosing lexical scope; existing views retain their actual owners. H-293 preserves read-only access through aliases and requires an explicit external owner on assignment-overload destinations, with a diagnostic for omission. [D-14](RegionalOwnership.md#local-bits-storage-and-initialization) records the source contract, comma-separated group initialization, transfer shorthand and deferred definite-initialization checks. These decisions do not change dynamic-symbol storage or authorize reading TLS effects from functions.

H-271 makes a variable read implicitly dereference its target. `ptr` names do not expose raw addresses or pointer arithmetic. The compiler retains referent provenance for assignment and explicit external borrowing even when the same name supplies loaded bits to an arithmetic expression. `=` rebinds only within the initializer-fixed owner; `<-` performs the admitted bits/ownership transfer. H-270/H-302 makes assignment-family results destination views: builtin `=` preserves access, while `<-` is read-only.

H-272 makes a default parameter's target local to the callee scope. Default bits arguments are evaluated/copied, so editing the parameter leaves the caller's storage unchanged. The implemented ordinary-pointer subset is described in [D-14](RegionalOwnership.md#bounded-pointer-and-initialization-implementation). External borrowing requires the H-273-confirmed owner-generic annotation in D-14; a shared owner variable constrains the actual owner instance, not just its root type. Non-bits regional objects cannot be copied into the local scope. Whole-region consuming calls retain their single-owner and exceptional-cleanup obligations while their ptr declaration mapping is coordinated separately. H-273 permits omitting ptr in function/procedure parameter declarations without changing these semantics or their owner constraints. Local declarations still require ptr. These rules do not broaden a function's access to dynamic bindings.

H-274 additionally requires region-specific owner parameters constrained by a root type, with the actual borrowed region available to the body. Such a function can invoke explicit-memory root-policy methods without reading a dynamic symbol. A generic owner parameter still accepts either a region or a lexical scope and does not by itself supply a region reference. H-275 confirms the region binder spelling and extends region-as-declaration to the whole language. [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language) keeps actual region references distinct from root-object ptr values, with result/symbol spelling proposals separately marked.

### Lexical declaration keywords

H-150 rejects reserved expression names in lexical value declarations: function/procedure parameters, ref/ptr/val declarations, loop indices and fold accumulators. A bare name that the expression parser treats as a literal or control/declaration form cannot introduce a local binding. For example, `function identity(true:bool) -> bool { true }` is a compilation error rather than a function with an inaccessible parameter.

The bounded grammar reserves `true`, `false`, `null`, `alloc`, `for`, `fold`, `let`, `ptr`, `ref`, `val`, `throws`, `const`, `typeof`, `intrinsic`, `effect`, `perform`, `behave`, `if`, `procedure`, `function`, `global` and `symbol` in those declaration positions. Contextual names with ordinary value lookup, such as `in` and `else`, remain available. This aligns accepted declarations with their subsequent lexical lookup, serving C-37/C-41’s readable composition through the existing candidate P-86/P-97 value paths. It does not change H-149’s visibility/uniqueness rule or dynamic symbol identity.

Field selectors retain contextual identifier meaning: `record.true` and `setfield(record, true, value)` are valid. Symbol names, module resource aliases, callable names, module globals, a general reserved-word policy and escaped identifiers retain their separate boundaries; this local decision supplies no escaping syntax or wider namespace rule.

Acceptance checks inaccessible parameter/local/loop names before execution, including during dependency scanning, while preserving ordinary contextual names and field selectors. Rejection names the offending spelling and its source location. P-130 records the confirmed local constraint; the complete language contract remains pending-human.

### Primary module-form keywords

The implemented H-161 keyword set reserves `type`, `data`, `regional`, `unit`, `array`, `using`, `import`, `mixin`, `export` and `conversion` against ordinary declarations and aliases. These names cannot introduce lexical functions, values, types or selected UoM aliases. A declaration such as `function type(...)` is rejected before execution, independent of where it would be called. Field selectors such as `record.type` and symbolic names such as `@using` retain their contextual meaning. H-149 shadowing and usable nonreserved contextual names remain unchanged. Data is the concrete-representation declaration keyword; regional remains reserved although the legacy regional type form is rejected. C-41/P-97's readable expression composition is the existing candidate value rationale; this local keyword decision does not qualify the complete grammar.

### Operator and intrinsic overloads

`const ptr` fixes each declaration-group binding while preserving the referent's independent access permissions. The `<=>` reference-swap and `<->` bits-swap operations yield void, evaluate both destinations left-to-right once, and admit function/procedure overloads with explicit external owners. Reference-swap methods receive both actual caller bindings. [D-14](RegionalOwnership.md#reference-and-bits-swap-operators) defines their checks and interaction with fixed bindings and loop invariants.

`intrinsic (operation)(arguments...)` explicitly chooses the checked builtin implementation and bypasses user overload selection. It supports delegation from a same-signature overload without changing ordinary lookup or the function/procedure boundary. `intrinsic` is reserved as an expression keyword; parenthesized names use the existing operation grammar, not a symbol identity. Missing builtin implementations reject. Ownership, access, lifetime, argument sequencing and the destination-return safeguard remain in force; [D-14](RegionalOwnership.md#explicit-checked-builtin-invocation) defines the bounded contract. Mixin preserves this explicit selection in the copied body.

Range `for ... else` executes its else block only when the body runs zero times. Bounds are evaluated once; exceptional exits do not enter else. Both blocks have separate local/dynamic scopes, and the index belongs only to the body. The expression yields void. [D-14](RegionalOwnership.md#loop-initialization-and-optional-empty-range-branch) defines the normal-path initialization join.

H-153 reserves a set of keywords against ordinary-name declarations and uses parenthesized operation names to declare overloads. An intrinsic spelling such as `len` must not silently conceal an accepted ordinary function declaration. `(len)` is an explicit overload name, following H-131's operator naming direction. The [prototype reference](../tests/cod/language/README.md#intrinsic-and-procedural-operation-overloads) lists its reserved expression, intrinsic and built-in type names. H-150's usable contextual names and field selectors retain their grammar; parentheses do not turn arbitrary control forms into eager calls. The complete future grammar remains open.

Operation spelling is separate from callable kind. H-155 permits function and procedure overloads under one operation, distinguished by parameter types. Under H-197/H-198, a function overload may write explicitly supplied regional storage; an overload accessing dynamic resources must be a procedure. Selected procedure overloads remain forbidden within functions. Conversion formulas are functions without dynamic-context access; the keyword itself supplies no automatic read-only guarantee.

The bounded implementation selects methods by argument types across both kinds, then checks whether the selected callable is legal in the caller. In a function, selecting a procedure would be an error rather than silently selecting a different pure overload. Two equally applicable definitions remain ambiguous; callable kind is not a proposed tie-breaker. Argument effects and automatic conversions must still obey H-148's left-to-right order and execute once.

H-158 requires intrinsics to use their compiler-internal representation without conflating it with CoD's symbol mechanism. A declaration such as `procedure (store)(...)` is an intrinsic procedure overload; it does not introduce an `@store` identity, a vocabulary alias or a symbolic-variable binding. Ordinary symbol-named procedures retain H-103's rules. Parentheses preserve token categories: `(@len)` groups a symbolic read, while `(len)` names an intrinsic operation. A symbol token or quoted string cannot declare an intrinsic overload. The intrinsic method's callable kind governs its permitted body operations.

H-159 specifies current-binding invocation for an operation-selected procedure, like `behave`. Its body sees the caller's dynamic context. An enclosing ordinary `perform @p` still establishes its own captured context; an intrinsic procedure invoked within that body sees that context. Capture analysis must therefore follow demanded typed calls without requiring effects used only by unselected overload candidates. Ordinary procedure definitions still resolve directly referenced symbol identities at their definition point; an alias requires its introducing module encounter first. This identity check does not require the corresponding dynamic value to be bound yet. No runtime method registry or user heap follows from operation syntax.

The implementation uses the compile-time method table and exact-before-conversion selection. Broader method replacement, explicit `perform` syntax for intrinsic operations and extensions to body-mixin syntax are separate work; none is implied by assigning intrinsics CoD symbol identities. H-160 defers new design decisions to a consolidated review at the end of the current work period, while already confirmed implementation work continues.

H-162 selects a distinct assignment operation `(=)`. Explicit `function (=)(...)` and `procedure (=)(...)` definitions, `using "module" (=);` and `(=)(...)` calls follow the same type dispatch, callable-context and current-binding rules as other eager operation methods. `(store)` remains independent. H-163/H-164 define `a[i] = value` as `([]=)(a, i, value)`, a separate family receiving the original container/index/value types. The receiver and index are evaluated once without loading the destination element; argument conversions finish before the next argument. The RHS is a full expression, and the result is the selected method’s result. Explicit `([]=)` declarations, source selections and calls use the same method rules. A visible method supplies behavior; there is no implicit forwarding to `(store)` or `(=)`. Record-field assignment and field-selector typing remain separate work; no first-class lvalue object or pointer-to-pointer representation is introduced.

Reference assignment is right-associative. H-302 makes parameter const qualifiers distinguish methods and makes builtin `=` preserve the destination view's access. Overload lookup precedes builtin access rejection; a writable-lhs/const-rhs method is callable if its body proves the required owner/access/result conditions. The lhs qualifier must match exactly; input views may weaken writable access to const without copying. Numerical/UoM conversion is not reference-rebinding adaptation. Types, owner constraints and qualifiers must yield one uniquely best method, without declaration-order or result-type tie breaking. Infix, explicit operator calls and selected callable aliases retain the same rules. [D-14's contract](RegionalOwnership.md#const-qualified-reference-assignment-methods) specifies caller-visible lhs binding and result proof. Transfer `<-` still returns a readonly destination view. General indexed destinations and graph-overload algorithms remain separately bounded.

H-206/H-208 extends transfer selection with both owner/root types and actual owner-reference arguments. The result is a const destination view, including when a user function/procedure supplies the graph algorithm; H-207 prevents consuming that view in a second owning move. Methods can use explicit source/destination region-policy interfaces without ambient lookup. Abstract root matching must retain concrete type/owner identity and reject unauthorized object/owner pairings. The two-operand source expansion and method header/result grammar remain proposed in D-14; this does not silently extend the existing exact-before-UoM-conversion matcher.

H-209 fixes <- as right associative and requires a unique most-specific applicable method regardless of declaration order. H-210 gives declarations the subtype relation <:; type names denote abstractions and data names concrete product/sum definitions. The keyword split changes scan/type-registration rules as well as constructor and method checking; it is not a backward-compatible alias for the current product parser. Detailed subtype/interface grammar is tracked in D-07. Operator association does not alter the existing argument-evaluation or function/procedure permission rules.

Field selection and resource ownership retain their own boundaries. `setfield` currently has a compile-time field selector, not three ordinary value arguments; assignment must identify a valid regional storage location without inventing an extra pointer layer. Returning a region created inside an overload cannot bypass its existing scope lifetime. Symbol-variable rebinding requires `effect @x = value`; transfer uses `effect @x <- value`. Regional assignment does not authorize bare `@x = value`.

The value rationale is readable operation syntax with inspectable type, effect and environment dependencies: C-41/P-97 and C-50/P-121 support readable typed composition, while C-37/P-86 and C-36/P-85 preserve explicit-input versus dynamic-context boundaries and contextual interpretation. These candidate value paths do not qualify the complete contract. Acceptance must distinguish function and procedure overloads of one spelling, reject a selected procedure in a function call tree, preserve current versus captured values and argument order, and diagnose ordinary keyword declarations. Existing intrinsic behavior and contextual field names must remain covered. The [prototype reference](../tests/cod/language/README.md#intrinsic-and-procedural-operation-overloads) describes the implemented subset. The earlier effect-arrow checkpoint passed 147 source checks, five composition cases and the regional roundtrip/relocation harness, with its source/compiler hashes in the [effect-arrow record](../tests/cod/language/effect-arrow-validation.json). The [assignment record](../tests/cod/language/assignment-validation.json) retains the focused purity, nominal-index and reserved-alias checks at their recorded source revision. Numeric and gather records retain their own run scope. These are language/runtime checks, not SNN acceptance. General lvalues, record-field assignment, field-selector overloads and intrinsic body-mixin extensions remain deferred; passing this subset does not qualify the full design.

### Storage carriers, capture types and lifetime

[Regional lexical captures](RegionalClosures.md) retain live regional slots or region access for named and anonymous callables. Capturing a slot observes its later writes/rebinding; capturing a lexical pointer cell is rejected. The table below describes **dynamic** capture at runtime callable-value construction, separate from module method registration. Regional lexical capture does not turn perform into a memory snapshot or alter symbolic-value restoration.

Ordinary user-program storage has lexical and regional carriers; symbolic bits payloads use the dedicated TLS storage specified above. H-177 permits ordinary variables to own region handles; H-179 permits consuming installation into a dynamic resource owner. Neither copies a regional object onto the stack. Host-native heap allocation belongs to system code such as compiler/runtime infrastructure. It cannot serve as a hidden way to extend the lifetime of a user closure, captured object or environment. No user `malloc`/`free` lifetime, automatic heap box, user GC object or reference-counted heap capture is introduced.

| Captured value | Meaning and current boundary |
| --- | --- |
| Bits | Preserve the value at procedure definition. Later scalar bindings do not change it. |
| Whole-region effect resource | Preserve the selected resource value and its owner/lifetime dependency. Replay does not move ownership, copy the graph or freeze its contents. |
| Interior regional reference | H-277 admits region-symbol-backed references with lifetime checking. Captured values preserve their actual region dependency under H-180; the compiler must prove that dependency rather than extend the resource lifetime. |

Reinterpreting an address as an integer must not bypass carrier/lifetime checks. A native heap pointer, stack pointer or arbitrary handle is not implicitly an admitted capture. System-resolved addresses are execution artifacts, not independently owned user objects.

Mutation through a valid resource reference changes its referent. Restoring a symbol binding does not undo that mutation. A read-only view prevents mutation through that view but still requires valid lifetime. Capturing a regional object by C++ copy/move is forbidden.

An initial SNN step can use non-escaping stack values and region-backed effect resources. Captured whole-resource values and region-backed interior references retain their respective ownership and lifetime dependencies. A regional pointer's owning-region association must remain valid; a saved offset cannot silently be interpreted relative to the caller's different `@region`. A procedure retained across frame reuse, region release or remapping needs a justified lifetime. No stack closure is automatically promoted to the native heap. The concrete procedure descriptor and compiler-generated stack temporaries remain representation questions. H-193 makes user-defined data objects regional. Scalar UoM identities retain scalar bits representations; they do not provide arbitrary record storage on the stack.

TLS directly stores symbolic bits payloads under H-277; it is not an allocation arena for non-bits user objects. Symbolic object references and captures must respect their region-symbol resource lifetime. Saved values and environment storage cannot extend that lifetime. `@region` allocation is available to procedures, while functions use their declared inputs/outputs and legal stack temporaries.

#### Function region-state access

H-197/H-198 permits functions to read and edit explicitly supplied memory and, once their source contracts exist, create regions, return ownership and send through an explicit mailbox. Functions cannot obtain regions through dynamic-variable lookup or call procedures to bypass that restriction. A procedure can supply a valid explicit view to a function while retaining its dynamic owner; the function does not acquire or rebind that owner's symbol.

References preserve the actual input-region association through fields, nested calls and returned values. A function may return a view into a still-live caller region. A view into a local region released at function exit is rejected; returning an intrinsic whole-region owner would instead transfer cleanup responsibility. Regional objects are never copied onto the stack, and function return does not itself end an input region.

There is no automatic read-only snapshot spanning nested functions. An inner function's writes are observable by subsequent reads in its caller and through aliased parameters. Native lowering must preserve source sequencing and actual memory dependencies, without inferring referential transparency from the keyword. No copied snapshot, version counter or per-object ownership system is introduced. Caller-visible edits are not automatically rolled back when local storage is released.

The implemented subset permits store, setfield and mutating function overloads over explicit regional inputs. A bare region local, parameter or result borrows an already held resource and retains its owner provenance. `alloc arena T(...)` accepts that named lexical borrow, so a function can construct objects and arrays in a caller-supplied region without reading a dynamic symbol. Functions still reject symbolic reads/writes, perform/behave and all selected procedure calls. A borrow cannot be installed as a new owning effect or escape the region's acquiring scope. [K-411](DesignAuthorityRegistry.md#k-411) records typed roots, make, ordinary owned locals/calls/results and preserved dynamic captures. Scalar-slot borrowing, general const propagation and mailbox operations remain unimplemented. Concrete data uses regional allocation; implicit object-field borrowing and qualifier syntax remain separate migration work.

Acceptance uses repeated mutating calls, alias-sensitive reads, returned views, cross-region store rejection, transitive dynamic-access rejection and the CoD free-list functions. The existing record/cycle and capture-owner native checks preserve their lifetime/relocation role. Historical snapshot reports identify earlier source scopes; they do not restore a read-only guarantee. Future const-function syntax, reachable-data restrictions and external-effect rules require their own business-driven design.

Persistent code identity, captured regional values and transient native access paths remain separate. Existing C++ pointer layouts do not settle their language representation. A TLS offset or native procedure address is not a portable stored program reference.

### Accepted temporary self-reference limitation

H-146 retains the current field-relative pointer encoding. When the first pointer field has the same address as its enclosing object and is assigned that object, its displacement is zero and it reads back as null. This known issue remains for ptr fields. A non-null ref field has a runtime representation guard rejecting the unrepresentable edge; the encoding has no new marker. The expected future resolution is the introduction of intrinsic object fields. Their contents, layout, representation and compatibility consequences remain to be designed; the current increment adds no such fields. Nonzero-distance references and cycles retain their existing behavior.

### Logical relationships between regions

H-156 identifies each region with one graph of directly linked objects. H-157 confines relationships between regions to logical business associations. A CoD physical pointer edge must stay inside its region. A business ID or key can describe an association with an entity in another region. Its resolution, identity and version policy, permissions and behavior under reload belong to the business protocol.

The runtime can resolve a native address for a region whose lifetime is established. Each language-level reference still belongs to one region, including when a procedure uses bindings or arguments from several regions. The Shilos C++ `global_ptr` facility belongs to the host storage layer. [Regional memory vision and implementation tracker](GraphGrainedMemoryManagement.md)

## Whole-program layout and specialization

The compilation input is the demanded program composition: package/module identities, module initialization, typed definitions and reachable caller/callee paths. Modules supply definitions and are not independently compiled. Foreign entries must establish a CoD entry context whose binding requirements can be checked; this does not choose an FFI ABI.

A proposed lowering sequence is:

1. Scan/load the complete entry dependency set; register allowed declarations and retain initializers, procedure bodies and executable top-level expressions without running their semantics. Resolve symbol identities and preserve module origins.
2. Compile execution semantics from the entry, incorporating dependency top-level execution in encounter order. Follow these initialization/caller paths and check required symbols, types and captured/current bindings for each call mode. Compute reusable summaries where useful.
3. Assign typed locations in the program's per-thread context, then lower known accesses to a base plus a constant offset.
4. Lower `effect` and `perform` to scoped save/install/restore operations; `behave` retains the current context.
5. Establish the admissible actual-type combinations for each selected family/context and emit their implementations and dispatch before execution. Share equivalent targets and prune only proved-impossible alternatives; closure and concrete-evidence mechanisms require the D-21 contract.
6. Specialize/in-line proven calls and promote bindings to ordinary values where their observations, addresses and foreign uses permit it. Retain the context representation where it remains observable.

A simple evaluator of the same whole-program-checked representation can validate semantics before optimized lowering. This is a validation candidate, not a separate-compilation execution model. Collection and layout make accesses direct; optimization can then remove accesses or save/restore work. Neither guarantees every access becomes a register or a single instruction. [LLVM's TLS reference](https://llvm.org/docs/LangRef.html#thread-local-storage-models) distinguishes platform/linkage-dependent access models. [LLVM's LTO design](https://llvm.org/docs/LinkTimeOptimization.html) explains the importance of definitions, uses and visibility for intermodule optimization; it supplies no automatic implementation of CoD's scope semantics.

If several CoD program instances execute on one host thread, their binding contexts must remain distinguishable, including reentrant foreign callbacks. A thread-local current-context pointer is one lowering candidate, not a selected ABI. Native-thread scope also does not automatically become task/fiber scope. The initial execution example can remain synchronous; worker entry must explicitly establish the intended context before parallel execution is added.

An on-demand compilation that changes the program's symbol set needs a compatible layout or a new layout/version with the affected code and contexts rebuilt/rebound at an appropriate boundary. Old code cannot keep using offsets whose meaning has changed. Cache identity must include the relevant program/type/layout dependencies; semantic symbol identity must remain independent of the current offset. This does not choose JIT or replace the existing build-and-run driver.

## Actor concurrency direction

H-180 separates cross-process regional send/receive from constructing a region-backed effect resource and exposing it throughout a dynamic scope. They share the regional storage substrate, not an ownership transition or captured execution context. Packet construction/send must not be forced through a dynamic-resource binding; resource shadowing must not be modeled as packet transfer.

H-168 supplies intrinsic mailboxes with a UDP-like send contract: report local system send errors, without delivery acknowledgement. This selects neither UDP as the physical transport nor a promise about FIFO order, retries or remote handler completion.

H-151 selects the Actor model for CoD concurrency. Processes and mailboxes are distinct language concepts, and the mail payload type is a type parameter. H-152 gives `Mainbox<WorkPkg>` as the example of a process’s main mailbox accepting work packages. The number of mailboxes a process can expose, the type of a process reference and concrete syntax are open.

A candidate use of regional mail is a request rooted at a user-defined type that describes a computation and its inputs. The receiver selects an interpretation of that protocol in its own execution context. For example, two processes can accept the same request type and bind its abstract operations to simulation or live handlers. This connects typed mail to the charter’s program and execution separation (§3.2) and environment-specific effects (§6.1). The existing IPC packet direction (H-06/C-08) addresses wire packets and entry application as a separate design scope.

### Region holding and transfer

Region holding and transfer are a design priority under H-152. The mail type determines which payload the mailbox accepts. The region contract determines who can hold and access the backing storage, what changes at a handoff, and who releases it. Those rules must cover creation, local send failure, local send acceptance, queueing, receipt and disposal. Local acceptance is not confirmed delivery. Packet transfer and dynamic resource ownership must not be treated as the same operation; their distinct lifetime obligations are in [D-13](GraphGrainedMemoryManagement.md#effect-held-region-ownership).

Exclusive region handoff could deliver mail without copying the payload within one address space. It requires preventing sender access after handoff through lexical references, dynamic bindings and procedure captures. Read-only sharing requires a shared lifetime and stable contents. Receiver-owned copying requires preserving the graph during copying or reconstruction and incurs a payload cost. Every supported mode must preserve regional lifetimes, intra-region pointer semantics and allocator symmetry. Scheduling, delivery order, mailbox capacity and physical transport remain open. H-168 limits send feedback to local system errors; ownership on such an error remains a separate question.

Proposed acceptance separates typed mail from region transfer. First check that a process can receive an admitted payload type and that an incompatible payload is rejected. For the exclusive handoff candidate, also check that the receiver can consume a graph after the sender’s scope ends, that its internal sharing survives, that stale sender access is rejected, and that failed or abandoned delivery leaves exactly one party responsible for release.

## Initial language features and their purpose

| Feature | Immediate purpose | Initial boundary to validate |
| --- | --- | --- |
| Two-phase module processing | Make declarations available before composing demanded execution | No scan-time captures/effects; dependency execution incorporation follows entry compilation encounters |
| Nominal modules and vocabulary symbols | Compose model modules and multiple versions without binding collisions | Alias identity and distinct-origin cases |
| Procedure source selection versus `mixin` | Reuse a source definition or instantiate its body in a new lexical/dynamic context | Four combinations with `perform`/`behave`, checked against their actual binding paths |
| Scalar types, products, sums and exhaustive matching | Describe parameters, state alternatives and explicit outcomes | Type-correct construction and rejected invalid cases |
| A small checked GADT program core | Keep input/output/effect relationships in inspectable program data | Correct type indexes through pure and effectful operations; no full theorem solver required |
| Functions versus procedures | Separate explicit-input computation/memory access from dynamically bound operations | Lexical/symbolic procedure naming, preserved captures on installation and transitive effect checking |
| Expressions and `void` | Compose value-producing and effectful computation consistently | Explicit scope results, sequencing, effect-only writes and no meaningful-result use of `void` |
| `effect`, `perform`, `behave` | Supply current execution context or replay captured values deliberately | Nested scopes, capture, mutation and restoration traces |
| Regional ownership plus bounded read/write views | Keep state relocatable and distinguish source-frame reads from destination writes | Borrow lifetime and mutation checks; no copy/move of regional objects |
| Typed arrays, loops and reductions | Execute incoming sparse gathers and frame computations | Bounds, reduction/numeric semantics and complete-frame publication |
| Native primitive boundary | Reuse runtime facilities and eventually compile numerical kernels | Same contract under reference and native interpretations |

The SNN owns the neuron equations, AP/rate interpretation, training rules and data layout choices. The language owns identity, callable/effect semantics, types, lifetimes and execution meaning. An application-specific neuron or slot class does not need to become a built-in language construct.

## Validation and unresolved contract scope

The [case records](../applications/steady-snn/research/cod-language-scope-cases.json) contain expected semantic traces and proposed checking cases. They remain specifications, with a link to selected native fixture evidence; the complete catalog has not been executed. They cover alias/version identity, scoped restoration, by-value capture, both invocation modes, transitive dependencies, function restrictions, thread/program isolation, scan-only registration, entry-driven dependency incorporation and the SNN's immutable-A gather.

Typed symbols, bits globals, explicit result checks, ordered argument conversion and known-call binding/lifetime checks have bounded native implementations described in the prototype reference. Remaining semantic work includes module cycles, compound implementation views, local value/call lookup, standalone method heads and the coherent owned/const/place/construction/transfer contract. Missing required dynamic bindings remain compilation errors. These boundaries affect observable behavior or valid programs and require design decisions; equivalent lowering layouts and inlining choices can instead be evaluated on actual programs.

[Racket's parameter reference](https://docs.racket-lang.org/reference/parameters.html) is a useful primary comparison for scoped bindings, restoration and thread-sensitive behavior. Its continuation/thread-cell model is not CoD's capture-by-value rule. No continuation system, general dynamic dictionary or scheduler follows from that comparison.

## First native prototype boundary

H-108 authorizes autonomous development toward a first prototype that compiles and runs on demand once the immediate review is resolved. H-109 supplies the reference trigger; H-166 supplies module-scope lifetime; H-288 supplies the current void effect result. The first milestone exercises the confirmed language semantics rather than selecting final SNN dynamics.

The initial acceptance is a CoD source entry with dependency modules that is scanned, checked as a demanded whole program, lowered to a native executable using the existing build toolchain, and run with observable scalar results. It must demonstrate nominal symbols/aliases, lexical functions, dynamically named procedures, both reuse forms and call modes, module execution boundaries, scoped value capture/restoration and missing-binding/function-effect diagnostics before execution. A small exact-integer incoming-gather example ties the language path to the confirmed two-frame application architecture without asserting a neuron equation or learned behavior.

The implementation can reuse the current driver's compiler/link/run facilities; it must not merely pass the new source through as C++ or claim interpreter-only output as native compilation. Compiler scratch state belongs to system code; prototype user values and captures preserve the stated storage boundaries. Begin with scalar bits and non-escaping values. Cross-region references, persistent closures, general GADT/ADT syntax, unrestricted higher-order recursion, concurrency and final numerical/model choices remain outside this first acceptance milestone. Unsupported forms receive an explicit diagnostic rather than a silently chosen permanent semantics.

Source syntax and lowering remain reviewable implementation work under these boundaries. The complete language contract is not retroactively qualified by this milestone. The authority registry records the direct implementation request and the exact local decisions separately from unconfirmed full-language mechanisms.

## Runnable prototype evidence

The [native prototype reference](../tests/cod/language/README.md) describes the implemented typed/module/regional subset and its experimental syntax. The parser, demanded-call checker and lowering are in `cod/lang_syntax.hh` and `cod/lang_frontend.hh`; `cod/main.cc` provides `--run`, `--check`, `--emit-llvm` and `--emit-bc`. Checked CoD lowers directly to LLVM IR, which the existing toolchain compiles and links with small C++ system services. H-143 selects this sole lowering path. LLVM artifacts are derived execution representations, not the language definition or a persistent semantic graph.

[Recorded validation](../tests/cod/language/validation.json) identifies a full language checkpoint; the prototype reference identifies subsequent focused corrections separately. Each linked record retains its own timestamp, source hashes and execution scope. H-144 requires meaningful semantic coverage rather than a growing case count. It exercises captured/current calls, using/import combinations and identity checks, restoration, once-only module incorporation, named product/UoM methods, pure conversions, typed SoA arrays, regional records, alias-sensitive memory operations, returned references and mutation through nested calls. Compilation errors prevent preceding top-level effects from running. A multi-module A/B gather operates on real regional arrays without selecting a neural transition or learning rule.

Known-call specialization uses fixed typed binding slots and saved values, with owning-region information specialized along demanded paths. User regional records stay in regions and cannot copy/move. The [stress record](../tests/cod/language/stress-validation.json) includes 512 modules, 1,000 procedure specializations and a 96-module regional borrow chain. [Relocation evidence](../tests/cod/language/regional-validation.json) checks the emitted layouts after a whole-region byte relocation. [Kernel measurements](../tests/cod/language/gather-full-pool-validation.json) cover the existing J16,q10 arithmetic layout, separately from any SNN acceptance.

Historical evidence remains scoped to its source revision: [abstract/data checks](../tests/cod/language/abstract-validation.json), [regional representation](../tests/cod/language/data-validation.json), [lexical mixin](../tests/cod/language/lexical-mixin-validation.json), [module integration](../tests/cod/language/module-integration-validation.json), [associated methods](../tests/cod/language/associated-method-validation.json), [context/export checks](../tests/cod/language/module-context-export-validation.json) and [borrowed regions](../tests/cod/language/region-borrow-validation.json). These records establish neither general ownership/qualifier syntax nor complete module/type acceptance.

Source conventions and the full language contract remain reviewable. General recursion, dynamic higher-order dispatch, persistent closures, business relationship resolution across regions, package-version identity protocols, compound UoMs, general GADT construction and concurrency are outside this evidence. Cross-platform parity and a general reference interpreter are not claimed.

## Complex composition and initial lowering

H-139 requires deliberate multi-module, complex-dependency and deep-call validation. Acceptance includes diamond graphs, repeated selections, shared vocabulary and nominal-type aliases, capture restoration across long call chains, and source initialization order without scan-time execution. H-140 requires each module to be incorporated once; emitted initialization is part of the resulting program. H-166 keeps module top-level scopes alive until process exit; initialization return restores active lookup.

H-141 accepts procedure expansion/inlining for the initial WPO implementation while program size remains reasonable. LLVM artifact size, native size and compile/run cost must be measured on representative deep and branching graphs. Reusing machine-code fragments is a medium/long-term optimization rather than an immediate architecture requirement. WPO call-path checking remains mandatory whether the eventual native lowering inlines or shares code.

H-144/H-145 require keeping the implementation and validation lean. Coverage is assessed by whether module incorporation, identity, capture/restoration, dynamic-context exclusion, typed conversion, control flow and regional lifetime/relocation behave as specified. Named numeric boundaries and an independent SoA gather reference address the relevant arithmetic risks. Duplicate syntax variants and large sample counts do not establish additional semantic coverage. The prototype introduces no backend abstraction, custom optimizer or code-sharing layer merely to support this validation.
