# CoD exception control and regional cleanup

Design revision: 151 · Recorded: 2026-11-24 · Status: confirmed bounded exception, lifecycle, scope, defer and return contracts; full-scope control and live lexical input contracts; current implementation availability is recorded in the source capability reference.

## Design authority

At r151 the [normal-completion decision](../CHANGELOG.md#h-normal-completion-20261124) qualifies [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state) for the unbound test at each cleanup point. P-482–P-484 and [D-24-TAIL](DesignAuthorityRegistry.md#d-24-tail) preserve callable ownership, result preparation and independent deferred failures. The paired exits and ordered observations below specify acceptance.

At r150 the [grouped installation decision](../CHANGELOG.md#h-grouped-local-proc-install-20261124) applies the existing scoped restoration obligation to each complete anonymous installation. Verify that a failed preparation preserves the previous handler and a checked throw through a method restores it on scope exit. [Handler contract](SymbolicDynamicScope.md#installing-a-local-family-with-several-methods)

[D-17-CATCH-HANDOFF](DesignAuthorityRegistry.md#d-17-catch-handoff), r149, governs [owning reception from a catch binding](#choosing-the-target-of-an-owning-catch-handoff). [H-TYPED-CATCH-OWNER-HANDOFF-20261123](../CHANGELOG.md#h-typed-catch-owner-handoff-20261123) confirms tip-first reception under the receiving type and delegates the fallback. P-707/P-708 preserve exception identity and a single release obligation under the catch and owner contracts. Acceptance distinguishes broad and narrow destinations, including failure before handoff. Physical transport and broader wrapper implementation retain their own review.

[K-DEFER-AGGREGATE-AVAILABILITY-20261122](../CHANGELOG.md#k-defer-aggregate-availability-20261122) continues r147’s implementation-availability table at r148. The [aggregate ownership scope](DesignAuthorityRegistry.md#d-17-aggregate-ownership) retains the confirmed post-pass and separate body-failure requirements. [Seed coverage](../tests/cod/language/README.md#deferred-execution-and-initialization) records the implemented propagation scope. Member access, general collection operations and full native qualification retain their separate scope.

[D-14-REF-ARGUMENT](DesignAuthorityRegistry.md#d-14-ref-argument) and [H-OWNER-REF-ARGUMENT-20261121](../CHANGELOG.md#h-owner-ref-argument-20261121) cover [checked NullPointerException](#checked-nullpointerexception) at r147. Explicit `not null` on a ref argument can fail in caller preparation. P-668/P-669 preserve checked coverage, failure order and pre-entry cleanup. Check a known-null selection before a later side effect.

[D-17-EXCEPTION-BINDINGS](DesignAuthorityRegistry.md#d-17-exception-bindings), r146 ([K-CATCH-ROOT-SPELLING-20261121](../CHANGELOG.md#k-catch-root-spelling-20261121)), specifies the [owning catch binding](#exception-declarations-and-catch-bindings). [H-CATCH-HOLDER-ONLY-20261121](../CHANGELOG.md#h-catch-holder-only-20261121) selects one holder per match and author-selected target constness. The [source and parsing baseline](../CHANGELOG.md#h-exception-bindings-20261121) covers ordinary ref borrows, explicit holder transfers and successful construction. P-650/P-651 tie readable source to one release obligation. Check the same header in ordinary and deferred handlers, writes through the default binding, and a local borrow under the caught owner. Runtime delivery remains qualified per operation.

[D-10-OWN-QUALIFIER](DesignAuthorityRegistry.md#d-10-own-qualifier) and [H-CONTEXTUAL-OWN-20261121](../CHANGELOG.md#h-contextual-own-20261121) cover the spelling of the optional qualifier on owning exception declarations outside catch headers.

[H-ORDINARY-NAMES-20261119](../CHANGELOG.md#h-ordinary-names-20261119) covers the loop-control lifetime examples at r128, continuing the existing defer and bounds contracts under [D-10-ORDINARY-NAMES](DesignAuthorityRegistry.md#d-10-ordinary-names).

- **Return inspection during defer execution.** [H-DEFER-RETURNING-20261119](../CHANGELOG.md#h-defer-returning-20261119) limits [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state) r127 to an executing deferred entry of the callable owning the return. [D-24-TAIL](DesignAuthorityRegistry.md#d-24-tail) supplies the invocation’s void completion and value-result preparation rules. P-482–P-484/P-631–P-632 preserve lexical control, cleanup order and actual owners. Check ordinary body and finally rejection, delayed body and attached handler acceptance, inner invocations, prepared value tails and normal no-payload completions. Representation and compiler support are separate.

- **Case completion.** [H-CASE-COVERAGE-20261119](../CHANGELOG.md#h-case-coverage-20261119) and [D-07-CASE](DomainLanguageDesign.md#case-expressions-and-branch-local-variables) permit proven value-case coverage or an explicit fallback. [H-CASE-THROW-20261119](../CHANGELOG.md#h-case-throw-20261119) confirms `() -> throw;` constructs a fresh unchecked UnexpectedCaseError at the written arm with no implicit cause. In `() -> { throw; };`, a lexically available catch supplies its incoming tip. D-17r126 preserves ordinary construction, propagation and cleanup.

- **Lexical yield in a full scope.** [H-GENERATOR-RECEPTION-20261119](../CHANGELOG.md#h-generator-reception-20261119) updates [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope) at r121. D-18-HERE and D-29-GENERALIZED-FOR distinguish calling a lexically available here parameter from the full scope’s own return and cleanup. Check the two exits in the [nested generator example](RangedForAndGenerators.md#a-generator-declaration-introduces-a-here-parameter-named-yield). P-564–P-566 and the existing exception contracts continue for their covered scopes.

- **Independent aggregate members.** [H-DEFER-MEMBER-TAKE-20261118](../CHANGELOG.md#h-defer-member-take-20261118) confirms that each residual member carries an independent owner and taking it transfers that owner alone. [D-17-AGGREGATE-OWNERSHIP](DesignAuthorityRegistry.md#d-17-aggregate-ownership), r118, applies P-582/P-583, C-54/C-63/C-65 and charter §§3.3/4.1/4.3/6.3/8.1. Acceptance covers indexed removal, surviving siblings, independent body failure and forwarding one holder across successive handler visits. The user also requests readonly enumeration through ordinary holder-to-view binding. D-17r119 documents the [exception-specific application](#enumerating-and-taking-deferred-failures) of [D-30’s collection operations](CollectionsAndBuiltinContainers.md#proposed-common-operations). [K-COLLECTION-INTERFACES-20261118](../CHANGELOG.md#k-collection-interfaces-20261118) records this dependency. Operation names, owning generator reception and native representation remain outside the confirmed implementation scope.

- **OOM during deferred reporting.** [H-OOM-BEST-EFFORT-20261118](../CHANGELOG.md#h-oom-best-effort-20261118) sets simplicity and best-effort retention as the governing OOM policy. The delegated [publication rule](#when-a-defer-pass-cannot-publish-its-aggregate) applies [D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve) to [D-17-AGGREGATE-OWNERSHIP](DesignAuthorityRegistry.md#d-17-aggregate-ownership) in r116. P-467–P-469/P-582–P-585 connect bounded reporting and single release to C-63/C-65 and charter §§3.3/4.1/4.3/6.3. Acceptance distinguishes failed registration, a complete aggregate containing an OOM, and resource failure during publication. Registration order, checked coverage and owner safety still apply. Native backing and ABI qualification remain open.

- **Placement when constructing with a cause.** [H-BRACED-CAUSE-20261118](../CHANGELOG.md#h-braced-cause-20261118) fixes a single position for `from cause` in braced [throw](#exception-construction-throw-and-rethrow) and [normal make](#constructing-a-normal-result-with-a-cause) in D-17r117. D-17-RETHROW/P-464–P-466 and D-17-MAKE-EXCEPTION use written effect order and an identifiable owner under charter §§3.3/4.1/4.3/6.3/8.1. Acceptance covers an effectful cause, a named cause, and a cause that fails before any field runs. H-SAME-OWNER-CAUSE-20261118 and [H-EXCEPTION-CAUSE-HANDOFF-20261118](../CHANGELOG.md#h-exception-cause-handoff-20261118) govern selection and handoff on publication. [K-CAUSE-BORROW-COST-20261118](../CHANGELOG.md#k-cause-borrow-cost-20261118) records the [deferred implementation of surviving typed borrows](#implementation-cost-of-preserving-the-borrow) under H-262/H-263. Source grammar and evaluation order have no native implementation qualification.

- **Fresh-region capacity.** [H-EXCEPTION-CAPACITY-20261118](../CHANGELOG.md#h-exception-capacity-20261118) records the [image default and aggregate sizing](#image-setting-for-exception-region-capacity) and the request for [site-specific tuning](#capacity-at-a-fresh-construction-site). [D-17-EXCEPTION-CAPACITY](DesignAuthorityRegistry.md#d-17-exception-capacity), r112, applies D-14 fixed region ownership, D-22 acquisition and D-17-AGGREGATE-OWNERSHIP to predictable recovery storage under charter §§4.1/4.3/6.3. Acceptance covers a smaller and a larger site budget, field/argument distinction, existing-owner propagation and complete oversized aggregates. The [construction grammar](#make-exception-expressions) fixes clause placement. The physical backend remains pending.

- **[Residual aggregate ownership](DesignAuthorityRegistry.md#d-17-aggregate-ownership).** D-17r110/P-582–P-583. [H-DEFER-AGGREGATE-REGION-20261118](../CHANGELOG.md#h-defer-aggregate-region-20261118) confirms that a raised `UnhandledDeferError` has a fresh region and internally retains independent failures in a dynamic array of `exception` holders. [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch) and [D-17-EXCEPTION-OWNER](DesignAuthorityRegistry.md#d-17-exception-owner) preserve occurrence identity, FIFO order and whole-owner release under C-54/C-63/C-65 and charter §§3.3/4. The [aggregate contract](#region-and-members-of-unhandleddefererror) covers forwarding, independent members and the separately retained body failure. The [image capacity rule](#image-setting-for-exception-region-capacity) covers initial sizing. The [OOM publication rule](#when-a-defer-pass-cannot-publish-its-aggregate) governs failure to construct the aggregate. VM backing and the projection ABI remain under review.

- **Owned-region exception model under review.** [D-17-EXCEPTION-OWNER](DesignAuthorityRegistry.md#d-17-exception-owner), r112, evaluates the [exception holder as an owned region and a selected target](#exception-as-an-owned-region-and-a-selected-target). [K-EXCEPTION-REGION-MODEL-20261118](../CHANGELOG.md#k-exception-region-model-20261118) records the review against C-54/C-63/C-65, D-14 ownership, D-22 root initialization/results and the same-owner cause requirement. The review covers fresh and caused construction, exact handoff, factory results, ordinary borrows, wrapper matching and aggregate retention. The [normal construction expression and owning factory result](#make-exception-expressions) use the region/target pair. The holder spelling and catch binding follow the [binding contract](#exception-declarations-and-catch-bindings). A sole-reference factory ABI remains open. Implementation qualification is pending. The successful throw consumes its holder under either model.

- **Construction at a throw site.** [H-THROW-INITIALIZER-20261118](../CHANGELOG.md#h-throw-initializer-20261118) selects `throw T { ... }` for allocation, field initialization and propagation of a fresh exception. [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow) and D-17-EXCEPTION-OWNER preserve existing propagation, creation-time provenance and owner identity under C-63/C-65 and charter §§3.3/4.3/6.3/8. [D-22](ConstructionAndAcquisition.md#typed-reservation-and-completed-construction) supplies field readiness, order and transfer. Acceptance below covers successful, empty and failing initialization. [H-MAKE-EXCEPTION-20261118](../CHANGELOG.md#h-make-exception-20261118) and [D-17-MAKE-EXCEPTION](DesignAuthorityRegistry.md#d-17-make-exception) settle fresh normal construction, optional capacity, complete initialization, owning delivery and a later throw of the identical object. D-22r49 applies that distinction to ordinary data construction. Source scope only.

- **Exception storage and application payloads.** [H-EXCEPTION-OBJECT-OWNER-20261118](../CHANGELOG.md#h-exception-object-owner-20261118) selects owner-bound, non-bits exception objects, transfer of program state and release after the last owner dependency. [D-17-EXCEPTION-OWNER](DesignAuthorityRegistry.md#d-17-exception-owner), r112, scopes the [result and release rules](#exception-object-owners-and-release). C-54/C-63/C-65 and charter §§3.2–3.3/4.1/4.3/6.3/8 link inspectable exception identity to explicit storage responsibility. D-14, D-22-TRANSFER, D-22-RESULT-OWNERS and D-28-OWNERS constrain target identity and transfer. H-269 preserves retained causes. The source rule supersedes independent exception-bits inputs. [H-CATCH-OWNER-TRANSFER-20261118](../CHANGELOG.md#h-catch-owner-transfer-20261118) covers writable recovery state, allocation in the caught owner and graph/bits transfer. The holder binding names that owner in the current catch grammar. [H-EXCEPTION-FIELD-OWNERS-20261118](../CHANGELOG.md#h-exception-field-owners-20261118) covers every user-defined reference field and bidirectional transfer. [H-EXCEPTION-MAKE-SCOPE-20261118](../CHANGELOG.md#h-exception-make-scope-20261118) assigns exception construction to an on-demand virtual handling scope. [K-EXPLICIT-EXCEPTION-OWNERS-20261118](../CHANGELOG.md#k-explicit-exception-owners-20261118) derives [explicit catch and result owners](#exception-owners-across-callable-boundaries) from these contracts. [H-SAME-OWNER-CAUSE-20261118](../CHANGELOG.md#h-same-owner-cause-20261118) requires one actual owner per cause chain. [Release responsibility](#who-releases-the-exception-handling-scope) covers managed lifetime and unique scope handoff. [H-EXCEPTION-WHOLE-OWNER-20261118](../CHANGELOG.md#h-exception-whole-owner-20261118) establishes that either owning spelling carries the entire scope, including allocation access for application payloads. [H-EXCEPTION-HOLDER-20261118](../CHANGELOG.md#h-exception-holder-20261118) establishes the one release obligation on a catch or an owning declaration. The [holder, borrow and target declarations](#exception-declarations-and-catch-bindings) distinguish whole-scope handoff from target access. [H-EXCEPTION-DECL-OWNED-20261118](../CHANGELOG.md#h-exception-decl-owned-20261118) makes the explicit ownership qualifier optional on exception variables and parameters. Borrows use `ref ... in ...`. [K-EXCEPTION-SCENARIOS-20261118](../CHANGELOG.md#k-exception-scenarios-20261118) applies those rules to [calls, control flow and retention](#exception-ownership-across-control-flow). Implementation details beyond the confirmed binding and transfer rules remain under review.

- **Cause and tip reference modes.** [H-CAUSE-PTR-TIP-REF-20261118](../CHANGELOG.md#h-cause-ptr-tip-ref-20261118) specifies the r97 field modes under D-17-RETHROW/P-464–P-466 and D-17-CATCH-WRAPPERS/P-470–P-472. Both target Exception. A nullable cause and non-null tip make absence explicit and preserve the identity of each selected exception. H-515 and H-CATCH-TIP-20261118 govern field immutability and catch selection. D-14 and D-17/H-313 govern owner and null checks. Charter §§3.2–3.3/4.3/6.3/8 connects those guarantees to typed reporting. Acceptance below covers absent and present causes, unwrapped and wrapped tips, and readonly access. Physical backing and general implementation qualification remain open.

- **Readonly access to the incoming exception.** [H-CATCH-TIP-20261118](../CHANGELOG.md#h-catch-tip-20261118) selects the readonly [catch-binding virtual field](#the-catch-bindings-readonly-tip-field) in [D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), r95. C-36/C-63/C-65 and P-470–P-472 connect typed recovery with complete diagnostics under charter §§3.2–3.3/6.3/8. H-520–H-522 govern operand identity and plain propagation. Acceptance covers direct and wrapped arrivals, distinct catches of one underlying object, and passing the selected exception to a helper. Physical representation and general inspection APIs retain separate status.

- **Standalone throw-only result.** [H-STANDALONE-VOID-20261118](../CHANGELOG.md#h-standalone-void-20261118) refines D-17-FULL-SCOPE/P-564–P-566 at r94. Without an established result contract a standalone throw-only body defaults to void. Explicit value returns, expected callback contracts, ordinary permission checks and the local checked interface retain their existing rules. See [D-24’s inference boundary](CallableResultInference.md#establishing-a-result-contract).
- **Application-facing rethrows.** [H-WRAPPER-USAGE-20261118](../CHANGELOG.md#h-wrapper-usage-20261118) covers [D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers) in r94. H-518–H-523 and P-470–P-472 preserve source-order matching, original exception identity and trace history. Charter §§3.2–3.3/6.3/8 connects those guarantees to readable exception contracts. The example below checks catch selection and consecutive cause links. The public wrapper name is legal and strongly discouraged in application interfaces. Full checker, identity and inspection mechanisms retain their separate status.

- **Live lexical slots in full scopes.** [K-FULL-SCOPE-PRODUCT-SLOTS-20261119](../CHANGELOG.md#k-full-scope-product-slots-20261119) continues [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope) under H-FULL-SCOPE-IMPLEMENTATION-20261118, H-CALLABLE-MODEL-20261118 and H-478. P-564–P-566 and charter §§3.2–3.3/4.3/6.3/8/12 connect direct access to the supplying activation with explicit control, owner and storage responsibility. The [seed coverage](../tests/cod/language/README.md#current-implementation-boundary) covers lexical product fields through live reference slots, including a declared place and a field projection that can reach the same embedded slot. Acceptance covers selected-field readiness, rebinding across normal and exceptional completion, cleanup order, and owner and access rejection. Whole-product value transport and regional layout retain their separate qualifications. [K-QUERIED-PRODUCT-READINESS-20261119](../CHANGELOG.md#k-queried-product-readiness-20261119) continues the same profile under H-445. Type queries check local product readiness through the body's control flow, with separate embedded-slot and referent obligations. Enclosing storage remains unevaluated. Acceptance includes sibling fields, rebinding, nested scopes, cleanup and finite required-reference components. [K-QUERIED-PRODUCT-CALLS-20261119](../CHANGELOG.md#k-queried-product-calls-20261119) covers the same readiness obligations across named helper calls. Acceptance checks missing reads, projected writes and rebinds on completion, checked failures, borrowed field results, and saved call operands with live embedded slots during cleanup. [K-QUERIED-SELECTED-FIELDS-20261119](../CHANGELOG.md#k-queried-selected-fields-20261119) continues direct and inline field readiness when a receiver can select several products. Acceptance checks retained selections, independent choices with the same candidates, shared helper inputs, and a cleanup receiver saved before its original slot is rebound. [K-QUERIED-BORROWED-FIELDS-20261119](../CHANGELOG.md#k-queried-borrowed-fields-20261119) carries the same proof through separate field borrows and named borrowed results. Complete-product checks use proved inline paths and continue to check embedded slots and required targets. Acceptance covers rebinding on both sides of the borrow, nullable results, nonexecuting helper inspection, and cleanup writes. [K-QUERIED-EMBEDDED-SLOTS-20261119](../CHANGELOG.md#k-queried-embedded-slots-20261119) distinguishes the selected embedded slot from the loaded reference through direct operations, immediate scopes and named helpers. Acceptance covers first binding, overlapping writes, earlier borrows, and registration-time slot selection with invocation-time helper loads. [K-QUERIED-RETURNED-REFERENTS-20261119](../CHANGELOG.md#k-queried-returned-referents-20261119) continues borrowed result inspection and preparation through helper completion. Acceptance covers exact scalar or product forwarding, independent null alternatives, a slot rebind before return preparation, and cleanup that rebinds the slot or initializes the earlier returned target. [K-QUERIED-EMBEDDED-CHAINS-20261119](../CHANGELOG.md#k-queried-embedded-chains-20261119) carries the same address and referent proofs through successive embedded loads. Acceptance covers scope and helper writes, rebinding intermediate slots, prepared cleanup, and returned direct fields or embedded referents. [K-QUERIED-GUARDED-CHAINS-20261119](../CHANGELOG.md#k-queried-guarded-chains-20261119) continues that proof when an intermediate reference can be absent. Acceptance covers later guarded reads through a stable selection with one or several possible referents, successive helpers, prepared cleanup, and rejection when a present path skips the write or a rebind selects unwritten storage. [K-QUERIED-GUARDED-INLINE-FIELDS-20261119](../CHANGELOG.md#k-queried-guarded-inline-fields-20261119) applies the same query proof to inline fields through a nullable selected reference. Acceptance covers nested inline fields, prepared assignments to a retained receiver across rebinding, and rejection for unwritten siblings and present paths without the write. [K-EXECUTABLE-EMBEDDED-SELECTION-20261119](../CHANGELOG.md#k-executable-embedded-selection-20261119) applies the selected-reference proof to executable field access through immediate scopes, helpers and prepared cleanup. Acceptance covers a checked null dereference, a guarded read after a conditional prepared write, and rejection after changing the slot values or skipping a present-path write. [K-SAVED-EMBEDDED-RECEIVER-20261119](../CHANGELOG.md#k-saved-embedded-receiver-20261119) carries the same proof to a saved reference after an intervening rebind. Acceptance covers guarded reads through a saved name, stored null tests and rejection when the saved reference is itself rebound to an unwritten target.

- **Prepared Boolean arguments in type queries.** [K-QUERY-PREPARED-BOOLEAN-20261119](../CHANGELOG.md#k-query-prepared-boolean-20261119) continues the full-scope profile under H-FULL-SCOPE-IMPLEMENTATION-20261118 and H-445. Readiness follows the prepared value and the selected receiver across ordinary calls and deferred cleanup. Acceptance covers constant and compound conditions, a later operand or cleanup changing the original slot, and rejection when an unwritten path remains possible. [K-QUERY-INPUT-GUARDS-20261119](../CHANGELOG.md#k-query-input-guards-20261119) continues the proof at callee entry. A reader can test an equivalent prepared Boolean to prove readiness of the same selected target, including after a forwarded call or prepared cleanup. Readiness still follows the prepared value and current target. Inspection remains nonexecuting.

- **Checked failures of double braces.** [H-DOUBLE-BRACE-THROWS-20261118](../CHANGELOG.md#h-double-brace-throws-20261118) confirms the [standalone checked interface](#checked-failures-of-standalone-full-scopes) in r93 and the [contextual callback rule](SymbolicDynamicScope.md#trailing-callback-arguments). D-17-FULL-SCOPE/P-564–P-566 and C-63/P-235/P-236 connect one inspectable call boundary to charter §§3.2/6.3/8. H-313/H-525 preserve declared obligations. The bounded r92 implementation profile carries forward with the standalone rule and acceptance cases below. [K-FULL-SCOPE-QUERY-20261118](../CHANGELOG.md#k-full-scope-query-20261118) applies its existing query checks to readonly writes and forbidden procedure effects. Wider callback lowering and deferred-pass mechanisms retain separate qualification.

- **Full-scope inputs and implementation.** [H-CALLABLE-MODEL-20261118](../CHANGELOG.md#h-callable-model-20261118), D-17-FULL-SCOPE r120/P-564–P-566. The r92 qualification continues for local control, inference, checks and cleanup. D-18r127 replaces independent outer inputs with direct lexical access. Acceptance below distinguishes writes to the original slot from a return local to the body. Mainline owns transport.

- **Qualified bounded dependencies:** [H-BOUNDED-IMPLEMENTATION-ADOPTION-20261117](../CHANGELOG.md#h-bounded-implementation-adoption-20261117)/[K-BOUNDED-IMPLEMENTATION-ADOPTION-20261117](../CHANGELOG.md#k-bounded-implementation-adoption-20261117) qualifies [D-17-DEFER-CHECKS](DesignAuthorityRegistry.md#d-17-defer-checks) semantic r3/P-561–P-563 and the surviving ordinary value inputs in D-18r115/P-485–P-487 within their stated acceptance and exclusions. The C-63/P-235/P-236 path continues. The static checker preserves eager preparation, independent callable bodies and nonexecuting queries; qualification does not establish compiler support or qualify full-scope control lowering, FIFO/aggregate/checked-pass, module/generator or new borrowed capture mechanisms. D-16 remains unchanged.

- **Named region roots:** [H-595](../CHANGELOG.md#h-595)/[K-NAMED-ROOTS-20261117](../CHANGELOG.md#k-named-roots-20261117), [D-13-ROOT-ACCESS](DesignAuthorityRegistry.md#d-13-root-access)/P-546–P-548, aligns this revision's regional examples with named roots, ordinary `r.name` assignment destinations and receiving specifications; the source root intrinsic is removed. Reuse C-74/C-65/C-72 and charter §§2/3.1–3.3/4.3/6.3/8/12 for explicit storage, access and lifetime responsibility. D-13 owns acceptance; preserve existing qualifications, actual-owner checks, nominal component identity, D-16 and D-18's existing qualification boundaries. H-596–H-599/D-22-REGION-INIT separately extends the root initializer surface while preserving whole-region cleanup; complete initialization proof/native coverage remains pending. Full structural/receiving proof and compiler migration remain separate; no C++ storage API changes follow.

- **Native-coverage reference continuity:** [K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) preserves this revision's D-26 dependency at its current AOT contract section. Only dependency links and this summary change; existing semantics, authority, acceptance and D-16 remain unchanged.

- **Full scopes.** H-FULL-SCOPE-CALLABLE-20261117 establishes local return destinations, separate cleanup and ordinary checked failures for double braces. D-18-HERE governs outer slots and bounded use.

- **Return inspection and cancellation:** [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state)/[H-542–H-547](../CHANGELOG.md#h-542), plus [H-548](../CHANGELOG.md#h-548)/[K-RETURNING-PROFILE-20261116](../CHANGELOG.md#k-returning-profile-20261116), covers r85's bounded return-state profile and cleanup interactions. [H-549](../CHANGELOG.md#h-549)/[K-RETURNING-BINDING-20261116](../CHANGELOG.md#k-returning-binding-20261116) confirms one-way use of the established result type and clarifies existing materialization failures. Human-confirmed recovery/filter/owner directions and cost-delegated scoped cancellation continue; H-548 delegates the remaining simple, coherent choices. The selected profile uses prepared local results, normal completion with or without a payload and ordinary bits, func or reference bindings; owning results support inspection without capture. Candidate C-54/C-63/C-65/P-482–P-484 serves author-controlled completion, observable result state and valid storage under charter §§2/3.2–3.3/4.3/6.3/8/12. Acceptance is below. Preserve FIFO failures, declared checked contracts, access/owners, exact release and D-16. No ownership-consuming binding, general regional copy or new pure-callable contract; full value-path/physical qualification and source support remain separate. Agent selections under delegation are not additional human confirmations.

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

- **Construction, propagation and OOM diagnostics:** [H-510–H-517](../CHANGELOG.md#h-510), [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve), [K-EXCEPTION-CONSTRUCTION-20261116](../CHANGELOG.md#k-exception-construction-20261116), records r72→73. H-510–H-515 separates immutable throw propagation from exception construction/trace capture, explicit constructor-form rethrow, implicit caught causes, bare wrapper rethrow and construction-only readonly cause. H-516/H-517 selects bounded process-level OOM information with the earliest record pinned and later records retained in a rolling window. Candidate C-36/C-63/C-65/P-464–P-469 relates inspectable failure provenance and bounded emergency diagnostics to charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve semantic checkedness, actual owners, stable exception identity and immutable provenance, owner-aware capture, deferred-queue boundaries and D-16. Acceptance and open physical/trace/aggregate policies are below; no source support, permanent ABI, cause-storage bound or full value-path qualification is inferred.

- **Deferred exception handling:** [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch), H-503/H-505/H-507–H-509 and [H-541](../CHANGELOG.md#h-541) govern local attached catches, queued bodyless handling, exclusion of ordinary body errors, once-per-entry finally and FIFO residual collection. Candidate P-461–P-463 connects C-36/C-44/C-63/C-65 to charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit failure coverage, observable order and valid resources. Preserve preparation, actual registration, type identity, contexts, nested scopes, owners and D-16. H-538 separately retains an ordinary body failure; H-539/H-540 retains later-return replacement and continued cleanup. Acceptance, completed-pass checked coverage and pending aggregate/physical policies are below; local decisions do not qualify the complete value path or establish compiler support.

- **Module shutdown order:** [H-501](../CHANGELOG.md#h-501)/[D-15-SHUTDOWN](DesignAuthorityRegistry.md#d-15-shutdown) selects application-wide reverse registration for module-level deferred actions after interleaved initialization. [K-MODULE-SHUTDOWN-20261116](../CHANGELOG.md#k-module-shutdown-20261116) records r69→70. Candidate C-36/C-44/C-59/C-63/C-65/P-458–P-460 serves charter §§2/3.3/4.3/6.3/12 through observable cleanup order and live owner obligations. Existing qualified per-action/per-scope preparation, failure, owner and nested-scope contracts continue. D-15 owns the 3/2/1 and partial-startup acceptance targets; D-17-DEFER-DISPATCH owns the handler distinction and remaining completion/representation questions. No runtime startup scheduler or new compiler-support claim follows.

- **Mainline continuity:** [K-719](../CHANGELOG.md#k-719) continues r67→68 while integrating the scoped assertion/naming decisions below. Current source/native capabilities, qualified value paths and unresolved mechanisms retain their separate boundaries; current compiler support is tracked in the source capability reference.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-17r68→69 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Type and predicate assertion failures:** [H-467–H-471](../CHANGELOG.md#h-468)/[K-ASSERT-SPECIALIZATION-20261115](../CHANGELOG.md#k-assert-specialization-20261115), [D-10-TYPED-EXPR](DesignAuthorityRegistry.md#d-10-typed-expr)/[D-10-ASSERT](DesignAuthorityRegistry.md#d-10-assert), P-411/P-413, integrates unchecked runtime mismatch and predicate failure with static resolution and explicit non-null refinement. Existing C-63/P-235/P-236 and charter §§3.3/4.3/6.3/12 preserve effects, ordinary recovery and cleanup. TypeAssertionFailedError/AssertionFailedError follow H-472 and select no ABI. Static failure rejects; unresolved failures need no throws/catch. Concrete-type operation selection preserves D-26 coverage and unknown-interpretation distinctions. Acceptance includes proof success/failure, runtime cleanup, checked operand failures and path-sensitive facts; broader syntax/evidence mechanisms and source implementation remain separate.

- **Default preparation and initialization:** [H-458](../CHANGELOG.md#h-458)/[K-DEFAULT-PHASE-20261115](../CHANGELOG.md#k-default-phase-20261115) records the selected entry, prepared-call and cleanup contract in [D-10-ARGUMENTS](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding). [K-715](../CHANGELOG.md#k-715) continues this scope together with the current bounds policy. Caller-context defaults prepare before entry; definition-environment defaults initialize in the entered callee. C-63/P-235/P-236 and charter §§3.3/6.3/12 preserve observable effect order and exact cleanup responsibility. Acceptance covers both failure phases and deferred registration/invocation; this adds no rollback, ownership restoration or implementation claim.

- **Bounds policy:** [D-17-BOUNDS](DesignAuthorityRegistry.md#d-17-bounds), r69 section below, records [H-448](../CHANGELOG.md#h-448)/[K-BOUNDS-POLICY-20261115](../CHANGELOG.md#k-bounds-policy-20261115). C-63/C-65/P-401–P-402 serves charter §§4.3/6.3/8/12: recoverable default bounds failures without mandatory throws annotations, with an explicit user-selected check opt-out. IndexOutOfBoundsError is unchecked; builtin indexing checks bounds by default. The opt-out transfers the range obligation to the program; invalid unchecked execution has no defined-result/exception/cleanup guarantee. Other type, owner/access, initialization, null and numeric contracts remain. Acceptance covers default recovery/cleanup, empty and boundary indices, read/write/view selection, proved check elimination and explicit opt-out isolation. The [source capability reference](../tests/cod/language/README.md) tracks the whole-artifact option and migration of its indexed test coverage. Broader indexing, mixed-artifact integration and varargs qualification remain separate.

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
| IndexOutOfBoundsError | Unchecked | Bounds checks for indexed access. Cause-link absence uses ordinary pointer rules. |
| UnexpectedCaseError | Unchecked, freshly constructed by an explicit throwing catch-all arm | Design only |
| MissingReturnError | Unchecked | Required-value fallthrough |
| RethrownException | Checked standard wrapper; requires a checked cause | Design only; constructor, trace and ownership integration unimplemented |
| RethrownError | Unchecked standard wrapper; requires an unchecked cause | Design only; constructor, trace and ownership integration unimplemented |
| UnhandledDeferError | Unchecked aggregate of residual deferred exceptions | The seed delivers and forwards the aggregate after the pass. It retains FIFO members and the separate body failure internally. Source projections of retained failures and member operations remain unimplemented. |
| UnsoundRuntimeTypingError | Unchecked | Category/vocabulary; outside-catalog typing validation remains separate |
| NullPointerException | Checked | Nullable target access or explicit non-null argument selection |
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
proc forward(exception e:Exception) throws Exception { throw e; }
proc forward_error(exception e:Error) { throw e; }
# Without throws or covering catch, forward's throw must reject,
# even if every currently visible call passes an Error.
```

An explicit throws Exception contract imposes a checked obligation on callers. A particular implementation, specialization or optimizer proof that only unchecked values occur does not erase that declaration. Ordinary checked propagation is allowed only through an appropriate explicit throws contract; module/entry code must handle its checked obligations. A type alias is not a narrowing operation. Explicitly selecting a narrower declaration or an admitted type assertion is a source change expressing the author's intent; diagnostic tooling must not apply it silently. Ordinary generic substitution follows the declared generic contract and does not rewrite an explicit Exception annotation. This boundary does not remove actual-type dispatch or other established flow proofs where their contracts permit them.

Keep **runtime failure classification** distinct from the **declared propagation obligation**. A root-typed value can actually be an Error, and catch matching must still use its actual type. Automatic rethrow from that value chooses the runtime-appropriate standard wrapper, but retains the checked source obligation of the Exception declaration; it is not a way to evade handling. Likewise a hidden tip selection does not narrow the declared obligation. Explicit fixed-wrapper construction must satisfy its cause constraints and cannot use inferred Error identity to bypass a broader incompatible declared cause contract. An explicitly named business exception may deliberately translate a failure under its own type's contract, as already selected; that is different from silently changing the source declaration.

**Design counterexample:** accepting a rejected Exception-typed throw merely because an initializer or current callers happen to supply Errors guesses an unexpressed narrower contract. Use a diagnostic to recommend the narrower declaration instead. Exact diagnostic formatting/severity and richer transparent throws summaries are separate.

**Bounded enforcement in the existing checker.** H-525 constrains ongoing implementation of the qualified declared-failure contract; enforcing that prohibition does not depend on first implementing the new Error root or arbitrary exception payloads. A change to actual-value or alias provenance must not introduce acceptance of an otherwise uncovered Exception-typed throw. Keep the declared obligation distinct from facts used for actual runtime category matching. This is a source-validation requirement, not a selected internal data structure or runtime carrier.

Resolve ordinary type identity and admitted generic substitution before applying the rule: an alias of Exception, or a declared T instantiated as Exception, retains the Exception obligation. A T instantiated under an admitted concrete unchecked contract has that concrete contract; current payload observations cannot silently change the substitution or an explicit Exception annotation. This follows ordinary semantic type identity rather than the spelling of the type token, and adds no generic exception-effect syntax.

Bounded acceptance includes rejecting uncovered root-typed throws despite known unchecked initializers, borrowed payload facts, catch bindings or helper results; retaining explicit throws Exception caller obligations; equivalent aliases/substitutions; ordinary covering root catches; and unchanged concrete unchecked propagation. Existing qualified ordinary catch and completion rules remain constraints. This does not decide whether narrower runtime-category evidence alone can establish coverage for a broad declared obligation, the full proof of transparent-wrapper throws coverage, or the changed deferred-error queue's recovery/suppression/aggregation rules. Keep those cases separately identified rather than using the current single-pending-error implementation to decide them. No proof improvement may revive H-525's rejected waiver; a missing coverage proof must be distinguished from a demonstrated language contradiction.

### Classification is not adapter-local

Once an exception type's defining contract establishes checkedness, adding conformance in an adapter cannot change it. In particular, importing a checked Failure and adding data Failure <: Error rejects, even though ordinary data-to-interface conformance is extensible. Aliases, selection context and load order cannot change that contract. An adapter may define its own new Error type and explicitly construct it with the original failure as cause; that preserves both identities and makes translation visible. Ordinary added conformance remains available when it preserves checkedness and satisfies its other contracts.

[D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) records H-527–H-535 disjoint declarations with existing ancestry among listed endpoints. Declaring type disjoint Error, Exception after Error <: Exception retains that ancestor relation as exempt, directly from the existing type graph; it therefore permits new Error-derived types without asserting an empty Error/Exception intersection. H-535's general rule rejects a subtype's second independent listed-parent relationship, including a checked Failure <: Exception later declaring Failure <: Error. This supplies the requested general mechanism for H-526. A redundant shorter declaration warns and preserves intermediate contracts. H-526's fixed checkedness rule and H-525's explicit Exception obligations continue; no hidden CheckedException root is introduced.

**Acceptance:** user-defined checked and Error-derived types; inherited uncheckedness and alias stability; Exception/Error/typed catches in source order; root-typed known Error throws still requiring checked coverage; actionable narrower-declaration suggestions; unchanged explicit throws Exception contracts across specialization/callers; automatic wrapper runtime category with conservative declared obligations; and rejection of checkedness-changing adapter conformance. General user payload construction/transport, full wrapper-aware throws inference and a physical type/identity ABI remain unimplemented or separately scoped. Existing finite intrinsic-category evidence does not prove these extensions.

## Exception construction, throw and rethrow

H-510–H-515 gives construction and propagation separate responsibilities. An exception's trace is captured when the exception is newly constructed. Throwing, forwarding, storing a reference to, or catching an existing exception preserves its identity, owner and construction trace. Only construction creates a new exception object.

| Form | Result |
| --- | --- |
| `throw e;` | Evaluate e once, select an existing exception through the catch-local identity rule below, and propagate it unchanged, including its type, fields, cause and trace. The operand may independently perform construction or other effects. |
| `throw;` | Propagate the nearest lexically available catch context's incoming tip unchanged. Requires that context under ordinary scope/capture rules. |
| `make exception T(...)` or `make exception T { ... }` | Produce a freshly constructed owning exception as a normal result. Choose fresh-region capacity or an explicit cause using the [placement grammar](#make-exception-expressions). |
| `throw SomeEx [from cause] { ... };` | Select the cause and its owner, or acquire fresh storage if omitted. Initialize the named exception and propagate the completed object. Its creation trace identifies this construction site. |
| `throw SomeEx(args...) [from cause];` | Construct the explicitly named exception operand, then propagate it. Construction captures the trace; throw adds no further exception or trace. Without from, no implicit caught cause is supplied. |
| `rethrow SomeEx(args...) [from cause];` | Construct and propagate the explicitly named exception with a fresh creation trace. An explicit from supplies its cause; omission uses the nearest lexically available catch context's incoming tip. |
| `rethrow;` | Construct a new RethrownException around a checked caught value, or RethrownError around an unchecked caught value, with the retained incoming tip as cause and a new creation trace; propagate the wrapper. Requires a caught context. |
| `rethrow from cause;` | Evaluate the explicit cause once, apply the catch-local identity selection below, then construct and propagate a fresh RethrownException for a checked cause or RethrownError for an unchecked cause. Capture a new creation trace; no caught context is required when the cause is explicit. |

Here SomeEx must resolve to an admitted constructible exception type; spelling an ordinary value or factory callable like a type does not make it constructor syntax. Type aliases and generic specialization follow their ordinary semantic identity rules. The constructor-form throw is explicit operand construction, not permission to attach a cause to an existing exception. `throw e from cause` and `rethrow e` for an already created value are not the selected forms. An ordinary factory expression can still be evaluated by plain throw; any exception construction occurs inside the factory under its own context.

**Field initialization at a throw site.** The braced form is a statement. The named type must be a constructible exception type. An explicit `from cause` goes immediately before the field block. Evaluate the cause once, apply catch-local tip selection, and reserve the target in that same region. With no cause, acquire a fresh region. Then evaluate the fields once in written order using ordinary data initialization. The braces introduce no callable or catch binding. An empty block still requires a complete object. Use `<-` for bits and `=` for reference binding. A nested ordinary `make` may use the proved destination owner of a reference field. A reference to a different, shorter-lived owner cannot serve as a graph transfer. To import such state, use an explicit graph transfer with a valid destination. The owner of the exception itself cannot be specified by the caller.

```cod
data RetryState { upload_id:u64; part:u64; }
data PartWriteFailed <: Exception { ref retry:RetryState; }

proc fail_part(id:u64, part_no:u64) throws PartWriteFailed {
    throw PartWriteFailed {
        retry = make RetryState { upload_id <- id; part <- part_no; };
    };
}
```

The nested `make` creates ordinary retry data in the field's owner. The caller can catch the completed `PartWriteFailed` and transfer that state to a retry queue. If allocation, initialization or preparation of an explicit cause fails, propagate that failure under the ordinary rules. A handler for the intended `PartWriteFailed` receives no partial object. Earlier initializer side effects retain their ordinary semantics. A subsequent `throw;` forwarding this failure preserves its object and trace.

For example, assume `take_failure()` returns an owning exception and `issue_ticket()` records a retry ticket.

```cod
throw RetryError from take_failure() {
    plan = make RetryPlan { ticket <- issue_ticket(); };
};
```

If `take_failure()` throws, that failure propagates before the ticket is issued. With an already-bound cause, write `throw RetryError from original { ... };`. On success, propagation takes the completed object and its owner.

### Make exception expressions

Use `make exception` to construct an exception and receive it normally. The result holds the new concrete object and its region. Omitting the placement clause selects a fresh region with the image default capacity. A later `throw` of that holder propagates the same identity and creation-site trace.

```text
make exception T(args...) [placement]
make exception T [placement] { field-initializers }
placement ::= in region (bytes -> capacity) | from cause
```

Brackets in the grammar mark optional syntax. Choose an ordinary constructor argument list or a complete field initializer. `()` selects an admitted niladic constructor. `{}` uses ordinary field initialization and must produce a complete object. A bare `make exception T` supplies neither. Resolve T using ordinary type lookup and specialization. It must denote a constructible exception subtype. The allocator's `capacity` label is distinct from a constructor parameter or a field with that name.

```cod
data SaveException <: Exception { job_id:u64; }

func diagnostic(id:u64, budget:u64) -> exception :SaveException {
    return make exception SaveException in region (budget -> capacity) {
        job_id <- id;
    };
}

exception saved:SaveException <- diagnostic(42u64, 50000000u64);
```

For fresh construction with parentheses, prepare explicit constructor arguments in source order, evaluate any capacity expression once, acquire the region and complete construction under the ordinary constructor contract. For fresh construction with braces, evaluate any capacity expression and acquire the region before running the field initializers in their written order. A fresh region with no explicit budget uses the final image default. Placement and payload initialization follow the same rules as the corresponding cause-less constructing throw. A nested ordinary `make` can use its field's destination owner. An escaping preparation or initializer failure propagates under its own contract. Unfinished construction releases the new region, publishes no result and preserves completed operand effects.

Making a checked type does not itself raise it or require `throws T`. Checked failures of arguments, capacity or initialization still need coverage. An owning result is delivered after required cleanup succeeds. Discard releases the holder. The later `throw saved;` consumes it immediately on successful propagation and incurs the checked obligation of its declared type. Using the make expression directly as a throw operand still creates one object and captures one trace.

Omitting `from` leaves the cause null, including inside a catch. Existing-owner placement (`in e`) and uninitialized reservation reject. `in region (...)` configures a new owner. Ordinary `make T` and a bare constructor call `T(...)` cannot bypass the exception constructor.

#### Constructing a normal result with a cause

Use one optional placement clause. With neither clause, acquire a fresh region using the image default. `in region (...)` acquires a fresh region with the given budget. The `from cause` clause constructs the new exception in the selected cause's existing region, retains that cause through the readonly link and returns the owning holder normally. Both clauses together reject in either order. Cause selection uses the usual catch-local tip rule.

```cod
data WriteException <: Exception { job_id:u64; }
data SaveException <: Exception { job_id:u64; }

func translate(exception original:WriteException) -> exception :SaveException {
    return make exception SaveException from original {
        job_id <- original.job_id;
    };
}
```

For `make exception E(f()) from g()`, prepare the explicit constructor arguments first, then evaluate the cause once. For braced initialization, `make exception E from g() { ... }` evaluates the cause once before reserving the object and evaluating its fields. The same grammar applies when the operand names an existing holder.

The cause supplies a non-null target and a provable handoff of its whole region. A plain borrowed argument alone cannot supply that handoff. While the new object is being initialized, a named source holder still covers reads such as `original.job_id`. At successful publication the result takes the one release obligation and the source holder is consumed. The old object remains reachable through `.cause`, along with its original trace. The new object has its own construction trace and uses the existing region's remaining capacity.

If initialization fails, no outgoing result is published. A still-live source holder remains responsible for the region, so a local handler can continue using it. If evaluation instead produced an owning temporary, its ordinary failure cleanup applies. Effects or independent ownership transfers already performed by operands stand. Failed construction in a live region creates no allocation rollback promise. Construction with a cause adds no implicit throw of either exception type.

##### Keeping a typed reference to the original

The named `exception` binding is consumed on successful handoff. Keeping an independently declared reference follows [D-14’s local transfer rule](RegionalOwnership.md#local-view-validity-after-ownership-transfer). The declaration records the target, its access and its actual owner while the source holder is available.

```cod
func translate_for_job(exception original:WriteException) -> exception :SaveException {
    ref detail:const WriteException in original = original;
    exception wrapped:SaveException <- make exception SaveException from original {
        job_id <- original.job_id;
    };
    report(detail.job_id);
    return wrapped;
}
```

Assume `report` accepts the scalar ID. On success, `wrapped` holds the same region. `detail` still selects the original `WriteException` with its declared access. Its `in original` names the region established at declaration time. A later read does not evaluate the consumed binding again. Using `original` itself to inspect, transfer or throw still rejects. This leaves each declaration with one mode on every path. If the author wants to keep the spelling `original` for reads, a separately named exception holder can be borrowed into a ref named `original` before the handoff.

A direct local make gives the checker the owner equality. Track the live holder of that instance across normal and exceptional completion. A local use of the saved reference needs proof of live, permitted access on every reaching path. Passing the holder to a consuming helper whose contract gives no such proof prevents subsequent use. Releasing it ends the borrow. This uses existing owner evidence and requires no extra per-reference ownership accounting. The direct case has one known transition. Joins, callbacks and independently compiled helpers still require their ordinary lifetime proofs.

If the only later use is to report the immediate cause, the readonly `wrapped.cause` view suffices under its existing null and access rules. In a transparent catch, that link can select the incoming tip. A typed borrow taken from the matched binding continues to select the matched business exception. This difference matters to code that needs fields specific to that type.

##### Implementation cost of preserving the borrow

**Disposition:** keep the H-263 same-region rule and defer compiler support for a typed borrow used after `from original`. The source `exception` binding remains consumed. Until the compiler can prove the surviving borrow, diagnose its post-handoff use as unsupported. An application needing only an ID or code can receive its bits before wrapping. The caller can also inspect the immediate cause while the wrapper lives when a readonly root-typed view suffices.

The [checker](../cod/lang_checked.hh) has separate availability, target and owner facts. Its `owner_alive` recognizes external owners and active lexical scopes. Membership in an active scope cannot establish that a releasable region instance still has a live holder. The [checked result modes](../cod/lang_checked_program.hh) cover values, borrowed refs and borrowed ptrs. The [LLVM exception value](../cod/lang_llvm.hh) contains five scalar fields. The [runtime cause table](../cod/runtime/cod_lang_runtime.c) stores diagnostic records by index. A borrow of an intrinsic value or its lexical slot therefore cannot serve as proof or native transport of the original business object inside a transferred region.

| Work | Engineering estimate and scope |
| --- | --- |
| Shared owning-exception groundwork | Multi-week work from the present compiler, with low schedule confidence until the representation is selected. Represent the actual region and selected target on normal and thrown outcomes, place typed payloads in it, preserve a distinct incoming tip when applicable, and validate the exact construction/publication/cleanup boundary. These are dependencies of owning exceptions themselves. |
| Direct named local handoff, once those dependencies work | Roughly 1–2 engineer-days for checker and lowering integration, assuming a stable target locator and reusable region-instance liveness evidence. Retain the original locator, reassign responsibility on the success edge and use the existing ownership state on the failure edge. |
| Focused verification of that direct case | Roughly 1–3 engineer-days. Check that success reads the original subtype’s field, use of the consumed name rejects, a local catch of initializer failure retains the source, and use after releasing or potentially consuming the final holder rejects. Check consistent behavior on shared and specialized native paths and one release per region, including injected allocation failure. Existing [region cleanup fixtures](../tests/cod/language/owned_region_tests.py) provide patterns. The [intrinsic exception borrow fixtures](../tests/cod/language/exception_borrow_staging_tests.py) exercise a different representation. |

These are planning ranges for one engineer familiar with the seed compiler. Revisit the direct case when the checker can distinguish a holder from the instance it covers and both native outcome paths transfer the selected object and owner with exact cleanup. The incremental estimate is 2–5 engineer-days at that point. It excludes proving the same fact through arbitrary calls, captures, loop iterations, deferred work or joins with different owners. Accept those additional cases as their ordinary proofs become available. A cast, a still-live lexical name or a second release obligation cannot stand in for the missing proof.

### Returning a locally caught exception

A handler that already owns a caught exception can return its holder normally. This requires neither a new object nor a second trace. A factory creating a fresh diagnostic can return the [owning make result](#make-exception-expressions) directly. A `ref T in O` result instead requires proof that O covers the caller's use. The previously discussed result-only borrowed-catch signature still needs a specified release holder when no other owner reaches the caller. It supplies no implicit handoff for `make exception`.

**Cause initialization.** An explicit canonical cause comes from a constructor’s `from` clause on a constructing throw, rethrow, or [normal make with a cause](#constructing-a-normal-result-with-a-cause). Positional arguments, named arguments, ordinary field initializers and parameter defaults cannot set it. The cause becomes readonly at publication and cannot be rebound or mutated through an alias or assertion. A business payload containing another exception remains distinct from the reserved cause relationship. Construction without a cause leaves the cause pointer null.

The readonly built-in `e.cause` yields a **nullable ptr** to the immediate cause of e. Its target owner is always the owner of e, including when the pointer is null. Every link in a cause chain therefore has one common actual owner. The readonly catch field [`e.tip`](#the-catch-bindings-readonly-tip-field) yields a **non-null ref** to the incoming exception. Both targets have type `Exception` and are readable through these fields. Neither field permits rebinding its link or writing through it. Reading an absent cause yields an ordinary null pointer with its owner evidence. Testing it with `is null` yields true. A payload read through that pointer follows the [checked null-dereference rules](#checked-nullpointerexception). The bounds-check switch has no effect on either field.

```cod
catch (e:const Exception) {
    report(e.tip);
    ptr c:const Exception = e.cause;
    if (c is not null) { report(c); }
}
```

Here report accepts a readonly Exception target. The null test reads no target. On the guarded path c supplies the proven non-null target. A read of `e.tip` always supplies a target, even when e arrived unwrapped. Both views require a single statically provable owner and valid backing for each use, including later use by a reporter. Binding or passing the view supplies no independent lifetime extension. A capture or return of the exception carries its actual owner dependency. See [exception object owners and release](#exception-object-owners-and-release).

Each exception has at most one immediate cause. Follow successive `.cause` links for deeper history. This is independent of `UnhandledDeferError.exceptions`, which contains distinct failures from the deferred pass. The indexed `cause_at` intrinsic is removed from the language design. The [source capability reference](../tests/cod/language/README.md) tracks the compiler migration.

For an incoming chain W2 → W1 → E → C, where E is the first application exception, an ordinary catch binds e to E. Then `e.cause` is C, `e.tip` is W2, and `e.tip.cause` is W1. Cause selection follows one link on the specified receiver. It does not perform the catch-local substitution that applies to `throw e;`.

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

The named constructor's semantic type determines matching and checkedness. A checked cause does not make an explicitly constructed unchecked business type checked, nor does an unchecked cause remove a checked business constructor's obligation. Standard rethrow wrappers have the additional cause constraint below. Bare rethrow and rethrow from cause choose the corresponding wrapper using the selected cause's actual semantic checkedness, even through the root Exception type or aliases. Catch matching uses the wrapper-prefix rule below: only consecutive standard rethrow wrappers are transparent. A named business exception or UnhandledDeferError stops standard-wrapper cause traversal. That traversal does not itself search arbitrary causes or aggregate members; H-537 places the finished deferred aggregate on the ordinary exception path, not back in the completed defer-handler pass. Checked failure coverage must reflect both the incoming checked type and the selected transparent matching rule; the [application-facing contract](#application-facing-failure-contracts) specifies ordinary checked declarations and the conservative wrapper-only case.

### Exception as an owned region and a selected target

The exception holder uses ordinary region ownership for the complete lifetime. Write `exception e:T` in a declaration or parameter, `-> exception :T` for an owning result, and `catch (e:T)` to receive the same kind of holder. The explicit spelling `own exception e:T` denotes the same holder. A catch also has its existing incoming-tip context. A nonconsuming helper declares `ref x:const T in O` or its writable counterpart. A fresh, cause-less construction establishes a distinct logical region on each execution. Its backing and layout can be optimized subject to ordinary owner identity, observable failure and lifetime rules.

**What the holder contains.** The semantic pair is an owned region `er` and a non-null locator for the selected `T` in that region. `in e` identifies `er`. The concrete exception and its regional business payload live in er. Creation-site trace retention follows the separate OOM diagnostic contract. For a structural root sketch use `region ExceptionStorage { ref root:Exception; }`. `Exception` is an abstract, non-bits type. An inline `val root:Exception` cannot hold every concrete exception's fields under the [ordinary root rules](ConstructionAndAcquisition.md#region-construction-and-named-root-initializers). The implementation allocates the concrete subtype and binds a root reference to it. These are internal operations. Application code does not receive a rebindable root, an independently consumable region handle or general permission to construct an exception with `make T in er`. A holder transfer keeps the selected locator and its actual owner together.

```cod
data SaveError <: Error { job_id:u64; }

func diagnostic(id:u64) -> exception :SaveError {
    return make exception SaveError { job_id <- id; };
}

proc inspect(ref x:const SaveError in O) where {const owner O} { report(x.job_id); }

proc submit(id:u64) {
    exception d:SaveError <- diagnostic(id);
    inspect(d);
    throw d;
}
```

`make exception T { ... }` allocates the region on demand, completes the concrete object, captures its creation trace and yields the holder normally. It has no explicit destination owner. Ordinary allocation or initializer failures still apply. Constructing a checked exception this way does not itself raise that checked type. `throw T { ... }` without a cause performs the same construction at that site and immediately hands it to propagation. `throw d;` transfers an existing holder and preserves the creation trace. Raising a checked type still requires ordinary coverage. Failed preparation of a fresh construction publishes no holder and cleans up its unfinished region. Completed initializer side effects keep their ordinary semantics.

| Path | Where the release obligation goes |
| --- | --- |
| Existing holder thrown | Successful throw takes it immediately, before unwinding or catch selection. Propagation owns it across intervening cleanup and skipped clauses. The matching catch takes it. Direct use of the consumed binding already rejects while the region is still live. |
| Catch completes, returns or throws | Normal handler exit releases a holder it still owns. `return e;` into an owning result prepares a handoff through required cleanup. A cancelled result is cleaned once. A successful `throw e;` or `throw;` hands the responsibility onward. A local ref can borrow from the caught holder while it is live. |
| Factory result or ordinary call | A normal owning result goes to the caller on successful completion, and discard releases it. An owning parameter consumes at entry. A reference-only result needs an independently covered owner, such as an input or a retained owner in the same structural result. An owning factory directly returns [the fresh holder](#make-exception-expressions). A reference alone supplies no release obligation. |
| Borrows, branches and recovery | `ref w:const SaveError in d = d` borrows the selected object. Holding w never duplicates the obligation. At any later use, prove the actual owner still live on every path and check access. A writable handler can allocate an ordinary retry graph `in e`. A durable queue can receive independent bits or an explicit graph transfer before release. Ordinary regional field/relocation constraints apply, including the prohibition on storing callable views in that graph. |
| Typed catch through wrappers | Given incoming W2 → W1 → E, a catch for E owns the same region and selects E. Its readonly `e.tip` selects W2. Matching never writes into an exception object. The handler can report W2 or propagate it unchanged. An owning parameter acquires the single holder and selects W2 when its destination type admits W2, otherwise E when E fits. Catch-local tip selection remains lexical. The exception root's private representation must support these different locators. A saved incoming locator suffices without a back-reference on E. |
| Readonly handler and broad static type | An owning `catch (e:const T)` can release and forward its holder while preserving readonly target access. A separate consuming helper would need compatible access. Moving responsibility cannot upgrade a readonly binding. A `throw` at declared type `Exception` keeps its checked obligation under H-525. |
| Existing cause | Every link must stay in one actual owner. A wrapper with cause e must be constructed in e's region. After successful construction the new selected object and the same region travel together. Bare rethrow uses the incoming tip. A failed translation cannot publish a partial wrapper or undo earlier initializer effects. Any still-live input holder follows its ordinary cleanup path. |

For example, evaluating `rethrow;` inside a catch constructs a new wrapper and captures a trace. Using a fresh region for the wrapper would create a forbidden cross-owner cause link. Building the wrapper under the current holder uses the same static ownership rule as enriching a failure with a retry plan. A cause-less `throw OtherError {};` in the same catch establishes a new owner. The two mechanisms have different owner choices even though both create a new exception object. For an explicitly supplied cause, a plain borrowed helper argument gives target access only. Constructing and propagating a wrapper additionally requires the ability to hand off that region. For both braced throw and [normal make](#constructing-a-normal-result-with-a-cause), evaluating the cause establishes the owner before the first field initializer.

#### Region and members of UnhandledDeferError

At the end of a defer pass with residual failures, raising `UnhandledDeferError` constructs the aggregate in a **fresh region**. Its internal dynamic array holds `exception` values. For each independently thrown member, its entry retains the original selected exception and its owned region. Building the aggregate hands that responsibility on from the completed pass. An empty pass creates no aggregate. Forwarding an existing aggregate transfers its existing holder.

```cod
proc close_job() {
    defer { throw FlushError {}; }
    defer { throw CloseError {}; }
    throw JobError {};
}
```

Assume three application Error types and all three raises execute. There are four regions, one per original failure and one for the aggregate. The array contains CloseError then FlushError in residual FIFO order. JobError is retained separately through the readonly `body_exception`. Array elements refer to the original objects in their own regions. Their payloads and creation traces survive the cleanup queue. A nested aggregate stays one member with its own contents. Each live entry carries its own unique release obligation. Two independently thrown errors of the same type, even with identical payloads, occupy separate entries. A handler forwarding the same object transfers its one holder from the selected occurrence to a new tail occurrence. The two occurrences are successive. The separately retained body failure keeps its own holder. Array growth preserves each member’s owner. Disposing of the aggregate releases its region and the holders still in its custody.

The array is specific to this built-in. An ordinary same-region offset cannot represent a member's ownership. Inspection borrows the selected member under its actual owner while the aggregate covers it. Publication delivers the complete result of the pass. Explicit extraction then transfers just the selected member. General owning arrays in user data and a serialized or native element layout remain separate choices.

##### Enumerating and taking deferred failures

`batch.exceptions` exposes the pending unchecked failures as an ordered collection. Its element mode is `exception` and its static element type is `Error`. The aggregate's own region stores the collection. Each member carries its own actual owner. `body_exception` is a separate readonly association and may hold a checked exception. It contributes no array index. A nested aggregate occupies one index.

An owning exception can supply a target view through ordinary reference binding. For example, assume `read_failure()` returns an owning exception with writable target access:

```cod
exception e:Exception <- read_failure();
ref v:Exception = e;
ref log_view:const Exception in e = e;
```

The initializer of `v` infers the owner named by `e`. A readonly source permits the readonly target view. Both views select the existing object; `e` keeps the region's release obligation. Subsequent uses of a view require its actual owner to remain provably live. Selecting a catch holder this way yields the matched target; `e.tip` explicitly selects its incoming tip.

Read-only enumeration is this operation applied to each member. A readonly batch supports inspection. Taking and draining require writable access to the collection. The following spellings are proposals. Assume a writable batch for this example and an in-range index `i`.

```cod
visit(batch.exceptions) (index, v) => {{
    report(index, v);
}};

exception selected:Error <- take_at(batch.exceptions, i);
retry(selected);

for exception e:Error <- drain(batch.exceptions) {
    report_or_retry(e);
}
```

The [common collection operations, owner and failure rules](CollectionsAndBuiltinContainers.md#proposed-common-operations) govern these proposed spellings. On this builtin, a visit borrows `ref v:const Error in O` with the selected member’s actual owner for each callback call. Taking one transfers that object, its region, cause chain and original trace. The other members keep their FIFO order. The batch can be forwarded with its remaining members and its separate body exception. Taking the last member leaves the existing batch live with zero members. The initial publication rule applies when the deferred pass constructs the aggregate.

##### When a defer pass cannot publish its aggregate

Register an action once its operands and action record are ready. Registration can raise ordinary `OutOfMemoryError` if its own storage allocation fails. Earlier operand effects stand, the successful prefix still drains, and the failed entry supplies no action or handler coverage. Lack of a separate reservation for a hypothetical future aggregate is no reason to fail registration.

Run all registered cleanup under the ordinary FIFO, handler and checked-coverage rules. An OOM already present as a body or deferred failure follows those same rules. When backing is available, publish the complete aggregate even if it contains an OOM. Implementations can reuse storage already held by the pass.

If the completed pass actually cannot acquire backing for the aggregate, propagate `OutOfMemoryError` to the enclosing ordinary exception handler. An unrepresentable checked extent follows the existing `InvalidCapacityError` rule. Both failures occur at aggregate construction, after the pass has drained. Keep whatever summary fits using the [bounded process OOM reserve](#bounded-process-oom-information) for OOM, for example the failed site, the residual count and whether a body exception was pending. Report unavailable detail honestly. The original failure objects and body exception can be lost to the caller on this path. Dispose of the unpublished aggregate and release each retained member owner and any separate body owner exactly once. The reporting failure needs no implicit cause link.

For example, if both closes in `defer { close_index(); } defer { close_data(); }` fail, successful publication delivers data then index. If only publication runs out of memory, the same two closes ran and an outer `catch (e:OutOfMemoryError)` receives the reporting failure. If a deferred handler handled both errors, there are no residuals to publish and any pending ordinary body exception propagates as usual.

#### Image setting for exception-region capacity

The final program image selects the default byte budget for fresh exception regions across incorporated modules. Use **100 MiB (104,857,600 bytes)** as the starting default on 64-bit targets with suitable virtual memory. An image for a tighter address space or a workload with many live failures can choose another budget. The image records the setting without embedding zero-filled regions in the executable. A fresh construction site's explicit budget takes precedence for that one allocation.

A fresh cause-less exception uses that budget once. Adding recovery data or constructing a same-owner wrapper uses the original region's remaining capacity. Catching, forwarding or returning an existing exception reserves no second range. Each fresh aggregate needs its own region in addition to the retained member regions. An existing region whose remaining budget cannot accommodate a placement raises `RegionCapacityExceededError`. A reported host failure to reserve or supply backing takes the `OutOfMemoryError` path. Basic OOM reporting needs independently available backing to avoid recursive acquisition.

One native strategy reserves a stable range at acquisition, enables backing a page or modest chunk at a time as allocation advances, and touches only storage that initialization needs. This preserves the fixed capacity, owner and locators. Full-capacity commitment, clearing or prefaulting at acquisition would defeat the intended resource behavior. On release, the backend can unmap or recycle the range and backing. With a full reservation per region, 1,024 live ordinary regions reserve 100 GiB of address space at this default before overhead. Both peak live region count and payload use matter when choosing an image setting.

Windows separates address reservation from commit charge, and page access determines physical residency. Linux accounts private writable mappings under the host's overcommit policy. Reserving inaccessible address space and enabling a writable prefix incrementally can defer that charge. Strict overcommit ignores `MAP_NORESERVE`. Exact RSS, page granularity and survival under OS-enforced memory pressure are host properties. The portable target is to avoid eagerly backing or touching the full capacity. [Windows virtual memory allocation](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc) and [Linux overcommit accounting](https://docs.kernel.org/mm/overcommit-accounting.html).

**Aggregate larger than the image default.** After the defer pass the residual count is known. At construction select the larger of the image default and the checked extent required for the aggregate root, all member holders, the separate body field when present, and their layout/alignment. The member payloads remain in their own regions. Their capacities and the budgets at the original throw sites do not enter this calculation. An existing region keeps its fixed capacity. Failure to construct the complete aggregate follows the [publication rule](#when-a-defer-pass-cannot-publish-its-aggregate). Backend acceptance covers stable locators, many live small failures, full capacity, allocator failure and exact nested cleanup. The [next-stage native runtime plan](../applications/cod-booter/README.md#next-stage-repositories-and-native-runtime) records where to investigate these guarantees.

#### Capacity at a fresh construction site

The placement clause is `in region (bytes -> capacity)`. It requests a fresh exception region with the given byte budget. Omission uses the image default. The clause follows constructor arguments or precedes a braced field initializer, matching the placement position in `make T in r { ... }`. The named allocation argument uses the [region construction](ConstructionAndAcquisition.md#region-construction-and-named-root-initializers) convention. Each site can choose a smaller or larger capacity.

```cod
# QuotaError's constructor itself has a parameter named capacity.
throw QuotaError(requested -> capacity) in region (50000000u64 -> capacity);

data SaveError <: Error { job_id:u64; }
proc fail_job(id:u64, diagnostic_bytes:u64) {
    throw SaveError in region (diagnostic_bytes -> capacity) { job_id <- id; };
}
```

In the first example, `requested` goes to the ordinary constructor and 50,000,000 bytes to the allocator. For `throw E(f()) in region (g() -> capacity);`, prepare the explicit constructor argument `f()`, then evaluate `g()` once, acquire the region, and complete construction under the ordinary constructor contract. If `f()` fails, `g()` does not run. In the braced form, evaluate the budget and acquire the region before evaluating any field initializer, so nested payload construction has a live destination. In either form, a failure preparing the budget or acquiring storage publishes no requested exception. Completed argument effects keep their ordinary semantics. The budget follows ordinary region capacity typing and validity rules.

Only a fresh cause-less exception construction can carry this clause. `throw e;`, `throw;` and rethrows retain the existing owner. `throw DomainError from e {};` must place the new object in the cause's region. A fresh-region clause on those forms rejects at compile time, including on a call returning an already constructed holder. There is no implicit resize. To tune such a holder, specify its budget at creation. For a normal make expression, write `make exception SaveError in region (diagnostic_bytes -> capacity) { job_id <- id; }`. Nested independent throws have their own budgets. Compiler-created residual aggregates use their own sizing rule.

### Exception object owners and release

Every exception object, including an intrinsic failure with only scalar diagnostic fields, has one statically identifiable owner. A normal result returning the object declares that relationship, as in `-> ref SaveError in ex where {owner ex}`. A bare `-> SaveError` cannot expose an independent bits value. A propagated failure keeps its object and owner through cleanup and matching. An ordinary caller variable that holds a reference has its own lexical storage. Its lifetime does not determine the lifetime of every other reference to the object.

Program state stored in the exception must be established in its owner. A numeric status field can receive bits. When the diagnostic needs a graph from a request region, the program explicitly transfers the required graph into exception-owned storage before releasing the request region. A field assignment that merely saves a reference back into the request cannot accomplish this. Construction with a cause creates the new exception in the actual owner of the selected cause, after applying the catch-local tip selection. This includes bare rethrow, named translation and explicit `from`. The link retains the original cause object and trace. Reject a contradictory owner requirement. Application fields imported from another owner must satisfy their ordinary transfer and placement rules. Never transfer or duplicate the cause to satisfy the rule. Owner equality alone grants no user allocation or write permission. Owner evidence needed by an initializer must be available when it executes; preserve the specified evaluation order and single evaluation of an explicit cause expression.

A virtual exception scope may release its storage once no outer operation still depends on that actual owner. Count a returned reference, a live catch tip, pending propagation, a deferred queue, a retained cause and an aggregate (including its body exception) for as long as they can still be observed. A discarded result with no such dependencies can be cleaned up even if the application continues. An unrelated saved diagnostic must remain valid. No source-visible application-wide arena, tracing collector, per-reference counter or particular release instruction is prescribed.

**Exception construction selects its owner.** A constructing throw or rethrow obtains storage from a virtual exception-handling scope on demand. This applies even when a local handler catches the exception and delivers it to the caller as a normal result. An existing-owner placement `in owner` rejects even if it names a writable handling owner. Fresh make and fresh constructing throw permit `in region (bytes -> capacity)` to configure their new owner. Apply this rule to the resolved constructed type, including through a type synonym or generic specialization. Runtime-generated failures and deferred aggregates retain their specified creation and cause contracts. Throwing an existing object preserves its identity and owner.

An `in ex` on a reference declaration or borrowed result describes the selected object's actual owner and requires a compatible binding. It cannot direct the allocation into a request region or cause an implicit transfer after construction. The rule applies only to exception construction. A handler with a writable holder e can allocate ordinary application data with `make RetryPlan in e`. Constructor arguments keep their established evaluation order. Transfer of a request-owned payload graph into the new object requires a valid destination in the handling scope; evaluation, transfer or initialization failure cannot publish an incomplete exception. No new scope backing is required merely to enter a procedure or perform a type query. Fresh exception identities do not by themselves imply distinct owner identities or one physical allocation each.

**User-defined reference fields.** In `data SomeException <: Exception { ... }`, each ref/ptr field has an actual target owner under the ordinary field, placement and lifetime rules. The compiler must prove and track that owner across construction, rebinding, capture, propagation, graph transfer and later dereference. The storage for a field slot belongs to the containing object. A reference in that slot is interpreted with its target owner's evidence, including for a null pointer. Shared owner evidence can serve several fields when their owner is proved identical. Where a field's layout requires a same-owner target, enforce that requirement on every write. Any otherwise admissible external dependency must remain live for every retained use. An annotation on the exception itself cannot establish that proof.

An explicit graph transfer can bring application state from a region or lexical scope into the exception owner and can export selected state to another eligible owner. The destination receives a graph valid under its own layout and owner constraints. Check and reconstruct the relevant reference edges according to the selected transfer, including shared or cyclic structure when that transfer supports it. An inbound transfer followed by release of the source and an outbound transfer followed by release of the exception scope must both leave the transferred payload usable. Merely binding a field to a source target records the original dependency. Plain propagation retains the existing exception and its provenance.

**The caught holder names the owner.** In `catch (e:T)` the pending failure hands e its release obligation. In `ref v:T in e = e` and `make RetryPlan in e`, the same name identifies the live owner. Each match can receive a different actual owner. Catch selection uses source order and the nominal [wrapper matching rule](#wrapper-aware-catch-matching). It has no test against a previously named owner. If a helper or result requires a particular O, ordinary owner checking must establish that relationship at the use. A generic [`where {owner O}` constraint](RegionalOwnership.md#confirmed-owner-syntax) names an actual owner and authorizes the applicable operations. Release responsibility comes from holding or receiving an owning resource. Payloads may enter or leave the exception region under the [transfer rules](ConstructionAndAcquisition.md#storage-eligibility-and-payload-transfer). The matched target and incoming tip share the owner even when they select different objects.

### Exception owners across callable boundaries

A constructing throw establishes the virtual handling owner on demand. An escaping failure carries its owner through intervening calls and cleanup, including module initialization and shutdown. A `throws T` declaration describes a checked failure that may escape. It supplies no source-level owner name or allocation authority to the propagating callable. A handler uses its caught holder to name the matched owner. These rules apply to functions and procedures with their ordinary effect permissions.

A borrowed normal result can name an owner supplied by an input, with the usual access and lifetime proof. A fresh owning result supplies its owner directly. Two independent calls that each return a freshly allocated exception region establish distinct live owners. Borrowing from one and passing the borrow where the other owner is required rejects. Forwarding a failure through an ordinary callable needs no universal exception-owner parameter.

Static owner identity and lifetime are separate obligations. During propagation the pending failure keeps its required handling storage valid. During a normal return the result must carry that obligation across callee cleanup and into every live receiving use. If cleanup abandons the result, its obligation ends unless another live dependency survives. Retained causes, incoming tips, pending deferred entries and aggregate members count as such dependencies. For a wrapper W around E, a catch that matches E binds their common owner. A reference bound to `e.tip` denotes W and a reference bound to e denotes E. Both depend on that owner. The intrinsic deferred aggregate's members and optional body failure can originate in separate chains. Their retention requires separate accounting. Request storage used to build a payload retains its own lifetime.

Each cause chain has a common owner. Independent failures have no implicit owner-equality guarantee. A physical implementation must demonstrate how surviving uses keep handling storage live and how unused backing is released. Bulk release alone cannot guarantee bounded memory when a long-running service keeps one diagnostic while discarding unrelated failures allocated under that same owner.

### Who releases the exception-handling scope?

A catch can name the actual handling owner. Constructing a wrapper in that owner creates no second cleanup obligation. For example, a caller can lend its owned failure to an inspection helper, then hand the same failure to a relay:

```cod
proc inspect(ref w:const SomeException in O) where {const owner O} {
    report(w.code);
}

proc relay(exception b:SomeException) throws SomeException {
    throw b;
}

proc submit(exception saved:SomeException) throws SomeException {
    inspect(saved);
    relay(saved);
}
```

The caller retains the obligation through `inspect`. At entry to `relay`, the callee takes it. Its throw passes the same object and its whole scope into propagation. An exit or another use of the consumed holder cannot release the scope a second time. Pending results across `finally`, deferred entries and retained aggregate members must also be covered. Internal cause links or cyclic application payloads cannot by themselves keep the scope alive.

A pending throw, handler or normal-result destination must hand off responsibility before it retires if a subsequent use requires the owner. On failed construction inside an already live scope, unwind the abandoned construction. The original cause and other live targets survive. The [owning factory](#make-exception-expressions) needs this protection across a successful normal return.

**One cleanup responsibility for the entire owner.** For deterministic release, associate one obligation with the virtual handling scope. At any point one live holder is responsible for discharging it. That holder may be the pending exception, an internal cleanup slot, a pending normal result or the caller's owning variable. Borrowing `e`, its `.cause`, its `.tip` or its payload creates no new obligation. The compiler must prove that all such uses occur while a holder covers them. Transfer responsibility at the successful handoff, retiring the previous holder. If result delivery is cancelled, perform the usual cleanup for the abandoned result. Release exactly once on the responsible holder's exit when no still-admitted borrow requires the scope. Multiple possible runtime holders on different control-flow paths are compatible with one actual holder per path.

#### Exception declarations and catch bindings

`own exception e:T` designates an exception object of type T and holds its entire handling scope. In a field selection, e denotes the selected object. In `in e`, e denotes the actual owner of that object and all other targets allocated there. Distinct exceptions in a cause chain can share that owner while retaining distinct object identities. A wrapper's cause and the matched exception can therefore supply different targets with the same lifetime.

| Declaration | Local use | As a callable parameter |
| --- | --- | --- |
| `exception e:T` or `own exception e:T` | The two declarations are equivalent. Initialize or transfer the selected object and its single scope cleanup obligation with `<-`. Use `in e` to name the handling owner. | Both are consuming parameters. The caller relinquishes the holder at callee entry after successful argument preparation. The callee must release it or transfer it onward on every exit. |
| `ref w:T in O` or `ref w:const T in O` | Bind a non-null target in the named owner with `=`. Target access follows the declared type. The holder covering the borrow must remain live. | Borrow the target under the declared owner and access relationship. A helper may inspect or update permitted fields. The caller retains release responsibility. |

```cod
catch (e:UploadInterrupted) {
    ref w:const Exception in e = e.tip;
    report(w);
    e.plan = make RetryPlan in e {
        job_id <- job;
        upload_id <- e.upload_id;
        receipts = e.receipts;
    };
}
```

This catch acquires the scope's unique obligation from the pending failure. With a transparent wrapper, e designates the typed matched object and w designates the incoming tip. The readonly target of `e.tip` rejects the writable spelling `ref w:Exception in e = e.tip` under the [ordinary receiving rule](ConstructionAndAcquisition.md#readonly-reference-reception-and-explicit-bits-transfer). For a second borrowed name of the matched object, use `ref b:UploadInterrupted in e = e` when the target is writable, or `ref b:const UploadInterrupted in e = e` for readonly access. To transfer it to a new owning local use `exception next:UploadInterrupted <- e`. This consumes e. The catch binding's virtual tip belongs to this handler invocation. A separately defined helper receives the target selected for its owning formal, or the explicitly borrowed target for its ref formal. Declaring an ordinary local or parameter with `exception` does not import a caller's catch context.

A catch header binds a name with an optional target annotation. `catch (e:T)` receives the owning exception and permits writes to T’s writable fields. `catch (e:const T)` explicitly requests readonly target access and still holds the same release obligation. Omitting the type annotation in `catch (e)` means `catch (e:Exception)`. The binding e has static type `Exception` and owns the caught holder. The same grammar applies in try, attached defer catches and bodyless defer catches, once per matched failure. The grammar excludes declaration prefixes (`ref`, `exception`, `own exception`) and catch-header `in` or `where` clauses. Contextual words still work as binding names, for example `catch (exception:T)`.

To lend the caught value, bind a local ref in e or pass e to an ordinary ref parameter. A local ref, parameter or result borrows under its lifetime proof. A consuming parameter or owning result can take the holder. On an ordinary handler exit, release the owner exactly once if e still holds it. The readonly virtual `.tip` and the readonly cause link keep their own access rules irrespective of the target annotation.

**Handoff on propagation.** A fresh constructing throw supplies the initial cleanup responsibility. A relay whose parameter is `exception e:T` already has that responsibility and can forward it with `throw e;`. A relay with only `ref w:T in O` cannot supply an owning catch by throwing w. Writable target access and `where {owner O}` for allocation do not let it take its caller’s holder. Within a catch, the established rules for `throw;` and catch-local identity selection still select the incoming tip; forwarding requires the pending or caught whole-scope responsibility to remain available. Selecting the same object or owner after handing the holder away cannot recover that responsibility.

A compiler-managed alternative for the bounded factory example is a caller cleanup slot associated with the reference result. Local throw/catch would use the slot only when needed, the caller would release it on discard or exit, and forwarding the only live result would forward the cleanup obligation. This avoids a new source spelling while retaining a real owner internally. Independently stored views and deferred entries still need a covering holder for their admitted uses. Any runtime bookkeeping used to establish that coverage must preserve a single release obligation per actual owner. Static elision or this slot alone supplies no general release proof. Same-owner cause links require no per-link reference counting.

The H-EXCEPTION-FACTORY-20261118 factory illustrates the normal result handoff. When the result carries the sole release obligation, an `own exception` result delivers the complete scope. An ordinary borrow with independently live backing carries its ordinary lifetime constraint. The source contract for the existing factory result and the accounting for deferred aggregates containing independent failures must both support the whole-scope handoff required by an owning catch.

**Recovery state carried during propagation.** When a multipart upload fails, the storage layer knows the upload ID and which parts the server acknowledged. The job layer knows the job ID. The retry worker needs all three after the request region has ended. The storage layer constructs `UploadInterrupted` with the upload ID, a receipt graph explicitly transferred from request storage into the exception owner, and a nullable `plan` initially set to null. The job layer installs its recovery plan on that same exception.

```cod
data UploadInterrupted <: Exception {
    upload_id:u64;
    ref receipts:ReceiptSet;
    ptr plan:RetryPlan;
}

proc send_job(id:u64) throws UploadInterrupted {
    try { upload_parts(); }
    catch (e:UploadInterrupted) {
        ref plan:RetryPlan in e = make RetryPlan in e {
            job_id <- id;
            upload_id <- e.upload_id;
            receipts = e.receipts;
        };
        e.plan = plan;
        throw;
    }
}

try { send_job(42u64); }
catch (e:const UploadInterrupted) {
    if (e.plan is not null) { enqueue_retry(e.plan); }
    else { report(e.tip); }
}
```

The two IDs are bits. The plan and the receipts already share the same owner, so binding `receipts` to `e.receipts` retains that graph's identity. Forwarding preserves the incoming tip and construction trace. The later typed handler sees the installed plan even through a transparent rethrow wrapper. Here `enqueue_retry` is an application procedure that explicitly transfers the plan graph into queue-owned storage before returning. The worker can then reconcile unacknowledged parts with the server, accounting for the case where a write succeeded but its acknowledgement was lost. Synchronous recovery may instead borrow the plan until the handler finishes. Successful uploads need no exception-state transfer.

**Readonly target access.** `ref w:const T in O` permits observation under O. Handing a failure to an owning catch requires release responsibility, which a helper holding only w cannot provide. Propagating the incoming failure through a catch that has its handling responsibility remains distinct from trying to acquire a caller's holder through a readonly argument. The holder's ownership and the access supplied by an individually projected target are checked separately.

**Acceptance and remaining work.** Exercise a handler retaining only a bits status after releasing its exception, returning a diagnostic with a transferred graph after the request owner ends, and preserving an existing cause through forwarding and eventual release. A stored cause chain retains its common owner. An aggregate may retain independently raised members. Their lifetimes and the unique-owner evidence for each projected member must be accounted for before admitting that path. Cover fresh construction by throw and by `make exception`, rejection of ordinary `make T` for exception subtypes and of direct `in o` placement, explicit placement of ordinary payload data in the handler, a normal caught-reference result, partial construction and discarded or replaced results. A nested construction with a cause must preserve both identities within the same actual owner. A bounded process OOM diagnostic window may evict trace details only under its own contract. Exception storage, reuse guarantees and cross-owner retention require a qualified implementation design.

#### Choosing the target of an owning catch handoff

In `exception own_e:Exception <- e`, a live catch holder `e` supplies its incoming outer exception and the one release obligation for its actual region. An owning formal `exception x:T` uses the same reception as a local `exception x:T <- e`. An explicitly typed owning result receives a returned catch holder by the same rule. Each handoff preserves the selected object's identity, existing cause chain and creation trace.

First establish the receiving type under ordinary declaration, type inference, generic substitution and callable selection. Those steps use the matched type of `e`; the hidden tip contributes no additional overload candidate. For each possible arrival, select the incoming tip if its actual nominal type is assignable to that receiving type. Otherwise select the catch's matched object if its type is assignable. The compiler must establish coverage for every admitted arrival before accepting the operation. A wrapper-only destination therefore needs proof that a compatible wrapper arrived. If neither target fits, reject at the handoff. The search stops at the matched object, even if that business exception has a compatible cause of its own.

For `RethrownException → RethrownException → UploadInterrupted`, an owning `Exception` destination receives the outermost wrapper, and an owning `UploadInterrupted` destination receives the matched failure. With a direct arrival both receive the original object. If both targets satisfy a non-root destination, the tip still wins. For example, `exception saved:Error <- e` in a catch for `IndexOutOfBoundsError` preserves an incoming `RethrownError`. Each example describes an independent handoff.

```cod
func take_tip(exception x:Exception) -> exception :Exception {
    return x
}

try { upload_parts() }
catch (e:UploadInterrupted) {
    val id <- e.upload_id;
    exception saved:Exception <- e;
    enqueue_failure(id, saved)
}
```

The handler can also pass `e` directly to an owning `Exception` formal, including `take_tip(e)`. Such a helper is ordinary user code. Its parameter already designates the selected target, and forwarding that holder preserves it. Only the live catch binding supplies the matched/tip pair for this reception. An ordinary holder that already received the narrower matched target cannot reconstruct an earlier wrapper by widening to `Exception`. In a nested handler, an explicitly used outer catch binding supplies its own incoming tip.

Select the object once. Owning reception transfers the rights of the holder and enforces the receiving access qualifier. A readonly catch can feed an owning `const Exception` destination; a writable destination requires writable access. Field and trace protections remain in force. Reading `e.tip` explicitly still yields a readonly borrow and supplies no release obligation. An owning local consumes at successful transfer, a call at callee entry after argument preparation, and a pending owning result follows the existing cleanup rules. Verify a failing later argument leaves the catch holder available, a callee failure after entry cleans or forwards it exactly once, and a second handoff from the consumed binding rejects. Plain catch-local `throw` and `from` retain their identity rules. A helper that throws a parameter declared `Exception` still has the declared checked obligation.

### Exception ownership across control flow

For each path, identify the selected exception object, the actual owner that contains it, and the holder responsible for releasing that owner. A catch can select an inner object while the pending failure hands over the whole owner. Reading an object, comparing identities, taking a bits snapshot or checking a type does not transfer that obligation.

| Operation | Consequence |
| --- | --- |
| Initialize a fresh failure | Publish only after its fields and cause are ready. Failure during preparation propagates the failure that actually occurred. A previously live cause and completed side effects survive under their ordinary contracts. A cause-less throw inside a catch has no implicit cause. Fresh objects alone prove neither equality nor inequality of owners. |
| Enrich recovery state | A writable catch can install application data in its owner. A readonly payload view can be reported while covered. To retain a receipt graph in a retry queue after the handler releases its owner, explicitly transfer the selected graph into queue storage. A reference back into request storage still requires that request owner on every use. Cycles internal to the exception owner create no external release obligation. |
| Pass arguments, including defaults | Two consuming formals cannot both take the same holder in one call. An earlier argument may inspect the exception and produce independent bits before a later parameter takes it. On a pre-entry failure the call has not taken the holder. Independent argument effects stand. Failure of a definition-site default after entry cleans the transferred input on the callee side. The caller cannot retry with its consumed holder. |
| Join branches or repeat a loop | A consuming use is valid on mutually exclusive branches when each taken path owns the exception. At a join or next iteration, another consuming use requires a live holder on every reaching path. A new occupant of the same variable cannot cover a borrow of a released instance. A handoff that keeps the actual owner live can cover a borrow when its full use is proved. |
| Inspect, propagate or translate in a catch | Inspecting the matched object or its tip borrows a target. Direct forwarding of the caught failure keeps the incoming tip and trace. A fresh wrapper takes its cause in that same owner and captures a trace. Once the handler has handed the owner to a consuming helper and that call has completed without returning it, a later bare throw cannot recover it. Checked obligations still follow the declared type. |
| Delay or capture a use | A prepared consuming deferred call needs the selected holder available until callee entry during the drain. Registering a second call that would consume it again rejects. An immediate full scope or separately defined helper follows ordinary argument and cleanup rules. An admitted borrowed capture must prove the owner live at each use. A repeatable callable cannot acquire independent exception owners by capturing one holder as a value. |
| Return and run cleanup | A candidate owning result is delivered only after required cleanup succeeds. Replacing or cancelling it disposes of the abandoned obligation exactly once. `if returning ()` can recognize that normal completion without extracting the hidden owning result. In a structural result, keeping a borrow while discarding its sole owner requires a separate liveness proof. An independent bits output can survive discard of the owner. |
| Drain deferred failures | Reverse execution of registered actions determines arrival order. Bodyless handlers visit the pending FIFO. Each occurrence retains its target and owner through its disposition. Forwarding hands the occurrence onward. A residual aggregate retains the ordered occurrences and the optional separate body exception through inspection. Successive forwarding of the same identity transfers one holder on each hop. Each simultaneously retained member owns a separate region. |
| Cross a generator or module boundary | A loop-body failure bypasses generator catches, including a catch around `yield`. Its owner survives required generator cleanup. A new escaping cleanup failure and a pending body failure follow the [generator cleanup rules](RangedForAndGenerators.md#cleanup-completion-at-the-for-boundary). Module startup failure and reverse-order shutdown keep each pending owner live through actual registered cleanup and final reporting. |
| Exhaust diagnostic storage | Reusing an OOM information slot may make trace detail unavailable. A live failure keeps its own identity, fields, cause and retained aggregate members. Ordinary allocation cannot be required for basic OOM reporting. The physical storage protocol remains open. |

For example, an upload service can retain the upload ID for audit and transfer the diagnostic to a relay in the same call:

```cod
proc send_failure(id:u64, exception failure:UploadInterrupted) throws UploadInterrupted {
    audit_id(id);
    throw failure;
}

proc fail_job() throws UploadInterrupted {
    try { upload_parts(); }
    catch (e:UploadInterrupted) {
        send_failure(e.upload_id, e);
    }
}
```

Here the first argument prepares independent bits. The second transfers responsibility at entry. If `audit_id` fails after entry, the callee cleans up the original owner during unwinding. If it succeeds, `throw failure;` propagates the selected `UploadInterrupted` unchanged. With an incoming transparent wrapper, direct `throw;` in the original catch preserves the outer tip. Here the helper’s owning formal is `UploadInterrupted`, so it receives the matched object. Replacing the first parameter with a consuming exception and passing `(e, e)` would require two handoffs and rejects.

A nested catch makes the handoff observable even when both handlers are in one callable:

```cod
catch (outer:UploadInterrupted) {
    val id:u64 <- outer.upload_id;
    try { throw outer; }   # Successful throw transfers the holder to propagation here.
    catch (inner:UploadInterrupted) { report(inner.tip); }
    audit_id(id);          # Valid. id holds independent bits.
    report(outer.tip);     # Reject. The throw already consumed outer.
}
```

The successful throw consumes outer before handler search. The use would still reject if inner passed the owner onward and its region remained live. Here inner handles the failure and returns normally. The outer handler's frame still exists. Its saved ID is independent bits, and the old binding no longer covers a use of the exception. The same liveness check applies to a previously saved reference to its tip. If the inner try instead raises and handles a separate failure while the outer handler retains its holder, the outer diagnostic remains available.

**Open source choices in the reference-factory baseline.** A factory returning only a reference needs an identified release holder at the receiving site when the caller is the only remaining user. The owning result of `make exception` already carries its release obligation. [D-30](CollectionsAndBuiltinContainers.md#proposed-common-operations) covers collection operations, with the [exception member rules](#enumerating-and-taking-deferred-failures) governing owner-bound inspection and removal of one independent holder. These questions concern observable source behavior. They grant no additional capture, result, collection or exception ABI contract.

### Standard wrapper construction and diagnostics

H-523 requires RethrownException to have a checked cause and RethrownError to have an unchecked cause. These constraints apply to semantic type identities, including aliases, and to the constructor forms of throw and rethrow. A standard wrapper without a cause rejects. Constructor-form rethrow may supply that cause implicitly from a valid lexical catch; constructor-form throw requires an explicit cause to build a standard wrapper. Business exceptions may deliberately translate a failure under their own type contracts; this restriction belongs to the two transparent standard wrappers.

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

Required diagnostic content for the mismatch includes the selected wrapper, the cause's conflicting checkedness and a direct `rethrow from e;` suggestion. Illustrative text: `RethrownError requires an unchecked cause, but e is checked. Use rethrow from e; to select the matching wrapper automatically.` An unknown checkedness diagnostic must say that it could not establish the fixed wrapper's requirement rather than claiming a proved mismatch. Preserve the user's actual cause expression in a suggested replacement. Exact formatting is not a new language ABI.

Every valid consecutive standard-wrapper prefix consequently has one checkedness and one canonical wrapper category. An explicit wrapper catch matches and binds the outermost incoming wrapper of that category; there is no differently classified inner standard wrapper to search for. Ordinary catches still match the first non-wrapper exception, and all clauses retain source order. A business exception ends that prefix even if its own cause has a different checkedness.

**Acceptance:** checked-to-checked and unchecked-to-unchecked named construction; symmetric mismatch rejection and diagnostic suggestions; missing causes; implicit constructor-rethrow causes; root-typed known/refined versus mixed causes; automatic selection inside/outside catches; aliases; same-identity helper results; one evaluation and one new wrapper trace; unchanged source exceptions; allocation/preparation failure; and no checkedness change along a valid standard-wrapper prefix. Physical storage and the remaining throws/inspection contracts are not settled by these rules.

**Trace and storage boundary.** An exception constructed at a throw site, caught and returned by a helper retains that construction trace when the caller propagates it later. A new rethrow wrapper records the new construction context while preserving the original exception and its trace as cause. A new UnhandledDeferError likewise has its own construction trace; its exceptions field preserves each member's existing information. No first-throw capture, automatic trace refresh, implicit captured-value recapture or mutation is allowed. Logical/native frame selection, optimized-frame reconstruction, maximum trace detail, access API and persistent representation remain engineering/design scope; the bounded source-site metadata in the current compiler does not establish full traces. Construction can require storage; plain propagation must not require a new wrapper or recapture allocation. Ordinary creation/chain-allocation failure and deferred-queue membership/replacement must be kept distinct.

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

Stop traversal at a business exception even if it has a cause. Stop at UnhandledDeferError rather than searching its exceptions field; H-537 routes this completed aggregate through ordinary propagation/catch after the defer-handler pass. An explicit wrapper clause binds the incoming outer wrapper. H-523 excludes mixed standard-wrapper categories within a consecutive prefix, so no deeper category search is needed. A root catch, written `catch (e:Exception)` or `catch (e)`, matches all remaining exception types in source order. Matching must not construct a new exception, duplicate a wrapper graph or capture a new trace. Immutable chains allow reuse of the unwrapped locator/type during one clause scan.

### Application-facing failure contracts

Declare and catch the checked failure that application code can handle. Use `rethrow;` or `rethrow from e;` to add the current construction trace while retaining the incoming chain. CoD strongly discourages naming `RethrownException` or `RethrownError` in ordinary application code, including public signatures. This is usage guidance. The explicit wrapper forms remain legal under their existing rules.

```cod
using "std:exceptions" NullPointerException;

proc relay(exception e:NullPointerException) -> void throws NullPointerException {
    rethrow from e;
}
proc retry_layer(exception e:NullPointerException) -> void throws NullPointerException {
    try { relay(e); }
    catch (caught:const NullPointerException) { rethrow from caught; }
}
```

Suppose `e` is an unwrapped exception. The first call constructs wrapper W1 with cause e. The catch in `retry_layer` binds `caught` to e and retains W1 as its incoming tip. Its rethrow constructs W2 with cause W1. The caller can catch `NullPointerException` and receive the original e. Both intermediate traces survive in `W2 → W1 → e`. Inside the same catch, `rethrow;` gives the same chain, and `throw;` propagates W1 unchanged. An explicit cause with a different identity remains that exact exception.

An ordinary checked declaration such as `throws NullPointerException` covers both a direct failure of that type and a transparent rethrow of that declared failure. A caller need not add a wrapper clause. If an API explicitly publishes only `throws RethrownException`, a checked failure delivered under that declaration has a wrapper, but the declaration identifies no particular underlying failure. Catching only `NullPointerException` cannot cover the general obligation. An explicit wrapper catch or a covering root catch can. For a business exception with its own cause, matching stops at the business exception even if a deeper cause would have matched an earlier clause.

### Matched binding and hidden tip

H-520 prefers retaining wrappers wherever the operation can use them, including explicit from e. H-521/H-522 selects a hidden tip reference with the matched catch binding e and **exception identity within the current lexical catch context**, including an unchanged exception returned by a helper. Keep e's payload access and ordinary nominal/actual type evidence consistent with the matched exception; retaining tip must not make typeof, generic inference or ordinary overload selection silently switch e to the wrapper's type.

The model keeps two existing targets: the matched exception and the incoming outer exception. No exception graph copy is required. Retaining/copying a small stable handle when required by capture/lifetime rules is not duplication of the graph. The hidden tip must refer to valid exception backing, not a mutable pending slot or an ended catch frame. Repeated deferred-handler invocations receive separate associations; nested catch exit restores the enclosing context rather than leaving a mutable last-caught value.

#### The catch binding’s readonly tip field

For a bound catch parameter e, `e.tip` is an intrinsic readonly virtual field yielding a non-null reference to a readonly `Exception` target. It selects the outermost exception received by **that binding’s handler invocation**, with its existing creation trace and cause chain. For a direct, unwrapped arrival the target is the matched object itself. For an explicit standard-wrapper catch the binding already denotes the incoming wrapper, so both select that wrapper.

```cod
try { run(); }
catch (e:const NullPointerException) {
    report(e.tip);  # Report the incoming object with its full chain.
    throw e;        # Propagate that same object unchanged.
}
```

The existing postfix member syntax parses this spelling. The checker resolves the intrinsic against the catch binding and reads its retained incoming value at runtime. Two catches can bind the same underlying object with different tips. In nested handlers, `outer.tip` and `inner.tip` select their respective incoming objects whenever both names are visible. Each invocation of a deferred catch likewise has its own association. Ordinary capture and lifetime rules apply when a selected tip is retained for later use.

Reserve `tip` as a field name on Exception and its subtypes. Other data types may use it normally. The intrinsic provides readonly target access. The catch retains the association, and the field cannot be rebound or supplied to a constructor. A helper borrowing e gets the matched exception. Pass `e.tip` for a borrowed view of the incoming object. An owning formal receives the target chosen at handoff. Its parameter has no catch field even if it is also named e. The root type provides one reporting interface. Propagation also requires the whole-scope handoff. When admitted, a root-typed throw retains its declared checked obligation under H-525.

**Identity selection.** Evaluate the exception operand once. At a throw-value or explicit from operation with a current lexical catch context, compare its exception identity with that context's matched exception. If they are the same, select that context's tip; otherwise select the operand itself. Without such a context, use the operand unchanged. Bare throw, bare rethrow and omitted rethrow cause directly select that context's tip. This is selection of an existing exception, not mutation or implicit wrapper creation.

In the catch that bound e, `throw e;` and `throw;` propagate exactly the same incoming object. With an incoming chain W2 → W1 → E they both propagate W2 with its existing trace. `rethrow;` constructs W3 with W2 as cause. A consuming relay with an owning `Exception` formal receives W2. With an owning formal of E’s concrete business type it receives E. A plain throw of that parameter in the relay propagates the received target. Passing `e.tip` to a borrowed reporting parameter lets the reporter see W2 and both intermediate traces. That readonly projection alone cannot initialize a consuming parameter. An ordinary parameter declared `Exception` still carries H-525’s checked obligation.

Identity is not equal type, fields, message or trace, and is not a relocatable raw address. Newly constructing an exception creates a distinct identity even with identical fields. Reference bindings, stored references, arguments and forwarding returns retain the existing exception's identity; they do not transport an ambient catch context or secretly change the nominal type. The selected behavior requires a stable identity/handle implementation, in addition to retaining tip. Current five-word intrinsic values do not establish that representation. No universal public identity ABI, general RTTI or physical heap allocation policy is selected. Runtime construction in a loop or recursive call must produce a fresh identity on each execution, even at an identical source site with equal fields. Value/capture/field/outcome transport and extracted causes or aggregate members preserve existing identities. Source-site IDs and reusable OOM diagnostic slots do not satisfy this requirement. A finite token/handle implementation needs a live-identity uniqueness and exhaustion/reuse contract; its observable fallback remains unselected. Identity comparison may be folded when proved, but that proof does not waive H-525's declared checked obligation. The compiler review identifies transport through the current five-word carrier and retained backing as real implementation work; one local hidden reference alone does not implement the full value-identity contract.

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
| Ordinary typing, payload access and borrowing e | The matched exception under ordinary type, owner and access rules. |
| Owning reception from a catch holder | [Prefer the incoming tip under the receiving type](#choosing-the-target-of-an-owning-catch-handoff). Transfer the one obligation with the selected object. |
| Bare throw, bare rethrow, or an omitted rethrow cause | The lexical catch context's incoming tip, preserving full wrapper history. Bare throw propagates it; rethrow constructs the selected new exception. |
| Explicit from operand or throw-value operand | Evaluate once; use tip exactly when the operand has the current context's matched exception identity. This includes unchanged helper results and aliases. |
| An operation in a separately defined helper | No access to its caller's catch context. A result returned unchanged can still match identity at a subsequent operation inside the caller's catch. |
| Passing `e.tip` to a reporter | The existing incoming exception, including its original chain. The reporter needs no caller catch context. |
| Other trace/category inspection, or a request to bypass local throw/from identity selection | Additional inspection facilities remain under review. |

The current context is the nearest lexically enclosing catch available at the operation, with its normal capture/lifetime checks. Do not search every active catch by identity or implicitly import a caller's context. If two nested catches have the same matched exception with different tips, the inner context governs operations in its body; leaving it restores the outer context. Identifier spelling, grouping and helper expression shape do not override this choice. A delayed operation retains the selected matched identity and tip/owners; it does not consult the unrelated catch or pending error active when invoked.

### Remaining wrapper-matching decisions

1. **Inspection and explicit extraction.** Explicit reporting selects the matched exception with e or the incoming exception with `e.tip`. Additional trace/category APIs and any deliberate bypass of catch-local throw/from normalization remain open. Under H-522, an expression yielding the same matched identity is normalized at a catch-local from/throw operation regardless of its syntax; assigning it to an alias alone is not an escape hatch. Reading a cause is not itself construction and must not mutate a chain or produce a graph cycle. No generic exception-view ABI or new inspection API is selected.
2. **Transparent checked obligations.** H-523 fixes cause presence and equal checkedness along the standard wrapper prefix. Static coverage must still correlate incoming outer type with matchable leaf type rather than treating independent category sets as arbitrary pairs. The [application-facing contract](#application-facing-failure-contracts) fixes the meaning of ordinary failure declarations and an explicitly named wrapper. Representing correlated outer/underlying alternatives across generic calls and control-flow joins remains checker work. A declared root obligation retains H-525’s coverage requirement. Runtime depth must not create one generic instantiation per wrapper level.

**Acceptance:** direct and multiply wrapped inputs; source-order ordinary/root/explicit-wrapper clauses; stopping at business errors and aggregates; matched e versus retained tip; once-only operand evaluation; identity preserved by an unchanged helper return or reference binding and distinguished from equal-field reconstruction; nested same-leaf/different-tip contexts, shadowing and delayed/repeated catches; absence of implicit caller context in separate helpers; checked outer/leaf correlation; no graph duplication; and each resolved inspection/wrapper case above. Existing exact-category matching and five-word intrinsic values do not establish this behavior. Owner-safe stable identities, hidden locators and normalized leaf caches are implementation candidates, not qualified public representations.

## Selected direction

CoD provides `try`, `catch` and `finally` control in the accepted contract. Ultimate out-of-memory failure is an **unchecked exception**: calling allocation does not require a checked-error result, a `throws` declaration or an enclosing catch merely to compile. A caller may catch the failure; otherwise it propagates toward the execution boundary. Unchecked does not mean ignoring ownership validity or turning invalid references into runtime exceptions.

`make` continues to produce its normal `own region :Root` result when construction succeeds. OOM is an exceptional exit, not an implicit null owner or a mandatory optional/result wrapper. H-265 does not select automatic allocation retries, region growth, compaction or a process recovery policy. “Ultimate” failure describes exhaustion after any separately defined policy has failed; no such policy is introduced here.

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

The current source compiler exposes `--no-bounds-checks` for an entire compiled source artifact, including imported modules and every general/specialized implementation. Generated calls retain the selected policy independently of runtime-source emission. A failed checked index reports the call site, requested index and available count through the existing exception fields. The source capability reference owns runnable coverage. Broader indexing, separate artifact/link/cache integration and static range diagnosis remain implementation work. Generated/shared/cached code must preserve the requested policy: an unchecked variant cannot silently satisfy a request for default-checked execution. This policy selects no permanent ABI or runtime registry.

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

A possibly null dereference and explicit `not null` selection of a possibly absent ref argument both incur the checked NPE obligation. The latter inspects reference identity and can throw before callee entry without reading the target. A statically absent selection throws when reached. A proven-present selection adds no NPE obligation of its own. See [owner-qualified reference arguments](RegionalOwnership.md#owner-qualified-reference-arguments). `null` is a reserved literal, never a binding. H-440/H-441’s `p is null` and `p is not null` evaluate the reference operand once and examine its selected owner/locator without loading the target; merely rebinding a pointer with `=` does not dereference it. Ordinary payload argument evaluation, including copying bits into val, must account for null access in the caller. A valid non-null ref needs no NPE obligation. Stable pointer tests may establish a non-null view on the corresponding branch; mutations invalidate the fact, and checking a mutable field does not prove a later reload unchanged.

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

A standalone full scope invokes its niladic here body once when execution reaches the expression. Its local variables, return state, checked failures and cleanup follow ordinary call rules. It uses the enclosing activation’s slots directly. A materialized handle or physical call is optional.

- A write to an enclosing scalar or a rebind of an enclosing mutable reference slot is visible after the call. Each access requires the actual owner and current locator. Fresh parameters and declarations inside the body have their ordinary local storage.
- Every return targets the nearest callable, including this implicit callable. Its deferred/finally returns replace only its own retained result. Only its own deferred entries can inspect that invocation’s return state. To return from the outer callable, write an outer `return {{ return value; }};`. An inner return supplies no result-inference evidence to the outer callable.
- An explicit local return or a normal value tail prepares the result. Both `{{ 7u64; }}` and `{{ return 7u64; }}` return 7. D-24's independent inference, required-value completion and expected-signature rules apply. Returned references must outlive this invocation; a reference to its local storage cannot escape merely because invocation was immediate.
- Without an expected result, a standalone body whose only completion is throwing defaults to `void`. Thus `typeof({{ throw e; }})` is `void` for a valid exception operand, while `typeof({{ return 7u64; }})` is `u64`. The same implicit invocation and result contract apply when the expression executes. A throwing path delivers no result. The default supplies no recursive inference anchor, waives no source checks, and does not change the callback parameter’s expected signature.
- Func/proc permissions remain those admitted at the expression's checking site; introducing the implicit callable cannot grant procedure effects in a function. Lexical slot access, initialization, checked failures, effect restoration and cleanup use the ordinary call rules. The full scope's `here` is its own invocation owner. Module definitions and module lifetime are not turned into implicit callable captures by this rule.
- A continue or return written in the full scope needs a destination inside that callable. The scope may call a lexical here parameter, including a generator's `yield`. The supplied body's own exits follow its lexical control destinations. Ordinary catch context and its hidden tip likewise do not become ambient across an ordinary call: bare throw/rethrow needs a catch in the implicit callable; explicitly supplied exception values follow ordinary propagation rules.

```cod
proc write_outer() -> u64 {
    val n <- 1u64;
    {{ n <- 2u64; }}
    return n;                         # 2: the body wrote the enclosing n.
}
proc local_return() -> u64 {
    {{ return 1u64; }}
    return 2u64;                      # 2: inner result was discarded.
}
proc borrowed_target() -> u64 {
    ref n:u64 = 1u64;
    {{ n <- 2u64; }}
    return n;                         # 2: write through the enclosing reference.
}
proc cleanup_return() -> u64 {
    defer { {{ return 9u64; }} }      # Local result discarded.
    return 2u64;                      # Still 2 if cleanup succeeds.
}
```

Caller-context defaults use the same local-return rule and D-10 lookup. A temporary receiving scope keeps its newly declared locals and exposes permitted updates to existing destinations. A full scope inside a generator loop body still has its own return.

A delayed block controls when its contained full-scope expression is reached. Thus `defer { {{ ... }} }` constructs/invokes that full scope during cleanup; it does not capture those inputs at registration. The direct full-scope block spelling `defer {{ ... }}` has that same delayed-body boundary. Plain deferred-call syntax still prepares its actual callable/arguments eagerly. Grouping alone introduces neither boundary.

**Acceptance.** Check the four outcomes above, nested cleanup, independent result inference, writes and rebinding, actual owner release and exceptional control.

#### Checked failures of standalone full scopes

The result type and checked failures are independent. A standalone throw-only scope with the default void result can still have a nonempty checked interface. A standalone `{{ ... }}` implicitly declares the checked obligations that can escape its invocation. Compute them statically under ordinary exception checking. Include explicit throws at their declared types, declared failures of calls, and checked operations such as a potentially null dereference. Apply local handling and ordinary completion rules first, including new failures from a handler or `finally`. An entirely handled failure adds no obligation. Unchecked errors can propagate without a checked declaration. The caller of the implicit callable must handle or declare each remaining obligation.

```cod
using "std:exceptions" NullPointerException;

proc load_required() -> void throws NullPointerException;

proc sample() -> void {
    try { {{ load_required(); }}; }
    catch (caught:const NullPointerException) {}
}
```

The call inside the braces contributes `throws NullPointerException` even though the block contains no literal `throw`. A direct constructing throw of that checked type contributes the same obligation. Moving the covering catch inside the block leaves its checked interface empty. Removing the only catch requires `sample` to declare the failure. An operand declared `Exception`, or a call declared `throws Exception`, contributes the general obligation even when its runtime value or implementation is known to be narrower. Nested standalone scopes compose through their immediate calls. Failures while preparing captures belong to the enclosing evaluation. Checked failures from a deferred action still need coverage within its owning defer pass.

A [trailing callback](SymbolicDynamicScope.md#trailing-callback-arguments) already has a callable parameter signature. That signature sets its allowed checked failures. An explicit helper also keeps its own ordinary declaration requirement.

### Full-scope implementation profile

Every standalone full scope uses the same checked body in validation, result inference, type queries and execution. It has its own `here` owner, return record and cleanup. Its free lexical names retain their declaration identity and designate the supplying activation’s slots. Check access, initialization, concrete target owner and possible invalidation at each use. A deferred full scope starts when its containing deferred action actually runs.

```cod
proc example() -> u64 {
    val n <- 1u64;
    val inner <- {{
        defer { n <- 3u64; }
        return n;
    }};
    return n + inner;                 # 4: inner saved 1, cleanup wrote 3.
}
```

If a full scope executes `p = other;` on an enclosing mutable reference, the next read of p in the supplier sees the new binding. Returning a reference to the inner scope’s own storage still rejects. A type-only query validates the same accesses without executing them. Explicit parameters of separately called helpers keep their ordinary passing contracts. Checked failure inference, early exit, unwinding and local return replacement retain their D-17 rules. See [D-18](CallableStorageAndHereAccess.md#what-an-invocation-needs) for direct access across recursion and backing growth. Native representation is delegated.

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

**Forwarding and replacement.** Throwing the selected value appends a new propagation occurrence of the same exception; it does not create an exception or refresh its trace. Constructor-form rethrow creates an exception under its own contract, then appends that escaping failure. Failure while constructing a replacement follows the same handler-failure rule; no automatic copy of the selected input or cause link is added. The selected holder leaves its old position before it is forwarded. Other entries with the same type or payload remain distinct.

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

**Residual deferred failures.** The [fresh aggregate region and dynamic array of exception holders](#region-and-members-of-unhandleddefererror) retain the residual FIFO. H-509 requires a single unchecked `UnhandledDeferError` containing all exceptions left after the defer list has been drained, exposed through its exceptions field. H-550/H-551 requires every such member to be unchecked; checked failures may exist during the pass but must be handled before this boundary. H-537 fixes its timing: complete the deferred-handler pass first; only then construct the residual aggregate. It is not another input to that completed pass and cannot be recaught by one of its retired defer-catch entries. An empty queue supplies no such failure. Use the same aggregate boundary for any nonempty residual queue, including one element; do not select a different public type based on the count. Preserve FIFO order and complete exception values, including any existing explicit causes.

The exceptions field initially holds every residual owning entry in FIFO order. Successive propagations of one holder do not create two live members. A callable may propagate UnhandledDeferError through the ordinary exception path, and an ordinary try/catch may catch that aggregate. Ordinary type matching sees the container; membership alone is neither a cause link nor implicit member-wise catch dispatch. The aggregate carries no escaping checked deferred-member obligation. Ordinary catch sees this unchecked container and does not dispatch to its members. If a deferred invocation calls another callable that propagates an UnhandledDeferError, that container is one unchecked error at the outer boundary; do not flatten it or rerun its completed inner pass. The callee's separately declared ordinary-body checked failures still need coverage in the outer pass. The [proposed collection operations](#enumerating-and-taking-deferred-failures) preserve independent ownership. Layout and native owner transport remain separately scoped. H-538 specifies the body-failure combination below. No native ABI or permission to allocate while reporting OOM is selected.

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

[D-24’s callable rules](CallableResultInference.md#results-of-proc-func-and-here-bodies) determine which invocation owns a prepared return. H-542–H-549 govern inspection, matching and cancellation. The compiler checks eligibility at the source use before checking the return pattern.

**Lexical placement.** Allow `if returning` only in the code executed by a deferred entry belonging to an invocation with a return destination. This includes the delayed block, its attached catch and finally bodies, and the catch/finally of a bodyless deferred handler. Ordinary groups, conditionals and try/catch/finally nested in that delayed code retain eligibility. A directly supplied single-brace here group retains its lexical supplier’s return destination; invoking or forwarding a group written elsewhere grants it no access. Inspect the owning invocation’s completion when the condition actually executes.

Reject it in an ordinary callable body, a standalone ordinary catch or finally, an eager defer operand at registration, or a module-level deferred action with no callable return. A helper called during the drain can inspect its own result in its own defer. The call grants no access to its caller’s result. An independent callable or `{{ ... }}` inside a deferred entry starts a fresh return context. It can inspect its own result inside its own defer. A rejected use is a compile-time placement error; only an eligible inspection can yield a false match.

```cod
func adjusted() -> u64 {
    defer {
        if returning (val saved:u64) { return saved + 1u64; }
    }
    7u64
}
```

The result is 8. The tail prepares 7, the deferred body observes it and the explicit return replaces it. In `try { return 7u64; } finally { if returning () { ... } }` in the ordinary body, the inspection rejects even though a return may be pending. An ordinary finally can still explicitly replace a completion under its normal rules.

Establish a return only after its operand, conversion and required result preparation succeed. Retain the checked result mode, value or selected target, actual owner/access and necessary type evidence. A later successful return replaces that result and can replace the ordinary body exception with normal return completion; remaining cleanup still runs. This includes a return in the else branch and follows the same explicit-return rule elsewhere in cleanup. Merely inspecting or binding a result does not recover an exception.

| Event | Result state and failure handling |
| --- | --- |
| Natural void exit of the owning invocation | Begin its cleanup in normal completion with no payload. |
| Successful explicit return or prepared func/full-scope here value tail | Install the prepared result; replace any previous result and the callable's ordinary body exception. Preserve unrelated deferred failures. |
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
    # The return was cleared. Complete this recovered proc path explicitly.
    return 0u64;
}
```

These examples assume the named helpers/error types have their indicated contracts. Source-level throws, failing calls, checks and construction all use the same propagation boundaries; source spelling does not determine cancellation. A locally handled helper failure supplies no escaping exception to cancel its caller's return. A deferred invocation's helper failure remains a deferred failure when it escapes that entry. This selects no new nested-aggregate dispatch policy.

**The unbound test.** At each eligible cleanup point, `if returning ()` asks whether this invocation is completing normally. The empty pattern binds nothing and imposes no result type. It matches natural void completion, an explicit empty return and a successfully prepared value or owning result. It is false while an ordinary exception propagates out of this invocation. A caught exception followed by normal completion matches. Reaching a cleanup point or exhausting a valued procedure’s body cannot manufacture a required result.

```cod
proc finish(early:bool, recover:bool) -> void throws WorkFailure {
    defer { if returning () { record(1u64) } else { record(11u64) } };
    defer { if returning () { record(2u64) } else { record(12u64) } };
    if (recover) {
        try { work() } catch (e:const WorkFailure) {}
    } else { work() };
    if (early) return;
}
```

Here `work()` either succeeds or throws the application’s `WorkFailure`. `record` appends its argument to a trace and completes normally.

| Exit of `finish` | Trace from its two deferred entries, in order |
| --- | --- |
| Work succeeds, then natural end | 2, 1 |
| Work succeeds, then explicit empty return | 2, 1 |
| Work fails and the exception propagates out of `finish` | 12, 11 |
| Work fails, its catch handles it, then natural end | 2, 1 |

Each deferred entry sees the state left by earlier cleanup. For example, on an escaping ordinary exception the first entry to run can observe false and explicitly return from the invocation. The next entry observes true. A failure queued by deferred work is settled by the existing FIFO rules and can still prevent ultimate delivery.

For a non-void control, return `7u64` with two deferred entries, one testing `if returning ()` and the other `if returning (val n:u64)`. Both match the same normal completion and only the typed pattern binds 7. With no payload, value/ref/ptr patterns have no result to bind. Binding an explicitly void value rejects. A cleanup return may still supply a required result before final delivery.

**Matching and binding.** Inspect the currently retained result when the condition executes. Try branches in source order; the first matching branch runs. A failed filter leaves the result and failure channels unchanged. Type matching uses the prepared result after its ordinary return conversion, without an additional numeric conversion or assertion exception. For example, a declared u64 result established by return 7u32 has type u64 for matching. Ordinary nominal membership and retained type evidence govern other matches; no target type can be invented from null. An unsupported/unproved matching operation cannot silently pretend that the pattern was false.

| Form | Required match / binding |
| --- | --- |
| `if returning ()` | The owning invocation is completing normally at this point. Matches every result kind, including void, and binds no result. |
| `if returning (in o)` | A borrowed ref/ptr result with existing owner o; no value binding, new owner declaration or implicit borrow of an owning result. |
| `if returning (val rtn)` | A val result, bound as an independent value under ordinary val rules. |
| `if returning (val rtn:T)` | The val mode and type T both match; bind rtn. |
| `if returning (ref rtn:T in o)` | The reference mode, type/access and existing owner match; retain the selected target. |
| `if returning (ptr rtn:T in o)` | The nullable-pointer mode, type/access and existing owner match; preserve nullability. |
| `if returning (rtn:T)` | Use the complete result annotation T's ordinary category/mode and access rules, then bind rtn accordingly. |

**Bounded pattern grammar.** Parentheses contain either nothing, `in owner`, or one ordinary binding declarator with an explicit val/ref/ptr mode or complete type annotation. Thus `(rtn:u64)` and `(val rtn:u64)` have the same scalar value meaning; `(rtn:u64 in o)` follows the ordinary ref default. Existing const/access qualifiers keep their ordinary meaning. A mode-only binding such as `(val rtn)` infers its type from the already checked callable result contract. An omitted ref/ptr owner must likewise be statically unique under existing inference; otherwise require an explicit owner or reject. A bare `(rtn)`, initializer, default, arbitrary predicate, destructuring or ownership-consuming binder is outside this grammar and rejects. Additional conditions use an ordinary nested if. H-549 confirms that if returning only consumes the already inferred or declared result type. Pattern annotations supply no return-type-inference evidence and do not repair a circular or unresolved callable result contract.

A complete type annotation retains its ordinary mode/access/owner meaning; matching never dereferences merely to copy a borrowed scalar into val. Ref and ptr remain distinct modes even when a ptr currently selects a non-null target. Alternate type names resolve by type identity. Owner tests use actual owner identity and existing shared owner evidence, not identifier spelling; they grant neither ownership nor additional access. The returned reference must already satisfy the static unique-owner rule. A typed null can supply its retained declared type and owner; absence alone supplies no narrower concrete target evidence. Owner-only matching does not establish non-nullness. Invalid forms such as val rtn:T in o or an unavailable owner reject at compile time; only well-formed nonmatches fall through. Statically established matches/nonmatches may be folded while preserving branch effects; unknown evidence must not be treated as a nonmatch.

```cod
# In the deferred body, with owner o in scope.
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

Else means that this pattern failed, including a type/mode/owner mismatch; failure of the unrestricted `()` test means the invocation is not currently completing normally. Pattern bindings exist in the matching branch and do not expose the hidden result slot as an assignment target. Changing a val binding or rebinding a ref/ptr binding does not replace the retained result; ordinary permitted target writes remain observable. Matching itself performs no user conversion or ownership transfer. After matching, the binding uses the ordinary rules for its result category. Matching adds no allocation, user conversion, target dereference or runtime assertion; ordinary scalar copying and ref/ptr binding add no special fallible operation. A type/mode/owner nonmatch simply tries the next branch; materialization failure instead follows exception control, with no branch retry. Invalid or unproved access/owner/lifetime remains a compile-time rejection, not a runtime binding check.

Scalar val bindings receive independent bits. A returned func binds according to the ordinary capture-free function-value rules. A ref/ptr binding retains its selected target, actual owner and access. Replacing the retained result leaves that independent binding in place. Further uses must still prove the owner live, and replacement or abandonment releases an owning result exactly once. A result pattern gives no write access to the private result slot.

```cod
proc inspect_result(f:func(u64)->u64) -> func(u64)->u64 {
    defer {
        if returning (saved:func(u64)->u64) {
            saved(0u64);
        }
    }
    return f;
}
```

Here the return prepares a capture-free func value. The matched binding invokes that function. Any failure while binding or executing the deferred body follows the ordinary deferred failure channel. A presence-only check needs no value binding.

**Owning-result limit.** The initial profile accepts `()` for any otherwise valid result, including an owned region. It provides no capture, extraction, consumption or borrowed view of the hidden owning result. An owned binding pattern rejects; ordinary ref/ptr patterns and `in o` do not reinterpret an owning result as a borrow. This conservative limit avoids introducing a second owning handle or a new borrow protocol across result replacement. A future extension must separately establish that protocol; it is not needed to implement this profile.

**Boundaries and acceptance.** The bounded grammar, no-value behavior and reuse of ordinary binding lifetimes above close this source profile's remaining choices under H-548. Acceptance covers lexical defer placement, attached catches/finally, rejection in an ordinary finally or helper, inner-scope reset, equivalence of natural and explicit void completion across proc, func and full-scope here; all admitted patterns and rejected grammar forms; normal replacement and else recovery; successful first/repeated returns; failed first preparation and failed replacement through both failure channels; local recovery preserving the suspended return; escaping cancellation before outer catch/cleanup; typed/mode/owner nonmatches and ordered fallback; typed null without fabricated subtype evidence; independent bits and capture-free func bindings; retained borrowed-target access; binding failure; capture/lifetime rejection; owning inspection without capture; exact result release and unchanged checked obligations. Unrelated queued failures still prevent normal delivery. Aggregate ownership/body-field integration remains separate; [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) distinguishes forbidden same-drain registration from independent explicit scopes/helpers. No new permanent representation, regional graph copy, general ADT value category or pure-callable policy is selected. D-16 conversion, units, validity and future fraction requirements remain unchanged. Full value-path qualification and source implementation retain the registry's separate evidence requirements.


**Standalone finalization.** No separate `defer finally { ... }` form is needed: `defer { ... }` already registers unconditional delayed work. Attached finally has a distinct purpose on a bodyless handler: it executes once after the scan rather than once per matching exception. Keep explicit grouping/full-scope rules unchanged.

### Remaining deferred-queue design scope

The [aggregate region and member array](#region-and-members-of-unhandleddefererror) retain the residual failures. The [publication rule](#when-a-defer-pass-cannot-publish-its-aggregate) covers a resource failure after the drain. Its physical implementation and owner-safe member access remain separately scoped. Checked deferred-member coverage is fixed above: only unchecked residual members may cross the completed pass. An incoming nested aggregate is one opaque unchecked member; there is no flattening or inner-pass replay. H-538 separately retains a pending ordinary body/startup exception in readonly body_exception, without an automatic cause. Its ordinary declared coverage remains distinct from the deferred-member rule; inspection/transport of that field and implementation of aggregate ownership still need their scoped design evidence. Explicit callable return replacement follows D-17-RETURN-STATE; module startup remains aborted. H-505 excludes ordinary body errors from bodyless deferred handlers. [H-FULL-SCOPE-DEFER-20261117](../CHANGELOG.md#h-full-scope-defer-20261117) keeps each draining sequence fixed while explicit inner scopes and permitted helpers retain independent registration and cleanup.

**Acceptance and implementation boundary.** Cover D2/D1/H(error_two)/H(error_one)/finally order; all-matching forwarding through multiple entries; selective [A, B, C] → [B, A′, C′] forwarding; replacement/construction failures at the tail; successive forwarding of one identity and separate simultaneous holders for independently raised errors of the same type; incoming-only visits; residual FIFO order; zero and multiple matching errors; stable unmatched entries; independent bodied execution/local catches; handler rethrow without self-reentry; finally exactly once; failed/skipped registrations; exclusion of ordinary body/startup errors; module contexts and semantic identities; repeated mutation/owner consumption; H-550/H-551's completed-pass checked coverage, accepted covering deferred/local catches, rejected throws-only and wrong-order cases, handler/finally outgoing failures, declared-root conservatism and partial/failed registrations; H-537's aggregate-after-pass and ordinary-catch boundary with unchecked-only members and opaque nested aggregates; all four H-538 body/deferred combinations, readonly association, retained identity/trace/chain and no automatic cause; H-539/H-540/H-541 queued FIFO order through repeated returns, later-result replacement, continued entries, once-only finally and preserved nonmatching failures; D-17-RETURN-STATE cancellation/recovery and filter acceptance; H-FULL-SCOPE-DEFER-20261117 rejection of same-drain registration through ordinary grouping/control flow; accepted independent full-scope/helper cleanup and ordinary pre-cleanup registrations; and each resolved policy above on normal/exceptional exit. The existing single-pending-error/body-skipping implementation does not establish this queue behavior. Handler effects need repeated-invocation flow checks even though each list entry is visited once. Exception propagation/cleanup must retain allocation-free failure delivery: a pending-queue insertion cannot depend on a fresh allocation from an already exhausted allocator. Completed action records can supply queue entries. Each bodied action adds at most one escaping occurrence, a matching handler replaces at most one per input, and its once-only finally can add one. The queue bound is one occurrence per successful registration in the pass. A nested aggregate occupies one occurrence. A separate body failure and the final aggregate root need their own storage. The required UnhandledDeferError collection and proposed rethrow causes need dedicated ownership/allocation analysis; current bounded intrinsic-exception evidence does not establish them. General user exception payload graphs remain separate.

### Explicit callable returns

[Callable result rules](CallableResultInference.md#results-of-proc-func-and-here-bodies) permit void proc fallthrough and require a result on non-void proc completion. Successful value tails in funcs and full-scope here invocations prepare the result through the ordinary return path. A cleanup may explicitly replace it. MissingReturnError and the existing missing-return diagnostics govern missing required results; fallthrough cannot supply a missing non-void result.

A return inside a deferred catch or finally targets the nearest lexical callable return destination and remaining deferred actions run. An explicit full scope has its own destination. Only deferred execution for that destination permits inspection. An explicit return can recover an ordinary body exception, and unrelated deferred failures remain in their FIFO queue. Explicit empty return and natural void completion yield the same unbound observation in the owning invocation’s deferred entries.

H-363 selects the `pure` declaration keyword for the const-function form whose body does not require explicit return: the final body expression supplies its result. Its boundary preserves readonly pre-existing external state, allows private mutation and constrains nested helper calls as described in [D-19](ConstFunctionsAndTailCalls.md). Pure-function implementation and TCO remain deferred by explicit user selection. No expression-bodied pure declaration using `=>` is selected; D-10's trailing callback parameter header uses that token in a distinct position. Ordinary single-brace grouping values remain separate from callable control-flow returns, and every return retains existing result-type, owner/access, storage and cleanup requirements. The executable proc/func fixtures use explicit returns; historical native checkpoint evidence retains its recorded source scope.

### Implementation availability

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Prepared values retain their independent snapshots; borrowed arguments retain the selected target and actual owner. Payload reads occur at invocation. Deferred registration contributes effects only after successful preparation; queue choices, failure paths and completion determine whether initialization is established. These are qualified behavioral obligations, not a mandated solver or runtime representation.

### Callable construction, installation and cleanup

H-380 cancels only affected dependency initialization TLS writes. Operand evaluation and callable construction retain their ordinary effects and exceptional exits; a produced value that is not installed follows existing temporary cleanup. No canceled binding is published or later restored. Procedure-local installations retain normal scoped cleanup. This supplies no permission to erase observable allocation failures or unrelated effects merely to shrink the final image.

H-372–H-376 distinguishes compile-time family registration from runtime callable materialization, TLS installation and invocation. A module definition is not a runtime resource acquisition to release during cleanup. Each execution of a body-bearing effect installs its complete anonymous family and creates one scoped restoration obligation for that activation. Those events retain the existing preparation/publication boundary, restoration on every exit, captured-owner validity and exact release requirements.

Deferred calls retain the selected callable view and prepared operands; a family view must preserve its source interpretation and later actual-type dispatch. Deferred bodies remain delayed, with explicit nested scopes for declarations/effect installation. New abstract evidence and family dispatch must preserve checked NPE, pending normal/return/error completion and explicit causes. Missing/ambiguous method behavior not covered by existing static diagnostics needs its own decision; no new exception category follows from the AOT vision. [D-21](AheadOfTimeExecutionPreparation.md) tracks that architecture work while the bounded defer implementation continues.

### Immediate implementation and acceptance

H-356 authorizes implementing this language work independently of the LLVM binding pilot. The compiler distinguishes ordinary grouping from full scopes and iteration effect boundaries. Complete scope-owned deferred work and explicit returns while preserving the existing exception and regional invariants:

1. Parse deferred calls separately from deferred blocks. Validate the call shape without executing the final invocation or assigning timing significance to redundant parentheses.
2. Reuse typed call preparation for callee evaluation, ordered argument conversions, stored callable values and borrowed operands. Retain prepared state in the owning scope and register it only on successful preparation. Keep block bodies delayed.
3. Integrate deferred execution with normal and exceptional scope exit, pending results, effects and intrinsic owner cleanup. Use existing LLVM call/cleanup machinery and D-18 storage; no new general closure or persistence framework is implied.
4. Verify a compact set of composed cases: initializer-returned cleanup; saved values versus delayed block reads; later callable replacement; argument/conversion order and preparation failure; reverse-order cleanup on normal/exceptional paths; whole-loop versus per-iteration explicit scopes, direct loop defer accumulation and invalid borrowed-owner use. Record allocation and emitted-IR costs for repeated scoped use and accumulated registrations.

Before claiming complete coverage, verify H-370’s owning-argument boundary and resolve any uncovered integration choice involving checked exceptions from cleanup or cleanup failure while another completion is pending. Existing D-17 rules remain authoritative where they already apply. Neither a parser-only change nor execution of a standalone cleanup sample satisfies acceptance.

A [trailing callback](SymbolicDynamicScope.md#trailing-callback-arguments) supplies the first here parameter. Double braces retain local completion and ordinary checked failure propagation. Single braces use the supplying callable’s control targets.

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

A delayed defer block cannot retain a loop index past its lifetime. A library fold step parameter follows its own invocation lifetime. `defer consume(i)` saves the current bits argument through ordinary call preparation; `defer { consume(i); }` is invalid. There is no implicit final-value capture, lexical boxing or lifetime extension. A borrowed argument saves the selected reference rather than a deep copy, so its owner must outlive eventual invocation. Call syntax does not make borrowing loop-control storage or iteration-owned resources valid for an outer deferred action.

### Zero-iteration else

Range `for ... else` executes its else branch only when the body executes zero times, not on exhaustion. The else branch follows the same rules: direct variable declarations are forbidden; its effects restore and fresh effect regions release at branch exit; direct defer registers to the enclosing full scope. A nested explicit `{{ ... }}` permits local declarations and local deferred work. The loop index is unavailable in else. Bound evaluation and initialization joins follow the range loop contract. A library fold supplies its own seed on empty input.

### Acceptance

Composed programs must establish outer/iteration effect values, normal/throw/return restoration, exact releases with captured/hidden owner retention, eager snapshots and global LIFO over dynamic registrations. Validate whole-loop and per-iteration cleanup batches; reject declarations through ordinary body/else groups, delayed loop-control captures and outer deferred borrows of iteration resources; accept nested explicit and callable scopes. Cover zero iterations and bound evaluation. Measure live registrations, peak storage and execution/IR costs as registration count grows. These checks must compose with return/error replacement and remaining cleanup. D-16 syntax/comments, numeric promotion, bits layout, transfer and formatting remain unchanged; fraction implementation stays deferred.

## Expression-oriented control

`try` is an expression, consistent with CoD's existing expression model. Normal completion and handled completion provide its result; an unhandled exception provides no normal result. A `finally` block performs completion work without replacing the result merely through its final expression value. No public bottom type or general continuation API is needed to express this planning requirement.

The exception identities are explicitly selected into the module before use:

```text
try {
  own region packet:Packet <-
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

For currently implemented intrinsic categories, a typed catch matches its exact category. The selected standard-wrapper extension follows the source-order prefix rules above and is not yet implemented. Duplicate categories reject; an optional final catch-all matches the remaining supported categories. No subtype search or clause-order priority among distinct exact categories is needed. The current compiler represents catch parameters with intrinsic bits and uses the common system-exception type for the catch-all. Its representation requires migration to [owner-bound exception objects](#exception-object-owners-and-release). This does not change ordinary parameter borrowing rules. Public field/accessor spelling is a routine API detail within the bounded payload, not authority to expose arbitrary runtime metadata.

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
function transform(own region packet:Packet) -> own region :Packet { ... }

edit(packet); # Borrow; the caller still owns packet.
own region next:Packet <- transform(packet);
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

Rolling records do not own semantic exception fields or the only copy of a cause needed by a still-live exception. A published aggregate retains each member until an explicit take or disposal. A published wrapper retains its cause while live. Diagnostic eviction changes neither obligation. If the aggregate cannot be published, the [reporting failure](#when-a-defer-pass-cannot-publish-its-aggregate) can outlive the discarded failure details. The window can retain a bounded diagnostic summary without holding any member regions. Repeated wrapper construction remains subject to the capacity of the cause owner.

**Acceptance:** basic OOM reporting with ordinary allocation unavailable; first record unchanged after more events than the window can hold; retained older exceptions whose diagnostics become unavailable rather than misidentified; stable cause/aggregate values; simultaneous recording/reading; bounded/truncated trace disclosure; and no implicit region growth, owner resurrection, raw dangling diagnostic view or allocation-dependent basic propagation. The current native cause helper can allocate a vector entry and the current drain releases registration records before invocation; neither implements this reserve or the required aggregate/cause lifetime protocol. No compiler code changes or new runtime validation are supplied here.

## Compiler and runtime implementation

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Native exception handling must preserve checked obligations, explicit causes, scope cleanup, effect restoration and exact release. Intrinsic system-failure information is not general reflection or a mandated user-visible C++ exception ABI. Concrete payload and control-flow representation remain implementation concerns.

### Retained explicit-cause lifetime

H-269 requires explicit cause relationships to survive returning or storing a caught exception beyond the catch. H-510–H-515 establishes those relationships only when constructing a new exception, with readonly cause thereafter. Runtime-managed storage must retain the relationship without mutating a supplied exception/cause. Ordinary cause-bearing construction may itself raise OutOfMemoryError; basic OOM propagation does not allocate and construction failure does not automatically attach either operand to the new OOM. Existing saved values retain their information. Under H-541, a matched deferred input is consumed and an escaping handler construction failure joins the queue tail; the input is not automatically duplicated or attached as a cause. The current compiler's throw-from operation and numeric cause handles are implementation coverage, not acceptance of the new constructor/rethrow/trace contract.

The bounded runtime uses execution-local numeric handles for immutable cause records. Regional fields contain bits and handles rather than host addresses; copying a regional image within the same execution does not require pointer patching. Generic exception storage also preserves its bounded failure-category discriminator. These handles, like source-site IDs, are not a cross-process persistence or delivery format. General user exception graphs and general RTTI remain excluded.

Retained backing storage is released at execution end, including uncaught failure. The initial implementation retains discarded records until that boundary; repeated explicit chaining consumes that storage and may raise OOM. It does not add per-value destructors, graph ownership or eager collection. Public cause traversal reads one nullable immediate link at a time. Inspection and propagation allocate no cause records. Exact reclamation optimizations can preserve this source lifetime contract later.

The native cause table is created only by explicit chaining. Its TLS slot is a trivial pointer, with no lazy destructor registration on basic OOM propagation or cleanup. Failed table creation, first-record allocation or later growth leaves existing histories intact. The native fault harness fails the actual allocation at each of these three boundaries, verifies a fresh unchained OOM, retries successfully and accounts for all backing allocations/releases.

### Allocation-to-publication cleanup

The regional allocator must release its byte block if root construction throws, using the matching allocator and exact capacity. [The construction fixture](../tests/shilos/region_construction_tests.py) checks failure before root entry, both throwing-root overloads and successful retention/release. This allocation-to-publication boundary supplies no existing-region rollback or graph transaction.

## Bounded acceptance and remaining scope

Acceptance requires nested normal/exceptional completion, pre-entry versus callee failure, explicit cause retention, cleanup replacement, deferred queues, failed construction and invalid-borrow rejection. Check exact acquisition/release on supported paths.

Mailbox transport, general user exception graphs, RTTI, cross-process causes and remaining ownership choices retain separate scope. Current implementation limits are maintained in the source capability reference.

## Type routing and handler scope

A try scope supplies its accepted exception type identities and cleanup/handler destinations. At a statically typed throw, WPO can resolve the destination and emit a direct branch. The routing information is distinct from the exception payload; it does not require general RTTI in each exception value. Plain throw propagation from a catch-all must preserve the arriving exception identity in the propagation context; explicit rethrow instead constructs the selected new primary exception. Merely retaining the handler's accepted identity cannot distinguish otherwise identical incoming values.

The user's H-268 suggestion motivates this separation. A runtime handler-registration table is not required by the source semantics or selected merely by the analogy; compiler-local handler scopes and direct LLVM edges are the first implementation route. Explicit-cause lifetime remains a correctness obligation: no dangling catch-local references, silent truncation or allocation-dependent basic OOM delivery.
