# Domain language design: discussion and decision tracker

Discussion revision: 217

Recorded: 2026-11-24

Phase: coordinated language/SNN design, implementation planning and validation.

## Design authority

[D-07-MEMBER-RECEIVER](DesignAuthorityRegistry.md#d-07-member-receiver) covers the [receiver qualifier](../CHANGELOG.md#h-member-receiver-const-20261124) at r216 and the explicit receiver owner at r217 under P-724/P-725. [D-07-MEMBER-NAMES](DesignAuthorityRegistry.md#d-07-member-names) at r217 covers field, accessor and method name collisions under P-726/P-727. The [project user’s decision](../CHANGELOG.md#h-member-names-and-owner-20261124) settles both questions. Acceptance compares target and allocation access independently and diagnoses colliding declarations even before a member is used. [Member declarations](#receivers-in-field-accessors-and-member-methods)

At r215 the [symbol identity decision](../CHANGELOG.md#h-runtime-symbol-identity-20261124) applies [D-10’s protected lexical name](SymbolicDynamicScope.md#the-bare-symbol-name-at-runtime) to source spellings and module origins. Distinct spellings may denote the same identity. A binding that would hide a visible symbol fails at its declaration.

D-07 r214 applies the [grouped installation decision](../CHANGELOG.md#h-grouped-local-proc-install-20261124) and the [local handler decision](../CHANGELOG.md#h-local-proc-handler-body-20261124) to DL-14/DL-15 under D-10-HANDLER-INSTALL and P-330. Use the same-signature lexical and dynamic calls to distinguish their targets. See [local procedure bodies](SymbolicDynamicScope.md#installing-a-local-procedure-body).

At r212 the [human decision](../CHANGELOG.md#h-actor-region-transport-and-chan-ends-20261124) applies D-32 regional transport and P-711 to [DL-04](#dl-04). Verify that a gap within a multi-record region exposes no partial root and that an intact following region remains deliverable. H-352 governs root compatibility and application settlement. [Transport contract](ActorChannels.md#regional-transport-over-akcp)

[Review backlog and status reconciliation](../CHANGELOG.md#h-review-backlog-duty-20261123) reconciles the scoped confirmations below with their registry records.

[Confirmed symbolic procedure forms](../CHANGELOG.md#h-lexical-symbolic-proc-20261123) applies the D-10 symbolic call and D-15 module selection contracts to DL-14 and DL-15. [Lexical and dynamic calls](SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)

[D-10-ACTOR-CHAN](DesignAuthorityRegistry.md#d-10-actor-chan) governs r208's Actor-local context and chan summaries. [H-ACTOR-CHAN-BOUNDARY-20261123](../CHANGELOG.md#h-actor-chan-boundary-20261123) selects Actor/chan terminology, dedicated-thread 1:1 execution, isolated TLS, static message typing and fire-and-forget sending with systemic-fault feedback. [K-ACTOR-CHAN-RESEARCH-20261123](../CHANGELOG.md#k-actor-chan-research-20261123) carries these scoped changes into G-06, DL-05 and compilation context wording. Existing module, owner, callable and cleanup invariants remain; [D-32](ActorChannels.md) keeps endpoint, transfer-on-fault and application-protocol candidates separate.

At r207, LI-04 writes the starred unit and its first conversion arrow as separate tokens under [D-10-OPERATOR-RUNS](DesignAuthorityRegistry.md#d-10-operator-runs). H-136–H-138 govern selection of the same direct or declared route. [Decision](../CHANGELOG.md#h-maximal-operator-runs-20261123)

[Editorial continuity](../CHANGELOG.md#k-lexical-callee-examples-20261123) aligns lexical calls and symbolic procedure installation with [D-18 callable value kinds](CallableStorageAndHereAccess.md#callable-value-kinds) and [D-10 handler installation](SymbolicDynamicScope.md#definition-installed-effectful-procedures). Existing qualification boundaries apply.

At r204, [D-07-DOCLETS](DesignAuthorityRegistry.md#d-07-doclets) qualifies [documentation at definitions and references](#definition-doclets) under [the project user’s doclet decision](../CHANGELOG.md#h-definition-doclets-20261123). P-686/P-687 connect readable source to stable definition identity and navigation. Acceptance covers a renamed reference, two overloads, an in-doclet link and an ordinary triple-quoted expression.

At r206, [guarded short heads](../CHANGELOG.md#h-guarded-short-heads-20261123) qualify optional pattern parentheses for constructor reception and guarded short arms under D-07-CASE, P-625/P-626. Compare the same received field and declining guard in both spellings.

At r203, [the short-arm decision](../CHANGELOG.md#h-short-case-arms-20261123) extends D-07-CASE/P-625–P-626 to bare constructor, enum-member and wildcard heads and single expression bodies. It reuses the same pattern lookup, result and control contracts. Acceptance covers equivalent long and short arms over real booter syntax rows and closed enum values.

The [diagnostic name decision](../CHANGELOG.md#h-diagnostic-names-20261122) applies D-20-DIAGNOSTICS to the H-214 module selection display at r202. Lookup and source syntax retain their existing contracts.

[D-07-ENUM](DesignAuthorityRegistry.md#d-07-enum), P-658/P-659, applies [the typed-enum decision](../CHANGELOG.md#h-typed-enum-20261121) at r201. A closed set of named bits values keeps application outcomes explicit across source modules and cases. Check declaration identity, lexical shadowing and complete coverage over distinct values. The scoped [enum contract](TypedEnums.md) records acceptance and the C boundary.

[H-ORDINARY-NAMES-20261119](../CHANGELOG.md#h-ordinary-names-20261119) covers the naming and field-selection entries at r200 under [D-10-ORDINARY-NAMES](DesignAuthorityRegistry.md#d-10-ordinary-names).

- **Construction and invocation.** [H-CTOR-CALL-SEPARATION-20261119](../CHANGELOG.md#h-ctor-call-separation-20261119) and [H-CTOR-CALL-FAMILIES-20261119](../CHANGELOG.md#h-ctor-call-families-20261119) confirm the [two method families](#constructor-and-call-operator-method-families). Check identical written arguments, ctor cleanup and receiver readiness. [D-07-FUNCTOR-LOOKUP](DesignAuthorityRegistry.md#d-07-functor-lookup), P-636/P-637, records the [type-level lookup rule](#type-level-call-operator-lookup). [H-FUNCTOR-PUBLICATION-20261119](../CHANGELOG.md#h-functor-publication-20261119) confirms publication with the type and conflict-checked composition in the receiving world at r198. C-10/C-35 connect a domain operation to the nominal type visible in its module world. Check unrelated types and competing adapters. At r196 [H-SUM-RECEIVER-PARAMETER-20261119](../CHANGELOG.md#h-sum-receiver-parameter-20261119) places alternative selection in the ordinary method signature. The example uses the user’s `self:const Accepted in O` spelling. At r197 [K-SUM-RECEIVER-DISPATCH-20261119](../CHANGELOG.md#k-sum-receiver-dispatch-20261119) applies the general dispatch contract to that signature, including a call through a Receipt parameter. Wider implementation qualification remains open. [Editorial continuity](../CHANGELOG.md#k-ctor-call-organization-20261119).

[H-CALLABLE-TAIL-20261119](../CHANGELOG.md#h-callable-tail-20261119) applies the [callable result rules](CallableResultInference.md#results-of-proc-func-and-here-bodies) to a double-brace case arm at r189. Check the local result after its own defer runs. D-07-CASE retains its source-order and coverage rules.

- **Case expressions and branch-local variables.** [D-07-CASE](DesignAuthorityRegistry.md#d-07-case) r206 specifies the [source rules and examples](#case-expressions-and-branch-local-variables). [H-CASE-CONTRACT-20261119](../CHANGELOG.md#h-case-contract-20261119) covers selection and lexical control. [H-CASE-COVERAGE-20261119](../CHANGELOG.md#h-case-coverage-20261119) permits a statically exhaustive value case to omit a fallback and places each arm directly in its own case body. [H-CASE-THROW-20261119](../CHANGELOG.md#h-case-throw-20261119) confirms a fresh unchecked error for the explicit throwing arm. Check it both outside and inside a catch. P-625/P-626 connect observable selection, binding, effect order and result delivery to C-10/C-65/C-72 and charter §§3.1–3.3/4.3/8.1. Check single evaluation, writes before a failed guard, nullable and closed subjects, fallthrough, nested arm ownership and cleanup. Rich diagnostics follow the [compiler-stage policy](../applications/cod-booter/README.md#diagnostics-by-compiler-stage). Native representation and extensions to owner proofs are outside scope.

- **Functor arguments.** [H-HERE-FUNCTOR-20261118](../CHANGELOG.md#h-here-functor-20261118) confirms direct passage of a data receiver with a compatible call operator to a here parameter. [H-FUNCTOR-ARGUMENT-20261119](../CHANGELOG.md#h-functor-argument-20261119) applies ordinary written argument evaluation to D-07-FUNCTORS r190. C-10/C-65/C-73 and P-605/P-606 connect an explicit domain operation to its receiver identity and live owner. Check a later argument rebinding the source slot, later writes to the original target, and zero or repeated invocations of a computed receiver. [H-CALL-INTENT-20261119](../CHANGELOG.md#h-call-intent-20261119) settles the crossed direct-data/here match at r191. P-606 depends on D-10-ARGUMENTS for explicit labels. Compare an ambiguous bare call, both single-label calls and a trailing here body. Compiler qualification remains separate.

- **Callable model.** D-18-HERE permits synchronous execution and forwarding of caller-supplied work. Ordinary storable functions are capture-free under H-CALLABLE-MODEL-20261118.

- **Callback parameters with call-bounded use.** [D-07-CALLBACK-BOUND](DesignAuthorityRegistry.md#d-07-callback-bound) uses [D-18-HERE-PARAM](DesignAuthorityRegistry.md#d-18-here-param) for different numbers of actual owners. Each supplier proves its own live bindings through synchronous calls and forwarding. [Worked example](#one-callback-parameter-for-different-owner-counts)

- **Functor receivers.** D-07-FUNCTORS relates C-10/C-65/C-73 to explicit operation and receiver lifetime under charter §§3.3/4.3. Respect the target access required by the selected call operator and reject unproved owner lifetimes. P-605/P-606 cover the review of preparation and overload selection.

- **Named sum constructors.** [D-07-SUM-CONSTRUCTORS](DesignAuthorityRegistry.md#d-07-sum-constructors), r173 DL-01, records the [source decision](../CHANGELOG.md#h-dctor-source-20261118). C-10/C-65/C-72 and charter §§4.3/8.1 connect readable alternatives, owner-qualified construction and definite field initialization. Acceptance covers nullable membership, successful and failed initializers, staged constructor-free cycles, distinct regional slot and target identities, and [make-site selection](#publishing-and-selecting-data-constructors). At r199 [H-DCTOR-FAMILY-MERGE-20261119](../CHANGELOG.md#h-dctor-family-merge-20261119) confirms that dctors are methods of their nominal type’s constructor overload family. [D-07-DCTOR-PUBLICATION](DesignAuthorityRegistry.md#d-07-dctor-publication) covers composition of adapter contributions and rejection of conflicting definitions. At r211 the [in-place initialization decision](../CHANGELOG.md#h-placement-init-20261123) applies the same constructor family and field rules to a reserved alternative under [D-22-PLACEMENT-INIT](DesignAuthorityRegistry.md#d-22-placement-init), P-705/P-706. Verify the selected constructor, both cyclic edges and required field readiness. Native layout and compiler proofs have separate scope.

- **Native integration.** [D-07](DesignAuthorityRegistry.md#d-07) r169, DL-11, follows [D-20-NATIVE-C](DesignAuthorityRegistry.md#d-20-native-c) and [H-NATIVE-C-INTERFACE-20261118](../CHANGELOG.md#h-native-c-interface-20261118). C-79/C-80 and charter §§4.3/6.3 connect typed external operations to checked CoD lifetimes. DL-11 supplies the caller, failure and release acceptance cases. Provider registration and the concrete ABI remain open.

- **Current capture-review reference:** [K-REPLACEMENT-EFFECTS-20261117](../CHANGELOG.md#k-replacement-effects-20261117) keeps the separate D-18 value-input qualification current. This file changes only review references; its semantic revision, governing value paths, constraints and acceptance are unchanged. The registry owns the current review scope.

- **Named region roots:** [H-595](../CHANGELOG.md#h-595)/[K-NAMED-ROOTS-20261117](../CHANGELOG.md#k-named-roots-20261117), [D-13-ROOT-ACCESS](DesignAuthorityRegistry.md#d-13-root-access)/P-546–P-548, aligns this revision's regional examples with named roots, ordinary `r.name` assignment destinations and receiving specifications; the source root intrinsic is removed. Reuse C-74/C-65/C-72 and charter §§2/3.1–3.3/4.3/6.3/8/12 for explicit storage, access and lifetime responsibility. D-13 owns acceptance; preserve existing qualifications, actual-owner checks, nominal component identity, D-16 and D-18's existing qualification boundaries. Full structural/receiving proof and compiler migration remain separate; no C++ storage API changes follow.

- **Structural result assessment:** [H-572](../CHANGELOG.md#h-572)/[K-STRUCTURAL-RESULTS-20261117](../CHANGELOG.md#k-structural-results-20261117) covers r166→167’s dependency summary. D-22-ANON-DATA/P-509–P-511 evaluates a return-only named product signature `-> { ... }`, received as independent lexical variables. Preserve H-571, explicit actual owners, existing nominal ADT field rules, no object copying/moving or lifetime extension and D-16 under charter §§2/3.2–3.3/4.3/6.3/8/12. This pending source restriction introduces no aggregate-field permission or implementation qualification; acceptance/details belong to D-22.

- **Callable placement and transfer boundaries:** [H-563](../CHANGELOG.md#h-563)/[H-567–H-568](../CHANGELOG.md#h-567), [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/P-503–P-505 and [D-22-TRANSFER](DesignAuthorityRegistry.md#d-22-transfer)/P-506–P-508 cover D-07r166's placement/operation terminology. Callable-view values have lexical-scope storage, never regional storage; only pure bits admit whole-payload transfer. CoD has no C++ object copying/moving model. C-54/C-65/C-72/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve regional relocatability and explicit identity/lifetime responsibility. D-18/D-22 own acceptance; retain dedicated callable construction/capture, graph transfer, actual owners and D-16. [K-LEXICAL-PRODUCTS-20261117](../CHANGELOG.md#k-lexical-products-20261117) aligns dependencies. D-22’s return-only structural result proposal remains under review and grants no ordinary ADT field extension, new implementation or full proof/physical/value-path qualification.

- **Owner-parameter symbol integration:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499, covers D-07r165's reference-symbol/owner dependency wording. The source form and staged sound acceptance are confirmed: installation selects one actual lexical or ordinary regional owner from the RHS; an `in @arena` declaration retains its additional selector constraint. C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve explicit dynamic context and lifetime responsibility. D-10 owns examples and acceptance. Preserve saved/captured owner validity, access/null/init, module symbol identity, no lifetime extension and D-16; full proof/physical/value-path qualification and source implementation remain separate. Other qualified scopes continue.

- **Disjoint declarations:** [D-10-DISJOINT](DesignAuthorityRegistry.md#d-10-disjoint), [H-527–H-535](../CHANGELOG.md#h-527) and [K-DISJOINT-ENDPOINTS-20261116](../CHANGELOG.md#k-disjoint-endpoints-20261116) cover this revision's constraint-related sections. Existing listed-endpoint ancestry permits inherited overlap; a second independent listed-parent relationship rejects, while prior redundant relationships warn about the path and intermediate methods. Preserve complete public records, per-pair definition/import authority, frozen validation, declared checkedness, demanded checking, owners/captures and D-16. Candidate C-56/C-10/P-476–P-478 serves explicit nominal intent and stable interpretation under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12. [D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) owns acceptance; finite generic/cyclic proof procedures, source implementation and full value-path qualification remain separate. No separate exemption-path store or runtime inheritance mutation.

- **Shared-information candidate:** [D-15-DISJOINT-POOL](DesignAuthorityRegistry.md#d-15-disjoint-pool)/P-479–P-481 under C-59/C-56/C-10 records [H-533's](../CHANGELOG.md#h-533) requested feasibility review. [D-15](ModuleWorlds.md#shared-disjoint-information-per-module-world) owns identity, upstream retention, applicable local/source checks, publication filtering and frozen-backing acceptance. Physical representation and the complete value path remain pending-human; no compiler support is inferred.

- **Module startup and entry selection:** [H-492](../CHANGELOG.md#h-492)/[D-15-CYCLES](DesignAuthorityRegistry.md#d-15-cycles) permits declaration-only cycles. H-493–H-498/[D-15-POSTPON](DesignAuthorityRegistry.md#d-15-postpon)/[D-15-STATIC-NS](DesignAuthorityRegistry.md#d-15-static-ns) requires static runtime namespace definitions first, one statically emitted sequence of implicit dependency prefixes and ordered explicit postpon blocks, then the entry executable prefix. H-499/H-500/[D-15-ENTRY](DesignAuthorityRegistry.md#d-15-entry) selects the namespace containing reserved `__main__`, allowing multiple entry namespaces per dependency package and prohibiting entry reuse by other modules. Candidate C-10/C-35/C-36/C-44/C-45/C-59/C-65/P-443–P-454 serves charter §§2/3.1–3.3/4.2/6.3/8/12 through explicit identities, contexts, readiness and valid native execution. Preserve publication, world isolation, owners/captures, ordinary failure cleanup, D-16 and once-only incorporation. [D-15](ModuleWorlds.md#module-dependency-cycles-and-initialization) owns acceptance and remaining proof/representation/CLI boundaries; [K-STATIC-MODULE-STARTUP-20261116](../CHANGELOG.md#k-static-module-startup-20261116) integrates this revision. No runtime scheduler, compiler-support claim or whole-contract qualification follows.

- **Source fallback and type-carried names:** [H-483](../CHANGELOG.md#h-483) preserves local shadowing and explicit extension; [H-484](../CHANGELOG.md#h-484)/[H-485](../CHANGELOG.md#h-485) selects public-source fallback and deferred information diagnostics at implicit-use sites. [D-15-TYPE-METHODS](DesignAuthorityRegistry.md#d-15-type-methods)/[D-15-LOOKUP](DesignAuthorityRegistry.md#d-15-lookup)/[D-15-LOOKUP-INFO](DesignAuthorityRegistry.md#d-15-lookup-info), C-35/C-45/C-57/C-59/P-425–P-442, supplies candidate value paths to charter §§3.1/3.2/4.2/6.3/8/9: usable definitions, explicit interpretation and visible provenance. D-15 owns r160's lookup acceptance and deferred work; [K-SOURCE-FALLBACK-20261116](../CHANGELOG.md#k-source-fallback-20261116) integrates the fallback model; [K-REEXPORT-METHODS-20261116](../CHANGELOG.md#k-reexport-methods-20261116) continues r159→160 with H-488–H-491, [D-15-METHODS-CLAUSE](DesignAuthorityRegistry.md#d-15-methods-clause) and [D-15-REEXPORT](DesignAuthorityRegistry.md#d-15-reexport). Preserve privacy, explicit publication, identities/worlds, helper contexts, name-before-dispatch and captures. H-486–H-491 adds multilevel fallback with private barriers, independent public re-export and explicit whole-callable bundles, as recorded in [D-15-LOOKUP-TRANSITIVE](DesignAuthorityRegistry.md#d-15-lookup-transitive). General distance algorithms, full value-path/operational qualification and compiler support remain separate.

- **Nominal ADT placement:** [H-478](../CHANGELOG.md#h-478)/[K-SINGLE-ADT-20261115](../CHANGELOG.md#k-single-adt-20261115) covers r155→156's alignment with [D-28-ADT](DesignAuthorityRegistry.md#d-28-adt), P-416–P-418. One CoD ADT retains nominal identity across lexical/regional placement; actual owners provide layout/access evidence. The candidate C-10/C-39/C-65/C-91 path serves charter §§3.1/4.3/6.3/7.1/8 through shared business types and checked storage. Preserve same-owner edges, bits classification, initialization, lifetimes and explicit graph transfer; no C++ regional-type stack allocation, capture/TLS extension, graph copy or runtime support is implied. Acceptance and unresolved physical protocols belong to D-28's composed cases. This scoped human decision and direct integration do not qualify the whole document or settle H-146's encoding mechanism.

- **Registry / current scope:** [D-07](DesignAuthorityRegistry.md#d-07); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole file pending-human.** H-06/H-07 establishes the tracker; C-08–C-11/P-12–P-21 supplies candidate paths. Local decisions route through D-10/D-11/D-14–D-18/D-21–D-24.
- **Value path:** C-10 supplies inspectable typed execution. C-35/C-43/C-45/C-46 through P-84/P-100/P-102/P-109–P-111 and C-56/C-57 through P-182–P-197 concern identity, composition, interfaces and explicit behavior selection. C-39/C-54 through P-94/P-143/P-155 supplies regional lifetime boundaries. C-58/P-199–P-202 adds explicit publication and module-scoped callable composition. These candidate paths serve charter §§2–4; dependency evidence does not qualify them.
- **Contract boundary:** G-01–G-07, DL-01–DL-16 and DD-001–DD-151 retain the decision/acceptance inventory. H-351/H-352 settles library graph-transfer responsibility and IPC/DBMR planning directions, not protocols, ABI, concurrency, snapshot failure or retention. Past work-period permissions are not a new task.
- **Invariants / non-goals:** preserve zero-cost regional relocation, H-478's owner-qualified one-ADT placement, no regional object copy/move, owner provenance, non-consuming capture replay and no dynamic effect/procedure calls from functions. No module-valued expression, runtime interface wrapper, allocator policy, bootstrap/toolchain change or SNN modeling choice is introduced. The confirmed method-selection increment does not settle qualifiers, ownership construction/transfer, standalone abstract signatures or chan semantics.
- **Acceptance:** Examples must distinguish source/caller contexts, visibility, conformance, use/extension permission, unique/ambiguous dispatch and demanded/unused implementations while preserving lifetimes and relocation. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines remain the H records identified by the registry; [K-INFERENCE-CLEANUP-20261114](../CHANGELOG.md#k-inference-cleanup-20261114) covers r154→155: consolidate standalone-declaration semantics against D-10/H-258 and D-24/H-423, preserving unrelated local decisions and pending scopes.

## Declaration annotations

Type annotations use one colon: `val count:i64`, `ref node:Node in O`, `symbol @limit:u64`, `region arena:Root` and `where {region r:NodeStore}`. Callable result annotations use an arrow, such as `function size(node:Node in O) -> const u64 where {owner O}`. Double-colon syntax rejects with a migration diagnostic. `<:` remains the separate subtype token; module paths are quoted; UoM and planned fraction syntax retain their existing roles. H-318 covers this source grammar and migration, with no numeric or representation change.

## Module-world architecture

Each module constructs its own world, explicitly exports information available to receivers, and merges information about identical imported identities in its own copy. Cross-module world editing is impossible at compile time and runtime under H-245. Compilation determines the flattened TLS shape; runtime effects change and restore typed symbolic pointers. Registered symbol objects, types, callable declarations and methods are module-world entities. Definitions/extensions compose those semantics without effects; reached handler definitions install scoped activations. Bare symbol lookup, TLS reads, construction and invocation are distinct. [Module worlds](ModuleWorlds.md) records the construction, publication and execution boundaries.

H-236 requires architecture to govern local lookup behavior. Receiver-owned identity records unify information reached through several aliases; selected callable interpretations retain their distinct contexts. H-235 keeps construction and invocation machinery internal. Ordinary calls do not require a public world object or an invocation protocol. Local-value versus direct-call lookup remains a separate naming question; this architecture does not silently decide it.

H-237/H-238 distinguishes different entity sets rather than two views of the same generic objects. H-239 defines TLS as a flat collection of typed symbolic pointers: symbol and type identities are independent, and a current value may originate from another module-world copy if its type identity is compatible. H-240 gives each module its own semantic world, using Julia as a comparison. H-241 places this architecture in [Global Variables Done Right](GlobalVariablesDoneRight.md). Execution stage is a separate dimension; [D-15](ModuleWorlds.md) inventories the entities, construction rules and their WPO implementation. No cross-module world editor is part of the model.

H-247 defines export as publication of an identity under an alias. H-248 requires identity-centered records with independent local/public aliases; source selection is read-only. H-253 requires conflicting inherited or selected overloads to reject. Future arbitration syntax and full-document qualification remain pending-human.

H-375 confirms actual-type family dispatch, including abstract parameters and results; direct method selection is an optimization. H-376 requires WPO/AOT coverage before execution of the selected composition. H-396 directs shared native method bodies and local micro-dispatch, using concrete-type discriminators and proven facts at call sites; coverage does not require tuple-specific method bodies. The remaining H-251 questions concern physical type evidence, ABI and reflection, not whether a valid abstract call may dispatch dynamically. [Concrete-type evidence](ModuleWorlds.md#actual-type-dispatch-and-concrete-type-evidence) and [D-21](AheadOfTimeExecutionPreparation.md) separate that mechanism work from the settled behavior. Static parallel module interpretations coexist independently of the AOT/JIT strategy.

H-257/H-258 requires nearest lexical callee lookup, shadowing warnings, distinct callable identities and signature-only declarations, and separate evaluated bits/storage places. Declarations require parameter/result signatures and permit generic type parameters and varargs. H-256 reserves fraction comments; H-260/H-261 requires representation-independent quantities and lossless widening. D-16 remains the fraction compatibility constraint; current compiler coverage is recorded separately.

## Confirmed direction

Cross-process send/receive of regional packets and region-backed resources shared through dynamic context are distinct scenarios under H-180. They share storage/lifetime invariants, but capture replay, resource installation and packet consumption are different operations. Neither scenario defines the other’s access or cleanup contract.

These are summaries of the user's statements, not inferred approval of the preceding assistant's recommendations. The original statements and their provenance are recorded in [H-06](../CHANGELOG.md#h-06), [H-07](../CHANGELOG.md#h-07).

| Goal | Confirmed direction | Discussion coverage |
| --- | --- | --- |
| G-01 | Use a memory region (`mem_region` in the discussion) as an IPC packet. Its header includes the total binary length. After complete reception, invoke a designated member method on the packet's entry object to apply it. | DL-02, DL-04, DL-05 |
| G-02 | Use DBMR as the local ontology database, supporting business retrieval, statistics and CRUD. Ordinary operations do not reclaim space; periodically rebuild its payload and backing file for compaction. | DL-06–DL-09 |
| G-03 | Give the domain language explicit ADT semantics, including sum/product types, method invocation and memory management. | DL-01–DL-03 |
| G-04 | Provide C/C++ compatibility interfaces for interoperability with other language runtimes. | DL-11 |
| G-05 | Advance language and SNN design, implementation and validation together, using simulator requirements to choose useful initial language features. | H-96; DL-12–DL-16 |
| G-06 | Nominal symbols, Actor-local scoped effects, captured/current procedure invocation, lexical functions and lexical or symbol-bound procedures, with whole-program call-path checking. | H-96–H-98, H-102–H-104, H-180; DL-02, DL-14, DL-15 |
| G-07 | User storage is stack or `@region`. Captures are bits or regional pointers. Each region contains one direct-pointer graph. Relationships between regions are logical business associations. | H-99/H-100/H-156/H-157; DL-03, DL-08, DL-15 |

H-352 assigns update/versioning behavior to the selected library's `<-` methods rather than one mandatory DBMR-wide policy. Actor-model design supplies language-level facilities for application outcomes and recovery; applications choose their business protocol. Snapshot save/replace and intrinsic SPI support are planning directions, with their operational interfaces still to be designed.

## Scope and invariants

The discussion joins language semantics, IPC entry execution, DBMR operations and compaction into one coherent design. It also tracks how the language relates to the charter's typed business graphs, ontology, effects, multi-region integration and readable source.

The governing context is [the charter](RegionalSemanticPlatformCharter.md), especially §§2–8, 11–14, with the human-confirmation limits recorded under D-00. Repository rules require zero-cost whole-region relocation, region-only allocation for regional objects, no regional copy/move semantics, and explicit region-aware references and lifetimes. A language proposal must explain how it preserves these constraints. They do not imply that object-by-object compaction is free.

H-96 puts coordinated language/application development in scope. Each implemented increment needs a bounded contract grounded in the recorded human decisions; the direction does not select all mechanisms or authorize toolchain/configuration changes. Build and test operations retain the repository rules. It does not select a concrete parser, type theory, transport, transaction model, concurrent collector, wire ABI, query language or code-generation dependency. The scope of the first language version is open; broader GADT, kind-level and multi-domain goals remain in view when choosing a smaller initial scope.

`apply`, `Result`, `execution_context` and similar names below are explanatory notation, not chosen syntax or API names. H-143 selects direct LLVM IR for execution lowering; C++ supplies native system services and compatibility boundaries. C++ compatibility does not determine the domain language's semantics.

## Discussion dashboard

**Status vocabulary:** `open` means discussion is needed; `proposed` means a candidate answer is recorded; `confirmed` requires an explicit human decision for the stated scope/revision; `superseded` preserves an older decision and links its replacement. Topic progress and implementation qualification are separate: a topic can contain a useful proposal while its design authority remains `pending-human`.

The sixteen topics contain confirmed local propositions and open implementation/design questions. DL-14/DL-15 link the current symbol, callable and storage decisions; DL-16 links the application-owned development plan. Topic completion does not follow from a direction alone.

| Topic | Decision to reach | Related topics |
| --- | --- | --- |
| [DL-01](#dl-01) | Semantic units, ADTs and the initial type/ontology core | DL-02, DL-08, DL-10 |
| [DL-02](#dl-02) | Methods, effects and entry-object execution | DL-01, DL-03, DL-05 |
| [DL-03](#dl-03) | Region ownership, borrowing and allocation | DL-04, DL-07, DL-08 |
| [DL-04](#dl-04) | Packet lifecycle, framing and binary compatibility | DL-01, DL-03, DL-08 |
| [DL-05](#dl-05) | Application outcomes, durability and retry behavior | DL-02, DL-06, DL-09 |
| [DL-06](#dl-06) | Database updates, visibility and history | DL-05, DL-07, DL-09 |
| [DL-07](#dl-07) | Retrieval, statistics and indexes | DL-03, DL-06, DL-08 |
| [DL-08](#dl-08) | Identity, references, versions and domain integration | DL-01, DL-03, DL-09 |
| [DL-09](#dl-09) | Compaction, retention and backing-file publication | DL-05, DL-06, DL-08 |
| [DL-10](#dl-10) | Source representation, diagnostics and semantic editing | DL-01, DL-08, DL-12 |
| [DL-11](#dl-11) | C/C++ and foreign-runtime interfaces | DL-02, DL-03, DL-04 |
| [DL-12](#dl-12) | Initial implementation scope and execution strategy | DL-01, DL-02, DL-10, DL-11 |
| [DL-13](#dl-13) | Worked examples, acceptance and development timing | All topics |
| [DL-14](#dl-14) | Symbols, dynamic bindings and whole-program call-path checking | DL-02, DL-08, DL-12 |
| [DL-15](#dl-15) | Callable kinds, expression results, storage and captures | DL-01–DL-03, DL-14 |
| [DL-16](#dl-16) | Paired language/SNN implementation and validation increments | DL-12–DL-15 |

## Topic records

<a id="dl-01"></a>
### DL-01 — Semantic units, ADTs and ontology

**Question:** What does the language define and preserve: domain types and values, method/program graphs, ontology relations, or which initial subset of them?

- Preserve strict branded identity under H-125 and independent compile-time UoM identities under H-128; structural layout alone does not establish equivalence. Use the named sum constructors and [case contract](#case-expressions-and-branch-local-variables) for ordinary alternatives and advisory coverage. Recursive types, equality and admissible regional fields retain their own scopes. Keep semantic type identity separate from physical layout.
- Distinguish ordinary ADTs from indexed GADTs. Specify the initial role of type parameters, kind checking, inference and type equivalence without silently treating the full long-term type theory as already selected.
- Decide which ontology relations and constraints are first-class, what is explicitly asserted versus inferred, and how independently defined domain concepts can be mapped. Similar field layouts do not establish conceptual equivalence.
- Explain how domain signatures can be loaded and checked without requiring every domain constructor to become a fixed host-language enum. Distinguish published definitions from parser/solver scratch state.

**Current scalar discussion:** [ScalarUnitsAndIndexTypes](ScalarUnitsAndIndexTypes.md), D-11, compares newtype-style wrappers with UoM arithmetic against actual pool formulas. Arrays declare an index type; scalar operations must preserve or explicitly transform its meaning. H-128 makes UoMs independent named compile-time entities with first-class type identities, same-name distinct definitions and identity-preserving local aliases; they are not represented by symbols. H-130 supplies pure conversion methods with Julia-style argument-type dispatch; H-134 specifies fresh UoM definitions and one-shot using selections, with no UoM import. H-135–H-138 specify parameter-controlled conversion and compile-time route choice. H-131 adds parenthesized operator names such as `(+)` for readable method definitions; D-11 distinguishes pure conversion/index functions from the mixed function/procedure operation overloads specified by H-153–H-155/H-158/H-159 in D-10; unary/binary methods use argument count and types. H-132 plans compound UoMs for future physical-quantity use and defers their implementation; compound equivalence/inference does not block this increment. Index origins and literal rules remain open within the bounded interface discussion. H-126 requests analysis before selecting the mechanism.

**Planned fraction compatibility:** [FixedWidthFractions](FixedWidthFractions.md), D-16, records H-254/H-255: intrinsic fixed-width `i64/i32`-style bits representations independent of UoM, unreduced arithmetic, explicit same-type `canonical`, `//` construction/output and `#` line comments. The compiler reserves `//` with a diagnostic and uses `#` comments under K-401. Fraction implementation is deferred. K-401/K-402 establish the separate H-256 comment/UoM prerequisites; H-260/H-261 supplies independent unit declarations, typed quantities and lossless primitive widening. Subsequent language designs and implementations must check this plan; conflicts with accepted requirements are design obstacles for human discussion before implementing the affected choice. Exact promotion, layout, qualifier and precedence rules remain open. This does not introduce denominator units, arbitrary precision or fraction-specific ownership.

**Proposal to discuss:** start from a small explicit sum/product and method core whose checked representation can express business logic and evolve toward the charter's GADT graphs. Decide the necessary indexed features from concrete programs.

**Decision evidence:** one domain definition, one business program with a result-dependent branch, an invalid construction that is rejected, and the exact boundary between value, type and kind representations.

#### Abstract types and data declarations

H-209 separates declarations by purpose: `type` declares an abstract type; `data` declares a concrete product/sum representation. H-210 uses `<:` to declare subtype relationships. Both are type definitions with nominal module-defined identities; data is not a value declaration, and an abstract type is not a symbolic dynamic variable.

Illustrative product and abstract declaration headers follow. Abstract declarations have no Interface body:

```text
type RegionStore;
type NodeStore <: RegionStore;

data ArenaRoot <: NodeStore { used:u64; limit:u64; }
data PoolRoot  <: NodeStore { head:u32; count:u32; }
```

The abstract NodeStore <: RegionStore edge is nominal and inherits methods. Data-to-abstract <: explicitly declares interface conformance under H-218; it is required rather than informational. Neither relationship implies shared fields or offsets. The illustrative names/headers specify no allocator behavior and are not executable fixtures. Type aliases preserve the original nominal identity, independently of whether a conformance declaration is visible in the receiving module.

| Declaration role | Design boundary |
| --- | --- |
| Abstract type | Declares a nominal abstraction; its methods are separate declarations that may be distributed across modules. There is no Interface body or directly constructible layout. |
| Product data | Declares the concrete fields/constructor of a region-resident user object. |
| Sum data | Declares the fixed alternatives and payloads described under [named sum constructors](#named-sum-constructors). The physical tag representation remains open. |
| Intrinsic type | Primitive bits, region ownership and intrinsic owner-bearing outcomes retain their language-defined rules; a user data declaration cannot manufacture those privileges. |

Abstract-to-abstract <: stays nominal; similar method signatures do not make abstract declarations parent/child or identical. Multiple abstract parents remain deferred. A concrete data identity may explicitly conform to several abstractions through separate declarations; this is not concrete-layout inheritance or a second data representation.

H-218 requires a declared relation for data applicability to an abstract constraint. Matching method signatures or fields cannot manufacture that relation. With an available declaration, the compiler can select methods constrained by the abstraction and then validate each demanded call's signature, access, effects, lifetime and visible implementation. An uncalled missing implementation does not invalidate the declaration. There is no Interface body, eager complete-body inventory or structural-conformance inference.

Head-only declarations refer to the same separately defined data identity and may occur in different modules. Illustrative adapter modules:

```text
# adapter-a.cod
import "model.cod" SomeADT;
import "interfaces.cod" SomeAbsType;
data SomeADT <: SomeAbsType;

# adapter-b.cod
import "model.cod" SomeADT;
import "interfaces.cod" AnotherAbsType;
data SomeADT <: AnotherAbsType;
```

H-527–H-535 adds [disjoint declarations with existing endpoint-ancestry exemptions](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) as a separately scoped constraint on the fused inheritance graph. A shorter conformance declaration for an already established indirect relation accepts with a warning: show the existing path and explain that intermediate abstractions and their method contracts remain applicable. This does not add a new independent path, erase an old path or eagerly require every method implementation. Publication carries the original record; H-532 requires a locally defined or explicitly imported endpoint for each newly constrained pair. A subtype cannot independently select two listed parents: X <: A followed by X <: B rejects even when B <: A is exempt; X <: B followed by X <: A only restates an existing relationship and warns. This general rule also prevents adding Error conformance to an established checked exception. The existing type graph supplies exemptions without a separate path store. D-10 retains finite generic/cyclic proof boundaries.

Neither adapter defines fields, allocates an object or creates another identity named SomeADT. The declarations add explicit relationships between resolved identities. A relation to a child abstraction can reach its nominal ancestors; satisfying multiple unrelated abstractions does not rank those abstractions against one another. If methods constrained by two unrelated applicable abstractions are equally specific, ordinary unique-best dispatch still reports ambiguity rather than choosing a declaration order.

H-220 distinguishes declaration permission from consumer visibility. An adapter must explicitly import the ADT before declaring `data D <: A;`; merely using D supplies no permission to extend it. A third-party consumer can use the adapter to obtain the declared conformance, without acquiring extension permission. The abstract endpoint must resolve to its intended type identity; H-220 does not impose an additional import requirement on that endpoint just to name the relation.

Illustrative consumer selection with the existing resource-selection shape:

```text
using "adapter-a.cod" SomeADT;
```

This source view can carry the adapter's declared conformance together with the existing data identity. An import of that adapter may additionally grant extension permission; using suffices for conformance-aware calls. Exact association, batch selection and re-export syntax remain to be completed. Module inclusion alone is not a substitute for selecting the adapter's exposed resources. Using SomeADT from its original representation module does not automatically discover relationships supplied only by an unrelated adapter. Aliases retain the same data identity; selecting several adapter views should compose their visible relationships without creating alias-specific types.

A declaration supplies admissibility, not a method body, storage permission or promise of complete behavior. Generic parent implementations become candidates through declared relationships, not through circular proof that those candidates themselves demonstrate conformance. The previous circular structural-inference issue therefore no longer requires a solver; method lookup, inherited applicability and demanded body checking still require precise rules. Duplicate declarations of the same identity pair should be idempotent evidence rather than repeated methods; that diagnostic/merging policy remains proposed.

Methods for an abstraction may be declared in several modules. There is no Interface body and no requirement for the defining module to enumerate or implement a closed method inventory. A module may provide one operation and another module a different operation; absence of an unused implementation does not invalidate the type or its claim. A demanded call must find a visible, applicable implementation during WPO compilation. “At the call” means compile-time checking of the demanded program path, not a runtime missing-method trap. Ambiguous calls, invalid signatures, forbidden effects and lifetime errors retain their existing checks.

Inheritance and visibility are independent: inheriting a method does not make an unselected module globally visible. Conformance declarations may reach consumers through using or import of the adapter; declaring new conformance on an external ADT requires import permission. Source-preserving using/import retains the body's lexical identities and captures; mixin supplies a new receiving definition. The current prototype combines source methods with caller-selected ordinary implementations and preserves established conformance of concrete arguments. H-230 changes the target identity boundary to extension-module identities; H-232 makes using preserve a source helper's implementation context and import adapt it to the receiving module, as specified below. Inherited applicability does not require cloning a body, recapturing effects, or allocating an object-layout prefix. Inclusion of inherited/generic methods in a type's source-visible selection also remains part of the association contract.

Neither nominal abstract inheritance nor an explicit data conformance declaration implies a shared concrete field layout. Code constrained by NodeStore uses the applicable visible methods; field access still requires a concrete representation or a separately valid operation. A region must be created with a concrete supported root representation, rather than attempting to allocate an unspecified abstract root. Root matching retains the actual concrete RT and owner instance.

Generic argument matching and generic subtyping must remain distinct. The intended root method can constrain a parameter R by `R <: NodeStore` and receive region :R; this does not automatically establish `region :ArenaRoot <: region :NodeStore` as a mutable-value conversion. The generic bound/header syntax is still proposed. Similar constraints apply to array element types, const access and owning qualifiers; <: cannot erase ownership or permission differences.

Sum alternatives and abstract descendants serve different roles. A nominal data sum declares a fixed set of alternatives suitable for exhaustive matching. An abstract family may acquire nominal abstract descendants and explicitly conforming data types in other modules; neither current set is an implicit closed sum. The defining sum determines alternative identity. A selected module world may add initializer overloads for an existing data constructor to the parent type’s constructor overload family. Future GADT constructors may refine indices of the declared data family; subtype syntax alone does not define those refinements or permit unchecked casts.

Storage classification is independent of nominal declaration identity. H-563 excludes callable-view values from regions; H-567/H-568 confines whole-payload transfer to pure bits and rejects C++ object copying/moving semantics. D-22 reviews return-only structural result signatures and named mapping to independent lexical variables. This candidate introduces no first-class aggregate or stored field and leaves ordinary ADT field permissions unchanged; callable-bearing named ADTs remain a separate unapproved extension. H-383/H-384 admits recursively finite bits ADTs to lexical or regional storage; stored references, ownership carriers, managed environments and implicit cleanup exclude that category. H-478/D-28-ADT extends placement to same-owner pointer-bearing lexical graphs while retaining the same nominal ADT. Full native layouts/initialization coverage remain outstanding; regional instances still use relative edges and C++ regional types remain region-only. The `data` keyword alone does not establish bits eligibility, immutability or GC. [D-22](ConstructionAndAcquisition.md#confirmed-requirements) records the confirmed boundary and its incomplete implementation. Ordinary references borrow, const non-bits references restrict holder writes without freezing aliases, and owned remains an intrinsic resource responsibility. User data containing owned-region fields, including through variants/generic instantiation, remains forbidden by H-196. Builtin resource optional/results are a separate intrinsic facility.

Static abstract bounds on a selected named root do not require a runtime object wrapper or a whole-region-to-object conversion. Persisting heterogeneous abstract-typed fields or dispatching on a runtime sum nevertheless needs a sound representation/discrimination contract; WPO does not make runtime alternatives constant. No object header, fat pointer or general inheritance runtime is selected solely to support the first root-policy method.

**Primary-language comparison:** Julia explicitly declares nominal subtyping, makes abstract types non-instantiable and concrete types non-subclassable, and does not generally make parametric types covariant. These are useful reference points; CoD has its own data sums, storage and ownership semantics. [Julia types](https://docs.julialang.org/en/v1/manual/types/)

**Prototype implementation:** `type A;` and `type B <: A;` declare nominal abstractions; `data D { ... }` declares a region-only product, and `data D <: A;` adds explicit conformance without layout changes. An inline data parent is also supported. An adapter imports the data identity before adding a relation. Selecting that data type through the adapter carries its source-visible relations, including relations already selected by the adapter; selecting only an unrelated source method does not publish them. Type-associated methods can be selected implicitly. Abstract parameter/result checks preserve the concrete representation and its established explicit conformance while WPO specializes the call. Conditional values with the same concrete type and owner retain the most specific nominal ancestor guaranteed by both branches; this alone does not establish a child-interface claim. Concrete storage/provenance remains unchanged. `type { ... }` and `regional type` are rejected. Concrete data requires regional allocation and explicit references for stored object edges in the current syntax. The native extension/union identity and export subset is described below; compound cross-alias context/extension policies, sum constructors and generic syntax remain unfinished. Repeated identical conformance declarations and abstract stored-reference layouts are explicitly outside this subset, not selected permanent language policies.

A library fold accumulator retains its explicit type guarantees and facts for its concrete value and possible stored-reference targets, including cycles. Unrelated facts visible only in a factory module do not become loop invariants. This is evidence preservation, not structural interface inference; unknown layouts cannot justify erasing required guarantees.

Lean acceptance must show one data identity with head-only conformance declarations in different modules, rejection of structural lookalikes without a declaration, adapter-import permission versus using-only declaration rejection, and consumer acceptance through using the adapter, invalid direct abstract construction, deterministic unique-best selection and an ambiguous crossed-specialization rejection. Product/sum construction and exhaustiveness need separate acceptance cases; no full GADT or general multiple-inheritance implementation is implied by this declaration split.

#### Constructor and call-operator method families

A nominal data type owns its constructor overload family. Each data constructor (dctor) is a method of that family. The type can also provide a call-operator family. The source operation selects the family before overload resolution, even when the explicit argument lists coincide.

| Expression | Method family | Instance at entry | Result |
| --- | --- | --- | --- |
| `make Accepted(7u64) in batch` | Constructor family of `Receipt`, selecting its `Accepted` dctor | The destination supplies its owner and the selected fields as initialization places. | A reference to the initialized instance on success. |
| `r(7u64)` for `r:Receipt in batch` | Call operators for the data receiver | The receiver supplies a reference to an initialized instance with its actual owner and access. | The selected method’s ordinary result. |

Adding a call operator leaves the initializer count of every constructor unchanged. A constructor overload alone cannot make a data value callable. See [named sum constructors](#named-sum-constructors) for the field contract and [functor receivers](#functor-receivers-and-callback-arguments) for receiver access and here arguments.

#### Typed enums

A [typed enum](TypedEnums.md#declaration-and-value-identity) declares a closed nominal bits type with named members. Two member names can denote one value for source compatibility. Select members using the type selector and match them in parenthesized case heads. An enum member requires no allocation or constructor call.

#### Named sum constructors

Each alternative of a nominal sum has a directly visible data constructor name. The same identity selects the alternative in `make Accepted(...)` and tests it in `r isa Accepted`. The predicate checks the active alternative without calling an initializer. An initialized null ptr tests false. A successful ptr test refines the target and non-null facts while the binding and the active alternative remain stable. The parent sum supplies the nominal result type.

```cod
data Receipt =
    Accepted { id:u64; ptr prior:Receipt; }
  | Skipped  {};

func Accepted(raw:u64) -> ref Receipt in O where {owner O} {
    assert raw != 0u64;
    id <- raw;
    prior = null;
}

# batch is a live writable owner.
ref a:Receipt in batch = make Accepted(42u64) in batch;
ref s:Receipt in batch = make Skipped in batch {};
```

The destination owner comes from ordinary `make` placement. The header may omit the result annotation because the constructor declaration already determines the parent sum, reference mode and actual destination owner. An explicit annotation must agree. The body can name the supplied owner with `where {owner O}`. The construction destination supplies this owner even for a fieldless alternative. Writing an ordinary callable result annotation still cannot invent an owner.

The selected alternative’s fields enter the ctor as individual initialization places. Here they behave as `init val var id:u64` and `init ptr var prior:Receipt in O`. Field writes, reads of initialized fields and delegation to an `init` helper follow the [place rules](ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing). Explicit parameters determine overload matching and can include references to other initialized objects. A parameter with the same name as an implicit field at the same binding level is a duplicate declaration.

The object under construction has no initialized instance receiver anywhere in the ctor body or its deferred cleanup, including after the last field write. A destination address passed by the compiler grants only the source-level field permissions. The ctor follows ordinary argument evaluation, lexical scope, effects and cleanup. Every normal exit must establish that all fields of the selected alternative are ready after cleanup. Only then does `make` deliver the reference. An escaping failure delivers no completed result. Bare and value-bearing returns targeting the ctor are forbidden, including from its deferred actions. A nested callable or standalone `{{ ... }}` owns its own returns.

Within the type’s merged constructor family, check for user-declared initializer overloads **for the named dctor, before testing applicability**. With the declaration above, `make Accepted in batch { id <- 0u64; prior = null; }` rejects and `make Accepted() in batch` reports a missing applicable overload. A `proc` initializer still counts when the call site only permits a `func`. Skipped has no initializer overload, so its empty block supplies a complete payload. An initializer-free alternative with required fields must initialize each on every normally completing path. For a sum, reserve a named alternative under the [sum construction rule](#sum-object-construction-and-reference-storage). A constructor-backed alternative needs its initializer before it can become an ordinary readable result.

#### Sum object construction and reference storage

For a nominal sum, the operand naming the object in `make` must select a data constructor. With the declaration below, `Node` is the type accepted by variables, parameters and reference fields. `Link` and `End` select layouts for construction. Reject `make Node in graph`, `make Node() in graph` and `make Node in graph { ... }` even when the receiving type or some field names are known. A product such as `data Pair { left:u64; right:u64; }` retains its existing `make Pair` spelling: the product name also identifies its sole constructor. Scalar allocation and intrinsic resource construction retain their own forms.

```cod
data Node = Link { key:u64; ref next:Node; } | End {};

# No Link initializer overloads in this world. graph is a writable region.
ptr a:Node in graph = make Link in graph;
ptr b:Node in graph = make Link in graph;
a.key <- 1u64;
b.key <- 2u64;
a.next = b;
b.next = a;
ref first:Node in graph = a;  # The required component is ready here.
```

Bare `make Link in graph` reserves one target with a known alternative and a stable identity. Forming destinations for its Link fields uses that construction evidence. Binding another edge can use the identity before the payload is readable. An ordinary payload read still requires initialization. `make End in graph {}` completes a fieldless alternative with zero overloads. `make DCtor(args) in graph`, including a call with zero arguments, selects an initializer by the ordinary overload rules. If Link has an overload, scattered field writes cannot substitute for it. For a ref group with explicitly reserved targets, [`init a(args)`](ConstructionAndAcquisition.md#in-place-construction-in-a-ref-group) invokes that alternative’s initializer on the existing target. A constructor can bind a same-owner edge to a peer supplied as `not init ref`. Its fields become ordinarily readable when the required component is ready.

A declaration such as `ref r:Node in graph;` makes a local binding whose eventual non-null target must belong to graph. `ptr p:Node in graph = null;` gives a local binding an initialized absence with that same target owner. An uninitialized slot has no readable reference identity. Neither declaration allocates a Node. Named roots and fields can already store references, and `var edge -> graph.head;` can name an existing root slot.

The forms `make ref Node in graph` and `make ptr Node in graph` allocate anonymous reference and pointer slots, respectively. In this regional example the result denotes a writable place in graph, subject to the same regional rule as a stored edge: every non-null target must also belong to graph. Whether a lexical slot points into another owner is already expressible by the named lexical declaration above. The new expression supplies a place to a declared place alias or compatible init-place parameter. It creates no Node and supplies no implicit forward reference.

```cod
var edge -> make ptr Node in graph;  # New uninitialized regional pointer slot.
edge = null;                         # Now reading its identity is valid.
ptr saved:Node in graph = edge;      # Independent lexical binding, currently null.
edge = make End in graph {};
# saved is still null.

var required -> make ref Node in graph;
required = edge;  # edge is known non-null here; required = null would reject.
```

Before the first binding write, `edge is null` would read an uninitialized slot. Passing the slot to a compatible `init ptr var` helper instead is valid. After binding, `=` changes the selected target and `<-` writes through it where the payload type admits that operation. A declared place alias or init-place parameter can use this storage directly. Anonymous lexical slot placement with a separately specified target owner is outside this scope.

#### Publishing and selecting data constructors

Exporting a nominal sum type publishes its constructor overload family. Every named alternative, including one with zero initializer overloads, has a dctor identity in that family. Selecting a dctor by name brings all its visible initializer overloads. The same identity determines `r isa Accepted` and which entries of the constructor family are eligible for `make Accepted(...)`. A product’s name identifies its sole dctor.

The receiving module [merges contributions from each selected view of the same nominal type](ModuleWorlds.md#merging-type-owned-method-families). The `make` site uses the resulting family. Equal explicit parameter signatures on distinct dctors, such as `Accepted(u64)` and `Skipped(u64)`, coexist because the construction expression names the dctor. Two independent conflicting definitions of `Skipped(u64)` reject the module composition even without a `make Skipped` expression. An overload confined to a parallel world has no effect.

The constructor selector goes after the existing module source and type name:

```cod
using  "model.cod" Receipt {Accepted};          # Select Receipt and the complete Accepted constructor.
import "model.cod" Receipt {Accepted, Skipped}; # Explicitly import both named dctors with their overloads.
import "model.cod" Receipt {..};                # Explicitly import all Receipt constructors.
```

Names in the list must denote constructors of the selected type. A misspelling or a constructor of a different type rejects. `using` preserves the published implementation context. `import` permits adding overloads to the selected constructor in the receiving world. Such an extension preserves the underlying alternative identity, so values remain recognizable across worlds that select that identity. The selector names constructors. Ordinary type-associated methods keep their existing `with methods` selection. No module operation selects an individual initializer out of a constructor.

```cod
# model.cod: Receipt includes the fieldless Skipped and declares no Skipped initializer.
export Receipt;

# audited.cod
import "model.cod" Receipt {Skipped};
func Skipped(reason:u64) { assert reason != 0u64; }
export Receipt;

# A client using the Skipped constructor from model.cod:
ref first:Receipt in batch = make Skipped in batch {};

# A different client using the Skipped constructor from audited.cod:
ref next:Receipt in batch = make Skipped(7u64) in batch;
# make Skipped in batch {} rejects here, even though Skipped has no fields.

# ranges.cod contributes a different overload for that same dctor.
import "model.cod" Receipt {Skipped};
func Skipped(first:u64, last:u64) { assert first <= last; }
export Receipt;

# app.cod selects both views of Receipt.
using  "audited.cod" Receipt {Skipped};
import "ranges.cod" Receipt {Skipped};
ref one:Receipt in batch = make Skipped(7u64) in batch;
ref span:Receipt in batch = make Skipped(7u64, 9u64) in batch;
```

A constructor list makes the listed names explicit. For `using "model.cod" Receipt {Accepted};`, ordinary public-source fallback can still find Skipped at lower priority. That implicit selection grants no extension permission. Selecting a subset leaves the full set of alternatives intact for type checking and coverage diagnostics.

Haskell 2010 spells the analogous import `import Model (Receipt(Accepted))` or `import Model (Receipt(..))`. Within explicit lists, bare `Receipt` selects the type alone. Haskell’s wildcard also covers exported record field names. CoD uses its ordinary `export Receipt;` to publish all constructors and their overloads. [Haskell 2010 Report, §§5.2–5.3](https://www.haskell.org/onlinereport/haskell2010/haskellch5.html)

#### Case expressions and branch-local variables

The full source form is `case subject of { (pattern) | guard -> { body }; ... }`. A guard is optional. In the full form, parentheses delimit the pattern and preserve the subject’s established result mode. Parentheses written around the subject itself follow ordinary expression rules. Ordinary statements may appear between arms. An arm header and its `->` must be a direct item of the body of the case that owns it. An ordinary group, `if`, full scope or callback nested in that body cannot declare another arm of that case. A nested `case ... of` owns a new arm sequence. Arrows in other grammar productions, such as `value -> parameter` in a call, retain their ordinary interpretation. After `->`, single braces group statements in the enclosing callable and double braces invoke a full scope. An empty header `()` and a wildcard `(_)` accept any subject without binding it, including a null pointer. A written bare `|` before `->` supplies no predicate. Thus `() | -> {{ ... }}` is a catch-all with a local return. With `() | ready() -> {{ ... }}`, call ready once when reached and enter the body only if it returns true. A trailing here argument uses `=>` and has its own handoff contract. The seed [expression grammar](../cod/lang_syntax.hh) provides `if` with `else`. Haskell’s case expressions supply the precedent for constructor patterns, source order and guards over pattern-bound names. [Haskell 2010 §3.13](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html#x8-460003.13)

**Short arms.** At the start of a direct case item, `Name -> expr` tests a visible constructor of the subject sum or a member of the subject enum exactly as `(Name) -> { expr }` does. A constructor can have fields that this arm ignores. Enum member lookup uses the subject’s member table, including alternate spellings. When the tokens form a direct short head, that pattern interpretation takes precedence over an ordinary expression starting with the same name and arrow. A bare name introduces no local. A constructor followed by a braced receiving specification introduces the same branch-local bindings as the parenthesized constructor pattern. Either head, an enum member or a wildcard can take an optional `| guard` before the arm arrow. The pattern parentheses are optional in each of these forms. For every case subject, `_ -> expr` is the unguarded wildcard `(_) -> { expr }`, including for null. The author chooses to cover all remaining values and assumes responsibility for what happens when a sum or enum grows. Using a wildcard alone calls for no diagnostic.

```cod
return case slot of {
    Vacant -> false;
    Occupied -> true
}
```

The parser recognizes the direct head from its tokens. The checker resolves the name once the subject type is known. An invalid name is a pattern error. Nested arrows, such as the argument label in `Occupied -> report(value -> code)`, use their enclosing construct’s grammar. After a short head, a single result expression contributes the same result and effects as the sole expression in a single-brace arm. A written semicolon separates items and preserves the result. A closing brace ends the last expression. Between items use a separator wherever the grammar requires one, irrespective of line breaks. Named and wildcard heads can also introduce the usual braced bodies. A standalone `|` introduces the guard after the complete pattern. The usual guard grammar owns nested calls and their argument labels, so `Name | eligible(n -> cartons) -> n` has one arm arrow. The tokens `||` and `|>` keep their operator meanings. Typed binding patterns retain their full header, for example `(ref n:Node in O) | ready(n) -> { use(n) }`. Multiple operations and explicit control transfers use the existing braced bodies, with the established `_ -> throw;` fallback below. [Newlines and semicolons](SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux)

Using the Receipt declaration above, suppose `latest` is writable storage independent of the receipt fields. In this example `audit(n)` records n and returns bool.

```cod
proc classify(ptr r:Receipt in O, latest:u64 in O) -> u64 where {owner O} {
    return case r of {
        (null) -> { 0u64 };
        Accepted { val n <- id; latest <- id; } | audit(n) -> n;
        Accepted -> latest;
        _ -> 0u64
    };
}
```

**Entry order.** Evaluate the subject once. An independent bits value retains its bits. A borrowed subject retains its selected target, actual owner and locator. Rebinding the source variable cannot redirect this selection. Try headers in source order. A constructor test uses the identity and active alternative established by `isa`. Overload selection, initializer arguments and their effects play no part. Only after the test succeeds may the spec receive fields of that alternative. Check the spec as a group using ordinary field-label, initialization, owner/access and overlap rules, then evaluate the optional Boolean guard once. A pattern miss or a false guard skips that body and resumes at the next source item. An ordinary statement reached between arms executes under the enclosing lexical permissions. An unguarded match or true guard enters the body once. Normal body completion finishes the entire case immediately. An executed `fallthrough;` leaves the arm and resumes at the next source item. Each later header runs its own test, reception and guard if reached.

**Branch-local variables and effects.** A declaration in the spec introduces a branch-local variable. Its name is visible to this guard and body, and remains unavailable to statements between arms, sibling arms and code after the case. Simultaneous receiving items resolve their inputs and existing destinations in the preceding environment. Reusing the same local spelling in a sibling introduces a fresh binding. As with loop controls, a header can introduce its pattern bindings with either body spelling. A single-brace arm body forbids direct variable declarations, including through nested ordinary groups. A double-brace body may declare new variables local to that invocation. Its new declarations become visible in ordinary source order inside the body; the guard cannot see them. The arm has direct access to its enclosing lexical context, with the enclosing callable’s ordinary effect permissions and control destinations. Single braces supply no independent callable, cleanup scope, or implicit storage owner. Ordinary exits end the branch bindings. Escaping references, including deferred uses, need their own lifetime proof.

A constructor mismatch in the example skips both reception and `audit`. If Accepted carries id 7 and `audit(7u64)` logs a rejection and returns false, reception has already written 7 to `latest`. The log and the write persist. The next Accepted arm can read the current outer slot. A guard or receiving operation that raises propagates the failure with its ordinary checked obligations. The compiler must preserve the observable order and frequency of attempted operations. A programmer who wants a write conditional on the guard puts the write in the body.

**Fields and live references.** `(Accepted)` tests only the tag. `(Accepted { val n <- id; ptr previous = prior; })` also fetches the id bits and binds the independent nullable reference slot to the selected prior target in its actual owner. A null prior is a valid received value. Omitted fields need no payload read. A readiness proof for the tag alone cannot authorize reading unfinished fields. A branch-local ref to an inline field needs the containing target and the field’s active alternative to remain valid at each use. A saved independent reference to another object retains that object’s own lifetime requirements. A guard may rebind an outer slot or call a procedure; use normal flow invalidation if it changes a necessary fact. The match supplies no snapshot of a graph. Further discrimination of a field can already be written as a nested case. The baseline uses an inner case for nested matching. It specifies no recursively nested field-pattern shorthand.

**Types, assertions and coverage.** Typed heads such as `(ref n:Node in O)` test a present target’s membership under the existing nominal/conformance rules, then bind it with the proven owner and access. A null or nonmember can proceed to the next arm. A receiver annotation inside a constructor’s field spec has its ordinary declaration/receiving meaning. Readiness, owner, readonly or lifetime violations are static errors. For a value subject, `(val n:u8)` can match an actual u8. It performs no narrowing conversion on a u64. For scalar alternatives, built-in Boolean and fixed-width integer literals use typed value matching. Numeric pattern extensions preserve unit and fraction type identity and the independent D-16 conversion rules.

An explicit `x : T` in the subject, guard or body is an ordinary expression. For example, inside an arm receiving an abstract Entity ref, `ref n = x : Node in O;` may raise TypeAssertionFailedError. A proved impossible assertion rejects statically. Nullable-to-ref requires a prior non-null fact. For example, given a nullable Entity slot p, `p isa Node` as a guard tests membership and establishes non-nullness on success. A later rebind of p requires fresh evidence for that slot. An ordinary numeric `x : u8` retains the specialization-based conversion and potential-loss rules. A failed `assert predicate;` raises AssertionFailedError under its own contract. None of these exceptions implicitly retries matching. [Type tests and assertions](SymbolicDynamicScope.md#expression-type-conversion-and-assertion)

**Coverage diagnostics.** A void case may be partial, including over an open abstract type. A value-producing case requires static evidence of completion, supplied by exhaustive arms or by an explicit fallback. Both can benefit from diagnostics about omitted constructors or null, guards that can decline, and shadowed arms. A partial import leaves the declared sum intact. The [compiler-wide diagnostic policy](../applications/cod-booter/README.md#diagnostics-by-compiler-stage) assigns additional coverage analysis to the CoD-written compiler. Definite initialization, owner validity and sound field access remain mandatory.

Join normal arm results under the [conditional result rules](ConstructionAndAcquisition.md#conditional-results). A single-brace `return` exits the enclosing callable. A double-brace `return v;` supplies this arm’s result after its own cleanup, and execution continues with the case’s consumer. A normally completing double-brace value tail supplies the same local arm result. Prepare it before running the arm’s [full-scope cleanup](ExceptionControl.md#full-scopes-as-immediate-ordinary-calls). Do not return a reference to that invocation’s fresh storage. A reference to a sufficiently long-lived outer target retains its ordinary owner. An early case break also supplies a result. Paths that leave by an enclosing return, throw or arm fallthrough contribute no result to the join.

**Exhaustive arms or an explicit fallback.** A value-producing case can omit the final unguarded catch-all when the checker proves that scanning cannot reach the end without a result or an ordinary exit. Two completing arms for true and false suffice for a stable Boolean. For a closed enum, completing arms for its distinct [member identities](TypedEnums.md#names-and-case-matching) suffice, including when only some names were imported. For a non-null closed sum, one completing arm for each declared constructor suffices. Count constructor identities from the whole sum definition, including alternatives outside an explicit import list. A nullable subject also requires coverage of null unless a valid non-null fact excludes it. Enumerating known descendants alone gives no completeness proof for an open abstract type.

```cod
func fee(expedited:bool) -> u64 {
    return case expedited of {
        (true) -> { 5u64 };
        (false) -> { 0u64 };
    };
}

func receipt_id(r:const Receipt in O) -> u64 where {const owner O} {
    return case r of {
        (Accepted { val n <- id; }) -> { n };
        (Skipped) -> { 0u64 };
    };
}
```

Check paths through the arm sequence. A guard that can decline leaves its input available to later arms. A reachable `fallthrough;` also continues at the following source item. For borrowed subjects, a call or write that may change the target’s discriminator invalidates prior elimination facts on that path. For example, testing a borrowed Boolean for true, writing true between arms and then testing for false can miss both arms when the incoming value is false. Independent bits retain the original value. A receiving action or guard that throws leaves by ordinary exception control. Use established flow facts for these checks. An implementation may not depend on optimizations or executing a user guard to accept the basic Boolean and closed-sum cases.

Where completion cannot be proved, require a final unguarded wildcard arm. Its head can be `_`, `(_)` or `()`. Its normal completion supplies the common result. It may also exit explicitly, for example by throwing. A reachable fallthrough past the last arm leaves the value requirement unsatisfied. A partial void case may finish normally. Discarding a successful result leaves the expression’s static result contract intact. The compiler never supplies an implicit throwing fallback.

**Explicit throwing fallback.** As a direct, unguarded catch-all arm, `_ -> throw;`, `() -> throw;` and `(_) -> throw;` each mean `() -> throw UnexpectedCaseError;`. In ordinary construction syntax its body is `{ throw UnexpectedCaseError(); }`. If the scan reaches this arm, construct a fresh built-in unchecked error at its source site with no implicit cause. A throwing path satisfies completion without contributing a value to the result type.

```cod
func checked_id(ptr r:const Receipt in O) -> u64 where {const owner O} {
    return case r of {
        (Accepted { val n <- id; }) -> { n };
        (Skipped) -> { 0u64 };
        () -> throw;
    };
}
```

Here the explicit fallback handles null. It has the same meaning inside a lexical catch. The braced arm `() -> { throw; };` contains an ordinary bare throw statement, which requires a current catch and propagates its incoming exception under the [exception rules](ExceptionControl.md#exception-construction-throw-and-rethrow). The shorthand introduces no general rule for omitting constructors in ordinary throw statements.

**Statements between arms and fallthrough.** The case walks its contents in source order until an arm completes it or control leaves through an ordinary exit. A pattern miss, a false guard and a body executing `fallthrough;` all resume with the first item after that arm. Each intervening statement executes once on a path that reaches it. An arm that completes normally skips the rest of the case, even when the arm produces void. A fallthrough path contributes no value to the case result.

```cod
# receipt is a nullable Receipt. The procedures below record or process it.
case receipt of {
    (null) -> { note_missing(); fallthrough; };
    count_attempt();
    (Accepted { val n <- id; }) | can_ship(n) -> { ship(n); };
    log_fallback();
    () -> { manual_review(); };
};
```

| Subject and guard | Calls in order |
| --- | --- |
| null | `note_missing()`, `count_attempt()`, `log_fallback()`, `manual_review()` |
| Accepted with id 7, guard true | `count_attempt()`, `can_ship(7)`, `ship(7)` |
| Accepted with id 7, guard false | `count_attempt()`, `can_ship(7)`, `log_fallback()`, `manual_review()` |
| Skipped | `count_attempt()`, `log_fallback()`, `manual_review()` |

Each header establishes its own bindings and refinements. In the example `n` exists for the Accepted guard and body. Both intervening statements can also run with a null receipt, so neither may use `n` or assume an Accepted receipt. A statement may change outer state read by the next guard. If it throws, ordinary exception propagation applies at that statement.

Only an arm body can execute `fallthrough;`. A guard or an ordinary statement between arms cannot use it. In a single-brace body an ordinary nested `if` may choose the transfer on one path. Other paths can still complete the case. A nested case handles its own arms. A helper or independently nested full-scope callable cannot transfer to its caller’s arm. The model needs no following arm as a jump destination. A final arm may fall through to the end of a void case. A value case must prove that its scan cannot reach the end, including along an explicit fallthrough path.

A directly attached `-> {{ ... }}` arm runs its full-scope cleanup on exit. A local `return v;` or a normal value tail supplies the completed arm’s result. An explicit `fallthrough;` requests continued scanning after cleanup succeeds. For example, `() -> {{ defer audit(); fallthrough; }};` runs audit before the intervening statement. An escaping cleanup exception follows ordinary propagation. A local cleanup that explicitly returns and successfully delivers a replacement result completes this arm under the ordinary full-scope rule. A fallthrough request alone sets no callable return value. Independently nested full scopes and separately called helpers retain their own control boundaries.

**Case scope for break and continue.** On entry to the case body, the case shadows both outer control destinations. It supplies a break destination for the case expression and blocks lookup of an outer continue destination. Resolve the construct that owns the statement lexically before checking its operand. This is a contextual source restriction. A loop or supplied single-brace here body nested in an arm establishes its own break and continue destinations under the [existing here control rules](CallableStorageAndHereAccess.md#break-and-continue-in-a-single-brace-here-body). An ordinary nested `if` introduces neither.

```cod
# items() and parts(n) are generators with void feedback.
for val n <- items() {
    val score <- case n of {
        (0u64) -> { break 10u64; };
        () -> {
            for val part <- parts(n) {
                if (part == 0u64) { continue; }
                observe(part);
            }
            if (n == 1u64) { break 20u64; }
            30u64
        };
    };
    consume(score);
    continue;
}
```

For n equal to zero, `consume(10u64)` runs in the current outer iteration. For n equal to one, the nested traversal runs and `consume(20u64)` follows. Placing either `continue;` or `continue value;` directly in either arm is a source error, even when the enclosing loop accepts the corresponding feedback. The inner continue belongs to parts, and the final continue belongs to items. The same boundary applies to ordinary statements between arms. A nested case establishes its own boundary. Forwarding or inlining a here body preserves its original control destinations.

In a single-brace arm, `break v;` can finish the case from any ordinary nested statement group. Evaluate v once and join this result with the other value-producing case paths under the ordinary conditional result rules, including mode, owner and lifetime. A bare `break;` is a void completion. A value/void mismatch rejects at the chosen case, even if an outer target could accept the exit. Throwing during operand evaluation follows ordinary exception control. Run cleanup for scopes actually exited before delivering the result. The enclosing callable’s return and deferred actions keep their ordinary destinations. A break in an ordinary statement between arms likewise completes the case. Case completion itself registers no return in the enclosing callable. Fallthrough requires an active arm and resumes the scan. Double-brace arms retain their local return and the specific fallthrough permission above. There is no general transfer across an independently invoked full scope.

#### Receivers in field accessors and member methods

A field accessor is declared with `func T.@name()` and read as `x.name`. A member method is declared with `func T.name(...)` and called as `x.name(...)`. Both receive the initialized target selected by `x` as their implicit first argument, `self`. Write `const` immediately before the receiver type for readonly access. Omitting it requires writable access in either form. The declaration marker `@` identifies an accessor in this position. Reading the member uses its receiver without looking up a dynamic symbol.

| Declaration head | Use | Equivalent receiver formal |
| --- | --- | --- |
| `func const T.@label()` | `x.label` | `ref self:const T in O` |
| `func T.@label()` | `x.label` | `ref self:T in O` |
| `func const T.inspect(n:u64)` | `x.inspect(n)` | `ref self:const T in O` |
| `func T.update(n:u64)` | `x.update(n)` | `ref self:T in O` |

Here O denotes the actual owner of the receiver in the equivalent signature. The table introduces no implicit source name for that owner. Prepare the receiver once, before the written arguments, under ordinary reference and completion rules. If a later argument rebinds the source slot, the call keeps the already selected target. If preparation fails, its later effects and the member body do not execute. A readonly target admits a readonly receiver. A fixed reference slot can still supply a writable target. For overloads otherwise identical, ordinary receiver-access ranking gives the writable match to a writable target. [Reference parameters](RegionalOwnership.md#owner-qualified-reference-arguments) · [Call-operator precedent](#functor-receivers-and-callback-arguments)

Receiver access, result access and allocation permission have separate contracts. A writable accessor returning `u64` supplies independent bits. Its receiver qualifier alone supplies no setter or destination for a write to the property. A borrowed result keeps its declared mode, target access and actual owner. `Exception.cause` already selects a nullable readonly target under the receiver’s owner. In contrast, `tip` belongs to the individual catch binding. Passing the matched object to a method supplies the ordinary receiver and its owner. Access to that catch’s incoming tip requires selecting it at the catch binding. [Call results](ScalarCallResults.md) · [Exception cause](ExceptionControl.md#returning-a-locally-caught-exception) · [Catch-binding tip](ExceptionControl.md#the-catch-bindings-readonly-tip-field)

**Explicit receiver owner.** Write `func (T in O).method(...)` or `func (const T in O).method(...)` to associate the receiver with O. The same receiver head applies before `.@name()`. The ordinary `where` clause declares O and its allocation permission. In the following method, the selected exception and its nullable cause have the same actual owner. Another argument declared under P would introduce its own owner constraint.

```cod
func (const Exception in O).get_cause() -> ptr const Exception in O
    where {const owner O} {
    return self.cause
}
```

**One name per visible member.** For a given receiver type a spelling can denote a stored data field, a computed accessor family or a named method family. Reject any collision between these categories where the declarations first become visible together, including at module composition. For example, `data Policy { check:Threshold }` conflicts with `func const Policy.check(n:u64)` even when the data field itself supports a call operator. A declaration of `Policy.@check()` also conflicts with that stored field. Check this when establishing the member set even if nobody writes `policy.check`. Unrelated receiver types may use the same spelling. Overloads within one family follow ordinary signature and access rules.

A writable accessor can change its receiver. Two evaluated reads of `counter.next` invoke the accessor twice and may produce different values. The choice between that interface and a named `counter.advance()` method belongs to the program author. Both use ordinary function, evaluation and result rules. This decision covers receiver syntax and member names. Extension permissions, setter declarations and bound method extraction have their own scope.

#### Functor receivers and callback arguments

A functor is a here callable when its receiver supplies a compatible call operator to a here parameter. A direct call such as `limit(12u64)` invokes that operator on the data receiver. The selected method retains its own func/proc permissions. The here use grants bounded access to the receiver and carries no permission for its method body to read arbitrary caller locals.

A data call-operator method requires a reference to an already initialized instance on entry. A reserved target identity alone cannot serve as its receiver. In the proposed declaration spelling, the receiver is the first parameter. The expression to the left of the call parentheses supplies its reference. Its target access is an ordinary overload constraint. A readonly target admits only methods callable through readonly access. A writable target can use either access mode. When two applicable methods differ only in receiver target access, the writable match is exact for a writable receiver. Other arguments, owner constraints, callable kind and checked failures retain their ordinary rules.

```cod
data Threshold { minimum:u64; }

func (())(self:const Threshold in O, x:u64) -> bool where {const owner O} {
    return x >= self.minimum;
}
```

A method may instead declare `self:Threshold in O` when it needs writable target access. A caller with `ref limit:const Threshold in O` cannot supply that access. A caller with `const ref limit:Threshold in O` has a fixed reference slot and a writable target. The owner's allocation permission is independent of both.

For a predicate with one u64 input, `test` has **two outer parameters**. Its first parameter is itself callable.

```cod
func test(here pred(x:u64) -> bool, n:u64) -> bool {
    return pred(n);
}

# current is a reference to a Threshold with the readonly call overload above.
test(current, 12u64);                         # pred gets current; n gets 12.
test(12u64) (x) => {{ return current(x); }};     # pred gets the trailing body; n gets 12.
```

With this signature, write `12u64` for `n:u64`. A bare `12` has type i64, which has no implicit whole-type widening to u64. The first argument supplies the predicate directly.

The `x` in the signature belongs to pred. Calling `pred(n)` supplies x inside the predicate. In a parenthesized argument position, `{{ current = strict; return 12u64; }}` is an immediate [full-scope expression](ExceptionControl.md#full-scopes-as-immediate-ordinary-calls) with result type u64. Thus `test(current, {{ current = strict; return 12u64; }})` has a u64 second argument. In the trailing spelling above the braces supply the predicate and execute when test invokes it.

**Receiver preparation.** Evaluate the receiver expression once at its written argument position. Bind its selected target, owner and access to the here argument. Forwarding the argument preserves that selection. If a later argument rebinds `current` from a threshold of 10 to one of 20, the direct predicate still tests 12 against 10 and returns true. If code instead changes the original target’s minimum to 20 before invocation, the same predicate returns false. An explicit body evaluating `current(x)` reads the lexical slot when invoked. A computed receiver such as `choose_limit()` runs once during argument preparation even when the callee skips or repeats the predicate. Ordinary checks must prove the target’s owner stays live throughout its use.

**Competing data and here overloads.** Consider this overload family on its own. Both declarations can use the same Threshold defined above.

```cod
func test(rule:const Threshold in O, n:u64) -> bool where {const owner O};
func test(here pred(x:u64) -> bool, n:u32) -> bool;

test(current, 12u32);                  # Ambiguous.
test(current -> rule, 12u32);          # Data overload. n widens to u64.
test(current -> pred, 12u32);          # Here overload. n is u32.
test(12u32) (x) => { current(x) };     # The trailing body supplies pred.
```

The first declaration accepts the receiver directly as data and needs numeric widening. The second binds the receiver as a here predicate and matches the numeric argument exactly. With the same remaining matches, direct receipt as data wins over binding the receiver as a here argument. In this example each declaration has a better match on a different argument. Reject the bare call as ambiguous. Apply the [named argument rules](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding) to each signature before ranking. The distinguishing label selects a parameter that only one declaration has, so the ordinary second argument can remain positional. The trailing spelling explicitly asks for a first here parameter. Its body reads `current` when invoked. The directly supplied predicate retains the receiver selected at argument preparation.

#### Type-level call-operator lookup

The call `r(args)` selects the visible `(())` family of the nominal receiver type at the call site. Each applicable method competes only within that family. A direct functor-to-here argument uses the same lookup at its supplying site. Synchronous forwarding preserves the bound receiver and the family selected at the supplying site. Invocation follows ordinary method dispatch within that family.

```cod
# Each module publishes a data type with a call operator accepting u64.
using "limits.cod" Threshold;
using "scales.cod" Scale;

func accepts(t:const Threshold in O) -> bool where {const owner O} {
    t(12u64)
}
```

Only Threshold’s methods participate in this call. For extension permission and competing adapters of the *same* type, see [publication of type-carried call operators](ModuleWorlds.md#publication-of-type-carried-call-operators).

**Sum alternatives in method signatures.** Express the specialization on the receiver parameter. Use the visible constructor name in the ordinary parameter annotation, as in `self:const Accepted in O`. The method name remains `(())`, and `where` declares the owner as usual.

```cod
func (())(self:const Receipt in O) -> u64 where {const owner O} { 0u64 }
func (())(self:const Accepted in O) -> u64 where {const owner O} { self.id }

func read_id(r:const Receipt in O) -> u64 where {const owner O} { r() }
```

Here `self:const Accepted in O` binds a readonly reference in O to an initialized Receipt whose active alternative is Accepted. Entering that method establishes the fact needed to read `self.id`, subject to ordinary invalidation if the alternative changes. Both methods belong to Receipt’s call-operator family. Matching uses the alternative’s identity irrespective of which initializer built the receipt. This use in a parameter needs no new nominal subtype or predicate evaluation in overload lookup.

The receiver is an ordinary dispatch argument. For an Accepted receipt, both signatures apply and the Accepted method is more specific. For a Skipped receipt, only the Receipt method applies. Thus `read_id(r)` returns 42 for an Accepted with id 42 and zero for a Skipped, including when the alternative is known only at runtime. A caller-side `isa` test is unnecessary. The [general dispatch and coverage rules](AheadOfTimeExecutionPreparation.md#actual-type-dispatch-and-wpo-coverage) allow direct selection when the compiler proves the alternative and require a prepared path for every admitted case.

#### One callback parameter for different owner counts

The same here signature accepts bodies using zero, one or several separately proved owners. Each body uses its own declaration sites and particular supplying activation. The receiving function can invoke it or forward it synchronously. A later use through a released target rejects even if its former slot now holds another live owner. See [the callable contract](CallableStorageAndHereAccess.md#what-an-invocation-needs) for recursion, relocation and a shared invocation interface.

#### Standalone callable declarations

[D-10](SymbolicDynamicScope.md#callable-declarations-and-methods) owns callable identities, signature declarations and methods. H-258 requires explicit parameter/result signatures on a bodyless declaration. [D-24](CallableResultInference.md#establishing-a-result-contract) applies H-423: an independently established declaration can supply a definition's omitted result contract. The same rules govern func/proc and named/anonymous definitions where applicable.

A declaration supplies requirements, not an executable dispatch candidate. Ordinary body-bearing methods compete under the existing applicability/specificity rules. An applicable inherited or generic body may satisfy a more-specific declaration if it meets that declaration's result, kind, access and owner obligations. A declaration does not implicitly introduce a negative or abstract override that blocks an otherwise applicable body.

A demanded call requires an applicable implementation satisfying the applicable declarations in its selected module world. A result or callable-kind mismatch is an error even if a particular caller discards the result or could otherwise accept it. Multiple declarations remain compatible obligations; declaration order supplies no tie-breaker, and a local annotation cannot waive a conflicting contract. Ordinary overload selection still does not dispatch on result types.

Declaration visibility, import/extension permission, explicit nominal conformance and selected module contexts remain governed by D-10/D-15. A declaration performs no capture or execution and does not manufacture a body, publish an unselected adapter or select general result covariance/effect subtyping. Current compiler support is recorded in the [source capability reference](../tests/cod/language/README.md).

#### Type-associated method visibility and extension

Using/import of a type makes its public source-visible behavior available. Under H-484, explicit source selection registers the source's published namespace for lower-priority fallback; symbol from uses the same model. A demanded public name can resolve there even if unrelated to the selected type. No recursive field/signature traversal is needed to introduce additional type names. H-231 excludes private resources, including private imported names; H-487/H-488 makes an intermediate private own binding a barrier unless the module explicitly publishes that alias, either for its local object or for a selected upstream object. A public re-export can coexist with the private local namesake. H-233 permits explicit re-exports in the published view; H-374 preserves whole symbol-family publication. Method identity, body context, lookup visibility and permission to add methods remain distinct.

| Type selection | Available operations | Extension permission |
| --- | --- | --- |
| using a type | Find public operations through source-preserving fallback. | No extension permission from this selection. |
| import a type | Find public operations through fallback with the selected import context. | Retain type-import permissions; a local method definition shadows the implicit name. Select the callable explicitly, by name or with methods, to choose a family for extension. |

H-489/H-490 additionally admits the optional with methods suffix on using/import/symbol-from selections. Signature association identifies related callable objects; the clause explicitly selects every such callable's complete public family. All using/import callable selections use that same unit, never a subset of methods. Explicit mode follows the reuse form: using/symbol-from preserves context without extension permission; import supplies its existing receiving context and extension permission. Existing collision rules, private barriers and identity merging apply. A type/symbol alias does not rename its associated callable aliases. [D-15](ModuleWorlds.md#explicit-associated-callables-with-with-methods) records examples and acceptance; this is not current compiler coverage.

H-212/H-232 separates three operations. Using preserves the selected definition and implementation context. Import permits extension and adapts an imported callable to the receiving implementation context; H-230 gives this extension context a distinct callable identity. It does not clone a type, rebind non-callable lexical resources or recapture effects. Mixin redefines a source body in the receiving lexical/dynamic context, with the existing compile-time symbol-map semantics. Import specialization is distinct from that new body-definition event. Categories beyond the existing procedure-body mixin mechanism remain open.

H-225 defines signature membership of the source-visible associated set; H-231 supplies its public-selection boundary: include parameter and result occurrences of the selected type, including supported reference wrappers, and inherited applicable methods. Result-only factories are included. Membership does not scan bodies or force unrelated layouts or implementations. A repeated original method is one candidate; distinct equally specific implementations remain ambiguous. Association does not introduce mutable wrapper covariance or typed-region syntax absent from the prototype.

H-483 makes a local declaration shadow implicitly brought method names after either using or import of a type. Mere spelling equality does not join the imported family. An explicit callable selection takes priority over the implicit tier. Explicit callable import-as chooses an extension target; unused implicit conflicts do not prevent type selection. Name resolution precedes argument dispatch: a failed explicit overload does not revive a shadowed implicit operation. Import permission belongs to the selected family, not an unrequested requirement to import every argument type.

H-227 selects caller-selected ordinary implementation views rather than program-wide publication. The native implementation separates semantic-family provenance from selected callable identity. H-230 requires distinct extension-module identities: differently aliased selections retain separate method sets, while a same-alias virtual union has a fresh identity in the selecting module. H-232 connects this contract to H-227: using preserves the selected implementation context, while import adapts a helper's callable dependencies to the receiving module's implementations. In either case unique-most-specific dispatch applies to the selected method set; equally specific distinct bodies are ambiguous. Neither method selection nor aliasing publishes unrelated conformance, changes a procedure capture, or adds a runtime dictionary or module namespace.

Compiler operation/intrinsic overloads retain lexical views in this increment, preserving source-helper native fallback. UoM's H-134 one-shot bundle selection and prohibition on copying/rebinding unit definitions remain unchanged. Source-preserving using/import, lexical/symbolic procedure mixin and once-only module incorporation retain their contracts; D-15 governs the emitted initialization order. Cross-alias extension grants and standalone abstract-signature syntax remain outside this increment.

Julia permits unqualified extension after import, and also permits extension through qualified function names. CoD adopts the using/import extension distinction but H-214 excludes module-qualified callable names: there is no corresponding qualified-name bypass. Automatic type-associated method selection is a separate CoD contract. [Julia modules](https://docs.julialang.org/en/v1/manual/modules/#using-and-import-with-specific-identifiers,-and-adding-methods)

H-214 keeps modules out of the receiving local namespace. A source path belongs in a selection declaration. The local name or a name/identity alias gives access to the selected definition. This retains package/version identity without introducing module-valued namespace objects. A diagnostic about a name selected under a name/identity alias shows the local spelling together with its canonical fully qualified name. IDE navigation can use the same identity. Object fields retain their usual dot syntax. Diagnostic qualification is display notation and grants no `Module.f` or `Module.@p` expression form. [Names in diagnostics](../applications/cod-booter/README.md#names-in-diagnostics)

H-215 proposes a separate explicit `import "source" method as local_alias;` declaration for local method renaming. It retains the source body and grants import's extension permission; H-232 also supplies receiving implementation-context adaptation. It does not perform mixin redefinition. A selector containing only a callable name naturally selects its source-visible overload family; H-490 excludes selecting a particular method signature as a using/import unit. Compile-time aliases and explicit TLS installation are separate: installing a compatible source-family view may deliberately target another identity without changing either method world. The prototype’s implicit symbolic-selection installation path requires migration under H-372/H-373.

H-484 implements H-217/H-483's lower-priority names through a source-namespace fallback relationship instead of requiring copied implicit alias maps. This relationship is compiler lookup state, not a module value or runtime namespace. Explicit/local names resolve first, and fallback uses only published source views:

| Binding source | Name lookup behavior |
| --- | --- |
| Explicit using/import callable alias | Shadows implicit names at that spelling. |
| Local function/procedure declaration | Shadows a same-named implicit family after using/import of a type; explicit callable-import extension remains separate. |
| Methods brought implicitly with a selected type | Supply names when no explicit binding shadows them. |
| Types/methods carried implicitly by symbol from | Use the same lower-priority layer; a shadow never changes the selected symbol's declared type identity. |
| Repeated paths to one original method | Preserve identity; do not create duplicate method implementations. |
| Different implicit operations with one spelling | Prefer a clearly nearer source; peer or unorderable candidates require explicit selection when demanded. |

For example:

```text
using "arena.cod" ArenaRoot;
using "pool.cod" PoolRoot;
import "arena.cod" reserve as arena_reserve;
import "pool.cod" reserve as pool_reserve;
```

The explicit names select their respective source operations; an implicit `reserve` spelling need not be deleted. An explicit `import "pool.cod" reserve as reserve;` can instead shadow the implicit spelling directly. A single renaming preserves the selected source callable identity, method signatures and captures. H-230 separately gives an extension its module identity and gives same-alias composition a receiving-module union identity. The union rule concerns versions of the same semantic callable; how unrelated same-spelled families are combined is not inferred. Import permission still governs adding methods. H-232 governs the helper dependency context as discussed below.

Name lookup precedes argument-type dispatch. The rule resolves one binding, then applies unique-most-specific dispatch within its visible methods. Failure to find a matching overload for an explicit binding does not revive a shadowed implicit operation. This prevents argument types from silently changing which module's operation a name denotes. It also preserves function/procedure restrictions, const/ownership permissions and argument evaluation order; aliases do not relax them. This lookup/dispatch boundary implements the H-217/H-483 priority distinction; execution evidence is recorded separately.

Implicit-name ambiguity is diagnosed at demanded lookup; explicit callable extension must independently establish its selected identity. Explicit bindings may shadow unused conflicts. Scan-before-incorporation and explicit alias availability remain unchanged; these rules do not hoist module initialization or grant cross-alias extension permission.

H-257 requires calls to resolve the nearest lexical value binding: a callable parameter or local supplies the callee, while a non-callable shadow makes the call ill-typed. [Lexical calls and shadowing](SymbolicDynamicScope.md#lexical-calls-and-shadowing) records the rule and shadowing diagnostics. This explicit decision is separate from the priority of implicitly selected type methods.

#### Method-selection examples and evidence

Callable identity, extension permission, selected implementations and body lexical context are separate. An imported extension has its receiving-module callable identity; a local declaration after using/import of a type shadows the implicit name with its local family. Combining equally specific distinct implementations remains ambiguous under H-209.

The [method-view fixtures](../tests/cod/language/method-views.cod) cover selected contexts, explicit aliases, result-only associations and repeated identities, but do not establish H-483/H-484's complete type/symbol selection and source-fallback behavior. [D-15](ModuleWorlds.md#type-selection-and-shadowable-associated-methods) owns current acceptance, conservative proximity and visibility boundaries. Explicit-family adaptation and unique-best dispatch retain their requirements; a local shadow has an independent identity and does not replace a helper dependency by spelling. H-485 reserves later information-level diagnostics at mentions resolved implicitly, suggesting an equivalent explicit selection. Current compiler availability remains separate.

Callable-dependency analysis follows expression meaning: the selector in `record.field` is a compile-time field label even when a lexical function has the same spelling. It cannot make an otherwise source-bound method depend on the importing module. Calls in the object or stored-value expressions remain ordinary dependencies.

Associated methods use declared signatures; body-inferred result types remain under separate review. Interface-only algorithms must select implementations from the permitted receiving context and check unavailable bodies when demanded.

#### Concrete-data dispatch through abstract parameters

H-229 requires dispatch to use the actual concrete data type. A parameter or result annotated Entity checks that interface requirement; it does not turn a Node into Entity for later method selection or erase a previously established Node <: Weighted relation. Source and caller-selected implementations still need a unique most-specific match for the concrete argument types. Abstract inheritance and explicit adapter selection determine applicability; field shape and mere module loading establish no conformance.

For Node <: Weighted <: Entity, the confirmed dispatch target is score(Weighted)=2 through direct, broad-parameter, nested and Weighted-helper calls. An exact score(Node)=7 remains more specific on all paths. Selecting the original Node without its adapter must still fail an Entity requirement.

Concrete identity, region provenance and established conformance must survive argument/result forwarding and borrowed traversal. The compiler may retain the needed evidence as compile-time metadata; it must not add runtime interface wrappers or new ownership. Branch joins and loop-carried values cannot manufacture an interface fact that lacks justification on a reachable path. These checks preserve explicit selection rather than publishing unrelated adapter relations to every source body.

Concrete dispatch chooses among visible candidates; the declared interface is not a ceiling on the actual concrete type. Direct, broad, nested, returned, field and nullable paths must preserve established concrete-type relations. A helper can use those relations for another instance of that concrete type without making unrelated caller relations ambient. Alias candidate visibility remains a separate review scope.

Same-owner branches retain the common interface requirements and selected concrete target; returned views still alias original storage. Reject unsupported narrowing, cross-owner branch results and escaping local views. Returning from a factory does not publish its adapter conformance onto unrelated fresh caller values. Source-preserved and import-adapted implementations retain their distinct contexts.

Symbolic reads retain the selected binding value’s established conformance facts. Perform uses the captured reference and its facts; behave uses the current reference and its facts. Rebinding does not accumulate previous interface guarantees on the symbol itself. This requires no runtime interface tag, new reference permission or owner-lifetime extension.

#### Extension-module callable identities and local unions

H-230 assigns distinct identities and method sets to base/ext1/ext2 versions of the same semantic callable. A using module preserves those identities when local aliases differ. When it selects those versions under the same alias, it forms a virtual method union with a fresh identity in that receiving module. This is compile-time composition, not mutation of the source modules or program-wide publication.

```text
using "base.cod" score as base;
using "ext.cod" score as extended;
function indirect(x:Sample) -> i64 { base(x) }
# base defines score(Metric)=1; ext adds score(Sample)=7.
base(x);      # required: 1
extended(x);  # required: 7
indirect(x);  # required: 1
```

Concrete dispatch uses Sample on all three calls. Candidate sets differ; entering a wrapper cannot add extended’s candidates to the distinct base identity. Acceptance requires the results 1, 7, 1.

```text
using "base.cod" score;
using "ext.cod" score;
# score names the consumer's new union identity.
```

The required composition must preserve original method definitions and source provenance. A shared inherited method reached twice is one candidate; two distinct equally specific implementations remain ambiguous under H-209. Source-defined unit constructors retain their lexical type identity and do not by themselves make an imported method context-dependent. Calls inside constructor arguments still participate in implementation selection. Declaration order cannot select a winner. A union through using does not grant import's extension permission. These constraints do not require a runtime registry, new object layout or module-qualified calls. Keeping provenance of the original semantic family separately from the selected identity is a compiler representation candidate, not a new user-visible symbol mechanism. Whether unrelated same-spelled families may be explicitly fused, and how a local method extends a union selected with mixed permissions, remain outside this confirmed scope.

**Helper dependency context follows using/import.** H-232 confirms the following schematic source, with standalone export-list syntax proposed below:

```text
# base.cod
function score(x:Metric) -> i64 { 1 }
function twice(x:Metric) -> i64 { score(x) + score(x) }
export score, twice;

# ext.cod
import "base.cod" score;
function score(x:Sample) -> i64 { 7 }
using "base.cod" twice;  # replace with import to select ext's implementation context
export score, twice;

# client.cod
using "ext.cod" twice;
twice(sample);
```

With `using "base.cod" twice;`, the client call returns2. Replacing that line with `import "base.cod" twice;` makes it return14. The consumer's using preserves whichever context ext exports. Import therefore has an observable adaptation role even when ext adds no overload to twice itself. It cannot be implemented as only an extension-permission bit on an otherwise unchanged callable alias.

| Reuse form | Callable implementation context | Body and other lexical resources |
| --- | --- | --- |
| using | Preserve the selected source context. A further using/re-export does not substitute consumer implementations. | Source-defined body and its non-callable lexical bindings remain. |
| import | Use the receiving module's selected implementations for the imported helper's callable dependencies; retain an extension-module callable identity. | Source-defined body and non-callable lexical bindings remain; this is not general body redefinition. |
| mixin | Redefine the body in the receiving lexical/dynamic context under its existing contract. | Receiving definition rules, explicit symbol-role mappings and new captures apply. |

This refines H-212's source-preserving import contract for callable implementation context, without changing type or symbol identity, private-resource selection permissions, source globals or captured effect values. Import is not an instruction to recapture a procedure, nor to replace every source name by an equal local spelling. The separately aliased base callable and a local wrapper using that alias must still return1; import adaptation cannot leak into unrelated calls. Definitions may share source bodies and machine code where equivalent, but compiler provenance must distinguish their selected contexts.

For the reusable pool algorithm, an integration module imports the source-defined total and selects the adapter's lookup(Pool), then explicitly exports the integrated total. A consumer can use that exported helper without reconstructing the adapter context or duplicating the algorithm. Native acceptance must show the2/14 distinction, preservation through a further using/re-export, restoration between integration paths, deep demanded helper calls and the1,7,1 alias trace. Merely changing an expected output while retaining ambient caller-method accumulation is insufficient.

H-253 requires conflicting inherited and receiver-selected overloads to reject during import adaptation. If A's helper uses score(Sample)=7 and the receiver selects B's score(Sample)=11, both definitions participate in the demanded dependency and neither silently wins. Repeated aliases of the same identity contribute once; repeated views of one declaration are distinguished from independent definitions. Ordinary unique-best typed dispatch detects conflicts and reports their locations. Direct alias selection and using-preserved helper contexts retain their own identity boundaries. Explicit conflict-arbitration syntax remains future design. The native compiler carries interpretation separately from source body resolution; import does not recapture procedures or become mixin.

#### Whole-callable selections and shared aliases

H-490 requires every using/import selection to select a whole callable; type association and with methods do not produce a method subset. Different selected module views may contribute their available family information, but aliases do not own filtered copies. When their identity is equal, the receiver merges the information into one local record. Both aliases and an adapted helper consult that record. Different identities remain distinct; spelling alone cannot merge their semantic records. Repeated import of one source callable into the same receiving module selects one receiver identity even when aliases differ. Locally declared overloads under an explicitly imported alias join that identity and are visible through its other aliases. This does not grant extension permission through using or merge distinct extension-module identities. H-252 requires a warning when one module declares overloads through multiple aliases of that shared identity, with opt-in promotion to an error. The prototype uses `-Wmulti-alias-extension`, `-Werror=multi-alias-extension` and the global `--warnings-as-errors` option; selecting aliases without multi-alias declarations remains warning-free. [D-15](ModuleWorlds.md#identity-records-aliases-and-publication) describes the identity-based check and preservation of helper contexts through successive imports.

```text
using "base.cod" First;
using "base.cod" score as zipped;
export import "base.cod" relay;
```

If the published score contains score(First) and score(Second), resolving it through fallback or explicitly as zipped accesses that family; the receiver's aliases and adapted relay share the corresponding information. Information from admitted source views still merges by identity. The source module's own information is unchanged. [world-composition.cod](../tests/cod/language/world-composition.cod) checks both concrete types through both aliases and an adapted helper. [world-source-isolation.cod](../tests/cod/language/world-source-isolation.cod) rejects a source helper's attempt to use an unselected method even after the receiver has merged it.

Compound adapted contexts and broader extension permission remain separate review boundaries.

#### Explicit exports and private module resources

H-231 requires `export`: every resource without an export declaration is private and cannot be selected by another module through using, import or mixin. This publication boundary applies before local aliasing or method-set composition. Import grants extension permission only after successful public selection; it cannot bypass privacy. H-233 requires an explicit re-export: an imported alias remains private in its receiving module until that module exports it. Export does not by itself grant consumers extension permission or change when module execution is incorporated.

The native standalone form exports one existing local name:

```text
export Sample;
export score;
export @print;
export (+);
```

H-233 also confirms export as a modifier before import:

```text
export import "base.cod" twice;
export import "adapter.cod" Sample;
```

Each line performs the normal import and explicitly exports all resources that this import brings into the receiving module. This includes methods implicitly selected with an imported type, subject to the source export boundary; it does not expose unrelated receiving-module declarations or private source resources. The first line exports the receiving-context helper selected by H-232. A downstream using preserves that exported context. Plain import has the same adaptation/extension behavior but no publication. The modifier does not perform an extra dependency encounter, create a runtime export action, grant consumers import permission, or alter once-only source-module incorporation. An exported source name is not automatically re-exported by modules that select it. H-484/H-487 nevertheless permits downstream implicit lookup to follow source fallback relationships to deeper public names when no intermediate private binding without a matching public export blocks the path. Explicitly selecting the name from an intermediate module still requires its export; implicit reachability and that module's declared public aliases are distinct.

H-488/H-491 adds `export from "source.cod" name [as public_name];` as publication-only source selection. It preserves the upstream object's identity/context and creates a public alias without creating an own local alias or extension target. A private local namesake remains private and keeps its local meaning; downstream lookup uses the explicit public identity. Ordinary export name still publishes the locally resolved object; export import still imports and publishes. The selected upstream resource must be public, and conflicting public alias identities reject. Source dependency/initialization rules remain; no TLS installation or capture is implied. [D-15](ModuleWorlds.md#publication-only-re-export) owns acceptance. Private resources, unexported aliases and ordinary imports are not automatically public.

A symbolic procedure selected by export import is also explicitly public under its receiving alias. Its source captures remain intact; publishing it neither re-executes the source module nor recaptures the caller’s later bindings.

Export import is the confirmed shorthand; analogous modifiers on using or mixin are not selected by this statement. Standalone re-export uses the same single-name export form. Grouped export lists remain an open surface-design detail.

Here Sample, score and @print name existing local bindings, including selected aliases. Exporting score exposes its callable set; exporting Sample alone does not expose private functions whose signatures mention Sample. Type-associated selection must filter by the source's export boundary, preserving public result-only factories and inherited applicable methods. A public source-defined callable can still use private helpers internally without making those helpers directly selectable. UoM bundles likewise select exported aliases of their existing conversion entries; a private original spelling stays unavailable, and aliases of one formula/context do not multiply conversion paths. Ordinary callable import alone does not register a new conversion formula; D-11 keeps that broader adaptation/registration boundary explicit. An exported mixin body remains subject to its receiving-context name rules; export supplies no private-name bypass.

Privacy, explicit re-export and export import are confirmed at the recorded named-resource scope. H-247 defines publication as public alias → identity. H-248 requires identity-centered compiler records with multiple aliases. Consumers resolve the published identity directly; the exporter's current local spelling is not another lookup step. `export import` publishes the actual imported identities, including associated methods, even when a different local binding has that spelling. [world-export-alias.cod](../tests/cod/language/world-export-alias.cod) checks both a competing local method and a frozen local helper; the imported identities remain public. A conflicting publication under the same public alias rejects. Grouped lists, broader non-callable bundles and cyclic re-exports remain draft details; H-490 excludes per-method using/import selection. H-491's export-from surface is confirmed but not implemented. A type appearing in an exported signature is not automatically directly selectable.

Acceptance needs a public function using a private helper, rejected direct/private selections through all three reuse forms, alias re-export, private associated methods staying hidden, and public factories remaining available. Together with the identity increment it must check separate aliases yielding1,7,1, same-alias union deduplication and ambiguity, plus using-preserved2/import-adapted14 and export import publication without implicit re-export. Existing initialization, captures, conformance, concrete dispatch and owner checks remain relevant. Acceptance does not qualify unresolved compound contexts or the complete language contract.

Lean acceptance should exercise explicit conformance without eager body completeness, rejection of undeclared conformance despite matching signatures, unrelated declared abstract constraints remaining ambiguous, and unused missing implementations remaining allowed while demanded missing implementations fail. An adapter's declared conformance must be usable through using without granting declaration permission; an unrelated same-identity view or mere WPO inclusion must not silently publish it. Different field layouts must be irrelevant to interface matching. Module cases need distributed parent/child methods, source-visible transitive methods, duplicate-path identity handling, using-only extension rejection and permitted import extension. Alias cases must show explicit shadowing of implicit conflicts, unchanged operation identity/source captures, and no fallback to a hidden operation after explicit dispatch fails. The native abstract checkpoint covers the explicit-conformance/applicability portion. The linked increment evidence covers the native association, name-priority and caller-view subset; unimplemented qualifier and owner contracts remain separate; K-400 covers the bounded typed-signature increment.

<a id="dl-02"></a>
### DL-02 — Methods, effects and entry execution

H-492 admits declaration-only module cycles after identities and independent signatures are registered; callable bodies may refer across the cycle. H-495–H-498 requires static definitions in runtime module namespaces before executable initialization. The compiler orders each dependency prefix as one implicit postpon block with all explicit ordered postpon suffixes, retaining each module's context; the entry's ordinary executable prefix runs last, after its own postponed blocks. Emit a fixed native order, rejecting unproved/cyclic readiness without runtime scheduling or block splitting. [D-15](ModuleWorlds.md#module-dependency-cycles-and-initialization) owns ordering, H-379 installation interactions and remaining proof/representation work.

Package means the dependency/version unit; namespace means an internal grouping of modules, and a namespace segment names one level. The CLI selects the namespace containing the reserved local entry module `__main__`, with several selectable entry namespaces permitted per package (H-499/H-500). Other modules cannot select/re-export an entry. Namespace ancestry grants no imports, publication, module ownership or initialization, and unselected sibling entries do not execute. [D-15](ModuleWorlds.md#packages-namespaces-and-entry-selection) owns entry acceptance and the still-unselected CLI grammar/path mapping; existing file-based CLI support is not the new contract.

**Confirmed local direction:** functions are lexical, may operate on explicitly supplied mutable memory, and cannot access dynamic bindings or invoke procedures (H-197/H-198). Procedures may have ordinary lexical names or belong to symbol-identified module families and may access effects in either case. Bare symbolic definitions register/extend methods; explicit effects install compatible views (H-372). Actual argument types select family methods, with admitted combinations compiled ahead of execution (H-375/H-376). `perform` installs captured values temporarily; `behave` uses current dynamic bindings. Module top-level scopes survive initialization until process exit under H-166. H-180 preserves both captured and shadowed regional owners. [D-10](SymbolicDynamicScope.md) specifies callable and selected implementation contexts. The bounded source frontend implements typed dispatch; packet entry-method application and the broader ownership grammar remain separate design work.

**Question:** What is a method's meaning, and how does the receiver select and execute the entry object's designated method?

- Specify receiver binding, arguments/results, dispatch, method identity/version and access to the execution context. Decide whether definitions travel with a packet, are imported, or are resolved from already loaded definitions.
- Distinguish explicit-argument operations from ambient dynamic-context access. A database write or external operation is not excluded from a function merely because it changes external state; its required inputs and allowed intrinsic contract must be explicit. Define ordering, errors and capability binding; suspension, persistent continuations and cancellation remain separate choices.
- Keep business decisions expressible in the typed program representation. Identify which primitives are supplied by native handlers and how two environments can interpret the same business program.

**Proposal to discuss:** an illustrative `entry.apply(context) -> Result<Response, ApplyError>` contract, with methods resolved against explicit definitions and bindings. Serialized native function/member-function addresses would not provide a relocatable cross-process method identity.

**Decision evidence:** trace a method invocation, including dispatch resolution, a branch based on an effect result, a missing capability and an application error. Record the chosen semantics rather than merely the spelling of `apply`.

<a id="dl-03"></a>
### DL-03 — Regions, ownership and lifetimes

H-262 permits a named non-optional owning region to supply an effect installation; after transfer its name is unavailable at compile time, with no implicit null value. H-182’s writable optional source is a separate facility for explicit optionality. H-183 still requires the remaining ownership language to be coherent. The [regional ownership draft](RegionalOwnership.md) coordinates construction, expression categories, writable-place syntax, optionality, borrow validity and cleanup. H-263 preserves pre-existing views while the new local owner remains alive and permits access; ending that scope or sending the region out revokes access. H-264 selects the first typed let/make/root-initializer form. H-267 qualifies D-17 E1–E5 for bounded typed construction, borrowing/owning calls and exception control; H-269 retains explicit causes beyond catch. K-414 records native acceptance. Extended qualifiers, reassignment and transport remain separate.

**Confirmed local direction:** ordinary user storage uses lexical scopes and regions; H-277 additionally places symbolic bits payloads directly in TLS. The native heap is reserved for system code. H-180 reaffirms capture of permitted dynamic values; resource capture/replay does not copy the graph or consume ownership. H-277 admits region-symbol-backed interior references with lifetime checks; H-280/H-281 distinguishes their scoped rebinding from bits transfer. H-177 allows ordinary owning handles. Each region holds one direct-pointer graph, and all of its pointer edges stay inside that region. H-99/H-100/H-156/H-157. [Capture and lifetime discussion](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime)

**Regional access:** functions operate on explicitly supplied regional data and may modify it; they cannot read/write dynamic bindings or invoke procedures. Procedures may call functions. H-197/H-198 establishes this mutable-state baseline; a separate const function may serve future read-only requirements. Regional references preserve their enclosing region association and lifetime across nested calls and returned views. See [D-10 storage semantics](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime).

**Ownership boundary:** H-190 makes whole-region ownership intrinsic. Intra-region allocation/reuse policies belong to CoD user code, including free lists and pools. Typed slot reuse neither releases part of the region nor guarantees that an old reference still identifies the same logical item. D-14 prioritizes the missing ordinary holder, construction/result, optionality and reference-validity contract before chan transfer.

Lifecycle acceptance requires each selected short-circuit scope to release its local region before the next iteration, skipped operands to allocate nothing, and captured/hidden enclosing owners to remain live. These requirements do not decide pending owning construction, transfer or chan choices.

**Question:** Which region owns every value and reference throughout packet application, querying and persistence?

- **Confirmed transfer mechanism:** cross-region object-graph transfer, including packet-to-DBMR and DBMR-to-DBMR reconstruction, uses `<-` with third-party library overloads for the graph data structures concerned (H-351). [The transfer contract](RegionalOwnership.md#bits-field-transfer-and-user-defined-graph-conversion) supplies owner/access checks and rejects default transfer of pointer-bearing objects without a matching method. Library algorithms define topology handling and failure behavior; this does not select a universal compiler-generated graph copier.
- Distinguish packet construction/receive storage, persistent DBMR storage, query scratch and response storage. Define allocation, borrowing, mutation and region destruction rules.
- The selected library defines which graph is transferred into DBMR and how its data is updated; the application's persistence protocol determines when those changes are durable. Independently retained regions participate through business identity/lifetime protocols; a short-lived packet pointer cannot become a stored cross-owner edge.
- Require each selected graph-transfer library to state its sharing/cycle and runtime-resource semantics. Cross-region reconstruction neither copies/moves a regional C++ object nor transfers whole-region ownership merely by using the same operator spelling.
- Specify when bindings or borrows pin a region and what happens on remapping, unloading and generation changes. Decide which guarantees the checker proves and which a runtime boundary verifies.

**Application/library work:** select the packet/database graph library and document its update, failure and temporary-storage contracts for each workflow. The language does not select one update/history policy for all libraries. Actor support and snapshot persistence retain the separate design boundaries in DL-05/DL-09.

**Decision evidence:** a lifetime diagram for one request, including a rejected escaping reference, a result surviving packet disposal and a query whose temporary allocations can be released.

<a id="dl-04"></a>
### DL-04 — Packet lifecycle, framing and compatibility

**Question:** What exact bytes form a complete, interpretable packet, and when may its entry method run?

**Initial compatibility decision (H-352):** use a manually assigned root-type compatibility identifier, such as a UUID. It records the producer/consumer's declared compatibility agreement; it is distinct from the compiler's type identity and does not itself derive compatibility from layouts. The identifier's exact declaration, encoding and location remain protocol details.

**Later direction:** consider compatibility checking across the layouts of all ADTs involved, with a checksum for fast matching. The layout description, dependency coverage, comparison rules and checksum choice remain to be designed. This direction neither introduces general RTTI nor establishes cross-architecture portability by itself.

- Define total-length meaning, header encoding, occupied bytes versus reserved capacity, root location/type and dependency identification. Decide what is carried in the packet versus pinned in the connection context.
- On the inter-Actor AKCP/UDP path, partial bytes stay private until the complete region passes admission. A failed transfer publishes no root. Define region framing across bounded records, recovery of the next boundary, structural validation and disposal of partial storage. [Whole-region transfer](ActorChannels.md#regional-transport-over-akcp)
- Specify the layout/ABI agreement represented by the initial manual identifier, including byte order, field widths and alignment. Semantic/schema versions remain distinct from layout and ABI compatibility; builds on multiple architectures do not establish image compatibility.
- Decide mutation rights during construction, after sending and after reception. Whole-region relocation preserves valid relative layouts; arbitrary truncation or object movement does not follow from that property.

**Proposal to discuss:** construction → sealing → complete reception → validation/binding → application. The total binary length is confirmed; sealing and this exact state model remain proposals.

**Decision evidence:** annotated packet layout, split/combined input examples, an incompatible root/dependency example and a declared byte-order, width and alignment policy for the chosen peers.

<a id="dl-05"></a>
### DL-05 — Application, commit and retry

**Question:** What observable event means that a packet has “taken effect”?

**Assigned design responsibility (H-352):** [Actor-model design](GlobalVariablesDoneRight.md#actor-implementation-readiness) must design the language-level support applications use for outcomes, failure and recovery. The packet format and graph-transfer operator do not impose a database transaction protocol.

- Actor design must state sending/receiving, handler completion and failure boundaries. D-10-ACTOR-CHAN refines H-168: sending is fire-and-forget except for reported systemic communication faults likely to compromise subsequent sends. It supplies no reception guarantee, receipt or application-completion acknowledgement.
- Applications use the Actor facilities to define any response, retry, duplicate-handling or compensation protocol they need. Distinguish fully received, applied in memory and durably committed outcomes in that protocol.
- Snapshot/file operations supply the persistence boundary in DL-09; their failure behavior must be explicit before applications can rely on it. D-10-ACTOR-CHAN fixes one dedicated OS thread per Actor. Priority/fairness policy, supervision, exactly-once application protocols and cross-region transactions retain their separate scope.

**Decision evidence:** a small outcome table for failure before application, during application, after commit but before acknowledgment, and during retry. Select only the guarantees needed for the first business workflow.

<a id="dl-06"></a>
### DL-06 — Updates, visibility and history

**Confirmed responsibility (H-352):** importing different libraries selects different `<-` overload logic for data processing. DBMR does not impose a universal overwrite, immutable-version or tombstone policy. Method selection follows the existing module-world and dispatch rules; an import does not mutate another module's methods.

Each library/application contract must state:

- Insertion/update/deletion behavior, visible records and statistics, retained history and any tombstone scheme.
- The relation between logical updates, allocation/reuse, event history and audit records. Allocation policy alone does not require recording reads or retaining every prior value.
- Writer/reader coordination, snapshot boundaries and index visibility using the selected Actor/persistence facilities.
- Which facts are authoritative, which derived structures can be rebuilt and which changes invalidate cached results or specializations.

These are contracts of the selected libraries and application, rather than one unresolved language-wide choice between overwrite and append.

**Decision evidence:** show the visible records, statistics and retained bytes after create → update → delete, including the same sequence across close/reopen.

<a id="dl-07"></a>
### DL-07 — Retrieval, statistics and indexes

**Question:** How does business code express queries, and what meaning, ownership and cost do their results have?

- Express lookup, traversal, filtering and aggregate operations through user-program libraries under H-352's extension direction. Dedicated query syntax is not selected; library contracts define their available operations and costs.
- Define result types, ordering where observable, visibility and errors. Decide whether results are borrowed views, iterators pinned to a snapshot, or independently owned response graphs.
- Specify index semantics, maintenance and rebuildability. Record when an index is merely an acceleration structure versus part of a required business constraint.
- Keep temporary query allocations distinct from permanent database growth; choose representative measurements before setting performance targets.

**Decision evidence:** lookup plus a filtered aggregate before/after updates and compaction, with explicit result lifetimes and a correct path when a rebuildable index is absent.

<a id="dl-08"></a>
### DL-08 — Identity, references and versions

**Question:** Which identities survive transport, independent region loading, compaction and definition changes?

DL-04's root-type compatibility identifier identifies a representation agreement, not a business entity or its location. Libraries/applications still define the logical keys used to find transferred or rebuilt data.

- Distinguish domain entity, graph node, type/method definition, region, physical location, execution and effect-occurrence identities. Decide which require persistent identifiers.
- Define efficient intra-region references and business protocols for relationships between independently managed regions. The protocol determines identity and version selection, missing targets and any access lifetimes.
- Decide how packets refer to existing entities when compaction moves them. Compare stable logical IDs and version-qualified business keys.
- Specify schema evolution and migration boundaries, plus how independent domain identities and mappings remain explicit. Type identity, schema revision and layout compatibility are separate questions.

**Proposal to discuss:** stable logical identity at externally retained boundaries, with compact relative references inside pinned regional layouts. The particular identifier, lookup structure and generation mechanism are open.

**Decision evidence:** independently reload two related regions, then compact one and resolve an earlier packet's target under the chosen version policy.

<a id="dl-09"></a>
### DL-09 — Compaction and backing-file publication

**Question:** Which retained semantic graph is rebuilt, and how does the new backing file become authoritative?

**Language support direction (H-352):** plan an operation that saves a region snapshot directly to a disk file, including replacement of an existing file. Applications decide and implement the business workflow around it. This is a required design work item, not an implemented API or a selected crash/durability guarantee.

- Applications and their libraries select roots, reachability and retention rules for current facts, history, tombstones, dependencies and live execution state. Retaining all history limits what compaction can discard.
- Reconstruct retained graphs in the new region through third-party library `<-` methods under DL-03. The library supplies traversal and reference reconstruction for its structures, preserving the identities, sharing and cycles required by its declared semantics. Whole-region relocation and graph compaction remain different operations; the language does not acquire a universal compactor from this transfer contract.
- Applications choose when to rebuild and how active users or queued requests are coordinated, then invoke the snapshot/file facility. An exclusive maintenance boundary and online rebuilding are application strategies, neither mandated by this language direction.
- The snapshot operation still needs a precise capture/consistency, byte-range, error, replacement and durability contract. Applications use that contract to implement publication, reopening, recovery and old-file removal. File replacement alone does not supply a transaction or recovery protocol.

**Remaining language work:** specify the snapshot save/replace interface and its observable outcomes. Library graph transfer supplies reconstruction; the application supplies compaction scheduling, retention and publication policy.

**Decision evidence:** retained facts/identities and query results agree before/after rebuilding; discarded state no longer occupies the new payload; interrupted publication has the outcome promised by DL-05.

<a id="dl-10"></a>
### DL-10 — Source, diagnostics and editing

**Question:** How do humans and tools author, inspect and modify the same semantics that DBMR stores and execution consumes?

- Decide surface notation after representative semantic examples exist. Existing YAML conversions are useful references, not an automatically selected language syntax.
- Distinguish readable partial projections from complete reconstructible source. Define how identity, sharing, cycles, bindings, imports, versions, comments and provenance survive a full round trip.
- Connect diagnostics, completion, hover, definition navigation and semantic edits to the same checked definitions. Distinguish source locations from stable semantic identity.
- Specify a bounded first editing workflow, including version conflicts and failed checks. Decide what is needed for the first language milestone versus later IDE integration.

The native source prototype reports selected implementation contexts alongside ambiguous method candidates and identifies the selected procedure declaration when a function demands an effectful method. These are source/checker diagnostics, not an implemented semantic-graph IDE protocol.

**Decision evidence:** render and reload a shared graph with imports; make one text edit and one equivalent structural edit; retain meaning and useful diagnostics. The current C++ clangd acceptance does not establish these future DSL capabilities.

#### Definition doclets

A triple-quoted token in leading documentation position attaches to the next named definition in the same declaration list. Put it directly before the definition, including its `export` or other declaration prefixes. Comments and whitespace, including blank lines, may intervene. A semicolon or an unrelated item ends this association. Line placement alone cannot turn a return operand into documentation. Use grammar position to decide attachment for a declaration, method overload, data constructor or named field. A closing delimiter ends the current list. A doclet cannot drift across that boundary or past a malformed declaration onto its next valid sibling. A triple-quoted operand, initializer or callable result remains a [runtime text literal](Utf8Strings.md#multiline-string-literals).

```cod
"""Checks whether a row contains a node. See [AstSlot](AstSlot)."""
export func occupied(slot:const AstSlot in O) -> bool where {const owner O} {
    case slot of { Occupied -> true; Vacant -> false }
}
```

The seed and booter need only accept the documented definition and skip its doclet in that position. Doclets have no evaluation, storage, checked effects or export identity. Future `codmate` analysis retains the doclet’s source range and the documented declaration’s identity. A tooltip at a reference follows the resolved definition, including through renamed imports and re-exports. A call resolved to one overload shows that declaration’s documentation. Where an incomplete reference has multiple candidates, the IDE preserves the distinction between them.

The tooltip supports navigable links in its text. In the example the link label is `AstSlot` and the destination is the definition selected by that name in the **documented declaration’s** context. Opening the tooltip in another module cannot retarget the link to that module’s same-spelled type. An unresolved or ambiguous link keeps its text available. Rich markup, rendering and IDE protocol details belong to the tooling. Verify the feature from a reference site with a renamed import and a shadowing name. Verify separately that adding the doclet leaves compiler behavior unchanged and returning the same text still produces its bytes.

<a id="dl-11"></a>
### DL-11 — C/C++ and other runtimes

**Question:** What interoperability contract exposes useful operations while preserving regional and language semantics?

**Planning direction (H-352):** provide a service provider interface (SPI) through which third parties can extend intrinsics; other extension logic belongs in CoD user-program libraries. External C and C++ systems can integrate through CoD’s C native interface. The SPI is the intended extension path. Provider registration and loading remain open.

- Specify ownership of buffers/regions, borrowed-view lifetimes, allocation/deallocation symmetry, error results, calling threads and callbacks. Separate physical image compatibility from a function-call ABI.
- Decide which foreign values must be materialized and which runtime handles can be temporarily bound. Native heap pointers, callbacks and operating-system handles need an explicit execution-lifetime treatment.
- Define how providers expose typed intrinsic operations and how external runtimes construct/check packets, invoke methods, consume responses and bind effects while preserving language invariants. Provider selection must fit module-world visibility and function/procedure contracts; it does not imply mutable shared registries or dynamic code loading.

**Remaining interface work:** typed provider declarations, compiler/runtime integration, native argument/result and error conventions, owner/lifetime checking and provider version compatibility. The CoD-facing interface uses C. A provider can adapt its own implementation language behind that boundary. Handle representation and the need for a wrapper depend on the operation. No third-party intrinsic extension is claimed implemented here.

**Decision evidence:** trace one foreign-runtime caller through construction, application, result consumption and release, including an error and a rejected lifetime violation.

<a id="dl-12"></a>
### DL-12 — Initial scope and execution strategy

**Question:** What is the smallest language and runtime implementation that can demonstrate the selected semantics without making the host language their only definition?

- Use direct LLVM IR as the sole execution lowering path under H-143. Keep source semantics and future persistent semantic graphs distinct from derived .ll/.bc artifacts; use LLVM's standard optimization pipeline without a multi-backend framework or custom micro-optimizations.
- Decide which typed representation, checking rules and handler interfaces are shared. Layout/validation facilities must follow one defined type contract; graph transfer/compaction algorithms remain library code under H-351/H-352, with no implicit compiler-generated universal traversal.
- Bound syntax, ADT/GADT features, effects, query primitives and FFI for an initial version. Distinguish deliberate omissions from restrictions that would prevent multi-domain integration or later type/kind work.
- H-106 separates complete dependency scanning/registration from entry-driven compilation of execution semantics. Top-level procedure declarations do not execute during scanning. H-495–H-498 orders once-incorporated initialization blocks statically, with static namespaces ready first and entry execution last.
- Keep program intent separate from native code and caches. H-96/H-102 specify demand-driven whole-program compilation, with the complete caller paths available and no independent module compilation. Collect required symbolic variables, check binding availability/types, then lay out direct access in each Actor's dynamic context on its dedicated OS thread. Concrete lowering, specialization and cache dependencies remain to be specified; this does not select machine-code JIT.

**Current bounded implementation:** the [native source prototype](../tests/cod/language/README.md) parses/checks CoD, specializes demanded call paths and emits LLVM IR/bitcode directly. The existing compiler/linker builds the native result with a small C++ system runtime. Experimental syntax and limits remain explicit. A persistent regional typed program core, general GADT and stable interoperation ABI remain design work. H-143 selects this execution path; H-144/H-145 require focused verification and removal of unnecessary complexity.

**Decision evidence:** a scoped feature list, one lowering/execution trace and a comparison criterion between reference behavior and the selected production handler.

<a id="dl-13"></a>
### DL-13 — Worked examples and readiness for development

**Question:** Which concrete example and acceptance boundaries make the first implementation worthwhile and reviewable?

**Selected first application for discussion:** [steady recurrent SNN activation](../applications/steady-snn/docs/README.md), supplied by the user in H-08 and maintained separately under H-11. Its model, resource planning and application decisions belong to that document set. Only explicitly selected reusable requirements enter the language topics here.

Candidate observations to agree during discussion:

1. A checked sum/product model and typed method express a selected application state transition, including a branch based on an input or computed result.
2. Framing, root resolution and entry invocation follow a stated protocol; incomplete or incompatible input cannot accidentally invoke the operation.
3. Packet, persistent and temporary data obey the chosen lifetime rules; references do not outlive their storage.
4. State visibility, statistics and any selected persistence/commit outcomes match the specified model, including its explicitly selected failure cases.
5. Rebuilding preserves retained identities, graph relationships and query results while removing the state the retention policy permits discarding.
6. One business program runs under reference and optimized interpretations, with the same defined semantics and explicitly bounded numerical differences where relevant. Persisting/reloading state is a separate acceptance item if included in the milestone.
7. A source round trip and a foreign caller exercise the same checked semantics, at the scope selected for the milestone.

This first local example is a possible intermediate milestone. The charter's broader validation target includes two independent domains, explicit integration and two effects interpretations; a single-domain demonstration alone would not establish that target. Decide when to add the second domain and integration example rather than dropping them implicitly.

**Development boundary:** H-96 explicitly places joint development in scope. The [paired development plan](../applications/steady-snn/docs/LanguageCoDesign.md) proposes a small symbol/call-path core followed by one complete incoming-gather frame. Record each implemented portion's explicit decisions, value derivation, invariants, interfaces, non-goals and acceptance criteria as a qualified contract. Unrelated open topics need not all be solved first, but the selected portion's dependencies and unresolved limits must be clear. Build and test authorization remains governed by repository rules.

<a id="dl-14"></a>
### DL-14 — Symbols and dynamic call paths

**Confirmed:** module-defined symbols have strict identity, imported aliases retain it, and independent origins/versions remain distinct. A typed symbol declaration fixes the binding type; use requires a supplied value along the demanded call path. Reads use `@name`; bits/ownership transfer uses `effect @name <- value`, and object-reference rebinding uses `effect @name = value`, restoring on scope exit. Bits symbols reject the equals form; reference effects satisfy their declared owner contract: region-selector/group forms retain their resource constraints, while H-561's explicit owner-parameter form admits proved lexical and ordinary regional owners. Each installation preserves its selected target and actual owner through restoration/capture. Runtime handler definition activation snapshots required symbolic bindings; module method registration does not; `perform` temporarily installs them, `behave` retains current bindings. Demand-driven WPO checks required bindings over caller paths; missing values are compilation errors, not runtime fallback.

**Confirmed module practice:** H-242 vocabulary modules define typed symbols, types (primarily abstract, with some supporting ADTs) and their method collections, with comments explaining their meaning. Selecting the same resolved vocabulary establishes common identities across independent module-world copies; this does not create one shared mutable world. H-242 broadens H-105's earlier symbol-only idiom. The [two-world inventory](ModuleWorlds.md#vocabulary-modules-establish-shared-identities) records the identity role. The following using/import/mixin rules describe the confirmed prototype baseline whose detailed world interaction remains under architecture review: `using` preserves the selected source definition and callable implementation context. `import` retains its source body, non-callable lexical bindings and captures, selects the receiving module's ordinary callable implementation context, and permits extension under H-230/H-232. `mixin` redefines a source body in the receiving lexical/dynamic scopes under H-212. These definition and implementation choices are separate from `perform`/`behave`'s captured/current value choice. D-10 records the native subset and pending compound-selection boundaries. H-231/H-233 requires explicit publication at every module boundary, including re-export.

**Confirmed module phases:** H-106/H-107 permit symbol/function definitions and bits globals initialized by literals or function calls, plus top-level `effect`/`perform`/`behave` expressions. Procedure definitions register during scanning; neither their definition semantics nor global initializers/top-level expressions execute during that scan. All entry dependencies are scanned/loaded before entry execution is compiled. Under H-495–H-498, static namespaces precede the fixed initialization sequence, dependency prefixes join that sequence as implicit blocks, and entry execution follows all initialization; source encounter does not override readiness.

**Confirmed immediate review:** every cross-module reference triggers module execution semantics (H-109). Its top-level scope remains alive until process exit (H-166); `using`/`import`/`mixin` retain H-110’s explicit selection of effective resources. Initialization completion is not resource destruction. Every `effect` yields void (H-288); object-reference rebinding uses `=`. A body-bearing effect installs its own anonymous family. `effect proc @p;` activates an existing lexical family explicitly.

**Implementation boundary:** the native subset checks typed bindings, demanded captures and bits-global initializers. Durable package/symbol identity encoding, general capture admissibility, scheduled initialization details and runtime layout/caching remain open; declaration-only cycles are permitted by H-492 but unimplemented in [SymbolicDynamicScope](SymbolicDynamicScope.md). The bounded implementation does not supply a persistent symbol ABI or independent module compilation.

**Origin/alias review:** H-112 requires explicit provenance and local aliases. H-113 excludes lexical symbolic variables and ordinary assignment. Selections resolve symbol names at compile time. The bare expression provides the selected identity at runtime. The symbolic value read accesses its active binding. H-115 permits a body alias map with matching-name defaults, now belonging to mixin under H-212. The prototype H-116 destination form uses `mixin "module" @source as @local`; its implicit installation requires migration under H-372/H-373. [D-10's relationship model](SymbolicDynamicScope.md#symbol-origins-aliases-and-dynamic-values) separates identity selection, handler definition activation and body translation. H-117 proposes `symbol from "module" @source as @local` and `with { @local as @source_role; }`, read as supplying a local symbol for the source body’s role. H-118 authorizes implementing these discussed forms. The target compiler must separate selection from explicit effect installation and resolve body aliases before runtime capture analysis; existing implicit-installation paths require migration. Broader mapping edge policies remain open; unsupported cases receive explicit diagnostics.

**Selection migration:** H-372/H-373 supersedes the implicit-installation portion of H-119/H-120’s using abbreviation. Selection/aliases and runtime installation must be separate; preserve same-identity aliases, explicit definition installation and once-only source initialization. The compiler’s current shorthand/destination paths remain implementation evidence pending coordinated migration, not the final stage contract. [D-10](SymbolicDynamicScope.md#source-selection-and-explicit-installation) identifies this boundary. Namespace and exact grammar choices remain DD-AOT-07 work.

**Entry installation:** H-379 requires an entry module that extends a dependency callable to install it explicitly; omission is a compilation error. Accepted entry installation cancels dependency installations, exposing unused world code to LLVM elimination. The [D-21 contract](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) preserves still-reachable views, evaluates canceled initialization operands and retains procedure-local installations; identity/conditional-coverage mechanisms remain under review.

**Decision evidence:** alias/version separation, nested effects, both call modes, transitive `behave` dependencies, a reachable path missing a required binding, scan-time absence of effects/captures and an encounter order different from dependency scan order. No independent module compiler is presumed.

<a id="dl-15"></a>
### DL-15 — Callable definitions, expressions and storage

**Confirmed:** functions are lexically defined and use explicit inputs/outputs. H-184 permits procedures with ordinary lexical names or symbolic dynamic bindings; both may access effects. A named lexical procedure supports direct calls. A handler definition can call such a procedure by name. H-186/H-187 allows `proc @p(...) -> RT { ... }` to introduce the symbol and method; H-372 separates this compile-time registration from runtime installation. An effect with a body installs its written local method or [grouped installation](SymbolicDynamicScope.md#installing-a-local-family-with-several-methods) for dynamic calls. The keyword `proc` is optional in that form. `effect proc @p;` activates the selected family. A direct `@p(...)` call selects that family lexically and uses current dynamic bindings. A symbolic procedure signature uses `proc(T1, T2) -> RT`. H-189 gives direct lexical calls current dynamic bindings; symbolic perform alone replays captured values. Module initialization is the top-level dynamic scope. Symbol reads and writes are expressions, and `void` indicates a result with no meaningful value. User storage is stack or regions, with ordinary owners and region-backed dynamic resources as separate uses. H-180 reaffirms dynamic-value capture; shadowing preserves owners and does not release hidden regions. Physical references stay within their region. Business associations between regions follow H-157.

**Open:** family-view compatibility/representation, concrete-type evidence through abstract results, coverage closure, namespace/staging and diagnostic mechanisms in D-21; the first regional GADT construction contract and business protocols for logical relationships between regions retain their separate scope. A scope-local `effect` restores on exit under either call mode; `behave` does not make rebinding permanent. No `malloc` closure, raw stack reference capture or user-visible host address is inferred.

**Decision evidence:** lexical function lookup and dynamic handler selection, effect access rejected in functions, expression result typing, scope restoration, and capture/lifetime examples. H-98–H-104, D-10.

<a id="dl-16"></a>
### DL-16 — Language and SNN development together

H-96 establishes coordinated language and SNN development within qualified scopes. [LanguageCoDesign](../applications/steady-snn/docs/LanguageCoDesign.md) proposes application-driven increments; [current source support](SymbolicDynamicScope.md#implementation-availability) is maintained separately. H-94’s target-owned two-frame pull bounds the application without fixing neuronal equations or training algorithms.

**Current increment:** H-221 authorizes ten hours of active language/runtime work, excluding waits, under the confirmed module, type, method and regional-access contracts. H-122/H-123 keeps SNN modeling and learning outside this scope. H-225–H-227 fixes associated signature membership, using/import-dependent local declarations and caller-selected ordinary implementation views. New unresolved policies remain queued for consolidated review under H-160. Application-shaped arithmetic fixtures validate programming facilities without becoming model equations.

The native subset includes nominal regional data products, abstract interfaces with explicit adapter conformance, FP32/FP64 values, typed function/procedure dispatch, parenthesized intrinsic/operator overloads, named UoMs and selected conversion routes. Bounded loops/reductions, static SoA arrays, caller-region allocation, explicit-memory mutation, borrowed traversal/results and repeated multi-module A/B gathers have native evidence. Whole-image relocation and scope-lifetime checks retain their own evidence. Source syntax and owning-region specialization are bounded prototype conventions; no full storage ABI, cross-region representation or modeling acceptance is implied. H-139–H-141 motivates complex dependencies, once-only incorporation after scanning and measured initial inlining.

Numerical evaluation motivates functions with explicit inputs; dynamic frame context and ambient services motivate procedures. Functions may edit explicitly supplied memory. Each increment requires semantic examples and execution evidence before optimization or SNN learning success is claimed. The [case catalog](../applications/steady-snn/research/cod-language-scope-cases.json) specifies application-driven targets; current compiler coverage is maintained in the source capability reference. Full catalog and neuronal acceptance remain separate.

<a id="immediate-increment-decisions"></a>
#### Immediate increment decisions

H-133–H-138, H-148–H-150 and H-153–H-155/H-158/H-159 resolve the following choices without requiring another lifetime mechanism or compound-unit algebra:

| ID | Decision | Current rule |
| --- | --- | --- |
| LI-01 | Result references from a function | H-133/H-197: returning an existing input-reachable reference retains its source-region lifetime. Mutation is allowed; returning a view cannot keep a locally released region alive. |
| LI-02 | UoM definition reuse | H-134: fresh definitions and one-shot using only; no UoM import. Select the intended UoMs and their associated conversions together. Partial selections exclude conversions involving omitted UoMs; repeated using does not extend the bundle. |
| LI-03 | Conversion during method selection | H-135: H-260/H-261 `amount:f64(m)` preserves unit identity while allowing lossless representation widening; `amount:f64(m*)` also permits selected unit conversions. This is a user-declared parameter policy, not a universal language restriction. |
| LI-04 | Conversion route selection | H-136: discover routes at compile time, choose a unique route automatically, warn about unresolved alternatives and allow warnings-as-errors. H-137/H-138: a direct conversion wins; otherwise use the declared indirect route, illustrated by `m* <- cm <- mm`. |
| LI-05 | Argument/conversion order | H-148: evaluate each argument once and finish its selected conversions before evaluating the next argument. An earlier failure prevents later argument effects. |
| LI-06 | Local value names | H-149: nested scopes may shadow outer locals; declarations within one scope must be unique. The initializer sees the outer value, and scope exit restores its visibility. Function parameters and loop bindings share their body’s declaration scope. |
| LI-07 | Reserved expression names | Reserve literal and control spellings that preclude lookup. The ten [ordinary names](SymbolicDynamicScope.md#ordinary-names-for-traversal-and-storage) use ordinary binding and dispatch. `touch @symbol` declares a symbolic dependency. |
| LI-08 | Intrinsic/operator overloads | H-153–H-155: reserve ordinary keyword names and allow parenthesized operation methods of either callable kind. H-158: intrinsics use compiler-internal representation, separate from CoD symbols. H-159: a selected procedural operation uses current bindings, like `behave`. UoM conversions remain pure. |

The run develops structured product values, named UoMs with these selection/conversion rules, Julia-style argument-type methods and parenthesized operator names, bounded numerical iteration, static SoA regional arrays and owner-tracked explicit-memory traversal. Regional references retain existing lifetime/relocation rules, including H-146’s accepted first-field self-reference-to-null limitation. [Local value shadowing](SymbolicDynamicScope.md#local-value-shadowing) follows H-149; symbolic writes still require `effect`. Compound-unit algebra, general GADT machinery, custom operator precedence and cross-region representation remain separate increments. H-161 reserves primary module-form words against ordinary declarations and aliases. H-162 selects the distinct `(=)` family; H-163/H-164 make indexed assignment call `([]=)(array, index, value)` while preserving nominal container/index types. Field writes use `record.field <- value` or `record.field = target` according to the destination. General computed field selectors remain open. The completed H-147 work period and H-160 review retain their historical scopes. No new SNN dynamics or learning rule enters this work.

Validation uses CoD language/runtime component checks and arithmetic fixtures: strict versus convertible parameters, using-bundle selection, route preference/warnings, structured index conversion, rejected domain mismatches, nested alias-sensitive reads and writes, returned input references, and repeated numerical gather/A-B execution. Existing numeric failure behavior supplies a reference for invalid arithmetic; no new exception/recovery framework is required. Source-only compilation uses the repository's active build tree and normal build command; build configuration and toolchains remain unchanged.

## Implementation evidence to account for

The linked facilities provide implementation context, not independent design authority. Source-language availability is maintained in the current capability reference.

| Existing facility | Relevant observation | Topics |
| --- | --- | --- |
| [Regional storage](../include/shilos/region.hh) | `regional_ptr` is self-relative; allocation advances `occupation_`; capacity and occupied bytes are distinct. The current `RegionalType` concept is a destructibility check, not a recursive proof of legal regional fields. | DL-01, DL-03, DL-04 |
| [Shilos C++ runtime pointer](../include/shilos/region.hh) | The host `global_ptr` facility holds a native reference to a region and an offset. CoD business-level identity and resolution have a separate design scope. | DL-03, DL-08 |
| [DBMR](../include/shilos/dbmr.hh) | Mapped-region validation checks header/root conditions. `constrict_on_close` trims unused tail capacity; it does not rebuild a graph to reclaim unreachable internal allocations. | DL-04, DL-06, DL-09 |
| [CoD driver](../cod/main.cc) | The [source capability reference](../tests/cod/language/README.md) records current checking, LLVM emission and native execution support. General GADT construction and stable language/interoperation ABI remain separate design scopes. | DL-02, DL-12 |

The [memory-region design](MemoryRegionAndRegionalTypes.md), [CoD driver design](CompileOnDemand.md) and [YAML conversion design](YamlConversionsOfRegionalTypes.md) remain relevant references with the qualification limits listed in the registry. Reusing code does not select its representation as the language's contract.

## Discussion process

Use confirmed CoD semantics to answer design questions. When an example follows from an existing rule, cite the rule and record its consequence. Current seed behavior belongs in implementation evidence.

Application design lives in its linked document set. Language topics record their current answer, governing constraints and open choices; application proposals become reusable language requirements only through an explicit scope and value decision. Human evidence and brief continuity entries belong in the [changelog](../CHANGELOG.md#authority-evidence-index). Update the relevant topic and current registry scope together; a proposal does not supply its own approval.
