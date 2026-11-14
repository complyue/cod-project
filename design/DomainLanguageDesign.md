# Domain language design: discussion and decision tracker

Discussion revision: 154

Recorded: 2026-10-22

Current discussion: 2026-11-13

Phase: coordinated language/SNN design, implementation planning and validation.

## Design authority

- **Registry / current scope:** [D-07](DesignAuthorityRegistry.md#d-07); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole file pending-human.** H-06/H-07 establishes the tracker; C-08–C-11/P-12–P-21 supplies candidate paths. Local decisions route through D-10/D-11/D-14–D-18/D-21–D-24.
- **Value path:** C-10 supplies inspectable typed execution. C-35/C-43/C-45/C-46 through P-84/P-100/P-102/P-109–P-111 and C-56/C-57 through P-182–P-197 concern identity, composition, interfaces and explicit behavior selection. C-39/C-54 through P-94/P-143/P-155 supplies regional lifetime boundaries. C-58/P-199–P-202 adds explicit publication and module-scoped callable composition. These candidate paths serve charter §§2–4; dependency evidence does not qualify them.
- **Contract boundary:** G-01–G-07, DL-01–DL-16 and DD-001–DD-151 retain the decision/acceptance inventory. H-351/H-352 settles library graph-transfer responsibility and IPC/DBMR planning directions, not protocols, ABI, concurrency, snapshot failure or retention. Past work-period permissions are not a new task.
- **Invariants / non-goals:** preserve zero-cost relocation, region-only non-bits storage, no regional object copy/move, owner provenance, non-consuming capture replay and no dynamic effect/procedure calls from functions. No module namespace, runtime interface wrapper, allocator policy, bootstrap/toolchain change or SNN modeling choice is introduced. The confirmed method-selection increment does not settle qualifiers, ownership construction/transfer, standalone abstract signatures or mailbox semantics.
- **Acceptance:** Examples must distinguish source/caller contexts, visibility, conformance, use/extension permission, unique/ambiguous dispatch and demanded/unused implementations while preserving lifetimes and relocation. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## Declaration annotations

Type annotations use one colon: `val count:i64`, `ref node:Node in O`, `symbol @limit:u64`, `region arena:Root` and `where {region r:NodeStore}`. Callable result annotations use an arrow, such as `function size(node:Node in O) -> const u64 where {owner O}`. Double-colon syntax rejects with a migration diagnostic. `<:` remains the separate subtype token; module paths are quoted; UoM and planned fraction syntax retain their existing roles. H-318 covers this source grammar and migration, with no numeric or representation change.

## Module-world architecture

Each module constructs its own world, explicitly exports information available to receivers, and merges information about identical imported identities in its own copy. Cross-module world editing is impossible at compile time and runtime under H-245. Compilation determines the flattened TLS shape; runtime effects change and restore typed symbolic pointers. Registered symbol objects, types, callable declarations and methods are module-world entities. Definitions/extensions compose those semantics without effects; explicit runtime effects materialize/install compatible family views. Bare symbol lookup, TLS reads, construction and invocation are distinct. [Module worlds](ModuleWorlds.md) records the construction, publication and execution boundaries.

H-236 requires architecture to govern local lookup behavior. Receiver-owned identity records unify information reached through several aliases; selected callable interpretations retain their distinct contexts. H-235 keeps construction and invocation machinery internal. Ordinary calls do not require a public world object or an invocation protocol. Local-value versus direct-call lookup remains a separate naming question; this architecture does not silently decide it.

H-237/H-238 distinguishes different entity sets rather than two views of the same generic objects. H-239 defines TLS as a flat collection of typed symbolic pointers: symbol and type identities are independent, and a current value may originate from another module-world copy if its type identity is compatible. H-240 gives each module its own semantic world, using Julia as a comparison. H-241 places this architecture in [Global Variables Done Right](GlobalVariablesDoneRight.md). Execution stage is a separate dimension; [D-15](ModuleWorlds.md) inventories the entities, construction rules and their WPO implementation. No cross-module world editor is part of the model.

H-247 defines export as publication of an identity under an alias. H-248 requires identity-centered records with independent local/public aliases; source selection is read-only. H-253 requires conflicting inherited or selected overloads to reject. Future arbitration syntax and full-document qualification remain pending-human.

H-375 confirms actual-type family dispatch, including abstract parameters and results; direct method selection is an optimization. H-376 requires WPO/AOT coverage before execution of the selected composition. H-396 directs shared native method bodies and local micro-dispatch, using concrete-type discriminators and proven facts at call sites; coverage does not require tuple-specific method bodies. The remaining H-251 questions concern physical type evidence, ABI and reflection, not whether a valid abstract call may dispatch dynamically. [Concrete-type evidence](ModuleWorlds.md#actual-type-dispatch-and-concrete-type-evidence) and [D-21](AheadOfTimeExecutionPreparation.md) separate that mechanism work from the settled behavior. Static parallel module interpretations coexist independently of the AOT/JIT strategy.

H-257/H-258 requires nearest lexical procedure lookup, shadowing warnings, distinct callable identities and signature-only declarations, and separate evaluated bits/storage places. Declarations require parameter/result signatures and permit generic type parameters and varargs. H-256 reserves fraction comments; H-260/H-261 requires representation-independent quantities and lossless widening. D-16 remains the fraction compatibility constraint; current compiler coverage is recorded separately.

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
| G-06 | Nominal symbols, thread-scoped effects, captured/current procedure invocation, lexical functions and lexical or symbol-bound procedures, with whole-program call-path checking. | H-96–H-98, H-102–H-104, H-180; DL-02, DL-14, DL-15 |
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

- Preserve strict branded identity under H-125 and independent compile-time UoM identities under H-128; structural layout alone does not establish equivalence. Discuss sum constructors and product fields, recursive types, pattern matching/exhaustiveness, equality and admissible regional fields. Keep semantic type identity separate from physical layout.
- Distinguish ordinary ADTs from indexed GADTs. Specify the initial role of type parameters, kind checking, inference and type equivalence without silently treating the full long-term type theory as already selected.
- Decide which ontology relations and constraints are first-class, what is explicitly asserted versus inferred, and how independently defined domain concepts can be mapped. Similar field layouts do not establish conceptual equivalence.
- Explain how domain signatures can be loaded and checked without requiring every domain constructor to become a fixed host-language enum. Distinguish published definitions from parser/solver scratch state.

**Current scalar discussion:** [ScalarUnitsAndIndexTypes](ScalarUnitsAndIndexTypes.md), D-11, compares newtype-style wrappers with UoM arithmetic against actual pool formulas. Arrays declare an index type; scalar operations must preserve or explicitly transform its meaning. H-128 makes UoMs independent named compile-time entities with first-class type identities, same-name distinct definitions and identity-preserving local aliases; they are not represented by symbols. H-130 supplies pure conversion methods with Julia-style argument-type dispatch; H-134 specifies fresh UoM definitions and one-shot using selections, with no UoM import. H-135–H-138 specify parameter-controlled conversion and compile-time route choice. H-131 adds parenthesized operator names such as `(+)` for readable method definitions; D-11 distinguishes pure conversion/index functions from the mixed function/procedure operation overloads specified by H-153–H-155/H-158/H-159 in D-10; unary/binary methods use argument count and types. H-132 plans compound UoMs for future physical-quantity use and defers their implementation; compound equivalence/inference does not block this increment. Index origins and literal rules remain open within the bounded interface discussion. H-126 requests analysis before selecting the mechanism.

**Planned fraction compatibility:** [FixedWidthFractions](FixedWidthFractions.md), D-16, records H-254/H-255: intrinsic fixed-width `i64/i32`-style bits representations independent of UoM, unreduced arithmetic, explicit same-type `canonical`, `//` construction/output and `#` line comments. The compiler reserves `//` with a diagnostic and uses `#` comments under K-401. Fraction implementation is deferred. K-401/K-402 establish the separate H-256 comment/UoM prerequisites; H-260/H-261 supplies independent unit declarations, typed quantities and lossless primitive widening. Subsequent language designs and implementations must check this plan; conflicts with accepted requirements are design obstacles for human discussion before implementing the affected choice. Exact promotion, layout, qualifier and precedence rules remain open. This does not introduce denominator units, arbitrary precision or fraction-specific ownership.

**Proposal to discuss:** start from a small explicit sum/product and method core whose checked representation can express business logic and evolve toward the charter's GADT graphs. Decide the necessary indexed features from concrete programs.

**Decision evidence:** one domain definition, one business program with a result-dependent branch, an invalid construction that is rejected, and the exact boundary between value, type and kind representations.

#### Abstract types and data declarations

H-209 separates declarations by purpose: `type` declares an abstract type; `data` declares a concrete product/sum representation. H-210 uses `<:` to declare subtype relationships. Both are type definitions with nominal module-defined identities; data is not a value declaration, and an abstract type is not a symbolic dynamic variable.

Illustrative declaration headers, with constructor/sum and standalone method-signature grammar still pending. Abstract declarations have no Interface body:

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
| Sum data | Declares alternatives and their payloads; the variant/constructor grammar and physical tag representation remain to be selected. |
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

Sum alternatives and abstract descendants serve different roles. The recommended initial data sum is a fixed alternative set suitable for exhaustive matching. An abstract family may acquire nominal abstract descendants and explicitly conforming data types in other modules; neither current set is an implicit closed sum. Sum closure/extension and constructor namespaces still need an explicit contract. Future GADT constructors may refine indices of the declared data family; subtype syntax alone does not define those refinements or permit unchecked casts.

Storage classification is independent of nominal declaration identity. H-383/H-384 admits recursively finite bits ADTs to lexical or regional storage; stored references, ownership carriers, managed environments and implicit cleanup exclude that category. Non-bits user objects remain region-only. The `data` keyword alone does not establish bits eligibility, immutability or GC. [D-22](ConstructionAndAcquisition.md#confirmed-requirements) records the confirmed boundary and its incomplete implementation. Ordinary references borrow, const non-bits references restrict holder writes without freezing aliases, and owned remains an intrinsic resource responsibility. User data containing owned-region fields, including through variants/generic instantiation, remains forbidden by H-196. Builtin resource optional/results are a separate intrinsic facility.

Static abstract root bounds do not require a runtime object wrapper. Persisting heterogeneous abstract-typed fields or dispatching on a runtime sum nevertheless needs a sound representation/discrimination contract; WPO does not make runtime alternatives constant. No object header, fat pointer or general inheritance runtime is selected solely to support the first root-policy method.

**Primary-language comparison:** Julia explicitly declares nominal subtyping, makes abstract types non-instantiable and concrete types non-subclassable, and does not generally make parametric types covariant. These are useful reference points; CoD has its own data sums, storage and ownership semantics. [Julia types](https://docs.julialang.org/en/v1/manual/types/)

**Prototype implementation:** `type A;` and `type B <: A;` declare nominal abstractions; `data D { ... }` declares a region-only product, and `data D <: A;` adds explicit conformance without layout changes. An inline data parent is also supported. An adapter imports the data identity before adding a relation. Selecting that data type through the adapter carries its source-visible relations, including relations already selected by the adapter; selecting only an unrelated source method does not publish them. Type-associated methods can be selected implicitly. Abstract parameter/result checks preserve the concrete representation and its established explicit conformance while WPO specializes the call. Conditional values with the same concrete type and owner retain the most specific nominal ancestor guaranteed by both branches; this alone does not establish a child-interface claim. Concrete storage/provenance remains unchanged. `type { ... }` and `regional type` are rejected. Concrete data requires regional allocation and explicit references for stored object edges in the current syntax. The native extension/union identity and export subset is described below; compound cross-alias context/extension policies, sum constructors and generic syntax remain unfinished. Repeated identical conformance declarations and abstract stored-reference layouts are explicitly outside this subset, not selected permanent language policies.

A fold accumulator retains its explicit type guarantees and facts for its concrete value and possible stored-reference targets, including cycles. Unrelated facts visible only in a factory module do not become loop invariants. This is evidence preservation, not structural interface inference; unknown layouts cannot justify erasing required guarantees.

Lean acceptance must show one data identity with head-only conformance declarations in different modules, rejection of structural lookalikes without a declaration, adapter-import permission versus using-only declaration rejection, and consumer acceptance through using the adapter, invalid direct abstract construction, deterministic unique-best selection and an ambiguous crossed-specialization rejection. Product/sum construction and exhaustiveness need separate cases once their grammar is selected; no full GADT or general multiple-inheritance implementation is implied by this declaration split.

#### Standalone method declarations: review draft

**Pending human confirmation; not native syntax.** H-216 supplies distributed abstract methods and demanded implementation checks. The exact declaration and specialization contract below is a proposal under C-56/C-57 and P-194/P-195. Its purpose is to state an interface operation without inventing an executable body, while preserving the existing identity, visibility and WPO boundaries. It does not qualify new compiler behavior.

The proposed surface reuses the existing callable header and ends it with a semicolon:

```text
function read(store:SlotStore, slot:u32(Slot)) -> i64;
function cell(store:SlotStore, slot:u32(Slot)) -> i64;
proc flush(store:SlotStore) -> void;
export read;
export cell;
export flush;
```

An explicit result type is required because there is no body from which to infer it. These are ordinary lexical callable declarations; `symbol @p:proc(...) -> ...;` already expresses a required dynamic procedure binding. A bodyless symbolic-procedure shorthand is not needed for this slice. There is no Interface body, module namespace, runtime method table, execution step or capture event attached to a head. Public selection and extension follow the same using/import/export rules as implemented methods. Importing a declaration supplies the family to extend; it does not manufacture an implementation. Mixin still requires a body.

The pool example makes the need concrete. Its interface declares read/cell/acquire/release families, its storage module implements them for Pool, and a separate module defines replace/peek/locate using only interface types. The adapter imports the algorithm helpers into the implementation context it selects. The native example currently uses deliberately unavailable fallback bodies to expose the families; those are test scaffolding, not the proposed declaration syntax or a runtime failure mechanism.

Two decisions must be resolved together before implementing the heads:

1. **Does a head compete with implementations?** Proposed: it records an operation requirement and is not a body-bearing dispatch candidate. A visible generic parent implementation can satisfy a demanded child operation when applicable. The alternative treats a more-specific head as an abstract override that blocks a less-specific body. For `Reader <: Store`, a read(Store) body plus a read(Reader) head would succeed under the first rule and require a Reader-specific body under the second. H-216's demanded-visible-implementation rule alone does not choose between them. The first avoids introducing negative overrides implicitly; either rule must remain independent of declaration order.
2. **Does a head add an interface contract beyond ordinary overload selection?** A contract-head design checks the applicable head's result requirement and callable kind at each demanded call; a function requirement would need a function implementation. A declaration-only design uses the head for family/signature visibility and takes the selected overload's own result/kind, with the surrounding caller enforcing its requirements. These alternatives differ even when a concrete call is otherwise valid. Neither is confirmed. Borrowed results must retain actual provenance in either case; general qualifier compatibility and typed-root generics remain separate work.

The method-head review distinguishes a declared interface from selected body-bearing overloads. Consider a Parent/Child pair and an imported adapter:

| Selected concrete overload | Direct call from module scope | Call through a function declared to return i64 |
| --- | --- | --- |
| proc score(Child) → i64, returning7 | Returns7. | Rejects the procedure call inside the function. |
| function score(Child) → bool, returning true | Returns true. | Rejects the helper’s bool result against its i64 declaration. |

Those executed sources all have bodies. They are evidence of current dispatch and call-site checks, not tests of proposed heads. Replacing the broad body by a contract-bearing head would add an obligation that can reject the direct calls too. That may be useful when an abstraction promises a result/effect contract; it must be an explicit choice, rather than an incidental consequence of parsing a semicolon. H-155’s mixed operation kinds and H-197/H-198’s function boundary remain in force. A stronger head must define its scope without silently redefining ordinary body-bearing overloads.

If contract heads are selected, the rule for which inherited/applicable heads constrain a selected body must also be specified. This draft does not assume an intersection of every visible requirement, result covariance, a new effect subtype relation, or eager validation of all conformances. Declaration-only heads can reuse the current selected-method and caller checks, but do not promise that every concrete implementation preserves an abstract head’s result/kind in all calling contexts.

The pool’s unavailable fallback bodies are also not neutral stand-ins for a head. A more-specific fallback body is a real dispatch candidate and blocks a less-specific inherited implementation. A requirement-only head would not do that. The parent-body/child-head example must therefore be checked explicitly when replacing that scaffolding. Missing unused operations may stay harmless under either rule; demanded availability is the existing requirement.

Neither choice permits structural conformance inference, an eager inventory of every possible operation, or globally published adapters. An unused operation without a body stays harmless. A demanded call must select a visible implementation; statically established missing or ambiguous cases diagnose with the operation declaration and demanded path. For abstract actual-type dispatch, D-21 must cover the admitted alternatives and settle any uncovered failure-timing contract without arbitrary method fallback. The existing concrete argument type and region owner must survive the call.

A minimal acceptance increment should replace the pool's fallback scaffolding, run the same shared algorithm against two storage layouts, reject a demanded missing method while accepting an unused one, preserve a returned payload reference, and exercise the chosen parent-body/head and result/kind rules. Parsing a semicolon or changing the missing-call diagnostic alone would not establish this contract. Until confirmation, the parser continues to require callable bodies and the native records claim only that implemented subset.

#### Type-associated method visibility and extension

H-211 defines a type selection as bringing both the type and its source-visible associated methods. H-231 additionally requires those selected resources to be exported; signature association alone cannot expose a private callable. This includes source-visible methods originating elsewhere; it is not restricted to bodies textually defined beside the type. Exporting a symbol-identified callable publishes its whole method family under H-374; no private method subset is hidden inside that export. Method identity, body context, selection visibility and permission to add methods are distinct facts.

| Type selection | Available operations | Extension permission |
| --- | --- | --- |
| using a type | Use the exported source-visible associated method set. | No extension permission from this selection. |
| import a type | Use that same associated set. | A local declaration extends its unique associated family under H-226; conflicting families require explicit callable selection. |

H-212/H-232 separates three operations. Using preserves the selected definition and implementation context. Import permits extension and adapts an imported callable to the receiving implementation context; H-230 gives this extension context a distinct callable identity. It does not clone a type, rebind non-callable lexical resources or recapture effects. Mixin redefines a source body in the receiving lexical/dynamic context, with the existing compile-time symbol-map semantics. Import specialization is distinct from that new body-definition event. Categories beyond the existing procedure-body mixin mechanism remain open.

H-225 defines signature membership of the source-visible associated set; H-231 supplies its public-selection boundary: include parameter and result occurrences of the selected type, including supported reference wrappers, and inherited applicable methods. Result-only factories are included. Membership does not scan bodies or force unrelated layouts or implementations. A repeated original method is one candidate; distinct equally specific implementations remain ambiguous. Association does not introduce mutable wrapper covariance or typed-region syntax absent from the prototype.

H-226 makes a local declaration shadow using-only implicit methods, but extend the uniquely identified associated family after import of the type. An explicit callable selection takes priority over the implicit tier. Multiple unrelated implicit families require an explicit callable import-as to choose an extension target; unused implicit conflicts do not prevent type selection. Name resolution precedes argument dispatch: a failed explicit overload does not revive a shadowed implicit operation. Import permission belongs to the selected family, not an unrequested requirement to import every argument type.

H-227 selects caller-selected ordinary implementation views rather than program-wide publication. The native implementation separates semantic-family provenance from selected callable identity. H-230 requires distinct extension-module identities: differently aliased selections retain separate method sets, while a same-alias virtual union has a fresh identity in the selecting module. H-232 connects this contract to H-227: using preserves the selected implementation context, while import adapts a helper's callable dependencies to the receiving module's implementations. In either case unique-most-specific dispatch applies to the selected method set; equally specific distinct bodies are ambiguous. Neither method selection nor aliasing publishes unrelated conformance, changes a procedure capture, or adds a runtime dictionary or module namespace.

Compiler operation/intrinsic overloads retain lexical views in this increment, preserving source-helper native fallback. UoM's H-134 one-shot bundle selection and prohibition on copying/rebinding unit definitions remain unchanged. Source-preserving using/import, lexical/symbolic procedure mixin and once-only encounter-ordered module incorporation retain their contracts. Cross-alias extension grants and standalone abstract-signature syntax remain outside this increment.

Julia permits unqualified extension after import, and also permits extension through qualified function names. CoD adopts the using/import extension distinction but H-214 excludes module-qualified callable names: there is no corresponding qualified-name bypass. Automatic type-associated method selection is a separate CoD contract. [Julia modules](https://docs.julialang.org/en/v1/manual/modules/#using-and-import-with-specific-identifiers,-and-adding-methods)

H-214 keeps modules out of the receiving local namespace. A source path belongs in a selection declaration; selected definitions are reached through local aliases, not Module.f or Module.@p expressions. This retains package/version identity without introducing module-valued namespace objects. Compiler diagnostics and IDE navigation may display the full origin without making it a callable expression. A dot on an object may still denote its field; that does not establish module-qualified lookup.

H-215 proposes a separate explicit `import "source" method as local_alias;` declaration for local method renaming. It retains the source body and grants import's extension permission; H-232 also supplies receiving implementation-context adaptation. It does not perform mixin redefinition. A selector containing only a callable name naturally selects its source-visible overload family; selecting one signature would need additional syntax and is not introduced here. Compile-time aliases and explicit TLS installation are separate: installing a compatible source-family view may deliberately target another identity without changing either method world. The prototype’s implicit symbolic-selection installation path requires migration under H-372/H-373.

H-217 assigns implicitly brought method names a shadowable lower-priority namespace. This is a compile-time name-lookup tier, not a module value or runtime namespace. The H-226 declaration rule applies within a receiving scope:

| Binding source | Name lookup behavior |
| --- | --- |
| Explicit using/import callable alias | Shadows implicit names at that spelling. |
| Local function/procedure declaration | Shadows using-only implicit names; extends a unique family brought by import of a type. |
| Methods brought implicitly with a selected type | Supply names when no explicit binding shadows them. |
| Repeated paths to one original method | Preserve identity; do not create duplicate method implementations. |
| Different implicit operations with one spelling | Retain the conflict for resolution by explicit selection; a demanded unresolved name must be diagnosed. |

For example:

```text
using "arena.cod" ArenaRoot;
using "pool.cod" PoolRoot;
import "arena.cod" reserve as arena_reserve;
import "pool.cod" reserve as pool_reserve;
```

The explicit names select their respective source operations; an implicit `reserve` spelling need not be deleted. An explicit `import "pool.cod" reserve as reserve;` can instead shadow the implicit spelling directly. A single renaming preserves the selected source callable identity, method signatures and captures. H-230 separately gives an extension its module identity and gives same-alias composition a receiving-module union identity. The union rule concerns versions of the same semantic callable; how unrelated same-spelled families are combined is not inferred. Import permission still governs adding methods. H-232 governs the helper dependency context as discussed below.

Name lookup precedes argument-type dispatch. The rule resolves one binding, then applies unique-most-specific dispatch within its visible methods. Failure to find a matching overload for an explicit binding does not revive a shadowed implicit operation. This prevents argument types from silently changing which module's operation a name denotes. It also preserves function/procedure restrictions, const/ownership permissions and argument evaluation order; aliases do not relax them. This lookup/dispatch boundary implements the H-217/H-226 priority distinction; execution evidence is recorded separately.

Implicit-name ambiguity is diagnosed at demanded lookup or at an extension declaration that needs one identity. Explicit bindings may shadow unused conflicts. Scan-before-incorporation and explicit alias availability remain unchanged; these rules do not hoist module initialization or grant cross-alias extension permission.

H-257 requires calls to resolve the nearest lexical value binding: a callable parameter or local supplies the callee, while a non-callable shadow makes the call ill-typed. [Lexical calls and shadowing](SymbolicDynamicScope.md#lexical-calls-and-shadowing) records the rule and shadowing diagnostics. This explicit decision is separate from the priority of implicitly selected type methods.

#### Method-selection examples and evidence

Callable identity, extension permission, selected implementations and body lexical context are separate. An imported extension has its receiving-module callable identity; a local declaration after using a type can shadow the implicit name with another family. Combining equally specific distinct implementations remains ambiguous under H-209.

[method-views.cod](../tests/cod/language/method-views.cod) exercises these cases with a common Metric interface and Sample representation. Two adapter paths select score(Sample) = 7 or 11, and the same source helper calls score twice through a private intermediate, returning 14 or 22. A using-only local score is a separate family; the source helper still calls its original score. Result-only create and Sample-parameter methods arrive with Sample. A repeated source path deduplicates methods, an explicit selection shadows an unused implicit-family conflict, and local intrinsic overloads combine in their lexical operation family. The separate negative fixtures distinguish identity ambiguity from equal-specificity implementation ambiguity.

Callable-dependency analysis follows expression meaning: a setfield field selector is a compile-time field label even when a lexical function has the same spelling. It cannot make an otherwise source-bound method depend on the importing module. Calls in the object or stored-value expressions remain ordinary dependencies.

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

<a id="partial-and-full-selections-of-one-callable--review-boundary"></a>
#### Partial and full selections of one callable

Type-associated selection may supply part of a source callable's information, while an explicit selection supplies more under another alias. When their identity is equal, the receiver merges the information into one local record. Both aliases and an adapted helper consult that record. Different identities remain distinct; spelling alone cannot merge their semantic records. Repeated import of one source callable into the same receiving module selects one receiver identity even when aliases differ. Locally declared overloads under an explicitly imported alias join that identity and are visible through its other aliases. This does not grant extension permission through using or merge distinct extension-module identities. H-252 requires a warning when one module declares overloads through multiple aliases of that shared identity, with opt-in promotion to an error. The prototype uses `-Wmulti-alias-extension`, `-Werror=multi-alias-extension` and the global `--warnings-as-errors` option; selecting aliases without multi-alias declarations remains warning-free. [D-15](ModuleWorlds.md#identity-records-aliases-and-publication) describes the identity-based check and preservation of helper contexts through successive imports.

```text
using "base.cod" First;
using "base.cod" score as zipped;
export import "base.cod" relay;
```

If First brings score(First) and zipped brings both score(First) and score(Second), this receiver's score, zipped and adapted relay can use both methods. The source module's own information is unchanged. [world-composition.cod](../tests/cod/language/world-composition.cod) checks both concrete types through both aliases and an adapted helper. [world-source-isolation.cod](../tests/cod/language/world-source-isolation.cod) rejects a source helper's attempt to use an unselected method even after the receiver has merged it.

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

Each line performs the normal import and explicitly exports all resources that this import brings into the receiving module. This includes methods implicitly selected with an imported type, subject to the source export boundary; it does not expose unrelated receiving-module declarations or private source resources. The first line exports the receiving-context helper selected by H-232. A downstream using preserves that exported context. Plain import has the same adaptation/extension behavior but no publication. The modifier does not perform an extra dependency encounter, create a runtime export action, grant consumers import permission, or alter once-only source-module incorporation. An exported source name is not automatically re-exported by modules that select it.

Explicit re-export preserves source lexical data, initialization identity and the selected callable context. Private resources, unexported aliases and ordinary imports are not automatically public.

A symbolic procedure selected by export import is also explicitly public under its receiving alias. Its source captures remain intact; publishing it neither re-executes the source module nor recaptures the caller’s later bindings.

Export import is the confirmed shorthand; analogous modifiers on using or mixin are not selected by this statement. Standalone re-export uses the same single-name export form. Grouped export lists remain an open surface-design detail.

Here Sample, score and @print name existing local bindings, including selected aliases. Exporting score exposes its callable set; exporting Sample alone does not expose private functions whose signatures mention Sample. Type-associated selection must filter by the source's export boundary, preserving public result-only factories and inherited applicable methods. A public source-defined callable can still use private helpers internally without making those helpers directly selectable. UoM bundles likewise select exported aliases of their existing conversion entries; a private original spelling stays unavailable, and aliases of one formula/context do not multiply conversion paths. Ordinary callable import alone does not register a new conversion formula; D-11 keeps that broader adaptation/registration boundary explicit. An exported mixin body remains subject to its receiving-context name rules; export supplies no private-name bypass.

Privacy, explicit re-export and export import are confirmed at the recorded named-resource scope. H-247 defines publication as public alias → identity. H-248 requires identity-centered compiler records with multiple aliases. Consumers resolve the published identity directly; the exporter's current local spelling is not another lookup step. `export import` publishes the actual imported identities, including associated methods, even when a different local binding has that spelling. [world-export-alias.cod](../tests/cod/language/world-export-alias.cod) checks both a competing local method and a frozen local helper; the imported identities remain public. A conflicting publication under the same public alias rejects. Grouped lists, per-signature publication, broader bundles and cyclic re-exports remain draft details. A type appearing in an exported signature is not automatically directly selectable.

Acceptance needs a public function using a private helper, rejected direct/private selections through all three reuse forms, alias re-export, private associated methods staying hidden, and public factories remaining available. Together with the identity increment it must check separate aliases yielding1,7,1, same-alias union deduplication and ambiguity, plus using-preserved2/import-adapted14 and export import publication without implicit re-export. Existing initialization, captures, conformance, concrete dispatch and owner checks remain relevant. Acceptance does not qualify unresolved compound contexts or the complete language contract.

Lean acceptance should exercise explicit conformance without eager body completeness, rejection of undeclared conformance despite matching signatures, unrelated declared abstract constraints remaining ambiguous, and unused missing implementations remaining allowed while demanded missing implementations fail. An adapter's declared conformance must be usable through using without granting declaration permission; an unrelated same-identity view or mere WPO inclusion must not silently publish it. Different field layouts must be irrelevant to interface matching. Module cases need distributed parent/child methods, source-visible transitive methods, duplicate-path identity handling, using-only extension rejection and permitted import extension. Alias cases must show explicit shadowing of implicit conflicts, unchanged operation identity/source captures, and no fallback to a hidden operation after explicit dispatch fails. The native abstract checkpoint covers the explicit-conformance/applicability portion. The linked increment evidence covers the native association, name-priority and caller-view subset; unimplemented qualifier and owner contracts remain separate; K-400 covers the bounded typed-signature increment.

<a id="dl-02"></a>
### DL-02 — Methods, effects and entry execution

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

**Ownership boundary:** H-190 makes whole-region ownership intrinsic. Intra-region allocation/reuse policies belong to CoD user code, including free lists and pools. Typed slot reuse neither releases part of the region nor guarantees that an old reference still identifies the same logical item. D-14 prioritizes the missing ordinary holder, construction/result, optionality and reference-validity contract before mailbox transfer.

Lifecycle acceptance requires each selected short-circuit scope to release its local region before the next iteration, skipped operands to allocate nothing, and captured/hidden enclosing owners to remain live. These requirements do not decide pending owning construction, transfer or mailbox choices.

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
- Define partial reception, frame boundaries, minimum structural checks and rejection behavior. Validation should establish the conditions needed for correct interpretation and application; its depth and cost belong in the protocol contract.
- Specify the layout/ABI agreement represented by the initial manual identifier, including byte order, field widths and alignment. Semantic/schema versions remain distinct from layout and ABI compatibility; builds on multiple architectures do not establish image compatibility.
- Decide mutation rights during construction, after sending and after reception. Whole-region relocation preserves valid relative layouts; arbitrary truncation or object movement does not follow from that property.

**Proposal to discuss:** construction → sealing → complete reception → validation/binding → application. The total binary length is confirmed; sealing and this exact state model remain proposals.

**Decision evidence:** annotated packet layout, split/combined input examples, an incompatible root/dependency example and a declared byte-order, width and alignment policy for the chosen peers.

<a id="dl-05"></a>
### DL-05 — Application, commit and retry

**Question:** What observable event means that a packet has “taken effect”?

**Assigned design responsibility (H-352):** [Actor-model design](GlobalVariablesDoneRight.md#actor-implementation-readiness) must design the language-level support applications use for outcomes, failure and recovery. The packet format and graph-transfer operator do not impose a database transaction protocol.

- Actor design must state local send/receive, handler completion and failure boundaries. H-168's intrinsic send reports local system errors only; it does not acknowledge delivery or application.
- Applications use the Actor facilities to define any response, retry, duplicate-handling or compensation protocol they need. Distinguish fully received, applied in memory and durably committed outcomes in that protocol.
- Snapshot/file operations supply the persistence boundary in DL-09; their failure behavior must be explicit before applications can rely on it. Selecting Actor support does not yet choose a scheduler, supervision strategy, exactly-once guarantee or cross-region transaction mechanism.

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

<a id="dl-11"></a>
### DL-11 — C/C++ and other runtimes

**Question:** What interoperability contract exposes useful operations while preserving regional and language semantics?

**Planning direction (H-352):** provide a service provider interface (SPI) through which third parties can extend intrinsics; other extension logic belongs in CoD user-program libraries. C/C++ remains the compatibility role from H-06. The SPI is the expected integration path, not a chosen registration, loading or native-call ABI.

- Specify ownership of buffers/regions, borrowed-view lifetimes, allocation/deallocation symmetry, error results, calling threads and callbacks. Separate physical image compatibility from a function-call ABI.
- Decide which foreign values must be materialized and which runtime handles can be temporarily bound. Native heap pointers, callbacks and operating-system handles need an explicit execution-lifetime treatment.
- Define how providers expose typed intrinsic operations and how external runtimes construct/check packets, invoke methods, consume responses and bind effects while preserving language invariants. Provider selection must fit module-world visibility and function/procedure contracts; it does not imply mutable shared registries or dynamic code loading.

**Remaining interface work:** typed provider declarations, compiler/runtime integration, native argument/result and error conventions, owner/lifetime checking and provider version compatibility. A C ABI or C++ adapter may serve that SPI, but neither an opaque-handle ABI nor a mandatory wrapper is selected. No third-party intrinsic extension is claimed implemented here.

**Decision evidence:** trace one foreign-runtime caller through construction, application, result consumption and release, including an error and a rejected lifetime violation.

<a id="dl-12"></a>
### DL-12 — Initial scope and execution strategy

**Question:** What is the smallest language and runtime implementation that can demonstrate the selected semantics without making the host language their only definition?

- Use direct LLVM IR as the sole execution lowering path under H-143. Keep source semantics and future persistent semantic graphs distinct from derived .ll/.bc artifacts; use LLVM's standard optimization pipeline without a multi-backend framework or custom micro-optimizations.
- Decide which typed representation, checking rules and handler interfaces are shared. Layout/validation facilities must follow one defined type contract; graph transfer/compaction algorithms remain library code under H-351/H-352, with no implicit compiler-generated universal traversal.
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

**Confirmed:** module-defined symbols have strict identity, imported aliases retain it, and independent origins/versions remain distinct. A typed symbol declaration fixes the binding type; use requires a supplied value along the demanded call path. Reads use `@name`; bits/ownership transfer uses `effect @name <- value`, and object/procedure rebinding uses `effect @name = value`, restoring on scope exit. Bits symbols reject the equals form; object references require live region-symbol backing and may change region owners. Runtime procedure-value construction captures used effect values; module method registration does not; `perform` temporarily installs them, `behave` retains current bindings. Demand-driven WPO checks required bindings over caller paths; missing values are compilation errors, not runtime fallback.

**Confirmed module practice:** H-242 vocabulary modules define typed symbols, types (primarily abstract, with some supporting ADTs) and their method collections, with comments explaining their meaning. Selecting the same resolved vocabulary establishes common identities across independent module-world copies; this does not create one shared mutable world. H-242 broadens H-105's earlier symbol-only idiom. The [two-world inventory](ModuleWorlds.md#vocabulary-modules-establish-shared-identities) records the identity role. The following using/import/mixin rules describe the confirmed prototype baseline whose detailed world interaction remains under architecture review: `using` preserves the selected source definition and callable implementation context. `import` retains its source body, non-callable lexical bindings and captures, selects the receiving module's ordinary callable implementation context, and permits extension under H-230/H-232. `mixin` redefines a source body in the receiving lexical/dynamic scopes under H-212. These definition and implementation choices are separate from `perform`/`behave`'s captured/current value choice. D-10 records the native subset and pending compound-selection boundaries. H-231/H-233 requires explicit publication at every module boundary, including re-export.

**Confirmed module phases:** H-106/H-107 permit symbol/function definitions and bits globals initialized by literals or function calls, plus top-level `effect`/`perform`/`behave` expressions. Procedure definitions register during scanning; neither their definition semantics nor global initializers/top-level expressions execute during that scan. All entry dependencies are scanned/loaded before entry execution is compiled; dependencies then contribute top-level execution semantics in encounter order.

**Confirmed immediate review:** every cross-module reference triggers module execution semantics (H-109). Its top-level scope remains alive until process exit (H-166); `using`/`import`/`mixin` retain H-110’s explicit selection of effective resources. Initialization completion is not resource destruction. Every `effect` yields void (H-288); procedure rebinding uses `=`. H-372/H-373 requires explicit installation and the combined effect form defines or extends before installing a compatible family view; the prototype’s `effect proc` spelling and implicit selection paths require coordinated migration.

**Implementation boundary:** the native subset checks typed bindings, demanded captures and bits-global initializers. Durable package/symbol identity encoding, general capture admissibility, cyclic initialization and runtime layout/caching remain open in [SymbolicDynamicScope](SymbolicDynamicScope.md). The bounded implementation does not supply a persistent symbol ABI or independent module compilation.

**Origin/alias review:** H-112 requires explicit provenance and local aliases. H-113 excludes lexical symbolic variables and ordinary assignment. H-114 makes identities compile-time values and aliases compile-time names; runtime values bind under identities. H-115 permits a body alias map with matching-name defaults, now belonging to mixin under H-212. The prototype H-116 destination form uses `mixin "module" @source as @local`; its implicit installation requires migration under H-372/H-373. [D-10's relationship model](SymbolicDynamicScope.md#symbol-origins-aliases-and-dynamic-values) separates identity selection, procedure-value installation and body translation. H-117 proposes `symbol from "module" @source as @local` and `with { @local as @source_role; }`, read as supplying a local symbol for the source body’s role. H-118 authorizes implementing these discussed forms. The target compiler must separate selection from explicit effect installation and resolve body aliases before runtime capture analysis; existing implicit-installation paths require migration. Broader mapping edge policies remain open; unsupported cases receive explicit diagnostics.

**Selection migration:** H-372/H-373 supersedes the implicit-installation portion of H-119/H-120’s using abbreviation. Selection/aliases and runtime installation must be separate; preserve same-identity aliases, deliberate cross-identity installation and once-only source initialization. The compiler’s current shorthand/destination paths remain implementation evidence pending coordinated migration, not the final stage contract. [D-10](SymbolicDynamicScope.md#source-selection-and-explicit-installation) identifies this boundary. Namespace and exact grammar choices remain DD-AOT-07 work.

**Entry installation:** H-379 requires an entry module that extends a dependency callable to install it explicitly; omission is a compilation error. Accepted entry installation cancels dependency installations, exposing unused world code to LLVM elimination. The [D-21 contract](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) preserves still-reachable views, evaluates canceled initialization operands and retains procedure-local installations; identity/conditional-coverage mechanisms remain under review.

**Decision evidence:** alias/version separation, nested effects, both call modes, transitive `behave` dependencies, a reachable path missing a required binding, scan-time absence of effects/captures and an encounter order different from dependency scan order. No independent module compiler is presumed.

<a id="dl-15"></a>
### DL-15 — Callable definitions, expressions and storage

**Confirmed:** functions are lexically defined and use explicit inputs/outputs. H-184 permits procedures with ordinary lexical names or symbolic dynamic bindings; both may access effects. Existing lexical procedures may be installed under typed symbols without redefining their bodies or captures. H-186/H-187 allows `proc @p(...) -> RT { ... }` to introduce the symbol and method; H-372 separates this compile-time registration from runtime installation. `effect @p;` installs the composed family without extension, and the combined effect form performs both stages. Explicit procedure types use `proc(T1, T2) -> RT`. H-189 gives direct lexical calls current dynamic bindings; symbolic perform alone replays captured values. Module initialization is the top-level dynamic scope. Symbol reads and writes are expressions, and `void` indicates a result with no meaningful value. User storage is stack or regions, with ordinary owners and region-backed dynamic resources as separate uses. H-180 reaffirms dynamic-value capture; shadowing preserves owners and does not release hidden regions. Physical references stay within their region. Business associations between regions follow H-157.

**Open:** family-view compatibility/representation, concrete-type evidence through abstract results, coverage closure, namespace/staging and diagnostic mechanisms in D-21; the first regional GADT construction contract and business protocols for logical relationships between regions retain their separate scope. A scope-local `effect` restores on exit under either call mode; `behave` does not make rebinding permanent. No `malloc` closure, raw stack reference capture or user-visible host address is inferred.

**Decision evidence:** lexical function lookup versus dynamic procedure rebinding, effect access rejected in functions, expression result typing, scope restoration, and capture/lifetime examples. H-98–H-104, D-10.

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
| LI-04 | Conversion route selection | H-136: discover routes at compile time, choose a unique route automatically, warn about unresolved alternatives and allow warnings-as-errors. H-137/H-138: a direct conversion wins; otherwise use the declared indirect route, illustrated by `m*<-cm<-mm`. |
| LI-05 | Argument/conversion order | H-148: evaluate each argument once and finish its selected conversions before evaluating the next argument. An earlier failure prevents later argument effects. |
| LI-06 | Local value names | H-149: nested scopes may shadow outer locals; declarations within one scope must be unique. The initializer sees the outer value, and scope exit restores its visibility. Function parameters and loop bindings share their body’s declaration scope. |
| LI-07 | Reserved expression names | H-150: reject literal/control spellings in parameters, `let`, loop indices and fold accumulators; retain usable contextual names and field selectors. |
| LI-08 | Intrinsic/operator overloads | H-153–H-155: reserve ordinary keyword names and allow parenthesized operation methods of either callable kind. H-158: intrinsics use compiler-internal representation, separate from CoD symbols. H-159: a selected procedural operation uses current bindings, like `behave`. UoM conversions remain pure. |

The run develops structured product values, named UoMs with these selection/conversion rules, Julia-style argument-type methods and parenthesized operator names, bounded numerical iteration, static SoA regional arrays and owner-tracked explicit-memory traversal. Regional references retain existing lifetime/relocation rules, including H-146’s accepted first-field self-reference-to-null limitation. [Local value shadowing](SymbolicDynamicScope.md#local-value-shadowing) follows H-149; symbolic writes still require `effect`. Compound-unit algebra, general GADT machinery, custom operator precedence and cross-region representation remain separate increments. H-161 reserves primary module-form words against ordinary declarations and aliases. H-162 selects the distinct `(=)` family; H-163/H-164 make indexed assignment call `([]=)(array, index, value)` while preserving nominal container/index types. General lvalues, record-field assignment and compile-time field-selector overloads remain open; native `setfield` provides field writes. The completed H-147 work period and H-160 review retain their historical scopes. No new SNN dynamics or learning rule enters this work.

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

Application design lives in its linked document set. Language topics record their current answer, governing constraints and open choices; application proposals become reusable language requirements only through an explicit scope and value decision. Human evidence and brief continuity entries belong in the [changelog](../CHANGELOG.md#authority-evidence-index). Update the relevant topic and current registry scope together; a proposal does not supply its own approval.
