# Domain language design: discussion and decision tracker

Discussion revision: 3

Recorded: 2026-10-22

Current discussion: 2026-10-23

Phase: design discussion; implementation deferred by the user.

## Design authority

- **Registry:** [D-07](DesignAuthorityRegistry.md#d-07), which controls qualification and scope.
- **Revision / covered scope:** discussion revision 3, all sections of this language tracker. Goals record human direction; topic proposals and candidate acceptance criteria remain `pending-human`. The application has a separate D-08 document set.
- **Value lineage:** candidate P-12–P-18 connect the packet, database, language and interoperability concepts C-08–C-11 and this document to C-01, C-02, C-05 and C-06. Explicit language semantics should preserve inspectable business intent; regional transport and deferred reclamation should serve efficient persistent semantics; source tools should keep that semantics readable. These proposed derivations are not yet qualified implementation authority.
- **Dependencies:** P-19–P-21 identify C-03, C-04 and C-02: multiple regions/domains, semantic levels and environment-specific effects. P-24 records application C-12 as an external example for DL-13; its model and resource choices are scoped to D-08. Existing C++ designs are reference material with their own registry status.
- **Contract boundaries:** see “Scope and invariants”; candidate acceptance and implementation boundaries are in DL-13. No language syntax, ABI, storage protocol or implementation contract is adopted by this revision.
- **Human baseline:** H-06 supplies the near-term goals; H-07 chooses language design and discussion tracking now, with development later; H-08 selects the first application; H-11 requires its own document set. These events do not approve the generated document's detailed proposals.
- **Continuity:** no qualified implementation baseline for D-07. K-02 and K-03 record discussion maintenance through revision 3, retaining `pending-human`; registry maintenance follows the same event records.

## Confirmed direction

These are summaries of the user's statements, not inferred approval of the preceding assistant's recommendations. The original statements and their provenance are recorded in [H-06](DesignAuthorityRegistry.md#h-06), [H-07](DesignAuthorityRegistry.md#h-07).

| Goal | Confirmed direction | Discussion coverage |
| --- | --- | --- |
| G-01 | Use a memory region (`mem_region` in the discussion) as an IPC packet. Its header includes the total binary length. After complete reception, invoke a designated member method on the packet's entry object to apply it. | DL-02, DL-04, DL-05 |
| G-02 | Use DBMR as the local ontology database, supporting business retrieval, statistics and CRUD. Ordinary operations do not reclaim space; periodically rebuild its payload and backing file for compaction. | DL-06–DL-09 |
| G-03 | Give the domain language explicit ADT semantics, including sum/product types, method invocation and memory management. | DL-01–DL-03 |
| G-04 | Provide C/C++ compatibility interfaces for interoperability with other language runtimes. | DL-11 |
| G-05 | Begin language design now, keep a discussion/decision document, and start implementation when the timing is appropriate. | All topics; DL-12, DL-13 |

“Only record” has not yet settled whether updates append immutable versions or mutate existing objects while allocation remains monotonic. It does not by itself require logging every read. “Apply” has not yet specified a durability guarantee, retry protocol or native method-pointer representation.

## Scope and invariants

The discussion joins language semantics, IPC entry execution, DBMR operations and compaction into one coherent design. It also tracks how the language relates to the charter's typed business graphs, ontology, effects, multi-region integration and readable source.

The governing context is [the charter](RegionalSemanticPlatformCharter.md), especially §§2–8, 11–14, with the human-confirmation limits recorded under D-00. Repository rules require zero-cost whole-region relocation, region-only allocation for regional objects, no regional copy/move semantics, and explicit region-aware references and lifetimes. A language proposal must explain how it preserves these constraints. They do not imply that object-by-object compaction is free.

This discussion does not authorize code development, toolchain changes, builds or tests. It does not select a concrete parser, type theory, transport, transaction model, concurrent collector, wire ABI, query language or code-generation dependency. The scope of the first language version is open; broader GADT, kind-level and multi-domain goals remain in view when choosing a smaller initial scope.

`apply`, `Result`, `execution_context` and similar names below are explanatory notation, not chosen syntax or API names. A C++ runtime, generated C++ or a reference interpreter are implementation candidates. C++ compatibility does not determine the domain language's semantics.

## Discussion dashboard

**Status vocabulary:** `open` means discussion is needed; `proposed` means a candidate answer is recorded; `confirmed` requires an explicit human decision for the stated scope/revision; `superseded` preserves an older decision and links its replacement. Topic progress and implementation qualification are separate: a topic can contain a useful proposal while its design authority remains `pending-human`.

All thirteen language topics are currently **open**, with proposals where indicated. DL-13 links the separately maintained first application.

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

## Topic records

<a id="dl-01"></a>
### DL-01 — Semantic units, ADTs and ontology

**Question:** What does the language define and preserve: domain types and values, method/program graphs, ontology relations, or which initial subset of them?

- Decide nominal versus structural identity, sum constructors and product fields, recursive types, pattern matching/exhaustiveness, equality and admissible regional fields. Keep semantic type identity separate from physical layout.
- Distinguish ordinary ADTs from indexed GADTs. Specify the initial role of type parameters, kind checking, inference and type equivalence without silently treating the full long-term type theory as already selected.
- Decide which ontology relations and constraints are first-class, what is explicitly asserted versus inferred, and how independently defined domain concepts can be mapped. Similar field layouts do not establish conceptual equivalence.
- Explain how domain signatures can be loaded and checked without requiring every domain constructor to become a fixed host-language enum. Distinguish published definitions from parser/solver scratch state.

**Proposal to discuss:** start from a small explicit sum/product and method core whose checked representation can express business logic and evolve toward the charter's GADT graphs. Decide the necessary indexed features from concrete programs.

**Decision evidence:** one domain definition, one business program with a result-dependent branch, an invalid construction that is rejected, and the exact boundary between value, type and kind representations.

<a id="dl-02"></a>
### DL-02 — Methods, effects and entry execution

**Question:** What is a method's meaning, and how does the receiver select and execute the entry object's designated method?

- Specify receiver binding, arguments/results, dispatch, method identity/version and access to the execution context. Decide whether definitions travel with a packet, are imported, or are resolved from already loaded definitions.
- Separate pure computation from effects such as database reads/writes and external operations. Define ordering, observable errors and capability binding; decide whether the first version needs suspension, persistent continuations or cancellation.
- Keep business decisions expressible in the typed program representation. Identify which primitives are supplied by native handlers and how two environments can interpret the same business program.

**Proposal to discuss:** an illustrative `entry.apply(context) -> Result<Response, ApplyError>` contract, with methods resolved against explicit definitions and bindings. Serialized native function/member-function addresses would not provide a relocatable cross-process method identity.

**Decision evidence:** trace a method invocation, including dispatch resolution, a branch based on an effect result, a missing capability and an application error. Record the chosen semantics rather than merely the spelling of `apply`.

<a id="dl-03"></a>
### DL-03 — Regions, ownership and lifetimes

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

- Compare a checked semantic representation with a reference interpreter, generated C++ and direct LLVM lowering. Identify their development cost and what each can validate.
- Decide which typed representation, checking rules and handler interfaces are shared. Any layout, validator, traversal or compactor generation should follow one defined type contract rather than diverging handwritten descriptions.
- Bound syntax, ADT/GADT features, effects, query primitives and FFI for an initial version. Distinguish deliberate omissions from restrictions that would prevent multi-domain integration or later type/kind work.
- Keep program intent separate from native code and caches. Choose AOT/JIT and specialization only with an explicit dependency/version and lifetime contract.

**Proposal to discuss:** a small semantic core and reference execution path, with C++ supplying runtime primitives; native optimization follows evidence from a working business flow. No backend or new dependency is selected.

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

**Development boundary:** implementation remains deferred. When the user chooses to begin a bounded portion, record its explicit decisions, value derivation, invariants, interfaces, non-goals and acceptance criteria as a qualified contract. Unrelated open topics need not all be solved first, but the selected portion's dependencies and unresolved limits must be clear. Build and test authorization remains governed by repository rules.

## Implementation evidence to account for

These observations describe repository commit `b72669e5d2bd9374e3aa69d6dd2c4ca1a5ca94c6`; they are inputs to discussion, not proof that a language mechanism is already implemented or approved.

| Existing facility | Relevant observation | Topics |
| --- | --- | --- |
| [Regional storage](../include/shilos/region.hh) | `regional_ptr` is self-relative; allocation advances `occupation_`; capacity and occupied bytes are distinct. The current `RegionalType` concept is a destructibility check, not a recursive proof of legal regional fields. | DL-01, DL-03, DL-04 |
| [Cross-region runtime pointer](../include/shilos/region.hh) | `global_ptr` holds a native reference to a region and an offset. A persistent/process-independent identity and resolution protocol still needs a design. | DL-03, DL-08 |
| [DBMR](../include/shilos/dbmr.hh) | Mapped-region validation checks header/root conditions. `constrict_on_close` trims unused tail capacity; it does not rebuild a graph to reclaim unreachable internal allocations. | DL-04, DL-06, DL-09 |
| [CoD driver](../cod/main.cc) | The current submission path generates and compiles a C++ runner. It is an execution facility, not an implementation of the proposed domain language. | DL-02, DL-12 |

The [memory-region design](MemoryRegionAndRegionalTypes.md), [CoD driver design](CompileOnDemand.md) and [YAML conversion design](YamlConversionsOfRegionalTypes.md) remain relevant references with the qualification limits listed in the registry. Reusing code does not select its representation as the language's contract.

## Discussion order and decision history

Application design proceeds in its separate document set, linked from DL-13. Language decisions remain open; application proposals become reusable language requirements only through an explicit scope and value decision.

For each discussion, update the relevant topic's current answer and append a decision entry. A useful entry contains the proposition and scope, alternatives and trade-off, human statement/source, affected topic and relationship IDs, revision coverage and remaining questions. Registry H-events hold confirmation evidence; an assistant proposal stays a proposal until explicitly decided. Replace decisions with linked new entries instead of erasing their history.

| Entry | Recorded date | Outcome / scope | Evidence |
| --- | --- | --- | --- |
| DD-001 | 2026-10-22 | IPC entry application, local DBMR operations with deferred reclamation, explicit language semantics and C/C++ interoperability are the near-term direction. Concrete protocols and representations remain open. | H-06; goals G-01–G-04 |
| DD-002 | 2026-10-22 | Start language design and maintain this discussion tracker; defer implementation until the user chooses the appropriate point. No generated mechanism proposal is thereby approved. | H-07; goal G-05 |
| DD-003 | 2026-10-23 | Select steady recurrent SNN activation as the first application to discuss. The application document set owns its model and acceptance questions. | H-08; DL-13; D-08 |
| DD-004 | 2026-10-23 | Keep application design in a separate document set; retain an example link and reusable language topics here. | H-11; DL-13; D-08 |

Dates above are recording dates. Exact source-message timestamps were unavailable; the registry records that provenance limit.
