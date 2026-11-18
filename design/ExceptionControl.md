# CoD exception control and regional cleanup

Design revision: 93 · Recorded: 2026-11-18 · Status: confirmed bounded exception, lifecycle, scope, defer and return contracts; bounded full-scope elaboration profile confirmed; current implementation availability is recorded in the source capability reference.

## Design authority

- **Bounded compiler continuity:** [K-FULL-SCOPE-VALUE-BOUNDARY-20261118](../CHANGELOG.md#k-full-scope-value-boundary-20261118) implements ordinary callable elaboration for admitted independent full-scope inputs under the confirmed r93 contract/P-564–P-566, preserving C-63/C-65/C-73/C-75/C-84 and charter §§2/3.2–3.3/4.3/6.3/8/12. Local returns, independent result/capture-owner evidence, nonexecuting body-validation queries and delayed block timing reuse ordinary checking/native completion. The [source reference](../tests/cod/language/README.md#current-implementation-boundary) bounds acceptance and remaining work: immediate ref/ptr transport, inferred checked interfaces, wider result proofs and complete fixture migration remain incomplete. Semantic r93, qualification, all exclusions and D-16 are unchanged; this is implementation continuity, not complete profile acceptance.

- **Checked failures of double braces.** [H-DOUBLE-BRACE-THROWS-20261118](../CHANGELOG.md#h-double-brace-throws-20261118) confirms the [standalone checked interface](#checked-failures-of-standalone-full-scopes) in r93 and the [contextual callback rule](SymbolicDynamicScope.md#trailing-callback-arguments). D-17-FULL-SCOPE/P-564–P-566 and C-63/P-235/P-236 connect one inspectable call boundary to charter §§3.2/6.3/8. H-313/H-525 preserve declared obligations. The bounded r92 implementation profile carries forward with the standalone rule and acceptance cases below. [K-FULL-SCOPE-QUERY-20261118](../CHANGELOG.md#k-full-scope-query-20261118) applies its existing query checks to readonly writes and forbidden procedure effects. Wider callback lowering and deferred-pass mechanisms retain separate qualification.

- **Qualified full-scope implementation:** [H-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#h-full-scope-implementation-20261118)/[K-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#k-full-scope-implementation-20261118) confirms the complete bounded r92 [ordinary-callable elaboration profile](#full-scope-implementation-profile), including immediate ref/ptr proof and transport, under [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope)/P-564–P-566. Its confirmed C-63/C-65/C-73/C-75/C-84 → charter §§2/3.2–3.3/4.3/6.3/8/12 path connects one callable model to explicit input, owner, result and cleanup responsibility without lifetime extension. H-FULL-SCOPE-CALLABLE-20261117 remains the source baseline; H-BOUNDED-IMPLEMENTATION-ADOPTION-20261117 independently qualifies the stated dependencies. The profile’s constraints, acceptance, migration and exclusions below delimit delegation; current implementation availability is separate. General escaping borrowed views and new declared-place capture mechanisms retain D-18-BORROW’s pending scope. Semantic r92 is unchanged; this update records qualification.

- **Qualified bounded dependencies:** [H-BOUNDED-IMPLEMENTATION-ADOPTION-20261117](../CHANGELOG.md#h-bounded-implementation-adoption-20261117)/[K-BOUNDED-IMPLEMENTATION-ADOPTION-20261117](../CHANGELOG.md#k-bounded-implementation-adoption-20261117) qualifies [D-17-DEFER-CHECKS](DesignAuthorityRegistry.md#d-17-defer-checks) semantic r3/P-561–P-563 and D-18's ordinary value-input profile r114/P-485–P-487 within their stated acceptance/exclusions. This document's semantic revision, C-63/P-235/P-236 path and current source contracts are unchanged. The static checker preserves eager preparation, independent callable bodies and nonexecuting queries; qualification does not establish compiler support or qualify full-scope control lowering, FIFO/aggregate/checked-pass, module/generator or new borrowed capture mechanisms. D-16 remains unchanged.

- **Named region roots:** [H-595](../CHANGELOG.md#h-595)/[K-NAMED-ROOTS-20261117](../CHANGELOG.md#k-named-roots-20261117), [D-13-ROOT-ACCESS](DesignAuthorityRegistry.md#d-13-root-access)/P-546–P-548, aligns this revision's regional examples with named roots, ordinary `r.name` assignment destinations and receiving specifications; the source root intrinsic is removed. Reuse C-74/C-65/C-72 and charter §§2/3.1–3.3/4.3/6.3/8/12 for explicit storage, access and lifetime responsibility. D-13 owns acceptance; preserve existing qualifications, actual-owner checks, nominal component identity, D-16 and D-18's existing qualification boundaries. H-596–H-599/D-22-REGION-INIT separately extends the root initializer surface while preserving whole-region cleanup; complete initialization proof/native coverage remains pending. Full structural/receiving proof and compiler migration remain separate; no C++ storage API changes follow.

- **Native-coverage reference continuity:** [K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) preserves this revision's D-26 dependency at its current AOT contract section. Only dependency links and this summary change; existing semantics, authority, acceptance and D-16 remain unchanged.

- **Full scopes are ordinary callable invocations:** [H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117)/[K-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#k-full-scope-callable-20261117), [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope)/P-564–P-566, covers r89→90 in the explicit-full-scope interactions below. The user confirms ordinary capture, local return and independent cleanup everywhere; D-17 owns the source acceptance. Reuse C-63/C-65/C-73/C-75/C-84 and charter §§2/3.2–3.3/4.3/6.3/8/12 for one inspectable callable/lifetime model. Preserve actual owners, permissions, declared failures and D-16; no shared cells, lifetime extension or implicit value return. [H-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#h-full-scope-implementation-20261118) separately qualifies D-17’s bounded r92 full-scope profile, including immediate ref/ptr inputs; general escaping borrowed-view and declared-place capture mechanisms remain outside it. D-18's value-input profile retains its independent qualification. D-10 separately records the selected trailing callback surface; complete implementation/value-path qualification remains pending.

- **Return inspection and cancellation:** [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state)/[H-542–H-547](../CHANGELOG.md#h-542), plus [H-548](../CHANGELOG.md#h-548)/[K-RETURNING-PROFILE-20261116](../CHANGELOG.md#k-returning-profile-20261116), covers r85's bounded return-state profile and cleanup interactions. [H-549](../CHANGELOG.md#h-549)/[K-RETURNING-BINDING-20261116](../CHANGELOG.md#k-returning-binding-20261116) confirms one-way use of the established result type and clarifies existing materialization failures. Human-confirmed recovery/filter/owner directions and cost-delegated scoped cancellation continue; H-548 delegates the remaining simple, coherent choices. The selected profile uses explicit-return markers, ordinary declarators and existing bits/callable/reference binding lifetimes; owning results support inspection without capture. Candidate C-54/C-63/C-65/P-482–P-484 serves author-controlled completion, observable result state and valid storage under charter §§2/3.2–3.3/4.3/6.3/8/12. Acceptance is below. Preserve FIFO failures, declared checked contracts, access/owners, exact release and D-16. No ownership-consuming binding, general regional copy or new pure-callable contract; full value-path/physical qualification and source support remain separate. Agent selections under delegation are not additional human confirmations.

- **FIFO deferred failures:** [H-541](../CHANGELOG.md#h-541)/[K-DEFER-FIFO-20261116](../CHANGELOG.md#k-defer-fifo-20261116) covers r82's pending-error order and outgoing-error placement under [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch)/P-461–P-463. Deferred entries retain reverse registration order; errors escaping deferred execution join a FIFO queue. Each bodyless handler processes only its incoming entries, oldest first. All-matching forwarding preserves order; selective forwarding of A/C from [A, B, C] leaves [B, A′, C′]. This serves C-36/C-44/C-63/C-65 and charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit propagation order, stable full forwarding and valid resources. Acceptance includes forwarding, repeated returns, finally, residual order and module cleanup below. Preserve identity/trace, declared obligations, body_exception, actual owners and D-16; allocation/ABI, aggregate storage/body-field integration remains separate; H-552 rejects nested defer declarations; D-17-RETURN-STATE bounds owning-result inspection to the non-capturing form. This local confirmation does not qualify the complete value path or claim compiler support.

- **Return results during deferred handling:** [H-539/H-540](../CHANGELOG.md#h-539) confirms retained-result replacement and queued handling under D-17-DEFER-DISPATCH/P-461–P-463 and C-63/P-323/P-324. Later returns replace earlier return results while incoming handler invocations and remaining deferred entries continue; H-541 supplies their FIFO order. Preserve once-per-entry finally, unrelated pending errors and exact abandoned-result cleanup under charter §§2/3.3/4.3/6.3/12. D-17-RETURN-STATE owns explicit body-error replacement/cancellation; full queue implementation/qualification remains separate.

- **Retained body failure:** [H-538](../CHANGELOG.md#h-538)/[K-DEFER-BODY-EXCEPTION-20261116](../CHANGELOG.md#k-defer-body-exception-20261116) records r80's readonly body_exception field on the residual aggregate under D-17-DEFER-DISPATCH/P-461–P-463. Preserve the still-pending ordinary body failure separately from deferred members, without automatically establishing a cause. This serves C-36/C-63/C-65 and charter §§2/3.1–3.3/4.3/6.3/12 through retained failure identity and explicit provenance. The completion table below owns acceptance; D-17-RETURN-STATE owns explicit return replacement. Caller coverage and physical ownership/absence encoding remain separately scoped. The full queue/value path and runtime implementation are not qualified by this field decision.

- **Compiler continuity boundary:** [K-DECLARED-DEFER-SYNC-20261116](../CHANGELOG.md#k-declared-defer-sync-20261116) and K-755 record bounded ordinary compiler support. Current deferred source requirements are governed by H-550/H-551 and D-17-DEFER-DISPATCH; existing accepted source/validation does not establish the completed-pass rejection rule or new FIFO runtime. Unaffected ordinary contracts retain their qualification.

- **Compiler enforcement and payload evidence:** [K-755](../CHANGELOG.md#k-755) continues r77→78 under the qualified C-63/P-235/P-236/P-280 contract and H-525 rejection boundary. Declared root obligations are enforced independently of actual category facts; shared finite selection conditions preserve admitted boolean/exception payload proofs. Charter §§3.1/3.3/4.3/6.3/8/12, explicit typed intent, owner/access/initialization and D-16 remain unchanged. Acceptance and current proof limits are recorded in the [source capability reference](../tests/cod/language/README.md). This updates implementation status without qualifying new roots, ABI, narrower-catch coverage or revised deferred-queue policies.

- **Checked deferred coverage:** [H-550/H-551](../CHANGELOG.md#h-550)/[K-DEFER-CHECKED-COVERAGE-20261116](../CHANGELOG.md#k-defer-checked-coverage-20261116) records r86 in [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch)/P-461–P-463. Checked failures may enter the FIFO but must be statically covered before the owning pass completes; defer catch counts as handling, while an enclosing throws declaration alone does not. This serves C-36/C-44/C-63/C-65 and charter §§2/3.1–3.3/4.3/6.3/8/12 through explicit recoverability, observable order and no unchecked packaging of an unhandled checked deferred member. Acceptance and conservative proof limits are below. Preserve H-525 declared contracts, FIFO, ordinary body_exception, owners and D-16; full value-path/physical qualification and implementation remain separate. Prior declared deferred escape is not current implementation authority.

- **Declared-obligation enforcement boundary:** [H-524/H-525](../CHANGELOG.md#h-525) is a binding rejection criterion on the already qualified D-17 checking scope through C-63/P-235/P-236/P-280. [K-CHECKED-DECLARATION-BOUNDARY-20261116](../CHANGELOG.md#k-checked-declaration-boundary-20261116) continues that scope in r77: better actual-value evidence cannot waive a declared Exception obligation. This preserves explicit typed intent under charter §§2/3.1/3.3/6.3/12; it does not qualify D-17-ERROR-ROOTS's pending complete value path, new root implementation, payload ABI, transparent-wrapper coverage or deferred-queue integration. The bounded acceptance and exclusions are under Declared handling obligations below.

- **Disjoint declarations:** [D-10-DISJOINT](DesignAuthorityRegistry.md#d-10-disjoint), [H-527–H-535](../CHANGELOG.md#h-527) and [K-DISJOINT-ENDPOINTS-20261116](../CHANGELOG.md#k-disjoint-endpoints-20261116) cover this revision's constraint-related sections. Existing listed-endpoint ancestry permits inherited overlap; a second independent listed-parent relationship rejects, while prior redundant relationships warn about the path and intermediate methods. Preserve complete public records, per-pair definition/import authority, frozen validation, declared checkedness, demanded checking, owners/captures and D-16. Candidate C-56/C-10/P-476–P-478 serves explicit nominal intent and stable interpretation under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12. [D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) owns acceptance; finite generic/cyclic proof procedures, source implementation and full value-path qualification remain separate. No separate exemption-path store or runtime inheritance mutation.

- **Shared-information candidate:** [D-15-DISJOINT-POOL](DesignAuthorityRegistry.md#d-15-disjoint-pool)/P-479–P-481 under C-59/C-56/C-10 records [H-533's](../CHANGELOG.md#h-533) requested feasibility review. [D-15](ModuleWorlds.md#shared-disjoint-information-per-module-world) owns identity, upstream retention, applicable local/source checks, publication filtering and frozen-backing acceptance. Physical representation and the complete value path remain pending-human; no compiler support is inferred.

- **Exception roots and declared contracts:** [H-524–H-526](../CHANGELOG.md#h-524)/[K-ERROR-ROOTS-20261116](../CHANGELOG.md#k-error-roots-20261116), [D-17-ERROR-ROOTS](DesignAuthorityRegistry.md#d-17-error-roots), records r74's scoped addition under candidate C-56/C-63/C-65/P-473–P-475 and charter §§2/3.1/3.3/4.3/6.3/8/12. Error <: Exception enables user unchecked exceptions; explicit Exception declarations/throws remain checked despite narrower proven values, and adapters cannot change an established checkedness contract. Preserve H-523 wrapper agreement, H-522 identity, owners/captures and D-16; acceptance is below. D-10 owns the selected disjoint rule and its remaining proof boundaries; arbitrary payload representation and broader throws/inspection policies remain pending. This replaces the prior neutral-root interpretation; existing concrete classifications and unrelated qualifications continue.

- **Checkedness-preserving wrapper construction:** [H-523](../CHANGELOG.md#h-523)/[K-RETHROW-FROM-20261116](../CHANGELOG.md#k-rethrow-from-20261116), [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), records r73→74: standard wrappers require a cause of the same checkedness; mismatches reject with a suggestion to use rethrow from e. The new explicit-cause shorthand selects the standard wrapper automatically, retaining H-522's local identity normalization and creation-time trace. Candidate P-464–P-466/P-470–P-472 under C-36/C-63/C-65 and charter §§2/3.3/4.3/6.3/8/12 preserves inspectable provenance without changing handling obligations. Acceptance covers both wrapper categories, missing/unknown causes, aliases, explicit/default causes, once-only effects and actionable diagnostics. Ordinary business exception translation, owners/captures and D-16 continue; inspection, transparent throws summaries, physical identity and source support remain separate.

- **Transparent wrappers and retained tip:** [H-518–H-522](../CHANGELOG.md#h-518)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), [K-WRAPPER-CATCH-20261116](../CHANGELOG.md#k-wrapper-catch-20261116), records wrapper-prefix matching in source order and the preference to preserve the incoming wrapper in exception operations, including from e. H-521/H-522 selects a hidden tip reference alongside the matched binding and recognizes the same exception identity within that catch, including unchanged helper results, without copying the chain. Candidate P-470–P-472 under charter §§2/3.2–3.3/4.3/6.3/8/12 preserves typed payload access, immutable provenance and valid owners. H-523 settles canonical-wrapper admission below. Inspection, wider transport and checked summary relations remain under review; no general value ABI, ordinary type-inference change or new compiler support is approved.

- **Construction, propagation and OOM diagnostics:** [H-510–H-517](../CHANGELOG.md#h-510), [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve), [K-EXCEPTION-CONSTRUCTION-20261116](../CHANGELOG.md#k-exception-construction-20261116), records r72→73. H-510–H-515 separates immutable throw propagation from exception construction/trace capture, explicit constructor-form rethrow, implicit caught causes, bare wrapper rethrow and construction-only readonly cause. H-516/H-517 selects bounded process-level OOM information with the earliest record pinned and later records retained in a rolling window. Candidate C-36/C-63/C-65/P-464–P-469 relates inspectable failure provenance and bounded emergency diagnostics to charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve semantic checkedness, actual owners, immutable exception values, current capture rules, deferred-queue boundaries and D-16. Acceptance and open physical/trace/aggregate policies are below; no source support, permanent ABI, cause-storage bound or full value-path qualification is inferred.

- **Deferred exception handling:** [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch), H-503/H-505/H-507–H-509 and [H-541](../CHANGELOG.md#h-541) govern local attached catches, queued bodyless handling, exclusion of ordinary body errors, once-per-entry finally and FIFO residual collection. Candidate P-461–P-463 connects C-36/C-44/C-63/C-65 to charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit failure coverage, observable order and valid resources. Preserve preparation, actual registration, type identity, contexts, nested scopes, owners and D-16. H-538 separately retains an ordinary body failure; H-539/H-540 retains later-return replacement and continued cleanup. Acceptance, completed-pass checked coverage and pending aggregate/physical policies are below; local decisions do not qualify the complete value path or establish compiler support.

- **Module shutdown order:** [H-501](../CHANGELOG.md#h-501)/[D-15-SHUTDOWN](DesignAuthorityRegistry.md#d-15-shutdown) selects application-wide reverse registration for module-level deferred actions after interleaved initialization. [K-MODULE-SHUTDOWN-20261116](../CHANGELOG.md#k-module-shutdown-20261116) records r69→70. Candidate C-36/C-44/C-59/C-63/C-65/P-458–P-460 serves charter §§2/3.3/4.3/6.3/12 through observable cleanup order and live owner obligations. Existing qualified per-action/per-scope preparation, failure, owner and nested-scope contracts continue. D-15 owns the 3/2/1 and partial-startup acceptance targets; D-17-DEFER-DISPATCH owns the handler distinction and remaining completion/representation questions. No runtime startup scheduler or new compiler-support claim follows.

- **Mainline continuity:** [K-719](../CHANGELOG.md#k-719) continues r67→68 while integrating the scoped assertion/naming decisions below. Current source/native capabilities, qualified value paths and unresolved mechanisms retain their separate boundaries; current compiler support is tracked in the source capability reference.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-17r68→69 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Type and predicate assertion failures:** [H-467–H-471](../CHANGELOG.md#h-468)/[K-ASSERT-SPECIALIZATION-20261115](../CHANGELOG.md#k-assert-specialization-20261115), [D-10-TYPED-EXPR](DesignAuthorityRegistry.md#d-10-typed-expr)/[D-10-ASSERT](DesignAuthorityRegistry.md#d-10-assert), P-411/P-413, integrates unchecked runtime mismatch and predicate failure with static resolution and explicit non-null refinement. Existing C-63/P-235/P-236 and charter §§3.3/4.3/6.3/12 preserve effects, ordinary recovery and cleanup. TypeAssertionFailedError/AssertionFailedError follow H-472 and select no ABI. Static failure rejects; unresolved failures need no throws/catch. Concrete-type operation selection preserves D-26 coverage and unknown-interpretation distinctions. Acceptance includes proof success/failure, runtime cleanup, checked operand failures and path-sensitive facts; broader syntax/evidence mechanisms and source implementation remain separate.

- **Default preparation and initialization:** [H-458](../CHANGELOG.md#h-458)/[K-DEFAULT-PHASE-20261115](../CHANGELOG.md#k-default-phase-20261115) records the selected entry, prepared-call and cleanup contract in [D-10-ARGUMENTS](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding). [K-715](../CHANGELOG.md#k-715) continues this scope together with the current bounds policy. Caller-context defaults prepare before entry; definition-environment defaults initialize in the entered callee. C-63/P-235/P-236 and charter §§3.3/6.3/12 preserve observable effect order and exact cleanup responsibility. Acceptance covers both failure phases and deferred registration/invocation; this adds no rollback, ownership restoration or implementation claim.

- **Bounds policy:** [D-17-BOUNDS](DesignAuthorityRegistry.md#d-17-bounds), r69 section below, records [H-448](../CHANGELOG.md#h-448)/[K-BOUNDS-POLICY-20261115](../CHANGELOG.md#k-bounds-policy-20261115). C-63/C-65/P-401–P-402 serves charter §§4.3/6.3/8/12: recoverable default bounds failures without mandatory throws annotations, with an explicit user-selected check opt-out. IndexOutOfBoundsError is unchecked; builtin indexing checks bounds by default. The opt-out transfers the range obligation to the program; invalid unchecked execution has no defined-result/exception/cleanup guarantee. Other type, owner/access, initialization, null and numeric contracts remain. Acceptance covers default recovery/cleanup, empty and boundary indices, read/write/view selection, proved check elimination and explicit opt-out isolation. [K-709](../CHANGELOG.md#k-709) continues this scope through cause_at and the bounded whole-artifact option below. Broader indexing, mixed-artifact integration and varargs qualification remain separate.

- **Registry / current scope:** [D-17](DesignAuthorityRegistry.md#d-17); the semantic revision stated above and the independently qualified scopes in the current registry. **Confirmed/continued bounded exception, lifetime, scope, defer and return contract.** C-63/P-235/P-236; H-267/H-269/H-313–H-315/H-319/H-356–H-362/H-365/H-370/H-371; P-321–P-324/P-326/P-328/P-329. D-26r9/H-400/H-401/H-404/H-405/H-407 through C-90/P-374/P-375 governs the unknown-type failure categories and explicit declaration obligations.
- **Value path:** C-63/P-235 → C-10 and D-17/P-236 → C-63: recoverable business failure must preserve explicit regional responsibility, effects and inspectable lifetimes. H-267 confirms this bounded path and scope under charter §§2–4, 6.2–6.3 and 12; it does not qualify unrelated ownership proposals.
- **Contract boundary:** Acceptance: construction failure, exact release, explicit causes, owner/effect restoration, pre-entry versus callee failure, completion replacement and deferred queues. Complete integration/proof coverage remains work. No regional copy, lifetime extension, native-handle/out policy, RTTI, persistent transaction or broad D-04 guarantee follows.
- **Invariants / non-goals:** P-237 links D-14's ownership/borrowing, P-238 links D-10's scoped bindings, and P-239 links D-01's allocation, allocator symmetry and relocation. H-180/H-181 preserves hidden/captured owners; H-262/H-263 separates consumed names from valid local views. H-265/P-240 supplies the unchecked-OOM constraint. H-251 reserves RTTI decisions for human discussion. one cleanup responsibility per region; no per-object ownership, implicit owner resurrection, allocation-dependent OOM delivery, graph rollback, foreign-world editing or general RTTI decision. No C++ exception type or ABI becomes a CoD user type by implication.
- **Acceptance:** Nested normal and exceptional paths preserve values, cleanup and effect restoration; controlled construction failures neither leak nor double-release; invalid exceptional-path borrows reject. Deferred registration, execution and completion replacement preserve their specified ordering. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines remain the H records identified by the registry. [K-RESIZE-20261114](../CHANGELOG.md#k-resize-20261114) continues r56→57: H-431 retains E4 ordinary consuming-call cleanup for the H-432 resize proposal, with pre-entry preparation and post-cleanup result delivery kept distinct. H-428 allowed allocation effects and the qualified completion contract remain unchanged; no language allocation checkpoint or ownership recovery is added. [K-REFERENCE-IDENTITY-20261114](../CHANGELOG.md#k-reference-identity-20261114) integrates H-439/H-440 owner-plus-locator identity and is null, preserving established access, result and lifetime behavior. Current source/native coverage and proof boundaries are recorded in the [source capability reference](../tests/cod/language/README.md). [K-IS-NOT-20261114](../CHANGELOG.md#k-is-not-20261114) adds H-441 is not as exact identity negation, preserving evaluation, contextual null ownership, access and lifetime rules; implementation coverage remains bounded by the [source capability reference](../tests/cod/language/README.md).
- **Current implementation alignment:** [K-716](../CHANGELOG.md#k-716) continues the qualified caller-binding, operation-identity, bounds and prepared-defer contracts. The [source capability reference](../tests/cod/language/README.md) records direct/prepared scalar, intrinsic-exception and complete callable-holder methods through shared invocation evidence. Saved slots retain invocation-time read, initialization, owner, permission, pending-completion and cleanup obligations; complete payload copies retain ordinary failure/release behavior. Materializing caller-binding methods as ordinary callable values, wider preservation proofs, allocation qualifiers and physical storage mechanisms remain separate.

## Built-in failure names and assertion responsibility

H-472 selects **AssertionFailedError** and a canonical built-in naming convention: **unchecked names end in Error; checked names end in Exception**. H-473 names the general root **Exception**. H-524 adds **Error <: Exception** as the root of unchecked exceptions, including user-defined ones. Exception is a checked declaration contract; Error and its exception descendants are unchecked. Exception still accepts both branches as a general exception type, and catch (e:Exception) can handle both.

Checkedness is a semantic type contract, not a test of the identifier's suffix. Canonical built-ins follow the naming convention; arbitrary user exception names are not restricted. Aliases preserve the original contract. Unchecked means no mandatory checked handling obligation, not fatal, unrecoverable or necessarily the programmer's fault. The root relation selects no concrete field inheritance, native address layout or general RTTI ABI.

**Assertion responsibility.** A program reaching a false assertion has violated its author's assertion; raising AssertionFailedError is the correct CoD behavior, not evidence of a CoD soundness defect. A failed explicit type assertion likewise raises TypeAssertionFailedError. Both differ from the established-typing backstop UnsoundRuntimeTypingError. Soundness would instead be at issue if a required check were omitted or a false/invalidated assertion fact were used to permit unsafe work. Successful-continuation facts, static rejection under H-470, once-only effects and ordinary cleanup retain their contracts. Diagnostic message/payload layout remains engineering work.

For currently implemented categories, the [compiler descriptors](../cod/lang_exceptions.hh) supply canonical names to vocabulary selection, native emission and services. Concrete supported categories retain their checkedness. The checker enforces declared Exception obligations separately from exact-category facts. The Error root and arbitrary user exception payloads remain unimplemented. Only canonical imports are admitted; no compatibility aliases for retired names are provided. User-selected aliases retain the resolved type's obligations.

| Canonical name | Classification | Current compiler support |
| --- | --- | --- |
| Exception | Checked declaration contract; general root including Error | Common-root values/catches and separate declared-obligation enforcement; new Error-root integration remains separate |
| Error | Unchecked root; Error <: Exception | Design only; root/conformance integration unimplemented |
| AssertionFailedError | Unchecked predicate failure | Predicate assert construct not implemented |
| TypeAssertionFailedError | Unchecked type-assertion mismatch | Expression type assertion not implemented |
| OutOfMemoryError | Unchecked | Callable, deferred-record and cause allocation failures |
| RegionCapacityExceededError | Unchecked | Category/vocabulary; source regional construction remains outside the current compiler slice |
| InvalidCapacityError | Unchecked | Category/vocabulary; source regional construction remains outside the current compiler slice |
| IndexOutOfBoundsError | Unchecked | cause_at bounds checking; broader indexed access remains separate |
| MissingReturnError | Unchecked | Required-value fallthrough |
| RethrownException | Checked standard wrapper; requires a checked cause | Design only; constructor, trace and ownership integration unimplemented |
| RethrownError | Unchecked standard wrapper; requires an unchecked cause | Design only; constructor, trace and ownership integration unimplemented |
| UnhandledDeferError | Unchecked aggregate of residual deferred exceptions | Design only; exceptions field and propagation integration are unimplemented |
| UnsoundRuntimeTypingError | Unchecked | Category/vocabulary; outside-catalog typing validation remains separate |
| NullPointerException | Checked | Nullable target access |
| StaticallyUnknownTypeException | Checked | Category/declaration obligations; outside-catalog interpretation remains separate |

**Acceptance:** every concrete canonical built-in name agrees with its existing checkedness; root Exception accepts both categories; aliases retain obligations; assertion failure is attributed to the author's assertion without weakening checks/refinement; existing catches, declared failures, causes and cleanup preserve their semantics. Vocabulary, diagnostics and services use the same canonical descriptors. A typed Exception catch follows the ordinary catch-all path, including aliases and generic substitutions, without catching sibling-handler failures or weakening checked rethrow obligations. It does not by itself implement missing assertion/bounds operations or qualify broader user exception graphs. No backward-compatibility aliases are selected here.

## Checked Exception and unchecked Error roots

H-524 provides the semantic root relation; H-525 requires source acceptance to honor the declared type rather than silently substituting a more precise inferred type. H-526 rejects additional conformance that changes an existing exception's checkedness.

```cod
# Illustrative standard abstract roots and user data headers.
type Exception;
type Error <: Exception;
data CacheUnavailable <: Error { code:u64; }       # Unchecked.
data RecordRejected <: Exception { code:u64; }     # Checked.
```

These headers use existing nominal abstraction and explicit data conformance; they do not add concrete-layout inheritance or establish the construction/owner protocol for arbitrary user exception payloads. User-defined Error types are the intended supported extension point. Source implementation, payload/trace/cause ownership and abstract-root constructibility remain within their separate contracts. Error-derived exceptions remain exceptions: root catches accept them, while Error catches accept only the unchecked branch. Both follow source-order matching and the standard-wrapper transparency rule. RethrownError and UnhandledDeferError belong to the Error branch; RethrownException belongs to the checked branch. Existing concrete built-in classifications do not change.

### Declared handling obligations

A throw operand declared Exception has a checked obligation even if dataflow proves its current value is an IndexOutOfBoundsError. A broad catch binding declared Exception likewise stays checked when propagated. The compiler may diagnose that a more specific declaration is available; it must not silently narrow the declaration or accept an otherwise invalid throw based on that proof. An explicit Error or concrete unchecked declaration has the unchecked contract. Ordinary validation of the narrower declaration still applies.

```cod
proc forward(e:Exception) throws Exception { throw e; }
proc forward_error(e:Error) { throw e; }
# Without throws or covering catch, forward's throw must reject,
# even if every currently visible call passes an Error.
```

An explicit throws Exception contract imposes a checked obligation on callers. A particular implementation, specialization or optimizer proof that only unchecked values occur does not erase that declaration. Ordinary checked propagation is allowed only through an appropriate explicit throws contract; module/entry code must handle its checked obligations. A type alias is not a narrowing operation. Explicitly selecting a narrower declaration or an admitted type assertion is a source change expressing the author's intent; diagnostic tooling must not apply it silently. Ordinary generic substitution follows the declared generic contract and does not rewrite an explicit Exception annotation. This boundary does not remove actual-type dispatch or other established flow proofs where their contracts permit them.

Keep **runtime failure classification** distinct from the **declared propagation obligation**. A root-typed value can actually be an Error, and catch matching must still use its actual type. Automatic rethrow from that value chooses the runtime-appropriate standard wrapper, but retains the checked source obligation of the Exception declaration; it is not a way to evade handling. Likewise a hidden tip selection does not narrow the declared obligation. Explicit fixed-wrapper construction must satisfy its cause constraints and cannot use inferred Error identity to bypass a broader incompatible declared cause contract. An explicitly named business exception may deliberately translate a failure under its own type's contract, as already selected; that is different from silently changing the source declaration.

**Design counterexample:** accepting a rejected Exception-typed throw merely because an initializer or current callers happen to supply Errors guesses an unexpressed narrower contract. Use a diagnostic to recommend the narrower declaration instead. Exact diagnostic formatting/severity and richer transparent throws summaries are separate.

**Bounded enforcement in the existing checker.** H-525 constrains ongoing implementation of the qualified declared-failure contract; enforcing that prohibition does not depend on first implementing the new Error root or arbitrary exception payloads. A change to actual-value or alias provenance must not introduce acceptance of an otherwise uncovered Exception-typed throw. Keep the declared obligation distinct from facts used for actual runtime category matching. This is a source-validation requirement, not a selected internal data structure or runtime carrier.

Resolve ordinary type identity and admitted generic substitution before applying the rule: an alias of Exception, or a declared T instantiated as Exception, retains the Exception obligation. A T instantiated under an admitted concrete unchecked contract has that concrete contract; current payload observations cannot silently change the substitution or an explicit Exception annotation. This follows ordinary semantic type identity rather than the spelling of the type token, and adds no generic exception-effect syntax.

Bounded acceptance includes rejecting uncovered root-typed throws despite known unchecked initializers, borrowed payload facts, catch bindings or helper results; retaining explicit throws Exception caller obligations; equivalent aliases/substitutions; ordinary covering root catches; and unchanged concrete unchecked propagation. Existing qualified ordinary catch and completion rules remain constraints. This does not decide whether narrower runtime-category evidence alone can establish coverage for a broad declared obligation, the pending transparent-wrapper throws relation, or the changed deferred-error queue's recovery/suppression/aggregation rules. Keep those cases separately identified rather than using the current single-pending-error implementation to decide them. No proof improvement may revive H-525's rejected waiver; a missing coverage proof must be distinguished from a demonstrated language contradiction.

### Classification is not adapter-local

Once an exception type's defining contract establishes checkedness, adding conformance in an adapter cannot change it. In particular, importing a checked Failure and adding data Failure <: Error rejects, even though ordinary data-to-interface conformance is extensible. Aliases, selection context and load order cannot change that contract. An adapter may define its own new Error type and explicitly construct it with the original failure as cause; that preserves both identities and makes translation visible. Ordinary added conformance remains available when it preserves checkedness and satisfies its other contracts.

[D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) records H-527–H-535 disjoint declarations with existing ancestry among listed endpoints. Declaring type disjoint Error, Exception after Error <: Exception retains that ancestor relation as exempt, directly from the existing type graph; it therefore permits new Error-derived types without asserting an empty Error/Exception intersection. H-535's general rule rejects a subtype's second independent listed-parent relationship, including a checked Failure <: Exception later declaring Failure <: Error. This supplies the requested general mechanism for H-526. A redundant shorter declaration warns and preserves intermediate contracts. H-526's fixed checkedness rule and H-525's explicit Exception obligations continue; no hidden CheckedException root is introduced.

**Acceptance:** user-defined checked and Error-derived types; inherited uncheckedness and alias stability; Exception/Error/typed catches in source order; root-typed known Error throws still requiring checked coverage; actionable narrower-declaration suggestions; unchanged explicit throws Exception contracts across specialization/callers; automatic wrapper runtime category with conservative declared obligations; and rejection of checkedness-changing adapter conformance. General user payload construction/transport, full wrapper-aware throws inference and a physical type/identity ABI remain unimplemented or separately scoped. Existing finite intrinsic-category evidence does not prove these extensions.

## Exception construction, throw and rethrow

H-510–H-515 gives construction and propagation separate responsibilities. An exception's trace is captured when the exception is newly constructed. Propagating, copying, storing, returning or catching an existing exception does not replace that trace or create a new exception merely to transport it. This is a semantic value/provenance rule, not a selected address identity or payload-copy ABI.

| Form | Result |
| --- | --- |
| `make SomeEx(args...) [from cause]` | Construct an exception, capture its creation trace and return the value. Without from, no cause is supplied. Construction itself can fail under its ordinary contract. |
| `throw e;` | Evaluate e once, select an existing exception through the catch-local identity rule below, and propagate it unchanged, including its type, fields, cause and trace. The operand may independently perform construction or other effects. |
| `throw;` | Propagate the nearest lexically available catch context's incoming tip unchanged. Requires that context under ordinary scope/capture rules. |
| `throw SomeEx(args...) [from cause];` | Construct the explicitly named exception operand, then propagate it. Construction captures the trace; throw adds no further exception or trace. Without from, no implicit caught cause is supplied. |
| `rethrow SomeEx(args...) [from cause];` | Construct and propagate the explicitly named exception with a fresh creation trace. An explicit from supplies its cause; omission uses the nearest lexically available catch context's incoming tip. |
| `rethrow;` | Construct a new RethrownException around a checked caught value, or RethrownError around an unchecked caught value, with the retained incoming tip as cause and a new creation trace; propagate the wrapper. Requires a caught context. |
| `rethrow from cause;` | Evaluate the explicit cause once, apply the catch-local identity selection below, then construct and propagate a fresh RethrownException for a checked cause or RethrownError for an unchecked cause. Capture a new creation trace; no caught context is required when the cause is explicit. |

Here SomeEx must resolve to an admitted constructible exception type; spelling an ordinary value or factory callable like a type does not make it constructor syntax. Type aliases and generic specialization follow their ordinary semantic identity rules. The constructor-form throw is explicit operand construction, not permission to attach a cause to an existing exception. `throw e from cause` and `rethrow e` for an already created value are not the selected forms. An ordinary factory expression can still be evaluated by plain throw; any exception construction occurs inside the factory under its own context.

**Cause initialization.** The canonical cause field is not an ordinary constructor parameter: it cannot be supplied positionally, through a named argument or through a parameter default. A from clause on exception construction is its single source-visible initialization mechanism, including construction via make, constructor-form throw and rethrow. The cause becomes readonly at publication and cannot be rebound or mutated through an alias or assertion. A business payload containing another exception remains distinct from the reserved cause relationship. Absence of cause and its representation are separate; no pointer/null-owner ABI is selected here.

Evaluate constructor arguments in their normal specified order, then an explicit cause expression with the catch-local identity selection below, and construct the exception once with the resulting values and trace. Omitted rethrow cause resolves to the caught context's retained incoming tip under the rules below, not an ambient dynamically replaced exception. If operand preparation fails, no partially established requested exception is propagated and prior permitted effects are not rolled back. Constructor failure follows its ordinary failure path; it does not mutate either the supplied cause or another existing exception. General defaults/construction rules retain their own contract.

**Implicit caught value.** A helper defined outside a catch cannot acquire an implicit cause merely because a catch calls it. Supply from explicitly there. Inside nested catches, use the nearest valid caught binding. A delayed action referring to an enclosing catch retains that value through the ordinary checked capture rules; it does not select whichever unrelated exception happens to be pending when it runs. Each invocation of a bodyless deferred catch has its own matched value and retained incoming tip. Its sibling attached finally has no implicit last-handled exception; it may use a different enclosing catch binding if one is valid, otherwise it needs an explicit cause. Missing required caught context is a compile-time error.

```cod
# Inside a catch with matched value e and incoming tip:
throw e;                         # Incoming tip and its trace, unchanged.
throw;                           # Same incoming tip, unchanged.
rethrow;                         # New checked/unchecked wrapper; cause is tip.
rethrow DomainError(details);     # New DomainError; implicit cause is tip.
rethrow DomainError(details) from another;
# New DomainError; cause is another, or tip if another has e's identity.
```

The named constructor's semantic type determines matching and checkedness. A checked cause does not make an explicitly constructed unchecked business type checked, nor does an unchecked cause remove a checked business constructor's obligation. Standard rethrow wrappers have the additional cause constraint below. Bare rethrow and rethrow from cause choose the corresponding wrapper using the selected cause's actual semantic checkedness, even through the root Exception type or aliases. Catch matching uses the wrapper-prefix rule below: only consecutive standard rethrow wrappers are transparent. A named business exception or UnhandledDeferError stops standard-wrapper cause traversal. That traversal does not itself search arbitrary causes or aggregate members; H-537 places the finished deferred aggregate on the ordinary exception path, not back in the completed defer-handler pass. Checked failure coverage must reflect both the incoming checked type and the selected transparent matching rule; the exact throws-summary relation remains under review below.

### Standard wrapper construction and diagnostics

H-523 requires RethrownException to have a checked cause and RethrownError to have an unchecked cause. These constraints apply to semantic type identities, including aliases, and to every construction form: make, constructor-form throw and constructor-form rethrow. A standard wrapper without a cause rejects. Constructor-form rethrow may supply that cause implicitly from a valid lexical catch; ordinary make/throw construction does not gain an implicit cause. Business exceptions may deliberately translate a failure under their own type contracts; this restriction belongs to the two transparent standard wrappers.

`rethrow from cause;` is the explicit-cause form of automatic standard wrapping. Prepare its expression once under ordinary evaluation/owner/access rules, use the current lexical catch's tip if H-522's identity rule matches, and choose the wrapper from that selected exception's actual checkedness. Allocate/construct one new wrapper and capture its creation trace; retain the existing cause chain unchanged. An explicit cause also works outside a catch and does not import a caller's hidden context. A failure while preparing or constructing the wrapper follows ordinary exception/cleanup rules; an invalid source construction does not execute merely to discover its mismatch.

For an explicitly named standard wrapper, static cause evidence must establish its checkedness constraint. Known mismatches reject. If the admitted cause can be either checked or unchecked, reject the fixed wrapper choice and recommend the automatic form; ordinary declared generic substitution or an explicitly narrower source declaration can establish the constraint. H-525 forbids using a more precise runtime-value proof to bypass an incompatible explicit cause contract. The automatic form may choose between the two prepared wrapper categories at runtime when necessary. This does not erase possible checked obligations, create a new runtime type family or specialize per wrapper depth. The remaining transparent throws-summary contract is below.

```cod
catch (e:const NullPointerException) {
    rethrow RethrownError() from e;  # Reject: checked cause, unchecked wrapper.
}
# Suggested replacement for that statement:
catch (e:const NullPointerException) {
    rethrow from e;                 # Constructs RethrownException from tip.
}
```

Required diagnostic content for the mismatch includes the selected wrapper, the cause's conflicting checkedness and a direct `rethrow from e;` suggestion. Illustrative text: `RethrownError requires an unchecked cause, but e is checked. Use rethrow from e; to select the matching wrapper automatically.` An unknown checkedness diagnostic must say that it could not establish the fixed wrapper's requirement rather than claiming a proved mismatch. Preserve the user's actual cause expression in a suggested replacement. For a make expression where replacing an expression with a statement would be invalid, explain the automatic propagation form without applying an invalid statement-shaped fix. Exact formatting is not a new language ABI.

Every valid consecutive standard-wrapper prefix consequently has one checkedness and one canonical wrapper category. An explicit wrapper catch matches and binds the outermost incoming wrapper of that category; there is no differently classified inner standard wrapper to search for. Ordinary catches still match the first non-wrapper exception, and all clauses retain source order. A business exception ends that prefix even if its own cause has a different checkedness.

**Acceptance:** checked-to-checked and unchecked-to-unchecked named construction; symmetric mismatch rejection and diagnostic suggestions; missing causes; implicit constructor-rethrow causes; root-typed known/refined versus mixed causes; automatic selection inside/outside catches; aliases; same-identity helper results; one evaluation and one new wrapper trace; unchanged source exceptions; allocation/preparation failure; and no checkedness change along a valid standard-wrapper prefix. Physical storage and the remaining throws/inspection contracts are not settled by these rules.

**Trace and storage boundary.** A value made in a helper and thrown later retains the helper's construction trace. A new rethrow wrapper records the new construction context while preserving the original exception and its trace as cause. A new UnhandledDeferError likewise has its own construction trace; its exceptions field preserves each member's existing information. No first-throw capture, automatic trace refresh, implicit captured-value recapture or mutation is allowed. Logical/native frame selection, optimized-frame reconstruction, maximum trace detail, access API and persistent representation remain engineering/design scope; the bounded source-site metadata in the current compiler does not establish full traces. Construction can require storage; plain propagation must not require a new wrapper or recapture allocation. Ordinary creation/chain-allocation failure and deferred-queue membership/replacement must be kept distinct.

**Acceptance:** compare direct propagation with aliases, helper calls, copies, stored returns and delayed propagation; distinguish creation trace from throw site; cover named/bare rethrow, checked/root-typed values, explicit/implicit/absent causes, nested catches, captured causes, rejection of cause constructor parameters and invalid outside-catch defaults, unchanged input values on construction failure, and finally without a sibling catch binding. Ordinary return/owner rules, explicit exception identities, independent deferred entries and D-16 remain constraints. The selected forms and trace rule are not implemented by this documentation revision.

## Wrapper-aware catch matching

H-518/H-519 selects source-order clause evaluation. Let tip denote the incoming exception. For ordinary clauses, follow consecutive standard RethrownException/RethrownError causes until the first non-wrapper exception and use that exception for matching. For a clause explicitly naming a standard wrapper type, match the incoming outer wrapper rather than its non-wrapper cause. Aliases of those types retain the same meaning; spelling does not change identity. Take the first matching clause in source order. There is no preliminary priority pass for explicit wrapper catches.

```cod
try { run(); }
catch (e:const IndexOutOfBoundsError) { handle_original(e); }
catch (e:const RethrownError) { inspect_wrapper(e); }
# For RethrownError -> IndexOutOfBoundsError, the first clause wins.
# Reverse these clauses to inspect the incoming wrapper first.
```

Stop traversal at a business exception even if it has a cause. Stop at UnhandledDeferError rather than searching its exceptions field; H-537 routes this completed aggregate through ordinary propagation/catch after the defer-handler pass. An explicit wrapper clause binds the incoming outer wrapper. H-523 excludes mixed standard-wrapper categories within a consecutive prefix, so no deeper category search is needed. An untyped/root catch retains its ordinary catch-all role and source-order restrictions. Matching must not construct a new exception, duplicate a wrapper graph or capture a new trace. Immutable chains allow reuse of the unwrapped locator/type during one clause scan.

### Matched binding and hidden tip

H-520 prefers retaining wrappers wherever the operation can use them, including explicit from e. H-521/H-522 selects a hidden tip reference with the matched catch binding e and **exception identity within the current lexical catch context**, including an unchanged exception returned by a helper. Keep e's payload access and ordinary nominal/actual type evidence consistent with the matched exception; retaining tip must not make typeof, generic inference or ordinary overload selection silently switch e to the wrapper's type.

The model keeps two existing targets: the matched exception and the incoming outer exception. No exception graph copy is required. Retaining/copying a small stable handle when required by capture/lifetime rules is not duplication of the graph. The hidden tip must refer to valid exception backing, not a mutable pending slot or an ended catch frame. Repeated deferred-handler invocations receive separate associations; nested catch exit restores the enclosing context rather than leaving a mutable last-caught value.

**Identity selection.** Evaluate the exception operand once. At a throw-value or explicit from operation with a current lexical catch context, compare its exception identity with that context's matched exception. If they are the same, select that context's tip; otherwise select the operand itself. Without such a context, use the operand unchanged. Bare throw, bare rethrow and omitted rethrow cause directly select that context's tip. This is selection of an existing exception, not mutation or implicit wrapper creation.

Identity is not equal type, fields, message or trace, and is not a relocatable raw address. Newly constructing an exception creates a distinct identity even with identical fields. Ordinary aliases, permitted payload copies, storage, arguments and unchanged returns retain the existing exception's identity; they do not transport an ambient catch context or secretly change the nominal type. The selected behavior requires a stable identity/handle implementation, in addition to retaining tip. Current five-word intrinsic values do not establish that representation. No universal public identity ABI, general RTTI or physical heap allocation policy is selected. Runtime construction in a loop or recursive call must produce a fresh identity on each execution, even at an identical source site with equal fields. Value/capture/field/outcome transport and extracted causes or aggregate members preserve existing identities. Source-site IDs and reusable OOM diagnostic slots do not satisfy this requirement. A finite token/handle implementation needs a live-identity uniqueness and exhaustion/reuse contract; its observable fallback remains unselected. Identity comparison may be folded when proved, but that proof does not waive H-525's declared checked obligation. The compiler review identifies transport through the current five-word carrier and retained backing as real implementation work; one local hidden reference alone does not implement the full value-identity contract.

```cod
# echo is defined outside the catch and returns its argument unchanged.
catch (e:const IndexOutOfBoundsError) {
    rethrow DomainError() from echo(e);  # Same identity: cause is incoming tip.
}
# Constructing another IndexOutOfBoundsError with equal fields is different:
# using that new exception as the cause keeps that new exception.
```

| Operation | Selected exception / boundary |
| --- | --- |
| Ordinary typing, type-specific payload access, arguments and results involving e | The matched exception; ordinary type/owner/access rules stay intact. |
| Bare throw, bare rethrow, or an omitted rethrow cause | The lexical catch context's incoming tip, preserving full wrapper history. Bare throw propagates it; rethrow constructs the selected new exception. |
| Explicit from operand or throw-value operand | Evaluate once; use tip exactly when the operand has the current context's matched exception identity. This includes unchanged helper results and aliases. |
| An operation in a separately defined helper | No access to its caller's catch context. A result returned unchanged can still match identity at a subsequent operation inside the caller's catch. |
| Cause/trace/category inspection, or an explicit request to bypass tip selection | Pending below; ordinary payload typing does not decide these APIs. |

The current context is the nearest lexically enclosing catch available at the operation, with its normal capture/lifetime checks. Do not search every active catch by identity or implicitly import a caller's context. If two nested catches have the same matched exception with different tips, the inner context governs operations in its body; leaving it restores the outer context. Identifier spelling, grouping and helper expression shape do not override this choice. A delayed operation retains the selected matched identity and tip/owners; it does not consult the unrelated catch or pending error active when invoked.

### Remaining wrapper-matching decisions

1. **Inspection and explicit extraction.** Specify which exception cause/trace/category inspection observes and how a programmer explicitly selects the matched object or an actual cause without tip normalization. Under H-522, an expression yielding the same matched identity is normalized at a catch-local from/throw operation regardless of its syntax; assigning it to an alias alone is not an escape hatch. Reading a cause is not itself construction and must not mutate a chain or produce a graph cycle. No generic exception-view ABI or new inspection API is selected.
2. **Transparent checked obligations.** H-523 fixes cause presence and equal checkedness along the standard wrapper prefix. Static coverage must still correlate incoming outer type with matchable leaf type rather than treating independent category sets as arbitrary pairs. Decide the transparent meaning of throws contracts and retained cause evidence; RethrownException alone does not prove a particular leaf category. Runtime depth must not create one generic instantiation per wrapper level.

**Acceptance:** direct and multiply wrapped inputs; source-order ordinary/root/explicit-wrapper clauses; stopping at business errors and aggregates; matched e versus retained tip; once-only operand evaluation; identity preserved by an unchanged helper return and permitted copies but not by equal-field reconstruction; nested same-leaf/different-tip contexts, shadowing and delayed/repeated catches; absence of implicit caller context in separate helpers; checked outer/leaf correlation; no graph duplication; and each resolved inspection/wrapper case above. Existing exact-category matching and five-word intrinsic values do not establish this behavior. Owner-safe stable identities, hidden locators and normalized leaf caches are implementation candidates, not qualified public representations.

## Selected direction

CoD provides `try`, `catch` and `finally` control in the accepted contract. Ultimate out-of-memory failure is an **unchecked exception**: calling allocation does not require a checked-error result, a `throws` declaration or an enclosing catch merely to compile. A caller may catch the failure; otherwise it propagates toward the execution boundary. Unchecked does not mean ignoring ownership validity or turning invalid references into runtime exceptions.

`make` continues to produce its normal `owned region :Root` result when construction succeeds. OOM is an exceptional exit, not an implicit null owner or a mandatory optional/result wrapper. H-265 does not select automatic allocation retries, region growth, compaction or a process recovery policy. “Ultimate” failure describes exhaustion after any separately defined policy has failed; no such policy is introduced here.

The initial error taxonomy distinguishes `OutOfMemoryError` (the memory provider cannot supply storage), `RegionCapacityExceededError` (placement exceeds an existing region's fixed budget), and `InvalidCapacityError` (an invalid or unrepresentable capacity request). All three are unchecked; none implies automatic growth or retry.

H-428 confirms allocation as an allowed initialization side effect. A failing initialization group does not rewind an existing region or undo earlier valid publication/effects. Its incomplete bindings are not successful results, while an already initialized target published into surviving same-owner state remains live with its region. Failed acquisition of a new whole region still releases its acquired resources exactly once. Application/library undo frameworks require their own contracts; the language supplies no allocation checkpoint or automatic rollback.

Compile-time type, moved-name, escape and binding errors remain compile errors. Other runtime checks are not all reclassified as catchable exceptions merely because OOM becomes catchable.

## Unchecked bounds exceptions and optional checks

H-448 selects IndexOutOfBoundsError as an unchecked exception for builtin indexed access, including admitted array indexing and executable variadic-pack indexing. A callee or caller need not declare throws or provide a catch merely to compile; a program may catch the exception, and ordinary exception propagation, defer/finally and cleanup apply when it is thrown. Empty sequences have no valid element index. Index-type admission/conversion remains a separate contract; this decision admits no fractional index or new numeric conversion.

Runtime bounds checking is enabled by default, including optimized builds unless the user explicitly opts out. A failing bounds check prevents the invalid element access or view construction and raises IndexOutOfBoundsError. Reads, writes and reference-producing builtin selection use the same policy. Preserve each operation's established operand preparation, conversion and evaluation order; adding a check must not reevaluate operands or silently change unrelated effects. User-defined indexing retains its selected method's own failure contract; any builtin indexing that method performs follows this policy.

```text
# Design example; executable varargs integration remains separate.
proc pick(i:u64, rest:u64...) -> u64 {
    return rest[i];           # No throws declaration required for bounds failure.
}
# With default checking, pick(0u64) raises IndexOutOfBoundsError.
```

An explicit compiler option may disable generated bounds checks. The programmer then assumes the range precondition for affected builtin accesses. Executing an out-of-range access with checking disabled is outside the defined safe execution contract: no result, exception delivery or cleanup behavior is guaranteed. In particular, a catch cannot be relied on to intercept that access. This is the interpretation of H-448's explicit at-own-risk opt-out, not a claim that unchecked exceptions themselves bypass cleanup. In-range execution retains its ordinary semantics.

The option controls source-level builtin bounds checking only; compiler/runtime internal consistency checks are not its target. It does not disable type/owner/lifetime/access/definite-initialization checking, checked NPE obligations, numeric validity or other runtime checks. It is not a compiler fallback for an unproved initialization or aliasing case under D-28-INIT/P-400. Static proof that an access is in range permits ordinary elimination of its redundant bounds check with no opt-out; lack of a range proof in the default mode requires the runtime check rather than unsafe acceptance.

The current source compiler exposes `--no-bounds-checks` for an entire compiled source artifact, including imported modules and every general/specialized implementation. Generated calls retain the selected policy independently of runtime-source emission. The admitted cause_at builtin reports the call site, requested index and available cause count through the existing exception fields; it adds no automatic cause or exception allocation. The source capability reference owns runnable coverage. Broader indexing, separate artifact/link/cache integration and static range diagnosis remain implementation work. Generated/shared/cached code must preserve the requested policy: an unchecked variant cannot silently satisfy a request for default-checked execution. This policy selects no permanent ABI or runtime registry.

Acceptance requires default no-throws invocation, optional catch with normal cleanup, empty/boundary accesses, preserved operand effects, consistent read/write/view behavior, proven check elision, and an explicit opt-out affecting only its chosen bounds checks. Opt-out acceptance must not assert a particular result from invalid unchecked execution. Current capability evidence must distinguish implemented source/native indexing paths from the remaining acceptance scope.

## Checked NullPointerException

H-313 accepts checked NPE: every potentially null dereference must be covered by a matching catch or declared by its immediately enclosing function/procedure with `throws NullPointerException`. Module top-level execution must handle it explicitly. A catch-all handles NPE when it covers the operation; no automatic boundary handler satisfies the source obligation. Null/default absence is not a substitute for definite payload initialization or valid owner lifetime.

```text
using "std:exceptions" NullPointerException;
function following(n:Node in O) -> const i64
  throws NullPointerException where {owner O} { n.next.count }
try { following(node) } catch (e : const NullPointerException) { 0 }
```

The exception name is selected and aliased through the normal type-identity rules. Throws may follow the result annotation or the where clause. It does not distinguish overloads. Bodyless contracts and symbolic procedure signatures retain the checked obligation; an implementation cannot expose more checked failure than its applicable declaration permits. A caller handles declared NPE even if a particular specialization does not throw. An outer caller's catch cannot excuse an undeclared callee body, and WPO inlining does not erase that boundary. Functions may throw/catch without gaining dynamic-symbol access.

Only a dereference creates the null-access obligation. `null` is a reserved literal, never a binding. H-440/H-441’s `p is null` and `p is not null` evaluate the reference operand once and examine its selected owner/locator without loading the target; merely rebinding a pointer with `=` does not dereference it. Ordinary payload argument evaluation, including copying bits into val, must account for null access in the caller. A valid non-null ref needs no NPE obligation. Stable pointer tests may establish a non-null view on the corresponding branch; mutations invalidate the fact, and checking a mutable field does not prove a later reload unchanged.

A handler's NPE or rethrow and NPE from finally must satisfy the enclosing callable's own contract. A business exception's NPE cause does not alter its matching category; standard rethrow wrappers follow the transparent prefix rule above. Existing E1–E5 cleanup, explicit causes and replacement semantics remain. OutOfMemoryError, RegionCapacityExceededError and InvalidCapacityError remain unchecked; type, expired-owner and definite-initialization errors remain compile errors. Checked metadata introduces no payload RTTI, allocation on throw, general exception-effect polymorphism or runtime initialization registry.

Acceptance covers direct/module and callee-boundary rejection, explicit propagation, local handling and finally cleanup, bodyless and symbolic signatures, literal/test/rebind operations without target access, val argument evaluation, branch refinement and nullable loop paths. D-14 retains the reference initialization, same-owner, fixed-owner and current relative-encoding constraints. General exception payload graphs and unrelated deferred exception mechanisms remain outside this increment.

## Unknown-type and typing-contract failures

[D-26](AheadOfTimeExecutionPreparation.md#generic-and-recursive-native-coverage) classifies unavailable required type interpretation as checked `StaticallyUnknownTypeException` and a detected violation of an established typing contract as unchecked `UnsoundRuntimeTypingError`. Both use the ordinary exception control, explicit causes and cleanup rules. Static type errors remain compile errors; incomplete compiler reasoning cannot be substituted with an arbitrary runtime exception and claimed as successful native coverage.

Generic syntax or valid uninterpreted passage alone creates no unknown-type obligation. A voluntary `throws StaticallyUnknownTypeException` declaration retains the fixed caller obligation, including known-type calls whose bodies succeed. Selected exception types, callable views, handlers and deferred invocations preserve this contract. Acceptance includes declaration coverage, retained categories and causes, ordered failure routing and exact release in specialized and shared native execution.

The source capability reference distinguishes category/declaration support from outside-catalog input and required-interpretation detection. This integration selects no new input validation protocol, carrier, permanent ABI, dynamic cast or runtime compilation mechanism; those D-26 boundaries remain separately scoped.

## Nonnumeric type assertion failures

H-467/H-468 selects an **unchecked assertion failure** for an admitted nonnumeric type assertion whose runtime check finds a mismatch. H-470 resolves provable assertions first: success needs no runtime validation, and failure rejects compilation. [D-10](SymbolicDynamicScope.md#nonnumeric-assertion-contract) owns the operation; TypeAssertionFailedError is its runtime category under H-472. H-469 separately adds [predicate assertions](SymbolicDynamicScope.md#predicate-assertions-and-flow-refinement), with unchecked category AssertionFailedError under H-472. Both preserve normal propagation, explicit causes, defer/finally, replacement and cleanup, without mandatory throws/catch. Neither unchecked classification nor numeric/bounds settings removes an unresolved assertion check.

A valid Entity holding an Edge can fail a request for a Node view without breaching Entity's established typing contract. Such mismatch is not UnsoundRuntimeTypingError. Lack of required outside-catalog interpretation follows D-26's StaticallyUnknownTypeException boundary rather than pretending that an unknown relationship is false. Operand evaluation keeps its own failure categories and obligations. No failed assertion undoes prior effects, restores consumed ownership or rewinds allocation.

Caller argument and caller-default failures precede entry; definition-default/body failures after entry use ordinary callee cleanup under H-458. H-469 requires an established non-null fact before requesting a ref view from a ptr: `assert p is not null` may supply it, and `:` itself adds no null-to-ref runtime check. Failed predicate assertions do not create facts on a handled-failure path. Operand dereferences retain checked NPE obligations; ordinary null-access rules are unchanged. H-470 compile-time rejection cannot be caught. H-471 concrete-type specialization preserves covered runtime failure paths when the actual input is unresolved; compiling a possible failing branch is not proof that the call necessarily fails. No parser/native implementation, object-header RTTI or broad source admission follows.

## Deferred work and explicit scopes

H-319 distinguishes expression grouping from scope creation. `{ ... }` groups expressions without starting a lexical or dynamic-effect scope. [H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117) makes `{{ ... }}` equivalent to defining an ordinary niladic callable and immediately invoking it: it owns its local storage lifetimes, restores its effects on exit and runs its deferred work. Calls and module top levels remain inherent full scopes. The [loop scoping rules](#loop-scoping-rules) specify loop-control bindings, iteration effect/resource restoration and enclosing-scope defer registration.

For resource-management idioms, `defer` is the explicit counterpart to C++ destructor-based scope cleanup. The cleanup obligation belongs to a registered action in a CoD scope; it is not an implicitly selected method on every object's type. Acquisition can supply the action with `defer init(...)()`. This analogy does not add independent destruction lifetimes to regional objects, replace intrinsic whole-region release, or make copied handles/callables acquire independent release authority.

H-356 selects two forms:

| Form | Registration | Scope exit |
| --- | --- | --- |
| `defer f(args...);` | Evaluate the callable and prepare its arguments once using normal call evaluation/conversion order. | Invoke the saved callable with the prepared arguments. |
| `defer { expressions... }` | Register the block without evaluating its body. | Evaluate the block. |

H-371 prohibits variable declarations and effect installation directly in a deferred block, including through ordinary grouping and control flow. Programs needing either operation must introduce an explicit nested `{{ ... }}` scope. The deferred action itself does not acquire an implicit invocation scope. Explicit nested scopes retain their own storage, effect restoration and deferred cleanup under [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117); they do not register into the enclosing draining scope. A return in an ordinary deferred group targets its nearest enclosing callable under H-362. A nested explicit full scope introduces a callable boundary, so its returns are local to that implicit invocation.

Outside the explicit block form, the deferred operand must be a function/procedure call. In `defer init(...)();`, the callable expression for the outer call is `init(...)`: it executes now, and its returned callable is retained for invocation at scope exit. In `defer factory(a())(b());`, factory acquisition and both argument computations run at registration in ordinary call order; only the final invocation waits. Parentheses have no special timing meaning. Arbitrary expressions require the explicit block form.

Saving an argument follows its declared passing mode. A bits value is prepared now; an externally borrowed argument retains its selected target and owner/access constraints, without copying the target's contents. A callable value retains its complete environment under D-18, so replacing its source holder later does not select different deferred code. Deferred direct procedure calls keep current-binding semantics when invoked; selecting their code at registration does not implicitly request `perform` or freeze unrelated TLS bindings. Deferred code retains the surrounding function/procedure permissions and explicit-only exception-chain rules.

Registration succeeds only after the callable and arguments have been prepared. An exception during preparation does not register a partial call; already completed preparations retain their ordinary cleanup responsibilities. Skipped control-flow paths register nothing. Registered work runs in reverse registration order within its owning scope. Borrowed values must remain valid until invocation; saving a view never extends its region or lexical owner's lifetime. H-370 keeps owning arguments under E4: registration identifies and saves the selected owning reference and corresponding actual region without consuming it. The original owner remains available for legal inspection and updates. Ownership transfers only when that registered call actually enters its callee at scope exit. Invocation uses the saved identity instead of resolving the source name again. Later operations must leave that ownership obligation valid; premature transfer/release and duplicate consumption must be identified by existing ownership checks. No owner copy, implicit lifetime extension, reservation protocol or new runtime exception follows. Preparation failure registers nothing and retains the existing preparation cleanup rules.

[Go's specification](https://go.dev/ref/spec#Defer_statements) similarly saves the function value and arguments before deferring the call. CoD retains its own explicit block scopes, checked exceptions, owner checks and delayed-block form. The earlier capture-list proposal is not needed for ordinary deferred calls; additional block-capture syntax is outside this bounded implementation item.

### Nested defer declarations

[H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) gives explicit `{{ ... }}` scopes the same independent cleanup responsibility as an immediately invoked helper. Each scope registers and drains its own deferred work. A deferred body, catch or attached finally cannot append a new defer directly to its currently draining scope. Ordinary grouping, conditionals and loops do not create a new cleanup scope; an explicit full scope does.

```cod
defer {
    defer release_resource();  # Reject: same draining scope.
}

defer {
    {{
        val n <- 1u64;
        defer report(n);       # Accept: this explicit scope owns the action.
    }}                         # Its cleanup finishes before outer work continues.
}
```

The same boundary applies in attached catches/finally and bodyless deferred handlers. Entering a full scope starts a fresh ordinary registration context; entering one of its own delayed actions starts that scope's deferred context. Returning from the inner scope restores the enclosing deferred context. Wrapping the inner action's body in ordinary grouping does not permit another same-scope registration.

Separately checked named/local/anonymous callable bodies likewise have their own invocation cleanup. Their declarations and construction expressions retain ordinary surrounding access and scope requirements; inlining cannot merge independent cleanup sequences. Intrinsic owner release and effect restoration remain intact. Eager preparation of a plain deferred call retains its actual execution context: ordinary argument preparation may register an action before the outer call is registered. Failed outer preparation registers no outer action and does not erase earlier successful registrations.

Each draining sequence remains fixed while independent inner scopes may register and drain their own sequences. Escaping failures follow the surrounding ordinary/deferred boundary; nested aggregates remain opaque under the existing rule. Full FIFO/checked-pass/aggregate transport and the bounded checker/value path retain separate qualification. [The static-check draft](DeferredExecutionChecks.md) records current acceptance; old prototype acceptance is not proof of it.

### Full scopes as immediate ordinary calls

[H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117) selects full ordinary-callable equivalence everywhere, rather than cleanup-only equivalence. A standalone full scope constructs the ordinary callable inputs at the point it is reached, invokes the niladic body once, completes that invocation's cleanup and produces its ordinary result. The compiler may eliminate the materialized view/call when observations are preserved; no allocation or physical call instruction is mandated.

- Ordinary scalar val and complete callable inputs follow D-18's capture snapshot and independent invocation-local value rules. Updating that local value does not update the enclosing holder. A ref/ptr capture keeps its selected target, access and actual owner: target writes may be externally observable, but rebinding the local reference does not rebind its source slot. Explicit declared place aliasing retains its separately checked borrowed-place meaning. No implicit shared cells or owner retention.
- Every return targets the nearest callable, including this implicit callable. Its deferred/finally returns replace only its own retained result. `if returning` inspects that same invocation's state. To return from the outer callable, write an outer `return {{ return value; }};`. An inner return supplies no result-inference evidence to the outer callable.
- Ordinary value results require an explicit return. `{{ 7u64; }}` does not return 7; `{{ return 7u64; }}` does. D-24's independent inference, required-value completion and expected-signature rules apply. Returned references must outlive this invocation; a reference to its local storage cannot escape merely because invocation was immediate.
- Func/proc permissions remain those admitted at the expression's checking site; introducing the implicit callable cannot grant procedure effects in a function. Capture construction, initialization, checked failures, effect restoration and cleanup use the ordinary call rules. The full scope's `here` is its own invocation owner. Module definitions and module lifetime are not turned into implicit callable captures by this rule.
- Loop continue/feedback, generator yield and nonlocal return privileges do not cross this boundary. An inner loop or generator follows its own declared contract. Ordinary catch context and its hidden tip likewise do not become ambient across an ordinary call: bare throw/rethrow needs a catch in the implicit callable; explicitly supplied exception values follow ordinary propagation rules.

```cod
proc value_capture() -> u64 {
    val n <- 1u64;
    {{ n <- 2u64; }}
    return n;                         # 1: outer holder unchanged.
}
proc local_return() -> u64 {
    {{ return 1u64; }}
    return 2u64;                      # 2: inner result was discarded.
}
proc borrowed_target() -> u64 {
    ref n:u64 = 1u64;
    {{ n <- 2u64; }}
    return n;                         # 2: write to the captured live target.
}
proc cleanup_return() -> u64 {
    defer { {{ return 9u64; }} }      # Local result discarded.
    return 2u64;                      # Still 2 if cleanup succeeds.
}
```

Caller-context defaults use the same local-return rule, with D-10's established lookup environment. D-22's temporary receiving form expands to reception inside this ordinary implicit callable; writes to captured values stay local, while admitted borrowed-place/target writes retain ordinary observability. D-29's direct loop body remains caller-owned; an explicit full scope inside it is an ordinary nested callable.

A delayed block controls when its contained full-scope expression is reached. Thus `defer { {{ ... }} }` constructs/invokes that full scope during cleanup; it does not capture those inputs at registration. The direct full-scope block spelling `defer {{ ... }}` has that same delayed-body boundary. Plain deferred-call syntax still prepares its actual callable/arguments eagerly. Grouping alone introduces neither boundary.

**Acceptance:** the four outcomes above; local explicit returns and independent inference; per-call capture independence versus permitted target/place updates; independent nested cleanup, exact owner release and failure propagation; no outer pending-result/catch/loop control access across the callable boundary; ordinary default/receiving scope lifetimes. Full-scope source semantics are selected. D-17-DEFER-CHECKS and D-18-VALUE-CAPTURE now have their bounded implementation/value-path qualifications; the bounded full-scope implementation profile below now has its own r92 qualification. Broader capture proofs and a permanent ABI retain their separate scopes.

#### Checked failures of standalone full scopes

A standalone `{{ ... }}` implicitly declares the checked obligations that can escape its invocation. Compute them statically under ordinary exception checking. Include explicit throws at their declared types, declared failures of calls, and checked operations such as a potentially null dereference. Apply local handling and ordinary completion rules first, including new failures from a handler or `finally`. An entirely handled failure adds no obligation. Unchecked errors can propagate without a checked declaration. The caller of the implicit callable must handle or declare each remaining obligation.

```cod
using "std:exceptions" NullPointerException;

proc forward(val e:NullPointerException) -> void throws NullPointerException {
    throw e;
}

proc sample(val e:NullPointerException) -> void {
    try { {{ forward(e); }}; }
    catch (caught:const NullPointerException) {}
}
```

The call inside the braces contributes `throws NullPointerException` even though the block contains no literal `throw`. Replacing `forward(e)` with `throw e` gives the same interface. Moving the covering catch inside the block leaves its checked interface empty. Removing the only catch requires `sample` to declare the failure. An operand declared `Exception`, or a call declared `throws Exception`, contributes the general obligation even when its runtime value or implementation is known to be narrower. Nested standalone scopes compose through their immediate calls. Failures while preparing captures belong to the enclosing evaluation. Checked failures from a deferred action still need coverage within its owning defer pass.

A [trailing callback](SymbolicDynamicScope.md#trailing-callback-arguments) already has a callable parameter signature. That signature sets its allowed checked failures. An explicit helper also keeps its own ordinary declaration requirement.

### Full-scope implementation profile

[H-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#h-full-scope-implementation-20261118) qualifies this bounded r92 implementation profile and its value path. It implements the confirmed source model through ordinary callable facilities, preserving local returns and independent captures. Its value is one checked callable meaning shared by source validation, result inference, queries and native execution, reducing duplicated control and lifetime rules. The ordinary callable is a semantic boundary; a physical call or allocated view is optional.

**Scope and dependencies.** Cover standalone `{{ ... }}` in every otherwise admitted expression position, including a delayed body's full-scope expression, ordinary callable arguments and nested full scopes. Use the exact independently qualified ordinary input, result, owner, effect and cleanup contracts. The adopted D-18 value-input profile covers its scalar, intrinsic-exception and complete-callable inputs. This profile additionally covers the bounded immediate ref/ptr proof and transport below; it does not borrow authority for those inputs from D-18’s value-only adoption. Other input mechanisms require their own qualification. No general escaping borrowed-view image, declared-place capture proof, new owning carrier, aggregate transport, generator protocol or trailing-callback grammar is qualified by reusing the boundary. Caller-context defaults and temporary receiving retain their selected lookup/expansion semantics when their respective contracts are implemented; they do not get a second full-scope interpretation.

**Common checked meaning.** Establish one implicit niladic callable with the full scope's source identity and ordinary resolved inputs. Preserve definition/symbol identities, the applicable lookup environment, access qualifiers and the captured actual owners; do not reinterpret a source name using a later namesake. Capture construction is checked in the enclosing context, while the body has its own invocation owner, locals, result evidence and completion state. Use ordinary capture classification: independent values supply invocation locals, reference captures retain selected targets and local reference slots, and declared place aliasing requires its ordinary proof. Module definitions and symbolic binding cells do not become shared mutable captures. A compiler may synthesize an ordinary callable representation or use an equivalent internal representation; neither syntax-tree duplication nor a permanent ABI is required.

**Bounded immediate ref/ptr inputs.** The implicit callable itself is invoked immediately and is not published as a stored callback. For every ordinary ref/ptr free input, retain the selected target (or null), exact target type, access, slot-rebinding qualifier and statically unique actual owner at input preparation. Supply an independent invocation-local reference slot; rebinding it cannot rebind the source slot. Preserve runtime target aliasing when separate inputs select the same target. Target writes remain observable according to ordinary access checks; reading the reference identity does not grant payload-read or initialization permission.

The compiler must prove that every borrowed actual owner and selected target remain valid across preparation, invocation and cleanup. Well-nested lexical ownership supplies that proof for ordinary enclosing lexical targets; regional targets require the ordinary live borrowed-owner evidence. Consuming/replacing an owner cannot coexist with outstanding uses, including deferred uses and pending borrowed results. A retained reference result needs its separate ordinary post-call liveness proof; immediacy does not make a local target escape safely. Nested calls, deferred work and admitted explicit place operations must preserve these dependencies through their ordinary contracts. Unknown/multiple owners or an unproved lifetime reject.

Transport may reuse ordinary borrowed ref/ptr parameters plus their owner information. It must preserve nullability, access and the current owner-plus-locator resolution contract from [D-28](RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations). Share proven common owner information where possible; do not replace actual owner identity with a cached base address or add per-reference owning responsibility. No new escaping environment format, runtime borrow registry, owner retention or lifetime extension is required. Eliding an intermediate view is permitted only while preserving capture timing, local slot independence and ordinary cleanup. Generic borrowed targets are admitted only with the same type/layout/owner evidence required by the corresponding ordinary parameter.

The new proof scope ends at the immediate call and its ordinary result checks. A returned or stored callable that retains one of these borrowed inputs still requires the independently qualified escaping-view proof/transport; this profile cannot establish it merely because the intermediate invocation was immediate. Explicit declared place aliasing is also distinct from the reference-slot snapshots covered here. The selected source behavior of these broader forms is unchanged, and their implementation gaps remain explicit.

```cod
proc local_reference_slot() -> u64 {
    ref a:u64 = 1u64;
    ref b:u64 = 10u64;
    ref p:u64 = a;
    {{
        p = b;      # Rebind the inner reference slot only.
        p <- 20u64; # Write b's selected target.
    }}
    assert p is a;
    return a + b;   # 21.
}
```

**Checking and queries.** At the immediate invocation apply the [standalone checked interface](#checked-failures-of-standalone-full-scopes), including failures of nested calls. Body validation, return inference and query-only inspection must agree on this boundary. An explicit inner return contributes to the implicit callable's result, not the enclosing callable's. A tail expression is not an implicit return. Expected-signature and missing-return handling follow D-24 and the ordinary callable rules. Queries may establish the result and capture requirements without invoking the body, reading captures, allocating inputs or registering cleanup. Func/proc permission, source well-formedness, readonly access, initialization and owner-escape checks still apply; the scope boundary grants no permission to accept an otherwise invalid source program.

For `typeof({{ ... }})`, the queried type is the result of the implicit invocation. Establish its inferred body’s source and access validity while deriving that result. A write to a `const val` capture rejects even when the return type is independently obvious. Dynamic procedure behavior inside a full scope in a `func` also rejects. Type-only inspection creates no executable demand, capture read, TLS lookup, or deferred registration. Calls with established signatures supply ordinary declaration evidence. In particular, querying through a separately annotated anonymous callable argument keeps its existing signature-only treatment.

**Runtime and completion.** When the expression is reached, prepare ordinary inputs once, acquire the complete required independent values before exposing writable invocation locals, and invoke once. Failed preparation releases only its successfully acquired prefix under ordinary cleanup; it enters no partially prepared body and rolls back no completed outer effect. The body uses its own `return`/`if returning`, catch context, loop control and cleanup state. On success, prepare its ordinary result before destroying local dependencies and deliver only after its cleanup succeeds. On failure, propagate through the ordinary call boundary. Discarded results, local complete values and owned resources receive their usual exact cleanup. A reference to invocation-local storage cannot escape; an admitted returned callable may retain independent values or independently proved surviving borrows under D-18. No capture or `here` owner is extended merely because the call is immediate.

During enclosing cleanup, the inner callable still has independent completion: an inner return cannot replace the enclosing retained result. Its escaping failure has the same effect as a separately defined helper failing at that call site. Delayed block forms construct the full scope's inputs when cleanup reaches the expression; eager preparation for plain deferred-call syntax remains unchanged. Inlining, shared code, immutable backing or elision must preserve those observations and owner boundaries.

```cod
proc independent_completion() -> u64 {
    val n <- 1u64;
    val inner <- {{
        defer { n <- 3u64; }
        return n;
    }};
    return n + inner; # 2: outer n stays 1; inner bits were captured before cleanup.
}
```

**Migration and acceptance.** Replace lexical-block outer writes, outer returns and tail-result acceptance in affected full-scope paths; do not leave them as a fallback for inputs whose ordinary callable preparation is unproved. Unsupported dependencies require an accurate implementation-gap diagnostic, not execution under the old meaning. Report the actual migration cost under AGENTS and remove obsolete checked/native paths as replacement coverage is established. Existing tests may remain only when their expectations agree with the current source contract; historical observations stay in Git. No new source-level feature flag or compatibility dialect is selected.

| Boundary | Required evidence |
| --- | --- |
| Value and reference inputs | The preceding four source examples, independent_completion and local_reference_slot have the stated outcomes. Captured values and reference slots are invocation-local; admitted target/place writes preserve their ordinary permissions. Readonly violations and premature capture reads reject. |
| Immediate borrow proof and transport | Accept non-null and nullable inputs, same-target inputs, distinct proven actual owners and references forwarded through ordinary borrowed parameters. Preserve null/identity tests, readonly access, source-slot independence and target writes through inner cleanup. Reject wrong/unknown owners, invalid lifetime or owner consumption with outstanding uses. A deferred borrowed use remains valid through the full invocation; a returned/stored callable gains no escaping-view proof. Cover both general and specialized transport. |
| Results and queries | Local explicit return, void fallthrough, ordinary required-value failure, independent inference and discarded results agree between checking/query and execution. Query inspection produces no capture/body effects. |
| Cleanup and outer control | Nested cleanup completes exactly once; inner return does not replace outer return. Independently admitted if-returning forms inspect only the inner state. An escaping inner failure propagates as an ordinary call failure. No outer loop/yield/catch context leaks inward. |
| Owners and complete values | Here denotes the inner invocation; returned local references reject. Admitted complete callable results retain their ordinary independent values and only proved live borrowed dependencies. Preparation, entry, body, result and cleanup failures release acquired responsibility exactly once. |
| Placement and timing | Nested expressions, argument evaluation and delayed/eager defer positions retain once-only evaluation and the selected capture timing. Source and query checking retain identical callable boundaries even for unused or generic bodies. |
| Native consistency and cost | General and specialized execution agree with an equivalent ordinary niladic callable. Track additional calls, storage and live owned values; demonstrate ordinary read-only input elision where valid. No allocation per invocation, materialized-view requirement or new permanent ABI is prescribed. |

This profile does not independently implement or qualify the complete deferred FIFO/aggregation runtime, return-inspection mechanism, broader aggregate/init/receiving proofs, caller-default or callback syntax, module startup, generator protocols, pure/TCO or D-16 fractions. Where one of those mechanisms is involved, its existing selected semantics remain mandatory and its qualification remains separate. Full ordinary-callable source equivalence is not weakened by a temporary implementation gap. An actual engineering obstacle that would require weaker behavior must be discussed as a design trade-off before changing that behavior.

### Module-level deferred actions

[Module shutdown](ModuleWorlds.md#cross-module-shutdown-order) follows H-501: module-level actions across the selected program execute in reverse order of actual successful registration, preserving each action's module context and valid owners. Registration A1 → B1 → A2 therefore drains A2 → B1 → A1, even though A1 and A2 belong to the same module. Entry registrations participate in the same order; ordinary nested full scopes retain their local cleanup boundaries.

Startup failure prevents later initialization/entry work and unwinds only established registrations/resources. Preparation failure registers no partial action; skipped paths register nothing. Apply the same ordinary-action/deferred-exception-queue rules below across the module-level sequence. Each entry retains its module's selected context; exception matching retains semantic type identity. Original startup/body exceptions are outside this deferred-error queue and cannot be handled by a bodyless deferred catch. Cleanup does not resume aborted startup, restore transferred owners or roll back initialization effects. Residual deferred errors, final process completion and intrinsic owner/effect restoration must follow the resolved boundary policy; no ordinary module-local catch acquires ambient cross-module coverage.

### Exceptions between deferred actions

The forms have distinct exception coverage under H-503/H-505:

| Form | Execution and coverage |
| --- | --- |
| `defer { body(); } catch (e:const E) { recover(e); } finally { cleanup(); }` | Run this body's own work once at its reverse-registration position, independently of errors from other actions. Its catches handle only its body's failure. Its finally runs once under ordinary try/catch/finally rules. |
| `defer catch (e:const E) { handle(e); } finally { finish(); }` | At this registration's position, inspect incoming deferred errors oldest first and invoke a matching catch for each applicable error. Leave unmatched errors pending. Optional finally runs once after that visit, including when the queue is empty or no type matches. |

H-541 selects a FIFO pending-deferred-exception queue. Ordinary actions and bodyless-handler entries are visited in reverse actual successful registration order. Every exception escaping deferred execution is appended at the queue's tail. After an ordinary action finishes or fails, the walk proceeds to the next list entry. A bodyless handler processes its incoming entries from oldest to newest when its list position is reached, without jumping ahead over intervening actions. One visit may execute its catch several times, with a fresh catch-variable binding for each selected error. Returning from an invocation retains this queued timing and continued handling under H-539/H-540. H-507's optional finally executes once per entry. The same rules apply to H-501's global module-level list.

```cod
# Helpers handle their checked failures; error_one/error_two are unchecked Error values.
defer catch (e:const Exception) { report(e); }  # H
      finally { close_log(); }
defer { throw error_one; }                    # D1
defer { throw error_two; }                    # D2
# Registration: H, D1, D2.
# Drain: D2 appends error_two; D1 appends error_one;
# H handles error_two, then error_one; close_log runs once.
```

A matching invocation consumes its selected pending occurrence. Normal handling removes it without replacement; an escaping handler failure appends an outgoing occurrence at the tail. Nonmatching entries remain pending in their relative order. A previously pending error neither skips a bodied action nor enters its attached catches; successful execution or local recovery cannot silently handle an unrelated error. Catch clauses cannot catch failures from their own handlers or attached finally. In particular, `defer catch (e:const Error) { throw e; } finally { cleanup(); }` must not repeatedly recatch its own propagation. Only entries pending when a handler visit begins are eligible for that visit; outgoing failures wait for later list entries. This is a logical eligibility boundary, not a requirement to copy the queue or exception objects, and does not guarantee termination of user handler code.

**Forwarding and replacement.** Throwing the selected value appends a new propagation occurrence of the same exception; it does not create an exception or refresh its trace. Constructor-form rethrow creates an exception under its own contract, then appends that escaping failure. Failure while constructing a replacement follows the same handler-failure rule; no automatic copy of the selected input or cause link is added. Separate occurrences are not deduplicated merely because they refer to the same object.

```cod
defer catch (e:const Error) { log(e); }    # Observer.
defer catch (e:const Error) { throw e; }  # Forwarder.
defer { log("A"); throw CloseError(); }
defer { log("B"); throw FlushError(); }
# B → A → forward FlushError → forward CloseError
#       → observe FlushError → observe CloseError.
```

An all-matching forwarder preserves the incoming order through any number of forwarding entries. Selective forwarding has a distinct, intentional consequence: from incoming [A, B, C], a handler that forwards/replaces A and C but does not match B leaves [B, A′, C′]. B retains its earlier pending position; A′ and C′ are later propagation occurrences. A failure escaping the once-only finally is appended after those handler outputs. Later ordinary deferred failures likewise join the tail, without restarting a retired handler.

**Finally and scope.** The optional finally belongs to the list entry, not to each exception handled. It executes once after the entry's scan, even with zero matches; a bodied action likewise retains one ordinary finally execution. The bodyless entry is finished after this visit, not retained as a look-ahead handler for older ordinary actions. Normal nested/full scopes still complete their own required cleanup before an escaping exception reaches the enclosing deferred invocation boundary. H-371's explicit-scope requirements remain: per-invocation declarations/effect installation require a nested full scope rather than an implicit handler-body allocation scope.

**Which failures enter the queue.** Only exceptions escaping deferred execution belong here: ordinary deferred calls/blocks, their attached catch/finally, and bodyless-handler execution/finally. A failure in a helper invoked by that execution is still part of the deferred invocation. An ordinary scope-body exception or a failure preparing a defer registration during ordinary body execution remains on its normal propagation path; a deferred catch cannot handle it. [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) rejects registrations into that same draining scope. A helper or explicit inner full scope can prepare its own independent registrations; any failure escaping the helper follows the enclosing invocation's ordinary deferred failure path. Handling every deferred error does not by itself handle an original body/startup error. Ordinary try/catch handles those errors; D-17-RETURN-STATE separately permits explicit callable return to replace their completion.

**Context and ownership.** Use actual successful registrations, not textual existence. A skipped or failed registration supplies no entry or coverage. Preserve the selected module/callable context, current-versus-captured binding behavior and semantic error type identity on every catch invocation. Registration captures are not recreated for each error. Mutations and transfers performed by one invocation constrain subsequent invocations; an unproved second consumption of the same owner rejects. Each error value and catch binding must remain valid through its admitted use. Explicit nested scopes can supply invocation-local work and normal release. No owner resurrection, automatic capture copy, lifetime extension or implicit cause chain follows.

A handler has ordinary local exception control for its own work, but its escaping failure must not re-enter that handler or its sibling catch clauses during the same visit. H-510–H-515 fixes the construction/propagation distinction; H-541 orders outgoing occurrences by escape time at the queue tail. No unrelated unhandled error may be discarded merely because another error is handled.

**Checked failures from deferred work.** H-550/H-551 requires static proof that no checked deferred failure remains when its owning defer-handler pass completes. A checked failure may escape one deferred action into the FIFO and be handled by an applicable later-visited `defer catch`; that is user handling. Ordinary catches within the action and its attached catches also count under their normal coverage rules. The requirement is on the completed pass, not on each individual action boundary. An enclosing `throws FlushException` declaration cannot authorize an unhandled checked deferred failure.

```cod
# flush() declares throws FlushException; recover() handles it without rethrowing.
proc recovered_work() {
    defer catch (e:const FlushException) { recover(e); }
    defer { flush(); }
}
# Accept: flush runs first; its checked failure is then handled.

proc locally_recovered_work() {
    defer { flush(); }
          catch (e:const FlushException) { recover(e); }
}
# Accept: the attached catch handles this action's own failure.

proc unhandled_work() throws FlushException {
    defer { flush(); }
}
# Reject: a throws declaration cannot discharge deferred-pass coverage.

proc wrong_order() {
    defer { flush(); }
    defer catch (e:const FlushException) { recover(e); }
}
# Reject: the handler retires before flush runs.
```

Coverage follows actual successful registration, reverse action order and incoming-only FIFO visits on every admitted path. A skipped or failed registration supplies no handler; source text alone does not prove coverage. The proof must include handler outputs, replacement/rethrow and attached finally: any checked outgoing failure must itself be covered before the pass finishes. A matched input is handled according to the existing occurrence-consumption rule, even when its handler subsequently raises an unchecked replacement. No return removes unrelated pending failures. Permitted conditional and repeated pre-cleanup registration require sufficient existing control/owner/initialization evidence; unproved coverage rejects conservatively, without a runtime check that permits checked residuals.

H-525's declared-type rule still applies. A call declared throws E supplies that obligation even if its current body is believed not to fail. Likewise `throw e` for a catch binding declared Exception is checked, even when the current queue entries happen to be errors; use an Error-typed catch for intended unchecked forwarding or provide further covering handling. No actual-value guess waives the written contract. The same residual rule applies to deferred calls/blocks, bodied and bodyless handlers, attached finally, nested full scopes and the application-wide module cleanup pass. Each owning pass must satisfy it before any aggregate crosses that boundary.

This does not ban checked failures from ordinary code that prepares a registration: callable/argument evaluation happens now and retains ordinary try/catch/throws rules. If that preparation instead runs inside deferred execution, its escaping failure is part of the active pass and needs that pass's coverage. Ordinary callable-body exceptions remain outside this queue. After checked deferred failures are covered, they add no throws requirement to the enclosing callable. Any independently written throws declaration still binds callers under H-525.

**Residual deferred failures.** H-509 requires a single unchecked `UnhandledDeferError` containing all exceptions left after the defer list has been drained, exposed through its exceptions field. H-550/H-551 requires every such member to be unchecked; checked failures may exist during the pass but must be handled before this boundary. H-537 fixes its timing: complete the deferred-handler pass first; only then construct the residual aggregate. It is not another input to that completed pass and cannot be recaught by one of its retired defer-catch entries. An empty queue supplies no such failure. Use the same aggregate boundary for any nonempty residual queue, including one element; do not select a different public type based on the count. Preserve FIFO order and complete exception values, including any existing explicit causes.

The exceptions field preserves the remaining FIFO order, including distinct occurrences of the same exception identity. A callable may propagate UnhandledDeferError through the ordinary exception path, and an ordinary try/catch may catch that aggregate. Ordinary type matching sees the container; membership alone is neither a cause link nor implicit member-wise catch dispatch. The aggregate carries no escaping checked deferred-member obligation. Ordinary catch sees this unchecked container and does not dispatch to its members. If a deferred invocation calls another callable that propagates an UnhandledDeferError, that container is one unchecked error at the outer boundary; do not flatten it or rerun its completed inner pass. The callee's separately declared ordinary-body checked failures still need coverage in the outer pass. Collection API/layout and owner transport remain separately scoped. H-538 specifies the body-failure combination below. No native ABI or permission to allocate while reporting OOM is selected.

**Ordinary body failure retained separately.** H-538 adds the independently readonly body_exception field to UnhandledDeferError. When an ordinary body exception is still pending after the deferred-handler pass and deferred failures also remain, the aggregate retains that body exception in body_exception and only the residual deferred failures in exceptions. The body exception never enters the deferred-error queue merely to populate this field and is not offered to those deferred handlers. It may be checked under its ordinary body/throws contract: H-550/H-551 constrains the residual exceptions collection, not this independently retained body field or an error's explicit cause. Retaining such diagnostic information does not make it a deferred queue member or an implicit catch target.

| Pending ordinary body exception | Residual deferred failures | Completion |
| --- | --- | --- |
| Present | Nonempty | Propagate UnhandledDeferError with that existing body exception in readonly body_exception and the residual failures in exceptions. |
| Absent | Nonempty | Propagate UnhandledDeferError with body_exception absent and the residual failures in exceptions. |
| Present | Empty | Propagate the original body exception unchanged; create no aggregate. |
| Absent | Empty | Continue the retained normal/return completion under the ordinary completion rules. |

Retain the original exception value, identity, creation trace, existing wrapper chain and explicit causes. The new field does not unwrap or reconstruct it, make it an ordinary catch's implicit match target, or establish a new cause link between the aggregate, the body exception and deferred members. Aggregate construction does not mutate either retained failure. The association is readonly after publication; absence has a semantic meaning here without selecting a nullable-pointer/optional-value spelling, owner encoding or payload ABI.

```cod
# close_resource() may throw CloseError.
proc work() throws WorkException {
    defer { close_resource(); }
    throw WorkException();
}
# Both fail: UnhandledDeferError retains WorkException in body_exception
# and CloseError in exceptions. No automatic cause relationship is added.
# If a covering deferred handler handles CloseError, WorkException escapes unchanged.
```

The same distinction applies to a pending ordinary module/startup failure during the global cleanup drain: preserve it separately if aggregation is needed, and never resume aborted startup. H-542 permits an explicit callable return during cleanup to replace its ordinary body exception; the return-state section below defines successful preparation and ordinary cancellation. Preservation applies to the body exception that remains pending at the aggregate boundary, not to every exception already replaced by explicit completion control. Module startup has no enclosing callable return target to recover through this mechanism. Exception backing must remain valid, including through OOM handling; the field decision adds no permission for unbounded emergency allocation or automatic graph copying.


**Return results during the drain.** H-539 confirms that a later executed return may replace a previously retained return result and that deferred work continues. H-540 preserves list-position handling, and H-541 selects FIFO within that visit: ordinary deferred failures accumulate until the bodyless handler's list position is reached. Returning from one matching invocation ends that invocation, handles its selected error on successful completion, and updates the callable's retained result. It does not stop the remaining incoming-error processing for that handler entry or the remaining deferred entries.

```cod
# CloseError and FlushError are unchecked Error types; log is illustrative.
proc run() -> u64 {
    defer catch (e:const Error) {
        log(e);
        return 0u64;
    }
    defer { log("A"); throw CloseError(); }
    defer { log("B"); throw FlushError(); }
    return 1u64;
}
# B → A → catch FlushError → catch CloseError; final return result: 0u64.
```

The first handler return replaces the body's retained 1u64; the second handler invocation still runs and may replace that result again. If two completed returns produce different values, the later one supplies the retained result. An invocation without a return leaves the previous result intact. Return-expression evaluation, result snapshots, owner constraints and release of abandoned owning results follow the existing return contract; an escaping failure during return preparation is not a successful handler return.

The entry's attached finally still executes once after its incoming-error processing, not after each return. Remaining deferred entries run normally. A later deferred failure is queued for applicable handler entries not yet visited; a completed handler entry is not restarted. No return implicitly handles a nonmatching queued error or skips its later coverage/aggregate boundary. If residual deferred failures remain, the aggregate boundary still applies instead of delivering a normal result. H-542 permits a successful explicit return to replace an independently pending ordinary body exception; H-538 retains that exception only when it remains pending at aggregation.

### Return-state inspection and replacement

H-542–H-545 selects `if returning (...) { ... } else { ... }` for inspecting the enclosing callable invocation's established return result and choosing an explicit replacement. H-546/H-547 supplies its ordinary-exception cancellation boundary. H-548 delegates the remaining bounded choices by implementation simplicity and semantic coherence; the profile below is the agent's selection under that delegation. This is a source design for current func/proc, not implemented syntax or a new ambient caller/callable-value context. A separately defined helper or nested callable observes its own invocation, not its caller's result. A module has no callable return target. Existing function/procedure effects and scope/capture restrictions still apply; future pure-callable rules are outside this profile.

Establish a return only after its operand, conversion and required result preparation succeed. Retain the checked result mode, value or selected target, actual owner/access and necessary type evidence. A later successful return replaces that result and can replace the ordinary body exception with normal return completion; remaining cleanup still runs. This includes a return in the else branch and follows the same explicit-return rule elsewhere in cleanup. Merely inspecting or binding a result does not recover an exception.

| Event | Result state and failure handling |
| --- | --- |
| Successful explicit return | Install the prepared result; replace any previous result and the callable's ordinary body exception. Preserve unrelated deferred failures. |
| Ordinary exception escapes and cancels a return it was completing | Invalidate that return before propagation continues outside the cancelling construct. A later ordinary catch does not revive it. |
| A local failure is recovered within finally and that finally finishes normally | Preserve the suspended surrounding completion and its return result. |
| Failure escapes a deferred entry | Retain the established result and append the failure to the FIFO queue under H-541. |
| Return preparation fails | Install no replacement. Its ordinary or deferred failure path then determines cancellation or queueing as above. |

This scoped cancellation rule is the implementation-cost choice delegated by H-547. Normal completion of a finally preserves its suspended surrounding completion; no global every-throw invalidation signal must cross nested saved completions. An escaping ordinary finally failure cancels the return that finally was completing. Required inner cleanup still precedes disposal of an abandoned owning result, which releases exactly once. Clearing refers to semantic availability as well as owned backing, not merely a diagnostic flag.

```cod
proc recovered_report() -> u64 {
    try { return 7u64; }
    finally {
        try { optional_report(); }
        catch (e:const ReportError) { recover(e); }
    }
}
# Reporting fails, recovery succeeds, finally completes normally: return 7.

proc cancelled_return() -> u64 {
    try {
        try { return 7u64; }
        finally { throw WorkError(); }
    } catch (e:const WorkError) { report(e); }
    # The return was cleared. A later if returning () is false.
    # Required-value fallthrough still needs another explicit return.
}
```

These examples assume the named helpers/error types have their indicated contracts. Source-level throws, failing calls, checks and construction all use the same propagation boundaries; source spelling does not determine cancellation. A locally handled helper failure supplies no escaping exception to cancel its caller's return. A deferred invocation's helper failure remains a deferred failure when it escapes that entry. This selects no new nested-aggregate dispatch policy.

**No-value completion.** A successful explicit `return;` establishes a return with no payload and matches `if returning ()`. A normal void fallthrough remains valid but does not establish that marker: cleanup on that path observes false unless an earlier cleanup action explicitly returns. No pattern binds a void payload; an explicit void value binding rejects. Ordinary value/ref/ptr filters do not match a no-payload return. The existing required-value fallthrough rule remains; cleanup can supply a valid explicit return before final delivery. Neither a compiler-generated epilogue nor an optimization may fabricate a source-level return observation.

```cod
proc explicit_exit() {
    defer { if returning () { record_explicit_exit(); } }
    return;  # The deferred condition is true.
}

proc normal_exit() {
    defer { if returning () { record_explicit_exit(); } }
    # Normal void fallthrough: the deferred condition is false.
}
```

The helper is an ordinary available procedure. Inspection is permitted wherever the current func/proc body permits an ordinary if, including cleanup. Before any retained return exists it is false; an omitted else simply performs no branch work. An ordinary resumed path after cancellation has no historical return to inspect.

**Matching and binding.** Inspect the currently retained result when the condition executes. Try branches in source order; the first matching branch runs. A failed filter leaves the result and failure channels unchanged. Type matching uses the prepared result after its ordinary return conversion, without an additional numeric conversion or assertion exception. For example, a declared u64 result established by return 7u32 has type u64 for matching. Ordinary nominal membership and retained type evidence govern other matches; no target type can be invented from null. An unsupported/unproved matching operation cannot silently pretend that the pattern was false.

| Form | Required match / binding |
| --- | --- |
| `if returning ()` | An established return, including explicit return with no payload or an owning result; no result binding. |
| `if returning (in o)` | A borrowed ref/ptr result with existing owner o; no value binding, new owner declaration or implicit borrow of an owning result. |
| `if returning (val rtn)` | A val result, bound as an independent value under ordinary val rules. |
| `if returning (val rtn:T)` | The val mode and type T both match; bind rtn. |
| `if returning (ref rtn:T in o)` | The reference mode, type/access and existing owner match; retain the selected target. |
| `if returning (ptr rtn:T in o)` | The nullable-pointer mode, type/access and existing owner match; preserve nullability. |
| `if returning (rtn:T)` | Use the complete result annotation T's ordinary category/mode and access rules, then bind rtn accordingly. |

**Bounded pattern grammar.** Parentheses contain either nothing, `in owner`, or one ordinary binding declarator with an explicit val/ref/ptr mode or complete type annotation. Thus `(rtn:u64)` and `(val rtn:u64)` have the same scalar value meaning; `(rtn:u64 in o)` follows the ordinary ref default. Existing const/access qualifiers keep their ordinary meaning. A mode-only binding such as `(val rtn)` infers its type from the already checked callable result contract. An omitted ref/ptr owner must likewise be statically unique under existing inference; otherwise require an explicit owner or reject. A bare `(rtn)`, initializer, default, arbitrary predicate, destructuring or ownership-consuming binder is outside this grammar and rejects. Additional conditions use an ordinary nested if. H-549 confirms that if returning only consumes the already inferred or declared result type. Pattern annotations supply no return-type-inference evidence and do not repair a circular or unresolved callable result contract.

A complete type annotation retains its ordinary mode/access/owner meaning; matching never dereferences merely to copy a borrowed scalar into val. Ref and ptr remain distinct modes even when a ptr currently selects a non-null target. Existing aliases resolve by identity. Owner tests use actual owner identity and existing shared owner evidence, not identifier spelling; they grant neither ownership nor additional access. The returned reference must already satisfy the static unique-owner rule. A typed null can supply its retained declared type and owner; absence alone supplies no narrower concrete target evidence. Owner-only matching does not establish non-nullness. Invalid forms such as val rtn:T in o or an unavailable owner reject at compile time; only well-formed nonmatches fall through. Statically established matches/nonmatches may be folded while preserving branch effects; unknown evidence must not be treated as a nonmatch.

```cod
if returning (val rtn:u64) {
    inspect_number(rtn);
} else if returning (in o) {
    inspect_owner_case();
} else if returning () {
    inspect_other_return();
} else {
    return 0u64;  # No retained return; may replace a body exception.
}
```

Else means that this pattern failed, including a type/mode/owner mismatch; only failure of the unrestricted () test establishes absence of a retained return. Pattern bindings exist in the matching branch and do not expose the hidden result slot as an assignment target. Changing a val binding or rebinding a ref/ptr binding does not replace the retained result; ordinary permitted target writes remain observable. Matching itself performs no user conversion or ownership transfer. After matching, binding uses existing storage/copy/borrow rules. Matching adds no allocation, user conversion, target dereference or runtime assertion; ordinary scalar copying and ref/ptr binding add no special fallible operation. A complete callable value can require storage for an independent captured environment: if that ordinary allocation fails, binding raises the existing unchecked OutOfMemoryError before entering the branch. A type/mode/owner nonmatch simply tries the next branch; materialization failure instead follows exception control, with no branch retry. Invalid or unproved access/owner/lifetime remains a compile-time rejection, not a runtime binding check.

Scalar bindings copy independent bits. A complete callable result bound through its ordinary callable type copies the complete value/environment under D-18, not just a header or code pointer; it is not reclassified as scalar bits or recaptured from its definition. Existing allocation failure, access, capture and borrowed-owner rules apply. Replacing the retained result does not mutate these independent copies. A ref/ptr binding retains its selected target and actual owner; replacing the retained reference does not rebind that local variable. Later reads and captures still require the target owner and binding storage to be valid. An unproved lifetime rejects: retaining a pattern binding cannot extend a result owner's lifetime, promote a lexical cell or justify borrowing a hidden owning result. Required branch cleanup precedes disposal of storage it legitimately uses; replacement preparation completes before abandoning the old result, with each owning backing released exactly once. No alias to the private result slot or new general lifetime mechanism is introduced.

```cod
proc inspect_result(f:proc(u64)->u64) -> proc(u64)->u64 {
    defer {
        if returning (saved:proc(u64)->u64) {
            saved(0u64);
        }
    }
    return f;
}
```

Assume f contains a runtime-sized captured environment. The return first prepares its complete result; the later matched binding makes an independent complete value saved. If the latter requires allocation and that allocation fails, saved's invocation is never reached. This failure escapes the deferred entry into its FIFO channel; it neither installs a new return nor tries another branch. Failure of saved's body after successful binding is a separate ordinary invocation failure. () would avoid the binding copy altogether. Stack allocation or copy elimination may avoid allocation where existing optimization rules permit; the example specifies a possible materialization failure, not mandatory allocation for every callable match. It is design acceptance pseudocode until if-returning source support exists.

**Owning-result limit.** The initial profile accepts `()` for any otherwise valid result, including an owned region. It provides no capture, extraction, consumption or borrowed view of the hidden owning result. An owned binding pattern rejects; ordinary ref/ptr patterns and `in o` do not reinterpret an owning result as a borrow. This conservative limit avoids introducing a second owning handle or a new borrow protocol across result replacement. A future extension must separately establish that protocol; it is not needed to implement this profile.

**Boundaries and acceptance.** The bounded grammar, no-value behavior and reuse of ordinary binding lifetimes above close this source profile's remaining choices under H-548. Acceptance covers explicit void return versus normal fallthrough; all admitted patterns and rejected grammar forms; normal replacement and else recovery; successful first/repeated returns; failed first preparation and failed replacement through both failure channels; local recovery preserving the suspended return; escaping cancellation before outer catch/cleanup; typed/mode/owner nonmatches and ordered fallback; typed null without fabricated subtype evidence; independent bits and complete callable bindings; retained borrowed-target access; binding failure; capture/lifetime rejection; owning inspection without capture; exact result release and unchanged checked obligations. Unrelated queued failures still prevent normal delivery. Aggregate ownership/body-field integration remains separate; [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) distinguishes forbidden same-drain registration from independent explicit scopes/helpers. No new permanent representation, regional graph copy, general ADT value category or pure-callable policy is selected. D-16 conversion, units, validity and future fraction requirements remain unchanged. Full value-path qualification and source implementation retain the registry's separate evidence requirements.


**Standalone finalization.** No separate `defer finally { ... }` form is needed: `defer { ... }` already registers unconditional delayed work. Attached finally has a distinct purpose on a bodyless handler: it executes once after the scan rather than once per matching exception. Keep explicit grouping/full-scope rules unchanged.

### Remaining deferred-queue design scope

Aggregate storage and ordinary-body integration remain separately scoped. Checked deferred-member coverage is fixed above: only unchecked residual members may cross the completed pass. An incoming nested aggregate is one opaque unchecked member; there is no flattening or inner-pass replay. H-538 separately retains a pending ordinary body/startup exception in readonly body_exception, without an automatic cause. Its ordinary declared coverage remains distinct from the deferred-member rule; inspection/transport of that field and the complete allocation-free aggregate ownership protocol still need their scoped design evidence. Explicit callable return replacement follows D-17-RETURN-STATE; module startup remains aborted. H-505 excludes ordinary body errors from bodyless deferred handlers. [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) keeps each draining sequence fixed while explicit inner scopes and permitted helpers retain independent registration and cleanup.

**Acceptance and implementation boundary.** Cover D2/D1/H(error_two)/H(error_one)/finally order; all-matching forwarding through multiple entries; selective [A, B, C] → [B, A′, C′] forwarding; replacement/construction failures at the tail; repeated identities without deduplication; incoming-only visits; residual FIFO order; zero and multiple matching errors; stable unmatched entries; independent bodied execution/local catches; handler rethrow without self-reentry; finally exactly once; failed/skipped registrations; exclusion of ordinary body/startup errors; module contexts and semantic identities; repeated mutation/owner consumption; H-550/H-551's completed-pass checked coverage, accepted covering deferred/local catches, rejected throws-only and wrong-order cases, handler/finally outgoing failures, declared-root conservatism and partial/failed registrations; H-537's aggregate-after-pass and ordinary-catch boundary with unchecked-only members and opaque nested aggregates; all four H-538 body/deferred combinations, readonly association, retained identity/trace/chain and no automatic cause; H-539/H-540/H-541 queued FIFO order through repeated returns, later-result replacement, continued entries, once-only finally and preserved nonmatching failures; D-17-RETURN-STATE cancellation/recovery and filter acceptance; H-FULL-SCOPE-DEFER-20261117 rejection of same-drain registration through ordinary grouping/control flow; accepted independent full-scope/helper cleanup and ordinary pre-cleanup registrations; and each resolved policy above on normal/exceptional exit. The existing single-pending-error/body-skipping implementation does not establish this queue behavior. Handler effects need repeated-invocation flow checks even though each list entry is visited once. Exception propagation/cleanup must retain allocation-free failure delivery: a pending-queue insertion cannot depend on a fresh allocation from an already exhausted allocator. Reusing completed action storage or preparing capacity at registration are possible mechanisms, not selected ABIs. The required UnhandledDeferError collection and proposed rethrow causes need dedicated ownership/allocation analysis; current bounded intrinsic-exception evidence does not establish them. General user exception payload graphs remain separate.

### Explicit callable returns

H-361 requires proc/func brace bodies to provide returned values with `return`. A value supplied from finally likewise requires explicit return syntax. A callable that finishes without a required explicit return raises an unchecked exception at runtime; the compiler should diagnose statically detectable missing-return paths through errors or warnings. The intrinsic vocabulary names this failure `MissingReturnError`; it carries the existing bounded system-exception details. Reachable required-value fallthrough produces `-Wmissing-return`, which `-Werror=missing-return` or `--warnings-as-errors` can reject. Static diagnosis does not turn this failure into a checked throws obligation.

H-362 confirms that `return value` inside a deferred catch or finally targets the nearest enclosing func/proc and remaining deferred actions still run. An explicit full scope is an ordinary implicit callable under H-FULL-SCOPE-CALLABLE-20261117; returns and return inspection inside it target that invocation, including when it is nested in outer cleanup. H-542 preserves explicit replacement of an ordinary body exception; H-546/H-547 scopes ordinary cancellation and locally recovered finally work. A handler without return preserves a retained normal result. H-539/H-540/H-541 preserves FIFO handling at each entry and continued deferred execution while later returns replace retained results; unrelated queued failures are not erased. If no required result is supplied before callable completion, the missing-return rule applies. Void callables may complete normally without `return;`; only explicit return establishes the no-payload marker observed by `if returning ()`.

H-363 selects the `pure` declaration keyword for the const-function form whose body does not require explicit return: the final body expression supplies its result. Its boundary preserves readonly pre-existing external state, allows private mutation and constrains nested helper calls as described in [D-19](ConstFunctionsAndTailCalls.md). Pure-function implementation and TCO remain deferred by explicit user selection. No expression-bodied pure declaration using `=>` is selected; D-10's trailing callback parameter header uses that token in a distinct position. Ordinary single-brace grouping values remain separate from callable control-flow returns, and every return retains existing result-type, owner/access, storage and cleanup requirements. The executable proc/func fixtures use explicit returns; historical native checkpoint evidence retains its recorded source scope.

### Implementation availability

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Prepared values retain their independent snapshots; borrowed arguments retain the selected target and actual owner. Payload reads occur at invocation. Deferred registration contributes effects only after successful preparation; queue choices, failure paths and completion determine whether initialization is established. These are qualified behavioral obligations, not a mandated solver or runtime representation.

### Callable construction, installation and cleanup

H-380 cancels only affected dependency initialization TLS writes. Operand evaluation and callable construction retain their ordinary effects and exceptional exits; a produced value that is not installed follows existing temporary cleanup. No canceled binding is published or later restored. Procedure-local installations retain normal scoped cleanup. This supplies no permission to erase observable allocation failures or unrelated effects merely to shrink the final image.

H-372–H-376 distinguishes compile-time family registration from runtime callable materialization, TLS installation and invocation. A module definition is not a runtime resource acquisition to release during cleanup. Repeated combined effect execution creates separate materialization/installation events, not repeated method registration. Those events retain the existing preparation/publication boundary, restoration on every exit, captured-owner validity and exact release requirements.

Deferred calls retain the selected callable view and prepared operands; a family view must preserve its source interpretation and later actual-type dispatch. Deferred bodies remain delayed, with explicit nested scopes for declarations/effect installation. New abstract evidence and family dispatch must preserve checked NPE, pending normal/return/error completion and explicit causes. Missing/ambiguous method behavior not covered by existing static diagnostics needs its own decision; no new exception category follows from the AOT vision. [D-21](AheadOfTimeExecutionPreparation.md) tracks that architecture work while the bounded defer implementation continues.

### Immediate implementation and acceptance

H-356 authorizes implementing this language work independently of the LLVM binding pilot. The compiler distinguishes ordinary grouping from full scopes and iteration effect boundaries. Complete scope-owned deferred work and explicit returns while preserving the existing exception and regional invariants:

1. Parse deferred calls separately from deferred blocks. Validate the call shape without executing the final invocation or assigning timing significance to redundant parentheses.
2. Reuse typed call preparation for callee evaluation, ordered argument conversions, stored callable values and borrowed operands. Retain prepared state in the owning scope and register it only on successful preparation. Keep block bodies delayed.
3. Integrate deferred execution with normal and exceptional scope exit, pending results, effects and intrinsic owner cleanup. Use existing LLVM call/cleanup machinery and D-18 storage; no new general closure or persistence framework is implied.
4. Verify a compact set of composed cases: initializer-returned cleanup; saved values versus delayed block reads; later callable replacement; argument/conversion order and preparation failure; reverse-order cleanup on normal/exceptional paths; whole-loop versus per-iteration explicit scopes, direct loop defer accumulation and invalid borrowed-owner use. Record allocation and emitted-IR costs for repeated scoped use and accumulated registrations.

Before claiming complete coverage, verify H-370’s owning-argument boundary and resolve any uncovered integration choice involving checked exceptions from cleanup or cleanup failure while another completion is pending. Existing D-17 rules remain authoritative where they already apply. Neither a parser-only change nor execution of a standalone cleanup sample satisfies acceptance.

The [trailing callback form](SymbolicDynamicScope.md#trailing-callback-arguments) separately constructs an ordinary callable argument. H-TRAILING-CALLBACK-20261117 places it in the first declared parameter; its body runs only when invoked by the callee. Standalone full scopes remain immediate calls and do not silently change meaning because a consumer happens to expect a callable. Ordinary capture/access/lifetime and func/proc restrictions continue to govern any admitted callback.

## Loop scoping rules

H-365 defines loops directly, without introducing a general-purpose scope abstraction. Ordinary grouping, explicit `{{ ... }}` scopes and inherent callable/module scopes retain their contracts. A loop combines loop-control bindings, per-iteration effects/resource lifetime and deferred work registered to a full scope; these responsibilities have distinct boundaries.

### Bindings and declarations

Loop-header index/accumulator bindings belong to the loop construct and follow its initialization/update rules. They are not fresh body variable declarations. An ordinary loop body forbids variable declarations, including storage, aliases, callable variables/named local callable definitions and region bindings. The restriction applies through ordinary nested groups and conditionals. A nested explicit `{{ ... }}` inside that body allows local declarations with the explicit scope's lifetime and cleanup. A full scope surrounding the whole loop does not authorize direct body declarations. Each nested loop needs its own per-iteration explicit boundary for body declarations; called functions/procedures retain their independent inherent scopes.

### Iteration effects and resources

Every iteration restores its entry effect bindings on every exit, including throw and return. A temporary binding cannot leak into the next iteration. Restoration changes bindings, not mutations already made to regional data. A fresh region installed through an effect, such as `effect @arena <- region(bytes)`, belongs to that iteration's resource lifetime: release it at iteration exit and restore the prior binding. Captured and hidden pre-existing owners remain alive; shadowing never frees the enclosing owner.

Existing access, transfer and provenance checks apply. An outer deferred action cannot borrow an iteration-owned region or a value dependent on it. Saving a view does not extend lifetime, and a dynamic binding does not permit ownership extraction. No allocator, regional representation or zero-cost-relocation rule changes.

### Deferred work and accumulation

Direct `defer` in an ordinary loop body is allowed. The iteration's effect restoration boundary does not own user deferred actions. Each successful dynamic registration goes to the nearest enclosing full scope (`{{ ... }}` or an inherent callable/module scope), skipping loops and ordinary grouping. Every registration remains distinct, and the owning scope runs all actions in global reverse registration order, including registrations before and after the loop. Accumulation is intentional for dynamic resource management; an implementation must not conceal its cost by cleaning up early.

```text
{{
  defer finish_batch();
  for (i in 0 .. count) { defer consume(i); }
  defer begin_cleanup();
}} # begin_cleanup, consume(count - 1) ... consume(0), finish_batch

for (i in 0 .. count) { {{ defer consume(i); }} } # one cleanup per iteration
```

Putting `{{ ... }}` around the whole loop forms a cleanup batch. Putting it inside the body creates per-iteration cleanup. Callee evaluation and argument preparation remain eager under H-356; partial preparation registers no action and preserves prior registrations and acquired cleanup obligations. D-17's pending return/error, deferred catches and finally rules apply. Outer deferred calls run after iteration effects have restored; direct procedure invocation sees its current bindings, without an implicit TLS snapshot.

### Loop-control snapshots and borrowing

A delayed defer block cannot capture loop-control variables, including the index and fold accumulator. `defer consume(i)` saves the current bits argument through ordinary call preparation; `defer { consume(i); }` is invalid. There is no implicit final-value capture, lexical boxing or lifetime extension. A borrowed argument saves the selected reference rather than a deep copy, so its owner must outlive eventual invocation. Call syntax does not make borrowing loop-control storage or iteration-owned resources valid for an outer deferred action.

### Zero-iteration else

Range `for ... else` executes its else branch only when the body executes zero times, not on exhaustion. The else branch follows the same rules: direct variable declarations are forbidden; its effects restore and fresh effect regions release at branch exit; direct defer registers to the enclosing full scope. A nested explicit `{{ ... }}` permits local declarations and local deferred work. The loop index is unavailable in else. Once-only bound evaluation, initialization joins and fold's initial accumulator are unchanged. This contract does not add `fold ... else`, break/continue or new loop forms.

### Acceptance

Composed programs must establish outer/iteration effect values, normal/throw/return restoration, exact releases with captured/hidden owner retention, eager snapshots and global LIFO over dynamic registrations. Validate whole-loop and per-iteration cleanup batches; reject declarations through ordinary body/else groups, delayed loop-control captures and outer deferred borrows of iteration resources; accept nested explicit and callable scopes. Cover zero iterations and bound evaluation. Measure live registrations, peak storage and execution/IR costs as registration count grows. These checks must compose with return/error replacement and remaining cleanup. D-16 syntax/comments, numeric promotion, bits layout, transfer and formatting remain unchanged; fraction implementation stays deferred.

## Expression-oriented control

`try` is an expression, consistent with CoD's existing expression model. Normal completion and handled completion provide its result; an unhandled exception provides no normal result. A `finally` block performs completion work without replacing the result merely through its final expression value. No public bottom type or general continuation API is needed to express this planning requirement.

The exception identities are explicitly selected into the module before use:

```text
try {
  owned region packet:Packet <-
    make region Packet(bytes -> capacity) { sequence <- initial };
  process_packet(packet)
} catch (error : const OutOfMemoryError) {
  handle_allocation_failure(error)
} finally {
  finish_attempt()
}
```

The same construct can occur in a function or a procedure. Each body's existing restrictions still apply: a function may operate on explicit memory but cannot access dynamic bindings or call a procedure. A lexical catch does not make dynamic lookup available inside a function. Exception capabilities do not create module exports, change callable identities or expose compiler call-routing machinery.

The control sequence is:

1. Execute the protected expression. On an exceptional exit, run cleanup for scopes being exited before entering the selected handler.
2. Execute at most one applicable catch. Unmatched exceptions continue outward. An exception raised by a catch is not caught again by that same try's sibling catches.
3. Execute `finally` once when leaving this try, after normal or handled completion, or while propagating an exception. Then deliver the pending result or propagate the pending exception.

This does not promise `finally` execution after forcible process termination or loss of the execution environment. Native resource release and TLS restoration are compiler obligations even when the program writes no `finally`; user finally code must not be required to free every intrinsic owner manually.

## Ownership and effects on exceptional paths

Exceptional control must preserve the actual ownership history; it is not a transaction that rewinds the program:

| Situation | Required preservation under the existing ownership/effect constraints |
| --- | --- |
| Region allocation fails before obtaining storage | No owner exists to expose, consume or release. |
| Allocation succeeds, then root initialization fails | The unfinished region still has one construction cleanup responsibility. Release it before propagation; never publish an uninitialized owner. |
| An effect's RHS construction fails | Do not install a new binding. Preserve the previous binding and owner; do not roll back unrelated edits made while evaluating the RHS. |
| A local owner transfers into an inner effect and that scope exits exceptionally | Release the region at its actual new owning scope. The original name stays consumed; old views cannot be used after that owner ends. |
| A transfer retains a live, accessible local owner | Preserve existing valid views under H-263, regardless of the old name's consumed state. |
| Perform temporarily shadows current bindings with captured values | Restore lookup on exceptional exit without freeing either pre-existing captured or hidden owner. Release only resources actually acquired by the exiting scope. |
| A caller-owned region was already edited before a later exception | Retain those edits unless the business algorithm separately implements rollback. |

Cleanup follows scope exit and reverse acquisition/restoration order within each scope. The handler's incoming ownership facts must include every throwing path that can reach it. If one such path transferred a region and another retained it, the handler cannot unconditionally use the old owning name. This extends the existing branch-validity requirement to exceptional edges; WPO does not make runtime failure positions statically known.

Pending expression results need explicit attention. If a try body produces an owning region but its finally subsequently throws, that not-yet-delivered result must still be released once. A borrowed result remains dependent on its actual owner through finally. Neither a stale pointer nor an abandoned owning result may escape by treating the normal result as delivered before finally completes. E4 below fixes the call/result transfer points; D-14 retains the wider ownership design.

## Exception data and low-memory operation

Raising OOM, routing it, restoring effects, releasing regions and entering a catch must not require a fresh allocation from the exhausted memory source. A handler can itself allocate and fail. Intrinsic system failures carry bounded scalar details; no borrowed payload may outlive its owner. General user exception payload graphs remain separate.

## Accepted first implementation contract

H-267 accepts E1–E5 together. Their value is a complete construction/edit/transfer/release workflow with usable failure handling and correct view validity, without per-object ownership or changes to regional pointer encoding. The explicit-cause amendment is part of E2/E3.

### E1 — Intrinsic exception categories and payload

The initial unchecked system failures are: `OutOfMemoryError`, `RegionCapacityExceededError` and `InvalidCapacityError`. The first means the memory provider cannot satisfy allocation; the second means an existing region's fixed budget cannot satisfy placement; the third means an invalid/unrepresentable capacity request. No automatic growth or retry is implied.

Expose their type identities through an ordinary standard vocabulary module, with normal explicit selection and local aliases. Their intrinsic payload is bounded bits: category, a source-site identifier and relevant scalar size information. The source-site identifier is resolved against immutable metadata for the executing program; it contains no native address and does not promise meaning under a different program image. Error formatting must not turn it into a stored cross-owner pointer. Catch selection uses a small runtime category discriminator. General type reflection, arbitrary user exception payloads and native C++ type matching remain outside this increment. H-267 accepts this bounded discrimination under H-251; it does not approve general RTTI. Matching needs a type identity at the throw/handler boundary, not a runtime type descriptor embedded in each payload. This bounded intrinsic payload describes current implementation support; the constructor-time trace contract above requires separately qualified trace storage and does not follow from a source-site identifier alone.

### E2 — Expression syntax, matching and completion

Use this source family, with exception identities already selected into the current module:

```text
try {
  operation()
} catch (error : const OutOfMemoryError) {
  recover(error)
} catch (error) {
  report_or_rethrow(error)
} finally {
  finish_attempt()
}
```

For currently implemented intrinsic categories, a typed catch matches its exact category. The selected standard-wrapper extension follows the source-order prefix rules above and is not yet implemented. Duplicate categories reject; an optional final catch-all matches the remaining supported categories. No subtype search or clause-order priority among distinct exact categories is needed. Catch parameters receive immutable bits values; the catch-all has the intrinsic common system-exception value type. This does not change ordinary parameter borrowing rules. Public field/accessor spelling is a routine API detail within the bounded payload, not authority to expose arbitrary runtime metadata.

Normal and handled arms must have the same result type under the existing conditional-expression rule; exceptional paths supply no normal value. Require `finally` to yield void. `try` may have catches, finally, or both, but at least one is required. `throw error` and bare `throw;` propagate existing values unchanged. New cause relationships are established at exception construction through from; rethrow uses the constructor/bare-wrapper forms above. Ordinary catch/finally does not add a cause automatically. Source creation of arbitrary user exception values/graphs is deferred. Catch bodies are outside their own sibling catch coverage. After all relevant cleanups/finally blocks, an uncaught exception gets a minimal diagnostic and a nonzero exit status at the program boundary.

### E3 — Failure from finally

For ordinary try/finally completion, a new exception from finally replaces that construct's pending exception or normal result. Outer cleanups/finally blocks still run; any abandoned owning result releases once. A deferred entry's escaping finally failure instead enters the deferred-error queue; it does not thereby replace every independently queued error. The replaced exception is not retained automatically. A program constructs a cause-bearing exception, for example with `rethrow SomeEx(...) from cause`, when it wants that relationship. A business exception's cause does not participate in catch matching; standard rethrow wrappers follow the transparent prefix contract above.

### Lexical backing and completion integration

[D-28](RegionalAndLexicalStorageRefactor.md#integrated-operations-and-lifetimes) records the relocation obligations for the selected lexical storage direction. Prepared references retain their selected targets/places; saved val results retain independent bits. Required deferred work, completion replacement and intrinsic release keep their existing order. Necessary owner metadata/backing remains valid through each admitted use, including cleanup that allocates, and ends with its semantic scope. Failure while preparing replacement lexical backing cannot invalidate existing targets before replacement is ready; it does not undo earlier allowed effects. This does not change E4 consuming region parameters or qualify a physical cleanup/anchor protocol.

### E4 — Consuming calls and results

An admitted owning parameter transfers responsibility at callee entry, after argument evaluation and conversion succeed. An external borrowed parameter retains the original owner without acquiring cleanup responsibility. H-271/H-272 requires explicit owner-generic declarations for external borrowing; the native prototype's unqualified `region :Root` borrowing spelling is a migration boundary, not the new default. The call itself has not consumed an owning argument if a later argument fails before entry; explicit transfers performed inside earlier argument expressions retain their actual effects. Validate availability and provenance again at the call boundary, including duplicate ownership transfers and later-argument invalidation of earlier views.

H-458 applies this boundary to parameter defaults: explicit inputs prepare in written order, then omitted caller-context defaults prepare in declaration order, all before entry. Definition-environment defaults run during parameter initialization after entry. A failure there cleans every input whose ownership transferred at entry, including an input whose named binding has not yet initialized. For an admitted plain deferred call, caller-default preparation belongs to registration and callee-default initialization to invocation; failed registration retains ordinary preparation cleanup and registers no partial call. Earlier valid effects are not rolled back. Named/default source support remains implementation work.

After entry, an owning parameter's region belongs to the callee scope until returned or otherwise transferred; an exceptional callee exit releases what that scope still owns. The caller's original name remains consumed. An owning result transfers only after the callee's required finally work completes successfully; a result abandoned by finally failure remains subject to cleanup. This preserves the existing per-argument evaluation/conversion order without treating ownership transfer as a numerical conversion.

H-431 explicitly applies this rule to consuming replacement, now proposed as resize(r, new_cap) under H-432 in D-28. “Call transfers ownership” means callee entry, not the beginning of argument evaluation. “Only successful completion returns ownership” includes required finally/deferred cleanup. A return selected before throwing cleanup is not delivered; its abandoned owning resource is released. Catching an exception does not restore the caller's consumed input. A function may explicitly return the same region on success, but that is receipt of an owning result, not reversal of the input transfer. Resize's separate replacement contract ends old-region views rather than retargeting them. Its proposed capacity checks happen after entry: even an undersized request that throws does not restore the input. D-28's exact mapping of resize failures to existing categories remains a scoped proposal; E4/E5 is unchanged.

The borrowed and consuming call forms are:

```text
function edit(region packet:Packet) -> void { ... }
function transform(owned region packet:Packet) -> owned region :Packet { ... }

edit(packet); # Borrow; the caller still owns packet.
owned region next:Packet <- transform(packet);
# packet is consumed after callee entry; next owns the successful result.
```

The first increment does not select overloads whose only distinction is owning versus borrowed mode; reject such a competing pair with an explicit unsupported-mode-overload diagnostic. Otherwise adding an overload could silently change whether a call consumes its operand. General mode-based overload ordering remains a separate language decision.

### E5 — Bounded construction and acceptance scope

The bounded lifecycle contract covers typed region construction and named-root access, fresh owning declarations, factory results, borrowed/owning calls, effect installation, branch/exception ownership joins, and try/catch/finally together; current implementation coverage is separately recorded. The required capacity is a runtime byte budget for the root and subsequent payload allocations, including their internal alignment consumption but excluding the fixed region header. No automatic growth is introduced. H-596–H-599 extends construction to separate root initializers or a free initialization block with a construction-local region name and init helpers. Successful completion requires every root contract established, with source-ordered effects and ordinary whole-region failure cleanup. Missing initialization rejects; repeated ordinary writes are permitted. D-22 owns this source extension and its separately pending full proof/native integration; root defaults remain unselected.

Ordinary owner reassignment/reinitialization, occupied owner destinations, general reference-assignment/const-view overloads, mailbox transport, user exception graphs and general RTTI are outside this increment. Existing typed memory operations can demonstrate root edits without pretending that the complete assignment surface is implemented. Exceptions must not roll back prior graph edits or ownership transfers.

Implementation includes the actual allocation-to-publication cleanup repair, the direct LLVM control path, the required build and focused CoD-language/shilos-region validation. Native system-call failure translation is internal; the implementation must preserve the user semantics above without exposing an ABI or adding an alternative backend. Acceptance covers complete construction/edit/call/transfer/release, captured/hidden owner preservation, normal and exceptional control, pre-entry versus in-callee failure, invalid owner/view use and deterministic low-memory failure injection. Source, diagnostics and evidence should be reviewable before any broader feature is added.

Cancellation, asynchronous faults, restartable/resumable exceptions and cross-process exception delivery remain outside this first contract. The compiler/runtime route described below is an implementation recommendation subject to the accepted semantics, not another user-visible mechanism to approve.

### Bounded process OOM information

H-516 selects a bounded information area statically reserved at process level for OOM handling. H-517 retains the earliest OOM record unchanged and uses the rest as a rolling window: publishing a new later record evicts the oldest retained rolling record. The first record remains pinned for the process lifetime. This is bounded diagnostic retention, not an expanding region or an unlimited emergency heap.

The reserved path must not allocate from the failing allocator to record basic OOM information. Separate exception semantics from diagnostic availability: a retained exception's category, scalar details, readonly cause and creation identity cannot change because a diagnostic slot is reused. A handle to an evicted trace/information record reports unavailable; it must never expose a different newer event now occupying that slot. Any public read must obtain a coherent bounded snapshot or equivalent checked view, not an escaping raw reference into overwritable storage. Internal record identity/generation and synchronization are engineering choices; no counter width or pointer ABI is selected.

Use one defined publication order across concurrent runtime users of the process reserve, with exactly one pinned earliest published OOM record. Readers cannot observe partial publication or associate another thread's record with an older exception. In-flight recording/reading, truncation indicators and rollover must not corrupt the pinned record or create allocation recursion. Bounded per-record trace capacity must expose unavailable/truncated diagnostic detail honestly; it cannot pretend to retain an unbounded stack. Exact capacity, formatting and synchronization protocol remain implementation work within this visible retention policy.

Rolling records do not own semantic exception fields or the only copy of a cause needed by a still-live exception. UnhandledDeferError must still retain every required member, and newly constructed rethrow wrappers must retain their immutable causes. Their owner/carrier protocol, repeated-wrapper storage growth and construction failures remain separate: the information window does not make E errors wrapped by H handlers fit into E+H semantic nodes or authorize overwriting live exception values. Prepared/reused deferred carriers are possible mechanisms for that separate obligation, not a consequence of this reserve.

**Acceptance:** basic OOM reporting with ordinary allocation unavailable; first record unchanged after more events than the window can hold; retained older exceptions whose diagnostics become unavailable rather than misidentified; stable cause/aggregate values; simultaneous recording/reading; bounded/truncated trace disclosure; and no implicit region growth, owner resurrection, raw dangling diagnostic view or allocation-dependent basic propagation. The current native cause helper can allocate a vector entry and the current drain releases registration records before invocation; neither implements this reserve or the required aggregate/cause lifetime protocol. No compiler code changes or new runtime validation are supplied here.

## Compiler and runtime implementation

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Native exception handling must preserve checked obligations, explicit causes, scope cleanup, effect restoration and exact release. Intrinsic system-failure information is not general reflection or a mandated user-visible C++ exception ABI. Concrete payload and control-flow representation remain implementation concerns.

### Retained explicit-cause lifetime

H-269 requires explicit cause relationships to survive returning or storing a caught exception beyond the catch. H-510–H-515 establishes those relationships only when constructing a new exception, with readonly cause thereafter. Runtime-managed storage must retain the relationship without mutating a supplied exception/cause. Ordinary cause-bearing construction may itself raise OutOfMemoryError; basic OOM propagation does not allocate and construction failure does not automatically attach either operand to the new OOM. Existing saved values retain their information. Under H-541, a matched deferred input is consumed and an escaping handler construction failure joins the queue tail; the input is not automatically duplicated or attached as a cause. The current compiler's throw-from operation and numeric cause handles are implementation coverage, not acceptance of the new constructor/rethrow/trace contract.

The bounded runtime uses execution-local numeric handles for immutable cause records. Regional fields contain bits and handles rather than host addresses; copying a regional image within the same execution does not require pointer patching. Generic exception storage also preserves its bounded failure-category discriminator. These handles, like source-site IDs, are not a cross-process persistence or delivery format. General user exception graphs and general RTTI remain excluded.

Retained backing storage is released at execution end, including uncaught failure. The initial implementation retains discarded records until that boundary; repeated explicit chaining consumes that storage and may raise OOM. It does not add per-value destructors, graph ownership or eager collection. Public inspection uses a bounded cause count and bounds-checked indexed access; inspection and propagation allocate no cause records. Exact reclamation optimizations can preserve this source lifetime contract later.

The native cause table is created only by explicit chaining. Its TLS slot is a trivial pointer, with no lazy destructor registration on basic OOM propagation or cleanup. Failed table creation, first-record allocation or later growth leaves existing histories intact. The native fault harness fails the actual allocation at each of these three boundaries, verifies a fresh unchained OOM, retries successfully and accounts for all backing allocations/releases.

### Allocation-to-publication cleanup

The regional allocator must release its byte block if root construction throws, using the matching allocator and exact capacity. [The construction fixture](../tests/shilos/region_construction_tests.py) checks failure before root entry, both throwing-root overloads and successful retention/release. This allocation-to-publication boundary supplies no existing-region rollback or graph transaction.

## Bounded acceptance and remaining scope

Acceptance requires nested normal/exceptional completion, pre-entry versus callee failure, explicit cause retention, cleanup replacement, deferred queues, failed construction and invalid-borrow rejection. Check exact acquisition/release on supported paths.

Mailbox transport, general user exception graphs, RTTI, cross-process causes and remaining ownership choices retain separate scope. Current implementation limits are maintained in the source capability reference.

## Type routing and handler scope

A try scope supplies its accepted exception type identities and cleanup/handler destinations. At a statically typed throw, WPO can resolve the destination and emit a direct branch. The routing information is distinct from the exception payload; it does not require general RTTI in each exception value. Plain throw propagation from a catch-all must preserve the arriving exception identity in the propagation context; explicit rethrow instead constructs the selected new primary exception. Merely retaining the handler's accepted identity cannot distinguish otherwise identical incoming values.

The user's H-268 suggestion motivates this separation. A runtime handler-registration table is not required by the source semantics or selected merely by the analogy; compiler-local handler scopes and direct LLVM edges are the first implementation route. Explicit-cause lifetime remains a correctness obligation: no dangling catch-local references, silent truncation or allocation-dependent basic OOM delivery.
