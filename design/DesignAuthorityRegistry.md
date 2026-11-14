# Design authority and value lineage

Registry revision: 619 · Audit date: 2026-11-13 (America/Los_Angeles)

## Design authority

[D-GOV](#d-gov), all registry-maintenance sections. Value path: V-03 → GOV-01 → GOV-02 through E-11/E-12. H-05 establishes human-scoped delegation; H-388/H-391 authorizes concise audit records; H-403 requests ongoing decision summaries without side-chat quotations. [K-618](#k-618) records revision 619 and the current source capability reference; K-597 retains external process tracking, K-596 the D-21 rename and H-410’s preparation/execution clarification; K-595’s bounded source availability and H-409’s method/implementation/local-operation contract remain. H-407's scoped closure approval and H-408's practical stage goals remain. The [charter](RegionalSemanticPlatformCharter.md) and [AGENTS.md](../AGENTS.md) govern.

Acceptance: preserve stable IDs, relationship endpoints/kinds, current qualification, exclusions and unresolved decisions; resolve historical evidence to immutable Git sources. Maintenance alone grants no product qualification; H-407 separately supplies human approval for its identified D-26 scope. Historical registry: `061a4a684770`; original event payloads: `0c0efbfde4dc`, both at this file's path. Newer decisions remain below as attributed records; H-403 governs side-chat paraphrases.

## 1. Audit and reading rules

1. Start with the relevant **document scope**, its named relationships and directly needed contracts. Read the charter when changing a design; unchanged qualified scope permits the local-contract shortcut in AGENTS.
2. **Confirmed** covers the stated proposition/revision/scope. **Continued** is an agent's contract-preserving assessment from that human baseline. **Pending-human** and **needs-reconfirmation** do not authorize implementation; **superseded/rejected** scopes are historical. Direction, concept, whole document and implementation acceptance are distinct.
3. Use one entry per independently decidable contract scope. The linked product document owns detailed semantics, non-goals and acceptance; this registry owns qualification and evidence. A routine implementation checkpoint needs no new concept, value edge or repeated status paragraph. Record coherent maintenance in one brief continuity event.
4. Nodes name topics. `derives-from` explains value; `depends-on` identifies prerequisites; `constrains` bounds behavior. Read each edge's scope/status: an ID, dependency or ancestor approval grants no additional authority. Apply current scopes and supersession before historical revision wording. Candidate value paths stay candidate even when associated behavior is confirmed.
5. For impact review, find both endpoints in the relationship table and use their contract links; follow value descendants and dependencies separately. These links route review, not new delegation. Preserve unaffected qualified scope; escalate actual evidence gaps, contradictions or uncovered material choices.
6. H-386 requires topic-based human discussion and eventual whole-file approval of new designs. Ask for concrete decisions, not internal IDs; do not reopen settled preferences. Build/test/toolchain permissions remain independent.
7. Human history is append-only in Git: preserve the actual source, statement, time uncertainty, scope and exclusions. Corrections/withdrawals get new events. Read original evidence when exact scope matters; an index or agent summary cannot replace confirmation. [§5](#5-evidence-locators) provides targeted retrieval.

## 2. Current document scopes

Revision labels identify inspected text, not approval of every section. **Pending whole-file qualification does not cancel independently confirmed local scopes.** Detailed acceptance and unresolved-topic inventories remain in the linked documents. Source headers and older registry paragraphs are evidence to reconcile, not independent authority.

| Contract / inspected revision | Authority and lineage | Boundary, acceptance and open work |
| --- | --- | --- |
| <a id="d-00"></a>**D-00** [Charter](RegionalSemanticPlatformCharter.md) | H-01–H-04 confirms V-01–V-03/E-01–E-10 directions; H-04 authorizes the charter’s creation. Initial charter commit `3fe2cd1dfd06ec41481f265386d23f3dce7e2ff6`, blob `9f0154035d52f976232000ab1601088b91948c0b`. | Value root, not blanket approval of generated details, examples, roadmap or §14 mechanisms. |
| <a id="d-gov"></a>**D-GOV** registry r619; AGENTS governance | **Continued** H-05/E-11/E-12 under H-388/H-391/H-403; K-618. | Traceable scoped delegation, current decisions and recoverable evidence. Side-chat evidence uses attributed decision paraphrases. No charter or operational-permission change. H-407 is separate human design approval, not qualification by registry maintenance. |
| <a id="d-01"></a>**D-01** [Regional storage](MemoryRegionAndRegionalTypes.md); blob `8dc4ed4012d1127087d83faf9e13ed49db5345d5` | **Pending-human**, P-01/P-02. | Find a confirmed pointer/lifetime/acceptance baseline before delegating this whole file. AGENTS regional invariants remain mandatory independently. |
| <a id="d-02"></a>**D-02** [C++ driver](CompileOnDemand.md); blob `cbd294de3eb9825c8522b9eb7781e44a5ba8d844` | **Pending-human**, P-03/P-04. | Stage-specific build-and-run facility; its no-JIT boundary does not define the platform’s long-term execution strategy. |
| <a id="d-03"></a>**D-03** [Package management](CodProjectStructureAndPkgMgmt.md); blob `4146412997de5179146a7010b5b034e32fcebfb9` | **Pending-human**, P-05. | Git/UUID/branch/lock mechanisms and semantic-unit distribution need scoped confirmation. Existing code is not approval. |
| <a id="d-04"></a>**D-04** [C++ exception safety](ExceptionSafety.md); blob `3dc6bbba41ee5c84f151c3c94bb866b47ddcdf6e` | **Pending-human**, P-06/P-07. | K-406 identifies contradictory null-result/strong-guarantee claims and unguarded allocation/root construction. This is not CoD cleanup or persistent-recovery evidence; D-17 owns the language contract. |
| <a id="d-05"></a>**D-05** [Regional YAML](YamlConversionsOfRegionalTypes.md); blob `34da18a3abab0a3a02b42d23287e644312f5c218` | **Pending-human**, P-08–P-10. | Confirm conversion scope/semantic preservation separately from the future source round-trip contract. |
| <a id="d-06"></a>**D-06** [YAML extensions](YamlSpecExt.md); blob `5dd33c2c2a5acc59383f0966a98300f84a296f63` | **Pending-human**, P-11. | Confirm syntax/comment benefits and limits; YAML is not selected as final business-language syntax. |
| <a id="d-07"></a>**D-07** [Language decisions](DomainLanguageDesign.md), r154 | **Whole file pending-human.** H-06/H-07 establishes the tracker; C-08–C-11/P-12–P-21 supplies candidate paths. Local decisions route through D-10/D-11/D-14–D-18/D-21–D-24. | G-01–G-07, DL-01–DL-16 and DD-001–DD-151 retain the decision/acceptance inventory. H-351/H-352 settles library graph-transfer responsibility and IPC/DBMR planning directions, not protocols, ABI, concurrency, snapshot failure or retention. Past work-period permissions are not a new task. |
| <a id="d-08"></a>**D-08** [SNN application](../applications/steady-snn/docs/README.md), 19 documents; README/LanguageCoDesign r220 | **Whole application pending-human.** H-08–H-95 supplies local goals/constraints; C-12–C-34/P-22–P-83 retains individual states. H-94 confirms target-owned two-frame pull (P-69/P-70/P-83); H-93 continuous source dynamics and H-95 capability preservation remain. K-209/K-305 identifies local revisions. | Use each file’s own revision; the set is not uniformly r220. Equations/state, readout, phase detection, final capacity, learning/migration, messages and persistence remain open. H-78/H-85 preserves comparisons; H-86 permits empirical alternatives without repeated questions. Research completion is not model validation; H-123 language support does not approve neural algorithms. |
| <a id="d-09"></a>**D-09** cognitive synthesis [English](../applications/steady-snn/docs/CognitiveModelProgress.en.md) / [Chinese](../applications/steady-snn/docs/CognitiveModelProgress.zh-CN.md), r2 | **Whole file pending-human.** H-87–H-95 are scoped premises, not approval of the synthesis. C-30–C-34/P-71–P-83; K-208/K-209. | S1–S9/CQ-01–CQ-08 retains readout, binding, learning, capacity, timing/history and coupled-response questions. Learned concepts are not automatically formal ontology/GADT evidence. No selected extra geometry/state or permission to run S8 candidates. |
| <a id="d-10"></a>**D-10** [Symbols and calls](SymbolicDynamicScope.md), r145 | **Whole file/value paths pending-human; recorded local decisions remain confirmed.** C-35–C-46/P-84–P-111 and later operation/owner/publication edges; P-248–P-254/P-330–P-343 governs current effects/families. K-572 bounds latest prototype evidence. | Preserve identities, selected contexts, perform/current-binding distinction, owners and function effect exclusion. D-14 owns storage, D-15 worlds, D-17 completion, D-18 callable values, D-21 families. Cyclic initialization, persistent ABI, broader captures/results/contexts and family mechanisms remain open; old passing reports do not qualify them. |
| <a id="d-11"></a>**D-11** [Scalars, units and indices](ScalarUnitsAndIndexTypes.md), r31 | **Whole file pending-human.** C-47/C-49–C-51/P-112–P-130; H-260/H-261/K-402 covers representation-independent quantities and lossless widening. | Separate unit identity, numerical representation and conversion policy. D-16 compatibility applies. Compound equality/broader conversions retain their open scope; no SNN parameter or fraction implementation follows. |
| <a id="d-12"></a>**D-12** [Globals, worlds and TLS](GlobalVariablesDoneRight.md), r26 | **Whole file pending-human.** C-52/C-59/P-131–P-135/P-213; H-241 places independent worlds in the vision, D-15 owns the operational proposal. | Actor scheduling/delivery/shutdown, optional outcomes and packet lifecycle need their own contract. Distinguish packet ownership from effect-resource replay; synchronous evidence is not Actor acceptance. |
| <a id="d-13"></a>**D-13** [Regional memory tracker](GraphGrainedMemoryManagement.md), r31 | **Whole file pending-human.** C-39/C-40/C-53–C-55/P-94/P-138–P-143/P-155; H-156/H-157 establishes the local graph boundary. | D-14/D-17 owns scoped lifecycle behavior. Same-region stored edges, hidden-owner preservation and explicit transfer remain; D-22 refines only bits-eligible placement. No transport, per-object ownership, borrow table or allocator policy is qualified by tracker/relocation examples. |
| <a id="d-14"></a>**D-14** [Ownership and storage](RegionalOwnership.md), r82 | **Whole file/value path pending-human; local requirements confirmed.** C-39/C-48/C-54/C-55/C-64/C-65/C-69–C-72/C-75; P-244–P-282/P-327 records exact owner, initialization, transfer and access scope. H-270–H-276/H-292–H-315/H-366–H-369; K-552 covers r81→82. | Preserve actual fixed owners, non-escalating access, initialization and cleanup. D-17/D-18 supplies completion/callable contracts; D-22 owns new construction/aggregates. General transport, owner unions, runtime lifetime machinery and whole-file qualification remain open. |
| <a id="d-15"></a>**D-15** [Module worlds](ModuleWorlds.md), r22 | **Whole operational contract pending-human.** C-59/C-60/P-204–P-220; H-234/H-237–H-249 specifies receiver-owned identity construction, explicit publication and typed TLS. H-387/K-573 covers the fused artifact; K-593 aligns its input boundary. | No foreign-world editing; H-253 rejects conflicting interpretations. Persistent identity/layout, general composition/namespace and physical RTTI remain unselected. H-387 preserves source-world isolation/owners; H-400–H-406/H-407/D-26 replaces the known-ADT-only input restriction without authorizing new executable methods. |
| <a id="d-16"></a>**D-16** [Fixed-width fractions](FixedWidthFractions.md), r7 | **Whole file pending-human; implementation deferred.** H-254/H-255 confirms enumerated requirements; C-61/C-62/P-221–P-227. H-256/P-228 prioritizes comment reservation and independent UoM; K-401/K-402 is prerequisite evidence. | Preserve integer-component fixed bits, unreduced arithmetic, explicit canonicalization, numerical comparison, orthogonal UoM and // with # comments. Promotion/widths, invalid values, exact grammar/access/layout/formatting remain open. Conflicting future language choices require review; prerequisites do not approve fractions. |
| <a id="d-17"></a>**D-17** [Exception control](ExceptionControl.md), r53 | **Confirmed/continued bounded exception, lifetime, scope, defer and return contract.** C-63/P-235/P-236; H-267/H-269/H-313–H-315/H-319/H-356–H-362/H-365/H-370/H-371; P-321–P-324/P-326/P-328/P-329. K-555 explicitly covers r52→53. | Acceptance: construction failure, exact release, explicit causes, owner/effect restoration, pre-entry versus callee failure, completion replacement and deferred queues. Complete integration/proof coverage remains work. No regional copy, lifetime extension, native-handle/out policy, RTTI, persistent transaction or broad D-04 guarantee follows. |
| <a id="d-18"></a>**D-18** [Callable captures and values](RegionalClosures.md), r90 | **Confirmed/continued only for recorded capture/value/native-boundary scope.** H-327–H-335/H-337–H-344/H-349/H-350; C-65/C-73/C-75/P-293–P-303. H-342 replaces callable alias initialization; H-344 permits runtime-sized scope-owned backing. K-572 covers bounded r88→89 continuity/evidence. | Preserve actual borrowed owners, complete nested values, recursion, caller destinations, access and cleanup; measure allocation/compilation. No regional callable payload, lexical-cell promotion, independent user heap lifetime or captured-region retention. Actual-owner runtime alternatives/D18-Q-result-discrimination remain pending; native fixtures do not establish D-21’s wider stage/family contract. |
| <a id="d-19"></a>**D-19** [Pure functions and TCO](ConstFunctionsAndTailCalls.md), r3 | **Confirmed deferred planning/compatibility scope; implementation contract pending-human.** H-346–H-348/C-76/P-304–P-307; H-363/P-325 selects pure with final-expression result. H-366–H-369/K-527 preserves independent qualifiers. | External state readonly over the outer call, private mutation allowed, result judged at immediate return. No permanent freeze, implicit copies, blanket optimizer purity or TCO outside this plan. Checking, callable types, outputs and tail eligibility remain open; ordinary recursion remains required. |
| <a id="d-20"></a>**D-20** [Compiler application](../applications/cod-booter/README.md), [pipeline](../applications/cod-booter/docs/CompilerPipeline.md), [LLVM interop](../applications/cod-booter/docs/LLVMInterop.md), r4 | **Planning authorized; detailed contracts pending-human.** H-353/C-80/P-314–P-318; H-354/C-81/P-319/P-320 proposes output initialization. K-515 synchronizes separately qualified H-356 defer work. | Schemas, reuse, native handles/provider lifecycle, out parameters and self-hosting acceptance remain open. Regional stage data/LLVM services are a plan, not toolchain or compiler-application implementation authorization. |
| <a id="d-21"></a>**D-21** [AOT execution preparation](AheadOfTimeExecutionPreparation.md), r11 | **Behavioral path continued** H-378/P-333/P-334/P-337 with H-372–H-382. **Replacement design/prototype and conditional merge authorized** H-396/H-397. K-593 aligns input/coverage to H-407/D-26; H-409 clarifies call-site specialization; H-410/K-596 makes AOT preparation and runtime selection explicit. C-86/C-87/C-88/P-363–P-370 detailed production paths remain pending-human. | H-409/K-594 separates one semantic method from its general/specialized implementations and local dispatch points; unique-most-specific selection by actual concrete identity/conformance; local operations and proven call-site facts. Preserve installation/operand effects, captures, owners and completion. D-26 supplies the qualified coverage responsibility; concrete analysis, permanent ABI, module composition and compatibility protocol remain open. ORC future-only. |
| <a id="d-22"></a>**D-22** [Construction and acquisition](ConstructionAndAcquisition.md), r2 | **Whole file/P-348–P-350 pending-human.** H-383–H-386/P-344–P-347/P-352/P-353 confirms bits eligibility, intrinsic ownership, owner mismatch rejection, result-only constness, contextual inference and coordinated-migration preference. K-545 is integration, not final approval. | Decide aggregate/native representation, constructor allocation/partial effects and DBMR capacity/failure/coordination. R3 exploration is authorized; final selection reserved. Preserve non-bits regional placement, same-owner edges and cleanup. Do not re-ask inference/no-transition preferences. |
| <a id="d-23"></a>**D-23** [Scalar call results](ScalarCallResults.md), r2 | **Pending-human whole-file/topic review**, P-354/P-355; K-556/K-558 is investigation/evidence. Existing H-201 borrowing/const values and owner/completion rules remain independently applicable. | Decide fresh arithmetic/mixed borrowed-fresh result categories and caller storage. Existing behavior is observation, not approval. Borrowing fixes do not select fresh storage, owner unions, hidden allocation, lifetime extension or aggregate ABI. |
| <a id="d-24"></a>**D-24** [Result-type inference](CallableResultInference.md), r1 | **Pending-human**, C-84/P-356–P-358/K-562; no confirmed baseline or implementation delegation. | Decide omitted result types, unanchored recursive cycles, declaration constraints and abstract results. Bodyless signatures, recursion, effects, owner/value and return rules remain; parser acceptance/tests do not approve inference. |
| <a id="d-25"></a>**D-25** [Micro-dispatch assessment and source migration](CompilationRuntimeArchitectureExploration.md), r15 | **Isolated delivery authorized** H-396/H-397; **source migration authorized** H-398/H-399; K-586/K-587/K-593/K-594/K-596. C-85/P-359–P-362 and C-86/C-87/C-88/P-363–P-370 detailed production paths remain pending-human. | Prior package preserved at 25ccc75 without final selection; new branch starts at prior main 8ff9576. No assumed checker/IR/backend/fallback, BBV or JIT requirement. Native experiment evidence covers only its finite explicit contract. Source migration begins at local multi-dispatch tag 2ab762d. H-399/H-408 permits a useful removal-first foundation with documented gaps; H-407 confirms D-26 coverage responsibility. H-409 clarifies specialization and independent local selections. No new budget or permanent ABI is authorized. Existing operational limits remain. |
| <a id="d-26"></a>**D-26** [Generic and recursive closure](GenericAndRecursiveClosure.md), r9 | **Confirmed whole-file behavioral/native-coverage scope and value path**, H-407; C-89/C-90/P-371/P-372/P-374/P-375. P-373/P-376 remains dependency only. H-400–H-402/H-404–H-406 supplies local decisions; K-593/K-594/K-596 continues the approved baseline through H-409’s hierarchy/initiator and H-410’s preparation/execution clarifications. H-398/H-399/H-408 governs bounded stage acceptance. | Permit multiple prepared implementations per semantic method and proven AOT direct calls. Preserve identities, same-T and valid uninterpreted passage; unavailable required interpretation is checked, breached typing unchecked. Generic bodies alone add no unknown-type throws; voluntary declarations retain obligations. Stop on detected resource exhaustion, without a universal analysis-nontermination detector. Concrete checking, failing-path analysis, carrier/ABI, input validation and result inference remain separate review scopes. |
| <a id="d-27"></a>**D-27** [Named quantities and numeric compatibility](NamedQuantitiesAndNumericCompatibility.md), r1 | **Pending-human whole-file/value-path review.** C-49/C-61/P-377–P-379 proposes a bounded contract around H-128/H-260/H-261; P-380 is dependency only. K-614 records the audit. | Preserve named identity, representation, whole-type lossless compatibility, ownership and checked native interpretation. No implementation delegation until review. Automatic unit routes, module composition, operator elaboration, compound units, fractions, new generic inference and permanent ABI remain outside this contract. |

### Current supersession and audit boundaries

**Source compiler restructuring (H-399):** on `main`, historical implementation evidence in D-07/D-10/D-15/D-17/D-18 and the source fixtures describes the preserved `multi-dispatch` baseline, not current feature availability. The source foundation supports checked scalar methods, direct AOT specializations, shared general bodies, recursion, lexical scalar storage, nullable local views, scalar borrows with cross-call initialization and returned-target summaries, complete nested callable values with structural generic signatures, bounded intrinsic exceptions and prepared/deferred cleanup with recovery. Regional, abstract-input, general callable capture, richer relational proofs and module-composition paths remain explicit implementation gaps. Confirmed language contracts remain the target. [The source capability reference](../tests/cod/language/README.md) describes current supported/missing behavior.

These are scoped replacements, not blanket invalidations. Apply them to older wording; unaffected requirements remain. Detailed originals remain in H/K evidence.

| Scope | Current decision / evidence | Affected contracts |
| --- | --- | --- |
| Capture/function effects | H-180/H-181 preserves captured values/hidden owners and forbids dynamic-owner extraction; H-197/H-198 replaces readonly function snapshots with explicit-memory access. | D-10/D-13/D-14/D-18 |
| Composition/publication | H-212 assigns redefinition to mixin; H-218/H-220 requires explicit conformance and distinguishes adapter import from consumer visibility; H-230–H-233 separates identities/aliases and requires publication; H-245 forbids foreign-world editing. | D-07/D-10/D-12/D-15 |
| Parameter storage | H-271/H-272 gives callee-local default targets; H-273–H-276 specifies explicit owner/region forms. Earlier external-borrow defaults are not universal. | D-10/D-14/D-17/D-18 |
| Symbolic writes/results | P-248/P-249 distinguishes inline bits from region-backed references; H-288–H-291/P-254 requires void effects. P-98’s old result/exclusive-arrow surface is superseded in those scopes. | D-10/D-14 |
| Callable values/backing | H-342/P-301 supersedes P-300’s existing-lvalue alias detail; caller destinations remain. H-344/P-303 permits scope-owned heap backing without independent user heap lifetime. | D-14/D-18/D-19 |
| Defer/return | H-356 prepares plain deferred calls at registration; H-361/H-362 specifies explicit proc/func returns/completion replacement. H-365 permits enclosing-scope loop defer with separate iteration restoration. H-363’s pure form remains deferred. | D-10/D-14/D-17/D-18/D-19 |
| Const access | H-366–H-369/P-327 separates binding const from transitive target readonly; ordinary parameter binding qualifiers do not change overload identity. | D-14/D-18/D-19 |
| Families/installation | H-372–H-378 separates definitions, materialization, installation and actual-type dispatch; H-380 limits cancellation to initialization with operand evaluation preserved; H-381/H-382 fixes lexical/inferred-owner lookup. | D-10/D-15/D-18/D-21 |
| Bits ADT placement | H-383/H-384 refines all-user-data-regional placement only for finite bits-eligible ADTs. Ownership stays intrinsic; non-bits regional placement/relocation and same-owner references remain. | D-07/D-13/D-14/D-17/D-22 |
| Execution strategy/input | H-396 directs AOT micro-dispatch with concrete discriminators and call-site facts; root checksums are separate DBMR compatibility planning. ORC is future tracing-JIT only. H-400 supersedes H-390's categorical known-ADT-only input limit: outside-catalog runtime types select through CoD `<any>`, with checked failure when required interpretation is unavailable. No new executable method loading or validation bypass. | D-15r22/D-21r9/D-25r13 aligned under K-593 to H-407/D-26r7; fixed prepared methods, interpretations and unaffected confirmed behavior remain. Historical finite-catalog fixtures do not restrict the current input boundary. |
| Method/implementation/local selection | H-409 distinguishes callable families, user methods and compiler specializations. Proven call sites drive AOT specialization/direct selection; one semantic body may have multiple native implementations and independent local micro-dispatch points. Compiler analysis does not execute runtime effects, and runtime bindings do not register methods/implementations. | D-21r10/D-25r14/D-26r8; H-407 coverage, actual owners/captures and exception behavior continue. |
| Runtime type checks | H-401 retains same-T and static mismatch diagnostics; actual evidence, not `<any>` projection, supplies runtime equality checks. UnsoundRuntimeTyping is unchecked; StaticallyUnknownType is checked under H-400. | D-07/D-17/D-18/D-21/D-26; new exception integration is not established by older validation results. |
| Generic checked obligations | H-405 rejects forcing StaticallyUnknownType into `twice`'s declaration merely because it is generic. Voluntary throws declarations retain the existing fixed caller obligation. No conditional exception-contract feature is selected. | D-26r5/D-17/D-21; genuine unknown-type interpretation remains checked under H-400/H-404. |
| Generic operator contracts | H-402 permits direct generic operation use under ordinary applicability rules. Formal operator/type contracts should be ordinary library definitions supporting readable requirements and navigation, with no new contract intrinsic; implementation is deferred. | D-07/D-11/D-16/D-21/D-26; bodyless declaration details, conversions and fraction choices remain open. |

Known gaps remain scoped above; shortening their records needs no new approval. D-17’s r52 summary is reconciled to r53 by K-555, D-15’s r19 summary to r20 by K-573, and D-08’s r219 overview to the local r220 update by K-305. These index repairs do not ratify whole documents. Report counts and research checkpoints belong in their source evidence.

## 3. Value roots, concepts and review routing

Node descriptions identify topics, not blanket approval. Relationship rows carry rationale/status; §2 carries current document qualification. Contract links support review in either direction through edge endpoints, without adding value or dependency edges. Inspect all edges naming a changed node, then both endpoints and their contracts; dependency cycles cannot prove value.

| Value root | Goal | Charter / human baseline |
| --- | --- | --- |
| <a id="v-01"></a>V-01 | 业务意图不因差异化执行环境而改写。 | §1、§3、§6; H-01；确认目标。 |
| <a id="v-02"></a>V-02 | 有原生 ontology 的领域数据与业务逻辑，可以跨 domain、跨语义层级即时集成。 | §1、§3—5; H-02、H-03；确认目标和层级方向。 |
| <a id="v-03"></a>V-03 | 高度结构化的持久语义单元，兼顾低开销存取、人类阅读与智能体顺手修改。 | §1、§7—10; H-04；确认目标。 |

| Concept | Description | Contracts to inspect |
| --- | --- | --- |
| <a id="c-01"></a>C-01 | Regional GADT 图承载领域知识、事实和业务逻辑意图。 | [D-00](#d-00) |
| <a id="c-02"></a>C-02 | LLVM 实现可按环境差异化的原生 effects handler。 | [D-00](#d-00) |
| <a id="c-03"></a>C-03 | 不同 domain 的多个 regions 可在同一进程即时组合。 | [D-00](#d-00) |
| <a id="c-04"></a>C-04 | Value/type/kind 层级可由 regions 承载。 | [D-00](#d-00) |
| <a id="c-05"></a>C-05 | DBMR 是高度结构化、支持低开销存取的持久语义单元。 | [D-00](#d-00) |
| <a id="c-06"></a>C-06 | DBMR 可快速呈现人类可读的文本源码，并有 IDE 集成。 | [D-00](#d-00) |
| <a id="c-07"></a>C-07 | 提供智能体顺手的语义编辑能力。 | [D-00](#d-00) |
| <a id="c-08"></a>C-08 | memory region 承载 IPC packet，总长度在包头，完整接收后调用入口对象的约定成员方法落地。 | [D-07](#d-07) |
| <a id="c-09"></a>C-09 | DBMR 支持本地 ontology 的检索、统计和 CRUD，日常不回收空间，按节奏重建载荷及 backing 文件压缩。 | [D-07](#d-07) |
| <a id="c-10"></a>C-10 | 显式 ADT/GADT、方法和内存语义；以 SNN 需求同步推进语言及应用设计、实现和验证。 | [D-07](#d-07) |
| <a id="c-11"></a>C-11 | C/C++ 作为与其他语言运行时互操作的兼容接口。 | [D-07](#d-07) |
| <a id="c-12"></a>C-12 | 首个讨论应用为稳态循环激活 SNN：每 CC 一个 region，交换 token 语义向量，目标几百到几万 CC。H-43 明确 CC 是语义呈现基本单元，CC 之间为非精确 replica，minicolumn／神经元的语义响应角色静态分配，不要求网络学习这些身份。当前 Flash 5120 维、每维 60+60；每 CC 按树莓派预算，节点 RAM 上限 16 GB，应用文档独立。 | [D-08](#d-08) |
| <a id="c-13"></a>C-13 | SNN 静态槽集总数以略大于神经元数为明确目标，优先按 scale law 形成便于算术寻址的固定拓扑；不要求保留旧场景连接容量。神经元指向当前集；每槽 4-byte 对端 ID / 1-byte FP8 权重并行数组，每集一个 owner ID。训练使用既存集合迁移，不通过堆分配维护槽拓扑。最小集 100 槽；少出边源的取舍与入边准入分开。 | [D-08](#d-08) |
| <a id="c-14"></a>C-14 | 每语义维分为正、负细胞群体，按身份区分极性，研究非负活动到有符号值的读出。H-27/H-35 的当前讨论限于 Flash 5120 维、每维 60+60；V4 Pro 人口比较暂不讨论。H-54 要求同维正负群体内在抵消（C-18）；上下文、幅度及预测语义需协调。 | [D-08](#d-08) |
| <a id="c-15"></a>C-15 | 以三相行为、入口收敛及有限 token 响应为目标的分阶段连接构建／学习：初步以直接激活链形成 DMN／token 循环，后期 flow 用 Hebbian；immersion 预期涌现，不主动训练。 | [D-08](#d-08) |
| <a id="c-16"></a>C-16 | 在 CC 非精确 replica、语义角色静态分配的前提下研究语义群体位置与生物解剖拓扑的兼容；区分角色、胞体位置、轴突／树突接触机会和存储顺序。 | [D-08](#d-08) |
| <a id="c-17"></a>C-17 | 注意／活动／DMN 依赖的背景白噪音调节：专注要求下全局显著抑噪；低激活比例时提升噪音，包括抵消后的残留仍存在、尚未检测 DMN 的情况；DMN 下支持活动与慢语义收敛。 | [D-08](#d-08) |
| <a id="c-18"></a>C-18 | 同一语义维正负群体的内在相互抑制／激活抵消，作为早期不建模显式抑制性突触连接时的固有作用；互补正面输入后降低活动并保留有用残值。 | [D-08](#d-08) |
| <a id="c-19"></a>C-19 | 柔性窗口内的状态检测／训练观测：DMN 广覆盖、沉浸局限小群、心流在小覆盖内扫过不同群；排除总激活比例并剔除附加白噪音对覆盖的影响。 | [D-08](#d-08) |
| <a id="c-20"></a>C-20 | 正常自循环活动必须保有外部 token 可转向性；没有外部 token 能打破或驱转该循环时才属于 stale。 | [D-08](#d-08) |
| <a id="c-21"></a>C-21 | 对失去外部输入可转向性的 stale 状态考虑系统性重置恢复。 | [D-08](#d-08) |
| <a id="c-22"></a>C-22 | 统一的 DMN/token 激活链构建与修补训练：观察下一步自然激活量，从理论上该激活而未激活的群体选足够比例补足；初建是现有支持不足的情形，持续迭代主要维护有用传播。 | [D-08](#d-08) |
| <a id="c-23"></a>C-23 | 全链约束下逐步选择修补目标：每 epoch 确定维护范围，以近似宽度和主体增强／回返削弱参数规划进程，再利用正确自然激活并补足缺失候选。 | [D-08](#d-08) |
| <a id="c-24"></a>C-24 | 个体神经元的离散时间 rate／有效活动动力学与自然传播终止：保留学习连接，活动可因支持不足或随机失败而停止；半衰期式目标宽松且 token 间可不同。 | [D-08](#d-08) |
| <a id="c-25"></a>C-25 | 分数式神经元选取／参与：以可分贡献表达训练目标，与 rate 协作缓解整细胞计数的语义割裂。 | [D-08](#d-08) |
| <a id="c-26"></a>C-26 | 背景支持下的有用小循环上下文：大链构建避免主动制造有害捷径，同时保留能让较早输入影响后续输出的小循环。 | [D-08](#d-08) |
| <a id="c-27"></a>C-27 | 累积／延后生效的衰减：保留跨小更新的衰减进度，可研究下一数值边界的未来触发。 | [D-08](#d-08) |
| <a id="c-28"></a>C-28 | 按建模量选择 INT8/INT4、无符号／定点或浮点表示，分别论证存储、缩放、累加与动态更新精度。 | [D-08](#d-08) |
| <a id="c-29"></a>C-29 | 两帧轮换的离散时间连入汇集：只读完整前帧，局部计算各目标后写入次帧，完成后交换。 | [D-08](#d-08) |
| <a id="c-30"></a>C-30 | 以长方形场景研究四分的功能关系：局部模式、物体识别、具身空间、自我觉察及后两者反馈。 | [D-08](#d-08) / [D-09](#d-09) |
| <a id="c-31"></a>C-31 | 概念本体由经验抽象形成的预期模型及连接支持的激活链吸引子承载；每个 CC 拥有完整概念动力学空间的非严格副本，感受输入形成位置偏向。 | [D-08](#d-08) / [D-09](#d-09) |
| <a id="c-32"></a>C-32 | 将 5120 维容量与稀疏参与、rate 值域、实用分辨率及时间复用共同研究，区分已学库、即时表达与协同推演容量。 | [D-08](#d-08) / [D-09](#d-09) |
| <a id="c-33"></a>C-33 | 有限精度个体 rate 建模须研究相连神经元 spike 时机的协同效益，分别评价传播与学习，避免把平均精度当作时序信息。 | [D-08](#d-08) / [D-09](#d-09) |
| <a id="c-34"></a>C-34 | 连续神经元动力学及其 AP 输出：连续 AP 来自同一持续演化状态，全局曲线可作为放电阶段输出函数；保留可能使能学习／泛化的机制，避免无需求的精确生理重建。 | [D-08](#d-08) / [D-09](#d-09) |
| <a id="c-35"></a>C-35 | 模块定义一等名义符号；别名保留身份，不同来源／共存版本可区分。 | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-36"></a>C-36 | Per-thread symbolic bindings and scoped effects; procedures capture used definition-time values under resolved identities. Perform temporarily shadows current lookup; behave uses current values. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-37"></a>C-37 | Functions use explicit inputs and mutable memory without dynamic-context access; procedures may use dynamic bindings and have lexical or symbolic names. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-38"></a>C-38 | 用实际 SNN 需求同步推进 CoD 语言与应用的设计、实现和验证。 | [D-08](#d-08) / [D-10](#d-10) |
| <a id="c-39"></a>C-39 | User storage is stack/region; host heap belongs to system code. Regional objects never copy/move; user libraries can recycle storage within a live region. | [D-13](#d-13) / [D-14](#d-14) |
| <a id="c-40"></a>C-40 | Logical business relationships between independently managed regions. | [D-13](#d-13) / [D-14](#d-14) |
| <a id="c-41"></a>C-41 | expression-oriented 语言方向及显式 void；符号读写作为表达式组合。 | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-42"></a>C-42 | 专门词汇表模块只声明符号并辅以注释，供模块共享身份。 | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-43"></a>C-43 | Source-preserving using/import and receiving-context body redefinition through mixin. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-44"></a>C-44 | Complete declaration scanning/registration precedes entry-driven compilation; dependency top-level execution semantics are incorporated in encounter order. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-45"></a>C-45 | Explicit source selectors/local aliases and lower-priority implicit method names, without local module namespaces. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-46"></a>C-46 | Compile-time mixin translation, source-body aliases and destination identities; installation follows the current staged contract. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-47"></a>C-47 | Strict branded scalar/index types, array-declared index domains and meaningful scalar arithmetic; compare newtype and UoM mechanisms. | [D-11](#d-11) |
| <a id="c-48"></a>C-48 | Explicit memory access retains provenance/lifetime; functions may edit permitted storage and return valid views. Const non-bits views restrict holder writes without freezing aliased contents. | [D-14](#d-14) |
| <a id="c-49"></a>C-49 | UoMs are independently named compile-time entities with first-class type identities and identity-preserving local aliases; symbols do not represent them. | [D-11](#d-11) |
| <a id="c-50"></a>C-50 | Pure UoM/index conversion functions use Julia-style argument-type method dispatch to associate operations with array types. | [D-11](#d-11) |
| <a id="c-51"></a>C-51 | Parameter-declared UoM conversion acceptance and compile-time route selection: exact or convertible targets, source route preference and configurable ambiguity warnings. | [D-11](#d-11) |
| <a id="c-52"></a>C-52 | Actor model concurrency with processes, mailboxes and a type parameter for the mail payload. | [D-12](#d-12) / [D-13](#d-13) |
| <a id="c-53"></a>C-53 | One region per graph of objects connected by direct pointers. Each CoD pointer edge stays inside its region. | [D-13](#d-13) / [D-14](#d-14) |
| <a id="c-54"></a>C-54 | Whole-region ownership in ordinary values and scoped effect resources, with explicit consuming transfers distinguished from lookup shadowing. | [D-14](#d-14) |
| <a id="c-55"></a>C-55 | Graph-transfer methods use both owner/root types, abstract root-policy interfaces and actual borrowed region references. | [D-14](#d-14) |
| <a id="c-56"></a>C-56 | Nominal abstract inheritance and explicit data-interface conformance, with head-only declarations across modules. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-57"></a>C-57 | Type-associated method selections, explicit alias/extension permission, lower-priority implicit names and separate mixin redefinition. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-58"></a>C-58 | Explicit publication: public alias → identity; private-by-default source selection. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-59"></a>C-59 | Independent module worlds with identity-keyed information and multiple aliases; receiver-owned construction cannot edit other module worlds. Compilation shapes TLS; runtime effects edit its typed bindings, with inline bits under H-277. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-60"></a>C-60 | Vocabulary modules establish common identities for typed symbols, mainly abstract types with some supporting ADTs, and their method collections. | [D-10](#d-10) / [D-15](#d-15) |
| <a id="c-61"></a>C-61 | Numerical representation and admissibility are independent of compile-time UoM identity. | [D-16](#d-16) / [D-11](#d-11) |
| <a id="c-62"></a>C-62 | Intrinsic fixed-width fractions with independently typed integer components and explicit canonicalization. | [D-16](#d-16) / [D-11](#d-11) |
| <a id="c-63"></a>C-63 | Lexical exception control, scoped cleanup, unchecked allocation failures and explicit exception causes. | [D-17](#d-17) |
| <a id="c-64"></a>C-64 | Destination-view result mode for assignment-family calls, distinct from ordinary const bits results. | [D-14](#d-14) |
| <a id="c-65"></a>C-65 | Ordinary reference-semantic ptr variables with initializer-fixed owners, generic external owners and root-constrained region owners. | [D-14](#d-14) |
| <a id="c-66"></a>C-66 | Explicit symbolic capture dependency through touch @symbol, without referent computation or automatic group capture. | [D-10](#d-10) / [D-14](#d-14) |
| <a id="c-67"></a>C-67 | Explicit checked builtin selection using intrinsic (operation)(...). | [D-10](#d-10) / [D-14](#d-14) |
| <a id="c-68"></a>C-68 | Zero-body-execution range-for else with normal-path initialization joins. | [D-10](#d-10) / [D-14](#d-14) |
| <a id="c-69"></a>C-69 | Distinct reference and bits exchange operations with two destinations and void results. | [D-14](#d-14) |
| <a id="c-70"></a>C-70 | Region is a declaration keyword for whole-resource locals, parameters, results and procedure signatures. | [D-14](#d-14) |
| <a id="c-71"></a>C-71 | Transfer methods return void or checked readonly destination views; indexed writes select logical container operations. | [D-14](#d-14) |
| <a id="c-72"></a>C-72 | Ref/ptr ordinary declarations distinguish compiler-checked initialization from nullable dereference. | [D-14](#d-14) |
| <a id="c-73"></a>C-73 | Unified owner-aware captures for named and anonymous functions/procedures. | [D-18](#d-18) |
| <a id="c-74"></a>C-74 | Structural product region definitions and required field subsets, with language-owned storage mechanics. | [D-13](#d-13) / [D-18](#d-18) |
| <a id="c-75"></a>C-75 | Explicit variable aliases naming existing storage without introducing a new holder. | [D-14](#d-14) |
| <a id="c-76"></a>C-76 | Boundary-pure const functions with private mutable state and deferred const-function TCO. | [D-19](#d-19) |
| <a id="c-77"></a>C-77 | Initial manual root-type compatibility identifier, with later consideration of all participating ADT layouts and checksum-assisted matching. | [D-07](#d-07) |
| <a id="c-78"></a>C-78 | Language support for saving/replacing a disk file with a region snapshot, with application-controlled persistence workflow. | [D-07](#d-07) |
| <a id="c-79"></a>C-79 | Third-party intrinsic extension through a service provider interface; ordinary extensions through CoD libraries. | [D-07](#d-07) |
| <a id="c-80"></a>C-80 | A compiler application designed in CoD: regional frontend/semantic artifacts, persistent stage reuse and selectively bound LLVM services, progressing toward self-hosting. | [D-20](#d-20) |
| <a id="c-81"></a>C-81 | Write-only out reference parameters establish initialized output on normal return; a bare local handle ref declaration allocates uninitialized target storage. | [D-20](#d-20) |
| <a id="c-82"></a>C-82 | Callable views preserve a selected family/context and dispatch by actual argument types, including abstract argument/result flows. | [D-21](#d-21) |
| <a id="c-83"></a>C-83 | Coverage of admissible type combinations and native preparation; replacement timing is under review in D-21/D-25. | [D-21](#d-21) |
| <a id="c-84"></a>C-84 | Infer implemented method result types as checked signature constraints, independently of native result transport. | [D-24](#d-24) |
| <a id="c-85"></a>C-85 | One fused artifact retains reachable parallel module interpretations, including dependency-local callables and procedure-local effects; runtime values and bindings remain execution state. H-400 replaces H-390's known-ADT-only input limit with C-90's unknown-type boundary, without authorizing new executable method loading. | [D-25](#d-25) / [D-15](#d-15) / [D-21](#d-21) / [D-26](#d-26) |
| <a id="c-86"></a>C-86 | One semantic definition per user method, realized by a general implementation and compiler-statically-registered specializations as demanded by proven call sites. AOT prepares local micro-dispatch code; runtime executes its remaining local operation selections within a body, separately from semantic method selection. H-396/H-409/H-410 confirms this direction, not a universal one-native-body limit or permanent implementation representation. | [D-21](#d-21) / [D-25](#d-25) / [D-26](#d-26) |
| <a id="c-87"></a>C-87 | Actual concrete-type discriminators for abstract/polymorphic values; semantic identity/conformance, call-site facts, owner-preserving forwarding and no mandatory second tag. H-396/H-397 principle, including persistent cross-world meaning; permanent carrier/ABI pending. | [D-21](#d-21) / [D-25](#d-25) |
| <a id="c-88"></a>C-88 | Root-associated DBMR type checksums as one compatibility input, separate from runtime dispatch. H-396 planning responsibility; format, loader, dependency/collision policy and migration remain pending. | [D-21](#d-21) / [D-01](#d-01) / [D-25](#d-25) |
| <a id="c-89"></a>C-89 | Finite checked native coverage, separate from runtime recursion, activation state and enumeration of every concrete type description. H-407 confirms its value/acceptance scope. H-400 constrains coverage through C-90; H-406 accepts detected resource-stop diagnostics without a universal analysis-nontermination premise. Concrete analysis procedures remain unselected. | [D-26](#d-26) / [D-21](#d-21) / [D-25](#d-25) |
| <a id="c-90"></a>C-90 | CoD `<any>` is an identified type object for selection when a runtime type is absent from the static catalog. Known concrete identities and same-T relations remain distinct. H-400/H-401 selects checked StaticallyUnknownType for unavailable interpretation and unchecked UnsoundRuntimeTyping for breached typing. H-404 permits valid uninterpreted passage; H-405 preserves generic/voluntary throws distinctions. H-407 confirms this value/acceptance scope. Physical representation and complete integration remain open. | [D-26](#d-26) / [D-21](#d-21) / [D-25](#d-25) / [D-17](#d-17) |
| <a id="gov-01"></a>GOV-01 | 人工确认价值衍生关系后，以合格设计契约委托智能体局部实施。 | [D-GOV](#d-gov) |
| <a id="gov-02"></a>GOV-02 | 保留概念／机制／抽象的关系图、人工证据和延续检查，疑点回到人类。 | [D-GOV](#d-gov) |

## 4. Relationship register

Each row preserves a distinct proposition, endpoints, relation kind, rationale and scope. Historical revisions identify evidence coverage; they do not override §2 or qualify later content. An unspecified formal value path remains pending-human. Local confirmations retain their stated extent; P-31 is rejected. Dependencies never grant authority.

### Founding relationships

| Edge | Child / relation | Parent / target | Rationale | Evidence / scope |
| --- | --- | --- | --- | --- |
| E-01 | C-01 derives-from | V-01 | 用 GADT 保存业务意图，使意图不依附某环境的执行代码。 | H-01；方向性 confirmed。 |
| E-02 | C-02 derives-from | V-01 | 用不同原生 handlers 承接环境差异，保留业务程序。 | H-01；方向性 confirmed。 |
| E-03 | C-02 depends-on | C-01 | handler 解释的是所承载的业务意图；此依赖本身不授予任意 handler 方案。 | H-01；角色关系 confirmed。 |
| E-04 | C-01 derives-from | V-02 | 图同时表达领域数据、业务逻辑及 ontology，而非不透明代码。 | H-02；方向性 confirmed。 |
| E-05 | C-03 derives-from | V-02 | 多 domain regions 共存以实现数据和业务逻辑即时集成。 | H-02；方向性 confirmed。 |
| E-06 | C-04 derives-from | V-02 | 原生 ontology 延伸到 type/value/kind 图，而不局限于业务值。 | H-03；层级方向 confirmed。 |
| E-07 | C-05 derives-from | V-03 | 高度结构化的持久表示服务低开销访问。 | H-04；方向性 confirmed。 |
| E-08 | C-06 derives-from | V-03 | 快速源码呈现和 IDE 让人可以理解并操作结构化语义。 | H-04；方向性 confirmed。 |
| E-09 | C-06 depends-on | C-05 | 呈现的是 DBMR 所承载的语义内容，不是另一份无关程序。 | H-04；角色关系 confirmed。 |
| E-10 | C-07 derives-from | V-03 | 修改机制应适合智能体操作，不以人手编辑二进制为前提。 | H-04；方向性 confirmed。 |
| E-11 | GOV-01 derives-from | V-03 | 以人工确认的局部设计承接任务，既保留价值依据，也减少无关上下文。 | H-05；本次治理要求 confirmed。 |
| E-12 | GOV-02 derives-from | GOV-01 | 记录衍生、依赖和确认，让局部委托与再次确认有据可查。 | H-05；本次治理要求 confirmed。 |

### Storage and application value paths

| Edge | Child / relation | Parent / target | Rationale, evidence and scope |
| --- | --- | --- | --- |
| P-01 | D-01 derives-from | C-05 | Regional representation serves low-overhead structured storage. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-02 | D-01 constrains | C-01 | Physical regional protocols constrain semantic-graph storage. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-03 | D-02 derives-from | C-02 | The C++ driver is a stage-specific native-execution facility. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-04 | D-02 depends-on | D-01 | Driver state uses regional storage. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-05 | D-03 derives-from | C-03 | Explicit versions/dependencies support traceable integration. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-06 | D-04 derives-from | C-05 | Exception guarantees aim to limit failed-operation damage. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-07 | D-04 depends-on | D-01 | Guarantees depend on allocation/lifetime protocols. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-08 | D-05 derives-from | C-06 | Regional YAML conversion supports readable representation. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-09 | D-05 depends-on | D-01 | Conversions require the regional type contract. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-10 | D-05 depends-on | D-06 | Conversions rely on the YAML extension format. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-11 | D-06 derives-from | C-06 | Text/comment rules support human understanding. Candidate, pending-human; scope is the D-01–D-06 registered blob in §2, not new confirmation. |
| P-12 | C-08 derives-from | C-01 | packet 将有类型的入口对象和业务意图带到另一进程执行；DL-02、DL-04、DL-05，需明确哪些定义随包传输。 |
| P-13 | C-09 derives-from | C-05 | 日常单调占用与周期重建尝试兼顾低开销存取及有界存储；DL-06—DL-09，历史／空间／停顿取舍未决。 |
| P-14 | C-10 derives-from | C-01 | 显式 ADT、方法和内存语义使业务图可检查而不依附宿主语言隐含行为；DL-01—DL-03、DL-12。 |
| P-15 | C-11 derives-from | C-02 | 兼容接口使环境侧原生能力可与语义执行连接；DL-11，具体 ABI 与 effects 边界未决。 |
| P-16 | D-07 derives-from | C-10 | 以可定位的问题、备选和人工决定逐步收敛语言契约；全文，整理授权不等于机制确认。 |
| P-17 | D-07 derives-from | C-05 | 将 packet 应用、数据库查询与 compaction 放在同一语义链上检查；DL-03—DL-09、DL-13。 |
| P-18 | D-07 derives-from | C-06 | 将可读源码、身份往返和工具交互纳入语言设计；DL-10，不预定语法或 IDE 协议。 |
| P-19 | D-07 depends-on | C-03 | 引用、存储和样例需保留多 region/domain 集成能力；DL-03、DL-08、DL-13。 |
| P-20 | D-07 depends-on | C-04 | 初始类型核心需解释与 value/type/kind 层级的边界；DL-01、DL-12。 |
| P-21 | D-07 depends-on | C-02 | 方法、互操作和执行策略需明确环境解释与业务图的关系；DL-02、DL-11—DL-13。 |
| P-22 | C-12 derives-from | C-01 | 用显式类型和方法表示 CC 状态、消息及循环激活逻辑，使算法语义可检查；D-08 document-set revision 79，不能据此把数值状态自动宣称为完整 ontology 或已实现的 GADT 程序。 |
| P-23 | C-12 depends-on | C-08 | CC 间瞬时状态交换为 regional packet 的消息身份、应用时序和生命周期提供具体问题；D-08 / SNN-05、SNN-06。 |
| P-24 | D-07 depends-on | C-12 | 以用户选定的首个应用推进语言讨论；D-07 DL-13 的独立案例链接；案例选择不批准其具体求解机制。 |
| P-25 | D-08 derives-from | C-12 | 独立规范应用状态、转移、容量取舍和验收，保留可检查的业务意图；D-08 document-set revision 79 全文，具体机制尚未确认。 K-112 / README107 汇总已记录目标、研究候选和经验比较职责，不改变正式价值链或授予资格。 K-207 / D-08 set202 记录 H-71 实质研究完成并统一当前状态；H-86 经验比较、全部局部人工约束和 pending-human 范围保持，不新增实施资格。 |
| P-26 | C-13 derives-from | C-12 | 按有用连接需求配置容量，以有限存储表达不均匀的上下文复杂度；D-08 document-set revision 79 / SNN-07，按 H-19 以 100 槽为下限且允许忽略少连源；度分布、忽略的任务影响、学习收益与成本需要验证；H-21 的暂定模板与入边计算提案不改变该候选理由，输入准入与输出影响分别验证；H-22 进一步区分储存容量与神经元激活语义；H-23 的权重补偿方向与入度参数评估保留该区分，H-24 将权重尺度策略明确归于训练而非容量配置；形式化价值关系待确认。 K-178 / CapacityPlanning、ConnectivityScalingResearch173 将明确的数组预算假设与 H-73/H-75/H-78 状态比较连接；容量配额、已算总额和需求先于辅助结构原则保持。 K-200 / CapacityPlanning195 将已记录的连接扫描算术纳入节点规划；RAM、保留度数、更新节奏和节点 CC 数分别约束可行性，不把全预留扫描当作强制算法或实测带宽。 K-202 / AdaptiveConnectivity、CapacityPlanning、ConnectivityScalingResearch197 精简 authority 摘要并直连独立 flow 文档；保留正文合同、来源、算术、开放选择及全部 H/K 历史，未授予实施资格。 |
| P-27 | C-14 derives-from | C-12 | 用固定极性群体表达有符号语义坐标，使非负细胞活动与业务输入／输出的关系可检查；D-08 document-set revision 82 / SNN-01—SNN-03、SignedPopulationEncoding。合理性与权衡待确认，H-25 仅请求分析。 H-75 明确同柱个体的上下文路径价值，未选择完整读出或 HTM 学习机制。 H-76 的强预测／弱广激活输入使个体上下文路径参与当前语义输入；分支明确，数值校准及完整关系待确认。 H-84/AD-090 明确广范围弱激活为生成模式显式操作，不依靠普通神经连接传播实现且不予学习；不能从该 fallback 选取 flow 教学／强化群体。陌生转移的学习机制另议，HTM 细节仅作分角色参考。 H-85/AD-091 将预测态正负并存／净抵消保持为将来对照选项，H-77 活动态操作不变；H-86/AD-092 允许明显经验问题直接保留比较空间。 Research/model choices remain pending in D-08; checkpoint-specific qualifications remain in the historical source. |
| P-28 | C-14 depends-on | C-13 | 正负群体人口需要有界槽池、身份、权重和迁移机制；D-08 document-set revision 79 / SNN-04、SNN-07。该依赖不规定 alpha、权重尺度或池必须加倍，也不授予资格。 |
| P-29 | C-15 derives-from | C-12 | 让连接构建／学习服务可检查的三相行为、入口收敛和有限 token 响应；D-08 当前覆盖表所列修订 / SNN-03/SNN-05/SNN-07、TrainingStrategy 及相关研究。H-61 提议直接构建 DMN／token 链、后期 Hebbian flow、immersion 涌现，并确认当前省略物理距离偏好；H-62 请求批组替代方案比较。方法方向是初步人类提案，形式化衍生链与参数算法仍 pending-human；H-41/H-42/H-50 边界不变。 H-63 的持续修补让训练维护既有行为；目标缺额与自然衰减分开，正式机制仍 pending-human。 H-79—H-81 的 flow 分支为 Hebbian/HTM 方向及初步设计授权，未选具体算法；不把 C-22 修补合同外推。 H-82/AD-088 以预测性预激活群与输入序列高度匹配作为 flow 直接训练验收；无需自主持续生成，生成算法另行设计。具体度量和学习规则仍待定。 H-83/AD-089 要求按序列长度验证：共享匹配片段允许多个预测，随上下文增长向训练序列收敛；不预设固定单候选或对每个未到达预测等量削弱。 H-84/AD-090 明确广范围弱激活为生成模式显式操作，不依靠普通神经连接传播实现且不予学习；不能从该 fallback 选取 flow 教学／强化群体。陌生转移的学习机制另议，HTM 细节仅作分角色参考。 H-85/H-86 preserves empirical comparison space; no learning algorithm, readout, normalization or update rule is selected. Research qualifications remain in D-08 and the historical source. |
| P-30 | C-15 depends-on | C-13 | 学习产生的连接受静态槽容量约束，容量迁移须保持身份和既有边；几何位置的保持仅在未来另经确认引入几何时适用，不预设当前已有位置；D-08 当前覆盖表所列修订 / SpatialConnectivityResearch、AdaptiveConnectivity、IncomingConnectivity、TrainingStrategy。仅依赖，不选择 degree exponent、weight scale 或空间距离核；pending-human。 K-103 区分源选择、既存边适配、缺失边发现和逐档容量可行性；不采用增长／删除、排序或分配结构。 K-107 / SynapticIntegrationResearch102 说明容量供给与整合选择性不同；不采纳树突分组或新增存储。 K-122 的条件连入遍历计数区分实际目标度数、源信用和写回数；不选择反向索引或元数据。 K-163 / FlowLearning158、SynapticIntegrationResearch158 明确 HTM 证据的扰动及人口缩放条件，不选择编码、门限或新状态。 |
| P-31 | C-15 depends-on | C-14 | **rejected**：H-42 / AD-035 拒绝本边原来服务的共享内部位置机制。revision 26 中对正负身份／上下文的依赖只留历史；不得据此延续等位 prior。 |
| P-32 | C-16 derives-from | C-12 | 检查语义分配与解剖位置的兼容，避免物理建模假设暗改 H-43 的语义呈现及学习边界；D-08 document-set revision 79（局部修订见覆盖表）/ SemanticCorticalMappingResearch、PhysicalDistancePreferenceResearch。H-50 明确语义安排已存在而物理度量不存在，H-51 要求深化其计算意义研究；H-44 授权研究；H-46 澄清 feature/minicolumn 非双射并不构成冲突。三状态目标不批准群体交错或其它映射；候选价值关系 pending-human。 H-61 确认当前省略物理距离倾向；该解剖兼容研究不再是当前构建设计的前置任务，候选研究价值关系保持 pending-human。 |
| P-33 | C-16 depends-on | C-14 | 拓扑研究须保留静态维度／极性归属，区分未定数值读出与角色身份；D-08 revision 28 / SemanticCorticalMappingResearch、SignedPopulationEncoding。仅依赖，不批准重编码、共同坐标或 replica 同步机制；pending-human。 |
| P-34 | C-17 derives-from | C-12 | 专注时降低背景干扰，活动不足时增噪并支持 DMN，服务柔性活动目标及残留增长／回落；D-08 revision70 / ActivationRegimes、Model、Decisions。功能方向据 H-53/H-55，正式价值链和控制算法 pending-human。 K-110 / ActivationRegimes105 区分噪音与活动占比的条件响应、反馈时序和有限驱动可达性；不选控制器、阈值、窗口或额外状态。 K-115 / ContextLoopsAndNumerics110 给出固定帧内噪音尺度下的 60+60 条件矩，不把无符号活动增加等同于语义残值增强。 K-120 / ContextLoopsAndNumerics115 以阈值响应的精确条件矩区分噪音使残留可响应、均值放大与可靠信息；不选转移函数或控制器。 K-177 / ContextLoopsAndNumerics172 以条件噪音协方差与抵消后组成分析核查语义／上下文支持价值；未新增噪音机制或状态。  K-185 / RateDynamics180、frame-time-noise-analysis 区分正值随机增量的均值与方差随帧长变化，不将零均值扩散缩放暗作激活输入律。未选噪音过程、偏置或新状态。 |
| P-35 | C-17 constrains | C-15 | 普通单次 token 的低噪音响应与抵消后活动不足增噪是不同的训练／比较条件；需验证残留先增强达峰再回落，不能把驱动差异当作连接／几何收益。D-08 revision70；H-53/H-55 的业务条件明确，正式约束关系及训练公式 pending-human。 K-110 要求实际比较区分固定驱动与完整反馈、冻结权重与学习后的响应；机制仍未选。 |
| P-36 | C-18 derives-from | C-12 | 通过同维相反群体活动抵消服务互补正面输入的活动降低与有用残留目标；D-08 revision84 / SignedPopulationEncoding、ActivationRegimes。H-54 的功能目标及 H-77 的活跃 rate 净余／比例保留操作明确，正式价值关系 pending-human。 K-168 / SignedPopulationEncoding163、Model163 明确暂态与完成后的活跃极性总量；净值可恢复 H-77 后的总量但不恢复细胞上下文，未选消息合同。  K-182 / SignedPopulationEncoding177、ActivationRegimes177、MR-S083 和 rated_valence_geometry 记录词评分／输入向量的描述性比较及反例；不把语义正负等同坐标相反或实际 DMN 状态，不选编码或评估器。  K-183 / SignedPopulationEncoding178、ActivationRegimes178 和 cancellation-properties-analysis 以同一净向量下不同细胞参与数的精确例区分调节量与幅度；不选门限或招募配额。 K-186 / SignedPopulationEncoding181、ActivationRegimes181、MR-S094 和 chinese_rated_valence_geometry 扩展至独立选取的中文评分词与真实单 token 向量；两种语言样本非配对翻译，不把等强净幅度或词评分当作活跃计数、残留意义或 DMN 证据。 K-205 / ContextLoopsAndNumerics200 将噪音单独覆盖例的独立、对称、无保留状态条件明确写出；不将其用作实际回返轨迹估计，不选噪音律、计数门限或检测结构。 |
| P-37 | C-18 depends-on | C-14 | 抵消依赖稳定的维度／极性归属及可比较的活动语义；D-08 revision84。只给依赖，H-77 已规定活跃 rate 的全量净余及比例保留并拒绝下标配对；预测相态、完整读出和新增存储不因依赖获批；pending-human。 |
| P-38 | C-18 constrains | C-15 | 早期连接学习不得借显式抑制性突触实现目标，需在内在正负群体抵消条件下评估学习动态；D-08 revision84，H-54/H-77 范围明确，正式约束链 pending-human。  K-144 / FlowLearning139、cancellation-properties-analysis 保留单步抵消与时序学习比较，不选择梯度法或预测抵消。 |
| P-39 | C-19 derives-from | C-12 | 用可观察的去噪窗口覆盖和路径使三相吸引／收敛目标可判别与训练，避免受调节的总占比代替相态；D-08 当前覆盖表所列修订，H-56—H-58 方向明确，正式关系 pending-human。 K-138 / ActivationRegimes133 给出 H-77 条件下广覆盖所需的跨极性参与下界；示例比例不变成固定阈值，向量几何不替代语义效价。 K-139 / ActivationRegimes134 区分双神经帧与观测历史，按所需行为比较 block／rolling／fading 窗口；不选窗口或字段。 K-176 / ActivationRegimes171 区分候选检测描述与已证实的可辨识性；相同允许观测无法单独证明不同因果来源，正式关系及具体规则仍待定。 |
| P-40 | C-19 depends-on | C-12 | 窗口覆盖需要 CC 神经元身份、可计数的活动定义及时间口径；D-08 当前覆盖表所列修订，仅依赖，不选 FP4 相态、采样器或历史数组；pending-human。 |
| P-41 | C-19 constrains | C-17 | 总激活比例进入活动调节但不进入相态分类，DMN 标签依窗口覆盖／路径；低占比增噪不自动表示 DMN。D-08 当前覆盖表所列修订，H-55/H-56 的职责边界明确，正式关系 pending-human。 |
| P-42 | C-20 derives-from | C-12 | 使正常自循环保持外部输入可驱转，从而服务 token 语义交互及状态转换；D-08 revision71 / ActivationRegimes、Model、Decisions、PhysicalDistancePreferenceResearch。H-59 的行为要求明确，正式价值链及运行判据 pending-human。 |
| P-43 | C-21 derives-from | C-20 | 在输入驱转能力丢失时考虑通过重置恢复该能力；D-08 revision71。H-59 仅提出可能需要，价值／代价及策略均为候选，pending-human。 |
| P-44 | C-21 depends-on | C-12 | 重置需先定义被影响和保留的活动、预测、连接及持久状态与恢复入口；D-08 revision71 / SNN-03/SNN-05/SNN-06。仅依赖，不授予清除状态或改动区域协议的实施资格；pending-human。 |
| P-45 | C-22 derives-from | C-15 | 以预期下一步响应与自然响应的缺额驱动持续修补，使初建和后续训练共同维护 DMN/token 行为及有限响应；D-08 当前覆盖表所列修订 / TrainingStrategy。H-63 明确提出方向，目标定义、教学与连接更新及形式化价值链仍 pending-human；后期 Hebbian flow 不因此被替换。 H-64 进一步要求兼顾预期进程及避免意外捷径，局部补足数量不足以说明完成目标。 H-79—H-81 明确范围为 DMN/token；flow 算法初设不继承此修补机制。 K-124 用条件反复补缺例说明修补后单独成功不证明与其他训练结果共存；不改变 H-63 的算法开放范围。  K-194 / TrainingStrategy189、MR-S098 深查嵌入网络的链传播、幅度保真及交叉激活证据；保留实际共同背景下的维护验收，不导入持续输入、同步、几何或细胞机制。 |
| P-46 | C-22 depends-on | C-13 | 修补必须适配静态 SoA 槽池、累计活跃入度及可用容量类；已有边调权不新增端点槽，新增独立边才计入分配，不能按完整批组重复建边。D-08 document-set revision 79 / TrainingStrategy、AdaptiveConnectivity；技术依赖 pending-human，不选更新优先级、配额或堆结构。 |
| P-47 | C-22 depends-on | C-14 | 理论应激活群体及缺额口径须保留固定坐标／极性与60细胞约束；候选目标不可任意跨语义补足，内在抵消和相态数值需纳入实际响应。D-08 document-set revision 79 / TrainingStrategy、Model；仅依赖，不采纳固定入口、精确配额算法、噪音因果标签或新编码；H-66 的每步／少数几步分布偏好约束目标，允许幅度增强与淡出。 |
| P-48 | C-23 derives-from | C-22 | 将局部补足激活纳入预期完整进程，使维护保留有用的顺序、回返及有限响应，避免宽度足够却走捷径；D-08 document-set revision 79 / TrainingStrategy。H-64 授权分析并提出方向，具体价值权衡与完整契约 pending-human。 H-65 以维护目标决定 epoch 范围与增强／削弱策略，参数角色明确而详细传递函数未定。 H-66 要求按连接回返的预期分析有限响应，并保持短时语义比例；有限人口与循环增益的具体取舍仍 pending-human。 |
| P-49 | C-23 depends-on | C-13 | 前瞻计划需考虑累计入度、可用容量类与共享图，当前可分配不代表以后步骤也可完成；D-08 document-set revision 79 / TrainingStrategy、AdaptiveConnectivity。仅依赖，不新增预留数组、出边索引或全图检查预算。 |
| P-50 | C-23 depends-on | C-14 | 后续步骤的语义候选域与每坐标／极性60细胞限制影响不重叠分配和回返，不能用任意神经元补足稀缺语义角色；D-08 document-set revision 79 / TrainingStrategy、Model、SignedPopulationEncoding。H-65 固定每 epoch 的 token 比例范围；H-66 偏好每步保持原向量分布，最多允许少数几步的有限偏离。原向量比例、整数范围、完整后继响应和实际强度须区分；窗口、容差、读出及采样算法仍待定。仅技术依赖，不授予实现资格。 H-67/H-69 开放 rate 与分数参与；整数不重用人口界仅在对应假设下适用，短时向量目标仍保留。 |
| P-51 | C-23 constrains | C-22 | 修补候选须在明确历史／回返范围内评估，并关注未来可行性及已有图造成的意外捷径；自然提前回激不能自动充当正确后继。D-08 document-set revision 79；H-64 指定要避免的现象，具体排除窗口、允许边、搜索与响应判据 pending-human。 H-66 的语义偏好适用于正确自然参与加修补后的完整后继群体，缺额子集不能无视已有参与再机械照搬全 token 比例；此为 K-82 的候选解释，不是已选算法。 |
| P-52 | C-24 derives-from | C-15 | 用明确的时间／活动强度语义表达可维持又可自然淡出的循环响应，服务三相行为与 token 语义转向；D-08 当前覆盖表所列修订 / RateDynamics、Model、ActivationRegimes、SynapticIntegrationResearch。H-67 授权讨论而非选定模型；正式价值权衡及实现契约 pending-human。 K-165 / RateDynamics160 明确 rate 降阶已包含的波动与显式噪音的意义，不选择噪音律或适应时间常数。 K-166 / RateDynamics161 区分平均与 H-77 抵消的次序，不以 ensemble 平均替代当前快照或加入分布状态。  K-169 / RateDynamics164、ContextLoopsAndNumerics164 以相同语义淡出下不同细胞组成保持的条件例，区分快照执行和自保持动力学；未选 leak、时间步或状态字段。  K-172 / RateDynamics167 深查 population-rate 降阶条件；不以群体平均加速替代个体上下文身份或宣称 CPU 收益。  K-185 / RateDynamics180、frame-time-noise-analysis 区分正值随机增量的均值与方差随帧长变化，不将零均值扩散缩放暗作激活输入律。未选噪音过程、偏置或新状态。 |
| P-53 | C-24 depends-on | C-14 | rate、分数与实际有符号输出须保留固定极性、预测／当前相态及内在抵消；D-08 revision84 / RateDynamics、SignedPopulationEncoding。依赖不允许静默重编码、丢弃上下文身份或改变活跃比例口径；pending-human。 H-76 输入需读取既有预测状态，H-77 规定活跃 rate 抵消；相态及时间边界不能隐式合并。 H-84/AD-090 明确广范围弱激活为生成模式显式操作，不依靠普通神经连接传播实现且不予学习；不能从该 fallback 选取 flow 教学／强化群体。陌生转移的学习机制另议，HTM 细节仅作分角色参考。 H-85/AD-091 将预测态正负并存／净抵消保持为将来对照选项，H-77 活动态操作不变；H-86/AD-092 允许明显经验问题直接保留比较空间。 |
| P-54 | C-24 constrains | C-22 | 修补需比较同一单位的期望和自然有效贡献，既有成功活动仍须计入；自然衰减不被补回恒定值，终止不删权重。D-08 当前覆盖表所列修订 / RateDynamics、TrainingStrategy；具体更新规则 pending-human。 |
| P-55 | C-25 derives-from | C-22 | 分数参与让缺额及弱语义分量不被过早四舍五入为整细胞，服务目标驱动的维护与短时语义保持；D-08 revision81。H-69 提出方向；双因素语义、收益／代价及正式关系 pending-human。 |
| P-56 | C-25 depends-on | C-24 | 分数与 rate 的条件含义及单位决定乘积、波动和持续状态，须避免重复衰减或重复存储；D-08 revision81 / RateDynamics。仅依赖，不确认 f*r 为最终公式或双字段布局；pending-human。 |
| P-57 | C-25 depends-on | C-14 | 每坐标／极性有效贡献与原向量之间需明确读出、精度、低信号误差和一次输入映射；D-08 revision81 / RateDynamics、SignedPopulationEncoding。H-66 短时语义目标保留，不引入运行时按已知 token 强制投影；pending-human。 |
| P-58 | C-25 constrains | C-23 | 分数参与和持续 rate 使整细胞不重用计数界成为条件比较，进程约束应区分持续活动、合法回返和意外捷径；同一身份分配给多个步骤仍共享其输出边。D-08 revision81 / RateDynamics、TrainingStrategy；具体历史与重叠规则 pending-human。 |
| P-59 | C-24 depends-on | C-13 | 当前静态 SoA 端点池可作为候选存储基础，但 rate 更新的扫描工作、数值状态及计算精度须另由算法说明；D-08 revision81。不改变容量预算假设、不预设缓存／索引或额外数组；pending-human。 |
| P-60 | C-26 derives-from | C-15 | 保留上下文对后续语义行为的影响，使有用记忆不只依赖大链主干延长输出；D-08 当前覆盖表所列修订 / ContextLoopsAndNumerics、TrainingStrategy、ActivationRegimes。H-71 确认方向，具体机制与正式关系 pending-human。 Research/model choices remain pending in D-08; checkpoint-specific qualifications remain in the historical source. |
| P-61 | C-26 constrains | C-23 | 主链选取中的防捷径偏好不得变为强力消除全网小循环；按实际进程破坏、语义干扰和不可驱转判错；D-08 revision84。H-71 方向明确，识别／干预策略 pending-human。 |
| P-62 | C-26 depends-on | C-24 | 小循环上下文需要明确 rate、噪音、自然失续／再激活及读出时间尺度；主导幅度与历史影响不同；D-08 当前覆盖表所列修订。技术依赖不选择独立上下文状态或专门网络，pending-human。 |
| P-63 | C-27 derives-from | C-24 | 保留小于表示量子但长期重要的衰减进度，使近似低精度执行仍服务实际淡出与维持；D-08 当前覆盖表所列修订 / ContextLoopsAndNumerics、RateDynamics。H-71 要求分析，具体定时／累积机制 pending-human。  K-173 / ContextLoopsAndNumerics168 区分源变化稀疏与连入依赖／同维抵消传播的影响范围；不预设事件索引、缓存或重算算法。  K-198 / ContextLoopsAndNumerics193、MR-S002 核查 QSS 的总量化误差（包括滞回）趋零及观察时域条件；不把收敛结论当固定低精度任意时长保证，未选事件结构或滞回机制。 K-204 / ContextLoopsAndNumerics199、numeric-representation-arithmetic 的 directed_decay_rounding 区分向下取整避免停滞与保留衰减时间；有限归零不是半衰期／自然断链的充分证据，不选取整律或新状态。 |
| P-64 | C-27 depends-on | C-13 | 延后生效需要与静态区域、SoA 和所需状态预算协调，按算法说明锚点／残差／事件的真实代价；D-08 当前覆盖表所列修订。仅依赖，不预分配定时器数组或堆队列，pending-human。 |
| P-65 | C-28 derives-from | C-24 | 以可检查的误差、范围和资源代价选择数值表示，避免存储便利悄改循环、抵消、衰减与训练语义；D-08 当前覆盖表所列修订。H-70/H-71 要求精度与整数比较，形式化取舍 pending-human。 Research/model choices remain pending in D-08; checkpoint-specific qualifications remain in the historical source. |
| P-66 | C-28 depends-on | C-14 | 位宽、符号、尺度与相态须保持有符号语义分量和残差含义，不能由整数格式自动推导新编码；D-08 revision84 / SignedPopulationEncoding、ContextLoopsAndNumerics。pending-human，仅依赖。 |
| P-67 | C-28 depends-on | C-13 | 端点 ID、权重、状态与可能新增元数据分别计费；INT4 权重不自动使五字节槽减半，累加宽度另论；D-08 revision84。容量基线与所有权不变，pending-human。 |
| P-68 | C-28 constrains | C-27 | 衰减的计时边界与小数进度受量化、缩放及舍入影响；不能把反复舍入停滞或重设时间当作自然记忆；D-08 当前覆盖表所列修订。具体误差容差／调度规则 pending-human。 |
| P-69 | C-29 derives-from | C-24 | 使一次神经转移只依赖同一前帧，避免目标访问顺序改写活动传播；低成本双状态数组配合现有入边布局，服务明确可复现的业务时间语义。D-08 当前覆盖表所列修订 / RateDynamics、IncomingConnectivity、Model；H-94 确认双帧拉取及避免无需求运行时复杂化的局部价值范围；该范围 confirmed。完整神经机制及尚未确认的上游价值链仍 pending-human。 K-113 / RateDynamics108 区分观察次序、模型时长、神经帧与学习曝光；不选事件队列、每 token 一帧或额外历史状态。 K-116 区分相同前帧快照与抵消前后不同数值转换边界；不增加帧或持久状态。 K-164 / RateDynamics159、FlowLearning159 区分状态布局与实际传导、输入和学习角色；不选择门控或新增状态。 |
| P-70 | C-29 depends-on | C-13 | 连入集提供目标拥有的源 ID/权重遍历；双帧仅复制所需状态字段，静态 SoA、容量与区域约束保留。D-08 set204；H-94 确认下游入连归属与双帧 pull 的依赖，局部范围 confirmed。依赖不授予实施资格，不选择并行调度器、第三帧、事件索引或连接池副本。 K-108 / IncomingConnectivity103 从真实遍历／增长／迁移需求比较保留前缀的端点标记与计数，不预设辅助数组。 |
| P-71 | C-30 derives-from | C-12 | 用同一长方形场景明确感知、身体参照和自我觉察的可观察联系；D-09r1 S1/S2，H-87 研究范围，正式关系待确认。 |
| P-72 | C-31 derives-from | C-12 | 让概念内容落实到可习得、可泛化的预测动力学，避免用 token 名称代替学习；D-09r1 S1/S3，H-88/H-89 前提，正式关系待确认。 |
| P-73 | C-32 derives-from | C-12 | 以任务误差、时间和资源界定实用容量，服务有界成本下的语义推演；D-09r1 S4/S8，H-89/H-90 研究要求，正式关系待确认。 |
| P-74 | C-33 derives-from | C-24 | 保留 rate 降阶后仍影响传播和学习的时间关系，避免只改数字精度而丢掉因果作用；D-09r1 S5—S8，H-91 方向，正式关系待确认。 |
| P-75 | C-30 depends-on | C-31 | 四分场景须使用网络已学得的对象、身体与观察概念；D-09r1 S2/S3，依赖不选择训练机制或授予权威。 |
| P-76 | C-33 depends-on | C-28 | 比较时序候选须给出 rate、权重、平均窗口及保留状态的数值语义；D-09r1 S5/S6，依赖不选择格式或字段。 |
| P-77 | D-09 derives-from | C-30 | 将四分探索收敛为场景、范围和功能问题；D-09r1 S1/S2/S8，H-92 授权整理，正式关系待确认。 |
| P-78 | D-09 derives-from | C-31 | 保存概念习得与近似副本前提及其训练义务；D-09r1 S1/S3/S8，H-92 授权整理，正式关系待确认。 |
| P-79 | D-09 derives-from | C-32 | 保留容量假设、证据边界和负荷／精度／时延比较；D-09r1 S4/S7/S8，H-92 授权整理，正式关系待确认。 |
| P-80 | D-09 derives-from | C-33 | 明确时序协同的证据、降阶候选、信息损失与成本问题；D-09r1 S5—S8，H-92 授权整理，正式关系待确认。 |
| P-81 | C-31 constrains | C-30 | 四组 CC 的分工不能解释为各自仅拥有一部分概念；每 CC 的完整近似副本及感受偏向是 H-89 的前提。D-09r1 S2/S3；正式约束登记待确认，不选择复制／同步算法。 |
| P-82 | C-34 derives-from | C-33 | 连续状态保留 AP 之间对时序协同可能有用的历史；以学习／泛化价值决定机制保留和复杂度。D-08 set204、D-09r2 S5.1；H-93/H-95 确认命题与目的，本正式价值衍生作为 agent 候选登记，pending-human，不补批 P-74/P-52 等上游关系。 |
| P-83 | C-34 constrains | C-29 | 下游读取源 effect 不推进、不消耗源状态，所有读取对应同一源时间视图；多帧 AP 不自动要求独立尾部队列或 push 传播。H-93/H-94 确认这一局部约束，confirmed；覆盖 D-08 set204 与 D-09r2 S5.1，具体状态／历史布局不在范围内。 |

### Symbols, calls and numerical meaning

| Edge | Child / relation | Parent / target | Rationale, evidence and scope |
| --- | --- | --- | --- |
| P-84 | C-35 derives-from | C-10 | Distinct symbol identities make language composition unambiguous across aliases/origins/versions; D-10r1 identity section. User mechanism H-96; formal value path pending-human. |
| P-85 | C-36 derives-from | C-02 | Explicit scoped bindings let an operation use its captured definition-time interpretation under perform or the current interpretation under behave; H-97/H-180, D-10r39. Candidate value path remains pending-human, with no general effect calculus selected. |
| P-86 | C-37 derives-from | C-01 | Separating pure computations from effectful execution keeps program dependencies inspectable; D-10r1 callable section. Formal value path pending-human. |
| P-87 | C-38 derives-from | C-12 | Use real simulator operations to choose language features and validate meaning/cost together; H-96 direction, D-08 LanguageCoDesign205. Formal value path pending-human; no equation selected. |
| P-88 | C-36 depends-on | C-35 | Capture/access requirements need stable symbol identities rather than spelling equality; D-10r1. Dependency proposal only. |
| P-89 | C-37 constrains | C-36 | H-98 confirms functions cannot access symbolic effects or invoke procedures. H-184 permits lexical and symbolic procedure names, superseding H-103’s exclusive naming restriction while preserving effectfulness. H-185 separates lexical direct calls from symbolic perform/behave. Module initialization remains a top-level dynamic scope. D-10r41 local scope, not full grammar/checker approval. |
| P-90 | D-10 derives-from | C-10 | Give explicit symbol/callable semantics a focused contract and reviewable cases; D-10r1 all sections. Formal value path pending-human. |
| P-91 | C-38 depends-on | C-29 | First frame example preserves the H-94 incoming-pull, immutable-A/B-publication contract; D-08 LanguageCoDesign205. Dependency does not qualify language implementation. |
| P-92 | C-36 depends-on | C-03 | Captured regional values and current region contexts need valid owner/lifetime meaning; D-10r1. Dependency proposal, no cross-region representation selected. |
| P-93 | D-08 depends-on | D-10 | LanguageCoDesign205 and README205 use scoped symbols/callables for the implementation plan; other application models do not acquire language-specific mechanisms automatically. |
| P-94 | C-39 derives-from | C-10 | Explicit stack/region ownership makes user storage and captured value lifetimes part of language semantics; D-10r1. Human memory boundary H-99/H-100; formal value path pending-human. |
| P-95 | C-39 constrains | C-36 | H-99 confirms stack/region user carriers and no implicit user heap lifetime. H-180 reaffirms permitted dynamic-value capture; it does not copy regional objects or create another owner. H-173/H-175’s full allowed-type/interior-pointer boundary remains under review. D-10r39 local scope; representation remains open. |
| P-96 | C-40 depends-on | C-03 | Business associations must account for independently managed regions and contexts. H-101 supplies the original placeholder, and H-157/P-140 supplies the current language scope. Dependency proposal `pending-human`. |
| P-97 | C-41 derives-from | C-10 | Expression results and explicit void make computation/effect composition visible in types; H-104 direction, D-10r1. Formal value path pending-human. |
| P-98 | C-41 constrains | C-36 | H-104/H-113 establishes symbolic read/write expressions and excludes ordinary lexical symbolic variables. H-111’s effect-result rule and the former exclusive-arrow write surface are superseded by H-277–H-281/P-248 and H-288–H-291/P-254: bits transfer, object-reference rebinding and void effects follow the current D-10 contract. Preserve scoped installation/restoration; this does not qualify the whole language. |
| P-99 | C-42 derives-from | C-35 | A symbol-only vocabulary lets independent modules share nominal concepts without selecting implementations; H-105 practice, D-10r1. Formal value path pending-human. |
| P-100 | C-43 derives-from | C-10 | Candidate value: explicit source-preserving selection and receiving-context body reuse make definition meaning inspectable. H-105 supplies the distinction; H-212 names redefinition mixin and uses import for selection/extension. D-10r52; full value path remains pending-human. |
| P-101 | C-43 depends-on | C-35 | Procedure reuse must distinguish symbol identities/aliases from definition contexts; D-10r1. Dependency proposal; destination-binding syntax remains open. |
| P-102 | C-43 constrains | C-36 | H-105/H-180/H-212: using/import retains source definition/captures; mixin redefines the body and captures in receiving lexical/dynamic context. D-10r52; perform replay, behave current lookup and owner-preserving shadows are unchanged. Import must not silently recapture effects. |
| P-103 | C-44 derives-from | C-10 | Make declaration availability and execution/capture order explicit in language semantics; D-10r2 module section, D-07r5 DL-12/DL-14. H-106 supplies the mechanism; this formal value path remains pending-human. |
| P-104 | C-44 constrains | C-36 | H-106/H-107 confirmed: top-level symbol/function/bits-global definitions, registered procedure bodies and effect/perform/behave expressions. Bits globals use literal or function-call initializers. All entry dependencies are scanned/loaded before entry execution compilation; dependency execution semantics are incorporated in encounter order. Initializers, dynamic definitions and top-level effects/calls execute with the module, not during scanning. H-109 confirms every cross-module reference as trigger; H-110 closes the source scope after execution and limits effective-resource transfer to using/import. D-10r4 local scope; H-140 requires once-only incorporation in encounter order after scanning; cyclic initialization remains open. D-10r13.  H-166 supersedes only source initialization-return teardown; module top-level scopes last until process exit, D-10r38. |
| P-105 | C-44 depends-on | C-43 | Source-preserving selection and mixin redefinition must participate in scan-first, once-only entry-driven module incorporation. D-10r52; H-109/H-140/H-166 retains encounter order and process-lifetime module scopes. H-212 keyword changes do not cause scan-time execution or repeated initialization. Dependency grants no new qualification. |
| P-106 | C-45 derives-from | C-35 | Explicit source selection exposes nominal provenance while short local aliases preserve readable operations; H-120 makes abbreviated using require identity agreement, with explicit as for intentional different-identity destinations. D-10r10 origin/alias section. Candidate value path pending-human. |
| P-107 | C-45 depends-on | C-43 | Naming a symbol identity must be distinguished from retaining or redefining a procedure resource; D-10r10. Dependency only; H-118 authorizes the bounded identity-only selector implementation, without qualifying a complete resource-selection contract. |
| P-108 | C-45 depends-on | C-44 | Every source selection retains H-109 incorporation and H-166 process-lifetime obligations; identity-only naming is not an initialization exemption. D-10r10. Combined using represents one source encounter rather than two sequential selections; this behavior is implemented and validated under H-121. No new repeated/cyclic encounter policy. |
| P-109 | C-46 derives-from | C-43 | Compile-time alias adaptation makes receiving-context body reuse explicit and destinations independently selectable; D-10r10. H-115/H-116 mechanism statements do not qualify this complete candidate value path. |
| P-110 | C-46 depends-on | C-35 | Translate source/receiving aliases to nominal compile-time values before runtime layout; aliases are not runtime variables or names. H-114 constraint; D-10r10. Mapping edge policies remain open. |
| P-111 | C-46 constrains | C-36 | H-113–H-121/H-212: body alias maps belong to mixin; capture follows receiving identity resolution, with known destination symbols and using identity guards. D-10r52; no ordinary symbolic assignment, identity unification or runtime alias lookup. Source-preserving import does not apply a body map. |
| P-112 | C-47 derives-from | C-10 | Candidate: meaningful scalar identities and typed indexing make domain mistakes visible in typed programs rather than host numeric conventions. D-11r4; H-125 requirement, not full value-path confirmation. |
| P-113 | D-11 derives-from | C-47 | Candidate: compare usable arithmetic and actual index formulas before choosing a mechanism, serving strict domain identity with lower caller burden. H-126 requests comparison; D-11r4 remains pending-human. |
| P-114 | C-48 derives-from | C-37 | Candidate: useful operations over explicitly supplied complete structures without dynamic-context access. H-197/H-198 adopt mutable functions; earlier snapshot guarantees are superseded. D-10r47/D-14r8 value-path qualification remains pending outside the identified local decisions. |
| P-115 | C-48 depends-on | C-39 | Snapshot traversal needs valid regional references and lifetimes, without copying/moving regional objects or introducing user heap storage. D-10r12 dependency; does not confer authority. |
| P-116 | C-47 constrains | C-38 | H-123/H-125 local confirmed scope: the planned language/runtime increment includes structured types and strict array index meanings; H-126 keeps the arithmetic mechanism under discussion. No SNN modeling/learning. |
| P-117 | C-48 constrains | C-36 | Functions and nested functions cannot read/write dynamic bindings or call procedures. H-197/H-198 supersede the H-124/H-127/H-129 read-only interval; explicit-reference mutation is allowed, and H-133 region lifetime still governs returned views. D-10r47. |
| P-118 | D-11 depends-on | C-13 | The static-pool formulas supply an arithmetic/indexing example; their application configuration and model authority do not follow from language validation. D-11r4 candidate dependency. |
| P-119 | C-49 derives-from | C-10 | Candidate: independently identified compile-time UoMs preserve meaning across modules and aliases while allowing typed numerical composition. H-128 supplies identity semantics; full arithmetic/value path pending. H-132 retains compound units as future physical-quantity capability, deferring implementation without selecting the algebra. |
| P-120 | C-49 constrains | C-47 | H-128 local confirmed rule: UoMs have their own first-class type identities, distinct same-name definitions are allowed, aliases preserve source identity, and symbols do not represent UoMs. Derived-expression equality is unselected. D-11r4. H-134 replaces H-130 UoM import with fresh definitions and one-shot using of selected units/conversions; omitted-unit conversions and repeated using are excluded. D-11r24/K-353 preserves eligible entry identity/context across exported converter aliases; this does not activate a general ordinary-import conversion-registration policy. |
| P-121 | C-50 derives-from | C-10 | Candidate: ordinary typed function methods make conversions and array access readable, composable pure program logic rather than host-language conversion operators. H-130 confirms the local syntax/dispatch direction; D-11r4 value path pending. H-131 supplies readable parenthesized operator naming within the same function-method direction; eager/control details remain proposed. |
| P-122 | C-50 depends-on | C-49 | Method signatures must distinguish the independently identified source/target UoMs and preserve fresh-definition/one-shot using rules. D-11r4 dependency, not authority. |
| P-123 | C-50 constrains | C-37 | H-130 local confirmed scope: UoM conversion formulas follow pure function semantics and cannot access effects or call procedures. Array/layout values must be explicit inputs under H-127/H-129 snapshots. D-11r4. H-197/H-198 supersedes the snapshot restriction for ordinary functions with explicit-memory semantics; the prototype conversion signature remains one scalar UoM input/result, and dynamic effects/procedure calls remain forbidden. D-11r24 reconciles that dependency without adding conversion memory parameters. |
| P-124 | C-51 derives-from | C-50 | Candidate: declaration-site conversion policy preserves usable typed method composition without hard-coding automatic conversion globally. H-135–H-138 supply local behavior; D-11r4 value path remains proposed. |
| P-125 | C-51 depends-on | C-49 | Compile-time routes use the selected conversion bundle and nominal UoM identities. H-134 selection is a prerequisite, not authority for arbitrary hidden conversions. D-11r4. |
| P-126 | C-51 constrains | C-37 | H-135–H-138 operate within H-130/H-127/H-129 purity: inserted conversion functions cannot call procedures or fetch ambient region effects; available explicit inputs/snapshots govern evaluation. D-11r4. |
| P-127 | C-51 constrains | C-47 | H-135–H-138 local scope: exact parameters do not convert; starred parameters may convert via selected formulas. Direct conversion is preferred, otherwise declared indirect routes, with unique discovered routes or configurable ambiguity warnings. Conversion does not merge unit identities. D-11r4. |
| P-128 | C-51 constrains | C-37 | H-148 local confirmed scope: evaluate each argument once and complete its selected pure conversions before evaluating the next argument, so conversion failure suppresses later argument effects. Functions, procedures and operator spellings agree. D-11r10/D-10r17; no new purity or runtime dispatch mechanism. |
| P-129 | C-41 constrains | C-37 | H-149 local confirmed scope: nested lexical value scopes may shadow outer names; declarations within one scope remain unique. This makes lexical expression lookup consistent without changing symbolic effect identity/binding or storage lifetime. D-10r19; full grammar and candidate value paths remain unqualified. |
| P-130 | C-41 constrains | C-37 | H-150 local confirmed scope: reject literal/control spellings that cannot resolve as local values in parameters, let, loop indices and fold accumulators. Preserve usable contextual names and field selectors. D-10r21; global/callable/module resource namespaces and escaping are outside this decision, and candidate value paths remain unqualified. |
| P-131 | C-52 derives-from | C-01 | Candidate, D-10r22 actor section: typed mail can carry inspectable program intent and inputs independently of the sender’s execution state. Charter §§3.1–3.2. Formal edge `pending-human`. |
| P-132 | C-52 derives-from | C-02 | Candidate, D-10r22 actor section: receiver-selected handlers let the same task protocol operate in different execution environments. Charter §6.1. Formal edge `pending-human`. |
| P-133 | C-52 depends-on | C-39 | Candidate regional-mail scope, D-10r22 actor section: delivery and disposal must preserve lifetimes, relocation and region-only allocation, including when a sender has reachable references. Charter §§2 and 4.1/4.3 and D-01 representation constraints. Transfer rules `pending-human`. |

### Regional, type and module contracts

| Edge | Child / relation | Parent / target | Rationale, evidence and scope |
| --- | --- | --- | --- |
| P-134 | D-12 derives-from | C-10 | Candidate, D-12r1: the register analogy and native evidence connect explicit typed execution context to inspectable language behavior and the charter’s separation of program, execution and environment in §§1, 3.2 and 6.1. Formal edge `pending-human`. |
| P-135 | D-12 depends-on | D-10 | D-12r12 tracks the current binding, call, storage and Actor boundaries; K-376 reconciles no-extraction and using/import/mixin/publication semantics with D-10r74. K-293 retains the candidate Actor entry/context review. Changes to those contracts or evidence require corresponding tracker review. Dependency remains `pending-human`; this edge supplies no qualification. |
| P-136 | C-41 constrains | C-37 | H-153 local confirmed direction: reserve keywords against ordinary names and use parenthesized intrinsic operation names for overload declarations. D-10r24; precise keyword inventory and broader control-form overloading remain open. |
| P-137 | C-37 constrains | C-41 | H-154/H-155 local confirmed scope: effectful operation overloads use procedures; one operation may contain function and procedure overloads distinguished by parameter types. Function purity and pure conversion formulas remain. D-10r26/H-158 keeps compiler operation representation separate from CoD symbols; H-159 selects current-binding invocation. Broader binding-set and syntax extensions remain pending. |
| P-138 | D-13 derives-from | C-01 | Candidate, D-13r1: a graph’s direct edges stay with its region and its business associations remain explicit across lifetime and relocation boundaries. Charter §§2–4. Formal edge `pending-human`. |
| P-139 | D-13 derives-from | C-05 | Candidate, D-13r1: whole-image relocation preserves internal edges and supports structured, efficient reload. Charter §7.1. Formal edge `pending-human`. |
| P-140 | C-53 constrains | C-40 | Confirmed local H-156/H-157 scope: one direct-pointer graph per region, with logical business associations across regions. D-13r1, D-10r25 and D-07r26. Business resolution and storage transfer protocols remain open. |
| P-141 | D-13 depends-on | D-01 | D-13r11 tracks the regional representation, allocator symmetry and lifetime constraints of D-01. Dependency `pending-human`. |
| P-142 | D-13 depends-on | D-10 | D-13r11 uses D-10r41’s language-level lifetime and Actor model scope for references, captures and region holding and transfer; K-293 adds candidate lifecycle obligations without selecting a transfer mechanism. Dependency `pending-human`. |
| P-143 | C-54 derives-from | C-10 | Candidate value: typed, inspectable owner transitions let WPO diagnose invalid regional access while user storage remains explicit. D-13r11; charter §§1–4 and 6. Formal value edge pending-human. |
| P-144 | C-54 depends-on | C-36 | Ownership must follow lexical/effect holders and cleanup transfers without being undone by dynamic-binding restoration. D-10r41/D-13r11; dependency alone grants no authority. |
| P-145 | C-54 depends-on | C-53 | Region-local pointer graphs make whole-region transfer possible; interior references still require a valid owning region. D-13r11; no physical cross-region edge or new encoding. |
| P-146 | C-54 constrains | C-39 | Superseded for dynamic-source consumption by H-181/P-151. H-169 records the earlier consuming effect-to-effect move and K-295 its implementation; that behavior is no longer the language contract. H-177/H-179 ordinary owners and consuming installation retain P-150’s separate scope. D-10r40/D-13r10. |
| P-147 | C-41 constrains | C-36 | H-172 confirms `effect @name <- rhs` as the only explicit dynamic-binding syntax, retaining RHS evaluation, result, scope and type-directed transfer/copy. D-10r41; local syntax constraint confirmed, not a new ownership policy. |
| P-148 | C-36 constrains | C-39 | Candidate restriction: dynamic values contain owning resources, independent bits or statically selected procedures, excluding ambient borrowed interior references. D-13r11/D-12r8 analyze why this can simplify lifetime dependencies through C-54/P-143/P-144, with C-48 snapshot access preserved. H-173 requests analysis; H-180 reaffirms value capture and H-181 explicitly admits business Maybe-wrapped resources; recursive admissibility and root access remain pending-human. Packet owning sums are a separate scenario and do not by themselves require admission as dynamic values. |
| P-149 | C-35 constrains | C-36 | Superseded by H-180: the H-176 identity-only dependency interpretation does not describe procedure capture. H-114 compile-time identities/aliases remain, and those identities key captured runtime values. Historical scope and correction remain in H-176/K-297 and H-180/K-300; use P-152 for owner-preserving shadowing. |
| P-150 | C-54 constrains | C-39 | H-177 permits ordinary variables to hold unique region owners. D-13r11; handles may move without copying/moving regional objects or introducing a user heap carrier. H-179 additionally confirms consuming transfer from an ordinary variable into a dynamic owner. Holder placement and this consuming edge are requirements; H-183 does not treat ordinary owner as a fully defined source category. Declaration, mutation, parameter/return/root/result and post-transfer syntax/semantics remain open in D-14r1. |
| P-151 | C-54 constrains | C-36 | H-181 confirms that ownership cannot be taken out of a region-valued symbolic dynamic binding; shadowing preserves owners under H-180. H-179’s ordinary-to-dynamic installation remains consuming. D-10r41/D-13r11; supersedes H-169’s dynamic-source move direction and resolves H-178’s proposal. Packet transfer is separate; no consuming extraction through an explicitly optional dynamic value. Exact typed access/representation remains open. |
| P-152 | C-36 constrains | C-54 | H-180 confirms that capture replay temporarily shadows symbol lookup while preserving ownership and lifetime of both captured and hidden regional resources. Removing that shadow releases neither resource. D-10r41/D-13r11; explicit resource installation and packet consumption are distinct operations. This local constraint is confirmed; no lifetime-extension, reference-counting or binding ABI is selected. |
| P-153 | C-36 constrains | C-41 | H-181 confirms that dynamic-binding availability is a WPO obligation independent of expression value type. Direct region references need no implicit Maybe; an explicitly declared Maybe<SomeRegion> remains valid business data. An unbound symbol is a compile error even for Maybe, while bound absence is a value. D-10r41/D-13r11; local semantic constraint confirmed, sum/layout implementation not selected. |
| P-154 | C-54 constrains | C-41 | Candidate under H-182: effect-resource installation obtains a fresh regional temporary or consumes a writable present optional place, leaving absence. D-14r1 compares expression categories, optionality and construction. H-183 leaves ordinary owner source forms undefined pending coherent design; rule not implemented or confirmed as complete. |
| P-155 | D-14 derives-from | C-10 | Candidate value: explicit coordinated ownership syntax, semantic transitions and usable construction/installation examples make lifetimes inspectable in CoD rather than inferred from the host language. D-14r1 all sections; H-182/H-183 authorize design exploration, formal path pending-human. |
| P-156 | D-14 depends-on | D-10 | Use typed identities, effect/purity/capture rules, symbol-only perform/behave and lexical direct calls. D-14r1/D-10r41; dependency, not implementation authority. |
| P-157 | D-14 depends-on | D-13 | Preserve region-level lifetime and the distinction between packets and dynamic resources. D-14r1/D-13r11; dependency only. |
| P-158 | D-14 depends-on | D-01 | Preserve region-only objects, no regional object copy/move, allocator pairing and relocation when constructing or transferring handles. D-14r1; no new pointer/layout mechanism follows. |
| P-159 | C-37 constrains | C-36 | H-184 permits ordinary lexical and symbolic dynamic procedure definitions and installation of an existing lexical procedure as an effect. Lexical naming does not grant purity or recapture on assignment. H-186 additionally confirms direct proc @p(...) -> RT declarations without an intermediate lexical procedure. H-187 additionally supplies the symbol declaration when @p was not declared/imported. D-10r44/K-305; local naming/installation direction and this declaration form confirmed, complete procedure representation and type grammar open. |
| P-160 | C-35 constrains | C-36 | H-184 requires every symbol definition to declare its value type, including procedure types; grouped form symbol a :: T1, b :: T2. Aliases preserve identity/type; standalone symbol declarations supply no runtime value. H-187 allows proc @p to introduce a missing symbol with its signature as the explicit procedure type; the procedure binding occurs at execution, not scanning. D-10r44/K-305 implements typed/grouped symbols and proc(T, ...) -> RT signatures. Local type/identity constraint confirmed; broader type compatibility remains open. |
| P-161 | C-37 constrains | C-41 | H-185 fixes perform/behave as calls through symbolic dynamic bindings and direct procedure calls as lexical. perform p and behave p are not lexical-call forms. H-189 selects the caller current dynamic context for direct lexical calls. D-10r44/K-305 implements this with captured/current symbolic semantics preserved. |
| P-162 | C-54 constrains | C-39 | H-190 confirms intrinsic ownership at whole-region granularity and leaves intra-region allocation/reuse policies, including free lists and pools, to CoD user programs. D-14r3/D-13r13/D-10r45; preserve typed regional access, lifetime and relocation. Region validity does not prove logical pool-item validity. No per-object ownership, allocator algorithm, raw-storage retyping or generation mechanism is selected; the local boundary is confirmed, full contracts remain pending-human. |
| P-163 | C-54 constrains | C-41 | Proposed explicit effectful whole-region construction: a user-facing resource type and a make expression with allocation capacity expose storage/lifetime intent in typed expressions. Scope D-14r6; value rationale follows C-54/P-143 and C-41/P-97. H-192 requests exploration and capacity inputs, not approval of region<Root>, exact grammar or transfer policy; pending-human. |
| P-164 | C-54 constrains | C-39 | H-193 requires references to retain compile-time region association for access/lifetime/escape checking. Scope D-14r7/D-10r46; local constraint confirmed, not a new per-pointer runtime field or complete implementation proof. H-202 extends the validity discussion to region and lexical carriers; H-203/P-174 gives the same-owner stored-edge boundary. |
| P-165 | C-39 constrains | C-41 | H-193 makes user-defined object representations region-internal without a regional modifier. H-209 supersedes its concrete declaration spelling: data defines product/sum representations and type defines abstractions. D-07r43/D-14r12; no stack/copyable user-aggregate exception follows. K-321 implements the regional data-product path and removes copyable stack records; sum/implicit borrowed-field/owned syntax remains unfinished. |
| P-166 | C-54 constrains | C-37 | Candidate access versus transferring region parameter contracts preserve readable calls and checked lifetimes. H-194/H-197 request analysis; D-14r8, owning/borrowed syntax and call markers remain pending-human. Both callable kinds follow the same proposed ownership contracts; functions exclude dynamic context. |
| P-167 | C-37 constrains | C-48 | H-197 adopts H-195 no-dynamic-context functions for implementation/validation, including explicit memory operations and explicit-argument send. Replaces automatic read-only snapshots; preserves provenance/lifetime. H-198 removes the fallback plan and makes this the ordinary-function baseline. D-14r8/D-10r47; local rule confirmed, unfinished syntax/transport separately scoped. |
| P-168 | C-54 constrains | C-39 | H-196 requires all owner-bearing types to be intrinsic, including transitive containment through optional/result forms; user-defined objects cannot wrap region owners. D-14r7/D-13r14; local boundary confirmed, intrinsic syntax/representations and ownership state policy remain open. |
| P-169 | C-54 constrains | C-37 | H-200 selects unqualified region<RT> for borrowed access and owned region<RT> for lifetime responsibility, with no borrowed keyword. This keeps common borrowed signatures short while retaining one owning scope. D-14r9/D-10r48; local direction confirmed, call-site adaptation, transfer state and complete construction/result grammar remain pending. Supersedes the agent explicit-borrowed recommendation compared under H-199. |
| P-170 | C-37 constrains | C-48 | H-198 confirms language-provided ST-like mutable-state semantics as the ordinary-function baseline and removes the read-only fallback plan. Future business read-only scopes may introduce const function separately. D-10r47/D-14r8; no monadic syntax, full runST equivalence or const implementation contract is selected. |
| P-171 | C-39 constrains | C-41 | H-200 gives unqualified variables borrowed access without lifetime extension, reserves explicit ownership, and keeps non-bits objects off the stack. Packet in place of ref<Packet> and make local for bits are proposed source mechanisms serving readable lifetime/storage semantics. D-14r9/D-10r48; default/lifetime direction locally confirmed, exact initialization, temporary extent and stored inline-versus-edge rules pending. No per-object region ownership or graph copying is implied. |
| P-172 | C-37 constrains | C-39 | H-201 establishes const bits parameters/results as independent values; unqualified ordinary data borrows. H-202 narrows the broader const wording: non-bits const values remain read-only borrows with observable writable aliases. D-14r10/D-10r49; no regional object copy, lifetime extension or const-function guarantee follows. |
| P-173 | C-48 constrains | C-39 | H-202 confirms const non-bits as read-only borrowed access: holder writes to the target are forbidden, other writable aliases may change its contents, and lifetime is not extended. D-14r10/D-10r49. No immutable snapshot or optimizer invariant-memory assumption follows; H-368/D-14r75 explicitly requires propagation through references to distinct targets and rejects shallow access; independently writable aliases remain observable. |
| P-174 | C-39 constrains | C-41 | H-203 requires stored object references to share the containing object's allocation owner, region or lexical-scope instance; cross-owner edges are compilation errors even if both are live. H-202 supplies static validity intent. D-14r10/D-10r49; borrowed bindings retain target provenance rather than acquiring the callee's allocation owner. Region relocation and no non-bits stack placement remain intact. |
| P-175 | C-54 constrains | C-41 | H-204/H-205 develops = reference assignment and <- ownership/bits transfer. H-206 fixes a const reference to the resulting destination; H-207 requires explicit ownership-move steps. Bits fields can transfer across owners; graph conversion needs selected user methods under P-177/C-55. D-14r11/D-10r50; occupied-owner replacement, ordinary lvalues and ([]<-) grammar remain pending. |
| P-176 | C-54 constrains | C-41 | H-206 selects a const destination-reference result, retaining destination type/value/lifetime. H-207 keeps ownership moves explicit; H-209 fixes right association, so a <- b <- c is a <- (b <- c) and consuming the const intermediate fails. D-14r12/D-10r51; header/result lowering and precedence against other forms remain pending. |
| P-177 | C-39 constrains | C-41 | H-206 permits builtin bits-field transfer and rejects default whole-object transfer when references occur, while allowing explicit user (<-) graph-conversion algorithms. D-14r11; construct/write permitted destination storage and form same-owner edges rather than copying source pointers or implicitly copying/moving regional objects. Topology and failure behavior belong to the selected algorithm's contract. H-351 confirms this library-transfer boundary for cross-region graphs including DBMR; no generic generated copier is selected. |
| P-178 | C-55 derives-from | C-39 | Candidate value: root-policy-aware graph conversion lets ordinary user code move domain data between storage strategies while respecting intrinsic whole-region lifetime and user-controlled internal allocation. H-206/H-208 directly requests this capability and its memory-method access; D-14r11/D-11r14 records local requirements, with the complete value path/design pending-human. |
| P-179 | C-55 depends-on | C-50 | Root-aware conversion needs typed multi-argument selection. H-209 confirms declaration-order-independent unique-best selection; H-210 declares subtype relations with <:. D-11r15 records this dependency, with complete abstract/generic conformance and conversion-versus-subtyping competition still open. No runtime method registry or implicit owner conversion follows. |
| P-180 | C-55 constrains | C-54 | H-208 requires real borrowed references to both regions in the selected function/procedure. Each must own its paired object and retain actual access permissions; type matching cannot substitute for owner identity, obtain ownership, or recover writable access from a const operand. D-14r11; explicit parameter ordering and compiler argument adaptation remain proposed. |
| P-181 | D-14 depends-on | D-11 | Transfer overloads depend on typed argument matching and the root-policy constraint extension documented in D-11r14. K-314 updates both scopes; existing scalar conversion evidence does not validate abstract matching or graph transfer, and this dependency grants no design qualification. |
| P-182 | C-41 constrains | C-54 | H-209 fixes right-associative <-, preserving the const destination result and rejection of ownership consumption through it. D-14r12/D-10r51; grouping does not reorder H-148 operand/conversion evaluation or duplicate destination evaluation. |
| P-183 | C-50 constrains | C-55 | H-209 requires a unique most-specific applicable method independent of declaration order; no applicable method or ambiguous maxima is an error. D-11r15/D-14r12; crossed source/destination specializations may require an intersection method. Complete constraint implication and conversion competition are not selected by this principle. |
| P-184 | C-56 derives-from | C-10 | Candidate value: separating abstractions from concrete ADT representations makes domain identity, root-policy contracts and storage layout inspectable. H-209/H-210 directly selects type/data/<: in this scope; D-07r43 preserves the charter §§2–5 distinctions. Full declaration/interface/GADT mechanisms remain pending-human. |
| P-185 | C-55 depends-on | C-56 | Root-policy dispatch requires explicitly declared conformance and nominal abstract method inheritance. D-07r46/D-11r18; H-218/H-220 replaces implicit structural inference. The adapter imports the ADT to declare relations; consumers may use the adapter. Called implementations must be visible. No root casts, mutable covariance or new ownership permissions follow. |
| P-186 | C-56 constrains | C-39 | The type/data split preserves H-193 region-only user objects and H-196 intrinsic ownership containers. D-07r43/D-14r12; data spelling does not grant object copy/move, stack placement, immutability, GC or user-defined owner fields. Abstract declarations specify no concrete root layout. |
| P-187 | C-56 constrains | C-50 | H-210/H-218 uses <: for nominal abstract inheritance and explicit data conformance. Multiple abstract parents remain deferred; multiple data-interface declarations are permitted across modules. D-07r46/D-11r18; unrelated applicable abstractions retain unique-best ambiguity rather than declaration-order priority. |
| P-188 | C-56 constrains | C-50 | Superseded in its conformance criterion by H-218/P-196: H-211/H-213/H-216 formerly used structural method-signature matching and informational data annotations. The retained boundary is no field/layout equivalence or eager complete-body inventory. D-07r46/D-11r18/D-14r15 now requires explicit declarations; H-216 open methods and demanded body checking remain. |
| P-189 | C-57 derives-from | C-10 | Candidate value: selecting a type with its source-visible behavior makes regional interfaces usable while explicit extension permission and mixin distinguish reuse from redefinition. Charter §§1–4 identity/context boundaries remain. H-211/H-212 selects the local operations, not the complete value chain or bundle/extension implementation. D-07r44/D-10r52, pending-human beyond that local scope. |
| P-190 | C-57 depends-on | C-50 | Type-associated methods require stable identities and unique-best argument dispatch after resolving a callable name. D-07r45/D-11r17; the lookup/dispatch separation is proposed under H-217, with exact generic membership, repeated-path handling and extension scope pending. No global method table or changed UoM conversion policy follows. H-225/H-226 confirms signature association and identity-based implicit selection in D-07r60; no body scan or mutable wrapper covariance. H-231 constrains cross-module associated selection to exported resources; signature association cannot bypass privacy. Exact family/signature publication remains draft. |
| P-191 | C-57 constrains | C-43 | H-212 separates import from mixin: import preserves source identities/definition contexts and permits extension; mixin redefines the source body with receiving lexical/dynamic bindings. D-10r52; existing capture, destination and alias-map requirements transfer with the body-redefinition mechanism. Not a claim of implemented syntax or approval of arbitrary type/UoM cloning. H-227 retains source lexical identities while caller-selected ordinary implementations participate in demanded specialization; no WPO-wide publication or operation fallback change. H-230 refines callable extensions to distinct module identities; same-alias unions also create local identities. Source-helper dependency routing under H-227 is unresolved; general lexical rebinding remains mixin. H-232 resolves the simple helper case: using preserves context, import selects receiving implementations, with source body/non-callable bindings retained; P-203 bounds this refinement. |
| P-192 | C-45 constrains | C-57 | H-214 keeps modules out of local names and excludes qualified function/procedure references. Use explicit source selections and local aliases; provenance remains compiler/tooling information. D-07r44/D-10r52; no qualified-name extension bypass, no selected alias-collision syntax, and no exception without a serious obstacle and design review. |
| P-193 | C-57 constrains | C-45 | H-215 proposes separate explicit import-as for method aliases; H-217 places implicit type-method names in a shadowable lower-priority namespace. D-07r45/D-10r53; value rationale is explicit origin choice without namespace objects, preserving C-10/C-45 identity semantics. Name-first dispatch, deferred implicit-name ambiguity and identity-based dedup are proposed elaboration, not separately human-confirmed mechanisms. H-226 selects using-local shadow versus unique imported-family extension; explicit callable aliases shadow implicit names. Conflicting families require explicit selection. H-230 preserves different-alias identity/method sets and specifies local same-alias virtual unions of the same semantic callable; P-202 records the narrowed target. |
| P-194 | C-56 constrains | C-50 | H-216 keeps nominal abstract method inheritance, distributed methods, no Interface body and demanded missing-implementation errors. H-218 now requires explicit data conformance instead of the earlier informational/structural criterion. D-07r46/D-11r18; a declaration establishes the relation without certifying every method body. Exact signature and generic-bound syntax remains pending. D-07r71 reviews bodyless lexical headers, inherited-body applicability and demanded result/kind compatibility; these proposals remain pending-human under K-352. |
| P-195 | C-56 depends-on | C-57 | Distributed methods and conformance declarations require source-visible selection and extension permissions. H-220 separates adapter import permission from consumers obtaining the relation by using the adapter. D-07r46/D-10r54; inheritance alone does not expose unselected modules, and source bodies retain lexical context. Full selector/re-export mechanics remain pending. H-227 permits caller-selected implementations along ordinary call paths, without ambient propagation of unrelated conformance. D-07r60. K-345/D-07r65 identifies the pending related-argument conformance boundary: proof retention versus a declared-interface ceiling. This review grants no new admission/dispatch policy. H-229/D-07r66 rejects the declared-interface ceiling: dispatch continues to use the concrete data type with established explicit conformance. This does not globally publish unselected adapters. |
| P-196 | C-56 constrains | C-50 | H-218 requires an explicit data-to-abstract relation and rejects implicit structural conformance. Separate head-only declarations for one ADT may name different interfaces in different modules. D-07r46/D-11r18/D-14r15; declarations retain the original data identity/layout, supply applicability without circular method evidence, and leave called implementations/signatures checked on demand. Abstract multiple-parent deferral is unchanged. |
| P-197 | C-56 constrains | C-57 | H-220 clarifies declaration versus use: an adapter imports the ADT before declaring conformance; a third-party consumer may obtain the relation using the adapter. D-07r46/D-10r54; using does not grant extension permission, conformance does not manufacture bodies/ownership, and same-name aliases do not create type identities. This corrects H-219's earlier consumer restriction. Precise bundle membership and re-export syntax remain pending. |
| P-198 | C-41 constrains | C-54 | H-222 requests right-associated =; H-223 supplies read-only RHS access; H-224 proposes `(=)(lhs::XX, rhs::const YY) -> const XX`. D-14r16/D-10r56 distinguishes readable sources and destination results without requiring RHS write permission. Alias permission, bits versus non-bits const results and lvalue representation remain pending; no general ownership or const bypass follows. |
| P-199 | C-58 derives-from | C-10 | Candidate value: explicit publication lets a domain expose intended typed operations without making implementation names integration contracts. Charter §§2–4; H-231 directly supplies private-by-default selection, not full qualification of export bundles or the value chain. D-07r67/D-10r63. |
| P-200 | C-58 constrains | C-57 | H-231 requires an export boundary before cross-module selection, including associated methods; exporting a type cannot alone authorize directly selecting private callables. Exact family versus signature and conformance bundle publication remains draft. H-225 signature membership and H-226 extension permissions remain distinct. H-233 confirms export import publishes the entire selected import, including implicitly selected methods within source privacy constraints; unrelated/private resources are not included. |
| P-201 | C-58 constrains | C-43 | H-231 forbids using/import/mixin of private resources. Source-preserving calls may still use private source dependencies internally; export does not grant general receiving-context access to them. Export-list and re-export details are proposed in D-07r67, not implemented. H-233 forbids implicit re-export and permits the export import modifier. It adds publication to the normal import operation without changing incorporation or granting consumers extension permission. |
| P-202 | C-57 constrains | C-45 | H-230 assigns extension-module callable identities; distinct local aliases retain selected identities/method sets, and same-alias virtual composition creates a receiving-module identity. The concrete required trace is1,7,1. Source semantic-family provenance remains necessary for explanation/deduplication, but its representation and helper-dependency routing are still design work. No global publication or intrinsic-symbol conflation follows. H-232 resolves the simple helper outcome to using2/import14. Several matching dependency views or same-body/different-context unions remain distinct compound cases. |
| P-203 | C-57 constrains | C-43 | H-232 requires using to preserve the selected helper implementation context and import to adapt callable dependencies to the receiving implementation context. The concrete twice trace is2 versus14, while H-230's separate-alias1,7,1 remains. This serves context-bound integration without general lexical body rewriting; source captures, symbols and globals are not recaptured/rebound. D-07r68/D-10r64; exact conflict handling for multiple selected views remains pending. |
| P-204 | C-59 derives-from | C-10 | Candidate value path: independently composed semantic information lets integration enrich typed interpretations without silently modifying source programs. Charter §§1–4; H-234 supplies the architectural principle, while the complete derivation/operational scope remains pending-human. D-15r1. |
| P-205 | C-59 constrains | C-58 | H-234 requires explicit exports and receiver-local merging of information for identical imported objects, with independent onward publication. H-231/H-233 retains private-by-default selection/re-export. Confirmed principle; publication granularity/timing and conflict mechanics remain under architecture review. |
| P-206 | C-59 constrains | C-35 | **Superseded by H-245/P-216.** H-234's former cross-module editing through TLS requirement no longer governs implementation. Cross-module module-world editing is impossible at both stages; only receiver-owned construction is permitted. The original confirmation remains in H-234; K-390 records the correction and affected scopes. |
| P-207 | D-15 depends-on | D-10 | Compile-time TLS shape/type analysis and runtime binding/capture/restoration require coherent dynamic-scope contracts. H-235 keeps machinery internal. H-245 excludes a compiler/runtime cross-module editor; this dependency grants no new effect ABI or capture exception. |
| P-208 | D-15 depends-on | D-07 | Identity, methods, explicit conformance and export selection supply the semantic objects being composed. Their existing implementation must be evaluated against the new architecture, not imposed as a representation constraint. |
| P-209 | D-15 derives-from | C-59 | Candidate document value path: one architecture contract connects receiver-owned construction, explicit publication, local identity merging and runtime typed bindings before local implementation choices. H-236 supplies the architecture-first order; H-245 removes cross-module editing. The complete operational contract remains pending-human. |
| P-210 | C-59 constrains | C-35 | H-237 explicitly distinguishes module worlds from TLS worlds and assigns them different aspects of objects. Do not collapse the TLS world into a module-world editor pointer, duplicate one catalog under two names, or equate the distinction with compile time versus runtime. The detailed division of object information is pending in D-15r2; no new binding ABI or runtime state layout is selected. |
| P-211 | C-59 constrains | C-57 | H-238 rejects treating the module/TLS worlds as two models of the same generic object set and requires their respective entities to be identified before further mechanisms. D-15r3 separates module-resident semantic types/callables/methods from TLS symbolic-pointer entries; this is not a new user-visible entity hierarchy. |
| P-212 | C-59 constrains | C-35 | H-239 defines TLS as a flattened collection of typed symbolic pointers. Symbol and type identities are separate; a symbol dynamically associates with a type-compatible value that may originate in a different module-world copy. Confirmed contract; pointer ABI, physical tags, general type compatibility, lifetime/capture lowering and dynamic-value category limits retain their separate scope. H-277 refines physical storage to inline bits and permitted references without changing separate symbol/type identities; H-280/H-281 distinguishes effect transfer and rebinding. |
| P-213 | C-59 derives-from | D-12 | H-241 explicitly places dedicated module-world copies within the Global Variables Done Right vision and requires coordinated maintenance. Module-local semantic integration and typed TLS association are complementary responsibilities. This relationship is confirmed for that architectural placement, not for every proposed editing, copying or native update mechanism in D-15; D-12's full charter path remains pending-human. |
| P-214 | C-60 derives-from | C-59 | H-242 gives vocabulary modules the purpose of establishing common identities for symbols, types and their method collections. Shared definition origins let independent module worlds integrate information for identical identities and let TLS references retain declared type identity across world copies. This local purpose/relationship is confirmed; the full architecture and implementation contract remain pending-human. |
| P-215 | C-60 constrains | C-58 | Vocabulary publication follows H-231/H-233 explicit exports/re-exports. H-242 broadens the contents, not the privacy boundary. Shared vocabulary identity does not confer a shared mutable world or automatically bind default effect values. |
| P-216 | C-59 constrains | C-35 | H-245 explicitly requires modules to construct only their own module world and forbids editing another world at compile time or runtime. Compilation determines the flattened TLS shape; TLS-world pointer edits occur at runtime. This replaces P-206 without changing symbol/type identities, explicit exports, captures or regional ownership. |
| P-217 | C-59 constrains | C-58 | H-247 defines export as making an identity importable under a public alias. Publication and local lookup are separate identity mappings; several aliases may expose one identity. D-15r12/D-07r105; confirmed scope, no new export-as grammar or runtime publication protocol. |
| P-218 | C-59 constrains | C-57 | H-248 requires compiler structures centered on identity and permits multiple aliases. Combined with H-234 receiver-local merging, aliases access one local information record rather than independent method subsets. D-15r12; confirmed representation constraint, not blanket approval of unresolved extension/context policies. |
| P-219 | C-59 constrains | C-57 | H-252 requires a warning for potentially misleading overload declarations through multiple aliases of one identity in one module, with opt-in escalation to an error. D-15r12/D-07r105; confirmed diagnostic requirement. Ordinary multiple aliases remain legal; no alias-based identity split, runtime check or new extension permission. |
| P-220 | C-59 constrains | C-57 | H-253 requires conflicting inherited/receiver-selected overloads in an import-adapted dependency to reject rather than silently choosing A or B. D-15r12/D-07r105; confirmed conflict boundary. Direct aliases and using-preserved contexts retain their separate identities; explicit arbitration syntax remains future design. |

### Fractions, ownership and symbolic storage

| Edge | Child / relation | Parent / target | Rationale, evidence and scope |
| --- | --- | --- | --- |
| P-221 | C-61 derives-from | C-49 | Candidate value: one unit identity across integer, floating and fraction representations keeps numerical storage choices from redefining domain meaning. H-254 accepts the planning direction; full charter derivation remains pending-human. D-16r1/D-11r27. |
| P-222 | C-62 derives-from | C-10 | Candidate value: explicit fixed-width exact ratios make numerical intent, costs and failure boundaries inspectable without hidden allocation. Charter §§1–3 via P-14/C-01; H-254 accepts scoped semantics, not the complete value path. D-16r1. |
| P-223 | D-16 derives-from | C-62 | Candidate: a focused draft coordinates fraction type/arithmetic/storage and preserves future compatibility before implementation. H-254 requests the draft; full revision qualification remains pending-human. |
| P-224 | D-16 depends-on | D-11 | Fraction quantities require independent UoM identity and typed conversion endpoints. H-254 leaves promotion/route details open; dependency does not authorize a numeric refactor. D-16r1/D-11r27. |
| P-225 | C-62 constrains | C-61 | H-254 requires fractions to be numerical representations composable with applicable UoMs, not reciprocal denominator units; fixed component types do not manufacture unit identities. Confirmed planning constraint, D-16r1. |
| P-226 | C-62 constrains | C-10 | H-254/H-255 confirms the scoped fraction requirements and requires subsequent language design/implementation to review conflicts and discuss an affected choice before implementation. No fraction implementation scheduled; unresolved grammar/layout/promotion remain unselected. D-16r1, D-07r106 and AGENTS. |
| P-227 | D-16 depends-on | D-10 | Fraction bits values and intrinsic signatures must preserve function/effect, const/borrow and lifetime rules; fixed pair storage does not select a new result-borrow or ownership contract. D-16r1; dependency only. |
| P-228 | C-61 constrains | C-10 | H-256 prioritizes representation-independent UoM and reserves // for future fraction construction with # comments as separate prompt follow-ups. This supersedes prerequisite deferral, not fraction-runtime deferral or open grammar/promotion decisions. D-16r4/D-11r30/D-07r107. |
| P-229 | C-37 constrains | C-57 | H-257 requires direct lexical procedure calls to use local bindings, with shadowing warnings; callable declaration identity/contracts are separate from executable methods. H-258 requires parameter/result signatures, generic type parameters and varargs. D-10r88/D-15r14; scoped human decisions, no runtime callable ABI. |
| P-230 | C-39 constrains | C-10 | H-257 directs the internal compiler model toward LLVM register optimization: evaluated bits and owner-backed places remain distinct while preserving lifetime, alias and relocation rules. D-10r88; ordinary-region ownership syntax/lifecycle remains a separate deferred work package. |
| P-231 | C-61 constrains | C-49 | H-260 selects unit m, numeric quantity annotations f64(m), representation-preserving m(value), and parameter-controlled f64(m*) conversions. H-261 requires whole-type lossless widening, exact-before-widened unique-best method dispatch and a unique least safe representation for builtin mixed scalar arithmetic. D-11r30; scoped confirmation, not generic fraction promotion or return-type dispatch. |
| P-232 | C-54 constrains | C-41 | H-262 selects a compiler-tracked consumed state for ordinary named region owners after transfer: later use of the name is a compile error, with no implicit null or mandatory optional storage. D-14r33 and D-07r112/D-10r89/D-13r19; scoped human confirmation serves explicit, inspectable responsibility and avoids implicit null dereferences. Runtime absence/send-error outcomes remain explicit. Construction, consuming call/result boundaries, reinitialization and old-view validity are excluded. |
| P-233 | C-54 constrains | C-39 | H-263 preserves existing borrowed views after a local ownership transfer while the new owner remains alive and permits access. D-14r34 and D-07r113/D-10r90/D-13r20; confirmed local constraint separates name consumption from region lifetime. Owning scope exit and outbound send revoke access; failed-send alias revival and a complete analysis algorithm are excluded. |
| P-234 | C-54 constrains | C-41 | H-264 selects `let packet :: owned region<Packet> <- make Packet(capacity = bytes) { sequence <- initial };`, with runtime capacity, direct regional root initialization and ownership transfer into the declaration. D-14r34; confirmed initial syntax serves explicit construction/lifetime intent. Allocation failure, capacity accounting, complex graph initialization and general owning call/result grammar remain separate. |
| P-235 | C-63 derives-from | C-10 | H-267 confirms the bounded D-17r3 E1–E5 value: recoverable business failure preserves explicit storage responsibility and typed intent. Charter §§2–4, 6.2–6.3 and 12. |
| P-236 | D-17 derives-from | C-63 | H-267 confirms D-17r3 E1–E5, its preservation obligations and acceptance, amended to explicit throw-from with no automatic chaining. General exception graphs/RTTI remain outside scope. |
| P-237 | D-17 depends-on | D-14 | Exception paths must preserve consumed owners, actual current responsibility and permitted live views, including unfinished/pending results. D-17r9/D-14r40; technical dependency, not qualification. |
| P-238 | D-17 depends-on | D-10 | Exiting scopes restores dynamic bindings and preserves captured/hidden owners; functions retain their dynamic-effect exclusion. D-17r9/D-10r94; technical dependency, not qualification. |
| P-239 | D-17 depends-on | D-01 | Regional allocation, allocator symmetry, no per-object ownership and zero-cost relocation constrain exceptional cleanup. D-17r1; technical dependency with an ordinary allocation/root-construction source gap recorded in K-406, not assumed validated guarantees. |
| P-240 | C-63 constrains | C-54 | H-265/H-267: unchecked allocation failures, no implicit null owner or mandatory Result; E4 fixes consumption at callee entry and E5 the typed construction boundary. D-17r9/D-14r40; unrelated ownership choices remain pending. |
| P-241 | C-64 derives-from | C-41 | Composing assignment expressions preserves the destination's observable storage identity while ordinary const bits calls preserve evaluated values. H-270 confirms the presented D-14r41/RO-Q1 result-mode choice for this purpose; it does not qualify all of C-41/P-97 or D-14's broader value path. |
| P-242 | C-64 depends-on | C-54 | A result view retains the destination owner and read-only access and cannot escape that lifetime or consume ownership. H-270's D-14r41/RO-Q1 retains these existing constraints; technical dependency, not authority for unresolved owner/place mechanisms. |
| P-243 | D-14 depends-on | C-64 | D-14r41/RO-Q1 uses the H-270-confirmed assignment-family result mode, including explicit calls and destination-return validation. H-271 separately resolves RO-Q2; wider const/place/alias interfaces remain pending. This dependency is not a prerequisite for completed D-17 E1–E5. |
| P-244 | C-65 derives-from | C-54 | Candidate rationale: fixing each pointer's target owner makes same-owner edits and external-call constraints explicit while preserving region/lexical lifetime responsibility (charter §§2–4). H-271/H-272 confirms the enumerated mechanisms; this inferred value relationship does not qualify all of C-54 or D-14. |
| P-245 | C-65 depends-on | C-39 | Region-only non-bits storage, same-owner stored references and no object copy/move constrain parameter materialization and owner-generic borrowing. D-14r42; technical dependency, not authority to change regional layouts or lifetimes. |
| P-246 | D-14 depends-on | C-65 | D-14r59 coordinates H-271–H-275 with H-270 results and D-17 cleanup. D-07r121/D-10r111/D-17r11/prototype162 carry the source-migration boundary. H-273 confirms generic-owner annotations; H-275 confirms the region binder and region-as-declaration direction. Result/symbol spellings remain proposed; no compiler acceptance inferred. |
| P-247 | C-65 depends-on | C-55 | H-274 requires a region-specific owner to expose the actual borrowed region and constrain its root, so body code can use the root's internal memory-management methods. This carries H-206/H-208 root-policy access into general owner-generic signatures while generic owner stays region-or-scope. D-14r59; H-275 confirms the region binder and declaration-keyword direction. Result/symbol spelling and remaining permission interfaces stay pending. Dependency grants no authority to new overload ranking or graph-transfer algorithms. |
| P-248 | C-36 constrains | C-65 | H-277–H-281 distinguishes dynamic symbolic bindings from ordinary fixed-owner pointers. Bits live directly in TLS and require effect-arrow transfer; object references use effect-equals rebinding and may change region owner. Scoped restoration remains required; bare writes remain invalid. Confirmed scope in D-10r111/D-14r59, not a new whole-document value qualification. |
| P-249 | C-36 depends-on | C-54 | Non-bits symbolic object references require a region supplied by a region symbol and must not outlive it (H-277). H-180/H-181 preserves captured/hidden owners and forbids extraction. D-10r111/D-13r26/D-14r59 requires actual-region provenance through branches/captures/restoration; no cross-owner stored graph edges or owner duplication. This dependency does not select a runtime borrow mechanism or confer authority on unconfirmed syntax. |
| P-250 | C-36 depends-on | C-55 / C-64 | H-282 proposes bare symbolic-object transfer through a user overload; H-283 confirms it is distinct syntax from effect-prefixed binding. D-10r111/D-14r59 reviews actual-owner graph dispatch, access/lifetime and read-only original-destination results under H-148/H-206/H-208/H-270. These existing contracts constrain the proposed form, not automatic transactional rollback. H-287 requests this bounded implementation; K-426 records actual evidence without qualifying unspecified graph algorithms or selecting a new runtime mechanism. |
| P-251 | C-36 depends-on | C-65 / C-54 | H-284 requires shared actual-region ownership and a root constraint for grouped object symbols; H-285 confirms evaluation before joint installation. D-10r111/D-14r59 reuses H-273–H-275 owner/root constraints and H-277 region-symbol-backed lifetime. H-286 selects rejection of incompatible partial replay and explicit touch dependencies; dependency does not qualify new runtime metadata, group results or graph transactions. |
| P-252 | C-66 derives-from | C-36 | H-286 explicitly requests touch @symbol to make desired procedure captures convenient without inventing referent computation. This provides an inspectable source-level dependency while retaining ordinary capture semantics and avoiding implicit whole-group capture. Confirmed local purpose/scope in D-10r111; the upstream full-design value paths remain separately pending. |
| P-253 | C-66 depends-on | C-54 / C-65 | Touched references retain actual-region lifetime and declared shared-owner constraints. H-286 resolves the partial-replay case under H-180/H-277/H-284; no owner duplication, lifetime extension or runtime registry follows. D-10r111/D-14r59 acceptance covers explicit companion capture, invalid replay, missing bindings and expired regions. Dependency does not qualify remaining owner/group grammar. |
| P-254 | C-36 constrains | D-10 / C-37 | H-288–H-291 requires void effect results and procedure rebind through =, with inline typed symbol introduction/reuse. This separates scoped context installation from ownership transfer while retaining definition-time capture and signature identity. D-10r111 callable/effect-result scope; no runtime closure ABI or whole-language qualification. H-372/H-373/P-330 supersedes implicit capture/installation from bare module definitions and the use of slot signature identity to prohibit method extension; void effects and compatible typed installation remain. |
| P-255 | C-65 constrains | D-14 | H-292 confirms that a fresh bits ptr initializer denotes scope-owned local storage and existing-storage initialization retains its owner. This makes arithmetic convenient without dangling temporary borrows, regional stack copies or mandatory physical allocation. D-14r59 local-initialization scope; C-65/P-244's wider value path retains its status. |
| P-256 | C-64 depends-on | C-65 / C-54 | H-293 requires an explicitly external owner for assignment destinations and preserves read-only access through new aliases. Caller-visible rebinding cannot be implemented on an implicit callee-local copy; result views keep actual provenance and permissions. D-14r59 assignment scope; no public pointer-cell API or new lifetime mechanism. |
| P-257 | C-65 derives-from | C-41 | Local rationale: distinguish individual value initialization from multi-variable initialization through a final comma-introduced block. H-296 supplies the two source roles and keeps H-292 value construction; H-295 supplies compile-time-only typeof. D-14r59 coordinates transfer shorthand, source order and lexical names. Source choices are human-confirmed; the wider candidate value path and complete ptr lowering are not qualified by that choice. |
| P-258 | C-65 constrains | D-14 / C-64 | H-297 permits later initialization but requires definite initialization before each read. Valid storage, initialized contents, owner lifetime and access permission are distinct conditions. Aliases/explicit-owner calls refer to the same storage fact; assignment overloads cannot read an uninitialized old destination. D-14r59 scope; no runtime initialized flag, implicit zeroing or new heap owner is selected. |
| P-259 | C-65 constrains | C-64 / D-14 | H-299 confirms rejection when a writable ptr would rebind to a read-only view, preserving explicit access permission across control flow. D-14r59 scope; same-owner checks, independently writable aliases and read-only transfer results remain. H-302 refines reference-result access and allows selecting a writable-lhs/const-rhs method subject to its body checks. This scoped constraint does not qualify the wider candidate value path. |
| P-260 | C-65 constrains | C-64 / D-10 / D-14 | H-302 confirms const-qualified method distinctions, exact assignment-destination qualification, safe input weakening, jointly unique specificity and selected-body checks. Reference results retain proven access; no implicit value conversion, const escalation or post-failure retry. Scope: D-14r59/D-10r111 ordinary pointer assignment and method matching; indexed destinations and full document qualification remain separate. This serves inspectable alias/access semantics without runtime permission machinery. |
| P-261 | C-64 constrains | D-14 / D-10 | H-303 confirms that ordinary (=) method bodies must finish with their destination parameter. Explicitly returning the selected destination protects assignment-result semantics; returning another alias rejects even when storage is identical. D-14r59/D-10r111 scope; this is a semantic safeguard rather than deferred alias-proof implementation. |
| P-262 | C-67 derives-from | C-37 | H-304 confirms explicit source delegation to an existing checked operation so an overload can implement its business behavior without recursive lookup or hidden caller-sensitive dispatch. D-10r111/D-14r59; the local purpose is confirmed, wider candidate charter paths remain pending. |
| P-263 | C-67 constrains | C-64 / C-65 | Explicit builtin invocation preserves assignment-result safeguards, fixed owners, const permissions, initialization/lifetime and function dynamic-context exclusion. H-304; no unchecked storage capability, new graph-transfer builtin or runtime dispatch table. |
| P-264 | C-68 derives-from | C-65 | H-304 confirms explicit empty-range initialization as a usable continuation of H-297 definite-initialization semantics, avoiding forced pre-loop fallback writes or runtime initialization flags. D-14r59; full parent qualification remains separate. |
| P-265 | C-68 depends-on | D-10 / C-41 | Reuse checked range evaluation, ordinary block cleanup, void control results and normal-exit joins. D-10r111/D-14r59 under H-304; index scope cannot extend into else. This dependency grants no broader language qualification. |
| P-266 | C-65 constrains | D-14 | H-305 requires initialized same-owner loop swaps to rely on invariant type/owner/access/initializedness rather than exact allocation parity or a general alias-history solver. Runtime target addresses vary; initialization remains a distinct proof obligation. D-14r59; an implementation correction, not a weaker lifetime rule. |
| P-267 | C-65 constrains | D-14 / D-10 | H-305 proposes and authorizes a non-rebindable const ptr declaration group. The modifier constrains binding changes, independently of referent constness; group initialization can write target bits. The implementation retains this bounded meaning without a new const dispatch dimension. |
| P-268 | C-69 derives-from | C-65 | H-306 makes pointer-target exchange and bits exchange separate readable operations, supporting A/B storage use without conflating aliasing and copying. D-14r59; this local purpose is confirmed, not the wider pending parent charter path. |
| P-269 | C-69 constrains | C-37 / C-64 / C-65 | Both operands are destinations, evaluated once in order; swaps yield void, exact destination qualifiers and explicit external owners apply to methods, and builtin type/owner/access checks remain. Reference methods receive caller bindings, functions exclude dynamic effects and procedures use current context. H-306; D-10r111/D-14r59. |
| P-270 | C-70 derives-from | C-65 | H-275/H-307 makes resource declarations visibly distinct from ordinary object pointers in every supported position, serving readable typed storage responsibility. D-14r60; scoped local value confirmed, not the wider pending parent charter path. |
| P-271 | C-70 constrains | C-54 / C-37 | H-307 preserves borrowed versus consuming calls/results, same-instance owner pairing, non-extractable effects and E1–E5 cleanup. Region is not a user data-field type or hidden generic value carrier. D-14r60/D-10r112/D-17r12; no storage/runtime change. |
| P-272 | C-71 derives-from | C-10 | H-307 permits a library's logical write without requiring an artificial addressable element, keeping the operation's business meaning in typed methods. H-308 limits bits-like intent to library-promised/recommended chains. D-14r60; scoped local value confirmed. |
| P-273 | C-71 constrains | C-64 / C-65 | Void methods expose no result; nonvoid methods expose readonly destination-owned views, with the ordinary original-destination safeguard. Inputs determine unique dispatch; conversions cannot manufacture destination storage. Owner/access/effect checks remain. H-307/H-308; D-14r60/D-10r112. |
| P-274 | C-72 derives-from | C-65 | H-309 makes ordinary declaration kind expose the intended initialization/dereference contract and adopts ptr-marked stored edges. This scoped readability/checking purpose continues explicit typed storage responsibility; D-14r61. Wider charter qualification is not inferred, and detailed layout/proof mechanisms remain pending-human. |
| P-275 | C-72 constrains | C-54 / C-65 | Ref/ptr does not waive fixed owners, same-owner edges, payload initialization, const access or lifetime checks. H-203/H-271/H-297 remain the baseline; H-309 introduces possible null dereference, not invalid-owner or arbitrary uninitialized-memory access. D-14r61/D-10r113; null/default policy, stronger non-null ref guarantees and checked-conversion mechanisms remain proposals. |
| P-276 | C-72 constrains | C-39 / C-62 / C-65 | H-310 explicitly chooses stored references for all user-declared ref/ptr fields, including scalar fields, rather than an inline-scalar exception. D-14r62/D-10r114/D-16r5 distinguish reference slots from same-owner targets; intrinsic numeric payloads remain fixed bits and array-element storage is not implicitly changed. This selects the field model, not constructor allocation, null/defaults, encoding width or a new per-object owner. |
| P-277 | C-72 constrains | C-39 / C-62 / C-65 | H-311 clarifies that an unmodified or val field directly stores bits; H-310/P-276 governs only ref/ptr-marked fields. D-14r63/D-10r115/D-16r6 retain direct numeric payloads and same-owner reference targets. This rejects the agent's extension to every field and requires no separate scalar allocation for a direct bits field. Local val syntax and reference-field constructor allocation remain unselected. |
| P-278 | C-72 constrains | C-63 | Pending-human under H-312: make possible null dereference a visible checked obligation at the immediate callable boundary while retaining the accepted cleanup mechanism and unchecked allocation failures. D-17r13 depends on D-14 ref/ptr validity; no RTTI or implementation is authorized. P-274 supplies the scoped declaration/checking purpose; this proposed extension is not human-confirmed. |
| P-279 | C-72 derives-from | C-65 | Pending-human under H-312: explicit val locals and parameters expose independent bits storage/passing, complementing references without a new lifetime carrier. D-14r64 preserves H-272 local copies, H-273 owner annotations and H-311 field storage. Grammar, method-mode/default interactions and complete qualification remain unselected. |
| P-280 | C-72 constrains | C-63 | H-313 confirms checked NullPointerException at the immediate callable boundary, preserved in signatures, with explicit module-top-level handling. D-17r16 serves the explicit failure obligations proposed by P-278 while retaining C-63 cleanup and unchecked allocation failure. No general RTTI or exception-effect polymorphism is authorized. |
| P-281 | C-72 derives-from | C-65 | H-313 accepts independent val bits locals/parameters as explicit storage/passing intent, preserving H-272 defaults, access and lifetime rules. D-14r67 replaces P-279's pending local recommendation for this scope; no per-object owner, non-bits stack allocation or hidden object copy. |
| P-282 | C-72 constrains | C-65 / C-37 | H-314 permits an ownerless null-initialized local until its first non-null assignment, with optional in r; established owners remain fixed. H-313/H-315 requires a reserved null literal and isnull raw-pointer intrinsic. D-14r67/D-10r118: field owners are inherited, incompatible owner paths reject and null tests do not dereference payloads. |
| P-283 | C-72 constrains | C-65 | H-316 requires current ref/ptr/val declarations and direct target types without legacy parser compatibility. D-14r67/D-10r118/prototype179 preserve copy/view intent, initialization, owners, access and checked NPE. No optional-result or allocation policy is inferred. Confirmed by H-316; continuity K-442. |
| P-284 | C-72 constrains | C-65 / C-63 | H-317 confirms nullable ptr parameters/results preserve views without dereference, while ref/default object results require non-null targets; D-14r68 retains explicit external owners, lifetimes and checked NPE only on actual access. Required-ref cycles remain deferred in favor of ptr edges. |
| P-285 | C-72 constrains | C-65 | H-318 authorizes single-colon declaration annotations after grammar review; D-07r143/D-10r119/D-14r68 use : with -> callable results and unchanged <: subtyping. This serves readable uniform declarations without compatibility aliases, numeric changes or module qualification. |
| P-286 | C-63 constrains | C-65 / C-36 | H-319 confirms explicit {{ }} lexical/effect/full scopes and ordinary grouping; calls/modules retain inherent scopes. H-359/H-365 requires nested per-iteration explicit scopes for body declarations and extends the rule to zero-iteration else. H-365/D-17r26 supersedes H-319’s direct-loop-defer prohibition: direct registrations accumulate in the nearest enclosing full scope, independently of iteration effect/resource restoration. Full lowering acceptance remains outstanding. |
| P-287 | C-63 depends-on | C-65 | H-319’s explicit defer capture-list proposal is superseded for ordinary deferred calls by H-356/P-321–P-322 registration-time preparation. Borrowed owners must remain valid without lifetime extension or object copy. H-365 forbids delayed loop-control capture and outer deferred borrows of iteration resources. Contextual block-to-callable adaptation remains separately reviewed under H-320/D-18. |

### Captures, planning and completion

| Edge | Child / relation | Parent / target | Rationale, evidence and scope |
| --- | --- | --- | --- |
| P-288 | C-73 derives-from | C-65 | Candidate: restricting borrowed captures to proven regional owners preserves ordinary owner/access/lifetime reasoning for both named and anonymous callables, avoiding automatic lexical-cell promotion. H-322 proposes this direction; D-14r70 does not treat it as approval of all capture modes, syntax or escape/storage rules. Pending-human for the detailed contract. |
| P-289 | C-73 depends-on | C-36 / C-54 | Candidate dependency: lexical captures must coexist with perform/behave's typed dynamic bindings and the stack/region storage discipline. Owner metadata neither transfers ownership nor chooses environment storage. D-10r121/D-14r70; H-322 requests coordinated review. This dependency grants no implementation authority. |
| P-290 | C-73 derives-from | C-65 | Candidate: capturing a regional variable slot preserves its actual owner's validity while permitting ordinary variable updates to remain observable in a callable. H-323 supplies the use case; D-14r71 keeps lexical-cell promotion, capture syntax and environment layout unselected. Slot capture does not depend on adopting structural region types. Pending-human for the detailed mechanism. |
| P-291 | C-74 derives-from | C-39 / C-65 | Candidate: explicit region products and required field subsets express reusable storage operations in CoD while retaining concrete allocation ownership, instead of requiring a particular host root class. H-323 proposes this value path; D-13r29 matching/identity/construction/allocator policy remains pending-human. |
| P-292 | C-74 depends-on | C-72 / C-54 / D-01 | Dependency: structural matching must account for val/ref/ptr storage, access and initialization, whole-region lifetime/transfer and relocation-safe physical representation. D-13r29/D-14r71; H-311/H-190 remain scoped baselines. This edge does not authorize a new allocator policy, per-object ownership, RTTI or a C++ representation change. |
| P-293 | D-18 derives-from | C-73 / C-74 | H-324 authorizes experimental alternatives and native comparison to find practical, low-debt regional storage/capture semantics within charter/repository principles. D-18r1 scope is confirmed for branch-local exploration and evidence, not adoption of each candidate mechanism. D-14/D-13/D-10/D-17/D-16 and D-01/D-15 constraints remain. |
| P-294 | D-18 derives-from | C-65 / C-73 | H-325 confirms source-visible callable owners as anchors for reasoning and diagnostics. H-329 confirms where declarations, with associations and an actual region inferred through an object parameter rather than a mandatory region argument. D-18r9/C-03 records this scope; general nested grammar and broader representation remain review items. Earlier branch checks retain their own implemented syntax/evidence. |
| P-295 | C-75 derives-from | C-65 / C-73 | H-327 accepts naming an existing storage place with no new holder, fixed alias selection, inherited owner/access checks and capture eligibility following storage ownership. Its optional kind assertion makes intent and variable nature locally readable. D-18r8/C-02 qualifies this ordinary-place contract only; capture-list integration and whole-region/TLS inclusion remain separate. |
| P-296 | C-75 depends-on | C-72 / C-36 / C-54 / C-71 | Existing storage kind, nullability, initialization, effect-only TLS writes, whole-region ownership and non-addressable container rules constrain aliases. Dependency grants no design authority or hidden proxy/environment allocation. D-18r8/C-02 preserves these boundaries under H-327; broader inclusion is not selected. |
| P-297 | C-73 derives-from | C-65 / C-75 | H-329 confirms omitted-list inference and explicit-list completeness checks, combining convenient regional-variable use with a readable declaration of capture intent. Missing entries error; unused entries warn. D-18r9/C-03 preserves actual storage owners and separates variables in the body from owner associations in the callable type. H-331 retains every explicit capture dependency, with owner validation before unused warnings. This does not authorize lexical-cell promotion or select runtime closure representation. |
| P-298 | C-73 derives-from | C-65 / C-75 | H-334/H-335 makes retained regions usable through cb -> owner_name, serving the explicit storage/lifetime contract rather than adding a separate owner lookup mechanism. The visible with-type supplies the name, including helper-local renaming; projection borrows captured access, never acquires ownership or extends lifetime. D-18r12/C-04 covers same-body selection and projection. Runtime callable ABI, persistence and RTTI remain excluded. |
| P-299 | D-18 derives-from | C-65 / C-73 | H-337/H-338 requires a runtime callable ABI with lexical-only payload/environment storage and distinct definition/reference syntax. This gives source-visible lifetime rules to runtime-selected behavior while retaining relocatable regional ADTs/methods. D-18r14 covers one static binary image, non-null func/proc references and nullable ptr references; no persistence, hidden user heap or RTTI. |
| P-300 | D-18 derives-from | C-65 / C-73 | H-339/H-340 confirms caller-provided complete fresh-result construction, allowing usable callback factories without dangling callee environments or captured-region lifetime extension. H-342/P-301 supersedes only existing-lvalue aliasing with independent complete callable values. Actual-owner runtime alternatives remain pending; D-18 owns current coverage. |
| P-301 | D-18 derives-from | C-65 / C-73 | H-342 confirms intrinsic lexical callable values so a returned wrapper retains its selected nested callable without depending on the factory's local payload. Complete representation movement must be valid; captured regions remain borrowed. D-18r15 covers func/proc declaration copies, nested capture by value, <- replacement and nullable ptr references. This supersedes P-300's existing-lvalue alias detail while retaining caller-provided construction; it does not permit ordinary lexical-variable capture or regional callable storage. |
| P-302 | D-18 derives-from | C-73 / C-65 | H-343 requires recursive functions/procedures in the runtime-callable work: callable behavior must execute across real invocation boundaries instead of depending on finite source expansion. D-18r16 retains static owner/effect/exception checks and one-image ABI boundaries; procedure recursion is included independently of the unresolved TCO boundary. No unbounded callable-value storage or unchecked lifetime is inferred. |
| P-303 | D-18 derives-from | C-65 / C-73 | H-344 separates lexical lifetime ownership from stack backing: runtime-sized callable environments may use heap memory that the owning scope releases. D-18r17 supports growing nested values without imposing a statically fixed payload maximum, preserving owner/access checks and normal/exceptional cleanup. No independently managed user malloc lifetime, regional callable storage, regional data copy or captured-region lifetime extension follows. |
| P-304 | C-76 derives-from | C-36 / C-65 | H-346/H-347 gives functional-style computation an explicit outer-call effect boundary while retaining checked storage/lifetimes and private mutation. This serves charter §§1–3/§6/§8's typed business intent, native execution and readable source. Confirmed direction for D-19r1 only; no chosen checker or TCO guarantee. |
| P-305 | D-19 derives-from | C-76 | H-348 requests a compact deferred plan so current recursion/callable work can preserve the optional future direction without speculative implementation. D-19r1 records the accepted boundary, open decisions and proposed future evidence. This authorizes the plan, not unconfirmed detailed language mechanisms. |
| P-306 | C-76 depends-on | D-10 / D-14 / D-17 / D-18 / D-16 | Function/effect separation, ownership/access, exceptions, callable environments/ABI and planned numerical semantics constrain the future const-function checker and TCO. D-01 relocation and D-15 world boundaries remain invariants. Dependency is not approval of a particular checker, publication mechanism or backend layout. |
| P-307 | C-76 constrains | D-10 / D-14 / D-18 and affected future language/runtime work | H-348 requires material conflicts with the deferred direction to be raised to a human before an incompatible choice is activated. Current work need not implement const/TCO or satisfy a universal tail ABI. Local implementation gaps do not automatically constitute a conflict. Scope is D-19r1's recorded direction, not speculative details. |
| P-308 | C-77 derives-from | C-08 | A declared root compatibility marker gives the initial packet workflow an explicit interpretation agreement; later layout matching can refine it. H-352 confirms this scoped direction in D-07r148 DL-04, not a complete portable wire format or checksum algorithm. |
| P-309 | C-77 depends-on | D-01 / C-10 | Compatibility checking needs defined regional layouts and the involved ADT identities/layouts. Dependency only; it grants no authority to change pointer encoding, add RTTI or equate identifier equality with derived layout proof. Detailed matching remains pending. |
| P-310 | C-78 derives-from | C-09 | Saving a selected region snapshot gives applications a persistence operation around which to implement their own rebuilding/publication policy. H-352 confirms the planning responsibility in D-07r148 DL-09; capture consistency, replacement and durability trade-offs remain pending. |
| P-311 | C-78 depends-on | D-01 / C-54 | Snapshot output requires a valid regional image and a defined lifetime/access boundary during capture. Dependency only; no frozen-copy mechanism, concurrent writer protocol, allocation strategy or ownership transfer is selected. D-07r148 DL-09 keeps these interface obligations open. |
| P-312 | C-79 derives-from | C-11 / C-02 | A provider interface can expose third-party native capabilities as typed intrinsics while business/data-structure extensions stay in CoD libraries. H-352 confirms investigation of this integration direction; D-07r148 DL-11 does not qualify a native ABI or provider-loading mechanism. |
| P-313 | C-79 depends-on | C-37 / C-54 / D-15 / D-17 | Intrinsic providers must preserve callable/effect classification, owner/lifetime responsibility, module-world selection and error contracts. This dependency identifies design obligations, not permission to bypass checks, edit another world or add runtime code loading. Detailed SPI design remains pending. |
| P-314 | C-80 derives-from | C-01 | A compiler implemented around typed CoD operations makes compiler intent inspectable and exercises the language on its own domain. H-353 confirms this planning direction, explicitly avoiding a C++ implementation port; D-20r1. Exact frontend/semantic encodings remain pending. |
| P-315 | C-80 derives-from | C-05 | Saving reusable regional compiler stages as DBMR can avoid repeated graph reconstruction in large-project compilation. H-353 requests investigation of this value; D-20r1. Stage boundaries, validity keys, publication and measured benefit are pending, not guaranteed by relocation. |
| P-316 | C-80 depends-on | C-02 / C-11 / C-78 / C-79 | Native code emission needs selected LLVM services; persistent stage reuse needs snapshot support. H-352/H-353 names these dependencies without selecting provider ABI, handle classification, save/replacement behavior or implementation order. |
| P-317 | D-20 derives-from | C-80 | A separate application plan makes compiler use cases, boundaries and remaining choices reviewable without changing language contracts by implication. H-353 authorizes drafting; this revision's detailed design/value path remains pending-human. |
| P-318 | C-80 depends-on | D-01 / D-14 / D-15 / D-16 / D-17 / D-18 | Regional layouts, owners/initialization, module identities, numeric conventions, exceptions/defer and callable values constrain the pilot. Dependency only: no lexical-cell capture, native-handle persistence, eager grammar or broader qualification follows. D-20r1 lists the review obligations. |
| P-319 | C-81 derives-from | C-65 | Explicit write-only parameters can expose initialization intent and establish a caller-visible postcondition without reading uninitialized payloads. H-354 proposes this value and local handle storage; D-20r2 records the candidate. Full syntax, scope and proof rules remain pending-human. |
| P-320 | C-81 depends-on | D-14 / D-17 / C-79 | Output arguments require actual external-owner storage, access/alias checks, normal-versus-exceptional flow analysis and a trustworthy native-provider contract. Dependency only; it does not approve lexical allocation of regional graphs, resource replacement, rollback or new runtime initialization exceptions. D-20r2. |
| P-321 | C-63 derives-from | C-65 / C-10 | H-356 selects call preparation at registration and delayed invocation, with a separate delayed block form, so acquisition can return its cleanup directly without timing-marker syntax. D-17r20 serves explicit resource responsibility and inspectable effect order under charter §§1–3/§6/§8/§12. Confirmed for this bounded timing/syntax and implementation priority; native-resource contracts are separate. |
| P-322 | C-63 depends-on | D-14 / D-18 / C-36 | Prepared arguments and callable environments retain declared passing modes, actual owners/access and direct/perform/behave semantics. H-356/D-17r20 does not authorize lifetime extension, regional callable storage or changing ownership transfer merely to save an argument. Dependency checks and cleanup evidence remain necessary. |
| P-323 | C-63 derives-from | C-10 / C-65 | H-357/H-358, D-17r22: later deferred handlers can recover from earlier cleanup failures, attached finally performs unconditional cleanup, and recovery resumes the retained normal result. This serves recoverable business failure while preserving explicit resource responsibility under charter §§2–3/§6/§12. H-360/D-17r23 adds direct catch entry for pending exceptions, bodyless catch/finally and original scope-body recovery. H-361 supplies explicit returns; H-362/D-17r24 confirms enclosing return/replacement with continued cleanup and void fallthrough; H-363 records the deferred pure surface. D-14/D-18 dependencies do not grant wider authority. |
| P-324 | C-63 constrains | D-10 / D-18 | H-361/D-17r23 requires explicit value returns from proc/func brace bodies and finally, with unchecked runtime missing-return failure and static diagnostics where possible. This separates visible business control from an incidental last expression under charter §§2–3/§6/§12. H-362/D-17r24 confirms enclosing return/replacement from deferred handlers with continued cleanup and void fallthrough. H-363/D-19r2 selects pure declarations with implicit final-expression results; purity implementation remains deferred. Existing value/owner/ABI contracts remain dependencies and retain qualification; implicit callable-body result syntax is superseded for future implementation. |
| P-325 | C-76 derives-from | C-37 / C-65 | H-363/D-19r2 selects pure as the const-function declaration keyword with an implicit final-expression body result, retaining the external-readonly/private-mutation boundary. This preserves readable functional computation and explicit state responsibility under charter §§2–3/§6/§12. Syntax is recorded; purity implementation and TCO remain explicitly deferred, with callable-type and checker mechanisms pending. |
| P-326 | C-63 constrains | C-36 / C-54 / C-65 | H-365/D-17r26: every iteration and zero-iteration else restores entry effects on every exit, releases only freshly owned effect resources, and preserves captured/hidden outer owners. User defer belongs to the nearest full scope with distinct eager registrations and global LIFO; delayed blocks cannot capture loop-control bindings and saved borrows must outlive invocation. This preserves explicit responsibility and inspectable effects under charter §§2–3/§6/§8/§12, with accumulation measured rather than erased by early cleanup. D-10r124/D-14r73 reference the dedicated loop contract; no new generic scope abstraction or lifetime extension. |
| P-327 | D-14 / D-18 derives-from | C-65 / C-48 / C-75 | Confirmed H-366–H-369, D-14r75/D-18r68: independent binding and propagated target-access qualifiers make storage permissions locally readable and prevent alias/call/result access escalation under charter §§1–3/§6/§8/§12. Ordinary parameter binding const is local, excluded from signature/overload identity; target const retains its access contract. Fixed alias places and H-272/H-293/H-302/H-327 owner/storage rules remain. D-19r3 depends on this distinction without implementing purity/TCO. No global freeze, regional copy or lifetime extension. |
| P-328 | C-63 constrains | C-65 | H-370/D-17r31 confirms deferred argument preparation retains selected owner identity without consuming it; ownership transfers at deferred callee entry under E4. Visible acquire/defer/use ordering and one transfer responsibility serve charter §§2–3/§6/§8/§12. Existing owner/lifetime checks prevent invalidated or duplicate consumption; no reservation mechanism, new runtime failure, copied owner or lifetime extension is selected. |
| P-329 | C-63 constrains | C-65 | H-371/D-17r32 requires explicit nested scopes for deferred-block declarations/effect installation. Visible lifetime/effect responsibility serves charter §§2–3/§6/§8/§12 while preserving H-319 grouping and H-362 enclosing returns. Confirmed H-371; D-17 deferred blocks, no implicit invocation scope or new lifetime extension. |

### Families, acquisition and architecture

| Edge | Child / relation | Parent / target | Rationale, evidence and scope |
| --- | --- | --- | --- |
| P-330 | C-59 constrains | C-36 | Confirmed H-372/H-373: definition/extension constructs module information at compilation without effect; runtime TLS installation requires effect. Combined effect syntax defines or extends and installs a compatible family view. Existing initialization incorporation is separate. D-15r17/D-10r128; supersedes only implicit registration/selection-as-installation interpretations of H-184/H-186/H-187/H-290/P-254. |
| P-331 | C-35 constrains | C-37 | Confirmed H-373: bare other denotes one registered symbol object with identity/semantics; @other reads TLS. effect @dst = other materializes its selected family view, effect @dst = @other preserves an existing value/captures, and effect @dst installs the current composed family. Cross-identity installation changes neither world's methods. D-15r17/D-10r128; shadowing/staging and public symbol ABI remain open. |
| P-332 | C-59 constrains | C-58 | Confirmed H-374: export of a symbol-identified callable publishes all its methods. Existing import/using/mixin authority and selected contexts remain; no hidden private subset, implicit TLS installation or cross-world mutation. D-15r17/D-07r149. |
| P-333 | C-82 derives-from | C-01 | Confirmed H-378: actual-type family dispatch preserves abstract business intent and selected interpretation independently of specialization. Charter §§2–3/§6.3/§8; D-21r1 behavioral baseline continued in r2 under K-535. Physical type evidence/ABI and other mechanism choices are excluded. |
| P-334 | C-83 derives-from | C-02 | Confirmed H-378: precompiled coverage makes native effects available for admitted abstract programs without compilation during covered execution; measured cost governs later strategy changes. Charter §§5.2/§6/§12; D-21r1 →2. Closure algorithms and physical dispatch mechanisms remain separate decisions. H-387 authorizes replacement-architecture exploration now; the measured-growth prerequisite no longer gates exploration. Native-before-execution remains the recorded AOT baseline, not a constraint imposed on every experiment; replacement production timing is needs-reconfirmation. |
| P-335 | C-82 constrains | C-37 / C-57 | Confirmed H-375: callable values reference family dispatchers with selected context/environment; actual types, including abstract parameters/results, select methods. Direct selection is an equivalent optimization. D-21r1/D-15r17/D-10r128/D-18r70; answers H-251's behavioral question, leaving concrete evidence/ABI and uncovered failure semantics open. |
| P-336 | C-83 constrains | C-82 | Confirmed H-376: WPO emits admissible leaf-ADT combinations and dispatch before execution. Share equivalent targets and prune proved-impossible cases without dropping coverage. JIT is only a future option if measured growth warrants it. D-21r1; no dense table, per-tuple code copy or runtime world mutation selected. H-387 authorizes replacement-architecture exploration now; the measured-growth prerequisite no longer gates exploration. Native-before-execution remains the recorded AOT baseline, not a constraint imposed on every experiment; replacement production timing is needs-reconfirmation. H-396’s current task prepares shared method bodies and local operation paths AOT; neither coverage nor the language requires one body per concrete tuple. |
| P-337 | D-21 derives-from | C-82 / C-83 | Confirmed H-378: the vision connects abstract dispatch, selected module interpretation, compilation coverage and measured costs before implementation. Qualified only for D-21r1 behavioral scope/acceptance, retained in r2; no blanket mechanism or whole-document approval. H-387 authorizes replacement-architecture exploration now; the measured-growth prerequisite no longer gates exploration. Native-before-execution remains the recorded AOT baseline, not a constraint imposed on every experiment; replacement production timing is needs-reconfirmation. |
| P-338 | D-21 depends-on | D-10 / D-14 / D-15 / D-18 | Stage/call-mode, owner/access/lifetime, world identity and complete-value contracts constrain family dispatch and concrete-type evidence. D-16 compatibility additionally governs intrinsic/UoM/representation interactions. D-21r1; dependency only, no added authority, regional allocation rule, owner-union policy or fraction implementation. |
| P-339 | C-59 constrains | C-36 | Confirmed H-379/H-380: entry extension requires an explicit callable installation; omission is a compile error. Correct entry installation cancels dependency module-initialization installations of that callable, retaining procedure-local installations. D-21r3; identity matching and conditional coverage remain mechanism design. |
| P-340 | C-83 constrains | C-82 | Confirmed H-379/H-380: LLVM excludes unused dependency-world code after CoD removes canceled initialization TLS writes as roots. Retain operand effects/failures, explicit source views and other reachable code. D-21r3; pass/linkage mechanisms and final-image acceptance remain outstanding. |
| P-341 | C-36 constrains | C-83 | Confirmed H-380: cancellation omits only the dependency initialization TLS write, not operand evaluation, callable construction or their effects/failures. Procedure-local effects remain active. This preserves observable typed execution while permitting LLVM to remove genuinely unused code. D-21r3/D-10r129/D-17r37; ordinary temporary cleanup applies, without new owner or exception rules. |
| P-342 | C-35 constrains | C-37 / C-41 | Confirmed H-381: bare-name and @ lookup use the nearest lexical binding; a local non-symbol binding blocks outer symbol access rather than opening a separate @ namespace. D-21r4/D-10r130; preserve existing lexical scope/identity rules. Runtime symbol storage/ABI and unresolved same-scope module-category collisions are not selected. |
| P-343 | C-65 constrains | C-35 / C-41 | Confirmed H-382: every inferred owner binder is an implicit const lexical binding in its callable body, including generic region-or-scope owners. It participates in P-342 nearest lookup; binding constness grants no region capability, content immutability, ownership or lifetime extension. D-14r77/D-10r131/D-21r5; actual owner remains inferred from argument provenance and needs no new runtime object/ABI. |
| P-344 | C-54 constrains | C-39 / C-65 | Confirmed H-383: only designated intrinsic resource references distinguish owned/borrowed, currently regions. Ordinary ref/ptr is borrowed; omission of an owner annotation supplies no release authority. Instance/storage owners and intrinsic cleanup holders are distinct; H-196 containment and H-203 same-owner fields remain. D-22r1/D-14r78. |
| P-345 | C-39 constrains | C-72 | Confirmed H-383/H-384 R1: recursively finite bits-only ADTs, including finite products/sums and eligible instantiations, may be lexical or regional; stored references/owners/environments/cleanup exclude bits. Non-bits ADTs remain region-only. This refines H-193/K-321 only for bits eligibility, without ABI, sum encoding, numeric conversion or arbitrary-bit validity approval. D-22r1/D-07r152/D-14r78. |
| P-346 | C-65 constrains | C-39 / C-72 | Confirmed H-384 R2 amendment: make T [in o] explicitly selects actual construction owner; receiver-owner mismatch rejects even for equal root types/lifetimes. Owner identity grants no implicit allocation capability. H-386/P-352 separately accepts contextual inference; integrated whole-file qualification remains pending. D-22r1/D-14r78. |
| P-347 | C-54 / C-65 constrains | C-09 | Confirmed H-385: make const region restricts the published result view, preserving explicitly requested preparation writes and acquisition cleanup. It does not freeze the file, imply snapshot/durability, or select grammar, sharing, resize/failure protocols. D-22r1/D-14r78/D-17r41. |
| P-348 | C-39 / C-65 / C-72 derives-from | C-01 / C-06 | Candidate scoped value: separate representation, actual storage owner, borrowed access and intrinsic resource responsibility so source construction preserves checkable business intent and readable lifetime boundaries (charter §§2–5/§8). D-22r1 ordinary requirements/constraints/acceptance; pending-human whole-document review under H-386. H-383/H-384 confirms the propositions, not this newly recorded derivation. |
| P-349 | C-09 / C-54 derives-from | C-05 | Candidate scoped value: explicit acquisition/preparation/publication makes DBMR access and cleanup inspectable while persistent data survives an acquisition (charter §§4.3/§7). D-22r1 DBMR scope; pending-human. H-385 result constness and H-384 exploration do not qualify a concrete file protocol. |
| P-350 | D-22 derives-from | C-39 / C-65 / C-72 / C-09 / C-54 | Candidate document relationship: organize ordinary storage construction through P-348 and intrinsic DBMR acquisition through P-349 with distinct representation/owner/access/cleanup responsibilities and composed acceptance. D-22r1 →r2; pending-human whole-document review under H-386, with explicit scope/exclusions retained. No whole-document or mechanism approval follows from handoff approval. |
| P-351 | D-22 depends-on | D-01 / D-14 / D-17 / D-10 / D-15 / D-16 / D-18 / D-21 | Physical relative storage, owner/capability/qualifiers, initialization/failures/cleanup, effect/world identity, numeric validity/layout and complete callable/native dispatch contracts constrain construction. Dependency only; no aggregate ABI, DBMR algorithm, fraction implementation, new ownership kind or toolchain authorization. |
| P-352 | C-65 constrains | C-39 / C-72 | Confirmed H-386 discussion decision: omitted make owner is inferred from a unique receiver/containing-field owner only with usable allocation capability. With no destination, bits use the current full lexical scope; non-bits require an explicit region. Missing capability never selects hidden allocation/fallback; existing references retain actual owner/access. D-22r2/D-14r79; complete document qualification remains pending. |
| P-353 | C-39 / C-54 constrains | D-22 | H-386 prefers a single coordinated migration to ordinary make T [in o] and intrinsic make region, with no transition phase or legacy compatibility burden because the user reports no CoD users yet. D-22r2 plans that direction and retains actionable migration diagnostics. Whole-file approval, aggregate ABI and DBMR protocols remain separate; this does not activate incomplete compiler syntax. |
| P-354 | D-23 derives-from | C-65 / C-72 / C-54 | Candidate document relationship: make scalar result storage, aliasing and value evaluation explicit so native staging preserves readable, checked business intent (charter §§2–3/§5.3/§6/§8/§12). D-23r2 all sections; pending-human whole-file review. H-201 constrains the topic but does not approve a newly selected fresh-result mechanism. |
| P-355 | D-23 depends-on | D-14 / D-17 / D-18 / D-16 / D-22 | Existing storage/access/initialization, completion, callable ABI, fixed bits validity and construction boundaries constrain scalar results. D-23r2; dependency only. No authority for caller-result allocation, mixed-owner selection, RTTI, fractions or aggregate ABI follows. |
| P-356 | C-84 derives-from | C-01 / C-06 | Candidate: infer redundant implemented-method result spelling while retaining complete checked signatures, readable source and finite recursive semantic graphs under charter §§2–3/§5.3/§6/§8/§12. D-24r1 all sections; pending-human, with no confirmed inference baseline. |
| P-357 | D-24 derives-from | C-84 | Candidate document relationship: make optional annotation, cyclic inference, native transport and result-category boundaries reviewable as a whole file. D-24r1; pending-human under H-386 review format. |
| P-358 | D-24 depends-on | D-10 / D-14 / D-17 / D-18 / D-16 / D-21 / D-22 / D-23 | Selected method/world, storage/access/initialization, completion, complete native values, numerical validity and unresolved dispatch/construction/scalar boundaries constrain inference. D-24r1; dependency only, no qualification of source policy, ABI or ownership. |
| P-359 | C-85 derives-from | C-01 / C-04 | Candidate value path: retain inspectable typed business intent and separate program/interpretation/state while allowing affordable execution strategies (charter §§1–3/§4.3/§5–6/§8/§12). H-387 authorizes exploration and its fused-artifact boundary; this agent-proposed detailed derivation is pending-human whole-file review, not production delegation. |
| P-360 | D-25 derives-from | C-85 / C-02 | Candidate document path: derive an affordable native execution architecture from preserved intent and discriminating evidence, excluding sunk costs. Detailed production path pending-human. H-396 authorizes the micro-dispatch replacement and scoped conditional merge; H-400 refines H-390's input boundary through D-26. H-392 evidence exclusions remain. Archived H-393–H-395/K-582–K-584 does not select the older recommendation. |
| P-361 | D-25 depends-on | D-01 / D-10 / D-14 / D-15 / D-17 / D-18 / D-21 / D-16 | Relative storage, owners/access, world identity, complete values and completion constrain experiments. Existing AOT timing is explicitly reviewed rather than conflated with semantic completeness. Dependency only; D-22/D-23/D-24 pending mechanisms remain unselected. |
| P-362 | C-85 constrains | C-82 / C-83 | H-387/H-390 as scoped by H-400: fused artifact is not merely entry world; preserve reachable dependency-local interpretations and procedure-local installations, initialization operand effects/failures/cleanup, actual owners and call modes. H-400 replaces the known-ADT-only input limit with D-26r3/C-90's `<any>` selection and checked unavailable-interpretation boundary; it does not authorize incoming executable methods or runtime compilation. Input-validation/transport and composition/publication protocols remain unselected. Experimental RTTI/layout checksums do not equate layout with nominal/domain/UoM/permission/method identity. |
| P-363 | C-86 derives-from | C-82 / C-83 | Candidate value path: retain abstract method behavior and prepared native execution while avoiding mandatory whole-method tuple cloning. H-396 authorizes the direction/experiment; full production derivation and limits remain pending-human. |
| P-364 | D-21 derives-from | C-86 | Candidate organization of semantic methods, general/specialized implementations, independent local operations and proven call-site specialization, bounded by H-378's unchanged behavioral path. D-21r11/H-396/H-409/H-410; preparation and runtime selection remain distinct, and detailed production qualification is pending. |
| P-365 | C-87 derives-from | C-82 / C-85 | Candidate path: preserve observable actual-type selection across abstract flows and selected interpretations without a parallel identity subsystem or unnecessary runtime metadata. H-396/H-397 confirms the discriminator and persisted-meaning principles, not its permanent physical encoding or whole derivation. |
| P-366 | C-86 depends-on | C-87 / D-10 / D-14 / D-15 / D-16 / D-17 / D-18 | Shared bodies require concrete evidence, complete environments, actual owners/access, context and completion contracts. Dependency only; no owner union or new numeric policy. |
| P-367 | C-88 derives-from | C-01 / C-04 | Candidate path: root compatibility evidence helps preserve inspectable semantic meaning while distinguishing stored data from a current program's type/version interpretation. H-396 planning direction; checksum equality alone is insufficient and production path remains pending. |
| P-368 | C-88 depends-on | D-01 / D-07 / D-16 | Root identity/schema, layout/relative references, semantic types and units constrain compatibility. Planning only; no loader/header/migration or canonicalization policy is qualified. |
| P-369 | C-87 depends-on | C-88 / D-01 / D-14 | H-397 requires persisted discriminator carriers to preserve intended identity interpretation across compatible readers alongside actual owner/reference validation. This scoped dependency does not require DBMR compatibility metadata for every in-memory value. |
| P-370 | D-25 depends-on | D-21 | The assessment uses D-21r11's separated AOT preparation, runtime execution, semantic method selection, implementation selection and local-operation responsibilities and its bounded evidence; it does not qualify pending interfaces through experiment success. |
| P-371 | C-89 derives-from | C-83 / C-02 | Confirmed H-407 for D-26r6 requirements, continued in r7: finite checked native coverage preserves prepared execution for abstract business calls without mandatory tuple cloning; uncertainty stays inspectable. Includes selected runtime failures and concrete compiler-limit diagnostics without enumerating every type or deciding arbitrary analysis termination. Concrete algorithms remain unselected. |
| P-372 | D-26 derives-from | C-89 | Confirmed H-407 for r6, continued in r7: organizes native coverage responsibility, recursive obligations, concrete diagnostics and acceptance for source migration. The whole-file scope fixes these requirements without selecting a concrete analysis procedure or requiring all acceptance paths in the current stage. |
| P-373 | D-26 depends-on | D-07 / D-10 / D-11 / D-14 / D-15 / D-16 / D-17 / D-18 / D-21 / D-24 | Method selection, generic/conformance meaning, owners/access, contexts, complete values, completion, units and pending result-inference policy constrain closure. Dependency only; no qualification of those pending choices. |
| P-374 | C-90 derives-from | C-82 / C-83 | Confirmed H-407 for D-26r6 requirements, continued in r7: retain inspectable type identity and abstract method selection while making the boundary of prepared interpretation explicit. Parent paths are C-82/P-333 → C-01 and C-83/P-334 → C-02. H-400/H-401/H-404/H-405 supplies the bounded behavior; physical mechanisms remain unselected. |
| P-375 | D-26 derives-from | C-90 | Confirmed H-407 for r6, continued in r7: organizes `<any>` selection, actual identity/same-T, valid uninterpreted passage, checked unavailable interpretation and unchecked soundness failure into a reviewable source/native contract. Preserves generic versus voluntary throws obligations; no permanent carrier/ABI or exception-polymorphism mechanism follows. |
| P-376 | C-90 depends-on | D-07 / D-14 / D-15 / D-16 / D-17 / D-18 / D-21 | Nominal relations, actual owners, selected world, numerical identity, exception/cleanup rules and complete callable evidence constrain runtime interpretation. Dependency only; no physical ABI, input-validation protocol, hidden owner retention or fraction implementation is approved. |
| P-377 | C-49 derives-from | C-01 | Pending-human for D-27r1: named unit identities keep domain distinctions explicit and checkable across representation choices, rather than relying on comments or bit-layout equality. H-128 supplies the identity requirement; it is not confirmation of this proposed value path. |
| P-378 | C-61 derives-from | C-01 | Pending-human for D-27r1: representation independence and whole-type lossless compatibility preserve domain meaning and numerical values while exposing storage/precision choices. H-260/H-261 supplies behavioral constraints, not approval of this relationship. |
| P-379 | D-27 derives-from | C-49 / C-61 | Pending-human r1: organizes the named-quantity and lossless-compatibility requirements, exclusions and native acceptance as a bounded implementation contract; does not qualify D-11 wholesale. |
| P-380 | D-27 depends-on | D-11 / D-16 / D-14 / D-17 / D-18 / D-21 / D-26 | Numeric/fraction boundaries, owner/access/initialization, preparation/cleanup, callable values and actual-type native coverage constrain r1. Dependency only; no new authority, generic inference, module mechanism or ABI follows. |


## 5. Evidence locators

H records are human evidence; K records are agent assessments, never additional human approval. Original wording, provenance, timestamps or uncertainty, exclusions and corrections remain in the immutable source. Line numbers refer to that source, not this file. Read only the relevant event.

| Source | Commit / path | Records |
| --- | --- | --- |
| A | `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848`: `design/DesignAuthorityRegistry.md` | H-01–H-387, K-00–K-577 |
| B | `061a4a684770447ba674e54079f9a157aa75e43e`: `design/DesignAuthorityRegistry.md` | H-388/H-389, K-578; earlier complete nodes, relations and status narratives |

Example: retrieve H-05 using its A-source range. Substitute another event's source/range as needed.

```sh
git show 0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md | sed -n '1229,1235p'
```

### Human evidence

| Event | Source: lines |
| --- | --- |
| <a id="h-01"></a>H-01 | A:1201–1207 |
| <a id="h-02"></a>H-02 | A:1208–1214 |
| <a id="h-03"></a>H-03 | A:1215–1221 |
| <a id="h-04"></a>H-04 | A:1222–1228 |
| <a id="h-05"></a>H-05 | A:1229–1235 |
| <a id="h-06"></a>H-06 | A:1236–1247 |
| <a id="h-07"></a>H-07 | A:1248–1259 |
| <a id="h-08"></a>H-08 | A:1260–1271 |
| <a id="h-09"></a>H-09 | A:1272–1283 |
| <a id="h-10"></a>H-10 | A:1284–1292 |
| <a id="h-11"></a>H-11 | A:1293–1301 |
| <a id="h-12"></a>H-12 | A:1302–1310 |
| <a id="h-13"></a>H-13 | A:1311–1319 |
| <a id="h-14"></a>H-14 | A:1320–1328 |
| <a id="h-15"></a>H-15 | A:1329–1337 |
| <a id="h-16"></a>H-16 | A:1338–1346 |
| <a id="h-17"></a>H-17 | A:1347–1355 |
| <a id="h-18"></a>H-18 | A:1356–1364 |
| <a id="h-19"></a>H-19 | A:1365–1373 |
| <a id="h-20"></a>H-20 | A:1374–1382 |
| <a id="h-21"></a>H-21 | A:1383–1393 |
| <a id="h-22"></a>H-22 | A:1394–1402 |
| <a id="h-23"></a>H-23 | A:1403–1413 |
| <a id="h-24"></a>H-24 | A:1414–1426 |
| <a id="h-25"></a>H-25 | A:1427–1437 |
| <a id="h-26"></a>H-26 | A:1438–1446 |
| <a id="h-27"></a>H-27 | A:1447–1457 |
| <a id="h-28"></a>H-28 | A:1458–1468 |
| <a id="h-29"></a>H-29 | A:1469–1479 |
| <a id="h-30"></a>H-30 | A:1480–1490 |
| <a id="h-31"></a>H-31 | A:1491–1501 |
| <a id="h-32"></a>H-32 | A:1502–1512 |
| <a id="h-33"></a>H-33 | A:1513–1523 |
| <a id="h-34"></a>H-34 | A:1524–1534 |
| <a id="h-35"></a>H-35 | A:1535–1545 |
| <a id="h-36"></a>H-36 | A:1546–1557 |
| <a id="h-37"></a>H-37 | A:1558–1568 |
| <a id="h-38"></a>H-38 | A:1569–1579 |
| <a id="h-39"></a>H-39 | A:1580–1590 |
| <a id="h-40"></a>H-40 | A:1591–1602 |
| <a id="h-41"></a>H-41 | A:1603–1614 |
| <a id="h-42"></a>H-42 | A:1615–1626 |
| <a id="h-43"></a>H-43 | A:1627–1638 |
| <a id="h-44"></a>H-44 | A:1639–1650 |
| <a id="h-45"></a>H-45 | A:1651–1664 |
| <a id="h-46"></a>H-46 | A:1665–1676 |
| <a id="h-47"></a>H-47 | A:1677–1688 |
| <a id="h-48"></a>H-48 | A:1689–1700 |
| <a id="h-49"></a>H-49 | A:1701–1712 |
| <a id="h-50"></a>H-50 | A:1713–1724 |
| <a id="h-51"></a>H-51 | A:1725–1737 |
| <a id="h-52"></a>H-52 | A:1738–1749 |
| <a id="h-53"></a>H-53 | A:1750–1762 |
| <a id="h-54"></a>H-54 | A:1763–1774 |
| <a id="h-55"></a>H-55 | A:1775–1786 |
| <a id="h-56"></a>H-56 | A:1787–1806 |
| <a id="h-57"></a>H-57 | A:1807–1818 |
| <a id="h-58"></a>H-58 | A:1819–1831 |
| <a id="h-59"></a>H-59 | A:1832–1843 |
| <a id="h-60"></a>H-60 | A:1844–1856 |
| <a id="h-61"></a>H-61 | A:1857–1869 |
| <a id="h-62"></a>H-62 | A:1870–1884 |
| <a id="h-63"></a>H-63 | A:1885–1897 |
| <a id="h-64"></a>H-64 | A:1898–1910 |
| <a id="h-65"></a>H-65 | A:1911–1928 |
| <a id="h-66"></a>H-66 | A:1929–1944 |
| <a id="h-67"></a>H-67 | A:1945–1958 |
| <a id="h-68"></a>H-68 | A:1959–1982 |
| <a id="h-69"></a>H-69 | A:1983–1994 |
| <a id="h-70"></a>H-70 | A:1995–2006 |
| <a id="h-71"></a>H-71 | A:2007–2026 |
| <a id="h-72"></a>H-72 | A:2027–2038 |
| <a id="h-73"></a>H-73 | A:2039–2050 |
| <a id="h-74"></a>H-74 | A:2051–2063 |
| <a id="h-75"></a>H-75 | A:2064–2077 |
| <a id="h-76"></a>H-76 | A:2078–2089 |
| <a id="h-77"></a>H-77 | A:2090–2101 |
| <a id="h-78"></a>H-78 | A:2102–2117 |
| <a id="h-79"></a>H-79 | A:2118–2129 |
| <a id="h-80"></a>H-80 | A:2130–2141 |
| <a id="h-81"></a>H-81 | A:2142–2153 |
| <a id="h-82"></a>H-82 | A:2154–2169 |
| <a id="h-83"></a>H-83 | A:2170–2185 |
| <a id="h-84"></a>H-84 | A:2186–2201 |
| <a id="h-85"></a>H-85 | A:2202–2217 |
| <a id="h-86"></a>H-86 | A:2218–2231 |
| <a id="h-87"></a>H-87 | A:2232–2247 |
| <a id="h-88"></a>H-88 | A:2248–2259 |
| <a id="h-89"></a>H-89 | A:2260–2271 |
| <a id="h-90"></a>H-90 | A:2272–2283 |
| <a id="h-91"></a>H-91 | A:2284–2295 |
| <a id="h-92"></a>H-92 | A:2296–2305 |
| <a id="h-93"></a>H-93 | A:2306–2319 |
| <a id="h-94"></a>H-94 | A:2320–2335 |
| <a id="h-95"></a>H-95 | A:2336–2345 |
| <a id="h-96"></a>H-96 | A:2346–2361 |
| <a id="h-97"></a>H-97 | A:2362–2371 |
| <a id="h-98"></a>H-98 | A:2372–2381 |
| <a id="h-99"></a>H-99 | A:2382–2391 |
| <a id="h-100"></a>H-100 | A:2392–2401 |
| <a id="h-101"></a>H-101 | A:2402–2411 |
| <a id="h-102"></a>H-102 | A:2412–2421 |
| <a id="h-103"></a>H-103 | A:2422–2431 |
| <a id="h-104"></a>H-104 | A:2432–2441 |
| <a id="h-105"></a>H-105 | A:2442–2451 |
| <a id="h-106"></a>H-106 | A:2452–2461 |
| <a id="h-107"></a>H-107 | A:2462–2471 |
| <a id="h-108"></a>H-108 | A:2472–2481 |
| <a id="h-109"></a>H-109 | A:2482–2491 |
| <a id="h-110"></a>H-110 | A:2492–2501 |
| <a id="h-111"></a>H-111 | A:2502–2512 |
| <a id="h-112"></a>H-112 | A:2513–2523 |
| <a id="h-113"></a>H-113 | A:2524–2534 |
| <a id="h-114"></a>H-114 | A:2535–2545 |
| <a id="h-115"></a>H-115 | A:2546–2556 |
| <a id="h-116"></a>H-116 | A:2557–2567 |
| <a id="h-117"></a>H-117 | A:2568–2577 |
| <a id="h-118"></a>H-118 | A:2578–2588 |
| <a id="h-119"></a>H-119 | A:2589–2599 |
| <a id="h-120"></a>H-120 | A:2600–2610 |
| <a id="h-121"></a>H-121 | A:2611–2621 |
| <a id="h-122"></a>H-122 | A:2622–2632 |
| <a id="h-123"></a>H-123 | A:2633–2647 |
| <a id="h-124"></a>H-124 | A:2648–2658 |
| <a id="h-125"></a>H-125 | A:2659–2669 |
| <a id="h-126"></a>H-126 | A:2670–2680 |
| <a id="h-127"></a>H-127 | A:2681–2691 |
| <a id="h-128"></a>H-128 | A:2692–2702 |
| <a id="h-129"></a>H-129 | A:2703–2713 |
| <a id="h-130"></a>H-130 | A:2714–2724 |
| <a id="h-131"></a>H-131 | A:2725–2735 |
| <a id="h-132"></a>H-132 | A:2736–2746 |
| <a id="h-133"></a>H-133 | A:2747–2757 |
| <a id="h-134"></a>H-134 | A:2758–2768 |
| <a id="h-135"></a>H-135 | A:2769–2779 |
| <a id="h-136"></a>H-136 | A:2780–2790 |
| <a id="h-137"></a>H-137 | A:2791–2801 |
| <a id="h-138"></a>H-138 | A:2802–2813 |
| <a id="h-139"></a>H-139 | A:2814–2824 |
| <a id="h-140"></a>H-140 | A:2825–2835 |
| <a id="h-141"></a>H-141 | A:2836–2845 |
| <a id="h-142"></a>H-142 | A:2846–2855 |
| <a id="h-143"></a>H-143 | A:2856–2865 |
| <a id="h-144"></a>H-144 | A:2866–2875 |
| <a id="h-145"></a>H-145 | A:2876–2885 |
| <a id="h-146"></a>H-146 | A:2886–2895 |
| <a id="h-147"></a>H-147 | A:2896–2905 |
| <a id="h-148"></a>H-148 | A:2906–2917 |
| <a id="h-149"></a>H-149 | A:2918–2929 |
| <a id="h-150"></a>H-150 | A:2930–2941 |
| <a id="h-151"></a>H-151 | A:2942–2951 |
| <a id="h-152"></a>H-152 | A:2952–2961 |
| <a id="h-153"></a>H-153 | A:2962–2971 |
| <a id="h-154"></a>H-154 | A:2972–2981 |
| <a id="h-155"></a>H-155 | A:2982–2991 |
| <a id="h-156"></a>H-156 | A:2992–3001 |
| <a id="h-157"></a>H-157 | A:3002–3013 |
| <a id="h-158"></a>H-158 | A:3014–3023 |
| <a id="h-159"></a>H-159 | A:3024–3033 |
| <a id="h-160"></a>H-160 | A:3034–3043 |
| <a id="h-161"></a>H-161 | A:3044–3053 |
| <a id="h-162"></a>H-162 | A:3054–3063 |
| <a id="h-163"></a>H-163 | A:3064–3073 |
| <a id="h-164"></a>H-164 | A:3074–3083 |
| <a id="h-165"></a>H-165 | A:3084–3093 |
| <a id="h-166"></a>H-166 | A:3094–3103 |
| <a id="h-167"></a>H-167 | A:3104–3113 |
| <a id="h-168"></a>H-168 | A:3114–3123 |
| <a id="h-169"></a>H-169 | A:3124–3133 |
| <a id="h-170"></a>H-170 | A:3134–3143 |
| <a id="h-171"></a>H-171 | A:3144–3153 |
| <a id="h-172"></a>H-172 | A:3154–3167 |
| <a id="h-173"></a>H-173 | A:3168–3177 |
| <a id="h-174"></a>H-174 | A:3178–3187 |
| <a id="h-175"></a>H-175 | A:3188–3197 |
| <a id="h-176"></a>H-176 | A:3198–3207 |
| <a id="h-177"></a>H-177 | A:3208–3217 |
| <a id="h-178"></a>H-178 | A:3218–3227 |
| <a id="h-179"></a>H-179 | A:3228–3237 |
| <a id="h-180"></a>H-180 | A:3238–3249 |
| <a id="h-181"></a>H-181 | A:3250–3261 |
| <a id="h-182"></a>H-182 | A:3262–3271 |
| <a id="h-183"></a>H-183 | A:3272–3281 |
| <a id="h-184"></a>H-184 | A:3282–3291 |
| <a id="h-185"></a>H-185 | A:3292–3301 |
| <a id="h-186"></a>H-186 | A:3302–3311 |
| <a id="h-187"></a>H-187 | A:3312–3321 |
| <a id="h-188"></a>H-188 | A:3322–3331 |
| <a id="h-189"></a>H-189 | A:3332–3341 |
| <a id="h-190"></a>H-190 | A:3342–3351 |
| <a id="h-191"></a>H-191 | A:3352–3361 |
| <a id="h-192"></a>H-192 | A:3362–3371 |
| <a id="h-193"></a>H-193 | A:3372–3385 |
| <a id="h-194"></a>H-194 | A:3386–3395 |
| <a id="h-195"></a>H-195 | A:3396–3405 |
| <a id="h-196"></a>H-196 | A:3406–3415 |
| <a id="h-197"></a>H-197 | A:3416–3427 |
| <a id="h-198"></a>H-198 | A:3428–3437 |
| <a id="h-199"></a>H-199 | A:3438–3447 |
| <a id="h-200"></a>H-200 | A:3448–3457 |
| <a id="h-201"></a>H-201 | A:3458–3469 |
| <a id="h-202"></a>H-202 | A:3470–3481 |
| <a id="h-203"></a>H-203 | A:3482–3491 |
| <a id="h-204"></a>H-204 | A:3492–3501 |
| <a id="h-205"></a>H-205 | A:3502–3511 |
| <a id="h-206"></a>H-206 | A:3512–3523 |
| <a id="h-207"></a>H-207 | A:3524–3535 |
| <a id="h-208"></a>H-208 | A:3536–3545 |
| <a id="h-209"></a>H-209 | A:3546–3559 |
| <a id="h-210"></a>H-210 | A:3560–3569 |
| <a id="h-211"></a>H-211 | A:3570–3581 |
| <a id="h-212"></a>H-212 | A:3582–3591 |
| <a id="h-213"></a>H-213 | A:3592–3601 |
| <a id="h-214"></a>H-214 | A:3602–3611 |
| <a id="h-215"></a>H-215 | A:3612–3621 |
| <a id="h-216"></a>H-216 | A:3622–3631 |
| <a id="h-217"></a>H-217 | A:3632–3641 |
| <a id="h-218"></a>H-218 | A:3642–3651 |
| <a id="h-219"></a>H-219 | A:3652–3661 |
| <a id="h-220"></a>H-220 | A:3662–3671 |
| <a id="h-221"></a>H-221 | A:3672–3681 |
| <a id="h-222"></a>H-222 | A:3682–3691 |
| <a id="h-223"></a>H-223 | A:3692–3701 |
| <a id="h-224"></a>H-224 | A:3702–3711 |
| <a id="h-225"></a>H-225 | A:3712–3723 |
| <a id="h-226"></a>H-226 | A:3724–3735 |
| <a id="h-227"></a>H-227 | A:3736–3747 |
| <a id="h-228"></a>H-228 | A:3748–3757 |
| <a id="h-229"></a>H-229 | A:3758–3767 |
| <a id="h-230"></a>H-230 | A:3768–3777 |
| <a id="h-231"></a>H-231 | A:3778–3787 |
| <a id="h-232"></a>H-232 | A:3788–3799 |
| <a id="h-233"></a>H-233 | A:3800–3810 |
| <a id="h-234"></a>H-234 | A:3811–3820 |
| <a id="h-235"></a>H-235 | A:3821–3830 |
| <a id="h-236"></a>H-236 | A:3831–3841 |
| <a id="h-237"></a>H-237 | A:3842–3852 |
| <a id="h-238"></a>H-238 | A:3853–3862 |
| <a id="h-239"></a>H-239 | A:3863–3872 |
| <a id="h-240"></a>H-240 | A:3873–3882 |
| <a id="h-241"></a>H-241 | A:3883–3893 |
| <a id="h-242"></a>H-242 | A:3894–3903 |
| <a id="h-243"></a>H-243 | A:3904–3913 |
| <a id="h-244"></a>H-244 | A:3914–3923 |
| <a id="h-245"></a>H-245 | A:3924–3935 |
| <a id="h-246"></a>H-246 | A:3936–3945 |
| <a id="h-247"></a>H-247 | A:3946–3955 |
| <a id="h-248"></a>H-248 | A:3956–3965 |
| <a id="h-249"></a>H-249 | A:3966–3975 |
| <a id="h-250"></a>H-250 | A:3976–3985 |
| <a id="h-251"></a>H-251 | A:3986–3995 |
| <a id="h-252"></a>H-252 | A:3996–4005 |
| <a id="h-253"></a>H-253 | A:4006–4017 |
| <a id="h-254"></a>H-254 | A:4018–4039 |
| <a id="h-255"></a>H-255 | A:4040–4049 |
| <a id="h-256"></a>H-256 | A:4050–4059 |
| <a id="h-257"></a>H-257 | A:4060–4072 |
| <a id="h-258"></a>H-258 | A:4073–4084 |
| <a id="h-259"></a>H-259 | A:4085–4094 |
| <a id="h-260"></a>H-260 | A:4095–4106 |
| <a id="h-261"></a>H-261 | A:4107–4122 |
| <a id="h-262"></a>H-262 | A:4123–4134 |
| <a id="h-263"></a>H-263 | A:4135–4146 |
| <a id="h-264"></a>H-264 | A:4147–4158 |
| <a id="h-265"></a>H-265 | A:4159–4170 |
| <a id="h-266"></a>H-266 | A:4171–4180 |
| <a id="h-267"></a>H-267 | A:4181–4191 |
| <a id="h-268"></a>H-268 | A:4192–4201 |
| <a id="h-269"></a>H-269 | A:4202–4211 |
| <a id="h-270"></a>H-270 | A:4212–4223 |
| <a id="h-271"></a>H-271 | A:4224–4233 |
| <a id="h-272"></a>H-272 | A:4234–4245 |
| <a id="h-273"></a>H-273 | A:4246–4255 |
| <a id="h-274"></a>H-274 | A:4256–4265 |
| <a id="h-275"></a>H-275 | A:4266–4277 |
| <a id="h-276"></a>H-276 | A:4278–4287 |
| <a id="h-277"></a>H-277 | A:4288–4297 |
| <a id="h-278"></a>H-278 | A:4298–4307 |
| <a id="h-279"></a>H-279 | A:4308–4317 |
| <a id="h-280"></a>H-280 | A:4318–4327 |
| <a id="h-281"></a>H-281 | A:4328–4337 |
| <a id="h-282"></a>H-282 | A:4338–4347 |
| <a id="h-283"></a>H-283 | A:4348–4357 |
| <a id="h-284"></a>H-284 | A:4358–4367 |
| <a id="h-285"></a>H-285 | A:4368–4377 |
| <a id="h-286"></a>H-286 | A:4378–4391 |
| <a id="h-287"></a>H-287 | A:4392–4401 |
| <a id="h-288"></a>H-288 | A:4402–4411 |
| <a id="h-289"></a>H-289 | A:4412–4421 |
| <a id="h-290"></a>H-290 | A:4422–4431 |
| <a id="h-291"></a>H-291 | A:4432–4441 |
| <a id="h-292"></a>H-292 | A:4442–4451 |
| <a id="h-293"></a>H-293 | A:4452–4463 |
| <a id="h-294"></a>H-294 | A:4464–4473 |
| <a id="h-295"></a>H-295 | A:4474–4487 |
| <a id="h-296"></a>H-296 | A:4488–4499 |
| <a id="h-297"></a>H-297 | A:4500–4511 |
| <a id="h-298"></a>H-298 | A:4512–4521 |
| <a id="h-299"></a>H-299 | A:4522–4533 |
| <a id="h-300"></a>H-300 | A:4534–4543 |
| <a id="h-301"></a>H-301 | A:4544–4553 |
| <a id="h-302"></a>H-302 | A:4554–4569 |
| <a id="h-303"></a>H-303 | A:4570–4579 |
| <a id="h-304"></a>H-304 | A:4580–4591 |
| <a id="h-305"></a>H-305 | A:4592–4601 |
| <a id="h-306"></a>H-306 | A:4602–4615 |
| <a id="h-307"></a>H-307 | A:4616–4625 |
| <a id="h-308"></a>H-308 | A:4626–4635 |
| <a id="h-309"></a>H-309 | A:4636–4647 |
| <a id="h-310"></a>H-310 | A:4648–4659 |
| <a id="h-311"></a>H-311 | A:4660–4671 |
| <a id="h-312"></a>H-312 | A:4672–4681 |
| <a id="h-313"></a>H-313 | A:4682–4691 |
| <a id="h-314"></a>H-314 | A:4692–4701 |
| <a id="h-315"></a>H-315 | A:4702–4711 |
| <a id="h-316"></a>H-316 | A:4712–4721 |
| <a id="h-317"></a>H-317 | A:4722–4731 |
| <a id="h-318"></a>H-318 | A:4732–4743 |
| <a id="h-319"></a>H-319 | A:4744–4759 |
| <a id="h-320"></a>H-320 | A:4760–4769 |
| <a id="h-321"></a>H-321 | A:4770–4779 |
| <a id="h-322"></a>H-322 | A:4780–4791 |
| <a id="h-323"></a>H-323 | A:4792–4799 |
| <a id="h-324"></a>H-324 | A:4800–4809 |
| <a id="h-325"></a>H-325 | A:4810–4817 |
| <a id="h-326"></a>H-326 | A:4818–4825 |
| <a id="h-327"></a>H-327 | A:4826–4837 |
| <a id="h-328"></a>H-328 | A:4838–4845 |
| <a id="h-329"></a>H-329 | A:4846–4865 |
| <a id="h-330"></a>H-330 | A:4866–4873 |
| <a id="h-331"></a>H-331 | A:4874–4881 |
| <a id="h-332"></a>H-332 | A:4882–4889 |
| <a id="h-333"></a>H-333 | A:4890–4897 |
| <a id="h-334"></a>H-334 | A:4898–4905 |
| <a id="h-335"></a>H-335 | A:4906–4913 |
| <a id="h-336"></a>H-336 | A:4914–4921 |
| <a id="h-337"></a>H-337 | A:4922–4929 |
| <a id="h-338"></a>H-338 | A:4930–4940 |
| <a id="h-339"></a>H-339 | A:4941–4949 |
| <a id="h-340"></a>H-340 | A:4950–4958 |
| <a id="h-341"></a>H-341 | A:4959–4966 |
| <a id="h-342"></a>H-342 | A:4967–4974 |
| <a id="h-343"></a>H-343 | A:4975–4982 |
| <a id="h-344"></a>H-344 | A:4983–4990 |
| <a id="h-345"></a>H-345 | A:4991–4998 |
| <a id="h-346"></a>H-346 | A:4999–5008 |
| <a id="h-347"></a>H-347 | A:5009–5018 |
| <a id="h-348"></a>H-348 | A:5019–5026 |
| <a id="h-349"></a>H-349 | A:5027–5034 |
| <a id="h-350"></a>H-350 | A:5035–5042 |
| <a id="h-351"></a>H-351 | A:5043–5052 |
| <a id="h-352"></a>H-352 | A:5053–5072 |
| <a id="h-353"></a>H-353 | A:5073–5090 |
| <a id="h-354"></a>H-354 | A:5091–5100 |
| <a id="h-355"></a>H-355 | A:5101–5112 |
| <a id="h-356"></a>H-356 | A:5113–5122 |
| <a id="h-357"></a>H-357 | A:5123–5134 |
| <a id="h-358"></a>H-358 | A:5135–5150 |
| <a id="h-359"></a>H-359 | A:5151–5160 |
| <a id="h-360"></a>H-360 | A:5161–5174 |
| <a id="h-361"></a>H-361 | A:5175–5184 |
| <a id="h-362"></a>H-362 | A:5185–5198 |
| <a id="h-363"></a>H-363 | A:5199–5212 |
| <a id="h-364"></a>H-364 | A:5213–5222 |
| <a id="h-365"></a>H-365 | A:5223–5259 |
| <a id="h-366"></a>H-366 | A:5260–5272 |
| <a id="h-367"></a>H-367 | A:5273–5281 |
| <a id="h-368"></a>H-368 | A:5282–5290 |
| <a id="h-369"></a>H-369 | A:5291–5303 |
| <a id="h-370"></a>H-370 | A:5304–5318 |
| <a id="h-371"></a>H-371 | A:5319–5327 |
| <a id="h-372"></a>H-372 | A:5328–5342 |
| <a id="h-373"></a>H-373 | A:5343–5355 |
| <a id="h-374"></a>H-374 | A:5356–5364 |
| <a id="h-375"></a>H-375 | A:5365–5373 |
| <a id="h-376"></a>H-376 | A:5374–5382 |
| <a id="h-377"></a>H-377 | A:5383–5403 |
| <a id="h-378"></a>H-378 | A:5404–5414 |
| <a id="h-379"></a>H-379 | A:5415–5423 |
| <a id="h-380"></a>H-380 | A:5424–5438 |
| <a id="h-381"></a>H-381 | A:5439–5449 |
| <a id="h-382"></a>H-382 | A:5450–5462 |
| <a id="h-383"></a>H-383 | A:5463–5481 |
| <a id="h-384"></a>H-384 | A:5482–5496 |
| <a id="h-385"></a>H-385 | A:5497–5509 |
| <a id="h-386"></a>H-386 | A:5510–5532 |
| <a id="h-387"></a>H-387 | A:5533–5543 |
| <a id="h-388"></a>H-388 | B:1611–1623 |
| <a id="h-389"></a>H-389 | B:1625–1630 |

### H-390 — External inputs instantiate known ADTs without extending fused semantics
<a id="h-390"></a>

- Human/source: current project user in the external-data-boundary side conversation; recorded on client date 2026-11-13 (America/Los_Angeles). Personal identity, exact message timestamp and durable message URL are unavailable. The user quotes D-25r5's unresolved external-definition sentence and explicitly instructs correction and follow-up assessment:

> 可以确定外部数据只能引入静态已知 ADT 的实际取值，比如 sum type 的真实类型等，不得修改 fused world 确定的 type level 语义集合。 这点你看对整改整个文档结论影响大不大？ 相应更正，然后看没有立即后续工作需要

- Accepted scope: P-362/C-85 and D-25r6's external-value boundary. Values may select known sum alternatives and source-unobserved combinations, but incoming data cannot introduce or change the fused world's type-level semantics. This resolves D-25r5's open new-definition question within H-387's selected-composition boundary; H-387's original evidence remains intact.
- Exclusions: no whole-file approval, runtime selection, native-timing change, detailed value-path confirmation, input encoding/validation/failure mechanism, separate composition/publication protocol, new experiment execution or production implementation. Architectural impact and next-stage priorities remain agent assessments, not part of the human confirmation.

### H-391 — Audit-oriented authority and lineage granularity
<a id="h-391"></a>

Human/source: current project user in this hygiene chat; personal identity, exact message timestamp and durable message URL unavailable. Recorded on client date 2026-11-13 (America/Los_Angeles).

> 整理授权记录、价值沿袭图谱，以你自主审计把关的最优粒度和篇幅为目标，优化精简。

Scope: choose practical granularity/length for autonomous audit; consolidate authority and lineage records under H-388/H-05. This authorizes maintenance, not product requirement, confirmation-standard or charter changes.

### H-392 — Remove JavaScript-related exploration material and reassess conclusions
<a id="h-392"></a>

Human/source: current project user in this architecture side conversation, recorded 2026-11-13 (America/Los_Angeles); exact message time, personal identity and durable message URL unavailable.

> 剔除所有js相关的物料，调整分析结论

Scope: remove the JavaScript/WebAssembly model and associated active exploration material, and revise D-25 conclusions to use the retained evidence. Independent LLVM and Java/native regional material is outside that removal. This follows the user's concern about an unacceptable performance class; it does not confirm a numerical threshold, engine selection, proposed native-first study, production timing change or whole-file qualification. Historical human evidence remains recoverable in Git.

### H-393 — Qualitative local-dispatch exploration and mainline handoff
<a id="h-393"></a>

Human/source: current project user, [architecture side chat](codex://threads/01a1d489-c701-7200-be68-e40d199e8dc8), authorized handoff received in the active mainline. Recorded 2026-11-13 America/Los_Angeles; exact message times and personal identity unavailable. Complete excerpts and context are in the external [main package revision 6](../../.codex-handoffs/cod-project/2026-11-12-architecture-exploration/WORK_PACKAGE.md) and [planning package revision 3](../../.codex-handoffs/cod-project/2026-11-12-architecture-exploration/COD_BOOTER_WORKLOAD_PACKAGE.md).

> “native fallback” 确实完全不应假设存在，当前是抛弃已经实现的东西，在完全未实现的前提下去推演评估，没有fallback。
>
> 不做穷举矩阵采样，此工作包只做定型评估判断，不要求定量测量性能数据。
>
> 48 小时只是当前检查点配额，耗尽时不认为是失败，只是停下跟我汇报并评估而已
>
> “realistic opaque-handle, LLVM-binding and `defer` workloads.” 不是授权边界，只是示例，主线智能体按这个原则自主把握边界。
>
> 交给主线程

Accepted scope: execute both packages in the mainline, independent compiler-domain planning first; investigate sub-callable local dispatch/Static BBV through engineering evidence and necessary small prototypes. Prefer mechanism reuse/reimplementation. No existing checker, IR, backend or fallback is assumed. Experimental representations, heuristics and modest language support that improve workload realism are autonomous choices. H-390's fixed semantic set, H-392's exclusion and confirmed world/effect/owner/completion/relocation constraints remain. Preserve future profiling compatibility without implementing profiling. The shared 48-hour active-work allowance is a report-and-assess checkpoint, not a deadline, failure condition or automatic renewal.

Supersedes D-25r7/K-581's proposed shared-native/optimized-native comparison as the current work plan. Excludes exhaustive matrix/sampling, required performance measurement, a comparison framework, self-hosting, production ABI/architecture qualification and toolchain/configuration authority. No existing history or concurrent work may be discarded. P-359/P-360 remain pending whole-file review; no final runtime/timing decision follows.

### H-394 — Focus the domain workload on abstract-family dispatch
<a id="h-394"></a>

Human/source: current project user in [the focus side chat](codex://threads/01a1d506-9661-7f90-8c1c-addac6fbfdab); user-authorized handoff delivered to the active mainline. Recorded 2026-11-13 America/Los_Angeles; precise message time and personal identity unavailable. The handoff reports that the user reviewed the distinction between abstract applicability and sum alternatives, then explicitly instructed:

> 对的，通知主线程转这个聚焦方向

Accepted scope: make abstractly typed arguments with several concrete implementations, overlapping/crossed method specializations and unique-most-specific selection central. Include explicit intersections or ambiguity rejection, multiple explicit concrete conformances (not multiple abstract parents), actual-type evidence through abstract calls/results/storage/TLS, separate family interpretations and captured/current state, and larger operations with different local argument dependencies. TypeTerm structural matching remains useful supporting work but is insufficient main dispatch evidence: different Atom names share one concrete representation. Illustrative compiler type names/signatures are examples, not permanent schema choices. H-393's autonomy, qualitative scope, fixed semantic set, operational limits, whole-file review and shared 48-hour checkpoint remain unchanged.

### H-395 — Give implicit callable context explicit architectural weight
<a id="h-395"></a>

Human/source: current project user in [the context side chat](codex://threads/01a1d514-4e11-77a3-9ad4-053b1b6c975e), user-authorized handoff delivered to the active mainline. Recorded 2026-11-13 America/Los_Angeles; precise message timestamp and personal identity unavailable. The user's proposal and delivery instruction were:

> TLS 认为是所有 callable 的隐含参数这个心智模型是否值得提一下？ 当然 body 没有实际用到的话也就完全可以当没有这个参数
>
> 通知主线程重视这个看法

Accepted scope: explicitly consider TLS as a callable's implicit required context, with no such argument when the dependency is absent. The instruction emphasizes the architectural mental model during H-393's exploration; it does not select a physical ABI, approve arbitrary transitive-effect analysis, change function/procedure rules, or make the whole TLS world a specialization key. D-25r10's transitive/touch/deferred projections and experimental contract analysis are agent elaborations to be assessed in the whole-file package, constrained by existing D-10/D-15/D-17/D-18 semantics. No production mechanism or value path gains qualification.

### H-396 — AOT micro-dispatch replacement, discriminator principle and conditional merge
<a id="h-396"></a>

Human/source: current project user in [the micro-dispatch side chat](codex://threads/01a1d51f-3ef4-7101-a86c-94666675f5b6), user-authorized package/handoffs received by the active mainline. Recorded 2026-11-13 America/Los_Angeles; precise message times and personal identity unavailable. The external [work package revision 4](../../.codex-handoffs/cod-project/2026-11-13-aot-micro-dispatch/WORK_PACKAGE.md) retains prior proposals and the latest controlling clarification. Relevant actual statements:

> ORC 可以留作未来 tracing JIT hot path 的选项。 你准备一个新的工作包，推演验证 AOT micro-dispatch 确实没有主要的坑，先用独立分支推进，确实成立的话合回main分支。 通知主线程将已完成的工作包提交到另外的独立分支，保持不做最后决定，然后基于之前main分支开新分支推进新工作包
>
> 新工作包包含把之前的 multiple-dispatch 设计改为 micro-dispatch
>
> 不是 per callable 而是 per method 吧，然后 method selection 可以使用 type 的 checksum 完成，声明的参数类型可以是 abstract type，abstract type 也计算自己的 checksum，只是不作为对象的rtti。
>
> method selection 时是不是使用 CoD type identity 更合适？ 简单有效
>
> “distinguish two semantically different types” 有真实需求吗？ 我比较怀疑
>
> CoD静态编译 `apply(Debit) ` 应该可以提供静态hint信息，参与 method selection，这个标准属于叫什么我不确定，但是看起来非常合理可行
>
> 调用位置，不是定义位置
>
> 哦，是不是可以立一个原则：类似 sum type 的运行时 discriminator ，给一个 abstract type 的所有可能的 concrete type 也都分配 discriminator，在运行时由这个 abstract type 类型的value携带为rtti？ CoD type identity 够短的话，就直接用作这种 discriminator 好了
>
> 很好。 然后 type checksum 就规划专门用来关联 root types，保存进DBMR，用来校验当前程序的 fused world 类型版本是否与之前保存 DBMR 的程序兼容，当前可能还需要更多信息，这不是唯一判据。

Accepted task scope: preserve the previous package independently without final selection; start from prior main; investigate/implement shared native bodies per method with AOT selection and local operation dispatch; actually replace D-21 and align the architecture/registry/dependent references. No existing checker/backend/fallback or BBV prerequisite. Optional easy specialization is not required for correctness. ORC is future tracing-JIT only. Conditional merge into main is authorized when the explicitly delivered scope is supported, focused validation passes and no unresolved major obstacle defeats that scope; otherwise retain the branch and report obstacles. This does not authorize bootstrap, unrelated tests, a fresh 48-hour checkpoint or subagents.

Latest confirmed principle: abstract/polymorphic values carry their actual concrete-type discriminator, reusing CoD identity if suitable or an explicitly mapped compact encoding. Proven call-site facts can narrow/eliminate selection without cloning method bodies. Preserve only observable distinctions; no second runtime identity tag or universal object header is required. This supersedes the earlier fingerprint-as-runtime-RTTI proposal for the current task. Type checksums are planned root-associated DBMR metadata and one compatibility input, not runtime dispatch tags or a sole compatibility predicate. No production file-format/loader/migration change is requested.

Qualification/exclusions: H-378's actual-type/selected-family/AOT behavioral path continues. C-86/C-87/C-88 and P-363–P-370 record the requested direction and candidate value/dependency paths; detailed production derivations/ABI, full generic closure, physical discriminator mapping and compatibility protocol remain pending whole-file review. Conditional merge is scoped delivery authorization, not human approval of every agent mechanism. Preserve nominal conformance, owner/access, captures/context, ordered completion, initialization operand effects, regional relocation and D-16 identity/UoM constraints. H-393–H-395 and K-582–K-584 retain their archival scopes at `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8`; they do not select the older recommendation for this branch.

### H-397 — Persisted discriminator meaning across compatible fused worlds
<a id="h-397"></a>

Human/source: current project user in [the same micro-dispatch side chat](codex://threads/01a1d51f-3ef4-7101-a86c-94666675f5b6); revision 5 of the external package and its authorized handoff reached the mainline before final validation/merge. Recorded 2026-11-13 America/Los_Angeles; exact timestamp/personal identity unavailable. The explicit correction is:

> 不能只考虑 CoD 单次构建的 fused world，DBMR肯定要允许被新版本的CoD程序以兼容方式读取，discriminator作为region内部载荷，要考虑跨 fused world 的可对比性，这点要守住，但是也不要为此过度工程。

Accepted scope refines H-396/C-87: stored discriminators preserve concrete meaning/comparability across compatible writer/reader fused worlds, including unrelated additions and discovery-order changes. A naked per-build ordinal and same-executable-only evidence do not satisfy this boundary. Prefer the smallest stable identity mechanism; any required context is durable and explicit. Do not rewrite tags or reconstruct the graph to fit a reader. No central registry service, universal migration or broad remapping framework is requested. Require a focused separately prepared older-writer/newer-reader observation over unchanged bytes and rejection of invalid interpretation before conditional merge. Keep tag overhead small; existing category-local Type.identity is not assumed universally persistent.

The full compatibility predicate, permanent ID allocation/ABI and DBMR loader/header/migration remain outside the implemented scope. Root checksums stay separate planned compatibility metadata and cannot repair an incorrectly interpreted discriminator. Experimental stable definition IDs and a fixture-root binding catalog are agent mechanisms for this authorized observation, not a new human-approved permanent schema. H-396's semantic, branch, checkpoint and build constraints remain; its historical statements are retained.

### H-398 — Source compiler migration and preserved baseline
<a id="h-398"></a>

Human/source: current project user in [the source-migration side chat](codex://threads/01a1d579-bd41-7bc0-a3de-219c64b37b45), through an explicitly authorized handoff received in the active mainline. Recorded 2026-11-13 America/Los_Angeles; exact human message time and personal identity unavailable. The handoff quotes:

> 感觉可以吧当前位置打个 multi-dispatch tag，然后继续推进。你觉得合适的适合通知主线程设准确goal自主推进。

The authorized task preserves exact baseline 2ab762d617b742f4fba7c2c617bacd303739ccef with a local annotated multi-dispatch tag, then advances the source compiler toward shared-method AOT execution with real source-to-native acceptance and coherent retirement of displaced specialization machinery. The tag annotation distinguishes existing source compiler code from the micro-dispatch design/isolated experiment already included there. No remote tag push, new budget, bootstrap or subagent authorization is supplied.

The side assistant recommends reviewing generic/recursive closure before fixing checked-method/value/emitter interfaces, and developing stronger proofs alongside source slices. That ordering is adopted as the agent work plan, not recorded as a newly human-approved generic rule. The user has not approved a specific admission/termination strategy or permanent ABI. D-26r1/C-89/P-371–P-373 therefore remain pending whole-file review. Preserve H-378/H-396/H-397 semantics, owners, captures, selected interpretations, effects/completion, D-16 compatibility and persisted discriminator meaning. Existing design qualification and operational constraints remain.

### H-399 — Remove architectural debt before restoring capability
<a id="h-399"></a>

Human/source: current project user in [the source-migration side chat](codex://threads/01a1d579-bd41-7bc0-a3de-219c64b37b45), explicit authorized follow-up received by the active mainline on 2026-11-13 America/Los_Angeles. Precise human message time and personal identity unavailable. Actual statement:

> 造成技术债务负担的历史实现，应该先砍掉，从清爽的基础上补缺失的部分，当前整改阶段不要求功能完整、测试全过

This supersedes the side assistant’s incremental-retirement recommendation and any current plan retaining old paths solely for coverage. Remove debt-producing historical checking/emission/context-specialization machinery first, then fill capability gaps on a clean coherent foundation. Current stage completion does not require feature completeness or all legacy tests passing. Temporary feature loss and known failing legacy cases are authorized; no repeated removal permission is required. The local multi-dispatch tag is the recovery point.

Preserve confirmed language contracts as the target and report gaps/results honestly. Do not weaken tests, silently convert missing capability into a language restriction, retain compatibility shims/parallel semantics/hidden fallback, or violate owners/effects/cleanup/regional invariants on supported paths. This is a stage-ordering/acceptance decision, not approval of new generic rules or permanent identity/ABI. Existing build/test/bootstrap permissions remain unchanged. D-26’s proposed closure contract remains pending-human.

### H-400 — CoD type-object boundary for unknown runtime types
<a id="h-400"></a>

Human/source: current project user in [the generic/recursive closure side discussion](codex://threads/01a1d58e-c14b-7ac0-a5c0-d190fe30d9f4). Recorded 2026-11-13 America/Los_Angeles; exact message times and personal identity unavailable. This and H-401–H-403 are decision paraphrases, not quotations, as requested by H-403.

Accepted scope: D-26r3's CoD `<any>` type-object direction. Register an identified type object in the fused world for generic selection; known concrete types, including distinct callable result types, retain their own identities. A runtime type absent from the static catalog uses the `<any>` selection path. The known possible alternatives are the full static catalog, narrowed by ordinary applicability and proven facts; the catalog is not the entire runtime universe. When a required interpretation of an outside type is unavailable, raise checked StaticallyUnknownType and statically require the corresponding checked handling. The exception name is provisionally selected for implementation. Do not import Julia Any semantics.

This replaces H-390/P-362's categorical known-ADT-only input restriction in the affected D-15/D-21/D-25 scope. It does not authorize new executable method loading, runtime compilation, arbitrary unsafe input admission, or a permanent identity/ABI. Exact opaque-value operations, validation timing and complete checked-effect integration remain open. C-90's behavior is locally confirmed; P-374/P-375 and D-26's whole-file proof contract remain pending-human.

### H-401 — Same-T preservation and unchecked typing backstop
<a id="h-401"></a>

Human/source: current project user in the H-400 side discussion; recorded 2026-11-13 America/Los_Angeles, exact message times unavailable. Accepted final decision: retain same-T, diagnose statically established type mismatches, and use runtime checking where needed to detect a breach of the established typing contract. Actual type evidence must be checked; two types selecting through `<any>` do not thereby become equal.

The provisional failure name is UnsoundRuntimeTyping. Its final classification is **unchecked**, superseding the tentative checked classification discussed before the user's clarification. This soundness backstop must not impose mandatory catch/throws obligations across generic code. It does not turn a statically invalid call into ordinary accepted input or remove required checks and cleanup. StaticallyUnknownType retains H-400's distinct checked role. D-26r3/C-90 records this decision; D-07/D-17/D-18/D-21 integration and concrete guard placement remain subject to their contracts. No dynamic-cast API or universal runtime validation policy is selected.

### H-402 — Permissive generic operations; library contracts deferred
<a id="h-402"></a>

Human/source: current project user in the H-400 side discussion; recorded 2026-11-13 America/Los_Angeles, exact message times unavailable. Accepted scope: allow generic bodies to use operations such as `x + x`, subject to ordinary method applicability and result/type checking. This does not implicitly define `(+)` for arbitrary types or select an additional capability mechanism.

Progressively enrich type/operator contracts so their requirements and definitions can support documentation and IDE navigation. Supply such contracts as ordinary CoD library definitions, with no new intrinsic mechanism for the contracts. This work is deferred; it is not a current migration prerequisite. The template analogy does not select mandatory native instantiation, concrete declaration syntax, bodyless-method semantics, numeric conversions, fraction behavior or an IDE implementation. D-26r3 records the direction and exclusion; D-07/D-11/D-16/D-21 remain dependencies.

### H-403 — Ongoing documentation with decision summaries
<a id="h-403"></a>

Human/source: current project user in the H-400 side discussion; recorded 2026-11-13 America/Los_Angeles, exact message times unavailable. The user requests prompt workspace documentation throughout topic review to retain decisions and unresolved details, and directs the authority record to summarize agreed meaning rather than quote informal side-chat wording. Preserve attribution, scope, exclusions and distinction between human decisions and agent proposals. This authorizes the current discussion-document/registry update, not whole-file design approval, a main-thread message, code execution or broader task mutation. Existing historical records remain intact.

### H-404 — Unknown-type failure at the required interpretation
<a id="h-404"></a>

Human/source: current project user in [the generic/recursive closure side discussion](codex://threads/01a1d58e-c14b-7ac0-a5c0-d190fe30d9f4), recorded 2026-11-13 America/Los_Angeles; exact message time unavailable. Decision paraphrase under H-403: the user accepts allowing unknown types through operations that need not interpret their contents. Given a valid borrow/carrier, a generic marker returning a constant succeeds for a type outside the static catalog. An operation such as `(+)` that requires an unavailable interpretation raises checked StaticallyUnknownType at that operation. Mere receipt/forwarding creates no unknown-type catch obligation; actual identity and owner evidence remain preserved.

Scope: D-26r4/C-90's operation boundary resolves that part of H-400's open timing question. Callee failures still propagate through forwarding calls; reference validity, lifetime, access and input validation remain required. Specific carrier/operation protocols, interprocedural summaries, ABI and the whole native proof contract remain open. This does not change H-401's unchecked soundness classification or qualify P-374/P-375's pending value path.

### H-405 — Generic body does not require an unknown-type throws declaration
<a id="h-405"></a>

Human/source: current project user in [the generic/recursive closure side discussion](codex://threads/01a1d58e-c14b-7ac0-a5c0-d190fe30d9f4), recorded 2026-11-13 America/Los_Angeles; exact message time unavailable. Decision paraphrase under H-403: the compiler does not require the shown generic `twice(x: T)` body to declare StaticallyUnknownType. A programmer may declare it voluntarily and then assumes the existing declared-exception obligations. Generic syntax alone must not manufacture a checked declaration or a catch obligation for a known supported call.

This corrects the assistant's premise behind the fixed-versus-conditional contract question recorded as open in D-26r4/K-590. No conditional throws mechanism or waiver of an explicit declaration is selected. H-400/H-404's checked failure at genuine unavailable-interpretation operations and H-401's unchecked typing backstop remain. D-26r5 records this distinction; actual analysis of unknown-type paths across calls remains work, not approval of automatic inference or a blanket change to D-17's other checked exceptions.

### H-406 — Detectable resource exhaustion and analysis termination
<a id="h-406"></a>

Human/source: current project user in [the generic/recursive closure side discussion](codex://threads/01a1d58e-c14b-7ac0-a5c0-d190fe30d9f4), recorded 2026-11-13 America/Los_Angeles; exact message time unavailable. Decision paraphrase under H-403: resource exhaustion can be detected, reported and used to stop compilation. The user challenges the remaining general noncompletion-detection premise as a halting-problem issue, rather than approving the proposed catch-all unproved classification.

D-26r6 records the resource-stop decision and removes reliance on a universal detector that arbitrary analysis will never finish. Its distinction between known implementation gaps and undecidability is an agent clarification under existing H-399, not an additional human-selected algorithm. No particular analysis, resource quota, universal recursive acceptance rule or whole-file qualification follows.

### H-407 — Conditional whole-file approval after input-boundary correction and review
<a id="h-407"></a>

Human/source: current project user in [the document-review side conversation](codex://threads/01a1d5e7-cab2-71a2-81e1-ab3becc2154f), recorded 2026-11-13 America/Los_Angeles; exact message time unavailable. Decision paraphrase under H-403: correct the known-ADT-only wording immediately, review the pending document comprehensively, and, if no outstanding pitfall prevents approval, notify the main thread that it is approved. This is conditional human approval, not an unconditional approval of additional mechanisms.

The review of D-26r6 found no approval-blocking contradiction within its stated behavioral/native-coverage scope and exclusions. That satisfies the user's condition for whole-file approval: C-89/C-90 and P-371/P-372/P-374/P-375, the finite native-coverage responsibility, concrete diagnostics, invariants and acceptance are confirmed. P-373/P-376 retains the dependency constraints without qualifying unresolved parent contracts. D-15/D-21/D-25's obsolete input restrictions are reconciled to H-400–H-406; D-26r7 records qualification without selecting new semantics. K-593 records the agent's audit and continuity assessment separately.

Concrete checking/recursive-analysis algorithms, real failing-path analysis across calls, checked representation, physical carrier/ABI, persistent encoding, input-validation protocols, D-24 result inference and deferred library contracts remain separately reviewed or deferred as the document states. Approval is not implementation-completion evidence, feature parity, authorization for toolchain changes, or an instruction to resume a paused goal.

### H-408 — Practical progress governs the stage goal
<a id="h-408"></a>

Human/source: current project user in [the document-review side conversation](codex://threads/01a1d5e7-cab2-71a2-81e1-ab3becc2154f), recorded 2026-11-13 America/Los_Angeles; exact message time unavailable. Decision paraphrase under H-403: question whether the current goal overvalues perfection and makes the main line overly cautious; effective engineering progress should take precedence over perfect results.

This reinforces H-399's usable foundation and documented capability-gap policy. It does not relax correctness for claimed supported behavior or settle an unresolved language boundary. A bounded outcome goal is the agent's recommendation for main-thread adoption, not an already changed goal or a user command to resume its paused state.

### H-409 — Semantic method hierarchy, call-site specialization and local micro-dispatch
<a id="h-409"></a>

Human/source: current project user through the authorized [conceptual-clarification side conversation](codex://threads/01a1d612-0889-7fe2-87f3-f4f1ff0772f5), received 2026-11-13 America/Los_Angeles; exact human message times and personal identity unavailable. Attributed decision summary under H-403: document these clarifications before continuing implementation.

- A callable family can contain multiple user-defined methods. Each method has one semantic definition/body and may have multiple compiler-statically-registered specializations. One semantic body does not mean exactly one emitted native body.
- Proven call-site information for a straightforward call such as twice(3u64) should demand/generate/reuse an appropriate AOT specialization and statically select/direct-call it when the semantic method and required interpretation are established. General-path correctness is not a policy forcing known calls through runtime discrimination. Specialization is not a correctness prerequisite or a requirement to clone every combination.
- Distinguish phase and initiator: user definition code constructs/registers semantic definitions in its definition/compilation stage; compiler processing of a call site drives checking and native specialization/preparation; runtime user calls invoke prepared implementations. Compiler processing of a body does not execute its runtime effects. Runtime installation/capture changes complete bindings, not method/specialization registration. Explicit user compile-time computation remains a separate activity.
- Micro-dispatch is local operation-selection code within a generated body. Independent x access, y access and a joint operation can have separate prepared selections, with branches returning to shared control flow. A local runtime branch creates/registers no specialization. A general or partially specialized body can contain many such points; AOT specialization may resolve some/all without requiring a Cartesian product of whole-body versions.

Semantic method selection and selection of an implementation of that method remain separate. A compiler specialization of generic f(T) for u64 is not a user overload and cannot override or compete with user-defined f(u64). Complete callable family/interpretation/captures and actual owners remain. Runtime owner instances/capture values are not automatically specialization keys. This clarifies C-86 and the D-21/D-26 coverage contract while preserving H-400–H-407 runtime-type, failure and checking requirements.

The side assistant's suggestion to represent a general implementation as a specialization with no extra narrowing assumptions is an engineering proposal, not a human-selected permanent representation. No physical carrier/ABI, specialization budget/key algorithm, runtime compilation or new method loading is approved. H-399/H-408 practical stage acceptance remains. The app goal was paused at receipt; the handoff explicitly instructs documentation followed by implementation, without changing the app-controlled goal record.

### H-410 — AOT prepares selections; runtime executes unresolved choices
<a id="h-410"></a>

Human/source: current project user in [this documentation side conversation](codex://threads/01a1d635-1576-70a1-8153-0e956ce19903), recorded 2026-11-13 America/Los_Angeles; exact message times and personal identity unavailable. Attributed decision summary under H-403: the document should fully express that AOT occurs during compilation and prepares micro-dispatch rather than executing an actual selection. After the assistant proposed the filename `AheadOfTimeExecutionPreparation.md`, the title “Ahead-of-time execution preparation and runtime micro-dispatch,” and consistent phase wording, the user instructed the assistant to apply the documentation changes, commit and push.

Scope: D-21r10→r11 retains its stable identity and qualified behavior. AOT checks and prepares native implementations, method selectors and local micro-dispatch paths. Proven facts may resolve method/implementation/local-operation choices and permit direct-call emission; runtime executes emitted calls and any remaining prepared selections using actual evidence. Preparing a selector does not execute it or the source body’s runtime effects. Semantic method selection and local operation selection remain distinct. D-25/D-26 summaries and current document links follow this clarification.

This approves the identified name and phase clarification and its documentation delivery, not a new mechanism or blanket whole-file qualification. H-378/H-407/H-409 baselines, current candidate value paths, owner/capture/context/completion constraints and pending ABI, analysis and other production choices retain their scopes. No source implementation, build/toolchain change, goal-state change or merge to main is requested.

### Agent continuity evidence

| Event | Source: lines |
| --- | --- |
| <a id="k-00"></a>K-00 | A:5546–5557 |
| <a id="k-01"></a>K-01 | A:5558–5567 |
| <a id="k-02"></a>K-02 | A:5568–5577 |
| <a id="k-03"></a>K-03 | A:5578–5588 |
| <a id="k-04"></a>K-04 | A:5589–5598 |
| <a id="k-05"></a>K-05 | A:5599–5607 |
| <a id="k-06"></a>K-06 | A:5608–5615 |
| <a id="k-07"></a>K-07 | A:5616–5624 |
| <a id="k-08"></a>K-08 | A:5625–5633 |
| <a id="k-09"></a>K-09 | A:5634–5643 |
| <a id="k-10"></a>K-10 | A:5644–5652 |
| <a id="k-11"></a>K-11 | A:5653–5661 |
| <a id="k-12"></a>K-12 | A:5662–5670 |
| <a id="k-13"></a>K-13 | A:5671–5680 |
| <a id="k-14"></a>K-14 | A:5681–5689 |
| <a id="k-15"></a>K-15 | A:5690–5696 |
| <a id="k-16"></a>K-16 | A:5697–5705 |
| <a id="k-17"></a>K-17 | A:5706–5715 |
| <a id="k-18"></a>K-18 | A:5716–5723 |
| <a id="k-19"></a>K-19 | A:5724–5730 |
| <a id="k-20"></a>K-20 | A:5731–5737 |
| <a id="k-21"></a>K-21 | A:5738–5744 |
| <a id="k-22"></a>K-22 | A:5745–5752 |
| <a id="k-23"></a>K-23 | A:5753–5761 |
| <a id="k-24"></a>K-24 | A:5762–5770 |
| <a id="k-25"></a>K-25 | A:5771–5779 |
| <a id="k-26"></a>K-26 | A:5780–5788 |
| <a id="k-27"></a>K-27 | A:5789–5798 |
| <a id="k-28"></a>K-28 | A:5799–5810 |
| <a id="k-29"></a>K-29 | A:5811–5816 |
| <a id="k-30"></a>K-30 | A:5817–5827 |
| <a id="k-31"></a>K-31 | A:5828–5839 |
| <a id="k-32"></a>K-32 | A:5840–5851 |
| <a id="k-33"></a>K-33 | A:5852–5862 |
| <a id="k-34"></a>K-34 | A:5863–5876 |
| <a id="k-35"></a>K-35 | A:5877–5890 |
| <a id="k-36"></a>K-36 | A:5891–5903 |
| <a id="k-37"></a>K-37 | A:5904–5916 |
| <a id="k-38"></a>K-38 | A:5917–5929 |
| <a id="k-39"></a>K-39 | A:5930–5942 |
| <a id="k-40"></a>K-40 | A:5943–5955 |
| <a id="k-41"></a>K-41 | A:5956–5968 |
| <a id="k-42"></a>K-42 | A:5969–5981 |
| <a id="k-43"></a>K-43 | A:5982–5994 |
| <a id="k-44"></a>K-44 | A:5995–6007 |
| <a id="k-45"></a>K-45 | A:6008–6020 |
| <a id="k-46"></a>K-46 | A:6021–6033 |
| <a id="k-47"></a>K-47 | A:6034–6046 |
| <a id="k-48"></a>K-48 | A:6047–6059 |
| <a id="k-49"></a>K-49 | A:6060–6072 |
| <a id="k-50"></a>K-50 | A:6073–6085 |
| <a id="k-51"></a>K-51 | A:6086–6098 |
| <a id="k-52"></a>K-52 | A:6099–6111 |
| <a id="k-53"></a>K-53 | A:6112–6124 |
| <a id="k-54"></a>K-54 | A:6125–6137 |
| <a id="k-55"></a>K-55 | A:6138–6150 |
| <a id="k-56"></a>K-56 | A:6151–6163 |
| <a id="k-57"></a>K-57 | A:6164–6177 |
| <a id="k-58"></a>K-58 | A:6178–6190 |
| <a id="k-59"></a>K-59 | A:6191–6203 |
| <a id="k-60"></a>K-60 | A:6204–6215 |
| <a id="k-61"></a>K-61 | A:6216–6228 |
| <a id="k-62"></a>K-62 | A:6229–6241 |
| <a id="k-63"></a>K-63 | A:6242–6254 |
| <a id="k-64"></a>K-64 | A:6255–6267 |
| <a id="k-65"></a>K-65 | A:6268–6280 |
| <a id="k-66"></a>K-66 | A:6281–6293 |
| <a id="k-67"></a>K-67 | A:6294–6306 |
| <a id="k-68"></a>K-68 | A:6307–6319 |
| <a id="k-69"></a>K-69 | A:6320–6331 |
| <a id="k-70"></a>K-70 | A:6332–6343 |
| <a id="k-71"></a>K-71 | A:6344–6356 |
| <a id="k-72"></a>K-72 | A:6357–6368 |
| <a id="k-73"></a>K-73 | A:6369–6382 |
| <a id="k-74"></a>K-74 | A:6383–6395 |
| <a id="k-75"></a>K-75 | A:6396–6410 |
| <a id="k-76"></a>K-76 | A:6411–6423 |
| <a id="k-77"></a>K-77 | A:6424–6437 |
| <a id="k-78"></a>K-78 | A:6438–6452 |
| <a id="k-79"></a>K-79 | A:6453–6466 |
| <a id="k-80"></a>K-80 | A:6467–6481 |
| <a id="k-81"></a>K-81 | A:6482–6494 |
| <a id="k-82"></a>K-82 | A:6495–6508 |
| <a id="k-83"></a>K-83 | A:6509–6524 |
| <a id="k-84"></a>K-84 | A:6525–6538 |
| <a id="k-85"></a>K-85 | A:6539–6552 |
| <a id="k-86"></a>K-86 | A:6553–6566 |
| <a id="k-87"></a>K-87 | A:6567–6579 |
| <a id="k-88"></a>K-88 | A:6580–6592 |
| <a id="k-89"></a>K-89 | A:6593–6608 |
| <a id="k-90"></a>K-90 | A:6609–6620 |
| <a id="k-91"></a>K-91 | A:6621–6631 |
| <a id="k-92"></a>K-92 | A:6632–6641 |
| <a id="k-93"></a>K-93 | A:6642–6652 |
| <a id="k-94"></a>K-94 | A:6653–6662 |
| <a id="k-95"></a>K-95 | A:6663–6673 |
| <a id="k-96"></a>K-96 | A:6674–6683 |
| <a id="k-97"></a>K-97 | A:6684–6694 |
| <a id="k-98"></a>K-98 | A:6695–6706 |
| <a id="k-99"></a>K-99 | A:6707–6718 |
| <a id="k-100"></a>K-100 | A:6719–6730 |
| <a id="k-101"></a>K-101 | A:6731–6742 |
| <a id="k-102"></a>K-102 | A:6743–6752 |
| <a id="k-103"></a>K-103 | A:6753–6762 |
| <a id="k-104"></a>K-104 | A:6763–6772 |
| <a id="k-105"></a>K-105 | A:6773–6782 |
| <a id="k-106"></a>K-106 | A:6783–6793 |
| <a id="k-107"></a>K-107 | A:6794–6804 |
| <a id="k-108"></a>K-108 | A:6805–6815 |
| <a id="k-109"></a>K-109 | A:6816–6826 |
| <a id="k-110"></a>K-110 | A:6827–6837 |
| <a id="k-111"></a>K-111 | A:6838–6848 |
| <a id="k-112"></a>K-112 | A:6849–6859 |
| <a id="k-113"></a>K-113 | A:6860–6870 |
| <a id="k-114"></a>K-114 | A:6871–6880 |
| <a id="k-115"></a>K-115 | A:6881–6890 |
| <a id="k-116"></a>K-116 | A:6891–6900 |
| <a id="k-117"></a>K-117 | A:6901–6910 |
| <a id="k-118"></a>K-118 | A:6911–6920 |
| <a id="k-119"></a>K-119 | A:6921–6930 |
| <a id="k-120"></a>K-120 | A:6931–6940 |
| <a id="k-121"></a>K-121 | A:6941–6950 |
| <a id="k-122"></a>K-122 | A:6951–6960 |
| <a id="k-123"></a>K-123 | A:6961–6966 |
| <a id="k-124"></a>K-124 | A:6967–6976 |
| <a id="k-125"></a>K-125 | A:6977–6985 |
| <a id="k-126"></a>K-126 | A:6986–6991 |
| <a id="k-127"></a>K-127 | A:6992–6999 |
| <a id="k-128"></a>K-128 | A:7000–7009 |
| <a id="k-129"></a>K-129 | A:7010–7017 |
| <a id="k-130"></a>K-130 | A:7018–7027 |
| <a id="k-131"></a>K-131 | A:7028–7036 |
| <a id="k-132"></a>K-132 | A:7037–7045 |
| <a id="k-133"></a>K-133 | A:7046–7055 |
| <a id="k-134"></a>K-134 | A:7056–7064 |
| <a id="k-135"></a>K-135 | A:7065–7074 |
| <a id="k-136"></a>K-136 | A:7075–7084 |
| <a id="k-137"></a>K-137 | A:7085–7093 |
| <a id="k-138"></a>K-138 | A:7094–7103 |
| <a id="k-139"></a>K-139 | A:7104–7114 |
| <a id="k-140"></a>K-140 | A:7115–7125 |
| <a id="k-141"></a>K-141 | A:7126–7136 |
| <a id="k-142"></a>K-142 | A:7137–7147 |
| <a id="k-143"></a>K-143 | A:7148–7158 |
| <a id="k-144"></a>K-144 | A:7159–7169 |
| <a id="k-145"></a>K-145 | A:7170–7180 |
| <a id="k-146"></a>K-146 | A:7181–7191 |
| <a id="k-147"></a>K-147 | A:7192–7202 |
| <a id="k-148"></a>K-148 | A:7203–7212 |
| <a id="k-149"></a>K-149 | A:7213–7222 |
| <a id="k-150"></a>K-150 | A:7223–7232 |
| <a id="k-151"></a>K-151 | A:7233–7243 |
| <a id="k-152"></a>K-152 | A:7244–7253 |
| <a id="k-153"></a>K-153 | A:7254–7263 |
| <a id="k-154"></a>K-154 | A:7264–7273 |
| <a id="k-155"></a>K-155 | A:7274–7283 |
| <a id="k-156"></a>K-156 | A:7284–7293 |
| <a id="k-157"></a>K-157 | A:7294–7303 |
| <a id="k-158"></a>K-158 | A:7304–7313 |
| <a id="k-159"></a>K-159 | A:7314–7323 |
| <a id="k-160"></a>K-160 | A:7324–7333 |
| <a id="k-161"></a>K-161 | A:7334–7343 |
| <a id="k-162"></a>K-162 | A:7344–7353 |
| <a id="k-163"></a>K-163 | A:7354–7362 |
| <a id="k-164"></a>K-164 | A:7363–7371 |
| <a id="k-165"></a>K-165 | A:7372–7380 |
| <a id="k-166"></a>K-166 | A:7381–7389 |
| <a id="k-167"></a>K-167 | A:7390–7397 |
| <a id="k-168"></a>K-168 | A:7398–7406 |
| <a id="k-169"></a>K-169 | A:7407–7415 |
| <a id="k-170"></a>K-170 | A:7416–7424 |
| <a id="k-171"></a>K-171 | A:7425–7433 |
| <a id="k-172"></a>K-172 | A:7434–7442 |
| <a id="k-173"></a>K-173 | A:7443–7451 |
| <a id="k-174"></a>K-174 | A:7452–7460 |
| <a id="k-175"></a>K-175 | A:7461–7469 |
| <a id="k-176"></a>K-176 | A:7470–7478 |
| <a id="k-177"></a>K-177 | A:7479–7487 |
| <a id="k-178"></a>K-178 | A:7488–7496 |
| <a id="k-179"></a>K-179 | A:7497–7505 |
| <a id="k-180"></a>K-180 | A:7506–7514 |
| <a id="k-181"></a>K-181 | A:7515–7523 |
| <a id="k-182"></a>K-182 | A:7524–7532 |
| <a id="k-183"></a>K-183 | A:7533–7541 |
| <a id="k-184"></a>K-184 | A:7542–7550 |
| <a id="k-185"></a>K-185 | A:7551–7559 |
| <a id="k-186"></a>K-186 | A:7560–7568 |
| <a id="k-187"></a>K-187 | A:7569–7577 |
| <a id="k-188"></a>K-188 | A:7578–7586 |
| <a id="k-189"></a>K-189 | A:7587–7595 |
| <a id="k-190"></a>K-190 | A:7596–7604 |
| <a id="k-191"></a>K-191 | A:7605–7613 |
| <a id="k-192"></a>K-192 | A:7614–7622 |
| <a id="k-193"></a>K-193 | A:7623–7631 |
| <a id="k-194"></a>K-194 | A:7632–7640 |
| <a id="k-195"></a>K-195 | A:7641–7649 |
| <a id="k-196"></a>K-196 | A:7650–7658 |
| <a id="k-197"></a>K-197 | A:7659–7667 |
| <a id="k-198"></a>K-198 | A:7668–7676 |
| <a id="k-199"></a>K-199 | A:7677–7685 |
| <a id="k-200"></a>K-200 | A:7686–7694 |
| <a id="k-201"></a>K-201 | A:7695–7703 |
| <a id="k-202"></a>K-202 | A:7704–7712 |
| <a id="k-203"></a>K-203 | A:7713–7721 |
| <a id="k-204"></a>K-204 | A:7722–7730 |
| <a id="k-205"></a>K-205 | A:7731–7739 |
| <a id="k-206"></a>K-206 | A:7740–7748 |
| <a id="k-207"></a>K-207 | A:7749–7758 |
| <a id="k-208"></a>K-208 | A:7759–7770 |
| <a id="k-209"></a>K-209 | A:7771–7782 |
| <a id="k-210"></a>K-210 | A:7783–7795 |
| <a id="k-211"></a>K-211 | A:7796–7807 |
| <a id="k-212"></a>K-212 | A:7808–7817 |
| <a id="k-213"></a>K-213 | A:7818–7828 |
| <a id="k-214"></a>K-214 | A:7829–7842 |
| <a id="k-215"></a>K-215 | A:7843–7855 |
| <a id="k-216"></a>K-216 | A:7856–7867 |
| <a id="k-217"></a>K-217 | A:7868–7880 |
| <a id="k-218"></a>K-218 | A:7881–7893 |
| <a id="k-219"></a>K-219 | A:7894–7906 |
| <a id="k-220"></a>K-220 | A:7907–7919 |
| <a id="k-221"></a>K-221 | A:7920–7931 |
| <a id="k-222"></a>K-222 | A:7932–7943 |
| <a id="k-223"></a>K-223 | A:7944–7957 |
| <a id="k-224"></a>K-224 | A:7958–7968 |
| <a id="k-225"></a>K-225 | A:7969–7981 |
| <a id="k-226"></a>K-226 | A:7982–7994 |
| <a id="k-227"></a>K-227 | A:7995–8007 |
| <a id="k-228"></a>K-228 | A:8008–8021 |
| <a id="k-229"></a>K-229 | A:8022–8033 |
| <a id="k-230"></a>K-230 | A:8034–8045 |
| <a id="k-231"></a>K-231 | A:8046–8056 |
| <a id="k-232"></a>K-232 | A:8057–8068 |
| <a id="k-233"></a>K-233 | A:8069–8077 |
| <a id="k-234"></a>K-234 | A:8078–8086 |
| <a id="k-235"></a>K-235 | A:8087–8096 |
| <a id="k-236"></a>K-236 | A:8097–8105 |
| <a id="k-237"></a>K-237 | A:8106–8115 |
| <a id="k-238"></a>K-238 | A:8116–8125 |
| <a id="k-239"></a>K-239 | A:8126–8135 |
| <a id="k-240"></a>K-240 | A:8136–8145 |
| <a id="k-241"></a>K-241 | A:8146–8164 |
| <a id="k-242"></a>K-242 | A:8165–8173 |
| <a id="k-243"></a>K-243 | A:8174–8183 |
| <a id="k-244"></a>K-244 | A:8184–8193 |
| <a id="k-245"></a>K-245 | A:8194–8203 |
| <a id="k-246"></a>K-246 | A:8204–8213 |
| <a id="k-247"></a>K-247 | A:8214–8223 |
| <a id="k-248"></a>K-248 | A:8224–8233 |
| <a id="k-249"></a>K-249 | A:8234–8242 |
| <a id="k-250"></a>K-250 | A:8243–8252 |
| <a id="k-251"></a>K-251 | A:8253–8262 |
| <a id="k-252"></a>K-252 | A:8263–8272 |
| <a id="k-253"></a>K-253 | A:8273–8281 |
| <a id="k-254"></a>K-254 | A:8282–8292 |
| <a id="k-255"></a>K-255 | A:8293–8303 |
| <a id="k-256"></a>K-256 | A:8304–8312 |
| <a id="k-257"></a>K-257 | A:8313–8322 |
| <a id="k-258"></a>K-258 | A:8323–8332 |
| <a id="k-259"></a>K-259 | A:8333–8342 |
| <a id="k-260"></a>K-260 | A:8343–8352 |
| <a id="k-261"></a>K-261 | A:8353–8362 |
| <a id="k-262"></a>K-262 | A:8363–8373 |
| <a id="k-263"></a>K-263 | A:8374–8383 |
| <a id="k-264"></a>K-264 | A:8384–8394 |
| <a id="k-265"></a>K-265 | A:8395–8405 |
| <a id="k-266"></a>K-266 | A:8406–8415 |
| <a id="k-267"></a>K-267 | A:8416–8425 |
| <a id="k-268"></a>K-268 | A:8426–8435 |
| <a id="k-269"></a>K-269 | A:8436–8444 |
| <a id="k-270"></a>K-270 | A:8445–8453 |
| <a id="k-271"></a>K-271 | A:8454–8463 |
| <a id="k-272"></a>K-272 | A:8464–8473 |
| <a id="k-273"></a>K-273 | A:8474–8483 |
| <a id="k-274"></a>K-274 | A:8484–8493 |
| <a id="k-275"></a>K-275 | A:8494–8503 |
| <a id="k-276"></a>K-276 | A:8504–8512 |
| <a id="k-277"></a>K-277 | A:8513–8520 |
| <a id="k-278"></a>K-278 | A:8521–8528 |
| <a id="k-279"></a>K-279 | A:8529–8537 |
| <a id="k-280"></a>K-280 | A:8538–8545 |
| <a id="k-281"></a>K-281 | A:8546–8553 |
| <a id="k-282"></a>K-282 | A:8554–8563 |
| <a id="k-283"></a>K-283 | A:8564–8572 |
| <a id="k-284"></a>K-284 | A:8573–8581 |
| <a id="k-285"></a>K-285 | A:8582–8589 |
| <a id="k-286"></a>K-286 | A:8590–8598 |
| <a id="k-287"></a>K-287 | A:8599–8607 |
| <a id="k-288"></a>K-288 | A:8608–8617 |
| <a id="k-289"></a>K-289 | A:8618–8626 |
| <a id="k-290"></a>K-290 | A:8627–8636 |
| <a id="k-291"></a>K-291 | A:8637–8645 |
| <a id="k-292"></a>K-292 | A:8646–8653 |
| <a id="k-293"></a>K-293 | A:8654–8663 |
| <a id="k-294"></a>K-294 | A:8664–8673 |
| <a id="k-295"></a>K-295 | A:8674–8684 |
| <a id="k-296"></a>K-296 | A:8685–8696 |
| <a id="k-297"></a>K-297 | A:8697–8706 |
| <a id="k-298"></a>K-298 | A:8707–8715 |
| <a id="k-299"></a>K-299 | A:8716–8724 |
| <a id="k-300"></a>K-300 | A:8725–8734 |
| <a id="k-301"></a>K-301 | A:8735–8743 |
| <a id="k-302"></a>K-302 | A:8744–8753 |
| <a id="k-303"></a>K-303 | A:8754–8761 |
| <a id="k-304"></a>K-304 | A:8762–8769 |
| <a id="k-305"></a>K-305 | A:8770–8779 |
| <a id="k-306"></a>K-306 | A:8780–8789 |
| <a id="k-307"></a>K-307 | A:8790–8800 |
| <a id="k-308"></a>K-308 | A:8801–8810 |
| <a id="k-309"></a>K-309 | A:8811–8820 |
| <a id="k-310"></a>K-310 | A:8821–8833 |
| <a id="k-311"></a>K-311 | A:8834–8844 |
| <a id="k-312"></a>K-312 | A:8845–8855 |
| <a id="k-313"></a>K-313 | A:8856–8867 |
| <a id="k-314"></a>K-314 | A:8868–8878 |
| <a id="k-315"></a>K-315 | A:8879–8889 |
| <a id="k-316"></a>K-316 | A:8890–8901 |
| <a id="k-317"></a>K-317 | A:8902–8913 |
| <a id="k-318"></a>K-318 | A:8914–8925 |
| <a id="k-319"></a>K-319 | A:8926–8936 |
| <a id="k-320"></a>K-320 | A:8937–8948 |
| <a id="k-321"></a>K-321 | A:8949–8961 |
| <a id="k-322"></a>K-322 | A:8962–8972 |
| <a id="k-323"></a>K-323 | A:8973–8982 |
| <a id="k-324"></a>K-324 | A:8983–8993 |
| <a id="k-325"></a>K-325 | A:8994–9004 |
| <a id="k-326"></a>K-326 | A:9005–9015 |
| <a id="k-327"></a>K-327 | A:9016–9025 |
| <a id="k-328"></a>K-328 | A:9026–9035 |
| <a id="k-329"></a>K-329 | A:9036–9045 |
| <a id="k-330"></a>K-330 | A:9046–9054 |
| <a id="k-331"></a>K-331 | A:9055–9063 |
| <a id="k-332"></a>K-332 | A:9064–9072 |
| <a id="k-333"></a>K-333 | A:9073–9081 |
| <a id="k-334"></a>K-334 | A:9082–9090 |
| <a id="k-335"></a>K-335 | A:9091–9099 |
| <a id="k-336"></a>K-336 | A:9100–9108 |
| <a id="k-337"></a>K-337 | A:9109–9116 |
| <a id="k-338"></a>K-338 | A:9117–9124 |
| <a id="k-339"></a>K-339 | A:9125–9133 |
| <a id="k-340"></a>K-340 | A:9134–9141 |
| <a id="k-341"></a>K-341 | A:9142–9152 |
| <a id="k-342"></a>K-342 | A:9153–9161 |
| <a id="k-343"></a>K-343 | A:9162–9170 |
| <a id="k-344"></a>K-344 | A:9171–9179 |
| <a id="k-345"></a>K-345 | A:9180–9187 |
| <a id="k-346"></a>K-346 | A:9188–9195 |
| <a id="k-347"></a>K-347 | A:9196–9203 |
| <a id="k-348"></a>K-348 | A:9204–9214 |
| <a id="k-349"></a>K-349 | A:9215–9224 |
| <a id="k-350"></a>K-350 | A:9225–9236 |
| <a id="k-351"></a>K-351 | A:9237–9247 |
| <a id="k-352"></a>K-352 | A:9248–9258 |
| <a id="k-353"></a>K-353 | A:9259–9268 |
| <a id="k-354"></a>K-354 | A:9269–9278 |
| <a id="k-355"></a>K-355 | A:9279–9288 |
| <a id="k-356"></a>K-356 | A:9289–9298 |
| <a id="k-357"></a>K-357 | A:9299–9310 |
| <a id="k-358"></a>K-358 | A:9311–9320 |
| <a id="k-359"></a>K-359 | A:9321–9331 |
| <a id="k-360"></a>K-360 | A:9332–9339 |
| <a id="k-361"></a>K-361 | A:9340–9348 |
| <a id="k-362"></a>K-362 | A:9349–9357 |
| <a id="k-363"></a>K-363 | A:9358–9366 |
| <a id="k-364"></a>K-364 | A:9367–9375 |
| <a id="k-365"></a>K-365 | A:9376–9383 |
| <a id="k-366"></a>K-366 | A:9384–9390 |
| <a id="k-367"></a>K-367 | A:9391–9400 |
| <a id="k-368"></a>K-368 | A:9401–9408 |
| <a id="k-369"></a>K-369 | A:9409–9417 |
| <a id="k-370"></a>K-370 | A:9418–9424 |
| <a id="k-371"></a>K-371 | A:9425–9433 |
| <a id="k-372"></a>K-372 | A:9434–9442 |
| <a id="k-373"></a>K-373 | A:9443–9452 |
| <a id="k-374"></a>K-374 | A:9453–9461 |
| <a id="k-375"></a>K-375 | A:9462–9470 |
| <a id="k-376"></a>K-376 | A:9471–9479 |
| <a id="k-377"></a>K-377 | A:9480–9488 |
| <a id="k-378"></a>K-378 | A:9489–9497 |
| <a id="k-379"></a>K-379 | A:9498–9506 |
| <a id="k-380"></a>K-380 | A:9507–9516 |
| <a id="k-381"></a>K-381 | A:9517–9526 |
| <a id="k-382"></a>K-382 | A:9527–9535 |
| <a id="k-383"></a>K-383 | A:9536–9545 |
| <a id="k-384"></a>K-384 | A:9546–9558 |
| <a id="k-385"></a>K-385 | A:9559–9567 |
| <a id="k-386"></a>K-386 | A:9568–9575 |
| <a id="k-387"></a>K-387 | A:9576–9585 |
| <a id="k-388"></a>K-388 | A:9586–9594 |
| <a id="k-389"></a>K-389 | A:9595–9605 |
| <a id="k-390"></a>K-390 | A:9606–9615 |
| <a id="k-391"></a>K-391 | A:9616–9629 |
| <a id="k-392"></a>K-392 | A:9630–9638 |
| <a id="k-393"></a>K-393 | A:9639–9652 |
| <a id="k-394"></a>K-394 | A:9653–9664 |
| <a id="k-395"></a>K-395 | A:9665–9677 |
| <a id="k-396"></a>K-396 | A:9678–9689 |
| <a id="k-397"></a>K-397 | A:9690–9699 |
| <a id="k-398"></a>K-398 | A:9700–9707 |
| <a id="k-399"></a>K-399 | A:9708–9718 |
| <a id="k-400"></a>K-400 | A:9719–9731 |
| <a id="k-401"></a>K-401 | A:9732–9741 |
| <a id="k-402"></a>K-402 | A:9742–9753 |
| <a id="k-403"></a>K-403 | A:9754–9764 |
| <a id="k-404"></a>K-404 | A:9765–9774 |
| <a id="k-405"></a>K-405 | A:9775–9784 |
| <a id="k-406"></a>K-406 | A:9785–9795 |
| <a id="k-407"></a>K-407 | A:9796–9805 |
| <a id="k-408"></a>K-408 | A:9806–9814 |
| <a id="k-409"></a>K-409 | A:9815–9823 |
| <a id="k-410"></a>K-410 | A:9824–9833 |
| <a id="k-411"></a>K-411 | A:9834–9844 |
| <a id="k-412"></a>K-412 | A:9845–9855 |
| <a id="k-413"></a>K-413 | A:9856–9864 |
| <a id="k-414"></a>K-414 | A:9865–9875 |
| <a id="k-415"></a>K-415 | A:9876–9885 |
| <a id="k-416"></a>K-416 | A:9886–9895 |
| <a id="k-417"></a>K-417 | A:9896–9905 |
| <a id="k-418"></a>K-418 | A:9906–9914 |
| <a id="k-419"></a>K-419 | A:9915–9924 |
| <a id="k-420"></a>K-420 | A:9925–9933 |
| <a id="k-421"></a>K-421 | A:9934–9941 |
| <a id="k-422"></a>K-422 | A:9942–9951 |
| <a id="k-423"></a>K-423 | A:9952–9961 |
| <a id="k-424"></a>K-424 | A:9962–9971 |
| <a id="k-425"></a>K-425 | A:9972–9980 |
| <a id="k-426"></a>K-426 | A:9981–9990 |
| <a id="k-427"></a>K-427 | A:9991–10000 |
| <a id="k-428"></a>K-428 | A:10001–10010 |
| <a id="k-429"></a>K-429 | A:10011–10020 |
| <a id="k-430"></a>K-430 | A:10021–10030 |
| <a id="k-431"></a>K-431 | A:10031–10039 |
| <a id="k-432"></a>K-432 | A:10040–10047 |
| <a id="k-433"></a>K-433 | A:10048–10055 |
| <a id="k-434"></a>K-434 | A:10056–10063 |
| <a id="k-435"></a>K-435 | A:10064–10072 |
| <a id="k-436"></a>K-436 | A:10073–10080 |
| <a id="k-437"></a>K-437 | A:10081–10088 |
| <a id="k-438"></a>K-438 | A:10089–10096 |
| <a id="k-439"></a>K-439 | A:10097–10104 |
| <a id="k-440"></a>K-440 | A:10105–10112 |
| <a id="k-441"></a>K-441 | A:10113–10123 |
| <a id="k-442"></a>K-442 | A:10124–10134 |
| <a id="k-443"></a>K-443 | A:10135–10142 |
| <a id="k-444"></a>K-444 | A:10143–10151 |
| <a id="k-445"></a>K-445 | A:10152–10160 |
| <a id="k-446"></a>K-446 | A:10161–10168 |
| <a id="k-447"></a>K-447 | A:10169–10175 |
| <a id="k-448"></a>K-448 | A:10176–10183 |
| <a id="k-449"></a>K-449 | A:10184–10191 |
| <a id="k-450"></a>K-450 | A:10192–10199 |
| <a id="k-451"></a>K-451 | A:10200–10207 |
| <a id="k-452"></a>K-452 | A:10208–10214 |
| <a id="k-453"></a>K-453 | A:10215–10220 |
| <a id="k-454"></a>K-454 | A:10221–10227 |
| <a id="k-455"></a>K-455 | A:10228–10233 |
| <a id="k-456"></a>K-456 | A:10234–10239 |
| <a id="k-457"></a>K-457 | A:10240–10246 |
| <a id="k-458"></a>K-458 | A:10247–10253 |
| <a id="k-459"></a>K-459 | A:10254–10260 |
| <a id="k-460"></a>K-460 | A:10261–10269 |
| <a id="k-461"></a>K-461 | A:10270–10276 |
| <a id="k-462"></a>K-462 | A:10277–10283 |
| <a id="k-463"></a>K-463 | A:10284–10289 |
| <a id="k-464"></a>K-464 | A:10290–10297 |
| <a id="k-465"></a>K-465 | A:10298–10306 |
| <a id="k-466"></a>K-466 | A:10307–10315 |
| <a id="k-467"></a>K-467 | A:10316–10326 |
| <a id="k-468"></a>K-468 | A:10327–10338 |
| <a id="k-469"></a>K-469 | A:10339–10348 |
| <a id="k-470"></a>K-470 | A:10349–10360 |
| <a id="k-471"></a>K-471 | A:10361–10372 |
| <a id="k-472"></a>K-472 | A:10373–10383 |
| <a id="k-473"></a>K-473 | A:10384–10393 |
| <a id="k-474"></a>K-474 | A:10394–10401 |
| <a id="k-475"></a>K-475 | A:10402–10408 |
| <a id="k-476"></a>K-476 | A:10409–10420 |
| <a id="k-477"></a>K-477 | A:10421–10433 |
| <a id="k-478"></a>K-478 | A:10434–10446 |
| <a id="k-479"></a>K-479 | A:10447–10460 |
| <a id="k-480"></a>K-480 | A:10461–10471 |
| <a id="k-481"></a>K-481 | A:10472–10485 |
| <a id="k-482"></a>K-482 | A:10486–10498 |
| <a id="k-483"></a>K-483 | A:10499–10511 |
| <a id="k-484"></a>K-484 | A:10512–10523 |
| <a id="k-485"></a>K-485 | A:10524–10534 |
| <a id="k-486"></a>K-486 | A:10535–10542 |
| <a id="k-487"></a>K-487 | A:10543–10553 |
| <a id="k-488"></a>K-488 | A:10554–10565 |
| <a id="k-489"></a>K-489 | A:10566–10577 |
| <a id="k-490"></a>K-490 | A:10578–10588 |
| <a id="k-491"></a>K-491 | A:10589–10599 |
| <a id="k-492"></a>K-492 | A:10600–10610 |
| <a id="k-493"></a>K-493 | A:10611–10620 |
| <a id="k-494"></a>K-494 | A:10621–10629 |
| <a id="k-495"></a>K-495 | A:10630–10638 |
| <a id="k-496"></a>K-496 | A:10639–10648 |
| <a id="k-497"></a>K-497 | A:10649–10659 |
| <a id="k-498"></a>K-498 | A:10660–10669 |
| <a id="k-499"></a>K-499 | A:10670–10679 |
| <a id="k-500"></a>K-500 | A:10680–10689 |
| <a id="k-501"></a>K-501 | A:10690–10697 |
| <a id="k-502"></a>K-502 | A:10698–10706 |
| <a id="k-503"></a>K-503 | A:10707–10715 |
| <a id="k-504"></a>K-504 | A:10716–10723 |
| <a id="k-505"></a>K-505 | A:10724–10731 |
| <a id="k-506"></a>K-506 | A:10732–10740 |
| <a id="k-507"></a>K-507 | A:10741–10749 |
| <a id="k-508"></a>K-508 | A:10750–10758 |
| <a id="k-509"></a>K-509 | A:10759–10767 |
| <a id="k-510"></a>K-510 | A:10768–10776 |
| <a id="k-511"></a>K-511 | A:10777–10785 |
| <a id="k-512"></a>K-512 | A:10786–10795 |
| <a id="k-513"></a>K-513 | A:10796–10803 |
| <a id="k-514"></a>K-514 | A:10804–10812 |
| <a id="k-515"></a>K-515 | A:10813–10819 |
| <a id="k-516"></a>K-516 | A:10820–10826 |
| <a id="k-517"></a>K-517 | A:10827–10832 |
| <a id="k-518"></a>K-518 | A:10833–10838 |
| <a id="k-519"></a>K-519 | A:10839–10846 |
| <a id="k-520"></a>K-520 | A:10847–10852 |
| <a id="k-521"></a>K-521 | A:10853–10861 |
| <a id="k-522"></a>K-522 | A:10862–10869 |
| <a id="k-523"></a>K-523 | A:10870–10878 |
| <a id="k-524"></a>K-524 | A:10879–10887 |
| <a id="k-525"></a>K-525 | A:10888–10898 |
| <a id="k-526"></a>K-526 | A:10899–10910 |
| <a id="k-527"></a>K-527 | A:10911–10922 |
| <a id="k-528"></a>K-528 | A:10923–10930 |
| <a id="k-529"></a>K-529 | A:10931–10937 |
| <a id="k-530"></a>K-530 | A:10938–10943 |
| <a id="k-531"></a>K-531 | A:10944–10951 |
| <a id="k-532"></a>K-532 | A:10952–10960 |
| <a id="k-533"></a>K-533 | A:10961–10968 |
| <a id="k-534"></a>K-534 | A:10969–10976 |
| <a id="k-535"></a>K-535 | A:10977–10984 |
| <a id="k-536"></a>K-536 | A:10985–10993 |
| <a id="k-537"></a>K-537 | A:10994–11001 |
| <a id="k-538"></a>K-538 | A:11002–11010 |
| <a id="k-539"></a>K-539 | A:11011–11018 |
| <a id="k-540"></a>K-540 | A:11019–11026 |
| <a id="k-541"></a>K-541 | A:11027–11036 |
| <a id="k-542"></a>K-542 | A:11037–11046 |
| <a id="k-543"></a>K-543 | A:11047–11055 |
| <a id="k-544"></a>K-544 | A:11056–11064 |
| <a id="k-545"></a>K-545 | A:11065–11073 |
| <a id="k-546"></a>K-546 | A:11074–11082 |
| <a id="k-547"></a>K-547 | A:11083–11090 |
| <a id="k-548"></a>K-548 | A:11091–11098 |
| <a id="k-549"></a>K-549 | A:11099–11107 |
| <a id="k-550"></a>K-550 | A:11108–11116 |
| <a id="k-551"></a>K-551 | A:11117–11125 |
| <a id="k-552"></a>K-552 | A:11126–11135 |
| <a id="k-553"></a>K-553 | A:11136–11145 |
| <a id="k-554"></a>K-554 | A:11146–11154 |
| <a id="k-555"></a>K-555 | A:11155–11165 |
| <a id="k-556"></a>K-556 | A:11166–11176 |
| <a id="k-557"></a>K-557 | A:11177–11186 |
| <a id="k-558"></a>K-558 | A:11187–11195 |
| <a id="k-559"></a>K-559 | A:11196–11205 |
| <a id="k-560"></a>K-560 | A:11206–11213 |
| <a id="k-561"></a>K-561 | A:11214–11221 |
| <a id="k-562"></a>K-562 | A:11222–11228 |
| <a id="k-563"></a>K-563 | A:11229–11236 |
| <a id="k-564"></a>K-564 | A:11237–11244 |
| <a id="k-565"></a>K-565 | A:11245–11252 |
| <a id="k-566"></a>K-566 | A:11253–11261 |
| <a id="k-567"></a>K-567 | A:11262–11270 |
| <a id="k-568"></a>K-568 | A:11271–11278 |
| <a id="k-569"></a>K-569 | A:11279–11286 |
| <a id="k-570"></a>K-570 | A:11287–11294 |
| <a id="k-571"></a>K-571 | A:11295–11302 |
| <a id="k-572"></a>K-572 | A:11303–11310 |
| <a id="k-573"></a>K-573 | A:11311–11317 |
| <a id="k-574"></a>K-574 | A:11318–11325 |
| <a id="k-575"></a>K-575 | A:11326–11332 |
| <a id="k-576"></a>K-576 | A:11333–11340 |
| <a id="k-577"></a>K-577 | A:11341–11348 |
| <a id="k-578"></a>K-578 | B:2217–2223 |

### K-579 — Fixed external-value boundary and architecture impact (2026-11-13)
<a id="k-579"></a>

- Registry/D-GOV 579 →580 continues H-05 evidence maintenance. D-25r5 →r6 records H-390's explicit decision, not agent approval of an open choice. C-85/P-362 now distinguish unknown values from fixed semantic definitions; P-359/P-360 and candidate production mechanisms remain pending whole-file review. Historical H/K payloads and immutable locators are preserved.
- Charter alignment: §§2–3/§5.1–5.4 separate values, definitions and composition stages; §4.3 retains identity/capability validation; §6 preserves selected interpretations and separates semantic integration from native generation; §§8/12 require inspectable contracts and discriminating checks. A fixed selected composition does not freeze all domains into the host compiler or rewrite the charter's separate integration goal. D-15/D-21 behavior, D-01/D-14 owners/relocation, D-17 completion and D-16 representation-independent identity/UoM remain unchanged; no dependent qualification is invalidated or newly granted.
- Impact assessment: remove runtime semantic registration/evolution from the candidate burden; shared-native coverage is more tractable, but unseen combinations, generic/capture evidence and owner/completion costs remain. Retained checked semantics remains recommended. A framework pilot is still a hypothesis about optimizer reuse, not a requirement caused by external inputs; engine and native timing remain unapproved. All existing fixtures use fixed definitions, so their evidence remains applicable without a rerun. Future boundary controls must reject semantic extension without changing the fused artifact; no validator mechanism is chosen.
- Documentation scope: update D-25 authority, responsibilities, criteria, recommendation, pilot boundaries and coverage together with this registry. Identify the deliverable audit as the unchanged revision5 snapshot at `0c0efbf`, not current-document hash verification. No code, build configuration, experiment results or test scripts are changed or executed.

### K-580 — Consolidate scopes and lineage for autonomous audit (2026-11-13)
<a id="k-580"></a>

- Agent continuity: registry/D-GOV 580→581 and AGENTS maintenance guidance; H-05/H-388/H-391, charter §§1/3/8/10/11. Qualification remains proposition/revision/scope-specific relative to the human baseline. Pending choices do not become qualified; no product document or charter change belongs to this audit. Concurrent H-390/K-579 and its relationship updates are retained intact.
- Scope rows replace repeated revision/checkpoint narratives. Node-to-contract links plus separate edge endpoints provide impact navigation. All 90 value/concept/governance IDs, 362 P IDs and 12 E IDs remain. P-01–P-11 are normalized from the legacy table, still pending. P-27/P-29/P-60/P-65 retain human-bearing rationale while research logs/qualifications remain in D-08 and source B.
- P-98 and P-300 are reconciled with recorded H-277–H-281/H-288–H-291 and H-342. K-555/K-573/K-305 supports D-17r53/D-15r20/local D-08r220. No new human choice, edge topology, acceptance or descendant qualification is inferred. H-390’s known-ADT input boundary and pending production mechanisms remain explicit.
- Acceptance: stable IDs/anchors resolve; historical ranges retrieve original events; relationship endpoints/kinds are unchanged; all edge rows except the six listed clarifications/compactions are preserved. Current revisions are reconciled to evidence. Documentation only; no build, test-script, configuration or toolchain action.

### K-581 — Retain native evidence and revise architecture conclusions (2026-11-13)
<a id="k-581"></a>

- Registry/D-GOV581→582 continues H-05 maintenance; D-25r6→r7 records H-392's removal instruction. The model, combined runner/report and aggregate completion audit are retired; original material remains at `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:experiments/architecture/`. Thirty independent LLVM rows are preserved exactly with explicit historical provenance, and the identical LLVM generator has a standalone runner. Whole-program/regional/managed sources and measured reports remain unchanged.
- Reassessment: withdraw the model's timing/code-sharing/identity/coverage evidence from active conclusions and the blanket aggregate completion claim. Known-ADT external combinations, same-layout identity and complete captures remain explicit gaps. Retained checked semantics is supported by whole-program/regional evidence; prioritize a proposed matched optimized-native/shared-lowering study. Truffle/RPython have no measured optimizer result or established priority. Native timing, engine, quantitative performance criteria and production qualification remain pending.
- Value/dependencies: charter §§2–3/§4.3/§5.3/§6/§8/§12 retain authoritative semantic structure, separate execution state, identity/owners and comparable native evidence. H-390/C-85/P-362 and D-15/D-21's selected-world behavior remain; D-01/D-14 relocation/ownership, D-17 completion and D-16 identity/UoM constraints are unchanged. P-359/P-360 remain candidate paths; no descendant is newly qualified or invalidated. Earlier H/K payloads are preserved; this event supersedes the recommendation/evidence scope of K-574/K-577/K-579 only where stated.

### K-582 — Record authorized scope and independent domain plan (2026-11-13)
<a id="k-582"></a>

- Registry/D-GOV 582→583; D-25r7→r8 records H-393, including workload revision 1. This is task authorization and agent planning, not qualification of pending candidate mechanisms. Prior H/K evidence remains unchanged.
- Charter §§1–3, 4.3, 5.3–5.4, 6, 8 and 12 aligns inspectable compiler-domain intent, separate state/interpretations, actual owners and end-to-end evidence. D-20/C-80/P-314–P-318 supplies requirements; P-359/P-360 remains proposed, P-361/P-362 and D-01/D-10/D-14/D-15/D-16/D-17/D-18/D-21 constraints remain. No descendant qualification or permanent production choice changes.
- The independent workload plan precedes BBV heuristic design. Current progress and time accounting belong in the work log; code/results and historical implementation success are not new architectural premises. No build configuration, toolchain, tests or production code change belongs to this reconciliation.

### K-583 — Center abstract method applicability (2026-11-13)
<a id="k-583"></a>

- Registry/D-GOV583→584; D-25r8→r9 and workload r1→r2 record H-394. Independent domain expectations now distinguish actual concrete implementations, crossed applicability and unrelated later stages from sum traversal. The first automatic transformation/storage scaffolding remains experimental supporting work, not family-dispatch evidence.
- Charter §§2–3/§5.3–5.4/§6/§8/§12, D-07 actual-type forwarding and explicit conformance, D-11 unique-best selection, D-15/D-18 selected interpretations/values and D-21 actual-type coverage preserve the same value/dependency boundaries. Multiple abstract parents, new conversion ranking, owner unions and production evidence ABI are not selected. Candidate paths remain pending; no descendant qualification changes. H/K history is preserved.

### K-584 — Implicit-context emphasis and completed qualitative review package (2026-11-13)
<a id="k-584"></a>

- Registry/D-GOV584→585; D-25r9→r10, supporting workload r2→r3, engineering evidence r1→r2 and local-dispatch assessment r1. Records H-395's actual emphasis while preserving H-393/H-394's autonomous experimental scope. The review package recommends continuing bounded local dispatch; its production concepts, paths and interfaces remain pending whole-file approval. This is agent assessment, not another human confirmation or qualification of a pending document. Historical H/K evidence is unchanged.
- Charter §§1–3/§4.3/§5.3–5.4/§6/§8/§12 align retained business intent, explicit actual owners, separated interpretations/state and inspectable native evidence. C-85/P-359–P-362, D-20/C-80 and D-01/D-07/D-10/D-11/D-14/D-15/D-16/D-17/D-18/D-21 remain the value/dependency boundary. D-22/D-23/D-24 choices, purity/TCO/fractions and production ABI are not selected. No descendant qualification changes.
- Evidence covers finite authored contracts, automatically generated native dispatch, one composed larger callable, local fact demand and permanent saturation, actual regional remapping and native LLVM resource operations. Full checking, generic coverage, owner/alias proof, context inference, module composition and general replay remain gaps. Commands/results and evolving work state belong in the supporting package and work log, not repeated registry checkpoints. Toolchain/configuration authority is unchanged.

### K-585 — Micro-dispatch replacement and discriminator/compatibility separation (2026-11-13)
<a id="k-585"></a>

- Current branch registry/D-GOV582→586; archival H-393–H-395/K-582–K-584 payloads are retained verbatim from 25ccc75 with their scopes. H-396/H-397 governs D-21r6→r7 and D-25r7→r11; archived D-25r8–r10 is not the new branch's architecture premise. New C-86/C-87/C-88 and separate P-363–P-370 value/dependency edges remain scoped as recorded. Earlier human history is unchanged.
- Charter §§1–3/§4.3/§5.3–5.4/§6.3/§8/§12 preserves typed business intent, actual owners, interpretation/state separation and inspectable native evidence. H-378's C-82/C-83/P-333/P-334/P-337 behavior continues: actual-type unique-most-specific method selection and prepared native execution. H-396 replaces the execution strategy with shared method bodies/local micro-dispatch; proven call-site facts need no body cloning. Concrete identity and root compatibility checksums have separate roles. No root-charter change, second identity system, permanent ABI or production DBMR format change follows.
- D-07r153→154, D-10r144→145, D-12r25→26, D-15r20→21 and D-18r89→90 are coordinated reference/authority clarifications. Existing selected-world, lexical, context/capture, owner/access, installation and completion contracts remain; their prior implementation reports retain their actual scopes. D-16 constraints, D-22/D-23/D-24 pending choices and pure/TCO/fraction deferrals are unchanged. No unaffected descendant qualification is invalidated or newly granted.
- The finite authored-contract prototype supplies native selectors/method bodies/local operations with optional specialization disabled, identity/field/alias checks and actual region/LLVM cleanup observations. A separate call-site control reuses the same bodies. H-397 adds separately compiled writer/reader evidence over unchanged saved bytes with stable definition IDs and invalid-interpretation rejection; its fixture catalog is not a production storage protocol. General generic closure, full checking/effect inference, admission/ABI, module composition and future ORC remain excluded. Conditional merge is H-396 task authorization, not agent self-approval of those production mechanisms. Exact runs, hashes, ancestry and merge-gate state are recorded in experiments/micro-dispatch, avoiding duplicated operational checkpoints here.

### K-586 — Source migration begins with closure review (2026-11-13)
<a id="k-586"></a>

- Charter alignment: §§2–3/§5.3–5.4/§6.3/§8/§12 require checked graph meaning, separation from execution instances, explicit uncertainty and source-to-native evidence. D-16 retains unit/representation identity and deferred fractions. D-24's omitted-result/unanchored-cycle policy is not resolved by closure planning.
- Registry/D-GOV586→587 preserves H-05/H-388/H-391 governance and all prior H/K payloads. H-398 records the new task; D-21r7→r8 and D-25r11→r12 route its review without changing established method, owner, context or cleanup behavior. D-26r1/C-89/P-371–P-373 are new pending propositions/relationships, not continued qualification.
- The isolated experiment remains bounded evidence; source migration has no existing general abstract carrier or independent checked emitter contract to assume. The local tag preserves the actual compiler baseline. Detailed changing plans, source inventory, commands and progress belong to MicroDispatchMigrationWorkLog.md; no source implementation or acceptance is claimed by this continuity event.

### K-587 — Removal-first restructuring scope (2026-11-13)
<a id="k-587"></a>

- H-399 changes the current stage’s ordering and completion conditions, not the target language. Charter §§2/§5.3–5.4/§6.3/§12 continues to require a coherent checked meaning and truthful supported-path evidence. The new instruction supersedes the incremental-retirement paragraph drafted under H-398.
- Registry/D-GOV587→588 preserves prior human/continuity evidence. D-26r1→r2 records the confirmed removal-first constraint without qualifying its pending closure proposal; D-25’s task index and migration work log record permitted capability gaps. No additional downstream semantic scope or ABI is qualified. The 12-case pre-edit baseline remains historical comparison evidence only.

### K-588 — Validation reference reflects the removal checkpoint (2026-11-13)
<a id="k-588"></a>

- Registry/D-GOV588→589 and tests/cod/language/README.md reference218→219 continue evidence maintenance under H-05/H-388/H-391 and H-399. Charter §§2–3/§6.3/§12 requires current support to be distinguished from intended contracts and historical observations.
- The test reference now states the removal checkpoint's actual availability and unchanged legacy expectations. Its complete prior reference and scoped authority remain recoverable at immutable commit 2ab762d617b742f4fba7c2c617bacd303739ccef, tests/cod/language/README.md. Fixtures/assertions/reports and all human evidence remain intact. No new semantics or qualification follows; detailed progress stays in MicroDispatchMigrationWorkLog.md.

### K-589 — Generic type-boundary discussion and dependency reconciliation (2026-11-13)
<a id="k-589"></a>

- Registry/D-GOV589→590 continues scoped evidence maintenance under H-05/H-388/H-391/H-403. D-26r2→r3 records H-400–H-402's newly confirmed local decisions; this is not continuity approval of the previously unqualified closure contract. Human history is appended; prior H/K payloads and immutable locators remain intact. Whole-file approval and candidate C-89/C-90 derivations P-371/P-372/P-374/P-375 stay pending.
- Charter §§2–3/§5.1/§5.3–5.4/§6.3/§7.1/§8–9/§12 supplies the reviewed alignment: preserve typed business meaning and shared execution, distinguish type identity from representation, expose unavailable interpretation, retain trusted checking and make library requirements inspectable. D-17 checked/unchecked completion and cleanup, D-14/D-18 actual owners/complete captures, and D-16 nominal/UoM/representation distinctions remain constraints. No hidden conversion, fraction implementation or new contract intrinsic follows.
- H-400 changes the input boundary relative to H-390; current C-85/P-362 and the scoped replacement index identify it explicitly. D-15/D-21/D-25 wording that categorically excludes outside runtime types is stale for this scope and must be reconciled before reliance; unaffected world isolation, prepared methods, installation effects, selection, ownership and historical bounded experiments remain. D-07/D-17/D-18/D-21 are reverse-linked for same-T/exception integration. The open opaque-operation boundary, proof algorithm and ABI are not agent-approved by this update.
- D-26 replaces mandatory type-substitution instance framing with shared native coverage and keeps compiler-unproved outcomes distinct from both named runtime failures. Future acceptance cases record required evidence, not completed implementation. Only the discussion document and authority registry are changed; source, tests, build configuration and mainline task state are untouched.

### K-590 — Interpretation boundary recorded after topic confirmation (2026-11-13)
<a id="k-590"></a>

- Registry/D-GOV590→591 continues H-05/H-388/H-391/H-403 evidence maintenance; the acceptance summary now accurately describes paraphrased side-chat decisions. D-26r3→r4 records H-404's newly confirmed boundary and matching marker/downstream-operation acceptance. This resolves one pending choice; it is not agent continuity approval of the whole file. Historical H/K payloads remain unchanged.
- Charter §§5.3–5.4/§6.3/§7.1 retains trusted type checking, observable operation behavior and distinct identity/representation validation. C-90/P-374/P-375 retains its candidate value path; D-14/D-18 owners and complete evidence, D-17 checked propagation/cleanup and D-16 type/UoM distinctions remain constraints. D-07/D-15/D-17/D-18/D-21/D-25's affected input/operation scopes route to D-26r4; no unrelated qualification changes.
- Only the discussion document and registry are updated. D-26 identifies the next unresolved choice against D-17's declared-NPE rule: unconditional throws obligations versus a defined type-dependent exception contract. H-404 does not approve conditional exception effects or specialization-based waivers. Carrier/ABI and input-validation protocols also remain open; no compiler, test, build or mainline task action is implied.

### K-591 — Generic and voluntary exception obligations distinguished (2026-11-13)
<a id="k-591"></a>

- Registry/D-GOV591→592 continues scoped evidence maintenance; D-26r4→r5 records H-405 and removes the agent-created conditional-throws choice from current pending decisions. The comparison is retained as historical evidence in K-590, not as an accepted requirement. Prior H/K payloads remain intact; local decisions do not qualify the whole document or its candidate value path.
- Charter §§5.3–5.4/§6.3/§8 supports accurate checked contracts independent of incidental optimization. D-17's explicitly declared exception obligations, H-400/H-404's genuine unknown-interpretation boundary, D-14/D-18 owner evidence and D-16 type/UoM distinctions remain. D-07/D-17/D-21 integration routes to D-26r5; no new exception-polymorphism feature or unrelated qualification is introduced. Documentation only.

### K-592 — Concrete analysis limits without a halting oracle (2026-11-13)
<a id="k-592"></a>

- Registry/D-GOV592→593 and D-26r5→r6 record H-406, retaining prior H/K history. Current scope and acceptance use concrete semantic errors, detected resource stops and H-399 implementation gaps rather than a universal unproved-program classification. The algorithm and whole-file/value path remain pending; this is not agent qualification of the draft.
- Charter §§5.3–5.4/§6.3 retains checked semantic meaning and native behavior independently of runtime termination. The draft distinguishes bounded analysis procedures and explicit limitations from deciding arbitrary future nontermination. C-89/P-371/P-372 and D-21/D-25 coverage references are affected; owners, exception behavior, D-16 compatibility, result-inference exclusions and all other local decisions remain. Documentation only; no compiler or resource-policy implementation follows.


### K-593 — Reviewed closure contract and aligned runtime-input boundary (2026-11-13)
<a id="k-593"></a>

- Registry/D-GOV593→594 preserves all prior H/K payloads and records the current user's conditional approval H-407 and practical-stage preference H-408. D-26r6 is the reviewed human-approved baseline; r7 updates qualification and current wording only. This continuity record supplies no independent human approval.
- The review checked the cumulative D-26 contract against charter §§1–3/§5.1/§5.3–5.4/§6.3/§7.1/§8–9/§12 and the directly needed D-15/D-17/D-18/D-21/D-25 boundaries. Finite prepared behavior preserves abstract business intent and inspectable meaning; C-89/C-90 and P-371/P-372/P-374/P-375 have H-407's confirmed value path. Dependency edges do not approve unresolved parent mechanisms.
- No blocker was found within the stated scope: selection through `<any>` preserves actual identity and same-T; valid uninterpreted passage retains owner/lifetime constraints; unknown interpretation is checked and breached typing unchecked; generic bodies do not invent throws declarations, while voluntary declarations remain binding. Runtime depth/type growth does not require native body growth. Concrete compiler limits remain distinct from language invalidity and cannot be disguised as runtime checks. D-16 nominal/unit/representation distinctions and deferred implementation remain intact.
- D-15r21→r22, D-21r8→r9 and D-25r12→r13 reconcile current input/coverage wording to H-400–H-406/H-407. Fixed prepared methods do not restrict input to known ADTs or permit new method loading. The historical finite-catalog fixture and preserved compiler evidence remain bounded observations. No other operational contract, persistent ABI, validation protocol or result-inference choice is qualified.
- H-399/H-408 permits a useful coherent foundation with honest capability gaps; acceptance rows apply to claimed supported paths and are not a requirement for stage-wide feature parity. Routine qualified implementation needs no repeated confirmation. Only affected work deciding unresolved semantics or material boundaries requires review. Documentation only; no source, test, build, Git-state or goal-state mutation.

### K-594 — Definition, implementation and local selection boundaries (2026-11-13)
<a id="k-594"></a>

- Registry/D-GOV594→595 records H-409 without changing prior H/K payloads. D-21r9→r10, D-26r7→r8 and D-25r13→r14 apply the explicit hierarchy, call-site specialization preference, phase/initiator distinction and local micro-dispatch correction. C-86/P-364/P-370 and the current index track affected scopes; pending production representation/ABI/key/budget mechanisms are not qualified.
- Charter §§3.1–3.3/§5.2–5.4/§6.1–6.3 preserves semantic definitions independently of execution instances and derived native code. H-407's C-89/C-90/P-371/P-372/P-374/P-375 value/coverage path continues: general correctness and finite native coverage coexist with AOT-specialized direct calls. A semantic method's compiler implementations cannot alter overload specificity, owner relationships, capture values, runtime effect order or D-16 nominal/unit identity.
- The 13-body unspecialized experiment remains its actual historical observation, not a universal code-count law. Migration acceptance now distinguishes semantic methods, prepared implementations and independent local points, and requires both known-call specialization/direct-call evidence and general-path correctness. No new runtime interpretation, user compile-time execution rule, persistent carrier or source syntax is inferred. The migration work log records the corrected plan before source implementation.

### K-595 — Bounded source-foundation availability (2026-11-13)
<a id="k-595"></a>

- Registry/D-GOV595→596 and source validation reference219→220 continue H-398/H-399/H-407–H-409 within their recorded scope. Charter §§2–3/§5.3/§6.3/§12 requires checked semantic obligations, separate derived native implementations and honest observable evidence. Existing H/K payloads and pending production mechanism scopes remain unchanged; this event grants no new design qualification.
- The current availability index routes scalar source/native evidence and outstanding capabilities to the migration work log. D-21r10/D-26r8 remains the behavioral contract, with D-10 function/context exclusion, D-11 primitive widening, D-17 completion and D-16 deferred fraction/unit distinctions preserved. Private execution-local scalar transport and finite checking records do not establish a persistent carrier ABI, full generic closure, outside-catalog admission or a permanent checked representation.


### K-596 — Execution-preparation name and phase continuity (2026-11-13)
<a id="k-596"></a>

- Registry/D-GOV596→597 records H-410 separately from this agent continuity assessment. D-21r10→r11 moves from `AheadOfTimeMultipleDispatch.md` to `AheadOfTimeExecutionPreparation.md`, retains D-21 and existing section anchors, and makes compilation preparation versus runtime execution explicit in its introduction, responsibilities, native path, hierarchy, phase table, local operations, call-site example, coverage and acceptance. The stale authority-block revision is reconciled with the registry’s r10 baseline and current r11 coverage. Prior H/K payloads remain unchanged.
- Charter §§3.2–3.3/§5.2–5.4/§6.3 distinguishes semantic intent, execution instances, compilation activity and derived native implementations. C-82/P-333 → C-01 and C-83/P-334 → C-02 through D-21/P-337 preserves actual-type family meaning and prepared native execution. H-409’s hierarchy/direct-call/local-operation contract and H-407’s checked native-coverage baseline continue cumulatively; preparing selectors does not run effects, and runtime local branches create no method or specialization. No ABI, identity, specialization policy, algorithm, owner, capture, completion or D-16 numeric/UoM requirement changes.
- D-25r14→r15 and D-26r8→r9 align their current phase summaries and authority blocks. D-07/D-10/D-12/D-13/D-14/D-15/D-16/D-17/D-18 and the experiment/validation references receive locator-only editorial maintenance; their semantic revision labels, qualification, constraints, non-goals and acceptance remain unchanged. P-364/P-370 and current scope indices route to the renamed contract. Link resolution, unique anchors, historical payload preservation and the documentation-only diff are the acceptance checks; historical native evidence is not rerun or reclassified.

### K-597 — Scalar-storage and capability-reference continuity (2026-11-13)
<a id="k-597"></a>

- Registry/D-GOV597→598 and source reference220→221 retain H-398/H-399/H-407–H-410’s scoped baseline. D-14 H-272/H-297/H-313/H-366–H-369, D-10 H-149/H-381 and D-17’s scope/completion rules govern scalar copies, permissions, initialization and lookup; D-16 numeric/representation distinctions and deferred fraction implementation remain. Charter §§2–3/§5.3/§6.3/§12 continues to require independently checked meaning and accurate native capability claims. No new value edge, language choice, persistent ABI or production checked representation is qualified.
- The user requested external process tracking to keep repository searches focused. Current capability locators in D-15r22/D-25r15 and this registry now resolve to the source reference; their semantic revisions, qualification and exclusions are unchanged. Existing human/continuity payloads remain intact. Operational history and run records are external; the repository retains current contracts, capability documentation and reproducible tests.

### K-598 — Scalar loop and dynamic cleanup capability continuity (2026-11-13)
<a id="k-598"></a>

- Registry/D-GOV598→599 and source reference221→222 continue H-304/H-356/H-365 through C-68/P-264–P-265 and C-63/P-321/P-326, with D-14 initialization/access and D-10 lexical scope constraints. Charter §§2–3/§6.3/§12 requires distinct execution occurrences, preserved effect order and observable native evidence. The bounded scalar implementation retains once-only range bounds, zero-iteration else, stable fold types, explicit declaration scopes and independent prepared registrations in the enclosing full scope's LIFO queue. Native acceptance covers return/failure paths, exact release, allocation failure and accumulation costs; operational reports remain external.
- D-21r11/D-26r9 and H-398/H-399/H-407–H-410 continue cumulatively: checking and prepared native bodies do not depend on runtime iteration counts, and general local operation choices preserve semantic methods. D-17r53's wider effect/resource, handler and owning/borrowed-value requirements remain outside this scalar coverage. No design revision, new value relationship, persistent ABI, regional layout, production checked representation or D-16 numeric/UoM/fraction decision is introduced. Existing human and continuity payloads remain unchanged.

### K-599 — Finally and delayed-block completion continuity (2026-11-13)
<a id="k-599"></a>

- Registry/D-GOV599→600 and source reference222→223 continue D-17r53/H-267/H-356–H-362/H-365/H-371 through C-63/P-235–P-236/P-321–P-324/P-326/P-329. D-14 initialization/access and D-10 lexical/function/effect constraints preserve visible control and resource responsibility under charter §§2–3/§6.2–6.3/§12. Ordinary and attached finally, delayed scalar reads, explicit replacement returns and nested registrations share completion routing without adding a source lifetime scope. Native acceptance covers composed normal/failure/return exits, exact queue release and allocation failure before/during cleanup.
- D-21r11/D-26r9/H-407/H-409's independently checked native preparation remains the bounded implementation baseline. Typed catches, exception values/causes, declared throws, queue-sensitive initialization and complete deferred-return proofs remain work; no narrower success claim replaces D-17's full contract. Persistent ABI, regional representation, owning/borrowed cleanup, D-16 numeric/UoM/fraction boundaries and pure/TCO deferral remain unchanged. Prior human/continuity payloads remain intact; operational state and run records stay outside the repository.

### K-600 — Intrinsic exception value and catch continuity (2026-11-13)
<a id="k-600"></a>

- Registry/D-GOV600→601 and source reference223→224 continue D-17r53/H-267/H-269/H-313–H-315 through C-63/P-235–P-236, with D-10 lexical/function boundaries and D-14 bits/access/initialization constraints. The current capability block consolidates K-597–K-599's unchanged scopes. Bounded exception values, exact catches, explicit retained causes and checked call obligations preserve inspectable business failure and resource responsibility under charter §§2–3/§6.2–6.3/§12. Acceptance covers native value copies/returns, generic passage, method relations, catch/finally completion, low-memory routing and immutable cause retention; operational evidence remains external.
- D-21r11/D-26r9/H-407/H-409 retains checked preparation and local operation selection. The private execution-local carrier is not a production checked representation or public/persistent ABI; no regional pointer, allocator, owning/borrowed boundary, arbitrary payload graph or RTTI contract changes. Deferred recovery/catch snapshots, full category/queue/result proofs and wider source coverage remain required work. D-16 numeric representation, UoM and fraction deferral are unchanged. Historical decision/continuity payloads and legacy test assertions remain intact.

### K-601 — Deferred caught-value capability continuity (2026-11-13)
<a id="k-601"></a>

- Registry/D-GOV601→602 and source reference224→225 continue D-17r53/H-267/H-269/H-356–H-362/H-365/H-371 through C-63/P-235–P-236/P-321–P-324/P-326/P-329. Immutable exception bits and independent dynamic registrations preserve explicit failure identity and cleanup under charter §§2–3/§6.2–6.3/§12. D-10 lexical bindings and D-14 access/lifetime remain constraints. Acceptance covers repeated and nested registration, named/bare rethrow, retained causes, recursive/general bodies, checked-failure source locations and exact release after registration failure. Operational evidence remains external.
- D-21r11/D-26r9/H-407/H-409's checked preparation and local operation selection remain unchanged. Captured exception bits use the existing private value carrier; ordinary mutable captures still read their lexical contents at invocation. No new language abstraction, value edge, lifetime extension, regional representation, persistent ABI or exception policy is selected. Deferred recovery and category/queue/result proof coverage remain incomplete; D-16 fractions and pure/TCO remain deferred. Prior human and continuity records are preserved.

### K-602 — Deferred recovery and scope-completion continuity (2026-11-13)
<a id="k-602"></a>

- Registry/D-GOV602→603 and source reference225→226 continue D-17r53/H-267/H-313–H-315/H-356–H-362/H-365/H-371 through C-63/P-235–P-236/P-321–P-324/P-326/P-329. Independent normal-result presence, explicit replacement returns, matching deferred handlers and owning-scope LIFO execution preserve visible business failure and resource responsibility under charter §§2–3/§5.4/§6.2–6.3/§12. D-10 lexical/effect and D-14 initialization/access remain constraints. Acceptance covers recovery from body/action failures, original/replaced/absent results, branch/loop/nested registrations, checked invocation coverage, fault injection and exact release. Operational evidence remains external.
- D-21r11/D-26r9/H-407/H-409's checked preparation and local operation selection remain the implementation boundary. Finite compiler-only queue equations and private native completion flags do not qualify a production checked representation, persistent ABI, regional layout or new source policy. Mutation/initialization guarantees, broader category relations, regional/borrowed values and wider closure support remain incomplete. D-16 numeric/UoM/fraction boundaries and pure/TCO deferral are unchanged; previous human and continuity payloads remain intact.

### K-603 — Cleanup initialization capability continuity (2026-11-13)
<a id="k-603"></a>

- Registry/D-GOV603→604 and source reference226→227 continue D-14r82/H-297/H-313 and D-17r53/H-267/H-319/H-356–H-362/H-365/H-371 through C-72/P-280–P-282 and C-63/P-235–P-236/P-321–P-324/P-326/P-329. Reads justified by completed writes preserve explicit storage and failure responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. D-10 lexical/function/effect and D-14 access/lifetime constraints remain. Acceptance covers late initialization, conditional/repeated/nested registrations, category-specific cleanup, post-scope guarantees, fresh locals, failed registrations and exact native release. Operational evidence remains external.
- D-21r11/D-26r9/H-407/H-409's checked native preparation remains unchanged. Read requirements and initialization guarantees extend the existing compiler-only completion equations; native records, source storage, allocator behavior and persistent representation are unchanged. No implicit zero initialization, runtime initialization flag, alias/lifetime extension or new source policy follows. Predicate, wider queue/input, mutation and owner proofs remain incomplete. D-16 numeric/UoM/fraction boundaries and pure/TCO deferral are unchanged; prior human and continuity records are preserved.


### K-604 — Local binding and target continuity (2026-11-13)
<a id="k-604"></a>

- Registry/D-GOV604→605 and source reference227→228 continue D-14r82/H-292–H-297/H-313/H-366–H-369 through P-255/P-258/P-280–P-282/P-327, constrained by D-10 lexical boundaries and D-17r53 cleanup/capture obligations. Distinct bindings sharing actual storage preserve inspectable access, initialization and owner responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. Acceptance covers local shared targets, independent copies/qualifiers, owner expiry, branch/loop/deferred initialization, caught-value aliases and allocation-failure cleanup; operational evidence remains external.
- The LLVM-independent checking boundary and D-21r11/D-26r9/H-407/H-409 preparation contract remain. No runtime alias allocation, initialization flag, lifetime extension, regional representation or permanent ABI is selected. Mutable/nullable targets, external-owner borrowing and D-23 fresh borrowed results retain their separate proof/decision boundaries. D-22 construction/aggregate exclusions, D-16 numeric/UoM/fraction requirements and pure/TCO deferral remain unchanged. Prior human and continuity evidence is preserved.


### K-605 — Mutable target and view-evaluation continuity (2026-11-13)
<a id="k-605"></a>

- Registry/D-GOV605→606 and source reference228→229 continue D-14r82/H-271/H-297/H-299/H-302/H-305/H-306/H-313/H-366–H-369 through P-258/P-260/P-266–P-269/P-280–P-282/P-327, constrained by D-10 lexical and D-17r53 completion/capture rules. Exact owners, non-escalating access and address selection before later effects preserve inspectable storage responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. Acceptance covers changing local targets, selected-target initialization, branch/loop/deferred interactions, native evaluation order and failed assignment/registration cleanup; operational records remain external.
- D-21r11/D-26r9/H-407/H-409's independently checked native preparation remains. Finite target discovery and current-selection guarantees are compiler facts, not a new source ownership policy or permanent checked representation. Execution-local address slots do not change scalar/call/exception carriers, region allocation or cleanup records. Nullable/external-owner boundaries, full target/correlation proofs, D-23 fresh borrowed results, D-22 construction/aggregate choices and D-16 fraction implementation retain their separate scope. Historical evidence and pure/TCO deferral remain unchanged.


### K-606 — Nullable local views and retained-owner continuity (2026-11-13)
<a id="k-606"></a>

- Registry/D-GOV606→607 and source reference229→230 continue D-14r82/H-313–H-315 and D-17r53 through C-72/P-280–P-282, with D-10 lexical rules and D-14 initialization/access constraints. Distinguishing absent targets, actual owners and initialized payloads preserves inspectable storage and failure responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. Acceptance covers non-dereferencing tests, immediate checked-NPE obligations, mutation-sensitive non-null proof, retained owners through null aliases, scope/iteration expiry, deferred recovery, allocation failure and exact native release. Operational records remain external.
- D-21r11/D-26r9/H-407/H-409's checked native preparation remains. Compiler binding/type/owner facts and execution-local null guards introduce no artificial payload allocation, runtime initialization registry, lifetime extension, regional encoding or permanent ABI. H-317/P-284's nullable external-owner call contract remains outside current implementation coverage, as do complete target/queue correlations, D-23 fresh borrowed results and D-22 construction/aggregate choices. D-16 numeric/UoM/fraction boundaries, pure/TCO deferral and prior human/continuity records remain unchanged.


### K-607 — Explicit scalar borrowing across native calls (2026-11-13)
<a id="k-607"></a>

- Registry/D-GOV607→608 and source reference230→231 continue D-14r82/H-271–H-273/H-317/H-366–H-369/H-382 and D-18's existing borrowing/recursion contract through C-65/C-72/P-284/P-294/P-299/P-302/P-327/P-343. Actual owners, explicit access and preserved nullable views retain inspectable business aliasing and call responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. D-17r53/H-356/H-361–H-362 constrains prepared arguments, checked failures and cleanup through C-63/P-235–P-236/P-321–P-324. Acceptance covers caller-visible writes, independent callee bindings, owner/access rejection, immediate NPE boundaries, recursive/generic view returns, address retention/replacement, registration failure and exact release; operational evidence remains external.
- D-21r11/D-26r9/H-407/H-409's checked method/implementation separation remains. The private execution-local call transport and compiler external-target facts introduce no runtime owner object, borrowed payload copy, lifetime extension, regional representation or permanent ABI. Uninitialized-target call summaries, richer result provenance, differing general-body value/view layouts, owner-constrained overload ranking, D-23 fresh-result storage and D-22 construction/aggregate choices retain their proof/decision boundaries. P-244's broader candidate value path is not newly qualified. D-16 numeric/UoM/fraction constraints, pure/TCO deferral and prior human/continuity records remain unchanged.


### K-608 — Ordered initialization proofs across calls and cleanup (2026-11-13)
<a id="k-608"></a>

- Registry/D-GOV608→609 and source reference231→232 continue D-14r82/H-297/H-313/H-317 through C-65/C-72/P-258/P-280–P-284 and D-17r53's K-541/K-603 cleanup/call scope through C-63/P-235–P-236/P-321–P-324. Preserving storage identity and the order of reads and completed writes serves explicit business state and failure responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. K-607's owner/access/preparation boundaries remain. Acceptance covers unwritten borrowed inputs, alias-sensitive read requirements, recursive/generic guarantees, argument order, selected targets, category-specific failures, finally/defer, failed registration and exact native release; operational evidence remains external.
- Callable summaries and cleanup equations share compiler-only ordered read/write facts. Their fixed points do not change native calls, cleanup records, source lifetimes, allocation or initialized-value representation. D-21r11/D-26r9/H-407/H-409's independently checked native coverage remains; no runtime initialization flags, implicit zeroing or permanent checked schema follows. Null-sensitive cleanup, precise selected-result provenance and broader relational proofs remain incomplete. D-16 fraction/numeric/UoM boundaries, D-22/D-23 pending choices, pure/TCO deferral and prior human/continuity records are preserved.


### K-609 — Returned-target proofs through shared completion equations (2026-11-13)
<a id="k-609"></a>

- Registry/D-GOV609→610 and source reference232→233 continue K-607/K-608's external scalar borrowing and ordered initialization scope. D-14r82/H-201/H-273/H-297/H-313/H-317 through C-65/C-72/P-258/P-280–P-284 and D-18's existing-result/recursion contract through P-294/P-299/P-302 preserve actual aliases and initialized selected targets. D-17r53/H-361/H-365 through C-63/P-235–P-236/P-321–P-324 governs return preservation, replacement and recovery. These facts preserve inspectable business state and failure responsibility under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. Acceptance covers unrelated borrowed arguments, selected versus allocation-wide initialization, recursive/generic forwarding, null handling, rebinding, finally/deferred replacement, cancelled returns, recovery, registration failure and exact release; process evidence remains external.
- Compiler-only returned selections compose with the existing cleanup equations and close before native emission. D-21r11/D-26r9/H-407/H-409's checked method/implementation boundary remains. No runtime carrier, allocation, initialization flag, owner union, lifetime extension or permanent checked schema changes. Null provenance, completion reachability and wider relational proofs retain conservative gaps. D-23 fresh/mixed ordinary-result storage, D-22 construction/aggregate choices, D-16 numeric/UoM/fraction constraints and pure/TCO deferral remain separate. Prior human/continuity records and descendant qualification are preserved.


### K-610 — Complete empty-environment callable values and shared invocation (2026-11-13)
<a id="k-610"></a>

- Registry/D-GOV610→611 and source reference233→234 continue D-18r90/H-337–H-344 through C-65/C-73/C-75/P-294/P-299/P-301–P-303 for complete independent lexical values, source-visible owners, caller destinations and native recursion. Empty environments are a bounded implementation scope, not a replacement of H-344's runtime-sized environment requirement. D-17r53/H-356/H-361/H-365 through C-63/P-235–P-236/P-321–P-324 preserves preparation order, selected values and cleanup; D-14r82/H-273/H-297/H-313/H-317 constrains holder borrowing, initialization and nullness. These scopes preserve inspectable business identity, state and execution under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. Acceptance covers copies/replacement, parameters and independent/borrowed results, factories, recursive/generic calls, nullable holders, deferred snapshots, checked failures, allocation/release and compilation growth. Process evidence remains external.
- D-21r11/D-26r9/H-407/H-409's semantic-method selection, shared general implementations and optional specializations remain. Concrete callable views use the existing selector; each selected body is checked before emission, and runtime callable identities are not type-instance keys. The private empty-environment transport and caller destination do not qualify a permanent ABI, capture representation or broader family/module mechanism. Nonempty/runtime-growing environments, captured contexts, abstract-input selection, wider adaptation and nested-signature inference remain work. No lexical-cell promotion, regional callable storage, lifetime extension, fraction/pure/TCO implementation or D-22/D-23 decision follows. Prior history and descendant qualification are preserved.


### K-611 — Complete nested callable environments and lexical value cleanup (2026-11-13)
<a id="k-611"></a>

- Registry/D-GOV611→612 and source reference234→235 continue D-18r90/H-329/H-331/H-342/H-344 through C-65/C-73/C-75/P-297/P-301–P-303. Explicit/inferred captures, independent nested snapshots, caller destinations and runtime-sized scope-owned storage preserve readable business intent, source-visible lifetime responsibility and separate execution state under charter §§2–3/§5.3–5.4/§6.2–6.3/§12. D-14r82's holder/access/initialization contract and D-17r53/H-356/H-361/H-365's preparation and completion rules constrain copying, replacement and failure cleanup. Acceptance covers capture lists/shadowing, copies, growing loop/recursive environments, parameters/results, callee timing, allocation/registration failures, initialization, pending-return recovery and exact image/record release. Operational evidence remains external.
- D-21r11/D-26r9/H-407/H-409 continues checked semantic selection and shared native execution. Concrete capture types contribute to finite checking contexts; runtime values and environment depth do not create native bodies. Fresh unaliased values can transfer into destinations, preserving complete-copy semantics for existing values. The private image/cleanup implementation qualifies no permanent ABI, allocator policy, captured-region retention or independent user heap lifetime. Captured-payload mutation, ptr-cell/regional/context captures, broader adaptation and nested-signature inference remain gaps. D-16, pure/TCO and pending D-22/D-23 boundaries are unchanged; no lexical-cell promotion, regional callable storage or new design choice is adopted. Prior history and descendant qualification are preserved.


### K-612 — Structural callable type relations and checked completion (2026-11-13)
<a id="k-612"></a>

- Registry/D-GOV612→613 and source reference235→236 continue D-21r11/D-26r9/H-401/H-407/H-409 through C-82/C-83/C-89/P-333/P-334/P-371–P-372. Shared structural applicability and containment preserve concrete callable identity and repeated-binder constraints while keeping native implementations distinct from semantic methods. This serves inspectable checked meaning and native execution under charter §§2–3/§5.3–5.4/§6.3/§12. D-18r90/K-610/K-611 constrains complete values/captures; D-14r82 preserves access/owners; D-17r53/H-361 requires explicit returns for non-void contexts after cleanup. Acceptance covers nested signatures, same-T, alpha-equivalent duplicates, unique-most-specific selection, exact qualifier/throws contracts, earlier-argument context, void/value results, selected-body checking, preparation failures and finite shared native coverage. Process evidence remains external.
- A determined void substitution is distinct from an undetermined binder; shared native completion follows the checked result contract. Concrete type evidence from earlier arguments supplies later callable contexts without reordering effects. No callback variance, implicit adapter, permanent type encoding/ABI, omitted-result inference or new language policy is selected. Later-argument contextual inference, lexical outer type binders, abstract ingress and wider generic closure remain implementation work. D-16 numeric/UoM/fraction constraints, pure/TCO deferral, D-22/D-23 pending choices and all prior history/descendant qualification remain unchanged.


### K-613 — Retained lexical type context and local conversion selection (2026-11-13)
<a id="k-613"></a>

- Registry/D-GOV613→614 and source reference236→237 continue K-610–K-612's complete callable values, checked type relations and native coverage. D-10/H-258's compile-time generic/signature constraint combines with D-18r90/H-329/H-342/H-344 through C-65/C-73/C-75/P-297/P-301–P-303 and D-21r11/D-26r9/H-401/H-407/H-409 through C-82/C-83/C-89/P-333/P-334/P-371–P-372. Preserving required enclosing type bindings and each local operation's selected interpretation serves inspectable typed intent and shared native execution under charter §§2–3/§5.3–5.4/§6.3/§12. Acceptance covers nested signatures/local annotations, binder shadowing, transitive requirements, copies/returns, recursion/defer, void/value completion, context-dependent conversion, preparation failure, exact release and finite method reuse. Process evidence remains external.
- Type context is separate from value captures and does not retain ordinary lexical cells or allocate a type-only environment. Unused bindings create no requirement; undetermined required binders remain diagnosed. General bodies receive private checked type evidence; operand types alone cannot select between different required local result conversions. Identity conversions preserve prepared value ownership. No runtime type-object API, permanent encoding/ABI, generic result-inference rule, higher-rank contract, regional capture or new source policy is selected. Later-argument contextual inference, abstract input, broader generic closure and pending D-22/D-23 scopes remain open; D-14/D-17 ownership/completion, D-16 numeric/fraction constraints and pure/TCO deferral remain. Prior history and descendant qualification are unchanged.


### K-614 — Named-quantity authority review and bounded contract proposal (2026-11-13)
<a id="k-614"></a>

- Registry/D-GOV614→615 records D-27r1 as pending-human. The original H-128/H-260/H-261 and K-401/K-402 records establish selected behavior but explicitly leave the broader D-11 value derivation pending. Charter §§1–3/§5.3–5.4/§6.3/§8/§12 was reviewed; P-377–P-379 proposes direct bounded value relationships for named identities and lossless compatibility. P-380 records dependencies only. Whole-file review is requested under the user's review preference; no confirmation is inferred and no quantity implementation is activated by this event.
- Existing qualified scalar/callable scopes, H-256 priority, D-16 deferral and all prior history remain intact. D-27 excludes automatic unit routes, operator elaboration, general module composition, compound units, new generic inference and permanent ABI. D-11 retains its existing local decisions and pending broader paths; no descendant qualification is silently changed. Process/evidence tracking remains outside the repository.


### K-615 — Exact intrinsic catches in checked generic contexts (2026-11-13)
<a id="k-615"></a>

- Registry/D-GOV615→616 and source reference237→238 continue D-17r53/H-267/H-356–H-362 through C-63/P-235–P-236/P-321–P-324 and D-21r11/D-26r9/H-401/H-407/H-409 through C-82/C-83/C-89/P-333/P-334/P-371–P-372. Resolving an existing exact catch against its checked generic context preserves inspectable type identity and failure handling while sharing native methods under charter §§2–3/§5.3–5.4/§6.3/§12. K-613's retained lexical type context and D-14/D-18 access, initialization and complete-value constraints remain. Acceptance covers both execution modes, exact/mismatched and duplicate categories, caught-value selection/returns, retained contexts, recursion, deferred recovery/snapshots, readonly values, initialization and checked rethrows. Process and native method/release evidence remains external.
- This covers known intrinsic categories and the existing typed-catch rules only. It selects no generic throws inference, exception subtyping, new payload, descriptor/ABI, abstract input, owner extension or new cleanup policy. SystemException still uses an untyped catch; invalid or undetermined substitutions retain diagnostics. General methods use their checked context for handler identity, without compulsory type-tuple body cloning. D-27 remains pending whole-file review; D-16 and pure/TCO deferral, other design boundaries and all prior decision history remain unchanged.


### K-616 — Checked argument preparation and common-exception callable passage (2026-11-13)
<a id="k-616"></a>

- Registry/D-GOV616→617 and source reference238→239 continue D-17r53/H-267/H-356's bounded exception values and ordered preparation through C-63/P-235–P-236/P-321–P-322, D-18r90/H-337–H-344's complete callable values through C-65/C-73/P-299/P-301–P-303, and D-21r11/D-26r9/H-407/H-409's checked shared native coverage through C-82/C-83/C-89/P-333/P-334/P-371–P-372. Making each argument's admitted passing mode and prepared type explicit preserves inspectable typed intent, source order and storage responsibility under charter §§2–3/§5.3–5.4/§6.3/§12. D-14 owner/access/initialization and prior completion/capture constraints remain. Acceptance covers direct/callable/deferred passage, exact versus common identity, payload/causes, original-type selection, recursion, retained contexts, operand order, target snapshots, fresh images, failed preparation/registration and exact release in both execution modes. Process and native evidence remains external.
- Callable-value calls admit the existing independent concrete-to-common intrinsic-exception passage. They do not gain numeric widening, borrowed-target conversion, callback variance, generic throws inference or abstract ingress. Semantic method selection precedes preparation; native invocation consumes the checked argument facts instead of inferring exception adaptation from a method's source signature. Fresh-value ownership survives local adaptation. No language policy, owner extension, regional allocation, public/persistent ABI or alternate backend is selected. D-27 remains pending review, D-16/pure/TCO deferrals remain, and all prior decision history and unrelated qualifications are preserved.


### K-617 — Explicit local owners within borrowed callable scope (2026-11-13)
<a id="k-617"></a>

- Registry/D-GOV617→618 and source reference239→240 continue K-606–K-608's nullable/external-owner call scope under D-17r53/C-63/P-235–P-236/P-280/P-321–P-324 and D-18r90/C-65/C-73/P-294/P-299/P-302, constrained by D-14r82/H-273/H-297/H-314/H-317/P-258/P-282/P-284. The original H-273/H-314/H-317 evidence retains named owner syntax, optional local annotations, fixed owners and nullable passage. Resolving an annotation to the existing owner binding preserves inspectable storage responsibility and typed calls under charter §§2–3/§5.3–5.4/§6.3/§12. Acceptance covers existing/absent targets, owner-preserving copies/returns/rebinding, nearest lookup, initialization/access, deferred and recursive use, callable holders, failed registration and native parameter/allocation evidence. Process records remain external.
- This scope annotates existing targets or absent views within current callable owner bindings. Fresh storage in an external owner, regional owner declarations, general owner inference/capture, actual-owner alternatives and D-22 construction choices remain separate gaps. No target allocation, runtime owner metadata, lifetime extension, changed null/initialization policy or regional encoding follows. D-14's broader candidate value paths are not newly qualified. D-21/D-26 checked native coverage, D-27 pending review, D-16/pure/TCO deferrals and prior decision history remain unchanged.


### K-618 — Fixed ordinary-slot aliases and independent access paths (2026-11-13)
<a id="k-618"></a>

- Registry/D-GOV618→619 and source reference240→241 continue D-18r90/H-327/C-75/P-295's qualified ordinary-place aliases and D-14r82/H-366–H-369/P-327's independent binding/target restrictions. Original confirmation evidence was checked at the immutable H-event locators. Fixed place selection, exact kind/type assertions and inherited owner/access obligations preserve readable business aliasing and inspectable storage responsibility under charter §§1–3/§5.3–5.4/§6.3/§8/§12. D-17 completion/cleanup and D-21/D-26 checked shared native coverage remain constraints. Acceptance covers shared mutation/rebinding, once-only selection, independent names and permissions, null/initialization facts, owner and slot lifetime, generic/recursive/deferred calls, complete callable holders, failed preparation/registration and exact release. Process, staging and sanitizer evidence remains outside the repository.
- Current source coverage selects statically identified lexical slots and creates no payload copy, reference slot or lifetime extension. Slot permissions cannot be weakened through an alias, and the slot owner is distinct from its target owner. Runtime-selected slots, regional fields, arrays and general regional captures remain implementation gaps; ordinary aliases do not promote lexical cells into captures. No new language rule, regional encoding, public/persistent ABI or broader D-14 qualification follows. Prior decision history, pending D-27 review, unrelated qualifications and D-16/pure/TCO deferrals remain unchanged.
