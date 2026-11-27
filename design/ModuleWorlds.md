# Module worlds and flat TLS bindings

Design revision: 87 · Recorded: 2026-11-27 · Status: module-world contract; implementation evidence refers to the preserved source-compiler baseline.

## Design authority

At r87 [D-15](DesignAuthorityRegistry.md#d-15) continues r86’s publication rules. The [family merge](#merging-type-owned-method-families) accounts for indexing alongside construction and data calls. [Receiver dispatch](DomainLanguageDesign.md#how-receiver-dispatch-works) distinguishes these operations from ordinary callable name lookup. [D-07-NAMED-FALLBACK](DesignAuthorityRegistry.md#d-07-named-fallback) uses ordinary lookup for the free tier of a dotted call.

At r86 [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families) applies r85’s receiver publication and ordinary callable lookup to [automatic abstract defaults and visible free indexing operators](CollectionsAndBuiltinContainers.md#abstract-defaults-and-free-indexing-operators). The two extension grants retain their existing scope.

At r85 [D-30-INDEX-FAMILIES](DesignAuthorityRegistry.md#d-30-index-families) extends r84 type-view publication to [receiver-carried indexing methods](CollectionsAndBuiltinContainers.md#receiver-families-for-indexing). Contributing a method requires authority over the receiver. Selecting an adapter can add its published method in the receiving world.

At r84 the [ASCII vocabulary example](../CHANGELOG.md#k-ascii-vocabulary-20261125) applies the existing [default and complete public selections](#default-and-complete-public-selections) to a type with many members under [D-15](DesignAuthorityRegistry.md#d-15) and [D-15-DEFAULT-EXPORTS](DesignAuthorityRegistry.md#d-15-default-exports), continuing P-692/P-693. Compare an omitted selector with an explicit member subset, then use another member through the selected enum type.

At r83 the [grouped method decision](../CHANGELOG.md#h-grouped-method-dispatch-20261125) applies r82's family identity and extension permissions to value-refined methods under [D-10-METHOD-GROUPS](DesignAuthorityRegistry.md#d-10-method-groups). Check a receiving world with independent cold-shipment and express-service methods, then supply the intersection there. Selection uses the same permissions and actual argument evidence in either import order.

At r82 the [OO call-operator decision](../CHANGELOG.md#h-oo-call-operator-head-20261125) applies to methods defined outside the type. [D-07-FUNCTOR-LOOKUP](DesignAuthorityRegistry.md#d-07-functor-lookup) continues P-636/P-637 from r81. The two-module example checks extension grants, method identity and conflicts in the receiving world.

At r81 the [bootstrap input decision](../CHANGELOG.md#h-manual-bootstrap-packages-20261125) supplies the project and its manually staged dependencies to the existing [module-path resolution](#resolving-source-module-paths). [D-15-MODULE-PATH](DesignAuthorityRegistry.md#d-15-module-path) continues P-690/P-691 for relative edges, ambiguity, bundled fallback and canonical module identity.

At r79 the [operation table](#callable-families-and-explicit-installation) applies the [functor-value decision](../CHANGELOG.md#h-functor-value-distinction-20261124) and its receiver-owner requirement to direct symbolic calls. D-10 and D-18 govern installation and value-kind checking.

At r78 the [function-valued symbol decision](../CHANGELOG.md#h-function-valued-symbol-20261124) qualifies the call distinction in the [operation table](#callable-families-and-explicit-installation). Resolve the declared kind in the selected module interpretation and preserve the direct procedure call without installation. [D-10 source contract](SymbolicDynamicScope.md#calling-a-function-valued-symbol)

At r77 the [symbol identity decision](../CHANGELOG.md#h-runtime-symbol-identity-20261124) applies [D-10’s protected lexical name](SymbolicDynamicScope.md#the-bare-symbol-name-at-runtime) to module selection and symbol registration. Distinct spellings may denote the same identity. A binding that would hide a visible symbol fails at its declaration.

[D-15](DesignAuthorityRegistry.md#d-15) r76 applies the [grouped installation decision](../CHANGELOG.md#h-grouped-local-proc-install-20261124) and the [local handler decision](../CHANGELOG.md#h-local-proc-handler-body-20261124) under P-330. A local body can bind an exported symbol for a scope while its published family and the receiver’s extension permissions stay fixed. Compare lexical and dynamic calls in the [same-signature example](SymbolicDynamicScope.md#installing-a-local-procedure-body).

[D-15-DEFAULT-EXPORTS](DesignAuthorityRegistry.md#d-15-default-exports) r74 continues the [provider example](../CHANGELOG.md#k-seed-empty-procedure-import-20261124) for [identity introduction by procedure import](#selecting-a-symbolic-procedure-family) under [H-PROC-IMPORT-IDENTITY-20261124](../CHANGELOG.md#h-proc-import-identity-20261124) and P-692/P-693. Verify the single directive on empty and populated families, optional explicit identity selection and a conflicting destination. The receiving interpretation and extension permission belong to the import.

[D-10-EMPTY-PROC-FAMILY](DesignAuthorityRegistry.md#d-10-empty-proc-family) at D-15r72 covers [selection before the first procedure method](#selecting-a-symbolic-procedure-family) under [the project user’s publication decision](../CHANGELOG.md#h-empty-procedure-family-20261124). P-709/P-710 retain H-374’s complete-family publication, destination identity and receiving-world permissions. Check a vocabulary with zero methods and an importer contributing the first one.

[D-15-PUBLIC-SOURCE](DesignAuthorityRegistry.md#d-15-public-source) r71 applies the [confirmed semicolon policy](SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux) to the closing delimiter of an [explicit resource group](#public-definitions-and-grouped-selections). P-662/P-663 preserve public identity and each verb's permissions. Compare both spellings at EOF and before a following module item in both frontends, including recovery from malformed lists. [Continuity and human baselines](../CHANGELOG.md#k-grouped-selector-boundary-20261123)

At r70 the [symbolic selector guidance](../CHANGELOG.md#h-symbolic-selector-guideline-20261123) annotates [family selection](#selecting-a-symbolic-procedure-family) under D-15-DEFAULT-EXPORTS and D-DOC-IDIOMATICS. P-692/P-693 preserve module authors' public vocabularies and receivers' choice of reuse. Check that each example identifies the required identity, family or extension and the site that installs it. The [family reuse baseline](../CHANGELOG.md#h-symbolic-family-reuse-idiom-20261123) and existing permissions govern all three examples.

[Review backlog and status reconciliation](../CHANGELOG.md#h-review-backlog-duty-20261123) reconciles the scoped confirmations below with their registry records.

[Confirmed symbolic procedure forms](../CHANGELOG.md#h-lexical-symbolic-proc-20261123) qualifies D-15-DEFAULT-EXPORTS for a kind-qualified procedure `using`. P-692/P-693 preserve explicit origins, receiver permissions and equivalent written and expanded selectors. Check a free spelling, identical prior selection and a conflicting origin. [Lexical and dynamic calls](SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)

[Editorial continuity](../CHANGELOG.md#k-lexical-callee-examples-20261123) aligns lexical calls and symbolic procedure installation with [D-18 callable value kinds](CallableStorageAndHereAccess.md#callable-value-kinds) and [D-10 handler installation](SymbolicDynamicScope.md#definition-installed-effectful-procedures). Existing qualification boundaries apply.

[D-15-DEFAULT-EXPORTS](DesignAuthorityRegistry.md#d-15-default-exports) and [D-15-PUBLIC-SOURCE](DesignAuthorityRegistry.md#d-15-public-source) at r65 apply the [declaration equivalence](../CHANGELOG.md#h-using-symbolic-variable-identity-20261123) to a [published symbolic variable](#selecting-a-symbolic-variable). P-692/P-693 retain the declared identity, source interpretation and permissions. Check both spellings before installation and compare actual reads across shadowing and cleanup. The rules for procedure-family selection remain with the receiving verb.

[D-15-MODULE-PATH](DesignAuthorityRegistry.md#d-15-module-path) and [D-15-DEFAULT-EXPORTS](DesignAuthorityRegistry.md#d-15-default-exports) at r62 cover [module path resolution](#resolving-source-module-paths) and [the default public selection](#default-and-complete-public-selections). The project user [confirmed the lookup order, precedence warning and module declarations](../CHANGELOG.md#h-module-paths-and-default-exports-20261123). P-690–P-693 connect reproducible dependencies and library-author intent to explicit module identity and readable source. Verify distinct sibling, package and bundled modules and actual string-library selection in both bootstrap compilers. Runtime effect installation, package acquisition, toolchain layout and overload dispatch retain their separate contracts.

[D-15-DEFAULT-EXPORTS](DesignAuthorityRegistry.md#d-15-default-exports) and [D-15-PUBLIC-SOURCE](DesignAuthorityRegistry.md#d-15-public-source) at r64 cover [default selector substitution and kind-qualified symbolic resources](#default-and-complete-public-selections). The [user decision](../CHANGELOG.md#h-module-default-selector-substitution-20261123) supersedes the r63 bulk-symbol interpretation. P-692/P-693 and P-662/P-663 preserve the author's public vocabulary and the caller's choice of reuse operation. Verify explicit and omitted lists under each verb, including failures, and verify independent symbol identity selection. Package precedence and overload dispatch have separate scopes.

[D-15-PUBLIC-SOURCE](DesignAuthorityRegistry.md#d-15-public-source) at r61 applies its single-identity publication rule to [a named owning handle symbol](../applications/cod-booter/docs/LLVMInterop.md#owning-native-handle-effects) under [H-NATIVE-HANDLE-EFFECT-20261121](../CHANGELOG.md#h-native-handle-effect-20261121). P-662/P-663 preserve vocabulary identity and separate publication from runtime installation. Check an exported symbol selected before any acquisition.

[D-15-PUBLIC-SOURCE](DesignAuthorityRegistry.md#d-15-public-source) at r60 applies the existing publication rule to a named `handle type` under [H-HANDLE-TYPE-WITHIN-20261121](../CHANGELOG.md#h-handle-type-within-20261121). P-662/P-663 preserve nominal identity and constructor contracts through selections and renamed re-exports. Check two different defining modules with identically spelled handle types.

[D-15-PUBLIC-SOURCE](DesignAuthorityRegistry.md#d-15-public-source), P-662/P-663, covers [definition-site export and grouped selection](#public-definitions-and-grouped-selections) at r59. [H-MODULE-SOURCE-STYLE-20261121](../CHANGELOG.md#h-module-source-style-20261121) establishes readable public definitions and source-grouped dependencies for the booter. Publication, selection grants, privacy and module initialization follow the individual operations. Check equivalent publication and lookup in both bootstrap compilers.

- **Data call operators.** [H-FUNCTOR-PUBLICATION-20261119](../CHANGELOG.md#h-functor-publication-20261119) confirms the [source publication and merge rule](#publication-of-type-carried-call-operators) at r57 under [D-07-FUNCTOR-LOOKUP](DesignAuthorityRegistry.md#d-07-functor-lookup). C-10/C-35 and P-636/P-637 connect domain operations to nominal type identity and the module world that selected them. Check compatible extensions, repeated selection of one method, incompatible contributions from two adapters and world isolation. Native representation and implementation qualification are separate.

- **Module top levels as here callables.** r53 [module scope](#here-callable-module-top-levels) records [H-HERE-MODULE-TOPLEVEL-20261118](../CHANGELOG.md#h-here-module-toplevel-20261118) under [D-18-HERE](DesignAuthorityRegistry.md#d-18-here). D-15-STATIC-NS, D-15-POSTPON and D-15-SHUTDOWN supply initialization readiness, shared module contexts and cleanup lifetime. These constraints serve charter §§3.3/4.3. Acceptance observes one module slot across interleaved startup fragments and preserves access checks on entry and shutdown. Qualification of the broader callable redesign remains open.

- **Constructor family.** [H-DCTOR-FAMILY-MERGE-20261119](../CHANGELOG.md#h-dctor-family-merge-20261119) confirms the [shared module-world merge rule](#merging-type-owned-method-families) for [D-07-DCTOR-PUBLICATION](DesignAuthorityRegistry.md#d-07-dctor-publication) at r58. C-10/C-35/C-45 and P-603/P-604 connect explicit selection to stable type and dctor identities. Check compatible and conflicting contributions to one dctor, equal signatures on different dctors, and braced construction in a base versus combined world. H-DCTOR-WORLD-PUBLICATION-20261118 and H-DCTOR-SOURCE-20261118 govern selectors and extension permission. Native implementation qualification remains separate.

- **Module shutdown reporting.** D-15r50 applies the [D-17 publication rule](ExceptionControl.md#when-a-defer-pass-cannot-publish-its-aggregate) to the D-15-SHUTDOWN/P-458–P-460 drain. [H-OOM-BEST-EFFORT-20261118](../CHANGELOG.md#h-oom-best-effort-20261118) records this dependency update. Startup remains aborted, committed cleanup runs and owners are released even if final reporting loses detail. Native integration remains open.

- **Full-scope dependency continuity:** [K-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#k-full-scope-implementation-20261118) updates only the referenced r92 qualification. This document’s semantic revision, own qualification, acceptance and exclusions are unchanged.

- **Current capture-review reference:** [K-REPLACEMENT-EFFECTS-20261117](../CHANGELOG.md#k-replacement-effects-20261117) keeps the separate D-18 value-input qualification current. This file changes only review references; its semantic revision, governing value paths, constraints and acceptance are unchanged. The registry owns the current review scope.

- **Callable placement and transfer boundaries:** [H-563](../CHANGELOG.md#h-563)/[H-567–H-568](../CHANGELOG.md#h-567), [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/P-503–P-505 and [D-22-TRANSFER](DesignAuthorityRegistry.md#d-22-transfer)/P-506–P-508 cover D-15r47's placement/operation terminology. Callable-view values have lexical-scope storage, never regional storage; only pure bits admit whole-payload transfer. CoD has no C++ object copying/moving model. C-54/C-65/C-72/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve regional relocatability and explicit identity/lifetime responsibility. D-18/D-22 own acceptance; retain dedicated callable construction/capture, graph transfer, actual owners and D-16. [K-LEXICAL-PRODUCTS-20261117](../CHANGELOG.md#k-lexical-products-20261117) aligns dependencies. H-564–H-566's callable-bearing ADT/anonymous-data direction remains under review, including type identity/results; no new aggregate implementation or full proof/physical/value-path qualification follows.

- **Owner-parameter symbol integration:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499, covers D-15r46's reference-symbol/owner dependency wording. The source form and staged sound acceptance are confirmed: installation selects one actual lexical or ordinary regional owner from the RHS; an `in @arena` declaration retains its additional selector constraint. C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve explicit dynamic context and lifetime responsibility. D-10 owns examples and acceptance. Preserve saved/captured owner validity, access/null/init, module symbol identity, no lifetime extension and D-16; full proof/physical/value-path qualification and source implementation remain separate. Other qualified scopes continue.

- **Native-coverage reference continuity:** [K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) preserves this revision's D-26 dependency at its current AOT contract section. Only dependency links and this summary change; existing semantics, authority, acceptance and D-16 remain unchanged.

- **Full scopes are ordinary callable invocations:** [H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117)/[K-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#k-full-scope-callable-20261117), [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope)/P-564–P-566, covers r48→49 in the explicit-full-scope interactions below. The user confirms ordinary capture, local return and independent cleanup everywhere; D-17 owns the source acceptance. Reuse C-63/C-65/C-73/C-75/C-84 and charter §§2/3.2–3.3/4.3/6.3/8/12 for one inspectable callable/lifetime model. Preserve actual owners, permissions, declared failures and D-16; full scopes preserve lexical lifetimes and follow the [callable result rules](CallableResultInference.md#results-of-proc-func-and-here-bodies). [H-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#h-full-scope-implementation-20261118) separately qualifies D-17’s bounded r92 full-scope profile, including immediate ref/ptr inputs; general escaping borrowed-view and declared-place capture mechanisms remain outside it. D-18's value-input profile retains its independent qualification. D-10 separately records the selected trailing callback surface; complete implementation/value-path qualification remains pending.

- **Checked deferred coverage:** [H-550/H-551](../CHANGELOG.md#h-550)/[K-DEFER-CHECKED-COVERAGE-20261116](../CHANGELOG.md#k-defer-checked-coverage-20261116), [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch)/P-461–P-463, requires checked failures to be handled before their owning defer pass completes. A covering defer catch counts; enclosing throws alone cannot authorize residual checked members. Preserve actual registration/FIFO order, H-525 declared obligations, retained ordinary body_exception, owners and D-16. D-17r86 owns acceptance and conservative proof boundaries. Candidate C-36/C-44/C-63/C-65 and charter §§2/3.1–3.3/4.3/6.3/8/12 serve explicit recoverability and valid ordered cleanup. This dependency alignment continues unrelated contracts; it does not qualify the whole mechanism/value path or assert compiler support.

- **Return-state dependency:** [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state)/[H-542–H-547](../CHANGELOG.md#h-542), [H-548](../CHANGELOG.md#h-548)/[K-RETURNING-PROFILE-20261116](../CHANGELOG.md#k-returning-profile-20261116), aligns this revision’s module cleanup under P-484. Candidate C-54/C-63/C-65 and charter §§2/3.2–3.3/4.3/6.3/8/12 preserve author-controlled completion, result/owner validity and observable cleanup. D-17/D-23 own acceptance: scoped cancellation, unbound inspection under the [normal-completion decision](../CHANGELOG.md#h-normal-completion-20261124), ordinary binding grammar/copy/borrow lifetimes and owning inspection without capture. Module cleanup has no callable return target. Preserve unrelated qualified contracts, FIFO failures and D-16. The H-548 agent selections do not add implementation support or full value-path/physical qualification; owning capture and future pure-callable rules are excluded.

- **Body failure during global cleanup:** [H-538](../CHANGELOG.md#h-538)/[K-DEFER-BODY-EXCEPTION-20261116](../CHANGELOG.md#k-defer-body-exception-20261116) continues the module cleanup constraints in r40 under D-15-SHUTDOWN/P-458–P-460 and D-17-DEFER-DISPATCH/P-461–P-463. D-17's four-case table retains a still-pending ordinary startup/body exception in readonly body_exception when residual deferred failures require an aggregate, without a new cause link. Preserve no body-error matching by defer catch, valid owners and aborted-startup control flow; candidate C-36/C-59/C-63/C-65 paths serve charter §§2/3.3/4.3/6.3/12. Physical support, caller coverage and remaining queue policies stay separate; no broader qualification follows.

- **Disjoint declarations:** [D-10-DISJOINT](DesignAuthorityRegistry.md#d-10-disjoint), [H-527–H-535](../CHANGELOG.md#h-527) and [K-DISJOINT-ENDPOINTS-20261116](../CHANGELOG.md#k-disjoint-endpoints-20261116) cover this revision's constraint-related sections. Existing listed-endpoint ancestry permits inherited overlap; a second independent listed-parent relationship rejects, while prior redundant relationships warn about the path and intermediate methods. Preserve complete public records, per-pair definition/import authority, frozen validation, declared checkedness, demanded checking, owners/captures and D-16. Candidate C-56/C-10/P-476–P-478 serves explicit nominal intent and stable interpretation under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12. [D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) owns acceptance; finite generic/cyclic proof procedures, source implementation and full value-path qualification remain separate. No separate exemption-path store or runtime inheritance mutation.

- **Shared-information candidate:** [D-15-DISJOINT-POOL](DesignAuthorityRegistry.md#d-15-disjoint-pool)/P-479–P-481 under C-59/C-56/C-10 records [H-533's](../CHANGELOG.md#h-533) requested feasibility review. [D-15](ModuleWorlds.md#shared-disjoint-information-per-module-world) owns identity, upstream retention, applicable local/source checks, publication filtering and frozen-backing acceptance. Physical representation and the complete value path remain pending-human; no compiler support is inferred.

- **Checked and unchecked roots:** [H-524–H-526](../CHANGELOG.md#h-524)/[K-ERROR-ROOTS-20261116](../CHANGELOG.md#k-error-roots-20261116), [D-17-ERROR-ROOTS](DesignAuthorityRegistry.md#d-17-error-roots), adds Error <: Exception and preserves explicit Exception handling obligations even for values proven unchecked. Checkedness-changing adapter conformance rejects. Candidate P-473–P-475 serves explicit typed failure contracts and user-defined errors under charter §§2/3.1/3.3/4.3/6.3/8/12. Existing actual-type dispatch, concrete failure categories, owner/capture and D-16 contracts continue. New roots/declaration enforcement and arbitrary exception payload support are not established by current source tests; general disjointness and remaining wrapper/throws/inspection scope remain separate.

- **Wrapper-aware catch dependency:** [H-518–H-522](../CHANGELOG.md#h-518)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), [K-WRAPPER-CATCH-20261116](../CHANGELOG.md#k-wrapper-catch-20261116), records source-order wrapper-prefix matching and same-exception-identity selection of the hidden tip within the lexical catch context under candidate P-470–P-472/charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve ordinary payload type evidence, complete incoming exception history for selected exception operations, actual owners and D-16. [H-523](../CHANGELOG.md#h-523)/[K-RETHROW-FROM-20261116](../CHANGELOG.md#k-rethrow-from-20261116) requires standard-wrapper cause/checkedness agreement and adds automatic rethrow from cause, with a direct diagnostic suggestion on mismatch. Inspection and transparent checked coverage remain pending in D-17; no copied graph, general value ABI or new source support is inferred.

- **Exception construction and OOM context:** [H-510–H-517](../CHANGELOG.md#h-510)/[K-EXCEPTION-CONSTRUCTION-20261116](../CHANGELOG.md#k-exception-construction-20261116) continues this dependency in r35→36. [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve), candidate P-464–P-469 under charter §§2/3.2–3.3/4.3/6.3/8/12, separates unchanged throw propagation from constructor-time trace/cause creation, lexically supplied rethrow defaults and the bounded process OOM diagnostic window. Preserve semantic checkedness, module/callable contexts, captures, actual owners, independent deferred-queue obligations and D-16. D-17 owns acceptance and remaining representation/aggregate/ordering questions; no compiler support or whole-scope qualification follows.

- **FIFO cleanup dependency:** [H-541](../CHANGELOG.md#h-541)/[K-DEFER-FIFO-20261116](../CHANGELOG.md#k-defer-fifo-20261116) covers r41 under D-15-SHUTDOWN/P-458–P-460 and D-17-DEFER-DISPATCH/P-461–P-463. Global actions retain reverse registration order; bodyless entries process their incoming errors FIFO, with outgoing failures appended and optional finally once. Candidate C-36/C-44/C-59/C-63/C-65 paths preserve observable order, selected contexts and live owners under charter §§2/3.2–3.3/4.3/6.3/8/12. Acceptance follows D-17 and the cross-module section below. Preserve H-538 body_exception, startup abort, checked obligations, owners and D-16; aggregate/body-field integration, return-inspection/ownership integration and physical realization and complete qualification remain separate.

- **Current continuity:** [K-MODULE-DRAIN-SYNC-20261116](../CHANGELOG.md#k-module-drain-sync-20261116) continues r33→34 under H-502: the ready-node rule distinguishes unproved actual context/alias/initialization from different legal intermediate values. H-501 order, all recorded value paths and pending completion/physical scopes are unchanged. Current source coverage incorporates K-749 without claiming startup support.

- **Initialization publication and shutdown:** [H-502](../CHANGELOG.md#h-502)/[D-15-PUBLICATION](DesignAuthorityRegistry.md#d-15-publication) resolves binding visibility through existing whole-block scheduling: reads between separate blocks may see intermediate state; no source-final-binding barrier. [H-501](../CHANGELOG.md#h-501)/[D-15-SHUTDOWN](DesignAuthorityRegistry.md#d-15-shutdown) confirms application-wide reverse registration for module-level deferred actions, with retained module contexts and valid owners, including established work on startup failure. P-455–P-460 retains candidate C-35/C-36/C-44/C-59/C-63/C-65 paths to charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit state, observable ordering and valid cleanup. [K-STARTUP-PEER-REVIEW-20261116](../CHANGELOG.md#k-startup-peer-review-20261116) records feasibility; [K-MODULE-SHUTDOWN-20261116](../CHANGELOG.md#k-module-shutdown-20261116) integrates global cleanup; [K-BLOCK-BINDING-VISIBILITY-20261116](../CHANGELOG.md#k-block-binding-visibility-20261116) records r32→33 and the block-boundary clarification. Preserve ordinary nested-scope cleanup, complete captures, actual owners, fixed startup scheduling and D-16. Acceptance is below. D-17-DEFER-DISPATCH owns the handler distinction and unresolved completion/representation boundaries; ordering confirmation alone does not qualify those policies or claim compiler support.

- **Declaration cycles and scheduled initialization:** [H-492](../CHANGELOG.md#h-492)/[D-15-CYCLES](DesignAuthorityRegistry.md#d-15-cycles) confirms declaration-only cycles. H-493–H-498/[D-15-POSTPON](DesignAuthorityRegistry.md#d-15-postpon) fixes ordered trailing postpon blocks, dependency prefixes as implicit blocks retaining their module contexts, entry execution last and compile-time ordering emitted as fixed native code. [D-15-STATIC-NS](DesignAuthorityRegistry.md#d-15-static-ns)/H-497 requires static definitions in runtime module namespaces before initialization. Candidate C-10/C-35/C-36/C-44/C-59/C-65 through P-443–P-451 serves charter §§2/3.1–3.3/4.2/6.3/8/12 through explicit dependencies, contexts and checked execution. Preserve publication, method identity, source-owned worlds, scope/owner/capture rules, once-only execution and ordinary failure cleanup. Acceptance and proof/representation exclusions are below. [K-STATIC-MODULE-STARTUP-20261116](../CHANGELOG.md#k-static-module-startup-20261116) integrates r29→30; the source compiler still rejects cyclic discovery and does not implement postpon. Direction confirmation does not qualify the complete value path or physical protocol.

- **Namespaces and entry selection:** [D-15-ENTRY](DesignAuthorityRegistry.md#d-15-entry), H-499/H-500 and P-452–P-454 cover the terminology and entry section below. Package is the dependency/version unit; the CLI selects the containing namespace of a reserved local `__main__` module, allowing several entry namespaces per package. Other modules cannot select/re-export an entry module. Candidate C-35/C-44/C-45/C-59 through charter §§2/3.2/4.2/8 separates identity, source organization and execution roots. Preserve library selection, privacy, module contexts and D-16; no directory-created owner, implicit sibling initialization or qualified expression syntax. Exact CLI grammar/path mapping and physical namespace representation remain open. H-500 confirms the containing-namespace choice, not the entire package-management design; the same continuity record covers this addition and its acceptance targets.

- **Implicit source lookup:** [D-15-TYPE-METHODS](DesignAuthorityRegistry.md#d-15-type-methods)/[D-15-LOOKUP](DesignAuthorityRegistry.md#d-15-lookup), r28's type-selection and fallback sections, records H-483's shadowing and [H-484](../CHANGELOG.md#h-484)'s source-module fallback direction. [H-485](../CHANGELOG.md#h-485)/[D-15-LOOKUP-INFO](DesignAuthorityRegistry.md#d-15-lookup-info) reserves informational diagnostics at implicit-use sites for later implementation. Candidate C-35/C-45/C-57/C-59 via P-425–P-433 serves charter §§3.1/3.2/4.2/6.3/8/9 through reusable public definitions, explicit identity/context and inspectable source provenance. Preserve privacy, source-world isolation, explicit extension/publication, name-before-dispatch, captures and unique interpretations. Acceptance and remaining engineering boundaries are below. Human confirmation covers the stated directions, not full value-path/operational qualification, a general distance algorithm or current compiler support. [K-SOURCE-FALLBACK-20261116](../CHANGELOG.md#k-source-fallback-20261116) records the r26→27 integration; existing identity/helper contracts continue. [H-487](../CHANGELOG.md#h-487)/[D-15-LOOKUP-TRANSITIVE](DesignAuthorityRegistry.md#d-15-lookup-transitive) additionally confirms the intermediate private-binding barrier in the multilevel model requested by H-486; P-434–P-436 records that scope. No runtime module object or qualified-access syntax is introduced.

- **Explicit callable bundles and re-export:** [H-488](../CHANGELOG.md#h-488) permits an upstream public re-export alongside a private local namesake and directs origin-based information suggestions. H-489/H-490/[D-15-METHODS-CLAUSE](DesignAuthorityRegistry.md#d-15-methods-clause) selects optional with methods and whole-callable selection. H-491/[D-15-REEXPORT](DesignAuthorityRegistry.md#d-15-reexport) selects publication-only export from, including public renaming. P-437–P-442 extends the candidate C-35/C-45/C-57/C-59 path to charter §§3.1/3.2/4.2/8 through explicit identities, selectable behavior and independent local/public intent. Preserve source contexts, public-source access, collision rules, owners/captures and D-16. Acceptance and implementation boundaries are below; [K-REEXPORT-METHODS-20261116](../CHANGELOG.md#k-reexport-methods-20261116) integrates r27→28. Direction/surface confirmation does not qualify a general module graph, physical representation or current compiler support.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-15r23→24 with the implemented vocabulary and typed-root catches recorded in [source coverage](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Registry / current scope:** [D-15](DesignAuthorityRegistry.md#d-15); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole operational contract pending-human.** C-59/C-60/P-204–P-220; H-234/H-237–H-249 specifies receiver-owned identity construction, explicit publication and typed TLS. H-387/K-573 covers the fused artifact; K-593 aligns its input boundary.
- **Value path:** C-59/P-204/P-209 relates receiver-owned semantic construction to C-10 and charter §§1–4. P-205 preserves explicit publication and local identity merging. H-245/P-216 governs world isolation and the compile-layout/runtime-binding distinction; P-206 is superseded. P-210–P-212 distinguishes the entity sets and typed-pointer roles. H-241/P-213 places module worlds in D-12's vision; H-242/C-60/P-214 supplies common vocabulary identities. H-252/P-219 requires a diagnostic for multi-alias extension while retaining shared identity; H-253/P-220 requires conflicting inherited/selected overloads to reject during import adaptation. Full document value paths remain pending-human. P-207/P-208 are dependencies, not authority.
- **Contract boundary:** No foreign-world editing; H-253 rejects conflicting interpretations. Persistent identity/layout, general composition/namespace and physical RTTI remain unselected. H-387 preserves source-world isolation/owners; H-400–H-406/H-407/D-26 replaces the known-ADT-only input restriction without authorizing new executable methods.
- **Invariants / non-goals:** A module constructs its own world; it cannot edit another world at either stage. Source selection reads published information and constructs receiver-local information. Symbol identity, type identity, a module-world copy and a referenced value are distinct. TLS is a flat typed-binding collection, not another type/method catalog or permission to edit a source world. Preserve capture and regional lifetime/ownership contracts. No cross-module world editor, editor-target effect, live method-table mutation protocol, public pointer ABI or transaction scheme is introduced.
- **Acceptance:** Validate receiver-owned identity, explicit publication, source-world isolation and typed TLS without foreign-world edits. General composition and persistent representation remain separate review scopes. Earlier compiler coverage is recorded in the [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## The two sets of entities

A module world is a module's own semantic runtime environment, containing registered symbol objects, types, callable contracts, methods and related semantic information. Runtime values and module initialization state have their own construction events. It is more than a source-name or declaration index. Different modules have independent world copies and explicitly select what to publish and integrate.

A TLS world is the thread's **flattened set of typed symbolic bindings**. Each symbol has its own identity and required type identity. Independent bits payloads are stored directly in TLS; H-482 separately admits explicit ref/ptr symbols borrowing scalar targets from region-symbol-backed storage. H-561's explicit owner-parameter references also borrow lexical or ordinary regional targets without installing an owner symbol. Other region-selector/group forms keep their resource constraints. Scoped rebinding may select another valid target/owner; all forms retain actual-owner lifetime checks. Values may originate from different module-world copies without importing those worlds into TLS. Region resources retain their ownership contracts. A handler activation retains its required symbolic bindings.

These worlds do not model the same set of entities. The relevant connection is **typed reference to a value**, not a pair of matching world records representing that value.

## Module-world inventory

| Entity or information | What belongs to the module's world | What must stay distinct |
| --- | --- | --- |
| Type | Type identity and its world-local type information: abstract relationships, data representation and other admitted type semantics. | Type identity versus the address of one copy of its type information; a type versus a particular value of that type. |
| Function/procedure | A callable identity’s declarations, methods and selected interpretation. | A family, one method, a materialized dispatcher value and an invocation. |
| Overloaded method | Signature, body, origin and membership in the callable's available method set. | A world-local method set versus a process-global shared mutable method table. |
| Explicit type relation | Selected abstract inheritance and data-interface declarations. | Declared conformance versus inference from similar fields or a value's storage address. |
| Symbol object | Identity, declared TLS value contract and module-world semantics, including callable methods where applicable; aliases name this object. | Bare symbol lookup supplies the nominal identity at runtime. For a variable symbol, `@` reads its active TLS value. |
| UoM/conversion | Unit identities and the selected pure conversion definitions. | A unit type versus a particular measured amount/index value. |
| Module globals and initialization | Permitted global declarations/values and the initialization semantics that establish module resources. | Module state versus an automatic thread-local copy of every global; exact sharing/copying details cannot be inferred from a host compiler container. |
| Names, selection permissions and exports | Source-local access to the world's definitions, extension permission and explicit publication. | Aliases versus identity; local availability versus onward export. |

Types, functions/procedures and their methods are the core entities in H-234. The additional rows locate existing language facilities; they do not add new categories of heap object or public syntax.

A symbolic procedure definition registers or extends module-world method information during compilation; it does not install TLS or capture runtime bindings. Reaching an effect with a body installs a fresh anonymous family containing its written methods. Name-only activation installs the selected lexical family. Each installation retains its defining interpretation and required symbolic bindings.

Each receiver integrates exported information for identical identities in its own module world. The operation does not silently mutate the source worlds. Physical copying, sharing of immutable definitions and regional representation must preserve that independence; this inventory does not choose their implementation.

## Identity records, aliases and publication

Identity is the key of semantic information. A module can have several local aliases and several public aliases for that identity. Aliases provide access; they neither own separate copies of the method set nor redefine the object. Selecting more information for the same identity merges into the receiving world's record, so every alias of that identity sees the locally combined information (H-234/H-248).

Import selects a receiving implementation context for a source callable. That receiver records one imported identity per source callable identity; different aliases of that import share its information. Scanning may register local overloads before their import is encountered. When an explicitly imported callable alias identifies their callable, those declarations join that receiver record. A family made available only implicitly by type selection has the lower-priority lookup behavior below; a same-named local definition does not automatically join that family. A second imported alias does not create another interpretation context. Distinct source/extension identities and explicit same-alias unions retain their own identities. This is compiler construction metadata, not RTTI or a runtime identity layout.

If one module declares overloads through several local aliases of the same callable identity, the compiler warns with `[-Wmulti-alias-extension]` and identifies the aliases and declaration locations (H-252). All aliases still access the shared method information; the diagnostic makes that non-obvious consequence visible. Prefer one alias for declarations within a module. Merely selecting several aliases, or declaring several overloads through one alias, does not trigger this warning. `-Werror=multi-alias-extension` promotes this category to an error; `--warnings-as-errors` promotes all language warnings. The unit-conversion warning remains separately selectable.

An imported helper retains the source-to-receiver sequence of its selected implementation contexts. Each receiving interpretation considers inherited definitions and the source-related callable information selected in that world, by identity. Several aliases of one identity contribute its information once. A repeated view of a declaration supplies its selected interpretation only through the same identity or its explicit receiver import. Sibling identities remain distinct even when they share a source body, so nested helper interpretations cannot silently replace one another. A distinct overload definition remains a dispatch candidate. Ordinary typed dispatch must have a unique best candidate. In H-253's case, A's score(Sample)=7 and B's score(Sample)=11 conflict when B is selected while importing A's helper, so compilation rejects and identifies both definitions.

This candidate comparison does not merge the published identities or edit any source world. Direct calls through distinct aliases still select those identities, while `using` preserves the helper's existing interpretation. `import` adds the receiving interpretation without erasing inherited overloads. Explicit syntax for informed conflict arbitration is a future design possibility, not a current feature. Source globals, non-callable names, procedure captures and runtime data layout retain their separate contracts. No arbitrary choice by alias order, original ancestor or preferred source identity resolves a conflicting overload.

Export means **make an identity importable under a public alias** (H-247). For a symbol-identified callable it publishes the entire method set available in that module (H-374); no per-method private subset is hidden inside the published family. The complete family can have zero methods, including when a procedure symbol has only its declared procedure type. View compatibility constrains usable calls, not publication privacy. Public aliases are a separate mapping from local lookup names. An ordinary `export name;` publishes the identity selected by that local name. `export from "source.cod" name [as public_name];` instead publishes the explicitly selected upstream object without creating an own local alias or extension target; a private local namesake remains private. `export import` publishes the identities selected by the import, including admitted associated callables. A competing local name does not retarget that publication. Two public aliases may expose the same identity; a single public alias cannot denote two conflicting identities. Selection-as followed by export can publish that local alias; export from also permits an independently chosen public alias.

The compiler represents this distinction directly:

| Information | Authoritative record and access |
| --- | --- |
| Callable information | Receiver-owned identity records contain methods and origins. Explicit aliases and resolved source-fallback lookups identify those records; fallback registration need not copy every source name. A virtual union refers to its member identities in the receiver. |
| Type information | Definitions have compiler identities; local type names resolve to identity-bearing types. Selected conformance belongs to the receiving world. |
| Symbols | Local aliases resolve to symbol IDs; declared types belong to those IDs. Pending source selectors are scanning state, not runtime bindings. |
| Globals | Values are indexed by declaration identity; local aliases point to them. |
| Publication | Public alias maps to a typed identity: symbol, type, callable or global. Consumers resolve that identity directly instead of looking up the exporter’s local spelling again. |

Source selection receives a read-only source module. It reads the source's published identity and information, merges permitted information into the receiver, binds a local alias and optionally publishes that identity. Mutable receiver construction and runtime TLS changes remain separate. Shared immutable definitions are compatible with independent worlds; shared mutable selection tables are not.

These operations give implementation helpers their purpose: declare an identity, select source information, merge a local record, bind an alias, combine a local union, or publish an identity. H-246/H-249 requires deriving such operations from the architectural contract and removing obsolete bookkeeping, rather than preserving helpers merely because earlier code used them. This is an engineering rule within confirmed semantics; it does not settle unrelated dispatch, ownership or syntax choices.

## Type selection and shadowable associated methods

Using/import of a type makes its public source-visible behavior available without selecting every operation separately. H-484 supplies the lookup model: register the selected source module as a fallback below the receiving module's own namespace, retaining its publication information and its own source-fallback relationships. If ordinary lexical/local lookup finds no binding, lookup may consult that source view. Symbol from establishes the same fallback for its source. This is a compile-time relationship between lookup environments, not a module-valued expression, runtime dictionary or global method registration.

Fallback resolves each visited module's own binding before following that module's fallback relationships. Downstream lookup first consults that module's explicit public alias at the spelling; without one, an unexported own binding is a shadowing barrier, not an absent name. Public aliases include explicit re-exports. It can therefore find public names unrelated to the initially selected type; it is not a recursive traversal of that type's fields or method signatures. A resolved callable name denotes its published callable identity and available family, rather than a permanently type-filtered name binding. H-225's signature association identifies related callables where required, including H-233's associated publication; it does not limit which public source names fallback can find. Receiver-local information still merges by identity under H-234/H-248. Do not expose private names or search arbitrary loaded modules.

A direct local definition at the same spelling shadows the implicitly brought name after either using or import of a type. It creates/selects the local definition's own identity according to ordinary declaration rules; it neither collides merely with that implicit name nor automatically extends its family. Name lookup selects the binding before argument dispatch. The hidden family is not a fallback when the local binding lacks a matching overload. Explicit callable aliases also take priority; independent explicitly declared names retain their ordinary collision rules.

```cod
# model.cod exports Node and score; score(Node in O) returns 1.
import "./model.cod" Node;              # score is available implicitly.
func score(x:Node in O) -> u64 where {owner O} {
    return 7u64;                     # Local family shadows implicit score.
}
proc use(x:Node in O) -> u64 where {owner O} {
    return score(x);                  # 7; same rule after using Node.
}
```

A dotted named call can reach this namespace after the receiver and its declared abstractions have supplied no applicable member. The call’s selected interpretation then resolves the free name once. For instance, a local `quote` that shadows an associated callable is the free family considered for `x.quote()`. A rename preserves the chosen callable’s identity. [Named member fallback and its diagnostic](DomainLanguageDesign.md#when-a-dotted-call-uses-a-free-callable)

To extend a source family, explicitly select that callable with import under the intended local alias, then use the established extension rules. Importing a type still grants its existing type-level permissions; this name rule does not remove permitted explicit conformance declarations. Explicit callable import, using's preserved implementation context, import's receiving interpretation, mixin body redefinition and explicit same-alias unions remain separate operations. Local shadowing alone is not explicit overload-conflict arbitration inside a composed family.

### Merging type-owned method families

When several `using` or `import` declarations select views of the same nominal data type, the receiving module merges the published contributions by family identity for its constructors, data call operator and the indexing operations it provides. All names for that type in the receiving world see the merged information. A definition reached by multiple paths with the same interpretation contributes once. Compatible overloads coexist. Conflicting contributions reject the module composition even when no expression uses the disputed method. The diagnostic identifies both definitions. Import order gives neither definition precedence. A separate world retains its own selections.

The constructor overload family belongs to the nominal type. Its dctors are methods of that family. For a sum, `export Receipt;` publishes every dctor and its visible initializer overloads. A constructor selector makes the named dctors explicit, and importing a dctor permits extending it. In the merged family, the overloads for the dctor named at the `make` site determine whether braced initialization is available. See the [constructor selectors and combined-world example](DomainLanguageDesign.md#publishing-and-selecting-data-constructors).

For the three indexing operations, selecting a type view likewise brings its published receiver families; an explicit import of the receiver permits an OO extension. Distinct ADTs can each supply their own variadic indexed value transfer in the same world. Selecting multiple views of one receiver applies the existing contribution identity, conflict and visibility rules. A declared conformer receives visible abstract indexing defaults. An unprefixed operator declaration belongs to the ordinary local or explicitly imported free family. Indexing can reach that family after both OO levels lack an applicable implementation. [Dispatch order and a two-world test](CollectionsAndBuiltinContainers.md#abstract-defaults-and-free-indexing-operators)

### Publication of type-carried call operators

Exporting a nominal data type also publishes the `(())` family visible on that type in the exporting world. Re-export of a selected type view carries its family. Plain `using` selects the family with the type. An `import` of the type also permits local extension. A standalone `export`, `using` or `import` of `(())` is invalid. Calls use [type-level lookup](DomainLanguageDesign.md#type-level-call-operator-lookup).

```cod
# model.cod
data Threshold { minimum:u64; }
export Threshold;

# checks.cod
import "./model.cod" Threshold;
func (const Threshold in O).(())(n:u64) -> bool where {const owner O} {
    n >= self.minimum
}
export Threshold;

# limits.cod
import "./model.cod" Threshold;
func (const Threshold in O).(())() -> u64 where {const owner O} {
    self.minimum
}
export Threshold;

# app.cod
using  "./checks.cod" Threshold;
import "./limits.cod" Threshold;
func at_limit(t:const Threshold in O) -> bool where {const owner O} {
    t(t())
}
```

The inner `t()` reads the minimum through the method from limits. The outer call uses the predicate from checks and returns true. If limits also independently defines the same `(Threshold, u64)` method as checks, app rejects the two definitions when it combines the views. If limits instead re-exports the checks definition, both paths lead to a single method. Within a valid combined family, ordinary method dispatch selects the unique best applicable overload for each call.

For `using "./model.cod" Receipt {Accepted};`, the constructor list explicitly selects the Accepted dctor with all its initializer overloads. Selecting Receipt itself carries the whole visible call-operator family. The [`with methods` clause](#explicit-associated-callables-with-with-methods) governs separately named associated callables such as `score`.

### Types carried by symbol selection

Selecting a symbol with `symbol from` retains its declared type information and registers its source for fallback under the same publication and shadowing rules. The explicitly selected symbol retains its declared nominal type identity, regardless of local aliases or later shadows. Public type/method names found through that source remain implicit; directly selecting a type remains an explicit selection.

```cod
# vocabulary.cod exports ticket, its declared type Ticket and Ticket's id method.
symbol from "./vocabulary.cod" @ticket;
# Ticket and id are implicitly available; no TLS value was installed.
data Ticket { code:u64; };
# This local Ticket shadows the implicit type name.
# The declared type of @ticket is still vocabulary.cod's original type identity.
```

This does not redefine the selected symbol, retag values, install an effect, construct a callable view or capture dynamic state. Private dependencies can remain required semantic information without becoming selectable local names. Only published resources gain imported names through this rule. Imported symbol aliases retain identity and share their existing TLS slot; name shadowing cannot replace its declared contract. H-481's definition/view materialization and H-482's reference-symbol owner rules remain unchanged.

### Source fallback, proximity and publication

The initial policy uses a conservative priority relation:

- Ordinary lexical/local bindings and explicit selections take priority over fallback. Finding an unsuitable binding is still finding a binding: no deeper search is triggered by a wrong kind, missing overload or failed type check.
- Source views registered by peer explicit selections have equal priority. In a simple chain, each additional fallback edge is farther from the receiving selection; count lookup relationships, not the original definition's provenance or type/field depth. Distinct candidates at that level are ambiguous when the name is demanded; source order, module spelling and argument types do not break the tie. A local or explicit binding can shadow an unused fallback conflict.
- Prefer a candidate when its proximity to the explicit selection is clearly established. If paths cannot be ordered without inventing a semantic-distance rule, reject the demanded lookup and request explicit using/import. No arbitrary shortest-path, field-depth or inheritance-distance metric is selected. More precise proven proximity may be supported later.
- Repeated paths can collapse only when they select equivalent identities and interpretations. Shared origin alone does not erase a distinction between source-preserving using and receiver-adapted import. If those contexts compete without an established priority, require explicit selection.

Lookup edges retain their selection context and publication/shadowing evidence. Using preserves source interpretation; import retains its established receiving interpretation; symbol from preserves the selected identity/source context. Discoverability does not grant extension permission for every name in the source: explicitly importing the callable, by name or through with methods, selects the family to extend. A local shadow does not substitute itself into a source helper merely by spelling.

A source's own fallback relationships can be followed recursively when it has no own binding for the demanded spelling. Only a public terminal binding may be used downstream. H-487 makes an intermediate unexported binding a barrier when no public alias at that spelling is provided. H-488 permits an explicit re-export to supply that public alias with a different upstream identity, leaving the local binding private; this is an explicit publication decision, not a skipped private shadow. Following a dependency path does not put that dependency's aliases into the intermediate module's explicit export map. Thus implicit lookup can reach core's public helper through facade, while `using "./facade.cod" helper` still requires facade to explicitly export helper. Source worlds remain read-only.

Ordinary export publishes its resolved identity; export import retains its existing explicit selected/associated publication contract rather than becoming a wildcard export of every reachable namespace. A cached implicit resolution or receiver identity record is not an own declaration/alias and cannot create a new private shadowing barrier.

```cod
# vocabulary.cod exports packet, Packet, Detail and detail_code.
symbol from "./vocabulary.cod" @packet;
# Packet, Detail and detail_code can be found in the source's public view,
# without traversing Packet's fields. No TLS value is installed.

# a.cod and b.cod each export their selected type and a distinct render.
using "./a.cod" Node;
using "./b.cod" Packet;
# A demanded render(...) is ambiguous: these source views are peers.
using "./a.cod" render as render;
# render now resolves explicitly; dispatch then uses that selected family.
```

### Identity, visibility and acceptance

Repeated selection paths preserve identities and do not duplicate implementations. Different implicit families sharing a spelling do not become one family merely by name. Export publishes selected identities according to the existing explicit-publication rules, not whichever later spelling happens to hide them. No module-qualified access syntax or automatic re-export is added.

Source-preserved helper bodies and already selected callable values retain their established definitions/contexts and captures. H-253 still rejects distinct equally applicable definitions admitted to an import-adapted helper; lookup shadowing cannot silently arbitrate that conflict. A new local family shadowing an implicit name has its own identity; spelling alone cannot make it replace a helper's source dependency. Existing explicit family adaptation remains valid. Connecting an imported family through an explicit callable import permits its ordinary extension rules; competing applicable implementations still obey H-253. Selection and declaration registration are compile-time operations; this rule does not hoist runtime initialization or define a live name-replacement event.

Acceptance must cover using/import type selection, result-only/inherited associations, public-source filtering, unrelated public-name fallback, local shadowing after both reuse forms, no typed fallback to a hidden family, explicit callable extension, repeated-path identity, demanded peer-source/context ambiguity, multilevel public-name lookup, private shadow barriers, explicit publication, symbol-carried type/method names and unchanged symbol type identity after local type shadowing. Preserve source-world independence, selected view/capture semantics, runtime initialization and fraction/UoM compatibility. These are design acceptance obligations, not a claim that current source composition supports the complete surface.

### Follow-up decisions for type-carried names

Identity merging and helper interpretation remain independent of namespace traversal. Resolving a public callable through fallback accesses its published family in the selected context; additional selections of the same receiver identity merge information for all its aliases. A local shadow creates/selects its own identity. Explicitly importing the source callable provides an extension target, subject to H-232 adaptation and H-253 conflicts.

```cod
# base.cod exports Scorable, Node, score and twice.
# Node conforms to Scorable. The base score is deliberately broader.
func score(x:Scorable in O) -> u64 where {owner O} { return 1u64; }
func twice(x:Node in O) -> u64 where {owner O} {
    return score(x) + score(x);
}

# receiver.cod
import "./base.cod" Node;
import "./base.cod" twice;
func score(x:Node in O) -> u64 where {owner O} { return 7u64; }
# For a Node n: local score(n) is 7; imported twice(n) is 2.
# The local family has no source-identity connection merely from its spelling.
```

In a separate receiver, explicitly importing base's score under the local declaration alias makes score(Node)=7 an extension of that imported family. It is more specific than score(Scorable)=1, so the established import-adapted twice uses it and returns 14. Equal-specificity conflicts still reject. These are consequences of existing identity/context contracts, not new source implementation evidence.

Source fallback avoids a recursive dependent-type-name closure algorithm. More elaborate proximity proofs, source-graph representation/caching and complete source implementation remain engineering work. They may not resolve an unclear ordering arbitrarily or weaken publication, interpretation, initialization or ownership checks. Existing signature-association information remains relevant to its separately specified uses, including associated publication.

### Multilevel fallback and private shadow barriers

H-486 requests multilevel source fallback; H-487/H-488 establishes its public/private boundary. Inside a module, ordinary local lookup retains its own binding. For a downstream implicit lookup at each visited module:

1. If the module explicitly publishes the requested alias, use that public identity and context, even if its private local binding at the same spelling is different.
2. Otherwise, an own binding is private and blocks deeper fallback. Diagnose the module/declaration and require an explicit same-named public choice there: publish the local object, or explicitly re-export an upstream object while retaining the local private binding.
3. Only when neither an own binding nor a public alias settles the name may lookup continue through that module's source fallbacks.

The check uses published identity directly, without resolving the module's private local spelling again. Source-body lookup still uses its private bindings normally.

```cod
# core.cod
func answer() -> u64 { return 1u64; }
export answer;
# Core is also exported.

# facade.cod
using "./core.cod" Core;
func answer() -> u64 { return 2u64; }  # Private; shadows core's answer.
# Facade is exported, but answer is not.

# app.cod
using "./facade.cod" Facade;
val n:u64 <- answer();
# Error: facade's private answer blocks implicit lookup.
# facade may add export answer; to publish its local answer (result 2),
# or export from "./core.cod" answer; to publish core's answer (result 1).
# Without an explicit public choice, this call must fail.
```

If facade has no own answer binding, app can find core's public answer and the result is 1. If facade explicitly selects `using "./core.cod" answer;` without exporting it, that private alias is also an own binding and blocks downstream implicit lookup unless an explicit public alias resolves the name, even though it identifies the same underlying definition. The rule concerns the selected name's publication, not merely the original object's export status. A local declaration of another kind also blocks the name; wrong kind or an inapplicable overload cannot restart fallback. Explicit names in app retain their higher priority and can choose a public resource directly.

The following consequences preserve deterministic lookup without a richer arbitration mechanism:

- **Nearest permitted lookup tier:** for ordinary acyclic chains, an own binding is nearer than any fallback beneath it; a direct source binding is nearer than one reached through an extra source edge. An explicit re-export belongs to the exporting module's own public bindings, regardless of the definition's older origin. Distinct peer bindings, including conflicting interpretation paths, require explicit selection. General contextual orderings that cannot be proved remain rejected.
- **Private barriers participate in lookup:** do not discard an inaccessible own binding as though the name were absent. Without an explicit public alias at that spelling, a winning private barrier is an error; a private barrier competing at the same priority cannot be silently removed in favor of a public peer. A strictly higher-priority explicit/public binding may already settle lookup without traversing the blocked lower path.
- **Diamond paths:** repeated paths to the same selected identity and equivalent interpretation do not duplicate a candidate or initialize a module again. Same definition with observably different using/import contexts is not equivalent merely because the source body is shared. Preserve the selected context sequence; unresolved equivalence or competing contexts reject.
- **Definitions keep their context:** app-level shadowing controls app's mention. It does not re-resolve a helper body's lexical names at app or turn an independent family into an imported helper dependency. Existing explicit adaptation and H-253 conflict checks continue.
- **Checking order cannot change names:** implicit lookup results may be cached, and selected information may merge by identity, but neither operation declares a private alias. A source body mentioning a fallback name does not newly block downstream access to it. Visibility comes from declarations/publication and the lookup graph, not which body was checked first.
- **Graph traversal does not activate modules:** retain existing once-only initialization and owner/capture rules. H-492 separately admits declaration-only module cycles; lookup traversal must terminate and retain public/private/identity distinctions. This does not itself authorize cyclic runtime initialization; the [ordered postponed initialization](#ordered-top-level-postpon-suffixes) contract requires statically proved readiness and defines its execution order. All private barriers and ambiguous winners remain compile-time diagnostics.

This permits an intermediate dependency change or a new nearer binding to change a downstream implicit resolution. That is a consequence of implicit lookup; H-485's future use-site information can expose the path, and explicit source selection can state the intended dependency. No stability against arbitrary dependency edits is promised. Acceptance includes all four facade cases (absent, private without publication, local public, upstream public alongside local private), private aliases of the same original identity, peer barriers, diamond contexts, declaration-kind shadows, check-order independence and explicit-export/implicit-reachability distinction. Full graph representation, general distance proofs and source implementation remain separate; these rules add no runtime pointer or method-table protocol.

### Resolving source module paths

Classify the decoded path by its spelling **before** filesystem normalization. A leading `./` requests a file relative to the directory of the source module containing the selection. Only this form gets that interpretation. A relative miss is an error at that dependency site. Other source module paths are logical namespace paths in the project's resolved package set, including the project's own package. Specify the complete namespace path even when both modules are in the same package. A bare filename requests that exact root namespace entry. A leading `../` or an absolute filesystem spelling cannot turn into an implicit relative or host-file import. This rule governs dependency edges. The launcher may still accept a filesystem path to an entry source.

| Source operand | Lookup |
| --- | --- |
| `"./string.cod"` | The sibling of this particular importing source. |
| `"billing/text/format.cod"` | The exact module path in the project package namespace, then the bundled library namespace on a miss. |
| `"string.cod"` | A project package's root entry of this name, otherwise the bundled module under the active compiler's toolchain root. |

Bootstrap package lookup uses the project root and its [hand-prepared dependency roots](CodProjectStructureAndPkgMgmt.md#directory-layout). Distinct eligible package modules claiming the same logical path are ambiguous. Directory enumeration and the invoking process’s working directory cannot give one package priority over another. CoD has no Java-style classpath. A failed read or check of a selected module reports that module's error. Only an absent package module selects the bundled fallback. The active compiler supplies its bundled library location from its toolchain root. The physical installation layout and the development-tree mapping are implementation concerns. Existing intrinsic resources with catalog identities such as `std:exceptions` retain their checked identity. A matching source spelling cannot grant an intrinsic capability.

When the package selection shadows a bundled module at the same logical path, use the package module and **warn** at lookup. Identify the requested path, selected package module and shadowed bundled module. An explicit `./` request only selects its local file. The warning follows directly from these resolver inputs in the seed and booter. Reaching the same module under two valid spellings preserves its identity and executes its initialization once under the existing module rules. Record the resolved dependency for reuse. A change in the selected package set or a new package module shadowing a previously used builtin must invalidate a stale selection.

### Default and complete public selections

A library author can mark existing public names as the default selection and can also mark a name at its definition:

```cod
# Public names already declared in string.cod
default { str, repr, (++) }

# In a different library
default export func version() -> u64 { 1u64 }
```

`default { ... }` names public entries under the module's exported spellings, including public re-exports. An unknown or private entry rejects. `default export` publishes a named definition under the ordinary export rules and adds that public entry to the same set. Marking one method selects its whole published overload family. Repeated marks for the same entry are idempotent. `default` is contextual in these module declarations.

| Consumer spelling | Explicit selection |
| --- | --- |
| `using "string.cod"`, `import "string.cod"` or `mixin "string.cod"` | Substitute the publisher's selector list for the omitted braces, then apply the written verb. A module with no defaults requires explicit selection and the diagnostic may suggest `{..}`. |
| `import "string.cod" {..}` or `using "string.cod" {..}` | All explicitly published entries of the selected module, including named re-exports. |
| `import "string.cod" { emitStr, emitRepr }` | The written entries, with ordinary binding, naming and extension rules. |
| `using "./palette.cod" Color {..}` | The members of the selected type under its existing rules. |

For a public enum `ASCII`, the module can write `default { ASCII { HT, LF, CR, SPACE, HASH } }`.
A consumer's `using "ASCII.cod";` then selects the same type and member bindings as
`using "ASCII.cod" ASCII { HT, LF, CR, SPACE, HASH };`. The complete member table remains
available through the type. A consumer without a lexical `ESC` binding can write `ESC:ASCII`
for that member. [Byte scanner example](../docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner) ·
[Enum lookup](TypedEnums.md#names-and-case-matching)

For example, a module can publish a symbolic variable and a symbolic procedure family under their respective kinds. The entries in its default list have the same spelling as the entries in a consumer’s list:

```cod
# hooks.cod, after the declarations
export var @sout
export proc @flush
default { var @sout, proc @flush }
```

For each of `using`, `import` and `mixin`, writing only `"./hooks.cod"` has exactly the meaning of writing `"./hooks.cod" { var @sout, proc @flush }` after that verb. Substitution is a source-level equivalence. The compiler can retain checked selectors with their published spellings, kinds, origins and order. Apply ordinary privacy, renaming, destination, access, extension and body-availability rules at the receiving directive. If the explicitly written list would fail for one item, the omitted form fails there too. Items are never silently dropped to make a verb applicable. This also governs generated entries from the complete public table where module `{..}` is admitted. The public table excludes private entries and implicit fallback names. Type-associated methods and nested type-member selectors retain their existing rules.

Use `var @name` for a symbolic variable resource and `proc @name` for a symbolic procedure resource in export, using, import and mixin selectors. The prefix states what is requested of the source. Match it to the published declaration and reject a missing or wrong kind. `var` in a selector declares no local place or writable access. It preserves the published storage and access contract, including a reference symbol’s owner role. `proc` selects the procedure family under the receiving verb. For both `using proc` and `import proc`, a free receiving name introduces the published symbol identity. An existing selection of that identity is compatible. Reject a conflicting identity even when the procedure signatures agree. Apply these checks at the local spelling selected by `as`, where present. `using` preserves the selected source interpretation. `import` brings the family into the receiving interpretation with its ordinary extension permission. `mixin` keeps its destination and body requirements. It does not extract a first-class procedure or perform a call. Bare `@name` remains the operand of the separate identity operation `symbol from "./hooks.cod" @name`. That operation can establish the typed identity before an `effect` installs anything. For a published variable, `using var` also selects that typed identity. For a published procedure, the `using proc` selector itself suffices.

For `var` under `using`, apply the [same identity selection as `symbol from`](#selecting-a-symbolic-variable). A missing dynamic installation cannot make either declaration fail.

A complete definition with an attached `export` already states the kind. For example, `export ref symbol @sout:Utf8StreamOut in O where {owner O}` publishes the same variable resource as the corresponding declaration followed by `export var @sout`. Likewise an already declared procedure symbol can be published by `export proc @flush`. A default mark on a definition contributes the same kind-qualified selector. Publication alone reads no active binding. Preserve explicit symbol identity selection for a caller that only wants to install a writer. Verify that omitted and explicit lists give the same result for a free procedure-import name, a conflicting symbol, a wrong kind, and a missing destination for a verb that requires one. A complete or default selection grants exactly what its expanded entries under that verb would grant.

A newline between a module path and an explicit selection is ordinary whitespace. Recognize a path-only directive at an unambiguous grammar boundary, with an explicit semicolon available for disambiguation. Preserve multiline named selectors and an ordinary identifier named `default` wherever the contextual declaration does not apply.

For acceptance, compile an external source containing `using "string.cod"` with a relocated toolchain. Exercise the default `str`, `repr` and `(++)` families, a public entry outside the default set, and independent access to a public symbol identity before installation. Check a namespaced same-package module, explicit sibling, package shadow and warning, ambiguous package claim, broken selected package module and absent fallback. Verify both compilers agree across working directories and reuse cached dependencies correctly after adding a shadowing module or changing a default. Migrate actual booter sibling imports and the repository-relative string-library imports alongside the resolver. Preserve intentional named selection, receiver-world extensions and re-exports in the formatter.

### Selecting a symbolic procedure family

A vocabulary can publish a procedure symbol before any module implements it:

```cod
# vocabulary.cod
symbol @p : proc() -> i64;
export proc @p;
```

```cod
# provider.cod
import "./vocabulary.cod" proc @p;
proc @p() -> i64 { return 7 }
```

The symbol declaration creates the empty family. Export publishes it under the symbol’s identity. In the provider, the single `import` both binds the vocabulary’s `@p` identity and grants permission to contribute the first method in the provider’s interpretation. Writing `symbol from "./vocabulary.cod" @p;` as well is valid. Both directives refer to the same symbol, and the import supplies the family and extension permission. Identity selection alone grants no extension. The vocabulary and an independent source-preserving `using` still see zero methods. Either can install a local body for that symbol without publishing a method. A `using` can select the empty family into a free name but grants no extension permission. A different preselected identity still conflicts under its verb’s rules. Export, import and using can all occur before a handler is installed. Written, default-expanded and complete selections apply the same per-item checks. A body mixin still requires a body. [Procedure declarations](SymbolicDynamicScope.md#procedure-families-before-the-first-method)

These excerpts belong to three separate modules. The tariff package publishes its methods under
the billing vocabulary's `@quote` identity.

```cod
# Billing engine calls the active tariff through behave @quote(shipment).
symbol from "./billing-vocabulary.cod" @quote
```

```cod
# Batch entry selects the complete carrier tariff for a scoped effect proc @quote installation.
using "./carrier-tariff.cod" proc @quote
```

```cod
# Refrigerated-shipping integration extends the published carrier family.
import "./carrier-tariff.cod" proc @quote
```

The batch entry can introduce a free `@quote` with the single `using` directive. Its `effect proc @quote` installs the family for downstream dynamic calls. Selection itself performs no installation. The refrigerated-shipping import introduces the tariff’s published symbol at a free name. An explicit identity selection from the billing vocabulary is also valid when it denotes that same symbol. Its contributed methods undergo the ordinary extension and overload checks. A module with only `using` cannot add that method. Prefer an ordinary named callable for a fixed calculation. [Programming guideline](../docs/IdiomaticsGuideline.md#selecting-a-symbolic-procedure-family) · [Worked example](../docs/IdiomaticCoD.md#keep-symbolic-procedure-families-at-integration-boundaries)

### Selecting a symbolic variable

These two alternative declarations have exactly the same meaning when the module exports `@sout` as a symbolic variable:

```cod
using "string.cod" { var @sout }
```

```cod
symbol from "string.cod" { @sout }
```

The singleton `symbol from "string.cod" @sout` has the same meaning. Braces group identity selections. The `var` spelling checks the published kind. Both select the same identity, declared type, owner role and access contract. The ordinary rules for visibility, local naming and conflicts, source-context fallback, and module dependency and initialization apply equally. Both declarations are valid before any stream is installed. Neither acquires the owner's release responsibility, captures a target, requires an incoming effect or adds an extension grant.

A use such as `writeUtf8(@sout, piece)` inside a procedure reads the currently installed target and its actual owner at argument evaluation. The checker must prove an installation with the required access and lifetime on each path reaching that read. A procedure can carry that requirement to its caller. A nested installation selects the inner writer until scope cleanup restores its predecessor. Existing operations that explicitly prepare an operand or capture an installed value keep their established evaluation points. Ordinary module initialization still runs under its own dependency and effect rules.

Verify an external module can select an uninstalled variable using either spelling, including a renamed local name. Install an outer writer, call a helper using the other spelling, shadow it in a nested scope and call the helper again after cleanup. Verify the outer, inner and restored targets on normal and caught-failure paths. A reachable read with no supplying installation must reject in either program. If a module's default list contains `var @sout`, the same checks follow by substituting the list under `using`. A wrong `var` kind, private source symbol or incompatible local name retains its ordinary declaration diagnostic.

### Public definitions and grouped selections

Write `export` immediately before a named, publishable module-local definition. Examples from the booter source vocabulary:

```cod
export array SourceBytes[u64]:u8;

export data SourcePosition {
    line:u64;
    column:u64;
}

using "./SourceStorage.cod" { SourceBytes, SourceLines, SourcePosition }
```

The prefix publishes the same local identity that a separate `export name;` would publish after the declaration. This includes named `data`, `array`, `func`, `proc`, `enum` and `handle type` declarations. It applies to a foreign-marked declaration with a separately publishable identity under its existing rules, including a local `data "C"` or `proc "C"` declaration. Marking one method publishes its complete locally resolved callable family. Likewise a public type carries its existing constructors, methods or member table according to that type's contract. A private definition sharing only a foreign object's spelling cannot publish that foreign object by accident. Use the [publication-only re-export](#publication-only-re-export) when the public identity comes from upstream.

The prefix applies to a declaration that has a single publishable name, including a named overload family. For `export symbol @s :own handle S;`, the published name is the symbol identity `@s`. Another module can name its identity with `symbol from "m" @s` before any resource is installed. Anonymous forms, expressions, initializer blocks and individual fields have no such publication. Split a declaration of multiple independent identities when their visibility differs, or select them using standalone exports. Prefixing a declaration adds no initialization, repeats no method body and confers no additional extension grant. Existing `export name;`, `export from` and `export import` retain their uses and privacy checks.

For several selections with one source and operation, place a comma-separated list **after the quoted module path**. Each item obeys the same source-name, destination-name and suffix grammar as a singleton of that operation. For example:

```cod
using "./SourceStorage.cod" { SourceBytes, SourcePosition as Position }
using "./Compiler.cod" { NativeError { Rejected, OpenFailed }, ExpressionScratchPlan }
```

The outer braces group public selections. The braces following NativeError select members *of that type*. The same distinction applies to sum constructors. Existing per-item `as`, `with methods` and a mixin's `with { ... }` symbol map attach only to their item, wherever that singleton already permits the combination. A parenthesized operator occupies one item. Commas inside a member selector and semicolons inside a symbol map belong to the nested production. An enumerated list requires at least one item and permits a trailing comma. The single-selection spelling remains idiomatic for one item.

The outer `}` completes an explicit module selection group. At end of file or before the next module item, the trailing semicolon is optional for `using`, `import`, `mixin` and grouped `export import`. The rule applies to every admitted item kind and to a module-level `{..}` wherever that verb admits it. For example, both directives here are complete:

```cod
using "string.cod" { var @sout }
symbol from "string.cod" { @sout }
```

A space in place of the newline yields the same two directives. The next item parses by its own grammar. A comma, item modifier or unmatched brace after the closed group cannot extend it. Inside the list, a newline cannot supply a missing comma or closing brace. An inner type-member selector's `}` closes only that selector. [Semicolon policy](SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux)

Apply grouped items in written order using the same source module and verb. A group can contain ordinary names and kind-qualified symbolic items, for example `using "./hooks.cod" { emit, var @sout, proc @flush }`. Each item has the same rules as its singleton. An identity-only dependency uses `symbol from`. The `using units` form keeps its own grammar. A grouped `export import` applies publication to each imported item. There is no group-wide post-brace modifier.

Repeated selections of the same identity/context, conflicting explicit names, private sources, adaptation, whole-family import and mixin mappings all receive the same outcome as their expanded source. A group neither resolves ambiguity by order nor initializes the source once per item. For migration, consolidate same-module items that express one purpose. Preserve semantically significant intervening declarations, selection order and business-meaningful separation. Identify an error at the offending item. Exercise both frontends on the migrated `applications/cod-booter/src` modules, including grouped lookup of sibling type names, private helpers, prefixed publication of an overloaded family, re-export under another name and generated C declarations. An unsupported underlying singleton retains its ordinary capability boundary.

### Publication-only re-export

H-491 selects:

```cod
# facade.cod; core.cod publicly exports answer.
func answer() -> u64 { return 2u64; }  # Private local object.
export from "./core.cod" answer;
# Local answer() still returns 2.
# Downstream answer resolves to core's exported object.

export from "./core.cod" answer as original_answer;
# A second public alias for that same upstream object.
```

Export from selects the source's explicitly published object and preserves its identity and selected interpretation. It adds a public alias, not an own local alias, extension target, method definition, callable capture or TLS installation. The source remains a normal construction dependency; source fallback can retain that upstream relationship without promoting the selected public alias into a local declaration. Existing once-only initialization rules apply. Local lookup still follows local bindings before fallback; it does not consult the publication map as a replacement for those bindings.

The forms express different operations: export name publishes an existing local resolution; export from publishes an explicit source object; export import performs its established receiving-context import and publication. Export from does not silently perform import adaptation. A pure re-export is not a new callable definition. Public aliases of distinct objects cannot collide merely because one was declared later; repeated identical identity/context publication is harmless. Private upstream names cannot be selected. Publishing a type does not itself publish every name in its source namespace; associated publication through export import remains its separately specified contract.

Acceptance distinguishes local answer=2 from downstream answer=1, verifies both public aliases and unchanged private status, rejects conflicting public identities/private source access, and preserves source interpretation and initialization across multiple re-export levels. This surface is planned source behavior; current compiler support is not claimed.

### Explicit associated callables with with methods

H-489/H-490 makes with methods an optional suffix on using/import/symbol-from selections of a type or a symbol with an associated value type:

```cod
using "./model.cod" Node with methods;
import "./model.cod" Node with methods;
symbol from "./vocabulary.cod" @node with methods;
```

These are separate selection examples. The clause explicitly brings in every associated **callable**, including its complete public method family. Signature association under H-225 identifies related callables; it never creates a callable containing only the matching methods. H-490 makes the selection unit general: using/import selects callables, not individual methods, whether the callable is named directly or selected through this clause. There is no source signature-selector or alias-specific method subset.

Resolve association within the admitted source view, respecting public identities, shadows, fallback priority and context. Do not scan callable bodies, recursively import every type in newly selected signatures, bypass a private barrier or filter overload candidates to disguise an ambiguous name. Repeated family identities deduplicate information. Public aliases for the associated callables retain their source spellings; renaming the initially selected type/symbol renames only that selection, not all its method names. Ordinary explicit aliases can be written separately when a bulk selection would conflict. No exclusion-list syntax is selected.

```cod
# model.cod exports Node and score, with both definitions:
func score(x:Node in O) -> u64 where {owner O} { return 1u64; }
func score(x:u64) -> u64 { return 2u64; }

# consumer.cod
using "./model.cod" Node with methods;
val n:u64 <- score(7u64);  # 2: the whole score callable was selected.
```

The explicit binding has the same lookup/permission status as selecting that callable by name. Using keeps its source context and grants no extension permission. Import supplies its established receiving context and extension permission; a local declaration through that explicitly selected alias participates in that family instead of creating H-483's independent implicit-name shadow. Symbol from retains identity/source-preserving behavior and supplies no callable extension grant. Initializing/capturing a symbolic value remains a separate operation.

Explicit names obey existing collision and intentional composition rules. The clause does not silently union unrelated same-spelled families or make an explicit conflict harmless because a name is unused. Without the clause, local definitions can shadow implicit associated names. With it, the same spelling is explicitly selected, so ordinary explicit-selection/declaration rules apply. Repeated equivalent selections remain valid; declarations through multiple aliases still follow H-252's diagnostic. If a bulk selection conflicts, omit the clause and select the desired callables/aliases individually.

A mention resolved through one of these explicit bindings no longer qualifies for H-485's implicit-use information; other names reached through fallback still do. With methods does not automatically re-export its local bindings: private explicit aliases can form H-487 barriers downstream. An explicit export decision remains necessary, subject to export import's existing selected-associated-publication contract. It does not grant unrelated type conformance, UoM conversion registration, intrinsic dispatch permission or new fraction semantics.

Acceptance covers complete-family selection, result-only/inherited associations, using/import/symbol-from modes, explicit priority, extension versus independent shadow, identical-path deduplication, private barriers, export separation, collisions and diagnostic provenance. Full source grammar/lowering and general graph/proof coverage remain implementation work; no executable support is claimed.

### Deferred information diagnostics for implicit lookup

H-485 reserves a later compiler/tooling capability: emit an **information-level diagnostic at a source mention whose successful resolution depends on implicit fallback**, recommending an explicit using/import selection. This is planned capability, not an implemented diagnostic or an immediate prerequisite for supporting fallback.

The diagnostic should identify the use site, resolved public name, actual defining module and selection path sufficiently to explain the dependency. Under H-488, a transparent re-export chain should recommend explicit selection from the actual defining module, using an available public alias and preserving any local renaming, rather than merely recommending the nearest forwarding module. Recommend an explicit selection preserving the resolved identity and using/import interpretation, with an alias when needed. Do not blindly replace a source-preserving selection with import or imply that the suggestion changes the symbol's declared type or installs TLS. An imported/adapted callable identity is distinct from an older source body; "actual defining module" must identify the selected semantic object, not discard its interpretation by jumping to that body's ancestor. An unsafe or unproved textual replacement must not be offered as an automatic fix.

For the example above, a use of detail_code can report: `information: detail_code resolved implicitly through vocabulary.cod; consider selecting it explicitly with using.` A use already covered by an equivalent explicit selection no longer depends on that fallback. Ambiguous lookup remains a rejection, rather than a successful implicit resolution accompanied only by information. Informational output does not change name priority, overload selection or program validity. Diagnostic category spelling, controls, repeated-use grouping and automatic fixes are deferred; no warning/error escalation policy is selected.

Future acceptance covers the actual mention location, correct source/identity/context provenance, a meaning-preserving explicit-selection suggestion, no implicit-use report after equivalent explicit selection, and unchanged ambiguity/privacy rules. Shared or specialized checking must not manufacture a new source mention for every generated instance. D-16 fraction and UoM identity/dispatch/storage rules are unaffected.

### Type-carried disjoint constraints and endpoint ancestry

[D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) owns H-527–H-535's n-ary constraint, warning, authority and independent-parent rejection rules. H-534 defines exemptions as already established ancestry among the listed endpoints, represented by the ordinary retained type graph. Export of any participating type publishes the complete record and its semantic ancestry information; using/import carries them automatically. This is type evidence, distinct from source-name introduction, with methods, or extension permission. Preserve it through aliases/re-export and applicable ancestor relations of selected subtypes. Referenced identities/relationship facts required to check a public record are public; unrelated private fields/bodies and private-name lookup retain their own contracts.

Retain each selected source world's established relation meaning while preparing the receiver. Combining constraints cannot newly grandfather a relation that an originating contract forbids. H-534 requires no copied path catalogue or historical whole-graph snapshot for each declaration. H-532 authoring authority is checked for new records; automatic consumption and unchanged re-export require no new grant. Validate the relevant relations/records before the fused world freezes. This metadata is not runtime initialization performed by postpon blocks.

### Shared disjoint information per module world

H-533 proposes one shared disjoint-information pool for each module world. Defined/imported type information refers to the receiving world's pool; using retains its selected source-world pool. This is a plausible sharing candidate, not yet a selected physical representation or an amendment of the existing identity-centered alias/interpretation rules. It can avoid per-type copies of the records while reusing the frozen type graph for H-534's endpoint ancestry.

| Type information selection | Candidate pool association |
| --- | --- |
| A locally defined type | The defining/receiving module world's shared pool. |
| An explicitly imported type | The receiving world's shared pool, retaining applicable upstream public records and their originating meaning. |
| A type selected through using | The selected source-world pool, without modifying or copying that source world. |

Keep the association on world-local type information or the selected interpretation, not a mutable process-global nominal type object. The same nominal identity can have information in different module worlds; changing its receiver association must not retarget source users or mint another nominal type merely to choose a pool. Existing same-world identity/alias merging and selected using/import interpretations constrain the implementation. A pointer spelling is not semantic type equality or authoring authority.

Four obligations remain even with one pool:

1. **Retain upstream facts.** Import into a receiver pool must not discard public disjoint records obtained from the source. Shared immutable references or an indexed union can serve this purpose; deep copying every record is not implied. A new receiver relationship cannot serve as its own proof that the source previously admitted it.
2. **Combine applicable checking contexts.** A local constraint can mention a using-selected A when another endpoint supplies H-532's authority. A still points to its source pool, yet the receiver's new constraint must govern its applicable local conformance checks. Looking only at A's pool would miss that rule. Reconcile relevant local and source records statically; this does not require a second runtime lookup.
3. **Filter publication.** Selecting one type carries its complete public n-ary records, not every unrelated private constraint stored in the same source pool. Retain record grouping/identity and publication/applicability information. Referenced semantic identities do not automatically become receiver aliases or expose private layouts/method bodies.
4. **Freeze and retain backing.** Build and validate pools with the static type graph, then share immutable information in the fused world. Retain valid world/record dependencies while they are used. H-534's existing endpoint ancestry comes from that graph; no separate exemption-path copy is required. Physical indexes/handles, cache layout and persistence protocol remain engineering scope subject to the ordinary world/region contracts.

For example, a receiver may use A, define local C, and declare type disjoint A, C. Keeping A's source association is compatible with that rule only if the receiver's checking environment also sees the local record. An imported alias and a source-preserved alias of one nominal identity cannot be used to evade an applicable local constraint. This preserves source independence without confusing a pool reference with the complete validation context.

**Acceptance:** many local/imported types sharing a pool; using without source mutation; imports retaining upstream public records; local constraints mentioning source-preserved types; aliases preserving identity and applicable rules; n-ary publication without unrelated private leakage; reuse of endpoint ancestry without path snapshots; stable frozen dependencies and ordinary lifetime/relocation constraints. No per-runtime-value pool pointer, new public ABI, arbitrary cross-world editor or runtime mutation is selected. The full representation/value path and finite generic/cyclic proof procedures remain pending; compiler support is absent.

## Vocabulary modules establish shared identities

A vocabulary defines symbols, types and their method collections so cooperating modules can use common identities from an explicit origin (H-242).

| Vocabulary content | Purpose |
| --- | --- |
| Typed symbols | Establish the symbolic identities used by the flattened TLS collection and their declared types. |
| Primarily abstract types | Establish shared semantic/interface identities across independently composed module worlds. |
| A small number of supporting ADTs | Supply concrete shared protocol/value types when the vocabulary needs them. |
| Associated callable/method collections | Establish shared callable identities, operation signatures and their relationships to the vocabulary's types. |
| Comments | Explain the intended meanings and contracts that identity alone cannot convey. |

For example, a vocabulary may define an abstract interface I, a small Request ADT, an operation on Request, and a request symbol whose declared type is Request. Modules A and B select those declarations from the same resolved vocabulary. Their world copies retain the common type/symbol/callable identities, while keeping their own integrated information. The TLS request entry may refer to a compatible Request value supplied by either world.

“Global unified identity” means agreement through the same defining vocabulary, not equality of names and not one shared mutable semantic world. Distinct resolved definitions or package versions remain distinguishable. Local aliases do not manufacture new identities; copying the vocabulary's information into a module world does not by itself redefine the vocabulary's declarations.

The ordinary export boundary still applies: a vocabulary explicitly publishes the definitions consumers may select. A receiver chooses its own onward exports. Vocabulary modules need no special module kind or new import syntax merely to fulfill this idiom.

A vocabulary can publish a callable with typed signature declarations and no implementations. H-257/H-258 separates its identity, declared contracts and executable methods. Each receiving world merges those distinct information sets by identity. Missing implementations matter when calls are demanded; importing the vocabulary does not fabricate a body or install a handler. [D-10](SymbolicDynamicScope.md#callable-declarations-and-methods) specifies generic/variadic signatures and the bounded calling rules.

## TLS-world inventory

The semantic entry is described schematically as:

```text
symbol identity S
    declared type identity T
    current bits payload or permitted reference, compatible with T
```

This is explanatory notation, not a public struct, boxed-value ABI or requirement for a runtime tag check. WPO may prove compatibility and resolve identities before execution. The contract still uses identities even when their checks have been discharged statically.

| TLS concern | Role |
| --- | --- |
| Symbol identity S | Selects the symbolic binding entry. Different module-local aliases of S refer to the same entry within the thread. Different symbol identities remain different entries even when their spellings match. |
| Declared type identity T | Constrains which values the entry may reference. T is not the symbol identity and is not replaced by a module-local type name or type-record address. |
| Current typed binding | Contains bits directly, or a permitted reference. Reference rebinding selects another target/actual owner when its declared selector or owner-parameter contract, type and lifetime checks hold. |
| Dynamic scope behavior | Effects shadow the current association and restore the preceding association on scope exit, following the binding/capture contracts. The flattened active collection does not require a dictionary or a separate TLS namespace per module. |

The [symbolic contract](SymbolicDynamicScope.md#symbolic-storage-and-regional-provenance) distinguishes inline bits, regional object references, whole-region resources and procedure bindings. Independent bits bindings require `effect @x <- rhs`; regional reference bindings, including explicit scalar ref/ptr symbols under H-482/H-561, use `effect @x = rhs`. These storage categories do not put type definitions, method sets or region contents into TLS. No raw symbolic assignment is permitted.

H-563 confines complete callable-view/environment storage to lexical scopes. Definition-installed handlers retain their family, interpretation and replay behavior; a binding record is not permission to put a callable view in a region or keep its borrowed owner alive.

The flat collection describes active entries. Saved values/pointers needed for scope restoration and procedure captures are supporting execution semantics, not a second catalog of module definitions. This does not prescribe a heap-allocated scope frame or runtime closure.

The pointer carrier alone does not redefine capture by value, bits transfer or resource ownership. Capturing a bits value must still preserve the required value; retaining a pointer to mutable storage is not automatically equivalent. Capturing a regional reference preserves its existing referent/lifetime relationship, without deep-copying the region. Representation must satisfy these existing semantics rather than replacing them.

## Values from different module-world copies

```text
Module world A                      Module world B
  type information with ID T         type information with ID T
  value a : T                        value b : T
            \                         /
             \                       /
TLS for one thread:  S :: T  -> a   # Reference with a declared selector or owner-parameter contract.
                     S :: T  -> b     after a scoped rebinding
```

The two values need not have the same value identity or address. Their originating module-world copies remain distinct. A is not merged into B, and neither world is imported into TLS by the rebinding. The symbol's type requirement is satisfied through type identity compatibility.

Conversely, a same-spelled type with a different identity is not interchangeable merely because its fields or representation look similar. The precise compatibility admitted for abstract/sum/conversion cases follows the type contracts; this inventory does not invent implicit conversions or erase nominal distinctions.

Type compatibility answers whether a value can be associated with S. It does not by itself choose a method set, redefine a callable's source context, grant ownership or extend the value's lifetime. Those are distinct obligations. In particular, origin in a different module world is not a reason to mint a new type identity or create a second per-module TLS entry.

Captured regional references preserve their referents: authorized referent edits remain observable, and capture does not deep-copy a region. Shared typed symbols and aliases retain their identity and scope-restoration requirements; concrete TLS representation is an implementation concern.

<a id="rtti-current-implementation-and-decision-boundary"></a>
## Actual-type dispatch and concrete-type evidence

The [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md) records earlier concrete-type checking and native coverage. A source metadata representation or fixture does not select a general runtime type-information facility, reflection API or persistent layout.

A region-root type identifier describes the stored root. Dispatch resolves the concrete target of the method call. [Compiler types](../cod/lang_syntax.hh) · [Method selection](../cod/lang_frontend.hh) · [Native object/TLS layout](../cod/lang_llvm.hh)

H-375 settles the behavioral requirement: family views dispatch on actual argument types, including abstract parameters/results. A selected Node/Edge result must retain enough evidence for a subsequent Entity-typed call to choose the appropriate method. A one-method or one-concrete-result implementation guard cannot define the language. Direct selection is permitted when equivalent for the selected family/context.

The fused artifact fixes the executable method definitions and interpretations prepared for execution, not every possible runtime type. Under [D-26](AheadOfTimeExecutionPreparation.md#generic-and-recursive-native-coverage), an outside-catalog type selects through CoD `<any>` without losing actual identity or required owner evidence. Valid uninterpreted passage does not add methods, edit a module world or require an unknown-type catch. An operation requiring unavailable interpretation raises checked `StaticallyUnknownTypeException`; same-T still uses actual evidence and breached established typing raises unchecked `UnsoundRuntimeTypingError`. Validation, access and lifetime remain required. This permits neither arbitrary opaque payload access nor executable method loading/runtime compilation.

The remaining H-251 boundary concerns concrete evidence/ABI, reflection and persistence, not whether dynamic dispatch is allowed. Type identity, selected world interpretation, value layout and actual owner remain separate. H-396 adopts actual concrete-type discriminators for abstract/polymorphic values and call-site facts that can narrow or omit selection. This does not require an object header or a second identity tag; physical transport and a permanent ABI remain separate. Root-type checksums are planned DBMR compatibility metadata, not dispatch discriminators. [AOT execution preparation](AheadOfTimeExecutionPreparation.md) records WPO/AOT coverage and that design backlog.

## Julia comparison and the CoD boundary

Julia provides the useful reference point of a runtime containing types, generic functions, methods and module bindings. Its current documentation describes global method/binding state and task-local world-age visibility; it does not describe independent semantic worlds owned by each module. [Julia world age](https://docs.julialang.org/en/v1/manual/worldage/) · [Julia functions and method tables](https://docs.julialang.org/en/v1/devdocs/functions/)

H-240 makes CoD's distinction explicit: **each module has its own dedicated world copy**. H-234 supplies explicit export/integration and local merging by identity. Julia's global world counter, method-table implementation, task world age and public namespace syntax are not selected by this comparison. CoD's TLS pointer set is not Julia's task-local world-age view.

This module-world independence belongs to the [Global Variables Done Right vision](GlobalVariablesDoneRight.md#two-complementary-worlds). It prevents selected semantic information from becoming one ambient mutable world shared by every module. The TLS side supplies explicit typed dynamic associations across those module boundaries.

## Callable families and explicit installation

The arms of a [named method group](SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch) contribute individual methods to the selected callable identity. The shared header describes their public call contract. Suppose one extension handles refrigerated shipments at any service level and another handles express service for any shipment. A client selecting both needs a unique winner for a refrigerated express request. An integrating module with extension permission can supply the method for that intersection. Reversing the order of selector directives leaves the result unchanged. A `using` of the callable family alone grants no extension permission. A locally installed `effect` group has only its written methods and must cover its procedure symbol when installed.

A declared symbol carries its identity and signature in the module world. Its bare name supplies that identity at runtime. Reject a declaration that would occupy or shadow its lexical name with a different meaning. For a procedure symbol, `perform` and `behave` select the installed handler, whose extraction as an ordinary procedure value rejects.

| Operation | Compilation/integration | Runtime |
| --- | --- | --- |
| Declare or extend a symbolic procedure | Register the method under existing module permissions. | Registration alone installs nothing. |
| An effect with a local method or a [method list](SymbolicDynamicScope.md#installing-a-local-family-with-several-methods) | Check the written family against the symbol contract. Its methods belong to this installation. | Snapshot its required bindings and install the complete anonymous family for the scope. |
| `effect proc @sym;` | Resolve the lexical family under the ordinary selection rules. | Activate it and install the scoped handler. |
| `@sym(args)` for a procedure symbol | Select the lexical family and apply ordinary argument dispatch. | Call under current dynamic bindings, even without an installation for this callee. |
| `@sym(args)` for a function-valued symbol | Check the ordinary call against the symbol’s function type. | Read the current function selection and call it with the arguments. |
| `@sym(args)` for a functor-valued symbol | Check the owner-qualified callable type. | Invoke the installed functor value on its bound receiver. |
| `effect @sym = other` or `effect @sym = @other` for procedure symbols | Reject assignment installation. | |

A family installation retains its selected interpretation and uses ordinary method dispatch. Export, using/import/mixin and extension remain module construction; none implicitly installs a receiver TLS value. Dependency initialization still follows its once-only incorporation contract and can execute its own explicit effects.

A skipped effect installs nothing. Reentering a body site creates a new activation of its local family. Whole-program compilation prepares dispatch for admitted actual types and value refinements. Independent module interpretations remain fixed for that composition and can coexist; a new explicit integration stage is separate from invocation.

Entry-module composition additionally follows H-379: extending a dependency callable requires an explicit entry installation, and that installation cancels dependency installations of the callable. Module definitions and explicitly retained source interpretations remain available. CoD must expose unused installation roots to LLVM elimination; the [cancellation contract](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) preserves initialization operand effects and all procedure-local installations. Once-only dependency initialization does not override this rule.

## Storage and execution stages

A referenced object still needs a live region supplied by a region symbol. Regional graphs, packets and application state reside in their regions. Symbolic bits payloads reside directly in TLS; lexical bits retain lexical storage. TLS is not a non-bits allocation arena, and its thread lifetime does not extend the lifetime of a referenced object. Scoped rebinding and captured replay do not transfer or destroy hidden region owners.

Compilation determines the flattened TLS shape: the admitted symbolic identities, their declared type constraints and the binding layout used by generated code. Runtime effects change the bits payloads or permitted references occupying that layout and restore preceding bindings as scopes end. The compiler's analysis of those effects is not execution of a source-program TLS-world edit at compile time.

Cross-module module-world editing is impossible at compile time and runtime (H-245). A module constructs only its own world. Reading another module's exports and merging their information locally is receiver construction, not an edit of the source world. TLS binding to a compatible value from another copy does not grant a way to rewrite that copy's semantic information. The runtime use of referenced values retains the established memory-access, capture and ownership contracts.

## Packages, namespaces and entry selection

Use the following vocabulary across module selection, startup and [bootstrap package inputs](CodProjectStructureAndPkgMgmt.md#terminology):

| Term | Meaning |
| --- | --- |
| Package | The dependency-resolution and version-management unit. Internal source directories do not each become a dependency package. |
| Namespace / subnamespace | A named grouping of modules and nested namespaces, corresponding to an internal source-directory level. |
| Namespace segment | One name component in a namespace path. The whole prefix identifies the nested namespace. |
| Module | A semantic unit with declarations, a module world and any executable initialization. |
| Directory | The physical source-tree location; concrete source-root and filename mapping are toolchain concerns. |

The module namespace hierarchy locates modules. A module's member lookup resolves its definitions, aliases and source fallbacks. Its module world supplies semantic identities and interpretations. These roles remain distinct even when diagnostics use the general word namespace. Namespace ancestry alone creates no module owner, initialization body, implicit import, publication or member-lookup fallback. It does not merge different package identities/versions into one world or introduce Module.f expressions.

H-499 reserves the canonical local entry-module name `__main__`, with two leading and two trailing underscores. H-500 makes the CLI select its containing namespace, not the dependency package as a single executable unit. One package can contain several independently selectable entry namespaces:

```text
acme-tools/                         dependency package
  tools/server/__main__.cod         entry in namespace tools/server
  tools/client/__main__.cod         entry in namespace tools/client
  shared/protocol.cod               ordinary reusable module
```

This is an illustrative source layout, not selected CLI argument syntax. Selecting tools/server resolves that namespace's `__main__` and its actual dependency closure. It does not run `tools/client/__main__` or initialize every module in acme-tools. A library package/namespace need not contain an entry. Selecting a namespace without an entry, or without a unique package/namespace identity, rejects; the compiler must not guess among packages, versions or entries. Concrete package qualification, path separators, source-root mapping and CLI flags remain separate surface work.

An entry module is an execution root and cannot be selected or re-exported by another module. This restriction applies to using/import, symbol-from, mixin, export-from and indirect aliases/fallback; selecting its definitions by another route must not evade the restriction. Entry modules may select ordinary library modules. Put reusable definitions in ordinary modules rather than importing another `__main__`. The rule concerns source-module selection; it does not prohibit an entry from passing an otherwise valid runtime value or callback. Publishing a name inside an entry does not make that entry selectable.

## Module dependency cycles and initialization

### Static namespaces and the startup sequence

H-495–H-498 fixes the following order for the selected entry and its incorporated dependency closure:

1. Static definitions populate the runtime module namespaces before any initialization block executes. Establish declared identities, types, signatures, method definitions and selected public-name relationships in their module contexts.
2. Execute one compiler-selected linear sequence containing each dependency module's ordinary initialization prefix as one implicit postpon block, together with all explicit postpon blocks, including the entry module's.
3. Execute the selected entry module's ordinary executable prefix last, after successful completion of that sequence.

Static namespace population is semantic preparation of runtime module worlds, not merely a checker symbol table. It does not evaluate arbitrary global initializers, invoke functions/procedures, install dynamic effects, acquire user-owned regions or materialize callable captures. A definition is available before its dynamically initialized value is ready. H-481's definition/view distinction and H-482's installation-selected owners remain in force. No runtime dictionary ABI, user-visible module value or permission to mutate another world's definitions is selected.

For a dependency module A, the required order is A.prefix → A.postpon1 → A.postpon2. Another module's whole block, including its implicit prefix, can occur between these nodes when readiness permits. There is no requirement to finish every dependency prefix before executing any explicit postpon block. Empty declaration-only prefixes need no runtime instructions.

The entry is the prefix-order exception: its explicit blocks retain their mutual source order and participate in initialization; its ordinary executable prefix follows all initialization blocks. Thus an entry postpon block can use registered entry definitions, but cannot rely on a value produced only by the entry's later executable prefix:

```cod
# Selected __main__ module; ordinary forms still precede the suffix in source.
symbol @config:u64, @answer:u64;
assert @answer == 42u64;             # Ordinary entry prefix: executes last.
postpon { effect @config <- 41u64; }
postpon { effect @answer <- @config + 1u64; }
```

This runs config installation, answer installation, then the assertion. Putting the sole config installation in the ordinary entry prefix instead creates an unavailable prerequisite for the earlier postpon block and rejects. Move that installation to an appropriately ordered entry postpon block; do not hoist it silently.

The compiler completes ordering statically and emits a fixed native instruction/direct-call sequence. It supplies no runtime dependency graph, ready queue, retries or runtime choice among alternative schedules. Ordinary branches and calls inside a block remain ordinary code; they do not dynamically reorder initialization nodes. Optimization must preserve the required observable order, without mandating inlining or a particular physical placement of function bodies. If static proof is insufficient, reject the case; any proposed need for runtime scheduling is a separate design question.

### Declaration-only cycles

H-492 allows a cycle whose participating modules contain declarations, with no executable module initialization. Establish the component's declared identities, public aliases and independently available signatures before checking callable bodies. The bodies may refer to one another:

```cod
# even.cod
using "./odd.cod" odd;
func even(n:u64) -> bool {
    if n == 0u64 { return true; }
    return odd(n - 1u64);
}
export even;

# odd.cod
using "./even.cod" even;
func odd(n:u64) -> bool {
    if n == 0u64 { return false; }
    return even(n - 1u64);
}
export odd;
```

Registering these bodies does not invoke them, install effects or materialize captures. Module dependency cycles, callable recursion and runtime initialization dependencies are distinct graphs. A module cycle alone must not become a permanent source-level rejection of this declaration-only case. The current source compiler's acyclic discovery is an implementation limitation.

Ordinary declaration checks still apply: unique/public name resolution, finite and anchored type/callable identities, valid layouts and owner constraints, independent result evidence and valid selected interpretation. A ring of re-export aliases with no defining object does not invent an identity. Mutually embedded by-value types do not gain a finite layout merely because module cycles are allowed. Recursive type inference without an independent anchor still rejects. General cyclic import-adaptation/extension proofs are not supplied by the even/odd case; unsupported proofs must be diagnosed rather than silently collapsing contexts.

Declaration-only cycles can be reached from the selected entry and otherwise normally initialized ordinary modules. Entry unselectability remains in force. A cycle involving executable initialization is governed separately by the postpon contract: its initialization readiness graph must have a proved legal linear schedule. An actual readiness cycle still rejects. This module permission does not authorize cycles in the package manager's dependency/version-resolution graph.

### Here-callable module top levels

The executable top level of each incorporated module is a here callable, including that of the selected `__main__`. Its ordinary prefix and its explicit `postpon` blocks access the same module's live lexical slots. Scheduling another module's block between two fragments preserves both modules' distinct contexts. Static namespace registration precedes this execution. A procedure's symbolic snapshot is taken at its own definition activation. [Here-callable forms](CallableViewsAndCaptures.md#here-callables-and-direct-lexical-access)

```cod
# A dependency module: its ordinary prefix runs before its own postpon suffix.
val count <- 1u64;
postpon { count <- count + 1u64; }
postpon { assert count == 2u64; }
```

The entry module has the same lexical rule and its existing entry-last prefix order. Each read still needs an initialized slot and each dereference a live target owner. Module contexts remain available for successfully registered module-level deferred actions through the [application shutdown drain](#cross-module-shutdown-order). An ordinary callable declared at top level retains its own access contract.

### Ordered top-level postpon suffixes

H-493 proposes the exact spelling `postpon { ... }` at module top level. H-494 requires these blocks to form the module's trailing suffix: after the first postpon block, only further postpon blocks and non-executable trivia may follow. Ordinary declarations, exports and executable top-level statements belong before that suffix. Nested postpon is not part of this form.

The compiler must preserve each module's explicit block order. Other modules' whole blocks, including implicit dependency prefixes, may occur between them to meet statically established dependencies. Include each dependency's prefix-before-suffix edge and the entry-prefix-last boundary stated above. If no such linear schedule can be established, reject. An out-of-order variant would require separately designed syntax and demonstrated value; no spelling or permission is selected for it.

For example, with source-order constraints A1 → A2 and B1 → B2, and checked data requirements A1 → B1 → A2, a possible schedule is A1, B1, A2, B2. The compiler cannot emit A2 before A1 to make the graph schedulable. Multiple legal global orders need not themselves reject; each accepted order must respect the same per-module and readiness constraints.

```cod
symbol @x:u64, @y:u64;
postpon { effect @x <- 1u64; }
postpon { effect @y <- @x; }
postpon { effect @x <- 2u64; }
# Source order is mandatory: @y becomes 1; the final @x is 2.
```

Repeated writes are not automatically competing providers: in this example their order and the read's binding version are established by the one module's chain. A compiler must not connect every writer to every reader or postpone the middle read until after the final write. Cross-module cases still require the intended target/binding context and predecessor state to be proved.

The registration pass records declarations and fragment bodies without executing them. Build readiness dependencies for the fragments in the admitted module-incorporation closure. H-140's once-only incorporation and scan-versus-execution distinction continue: scanning an otherwise unincorporated module does not create a new execution root. Encounter order identifies incorporation/provenance; it cannot override the required static initialization order. Repeated aliases and diamonds reuse the same module context and nodes. Distinct resolved module/package identities remain distinct.

### Readiness dependencies, rather than textual mentions

A proposed dependency is an obligation that must be established before an operation can execute safely and with its selected meaning. Resolve the name to identity and interpretation first. Keep these cases distinct:

| Use in a fragment | Required evidence |
| --- | --- |
| Mention a declared type/callable or inspect an independently known type | Declaration/signature availability; no initializer dependency solely from this mention. |
| Read a scalar, dereference a target or use a current symbolic binding | The selected value/binding/target must have the necessary initialization, access and owner evidence. |
| Pass/store a reference identity, or test identity/nullness | The identity/binding must be available; do not invent a payload-read requirement where ordinary init/reference rules permit identity-only use. |
| Invoke a callable | Include its checked transitive requirements and normal-completion writes in the selected context, not just argument spellings. |
| Materialize a callable value | Include its actual capture and lifetime requirements at materialization; declaration availability does not supply captured runtime bindings. |
| Invoke an installed handler with `perform` | Preserve its captured bindings and owners; do not substitute the requirements of a direct/current-binding call. |

For example, a fragment calling `proc read_b() -> u64 { return @b; }` depends on the supplying initialization even when its own body never spells @b. Conversely, inspecting an independently declared return type does not invoke read_b. Used default expressions follow their selected origin/context and ordinary preparation phase; unused defaults and unselected methods are not execution dependencies.

Dependencies identify actual module/interpretation context and target/binding identity, not just a symbol's spelling. Reordering cannot make another module's unselected dynamic resource visible, reinterpret an offset under another owner, or grant cross-world edits. Aliases, indirect calls and selected methods must retain conservative dependency evidence. Unknown required relationships reject; a name scan that misses a helper's read is not sound initialization analysis.

A provider must establish the required state on every normal completion that permits consumers to execute. Conditional writes are insufficient unless the required fact is proved on all such paths. A path that exits with an uncaught failure need not initialize a consumer that will never run. Static branch proofs may remove impossible dependencies; runtime guesses do not choose a different schedule. Distinguish an actual dependency cycle, a missing provider and insufficient analysis in diagnostics.

### Scheduling granularity, scope and failure

The scheduling node is a whole implicit dependency prefix or explicit postpon block. Keep source order and ordinary evaluation/cleanup behavior inside it. Do not split a node or interleave its statements with another fragment to rescue a cycle:

```cod
symbol @a:u64, @b:u64, @c:u64;
postpon { effect @a <- 1u64; effect @c <- @b; }
postpon { effect @b <- @a; }
# Reject: the first block needs @b, but it must precede the second block.
```

The author can instead split the first block and write three blocks in the intended order: initialize @a, initialize @b, then initialize @c. The compiler cannot invent that statement-level interleaving. Likewise, a first block that reads @b followed by a later block that initially supplies @b rejects even if swapping them would work. Scheduling nodes are not transactions: ordinary effects within a running block can be observable immediately, and no allocation checkpoint or rollback is introduced.

H-495's mental model associates each block with its owning module context as its capture. Blocks of that module share access to the same evolving context, rather than receiving stale value snapshots before initialization. This is an internal execution-context association, not a new ordinary callable capture permission or a requirement to allocate a closure per block. Scheduling another module's block does not make its bindings ambient in this one.

H-319's braces already distinguish grouping from a full scope. Postpon's `{ ... }` is a delayed fragment of its owning module scope, not a fresh callable invocation or an implicit `{{ ... }}` scope. Preserve module lexical resolution, declared exports and its selected dynamic/resource context. Explicit nested full scopes retain their own storage, restoration and cleanup. H-166's process-lifetime module owners remain subject to actual ownership transfer; scheduling does not extend a borrowed owner's lifetime or capture another module's binding automatically. Deferred work registers when the fragment actually runs and belongs to the existing enclosing full scope.

Each scheduled fragment executes once when reached in the emitted initialization plan. An uncaught failure follows ordinary propagation/cleanup and stops the remaining plan on that failing path; the entry executable prefix is not reached. No retry, partial-success readiness, speculative execution or whole-plan rollback is supplied. A failure handled inside the block contributes only the normal-completion state actually proved. Topological schedulability proves an allowed order, not termination of arbitrary called code or freedom from runtime errors.

### Entry installation and remaining analysis boundaries

**Entry callable installations.** [D-21's entry-installation contract](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) still requires explicit installation when entry extends a dependency callable, and retains operands/effects/failures when affected dependency TLS writes are suppressed. Static definition preparation does not perform that installation. If a dependency initializer requires the entry's installed value, the entry installation belongs in a sufficiently early entry postpon block with valid capture dependencies. A future entry-prefix write cannot justify an earlier read or suppressed-write replacement. If the resulting requirements cycle, reject; neither implicit hoisting nor runtime scheduling repairs them. General identity/cancellation proofs remain D-21 work.

**Binding versions and proof coverage.** H-494 preserves the same-module repeated-write sequence; H-502 permits another module to observe intermediate state at a legal boundary between separate blocks. For the chosen static schedule, prove each block's incoming state and normal-completion changes using actual contexts, target/binding identities and aliases. Distinct legal schedules yielding different values do not by themselves create an ambiguity error or require every read to follow the last source write. A reference to one symbol identity in two retained module contexts is not automatically a shared provider. Unproved context/alias/initialization facts still reject; more precise recognition can be added without changing the established order.

**Independent ready fragments.** The compiler can choose a reproducible ready-node order, for example using the existing module encounter order as an implementation tie-break. This is not an extra source-level dependency or permission to change per-module order. Missing dependencies, unproved context/alias/initialization evidence and invalid resource contexts cannot be repaired by that tie-break. Different valid block-boundary states alone do not establish such a proof failure. Code needing a cross-module observable effect sequence must establish the corresponding ordering relationship; no inference of external business intent from ordinary mentions is promised.

### Binding visibility at block boundaries

H-502 makes block membership decisive. The scheduler executes each initialization block as a whole; its normal-completion state is available to later scheduled blocks. There is no additional barrier requiring a consumer to wait for all later writes to that binding in its source module.

```cod
# a.cod: both writes are in one scheduling node.
symbol @x:u64;
export var @x
postpon {
    effect @x <- 1u64;
    effect @x <- 2u64;
}
# Other scheduled work cannot run between these writes.
# After normal completion, the source binding holds 2.
```

Splitting the writes exposes an interleaving boundary:

```cod
# a.cod
symbol @x:u64;
export var @x
postpon { effect @x <- 1u64; }  # A1
postpon { effect @x <- 2u64; }  # A2

# b.cod
using "./a.cod" var @x
symbol @seen:u64;
postpon { effect @seen <- @x; }
```

The selection of `var @x` in B names the identity and creates no scheduling requirement for a value. The read is the right-hand `@x` in B’s `postpon` block. If its execution context has the installation established by A1, it can observe 1 before A2 and 2 after A2 replaces it. The schedule and context must prove which binding actually reaches the read. Sharing a declaration across modules cannot establish that the binding reaches B. A value already read retains the ordinary rules for that value.

Source order A1 → A2 remains mandatory. Other established dependencies can force the consumer before or after A2; when both schedules are valid, the compiler may choose a reproducible permitted order under the ready-node rule above. This decision selects neither an earliest-reader nor a latest-writer priority. No special ambiguity rejection or separately named intermediate symbol is required merely because the writes occupy different blocks. Splitting a block deliberately permits additional interleaving; joining blocks removes those scheduling boundaries.

Whole-block scheduling is not a transaction. Ordinary calls and effects inside the block retain their sequencing and can observe its intermediate state; the compiler simply cannot insert another scheduled block between its statements. Failure retains prior allowed side effects and stops the remaining plan as already specified. No implicit value snapshot, publication buffer or allocation rollback is introduced.

Reuse checked entry requirements and normal-completion summaries by module interpretation/context and binding/target identity. Every read must have valid initialization and owner/access evidence along the chosen static schedule; hidden calls, aliases and cleanup completed inside a block contribute their actual effects. Process-exit defers do not supply startup readiness. Unknown indirect effects may conservatively reject, and D-10's conditional-effect restrictions remain. Summaries describe successive block states rather than imposing a last-write dependency on every external read.

Complete captured values remain independent: if A1 activates a handler with x=1 and A2 later installs x=2, invoking that handler with `perform` still replays 1. Mutable referents and actual owners retain their ordinary semantics, and declaration/type-only queries add no runtime-value edge. Acceptance distinguishes same-block writes from separate-block interleaving, preserved source order, source versus retained receiver/capture values, dependency-forced positions, hidden/aliased writes and block-completion cleanup. The visibility rule is settled; general proof coverage and source implementation remain work.

### Cross-module shutdown order

H-501 extends reverse registration order across the selected program's module-level deferred actions. Module scopes retain their contexts and required owners through the drain. For example:

```cod
# a.cod
symbol from "std:io" @print
postpon { defer { behave @print(1u64); } }  # A1
postpon { defer { behave @print(3u64); } }  # A2

# b.cod
symbol from "std:io" @print
postpon { defer { behave @print(2u64); } }  # B1
```

For the valid emitted startup order A1 → B1 → A2, shutdown runs the deferred actions in order A2 → B1 → A1, producing **3, 2, 1**. Use actual successful registration order, including multiple or dynamically repeated registrations within a block. Do not drain a whole module as one group or substitute source-file order. Each action executes with its retained module context; its current-binding versus captured-value behavior follows the ordinary defer/call rules.

The sequence includes module-level registrations in dependency prefixes, explicit postpon blocks and the selected entry's ordinary prefix. Ordinary nested full scopes keep their own exit behavior and do not contribute their private deferred actions to this module-level sequence. Entry module registrations join at their actual execution points; no separate entry-module cleanup priority overrides registration order.

Preparation must complete before an action is registered. Skipped paths and failed registrations contribute no action. Startup failure stops future initialization and the entry prefix, then unwinds only established work in reverse registration order. Owner transfer is not undone: transferred or already ended owners cannot be resurrected, and each remaining action must have valid owners through its actual invocation. Preserve exact intrinsic cleanup and the existing no-rollback/no-retry boundary.

Apply [D-17's FIFO deferred-error queue](ExceptionControl.md#exceptions-between-deferred-actions) to the global module list under H-503/H-505/H-507–H-509/H-541. An ordinary action executes independently; its attached catch handles only its own body. Escaping deferred failures join the queue tail. Checked failures are allowed there only with static coverage before the application-wide pass completes; a covering later-visited defer catch is user handling under H-550/H-551. A bodyless defer-catch entry processes only its incoming errors, oldest first, handling matches and retaining nonmatches. Handler outputs join the tail for later entries; optional finally executes once and any escaping failure joins after those outputs. A module boundary does not prevent handling, and each invocation retains its own module context and semantic error identities. Handler invocation occurs at its list position with its explicitly selected resources. All-matching forwarding preserves order; selective forwarding follows D-17's [A, B, C] → [B, A′, C′] rule.

Ordinary startup/body exceptions remain outside the deferred-error queue and cannot be handled by these entries. Startup-abort control state cannot be cleared by cleanup; later blocks and the entry prefix do not resume. D-17 requires UnhandledDeferError to retain all residual deferred exceptions, which H-550/H-551 constrains to unchecked members. H-538 additionally preserves a still-pending ordinary startup/body exception in the separate readonly body_exception field, without an automatic cause link; when no deferred failures remain, the original exception propagates unchanged. Residual exceptions retain FIFO order. A nested aggregate is one opaque unchecked error, without flattening or replay. The ordinary body/startup field may separately retain a checked exception; it is not a deferred member. [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) permits explicit full scopes and called helpers to own their cleanup inside module-level deferred execution; direct registrations into the draining module scope still reject, including from its attached catches/finally. Independent inner passes do not append to the outer sequence. If allocation for the complete aggregate fails after the module drain, [D-17’s publication rule](ExceptionControl.md#when-a-defer-pass-cannot-publish-its-aggregate) reports that failure. Module owners still receive their ordinary cleanup. D-17-RETURN-STATE permits explicit callable return replacement of an ordinary body error; module startup has no callable return target and remains aborted. Owner/effect restoration remains integration work. A physical shared action sequence/error queue is cleanup bookkeeping, not runtime startup scheduling, and grants no new exception payload ABI or owner lifetime. H-516/H-517's process OOM information window is separate runtime diagnostics, not a module world or ambient TLS binding. Eviction cannot change retained exception values, causes or module cleanup ownership.

Acceptance must cover 3/2/1, entry registrations, repeated/conditional successful registration, failed preparation, partial startup failure deferred actions borrowing still-live module resources, and cross-module FIFO handling/forwarding with once-only finally; checked coverage must remain valid on partial-startup and failed-registration paths. An implementation may optimize storage or statically known cleanup sequences only while preserving these observations. Binding visibility follows the separate block-boundary rule above.

### Implementation and acceptance boundary

The graph ordering algorithm is straightforward; producing sound context-sensitive readiness summaries is the substantial work. Reuse checked call, owner, capture, initialization and completion evidence rather than adding a separate textual-name analysis. A first implementation can admit directly established providers, simple references and known callable summaries, rejecting unproved alias, binding-version and indirect-call cases. No runtime scheduler, continuation splitting, general transactional state or automatic graph construction is needed for that subset.

Future acceptance must cover declaration-only mutual imports; rejection of ordinary forms after a postponed suffix starts and nested postpon forms; no-defining-object export cycles; unique identity/context through diamonds; successful interleaving of dependency prefixes and ordered suffixes; rejection when a dependency opposes per-module order; hidden helper and used-default dependencies; declaration/type-only mentions; capture versus invocation; conditional outputs; same-module successive binding versions and unproved cross-module providers; an unschedulable cycle with an explanatory dependency path; ordinary-order constraints; preserved module resource visibility/lifetimes; exact once-only execution; failure cleanup with retained prior side effects; and reproducible independent-node ordering. D-16 fraction validity, bits representation, UoM dispatch and the separation of explicit init flow from future pure knot-tying remain unchanged.

Startup/entry acceptance additionally requires static definition availability without premature dynamic-value readiness; shared evolving module contexts; entry postpon before entry prefix; rejection of a dependency on a later entry-prefix installation; correct H-379 installation availability; emitted fixed order with no runtime scheduler; a missing/ambiguous namespace entry diagnostic; two selectable entry namespaces in one package with only the selected closure initialized; rejection of entry reuse through every source-selection/publication route; and no entry execution after uncaught initialization failure.

The existing source compiler still discovers only acyclic modules and has no postpon parser/planner. Its CLI accepts --run FILE / --check FILE; namespace-selected `__main__` entry routing and the new static-namespace/startup contract are not implemented by this documentation change. No executable acceptance evidence is supplied. Full dependency-proof coverage, namespace representation and exact CLI grammar remain separate work; the stated ordering and namespace-selection decisions are settled.

## Architecture implementation boundaries

H-245's implementation goal covers independent module worlds and flat runtime TLS, including H-242's vocabulary identities. The requirement audit covers construction, composition, isolation, compile-time TLS layout and native binding behavior together. Its evidence is organized by those requirements, including rejection paths and source inspection; successful examples alone cannot establish world isolation.

The architecture separates three operations:

1. **Receiver-owned module construction:** read explicitly exported source information and merge information for identical identities in the receiver's own world. Local aliases provide access; distinct identities remain distinct. The operation never edits a source world and does not automatically re-export imported information.
2. **Compile-time TLS layout:** determine the flattened typed-binding collection and its declared type constraints, then check demanded paths and captures against that model.
3. **Runtime TLS editing:** change a symbol's current bits payload or permitted reference, potentially selecting a compatible value from another module-world copy, and restore the preceding binding on scope exit. Binding changes do not edit module-world definitions or method sets.

Canonical identity-based composition must follow this ownership boundary. Compiler-internal construction and call machinery stays internal. No cross-module world-edit primitive, TLS editor-target protocol or runtime method-table update mechanism is required or permitted by this goal. Native lowering may specialize statically known semantic information without inventing runtime world objects solely to host an editor.

## Implementation availability and open work

Earlier compiler support, implementation entry points and validation commands are recorded in the [archived source capability reference](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Conflict arbitration, extension of unrelated callable unions, general composition and persistent representation retain separate review scope. Declaration-only cycles follow H-492; initialization order and namespace-selected entry follow H-493–H-500, with dependency-proof and representation mechanisms still under review. No cross-module world editor is a pending deliverable.
