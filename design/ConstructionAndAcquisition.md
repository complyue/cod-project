# CoD construction, bits ADTs and region acquisition

Design revision: 53 · Recorded: 2026-11-18 · Status: discussion draft with confirmed requirements; logical result destinations, named return/receiving forms and result-bound owners confirmed; bounded structural-result and receiving source rules selected; remaining conflicts/extensions and full proofs under review; whole-document approval pending; implementation incomplete.

## Design authority

- **Scalar regression cases.** Revision 53 adds [paired target-identity checks](#scalar-expression-checking-and-lowering) to the confirmed [D-22 conditional](DesignAuthorityRegistry.md#d-22-conditional) and [D-10 parentheses](DesignAuthorityRegistry.md#d-10-parens) scopes. The examples exercise the existing C-65/C-72 ownership and value rules under charter §§4.3/6.3 and D-23 reception. [K-SCALAR-CORRECTION-20261118](../CHANGELOG.md#k-scalar-correction-20261118) records the engineering assessment.

- **New DBMR publication.** [D-22-DBMR-PUBLICATION](DesignAuthorityRegistry.md#d-22-dbmr-publication), r52, records the [confirmed visibility rule](../CHANGELOG.md#h-dbmr-new-publication-20261118). A reader opening the final name can rely on complete initialization. The success, failure, retry and competing-creator cases below cover the C-05/C-09/C-54 consistency and ownership goals in charter §§4.3/7.2. Source spelling, existing-file updates and crash recovery remain open.

- **Exception placement.** [H-MAKE-EXCEPTION-20261118](../CHANGELOG.md#h-make-exception-20261118) scopes D-22r49 to the boundary with [D-17 fresh exception construction](ExceptionControl.md#make-exception-expressions). D-22 supplies constructor arguments, field readiness and ordinary payload allocation. C-54/C-65 and charter §§3.3/4.1/4.3 motivate a provable owner. Accept dedicated, complete exception construction with an optional fresh-region capacity. Reject ordinary `make T` for an exception subtype and direct `in o` placement. [H-BRACED-CAUSE-20261118](../CHANGELOG.md#h-braced-cause-20261118) carries the [normal make with a cause](ExceptionControl.md#constructing-a-normal-result-with-a-cause) into D-22r51. The cause supplies the owner before any field runs, including for a nested allocation. D-17 owns the handoff contract. Physical support remains pending.

- **Conditional results.** [H-CONDITIONAL-RESULT-20261118](../CHANGELOG.md#h-conditional-result-20261118) confirms [D-22-CONDITIONAL](DesignAuthorityRegistry.md#d-22-conditional) in r46. A single inferred result type and category makes target identity and scalar reads predictable from the expression. C-65/C-72 and P-348/P-350 connect this source rule to charter §§3.3/4.3/6.3/8. The scalar examples below define acceptance under D-23’s receiving rules. Non-bits carriers, lowering and wider qualification remain open.

- **Parenthesized bits and reference reception:** [H-PARENTHESIZED-VALUES-20261118](../CHANGELOG.md#h-parenthesized-values-20261118)/[K-PARENTHESIZED-VALUES-20261118](../CHANGELOG.md#k-parenthesized-values-20261118) records r44→45 under [D-10-PARENS](DesignAuthorityRegistry.md#d-10-parens)/P-573–P-575 and existing D-23 reception. Expression parentheses supply a value-category result; scalar bits are projected independently even from an operator-result view. Bare readonly reception still preserves access and rejects a writable receiver. Candidate C-39/C-65/C-72/P-348/P-350 and charter §§2/3.3/4.3/6.3/8/12 preserve explicit storage and identity intent; no move, graph transfer or owner extension follows. Acceptance is in the RHS section. Full type-specific carriers/grammar and whole-document implementation/value-path qualification remain under review; D-16 is unchanged.

- **Full-scope dependency continuity:** [K-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#k-full-scope-implementation-20261118) updates only the referenced r92 qualification. This document’s semantic revision, own qualification, acceptance and exclusions are unchanged.

- **Readonly receiving boundary:** [H-READONLY-REF-BINDING-20261117](../CHANGELOG.md#h-readonly-ref-binding-20261117)/[K-READONLY-REF-BINDING-20261117](../CHANGELOG.md#k-readonly-ref-binding-20261117) records r43→44: a readonly returned reference cannot initialize a writable reference through `=`; fresh storage is not an implicit repair for that incompatibility. D-23 retains existing-target binding and explicit bits transfer. C-39/C-65/C-72 and candidate P-348/P-350 preserve charter §§2/3.3/4.3/6.3/8/12: readable operations retain target identity, access and effects. Acceptance covers rejected writable reception, retained readonly target identity, explicit independent bits and unchanged owner checks. H-PARENTHESIZED-VALUES-20261118 selects value-category parentheses while preserving non-bits access/owner/transfer constraints; the whole-document value path and complete native mechanisms remain separately pending. D-16 and D-18’s bounded qualification are unchanged.

- **Full scopes are ordinary callable invocations:** [H-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#h-full-scope-callable-20261117)/[K-FULL-SCOPE-CALLABLE-20261117](../CHANGELOG.md#k-full-scope-callable-20261117), [D-17-FULL-SCOPE](DesignAuthorityRegistry.md#d-17-full-scope)/P-564–P-566, covers r41→42 in the explicit-full-scope interactions below. The user confirms ordinary capture, local return and independent cleanup everywhere; D-17 owns the source acceptance. Reuse C-63/C-65/C-73/C-75/C-84 and charter §§2/3.2–3.3/4.3/6.3/8/12 for one inspectable callable/lifetime model. Preserve actual owners, permissions, declared failures and D-16; no shared cells, lifetime extension or implicit value return. [H-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#h-full-scope-implementation-20261118) separately qualifies D-17’s bounded r92 full-scope profile, including immediate ref/ptr inputs; general escaping borrowed-view and declared-place capture mechanisms remain outside it. D-18's value-input profile retains its independent qualification. D-10 separately records the selected trailing callback surface; complete implementation/value-path qualification remains pending.

- **Bounded source review status:** [K-TRACKER-BOUNDARIES-20261117](../CHANGELOG.md#k-tracker-boundaries-20261117) records r40→41 under H-604's instruction to continue deriving straightforward consequences. The engineering-assessment section now distinguishes selected result/receiving/init/root rules from remaining proof/transport work and optional source extensions. Preserve H-571–H-599/H-601/H-602, C-65/C-72/C-95 and the existing scoped relationships/value rationale under charter §§2/3.3/4.3/6.3/8/12. No new variance, aggregate kind, modifier access, construction policy or qualification is selected; D-16 and D-18's existing qualification boundaries remain unchanged.

- **Bounded init obligations:** [K-INIT-FLOW-OWNER-REVIEW-20261117](../CHANGELOG.md#k-init-flow-owner-review-20261117), [D-22-INIT-PLACES](DesignAuthorityRegistry.md#d-22-init-places)/P-555–P-557, derives r40's scalar/reference-slot nullability, write-before-read and completion consequences from H-445/H-447/H-601/H-602 under H-592's delegation. C-65/C-72/C-95 → charter §§2/3.3/4.3/6.3/8/12 preserves explicit storage responsibility and common flow proof without access escalation or rollback. The bounded table/examples below supply acceptance; aggregate publication, remaining modifier grammar, general proof/view/native and full value-path qualification remain separate. [K-796](../CHANGELOG.md#k-796) continues r41’s reference-operation terminology editorially, with unchanged obligations and qualification. Preserve D-16 and D-18's existing qualification boundaries.

- **Init shorthand and explicit place parameters:** [H-601](../CHANGELOG.md#h-601)/[H-602](../CHANGELOG.md#h-602), [D-22-INIT-PLACES](DesignAuthorityRegistry.md#d-22-init-places)/P-555–P-557 and [K-INIT-PLACE-CORRECTION-20261117](../CHANGELOG.md#k-init-place-correction-20261117) covers r39. Bare init n:T in O retains the init ref spelling's meaning; init ref var explicitly supplies caller reference-slot declared place aliasing. Init adds initialization responsibility, not another reference indirection or a target modifier. This corrects the overly broad earlier attribution of reference-slot roles to H-571; original evidence remains in the changelog. Candidate C-65/C-72/C-95 → charter §§2/3.3/4.3/6.3/8/12 preserves readable operations, actual storage and common initialization flow. Source spelling/direction is selected; derived operation distinctions and acceptance are below. Complete modifier/nullability, field/publication, view/native and value-path proofs remain separate; D-16 and D-18's existing qualification boundaries are unchanged.

- **Region roots and per-root initialization:** [H-595](../CHANGELOG.md#h-595)–[H-599](../CHANGELOG.md#h-599), [D-13-ROOT-ACCESS](DesignAuthorityRegistry.md#d-13-root-access)/P-546–P-548 and [D-22-REGION-INIT](DesignAuthorityRegistry.md#d-22-region-init)/P-549–P-551, covers r37's region-source receiving and named root initializer assessment. Direct root destinations and receiving reuse ordinary field operations/lifetimes; omitted existing roots are untouched. Separate root initializers retain one new owner, in-place preparation and ordinary failure cleanup. Candidate C-74/C-65/C-72/C-63 → charter §§2/3.1–3.3/4.3/6.3/8/12 preserves visible storage intent without whole-root conversion, hidden retention or rollback. [K-NAMED-ROOTS-20261117](../CHANGELOG.md#k-named-roots-20261117) records confirmed source choices versus derived consequences and the construction-local owner binder and init-helper integration. Acceptance is below; preserve D-16, D-17, D-23 and D-18's existing qualification boundaries. Complete initialization/native/value-path proofs remain separate.

- **Result-bound owners:** [H-594](../CHANGELOG.md#h-594), [D-22-RESULT-OWNERS](DesignAuthorityRegistry.md#d-22-result-owners)/P-543–P-545, confirms r36's explicit result-owner dependency and ordinary liveness criterion, including cleanup of an unreceived owned region. There is no special paired-field reception requirement. Candidate C-63/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 preserves explicit lifetime responsibility, compositional results and exact release without implicit retention. The section below owns acceptance, temporary-read and projection consequences; [K-RESULT-OWNERS-20261117](../CHANGELOG.md#k-result-owners-20261117) separates human confirmation from derived integration. Preserve D-17 successful ownership delivery, D-23 value versus reference reception, D-28 actual owners, ordinary ADT restrictions and D-16. Complete checker/native protocols, whole mechanism/value-path qualification and D-18's separate value-input review remain separate.

- **Derived receiving and result rules:** [H-592](../CHANGELOG.md#h-592) authorizes resolving straightforward tracker items without repeated confirmation. [K-RECEIVING-CORE-20261117](../CHANGELOG.md#k-receiving-core-20261117), D-22r34→35, records agent-derived grammar normalization, prior-environment lookup, ordinary explicit temporary scope, componentwise independent inference and whole-result preparation under H-571–H-575/H-591. D-22-ANON-DATA/RESULT-DESTINATION/RECEIVE-SCOPE/RESULT-COMPAT and P-509–P-517/P-537–P-539 retain their candidate value paths: readable named composition, explicit storage/lifetime responsibility and preserved effects under charter §§2/3.2–3.3/4.3/6.3/8/12. Acceptance is in the corresponding sections. This resolves bounded source consequences, not full mechanism/value-path qualification, complete owning-output proof/transport or new field-inspection syntax. H-594 separately selects the result-bound-owner source rule. D-16 and the separate D-18 value-input review remain; D-23/D-29 receive only the H-593 overlap dependency alignment.

- **Structural result compatibility:** [H-591](../CHANGELOG.md#h-591), [D-22-RESULT-COMPAT](DesignAuthorityRegistry.md#d-22-result-compat)/P-537–P-539, accepts automatic projection of extra callable result fields and delegates straightforward consequences of the existing structural-result/partial-reception model. R34 distinguishes order-independent schema identity from directional result compatibility, preserving complete component contracts, producer effects/checked failures, omitted cleanup and owner dependencies. Candidate C-10/C-63/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 supports readable typed composition without a group object or hidden retention. Acceptance and exclusions are in the compatibility section below; [K-RESULT-PROJECTION-20261117](../CHANGELOG.md#k-result-projection-20261117) records the source decision and derived details separately from pending mechanism/value-path/ABI qualification. D-16 and the separate D-18 value-input review remain.

- **Receiving-overlap boundary:** [H-576](../CHANGELOG.md#h-576)/[H-593](../CHANGELOG.md#h-593), [D-22-RECEIVE-OVERLAP](DesignAuthorityRegistry.md#d-22-receive-overlap)/P-518–P-520, selects author-responsibility UB for cross-entry write/borrowed-read overlap and write/write overlap. Use the same warning/opt-in-error diagnostic direction; costly analysis may be deferred. The rationale serves C-65/C-72/C-73 and charter §§3.3/4.3/6.3/8/12 through explicit author reasoning, straightforward sequential workarounds and compiler scheduling freedom. Preserve defined-program effects, distinct-slot reference sharing, ordinary sequential writes, owner/access/init and D-16. Scope/examples/acceptance below and [K-RECEIVING-CORE-20261117](../CHANGELOG.md#k-receiving-core-20261117) bound this change; no analysis algorithm, full receiving mechanism or ABI is qualified.

- **Receiving-specification assessment:** [H-574](../CHANGELOG.md#h-574)/[H-575](../CHANGELOG.md#h-575), [D-22-RECEIVE-SCOPE](DesignAuthorityRegistry.md#d-22-receive-scope)/P-515–P-517, covers r37’s normalized source form: `{ val count <-; ref selected = node; existing <- field; } := producer();`. H-575 requires a declarative receiving specification and excludes arbitrary expressions, rather than an executable receiving block. H-574 supplies explicit item operations and same-name selector omission; H-573 supplies existing-variable/renaming and temporary-scope goals. Candidate C-65/C-72/C-73 → charter §§3.3/4.3/6.3/8/12 preserves readable routing intent and ordinary place/result/lifetime responsibilities. Preserve D-23’s operations/pre-RHS destination selection, H-571 private completion, D-16 and D-18's existing qualification boundaries. H-576 separately settles the borrowed-read conflict boundary; the bounded grammar/scope below is selected under H-592 delegation; H-593 supplies the write/write UB boundary; diagnostic precision and full proof/ABI qualification remain pending; acceptance below and [K-RECEIVING-SPEC-20261117](../CHANGELOG.md#k-receiving-spec-20261117) bound this assessment.

- **Return-only structural result assessment:** [H-572](../CHANGELOG.md#h-572), D-22-ANON-DATA/P-509–P-511, covers r29’s preferred proposal: `-> { ... }` describes a named product of return contracts, received as independent lexical variables without a source-visible group object. Product-only/return-only use and independent component owners are the human’s proposed scope, not a full type/ABI/proof approval. The candidate C-10/C-65/C-72/C-73 rationale serves charter §§2/3.2–3.3/4.3/6.3/8/12 through readable composition using existing result modes and lifetime rules. Preserve H-571 syntax/completion, ordinary nominal ADT same-owner fields, D-16 and the separate D-18 value-input review. Supported uses, exclusions, acceptance and unresolved interfaces are below; [K-STRUCTURAL-RESULTS-20261117](../CHANGELOG.md#k-structural-results-20261117) records this assessment.

- **Accepted output and receiving source contract:** [H-571](../CHANGELOG.md#h-571) confirms the recommendations in [D-22-RESULT-DESTINATION](DesignAuthorityRegistry.md#d-22-result-destination)/P-512–P-514 and the named-return/receiving scope of D-22-ANON-DATA/P-509–P-511 at r28: logical caller-provided results, pending completion before delivery, general caller-provided initialization, clarified below by H-601/H-602, `return { expr -> field, ... };`, and new-declaration groups `{ ... } := expression;` with ordinary per-field initialization. The C-54/C-63/C-65/C-72/C-73 rationale serves charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit storage responsibility, preserved effects/cleanup and readable named composition. The human confirms this bounded source recommendation, not full aggregate/init proof mechanisms, physical ABI, foreign handles, every modifier combination or the separate D-18 value-input review. Preserve D-23 modes, D-17 completion, actual owners, no lifetime extension, bits-only payload transfer, ordinary same-owner ADT restrictions, and D-16. Acceptance is below; [K-OUTPUT-ACCEPT-20261117](../CHANGELOG.md#k-output-accept-20261117) records integration and unaffected qualifications.

- **Structural-product UX review:** [H-569](../CHANGELOG.md#h-569), [D-22-ANON-DATA](DesignAuthorityRegistry.md#d-22-anon-data)/P-509–P-511, covers r25's partial named reception requirement and structural-anonymous-data candidate. Preserve named nominal identity, once-only evaluation, per-field operations, required cleanup, owner/access and H-563/H-567/H-568. D-22-RESULT-COMPAT settles schema identity and directional callable-result projection; broader variance and full mechanism/value-path qualification remain pending; H-571 selects named return and receiving-group forms, while partial receiving does not itself require general structural subtyping.

- **Callable placement and transfer boundaries:** [H-563](../CHANGELOG.md#h-563)/[H-567–H-568](../CHANGELOG.md#h-567), [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/P-503–P-505 and [D-22-TRANSFER](DesignAuthorityRegistry.md#d-22-transfer)/P-506–P-508 cover D-22r24's placement/operation terminology. Callable-view values have lexical-scope storage, never regional storage; only pure bits admit whole-payload transfer. CoD has no C++ object copying/moving model. C-54/C-65/C-72/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve regional relocatability and explicit identity/lifetime responsibility. D-18/D-22 own acceptance; retain dedicated callable construction/capture, graph transfer, actual owners and D-16. [K-LEXICAL-PRODUCTS-20261117](../CHANGELOG.md#k-lexical-products-20261117) aligns dependencies. D-22’s return-only structural result proposal is separate from ordinary ADT field eligibility; no new aggregate implementation or full proof/physical/value-path qualification follows.

- **Typed reservation and completed construction:** [H-562](../CHANGELOG.md#h-562)/[K-TYPED-RESERVATION-20261117](../CHANGELOG.md#k-typed-reservation-20261117), [D-22-RESERVATION](DesignAuthorityRegistry.md#d-22-reservation)/P-500–P-502 covers D-22r23's initialization boundary. Bare `make T [in o]` reserves typed storage; an attached initializer block requests completed construction. C-65/C-72/C-95 and charter §§3.3/4.3/6.3/8 preserve explicit allocation and flow-checked readability. D-22 owns examples/acceptance; preserve established identity versus payload readiness, actual owners, staged sound proofs, no rollback and D-16. The source distinction is confirmed; aggregate algorithms/native support and complete value-path qualification remain separate. Other qualified contracts continue.

- **Owner-parameter symbol integration:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499, covers D-22r22's reference-symbol/owner dependency wording. The source form and staged sound acceptance are confirmed: installation selects one actual lexical or ordinary regional owner from the RHS; an `in @arena` declaration retains its additional selector constraint. C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve explicit dynamic context and lifetime responsibility. D-10 owns examples and acceptance. Preserve saved/captured owner validity, access/null/init, module symbol identity, no lifetime extension and D-16; full proof/physical/value-path qualification and source implementation remain separate. Other qualified scopes continue.

- **Capture/lifetime alignment:** [H-553–H-557](../CHANGELOG.md#h-553)/[K-CAPTURE-LIFETIMES-20261117](../CHANGELOG.md#k-capture-lifetimes-20261117), [D-18-VALUE-CAPTURE](DesignAuthorityRegistry.md#d-18-value-capture)/P-485–P-490 and [D-12-LIFETIME](DesignAuthorityRegistry.md#d-12-lifetime)/P-491–P-493 cover this revision's capture/owner dependency wording. Preserve independent value locals, selected reference targets, explicit var places, unique actual owners and statically bounded borrows; reject implicit slot sharing, closure-cell promotion and lifetime extension. C-36/C-54/C-65/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve visible storage/lifetime responsibility. D-18 owns acceptance and H-327/H-329 alias continuity. D-16, unrelated qualified scopes and existing symbol-declaration restrictions continue. New proof/physical/value-path qualification and source support remain separate; no allocator or graph-copy mechanism follows.

- **Nominal ADT placement:** [H-478](../CHANGELOG.md#h-478)/[K-SINGLE-ADT-20261115](../CHANGELOG.md#k-single-adt-20261115) covers r19→20's alignment with [D-28-ADT](DesignAuthorityRegistry.md#d-28-adt), P-416–P-418. One CoD ADT retains nominal identity across lexical/regional placement; actual owners provide layout/access evidence. The candidate C-10/C-39/C-65/C-91 path serves charter §§3.1/4.3/6.3/7.1/8 through shared business types and checked storage. Preserve same-owner edges, bits classification, initialization, lifetimes and explicit graph transfer; no C++ regional-type stack allocation, capture/TLS extension, graph copy or runtime support is implied. Acceptance and unresolved physical protocols belong to D-28's composed cases. This scoped human decision and direct integration do not qualify the whole document or settle H-146's encoding mechanism.

- **Group-scope continuity:** [K-KNOT-COST-20261115](../CHANGELOG.md#k-knot-cost-20261115) covers r16→17 under H-317/P-284, H-428 and H-444. C-65/C-72 and candidate P-348/P-350 preserve ordered, checked construction and visible effects under charter §§3.3/6.3/8; no value path is newly qualified. Ordinary groups gain no forward data bindings or provisional ref escape; required-ref cycles remain deferred after D-28's cost assessment. Future pure knot-tying belongs to D-19-KNOT. Acceptance and whole-document exclusions remain unchanged; no implementation claim follows. H-447/[K-INIT-CONFIRM-20261115](../CHANGELOG.md#k-init-confirm-20261115) continues r17→18 with the confirmed staged-acceptance plan; it introduces no forward names, rollback or automatic incomplete-object publication.

- **Registry / coverage:** [D-22](DesignAuthorityRegistry.md#d-22), revision46. H-383–H-386 confirms the scoped construction requirements; H-425–H-427 develops RHS normalization and intentional lifetime separation without move semantics. H-428 confirms allocation effects without language checkpoints/rewind. [H-429](../CHANGELOG.md#h-429) establishes distinct regional/lexical storage directions; [D-28](RegionalAndLexicalStorageRefactor.md) owns that refactoring and the proposed C++ external_ptr naming. [H-430](../CHANGELOG.md#h-430) rules out automatic/in-place language growth in favor of consuming replacement; [H-431](../CHANGELOG.md#h-431) selects ordinary owned-parameter failure cleanup. H-432 proposes lossless resize(r, new_cap) for growth and shrinkage. Detailed RHS, capacity/error mapping and resize backend protocols remain under review; P-348–P-350 and whole-document qualification remain pending-human. H-433 sets the lexical pointer-bearing goal; H-478/D-28-ADT selects one nominal ADT across both domains. Bits-only lexical construction remains an implementation fallback, with physical layout/encoding and complete coverage still outstanding. H-435 selects dynamic lexical capacity and owner-base-relative lexical storage independently of the remaining ADT/ABI work. H-436 separates lexical slot location from reference contents and permits register/stack optimization. [K-OWNER-ALLOCATION-20261114](../CHANGELOG.md#k-owner-allocation-20261114) records H-438 allocation through non-const owners and optional const-owner restriction with writable targets, using H-437 shared owner information. [K-REFERENCE-IDENTITY-20261114](../CHANGELOG.md#k-reference-identity-20261114) integrates H-439/H-440 owner-plus-locator identity and is null, preserving established access, result and lifetime behavior. Current bounded source/native coverage is recorded in the [source capability reference](../tests/cod/language/README.md). Placement, allocator transport and whole-document qualification remain separate.
- **Candidate value path:** C-39/C-65/C-72 → C-01/C-06 (P-348) makes storage, access and lifetime intent explicit and checkable in readable source, including distinct lexical/regional construction and explicit initialization effects; C-09/C-54 → C-05 (P-349) separates persistent data from acquisition responsibility and preserves inspectable preparation effects. D-22 organizes these responsibilities through P-350. Charter §§2–5/§7–8 governs those boundaries; §§6/12 requires practical native execution and measured costs. These scoped paths do not qualify the wider ownership or DBMR designs.
- **Dependencies:** P-351: D-01 physical allocation/relative references; D-14 owners, capabilities, fields and qualifiers; D-23 scalar value/view reception; D-17 initialization, failure and cleanup; D-10/D-15/D-18/D-21 dynamic context, values and native dispatch; D-16 bits validity, layout and numeric compatibility. Dependency edges grant no authority.
- **Invariants / non-goals:** no ordinary reference ownership, per-object cleanup, owner-free heap graphs, cross-owner stored edge, implicit graph copy/move, lifetime extension or native callable environment in a regional field. Lexical scopes are not backed by implicit regions; D-28 records H-435 owner-base lexical views and H-478 same-nominal lexical ADTs; physical carrier/layout protocols remain separate. Regional C++ types remain region-only and noncopyable. Actual-owner unions, abstract-result RTTI, fraction execution, pure/TCO and the compiler pilot remain separate.
- **Human baseline:** current project user in [the construction side chat](codex://threads/01a1d051-f608-7c13-b98a-684f065884d7), recorded 2026-11-12 America/Los_Angeles; exact message timestamps/personal name unavailable. H-383 preserves the ownership/allocation corrections; H-384 confirms R1, explicit R2 owner syntax/mismatch rejection and autonomous R3/R4 comparison with final selection reserved; H-385 confirms result-only constness. The [active main chat](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6) subsequently accepts contextual inference and prefers one coordinated migration without compatibility staging (H-386). DBMR protocols, aggregate ABI and final whole-file approval remain outstanding.
- **Acceptance:** the matrix below requires composed positive/rejection cases, exact acquisition/release accounting, relocation and measured compilation/runtime costs. No implementation or new performance result is claimed by this revision.

- **Current implementation alignment:** [K-704](../CHANGELOG.md#k-704) continues r18→19's capability pointer while retaining H-447's staged planning scope. Bounded lexical identity and scalar/caller-slot cleanup evidence does not establish aggregate reservation, publication, mandatory-write contracts or actual-owner allocation transport.

## Confirmed requirements

Only designated intrinsic resource-reference kinds distinguish owned and borrowed; the current kind is the region reference. Ordinary ref/ptr is borrowed, with target provenance, lifetime and access, without independent release or transfer authority. Storage ownership, reference-slot storage and the holder of region cleanup responsibility are distinct. User ADTs cannot contain intrinsic ownership carriers, including through wrappers.

Every ADT instance/storage view has an actual owner. Direct/val bits fields occupy that owner's storage; ref and non-null ptr fields target the same actual owner. Nominal type identity does not identify an owner instance. A bits copy across owners copies neither owner identity nor cleanup responsibility. Ordinary lexical borrows of outer storage do not become stored graph edges.

R1 admits a concrete ADT as bits when its finite fixed representation recursively consists only of bits fields/alternatives/tags, without stored references, ownership carriers, managed environments or implicit cleanup. This includes finite products, sums and qualifying generic instantiations. A one-word reference is still non-bits. An unresolved/abstract type requires sufficient bits evidence; R1 does not choose the source spelling for that constraint, a sum encoding, ABI, packing or validity of arbitrary bit patterns.

Bits ADTs may occupy lexical or regional storage, including a region root. Fresh lexical bits construction belongs to the current full scope; an ordinary grouping block adds no owner. H-478 selects the same nominal pointer-bearing ADT in either domain, with same-owner fields and automatic addressing for its actual placement. Adding a reference field still invalidates dependent bits uses; lexical eligibility does not make the object an independent bits value. D-28 supplies the selected contract and H-438 owner-qualified construction, while exact layout/allocation transport and complete native coverage remain outstanding. Unsupported lexical layouts retain a diagnosed bits-only implementation fallback. No builtin graph-copy extension follows. The current compiler's product-only/scalar-bits subset is an implementation limit, not the language definition.

Ordinary data construction uses `make T [in o]`. Exception construction requires an implicit handling owner under [D-17](ExceptionControl.md#exception-object-owners-and-release); intrinsic resource acquisition uses `make region`. Explicit `in o` selects the actual owner and a receiving-variable owner mismatch is a compile error. Equal root types or lifetimes do not establish owner equality. Reference binding preserves provenance and access. Independent bits materialization and explicit bits transfer remain distinct from readonly reference reception; scalar grouping projects independent bits under D-10-PARENS; scalar conditional reception follows the common result contract below. Under H-438, `where {owner o}` supplies allocation access when supported by at least one writable-target input; `where {const owner o}` forbids allocation and is required for all-readonly inputs. A callable may deliberately select const owner with writable inputs. Generic owner construction does not expose a regional root-policy reference or grant resource ownership.

`make const region` restricts the published result view. Explicit preparation writes, including selected create/initialize/expand/eligible-trim operations, may precede that result subject to permissions and the selected protocol. Constness does not freeze the file, imply a snapshot or remove the owning acquisition's release obligation. Final DBMR grammar, sharing/defaults and operation/failure protocols remain unselected.

Callable payloads retain D-18's complete intrinsic value category and scope-owned backing; they are not bits ADTs or ordinary independently owned references. Local ADT placement alone authorizes no capture or lifetime extension. D-18 admits value snapshots and explicit non-escaping owner-qualified references under H-553–H-556, without promoting lexical cells or copying regional graphs. H-370's deferred owning-argument transfer at invocation and H-365/H-371's scope restrictions remain unchanged.

## Storage eligibility and payload transfer

H-563 prohibits regional storage of callable-view values. H-567/H-568 separately confines whole-payload transfer to pure bits and rejects a C++ object copy/move model. These are independent checks: an ordinary reference-bearing graph may be regional and zero-cost relocatable without being transferable as a whole value; a callable view may have an intrinsic lexical construction contract without being bits. Initialization and bare reservation do not override either check.

| Operation | Contract retained |
| --- | --- |
| Pure bits payload transfer | Require initialized valid bits and the ordinary destination/type/access rules. It copies no object identity, owner or cleanup responsibility. |
| Reference binding | Retain a selected target/actual owner; do not duplicate the referent. |
| Callable-view construction/capture | Use D-18's dedicated complete-environment and borrow rules. This supplies no generic aggregate-copy operation. |
| Whole-region ownership transfer | Consume/pass release responsibility under the explicit owned contract; regional objects remain the same graph. |
| Graph transfer | Explicit destination construction and graph mapping under the selected library contract; no implicit copy/move or automatically chosen policy. |

A regional field, array or generic compound may not contain callable-view values. Proving a field's bytes relocatable is insufficient. The return-only structural result proposal below introduces no stored ADT field and grants no new field permission; ordinary callable-bearing ADT eligibility remains a separate unresolved extension. D-16 bits validity and existing no-owning-resource-in-user-ADT constraints continue.

## Structural result types — return-only proposal

H-572 proposes a **structural result type**, restricted to callable results and a product shape. The result-position spelling omits the data keyword: `-> { ... }`. This role is distinct from ordinary nominal ADTs. H-571’s named return and receiving forms remain selected. The preferred proposal describes named output components; it does not allocate an ADT instance containing those components or create a first-class group value in caller code. It is a source/typing proposal, not current compiler support or a selected native ABI.

```cod
# Proposed result-type use; Node and each owner follow ordinary contracts.
proc inspect(left:Node in A, right:Node in B)
    -> { count:u64; ref first:Node in A; ref second:Node in B; }
    where {owner A, owner B} {
    return { 2u64 -> count, left -> first, right -> second };
}
# Proposed receiving items; total, first and second already exist.
# Their declared contracts admit these outputs, whose actual owners are a and b.
{ total <- count; first =; second =; } := inspect(left, right);
```

The proposed receiving items update three existing lexical variables without introducing field-name bindings. Each ref/ptr retains its own statically proved target owner; the group has no target owner, storage identity, address, referent or user-visible cleanup resource. The actual receiver slots have their ordinary lexical owners, independently of their referents. Therefore different result components can name different owners without a stored cross-owner ADT edge or an owner union for any individual reference. Borrowed callee-local storage still cannot escape, and caller-provided result storage cannot repair such an escape.

### Product shape and permitted uses

The bounded source boundary is a fixed finite set of uniquely named output contracts. Its semantics include label-to-contract matching, each component’s value/reference/access/owner contract, ordered production and one callable completion. “No structural group” means no first-class group object; eliminating the signature or completion semantics would make return checking and failure behavior undefined.

| Use | Source treatment |
| --- | --- |
| Callable result annotation `-> { ... }` | Admitted within the proposal. Components use independently valid ordinary result contracts. |
| Result position inside a func/proc type or typeof-origin contract | Retain the structural output metadata; this does not turn it into a callable input value. |
| `return { expr -> label, ... };` | Map expressions to the declared/inferred outputs, once in source order. No local product object is implicitly moved. |
| `{ receiving specification } := call();` | H-574/H-575’s explicit operations map fields to new or existing destinations, with optional same-name selectors and no arbitrary expressions. |
| `{{ receiving specification }} := call();` | Temporary form equivalent to an ordinary full scope around the receiving operation. New bindings end with that invocation; captured-value writes stay local, while ordinary borrowed target/place writes remain observable. The spec admits no executable statements. |
| `return other_call();` with a compatible result contract | Recommended direct forwarding of the outputs and their obligations, retaining the outer invocation’s cleanup/replacement boundary. |
| Discard an entire call result | Run the call and required component cleanup; discarding creates no group value. |
| Variable, ordinary value parameter, ADT field, array/container element or region allocation of this structural type | Excluded from the proposed return-only scope, even if every component is bits. |
| Whole-result `.field` access, address-taking, declared place aliasing, capture, equality or payload transfer | No group value/place exists for these operations. Receive selected outputs into ordinary variables first. |
| Structural sum variants or branch-dependent output labels | Excluded. The result schema is one product; branches must satisfy one established schema. |
| A structural result type nested as a component | Not included initially; it would introduce another grouping contract. A callable component may itself have a structural result signature. |

The return-only restriction concerns program values; compiler/type metadata may still describe and inspect the result schema without constructing or executing a group. The product-only restriction concerns the output schema. It does not erase an otherwise valid nominal sum type used under an admitted individual result contract. Schema identity and directional result compatibility follow the rules below; effects retain source order. No new numeric conversion, mutable-reference covariance, row polymorphism or structural equivalence for named ADTs follows.

### Schema identity and callable result projection

H-591 accepts projection as an ordinary consequence of structural results and partial reception. Schema identity uses the same unique label set and the same resolved component contracts, independent of declaration order. Thus `{ count:u64; ready:bool; }` and `{ ready:bool; count:u64; }` describe the same result schema. Written producer expressions still evaluate once in source order; canonicalizing type metadata never reorders effects.

Compatibility is directional. A callable producing schema S may satisfy an expected result schema R when every label required by R occurs in S and its complete result contract is compatible under the existing rules. Extra S outputs are discarded using ordinary partial-result cleanup. S and R need not be identical. Missing labels or incompatible type/mode/access/owner/initialization/ownership contracts reject; the projection itself introduces no conversion or permission.

```cod
func inspect() -> { count:u64; ready:bool; } {
    return { 7u64 -> count, true -> ready };
}

proc read_count(f:func() -> { count:u64; }) -> u64 {
    { val count <-; } := f();
    return count;
}

read_count(inspect);  # Accept; returns 7u64.
```

The same result compatibility applies to ordinary direct forwarding with an independently established destination contract, such as a callable declared to return `{ count:u64; }` executing `return inspect();`. Preserve the forwarding callable's own pending completion and cleanup before delivery. Construction by `return { ... };` still supplies exactly the fields of its established schema: an unexpected label in that explicit construction is an error, not an instruction to silently drop a possibly misspelled field.

Projection selects results, not execution: it does not skip the producer, extra-field expressions, their effects, declared failures or required cleanup. Discarded callable/owning components receive ordinary complete-value cleanup, with failures and partial-preparation/abandonment handled under the existing completion rules. Every retained borrower requires the ordinary proof that its actual owner remains live for the required lifetime; omission is not a separate rejection criterion. Apply the result-bound-owner rule below and account for ordinary release of omitted owning outputs. Projection cannot extend a lifetime, retain an omitted owner implicitly or place a callable in a region. Optimizations must preserve those observable obligations.

The callable must still satisfy the expected input, method-family, effect, throws and owner contracts. Extra-output production cannot erase a checked failure just because its field is omitted. Under `typeof origin`, the origin's public argument names/defaults and complete required method family remain in force; result projection does not select a different overload or hide a missing required method. No new source-level wrapper declaration, group value or closure storage is required. This states source compatibility, not a selected adapter representation or ABI.

An inferred producer schema remains established independently of its receivers. Do not invent a common subset for branches with different output labels, use output projection to select an overload, identify a labelled singleton with an unnamed scalar, or derive general input-record subtyping or generator protocol variance. Those are separate contracts. Named ADTs remain nominal.

**Acceptance / proof scope:** accept reordered equivalent schemas, extra bits/callable outputs under their existing cleanup contracts, compatible callable arguments and declared direct forwarding. Reject missing labels, incompatible retained contracts, narrower throws/effect contracts and unproved owner lifetimes after omitted-output cleanup. Observe once-only source-ordered production, cleanup of omitted outputs, forwarding cleanup before publication and exact release on cancellation. Complete multi-output checker/native/ownership proofs remain required before implementation; this source rule is not evidence of current support.

### Result-bound owners and ordinary liveness

H-594 allows a structural result contract to name an owned-region output as the owner of another result component:

```cod
proc build() -> {
    arena:owned region:Graph;
    ref root:Node in arena;
};

{ owned region storage:Graph <- arena; ref start = root; } := build();
```

Here `arena` denotes the actual region returned by this invocation. It does not denote the lexical storage of the owning handle, the nominal root type Graph or a union of possible owners. The producer must prove that root's selected target belongs to that region. An admissible existing owned region may be returned; the declaration does not require fresh allocation. Receiver renaming preserves the same actual owner dependency, so start depends on the region held by storage, not on the spelling arena.

This is a relationship in the result contract. Resolve result labels and their owner dependencies independently of declaration order; runtime result expressions still evaluate once in source order and obey ordinary consumption checks. Output labels do not become variables visible in the producer body. Nor does this dependency permit arbitrary expressions or references to newly introduced receiver bindings inside a receiving specification. Existing receiver owner constraints must match the actual result owner; a matching nominal region type is insufficient. Every ref/ptr still has one statically provable owner.

There is **no requirement to receive a particular pair of fields**. For each retained reference or other borrowed dependency, prove under the ordinary rules that its actual owner remains live throughout the required lifetime. An unreceived owned-region output is released at the ordinary omitted-output cleanup boundary, exactly once. No hidden owning handle, reference counting, owner duplication or lifetime promotion keeps it alive. Consequently, retaining root beyond that release rejects because the lifetime proof fails. Keeping both outputs does not waive subsequent release, consumption, resize, access or initialization checks.

Independent value reception illustrates why the criterion is lifetime rather than field pairing:

```cod
proc make_number() -> {
    arena:owned region:Number;
    ref value:u64 in arena;
};

{ val n <- value; } := make_number();
```

An admitted reception reads value while its temporary result owner is still live; n then holds independent bits. Ordinary omitted-output cleanup releases arena after the required read and before leaving the receiving operation. This creates no lasting owner dependency for n. The temporary lifetime and access/readiness proof must actually hold; neither the receiving-overlap UB rule nor a physical optimization can excuse an invalid owner. Retaining a reference instead of transferring its bits requires the longer reference lifetime proof. Keeping only arena or discarding all outputs also follows ordinary component cleanup, without a special owning-result restriction.

Projection and forwarding preserve this dependency evidence. A smaller exposed schema cannot simply erase `in arena` while continuing to return the borrowed root, or secretly retain an omitted owner. Check the exposed contract and its actual owner lifetime after its own omitted-output cleanup; compatible declared owner evidence is required. Receiving bits before cleanup and forwarding a borrowed result across that cleanup are different operations. A forwarding callable still completes its own cleanup before delivering ownership. All result components remain one pending completion under D-17: preparation failure, replacement, cancellation and callee-cleanup failure release abandoned owned components exactly once without publishing a partial successful result.

**Acceptance / proof scope:** cover renamed/reordered receivers, both outputs retained with valid subsequent borrows, owner-only and all-output discard, independent bits reception before temporary-owner release, mismatched actual owners, unproved retained-reference lifetimes, projection that would erase required owner evidence, duplicate consumption and failure/return replacement. Preserve ordinary result inference, effect order, no rollback, regional relocatability, same-owner ADT fields and D-16 bits validity. This selects the source dependency/liveness contract, not a physical descriptor, per-reference owner payload, complete multi-output proof, generator-local lending protocol or implementation support.

### Independent inference and complete result preparation

Reuse D-24's callable-wide inference policy. An explicit result annotation or independently established declaration/expected callable contract provides the schema. Otherwise independent return evidence must establish one unique label set; infer each component using its ordinary permitted result joins, preserving mode, access, initialization, actual owner and callable/ownership obligations. Label order does not matter. An independent `return { 0u64 -> count };` can anchor a recursive result; only forwarding an unresolved recursive call cannot. Every recursive/dependent return is then checked against the established contract.

Different inferred branch label sets do not silently become their intersection or union. Declare the intended result schema and use compatible forwarding/projection or explicit field construction. No receiver type, receiving subset, new local name or if-returning pattern supplies missing producer evidence. A labelled singleton remains a schema; no unnamed scalar conversion follows. Nested result schemas, structural sums and group-valued inputs/storage remain outside the bounded result-only scope; an independently valid nominal component or callable component retains its own contract.

All components form one pending return completion. Prepare written expressions once in source order; snapshot val bits and retain selected borrowed targets with their actual owners. If preparation fails, release only prepared obligations exactly once under the ordinary failure rules; no completed result or new receiving bindings have been published. Prior permitted effects and allocations are not rolled back. A later return prepares its whole replacement before replacing the pending result; failed replacement follows D-17's ordinary/deferred failure distinction rather than inventing a partially replaced field set.

Required callee cleanup, return replacement/cancellation and omitted-output cleanup retain their existing boundaries. A forwarding callable retains its own cleanup before public result delivery. Dropped or abandoned components receive category-specific cleanup, including complete callables and owned resources; caller-provided hidden storage is not a public receiving write. Optimize storage/transport only while preserving those observations and actual-owner validity through growth. Result-bound owners follow the source contract above. Complete multi-output checking/transport still requires proof; an unknown owner dependency rejects rather than granting implicit retention.

For return inspection, existing `if returning ()` can test a pending structural return without binding it. Ordinary single-result patterns do not implicitly select a field, and no whole-group binding or field-level pattern is introduced by receiving syntax. A future labelled inspection form remains a separate extension; it is not required for the bounded receiving/forwarding contract.

### Field mapping without stored aggregate fields

Here “field” means a label in a result contract. Receiving items map labels to ordinary new declarations or existing destinations; it does not introduce a named aggregate that owns them. The producer is evaluated once, and selected receivers use the ordinary bits, reference-binding or dedicated callable operation admitted by that receiving form. Existing-place updates do not acquire the materialization permission of a new ref declaration. Omitted outputs still undergo specified production effects and required cleanup; their payloads need not be read merely to discard them.

Reference outputs carry selected target/owner/access, whereas value outputs carry evaluated independent values. A scalar variable used as a value output is read at return preparation; a borrowed scalar later received into val storage is read under D-23’s post-call receiving rule. Callable outputs retain complete values and their borrowed dependencies without becoming fields in a lexical ADT. References obtained from different output components may share a target; no new exclusivity is inferred.

Mapping fields to lexical variables does not imply contiguous storage. Compiler-private output buffers, registers, stable slots or forwarded destinations may implement the same source contract. There is no source-visible aggregate identity or mandatory one-owner descriptor, and references sharing an actual owner can share sufficient resolution evidence under D-28. Growth must preserve selected destinations and target resolution.

All output preparation, return replacement/cancellation and final delivery retain H-571/D-17. A partially prepared result does not publish a successful subset of the return. A partial receiving pattern does not suppress callee work or repair an invalid producer. The proposal changes no allocation rollback rule. Variables updated or introduced by receiving can subsequently be used, captured or returned only under their own contracts.

### Remaining typing and ownership interfaces

D-24 still requires an independently established complete result contract. A receiving subset cannot choose the callee’s labels, result modes or overload. A bounded inference approach requires matching label sets across returning branches and uses existing permitted joins component by component; different labels do not silently synthesize a sum or optional output. Every reference component still requires one statically provable owner. Recursive inference requires independent evidence for the result schema and its components, not only a receiver pattern.

Forwarding may reuse eligible destinations but must preserve both callees’ completion, ownership and lifetime obligations. A forwarding wrapper must not deliver its result before its own cleanup; a later return replaces the logical set of pending outputs, releasing abandoned components as required. This is not C++ object copying/moving. Direct remapping can instead receive ordinary variables and return another labelled mapping without materializing a group.

The receiving specification below supplies field/local renaming, explicit operations, destination-conflict and scope rules. Result-bound owners use ordinary liveness and omitted-output cleanup as specified above. Broader result variance, nested schemas and field-level if-returning inspection remain separate extensions. Existing `if returning ()` can test a pending return without a payload binding; a first-class binding of the whole structural result is inconsistent with this proposal. No additional pattern syntax is selected. The ordinary ban on owning resources inside user ADT fields remains unchanged because these outputs are not ADT fields. Complete checking and native transport of dependent outputs still require proof.

Ordinary named ADTs retain their nominal identity, existing eligible placements and same-owner stored-reference rules. Callable-bearing or external-owner fields in ordinary ADTs are not required by this multiple-result proposal and remain separate unapproved extensions. No new kind of “lexical ADT” is introduced. The fixed regional callable-view prohibition and pure-bits-only whole-payload transfer continue.

### Caller-provided result destinations

H-571 confirms one internal output model for all result forms: a **caller-provided logical result destination**, whose contents remain governed by the callable's declared result contract. It is not a requirement to allocate a memory cell for every scalar or to pass one raw output pointer in every native ABI. Register/SSA results, scalar replacement and forwarding an eligible construction destination remain possible when observable behavior is preserved.

| Result category | What the logical destination receives |
| --- | --- |
| Independent scalar/bits result | Evaluated, converted, initialized bits. No source owner or target identity accompanies the value. |
| Borrowed ref/ptr result | Selected target/absence, actual owner, access and necessary type/layout evidence. The reference carrier's destination does not allocate or relocate its referent. |
| Complete callable result | A complete independent value under D-18's construction/result and borrowed-dependency rules. It is not a generic bits payload. |
| Fresh non-bits compound, if admitted | Construction in the selected eligible destination, with field initialization and cleanup responsibilities. Returning an existing callee-local object cannot substitute implicit copying, moving or owner promotion. |
| Owning region result | The ownership carrier and release responsibility delivered on successful completion; the regional graph is not moved into the result slot. |
| Void completion | Completion state without a payload allocation; explicit return and ordinary fallthrough retain D-17's distinction. |

The callable's selected result contract determines the mode; receiver spelling cannot change a value result into a borrowed result or change overload selection to avoid an omitted field. Independent return-type inference remains D-24's contract. Caller-provided backing does not supply permission to allocate in an arbitrary owner, weaken same-owner fields, extend a capture dependency, or put a callable view in a region.

**Preparation is distinct from delivery.** A return first prepares a candidate result, then the invocation completes its required cleanup. A later return can replace it; an applicable escaping failure can cancel/prevent delivery. Reuse D-17/D-23's completion rules and category-specific abandoned-result cleanup. A candidate can occupy caller-owned hidden storage without becoming the caller's visible receiving binding. Direct construction into final storage is an optimization only where it preserves these observations and valid outstanding borrows.

```cod
proc inspect_value(x:u64 in O) -> u64 where {owner O} {
    defer { x <- 9u64; }
    return 7u64;
}
val n:u64 <- 1u64;
n <- inspect_value(n);
# Final n is 7. Writing 7 directly into n before the defer would instead leave 9.
```

If callee cleanup throws, there is no final receiving update, although permitted earlier writes/effects remain. Explicit init-parameter writes are operations on their selected caller places and are not automatically delayed until call success. Thus a hidden return destination may share place/initialization machinery with init parameters without treating an ordinary return as an immediately visible output write. A hidden destination is not an allocation transaction: H-428 still forbids inferring rollback of existing-owner allocations or valid publication effects.

Destination forwarding through nested calls must preserve result replacement, actual owner lifetime, access, construction effects, valid partial-state cleanup and all admitted observations. D-28 additionally requires selected destinations to survive growth of their actual backing through owner-plus-locator resolution or another qualified stable-storage protocol. One raw address retained across a relocating call is insufficient. A common logical interface does not select one physical layout or require heap allocation.

### Init destinations and declared place aliasing

H-601 preserves ordinary parameter shorthand: `init n:Node in O` and `init ref n:Node in O` have the same meaning. Adding init supplies an initialization obligation; it does not add an extra reference indirection, change the declared type/owner, or invent a separate target parameter category. The parameter's ordinary value/reference/place role still determines which storage it exposes.

H-602 makes caller-slot declared place aliasing explicit:

```cod
init ref var n:Node in O
```

Here ref describes the reference binding, var makes n a declared place alias of the caller-selected reference slot, and init supplies the initialization contract. This is distinct from plain init ref n, which does not implicitly alias the caller's reference slot. The target owner O and the owner of that reference slot remain separate evidence. No hidden allocation, capture, ownership transfer or lifetime extension follows.

```cod
data Node { value:u64; ptr next:Node; }

proc create(init ref var n:Node in O) where {owner O} {
    n = make Node in O { value <- 1u64; next = null; };
    n.value <- 2u64;
}

# arena is an existing live writable region.
ref node:Node in arena;
create(node);
# node now selects the constructed, initialized Node.
```

The ordinary operators distinguish the places without another parameter modifier. `n = ...` initializes/rebinds the aliased caller reference slot. Once its identity is established, `n.value <- ...` initializes/writes a field of its selected target. If the slot is uninitialized, field selection cannot obtain a target from it. If the target is unfinished, knowing its identity does not make its payload readable. The compiler tracks these facts and mandatory writes separately; writing a target field cannot substitute for an owed reference-slot initialization.

For ordinary target initialization, the existing reference parameter form suffices:

```cod
proc fill(init n:Node in O) where {owner O} {
    n.value <- 1u64;
    n.next = null;
}

ptr reserved:Node in arena = make Node in arena;
fill(reserved);
```

Bare make establishes this non-null target identity without payload readiness. The init reference contract permits that incomplete destination and requires the declared initialization on successful completion; ordinary readable-ref interfaces cannot erase the restriction. Writing init ref n explicitly gives the same contract. Ordinary parameter-local rebinding does not rebind reserved or discharge initialization owed to the originally selected target. No target keyword is needed.

The same explicit-place composition gives `init val var x:T` for a caller bits place and `init ptr var p:T in O` for a caller nullable reference slot. These are derived combinations of the existing place and initialization roles, not permission to treat every plain init val/ref/ptr parameter as a caller-slot alias. All modifiers retain their ordinary type, access, owner and nullability restrictions; the bounded consequences are specified below, while complete shorthand/combination and nullable-target proof coverage remain separate. A native handle's category must be established before selecting one of these forms.

Initialization facts follow the selected storage through local flow, field writes and compatible helper delegation. Successful completion discharges the declared obligations, including mandatory writes; pre-existing readable caller state does not substitute for promised writes. Argument preparation may select an uninitialized destination without reading its old payload, but cannot obtain an object identity from an uninitialized reference slot. The selected place/target and actual owner must remain valid through effects, cleanup and permitted backing growth. Reference identity use is not a payload read and cannot manufacture missing readiness.

Public init writes take effect when executed; exceptions promise neither complete initialization nor rollback. Init region retains its separate already-selected root-storage contract, and ordinary ADT fields, graph readiness, D-16 numeric validity and sound conservative rejection remain. No implicit graph transfer, whole-object copying/moving, arbitrary resource reinitialization, reference-slot encoding conversion or native ABI follows. [D-20's native output planning](../applications/cod-booter/docs/LLVMInterop.md#initialization-through-an-output-reference) consumes these distinctions without selecting its handle representation.

### Bounded init obligations and nullability

The destination's ordinary role determines the promised write. Init does not mean that an existing payload is absent at runtime, but its contract cannot use pre-existing readiness in place of its promised initialization. Apply H-445's write-before-read rule to the selected init storage, including declared place aliasing and runtime target aliasing through references that retain that initialization responsibility; no implicit input/output parameter mode follows.

| Parameter role | A successful initialization establishes | What does not discharge it |
| --- | --- | --- |
| `init val var x:T` | A valid bits write to the selected caller place before reading that init payload. | The caller having initialized x earlier; initializing an unrelated callee-local value. |
| `init ref var r:T in O` | A reference-slot write satisfying its declared non-null owner/access contract, with required target readiness by successful completion. | Merely writing through its old binding; rebinding a separate local ref; writing null. |
| `init ptr var p:T in O` | A nullable reference-slot write preserving O and declared access/readiness obligations for any selected target. | Leaving the old slot untouched, even if already null. |
| `init ref x:T in O` / shorthand `init x:T in O` | Initialization of the originally selected target under its declared obligations. | Establishing identity alone; rebinding the parameter locally to another target. |
| `init ptr x:T in O` | The ordinary target write on every normal completion, with non-nullness established before dereference. | Taking a null branch that completes normally without the promised target write. |

For a pointer-slot destination, null is an initialized absence value with the already established owner:

```cod
proc clear(init ptr var p:u64 in O) where {owner O} {
    p = null;
}

# O is fixed independently of the slot's uninitialized contents.
ptr output:u64 in arena;
clear(output);  # Initializes the slot to null in arena; no target is allocated.
```

This differs from a nullable borrowed target:

```cod
proc maybe_fill(init ptr p:u64 in O) where {owner O} {
    if (p is not null) { p <- 0u64; } else {}
}  # Reject: the null path completes normally without a target write.
```

Null/identity tests are permitted once the reference identity is available. An initializer can require a non-null ref, or use an ordinary failure/assertion path when no target exists; an exceptional exit owes no successful initialization. It cannot silently weaken init into an optional write. If an init reference slot is itself uninitialized, even a null test reads unavailable slot contents; initialize the slot before inspecting it. A bare reserved target identity remains usable without reading its unfinished payload.

```cod
proc increment(init n:u64 in O) where {owner O} {
    n <- n + 1u64;
}  # Reject: reads the init payload before the promised write.
```

Passing an already initialized scalar does not make increment an initializer. Ordinary read/write parameters express that operation. A write followed by a read, or a delegated init helper's successful completion followed by a read, uses the resulting flow evidence. Binding another reference to an init target does not erase its restrictions; distinct binding slots remain distinct places, and no global target-disjointness assumption follows. Unproved relationships may conservatively reject under H-447.

Init does not upgrade readonly target or field-path access. Initialization of a previously uninitialized const binding follows its ordinary one-time initialization permission; it does not permit rebinding an already initialized const slot. A writable target behind a const reference binding remains distinct from permission to rebind that binding. General resource reinitialization and remaining modifier spellings are not supplied by this table.

Promised writes must hold on every successful completion, after its required cleanup. Registering a deferred write establishes nothing until that action executes. A permitted deferred write can contribute to the successful postcondition; a skipped write, failing cleanup or a return path without required initialization cannot manufacture it. Failure leaves already performed public writes/effects intact and provides no blanket completion promise. Ordinary field/readiness facts may remain usable only where separately proved; neither rollback nor automatic publication follows.

**Acceptance / proof scope:** cover initialized and uninitialized actual places, pointer-slot null versus missing target, null tests before/after slot initialization, mandatory writes on normal branches and after cleanup, rejected early reads, identity-only use, const slot versus readonly target, helper delegation, original-target preservation and owner/access invariants. Complete aggregate/publication, generic/callable-view/recursive summaries, remaining modifier grammar and native transport remain work. Existing compiler argument modes and assignment-family destinations do not constitute a general init implementation.

### Named return construction and declaration-directed reception

H-571 selects the named construction form:

```cod
return { n -> count, true -> ready };
```

The form supplies an anonymous named result directly. Under H-572’s return-only proposal it maps expressions to output components, without constructing a first-class group object. Evaluate field expressions once in source order; labels select result fields and must be unique. Apply an independently declared field contract where present, otherwise the established result-inference policy. Do not canonicalize effect order when comparing structural shapes. For scalar value fields, snapshot at return preparation; later cleanup cannot reread n to change count. Missing required fields, incompatible types/modes/owners and incomplete construction reject. Schema identity and projection follow the compatibility section above; per-component destination capability and broader contextual grammar retain their scoped proof requirements.

At this labelled-field boundary, `-> name` denotes a field label. A callable-owner projection used as a field expression needs explicit grouping, as at D-10's named-argument boundary: `(callback -> O) -> owner_field`. This is a grammar distinction only; it does not grant an otherwise inadmissible field kind. Unlabelled return blocks keep their existing expression semantics; empty-product syntax is not selected.

### Receiving specifications

H-574/H-575 supplies a **receiving specification**, normalized below under H-592 delegation: a static association between returned field labels, declared or existing destinations, and explicit receiving operations. It is not an executable body or a first-class structural value:

```cod
# count and selected are new; total and cursor already exist.
{
    val count <-;
    ref selected = node;
    total <- count;
    cursor = node;
} := inspect();
```

Every specification entry requires an explicit `=` or `<-`. A declaration keyword introduces a new variable under ordinary declaration/shadowing rules; without that keyword, the lhs must name an already declared admissible destination. A declaration does not become an update merely because an outer variable has the same name. The rhs of each item is a **source-field selector** (a structural result label or named regional root), not a caller expression. Omitting it supplies the simple lhs variable name. It does not infer an operation or a destination kind.

| Receiving item | Meaning |
| --- | --- |
| `val count <-;` | Introduce count from result field count using ordinary val initialization. |
| `val total:u64 <- count;` | Introduce typed total from differently named field count. |
| `count <-;` | Apply ordinary `<-` to existing count using field count. |
| `total <- count;` | Write existing total from field count. |
| `ref selected = node;` | Introduce selected using ordinary ref initialization from field node. |
| `selected = node;` | Apply ordinary `=` to the existing reference slot selected. |
| `selected =;` | Rebind existing selected from field selected. |
| `selected <- count;` | Write through existing selected to its scalar target using count. |
| `proc action = callback;` | Introduce a complete callable value using its ordinary declaration contract. |
| `action <- callback;` | Replace an existing callable value under its dedicated contract. |

The bounded standalone grammar is:

```text
receiving_statement := "{" receiving_item* "}" ":=" producer ";"
                     | "{{" receiving_item* "}}" ":=" producer ";"
receiving_item      := receiver_head ("=" | "<-") field_label? ";"
receiver_head       := ordinary_declaration_head | existing_simple_name
```

An ordinary declaration head contains its existing kind/type/owner/access syntax, without an initializer; otherwise the name must already resolve to an admissible destination. An unknown bare name does not implicitly declare a variable. The semicolon terminates each item. An empty specification discards every component of an independently established structural result, preserving all production effects and required cleanup; it does not introduce an empty structural type or relax producer checking.

Type/owner/access annotations retain their ordinary declaration meanings. A new scalar ref initialized from an independent value may use D-23’s permitted lexical materialization; an existing ref rebound with `=` cannot obtain that permission merely by appearing in a receiving group. A new `val` declaration with `=` or a new `ref` declaration with `<-` still rejects. Preserve independently admitted ptr shorthand rather than inventing a universal ref/ptr declaration rule. Numerical compatibility and D-16 validity remain unchanged; neither labels nor type inference provide a new conversion.

For example, when count is a val scalar output, `{ r <- count; } := inspect();` can write an existing writable scalar referent. `{ r = count; } := inspect();` rejects for an ordinary existing ref because the field supplies no storage target. `{ ref r = count; } := inspect();` is instead a new declaration and may materialize the returned scalar under its existing rules. Explicit operators therefore remove the need to choose an implicit slot-versus-target policy from the alias or field type.

Reference rebinding keeps the destination’s fixed target owner and permitted access. Bits transfer may cross owners without retaining a source reference. A `const ref` slot cannot be rebound but may still support `<-` through a writable target; `ref r:const T` may support admissible rebinding but cannot write the readonly target. Ptr binding can retain null under the fixed-owner contract; payload operations still require valid non-null/readable storage or the already admitted checked operation. Complete callable and owning operations retain their separate preparation, consumption and cleanup rules rather than becoming generic bits copying.

**Selectors and lexical lookup.** The bounded specification uses simple receiver names and simple source-field labels. New-receiver annotations use ordinary declaration/type syntax; they do not introduce arbitrary runtime expressions. An explicit selector is one source-field label. An omitted selector uses the lhs name, so `count <-;` selects field count and `selected =;` selects field selected. Empty-rhs assignment remains invalid outside this grammar. Unknown fields reject even if a caller variable has that name. Missing receiving entries are allowed; missing required fields of a structural result construction are not. Existing-region selection leaves omitted stored roots untouched. A receiving subset supplies no overload-selection or return-type-inference evidence.

No entry may contain arithmetic, a call, indexing, a computed field selector, a nested assignment or control flow. For an independently admissible addressable field/element, the caller can first declare an ordinary place alias outside the spec, then name that alias as an existing receiver. That preserves the ordinary one-time place selection and owner dependency without adding an expression evaluator to the specification. Type/owner annotations and omitted inferred types retain their ordinary static checking; this restriction grants no new type-query effects.

All existing receiver names and the producer expression resolve in the preceding caller environment. A new receiver introduces a binding only under successful reception; it is not an input to another specification entry or to the producer. In `{ val count <-; total <- count; } := inspect();`, the second count selects the same output field, not the new local value. Duplicate new declarations and a new/existing receiver-name collision within one specification reject; do not guess whether a bare name denotes a new binding or an outer variable. Reusing an output selector is distinct from conflicting writes to a destination, whose policy remains separately scoped. Ordinary shadowing outside that ambiguity remains unchanged.

Consequently, `{ ref chosen = cell; chosen <- count; } := inspect();` cannot use the newly declared chosen as the second destination. That would describe a program which follows one result to determine another destination. Write separate ordinary code after reception if that dependency is intended. The lhs itself declares only destinations and operations, not statements to execute after the call.

Single braces do not add a full lexical scope. Successful new bindings remain in the enclosing scope; existing-only specifications introduce no auxiliary field names at all. Double braces implement H-573’s temporary-scope goal using the same restricted specification:

```cod
# Temporary declaration: count ends with this operation's explicit full scope.
# A captured ref preserves its selected live target.
ref total:u64 = 0u64;
{{ val count <-; total <- count; }} := inspect();
# The target update remains; a val total would instead be updated only locally.
```

The standalone double-brace form is equivalent to placing its single-brace receiving statement inside an ordinary explicit full scope: that scope covers producer evaluation, reception and required cleanup/effect restoration. Existing destinations still resolve from the preceding environment; new names are invisible to the producer and do not escape the operation. H-FULL-SCOPE-CALLABLE-20261117 applies ordinary capture rules to those destinations: scalar/callable value writes and independent reference-slot rebinding affect invocation locals; writes through a captured reference or admissible declared place remain observable, without rollback. Use single-brace reception in the enclosing scope to update an enclosing value holder directly. Temporary receiving does not create a special capture-by-place permission. Ordinary full-scope lifetime/access checks forbid escaping a newly materialized target through another receiver or capture.

This syntax adds no arbitrary executable statements, callbacks or explicit defer declarations to the spec. To use the new variables in further code, write an ordinary explicit full scope containing the single-brace receiving statement and those subsequent statements. Existing-only reception needs no temporary names merely to rename fields. Generator headers reuse item mappings but retain D-29's separate repeated-handoff and control-lifetime obligations; this standalone temporary form does not decide their storage lifetime.

### Region sources for receiving

H-595 also admits an independently typed region as the source of the same receiving specification. [D-13 named roots](GraphGrainedMemoryManagement.md#named-root-access-and-receiving-specifications) defines the selected source fields. Existing destination selection precedes RHS evaluation as usual; evaluate the region expression once, then use its actual owner/layout/access and field readiness. New receivers do not become inputs, and ordinary partial/reordered/repeated selectors, temporary scopes and overlap rules continue.

A callable structural result supplies prepared component results; an existing region supplies addressable stored fields. This difference determines ordinary operation and cleanup behavior. `{ ref n = count; } := state;` borrows the regional bits place, whereas a callable's independent val result may materialize a new lexical target under D-23. `{ ptr p = next; } := state;` creates an independent local reference binding to the selected target, not a declared place alias of the regional slot. Use ordinary `state.next = target` or standalone `var link -> state.next` to update that slot. Receiving introduces no extra item operator or source-place alias form.

Reading an existing region for reception does not consume its owning holder. Unselected roots remain untouched, including unread payloads; there are no omitted-component cleanup actions for those stored fields. An empty spec evaluates its region source but selects no roots. Any temporary owned region produced by that expression still has ordinary temporary cleanup: a retained reference must not outlive it, while independently received bits can. This is not structural-result projection or a new whole-region transfer operation. A callable returning a structural result with an arena component is still governed by its result schema; do not automatically flatten that component's roots.

The reuse adds no first-class structural result value, arbitrary pattern expressions, regional callable storage, owning user-ADT fields or generator form accepting a region in place of a generator call. Accept source-field/mode/owner-compatible reception and exact source effects; reject missing fields, invalid operations, unproved root readiness, access escalation and owner escape. Region construction's per-root initializer is a separate preparation boundary, not a receiving block.

### Receiving semantics and optimization boundary

A specification expresses destination relationships, while the compiler chooses storage and delivery machinery that preserves their semantics. It creates neither a callable body nor a required runtime mapping object. Field resolution and receiving operations are checked statically. Each field retains its independently established value/reference/callable/ownership contract; `=` and `<-` retain the admitted operation rather than asking the compiler to infer intent from types.

**Preserved ordinary boundaries:** existing receivers use D-23’s destination selection before the producer. For `=`, retain the selected binding slot; for `<-`, retain the selected payload target. A pre-existing named place alias retains its earlier selected place. Necessary target/null/lifetime checks follow the ordinary operation, and owner-plus-locator resolution survives permitted backing growth. New receiver storage follows its ordinary declaration/initialization contract; a specification does not turn a new binding into a readable forward reference or add allocation rollback.

Execute the producer exactly once and finish its required cleanup/final return selection before successful receiving updates. An admitted producer-side rebind of r does not change the target already selected by `{ r <- count; } := inspect();`. A source-level equivalent optimization may eliminate materialized temporaries or use registers/forwarded private destinations, but it may not write a public receiver early when the producer could observe that write. Independent val outputs retain their prepared bits; borrowed outputs retain selected targets/owners/access, not frozen payloads. The spec grants no hidden lifetime extension or source-variable reread.

<a id="borrowed-read-overlap-and-diagnostics"></a>
### Receiving overlap and diagnostics

H-593 additionally makes **cross-entry write/write overlap undefined behavior**, including when every returned component is an independent val snapshot. Distinct receiving entries must not write overlapping semantic destination storage. This covers payload writes, reference-slot writes and otherwise admitted overlapping subobjects; reference rebinding writes its slot, not the selected referent. Accidental equality of the written values does not create an exception. Compiler-created temporaries or storage reuse cannot manufacture a source-level conflict.

For `{ a <- first; b <- second; } := pair();`, where pair returns independent 1 and 2 and a/b select the same target, there is no promised final 1, 2 or other result. To require ordered writes, receive into distinct ordinary values and then execute `a <- first; b <- second;` as separate statements. Those sequential operations retain their ordinary semantics even when a and b share a target.

H-576 makes **cross-entry receiving write / borrowed-read overlap undefined behavior**, under the program author’s responsibility. For distinct spec entries i and j, a receiving write by i must not overlap storage whose payload j reads through a returned borrowed reference. The rule concerns the semantic storage selected by the source program, including otherwise admitted overlapping subobjects; compiler-created temporary reuse cannot manufacture a violation. Neither receiver spelling nor a promise of readonly access proves target disjointness.

```cod
proc views(a:u64 in O, b:u64 in O)
    -> { ref first:u64 in O; ref second:u64 in O; }
    where {owner O} {
    return { a -> first, b -> second };
}
val a:u64 <- 1u64;
val b:u64 <- 2u64;
{ a <- second; b <- first; } := views(a, b);
# Undefined: each receiving write overlaps the other entry's borrowed read.
```

This spec has no guaranteed (2,2), (2,1) or other result. The compiler may snapshot borrowed payloads, reorder reads/writes or otherwise optimize under the non-overlap requirement. A warning does not select a recovery behavior. This is not an unspecified choice between two permitted outcomes and does not require a runtime check or exception. For defined inputs, ordinary effect, value, owner, initialization, failure and cleanup contracts still govern optimization.

| Situation | Scope of the rule |
| --- | --- |
| One entry writes a target which another entry reads through a borrowed result | Undefined, including a readonly borrowed view when a different writable path performs the write. |
| Independent val output fields swapped into existing variables | No borrowed payload reads remain; ordinary independent value semantics apply. The corresponding a/b example gives (2,1). |
| A new val receiver reads a borrowed field while another entry overwrites that field’s target | Undefined too; declaring the receiving variable val does not retroactively make the producer return a value snapshot. |
| Several borrowed reads share a target, with no conflicting receiving write | Allowed under ordinary access/lifetime rules. |
| Distinct ref/ptr destination slots bind the same returned target | Ordinary runtime target sharing; binding reference identity does not itself read or write the referent payload. |
| One ordinary transfer reads and writes its own target within a single entry | This cross-entry rule adds no prohibition; retain that operation’s established semantics. |
| Multiple entries write overlapping destinations, including independent val outputs | Undefined under H-593; neither source order nor a bounded set of final values is promised. |

The compiler must not turn this scoped condition into a global no-target-sharing assumption. Unique owners do not imply unique targets, and differently named owner parameters need not denote distinct actual owners. Existing single-destination operations outside a receiving spec are unchanged. Owner lifetime, fixed-owner rebinding, non-nullness, access and definite-initialization requirements remain in force; this decision does not relabel unrelated invalid references as permitted source.

**Diagnostic plan.** Report detected receiving write/borrowed-read or write/write overlap as a warning, with opt-in promotion to a compile error. This includes statically established conflicts; warning-as-error is the selected escalation, not a mandatory new runtime trap. The diagnostic should identify the spec entries, selected write and conflicting borrowed-read/write provenance. No warning code or command-line flag spelling is selected here.

Diagnostic completeness is not a prerequisite for the source rule. Cheap direct/proven-place cases are a reasonable first implementation; broader interprocedural, subobject or potential-overlap analysis may follow when its cost is justified. Absence of a warning is not a guarantee of disjointness. Do not impose a universal proof-of-disjointness requirement or automatically reject every uncertain receiving spec. The exact precision policy for possible rather than established overlap remains implementation/design work; no analysis, warning flag or native optimization is claimed implemented.

Acceptance should check warning evidence and opt-in error behavior once the diagnostic exists; a UB program must not acquire a test asserting one particular execution result. Defined controls include value-returning swaps into distinct destinations, read/read target sharing, binding one target into distinct reference slots, sequential writes outside the spec and valid single-entry self-transfer. Preserve diagnosis of independent type/owner/access/init errors and D-16 validity. The bounded receiving grammar is specified above; cleanup/ownership transport and diagnostic precision retain their separate review boundaries.

A producer failure performs no final receiving updates, although earlier permitted destination-preparation/producer effects remain. For defined receiving specifications, a later receiving-operation failure retains earlier valid effects under the applicable operation and initialization-group/unwinding rules; incomplete new bindings are not published as successful results. Retained complete/owning components require exact cleanup; omitted borrowed outputs do not destroy their targets. There is no whole-group transaction, automatic allocation rewind or C++ object copy/move model.

Repeated result selection is permissible only under the component’s ordinary consumption rules: bits may supply several transfers, borrowed targets may be reused under access/lifetime rules, and dedicated callable construction retains its own independence/cleanup. An owning result cannot be consumed twice. Different existing names may denote the same place/target; this supplies no general exclusivity, but H-576/H-593’s cross-entry write/borrowed-read and write/write conditions must be obeyed. Releasing or replacing an owner needed by a pending component/destination remains subject to checked dependencies; unproved combinations reject. The specification no longer requires creating a var alias of an owning holder, but this does not itself qualify a new multi-output ownership protocol.

**Acceptance / unresolved scope.** Cover new/existing/mixed specifications; same-name omission and explicit renaming; partial/reordered/repeated selectors; producer and existing-receiver lookup before new bindings; rejection of arbitrary expressions, forward receiver dependencies and ambiguous new/existing-name collisions; ref initialization versus rebind versus target-write; const/null/owner boundaries; direct/stored/recursive producers; pending return replacement/cleanup; preserved pre-RHS selected destinations; write/borrowed-read and write/write UB boundaries, planned warning/error escalation and defined sharing controls; preparation/receiving/temporary-scope cleanup failures and exact release; rejected scope escape/owning dependency errors; and D-16 compatibility. H-575 selects the declarative/no-arbitrary-expression requirement; H-576/H-593 selects the scoped receiving-overlap UB and warning policy. The bounded standalone grammar/lookup/temporary scope is selected under H-592; diagnostic precision, broader lifetime extensions and full proof/native transport remain pending-human; D-18's separate value-input review and current compiler coverage are unchanged.

### Engineering assessment and acceptance boundary

A return-only structural contract avoids implementing general anonymous aggregate objects, their field mutation/lookup, aggregate storage classification or a group lifetime solely for named multiple results. It still needs checked multi-output signatures, label mappings, shared/specialized native transport, per-component initialization/owner evidence and exact preparation/abandonment cleanup. Reuse the logical pending-result/completion model; a function returning several outputs is not a set of unrelated calls or immediate public init writes. Current general aggregate field support is incomplete, but a result-only design should not require it for independently supported scalar, borrowed or callable output categories.

The mainline developer’s read-only assessment finds this meaningfully smaller than general lexical products, with no unavoidable first-class-product requirement or separate runtime. Current syntax/checking/native paths transport one result payload; adding several outputs is substantive callable-boundary work, not a parser-only change. A checked result-contract sequence with statically mapped labels is a plausible organization, not a selected IR or ABI. Complete callable values may themselves have these result signatures without capturing a group payload. Owned-output dependencies, failure-prefix cleanup and propagation through every general/specialized path remain important proof boundaries. No code/build/tests were performed for that consultation.

Acceptance must cover explicit/inferred/generic callable results; direct/stored and recursive forwarding; label reorder with unchanged effect order; partial reception and omitted callable cleanup; independent per-component owners and dead-owner rejection; preparation failure, return replacement and outer cleanup failure; growth-safe destinations; and rejection of structural group variables, storage, sum schemas and incompatible branch labels. The selected schema identity/projection and result-bound-owner rules need complete checking and transport evidence; foreign transport and new inspection syntax remain under review. No current compiler support, performance estimate or physical ABI is claimed.

### Bounded result and initialization work remaining

The current structural-result, receiving, named-root and init profile has explicit source rules. Its remaining complete proof/native work does not reopen their already selected syntax or ordinary owner/lifetime criteria:

| Area | Selected source boundary | Remaining implementation/qualification scope |
| --- | --- | --- |
| Results and receiving | Result-only labelled products, explicit per-item operations, partial/renamed/new/existing reception, prior destination selection and scoped overlap UB. | Complete multi-output typing, private preparation/delivery, diagnostic coverage and transport through direct/stored/specialized calls. |
| Projection and owners | H-591 ordinary result projection and H-594 result-bound owner dependencies; omitted owned outputs release under ordinary liveness. | Dependency-preserving projection, independent inference, exact omitted/abandoned cleanup and growth-safe destinations. |
| Init destinations | H-601/H-602 ordinary reference versus explicit var-place roles; bounded null/write-before-read/completion rules above. | Complete modifier grammar, aggregate/readiness/publication proofs, recursion/indirect calls and native destination transport. |
| Region construction | Named roots, per-root/free initialization, construction-local owner, whole-region init helpers and no allocation rollback. | Actual-schema completion, conservative partial/recursive proof coverage and exact acquired-region failure cleanup. |

The overlap-warning implementation may begin with direct/proven cases under H-576/H-593; missing diagnostics do not establish safety or change UB into a defined execution order. Common init/construction cases may be recognized first under H-447, with explicit conservative rejection when required facts cannot be proved.

Field-level structural `if returning` patterns, broader variance beyond ordinary admitted component compatibility, first-class anonymous aggregates, callable-bearing ordinary ADTs and generalized stored external-owner fields are separate unselected extensions. They are not required for the current result-only profile. The current `if returning ()` presence check and explicit complete return reconstruction remain available under their existing contracts. This distinction is not blanket implementation approval: each new mechanism still needs its scoped authority and acceptance evidence.

## Bits classification and representation obligations

The proposed compiler separation is nominal type identity → resolved representation facts → placement/view → native transport. Classification must not replace nominal identity, actual-owner facts or abstract dispatch evidence.

| Obligation | Proposed integration / unresolved boundary |
| --- | --- |
| Concrete product | Check each instantiated direct field recursively; reject a ref/ptr or other non-bits constituent. Cache by resolved type identity and arguments, not source alias or actual owner. |
| Finite sum | Check all payloads and tags for finite bits representation. Preserve active-alternative validity. Concrete source/tag/layout work remains required; no tagged-union ABI is selected here. |
| Generic or recursive type | Distinguish unresolved evidence from proved non-bits. Detect infinite by-value expansion; pointer recursion is non-bits rather than a recursion escape in the classifier. Source constraints and diagnostics need an explicit contract. |
| Storage and field access | Address lexical and regional payloads through a common place abstraction retaining owner, access and initialization facts. Relative stored references keep D-01 encoding; a temporary native address is not a persisted CoD edge. |
| Payload transfer and borrowing | Only pure bits permit whole-payload transfer, requiring initialized valid contents. Reference binding retains a target/actual owner and does not duplicate the referent. Field readiness does not create generic object copy/move semantics. |
| Layout and native calls | Separate aggregate payload layout from reference transport. Preserve existing regional offsets; choose local aggregate argument/result transport with recursive callable and AOT requirements visible. No ABI choice follows from adding a classifier predicate. |
| Numeric validity | Bits eligibility promises no zero-validity or numeric conversion. D-16 fractions retain fixed payload/nonzero denominator and representation-independent UoM; array SoA/layout policy remains separate. |

The implementation dependency order is representation facts, aggregate places/initialization, payload operations, construction, then parameter/result/capture/TLS integration. Each increment must name incomplete admitted forms rather than present them as forbidden language. Classifying bits ADTs is not permission to make C++ `regional_*` objects stack-copyable.

## Owner-directed construction

The proposed compiler check for the confirmed construction behavior carries the resolved target type, actual destination owner, allocation capability, ordered field initializers and result access. Source forms feed that check; ordinary storage construction and intrinsic acquisition have distinct cleanup responsibility. This is a compiler organization proposal, not a new runtime object or universal allocator interface.

| Source situation | Required behavior and implementation boundary |
| --- | --- |
| Exception type T | Use [`make exception T(...)` or `make exception T { ... }`](ExceptionControl.md#make-exception-expressions) to receive a complete owning result normally. The optional `in region (bytes -> capacity)` configures a fresh region. In `make exception T from cause { ... }`, evaluate the cause once and construct the fields in its region. Combining the clauses rejects. Ordinary `make T [in o]` rejects for an exception subtype. |
| Explicit `make T in r` for ordinary data | Use r only if it supplies the required placement/capability. Reject a conflicting receiver owner before runtime evaluation. H-384 confirms explicit selection and mismatch rejection; H-438 supplies the generic owner allocation-access rule. |
| Ordinary data, omitted owner, receiver has one unique owner and suitable allocation access | Construct there. A field may supply its containing owner to an explicitly written nested make. H-386 accepts this contextual inference. |
| Ordinary data, omitted owner, no destination | Bits use the current full lexical scope. Non-bits still require an explicit actual owner when none is supplied by context; H-478 admits `in here` as well as a regional destination. No TLS fallback, hidden region or automatic owner choice follows. H-386's explicitness boundary remains; missing capability never causes a hidden allocator or fallback. |
| Ordinary data, receiver selects a const owner | Reject construction in that owner, including a fresh readonly result or otherwise admitted implicit target materialization. Do not silently choose another owner. |
| Ordinary data, receiver selects a non-const owner | H-438 permits construction using that actual owner; retain placement, policy, capacity, initialization and failure checks. |
| Initializer retains a reference result | Check actual owner/access. A non-bits graph is never implicitly copied; independent bits materialization and readonly reference reception remain distinct below. |
| Ref/ptr field initialized without explicit make | Independent bits may materialize a target under the proposed owner-directed construction rule. Reference results retain their target and access; readonly-to-writable binding rejects without allocation fallback. Regional materialization remains under review. |

Illustrative candidate source, not current parser acceptance:

```text
ref p:Point = make Point { x <- 1; y <- 2 };
ref n:Node in r = make Node in r { ... };
ref bad:Node in r = make Node in s { ... }; # Reject distinct actual owners.
owned region arena:Root <- make region Root(bytes -> capacity) { ... };
```

Completed construction must preserve initializer order and effects and publish only a valid initialized result. Bare typed reservation has the separate identity-only contract below. Existing D-17 failed acquisition cleanup does not promise rollback of effects or arbitrary reservations in an already-live region. Nested constructor naming/access, partial construction and allocator-policy interaction require review before implementing an uncovered case. Explicit owner spelling cannot bypass readonly allocation checks or same-owner stored edges.

## Region construction and named root initializers

H-596 requires individual root initializers; H-597 accepts a construction-local owner name, and H-598/H-599 admits initialization through helpers in a free initialization block. A named structural region schema fixes the root labels, storage kinds and full component contracts; construction supplies their initial contents in the new region. The bounded form uses `make region`, with allocator arguments separate from the initialization block:

```cod
data Node { value:u64; }
region State { count:u64; ref head:Node; ptr optional:Node; }

owned region state:State <- make region State(4096u64 -> capacity) {
    count <- 1u64;
    head = make Node { value <- 7u64; };
    optional = null;
};
```

The head destination supplies the new region as the contextual owner for its nested make; null obtains that same owner from the declared ptr root. Explicit owner annotations must agree. `in here` continues to denote its ordinary lexical owner and is not silently reinterpreted as the region under construction. No omitted constructor schema is inferred merely from arbitrary initializer labels; independently established construction context retains its ordinary inference role.

Every root must be definitely initialized on each successful completion of the block. Named initializer entries cannot name an unknown root; repeated ordinary writes are allowed and do not create another field. Missing initialization rejects; no implicit zero/default-null root initialization is added. An empty block completes only a schema with no outstanding root initialization obligations. Use `<-` for direct bits roots and `=` to establish ref/ptr root bindings, following ordinary field-kind rules. A root ref initialization is not a later write through an uninitialized reference. Implicit materialization beyond the independently admitted bits cases retains its separate D-22 review; the explicit nested make above needs no non-bits transfer or graph copy.

Evaluate allocation arguments once in source order before acquisition. Capacity is the runtime allocation budget governed by D-17/D-28, not an automatically growing hint. After acquiring the new owner and reserving its typed root storage, evaluate root initializers once in written order, independently of schema declaration order. Each completed item establishes its root's initialization fact. A field called capacity is distinct from the allocator argument. The free block admits ordinary initialization statements, admitted local declarations, control flow and helper calls. It executes in source order; it is not an unordered root map. Its local names remain internal to construction, while ordinary explicit full scopes retain their own lifetimes and cleanup. Root labels on initializer destinations select that new region's fields; a local-name collision requires explicit qualification through the construction binder rather than an ambiguous implicit choice.

All roots belong to the same actual region. Their direct bits occupy its storage; every non-null stored ref/ptr targets that same owner. An externally live object in another owner cannot be installed merely because its lifetime is long enough or its type matches. Setting a reference slot does not by itself prove its target readable. Successful construction must satisfy every root's declared access/null/readiness contract under the existing initialization proof; staged graph construction does not gain a bypass.

The new resource has one cleanup responsibility from acquisition until successful delivery. Publish the completed owning result only after all root initializers and required construction cleanup succeed. On failure, release that unfinished region exactly once and perform ordinary cleanup of initializer temporaries. Prior effects on existing storage remain: this is normal cleanup of a newly owned resource, not an allocation checkpoint/transaction for other live regions. A reference published through an initializer must satisfy ordinary lifetime checks including failure paths; do not silently retain failed construction storage to repair an escape. The caller's new owning declaration is not available as an initialized source inside its own RHS.

### Construction-local owner and helper initialization

H-597 accepts an optional region binder that lets later initializers name already initialized roots and give initialization/allocation helpers the new owner explicitly:

```cod
owned region result:State <-
    make region r:State(4096u64 -> capacity) {
        count <- 1u64;
        head = make Node in r { value <- r.count; };
        optional = r.head;
    };
```

The binder r is a construction-local borrowed region name; it does not create another owner, another root or a lexical-scope region. Its field reads require preceding initialization facts, so reversing the first two items would not permit reading count. A whole-region/helper argument must satisfy the helper's actual shape, access and readiness contract; a complete-region annotation cannot launder unfinished roots. Intrinsic allocation may use the established owner without reading root contents. Ordinary region parameters do not bypass whole-region readiness merely by requiring a smaller field subset; initialization helpers retain their explicit init contract. No consuming operation may extract the constructor's cleanup responsibility before success.

Without the optional binder, contextual make/null cases remain expressible. Use the binder for explicit region/root access in helpers and dependent initialization; do not introduce an ambient default allocator or reinterpret here.

H-598/H-599 admits delegation of the whole construction to an init helper:

```cod
proc initialize(init region r:State) {
    r.count <- 1u64;
    r.head = make Node in r { value <- r.count; };
    r.optional = r.head;
}

owned region result:State <- make region r:State(4096u64 -> capacity) {
    initialize(r);
};
```

`init region` borrows the already allocated region for root initialization. It does not initialize/rebind the caller's owning handle, transfer cleanup responsibility or accept a region schema as a user-data value. Its normal-completion contract requires initialization of every root in that actual construction schema, with the ordinary mandatory-write obligation; merely receiving already initialized storage cannot substitute for the promised writes. Before a root's first required write, reading that root is forbidden. Passing responsibility to another init helper is permitted and retains the same obligation. An ordinary region parameter requires the whole region to be ready; no guessed helper usage or optimizer evidence weakens that boundary.

Treat whole-region readiness as coarse state: constructing until all root contracts are established, then ready. Local definite-initialization facts for the finite named slots are enough to recognize an earlier initialized count and derive that completion. Branch joins retain only facts established on every continuing path; reads and helper calls cannot manufacture missing facts. No runtime readiness flag or full field-sensitive interprocedural analysis is implied. For the first sound implementation, whole-region init summaries establish the complete set on normal return; unknown partial/recursive relationships may reject. A narrower structural parameter requirement must not mark additional concrete roots ready without proof; require full construction-shape coverage for the whole-region summary.

Explicit `init val var`/`init ref var`/`init ptr var` place parameters may separately initialize eligible root slots under their declared bits/reference-slot roles. Plain init ref preserves ordinary borrowed-target semantics; it does not silently select a root reference slot. Selecting an uninitialized slot as an init destination does not read its old contents; initializing a ref/ptr slot still does not initialize its referent. A helper with only ordinary readable parameters cannot receive an unfinished region. On exceptional helper exit, no successful whole-region completion is promised; construction follows its ordinary cleanup, retaining permitted external effects. Init does not grant owning extraction, early publication with invalid lifetimes or arbitrary storage retyping.

The free block must finish normally with a ready region before make produces its owning result. An escaping exception or other existing abrupt completion abandons the unfinished construction under ordinary cleanup; no constructor-specific return value or implicit final-expression result is added. Concrete syntax/proofs for all init modifier combinations and deeper graphs remain separately bounded.

**Acceptance / proof scope:** cover independent root expressions, declared/written-order differences, mixed direct/ref/ptr roots, contextual make/null and explicit owner mismatch, a root called capacity, missing/unknown roots, repeated valid writes, unreadable slots/targets, initialization failure at every prefix and exact owning-result cleanup. Prior permitted external effects remain observable; stale/escaping construction borrows reject. The optional binder and free-block/init-helper source direction are selected; anonymous schema spelling and complete native/initialization proofs are not qualified by these examples. No regional callable/owned-user-field, implicit graph transfer, allocation rollback or D-16 validity change follows.

## Typed reservation and completed construction

H-562 fixes the distinction through the existing initializer-block syntax; no extra allocation keyword is needed:

| Form | Completion contract |
| --- | --- |
| Bare `make T [in o]` | Reserve suitably typed storage with a selected owner and a non-null target identity on successful allocation. Establish no payload initialization merely from allocation. |
| `make T [in o] { ... }` | Reserve and initialize storage, then provide a completed result. An empty block still requests completion and must satisfy the type's initialization requirements. |

This distinction concerns the bare ordinary allocation form. Existing constructor-argument calls and intrinsic resource/exception construction retain their own completion contracts; absence of a trailing block does not reinterpret `make region` or an exception constructor as incomplete raw storage. Type layout and owner allocation capability must be established before reservation. Physical zero-filled bytes are not evidence of initialized fields or valid typed values.

A ptr may retain a reserved target's identity under the existing initialization-flow rules. Fresh allocation supplies non-nullness, not readable payload. Projection of a known field destination, reference-identity assignment and `is` observations require their own owner/layout/reference-slot facts but need not read the target payload. Ordinary readable-ref uses, payload copies, captures or calls cannot erase outstanding initialization restrictions.

```cod
data Node { value:u64; ref next:Node; }
# r is an existing live writable owner; design acceptance, not compiler coverage.
ptr a:Node in r = make Node in r;
ptr b:Node in r = make Node in r;
a.value <- 1u64;
b.value <- 2u64;
a.next = b;
b.next = a;
ref ready:Node in r = a;  # Allowed once the complete component is proved ready.
```

The two reference-field assignments establish identities and dependency edges; they do not read the other node's payload. A finite required-reference cycle can become readable when every required field and edge in the tracked component is established. Merely closing a cycle proves no missing bits initialized. Before that point, initialization restrictions follow the targets through local flow and calls under D-28; ordinary readable publication rejects. This is the already selected staged proof model, not an implicit graph copier, runtime readiness tag or lazy evaluation mechanism.

Reservation is an allowed allocation side effect. Abandoning an unread incomplete target need not force a pointless write; its storage remains governed by its actual owner. Failure promises no allocation rewind in a live region, and lexical/whole-region cleanup retains its existing obligations. The distinction does not enlarge a type's storage eligibility: callable-view values remain excluded from regional storage, and D-16 fraction reads require a valid initialized payload, not merely allocated component bytes.

Acceptance covers bare reservation versus empty/nonempty initializer blocks, non-null identity before payload readiness, rejected early reads/ref publication, sequential and branched field initialization, the finite two-node component, and failure/abandonment without rollback. The same source distinction applies to eligible lexical and regional storage. Common proved cases may be implemented first; incomplete aggregate support or unknown lifetime/readiness facts must reject explicitly. Exact field/proof algorithms, qualifier combinations and broader publication precision remain separate.

## Unified lexical and regional initialization — proposal

The proposed common model is: select an owner with construction access, establish storage, initialize it, and establish the completed group bindings. Allocation is an allowed initialization side effect under H-428. P-348/P-350's candidate value is explicit allocation and lifetime policy in readable source, while charter §§3.3/4.3/6.3/7.2 requires preserved sharing, effects, access and failure behavior. H-PARENTHESIZED-VALUES-20261118 selects parenthesized scalar-value projection; H-READONLY-REF-BINDING-20261117 preserves ordinary readonly-reference access checking. Complete typed group transport and explicit D-01 region-resize extensions need the scoped review below. D-28 owns the distinct storage/representation refactoring. H-428 preserves D-17's separation of initialization effects from whole-region cleanup. Source examples illustrate design, not current compiler acceptance.

### RHS categories, grouping and value normalization

Initialization distinguishes an independent bits result from a retained target view. Receiver mutability or owner mismatch cannot silently change the RHS category. The common construction model retains these boundaries:

| RHS category | Ref/ptr initialization with `=` |
| --- | --- |
| Independent val bits, including a scalar value-result call | Allocate a fresh destination in the selected owner and initialize its payload. |
| Readonly target view of initialized bits | Retain the target with readonly access and its actual owner. A writable receiving reference rejects; no implicit fresh-target fallback. Explicit bits transfer may read the source into independent storage. |
| Mutable target view | Bind to the target, preserving actual owner/type and checking destination access compatibility. |
| Non-bits object view | Preserve the actual target, owner and access; readonly access supplies no graph-transfer permission. |
| Nullable/absent pointer view | Preserve pointer absence/provenance under D-14. Neither null nor an ordinary pointer view silently becomes a bits payload read. |

Here readonly describes target access (`ref r:const T`), not declaration-leading const (`const ref r:T`), which only restricts rebinding the reference slot. A const binding may still expose a writable target. Copy eligibility does not depend on a machine representation being one word: ordinary references, intrinsic ownership carriers and callable environments retain their separate categories.

H-PARENTHESIZED-VALUES-20261118 selects the [parenthesized value-expression rule](SymbolicDynamicScope.md#parenthesized-value-expressions-and-operator-idioms): `(x)` projects readable scalar bits to an independent val result. Nested grouping is idempotent with respect to this category; it performs no ownership transfer, invalidation or C++ move. A bare addressable storage expression still supplies its place/view, including when the storage was declared val; the declaration’s storage kind and the expression’s result category are distinct. A scalar view returned by an operator follows the same projection rule when explicitly parenthesized.

```text
data Box { ref n:u64; }
# r supplies live allocation access.
val local:u64 <- 7u64;
ref a:Box in r = make Box in r { n = (local); }; # Fresh copied bits in r.
ref b:Box in r = make Box in r { n = a.n; };     # Mutable view: alias a.n.
ref c:Box in r = make Box in r { n = (a.n); };   # Fresh copied bits, even in r.
```

A `val T in O` remains invalid. Fresh target placement comes from the receiver/containing object or the full-scope default, with usable allocation access; the copied value carries no owner. No owner-mismatch heuristic selects copying. Independent targets remain distinct even when their bits compare equal. Direct/val fields use `<-` to initialize inline payloads, without a separate target. Ref declaration `<-` stays rejected; later `=` rebinding and `<-` payload writes retain their separate contracts. D-14's ptr transfer-declaration shorthand is not silently extended or removed.

Expression parentheses can change admissible parameter behavior and overload selection: `consume(x)` and `consume((x))` do not necessarily supply the same category. `(x) <- value` cannot write x’s scalar storage, and `(x) is x` cannot obtain a target identity for the independent bits. Call argument delimiters do not themselves project values. The operator’s own result stays unchanged: `ref live:const u64 = n <- V();` retains its readonly view, while `ref snapshot:u64 = (n <- V());` materializes explicitly projected bits. Preserve once-only evaluation and completed RHS/callee effects before the projection.

For non-bits operands, the selected principle is still a value-category result, not transparent forwarding of an original source place. This does not invent an owner-free val ADT, generic object transfer or new mutable storage. An admitted computed borrowed result keeps its actual owner/target, nullability and access; readonly access is not removed, and parentheses themselves do not consume an owned region. Callable values retain D-18’s dedicated rules. Exact owning-carrier and non-bits result/place combinations remain typed-operation work; an unknown result carrier must not fall back to transparent original-place forwarding. No physical carrier or lifetime extension is selected here.

### Readonly reference reception and explicit bits transfer

A readonly returned reference remains a reference to existing storage. `=` cannot turn its target writable, and the compiler must not make an incompatible declaration succeed by inventing fresh storage. Readonly access also does not freeze the target: another writable reference may update it.

```text
proc watch(x:const u64 in O) -> ref const u64 in O
    where {const owner O} { return x; }

ref n:u64 = 7u64;
ref live:const u64 = watch(n);    # Retain n's target through a readonly view.
ref bad:u64 = watch(n);          # Reject: readonly target cannot become writable.
val snapshot:u64 <- watch(n);    # Explicitly transfer readable bits.
ref independent:u64 = snapshot; # Bind to snapshot's independent writable storage.
n <- 9u64;
# In the accepted program with bad omitted: live observes 9; snapshot observes 7.
# live is n; independent is snapshot; independent is not n.
```

The rejection is confirmed by H-READONLY-REF-BINDING-20261117; the accepted bindings and bits transfer follow D-23’s existing matrix. An inferred receiving reference preserves the returned view’s access and unique owner; omitting a type annotation cannot obtain writable access. Explicit receiver owners must agree with the retained target. No owner-mismatch or receiver-mutability heuristic selects materialization. Declaration-leading `const ref` constrains the reference slot and cannot remove target constness. Ref-field initialization and new-reference receiving items obey the same access check; existing-reference assignment retains its ordinary checks. A declared place alias has no permission to upgrade the underlying place or target.

Reading initialized readonly bits into an independent value does not change the source’s access. `<-` already makes this operation explicit; a value-result callable also supplies independent bits. H-PARENTHESIZED-VALUES-20261118’s `(watch(n))` is an explicit independent scalar-bits expression and can initialize fresh writable storage; this does not remove constness from the original target. Full typed carrier/implementation qualification remains separate. None of these operations extends the source lifetime or permits whole non-bits object transfer. D-16 numerical compatibility and validity, callable result modes and later assignment operators remain unchanged.

### Conditional results

Infer one result type and one result category for an `if` expression from all its result-producing arms, including `else if` arms. This happens before reception. Every arm that produces a result must support that same contract through admitted operations. Reject an expression with no unique admissible join. A path that leaves by an enclosing return or a throw supplies no value to the join and keeps its usual control flow. Check branch code statically. At runtime evaluate each condition reached once, in order, and evaluate only the selected arm, once.

```cod
proc example(use_n:bool) -> u64 {
    ref n:u64 = 1u64;
    ref chosen:u64 = if (use_n) { n } else { 7u64 };
    chosen <- 9u64;
    return n;
}
```

Both `example(true)` and `example(false)` return `1u64`. A readable scalar target and independent bits of the same type join as `val u64`. The `n` arm reads its payload when it produces that value. The declaration materializes the resulting bits in fresh lexical storage on either path. Consequently `chosen is n` is false on both. If the two arms instead yield `n` and another compatible writable target `m` with the same proven owner, the result retains the selected reference and writing through `chosen` updates that target. Reordering the arms preserves the inferred contract.

A value result carries no owner. A selected readable target can therefore contribute bits from another live owner. Require initialization and ordinary read access at the point of projection. A borrowed call result becomes available after that call's cleanup, so its projection observes completed callee writes. Cleanup enclosing an already evaluated bits result cannot change its snapshot. Reference results retain the common access and require a single statically provable owner. The receiver cannot promote readonly target access or invent an external owner for fresh storage. Later rebinding and borrowed arguments still require an existing target.

The join uses existing type, unit, validity and conversion rules, including D-16 where applicable. It supplies no new implicit dereference of a nullable pointer, whole-object bits transfer, ownership conversion or graph transfer. Other result kinds need an admitted way to produce their common contract.

### Scalar expression checking and lowering

For supported scalar bits and readable scalar targets, the following checks apply to expression parentheses and conditionals. Determine the common result type and category before applying a receiver. Preserve the distinction between expression parentheses and syntax delimiters. On a selected value arm, obtain the independent bits once where that arm produces its value. On a selected reference arm, retain the target, its access and its proven owner through reception. An implementation can use registers, branches or a result destination as long as these observations agree.

```cod
proc observe(left:bool) -> u64 {
    ref a:u64 = 1u64;
    ref b:u64 = 2u64;
    ref chosen:u64 = if (left) { a } else { (b) };
    chosen <- 9u64;
    return a * 10u64 + b;
}
```

| Arms (true / false) | Result used by the receiver | `observe(true)` | `observe(false)` |
| --- | --- | ---: | ---: |
| `a` / `(b)` as written | Independent bits on either path. The declaration supplies fresh storage. | 12 | 12 |
| `a` / `b` | Both writable references have the same lexical owner. The declaration binds the selected target. | 92 | 19 |

Exercise both arm orders. A value receiver transfers bits under its ordinary rules. An existing reference rebind or an argument requiring an existing target must reject a projected value. A bare readonly target retains its access. A retained reference still requires one provable owner. The condition and selected arm execute once. A nonexecuting type query must use the same static result and leave program state untouched. A returning or throwing arm supplies no value to the join. For a call yielding a scalar reference, projection reads after the call’s cleanup and before later enclosing cleanup can change those bits.

Check the examples and these acceptance cases in the checker and in both native lowering modes. Acceptance includes the mixed reference/literal example above and the reference/reference control. Existing numeric types, units and validity rules apply. Non-scalar carriers, implicit ownership or graph transfer and new numeric conversions are outside this scope. [Expression parentheses](SymbolicDynamicScope.md#parenthesized-value-expressions-and-operator-idioms) · [Scalar receiving rules](ScalarCallResults.md#call-site-matrix-new-declarations)

### Initialization order and allowed allocation effects

H-427's joint initialization establishes the group bindings on success; H-428 explicitly permits allocation as an initialization side effect. There is no CoD allocation checkpoint, automatic cursor rewind or implied transaction over initializer effects. Initializers run in source order and evaluate once. For a fresh target, H-416's conceptual order remains reserve, evaluate/convert, initialize; failure to reserve may prevent RHS effects. Earlier bindings are available to later initializers and the final group block. Lookup in an individual initializer still uses the preceding lexical environment. Owner liveness and allocation access must survive every pending write.

Joint initialization is not recursive data binding. Ordinary proc/func groups do not expose later targets or incomplete objects as readable refs. H-317's nullable-edge route remains the current ordinary contract. H-447 confirms [bounded common-case acceptance followed by demand-driven proof precision](RegionalAndLexicalStorageRefactor.md#two-stage-acceptance-with-invariant-soundness), including finite cycles when aggregate and completion prerequisites are ready; this does not change ordinary group lookup. Future pure knot-tying is a separate [D-19 research goal](ConstFunctionsAndTailCalls.md#knot-tying-as-a-pure-design-goal), not a change to the ordering or effects rules here.

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

Construction can share an owner/access/initialization model across those domains without sharing their allocator or reference encoding. Current full-scope and ordinary-grouping boundaries retain D-17. Lexical bits construction remains available; H-478 selects reference-bearing lexical placement under D-28's one-ADT contract, whose physical layout and complete native coverage remain outstanding. No scope-region resource, lexical region-capacity hint, automatic capture permission or new outer-selector syntax is implied.

### Lexical ADT construction integration

The [integrated lexical ADT review in D-28](RegionalAndLexicalStorageRefactor.md#lexical-adts-as-part-of-the-complete-model) records H-478's selected one nominal ADT with placement-specific layout/access evidence. Same-owner fields make a bounded lexical graph model possible without first admitting cross-owner view records. Pointer fields remain non-bits even when represented by integer offsets. Required ref initialization, nullable cycles, array/inline layout and partial publication need composed proof/representation coverage; bits-only lexical support remains an implementation fallback.

[Owner-qualified construction in D-28](RegionalAndLexicalStorageRefactor.md#owner-qualified-construction) selects the common permission for caller-owned and current-scope storage. A writable-target parameter permits owner O; const owner O remains available as an explicit no-allocation contract even with writable targets. All-readonly parameters permit only const owner O. Allocation uses the actual owner's path carried through shared runtime information, without an extra builder parameter merely to request permission. A callee's own storage cannot stand in for caller-owned graph nodes.

Traversal remains in selected library `(<-)` methods, using H-439/[D-28 identity contract](RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator) for owner-plus-locator identity and graph memoization support. Libraries retain sharing/cycle policy and failure expectations; ADTs need no mandatory application ID field merely to distinguish reference targets. H-437 metadata sharing preserves each H-438 qualified access path; it neither merges permissions nor supplies a default graph-copy algorithm. H-478 settles nominal identity and lexical placement; physical allocator transport and concrete layouts remain separately reviewed. H-428 effects, D-16 validity, register/stack optimization and existing capture/TLS boundaries remain unchanged.

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

H-426 retains inline TLS payloads for bits ADTs and the separately defined callable-view category. Explicit symbolic references require an owner contract and valid access/lifetime evidence. D-10/H-561 permits an owner parameter instantiated from a proved lexical or ordinary regional target; the region-selector form keeps its separate constraint. TLS holds a selected reference rather than an inline non-bits object. The lexical owner-base representation supplies no lifetime extension. Intrinsic region-resource bindings retain their ownership/restoration role; no generic ADT can carry an intrinsic owning resource.

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

### Publishing a newly created DBMR

The following uses candidate acquisition syntax. The final-path rule applies to either spelling.

```cod
region Catalog { version:u64; rows:u64; }
owned region catalog:Catalog <- make region Catalog from new path {
    version <- 1u64;
    rows <- read_manifest();
};
```

A creator may prepare storage privately. It may publish the final pathname only after every root and all fallible acquisition preparation have completed. Publication installs the complete region in one step conditional on the destination still being absent. Only a successful publisher delivers its owning result.

| Observation | Required behavior |
| --- | --- |
| Another process opens the destination during initialization | This attempt exposes nothing there. Once published, the first successful open can observe both initialized roots. |
| `read_manifest()` throws, or publication fails | Deliver no owner and release this attempt’s private resources exactly once. The attempt leaves no final entry of its own. A retry can use the name if it is still free. |
| The destination already exists, or two creators race | Preserve the existing entry. At most one creator can publish at an absent name. A loser cannot overwrite, truncate or unlink the winner’s file, including during cleanup. |

Initializer effects on other resources follow ordinary CoD rules. Visibility at the final name is distinct from persistence after a process or system crash. Recovery of abandoned staging storage, flush guarantees and modifications to existing images require separate protocols.

### Other acquisition operations

The matrix applies to both source candidates. Fresh publication is settled above. The other file protocols are proposals.

| Situation | Contract or open question |
| --- | --- |
| New source, destination exists | Acquisition fails and preserves that entry under the fresh-publication rule. |
| Existing source missing, invalid or incompatible | Reject; no silent create, rebuild or clear. Specify root/layout/reference validation and failure types. |
| Const result with requested preparation writes | Obtain authorized preparation access, perform specified writes, then publish the restricted result. Decide physical mapping protection independently of the already-confirmed language access rule. |
| Expansion/trim request already satisfied | Specify exact-size versus minimum-capacity semantics, total/payload/free-byte units, limits and idempotence; do not inherit the C++ reserve-free API accidentally. |
| Trim intersects occupied storage | Reject. Tail trim preserves retained offsets and is not compaction. Validate under the selected coordination boundary. |
| Failure while preparing an existing image | Release every acquired handle/mapping exactly once. The remaining file contents and extent require a separate contract. |
| Shared mapping and preparation/replacement | Define admissible access and coordination. H-430 excludes ordinary in-place language growth; acquisition preparation or a consuming replacement still needs a backing-file/shared-mapping protocol. Shared mapping supplies neither ownership exclusivity, locking nor transactions. |
| Release/flush failure | Define explicit observable failure and cleanup behavior; closing an acquisition does not delete data or guarantee durable publication. |

Before final R3 selection, compare a concrete capacity/failure/coordination contract and its costs against the existing primitives. Include a state-by-state failure table, access requirements and publication point. Existing snapshot save/replace (C-78) and compatibility-marker planning (C-77) remain separate inputs, not completed protocols.

## Implementation dependencies

The [source capability reference](../tests/cod/language/README.md) owns current compiler support. Construction integration must keep syntax, payload placement, field access, owner/access/initialization proofs, callable transport and cleanup consistent. A bits predicate alone cannot establish aggregate transport or lexical field-place support.

The current C++ [DBMR::create](../include/shilos/dbmr.hh) opens or creates the final file, resizes and maps it, then constructs the root. A throwing root constructor unmaps the view after the name is already visible. The [new C native provider](../applications/cod-booter/README.md#next-stage-repositories-and-native-runtime) must implement exclusive publication of the initialized image. A separate existence check followed by an overwriting publish would race with another creator. Measurements must distinguish counted operations from observed latency and resource totals.

[K-RESIZE-20261114](../CHANGELOG.md#k-resize-20261114) records the current proposal review; whole-file and unresolved construction/acquisition qualification remain pending-human.

## Acceptance and decisions before implementation

| Scope | Required evidence |
| --- | --- |
| Bits and owners | Local/regional nested products, finite sums and generic instances; copies independent across owners; aliases retain owner/access; reference/owner/callable-bearing types fail bits classification; invalid local escape and cross-owner edge reject. |
| Owner allocation access | Mutable owner permits allocation; const owner forbids direct/indirect allocation while preserving declared target writes. Cover all-readonly, mixed, nullable and binding-const inputs; metadata sharing cannot upgrade access. |
| Construction | Explicit owner agrees with receiver; equal root type never hides distinct owners; selected inference requires usable capability; existing borrows retain provenance; once-only ordered initialization, premature-read rejection and partial-failure cleanup. |
| Unified initialization proposal | Compare bare places, grouped val results, readonly bits, retained mutable/non-bits views and null across declarations/fields. Check normalized conditional identity, live source reads before cleanup, grouping/dispatch effects, const-binding separation, retained readonly target identity, rejected readonly-to-writable reference binding, explicit independent bits and unchanged later assignment. |
| Initialization effects | A later initializer failure preserves earlier valid publications and allocations in an existing live region; incomplete bindings/readable objects are not exposed. Preserve source order, exact cleanup for failed whole-region acquisition, nested effects and existing defer/completion rules. No implicit allocation or graph rollback. |
| Distinct storage and explicit resize | Use D-28 placement/representation acceptance. Cover grow/shrink/equal-capacity replacement, lossless retained extent, pre-entry failure, consuming entry, old-view invalidation, fresh result views, post-entry rejection/failure cleanup and abandoned owning results. No automatic growth, ownership restoration or scope-region backing; capacity/backend details need their own acceptance. |
| Composition | Aggregate arguments/results through native recursion, callable captures/dynamic bindings, deferred execution and loops preserve value/view semantics and existing ownership transfer timing. |
| File acquisition | Fresh creation covers reader visibility, failure and retry, and two racing publishers with exactly one winner when both are otherwise successful. Further coverage includes the selected source/access/capacity operation matrix, invalid image rejection, exact handle/mapping release, relocation preserving sharing/cycles and observable preparation effects before const publication. |
| Costs | Compile time, generated text/bitcode, stack/backing bytes, region/mapping count and peak extent; no accidental per-object cleanup or graph copy. No numeric target is assumed. |

## Review process and remaining topics

Discuss unresolved design topics in terms of source examples, behavior, alternatives and consequences. Registry identifiers are internal traceability, not items the user must approve individually. Once the topics are resolved, present the complete revised document for approval. This revision records the discussion decisions; it does not claim final approval or ask the user to approve competing alternatives.

Contextual owner inference and the coordinated migration direction are recorded above and need no repeated preference question. The remaining topics are:

- **Aggregate values and native calls:** define concrete product/sum representation, sufficient generic bits evidence, initialization granularity and parameter/result transport while preserving nominal identity, relative graphs, complete callable values and fraction validity. Distinguish internal compiler choices from observable layout or validity commitments.
- **RHS normalization:** complete type-specific owning/non-bits carrier integration under the common conditional result rule. Parenthesized scalar bits and assignment-result projection are selected; bare readonly reference reception preserves access and rejects incompatible writable reception. Value-category parentheses grant no generic object copying/moving, graph transfer or lifetime extension.
- **Initialization integration:** preserve H-428 allowed allocation effects and D-17 cleanup while completing precise RHS/name/order checking. No checkpoint or publication-rollback choice remains open at the language level.
- **Storage refactoring and explicit-region resize:** D-28 owns lexical/regional reference representation, any lexical non-bits layout and external_ptr naming. H-430–H-432 supplies explicit consuming resize with lossless success and ordinary failure cleanup; review the capacity/error mapping and DBMR backing protocols.
- **Persistent acquisition:** complete the source/access/capacity/failure/coordination matrix and compare its implementations before choosing the DBMR surface. Result-only constness and fresh final-path publication are confirmed. Mapping protection, capacity units, existing-image effects, validation and release/durability outcomes remain open.

The candidate value rationale, constraints, non-goals and acceptance remain part of the eventual whole-document review. No separate approval of internal relationship numbers is requested.
