# CoD symbols, dynamic scope and callable kinds

Design revision: 213 · Recorded: 2026-11-18 · Phase: language/application co-design.

## Design authority

- **Language feature overview.** [K-GENERALIZED-FOR-20261118](../CHANGELOG.md#k-generalized-for-20261118) updates D-10r213 for the feature overview. D-29-GENERALIZED-FOR/P-623–P-624 owns equivalent call forms. D-10-TRAILING-CALLBACK/P-567–P-569 supplies the binding rules. H-HERE-ORIGIN-HANDOFF-20261118, H-HERE-BREAK-CONTINUE-20261118 and D-29-EFFECTS explain synchronous caller-owned work under charter §§3.2/6.1/8.1. The [generalized for-loop examples](RangedForAndGenerators.md#generalized-for-loops) compare live writes, feedback and control targets within the existing source contracts. Collection conformance and lowering remain separately scoped.

- **Trailing single braces.** [H-HERE-BREAK-CONTINUE-20261118](../CHANGELOG.md#h-here-break-continue-20261118), D-10r212, D-18-HERE-GROUP/P-619–P-620 and D-18-HERE-BREAK/P-621–P-622. Unprefixed trailing groups use separate destinations for body feedback, early completion of the supplying call expression and lexical return. The [forwarding example](CallableStorageAndHereAccess.md#break-and-continue-in-a-single-brace-here-body) checks each operand against its destination.

- **Definition-installed effectful procedures.** [D-10-HANDLER-INSTALL](DesignAuthorityRegistry.md#d-10-handler-install), r207, covers the [restricted source forms](#definition-installed-effectful-procedures) under [H-EFFECTFUL-PROC-INSTALL-20261118](../CHANGELOG.md#h-effectful-proc-install-20261118). C-36/C-65 and charter §§3.2/4.3/6.1 connect scoped handler selection with statically bounded access to runtime state. Activation-time snapshots and ordinary restoration supply the baseline. [H-EFFECT-CONSUMER-CALL-20261118](../CHANGELOG.md#h-effect-consumer-call-20261118) fixes the direct-definition versus ordinary callable-value distinction for lexical helpers that reach perform/behave. Acceptance covers both invocation modes, forbidden handler extraction/assignment, direct helper chains and rejection through a stored value. [D-18-HERE-PARAM](DesignAuthorityRegistry.md#d-18-here-param) reviews the proposed lexical permission for procedural literals at a here formal. Family integration and implementation qualification remain open.

- **Type membership and branch refinement.** [H-ISA-20261118](../CHANGELOG.md#h-isa-20261118) confirms the [`isa` source rules](#type-membership-and-branch-refinement) at r205. [D-10-ISA](DesignAuthorityRegistry.md#d-10-isa), P-587/P-588, connects explicit typed intent and reference safety (C-10/C-65) to charter §§3.3/4.3/6.3/8.1. Acceptance covers a nullable target, early exit, rebinding and an independently retained reference. Owner, access and initialization checks still apply. Runtime evidence transport and compiler qualification remain open.

- **Trailing callback failure checking.** [H-DOUBLE-BRACE-THROWS-20261118](../CHANGELOG.md#h-double-brace-throws-20261118) confirms the checked interface in [D-10-TRAILING-CALLBACK](DesignAuthorityRegistry.md#d-10-trailing-callback), r204. P-567–P-569 connect a readable callback body to its independently selected signature under charter §§3.3/6.3/8. Check the body against the callback parameter’s declared failures, including an empty checked set. Selection and ordinary caller obligations remain fixed. Examples below give acceptance. Full trailing-callback implementation qualification remains pending.

- **Parenthesized value expressions:** [H-PARENTHESIZED-VALUES-20261118](../CHANGELOG.md#h-parenthesized-values-20261118)/[K-PARENTHESIZED-VALUES-20261118](../CHANGELOG.md#k-parenthesized-values-20261118), [D-10-PARENS](DesignAuthorityRegistry.md#d-10-parens)/P-573–P-575, selects r203’s value-category expression parentheses and preference for unparenthesized operator idioms. Scalar bits projection, unparenthesized reference-result chaining and the delimiter distinction follow the confirmed choice; no C++ copying/moving, graph transfer, access upgrade or lifetime extension follows. Candidate C-06/C-41/C-65/C-72 → charter §§2/3.3/4.3/6.3/8/12 makes value-versus-target intent explicit while preserving readable composition. Acceptance and type-specific limits are below; complete grouping grammar/checker/value-path qualification and non-bits carrier details remain separate. D-16, ordinary evaluation and the r92 full-scope profile retain their stated boundaries.

- **Full-scope dependency continuity:** [K-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#k-full-scope-implementation-20261118) updates only the referenced r92 qualification. This document’s semantic revision, own qualification, acceptance and exclusions are unchanged.

- **Semicolon omission and syntax UX:** [H-SEMICOLON-UX-20261117](../CHANGELOG.md#h-semicolon-ux-20261117)/[K-SEMICOLON-UX-20261117](../CHANGELOG.md#k-semicolon-ux-20261117), [D-10-SEMICOLON-UX](DesignAuthorityRegistry.md#d-10-semicolon-ux)/P-570–P-572, covers r201→202's general source/diagnostic principle below. The human selects permissive omission where practical and warnings with clearer alternatives for misleading valid spellings; corner cases are ongoing UX work. Candidate C-06 → charter §§8.1–8.3/9/12 supports readable source and actionable diagnostics without guessing intent. Preserve deterministic syntax, source meaning, owners/effects and D-16; exact newline/insertion grammar, warning heuristics and complete implementation/value-path qualification remain pending. Existing examples with explicit separators are not a universal mandatory-semicolon policy.

- **Trailing here arguments.** [H-CALLABLE-MODEL-20261118](../CHANGELOG.md#h-callable-model-20261118), D-10-TRAILING-CALLBACK/P-567–P-569, continues first-slot insertion and default evaluation order. D-18 r127 governs live lexical access, nonescape and the control destination selected by the braces.

- **Generator body handoff:** [H-603](../CHANGELOG.md#h-603)/[K-GENERATOR-CALLER-EFFECTS-20261117](../CHANGELOG.md#k-generator-caller-effects-20261117), [D-29-EFFECTS](DesignAuthorityRegistry.md#d-29-effects)/P-558–P-560, covers r199's function/effect boundary. Yield transfers control to the caller-owned loop body, which is checked under that caller's lexical permissions and is not an indirect procedure call by a func generator. Preserve H-197/H-198 for generator-owned statements and ordinary calls, existing dynamic binding/restoration, owners and D-16; D-29 owns the nested/feedback/cleanup acceptance matrix. C-37/C-96/C-65/C-84 → charter §§2/3.2–3.3/4.3/6.1–6.3/8/12 serves reusable traversal with visible context responsibility. Only the source boundary/rationale is confirmed; full mechanism/value-path/native qualification and D-18's value-input review remain separate.

- **Named region roots:** [H-595](../CHANGELOG.md#h-595)/[K-NAMED-ROOTS-20261117](../CHANGELOG.md#k-named-roots-20261117), [D-13-ROOT-ACCESS](DesignAuthorityRegistry.md#d-13-root-access)/P-546–P-548, aligns this revision's regional examples with named roots, ordinary `r.name` assignment destinations and receiving specifications; the source root intrinsic is removed. Reuse C-74/C-65/C-72 and charter §§2/3.1–3.3/4.3/6.3/8/12 for explicit storage, access and lifetime responsibility. D-13 owns acceptance; preserve existing qualifications, actual-owner checks, nominal component identity, D-16 and D-18's existing qualification boundaries. Full structural/receiving proof and compiler migration remain separate; no C++ storage API changes follow.

- **Callable placement and transfer boundaries:** [H-563](../CHANGELOG.md#h-563)/[H-567–H-568](../CHANGELOG.md#h-567), [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/P-503–P-505 and [D-22-TRANSFER](DesignAuthorityRegistry.md#d-22-transfer)/P-506–P-508 cover D-10r197's placement/operation terminology. Callable-view values have lexical-scope storage, never regional storage; only pure bits admit whole-payload transfer. CoD has no C++ object copying/moving model. C-54/C-65/C-72/C-73/C-75 and charter §§2/3.2–3.3/4.3/6.3/8 preserve regional relocatability and explicit identity/lifetime responsibility. D-18/D-22 own acceptance; retain dedicated callable construction/capture, graph transfer, actual owners and D-16. [K-LEXICAL-PRODUCTS-20261117](../CHANGELOG.md#k-lexical-products-20261117) aligns dependencies. H-564–H-566's callable-bearing ADT/anonymous-data direction remains under review, including type identity/results; no new aggregate implementation or full proof/physical/value-path qualification follows.

- **Owner-parameter reference symbols:** [H-561](../CHANGELOG.md#h-561)/[K-OWNER-SYMBOL-ACCEPT-20261117](../CHANGELOG.md#k-owner-symbol-accept-20261117), [D-10-OWNER-SYMBOL](DesignAuthorityRegistry.md#d-10-owner-symbol)/P-497–P-499 covers r196's source form and staged sound acceptance. An installation obtains its one actual lexical or ordinary regional owner from the RHS; the region-selector form retains its extra owner constraint. Candidate C-36/C-54/C-65/C-93 and charter §§2/3.2–3.3/4.3/6.3/8 preserve reusable dynamic context with explicit storage/lifetime responsibility. Preserve owner/access/null/init, saved/captured validity, module symbol identity, no lifetime extension and D-16. Acceptance, cost evidence and exclusions are below. The source choice is confirmed; full proof/physical/value-path qualification and compiler support remain separate. Other qualified contracts continue.

- **Native-coverage reference continuity:** [K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) preserves this revision's D-26 dependency at its current AOT contract section. Only dependency links and this summary change; existing semantics, authority, acceptance and D-16 remain unchanged.

- **Callable-reference continuity:** [K-ALIAS-GLOSSARY-20261117](../CHANGELOG.md#k-alias-glossary-20261117) preserves this revision's existing D-18 dependency under the canonical CallableViewsAndCaptures.md path. Only dependency links and this continuity summary change; existing contracts, qualification, acceptance and D-16 remain unchanged.

- **Lexical lifetime.** D-18-HERE and D-12-LIFETIME require the actual supplying activation and actual owners through the final use. Only symbolic handler replay acquires a snapshot of required dynamic bindings at definition activation.

- **Full scopes.** D-17-FULL-SCOPE preserves invocation-local return and cleanup. Standalone and caller-default bodies use direct lexical access.

- **Checked deferred coverage:** [H-550/H-551](../CHANGELOG.md#h-550)/[K-DEFER-CHECKED-COVERAGE-20261116](../CHANGELOG.md#k-defer-checked-coverage-20261116), [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch)/P-461–P-463, requires checked failures to be handled before their owning defer pass completes. A covering defer catch counts; enclosing throws alone cannot authorize residual checked members. Preserve actual registration/FIFO order, H-525 declared obligations, retained ordinary body_exception, owners and D-16. D-17r86 owns acceptance and conservative proof boundaries. Candidate C-36/C-44/C-63/C-65 and charter §§2/3.1–3.3/4.3/6.3/8/12 serve explicit recoverability and valid ordered cleanup. This dependency alignment continues unrelated contracts; it does not qualify the whole mechanism/value path or assert compiler support.

- **Return-state dependency:** [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state)/[H-542–H-547](../CHANGELOG.md#h-542), [H-548](../CHANGELOG.md#h-548)/[K-RETURNING-PROFILE-20261116](../CHANGELOG.md#k-returning-profile-20261116), aligns this revision’s lexical binding/context under P-484. Candidate C-54/C-63/C-65 and charter §§2/3.2–3.3/4.3/6.3/8/12 preserve author-controlled completion, result/owner validity and observable cleanup. D-17/D-23 own acceptance: scoped cancellation, explicit-return markers, ordinary binding grammar/copy/borrow lifetimes and owning inspection without capture. Module cleanup has no callable return target. Preserve unrelated qualified contracts, FIFO failures and D-16. The H-548 agent selections do not add implementation support or full value-path/physical qualification; owning capture and future pure-callable rules are excluded.

- **FIFO cleanup dependency:** [H-541](../CHANGELOG.md#h-541)/[K-DEFER-FIFO-20261116](../CHANGELOG.md#k-defer-fifo-20261116) updates this revision’s module lifetime/context section under D-17-DEFER-DISPATCH/P-463. C-36/C-63/C-65 and charter §§2/3.3/4.3/6.3/12 require preserving observable FIFO handling, tail insertion, incoming-only visits and existing owner/context boundaries. D-17 owns acceptance and remaining policies; unchanged lookup, capture and native-coverage contracts continue. No new runtime support, whole-design qualification or D-16 change follows.

- **Disjoint declarations:** [D-10-DISJOINT](DesignAuthorityRegistry.md#d-10-disjoint), [H-527–H-535](../CHANGELOG.md#h-527) and [K-DISJOINT-ENDPOINTS-20261116](../CHANGELOG.md#k-disjoint-endpoints-20261116) cover this revision's constraint-related sections. Existing listed-endpoint ancestry permits inherited overlap; a second independent listed-parent relationship rejects, while prior redundant relationships warn about the path and intermediate methods. Preserve complete public records, per-pair definition/import authority, frozen validation, declared checkedness, demanded checking, owners/captures and D-16. Candidate C-56/C-10/P-476–P-478 serves explicit nominal intent and stable interpretation under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12. [D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) owns acceptance; finite generic/cyclic proof procedures, source implementation and full value-path qualification remain separate. No separate exemption-path store or runtime inheritance mutation.

- **Shared-information candidate:** [D-15-DISJOINT-POOL](DesignAuthorityRegistry.md#d-15-disjoint-pool)/P-479–P-481 under C-59/C-56/C-10 records [H-533's](../CHANGELOG.md#h-533) requested feasibility review. [D-15](ModuleWorlds.md#shared-disjoint-information-per-module-world) owns identity, upstream retention, applicable local/source checks, publication filtering and frozen-backing acceptance. Physical representation and the complete value path remain pending-human; no compiler support is inferred.


- **Checked and unchecked roots:** [H-524–H-526](../CHANGELOG.md#h-524)/[K-ERROR-ROOTS-20261116](../CHANGELOG.md#k-error-roots-20261116), [D-17-ERROR-ROOTS](DesignAuthorityRegistry.md#d-17-error-roots), adds Error <: Exception and preserves explicit Exception handling obligations even for values proven unchecked. Checkedness-changing adapter conformance rejects. Candidate P-473–P-475 serves explicit typed failure contracts and user-defined errors under charter §§2/3.1/3.3/4.3/6.3/8/12. Existing actual-type dispatch, concrete failure categories, owner/capture and D-16 contracts continue. New roots/declaration enforcement and arbitrary exception payload support are not established by current source tests; general disjointness and remaining wrapper/throws/inspection scope remain separate.

- **Wrapper-aware catch dependency:** [H-518–H-522](../CHANGELOG.md#h-518)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), [K-WRAPPER-CATCH-20261116](../CHANGELOG.md#k-wrapper-catch-20261116), records source-order wrapper-prefix matching and same-exception-identity selection of the hidden tip within the lexical catch context under candidate P-470–P-472/charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve ordinary payload type evidence, complete incoming exception history for selected exception operations, actual owners and D-16. [H-523](../CHANGELOG.md#h-523)/[K-RETHROW-FROM-20261116](../CHANGELOG.md#k-rethrow-from-20261116) requires standard-wrapper cause/checkedness agreement and adds automatic rethrow from cause, with a direct diagnostic suggestion on mismatch. Inspection and transparent checked coverage remain pending in D-17; no copied graph, general value ABI or new source support is inferred.

- **Caught-value construction defaults:** [H-510–H-517](../CHANGELOG.md#h-510)/[K-EXCEPTION-CONSTRUCTION-20261116](../CHANGELOG.md#k-exception-construction-20261116) continues this dependency in r184→185. [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve), candidate P-464–P-469 under charter §§2/3.2–3.3/4.3/6.3/8/12, separates unchanged throw propagation from constructor-time trace/cause creation, lexically supplied rethrow defaults and the bounded process OOM diagnostic window. Preserve semantic checkedness, module/callable contexts, captures, actual owners, independent deferred-queue obligations and D-16. D-17 owns acceptance and remaining representation/aggregate/ordering questions; no compiler support or whole-scope qualification follows.

- **Deferred-queue contexts:** [H-503/H-505–H-509](../CHANGELOG.md#h-503)/[D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch), [K-DEFER-STACK-20261116](../CHANGELOG.md#k-defer-stack-20261116), continues r183→184's context/owner scope under candidate P-461–P-463 and charter §§3.2–3.3/4.3/6.3/8. Multiple catches during one bodyless handler visit retain selected current/captured contexts, semantic error identities and actual owners; they do not recapture values or expose ambient module bindings. D-17 owns the queue direction, acceptance and remaining completion/order policies. Existing symbol/capture/ownership contracts and D-16 remain unchanged; no compiler-support claim follows. H-509 requires a single unchecked UnhandledDeferError retaining all residual deferred exceptions; H-510–H-515 supplies the current construction/rethrow contract; FIFO order follows H-541; H-550/H-551 requires checked coverage before pass completion; aggregate/body-field representation remains separately scoped.

- **Module cleanup continuity:** [H-501](../CHANGELOG.md#h-501)/[D-15-SHUTDOWN](DesignAuthorityRegistry.md#d-15-shutdown) orders module-level deferred actions by application-wide reverse registration, with retained module contexts and valid owners. [K-MODULE-SHUTDOWN-20261116](../CHANGELOG.md#k-module-shutdown-20261116) records r181→182 under candidate C-36/C-44/C-59/C-63/C-65/P-458–P-460 and charter §§2/3.3/4.3/6.3/12. Preserve ordinary captures/current bindings, actual owner transfer, scope restoration, nested cleanup and D-16; D-15/D-17 own acceptance and remaining cross-module completion/representation boundaries. H-502/[D-15-PUBLICATION](DesignAuthorityRegistry.md#d-15-publication) resolves binding visibility through whole-block scheduling, without a source-final-binding barrier; [K-BLOCK-BINDING-VISIBILITY-20261116](../CHANGELOG.md#k-block-binding-visibility-20261116) records r182→183. Existing selection/capture timing remains; no compiler support is claimed.

- **Module startup and entry selection:** [H-492](../CHANGELOG.md#h-492)/[D-15-CYCLES](DesignAuthorityRegistry.md#d-15-cycles) permits declaration-only cycles. H-493–H-498/[D-15-POSTPON](DesignAuthorityRegistry.md#d-15-postpon)/[D-15-STATIC-NS](DesignAuthorityRegistry.md#d-15-static-ns) requires static runtime namespace definitions first, one statically emitted sequence of implicit dependency prefixes and ordered explicit postpon blocks, then the entry executable prefix. H-499/H-500/[D-15-ENTRY](DesignAuthorityRegistry.md#d-15-entry) selects the namespace containing reserved `__main__`, allowing multiple entry namespaces per dependency package and prohibiting entry reuse by other modules. Candidate C-10/C-35/C-36/C-44/C-45/C-59/C-65/P-443–P-454 serves charter §§2/3.1–3.3/4.2/6.3/8/12 through explicit identities, contexts, readiness and valid native execution. Preserve publication, world isolation, owners/captures, ordinary failure cleanup, D-16 and once-only incorporation. [D-15](ModuleWorlds.md#module-dependency-cycles-and-initialization) owns acceptance and remaining proof/representation/CLI boundaries; [K-STATIC-MODULE-STARTUP-20261116](../CHANGELOG.md#k-static-module-startup-20261116) integrates this revision. No runtime scheduler, compiler-support claim or whole-contract qualification follows.

- **Source fallback and type-carried names:** [H-483](../CHANGELOG.md#h-483)/[H-484](../CHANGELOG.md#h-484) governs r179's selection/alias paragraphs: explicit/local bindings shadow lookup through a selected source's published namespace; symbol from uses that fallback without changing symbol/type identity. [H-485](../CHANGELOG.md#h-485) reserves later information diagnostics at implicit-use sites. [D-15-TYPE-METHODS](DesignAuthorityRegistry.md#d-15-type-methods)/[D-15-LOOKUP](DesignAuthorityRegistry.md#d-15-lookup)/[D-15-LOOKUP-INFO](DesignAuthorityRegistry.md#d-15-lookup-info), C-35/C-45/C-57/C-59/P-425–P-442, supplies candidate paths to charter §§3.1/3.2/4.2/6.3/8/9 through usable definitions, explicit context and inspectable provenance. D-15 owns acceptance; [K-SOURCE-FALLBACK-20261116](../CHANGELOG.md#k-source-fallback-20261116) integrates fallback; [K-REEXPORT-METHODS-20261116](../CHANGELOG.md#k-reexport-methods-20261116) integrates r178→179 under H-488–H-491 with [D-15-METHODS-CLAUSE](DesignAuthorityRegistry.md#d-15-methods-clause)/[D-15-REEXPORT](DesignAuthorityRegistry.md#d-15-reexport). Preserve privacy, publication, explicit extension, helper identity, dispatch and H-481/H-482 capture/owner rules. H-486/H-487/[D-15-LOOKUP-TRANSITIVE](DesignAuthorityRegistry.md#d-15-lookup-transitive) adds multilevel fallback and intermediate private-shadow errors. General proximity algorithms, full qualification and implementation remain separate.

- **Explicit regional reference symbols:** [D-10-REF-SYMBOL](DesignAuthorityRegistry.md#d-10-ref-symbol), r175's symbolic-storage section, records [H-482](../CHANGELOG.md#h-482)/[K-REFERENCE-SYMBOLS-20261116](../CHANGELOG.md#k-reference-symbols-20261116). Explicit ref/ptr symbols may borrow scalar or object targets from the actual region selected at effect installation through a declared region symbol. Later shadowing, capture and restoration retain that original association while its owner effect lives. P-422's candidate C-36/C-54/C-65/C-93 path serves charter §§3.2/3.3/4.3/6.3/8; P-423/P-424 records dependencies and downstream constraints. Preserve separate shared-owner groups, null/access/init, function exclusion, scoped restoration and cleanup; no ownership extraction/extension, arbitrary lexical-owner backing, val-symbol syntax or ABI. Acceptance and exclusions are below. Source behavior is confirmed; full value-path/physical qualification and compiler support remain separate.

- **Definition mentions:** [H-481](../CHANGELOG.md#h-481)/[K-CALLABLE-MENTIONS-20261116](../CHANGELOG.md#k-callable-mentions-20261116) continues r173→174's existing symbol/value distinction and integrates [D-18-MENTION](DesignAuthorityRegistry.md#d-18-mention), D-18r105/P-419–P-421. Definition-bound callable names denote definitions; a required runtime view captures at materialization, including recursive mentions. Stored values preserve their captures. The candidate C-65/C-73/C-75 path serves charter §§3.2/3.3/4.3/6.3/8 through explicit identity, context and storage; acceptance is in D-18. Preserve perform/current invocation, lexical lookup, owner/access/init, function exclusion, type-only queries and default origins. H-481 confirms this bounded source rule; complete value-path/physical qualification and recursive source implementation remain separate. No new source-level executing-callable value, capture permission or mandatory allocation.

- **Nominal ADT placement:** [H-478](../CHANGELOG.md#h-478)/[K-SINGLE-ADT-20261115](../CHANGELOG.md#k-single-adt-20261115) covers r172→173's alignment with [D-28-ADT](DesignAuthorityRegistry.md#d-28-adt), P-416–P-418. One CoD ADT retains nominal identity across lexical/regional placement; actual owners provide layout/access evidence. The candidate C-10/C-39/C-65/C-91 path serves charter §§3.1/4.3/6.3/7.1/8 through shared business types and checked storage. Preserve same-owner edges, bits classification, initialization, lifetimes and explicit graph transfer; no C++ regional-type stack allocation, capture/TLS extension, graph copy or runtime support is implied. Acceptance and unresolved physical protocols belong to D-28's composed cases. This scoped human decision and direct integration do not qualify the whole document or settle H-146's encoding mechanism.

- **Current scope:** [K-722](../CHANGELOG.md#k-722) covers D-10r172, preserving confirmed explicit call choices and post-selection default analysis, qualified compiler contracts and pending boundaries below. Current source coverage includes canonical failure names and typed root catches; default arguments and predicate/type assertions retain their separate implementation limits.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-10r167→168 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Named arguments and defaults:** [D-10-ARGUMENTS](DesignAuthorityRegistry.md#d-10-arguments), H-451/H-452, covers complete declaration-directed binding and a callable initialization preamble. [H-453–H-457](../CHANGELOG.md#h-453)/[K-ARGUMENT-CONTEXTS-20261115](../CHANGELOG.md#k-argument-contexts-20261115) selects expression-first arrow labels with grouped owner projection, default definition/caller environments, per-omitted-call evaluation, and definition-resolved symbolic identities with caller binding lookup. Candidate P-405 through C-10/C-57 serves charter §§3.1/3.3/6.3/8/12; P-406 preserves parameter, scope, effect, exception and native-coverage dependencies. [H-458](../CHANGELOG.md#h-458)/[K-DEFAULT-PHASE-20261115](../CHANGELOG.md#k-default-phase-20261115) selects caller-default preparation before entry and definition-default initialization in the callee preamble. [H-459](../CHANGELOG.md#h-459)/[K-NAMED-DEFAULT-ORIGIN-20261115](../CHANGELOG.md#k-named-default-origin-20261115) restricts default authorship to named callable declarations and permits type-position typeof to retain their originating contract. [H-460](../CHANGELOG.md#h-460)/[K-COMPATIBLE-DEFAULT-TARGETS-20261115](../CHANGELOG.md#k-compatible-default-targets-20261115) admits compatible targets under that inherited contract. [H-461](../CHANGELOG.md#h-461)/[K-TYPE-DEFAULT-EXPRESSION-20261115](../CHANGELOG.md#k-type-default-expression-20261115) places the default expression and definition-context association in the callable type contract, not a new capture in the adapted value. [H-462](../CHANGELOG.md#h-462)/[K-CALLABLE-FAMILY-CONTRACT-20261115](../CHANGELOG.md#k-callable-family-contract-20261115), P-409, requires every method signature in a multi-method typeof origin to be implemented and permits caller-local named adapters. [H-474](../CHANGELOG.md#h-474)/[K-DEFAULT-RANKING-20261115](../CHANGELOG.md#k-default-ranking-20261115) selects ordinary type matching first, then fewer used defaults on a tie, with remaining ambiguity rejected. [H-475](../CHANGELOG.md#h-475)/[K-EXPLICIT-DEFAULT-SELECTION-20261115](../CHANGELOG.md#k-explicit-default-selection-20261115) rejects crossed fixed/default preferences and adds an explicit default request `-> parameter` and contextual empty-pack selection `[]...`. [H-476](../CHANGELOG.md#h-476)/[K-DEFAULT-TYPE-EVIDENCE-20261115](../CHANGELOG.md#k-default-type-evidence-20261115) permits established types of used defaults to supply generic inference evidence without evaluation; [H-477](../CHANGELOG.md#h-477)/[K-POST-SELECTION-DEFAULTS-20261115](../CHANGELOG.md#k-post-selection-defaults-20261115) places that analysis after overload selection. Defaults do not influence candidate selection through their contents, and analysis failure never retries another candidate; circular or unresolved inference rejects. Physical context evidence, broader type equality/erasure and full qualification remain under review. H-449/H-450 retain their unopposed cases under H-475; no source implementation is claimed.

- **Expression typing and predicate assertions:** [D-10-TYPED-EXPR](DesignAuthorityRegistry.md#d-10-typed-expr) and [D-10-ASSERT](DesignAuthorityRegistry.md#d-10-assert), H-453/H-466–H-471, cover the sections below. [K-ASSERT-SPECIALIZATION-20261115](../CHANGELOG.md#k-assert-specialization-20261115), P-407–P-413, integrates concrete-type operation selection, static resolution and explicit assert-based non-null refinement. Candidate C-10/C-65 paths serve charter §§3.1/3.3/4.3/6.3/8 through readable intent, checked flow facts and preserved effects. Pure numeric value conversion retains warnings/UB; other admitted assertions resolve statically or check at runtime with unchecked failure. Owner/access/init, callable contracts, cleanup and D-26 prepared coverage remain. Acceptance and remaining grammar/evidence boundaries are below; local decisions do not qualify the whole design or claim source implementation. Julia remains comparative evidence only.

- **Variadic call direction:** [D-10-VARARGS](DesignAuthorityRegistry.md#d-10-varargs), [H-449](../CHANGELOG.md#h-449)/[K-VARIADIC-PACK-20261115](../CHANGELOG.md#k-variadic-pack-20261115), covers the runtime-array expansion and single logical pack parameter section below. H-258 supplies typed variadic declarations; H-449 selects ordinary-array expansion and treats the variadic tail as one overload input whose length is runtime data. [H-450](../CHANGELOG.md#h-450)/[K-VARIADIC-RANKING-20261115](../CHANGELOG.md#k-variadic-ranking-20261115) selects ordinary type matching first, with a fixed-over-implicit-pack preference only for otherwise equal matches. Candidate value path P-403 through C-57/C-89 serves charter §§3.1/6.3/8/12 by keeping typed call structure inspectable and independent of runtime count; this is a local semantic decision, not whole-file/value-path qualification. P-404 retains owner/access/initialization, exception, numeric and native-coverage dependencies. H-475 rejects an otherwise tied call when fixed-form and fewer-defaults preferences conflict; explicit empty expansion selects pack form with established element/owner evidence. Acceptance and unresolved overload/representation boundaries are stated in that section; no compiler support is claimed.

- **Builtin bounds policy:** [H-448](../CHANGELOG.md#h-448)/[K-BOUNDS-POLICY-20261115](../CHANGELOG.md#k-bounds-policy-20261115) continues r151→152 with [D-17-BOUNDS](ExceptionControl.md#unchecked-bounds-exceptions-and-optional-checks), C-63/C-65/P-401–P-402 and charter §§4.3/6.3/8/12. Builtin indexing raises unchecked IndexOutOfBoundsError with runtime checks on by default; explicit opt-out affects bounds only. Evaluation, owner/access, initialization and checked NPE remain unchanged. Acceptance and unsafe-range consequences belong to D-17; broader executable pack contracts and implementation remain separate, and whole-file qualification is unchanged.

- **Registry / current scope:** [D-10](DesignAuthorityRegistry.md#d-10); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole file/value paths pending-human; recorded local decisions remain confirmed.** C-35–C-46/P-84–P-111 and later operation/owner/publication edges; P-248–P-254/P-330–P-343 governs current effects/families. K-572 bounds latest prototype evidence.
- **Value path:** C-10's inspectable typed execution motivates C-35/C-43/C-45/C-46 identity/composition through P-84/P-100/P-102/P-109–P-111; C-56/C-57/P-182–P-197 covers abstractions, dispatch and adapter visibility. C-39/C-54/P-94/P-143/P-155 retains regional lifetime boundaries; C-58/P-199–P-203 covers publication and selected contexts. These candidate paths serve charter §§2–4 without borrowing authority from technical dependencies.
- **Contract boundary:** Preserve identities, selected contexts, perform/current-binding distinction, owners and function effect exclusion. D-14 owns storage, D-15 worlds, D-17 completion, D-18 callable values, D-21 families. Cyclic initialization, persistent ABI, broader captures/results/contexts and family mechanisms remain open; old passing reports do not qualify them.
- **Invariants / non-goals:** preserve H-478's owner-qualified one-ADT placement, zero-cost regional relocation, no regional copy/move, same-owner stored edges, preserved hidden/captured owners and no dynamic access or procedure calls from functions. No module namespace, runtime interface/borrow registry, immutable snapshot, allocator/toolchain change or SNN model is selected. Compound views, higher-order overload values, full qualifiers/ownership construction/transfer and mailbox remain bounded or pending.
- **Acceptance:** Validate lexical and captured/current calls, explicit-memory versus dynamic effects, scope restoration, owner preservation and cleanup across normal and exceptional completion. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-NULL-OWNER-20261114](../CHANGELOG.md#k-null-owner-20261114) covers r146→147's ordinary-reference null-owner syntax alignment under H-418/H-419. Symbolic installation, current/captured context and independent qualified behavior retain their scope. [K-REFERENCE-IDENTITY-20261114](../CHANGELOG.md#k-reference-identity-20261114) integrates H-439/H-440 owner-plus-locator identity and is null, preserving established access, result and lifetime behavior. Current source/native coverage and proof boundaries are recorded in the [source capability reference](../tests/cod/language/README.md). [K-UNARY-NOT-20261114](../CHANGELOG.md#k-unary-not-20261114) integrates H-442 not/! as one unary operation with identical precedence, typing, evaluation and method selection; is not retains identity-negation parsing. Current bounded implementation coverage is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Current implementation alignment:** [K-702](../CHANGELOG.md#k-702) continues the confirmed contract and points to the [source capability reference](../tests/cod/language/README.md) for stable lexical identity/null observation and unary spelling support. Broader storage, key, allocation and native-interface obligations remain separate.

## Ordinary storage and checked nullable access

H-309–H-315 distinguishes direct val bits, initialization-checked ref targets and nullable ptr targets. [D-14](RegionalOwnership.md#ref-and-ptr-declaration-direction) owns field storage, local lifetimes and H-417's unique static owner requirement and H-418/H-419's declaration-side or literal-side null owner, with here/named-owner qualification and no outer syntax. [D-17](ExceptionControl.md#checked-nullpointerexception) owns checked NPE: local handling or immediate func/proc throws, with mandatory handling at module top level. Symbolic procedure signatures retain this contract; neither perform captures nor behave's current bindings erase it. H-440 uses p is null for the null test; null remains reserved, and the final comparison does not dereference its pointer operand. H-439 selects the fixed is identity operation through [D-28 identity contract](RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator). Syntax/checker migration is not claimed by existing native evidence.

These ordinary storage declarations do not replace the flat typed TLS slots. Effect-only writes, capture, shadow restoration, selected actual owners and non-extractable resources retain their contracts. Val parameters copy supported bits into callee-local storage, while external references retain explicit owners. New source acceptance is recorded with the current validation; earlier native evidence retains its source/compiler scope.

## Symbolic storage and regional provenance

Dynamic symbolic variables have their own storage and rebinding contract (H-277–H-281/H-482). Binding mode is independent of whether the target type is bits:

| Binding category | Storage and writes |
| --- | --- |
| Plain bits-valued symbol, such as `symbol @x:u64` | Its bits payload is allocated directly in TLS. `effect @x <- rhs` transfers bits rather than rebinding a pointer to the RHS's lexical storage. The payload does not borrow that storage. |
| Existing non-bits object-reference symbol | It references an object inside a region supplied by a region symbol. `effect @x = rhs` may select another such region, subject to the declared type and actual region lifetime. No object is copied into TLS. |
| Explicit `ref/ptr symbol @x:T in @arena` | A borrowed target in the region selected through @arena at installation; T may be scalar/bits or an admissible object type. `effect @x = rhs` installs the reference and actual-owner association. |
| Explicit `ref/ptr symbol @x:T in S where {owner S}` | A borrowed target whose one actual owner is established from the RHS; lexical and ordinary regional owners are admitted under H-561 without installing a region symbol. |
| Region symbol | It provides the whole-region resource under the existing ownership contract. Shadowing preserves hidden owners; extraction remains forbidden. |
| Procedure-valued symbol | It selects/replays lexical callable-view values under the separate scoped capture contract. H-563 forbids regional view storage; TLS binding bookkeeping cannot retain an expired lexical/captured owner or authorize a regional carrier. |

Symbolic binding writes require `effect`: `effect @x = rhs` performs rebinding, while `effect @x <- rhs` performs ownership/bits transfer (H-280). Bare `@x = rhs` and bare `@x <- rhs` cannot write a symbolic binding. The proposed object-content transfer form is distinguished below. Both forms have scoped validity: an effect saves the preceding binding and restores it when its scope ends. For bits, the saved state is the preceding bits value; for an object reference, it is the preceding reference and its provenance. Edits to referenced regional contents are not undone by restoration. Every effect expression has type `void` (H-288). A following explicit symbolic read obtains the current binding; an effect cannot supply a value argument or form an assignment chain.

The object-reference rebinding form does not take ownership away from the supporting region symbol. A fresh whole-region resource still uses consuming transfer under the existing no-extraction rule. H-281 requires `<-` for independent TLS bits payloads and `=` for reference rebinding. H-482 applies the latter to explicit ref/ptr symbols even when their target type is bits. `effect @bits = rhs` is rejected; it cannot introduce a pointer to RHS lexical storage. An object-reference binding is not a bits-valued symbol merely because its native address occupies bits. Whole-region borrowing/replay and regional graph transfer retain separate interfaces. Neither syntax admits an implicit graph copy or extraction from an existing region symbol.

The same symbolic identity may select different permitted targets and actual owners over time. This is an explicit exception to initialization-fixed owners for ordinary `ptr` variables. It changes a TLS reference, not a pointer field stored in either regional graph; same-owner stored-edge rules and zero-cost relocation remain intact. The region-selector and existing shared-region forms require their declared regional resource. H-561's explicit owner-parameter form instead admits a proved ordinary regional or lexical owner. Neither form implicitly installs an owning resource or transfers its ownership.

The compiler must preserve the reference's actual region association through branches, capture and restoration, and reject a use that can outlive its supporting resource. Equal root types or equal symbol spellings do not prove equal region instances. Rebinding or shadowing a region symbol does not retarget existing references: a hidden owner remains alive under H-180, and an old reference still denotes its original region. TLS thread lifetime cannot extend that region's lifetime.

Procedure capture retains bits values or the selected regional reference with its lifetime dependency. Perform replays those captured values; behave uses current bindings. Capturing an object reference does not own the region or copy its graph. Detailed capture/escape lowering must prove this lifetime condition; the earlier proposal to exclude every interior reference is not an accepted restriction on all symbolic bindings. Functions still cannot read or write symbolic bindings.

Bounded acceptance must demonstrate independent TLS bits payloads, bits/reference restoration, one object symbol rebound between two live symbol-backed regions, and a captured reference retaining its original region. H-561 additionally admits proved ordinary-owner backing through the owner-parameter form; references that escape their actual owner lifetime must reject, including through branches/captures. No runtime borrow registry, RTTI, extra per-object ownership or fraction representation is selected. Fixed-width fraction bits, when implemented, retain their declared representation; this rule chooses no new promotion or alignment policy.

### Explicit ref and ptr symbols backed by a region symbol

H-482 selects these declaration forms; they are design syntax, not current parser/runtime support:

```cod
region symbol @arena:State;
ref symbol @bias:u64 in @arena;
ptr symbol @optional:u64 in @arena;
```

The owner-symbol identity is resolved in the declaration's lexical context. Its currently installed region supplies the actual owner for a new reference effect. A declaration supplies no effect value, implicit region installation or default null. Plain `symbol @bias:u64` remains independent TLS bits storage; explicit val-symbol syntax is not selected.

`effect @bias = rhs` requires an initialized non-null target with compatible type/access and the actual owner selected through @arena for that installation. `effect @optional = rhs` additionally admits null. `effect @optional = null` obtains owner context from the declaration and the installed region; a qualified null must agree with it. Null remains associated with that owner and does not waive resource availability/lifetime requirements. A non-null proof can admit a ptr source for ref under ordinary rules; nullable dereference and proof invalidation retain their existing behavior.

Installing a reference is distinct from writing its target. `effect @bias <- 7u64` does not initialize a reference effect with independent TLS bits, and bare `@bias = rhs` cannot rebind TLS. Through a writable installed reference, `@bias <- 7u64` follows ordinary admitted target-transfer rules; such a regional write is not undone by effect restoration. Scalar reads/copies retain the ordinary value-receiving rules. Readonly access cannot be upgraded by rebinding, capture, replay or the region symbol's own permissions.

The selected association is **per installed effect**, not a requirement to agree continuously with the visible @arena binding. Later shadowing can hide R1 with R2 without retargeting a reference already installed into R1. The original owning effect, its actual owner identity and the selected target remain the basis of access. Removing the shadow restores preceding bindings with their original owners; it never interprets an old locator relative to a newly visible region.

```cod
# Design sketch: State has a bits field bias.
effect @arena <- make region State(256u64 -> capacity) { bias <- 10u64; };
effect @bias = @arena.bias;       # Borrow R1's scalar target.
{{
    effect @arena <- make region State(256u64 -> capacity) { bias <- 20u64; };
    val old <- @bias;                  # 10: R1 is hidden but alive.
    effect @bias = @arena.bias;   # New reference effect selects R2.
    val current <- @bias;              # 20.
}};
val restored <- @bias;                 # 10: preceding R1 binding restored.
```

Effect evaluation retains the existing preparation/publication boundary: evaluate RHS expressions once, left-to-right; establish each new installation's type/access, actual-owner and lifetime requirements; then publish the group together. Resolve the declared owner symbol's actual region in the effective context at this installation boundary. A pending group member is not visible to another RHS. Explicit RHS effects/mutations keep their ordinary semantics; failure installs none of the pending reference bindings and promises no allocation or mutation rollback. This rule adds no syntax or simultaneous-publication contract for a mixed owning-region/reference group; install the region before installing its interior references.

Capturing @bias retains its selected target and actual-owner association, rather than copying the scalar payload or retaining a mutable TLS slot. A later write to that regional scalar remains observable through the captured reference. Rebinding @bias or shadowing @arena does not change the saved reference. Capturing this reference alone does not implicitly capture/replay @arena: owner-resolution evidence and lifetime dependencies are not a replacement symbolic binding. perform may replay an R1 reference while the caller's @arena denotes R2, provided R1's owning effect is still live and all separately declared group constraints hold. behave uses the current reference binding. H-481's fresh-definition-view versus stored-value-copy rule remains unchanged.

Two independent ref/ptr symbol declarations naming @arena can select different actual owners at different installations. The annotation alone does not impose equality between all their effective bindings. H-284–H-286's explicit shared-owner relation remains a separate constraint: incompatible partial rebinding/replay rejects, and touch explicitly adds companion capture dependencies. These declarations neither weaken those groups nor silently create new ones.

The compiler must prove validity through active and saved restoration bindings, branches, calls, aliases, captured/returned callable values and deferred work. A hidden original owner remains valid until its owning effect ends; a reference, capture or restored binding cannot keep it alive afterward. Missing or unproved owner evidence rejects. Reading a reference while its owner symbol is shadowed needs the retained actual owner, not a fresh lookup. A body that installs a new reference effect therefore has a dependency on the declared owner-symbol binding even when that selector appears only in the declaration; reading an existing reference needs only its retained actual-owner evidence. New installation still requires the declared region-symbol resource; an ordinary lexical owner or merely live local region is insufficient. Shared runtime owner information may serve multiple references without redundant per-reference owner records, provided each target is resolved using its own actual owner and locator.

Acceptance covers regional scalar and object targets, unchanged plain TLS scalar copies, ref/ptr nullability, same-owner installation, R1/R2 shadows and restoration, copied reference captures observing target mutation, replay with a hidden live owner, independent selectors versus shared-owner groups, readonly rejection, RHS failure and escaped-lifetime rejection. No refcount, borrow registry, permanent pointer encoding or complete compiler implementation is selected. The owner-parameter section separately admits ordinary lexical owners; they cannot satisfy this form's region-selector constraint by themselves.

### Lexical borrows and symbolic declaration boundaries

H-555 permits an explicit `in O` lexical-owner borrow when it cannot outlive O, including through a callable view. This is independent of the regional symbol form above: a live lexical owner does not implicitly satisfy an `in @arena` declaration or create a regional resource. D-18 owns ordinary references and explicit place-alias captures; all transitive views and deferred uses must retain their actual owner dependencies. No implicit slot sharing, TLS-cell capture or owner extension follows.

Module symbol identities remain static; H-113's exclusion of lexical symbolic variable declarations is unchanged. The owner-parameter form below supplies a declaration-level owner binder, with installation-specific actual-owner evidence, rather than creating a lexical symbol identity. H-561 confirms this extension and staged acceptance; detailed proof algorithms and native representation remain separate. The compiler must reject unproved installations/restorations rather than fabricate a module-visible owner, silently promote a local slot or restore an expired binding. H-482's region-selector form keeps its installation-time owner constraint.

### Owner-parameter reference symbols and implementation cost

H-561 confirms the owner-parameter source form and staged sound acceptance after H-560's cost review. A first implementation may accept common proved cases and reject unproved ones explicitly. The comparison below identifies shared work and incremental cost; it is not a claim that either reference-symbol form is implemented or that its complete proof/physical value path is qualified.

```cod
ref symbol @current:u64 in S where {owner S};

proc bump() {
    @current <- @current + 1u64;
}

proc increment(ref n:u64 in O) where {owner O} {
    effect @current = n;
    bump();
}

proc run() -> u64 {
    val n <- 0u64;
    increment(n);
    return n;  # 1; increment's effect was restored before it returned.
}
```

The symbol identity is module-declared. S is a declaration-scoped owner parameter, instantiated from the single proved actual owner of the RHS at each installation. It is not a global owner, a newly allocated region, a new symbol or an ambient owner value visible in every procedure. A lexical target and an ordinarily held regional target can both supply that owner. No owning-region symbol needs to be installed first. The existing `in @arena` form adds an installation-time owner-selector constraint; the owner-parameter form omits that constraint, without weakening reference validity.

An installation borrows the selected target; it does not retain n's reference-binding slot. Target writes remain visible to other references to that target: runtime target aliasing. Explicit `var a -> slot` retains its separate declared place-alias meaning, and symbol selection retains name/identity aliasing. Symbol identity, target identity, actual owner and current physical address remain distinct.

The required invariants are the existing ones, applied to the selected actual owner:

- Each installation has one statically established owner association. Different calls or nested shadows may instantiate S differently; that does not authorize a single reference with an unresolved set of possible owners. Separate declarations spelling S do not acquire an owner-equality constraint. Recursive activations remain distinct unless their actual owners are proved equal; sharing resolution information by the spelling S would be incorrect.
- Active bindings, hidden saved bindings, captured views and deferred uses must all stay within that owner's logical lifetime. Scope restoration restores the original target/owner association. Neither temporary shadowing nor copying a view erases the dependency.
- Dereference uses the selected actual owner and locator, including the current lexical backing base when that backing can grow. Stack/register allocation, stable-address optimization and shared owner-resolution information remain available under D-28; no pointer ABI or mandatory per-reference owner allocation is selected.
- Ref/ptr nullability, initialized-target requirements and access permissions remain unchanged. The corresponding `ptr symbol @optional:u64 in S where {owner S};` can receive `null in here` or a suitably named owner. Bare null supplies no evidence for a fresh unconstrained S; it must not acquire an owner from a previous installation or a later first non-null assignment. An independently established contextual owner may still supply ordinary null typing.
- Type-only inspection may use the declared reference/owner contract without reading a TLS binding; declaration alone supplies no installed actual owner. An owner binder does not grant owning access, a region root or lifetime extension. Ordinary owner-parameter permission inference still applies when a selected reference is passed to another callable. Functions remain unable to access dynamic bindings.
- RHS preparation, failure, publication and restoration follow H-482. Saved owner information is restored on every completion path, before its own dependencies end; proof cannot repair an invalid program by reordering observable user cleanup. Target mutations and allocations performed by an RHS are not rolled back.

For example, nested full scopes may temporarily bind @current to their own local m, then restore an outer n binding before m's owner ends. The same procedure body can serve either binding without making their owners identical. In contrast, returning a callable view that retains the current invocation's local target must reject:

```cod
proc read_current() -> u64 { return @current; }

proc leak() -> proc()->u64 {
    val n <- 0u64;
    effect @current = n;
    return read_current;  # Reject: this fresh view retains a borrow of n.
}
```

A helper returning a view that borrows its caller's still-live owner is not intrinsically an escape error. Acceptance depends on the actual owner's lifetime and all transitive uses, as in D-18; an implementation may conservatively reject an unproved case. A result contract that cannot express/preserve the borrowed owner cannot erase it into an unconstrained reference result.

#### Cost comparison and source evidence

The comparison is against **complete H-482 reference symbols**, including hidden owners and captured replay, not just the existing bits-valued TLS implementation. The latter cannot already implement either reference-symbol form.

| Work | Required by the region-symbol-backed baseline | Increment for the owner-parameter form |
| --- | --- | --- |
| Declaration and typing | Explicit ref/ptr mode, target/access/null contract and owner-selector identity. | Parse an owner binder without a required region-root type; obtain its actual owner from RHS evidence. Small, localized addition. |
| Effect checking and summaries | Track selected targets and actual owners through installation, demanded calls, saved bindings and restoration. Current symbol-presence facts alone are insufficient. | Admit ordinary regional and lexical owner sources, substitute them through the same summaries and check their validity. Moderate extension; no new analysis category is apparent. |
| Native transport | Carry enough target/owner-resolution information through TLS, saved bindings and captured contexts; a stale raw address cannot stand in for the H-482 owner contract. | Use the already required generic owner resolver for lexical as well as regional storage. No separate ownership manager is justified. |
| Captures and lifetime proof | Retain the original owner under owner-symbol shadowing, copying and replay; reject use after its owning effect ends. | Lexical scope exit and ordinary regional ownership consumption become additional invalidation boundaries. This shares D-18/D-28's ordinary borrowed-view work. |
| Precision and validation | Check branches, calls, cleanup, hidden restoration and independent/grouped owner relationships. | Add lexical nesting, recursive activations, growth and non-symbol-backed region cases. The test matrix grows; no constant-factor or schedule estimate is established. |

The current compiler separates proof owner identities from executable owner handles (`EvidenceOwner` in [lang_checked.hh](../cod/lang_checked.hh)), and `CheckedFrame::owners` already transports one owner identity per input-bound owner rather than duplicating it for every target. These are reusable foundations, not proof that reference symbols are implemented. `CheckedSymbol` and the dynamic-symbol checker currently cover supported bits/complete-callable payloads and binding availability; `register_module` explicitly rejects `symbol_owner` contracts. [lang_syntax.hh](../cod/lang_syntax.hh)'s `SymbolOwner` requires a region root and its parser recognizes the grouped region form, so merely lifting a rejection is insufficient.

[lang_llvm.hh](../cod/lang_llvm.hh)'s native-call owner transport, saved `perform` bindings and callable-image construction supply relevant integration points. Their current scalar/image paths need checked target/owner provenance before they can transport arbitrary borrowed symbols. No cost conclusion here assumes that copying an image header copies ownership correctly, or that an address remains valid after lexical backing growth. Compiler implementation evidence is read-only inspection at `44bc1765be05f255e576cccce0f5ec4a2209546e`; this is a qualitative architectural assessment, not measured implementation effort or a native acceptance report.

The [mainline developer](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6) independently reviewed these paths and reached the same bounded conclusion: no inherent source-semantic blocker or separate backend is apparent; the extra form can reuse the owner-bearing installation architecture required by H-482. That review also identifies the current native `owner_identity` gap for an external owner without input-bound actual context. A reference symbol must provide an installed/captured actual-owner source; merely removing that diagnostic would not implement the contract. This consultation is engineering evidence, not human design confirmation or implementation approval.

The difficult total work is therefore the outstanding reference-symbol and borrowed-view support. Requiring a region symbol does not remove its hard hidden-owner, replay and cleanup obligations. Adding an owner parameter is not cost-free, but no separate closure, garbage collector, runtime borrow registry, retained lexical frame or graph-copy subsystem is needed. Restricting ordinary source usage to region symbols would save some cases, not eliminate the core machinery.

#### Bounded implementation profile and remaining review

A first implementation should accept direct lexical and ordinary regional targets, synchronous procedure calls, nested shadow/restoration, ordinary target mutation and normal/exceptional exits whenever existing owner evidence proves them. Keep the same language model for captured replay; implement that acceptance when the corresponding D-18 owner-dependency proof and native carrier are available. Missing precision should produce a specific unsupported/unproved-owner diagnostic, not silently capture an owning region, extend a lifetime or reinterpret a reference as a copied scalar.

Conservatively reject unresolved owner joins, unknown transitive escapes, invalid restoration, consuming/replacing a region while an active/saved/captured binding still requires its owner, cross-context transport without a lifetime contract and reference results whose owner cannot be retained in their contract. Generalize the existing finite summaries only where needed for accepted programs. This is staged compiler coverage, not a permanent language ban on every returned view or an excuse to accept a case without proof.

The confirmed single-symbol scope does not choose combined multi-symbol generic-owner grammar, new source annotations for higher-order escape summaries, a runtime carrier/ABI, thread migration or persistent captured lexical storage. Existing H-284–H-286 shared-region groups remain separate; spelling one binder in independent declarations cannot silently create such a group. The source form and staged acceptance are confirmed by H-561; exact proof algorithms, physical transport and the complete value path remain pending-human. D-16 fraction payloads retain fixed bits representation and validity invariants; the proposal changes neither numerical promotion nor regional graph transfer.

### Shared-region symbol declarations

H-284/H-287 extends owner annotations to a set of symbolic object references:

```text
symbol @o1:T1 in r, @o2:T2 in r
  where {region r:RT};

effect @o1 = xxx, @o2 = yyy;
```

The shared r constrains the referenced objects to one actual region instance whose root satisfies RT. It does not merely require equal root types, nor require T1 and T2 to be equal. A concrete root constraint and an abstract root-policy constraint follow the same type identity, explicit visible conformance and concrete-dispatch rules as region owner parameters. Regional lifetime, access permission and region-symbol backing still apply independently. Inferring r does not install a new region resource or transfer ownership.

This existing form concerns non-bits object-reference symbols. H-482 separately admits explicitly reference-marked scalar symbols; the exact combined spelling with this group binder remains outside that decision. Independent bits-valued symbolic payloads stay in TLS, and a declaration does not itself bind either symbol. The source examples retain the explicit @ marker; dropping it from effect targets is not inferred from the abbreviated discussion example.

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

Acceptance covers one overload-backed update, persistent content after the enclosing effect binding restores, destination stability when the RHS shadows the same symbol, actual-owner access under captured replay, and rejection of effect-prefixed object transfer, missing overload, const destination or invalid lifetime. Fixed-width fractions remain bits and use the bits-transfer rule; no fraction layout or promotion choice follows.

## Callable effects

### Definition-installed effectful procedures

The restricted design admits a handler through a definition such as `effect @quote() -> u64 { ... };`. Source calls select its currently installed binding with `perform @quote()` or `behave @quote()`. Assigning a procedure into the slot, extracting the installed handler as a value, or adapting, returning or passing it through a callable type rejects. This applies even to a handler whose body returns a constant. A type-only use of a declared signature and compile-time imports of a symbol identity or its methods request no runtime handler. Independent bits, ref/ptr and owned-region effect bindings keep their respective installation rules.

```cod
symbol @rate:u64;
symbol @quote:proc()->u64;
effect @rate <- 10u64;
effect @quote() -> u64 { return @rate; };
effect @rate <- 20u64;
val replayed <- perform @quote();   # 10
val current <- behave @quote();     # 20
# proc saved = @quote;             # Reject extraction.
# effect @other = @quote;          # Reject assignment installation.
```

The definition activates with the required symbolic bindings, then publishes its scoped installation. Snapshotting occurs once at that activation. `perform` uses the selected handler's snapshot; `behave` uses current bindings. An inner handler can name another symbol and invoke it by either mode. The runtime may retain the selected outer handler internally for replay and restore shadowed installations on normal or exceptional exit. Source code receives no handle to that record. Frame validity covers those internal uses, prepared delayed invocations and restoration. Releasing a separately owned region while a later call still needs one of its targets remains invalid.

**Calls through lexical definitions.** A lexical proc can invoke an installed handler by symbol. Calling that proc by its definition exposes its body and the required binding to the checker (H-185/H-189). Calling an ordinary stored or passed callable that would reach the same perform/behave rejects. This is a source rule: a single possible target, inlining or a live handler at the indirect call site cannot legalize that call.

```cod
# @quote is installed above; @rate is currently 20.
proc read_quote() -> u64 { return behave @quote(); }
proc price() -> u64 { return read_quote(); }

val a <- price();         # 20 through two named definitions.
# proc later = read_quote;  # Reject: proc cannot initialize a callable value.
```

A diagnostic can identify the named procedure and suggest a direct call. The two callable value kinds are func and here; reading a handler or adapting a proc name cannot produce an ordinary stored callable, regardless of its body. Type-only signature queries never execute a handler.

A standalone `{{ ... }}` is an immediate call to its own definition. A generator's `yield` has a dedicated handoff to the caller's lexical loop body. Both retain their existing effect and binding checks. The proposed unified here parameter lets a receiver, including a func, invoke a procedural literal explicitly supplied from a proc. Its own code retains the restrictions of its declared kind. [Here callback example](CallableStorageAndHereAccess.md#an-effect-in-a-here-callback)

**Definition and method reuse.** A declaration supplying a symbol's signature, a static method import and runtime installation are distinct. On a strict definition-with-body reading, bare `effect @quote;` cannot install anything. It also has a possible narrower meaning as activation of a statically selected definition, without extracting an installed handler. The term “definition installation” alone does not decide between these spellings. D-21 requires an entry module extending a dependency family to supply an explicit installation. Its existing name-only examples would need a replacement that activates the composed family in the entry context. Two nested definition sites offering the same signature raise a separate issue: existing combined definitions contribute methods in the module world. Dynamic shadowing cannot by itself resolve equally specific distinct bodies in a single family. Per-installation implementations versus shared family extension require a rule before promising this replacement for swapped handler values. [Entry composition](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code)

### Method registration and scoped installation

A declaration can register or extend a symbol's module-world method family. Import and extension permissions, declaration promises, applicability and ambiguity govern that step. When execution reaches a combined definition, it activates the compatible family selected in the current module world and installs the resulting handler. A skipped branch installs nothing. Leaving the scope restores the prior installation on normal and exceptional exits. The result of installation is void.

The [entry-module requirement](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) still needs an explicit installation point for an extended family. The choice about name-only activation above determines whether that point requires a body. An assignment from a selected source family or from an active binding cannot supply it.

The prototype currently accepts `effect proc`; treatment of that spelling, defaults and detailed grammar migration remains DD-AOT-07. The selected `effect @sym(...)` behavior is settled. A skipped runtime path installs nothing; repeated execution creates installations without repeatedly registering methods. Existing declaration/scope boundaries apply.

H-379 requires the entry module to explicitly install a dependency callable that it extends; omission is a compile error. Correct entry installation cancels the applicable dependency installations so they do not retain unused world code. [D-21](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) owns initialization-only cancellation with operand evaluation preserved and LLVM reachability acceptance; identity/coverage and native mechanisms remain under review.

## Native binding boundary

Native binding layout must preserve typed payloads, actual owner provenance, captured/current selection and scope restoration. D-15 governs independent module worlds. Concrete layout and current implementation availability are recorded in the source capability reference; this boundary selects no runtime type registry or new ownership mechanism.

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
| `effect @s = value` | Scoped non-consuming reference rebinding under the symbol's declared owner constraint; H-561 admits lexical and ordinary regional owners for the owner-parameter form. Rejected for plain bits-valued symbols; explicit ref/ptr symbols may have bits targets. |
| Activate a symbolic procedure definition | Select the family and establish the scoped handler with its required symbolic bindings. Module registration is static. |
| `perform @p` | Resolve the procedure, temporarily install its captured effect values in its execution scope, execute its body, and restore the caller's bindings. |
| `behave @p` | Resolve and execute the procedure with its definition's lexical environment and the current dynamic bindings. Its stored dynamic captures are not installed. |
| Function | Lexically defined; compute from explicit inputs to outputs, with no symbolic effect access, `perform` or `behave`. |
| Procedure | May have an ordinary lexical name or a symbolic dynamic binding; may access effects, invoke procedures and call functions in either case. |
| Module top level | Declare symbols, functions and bits globals initialized by literals or function calls; register procedure definitions. Top-level `effect`, `perform` and `behave` expressions are also permitted. Scanning runs none of their execution semantics. |
| Module execution composition | Scan/load definitions first. Populate static runtime module namespaces before executing the statically ordered implicit dependency prefixes and explicit postpon blocks; entry execution runs last. Each block retains its module context. |
| Required bindings | Checked on the effective caller paths during on-demand whole-program compilation; absence is a compilation error. |
| Expression / `void` | Evaluation yields a result; `void` explicitly indicates no meaningful returned value. The language direction uses expressions without a separate statement category. |

Static registration publishes a procedure declaration before its definition activation executes. For an installed handler, activation establishes the required symbolic snapshot. Compiling or importing a definition does not read the current bindings. Named procedure declaration forms are described below; additional procedure-value expression syntax remains open.

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

A symbol declaration registers an identity, declared TLS contract and local access name. Bare `p` names that symbol object; it is not a second ordinary callable alias. A symbolic method declaration constructs the family’s semantic information. Neither supplies a current TLS binding. For a symbolic proc, perform/behave resolve the installed handler. A free-standing value expression `@p` rejects. Effect-destination syntax identifies the slot.

| Operation | Semantic result | Runtime installation |
| --- | --- | --- |
| Declare/select a symbol | Introduce or alias its identity and semantics. | None from selection alone. |
| Define/extend a symbolic callable | Construct receiver-local method information under selection permissions. | None without effect. |
| Combined effect definition | Select a compatible family in the current module interpretation. | Activate and install the scoped handler. |
| `effect @p;` | Candidate activation of the composed definitions without a body at this site. | [Under review](#definition-installed-effectful-procedures). |
| `effect @p = source;` or `effect @p = @source;` | Assignment installation of a symbolic procedure. | Reject. |

Two names for one symbol identity address the same TLS slot. Static name/identity aliasing supplies no runtime installation or access to a stored procedure value.

<a id="combined-using-and-identity-checked-abbreviation"></a>
### Source selection and explicit installation

Using/import/mixin composes module information according to existing permissions and selected contexts. Exporting a symbol-identified callable publishes its entire method set. Selecting a bodyless contract does not fabricate an implementation. Selection alone installs no receiver value; a dependency can still perform explicit effects in its once-only incorporated initialization.

The prototype’s `using symbol` and `... @source as @destination` paths combine selection with installation. Their old implicit-installation behavior requires coordinated grammar/example migration under H-372/H-373. For distinct identities, source selection alone cannot install the procedure of one identity under the other. Preserve name/identity aliasing and interpretation under the source-selection contract. A same-spelled distinct identity must not be silently merged; an installation destination is not a compile-time alias. Namespace collisions, shadowing and the final selection grammar remain scoped design work, not an assumed independent `@` lookup escape.

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

The mapping is compiler input and creates no runtime alias relation. `using` keeps source body/identities and interpretation; `mixin` translates the body into the receiving context. Activating a handler definition establishes its symbolic replay data. `perform` installs whichever values that resulting procedure captured; `behave` reads current dynamic bindings under that procedure's resolved identities. Capture values can be computed at runtime even though the set of identities and access locations is determined during compilation.

In that prototype form, outer `as @step` chooses the destination binding; the target language must expose installation as an effect, separately from alias selection. The inner `@target as @tgt_neuron` supplies an existing local symbol for a role in the mixed-in body. Neither should be treated as an implicit text-wide rename. Translation must cover symbolic reads, effect targets and procedure-call symbols consistently. Source initialization and other already-created procedures are not silently rewritten by this body's map; H-109's source execution and its own binding checks remain separate.

The prototype requires each explicit source role to occur in the selected body, rejects duplicate source-role entries, and diagnoses attempts to split aliases of one source identity across different receiving identities. Mapping multiple such aliases to one receiving identity works. These are bounded diagnostic policies; the complete language policy for broader mappings remains open. No general symbol-mapping framework or runtime registry is required.

### Syntax and practical use

The prototype `using/mixin "module" @source as @local` spelling exposes origin and destination but still conflates selection with installation. Coordinated migration must retain identity/context while making installation explicit. `symbol @local : T` introduces a new identity; `symbol from "module" @source as @local` names an existing identity. This keeps compile-time symbol declarations together and uses using/import for source-preserving selection and mixin for receiving-context definition. Neither declaration permits `@local = ...` assignment. H-214 excludes a module-prefix namespace and qualified callable names. Grouped selections may be considered without binding a module name; callable use still goes through selected local names.

Keep source provenance at declarations/selections and short names in procedure bodies. A local alias rename accompanied by the corresponding explicit mixin-map update should preserve the selected identities and runtime behavior. Auto-matching source-body spellings is convenient, but a change to one side alone can change or break the correspondence; navigation and diagnostics should show the resolved identities, not just matching text. Ambiguous aliases should require explicit disambiguation instead of depending on selection order.

Symbol selection still triggers source execution under H-109, after complete scanning and during entry-driven composition. The source top-level scope remains alive until process exit under H-166; identity-only selection still does not select a bound resource under H-110’s resource-selection rule. H-140 makes repeated encounters reuse the module incorporated at its first encounter. Grouped selection syntax remains open; direct module-qualified callable expressions are excluded by H-214. A local alias read uses an already selected identity rather than performing another module selection.

Rust's `use path as name` illustrates local aliases for explicit source paths. Racket's `only-in`, `rename-in` and `prefix-in` illustrate selection and spelling controls. These are useful naming comparisons; they do not supply CoD's dynamic-variable, destination-binding, capture or body-mixin semantics. [Rust Reference](https://doc.rust-lang.org/stable/reference/items/use-declarations.html), [Racket Guide](https://docs.racket-lang.org/guide/module-require.html).

### Module origins without local module names

H-214 keeps source modules as selection origins without module-valued local bindings or qualified-access syntax. H-484 additionally permits compiler lookup relationships to their published namespaces. Using/import/mixin names the source at the selection site; calls use local lexical names, selected operator/intrinsic names or symbolic perform/behave. A single alias preserves the selected source identity; H-230 separately creates identities for extension-module callables and same-alias unions. Diagnostics can display package/module provenance without admitting Module.f or Module.@p as expressions. There is no qualified-call or qualified-definition escape from extension permissions. H-215 proposes a separate explicit import-as declaration for selected method names. H-217/H-483 gives implicit names lower priority; H-484 realizes that through source-namespace fallback without a runtime module object. H-226 resolves explicit versus implicit names before typed method dispatch, preserves operation identity through aliases and rejects a demanded unresolved implicit-name conflict. H-490 fixes the selection unit at whole callables; alias availability and remaining surface details retain their separate scope. [The method-selection review](DomainLanguageDesign.md#type-associated-method-visibility-and-extension) records H-225–H-227 and the remaining boundaries. Preserve this boundary unless an actual serious design obstacle is identified.

### Explicit method aliases and implicit method names

[The type-selection and fallback contract](ModuleWorlds.md#type-selection-and-shadowable-associated-methods) also covers symbol from: selecting a symbol retains its declared type identity and registers its source's published namespace below local/explicit lookup. Public type and method names can resolve through multiple source levels without recursively traversing the symbol's type. H-487 rejects an intermediate private own binding when no same-named public alias exists. H-488 permits explicit re-export of an upstream identity under that public alias while keeping the local namesake private; lookup uses the public identity rather than skipping a private barrier implicitly. Caching an implicit resolution cannot create an own alias or a new private barrier. A directly declared local type/method may shadow those spellings without changing the symbol's type, installing TLS or materializing a callable value. Identity-based information merging and helper context remain intact; unclear source/context proximity requires explicit selection. Direct explicit-name collisions, private-resource access and H-253 overload conflicts are not waived. [Deferred information diagnostics](ModuleWorlds.md#deferred-information-diagnostics-for-implicit-lookup) will make reliance on implicit lookup visible at the actual mention.

The form `import "module" operation as local_operation;` selects a source-preserving callable alias with extension permission. An alias names the selected callable in its import context. H-230/H-232 gives receiving-context callable adaptation and same-alias unions distinct identities while retaining source method definitions; this is not body redefinition as mixin does. Compile-time aliases must not be confused with an explicit effect that installs a value under a predeclared destination identity. The local spelling is not the signature's identity.

H-217 gives automatically brought method names lower priority than explicit bindings in the same receiving scope. Name lookup first resolves the local binding, then argument-type dispatch uses that operation's visible method set. An explicit binding with no applicable method does not fall back to a hidden implicit name. Ordinary nested-scope shadowing and duplicate explicit-declaration rules remain; no new method is authorized merely by a using selection. Separate import-as can both supply an explicit alias and grant extension permission.

Implicit collisions remain unresolved until a demanded name or imported-family extension needs one identity, so explicit aliases can resolve the conflict without encounter-order winners. H-483 selects local shadowing of implicit type-associated names after both using and import; explicitly importing a callable, by name or through with methods, selects an extension family; it does not authorize use of an explicit alias before its selection event or new initialization effects. Source bodies retain definition context under using/import; H-232 adapts an imported helper's callable implementations to the receiving module; this must not silently become general lexical rebinding. Methods may be distributed across modules and inherited through abstract <: without an Interface body; only demanded missing implementations are errors. The full signature and visibility rules are in D-07.

H-489/H-490 makes with methods an optional explicit associated-callable selection on using/import/symbol-from. It selects entire callable families under the ordinary reuse form; signature association determines which callables are related, not which methods may be imported. Explicit names obey ordinary priority, collision/composition and extension rules; symbol from does not install TLS or grant import extension rights. Repeated aliases still share identity. [D-15](ModuleWorlds.md#explicit-associated-callables-with-with-methods) owns the detailed contract and implementation boundary.

A private helper remains a source lexical definition even when its enclosing imported helper adapts public callable dependencies. That selected implementation context propagates through the private body; an unrelated same-spelled receiving definition does not replace it. Direct using/import/mixin of the private name still requires an export.

### Captured callees and selected implementations

A symbolic call uses the selected procedure value in its dynamic binding. Importing the containing helper does not rewrite that stored value into a different implementation. `perform` restores the helper’s captured values; `behave` uses the current bindings. An explicit effect installation can supply an imported procedure whose own ordinary callable dependencies use an adapted implementation context. These are separate selections, with no recapture or runtime method dictionary.

Consider a source inner procedure computing score(x) + @bias. The source score returns 1, the adapter score returns 7, the definition-time bias is 1 and the current bias is 5. A routing procedure invokes procedure-valued @task, which the caller binds to the adapter-selected inner procedure.

| Call | Native result |
| --- | --- |
| Direct source-preserved helper | 6 |
| Direct import-adapted helper | 12 |
| Perform the source-preserved or import-adapted router | 2 for each; both restore the original captured @task value. |
| Behave through either router after explicit @task rebinding | 12 for each; both use the selected current procedure value. |
| Perform the explicitly adapted inner procedure | 8; adapted score7 with its retained captured bias1. |
| Read caller @bias after the calls | 5; temporary captures did not replace its binding. |

The complete generated sources and output are preserved in the record. This checks composition of the existing capture/context contracts; it does not select a policy for competing dependency views or establish lexical-call acceptance; H-257 and K-400 supply that separate contract and evidence.

### Callee selection and argument rebinding

Call preparation selects the procedure value, evaluates arguments in the caller environment, then invokes that selected value. Perform uses its stored captures; behave uses the environment after argument evaluation. Argument effects belong to the enclosing scope; restoring the procedure body does not undo them.

The example selects an exported read procedure with captured bias10 and defines an alternate with captured bias20. Passing an argument that binds bias31 gives41 under perform and leaves caller bias31; binding32 gives64 under behave and leaves32. Rebinding the callee symbol to the alternate inside an argument does not change the in-flight selected read: it returns43 under perform or68 under behave. Subsequent calls use the alternate, producing3301/3401 with current bindings or2001 with its capture.

This is actual prototype behavior, not a newly confirmed language-wide call-order rule. H-148 fixes each argument's conversion before the next argument; it does not expressly place symbolic callee resolution relative to effectful arguments. That timing distinction can be reviewed with the local value/call namespace question. The probe adds no callback-call syntax, runtime dispatch or changed capture mechanism.

### Explicit conformance: declaration permission and consumer visibility

H-218 permits `data D <: A;` to add conformance for an existing data identity, including declarations in different modules and more than one abstraction. Available signatures never infer conformance. H-220 requires an adapter to import the ADT before declaring such a relation. Using the ADT is insufficient to authorize the declaration; using the adapter is sufficient for consumers to obtain that declared relation. Visibility to use and permission to extend are separate facts.

Source-preserving functions/procedures retain their source body, non-callable lexical bindings and captured values. The native implementation retains source-family provenance while selecting ordinary methods under distinct module/context identities; it does not accumulate every caller-visible method. H-230 requires distinct extension/union identities; H-232 gives using the preserved selected context and import the receiving implementation context; compound alias/context conflicts remain to be specified. H-229 preserves established conformance of concrete arguments through broad constraints and nested traversal, without publishing unrelated caller type relations. Operator/intrinsic candidate lookup remains lexical. Mixin checks the redefined body in its receiving context: interface use needs the corresponding adapter view, and declaring new conformance needs import permission on the ADT. This is independent of perform/behave's dynamic-value selection. No new effect binding, body capture, region permission or runtime type field follows from conformance.

D-07 illustrates selecting the data type through an adapter with using. Explicit export/re-export and export import have a bounded implementation; H-491 additionally specifies publication-only export from, which is not implemented; broader publication choices remain pending. Neither requires module-qualified calls or a runtime interface registry. A source body may use its own declared relations without exporting those facts through every callable selection. Once-only incorporation and explicit alias availability retain their existing rules.

### Disjoint declarations and exempt inheritance paths

H-527 selects the direction `type disjoint A, B, C;`, including larger lists. One declaration constrains every unordered pair of distinct resolved endpoints; textual order within the list does not establish priority. Aliases resolve before comparison. At least two distinct endpoints are needed; duplicate identities should diagnose rather than become a self-disjointness claim. Pair constraints are symmetric, but disjointness between A/B and B/C does not imply disjointness between A/C.

This is **disjointness with existing endpoint-ancestry exemptions**, not an unconditional assertion that the endpoint types have no common values. H-528 fixes the inheritance graph in the fused world before execution. Source declarations and module incorporation supply static evidence; no runtime inheritance mutation, runtime constraint publication or startup scheduling is involved. H-532 fixes declaration authority below; formal validation remains under review.

#### Exemptions are existing ancestry among listed endpoints

H-534 defines the exemption narrowly: an inheritance relation already established **between types named in the disjoint list** remains admitted. The disjoint rule prevents adding a previously absent relation between those endpoints later during static preparation/fusion. Existing intermediate nodes may explain that relation; they are ordinary type-graph evidence, not a separately copied exemption graph.

```cod
type Exception;
type Error <: Exception;
type disjoint Error, Exception;   # Existing Error → Exception remains admitted.
data MyError <: Error { code:u64; }  # Also an Exception through that ancestry.
```

The retained frozen type graph already represents the admitted endpoint ancestry. There is no requirement to copy every path, snapshot each whole module graph, grandfather arbitrary old common conformers or keep a separate per-constraint exemption-path catalogue. Constraint origin and ordinary declaration/relationship evidence are still needed for authoring, publication and diagnostics; that is distinct from duplicating ancestor paths. A candidate new relation must not be installed and then used as the proof of its own exemption. During module composition, retain the selected source-world meaning rather than treating every relation in a newly combined graph as previously admitted.

Consequently an old data type conforming independently to unrelated A and B is not exempt merely because it predates type disjoint A, B. There is no pre-existing A/B ancestry to supply that exemption; such a type is a conflict when validating the constraint. Conversely an existing B <: A relation permits the overlap inherently inherited by B and its descendants. This scopes the negative fact precisely instead of claiming an empty intersection whenever a constraint exists.

H-535 confirms the general rule: a subtype cannot independently declare two types from the disjoint list as parents. Existing ancestry among the listed types permits the relationship inherited through one selected parent; it does not permit adding another independent parent relationship to a subtype that already entered through a different listed type. This is the mechanism that also enforces H-526's checked-to-Error rejection, not an exception-only special case.

```cod
type A;
type B <: A;
type disjoint A, B;
data X <: A { code:u64; }
data X <: B;                  # Reject: a second independent listed parent.

data Y <: B { code:u64; }     # Accept: A is inherited through existing B → A.
data Y <: A;                  # Accept with warning: Y → B → A already exists.
```

The two declaration orders differ because only the second example's final statement asserts an already established relationship. A rejected new relationship cannot retroactively make an earlier declaration redundant. Check inherited relationships too: an intermediate parent or alias must not hide independent conformance to both endpoints. Reuse the ordinary graph and source/selected-world evidence for these checks; no exemption-path catalogue is required.

Multiple applicable constraints remain conjunctive. Their originating identities, endpoint lists and selected-world meaning survive import/re-export; declaration deduplication does not authorize replacing an older contract or inventing receiver exemptions. The source graph and constraints freeze before execution, and module traversal/cache order does not manufacture an earlier relationship.

#### Redundant shorter paths and intermediate method contracts

H-530 refines H-529's treatment of redundant paths: when the source and target already have an established indirect subtype relationship, a new shorter declaration is accepted with a warning. It records a redundant assertion of that existing relationship, not a new independent route or an enlarged exemption. The witness must already be established without the new declaration; a later relation cannot justify it retroactively.

```cod
type Resource;
type Reader <: Resource;
type disjoint Reader, Resource;
data File <: Reader { id:u64; }
data File <: Resource;           # Accept with warning: File → Reader → Resource.
```

H-531 explains the warning's purpose: the shorter declaration may reveal that the author has overlooked the intermediate abstraction's methods and believes only the higher parent's contract matters. The compiler must not assert that this was the author's intent or that an implementation is actually missing. It should show the existing path, identify the intermediate abstraction(s), and explain that the direct declaration does not bypass their method contracts or existing method-selection behavior. Suggested wording:

```text
warning: File already conforms to Resource through File → Reader → Resource.
note: declaring File <: Resource does not bypass Reader or its method contracts.
      Check the Reader requirements if the direct declaration was intended to replace that path.
```

Preserve the effective inheritance/conformance graph and all intermediate obligations. A shorter source path does not make the higher-parent method more specific: existing nominal specificity, not inheritance-path length, still governs dispatch. Retain the source declaration and its diagnostic/proof mapping; acceptance of redundant evidence is not permission to silently edit the author's text. Implementations may already come from visible generic or separately supplied methods; the warning does not demand that File redeclare them. H-216/H-218's demand-based checking remains: an unused missing implementation is not newly rejected just because this warning is emitted. When several witness paths exist, a compact diagnostic can show one established path and provide locations for additional evidence; it must not imply that unshown intermediate contracts disappear.

A checked Failure <: Exception acquiring Failure <: Error is not a redundant declaration: no prior Failure → Error relation exists. H-535's general disjoint rule rejects it, satisfying H-526's required checkedness stability. A new relation cannot retroactively turn the earlier declaration into a redundant assertion.

#### Authority to declare a constraint

H-532 requires the declaring module to have authority over at least one endpoint of **each unordered pair**: the endpoint is defined by that module, or explicitly selected through import with extension permission. Merely using a foreign type supplies no such grant. Resolve nominal identities and the applicable import binding at the declaration point; a local alias for a foreign type does not count as defining that type. Ordinary source-name/selection availability and extension permissions remain in force.

```cod
using "a.cod" A;
using "b.cod" B;
type disjoint A, B;          # Reject: neither endpoint supplies declaration authority.

# Alternative selection in a module that intends to declare the constraint:
import "a.cod" A;
using "b.cod" B;
type disjoint A, B;          # Authoring permission satisfied; paths still need validation.
```

For an n-ary declaration, the equivalent concise condition is that at most one distinct endpoint lacks declaration authority. Defining local C does not authorize the A/B pair in type disjoint A, B, C when both A and B were merely used. The diagnostic should identify an unauthorized pair and explain which import/definition grant is missing; it must not silently rewrite using to import, because the selections have their own contracts. An import grant permits declaring the constraint, not violating an existing one or enlarging another declaration's exemptions.

This check applies to authoring a **new** record. Consuming a public record through using/import, deduplicating the same originating record or re-exporting that unchanged record does not author it again and needs no new endpoint grant. The original authority/evidence travels with its provenance; consumers must still satisfy the constraint. Automatic type-carried evidence therefore remains compatible with H-527's using path and does not grant consumers new extension permission.

#### Public evidence travels with selected types

H-527 makes the disjoint record and its exempt paths public when any participating type is exported. Using/importing a participating type carries the corresponding constraint evidence automatically, without a with methods clause or separate import of the constraint. Treat an n-ary declaration as one complete contract: selecting A from a declaration covering A/B/C carries its pair constraints and established endpoint ancestry, including the B/C constraint. Selecting a subtype also requires the applicable ancestor evidence; erasing it through a broad alias or an intermediate re-export must not bypass validation.

Publication is part of the completed static module interface. An export appearing textually before the disjoint declaration still publishes its applicable record; this does not backdate or newly establish an endpoint ancestry relation. Evidence must retain referenced type and relation identities, including intermediate path nodes needed for validation and diagnostics. It does not by itself introduce every referenced name into the receiver's explicit namespace, select every associated callable, expose private fields/bodies or grant extension permission. Ordinary source fallback/private-name rules continue for name lookup; the explicitly public relation/path facts themselves cannot be hidden in a way that prevents checking.

A receiver validates applicable local and selected-source constraints against the admitted type relations before freezing its world. Contradictory combinations reject with the declaration and relevant relationship evidence. Receiver traversal/cache discovery order cannot establish ancestry or choose a conflict winner. Keep stable record/group identities, selected-world meaning and dependencies; an anonymous pair without its meaning in the existing type graph is insufficient.

#### Shared information and remaining validation boundaries

[H-533's module-world pool candidate](ModuleWorlds.md#shared-disjoint-information-per-module-world) can share constraint records without duplicating them for each type. Defined/imported type information selects the receiving world's pool, while using retains its source-world association. That is a representation candidate under review, not a new nominal type identity, a complete constraint-applicability rule or a permanent pointer ABI. H-534 removes the need for a separate old-path snapshot catalogue; existing type relations supply endpoint ancestry.

- **Static evidence:** declaration-only module cycles remain allowed under D-15. Independently grounded type declarations may be established within an import cycle; self-dependent justification cannot supply an already existing relation. This is ordinary static relation/admission evidence, not a new exemption-history solver or runtime schedule. Future/self-dependent evidence cannot justify H-530's warning-only shorter declaration.
- **Selected worlds and publication:** a pool reference must preserve inherited public constraints, local constraints involving source-preserved using types and publication filtering. Source type information remains read-only; aliases must not evade applicable local contracts. Precise overlap consequences, not bare pool membership, govern coverage/overload/optimization.
- **Implementation scope:** reuse the frozen nominal graph and existing conformance evidence. Per-world indexes and shared record references are possible mechanisms; no copied path inventory, complexity guarantee or runtime inheritance update is selected. Generic/cyclic checking stays within D-26's finite evidence and resource-diagnostic boundaries; a missing proof is not a demonstrated contradiction.

**Acceptance:** n-ary pair symmetry without disjointness transitivity; existing ancestry between listed endpoints; descendants inheriting that relation; rejection of newly introduced endpoint ancestry or a subtype's second independent listed-parent relationship; unrelated old common conformers supplying no exemption; the X/Y order distinction above; redundant shorter declarations with prior-path/intermediate-method warnings; per-pair authoring grants; original complete-record publication/using/import/re-export; local/source constraint combination without private-record leakage or alias bypass; grounded static evidence and deterministic frozen-world conflicts; unchanged checkedness/demanded method checking; and no separate exemption-path store or runtime mutation. Physical pool representation and finite generic/cyclic proof procedures remain separately scoped; there is no source implementation evidence.

### Explicit publication and extension identities

H-231 makes module resources private unless declared with export. Using/import/mixin, symbol selection and implicit associated-method selection must respect that boundary; an alias or selected type cannot bypass it. Public source bodies can use their own private dependencies without making them directly selectable. H-233 requires a receiving alias to have its own explicit export before it can be selected again. The confirmed shorthand export import performs the normal import and publishes everything that statement selects, including its associated method selections, within source privacy constraints. The proposed export list and outstanding conformance/conversion bundle details are in [D-07](DomainLanguageDesign.md#explicit-exports-and-private-module-resources); the prototype supports single-name export declarations and export import. H-491 selects export from with optional public renaming: publish the source object without creating an own local alias or extension target, allowing a distinct private local namesake. This new form, grouped lists and broader bundle support are outside current implementation.

H-230 requires distinct callable identities for extension modules. Different local aliases preserve their selected method sets; same-alias composition of versions of the same semantic callable creates a virtual union identity in the selecting module. The required base/extended/indirect example is1,7,1. The [helper-context example](DomainLanguageDesign.md#extension-module-callable-identities-and-local-unions) has a confirmed result under H-232: using base twice gives2, while import base twice gives14 through the receiving score implementation. A later using preserves the selected exported context. This adaptation does not change symbol identity, dynamic bindings, capture mode, source globals or general lexical body resolution; mixin remains body redefinition. H-253 requires conflicting inherited and receiver-selected overloads to reject during import adaptation; no alias/identity priority silently selects a winner. Explicit arbitration syntax remains future design.

### Implementation boundary and acceptance

The selected module contract uses using/import for existing definitions and mixin for receiver-local definitions. Source-preserving selections retain types, lexical resolution and captures; symbolic destinations must be declared before selection. Omitting as requires matching source/destination symbol identities, while explicit as can install the source-defined procedure under a different predeclared destination. Combined identity introduction remains using symbol. Direct named-callable extension requires import under the extended local alias. The module contract keeps semantic-family provenance separate from extension/local-union identities and checks exports for every cross-module selector. Original methods with no ordinary callable dependencies deduplicate across contexts; distinct observable contexts are not silently erased. Import stores a receiving implementation context, using preserves the selected context, and scope restoration prevents cross-alias leakage. Selecting a type establishes lower-priority lookup through the source's published namespace, making associated behavior available along with its other public names. H-483 requires local ordinary definitions to shadow names brought implicitly by either type reuse form. Explicit callable imports retain their extension rules; symbol from establishes the same public-source fallback. The complete new surface is not established by this implementation evidence. Original methods deduplicate across paths. Cross-alias grants and extension of unrelated merged callable families remain separately scoped. These limits do not change the separate declaration rules for compiler operations.

Mixin translates source-body symbol roles to receiving identities before checking required bindings and collecting captures. Repeated mixing uses the current provider's aliases; it does not rewrite the source initialization or existing definitions. Body maps are rejected on using/import, and built-in procedures have no body to mix. Lexical procedures may also be mixed under ordinary local names. A symbolic installation requires a definition. Operation-overload mixin remains outside this subset; mixin does not copy type definitions, function definitions or globals. UoM selection retains its separate using-units contract.

## Vocabulary modules and procedure reuse

Vocabulary modules publish common symbol/type identities and callable contracts/methods. Primarily abstract types and supporting ADTs allow independent worlds to agree on meaning. Selecting a symbol supplies no default TLS value. Export of a symbol-identified callable includes its whole method family; view compatibility is separate from privacy.

| Semantic selection | Body and non-callable lexical context | Callable interpretation |
| --- | --- | --- |
| `using` | Preserve the source definition. | Preserve its selected interpretation; no extension permission. |
| `import` | Preserve the source body and non-callable bindings. | Admit receiver-local extension and the selected receiving interpretation. |
| `mixin` | Redefine the body in the receiver. | Resolve under that receiving definition, with explicit symbol mapping where supplied. |

These compile-time operations neither install a TLS binding nor snapshot runtime values. Activating the symbolic definition selects its family and snapshots the required symbols. Source lexical slots and actual owners keep their defining identities.

When an installed definition activated with @x equal to 1 and the caller subsequently installs 7, perform replays 1 and behave sees 7. Both use the selected handler’s family and interpretation.

H-230’s base/extended/indirect `1,7,1` and H-232’s using/import interpretation remain acceptance constraints. Source body/non-callable bindings and independent identities persist. Equivalent definitions may share machine code, but merely removing the one-method guard or renaming an internal identity cannot implement family values.

## Functions and procedures

H-197/H-198 establishes functions as lexical callables that may operate on explicit memory and explicit-argument system intrinsics without reading or writing dynamic bindings. Procedures may additionally use dynamic context. The ordinary function contract is mutable; a future const-function form would be a separate design.

A function definition binds a lexical name and cannot access dynamic effect variables. A procedure may have an ordinary lexical name or belong to a symbol-identified module family. Explicit effects install family views into dynamic scope. Both procedure forms may use effects; lexical naming does not make a procedure a function. A function cannot call either procedure form, directly or indirectly. Under [H-603's generator boundary](RangedForAndGenerators.md#callable-kind-and-supplied-body-effects), yield transfers control to the caller's lexical loop body; that body is not an ordinary call made by the generator. A procedure caller may therefore use procedure operations in the body of a for invoking a func generator. The generator's own statements, ordinary callees, yield operands and cleanup retain the function restriction.

A lexical procedure keeps its definition context. A helper that reaches perform/behave can be invoked [by its definition](#definition-installed-effectful-procedures); invoking it through an ordinary callable value rejects. Assignment of a lexical procedure to an effect slot rejects. Procedure mixin redefines a source body in the receiving context.

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

A parameter or local declaration that shadows an outer local, module callable or module value/type binding emits `[-Wshadow]`. `-Werror=shadow` or the general warnings-as-errors option makes it an error. Same-scope duplicate declarations remain errors. No user-visible call protocol, escaping callable-view representation, general function-value type or runtime callable dispatch is introduced.

### Callable declarations and methods

A callable identity can exist without any executable methods. Its signature declarations and its methods are separate module-world information. Using/import, explicit exports, aliases and receiver-local merging apply to both; selecting or exporting an unimplemented declaration does not execute or fabricate a body. A demanded call requires an applicable implementation. A function declaration promises a function implementation with a compatible result for its applicable inputs; a procedure declaration supplies the corresponding procedure contract. These promises are checked on demanded calls, alongside the selected method's own signature.

H-258 requires parameters and an explicit return type on a bodyless declaration; an untyped identity-only `function score;` is not supported. Generic declarations and methods use compile-time type parameters, including abstract upper bounds, and a final variadic parameter:

```text
function score(x:T) -> i64 where T <: Metric;
function combine(first:T, rest:T...) -> T where T;
proc dispatch(first:T, rest:T...) -> void where T;
function pair(x:T, y:U) -> i64 where {T, U};
```

Type parameters are inferred from argument type evidence at a demanded call. Repeated occurrences constrain those types to agree; bounds use the caller's selected conformance information. A type parameter that cannot be inferred from inputs is diagnosed. Unique-most-specific dispatch compares signature constraints, including concrete types, bounds and repeated-variable relationships, independently of declaration order. Equally specific candidates remain ambiguous. The variadic pack contract below distinguishes signature shape from runtime element count; H-450 ranks fixed-versus-variadic competition for implicitly collected arguments by type matching first and fixed form only on an otherwise equal match; H-475 rejects a crossing conflict with fewer-defaults preference. This bounded prototype does not supply generic ADT constructors, explicit type-argument calls or a complete higher-rank type system.

### Runtime-array expansion and one variadic parameter

H-449 selects `values...` for an ordinary runtime-sized array as well as forwarding an existing variadic pack. A final `rest:T...` is **one logical parameter** after any fixed prefix. Its element type and declared passing/access/owner constraints participate in matching; its element count is runtime data. Treating the tail as one input does not erase per-element constraints or repeated-type relationships.

```text
# Design examples; executable variadic bodies and array expansion remain implementation work.
func choose(x:u64) -> u64 { return 1u64; }
func choose(rest:u64...) -> u64 { return 2u64; }
# values is an ordinary runtime-sized u64 array.
choose(values...); # supplies one pack, including when values has zero or one element
choose(7u64);      # both match u64 exactly; fixed form wins and returns 1u64
```

Explicit expansion supplies the variadic pack; it does not flatten its elements into a variable number of fixed parameters. Changing the array length cannot select `choose(x:u64)`, a zero-argument method or a different fixed-arity method. Knowing the length during optimization does not change that source meaning. A typed empty array still provides element-type evidence while supplying zero elements. H-475 also admits explicit `[]...` with element and any required owner evidence established from the candidate signature or other inputs; in `mixed(7u64, []...)` below the declared u64 pack supplies that element contract. Empty expansion provides no element-derived type or owner evidence of its own. It cannot choose arbitrarily between otherwise ambiguous element contracts or infer an unconstrained generic element type/owner from no elements. This scoped contextual empty-pack form does not select general array-literal construction, storage or allocation semantics. Multiple variadic overloads still need ordinary type/constraint selection and native coverage under [D-26](AheadOfTimeExecutionPreparation.md#generic-and-recursive-native-coverage).

H-450 settles fixed-versus-implicit-pack competition for an ordinary unspread call. Compare the existing type/constraint matches first, preserving exact-before-lossless conversion selection. Only when those matches are otherwise equally specific does the fixed form win over implicit tail collection, subject to H-475: a contrary fewer-defaults preference leaves the candidates ambiguous. This preference cannot resolve incomparable type constraints or make an inapplicable method applicable. For example, with `select(x:u64)` and `select(rest:u32...)`, `select(7u32)` selects the exact u32 variadic match over widening to the fixed u64 parameter. Explicit `select(values...)` remains a pack input, independent of count.

The selected parameter mode governs element passage. Supported value elements are independent values, so writes to callee value elements cannot modify the source array. Borrowed elements retain their actual targets, owners, access and initialization obligations. Expansion introduces no regional copy/move, lifetime extension or permission upgrade. Any dynamically selected borrowed element must retain a statically unique owner; treating the tail as one parameter does not make unrelated owners interchangeable. Ordinary source-order argument preparation and failures remain observable; the precise interleaving of element preparation within an expansion remains part of the operational review.

No public pack type, heap allocation, permanent ABI or native specialization per length is selected. Arbitrary iterator expansion is outside this initial scope. Initial implementation may accept a conservative, sound subset and diagnose unsupported cases; it must not choose a different overload or discard owner/access/initialization constraints to make an unsupported call run. Existing typed bodyless variadic declarations can constrain fixed-arity method implementations; that declaration-contract checking is distinct from executing a method with a pack parameter.

**Acceptance:** cover empty, singleton and longer arrays with the same element contract selecting the same variadic signature; inferred element types from typed empty arrays; independent scalar values versus preserved borrowed targets; rejected owner/access/init mismatches; repeated-type constraints; equivalent selection with or without constant-length optimization; a fixed u64 parameter winning an otherwise tied singleton u64 pack match; and an exact u32 pack match winning over fixed u64 widening. H-475 additionally requires rejection of crossed fixed/default preferences, explicit contextual empty-pack selection, and rejection where empty expansion leaves required type/owner evidence missing or alternatives ambiguous. H-448's bounds policy applies whenever builtin element indexing is admitted. No existing prototype evidence establishes this complete acceptance scope.

**Still under review:** grouping and specificity when variadic candidates have different fixed prefixes; multiple expansions and precise preparation order; indexed element result/mutation modes; escaping/storage and callable-view contracts; physical pack transport. The proposed body operations are `len(rest)`, `rest[i]` and forwarding through `other(first, rest...)`; their remaining details need the operational contract before implementation delegation.

H-448 separately settles the builtin bounds policy through [D-17](ExceptionControl.md#unchecked-bounds-exceptions-and-optional-checks): out-of-range indexing raises unchecked IndexOutOfBoundsError under default runtime checks, with an explicit compiler opt-out at the programmer's risk. No throws annotation is required for this exception. It does not qualify broader pack operations or change argument evaluation, owners, initialization or the selected user-defined indexing method's contract.

### Expression type conversion and assertion

H-453 selects `expression : Type` for explicit conversion/type assertion. It is not a named-argument separator; declaration annotations retain their own grammar. H-468/H-471 chooses the operation from its concrete types at semantic specialization. A pure numeric value-to-value pair uses [H-466 numeric conversion](ScalarUnitsAndIndexTypes.md#explicit-numeric-conversion-and-loss-warnings): warn unless losslessness is proven, permit opt-in errors, and insert no runtime conversion enforcement; actual loss/unrepresentability is undefined behavior. Other admitted type assertions follow H-467/H-470: resolve provable success/failure at compile time and check unresolved membership at runtime, throwing an unchecked mismatch. Numeric warnings and bounds-check opt-out do not disable those checks. A numeric payload inside a ref/ptr, unit-bearing type or other nominal contract does not erase that contract or turn the whole operation into a pure numeric value conversion.

Conversion produces a value through an admitted conversion; assertion checks whether an existing value/view satisfies the requested type. Once the assertion path is selected, it invokes no constructor, user conversion, payload copy, graph transfer or method-body probe to make the check pass. Its result retains the value or selected target, actual type identity, owner/locator and permitted lifetime/access. A narrowed reference view is not a new object. Ordinary initialization/passage rules still govern how the surrounding expression receives that result.

#### Type membership and branch refinement

`x isa T` is a built-in `bool` predicate. Resolve T statically, including a bound type parameter. Evaluate x once. Test its actual type, or the selected target’s type for a ref or ptr, against T using the same nominal membership and visible conformance relations as a [nonnumeric assertion](#nonnumeric-assertion-contract) in the selected fused world. An abstract T accepts its conforming instances. A mismatch yields false, including a statically provable mismatch such as `7u32 isa u64`. No numeric conversion takes place. Folding a known answer preserves operand effects and failures. An unresolved test requires sufficient runtime type evidence.

For `ptr p:Entity in O`, null matches no target type. **A successful `p isa Node` proves both that p is non-null and that its selected target is a Node.** The compiler makes both facts available wherever control flow establishes that result. With Entity and Node declared as in the assertion example below:

```cod
func count_or_zero(ptr p:Entity in O) -> u64 where {const owner O} {
    if (!(p isa Node)) { return 0u64; }
    return p.count;
}
```

A false result excludes Node for that selection. A nullable p might still be null or select another Entity. Inferring a particular alternative requires evidence that it is the only possibility on that path. At a join, a use requires the relevant facts on every reaching path.

**Rebinding a ref or ptr slot invalidates the old target proof for subsequent reads of that slot.** This also applies when an intervening operation can rebind it through a declared place alias. A write to an ordinary payload field leaves the selection intact. A separately bound reference to the tested target keeps its own evidence for as long as its owner is live:

```cod
func previous_count(ptr p:Entity in O, ptr other:Entity in O) -> u64 where {owner O} {
    if (p isa Node) {
        ref saved:Node in O = p;
        p = other;
        # p.count would reject here: other may be null or select a different type.
        return saved.count;
    }
    return 0u64;
}
```

The guard preserves owner, access, initialization obligations and the declared binding mode. It grants no ownership transfer or lifetime extension. The predicate takes a type operand; `p is q` and `p is null` remain target-identity tests. The examples specify source acceptance. Compiler and native support are tracked separately.

#### Nonnumeric assertion contract

In this design example, both concrete types conform to Entity, and the source form is not a claim of parser support:

```cod
type Entity;
data Node <: Entity { count:u64; }
data Edge <: Entity { weight:u64; }

proc node_count(x:Entity in O) -> u64 where {owner O} {
    ref n = x : Node in O;
    return n.count;
}
```

If the selected target is a Node, the assertion succeeds and n refers to that same target. An Edge produces the unchecked assertion failure before `n.count` when the distinction is resolved only at runtime; a statically established impossible assertion rejects under H-470. Equal layout or field types do not establish nominal compatibility. Abstract targets use admitted nominal/conformance relations in the selected interpretation, including visible adapter declarations, rather than exact-concrete equality or structural method search. Aliases preserve identities; same-spelled independent declarations do not. Broader views retain the actual-type evidence used by later method selection.

The assertion evaluates its operand once on admitted execution. H-470 requires statically proven true checks to need no runtime validation and statically proven false assertions to reject compilation. Removing a redundant check preserves operand effects/failures. Check success and later use concern the selected target, not a second read of a mutable slot; preserve owner/locator and type/layout evidence across lexical relocation. A later reload does not inherit facts about an earlier target. Compile-time failure is not catchable by surrounding try/catch. Ordinary source/type-checking rules still govern unreachable or undemanded code; this decision introduces no blanket diagnostic waiver there.

The failure-category name is **TypeAssertionFailedError** under H-472; payload details remain engineering work. It requires no mandatory throws declaration or catch, but is catchable and follows D-17 propagation, defer/finally, explicit causes and cleanup. Operand evaluation may have its own checked failures; classifying the final assertion as unchecked does not erase them. A failed assertion is an expected outcome of a requested test, not D-26's UnsoundRuntimeTypingError soundness backstop. It does not undo prior writes or allocations. Call argument/default preparation keeps H-458's entry/ownership phase boundaries.

Static owner, lifetime, access and initialization constraints remain obligations, not new runtime escape hatches. An assertion cannot relabel O as another owner, obtain ownership, upgrade readonly access or turn uninitialized payload into readable data. Where available independent type evidence permits a check without reading payload, it still carries the original init obligation; payload-dependent checks require the payload's ordinary initialization proof. A type check itself does not authorize a new object-header or per-reference owner/type allocation. H-469 places non-null refinement in an explicit predicate assertion or an existing proven branch, rather than adding a null check to a type assertion.

D-26's distinction between known mismatch and unavailable interpretation remains. Sufficient actual identity/conformance evidence may establish success or failure without interpreting payload; being outside the fused catalog is not by itself a mismatch or checked exception. If an admitted operation genuinely requires interpretation that the prepared artifact cannot supply, existing checked StaticallyUnknownTypeException rules apply. Lack of compiler proof/evidence cannot simply be reported as a false assertion or successful cast. Runtime evidence transport and admission coverage retain their implementation/design boundaries.

Callable, generic and aggregate targets use their own admitted compatibility rules; this operator does not create new covariance or structural conformance. A callable-view check cannot invoke a target as a probe, discard required origin signatures, invent an adapter/default capture or bypass native coverage. Detailed callable-view/type-context admission remains separate from H-467's failure classification. Runtime type-valued expression grammar/staging is also not selected by this decision.

**Nullability boundary:** `p : ref Node in O` requires an already established non-null fact, for example from `assert p is not null` or a [successful target membership test](#type-membership-and-branch-refinement) on the same selection. The type assertion does not discharge that separate obligation by throwing on null. Without the fact, reject the requested ref view. An admitted nullable-target assertion preserves null when the destination accepts absence and owner/access constraints match; this supplies neither a payload-type witness nor a non-null fact. Non-null targets still undergo the requested membership check. No form invents an owner for unqualified null.

**Acceptance:** success preserves target identity/owner/access; static mismatch rejects and unresolved runtime mismatch is catchable without throws before dependent access. Preserve operand effects, stable selections across mutation/relocation, actual-type dispatch, init obligations, cleanup and argument/default phases. Include abstract targets, aliases versus homonyms, concrete numeric specializations, explicit non-null proof and sufficient versus unavailable evidence. Invalid owner/access/type syntax remains statically rejected. No compiler or native support is claimed.

#### Julia reference and CoD specialization

Reference review: official Julia documentation and the v1.12.0 core implementation, consulted 2026-11-15. No local Julia interpreter was available; these are documentation/source checks, not executed experiments or CoD authority.

| Julia reference behavior | CoD relevance |
| --- | --- |
| `x::T` checks `isa`, throws TypeError on mismatch and returns x on success. The core validates that T is a type and calls no converter. [Core implementation](https://github.com/JuliaLang/julia/blob/v1.12.0/src/builtins.c#L588-L596) | Supports the selected test-and-preserve assertion model. CoD numeric `:` has its separately selected conversion policy. |
| Abstract targets accept conforming instances; parameterized composites are invariant. Union/UnionAll describe admitted alternatives; `nothing` has type Nothing. [Type manual](https://docs.julialang.org/en/v1/manual/types/) | Use CoD's own nominal/generic relations. Do not import a union/top type, blanket variance or a Nothing-based model of owner-bearing null. |
| `Type{T}` can select the type object T itself. [Type selectors](https://docs.julialang.org/en/v1/manual/types/#Type{T}-type-selectors) | Type-object identity and payload instance compatibility are distinct; CoD type/kind staging remains its own contract. |
| Typed assignment and typed return perform `convert`; expression assertion does not. [Conversion triggers](https://docs.julialang.org/en/v1/manual/conversion-and-promotion/#When-is-convert-called?) | CoD declaration initialization, return handling and expression assertion must retain their distinct rules. |
| Argument type annotations constrain method applicability. [Function arguments](https://docs.julialang.org/en/v1/manual/functions/#Argument-type-declarations) | Assertion cannot add implicit conversion routes to CoD dispatch or satisfy a missing callable method. |

**Concrete-type selection (H-468/H-471).** Resolve the operation using concrete type evidence within the semantic specialization; an unconstrained type-parameter spelling does not force the assertion path. For example:

```cod
func cast(x:T) -> u64 where {T} {
    return x : u64;
}
cast(7u32);     # Concrete u32 -> u64: lossless conversion, result 7.
cast(3.5f64);   # Concrete f64 -> u64: numeric warning/opt-in error; execution is UB.
```

Pure numeric membership is semantic, not a representation-width or layout test. Bool, nominal ADTs, references/owners, callable contracts and unit identities do not become numeric because their representation contains bits. Existing unit routes and D-16 fraction rules retain their scope; this choice adds no bool conversion, unit erasure or fraction implementation. For nonnumeric assertions, concrete evidence determines admitted compatibility and H-470 applies before execution wherever the current call/specialization is provably impossible.

This fits [D-26's prepared implementation and local-dispatch model](AheadOfTimeExecutionPreparation.md#recursive-coverage-obligations). A statically known pair chooses its operation directly. If the concrete pair is determined only during execution, prepare the required local type-based selector and covered operation/failure paths. Preparing a possible failing branch does not prove the actual call will fail: an unchecked mismatch selected by runtime evidence remains a runtime failure. Conversely, an actually selected, statically impossible call cannot be hidden in a generic wrapper. Source checking and shared/specialized native emission must preserve that distinction rather than change the result through optional optimizer cloning.

No whole callable body per numeric pair, owner instance, argument value or recursion depth is required. The current primitive set is finite; [numeric classification](../cod/lang_syntax.hh) and [native widening](../cod/lang_llvm.hh) already provide reusable infrastructure. The explicit lossy paths, expression syntax, diagnostics, assertion evidence and predicate-flow integration still need implementation. Unknown concrete types retain D-26's existing prepared-interpretation boundary; no hidden JIT or universal runtime type converter is implied. This is a feasibility assessment, not measured implementation cost or a claim that the feature is already supported.

**Static resolution (H-470).** CoD rejects a provably false nonnumeric type assertion rather than retaining an executable unconditional failure as Julia permits. Proven success removes validation while preserving operand effects. The separately confirmed pure numeric warning/UB policy still applies even to a known lossy numeric constant; H-470 does not turn that warning into an unconditional hard error.

Type grammar must distinguish `f(value : T)` from `f(value : T -> parameter)` and preserve callable arrows within types. Detailed type-valued expression staging, broader view/callable admission and proof algorithms remain under review; the generic operation direction and explicit non-null/static-resolution choices are selected. D-16 remains unchanged.

### Predicate assertions and flow refinement

H-469 selects the source form:

```cod
assert predicate;
assert predicate, "error text";
```

`assert` is a semantic checking construct, with a bool predicate and an optional quoted diagnostic literal. The initial message form needs no runtime string type, interpolation or message-producing call. Broader message expressions are not selected. It completes with void on success and creates no new lexical/effect scope. The parser must respect nested expression grammar before the optional top-level comma; `is` retains reference identity semantics and does not become a type-test operator.

Apply H-470's static-resolution principle: prove true and remove validation, prove false and report a compile-time error, otherwise evaluate the predicate once and throw an unchecked assertion failure if false. H-472 names the predicate failure **AssertionFailedError**, distinct from TypeAssertionFailedError for a type mismatch. A false author assertion is a program-author responsibility; throwing this error is correct language execution, not a CoD soundness defect. Predicate evaluation retains its ordinary effects and possible checked failures; the assertion's own unchecked category does not erase them. Even when the result is proven, observable evaluation cannot simply disappear. Optional literal text is diagnostic information, not a payload read or extra business effect.

Assertions remain semantically enforced in optimized builds. A debug-style flag must not erase a check whose successful continuation supplies type/nullability facts. H-448's bounds opt-out and numeric warning settings do not disable assertions. Ordinary exception propagation and defer/finally apply, with no rollback of earlier effects.

Successful normal continuation gains facts justified by the predicate, using the same sound flow reasoning as a true conditional branch. Initially support recognized predicates and common stable bindings rather than requiring a general theorem prover. Unknown proofs do not grant facts; the check may still run, while a later operation requiring an unproved property must follow its existing checking rules. In particular:

```cod
proc checked_node_count(ptr p:Entity in O) -> u64 where {owner O} {
    assert p is not null, "node required";
    ref n = p : Node in O;   # Non-null proof already exists; check Node membership.
    return n.count;
}
```

The final `is not null` test reads reference identity without dereferencing its target. A successful check refines the stable selected binding; it does not change its declared ptr type, owner, access, lifetime or payload-initialization state. Rebinding p or a possible alias write invalidates the applicable fact. Testing a mutable field does not prove that a later reload after intervening mutation still has that property; retain a selected local view when needed. Existing proof of non-nullness, including a branch fact or ref input, also suffices without a redundant assertion.

A fact exists only on paths where the assertion succeeds. Catching its failure and continuing does not establish it; a join requires the fact on every reaching normal path. Cleanup that rebinds a relevant slot invalidates it normally. Assertions cannot initialize unread payload, recover expired owners or bypass readonly constraints. Operand dereferences before the final predicate still have ordinary NPE obligations; an assertion is not retrospective protection for them.

**Acceptance:** both spellings; bool-only predicates; static success/failure; unresolved false predicates with optional diagnostic text and ordinary cleanup; once-only evaluation and retained effects; non-null ref use after success; rejection without a fact or after rebinding; no fact after a handled failure; no assertion-elision flag; unchanged owner/access/init, checked operand failures and numeric policy. Source inspection finds no assert handling in the current parser/checker, so this section is a plan, not implemented support. Complete grammar, proof coverage and diagnostic representation remain engineering work within the stated boundary.

### Semicolon omission and syntax UX

[H-SEMICOLON-UX-20261117](../CHANGELOG.md#h-semicolon-ux-20261117) establishes the general principle: **allow semicolon omission wherever practical, and warn when a valid spelling is easy to write with an unintended meaning.** Such a warning should explain the selected interpretation and suggest a concrete, less error-prone spelling. Fine-grained omission and diagnostic cases are ongoing language UX work; this principle does not require an exhaustive automatic-semicolon-insertion system before useful cases can be supported.

Keep explicit semicolons available to make boundaries clear. Routine omission is not itself a reason to warn. A real syntax ambiguity or invalid program still needs an ordinary error; a warning cannot grant a missing type, owner, lifetime or effect permission. Parser interpretation is deterministic and does not retry a different statement split based on overload success, runtime behavior or a guess about the author's intention. Diagnostics do not change that interpretation. Existing opt-in warning-as-error policy remains applicable; no new mandatory diagnostic switch is selected.

A current review case is the distinction between a trailing callback and a separate immediate full scope:

```cod
# Assume run has a defaulted niladic callback, so either intent is expressible.
run()
{{ work(); }}

# Explicit boundary when two successive operations are intended:
run();
{{ work(); }}
```

Where the grammar attaches the first block as a callback, a suspicious presentation can warrant a warning that the block is passed to run and is invoked only under run's behavior; inserting `;` states the independent-scope intent. An intentionally formatted multiline callback must not warn merely because a newline precedes its block. Exact warning triggers and safe formatter output need focused examples and incremental refinement; indentation is not proof of intent.

Other review cases include return/throw operands across lines, call/index/operator continuations, comments and blank lines, and separators in callback headers or receiving specifications. Preserve D-16's reserved fraction `//` and `#` line comments. No newline rule, header-terminator omission, precedence change, mandatory warning catalogue or formatting algorithm is selected by this general principle. In particular, the explicit `;` after a trailing callback's parameter header remains an unambiguous spelling; any omission needs a defined header/body boundary, not a parser guess.

Acceptance of incremental grammar/diagnostic work should show both the admitted omission and its deterministic meaning, meaningful warnings with valid suggested rewrites for the targeted trap, and ordinary omission/multiline cases without unnecessary warnings. Preserve selected argument order, first-slot callback binding, local returns, captures and cleanup. Current compiler support must be reported separately; this source principle alone does not imply that every illustrated omission is already parsed or diagnosed.

### Trailing callback arguments

A generator declaration with `yields` supplies the implicit first here parameter for a single-brace call. Apply the slot-one rules below to that protocol, keeping the written ordinary parameters, names and defaults in their original positions. A callback-first generator-compatible declaration exposes the same body to a `for` header. [Generalized for-loops](RangedForAndGenerators.md#generalized-for-loops)


A [single-brace here body](CallableStorageAndHereAccess.md#single-brace-bodies-for-a-here-callable) sends an outward return or escaping exception to its lexical supplier. Write the trailing group directly after the call. Within the group, `continue expr;` supplies feedback to the current body invocation. A `break expr;` supplies the whole originating call’s result, even if another function invoked the forwarded formal.

A first parameter declared as a [here callable](CallableStorageAndHereAccess.md#an-effect-in-a-here-callback) accepts either brace form. Forwarding to another synchronous here formal preserves the supplied body, its authority and its control destinations.

[H-TRAILING-CALLBACK-20261117](../CHANGELOG.md#h-trailing-callback-20261117) selects a trailing callable body using double braces, with parameters introduced by an initial `=>` and terminated by `;`:

```cod
proc apply(here body(x:u64) -> u64, value:u64) -> u64 {
    return body(value);
}
apply(7u64) {{=> x;
    return x + 1u64;
}};                                      # 8.
apply(7u64 -> value) {{=> x;
    return x + 1u64;
}};                                      # Same binding.

proc invoke(here body() -> u64) -> u64 { return body(); }
invoke() {{ return 9u64; }};               # Niladic shorthand, result 9.
```

The callback is the **first declared parameter**, as in [Julia's do-notation](https://docs.julialang.org/en/v1/manual/functions/#Do-Block-Syntax-for-Function-Arguments). CoD retains its own return, lexical access and lifetime rules. Bind the trailing callable to that first slot before binding the written argument list. Positional arguments then start at the second declared slot and otherwise preserve ordinary order, including the positional-prefix/named-suffix rule. Named arguments still bind their declared names. The example's mapping is `apply(callback, 7u64)`; this describes parameter destinations, not a new evaluation order.

There is no search for a callback parameter later in the signature. A non-callable first parameter, an explicitly supplied value/default request for the first parameter, or another missing required argument rejects. A declared default on the first parameter is suppressed by this explicitly supplied callback. Defaults on remaining omitted slots retain their existing rules. Calls without a trailing block keep ordinary positional binding. The callback construction is outside the parenthesized argument list, so it does not violate that list's ban on a positional argument after a named argument. The bounded form supplies one trailing callback; use explicit named callable arguments for other callbacks.

**Header and expected type.** `{{=> x, index; statements... }}` binds callback parameters in order. A header is distinct from executable body expressions; `=>` is recognized only in that leading position, and an explicit semicolon ends the header. The general semicolon-omission principle above applies to future refinement of this boundary; it does not yet select a replacement delimiter or a newline rule. No header means zero parameters, not an implicit variadic or ignored-argument callback. Parameter names are local to the callback. The independently selected expected callable signature supplies each parameter's type, val/ref/ptr/init/place mode, owner/access constraints, func/proc kind, result contract and failure requirements. This is contextual declaration checking, not type assertion or a new operation on the callback's parameters. Any explicit ordinary parameter annotations must satisfy that contract; they cannot erase owner/access/init obligations. No default expressions are authored by this anonymous header.

The selected callable **parameter type** supplies the callback's `throws` contract. Every checked obligation that escapes the callback body must be covered by it. The body may handle additional failures locally or use fewer than the signature permits. An outer catch around the call, or the receiving procedure's own `throws`, cannot grant the callback a larger interface. The procedure that invokes the callback handles or declares the callback signature's checked obligations as for any ordinary call.

```cod
using "std:exceptions" NullPointerException;

proc invoke(here body() -> void throws NullPointerException) -> void
    throws NullPointerException { body(); }
proc quiet(here body() -> void) -> void { body(); }

proc use(val e:NullPointerException) -> void throws NullPointerException {
    invoke() {{ throw e; }};   # Accepted by invoke's callback signature.
    quiet() {{ try { throw e; } catch (caught:const NullPointerException) {} }};
    # quiet() {{ throw e; }};  # Rejected even though use declares this failure.
}
```

A callback written at the call site therefore keeps its expected signature. Its body cannot widen that signature, rank overload candidates, or narrow the obligations of a call through that signature. Standalone double braces use the [inferred checked interface](ExceptionControl.md#checked-failures-of-standalone-full-scopes).

The initial shorthand covers a single established fixed-arity callback signature, with a header naming all its parameters. It does not synthesize multiple methods for `typeof` a callable family, resolve a circular generic result, or reinterpret a pack as several fixed parameters. Richer signatures can use an ordinary explicitly declared callable where compatible; no new method-coverage, variance, pure or parameter-default policy follows. Structural callback results use the ordinary declared result schema and explicit named return form.

**Selection and evaluation.** Bind/check the candidate declaration shapes and independently available written-argument evidence before checking the callback body against one established expected signature. Header arity is syntactic evidence, but do not trial-check its return expressions, outer-slot uses or effects to choose between incompatible expected signatures. If that information leaves ambiguity, reject and require an explicit callable contract or named adapter. Body checking cannot retroactively repair overload selection. A literal written in a func retains func permissions. A proc-authored here body may do procedural work.

At runtime evaluate the callee and written value arguments in their existing order, associate the trailing here body with the supplying activation at its written position, then prepare caller-context defaults and enter the callee. The body runs only on invocation. It may run repeatedly or be forwarded synchronously. Deferred use must still fit the original activation and all required owner lifetimes.

Every return belonging to a double-brace callback is local. Each invocation has its own locals, cleanup, owner and return state. Ordinary callback-call propagation and catches apply. A required value uses explicit return. Both brace forms access the live enclosing slots with their actual owners.

**Parsing boundary and acceptance.** The attached block is part of one call expression, not an immediately executed expression after an incomplete call. A failed callback match is a source error, never a fallback to executing a separate scope. `apply(...); {{ ... }}` explicitly separates the call and an immediate full scope. Enclosing expression/control grammar determines attachment; overload lookup must not reclassify a control statement's body. Complete parser integration remains engineering work.

Accept niladic and parameterized examples, local typed returns, named remaining arguments, skipped defaults and ordinary borrowed-owner use; preserve written preparation order and zero/multiple callback invocation behavior. Reject duplicate first-slot bindings, misplaced callback parameters, arity/type/access/owner mismatches, unresolved overload expectations, forbidden effects and escaping borrows. This is a selected source direction and bounded derived contract; complete syntax/checker/native/value-path qualification remains pending, and D-18's qualification boundaries are preserved.

### Named arguments, defaults and complete binding

H-451 selects a positional prefix followed by named arguments in any order. **No positional argument may follow the first named argument.** Names bind against the declaration signature. Every required parameter must be supplied; a missing parameter with a declared default is completed by that default. Omission does not mean partial application.

H-457 selects the expression-first named form `expression -> parameter`; H-475 adds `-> parameter` to request that parameter's declared default explicitly. The destination is a parameter name in the declaration, not an expression or caller variable; the arrow does not copy, rebind or transfer ownership. The declared parameter mode still controls initialization. A top-level argument-label arrow binds the complete preceding argument expression (and any explicit spread), independently of overload lookup. `=` and `<-` inside that expression retain their ordinary operations. `:` retains expression typing. Exact interaction with all compound expressions/types still needs parser acceptance; do not recognize labels by scanning raw text without respecting nested grammar.

```text
# Default-block placement follows the selected direction; full signature grammar is pending.
func adjust(value:u64, scale:u64 { 2u64 }, bias:u64 { 0u64 }) -> u64 {
    return value * scale + bias;
}
adjust(5u64, 3u64 -> bias);                  # value=5, scale defaults to 2, bias=3
adjust(3u64 -> bias, 5u64 -> value);          # same bindings, named order unrestricted
adjust(4u64 -> scale, 3u64 -> bias, 5u64 -> value);
adjust(3u64 -> bias);                       # reject: required value is missing
adjust(5u64 -> value, 3u64);                 # reject: positional after named
adjust(5u64, 6u64 -> value);                 # reject: value supplied twice
adjust(5u64, 3u64 -> unknown);               # reject: unknown parameter
```

Ordinary positional arguments fill leading declaration slots in order without skipping optional parameters. The explicitly selected trailing-callback form above first fills slot one with its callback, so its written positional arguments start at slot two. Named arguments fill matching unfilled slots. Duplicate names, supplying a slot positionally and by name, unknown names and missing required slots reject that candidate binding. A call needs an applicable unambiguous declaration/method; do not run operands or defaults speculatively to try candidates. An explicitly supplied value suppresses its default; explicit null is still an argument with ordinary owner/nullability checks. An explicit default request instead uses the declared initializer as specified below. A required parameter may follow a default-bearing one because named binding can complete it. Ordinary omission needs no positional hole marker; `-> parameter` is an explicit named default request, not a positional placeholder. These rules apply to both functions and procedures.

**Selection precedes default analysis (H-477).** Select a unique overload using the declared signatures, supplied argument types and binding forms, declared constraints, and default-presence/count metadata under H-474/H-475. Do not analyze a default expression to infer candidate types, resolve its free names, check its effects or decide whether a competing candidate is applicable. An omitted slot or `-> parameter` needs a declared default; checking that the recipe exists is not analysis of its contents.

Parsing records default syntax and its definition/caller lookup context. H-456's definition-site symbolic identities remain tied to that context; deferred semantic analysis cannot rebind them to caller identities. Only after unique overload selection, analyze the defaults needed for that call and complete their type/owner/access/effect and generic obligations. This analysis precedes execution; H-458 separately governs when the checked defaults actually run.

A generic variable needed only by defaulted parameters may remain unresolved during selection when the declared signature and supplied evidence still establish a unique overload. H-476 can then infer it from the selected default. If selecting among competing signatures would require inspecting their defaults, reject the unresolved/ambiguous call instead of inspecting those expressions. Ordinary rejection from supplied-argument or declared-signature constraints remains; unknown default contents cannot supply an applicability filter or a ranking advantage.

**Default-count tie-break (H-474/H-475).** After binding supplied values and explicit default requests to each applicable declaration, apply ordinary type matching and specificity first. Only otherwise tied candidates are compared by how many formal parameters would be supplied by declared defaults: fewer is preferred. The fixed-form preference and default-count preference do not override one another: a fixed candidate using more defaults than an otherwise equally matched implicit-pack candidate remains ambiguous under H-475. If only one preference distinguishes the candidates, or both agree, retain that preference. A unique surviving candidate is required; remaining equality or crossed preferences reject without a declaration-order tie-break. A better type match cannot lose merely because it needs defaults, and fewer defaults cannot make an inapplicable candidate applicable or resolve incomparable type constraints.

Count used default slots, not how many defaults the declaration offers. An explicitly supplied argument counts as supplied even if its value equals the default. An explicit `-> parameter` request still counts as one used default, exactly as omission does. Definition-environment and caller-context defaults each count once per defaulted parameter; named-argument order, expression cost and constant folding do not change the count. Implicit variadic collection is not a declared parameter-default expression. Evaluate defaults only after selecting the candidate; do not execute them, inspect their runtime results or speculate their effects to choose an overload. H-476 permits type evidence from the selected overload's used defaults as described below. H-477 forbids obtaining that evidence during overload selection, even without executing the default; post-selection errors are diagnostics, not fallback selection.

```cod
func pick(x:u64) -> u64 { return 1u64; }
func pick(x:u64, y:u64 { 10u64 }) -> u64 { return 2u64; }
pick(7u64);    # Equal type match; zero defaults beats one, result 1.

func exact(x:u64) -> u64 { return 1u64; }
func exact(x:u32, y:u64 { 10u64 }) -> u64 { return 2u64; }
exact(7u32);   # Exact u32 beats widening to u64, despite using one default.

func tied(x:u64, y:u32 { 0u32 }) -> u64 { return 1u64; }
func tied(x:u64, z:bool { false }) -> u64 { return 2u64; }
tied(7u64);    # Same explicit match and one default each: ambiguous.
```

**Explicit choice when preferences cross (H-475).** The caller supplies enough binding information to select the intended form:

```cod
func mixed(x:u64, y:u64 { 0u64 }) -> u64 { return 1u64; }
func mixed(x:u64, rest:u64...) -> u64 { return 2u64; }
mixed(7u64);          # Reject: fixed form uses one default; pack form uses none.
mixed(7u64, -> y);    # Request y's declared default: fixed form, result 1.
mixed(7u64, []...);   # Explicit empty u64 pack: variadic form, result 2.
```

The default request names a parameter that must exist and have a declared default in each applicable candidate. It therefore excludes the variadic declaration above, which has no y parameter. Explicit expansion excludes the fixed declaration even for an empty or statically known-length array; no length-based flattening or fixed-arity selection is introduced. Neither spelling forces a choice when several candidates still satisfy it: ordinary type ranking and ambiguity checks continue.

**Explicit named default request.** `-> parameter` is allowed only as a call-argument form. It contributes no value expression and is not a standalone default value, null, callable-owner projection or type witness. The destination binds to the declaration/origin contract, including through `typeof origin`; a target implementation's private names/defaults cannot satisfy it. Require a declared default, not merely an implicitly empty variadic tail.

A request starts the named-argument portion, so no positional value or positional expansion may follow it. Named values and default requests may be reordered within that portion. A slot may be occupied once: a repeated request, a request plus a supplied value, or a request for a positionally supplied parameter rejects. Unknown names and parameters without defaults reject that candidate. All remaining required slots still need bindings; a request is not partial application.

```cod
adjust(5u64, -> scale, 3u64 -> bias); # scale uses its declared default.
adjust(5u64, -> scale, -> bias);     # Both defaults run in their normal phases.
adjust(-> scale, 5u64);             # Reject: positional argument after named.
adjust(5u64, -> value);             # Reject: value already supplied/no default.
adjust(5u64, -> scale, 3u64 -> scale); # Reject: duplicate binding for scale.
```

Request position does not move or duplicate evaluation. Explicit value expressions retain written order. Requested caller-context defaults run with omitted caller-context defaults in declaration order before entry; requested definition-context defaults run in the callee preamble in declaration order. Existing ownership-transfer, checked-failure, deferred-call and cleanup boundaries remain unchanged. Count a requested default as used even when it folds to a constant; do not execute losing candidates' defaults.

**Generic inference from defaults (H-476/H-477).** After unique overload selection, a default used by omission or explicit `-> parameter` may supply its independently established expression type to complete generic parameter inference. This is the same parameter-initialization contract in both cases:

```cod
func seed(x:T { 0u64 }) -> T where {T} {
    return x;
}
seed();          # T = u64 from the used default's established type.
seed(-> x);      # Same evidence and result contract.
seed(7u32);      # T = u32 from the supplied value; the default is unused.
```

Declaration-directed binding determines which slots use defaults without analyzing the expressions. After selecting the overload, establish those default types and combine them with supplied-argument evidence under existing generic constraints and permitted conversions. A used default cannot override contradictory established evidence or invent a conversion, owner, access permission or common type. A skipped default contributes no call-specific inference evidence; ordinary declaration validity still applies. Inference does not cache a default value or add a default capture to an adapted callable.

Type evidence can come from a typed literal, an already typed binding or an independently established callable result contract; it need not be a constant expression. Inspect that evidence only after selection, without invoking helpers, reading mutable payload to discover its value, allocating default storage or performing effects. Defaults execute once only after selection, at H-458's caller-preparation or callee-preamble phase. A call with an effectful default still has that default's ordinary effect and checked-failure obligations when it is used.

Apply H-454–H-456's environments during type checking as well as evaluation. Definition-default names keep their definition bindings and permitted earlier-parameter evidence; caller-default names use the actual caller context with definition-resolved symbolic identities. Through `typeof origin`, use the origin contract's default evidence, not target-local defaults. Use only the selected overload's default evidence; a competing declaration's default cannot fill missing type information. The chosen method remains fixed: completing inference does not rerun type specificity or H-474/H-475's comparisons, and an error cannot cause another candidate to be tried.

Inference must have an independent basis. Assuming T in order to type a default and then citing that assumed type as proof of T is circular. Unresolved cycles or missing required type evidence reject; provide an independently typed input or declaration instead. [D-24](CallableResultInference.md)'s established result contracts and [D-26](AheadOfTimeExecutionPreparation.md#generic-and-recursive-native-coverage)'s prepared coverage remain dependencies; this decision neither executes a recursive search for values nor selects a new solver, runtime type fallback or native implementation per substitution. Empty packs and contextual null retain their own type/owner-evidence requirements; H-476 cannot manufacture an element or owner from absence.

**Ambiguity and selected-default errors (H-477).** Default contents cannot remove ambiguity. An unavailable name, invalid expression, type/owner/effect mismatch or failed generic constraint discovered while analyzing the selected default rejects that call; it does not discard the selected method and restart overload selection.

```cod
func choose(x:u64, y:u32 {{ return fallback; }}) -> u64 { return 1u64; }
func choose(x:u64, z:bool { false }) -> u64 { return 2u64; }
# Caller has no local named fallback.
choose(7u64);         # Ambiguous: equal supplied matches and one default each.
choose(7u64, -> y);   # Select first; then diagnose unavailable fallback.
choose(7u64, -> z);   # Select second; analyze its false default, result 2.
```

The ambiguous call does not analyze either default. The explicit z call does not inspect the excluded y default. A declared default's invalid contents are distinct from the absence of a default for a required slot, which declaration-directed binding can diagnose before selection. No silent fallback, default execution during checking or new diagnostic-severity option is introduced.

**Captured-owner projection.** [D-18](CallableViewsAndCaptures.md#owner-projection) uses `cb -> state` to borrow an associated owner. H-457 supplies the explicit grouped form when this is an argument value:

```text
use((cb -> state));          # positional borrowed region
use((cb -> state) -> arena); # same borrowed region, named parameter arena
use(cb -> state);            # named parameter state receives cb
```

The final line cannot fall back to owner projection when no parameter named state matches; it is a failed named binding. This is a deliberate call-argument grammar distinction, not a change to owner projection outside that position. Parentheses around the borrowed region do not apply the scalar grouping-to-val proposal: region identity, permissions, owner and lifetime remain intact; no owning result or retention is created. Scalar grouping elsewhere keeps its own category consequences. An explicit `values... -> rest` names the one variadic pack; it does not flatten elements into fixed slots or make runtime count a dispatch dimension. Broader pack composition stays under its existing review boundary.

**Named declaration origin.** H-459 permits parameter-default expressions only on named callable declarations with an identifiable declaration origin. Merely binding an anonymous callable value to a variable does not authorize a new default declaration. Inline callable types used as parameter annotations cannot introduce default expressions; neither can anonymous callable expressions introduce them independently. A type-position `typeof named_callable` may retain that named declaration's parameter names/default contract. It does not evaluate the callable, its body or a default, nor does it create a new default-definition site.

```text
# Design syntax; callable-contract typeof integration remains implementation work.
func original(n:u64 { 1u64 }) -> u64 { return n; }
proc run(f:typeof original) -> u64 { return f(); }
run(original); # inherits original's n/default contract, so the result is 1u64

# Rejected: an inline callable type invents a new default expression.
proc invalid(f:func(value:u64 { 9u64 })->u64) -> u64 { return f(); }
```

The named declaration remains the origin of the default expression and its definition-bound identities through typeof, imports and identity-preserving aliases. `{ ... }` keeps that original definition environment; the type annotation's lexical site does not replace it. `{{ ... }}` still evaluates in the caller context of the actual omitted-argument invocation, with H-456's symbolic identities resolved at the original named declaration. For `f()` inside run, run is that immediate caller; the outer call passing f is not the default's call site. The original default's capture/owner/access/effect requirements must remain satisfiable; typeof cannot manufacture a runtime environment or extend its lifetime.

A plain structural type such as `func(u64)->u64` supplies no default contract, so an omitted argument through that view rejects. The optimizer knowing a particular target does not restore erased defaults or names. Whether parameter names without default expressions may be written in an inline callable type is a separate surface question; H-459 restricts default authorship. Preserving or erasing the inherited contract during further type conversions remains under review. H-462 retains a multi-method named origin as a complete callable contract rather than requiring typeof to select one overload. H-460's compatible-target direction and H-461's type-bound default-expression rule are stated below. A borrowed default from a different owner's environment cannot be made valid merely by matching the underlying parameter types.

**Compatible target identities.** H-460 permits the ordinary case where a different callable satisfies the inherited contract and the default's required environment is established. `typeof original` does not require that every target be original itself. The view retains original's public parameter names and defaults, while the supplied target provides the body. Map corresponding parameters by their declaration slots; a target's local names do not rename the view's inputs. Its own defaults are not used when the inherited contract supplies that argument. Do not invoke the original callable's body merely to obtain a default.

```text
func original(n:u64 { 1u64 }) -> u64 { return n; }
func other(x:u64 { 99u64 }) -> u64 { return x + 10u64; }
proc run(f:typeof original) -> u64 { return f(); }
run(other); # 11u64: original's default, other's body
other();   # 109u64: direct call uses other's own default
```

Compatibility must hold for the view's admitted calls, including explicitly supplied arguments, not just one default-filled example. Preserve callable kind, accepted parameter/result contracts, modes, owner/access/nullability/init obligations, checked exceptions and effects under existing compatibility rules. Inherited defaults retain their own failure and dependency obligations even if the target body is simpler; skipped target defaults cannot supply missing guarantees or add effects to this call. H-458's one logical invocation boundary governs preparation, preamble, body and cleanup; a physical adapter cannot duplicate initialization, default effects or ownership transfer. A target cannot borrow a default's owner or erase a requirement solely because scalar types match.

**Multi-method contract completeness.** H-462 requires an argument declared `f:typeof origin` to implement **all method signatures** of the selected origin family in its selected interpretation. The obligation is checked when admitting the argument under that contract; it is not reduced to the overloads currently called in the receiver's body, reachable after optimization or exercised by one test. A partial implementation rejects even when the present receiver only invokes a supported method.

```text
func origin(n:u64) -> u64 { return n; }
func origin(flag:bool) -> bool { return flag; }
proc consume(f:typeof origin) -> u64 { return f(7u64); }

proc caller() -> u64 {
    # Design example: one local named callable family with two methods.
    func adapted(n:u64) -> u64 { return n + 1u64; }
    func adapted(flag:bool) -> bool { return not flag; }
    return consume(adapted);
}
# Removing adapted(bool) leaves the required bool contract uncovered and rejects,
# even though consume currently calls only f(u64).
```

Each required signature includes its callable-kind, input/result, passing/access, owner, initialization, generic/variadic and exception obligations under the existing compatibility rules. Correspondence preserves the origin's named/default contract and maps target parameters by slot. Coverage concerns semantic behavior under ordinary applicability and specificity: a compatible generic implementation may cover multiple required signatures when that can be proved; it need not duplicate their source bodies. Existing origin overload selection determines the required contract for overlapping domains, rather than treating mutually overriding method signatures as simultaneously selected bodies.

A target may contain additional methods, but they do not widen the calls admitted through the origin view. Check the actual selected target method for each required call domain: an extra overload cannot introduce ambiguity or intercept a required case with an incompatible result, permission, owner or failure contract. A merely present compatible method is insufficient if ordinary dispatch would select an incompatible one. Unknown coverage remains an explicit proof/implementation gap; it cannot be silently accepted because the current caller happens not to exercise that case. Detailed generic/domain proofs remain under D-26's bounded coverage process.

The caller may define a local named callable to adapt available operations explicitly and pass that family. It can forward each required signature, perform already permitted conversions, or implement an appropriate generic method. Its bodies must satisfy the same owner, capture, effect and cleanup rules; an adapter does not waive incompatibilities. Local multi-method declaration grouping/spelling is implementation and grammar work: the example denotes one callable binding with several methods, not repeated ordinary local-variable bindings or authorization for arbitrary same-scope duplicates. Source/target default expressions stay in their declared type contracts; adaptation does not duplicate them as value captures.

This whole-family compatibility obligation is distinct from [D-21's semantic-method/native-implementation hierarchy](AheadOfTimeExecutionPreparation.md#callable-method-and-native-implementation-hierarchy). It does not demand one native body per origin signature, forbid code sharing or require emitting dead code. Code elimination cannot weaken source compatibility. Defaults and named arguments still need an applicable unambiguous origin signature at each call. Apply H-474's default-count rule to that origin contract; whole-family completeness supplies no extra ambiguity tie-break. The target's own unused defaults cannot change the origin-view choice. The selected family/world defines the contract, without promising coverage of future unrelated extensions or interpretations.

**Type-bound default expressions.** H-461 places the expression that evaluates a default in its originating definition context in the callable type contract. `typeof origin` retains that expression, its resolved declaration identities and its context/dependency association. It does not evaluate fallback early or turn its current bits into a saved default value. The supplied callable value still supplies its selected implementation; adapting it to the contract does not independently capture fallback into that value.

```text
# values is a live regional i64 array; i selects a target in that array.
val var fallback -> values[i];
func origin(n:i64 { fallback }) -> i64 { return n; }
func target(n:i64) -> i64 { return n + 1i64; }
func adapted:typeof origin = target;
# On omission: the type's default expression reads fallback in origin's context;
# the selected function adds one to that argument.
```

The declared place alias selects fallback's target once. Later evaluation of the type-bound expression reads that target's current contents; it does not rerun `values[i]` with a changed index or resolve fallback as an unrelated caller local. Changing the target's contents before a later omitted call is observable. Explicitly supplying n skips this default expression entirely. Different admitted callable type contracts can give compatible targets different named/default interfaces without adding the defaults to those targets' body capture sets. No call to origin's body is implied.

Type level and evaluation stage are separate under charter §5. Retaining a default recipe and its context association does not require analyzing it before overload selection, eagerly reading runtime contents, adding body captures to each typed callable value or declaring a permanent runtime type ABI. Where the definition context has runtime owner/locator bindings, the compiler must establish the corresponding context evidence when it evaluates the default. That evidence may be shared or supplied through the type/evaluation context; this contract does not require copying a default-provider environment into each adapted value. A descriptor containing only syntax cannot be treated as proof that an expired or unavailable context is valid.

The ordinary owner/access/initialization and lifetime rules therefore apply to the type-bound default dependency and every explicit argument. Returning or forwarding a typed callable cannot silently detach the default from its definition context, keep an expired lexical alias cell, or extend a regional owner's lifetime. Unsupported context transport is an implementation gap and invalid lifetime/access evidence rejects. Physical representation, dependent context instantiation/sharing, exact callable-type equality/erasure and complete proof coverage for the selected call/default rules remain engineering/review work; no per-value default capture or precomputed default snapshot is selected. H-454–H-458 still fixes lookup and execution phases.

**Default environments.** H-454/H-455 distinguishes two parameter-default forms. Each used default runs once when omitted or explicitly requested with `-> parameter`, not once when the callable is defined. Supplying a value skips the default entirely.

| Default form | Environment and scope |
| --- | --- |
| `{ expression }` | Use the callee's definition environment. It is not an extra full lexical scope merely because it is grouped. |
| `{{ return expression; }}` | Immediately invoke an ordinary niladic callable in the caller context. Its explicit return supplies the default value. An ordinary free local name resolves in that caller context, with direct access to the caller’s live slots. |

H-456 recommends that caller-context defaults depend on **symbolic variables rather than ordinary caller-local names**. This is guidance, not a prohibition of H-455's caller-local lookup. Resolve a symbolic reference's identity at the callee definition site; read that identity's current binding in the caller context. A caller's unrelated same-spelled symbol or local cannot retarget that symbolic reference. Ordinary symbol availability, owner/access/readiness and effect permissions still apply; there is no fallback to another identity or captured value.

```text
symbol @step:u64;
proc next(n:u64 {{ return @step; }}) -> u64 { return n + 1u64; }
# The default retains this definition's @step identity.
# If its active caller binding holds 4u64, next() receives n=4u64.
# Another symbol also spelled @step is a different identity.
```

These environments are specific to parameter defaults. A caller-default return is local to the double-brace invocation. Its accesses use the caller’s current slots. A type query checks the dependency without executing the default.

**Parameter initialization and phase order.** H-452's callable preamble initializes parameter bindings in declaration order in the callable's inherent lifetime. Each explicit expression/conversion runs once in call-site written order before entry, irrespective of its named destination. Value/ref/ptr/owned parameter modes and ordinary initializer-before-binding lookup remain; initializing an init-reference binding does not discharge its referent's write obligation. H-458 selects the following execution phases:

1. Prepare explicit value arguments in written order, then omitted or explicitly requested caller-context defaults in declaration order within that subset.
2. Enter the selected callable, transferring all supplied owning inputs under D-17. Initialize parameters in declaration order; use prepared inputs or evaluate an omitted or explicitly requested definition-environment default in that parameter's preamble initializer.
3. Start the body only when parameter binding initialization succeeds. Apply ordinary effects, readiness and cleanup rules throughout.

This order preserves earlier-parameter visibility for definition-environment initializers and gives caller-defaults a pre-entry preparation boundary. In `proc f(a:u64 { mark(1u64) }, b:u64 {{ return mark(2u64); }})`, `f()` evaluates b's caller-default before a's callee-default: the observed order is 2, then 1. Declaration order applies within each phase; there is no single declaration-order evaluation across the caller/callee boundary. Caller-context free names keep H-455/H-456 lookup rather than becoming callee parameter bindings.

Caller-default failure precedes input ownership transfer; callee-default failure follows entry and cleans all transferred owned inputs, even those not yet installed in their named bindings. No rollback of earlier valid effects/allocation or ownership restoration is implied. Under existing D-17 prepared-call rules, plain deferred calls prepare caller-defaults at registration and run callee-defaults at invocation. A preparation failure registers no partial call; borrowed prepared results must remain valid until invocation. Owning-input responsibility still transfers only at actual callee entry. Complete implementation and proof integration remain outstanding.

**Further review:** complete block grammar; helper/type-name hygiene and dependency checks in caller-defaults; public declaration versus implementing-method parameter names; conflicting defaults across declarations; type-bound default context transport and explicit contract erasure; and broader named pack composition. H-474/H-475 settles type-first comparison, fewer defaults, unopposed fixed-form preference, rejection of crossed preferences, and explicit named-default/empty-pack selection; this ranking is no longer an open choice. H-476/H-477 admits established type evidence from used defaults only after unique overload selection, with circular/unresolved inference rejected; default analysis never filters competing candidates and a selected-default error never retries selection. The declaration is the public naming/default contract; implementation-local names cannot change it. Default effects and checked failures stay visible through the selected contract. D-16 remains unchanged.

**Acceptance:** complete reordered bindings and default omission; reject positional-after-named, duplicate, unknown and missing-required cases; preserve written explicit evaluation/conversion order and mode/owner/access/init checks; distinguish named arrows from grouped borrowed-owner projection; evaluate defaults once per omission; observe definition-versus-caller locals and fixed symbolic identity with caller-current values; reject escaping default-local views and invalid effect use. Acceptance also requires mixed defaults observing caller-before-callee order, pre-entry versus post-entry failure/ownership cleanup, and registration-versus-invocation defaults in deferred calls. Acceptance also requires named-only default authorship, rejection of defaults in inline callable types/anonymous declarations, unevaluated typeof with retained declaration origin, unchanged definition-site symbolic identities, actual-invocation caller lookup, and rejection of omission through a plain structural view. Acceptance includes compatible targets with different local names/defaults, original-default versus direct-target behavior, full-signature obligations and rejection of missing default-context evidence. Acceptance also requires live reads through the original alias without precomputing its value or retargeting it after index changes, unchanged target-body captures, and preserved context/lifetime obligations on forwarding. Acceptance further requires complete multi-method coverage independent of the receiver's current uses, rejection of a missing signature or an incompatible/ambiguous intercepting overload, preservation of origin defaults/names, and valid caller-local adapters. Acceptance also covers zero-versus-one default on equal type matches, exact matching with a default beating widening without one, equal-count ambiguity, named binding before counting, and no evaluation of losing defaults. H-475 acceptance adds crossed-preference rejection, explicit default and empty-pack selection, default-count/phase preservation for requests, and rejection of unknown/required/duplicate requests, positional-after-request and unresolved empty-pack type/owner evidence. H-476/H-477 acceptance includes unique selection before omitted/requested-default inference, supplied-value inference with skipped defaults, established helper-result evidence without execution, preserved context/origin and phases, and rejection of contradictory or unanchored evidence. It also requires ambiguity without analyzing competing defaults, a missing-name diagnostic only for the selected needed default, no inspection of an excluded default, and no retry after selected-default failure. Physical type-context transport, local family grammar and detailed inference procedures remain under review; selected behavior still requires implementation and proof coverage. Current source does not implement named arguments, parameter-default blocks or expression-colon typing.

### Evaluated bits and storage places

H-257 directs the compiler model toward LLVM register optimization. An evaluated bits value is an SSA value; a storage place denotes a typed address in a tracked owner. Reading a place produces the value at that point. Writing the place does not retroactively change that SSA value. A retained regional reference permits a subsequent read to observe intervening writes. Const non-bits access therefore does not imply immutable underlying memory.

LLVM lowering uses typed places for referent, field and element access, with common read/write operations. Regional aggregate views retain their offsets and owners; relative reference fields keep their existing encoding. Bits operations and variadic argument selection do not require artificial memory cells. This internal separation does not introduce `owned`/`const`/`make` source syntax, change the assignment-result contract, extend owner lifetime or select RTTI. The complete ordinary-region lifecycle is the separately deferred fourth work package.

### Direct symbolic procedure declaration

```text
proc @p(arg:T) -> RT { ... }
effect @p;  # Name-only activation is under review.
perform @p(argument);
behave @p(argument);
```

The method declaration defines or extends the symbol’s module-world family without a runtime binding. If no symbol exists, its explicit signature can introduce the nominal identity and declared contract under the existing declaration rules; an existing/imported symbol retains its identity and permissions. Declaration promises and compatible view checks remain distinct from method addition.

Bare p names the registered symbol. The combined definition activates and installs a handler. Separate name-only activation of registered definitions is [under review](#definition-installed-effectful-procedures). Neither form supplies a first-class handler or grants functions dynamic access.

The existing `proc(T1, T2) -> RT` view contract remains typed. Several methods do not make a callable value ambiguous: invocation dispatches by actual types within its selected interpretation. The compiler’s present one-method construction guard is an implementation gap, not a restriction of this contract. General view compatibility, concrete-type evidence and grammar migration are tracked in [D-21](AheadOfTimeExecutionPreparation.md).

### Callable context and memory access

The procedure’s name binding, its body’s lexical environment and its use of dynamic effects are separate facts. Registering a body during scanning does not perform captures; its runtime definition semantics still follow the existing scan/incorporation boundary. Nested-definition placement, arbitrary lexical captures and ordinary-name overload collision policy require their own bounded design. No hidden shared capture cell or new capture lifetime follows merely from adding lexical procedure names.

A function uses explicit inputs and may read/edit their reachable memory. It may call functions and use permitted lexical definitions/constants, but cannot read or rebind an ambient @symbol, invoke a procedure through another spelling, or obtain hidden dynamic context. Explicit-argument intrinsics can have external consequences under H-197. No referential-transparency or read-only guarantee follows from the function keyword. Lexical captures must not smuggle dynamic access or an invalid storage lifetime into a function.

Procedure signatures may have explicit arguments and results as well as effect requirements. “Procedure” does not imply a void return. Function-valued arguments require functions; passing a procedure there cannot bypass the effect restriction. Methods need the same callable-kind distinction.

The checker should retain each procedure's required symbol identities and read/write/binding use through calls. This is a proposed implementation of the user's distinction, not a commitment to a general row-polymorphic effect calculus. A small finite requirement set is sufficient for the first examples. Foreign calls need a trusted declaration of their effects; an opaque native call is not presumed pure.

Constructing a description and executing its requested operation remain distinct. A function may construct/edit regional data through explicit resources once the source allocation contract exists. Looking up @region, or creating a procedure that captures current effects, accesses dynamic context and therefore cannot occur inside a function. Context exclusion applies transitively through helpers and foreign intrinsics. The generator yield handoff retains separate lexical caller eligibility, rather than admitting a procedure-valued ordinary callback. Symbolic values still follow active dynamic bindings; yield supplies no implicit capture or TLS snapshot/replay. Nested forwarding and native outlining must preserve the distinct source sites.

The SNN support layer may use functions for numerical work and explicit array edits, with procedures supplying ambient target/frame resources. This callable boundary selects no neural transition or learning algorithm. H-200/H-201's borrowing, local-bits and const-value contract needs its own source implementation; K-311 validates the existing explicit-reference memory subset only.

## Captured values and call behavior

[D-18’s definition-mention contract](CallableViewsAndCaptures.md#definition-mentions-and-runtime-view-construction) separates named definitions from eligible stored callable values. For an installed handler only definition activation captures its required symbolic bindings. Direct calls and type-only queries do not construct views merely by naming a definition. See the [direct-call rule](#definition-installed-effectful-procedures) for ordinary procedures.

Let `D` be the caller's current symbolic bindings, and let the installed handler record for `@p` supply its selected family/interpretation, direct lexical access `L`, and saved bindings `C` indexed by its required symbols. These are semantic components, not a prescribed heap object layout.

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

For a scalar example, evaluate `effect @x <- 1`, activate a handler definition under `@p` whose selected method reads `@x`, then evaluate `effect @x <- 2` in the caller scope:

| Observation | Result |
| --- | --- |
| `perform @p` | 1 |
| `@x` after that call | 2 |
| `behave @p` | 2 |

If the body evaluates `effect @x <- @x + 1` and then reads `@x`, its local binding is restored on exit in **both** call modes. With capture 1 and caller value 7, two `perform @p` calls each observe 2; two `behave @p` calls each observe 8. The caller remains 7 after every call. `behave` omits capture installation; it does not make the body's own scoped writes permanent. Installing a captured scalar does not mutate the stored capture itself.

Scope exit includes the selected language's structured returns and failure/unwind paths. Lowering must arrange restoration on each such path. It does not promise to run cleanup after process termination, or roll back effects performed through referenced objects.

### Regional resource capture preserves owners

Three different facts must remain distinct: the symbol identity used for lookup, the value captured at callable materialization, and the region’s ownership/lifetime. A procedure captures the then-current value of each required dynamic variable under its resolved identity. It does not reread an old variable’s later value as a substitute for that snapshot. For a region-backed resource, the captured value identifies the existing resource; it is not a copy of the object graph or a new independent owner.

Suppose `@work` denotes R1 when the value installed under `@p` is materialized. The caller subsequently shadows `@work` with R2 while both owning scopes remain alive:

| Point | Region reached through `@work` | Ownership and lifetime |
| --- | --- | --- |
| Before `perform @p` | R2 | R1 and R2 retain their respective owners. |
| Inside `perform @p` | Captured R1 | R2 is hidden through this variable only. Its ownership and cleanup responsibility are untouched. R1 also retains its existing owner. |
| After `perform @p` | R2 | The caller binding is restored. Neither region was released by capture replay or shadow removal. |
| Inside `behave @p` from that caller | R2 | The procedure uses current values; its saved R1 is not installed. |

A previous reference or another valid binding to R2 is not invalidated merely because `@work` is shadowed. Mutating R1 through the captured resource changes the live graph; leaving the call restores lookup, not graph contents. Removing the temporary shadow does not free R1 either. Only actual ownership/scope events release resources, and a region newly acquired within the call follows its own owning scope.

Captured/current symbolic lookup must preserve the selected reference value’s established interface facts. Rebinding cannot accumulate the previous value’s guarantees. Evidence propagation adds no runtime tag, reference permission, graph snapshot or lifetime extension.

Captured resources must be valid for their uses. H-180 supplies owner preservation during temporary shadowing; it does not choose a reference-counting system, an implicit lifetime extension for an escaped procedure, or a region-copying snapshot. Materialization-time capture, temporary replay and explicit ownership transfer must have different checking/lowering paths. H-181’s restriction on moving a resource out of a dynamic owner must not reject legitimate captured replay.

### What “uses” includes

Capturing only the symbol spellings appearing directly in a body is insufficient. If `p` executes `behave @q` and `q` reads `@gain`, then `@gain` can be a dynamic dependency of `p` even when the name never appears in `p`'s source. Conversely, a `perform` call can supply a callee's dependencies from that callee's own captures.

The required-symbol analysis must account for dispatch, transitive calls, higher-order arguments, reads/writes and bindings that already supply a value inside the procedure. Native regional snapshot demand follows those same checked uses through copies and recursive results; an unused definition-time binding alone does not force a distinct native context. Final sharing retains every discovered read/touch dependency, including uses propagated through a reused native body. A current-context helper called under perform receives the active captured overlay and its actual invocation bases; equivalence must preserve owner aliases and the effective shared-region groups. Replayed procedure values follow the same native-input proof as callable arguments, including complete image schemas, retained owners and returned-image independence. Captures are taken before later control-flow branches are known; a conditional read is still a potential use unless checking proves it unreachable. A procedure that provides its own binding before every access need not capture an irrelevant outer value.

CoD checks the demanded whole-program execution with its caller paths available, rather than compiling a module against an unknowable future caller. A finite summary or fixed-point analysis is a candidate way to handle recursion and branches; the semantic requirement is not to enumerate an unbounded runtime stack. Missing requirements cannot be hidden behind an empty summary or deferred to an ordinary runtime lookup failure.

### Call-path binding checks

H-102 locates checking at compilation of the demanded call paths. The compiler follows module initialization, callers, scoped bindings, callable selection and invocation mode; declaration text in an isolated module is not the compilation unit.

- For `behave @p`, required effects must be available in the current effective dynamic scope along each reachable path to their use, accounting for bindings established inside the body.
- For `perform @p`, the values it must install must have been obtainable at the applicable definition event. Missing required capture values are compilation errors, with no fallback to the caller's values.
- Branches and possible callees are checked in their corresponding contexts. Knowing caller paths does not require knowing runtime branch values; a reachable path lacking a required binding cannot be excused by another path that supplies it.

Deferred current-context invocation uses the bindings present when its owning full scope drains. A conditional registration requires those bindings on every path reaching that action, including failures after registration; a path that never registered it supplies no invocation obligation. Binding and owner proofs share the queue's branch choices. Later rebinding remains observable, iteration effects restore at their own boundary, and temporary effects inside an explicit cleanup scope restore before later actions. These guarantees do not make a missing binding available to an independent callable or create a definition-time capture.

The reusable-operation case is therefore checked through its complete definition/call paths and chosen mode. This does not select a runtime missing-value sentinel, implicit zero value, or a separate-module rejection policy. The precise analysis must preserve by-value captures while avoiding requirements for values that the checked path does not use.

### Module scanning and execution composition

H-492 permits declaration-only module dependency cycles: establish declared identities, exports and independently known signatures before body checking, without executing user code. H-493–H-498 fixes ordered top-level postpon suffixes and a single static initialization plan. H-499/H-500 selects a containing namespace's `__main__` as the entry, with multiple entry namespaces allowed within one dependency package. Ordinary modules cannot select/re-export an entry. [D-15](ModuleWorlds.md#module-dependency-cycles-and-initialization) owns the complete startup ordering, entry boundaries and acceptance.

H-502 makes initialization-block boundaries determine available binding versions. A block containing both x=1 and x=2 finishes before another scheduled block can observe that source state; separate blocks allow a consumer between them to see 1. There is no extra final-source-write barrier. Existing explicit resource selection, retained receiver values, complete captures and actual module contexts determine which binding an operation reads. [D-15](ModuleWorlds.md#binding-visibility-at-block-boundaries) owns examples and proof boundaries.

H-106 separates registration of the dependency set from execution compilation. The module top level admits symbol/function definitions and bits globals initialized by literals or function calls, plus effect/perform/behave expressions under H-107. Procedure bodies are registered during scanning; executable expressions and initializers are retained without invoking them or materializing captures.

| Phase | Required work | Declaration/value distinction |
| --- | --- | --- |
| 1. Static preparation | Resolve the selected entry's incorporated closure and prepare the static definitions in runtime module namespaces before initialization. | Identities, signatures and method bodies are available; no dynamic binding installation, arbitrary initializer evaluation or capture follows from registration alone. |
| 2. Fixed initialization | Compile one legal order of implicit dependency-prefix blocks and all explicit postpon blocks. Execute the emitted native instruction/call sequence with each block's module context. | Dependency prefix precedes its own suffix; each explicit suffix retains source order. The entry's explicit suffix joins this sequence before its ordinary prefix. Runtime binding/capture readiness must be proved. |
| 3. Entry execution | Execute the selected `__main__` module's ordinary executable prefix after successful initialization. | An initialization block cannot use a value supplied only by this later prefix. Uncaught initialization failure prevents this phase. |

Any admitted cross-module reference triggers H-109 incorporation, including vocabulary symbols, using/import, mixin and function/global references. H-140 incorporates each selected module only once. Scan order and first encounter are provenance, not authority to override the proved initialization order. Scanning an otherwise unincorporated module does not create an execution root. A diamond or repeated alias reuses the same context and initialization nodes; an unselected sibling `__main__` is not incorporated merely because its package is present.

An exported symbolic procedure preserves its original capture through aliases and diamond-shaped module incorporation. Each shared initializer runs once; checking emits no user-program effects. Local declaration order does not alter encounter availability when selected type/conformance visibility is unchanged.

“Compile execution semantics” means constructing the resulting program, not running user initialization effects in the compiler. Capture values follow runtime callable materialization in that program’s execution; they are not snapshots of the scanner's state. Required bindings are still checked at compile time over the resulting caller/initialization paths. Declaration availability therefore does not prove that a required dynamic value is available.

The entry module and every incorporated dependency retain their top-level scopes until process exit under H-166. Finishing a dependency’s initialization body does not end that scope or release its still-held regions. Ordinary nested procedure/block scopes keep their own exit boundaries. A region explicitly transferred out of a top-level holder need not remain allocated until process exit.

Scope lifetime and active binding lookup must be distinguished. H-110’s resource-selection boundary remains: `using`/`import` selects the source-defined procedure/resources and `mixin` defines the selected body in the receiving context. Merely selecting a vocabulary identity does not select its bound resource. H-166 changes lifetime; it does not explicitly replace these selection rules with automatic ambient exposure of every dependency binding. The implementation must retain module contexts without treating initialization return as their destruction. H-501 orders module-level deferred actions across contexts by application-wide reverse successful registration, including entry registrations and established work on startup failure. H-503/H-505/H-507–H-509/H-541 distinguishes local attached catches from bodyless handlers processing incoming deferred errors FIFO at their list position. Outgoing failures join the tail for later entries; optional finally runs once and ordinary body/startup errors are excluded. A handler may run its catch for several errors while retaining its selected context and actual owners. Ordinary nested scopes keep their own exits. [D-15](ModuleWorlds.md#cross-module-shutdown-order)/[D-17](ExceptionControl.md#remaining-deferred-queue-design-scope) owns the remaining queue/completion policies; retained-context representation and intrinsic owner/effect restoration remain integration work.

A source-defined procedure can therefore refer to a region still held by its module top level after initialization completes. Its captured references remain subject to the owning region’s lifetime, and a capture itself does not retain an independently ended nested region. Source versus receiving contexts, capture-by-value and current-binding `behave` remain distinct.

**Implemented boundary:** module-owned regions remain live after initialization and are released at normal program exit. Returning to the incorporating module restores its active binding lookup; the retained source context and its owners remain available through explicitly selected resources. Ordinary nested blocks and calls still release their local owners on exit. The source-module capture fixture reads its region after initialization; the nested-region escape fixture remains rejected. [K-295](../CHANGELOG.md#k-295) records the implementation and native evidence. This synchronous prototype has no user shutdown hooks or Actor lifecycle policy.

Unscoped conditional rebinding is rejected. Short-circuiting preserves captured/current lookup and nested-owner lifetime; a skipped operand performs no acquisition. Expression control flow does not establish conditional module incorporation.

Any admitted cross-module reference encountered during execution compilation triggers incorporation, including import and mixin; definition reuse does not exempt source initialization. H-140's once-only incorporation preserves selected resources without duplicate effects or recapture under another caller. H-495–H-498 governs executable order: dependency prefixes and explicit postpon blocks share the compile-time graph, followed by entry execution. Module contexts are shared evolving environments, not pre-initialization value snapshots, and another module's bindings do not become ambient through scheduling. Declaration-only cycles remain allowed; unproved/cyclic runtime readiness rejects. General dependency/cancellation proofs and physical context transport remain work. The source compiler still diagnoses cyclic discovery and does not implement postpon or namespace-selected entry routing.

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

[Exception control](ExceptionControl.md) supplies H-267/H-269's accepted try/catch/finally, unchecked failures and retained explicit causes. Leaving an effect scope exceptionally must restore its preceding binding and release only resources owned by that exiting scope. Perform replay must preserve pre-existing captured and hidden owners. Neither catching an exception nor restoring an environment snapshot can resurrect a consumed ordinary owner. H-510–H-515's omitted rethrow cause is a lexical caught-value dependency under ordinary capture rules, not a new dynamic exception binding. Helpers without that binding require an explicit from cause; root-typed values retain actual exception classification. Functions retain their exclusion from dynamic bindings/procedure calls; explicit-memory failure propagation does not grant new effect access. K-414 supplies focused exceptional-path evidence, including captured/hidden resource preservation; it does not qualify wider callable-view or concurrency contracts.

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

H-149 permits a nested lexical scope to shadow an outer local value, while duplicate declarations within one scope are compilation errors. The initializer is evaluated before the new binding becomes visible. Under H-271, a `ptr` declaration fixes its own target owner at initialization; rebinding that variable cannot select another owner. A nested declaration introduces a distinct binding, so this owner restriction is not a prohibition on shadowing. Leaving the scope restores lexical lookup of the outer name. Local names remain distinct from symbol variables written through `effect`. Ref/ptr/val declarations require initializer-before-binding lookup and fixed-owner checks.

Function parameters belong to the callable's inherent scope. Ordinary braces and conditional grouping do not create lexical scopes; explicit `{{ ... }}` does through its ordinary implicit callable invocation. Its returns and value captures follow D-17; it does not share an enclosing variable slot implicitly. A loop's index and fold accumulator belong to the loop construct. Body and zero-iteration else declarations require a nested explicit scope, while iteration effects and enclosing-scope deferred work follow [D-17's loop scoping rules](ExceptionControl.md#loop-scoping-rules). Scope-local name tracking is compiler metadata; no runtime lexical binding table or new regional representation is implied.

Acceptance covers an initializer reading the outer value, restoration after explicit scopes and loop iterations, parameter and loop binding uniqueness, and rejection of repeated declarations in one scope before program output. H-149 resolves K-253's naming alternatives; global declaration, function/type namespaces, dynamic symbols and broader scope-result rules retain their existing boundaries.

### Pointer reads and parameter owners

H-292 makes a fresh bits ptr initializer shorthand for `make local T { expression }` in the enclosing lexical scope; existing views retain their actual owners. H-293 preserves read-only access through aliases and requires an explicit external owner on assignment-overload destinations, with a diagnostic for omission. [D-14](RegionalOwnership.md#local-bits-storage-and-initialization) records the source contract, comma-separated group initialization, transfer shorthand and deferred definite-initialization checks. These decisions do not change dynamic-symbol storage or authorize reading TLS effects from functions.

H-271 makes a variable read implicitly dereference its target. `ptr` names do not expose raw addresses or pointer arithmetic. The compiler retains referent provenance for assignment and explicit external borrowing even when the same name supplies loaded bits to an arithmetic expression. `=` rebinds only within the initializer-fixed owner; `<-` performs the admitted bits/ownership transfer. H-270/H-302 makes assignment-family results destination views: builtin `=` preserves access, while `<-` is read-only.

H-272 makes a default parameter's target local to the callee scope. Default bits arguments are evaluated/copied, so editing the parameter leaves the caller's storage unchanged. The implemented ordinary-pointer subset is described in [D-14](RegionalOwnership.md#pointer-and-initialization-obligations). External borrowing requires the H-273-confirmed owner-generic annotation in D-14; a shared owner variable constrains the actual owner instance, not just its root type. Non-bits regional objects cannot be copied into the local scope. Whole-region consuming calls retain their single-owner and exceptional-cleanup obligations while their ptr declaration mapping is coordinated separately. H-273 permits omitting ptr in function/procedure parameter declarations without changing these semantics or their owner constraints. Local declarations still require ptr. These rules do not broaden a function's access to dynamic bindings.

H-274 additionally requires region-specific owner parameters constrained by a root type, with the actual borrowed region available to the body. Such a function can invoke explicit-memory root-policy methods without reading a dynamic symbol. A generic owner parameter still accepts either a region or a lexical scope and does not by itself supply a region reference. H-275 confirms the region binder spelling and extends region-as-declaration to the whole language. [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language) keeps actual region references distinct from root-object ptr values, with result/symbol spelling proposals separately marked.

### Lexical declaration keywords

H-150 rejects reserved expression names in lexical value declarations: function/procedure parameters, ref/ptr/val declarations, loop indices and fold accumulators. A bare name that the expression parser treats as a literal or control/declaration form cannot introduce a local binding. For example, `function identity(true:bool) -> bool { true }` is a compilation error rather than a function with an inaccessible parameter.

The bounded grammar reserves `true`, `false`, `null`, `alloc`, `for`, `fold`, `let`, `ptr`, `ref`, `val`, `throws`, `const`, `typeof`, `intrinsic`, `effect`, `perform`, `behave`, `if`, `procedure`, `function`, `global` and `symbol` in those declaration positions. Contextual names with ordinary value lookup, such as `in` and `else`, remain available. This aligns accepted declarations with their subsequent lexical lookup, serving C-37/C-41’s readable composition through the existing candidate P-86/P-97 value paths. It does not change H-149’s visibility/uniqueness rule or dynamic symbol identity.

Field selectors retain contextual identifier meaning: `record.true` and `setfield(record, true, value)` are valid. Symbol names, module resource aliases, callable names, module globals, a general reserved-word policy and escaped identifiers retain their separate boundaries; this local decision supplies no escaping syntax or wider namespace rule.

Acceptance checks inaccessible parameter/local/loop names before execution, including during dependency scanning, while preserving ordinary contextual names and field selectors. Rejection names the offending spelling and its source location. P-130 records the confirmed local constraint; the complete language contract remains pending-human.

### Primary module-form keywords

The implemented H-161 keyword set reserves `type`, `data`, `regional`, `unit`, `array`, `using`, `import`, `mixin`, `export` and `conversion` against ordinary declarations and aliases. These names cannot introduce lexical functions, values, types or selected UoM aliases. A declaration such as `function type(...)` is rejected before execution, independent of where it would be called. Field selectors such as `record.type` and symbolic names such as `@using` retain their contextual meaning. H-149 shadowing and usable nonreserved contextual names remain unchanged. Data is the concrete-representation declaration keyword; regional remains reserved although the legacy regional type form is rejected. C-41/P-97's readable expression composition is the existing candidate value rationale; this local keyword decision does not qualify the complete grammar.

### Parenthesized value expressions and operator idioms

H-PARENTHESIZED-VALUES-20261118 selects expression parentheses as producing an rvalue-like result, rather than transparently forwarding the original expression's place category. This is CoD value-category terminology, not C++ object copying/moving. A parenthesized result is not automatically an owner-free `val T` for every T: its admitted typed result still follows the ordinary bits, borrowed-reference, complete-callable or ownership contract. Parentheses themselves neither acquire ownership nor perform graph transfer.

For readable scalar bits, `(expr)` evaluates expr once and projects its completed result to independent bits of the same type. If expr returns a target view, read its initialized payload at this boundary while its owner is live. The independent bits carry no source owner or target identity. A later receiving declaration may materialize them in its own permitted storage. Nested parentheses are idempotent for this category; numeric type, units and validity are unchanged. Future fractions remain pure bits under D-16, without implicit reduction/conversion or authorization to implement them. Complete bits-ADT representation and transport remain under their own contracts.

```cod
ref n:u64 = 1u64;
ref live:const u64 = n <- 2u64;     # Retain the operator's readonly target view.
ref snapshot:u64 = (n <- 3u64);     # Explicit bits projection, then fresh storage.
n <- 4u64;
assert live == 4u64;
assert snapshot == 3u64;
assert live is n;
assert snapshot is not n;
```

An operator still produces its declared result before the surrounding expression consumes it. Builtin bits `<-` returns its readonly destination view; a parenthesized use projects that view to bits, while an unparenthesized use retains the reference contract. Overload bodies cannot evade owner/access/initialization checks. In particular, a readonly returned view cannot initialize a writable ref unless an explicit value-producing operation supplies independent bits; receiver mutability is never a materialization heuristic.

`(n) is n` rejects when n denotes scalar bits: the left operand is now an independent value without target identity. `(n) <- value` likewise cannot use the projected result as n's storage. `n is n` and `n <- value` retain their ordinary meanings. No temporary target is allocated merely to make `is` valid. An expression that selects a new place from an admitted computed view is checked by its ordinary field/index rules; parentheses grant no access escalation.

Only expression-level parentheses perform this projection. Call/signature parameter delimiters, type/pattern grammar and operator-name syntax do not implicitly wrap every enclosed item in a value expression. Thus `f(n)` preserves the ordinary argument expression, while `f((n))` explicitly projects n's bits. A parenthesized callable-owner projection such as `(cb -> O)` still supplies the existing computed borrowed-region result, with its owner/access and no ownership transfer; it is not an independent region object. Nullable pointer results retain their absence and fixed owner rather than implicitly reading their pointee. Complete callable values use D-18's dedicated value rules. Exact owning-carrier and non-bits result/place combinations need their corresponding typing/transport review; the compiler must not silently restore original place semantics as a fallback.

**Operator-design default.** Design reference-result overloads for useful unparenthesized chains. Prefer precedence/associativity that makes the intended common idiom readable without parentheses inserted just to override precedence. This is a default design and UX principle, not a prohibition on parentheses or a blanket rewrite of the existing precedence table. A concrete operator may justify a specifically recorded exception. Do not choose meaning from guessed user intent or retry failed overload selection under another grouping interpretation.

The parser and source tools must preserve the parenthesized expression boundary until its typed effect is established. A formatter cannot add/remove parentheses as a cosmetic precedence edit without proving that value category, identity, overload selection and evaluation remain the same. Parentheses alter neither the established once-only operand/conversion order nor callable cleanup order; a returned view is projected only after its call completes. No new lexical scope is introduced.

**Acceptance and remaining work:** distinguish bare/parenthesized scalar places and returned readonly/writable views; preserve snapshots across later writes; retain operator-result views for unparenthesized chains; reject scalar projected destinations/identity operands; distinguish nested grouping from call/type/operator-name delimiters. Preserve target-read readiness, actual owners, numeric/UoM/fraction validity, callable value rules and D-17 cleanup. Source policy is selected; typed group representation, complete generic/non-bits carrier integration and checker/native migration are not established by older parser evidence that erases parentheses.

### Operator and intrinsic overloads

`const ptr` fixes each declaration-group binding while preserving the referent's independent access permissions. The `<=>` reference-swap and `<->` bits-swap operations yield void, evaluate both destinations left-to-right once, and admit function/procedure overloads with explicit external owners. Reference-swap methods receive both actual caller bindings. [D-14](RegionalOwnership.md#reference-and-bits-swap-operators) defines their checks and interaction with fixed bindings and loop invariants.

`intrinsic (operation)(arguments...)` explicitly chooses the checked builtin implementation and bypasses user overload selection. It supports delegation from a same-signature overload without changing ordinary lookup or the function/procedure boundary. `intrinsic` is reserved as an expression keyword; parenthesized names use the existing operation grammar, not a symbol identity. Missing builtin implementations reject. Ownership, access, lifetime, argument sequencing and the destination-return safeguard remain in force; [D-14](RegionalOwnership.md#explicit-checked-builtin-invocation) defines the bounded contract. Mixin preserves this explicit selection in the copied body.

Range `for ... else` executes its else block only when the body runs zero times. Bounds are evaluated once; exceptional exits do not enter else. Both blocks have separate local/dynamic scopes, and the index belongs only to the body. The expression yields void. [D-14](RegionalOwnership.md#loop-initialization-and-optional-empty-range-branch) defines the normal-path initialization join.

H-153 reserves a set of keywords against ordinary-name declarations and uses parenthesized operation names to declare overloads. H-439/H-441 adds word operators is and is not with fixed owner-plus-locator identity and exact-negation semantics; it is not a payload-equality overload point. It uses equality precedence and reference selection rather than default value-parameter copying. Any explicit invocation through the operator surface must preserve the same behavior. H-440/H-441 uses is null and is not null for absence/non-null testing. H-442 also reserves not as the unary word spelling of ! under the existing keyword/field-selector rules. After is, the pair is not is parsed as the identity comparison; prefix not otherwise follows the unary rules below. An intrinsic spelling such as `len` must not silently conceal an accepted ordinary function declaration. `(len)` is an explicit overload name, following H-131's operator naming direction. The [prototype reference](../tests/cod/language/README.md) lists its reserved expression, intrinsic and built-in type names. H-150's usable contextual names and field selectors retain their grammar; parentheses do not turn arbitrary control forms into eager calls. The complete future grammar remains open.

<a id="unary-logical-negation"></a>
**Unary logical negation.** H-442 selects not and ! as two spellings of one semantic operation. Both use the same tight unary precedence, nesting, operand admission, conversions, once-only evaluation, result contract and failures. The builtin takes bool and returns independent val bool; neither spelling adds truthiness or treats a pointer as a nullness test. If operand evaluation dereferences a nullable bool, both retain its ordinary NPE obligation. Existing operator-method rules apply identically: (not) and (!) normalize to the same operation for definitions, explicit calls, aliases and intrinsic selection. Different spellings cannot define competing implementations of the same signature; ordinary duplicate-definition rules apply.

```text
not flag;              # Same as !flag.
not not flag;          # Same as !!flag.
not (p is null);       # Same as !(p is null), and p is not null.
not flag && ready;     # Same grouping as (!flag) && ready.
```

Unary not binds as tightly as !, so not a is b groups as (not a) is b; it does not mean not (a is b). The two-word a is not b remains a single identity comparison, equivalent to either parenthesized negation of a is b. Nullability refinement recognizes equivalent negated null tests without re-evaluating operands or retaining facts across invalidating writes. Acceptance covers both builtin spellings, nesting, shared method identity, explicit/intrinsic forms, mixed precedence, once-only side effects, null access and branch refinement. These are selected source rules, not a claim that the current parser accepts the new word spelling.

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

The value rationale is readable operation syntax with inspectable type, effect and environment dependencies: C-41/P-97 and C-50/P-121 support readable typed composition, while C-37/P-86 and C-36/P-85 preserve explicit-input versus dynamic-context boundaries and contextual interpretation. These candidate value paths do not qualify the complete contract. Acceptance must distinguish function and procedure overloads of one spelling, reject a selected procedure in a function call tree, preserve current versus captured values and argument order, and diagnose ordinary keyword declarations. Existing intrinsic behavior and contextual field names must remain covered. Current implementation availability is maintained in the source capability reference; unresolved elaboration and composition choices retain their individual review scope.

### Storage carriers, capture types and lifetime

[Callable captures](CallableViewsAndCaptures.md#value-snapshots-and-invocation-local-captures) distinguish ordinary scalar/complete-callable snapshots, selected ref/ptr values and explicit var place aliases. Ordinary reference copies do not retain the source binding slot; an explicit alias retains its selected place and observes permitted later writes/rebinding. Borrowing lexical storage is admitted only with proof that every use stays within its actual owner lifetime. No hidden closure cell or owner extension is permitted. The table below describes **dynamic** capture at runtime view materialization, separate from module method registration. It does not turn perform into a graph snapshot or change effect restoration.

Ordinary user-program storage has lexical and regional carriers; symbolic bits payloads use the dedicated TLS storage specified above. H-177 permits ordinary variables to own region handles; H-179 permits consuming installation into a dynamic resource owner. Neither copies a regional object onto the stack. Host-native heap allocation belongs to system code such as compiler/runtime infrastructure. It cannot serve as a hidden way to extend the lifetime of a source variable, captured object or borrowed owner. No user `malloc`/`free` lifetime, automatic heap box, user GC object or reference-counted heap capture is introduced.

| Captured value | Meaning and current boundary |
| --- | --- |
| Bits | Preserve the value at runtime view materialization. Later scalar bindings do not change it. |
| Whole-region effect resource | Preserve the selected resource value and its owner/lifetime dependency. Replay does not move ownership, copy the graph or freeze its contents. |
| Borrowed target reference | H-482 retains installation-selected regional owners; H-561 admits lexical and ordinary regional owners through an owner parameter. Captures preserve the selected target/actual-owner dependency; the compiler proves validity without extending the owner lifetime. |

Reinterpreting an address as an integer must not bypass carrier/lifetime checks. A native heap pointer, stack pointer or arbitrary handle is not implicitly an admitted capture. System-resolved addresses are execution artifacts, not independently owned user objects.

Mutation through a valid resource reference changes its referent. Restoring a symbol binding does not undo that mutation. A read-only view prevents mutation through that view but still requires valid lifetime. Capturing a regional object by C++ copy/move is forbidden.

An initial SNN step can use non-escaping stack values and region-backed effect resources. Captured whole-resource values and region-backed interior references retain their respective ownership and lifetime dependencies. A regional pointer's owning-region association must remain valid; a saved offset cannot silently be interpreted relative to the caller's different `@region`. A procedure retained across frame reuse, region release or remapping needs a justified lifetime. No lexical variable or invocation frame is automatically promoted to a longer-lived native heap allocation. The concrete procedure descriptor and compiler-generated stack temporaries remain representation questions. H-193 makes user-defined data objects regional. Scalar UoM identities retain scalar bits representations; they do not provide arbitrary record storage on the stack.

TLS directly stores symbolic bits payloads under H-277; it is not an allocation arena for non-bits user objects. Symbolic object references and captures must respect their region-symbol resource lifetime. Saved values and environment storage cannot extend that lifetime. `@region` allocation is available to procedures, while functions use their declared inputs/outputs and legal stack temporaries.

#### Function region-state access

H-197/H-198 permits functions to read and edit explicitly supplied memory and, once their source contracts exist, create regions, return ownership and send through an explicit mailbox. Functions cannot obtain regions through dynamic-variable lookup or call procedures to bypass that restriction. A procedure can supply a valid explicit view to a function while retaining its dynamic owner; the function does not acquire or rebind that owner's symbol.

References preserve the actual input-region association through fields, nested calls and returned values. A function may return a view into a still-live caller region. A view into a local region released at function exit is rejected; returning an intrinsic whole-region owner would instead transfer cleanup responsibility. Regional objects are never copied onto the stack, and function return does not itself end an input region.

There is no automatic read-only snapshot spanning nested functions. An inner function's writes are observable by subsequent reads in its caller and through aliased parameters. Native lowering must preserve source sequencing and actual memory dependencies, without inferring referential transparency from the keyword. No copied snapshot, version counter or per-object ownership system is introduced. Caller-visible edits are not automatically rolled back when local storage is released.

The implemented subset permits store, setfield and mutating function overloads over explicit regional inputs. A bare region local, parameter or result borrows an already held resource and retains its owner provenance. `alloc arena T(...)` accepts that named lexical borrow, so a function can construct objects and arrays in a caller-supplied region without reading a dynamic symbol. Functions still reject symbolic reads/writes, perform/behave and all selected procedure calls. A borrow cannot be installed as a new owning effect or escape the region's acquiring scope. [K-411](../CHANGELOG.md#k-411) records typed roots, make, ordinary owned locals/calls/results and preserved dynamic captures. Scalar-slot borrowing, general const propagation and mailbox operations remain unimplemented. Concrete data uses regional allocation; implicit object-field borrowing and qualifier syntax remain separate migration work.

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
2. Compile execution semantics from the namespace-selected entry, incorporating each dependency once. Prove the fixed block order under D-15, with static namespaces ready first and entry execution last. Follow initialization/caller paths and check symbols, types, owners and captured/current bindings for each call mode. Compute reusable summaries where useful; insufficient proof rejects without runtime scheduling.
3. Assign typed locations in the program's per-thread context, then lower known accesses to a base plus a constant offset.
4. Lower `effect` and `perform` to scoped save/install/restore operations; `behave` retains the current context.
5. Establish the admissible actual-type combinations for each selected family/context and emit their implementations and dispatch before execution. Share equivalent targets and prune only proved-impossible alternatives; callable-view and concrete-evidence mechanisms require the D-21 contract.
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

### Generalized for-loops

A library can provide one traversal and let each caller use either `for val n <- ascending() { ... }` or `ascending() {=> n; ... };`. Both select the same generator, enter the caller’s live lexical scope in the same order and deliver the same feedback. Inside the library, the traversal may call `yield(item)` or an explicit first here parameter such as `step(item)` with the same protocol. Changing the selected generator can change the visit order or membership effects under either spelling. [Ascending and descending in both call forms](RangedForAndGenerators.md#generalized-for-loops)

For the proposed collection interface, `visit(xs) {=> index, item; ... };` exposes a readonly item for each visit. In a void visit, `continue;` finishes the current observation and `break;` ends the whole visit. The collection stays intact in both cases. A library function with a non-void result can also let the caller supply that result with `break value;`. [Collection examples](CollectionsAndBuiltinContainers.md#observation-and-owner-scope)

## Validation and unresolved contract scope

The [case records](../applications/steady-snn/research/cod-language-scope-cases.json) contain expected semantic traces and proposed checking cases. They remain specifications, with a link to selected native fixture evidence; the complete catalog has not been executed. They cover alias/version identity, scoped restoration, by-value capture, both invocation modes, transitive dependencies, function restrictions, thread/program isolation, scan-only registration, entry-driven dependency incorporation and the SNN's immutable-A gather.

Typed symbols, bits globals, explicit result checks, ordered argument conversion and known-call binding/lifetime checks have bounded native implementations described in the prototype reference. Remaining semantic work includes the complete postponed-initialization contract, compound implementation views, local value/call lookup, standalone method heads and the coherent owned/const/place/construction/transfer contract. Missing required dynamic bindings remain compilation errors. These boundaries affect observable behavior or valid programs and require design decisions; equivalent lowering layouts and inlining choices can instead be evaluated on actual programs.

[Racket's parameter reference](https://docs.racket-lang.org/reference/parameters.html) is a useful primary comparison for scoped bindings, restoration and thread-sensitive behavior. Its continuation/thread-cell model is not CoD's capture-by-value rule. No continuation system, general dynamic dictionary or scheduler follows from that comparison.



## Implementation availability

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Checked CoD emits LLVM artifacts for native execution. LLVM is a derived execution representation, not the language definition or a persistent semantic graph. Semantic coverage matters more than case counts; application and cross-platform acceptance remain separately scoped.

## Complex composition and initial lowering

H-139 requires deliberate multi-module, complex-dependency and deep-call validation. Acceptance includes diamond graphs, repeated selections, shared vocabulary and nominal-type aliases, capture restoration across long call chains, and source initialization order without scan-time execution. H-140 requires each module to be incorporated once; emitted initialization is part of the resulting program. H-166 keeps module top-level scopes alive until process exit; initialization return restores active lookup.

H-141 accepts procedure expansion/inlining for the initial WPO implementation while program size remains reasonable. LLVM artifact size, native size and compile/run cost must be measured on representative deep and branching graphs. Reusing machine-code fragments is a medium/long-term optimization rather than an immediate architecture requirement. WPO call-path checking remains mandatory whether the eventual native lowering inlines or shares code.

H-144/H-145 require keeping the implementation and validation lean. Coverage is assessed by whether module incorporation, identity, capture/restoration, dynamic-context exclusion, typed conversion, control flow and regional lifetime/relocation behave as specified. Named numeric boundaries and an independent SoA gather reference address the relevant arithmetic risks. Duplicate syntax variants and large sample counts do not establish additional semantic coverage. The prototype introduces no backend abstraction, custom optimizer or code-sharing layer merely to support this validation.
