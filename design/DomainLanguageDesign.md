# Domain language design: discussion and decision tracker

Discussion revision: 22

Recorded: 2026-10-22

Current discussion: 2026-11-05

Phase: coordinated language/SNN design, implementation planning and validation.

## Design authority

- **Registry:** [D-07](DesignAuthorityRegistry.md#d-07), which controls qualification and scope.
- **Revision / covered scope:** discussion revision 22, all sections of this language tracker. Goals record human direction; topic proposals and candidate acceptance criteria remain `pending-human`. The application has a separate D-08 document set.
- **Value lineage:** candidate P-12–P-18 connect the packet, database, language and interoperability concepts C-08–C-11 and this document to C-01, C-02, C-05 and C-06. Explicit language semantics should preserve inspectable business intent; regional transport and deferred reclamation should serve efficient persistent semantics; source tools should keep that semantics readable. These proposed derivations are not yet qualified implementation authority.
- **Dependencies:** P-19–P-21 identify C-03, C-04 and C-02: multiple regions/domains, semantic levels and environment-specific effects. P-24 records application C-12 as an external example for DL-13; its model and resource choices are scoped to D-08. Existing C++ designs are reference material with their own registry status.
- **Contract boundaries:** see “Scope and invariants”; candidate acceptance and implementation boundaries are in DL-13. H-96–H-141 supply the local symbol, callable, storage, snapshot and scalar-identity directions linked below; a complete grammar, ABI, storage protocol and implementation contract remain open.
- **Human baseline:** H-06 supplies the near-term goals; H-07 establishes discussion tracking; H-08 selects the first application; H-11 requires its own document set. [H-96–H-121](DesignAuthorityRegistry.md#h-96) direct coordinated development and define the scoped language propositions in [D-10](SymbolicDynamicScope.md). H-122/H-123 bound the forthcoming run to useful language/runtime support; H-124/H-127/H-129 specify regional snapshot access, and H-125/H-126/H-128 require strict scalar identities and a substantive UoM comparison. They do not approve every proposed implementation detail.
- **Continuity:** [K-231](DesignAuthorityRegistry.md#k-231) records revision 22: H-143 selects direct LLVM lowering, H-144 contract-focused tests and H-145 ongoing simplification. [K-228](DesignAuthorityRegistry.md#k-228) records revision 21 and the regional storage/snapshot implementation evidence. [K-227](DesignAuthorityRegistry.md#k-227) records revision 20, bounded iteration/numeric representations and the typed pool-index fixture. [K-226](DesignAuthorityRegistry.md#k-226) records revision 19 and the bounded UoM implementation checkpoint. [K-224](DesignAuthorityRegistry.md#k-224) records revision 18 and the H-139–H-141 complex-composition, once-only incorporation and initial-inlining scope. no qualified implementation baseline for D-07. K-02/K-03 record the earlier discussion baseline; [K-223](DesignAuthorityRegistry.md#k-223) records revision 17 and the resolved caller-scope, UoM-selection and parameter-conversion decisions; K-222 records the compound-UoM roadmap/initial-scope split; K-221 records parenthesized operator-method naming; K-220 records the bounded planning/typing discussion. K-219 retains the preceding implementation evidence. The full contract remains `pending-human`; this is not continuity-based qualification of an unqualified design.

## Confirmed direction

These are summaries of the user's statements, not inferred approval of the preceding assistant's recommendations. The original statements and their provenance are recorded in [H-06](DesignAuthorityRegistry.md#h-06), [H-07](DesignAuthorityRegistry.md#h-07).

| Goal | Confirmed direction | Discussion coverage |
| --- | --- | --- |
| G-01 | Use a memory region (`mem_region` in the discussion) as an IPC packet. Its header includes the total binary length. After complete reception, invoke a designated member method on the packet's entry object to apply it. | DL-02, DL-04, DL-05 |
| G-02 | Use DBMR as the local ontology database, supporting business retrieval, statistics and CRUD. Ordinary operations do not reclaim space; periodically rebuild its payload and backing file for compaction. | DL-06–DL-09 |
| G-03 | Give the domain language explicit ADT semantics, including sum/product types, method invocation and memory management. | DL-01–DL-03 |
| G-04 | Provide C/C++ compatibility interfaces for interoperability with other language runtimes. | DL-11 |
| G-05 | Advance language and SNN design, implementation and validation together, using simulator requirements to choose useful initial language features. | H-96; DL-12–DL-16 |
| G-06 | Nominal symbols, thread-scoped effects, captured/current procedure invocation, lexical functions and dynamically symbol-bound procedures, with whole-program call-path checking. | H-96–H-98, H-102–H-104; DL-02, DL-14, DL-15 |
| G-07 | User storage is stack or `@region`; captures are bits or regional pointers. Keep cross-region references as a concept for progressive design. | H-99–H-101; DL-03, DL-08, DL-15 |

“Only record” has not yet settled whether updates append immutable versions or mutate existing objects while allocation remains monotonic. It does not by itself require logging every read. “Apply” has not yet specified a durability guarantee, retry protocol or native method-pointer representation.

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

- Preserve strict branded identity under H-125 and independent compile-time UoM identities under H-128; structural layout alone does not establish equivalence. Discuss sum constructors and product fields, recursive types, pattern matching/exhaustiveness, equality and admissible regional fields. Keep semantic type identity separate from physical layout.
- Distinguish ordinary ADTs from indexed GADTs. Specify the initial role of type parameters, kind checking, inference and type equivalence without silently treating the full long-term type theory as already selected.
- Decide which ontology relations and constraints are first-class, what is explicitly asserted versus inferred, and how independently defined domain concepts can be mapped. Similar field layouts do not establish conceptual equivalence.
- Explain how domain signatures can be loaded and checked without requiring every domain constructor to become a fixed host-language enum. Distinguish published definitions from parser/solver scratch state.

**Current scalar discussion:** [ScalarUnitsAndIndexTypes](ScalarUnitsAndIndexTypes.md), D-11r5, compares newtype-style wrappers with UoM arithmetic against actual pool formulas. Arrays declare an index type; scalar operations must preserve or explicitly transform its meaning. H-128 makes UoMs independent named compile-time entities with first-class type identities, same-name distinct definitions and identity-preserving local aliases; they are not represented by symbols. H-130 supplies pure conversion methods with Julia-style argument-type dispatch; H-134 specifies fresh UoM definitions and one-shot using selections, with no UoM import. H-135–H-138 specify parameter-controlled conversion and compile-time route choice. H-131 adds parenthesized operator names such as `(+)` for readable method definitions; D-11 proposes ordinary pure-method dispatch and arity-based unary/binary selection. H-132 plans compound UoMs for future physical-quantity use and defers their implementation; compound equivalence/inference does not block this increment. Index origins and literal rules remain open within the bounded interface discussion. H-126 requests analysis before selecting the mechanism.

**Proposal to discuss:** start from a small explicit sum/product and method core whose checked representation can express business logic and evolve toward the charter's GADT graphs. Decide the necessary indexed features from concrete programs.

**Decision evidence:** one domain definition, one business program with a result-dependent branch, an invalid construction that is rejected, and the exact boundary between value, type and kind representations.

<a id="dl-02"></a>
### DL-02 — Methods, effects and entry execution

**Confirmed local direction:** functions are lexical and limited to explicit inputs/outputs; procedures are dynamically defined under symbol names and may access effects. `perform` installs captured values temporarily; `behave` uses current dynamic bindings. Module top-level dynamic scope is module initialization. H-96–H-111 and [D-10](SymbolicDynamicScope.md) supply the exact scope; method syntax and dispatch remain open.

**Question:** What is a method's meaning, and how does the receiver select and execute the entry object's designated method?

- Specify receiver binding, arguments/results, dispatch, method identity/version and access to the execution context. Decide whether definitions travel with a packet, are imported, or are resolved from already loaded definitions.
- Separate pure computation from effects such as database reads/writes and external operations. Define ordering, observable errors and capability binding; decide whether the first version needs suspension, persistent continuations or cancellation.
- Keep business decisions expressible in the typed program representation. Identify which primitives are supplied by native handlers and how two environments can interpret the same business program.

**Proposal to discuss:** an illustrative `entry.apply(context) -> Result<Response, ApplyError>` contract, with methods resolved against explicit definitions and bindings. Serialized native function/member-function addresses would not provide a relocatable cross-process method identity.

**Decision evidence:** trace a method invocation, including dispatch resolution, a branch based on an effect result, a missing capability and an application error. Record the chosen semantics rather than merely the spelling of `apply`.

<a id="dl-03"></a>
### DL-03 — Regions, ownership and lifetimes

**Confirmed local direction:** user objects have only stack and `@region` carriers; only system code uses the native heap. Capture payloads are bits or regional pointers. There is no implicit heap promotion of user closures or regional-object copying. C-40 reserves cross-region references without choosing their representation or extending capture categories. H-99–H-101; [capture and lifetime discussion](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime).

**Regional access:** H-124 makes regions dynamically scoped resources operated on by procedures. H-127/H-129 permit a function to read the complete reachable structure, including pointer dereferences, through its explicit input values. The outermost function call and its nested function calls share one read-only snapshot; it expires when that outer call returns. Procedures may call functions; functions cannot call procedures. This is a single-threaded semantic read boundary, not graph copying, region freezing or permission for effects inside a function. See [D-10 storage semantics](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime).

**Question:** Which region owns every value and reference throughout packet application, querying and persistence?

- Distinguish packet construction/receive storage, persistent DBMR storage, query scratch and response storage. Define allocation, borrowing, mutation and region destruction rules.
- Decide how received data becomes durable: explicitly materialize a graph in DBMR, retain the received region as a persistent dependency, or provide separately specified alternatives. A short-lived packet reference cannot silently become a durable reference.
- Distinguish legal graph construction/materialization from copying or moving a regional C++ object. Define cycle/sharing preservation and the treatment of runtime-only resources.
- Specify when bindings or borrows pin a region and what happens on remapping, unloading and generation changes. Decide which guarantees the checker proves and which a runtime boundary verifies.

**Proposal to discuss:** materialization into DBMR for the first packet workflow, with temporary query/response regions. This is a scope choice, not a commitment to prohibit persistent multi-region graphs.

**Decision evidence:** a lifetime diagram for one request, including a rejected escaping reference, a result surviving packet disposal and a query whose temporary allocations can be released.

<a id="dl-04"></a>
### DL-04 — Packet lifecycle, framing and compatibility

**Question:** What exact bytes form a complete, interpretable packet, and when may its entry method run?

- Define total-length meaning, header encoding, occupied bytes versus reserved capacity, root location/type and dependency identification. Decide what is carried in the packet versus pinned in the connection context.
- Define partial reception, frame boundaries, minimum structural checks and rejection behavior. Validation should establish the conditions needed for correct interpretation and application; its depth and cost belong in the protocol contract.
- Choose the initial compatibility envelope: identical layout/ABI peers, a specified portable image, or explicit conversion between representations. Separate semantic/schema versions from layout and ABI versions; builds on multiple architectures do not establish image compatibility.
- Decide mutation rights during construction, after sending and after reception. Whole-region relocation preserves valid relative layouts; arbitrary truncation or object movement does not follow from that property.

**Proposal to discuss:** construction → sealing → complete reception → validation/binding → application. The total binary length is confirmed; sealing and this exact state model remain proposals.

**Decision evidence:** annotated packet layout, split/combined input examples, an incompatible root/dependency example and a declared byte-order, width and alignment policy for the chosen peers.

<a id="dl-05"></a>
### DL-05 — Application, commit and retry

**Question:** What observable event means that a packet has “taken effect”?

- Distinguish fully received, semantically accepted, applied in memory and durably committed. Choose what an acknowledgment promises and what response state can be queried after a lost connection.
- Define application failure, persistent-write failure and process failure separately. State the first fault model and any unsupported recovery cases.
- Decide request/execution identity, duplicate handling and retry behavior. No framing or member-call convention alone supplies exactly-once effects.
- Specify which changes can be rolled back and which external effects need retry, compensation or an explicit failure result. Relate this to single-region and cross-region commit boundaries.

**Decision evidence:** a small outcome table for failure before application, during application, after commit but before acknowledgment, and during retry. Select only the guarantees needed for the first business workflow.

<a id="dl-06"></a>
### DL-06 — Updates, visibility and history

**Question:** Does “only record” mean immutable appended versions, mutable current objects with monotonic allocation, or another explicitly defined model?

- Specify insertion, update, deletion and the visible view for reads/statistics. Decide whether deletion uses tombstones, whether prior versions are queryable and which state establishes the current version.
- Separate allocation policy from logical mutation, event history and audit logging. Clarify whether reads create records at all.
- Choose the initial writer/reader model, snapshot boundaries and index visibility. A single writer with bounded read views is one candidate; it is not an approved concurrency contract.
- Identify authoritative facts, retained history and rebuildable derived state. Specify which changes invalidate queries, bindings or compiled specializations.

**Proposal to discuss:** append new logical versions and deletion markers, then reclaim discarded history during rebuilding. Compare this against in-place logical updates with monotonic allocation before choosing.

**Decision evidence:** show the visible records, statistics and retained bytes after create → update → delete, including the same sequence across close/reopen.

<a id="dl-07"></a>
### DL-07 — Retrieval, statistics and indexes

**Question:** How does business code express queries, and what meaning, ownership and cost do their results have?

- Choose initial lookup, traversal, filtering and aggregate operations; decide whether they are ordinary typed methods/effects or require separate query syntax.
- Define result types, ordering where observable, visibility and errors. Decide whether results are borrowed views, iterators pinned to a snapshot, or independently owned response graphs.
- Specify index semantics, maintenance and rebuildability. Record when an index is merely an acceleration structure versus part of a required business constraint.
- Keep temporary query allocations distinct from permanent database growth; choose representative measurements before setting performance targets.

**Decision evidence:** lookup plus a filtered aggregate before/after updates and compaction, with explicit result lifetimes and a correct path when a rebuildable index is absent.

<a id="dl-08"></a>
### DL-08 — Identity, references and versions

**Question:** Which identities survive transport, independent region loading, compaction and definition changes?

- Distinguish domain entity, graph node, type/method definition, region, physical location, execution and effect-occurrence identities. Decide which require persistent identifiers.
- Define efficient intra-region references and resolvable cross-region references, including generation/version selection, dangling references and binding lifetimes. Do not make physical offsets the implicit business identifier.
- Decide how packets refer to existing entities when an intervening compaction moves them. Compare stable logical IDs, generation-qualified locations and forwarding/indirection where appropriate.
- Specify schema evolution and migration boundaries, plus how independent domain identities and mappings remain explicit. Type identity, schema revision and layout compatibility are separate questions.

**Proposal to discuss:** stable logical identity at externally retained boundaries, with compact relative references inside pinned regional layouts. The particular identifier, lookup structure and generation mechanism are open.

**Decision evidence:** independently reload two related regions, then compact one and resolve an earlier packet's target under the chosen version policy.

<a id="dl-09"></a>
### DL-09 — Compaction and backing-file publication

**Question:** Which retained semantic graph is rebuilt, and how does the new backing file become authoritative?

- Select roots, reachability and retention rules for current facts, history, tombstones, dependencies and live execution state. Retaining all history limits what compaction can discard.
- Define typed reconstruction and reference rewriting when objects change relative positions; preserve required identities, sharing and cycles. Whole-region relocation and graph compaction are different operations.
- Choose when compaction may run: a maintenance boundary with quiescent users, snapshot-based rebuilding with catch-up, or another stated model. Define active readers and queued requests during publication.
- Specify candidate-file completion, publication, reopening and eventual old-file removal under the chosen crash/durability model. File replacement alone does not define the complete recovery protocol.

**Proposal to discuss:** first prove rebuilding at a maintenance boundary, avoiding an unchosen online-compaction protocol. This would bound the initial implementation without fixing the long-term concurrency model.

**Decision evidence:** retained facts/identities and query results agree before/after rebuilding; discarded state no longer occupies the new payload; interrupted publication has the outcome promised by DL-05.

<a id="dl-10"></a>
### DL-10 — Source, diagnostics and editing

**Question:** How do humans and tools author, inspect and modify the same semantics that DBMR stores and execution consumes?

- Decide surface notation after representative semantic examples exist. Existing YAML conversions are useful references, not an automatically selected language syntax.
- Distinguish readable partial projections from complete reconstructible source. Define how identity, sharing, cycles, bindings, imports, versions, comments and provenance survive a full round trip.
- Connect diagnostics, completion, hover, definition navigation and semantic edits to the same checked definitions. Distinguish source locations from stable semantic identity.
- Specify a bounded first editing workflow, including version conflicts and failed checks. Decide what is needed for the first language milestone versus later IDE integration.

**Decision evidence:** render and reload a shared graph with imports; make one text edit and one equivalent structural edit; retain meaning and useful diagnostics. The current C++ clangd acceptance does not establish these future DSL capabilities.

<a id="dl-11"></a>
### DL-11 — C/C++ and other runtimes

**Question:** What interoperability contract exposes useful operations while preserving regional and language semantics?

- Specify ownership of buffers/regions, borrowed-view lifetimes, allocation/deallocation symmetry, error results, calling threads and callbacks. Separate physical image compatibility from a function-call ABI.
- Decide which foreign values must be materialized and which runtime handles can be temporarily bound. Native heap pointers, callbacks and operating-system handles need an explicit execution-lifetime treatment.
- Define how external hosts construct/check packets, invoke methods, consume responses and bind effects without bypassing the chosen invariants.

**Proposal to discuss:** a small C ABI with opaque handles and explicit buffer/ownership contracts, plus a C++ wrapper. Direct C++ integration remains an alternative for comparison; the user has approved interoperability's role, not this exact ABI structure.

**Decision evidence:** trace one foreign-runtime caller through construction, application, result consumption and release, including an error and a rejected lifetime violation.

<a id="dl-12"></a>
### DL-12 — Initial scope and execution strategy

**Question:** What is the smallest language and runtime implementation that can demonstrate the selected semantics without making the host language their only definition?

- Use direct LLVM IR as the sole execution lowering path under H-143. Keep source semantics and future persistent semantic graphs distinct from derived .ll/.bc artifacts; use LLVM's standard optimization pipeline without a multi-backend framework or custom micro-optimizations.
- Decide which typed representation, checking rules and handler interfaces are shared. Any layout, validator, traversal or compactor generation should follow one defined type contract rather than diverging handwritten descriptions.
- Bound syntax, ADT/GADT features, effects, query primitives and FFI for an initial version. Distinguish deliberate omissions from restrictions that would prevent multi-domain integration or later type/kind work.
- H-106 separates complete dependency scanning/registration from entry-driven compilation of execution semantics. Top-level procedure declarations do not execute during scanning; dependency execution semantics are incorporated in encounter order.
- Keep program intent separate from native code and caches. H-96/H-102 specify demand-driven whole-program compilation, with the complete caller paths available and no independent module compilation. Collect required symbolic variables, check binding availability/types, then lay out direct per-thread access. Concrete lowering, specialization and cache dependencies remain to be specified; this does not select machine-code JIT.

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

**Confirmed:** module-defined symbols have strict identity, imported aliases retain it, and independent origins/versions remain distinct. Symbol-variable use declares the per-thread variable. Reads use `@name`; writes use `effect @name = value`, restoring on scope exit. Procedures capture used effect values; `perform` temporarily installs them, `behave` retains current bindings. Demand-driven WPO checks required bindings over caller paths; missing values are compilation errors, not runtime fallback.

**Confirmed module practice:** vocabulary modules declare symbols with comments. `using` retains an external procedure's source definition; `import` redefines its body in the current module's lexical/dynamic scopes. This definition choice is separate from `perform`/`behave`. H-105; D-10.

**Confirmed module phases:** H-106/H-107 permit symbol/function definitions and bits globals initialized by literals or function calls, plus top-level `effect`/`perform`/`behave` expressions. Procedure definitions register during scanning; neither their definition semantics nor global initializers/top-level expressions execute during that scan. All entry dependencies are scanned/loaded before entry execution is compiled; dependencies then contribute top-level execution semantics in encounter order.

**Confirmed immediate review:** every cross-module reference triggers module execution semantics (H-109). Its top-level scope ends when execution finishes; only `using`/`import` bring effective resources into the receiver (H-110). `effect` yields the newly bound value (H-111).

**Open:** symbol identity encoding, typed-use reconciliation, precise capture/dependency analysis, cyclic initialization, bits-global realization and native layout/caching. These details are tracked in [SymbolicDynamicScope](SymbolicDynamicScope.md), D-10 revision 13.

**Origin/alias review:** H-112 requires explicit provenance and local aliases. H-113 excludes lexical symbolic variables and ordinary assignment. H-114 makes identities compile-time values and aliases compile-time names; runtime values bind under identities. H-115 permits an import-body alias map with matching-name defaults. H-116 requires a preexisting local destination symbol for `using/import "module" @source as @local`. [D-10's relationship model](SymbolicDynamicScope.md#symbol-origins-aliases-and-dynamic-values) separates identity selection, procedure-value installation and body translation. H-117 proposes `symbol from "module" @source as @local` and `with { @local as @source_role; }`, read as supplying a local symbol for the source body’s role. H-118 authorizes implementing these discussed forms. The compiler separates identity selection from procedure installation and resolves each imported body’s aliases before capture analysis. Broader mapping edge policies remain open; unsupported cases receive explicit diagnostics.

**Using abbreviation:** H-119 proposes `using symbol "module" @p;` to select identity and procedure together. H-120 requires an already bound same-name alias to have the source identity; otherwise ordinary using must explicitly select the destination with `as`, even `using "module" @p as @p;`. Shared vocabulary identities permit short forms. Explicit destination selection does not merge identities. The [D-10 rule table](SymbolicDynamicScope.md#combined-using-and-identity-checked-abbreviation) separates absent/same/distinct identity cases and preserves one source encounter. H-121 authorizes implementation and validation; the compiler implements this refinement with separate explicit-destination and identity-preserving cases. Import-body maps retain H-115 semantics; extending the outer rule to import is a separate open scope.

**Decision evidence:** alias/version separation, nested effects, both call modes, transitive `behave` dependencies, a reachable path missing a required binding, scan-time absence of effects/captures and an encounter order different from dependency scan order. No independent module compiler is presumed.

<a id="dl-15"></a>
### DL-15 — Callable definitions, expressions and storage

**Confirmed:** functions are lexically defined and use explicit inputs/outputs; procedures are dynamically defined with symbol names and may access effects. Module initialization is the top-level dynamic scope. Symbol reads/writes are expressions; the current direction uses expressions with a special `void` type for deliberately returning no meaningful value. User storage is stack or `@region`, with bits/regional-pointer captures and no native-heap object lifetime. Cross-region references remain an explicit design placeholder.

**Open:** expression sequencing and scope results, procedure/capture representation and lifetime checking, the first regional GADT construction contract, and cross-region scenarios. A scope-local `effect` restores on exit under either call mode; `behave` does not make rebinding permanent. No `malloc` closure, raw stack reference capture or user-visible host address is inferred.

**Decision evidence:** lexical function lookup versus dynamic procedure rebinding, effect access rejected in functions, expression result typing, scope restoration, and capture/lifetime examples. H-98–H-104, D-10.

<a id="dl-16"></a>
### DL-16 — Language and SNN development together

H-96 establishes joint development; H-108 requests a checkpoint/push and an autonomous goal for the first demand-compiled runnable prototype after reviewing immediate choices. H-109–H-111 resolve that review; [D-10's first-prototype boundary](SymbolicDynamicScope.md#first-native-prototype-boundary) identifies the milestone. The application-owned [LanguageCoDesign](../applications/steady-snn/docs/LanguageCoDesign.md) proposes increments from nominal symbols and call modes to one immutable-A/incoming-gather/B-publication step, then a small checked program representation and native specialization. H-94's target-owned two-frame pull is the application boundary; it does not fix neuronal equations or training algorithms.

**Current increment:** H-122 requests a ten-hour autonomous goal after immediate choices are discussed; H-123 limits it to language/runtime programming support with structured types, static regional storage and index-conversion capability. SNN modeling and learning are excluded from this run. H-125/H-126 request scalar arithmetic design review; H-132 defers compound-UoM implementation. H-133–H-138 resolve the immediate choices below for the bounded run. Application-shaped arithmetic fixtures can validate programming facilities without becoming model equations. The native prototype now supports named bits products, FP32/FP64 values, exact method dispatch, parenthesized operator names and named UoMs with selected pure conversion routes. Bounded native loops/reductions, compact integer representations, checked numeric casts and a multi-module typed pool-index fixture are also implemented. The native subset includes fixed-capacity regional allocation, nominal regional records, typed SoA arrays, pure input-reachable pointer traversal, returned existing references and repeated multi-module A/B gathers. Whole-image relocation and caller-scope lifetime checks are validated. Source syntax and owning-region specialization are bounded prototype conventions; no full storage ABI, cross-region representation or modeling acceptance is implied. H-139 adds complex dependency graphs and deep calls; H-140 specifies once-only incorporation after scanning; H-141 prioritizes a simple inlined procedure implementation with measured size over immediate machine-code reuse.

Numerical source evaluation and integration motivate pure functions with explicit inputs. Target/frame context, selected randomness, regional writes and publication motivate procedures. Each increment needs semantic examples and actual execution evidence before optimization or SNN learning success can be claimed. The [case file](../applications/steady-snn/research/cod-language-scope-cases.json) links selected [native fixture evidence](../tests/cod/language/validation.json). Sixty-six source fixtures and one frontend-only check establish the bounded scalar/module path, identity selection, import maps and a two-target gather; the complete case catalog, regional frame implementation and neuronal behavior remain unvalidated.

<a id="immediate-increment-decisions"></a>
#### Immediate increment decisions

H-133–H-138 resolve the immediate review without requiring another lifetime mechanism or compound-unit algebra:

| ID | Decision | Current rule |
| --- | --- | --- |
| LI-01 | Result references from a pure function | H-133: the calling procedure scope, including module top level, keeps inputs alive around the call. Returning an existing input-reachable reference uses that ordinary lifetime; outer snapshot expiry does not destroy the object. |
| LI-02 | UoM definition reuse | H-134: fresh definitions and one-shot using only; no UoM import. Select the intended UoMs and their associated conversions together. Partial selections exclude conversions involving omitted UoMs; repeated using does not extend the bundle. |
| LI-03 | Conversion during method selection | H-135: `amount (m)` forbids inserted conversion; `amount (m*)` permits it. This is a user-declared parameter policy, not a universal language restriction. |
| LI-04 | Conversion route selection | H-136: discover routes at compile time, choose a unique route automatically, warn about unresolved alternatives and allow warnings-as-errors. H-137/H-138: a direct conversion wins; otherwise use the declared indirect route, illustrated by `m*<-cm<-mm`. |

The run develops structured product values, named UoMs with these selection/conversion rules, Julia-style argument-type methods and parenthesized operator names, bounded numerical iteration, static SoA regional arrays and pure snapshot traversal. Regional references retain existing lifetime/relocation rules. Compound-unit algebra, general GADT machinery, custom operator precedence and cross-region representation remain separate increments. No new SNN dynamics or learning rule enters this work.

Validation uses CoD language/runtime component checks and arithmetic fixtures: strict versus convertible parameters, using-bundle selection, route preference/warnings, structured index conversion, rejected domain mismatches, nested snapshot reads, returned input references, and repeated numerical gather/A-B execution. Existing numeric failure behavior supplies a reference for invalid arithmetic; no new exception/recovery framework is required. Source-only compilation uses the repository's active build tree and normal build command; build configuration and toolchains remain unchanged.

## Implementation evidence to account for

Storage observations below describe repository commit `b72669e5d2bd9374e3aa69d6dd2c4ca1a5ca94c6`. The driver row links the current native prototype with hashed execution evidence. Implementation observations do not grant design authority.

| Existing facility | Relevant observation | Topics |
| --- | --- | --- |
| [Regional storage](../include/shilos/region.hh) | `regional_ptr` is self-relative; allocation advances `occupation_`; capacity and occupied bytes are distinct. The current `RegionalType` concept is a destructibility check, not a recursive proof of legal regional fields. | DL-01, DL-03, DL-04 |
| [Cross-region runtime pointer](../include/shilos/region.hh) | `global_ptr` holds a native reference to a region and an offset. A persistent/process-independent identity and resolution protocol still needs a design. | DL-03, DL-08 |
| [DBMR](../include/shilos/dbmr.hh) | Mapped-region validation checks header/root conditions. `constrict_on_close` trims unused tail capacity; it does not rebuild a graph to reclaim unreachable internal allocations. | DL-04, DL-06, DL-09 |
| [CoD driver](../cod/main.cc) | The C++ submission path remains; the added CoD source path parses/checks the bounded scalar/module language before native compilation. [Prototype reference and evidence](../tests/cod/language/README.md) identify its limits; no full ADT/GADT or regional source-language implementation is claimed. | DL-02, DL-12 |

The [memory-region design](MemoryRegionAndRegionalTypes.md), [CoD driver design](CompileOnDemand.md) and [YAML conversion design](YamlConversionsOfRegionalTypes.md) remain relevant references with the qualification limits listed in the registry. Reusing code does not select its representation as the language's contract.

## Discussion order and decision history

Application design proceeds in its separate document set, linked from DL-13. Language decisions are recorded at their confirmed scope; application proposals become reusable language requirements only through an explicit scope and value decision.

For each discussion, update the relevant topic's current answer and append a decision entry. A useful entry contains the proposition and scope, alternatives and trade-off, human statement/source, affected topic and relationship IDs, revision coverage and remaining questions. Registry H-events hold confirmation evidence; an assistant proposal stays a proposal until explicitly decided. Replace decisions with linked new entries instead of erasing their history.

| Entry | Recorded date | Outcome / scope | Evidence |
| --- | --- | --- | --- |
| DD-001 | 2026-10-22 | IPC entry application, local DBMR operations with deferred reclamation, explicit language semantics and C/C++ interoperability are the near-term direction. Concrete protocols and representations remain open. | H-06; goals G-01–G-04 |
| DD-002 | 2026-10-22 | Start language design and maintain this discussion tracker; defer implementation until the user chooses the appropriate point. No generated mechanism proposal is thereby approved. | H-07; goal G-05 |
| DD-003 | 2026-10-23 | Select steady recurrent SNN activation as the first application to discuss. The application document set owns its model and acceptance questions. | H-08; DL-13; D-08 |
| DD-004 | 2026-10-23 | Keep application design in a separate document set; retain an example link and reusable language topics here. | H-11; DL-13; D-08 |
| DD-005 | 2026-11-05 | Advance language/SNN design, implementation and validation together; introduce nominal symbols, scoped per-thread effects, WPO and two procedure invocation modes. Supersedes DD-002's development deferral for this coordinated direction; detailed mechanisms remain scoped. | H-96; DL-12–DL-16; D-10 |
| DD-006 | 2026-11-05 | Capture used effect values; `perform` temporarily installs them and `behave` respects current dynamic bindings. | H-97; DL-14 |
| DD-007 | 2026-11-05 | Functions use explicit inputs/outputs and cannot access effects or invoke procedures; procedures own effect access. | H-98; DL-02, DL-15 |
| DD-008 | 2026-11-05 | User storage is stack/`@region`; captures are bits/regional pointers. Native heap belongs only to system code. | H-99/H-100; DL-03, DL-15 |
| DD-009 | 2026-11-05 | Keep a cross-region reference placeholder and progressively develop its scenarios; no representation selected. | H-101; DL-03, DL-08 |
| DD-010 | 2026-11-05 | Check missing required bindings as compilation errors over demanded whole-program caller paths; do not compile modules independently. | H-102; DL-12, DL-14 |
| DD-011 | 2026-11-05 | Functions are lexically defined; procedures are dynamically defined under symbols. Module initialization is the top-level dynamic scope. | H-103; DL-14, DL-15 |
| DD-012 | 2026-11-05 | Read with `@name`, write only with `effect`; both are expressions. Pursue an expression-oriented language with explicit `void`; detailed expression-result rules remain open. | H-104; DL-15 |
| DD-013 | 2026-11-05 | Vocabulary modules contain symbols and comments; `using` retains a source procedure definition, while `import` redefines its body in the receiving lexical/dynamic context. | H-105; DL-14; D-10 |
| DD-014 | 2026-11-05 | Limit ordinary top-level definitions to symbols, functions and literal bits globals; register top-level procedures without scan-time execution. Scan/load all entry dependencies first, then compile entry execution and dependency top-level execution semantics in encounter order. | H-106; DL-12/DL-14; D-10r2 |
| DD-015 | 2026-11-05 | Bits global initializers must be bits literals or function calls. Top-level effect/perform/behave expressions are permitted; their execution belongs to module execution, preserving scan-only registration and entry-driven composition. | H-107; DL-12/DL-14; D-10r3 |
| DD-016 | 2026-11-05 | Review immediate choices, then checkpoint/push and pursue an autonomous goal to a prototype that compiles and runs on demand. This is implementation authorization, not approval of all open language mechanisms. | H-108; DL-16 |
| DD-017 | 2026-11-05 | Every kind of cross-module reference triggers source-module execution semantics, including vocabulary symbols and imported bodies. | H-109; DL-14 |
| DD-018 | 2026-11-05 | Source-module top-level scope ends after execution; only using/import bring effective resources into the current module. Source effects do not stay ambient in the receiver. | H-110; DL-14 |
| DD-019 | 2026-11-05 | An effect expression evaluates to its newly bound value. | H-111; DL-15 |
| DD-020 | 2026-11-05 | Review explicit symbol origins and local aliases through syntax, semantics and practical use. No concrete new spelling or resource-selection mechanism is approved by this request. | H-112; DL-14; D-10r6 |
| DD-021 | 2026-11-05 | Symbolic variables have dynamic scope only; ordinary `@x = 2` is invalid and assignment requires `effect @x = 2`. Source aliases select identity without creating lexical symbol variables. | H-113; DL-14/DL-15; D-10r6 |
| DD-022 | 2026-11-05 | Symbol identity is a compile-time value; module-local aliases exist only at compile time. Runtime values bind under symbol identities. | H-114; DL-14; D-10r6 |
| DD-023 | 2026-11-05 | Procedure import permits a symbol-alias mapping table; matching source/local aliases need no explicit mapping entry. Exact table spelling and edge rules remain open. | H-115; DL-14; D-10r6 |
| DD-024 | 2026-11-05 | Permit `using/import "<modu>" @src_name as @local_name`; the destination symbol must already be declared locally or introduced from a vocabulary module. This selects where the procedure value is installed, not a fresh alias of the source identity. | H-116; DL-14; D-10r6 |
| DD-025 | 2026-11-05 | Propose `symbol from "module" ...` for vocabulary identity selection and review `with { @local as @source_role; }` as supplying the receiving symbol in the imported body’s role. The working draft uses this direction; full grammar and remaining edge policies are not thereby approved. | H-117; DL-14; D-10r7 |
| DD-026 | 2026-11-05 | Implement the discussed symbol-from selection, predeclared procedure destinations and receiving-symbol-as-source-role import maps; run tests, then commit and push. This authorizes the bounded increment, not unresolved full-language mechanisms. | H-118; DL-14–DL-16; D-10r8 |
| DD-027 | 2026-11-05 | Propose one-line `using symbol "std:io" @print;` and assess its semantic risks. The discussion interprets it as selecting the source identity and procedure together; H-120 supplies the existing-name rule. | H-119; DL-14; D-10r9 |
| DD-028 | 2026-11-05 | When a local same-name symbol already has a compile-time identity, abbreviated using requires equality with the source identity. A mismatch is a compilation error unless ordinary using explicitly selects the destination, including `as @print` with unchanged spelling. Shared vocabulary identities retain short forms. | H-120; DL-14; D-10r9 |
| DD-029 | 2026-11-05 | Implement and validate the discussed combined using and identity-checked abbreviation, then commit and push. This is direct authorization for that bounded increment, not approval of unresolved import or full-language policies. | H-121; DL-14–DL-16; D-10r10 |
| DD-030 | 2026-11-05 | Discuss decisions before starting a ten-hour autonomous language/application goal. | H-122; DL-16 |
| DD-031 | 2026-11-05 | Limit this run to useful CoD language/runtime support, including structured regional types and consideration of scale-law index conversions; exclude SNN modeling and learning. | H-123; DL-01/DL-03/DL-16 |
| DD-032 | 2026-11-05 | Regions are dynamic resources available to procedures. Functions receive dereferenced values and may read their complete reachable graph in a pure snapshot. | H-124/H-127; DL-02/DL-03/DL-15; D-10r11 |
| DD-033 | 2026-11-05 | Require strict branded types, array-specific index-type declarations and meaningful scalar arithmetic; compare UoM and newtype mechanisms before choosing. | H-125/H-126; DL-01; D-11r1 |
| DD-034 | 2026-11-05 | UoMs are named compile-time entities with independent first-class type identities; same names from different definitions may denote different entities, local aliases preserve the selected identity, and symbols do not represent them. | H-128; DL-01; D-11r1 |
| DD-035 | 2026-11-05 | Procedures may call functions, functions cannot call procedures; nested functions share the outermost function call's read-only snapshot of input-reachable external objects, and that snapshot expires on return. | H-129; DL-02/DL-03/DL-15; D-10r11 |
| DD-036 | 2026-11-05 | UoM using preserves identity; import copies the definition into a fresh local identity. Conversion formulas are pure, and array/conversion integration uses Julia-style function parameter-type dispatch. | H-130; DL-01/DL-02; D-11r1 |
| DD-037 | 2026-11-05 | Include parenthesized operators as magic-method names so operator overloads are readable and convenient to define. Complete grammar and indexing/control forms remain under discussion. | H-131; DL-01/DL-02/DL-15; D-11r2 |
| DD-038 | 2026-11-05 | Plan compound UoMs for future physical-quantity calculations, without requiring immediate implementation; review the remaining decisions worth settling before the autonomous increment. | H-132; DL-01/DL-16; D-11r3 |
| DD-039 | 2026-11-05 | Function inputs live around the call in its enclosing procedure/module-top-level scope; returning an existing input reference uses that lifetime without another snapshot mechanism. | H-133; DL-02/DL-03/DL-15; D-10r12 |
| DD-040 | 2026-11-05 | UoMs support fresh definition and one-shot using selections with their conversions, not definition import. Partial selection excludes conversions involving omitted units; repeated using is disallowed. | H-134; DL-01; D-11r4; supersedes H-130's UoM-import choice |
| DD-041 | 2026-11-05 | Parameter syntax controls automatic conversion: `amount (m)` disallows insertion and `amount (m*)` permits it. | H-135; DL-01/DL-02; D-11r4 |
| DD-042 | 2026-11-05 | Discover conversion routes at compile time; choose a unique route, warn for unresolved alternatives, and allow a compiler option to make the warning an error. | H-136; DL-01/DL-02; D-11r4 |
| DD-043 | 2026-11-05 | Source syntax can prioritize an indirect conversion route, illustrated by `amount (m*<-cm<-mm)`; direct conversion is preferred whenever available. | H-137/H-138; DL-01/DL-02; D-11r4 |

| DD-044 | 2026-11-05 | Deliberately stress multiple modules, complex dependencies and deep calls to expose and fix design/implementation issues. | H-139; DL-14/DL-16 |
| DD-045 | 2026-11-05 | After scanning/loading, incorporate module top-level semantics into the result program in encounter order, once per module; encounters do not immediately execute user code. | H-140; C-44/P-104; D-10r13 |
| DD-046 | 2026-11-05 | Retain initial procedure inlining while generated size remains reasonable; shared machine-code fragments are a medium/long-term optimization. | H-141; DL-16; D-10r13 |
| DD-047 | 2026-11-05 | Use LLVM IR as the sole CoD execution intermediate; no generated-C++ backend or multi-backend machinery, and no micro-optimization work. | H-142/H-143; DL-12; D-10r15 |
| DD-048 | 2026-11-05 | Judge tests by critical semantic contracts and failure consequences; consolidate overlap and reduce implementation/test debt as work advances. | H-144/H-145; DL-12–DL-16 |

Dates above are recording dates. Exact source-message timestamps were unavailable; the registry records that provenance limit.
