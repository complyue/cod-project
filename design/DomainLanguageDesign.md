# Domain language design: discussion and decision tracker

Discussion revision: 117

Recorded: 2026-10-22

Current discussion: 2026-11-09

Phase: coordinated language/SNN design, implementation planning and validation.

## Design authority

- **Bounded exception/lifecycle contract:** [H-267](DesignAuthorityRegistry.md#h-267)/[K-408](DesignAuthorityRegistry.md#k-408) accepts [D-17r3 E1–E5](ExceptionControl.md#accepted-first-implementation-contract), including explicit throw-from only, cleanup, fresh owners and call/result transfer. This overrides pending labels for that enumerated scope; wider qualifier, assignment, mailbox and user exception graph choices remain open. [K-414](DesignAuthorityRegistry.md#k-414) records completed H-267/H-269 E1–E5 acceptance and [current native evidence](../tests/cod/language/exception-ownership-validation.json). Wider unconfirmed scopes retain their status.

- **Registry / scope:** [D-07](DesignAuthorityRegistry.md#d-07), revision117, all sections; [K-411](DesignAuthorityRegistry.md#k-411) records bounded typed-region ownership and exception cleanup under D-17; [H-265](DesignAuthorityRegistry.md#h-265)/[K-406](DesignAuthorityRegistry.md#k-406) establishes exception-control planning and unchecked ultimate OOM in [D-17](ExceptionControl.md); [H-263/H-264](DesignAuthorityRegistry.md#h-263)/[K-405](DesignAuthorityRegistry.md#k-405) supplies permitted local-view survival and the initial typed construction form; [H-262](DesignAuthorityRegistry.md#h-262)/[K-404](DesignAuthorityRegistry.md#k-404) confirms the ordinary named-owner consumed state; D-17 E4/E5 bounds implemented construction/transfer; [K-400](DesignAuthorityRegistry.md#k-400) records successful repair/build and28 focused checks; H-257/H-258/[K-399](DesignAuthorityRegistry.md#k-399) directs lexical call/shadowing correction, mandatory generic/variadic callable signatures and LLVM value/place modelling, with ordinary ownership covered separately by D-17. [K-398](DesignAuthorityRegistry.md#k-398) records H-256 priority follow-up for comment reservation and representation-independent UoM; [K-397](DesignAuthorityRegistry.md#k-397) adds the H-254/H-255 fraction draft and future compatibility-review requirement without scheduling implementation; [K-396](DesignAuthorityRegistry.md#k-396) records the completed architecture audit on the K-395 compiler, including H-253 conflict rejection, H-252 warning controls and cross-world TLS. Earlier checkpoints retain their actual scopes. H-234/H-237–H-249 supplies independent module worlds, separate typed TLS, vocabulary identities, receiver-local composition and identity-based publication; H-245 supersedes cross-module editing. H-251 reserves RTTI for human discussion. Earlier decisions/evidence retain their scopes in the registry. Full contracts and candidate value paths remain pending-human; this revision does not qualify unresolved mechanisms. [K-401](DesignAuthorityRegistry.md#k-401) records H-255/H-256 comment reservation and H-260/H-261 quantity/widening decisions; full fraction execution remains deferred. [K-402](DesignAuthorityRegistry.md#k-402) records the implemented quantity/widening subset and56 focused checks.
- **Value lineage:** C-10 supplies inspectable typed execution. C-35/C-43/C-45/C-46 through P-84/P-100/P-102/P-109–P-111 and C-56/C-57 through P-182–P-197 concern identity, composition, interfaces and explicit behavior selection. C-39/C-54 through P-94/P-143/P-155 supplies regional lifetime boundaries. C-58/P-199–P-202 adds explicit publication and module-scoped callable composition. These candidate paths serve charter §§2–4; dependency evidence does not qualify them.
- **Human baseline:** H-96–H-141 supplies symbols, callables, modules and units, subject to later supersession. H-143–H-146 selects LLVM, lean validation, simplification and the known zero-offset limitation. H-166/H-180/H-181 preserves module lifetime, captured/hidden owners and non-extractable resources; H-184–H-189 supplies typed lexical/symbolic procedures. H-193/H-196–H-203 supplies regional data, intrinsic owners, mutable explicit-memory functions, borrowing, const and same-owner edges. H-209–H-220 supplies abstract/data types, explicit conformance, source-preserving selections, mixin, aliases and dispatch. H-222–H-224 supplies the const-RHS assignment discussion. H-225–H-227 selects associated signature membership, using/import-dependent local declarations and caller-selected ordinary implementation views. H-229 preserves concrete-data dispatch; H-230 refines callable identities and alias composition; H-231 requires explicit exports; H-232 selects using-preserved versus import-adapted helper implementations; H-233 requires explicit re-export and permits export import.
- **Invariants / non-goals:** preserve zero-cost relocation, region-only non-bits storage, no regional object copy/move, owner provenance, non-consuming capture replay and no dynamic effect/procedure calls from functions. No module namespace, runtime interface wrapper, allocator policy, bootstrap/toolchain change or SNN modeling choice is introduced. The confirmed method-selection increment does not settle qualifiers, ownership construction/transfer, standalone abstract signatures or mailbox semantics.
- **Acceptance / evidence:** H-245 authorizes the corrected architecture goal, whose implementation audit is recorded by K-396. H-221 records the completed ten-hour work period. Native examples must distinguish source/caller contexts, visible/hidden conformance, use/extension permission, unique/ambiguous dispatch and demanded/unused missing implementations, while preserving lifetimes and relocation. The [prototype coverage table](../tests/cod/language/README.md#current-bounded-acceptance) identifies implemented subsets and their actual evidence. [Method-context observations](../tests/cod/language/method-context-observations.json) preserve the source-only prototype baseline for the confirmed caller-view increment; they are not acceptance evidence for that increment.

## Module-world architecture

Each module constructs its own world, explicitly exports information available to receivers, and merges information about identical imported identities in its own copy. Cross-module world editing is impossible at compile time and runtime under H-245. Compilation determines the flattened TLS shape; runtime effects change and restore typed symbolic pointers. Types, functions/procedures and overloaded methods are the core module-world entities. [Module worlds](ModuleWorlds.md) records the construction, publication and execution boundaries.

H-236 requires architecture to govern local lookup behavior. Receiver-owned identity records unify information reached through several aliases; selected callable interpretations retain their distinct contexts. H-235 keeps construction and invocation machinery internal. Ordinary calls do not require a public world object or an invocation protocol. Local-value versus direct-call lookup remains a separate naming question; this architecture does not silently decide it.

H-237/H-238 distinguishes different entity sets rather than two views of the same generic objects. H-239 defines TLS as a flat collection of typed symbolic pointers: symbol and type identities are independent, and a current value may originate from another module-world copy if its type identity is compatible. H-240 gives each module its own semantic world, using Julia as a comparison. H-241 places this architecture in [Global Variables Done Right](GlobalVariablesDoneRight.md). Execution stage is a separate dimension; [D-15](ModuleWorlds.md) inventories the entities, construction rules and their WPO implementation. No cross-module world editor is part of the model.

The compiler stores callable/global information by identity, with independent local/public alias maps; source selection is read-only. H-247 defines export as publication of an identity under an alias. H-248 permits multiple aliases per identity, and H-246/H-249 directs first-principles implementation and helper removal. [K-396's architecture audit](../tests/cod/language/world-architecture-validation.json) establishes implementation across the goal's requirements on one compiler revision, including shared vocabularies, local merging, publication, TLS and lifetime behavior. H-253 requires conflicting inherited/selected overloads to reject; future arbitration syntax remains undecided. Full-suite and relocation reports retain their earlier compiler revisions, and full-document qualification remains pending-human.

RTTI remains an explicit discussion item under H-251. The current compiler tracks type identity and selects concrete methods statically; that does not establish a runtime reflection or dispatch contract. [Current RTTI boundary](ModuleWorlds.md#rtti-current-implementation-and-decision-boundary) separates compiler metadata, typed TLS pointers and the storage layer's region-root UUID. Related decisions require discussion with the user before design or implementation.

H-257/H-258 prioritizes three language corrections: nearest lexical procedure bindings govern direct calls and shadowing warns; callable identities and signature-only declarations are distinct from implemented methods; LLVM lowering distinguishes evaluated bits from storage places. Declarations require parameter/result signatures and permit generic type parameters and varargs. D-10r88 and D-15r14 record the contract and K-400 records28 focused checks; complete ordinary-region ownership is a separate design discussion. H-256's comment reservation and H-260/H-261's representation-independent quantities/lossless widening pass K-401/K-402; D-16 remains the fraction compatibility constraint.

## Confirmed direction

Cross-process send/receive of regional packets and region-backed resources shared through dynamic context are distinct scenarios under H-180. They share storage/lifetime invariants, but capture replay, resource installation and packet consumption are different operations. Neither scenario defines the other’s access or cleanup contract.

These are summaries of the user's statements, not inferred approval of the preceding assistant's recommendations. The original statements and their provenance are recorded in [H-06](DesignAuthorityRegistry.md#h-06), [H-07](DesignAuthorityRegistry.md#h-07).

| Goal | Confirmed direction | Discussion coverage |
| --- | --- | --- |
| G-01 | Use a memory region (`mem_region` in the discussion) as an IPC packet. Its header includes the total binary length. After complete reception, invoke a designated member method on the packet's entry object to apply it. | DL-02, DL-04, DL-05 |
| G-02 | Use DBMR as the local ontology database, supporting business retrieval, statistics and CRUD. Ordinary operations do not reclaim space; periodically rebuild its payload and backing file for compaction. | DL-06–DL-09 |
| G-03 | Give the domain language explicit ADT semantics, including sum/product types, method invocation and memory management. | DL-01–DL-03 |
| G-04 | Provide C/C++ compatibility interfaces for interoperability with other language runtimes. | DL-11 |
| G-05 | Advance language and SNN design, implementation and validation together, using simulator requirements to choose useful initial language features. | H-96; DL-12–DL-16 |
| G-06 | Nominal symbols, thread-scoped effects, captured/current procedure invocation, lexical functions and lexical or symbol-bound procedures, with whole-program call-path checking. | H-96–H-98, H-102–H-104, H-180; DL-02, DL-14, DL-15 |
| G-07 | User storage is stack or `@region`. Captures are bits or regional pointers. Each region contains one direct-pointer graph. Relationships between regions are logical business associations. | H-99/H-100/H-156/H-157; DL-03, DL-08, DL-15 |

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

data ArenaRoot <: NodeStore { used::u64; limit::u64; }
data PoolRoot  <: NodeStore { head::u32; count::u32; }
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

Generic argument matching and generic subtyping must remain distinct. The intended root method can constrain a parameter R by `R <: NodeStore` and receive region<R>; this does not automatically establish `region<ArenaRoot> <: region<NodeStore>` as a mutable-value conversion. The generic bound/header syntax is still proposed. Similar constraints apply to array element types, const access and owning qualifiers; <: cannot erase ownership or permission differences.

Sum alternatives and abstract descendants serve different roles. The recommended initial data sum is a fixed alternative set suitable for exhaustive matching. An abstract family may acquire nominal abstract descendants and explicitly conforming data types in other modules; neither current set is an implicit closed sum. Sum closure/extension and constructor namespaces still need an explicit contract. Future GADT constructors may refine indices of the declared data family; subtype syntax alone does not define those refinements or permit unchecked casts.

Storage remains independent of declaration spelling. Non-bits user objects stay in regions even when all their fields are bits; data does not imply immutability, stack eligibility, copying or GC. Ordinary references borrow, const non-bits references restrict holder writes without freezing aliases, and owned remains an intrinsic resource responsibility. User data containing owned-region fields, including through variants/generic instantiation, remains forbidden by H-196. Builtin resource optional/results are a separate intrinsic facility.

Static abstract root bounds do not require a runtime object wrapper. Persisting heterogeneous abstract-typed fields or dispatching on a runtime sum nevertheless needs a sound representation/discrimination contract; WPO does not make runtime alternatives constant. No object header, fat pointer or general inheritance runtime is selected solely to support the first root-policy method.

**Primary-language comparison:** Julia explicitly declares nominal subtyping, makes abstract types non-instantiable and concrete types non-subclassable, and does not generally make parametric types covariant. These are useful reference points; CoD has its own data sums, storage and ownership semantics. [Julia types](https://docs.julialang.org/en/v1/manual/types/)

**Prototype implementation:** `type A;` and `type B <: A;` declare nominal abstractions; `data D { ... }` declares a region-only product, and `data D <: A;` adds explicit conformance without layout changes. An inline data parent is also supported. An adapter imports the data identity before adding a relation. Selecting that data type through the adapter carries its source-visible relations, including relations already selected by the adapter; selecting only an unrelated source method does not publish them. Type-associated methods can be selected implicitly. Abstract parameter/result checks preserve the concrete representation and its established explicit conformance while WPO specializes the call. Conditional values with the same concrete type and owner retain the most specific nominal ancestor guaranteed by both branches; this alone does not establish a child-interface claim. Concrete storage/provenance remains unchanged. `type { ... }` and `regional type` are rejected. Concrete data requires regional allocation and explicit references for stored object edges in the current syntax. The native extension/union identity and export subset is described below; compound cross-alias context/extension policies, sum constructors and generic syntax remain unfinished. Repeated identical conformance declarations and abstract stored-reference layouts are explicitly outside this subset, not selected permanent language policies.

The [full source checkpoint](../tests/cod/language/validation.json) passes183/183 checks, including the strengthened field-selector, symbolic-reference, fold-conformance, guarded-owner and export-diagnostic fixtures. [Five composition cases](../tests/cod/language/stress-validation.json) pass with512-module function calls,1000-procedure inline calls, a22-module/2047-call diamond,16 unit re-exports with31 conversion steps, and96 levels of regional borrowing/mixin/nominal dispatch. Deep failure and implementation-conflict paths retain bounded diagnostics. All three reports identify the same compiler executable and their recorded source hashes match the current files. The [native ownership and relocation checks](../tests/cod/language/regional-validation.json) observe four acquisitions/releases with18 live reads, a120-byte cyclic graph and a152-byte pool with10000 CoD reuse laps. The independent C++ relocation check frees the original image before inspecting/reusing its copy; CoD execution does not resume after relocation. These measurements concern the recorded examples, not arbitrary program-size bounds or unfinished method-head/ownership features.

[Fold conformance evidence](../tests/cod/language/fold-conformance-validation.json) separates an accumulator’s retained explicit type guarantees from unrelated facts visible only in its factory module. When its possible referent types are known, unrelated facts are excluded from the loop invariant; a private type with no path from the accumulator cannot impose another iteration requirement. Facts for the concrete accumulator and its possible stored-reference targets remain checked, including cycles. This is metadata filtering, not structural interface inference. Unknown layouts conservatively keep the initial facts without forcing otherwise undemanded declarations. The existing forwarding fixture and12 focused checks pass, together with five targeted positive/negative probes.

Lean acceptance must show one data identity with head-only conformance declarations in different modules, rejection of structural lookalikes without a declaration, adapter-import permission versus using-only declaration rejection, and consumer acceptance through using the adapter, invalid direct abstract construction, deterministic unique-best selection and an ambiguous crossed-specialization rejection. Product/sum construction and exhaustiveness need separate cases once their grammar is selected; no full GADT or general multiple-inheritance implementation is implied by this declaration split.

#### Standalone method declarations: review draft

**Pending human confirmation; not native syntax.** H-216 supplies distributed abstract methods and demanded implementation checks. The exact declaration and specialization contract below is a proposal under C-56/C-57 and P-194/P-195. Its purpose is to state an interface operation without inventing an executable body, while preserving the existing identity, visibility and WPO boundaries. It does not qualify new compiler behavior.

The proposed surface reuses the existing callable header and ends it with a semicolon:

```text
function read(store::SlotStore, slot::u32(Slot))::i64;
function cell(store::SlotStore, slot::u32(Slot))::ref<i64>;
proc flush(store::SlotStore)::void;
export read;
export cell;
export flush;
```

An explicit result type is required because there is no body from which to infer it. These are ordinary lexical callable declarations; `symbol @p::proc(...) -> ...;` already expresses a required dynamic procedure binding. A bodyless symbolic-procedure shorthand is not needed for this slice. There is no Interface body, module namespace, runtime method table, execution step or capture event attached to a head. Public selection and extension follow the same using/import/export rules as implemented methods. Importing a declaration supplies the family to extend; it does not manufacture an implementation. Mixin still requires a body.

The pool example makes the need concrete. Its interface declares read/cell/acquire/release families, its storage module implements them for Pool, and a separate module defines replace/peek/locate using only interface types. The adapter imports the algorithm helpers into the implementation context it selects. The native example currently uses deliberately unavailable fallback bodies to expose the families; those are test scaffolding, not the proposed declaration syntax or a runtime failure mechanism.

Two decisions must be resolved together before implementing the heads:

1. **Does a head compete with implementations?** Proposed: it records an operation requirement and is not a body-bearing dispatch candidate. A visible generic parent implementation can satisfy a demanded child operation when applicable. The alternative treats a more-specific head as an abstract override that blocks a less-specific body. For `Reader <: Store`, a read(Store) body plus a read(Reader) head would succeed under the first rule and require a Reader-specific body under the second. H-216's demanded-visible-implementation rule alone does not choose between them. The first avoids introducing negative overrides implicitly; either rule must remain independent of declaration order.
2. **Does a head add an interface contract beyond ordinary overload selection?** A contract-head design checks the applicable head's result requirement and callable kind at each demanded call; a function requirement would need a function implementation. A declaration-only design uses the head for family/signature visibility and takes the selected overload's own result/kind, with the surrounding caller enforcing its requirements. These alternatives differ even when a concrete call is otherwise valid. Neither is confirmed. Borrowed results must retain actual provenance in either case; general qualifier compatibility and typed-root generics remain separate work.

The distinction is visible without introducing new runtime mechanisms. With ordinary body-bearing methods, the current compiler admits a broad function and a more-specific overload whose result or callable kind differs. [Recorded existing behavior](../tests/cod/language/method-head-review-observations.json) uses a Parent/Child pair and an imported adapter:

| Selected concrete overload | Direct call from module scope | Call through a function declared to return i64 |
| --- | --- | --- |
| proc score(Child) → i64, returning7 | Returns7. | Rejects the procedure call inside the function. |
| function score(Child) → bool, returning true | Returns true. | Rejects the helper’s bool result against its i64 declaration. |

Those executed sources all have bodies. They are evidence of current dispatch and call-site checks, not tests of proposed heads. Replacing the broad body by a contract-bearing head would add an obligation that can reject the direct calls too. That may be useful when an abstraction promises a result/effect contract; it must be an explicit choice, rather than an incidental consequence of parsing a semicolon. H-155’s mixed operation kinds and H-197/H-198’s function boundary remain in force. A stronger head must define its scope without silently redefining ordinary body-bearing overloads.

If contract heads are selected, the rule for which inherited/applicable heads constrain a selected body must also be specified. This draft does not assume an intersection of every visible requirement, result covariance, a new effect subtype relation, or eager validation of all conformances. Declaration-only heads can reuse the current selected-method and caller checks, but do not promise that every concrete implementation preserves an abstract head’s result/kind in all calling contexts.

The pool’s unavailable fallback bodies are also not neutral stand-ins for a head. A more-specific fallback body is a real dispatch candidate and blocks a less-specific inherited implementation. A requirement-only head would not do that. The parent-body/child-head example must therefore be checked explicitly when replacing that scaffolding. Missing unused operations may stay harmless under either rule; demanded availability is the existing requirement.

Neither choice permits structural conformance inference, an eager inventory of every possible operation, or globally published adapters. An unused operation without a body stays harmless. A demanded call must select a visible implementation and report missing or ambiguous behavior at compile time, with the operation declaration and demanded call path in the diagnostic. The existing concrete argument type and region owner must survive the call.

A minimal acceptance increment should replace the pool's fallback scaffolding, run the same shared algorithm against two storage layouts, reject a demanded missing method while accepting an unused one, preserve a returned payload reference, and exercise the chosen parent-body/head and result/kind rules. Parsing a semicolon or changing the missing-call diagnostic alone would not establish this contract. Until confirmation, the parser continues to require callable bodies and the native records claim only that implemented subset.

#### Type-associated method visibility and extension

H-211 defines a type selection as bringing both the type and its source-visible associated methods. H-231 additionally requires those selected resources to be exported; signature association alone cannot expose a private callable. This includes source-visible methods originating elsewhere; it is not restricted to bodies textually defined beside the type. Method identity, body context, selection visibility and permission to add methods are distinct facts.

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

H-215 proposes a separate explicit `import "source" method as local_alias;` declaration for local method renaming. It retains the source body and grants import's extension permission; H-232 also supplies receiving implementation-context adaptation. It does not perform mixin redefinition. A selector containing only a callable name naturally selects its source-visible overload family; selecting one signature would need additional syntax and is not introduced here. The identity/access contract must distinguish ordinary lexical aliases from existing symbolic `@source as @destination` procedure installation, which may deliberately target another already declared symbol.

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

Local value lookup versus callable lookup is a separate unresolved boundary. H-149 confirms nested value shadowing while excluding function/type namespace changes. The prototype can read a local scale value and still resolve scale(...) to a module method; a procedure parameter currently needs symbolic installation to invoke its supplied value. [The concrete review](SymbolicDynamicScope.md#local-values-and-callable-names--review-boundary) records native examples and the pending separate-versus-unified lookup choice. No callable namespace policy follows from the implicit type-method priority rule.

#### Method-selection examples and evidence

Callable identity, extension permission, selected implementations and body lexical context are separate. An imported extension has its receiving-module callable identity; a local declaration after using a type can shadow the implicit name with another family. [Module context/export evidence](../tests/cod/language/module-context-export-validation.json) records the current implementation. Earlier linked checkpoints retain their original compiler and pre-export scope. Combining equally specific distinct implementations remains ambiguous under H-209.

[method-views.cod](../tests/cod/language/method-views.cod) exercises these cases with a common Metric interface and Sample representation. Two adapter paths select score(Sample) = 7 or 11, and the same source helper calls score twice through a private intermediate, returning 14 or 22. A using-only local score is a separate family; the source helper still calls its original score. Result-only create and ref<Sample>-parameter methods arrive with Sample. A repeated source path deduplicates methods, an explicit selection shadows an unused implicit-family conflict, and local intrinsic overloads combine in their lexical operation family. The separate negative fixtures distinguish identity ambiguity from equal-specificity implementation ambiguity.

Callable-dependency analysis follows expression meaning: a setfield field selector is a compile-time field label even when a lexical function has the same spelling. It cannot make an otherwise source-bound method depend on the importing module. Calls in the object or stored-value expressions remain ordinary dependencies. [Field-selector evidence](../tests/cod/language/field-selector-context-validation.json) records the inherited-setter regression and separate adapter/value-call ambiguity controls; the existing module-method example includes a Counter field named score alongside the score family.

[Historical method-context observations](../tests/cod/language/method-context-observations.json) and [pool observations](../tests/cod/language/pool-method-context-observations.json) preserve the source-only baseline that motivated H-227. [The associated-method checkpoint](../tests/cod/language/associated-method-validation.json) records the actual compiler, sources and results, including the interface-only pool algorithm selecting its later adapter and returning 18 for slots containing 7 and 11. Its intentionally unavailable fallback body is checked only if selected; this is not implementation of standalone abstract-method syntax. The [deep-view checkpoint](../tests/cod/language/deep-method-view-validation.json) adds 96-module alias propagation, restoration between two adapter paths, demanded leaf ambiguity and source-family diagnostic locations. [The pool integration](../tests/cod/language/pool-algorithm-validation.json) supplies a checked-in interface-only algorithm module: replace/peek/locate are defined once and imported by the adapter that selects storage methods. A temporary second layout uses the same bodies through separate aliases and produces13,6,9,13, preserving owner-dependent payload writes and the first adapter after the second call. A source-frozen helper fails at its demanded unavailable operation. The primary pool retains10000 reuse laps,152 occupied bytes and independent whole-image relocation/free-chain checks. The pool interface uses typed bodyless declarations; K-400 covers demanded implementation checks through the imported algorithms. Association in this subset reads declared signatures, not body-inferred return types; broader inference is not selected here.

#### Concrete-data dispatch through abstract parameters

H-229 requires dispatch to use the actual concrete data type. A parameter or result annotated Entity checks that interface requirement; it does not turn a Node into Entity for later method selection or erase a previously established Node <: Weighted relation. Source and caller-selected implementations still need a unique most-specific match for the concrete argument types. Abstract inheritance and explicit adapter selection determine applicability; field shape and mere module loading establish no conformance.

[K-345's native observations](../tests/cod/language/conformance-forwarding-observations.json) identify the implementation gap: Node <: Weighted <: Entity selects score(Weighted)=2 directly, but the old Entity helper selects score(Entity)=1. The confirmed target is2 through direct, broad-parameter, nested and Weighted-helper calls. An exact score(Node)=7 remains more specific on all those paths. Selecting the original Node without its adapter must still fail an Entity requirement.

Concrete identity, region provenance and established conformance must survive argument/result forwarding and borrowed traversal. The compiler may retain the needed evidence as compile-time metadata; it must not add runtime interface wrappers or new ownership. Branch joins and loop-carried values cannot manufacture an interface fact that lacks justification on a reachable path. These checks preserve explicit selection rather than publishing unrelated adapter relations to every source body.

This settles the declared-interface-ceiling question. It does not settle which overload implementations aliases expose: concrete dispatch chooses among the visible candidates, so the alias example below remains a separate review. The historical probe records its old compiler. [Concrete-dispatch validation](../tests/cod/language/concrete-dispatch-validation.json) records the implemented target: direct, broad, nested, returned, reference/field and nullable paths retain the established concrete-type relations. A helper receiving that concrete type also preserves its relation for another instance allocated within the call. Related facts travel as compile-time evidence and are restored at call exit; unrelated caller type relations are not made ambient. The conditional/fold negatives exercise actual missing path guarantees, not artificial erasure at a broad parameter.

[Borrowed-context evidence](../tests/cod/language/borrowed-context-validation.json) composes these checks with the H-232 helper rules. A same-owner branch joining sibling interface facts retains their common Entity requirement and the selected concrete object. A fold follows a linked Node graph, including zero steps and the terminal node, while source-preserved rank and imported concrete rank retain their different results. Mutation through the returned view reaches the original storage. The corresponding checks reject unsupported interface narrowing, a cross-owner branch result and an escaping inner-region view; returning from a factory call does not publish its adapter conformance onto an unrelated fresh value in the caller. These probes preserve the existing region representation and do not add default scalar borrowing or const-place syntax.

In the supported typed-reference subset, symbolic reads retain the selected binding value's established conformance facts. Perform uses the captured reference and its facts; behave uses the current reference and its facts. Rebinding does not accumulate the old interface guarantees on the symbol itself. [Effect-conformance evidence](../tests/cod/language/effect-conformance-validation.json) checks factory-returned references without importing their adapter conformance into the consumer: direct/symbolic reads and captured calls select Weighted, while a current Alternate binding selects Entity and rejects a Weighted requirement. This preserves concrete dispatch and does not add a runtime interface tag, new reference permission or owner lifetime.

#### Extension-module callable identities and local unions

H-230 assigns distinct identities and method sets to base/ext1/ext2 versions of the same semantic callable. A using module preserves those identities when local aliases differ. When it selects those versions under the same alias, it forms a virtual method union with a fresh identity in that receiving module. This is compile-time composition, not mutation of the source modules or program-wide publication.

```text
using "base.cod" score as base;
using "ext.cod" score as extended;
function indirect(x::Sample)::i64 { base(x) }
# base defines score(Metric)=1; ext adds score(Sample)=7.
base(x);      # required: 1
extended(x);  # required: 7
indirect(x);  # required: 1
```

Concrete dispatch still uses Sample on all three calls. The candidate sets differ; entering a wrapper cannot add extended's candidates to the distinct base identity. [Historical alias observations](../tests/cod/language/selection-surface-observations.json) record the earlier1,7,7 behavior. [The current context fixture](../tests/cod/language/method-context-selection.cod) passes1,7,1 and also checks a same-alias union, using/import helpers, re-export and captured/current procedure calls. The complete local reproducer is build/validation/language-alias-example.cod.

```text
using "base.cod" score;
using "ext.cod" score;
# score names the consumer's new union identity.
```

The required composition must preserve original method definitions and source provenance. A shared inherited method reached twice is one candidate; two distinct equally specific implementations remain ambiguous under H-209. Source-defined unit constructors retain their lexical type identity and do not by themselves make an imported method context-dependent. Calls inside constructor arguments still participate in implementation selection. [Native evidence](../tests/cod/language/module-unit-validation.json) distinguishes a constructor-only shared method from two helper-selected implementations that return different results. Declaration order cannot select a winner. A union through using does not grant import's extension permission. These constraints do not require a runtime registry, new object layout or module-qualified calls. Keeping provenance of the original semantic family separately from the selected identity is a compiler representation candidate, not a new user-visible symbol mechanism. Whether unrelated same-spelled families may be explicitly fused, and how a local method extends a union selected with mixed permissions, remain outside this confirmed scope.

**Helper dependency context follows using/import.** H-232 confirms the following schematic source, with standalone export-list syntax proposed below:

```text
# base.cod
function score(x::Metric)::i64 { 1 }
function twice(x::Metric)::i64 { score(x) + score(x) }
export score, twice;

# ext.cod
import "base.cod" score;
function score(x::Sample)::i64 { 7 }
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

This refines H-212's source-preserving import contract for callable implementation context, without changing type or symbol identity, private-resource selection permissions, source globals or captured effect values. Import is not an instruction to recapture a procedure, nor to replace every source name by an equal local spelling. [Typed forwarding evidence](../tests/cod/language/procedure-context-validation.json) carries selected procedure values through eight modules before symbolic binding: perform retains source captures, behave uses current bindings, and both retain the selected callable implementation context. The separately aliased base callable and a local wrapper using that alias must still return1; import adaptation cannot leak into unrelated calls. Definitions may share source bodies and machine code where equivalent, but compiler provenance must distinguish their selected contexts.

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

[The K-363 observation](../tests/cod/language/method-subset-observations.json) preserves the historical alias-order defect. The current architecture uses identity records rather than per-alias subsets; [K-391 evidence](../tests/cod/language/world-composition-validation.json) covers that bounded repair. Compound adapted contexts and broader extension permission remain separate review boundaries.

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

[Aliased re-export evidence](../tests/cod/language/reexport-surface-validation.json) covers two explicit re-export layers for a bits global, an intrinsic function overload selected under an ordinary callable alias, and a lexical procedure. The native trace preserves one source initialization, source lexical data and the caller’s dynamic binding. Ordinary imports, old source spellings and private source resources remain unavailable at the receiving boundary. These checks add no publication mechanism or permanent fixture.

A symbolic procedure selected by export import is also explicitly public under its receiving alias. Its source captures remain intact; publishing it neither re-executes the source module nor recaptures the caller’s later bindings. [The diamond evidence](../tests/cod/language/module-order-validation.json) reaches one shared module through two branches, reuses the modifier-exported resource through another alias, and observes one initialization with the original captured value.

Export import is the confirmed shorthand; analogous modifiers on using or mixin are not selected by this statement. Standalone re-export uses the same single-name export form. Grouped export lists remain a surface-design detail outside the native prototype.

Here Sample, score and @print name existing local bindings, including selected aliases. Exporting score exposes its callable set; exporting Sample alone does not expose private functions whose signatures mention Sample. Type-associated selection must filter by the source's export boundary, preserving public result-only factories and inherited applicable methods. A public source-defined callable can still use private helpers internally without making those helpers directly selectable. UoM bundles likewise select exported aliases of their existing conversion entries; a private original spelling stays unavailable, and aliases of one formula/context do not multiply conversion paths. Ordinary callable import alone does not register a new conversion formula; D-11 keeps that broader adaptation/registration boundary explicit. An exported mixin body remains subject to its receiving-context name rules; export supplies no private-name bypass.

Privacy, explicit re-export and export import are confirmed and implemented in the named-resource subset. H-247 defines publication as public alias → identity. H-248 requires identity-centered compiler records with multiple aliases. Consumers resolve the published identity directly; the exporter's current local spelling is not another lookup step. `export import` publishes the actual imported identities, including associated methods, even when a different local binding has that spelling. [world-export-alias.cod](../tests/cod/language/world-export-alias.cod) checks both a competing local method and a frozen local helper; the imported identities remain public. A conflicting publication under the same public alias rejects. Grouped lists, per-signature publication, broader bundles and cyclic re-exports remain draft details. A type appearing in an exported signature is not automatically directly selectable. Historical [K-373 diagnostics](../tests/cod/language/export-location-validation.json) describe the superseded local-shadow publication rejection, not current acceptance.

Acceptance needs a public function using a private helper, rejected direct/private selections through all three reuse forms, alias re-export, private associated methods staying hidden, and public factories remaining available. Together with the identity increment it must check separate aliases yielding1,7,1, same-alias union deduplication and ambiguity, plus using-preserved2/import-adapted14 and export import publication without implicit re-export. Existing initialization, captures, conformance, concrete dispatch and owner checks remain relevant. [Boundary evidence](../tests/cod/language/module-boundary-validation.json) covers inherited-method name binding and publication identity at its recorded compiler revision. [The module composition record](../tests/cod/language/module-context-export-validation.json) retains its179-check compiler revision, all five composition cases and native LLVM/relocation/lifetime validation. The context fixture additionally distinguishes frozen/adapted direct procedures, perform retaining captured bias1 and behave using current bias5. Existing test-library APIs and generated composition modules explicitly export their selected resources; unrelated entry-local definitions remain private. The record does not qualify unresolved compound contexts or the complete language contract.


Lean acceptance should exercise explicit conformance without eager body completeness, rejection of undeclared conformance despite matching signatures, unrelated declared abstract constraints remaining ambiguous, and unused missing implementations remaining allowed while demanded missing implementations fail. An adapter's declared conformance must be usable through using without granting declaration permission; an unrelated same-identity view or mere WPO inclusion must not silently publish it. Different field layouts must be irrelevant to interface matching. Module cases need distributed parent/child methods, source-visible transitive methods, duplicate-path identity handling, using-only extension rejection and permitted import extension. Alias cases must show explicit shadowing of implicit conflicts, unchanged operation identity/source captures, and no fallback to a hidden operation after explicit dispatch fails. The native abstract checkpoint covers the explicit-conformance/applicability portion. The linked increment evidence covers the native association, name-priority and caller-view subset; unimplemented qualifier and owner contracts remain separate; K-400 covers the bounded typed-signature increment.

<a id="dl-02"></a>
### DL-02 — Methods, effects and entry execution

**Confirmed local direction:** functions are lexical, may operate on explicitly supplied mutable memory, and cannot access dynamic bindings or invoke procedures (H-197/H-198). Procedures may use ordinary lexical names or symbolic dynamic bindings and may access effects in either case under H-184. `perform` installs captured values temporarily; `behave` uses current dynamic bindings. Module top-level scopes survive initialization until process exit under H-166. H-180 preserves both captured and shadowed regional owners. [D-10](SymbolicDynamicScope.md) specifies callable and selected implementation contexts. The bounded source frontend implements typed dispatch; packet entry-method application and the broader ownership grammar remain separate design work.

**Question:** What is a method's meaning, and how does the receiver select and execute the entry object's designated method?

- Specify receiver binding, arguments/results, dispatch, method identity/version and access to the execution context. Decide whether definitions travel with a packet, are imported, or are resolved from already loaded definitions.
- Distinguish explicit-argument operations from ambient dynamic-context access. A database write or external operation is not excluded from a function merely because it changes external state; its required inputs and allowed intrinsic contract must be explicit. Define ordering, errors and capability binding; suspension, persistent continuations and cancellation remain separate choices.
- Keep business decisions expressible in the typed program representation. Identify which primitives are supplied by native handlers and how two environments can interpret the same business program.

**Proposal to discuss:** an illustrative `entry.apply(context) -> Result<Response, ApplyError>` contract, with methods resolved against explicit definitions and bindings. Serialized native function/member-function addresses would not provide a relocatable cross-process method identity.

**Decision evidence:** trace a method invocation, including dispatch resolution, a branch based on an effect result, a missing capability and an application error. Record the chosen semantics rather than merely the spelling of `apply`.

<a id="dl-03"></a>
### DL-03 — Regions, ownership and lifetimes

H-262 permits a named non-optional owning region to supply an effect installation; after transfer its name is unavailable at compile time, with no implicit null value. H-182’s writable optional source is a separate facility for explicit optionality. H-183 still requires the remaining ownership language to be coherent. The [regional ownership draft](RegionalOwnership.md) coordinates construction, expression categories, writable-place syntax, optionality, borrow validity and cleanup. H-263 preserves pre-existing views while the new local owner remains alive and permits access; ending that scope or sending the region out revokes access. H-264 selects the first typed let/make/root-initializer form. H-267 qualifies D-17 E1–E5 for bounded typed construction, borrowing/owning calls and exception control; H-269 retains explicit causes beyond catch. K-414 records native acceptance. Extended qualifiers, reassignment and transport remain separate.

**Confirmed local direction:** user objects are confined to stack and `@region`. The native heap is reserved for system code. H-180 reaffirms capture of permitted dynamic values; resource capture/replay does not copy the graph or consume ownership. H-173/H-175’s complete allowed-type/interior-pointer boundary remains under review. H-177 allows ordinary owning handles. Each region holds one direct-pointer graph, and all of its pointer edges stay inside that region. H-99/H-100/H-156/H-157. [Capture and lifetime discussion](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime)

**Regional access:** functions operate on explicitly supplied regional data and may modify it; they cannot read/write dynamic bindings or invoke procedures. Procedures may call functions. H-197/H-198 establishes this mutable-state baseline; a separate const function may serve future read-only requirements. Regional references preserve their enclosing region association and lifetime across nested calls and returned views. See [D-10 storage semantics](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime).

**Ownership boundary:** H-190 makes whole-region ownership intrinsic. Intra-region allocation/reuse policies belong to CoD user code, including free lists and pools. Typed slot reuse neither releases part of the region nor guarantees that an old reference still identifies the same logical item. D-14 prioritizes the missing ordinary holder, construction/result, optionality and reference-validity contract before mailbox transfer.

[Native owner/relocation evidence](../tests/cod/language/short-circuit-owner-validation.json) checks the bounded implemented lifecycle: selected short-circuit operands release each inner region before the next loop iteration; a skipped operand allocates nothing; captured and hidden owners remain live and the module owner releases last. Independent hooks observe four acquisitions/releases and eighteen valid reads. The same run validates the 120-byte shared/cyclic graph image and 152-byte reusable pool image after relocation, with10000 CoD reuse laps. These are runtime and prototype checks, not acceptance of the pending owning construction, transfer or mailbox syntax.

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
- Define efficient intra-region references and business protocols for relationships between independently managed regions. The protocol determines identity and version selection, missing targets and any access lifetimes.
- Decide how packets refer to existing entities when compaction moves them. Compare stable logical IDs and version-qualified business keys.
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

The native source prototype reports selected implementation contexts alongside ambiguous method candidates and identifies the selected procedure declaration when a function demands an effectful method. These are source/checker diagnostics, not an implemented semantic-graph IDE protocol.

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

**Confirmed:** module-defined symbols have strict identity, imported aliases retain it, and independent origins/versions remain distinct. A typed symbol declaration fixes the binding type; use requires a supplied value along the demanded call path. Reads use `@name`; writes use `effect @name <- value`, restoring on scope exit. Procedures capture used effect values; `perform` temporarily installs them, `behave` retains current bindings. Demand-driven WPO checks required bindings over caller paths; missing values are compilation errors, not runtime fallback.

**Confirmed module practice:** H-242 vocabulary modules define typed symbols, types (primarily abstract, with some supporting ADTs) and their method collections, with comments explaining their meaning. Selecting the same resolved vocabulary establishes common identities across independent module-world copies; this does not create one shared mutable world. H-242 broadens H-105's earlier symbol-only idiom. The [two-world inventory](ModuleWorlds.md#vocabulary-modules-establish-shared-identities) records the identity role. The following using/import/mixin rules describe the confirmed prototype baseline whose detailed world interaction remains under architecture review: `using` preserves the selected source definition and callable implementation context. `import` retains its source body, non-callable lexical bindings and captures, selects the receiving module's ordinary callable implementation context, and permits extension under H-230/H-232. `mixin` redefines a source body in the receiving lexical/dynamic scopes under H-212. These definition and implementation choices are separate from `perform`/`behave`'s captured/current value choice. D-10 records the native subset and pending compound-selection boundaries. H-231/H-233 requires explicit publication at every module boundary, including re-export.

**Confirmed module phases:** H-106/H-107 permit symbol/function definitions and bits globals initialized by literals or function calls, plus top-level `effect`/`perform`/`behave` expressions. Procedure definitions register during scanning; neither their definition semantics nor global initializers/top-level expressions execute during that scan. All entry dependencies are scanned/loaded before entry execution is compiled; dependencies then contribute top-level execution semantics in encounter order.

**Confirmed immediate review:** every cross-module reference triggers module execution semantics (H-109). Its top-level scope remains alive until process exit (H-166); `using`/`import`/`mixin` retain H-110’s explicit selection of effective resources. Initialization completion is not resource destruction. `effect` yields the newly bound value (H-111).

**Implementation boundary:** the native subset checks typed bindings, demanded captures and bits-global initializers. Durable package/symbol identity encoding, general capture admissibility, cyclic initialization and runtime layout/caching remain open in [SymbolicDynamicScope](SymbolicDynamicScope.md). The bounded implementation does not supply a persistent symbol ABI or independent module compilation.

**Origin/alias review:** H-112 requires explicit provenance and local aliases. H-113 excludes lexical symbolic variables and ordinary assignment. H-114 makes identities compile-time values and aliases compile-time names; runtime values bind under identities. H-115 permits a body alias map with matching-name defaults, now belonging to mixin under H-212. H-116 requires a preexisting local destination symbol for procedure selection/redefinition; the latter now uses `mixin "module" @source as @local`. [D-10's relationship model](SymbolicDynamicScope.md#symbol-origins-aliases-and-dynamic-values) separates identity selection, procedure-value installation and body translation. H-117 proposes `symbol from "module" @source as @local` and `with { @local as @source_role; }`, read as supplying a local symbol for the source body’s role. H-118 authorizes implementing these discussed forms. The compiler separates identity selection from procedure installation and resolves each redefined body’s symbol aliases before capture analysis. Broader mapping edge policies remain open; unsupported cases receive explicit diagnostics.

**Using abbreviation:** H-119 proposes `using symbol "module" @p;` to select identity and procedure together. H-120 requires an already bound same-name alias to have the source identity; otherwise ordinary using must explicitly select the destination with `as`, even `using "module" @p as @p;`. Shared vocabulary identities permit short forms. Explicit destination selection does not merge identities. The [D-10 rule table](SymbolicDynamicScope.md#combined-using-and-identity-checked-abbreviation) separates absent/same/distinct identity cases and preserves one source encounter. H-121 authorizes implementation and validation; the compiler implements this refinement with separate explicit-destination and identity-preserving cases. Mixin body maps retain H-115 semantics; the exact outer shorthand and extension-permission grammar remain separate scope.

**Decision evidence:** alias/version separation, nested effects, both call modes, transitive `behave` dependencies, a reachable path missing a required binding, scan-time absence of effects/captures and an encounter order different from dependency scan order. No independent module compiler is presumed.

<a id="dl-15"></a>
### DL-15 — Callable definitions, expressions and storage

**Confirmed:** functions are lexically defined and use explicit inputs/outputs. H-184 permits procedures with ordinary lexical names or symbolic dynamic bindings; both may access effects. Existing lexical procedures may be installed under typed symbols without redefining their bodies or captures. H-186/H-187 allows `proc @p(...) -> RT { ... }` to declare the symbol and procedure together. Explicit procedure types use `proc(T1, T2) -> RT`. H-189 gives direct lexical calls current dynamic bindings; symbolic perform alone replays captured values. Module initialization is the top-level dynamic scope. Symbol reads and writes are expressions, and `void` indicates a result with no meaningful value. User storage is stack or regions, with ordinary owners and region-backed dynamic resources as separate uses. H-180 reaffirms dynamic-value capture; shadowing preserves owners and does not release hidden regions. Physical references stay within their region. Business associations between regions follow H-157.

**Open:** expression sequencing and scope results, procedure/capture representation and lifetime checking, the first regional GADT construction contract, and business protocols for logical relationships between regions. A scope-local `effect` restores on exit under either call mode; `behave` does not make rebinding permanent. No `malloc` closure, raw stack reference capture or user-visible host address is inferred.

**Decision evidence:** lexical function lookup versus dynamic procedure rebinding, effect access rejected in functions, expression result typing, scope restoration, and capture/lifetime examples. H-98–H-104, D-10.

<a id="dl-16"></a>
### DL-16 — Language and SNN development together

H-96 establishes joint development; H-108 requests a checkpoint/push and an autonomous goal for the first demand-compiled runnable prototype after reviewing immediate choices. H-109–H-111 resolve that review; [D-10's first-prototype boundary](SymbolicDynamicScope.md#first-native-prototype-boundary) identifies the milestone. The application-owned [LanguageCoDesign](../applications/steady-snn/docs/LanguageCoDesign.md) proposes increments from nominal symbols and call modes to one immutable-A/incoming-gather/B-publication step, then a small checked program representation and native specialization. H-94's target-owned two-frame pull is the application boundary; it does not fix neuronal equations or training algorithms.

**Current increment:** H-221 authorizes ten hours of active language/runtime work, excluding waits, under the confirmed module, type, method and regional-access contracts. H-122/H-123 keeps SNN modeling and learning outside this scope. H-225–H-227 fixes associated signature membership, using/import-dependent local declarations and caller-selected ordinary implementation views. New unresolved policies remain queued for consolidated review under H-160. Application-shaped arithmetic fixtures validate programming facilities without becoming model equations.

The native subset includes nominal regional data products, abstract interfaces with explicit adapter conformance, FP32/FP64 values, typed function/procedure dispatch, parenthesized intrinsic/operator overloads, named UoMs and selected conversion routes. Bounded loops/reductions, static SoA arrays, caller-region allocation, explicit-memory mutation, borrowed traversal/results and repeated multi-module A/B gathers have native evidence. Whole-image relocation and scope-lifetime checks retain their own evidence. Source syntax and owning-region specialization are bounded prototype conventions; no full storage ABI, cross-region representation or modeling acceptance is implied. H-139–H-141 motivates complex dependencies, once-only incorporation after scanning and measured initial inlining.

Numerical source evaluation and integration motivate functions with explicit inputs. Dynamic target/frame context and ambient services motivate procedures; explicitly supplied memory can also be edited by functions. Each increment needs semantic examples and actual execution evidence before optimization or SNN learning success can be claimed. The [case file](../applications/steady-snn/research/cod-language-scope-cases.json) links selected [native fixture evidence](../tests/cod/language/validation.json). Current native records cover the typed/module/regional subset, arithmetic A/B gather, complete-image relocation and deep composition. Each record identifies its run scope and source revision; the historical full-component run is identified separately. The complete semantic catalog and neuronal behavior still require their own evidence.

<a id="immediate-increment-decisions"></a>
#### Immediate increment decisions

H-133–H-138, H-148–H-150 and H-153–H-155/H-158/H-159 resolve the following choices without requiring another lifetime mechanism or compound-unit algebra:

| ID | Decision | Current rule |
| --- | --- | --- |
| LI-01 | Result references from a function | H-133/H-197: returning an existing input-reachable reference retains its source-region lifetime. Mutation is allowed; returning a view cannot keep a locally released region alive. |
| LI-02 | UoM definition reuse | H-134: fresh definitions and one-shot using only; no UoM import. Select the intended UoMs and their associated conversions together. Partial selections exclude conversions involving omitted UoMs; repeated using does not extend the bundle. |
| LI-03 | Conversion during method selection | H-135: H-260/H-261 `amount::f64(m)` preserves unit identity while allowing lossless representation widening; `amount::f64(m*)` also permits selected unit conversions. This is a user-declared parameter policy, not a universal language restriction. |
| LI-04 | Conversion route selection | H-136: discover routes at compile time, choose a unique route automatically, warn about unresolved alternatives and allow warnings-as-errors. H-137/H-138: a direct conversion wins; otherwise use the declared indirect route, illustrated by `m*<-cm<-mm`. |
| LI-05 | Argument/conversion order | H-148: evaluate each argument once and finish its selected conversions before evaluating the next argument. An earlier failure prevents later argument effects. |
| LI-06 | Local value names | H-149: nested scopes may shadow outer locals; declarations within one scope must be unique. The initializer sees the outer value, and scope exit restores its visibility. Function parameters and loop bindings share their body’s declaration scope. |
| LI-07 | Reserved expression names | H-150: reject literal/control spellings in parameters, `let`, loop indices and fold accumulators; retain usable contextual names and field selectors. |
| LI-08 | Intrinsic/operator overloads | H-153–H-155: reserve ordinary keyword names and allow parenthesized operation methods of either callable kind. H-158: intrinsics use compiler-internal representation, separate from CoD symbols. H-159: a selected procedural operation uses current bindings, like `behave`. UoM conversions remain pure. |

The run develops structured product values, named UoMs with these selection/conversion rules, Julia-style argument-type methods and parenthesized operator names, bounded numerical iteration, static SoA regional arrays and owner-tracked explicit-memory traversal. Regional references retain existing lifetime/relocation rules, including H-146’s accepted first-field self-reference-to-null limitation. [Local value shadowing](SymbolicDynamicScope.md#local-value-shadowing) follows H-149; symbolic writes still require `effect`. Compound-unit algebra, general GADT machinery, custom operator precedence and cross-region representation remain separate increments. H-161 reserves primary module-form words against ordinary declarations and aliases. H-162 selects the distinct `(=)` family; H-163/H-164 make indexed assignment call `([]=)(array, index, value)` while preserving nominal container/index types. General lvalues, record-field assignment and compile-time field-selector overloads remain open; native `setfield` provides field writes. The completed H-147 work period and H-160 review retain their historical scopes. No new SNN dynamics or learning rule enters this work.

Validation uses CoD language/runtime component checks and arithmetic fixtures: strict versus convertible parameters, using-bundle selection, route preference/warnings, structured index conversion, rejected domain mismatches, nested alias-sensitive reads and writes, returned input references, and repeated numerical gather/A-B execution. Existing numeric failure behavior supplies a reference for invalid arithmetic; no new exception/recovery framework is required. Source-only compilation uses the repository's active build tree and normal build command; build configuration and toolchains remain unchanged.

## Implementation evidence to account for

Storage observations below describe repository commit `b72669e5d2bd9374e3aa69d6dd2c4ca1a5ca94c6`. The driver row links the current native prototype with hashed execution evidence. Implementation observations do not grant design authority.

| Existing facility | Relevant observation | Topics |
| --- | --- | --- |
| [Regional storage](../include/shilos/region.hh) | `regional_ptr` is self-relative; allocation advances `occupation_`; capacity and occupied bytes are distinct. The current `RegionalType` concept is a destructibility check, not a recursive proof of legal regional fields. | DL-01, DL-03, DL-04 |
| [Shilos C++ runtime pointer](../include/shilos/region.hh) | The host `global_ptr` facility holds a native reference to a region and an offset. CoD business-level identity and resolution have a separate design scope. | DL-03, DL-08 |
| [DBMR](../include/shilos/dbmr.hh) | Mapped-region validation checks header/root conditions. `constrict_on_close` trims unused tail capacity; it does not rebuild a graph to reclaim unreachable internal allocations. | DL-04, DL-06, DL-09 |
| [CoD driver](../cod/main.cc) | The CoD source path checks the bounded typed/module/regional subset and emits LLVM IR/bitcode for native compilation with small system services. [Prototype reference and evidence](../tests/cod/language/README.md) cover its products/UoMs, regional arrays/records, explicit-memory functions, captured effects and call composition. General GADT construction and a stable language/interoperation ABI remain open. The C++ submission path is separately available. | DL-02, DL-12 |

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
| DD-049 | 2026-11-06 | Preserve the temporary first-field self-reference-to-null behavior; future intrinsic fields are the expected resolution, with layout/compatibility still open. | H-146; DL-03; D-10r16 |
| DD-050 | 2026-11-06 | Continue the approved language/runtime work for ten additional active hours, excluding waiting and keeping separate accounting from the preceding completed goal. | H-147; DL-16 |
| DD-051 | 2026-11-06 | Complete each argument’s selected conversions before evaluating the next argument, consistently across functions, procedures and operator spellings. | H-148/P-128; DL-02/DL-15; D-10r17/D-11r10 |
| DD-052 | 2026-11-06 | Allow nested-scope local shadowing and reject duplicate declarations within one scope. | H-149/P-129; DL-15/DL-16; D-10r19 |
| DD-053 | 2026-11-06 | Reject reserved expression names in lexical value declarations, preserving contextual names and field selectors. | H-150/P-130; DL-15/DL-16; D-10r21 |
| DD-054 | 2026-11-06 | Use one direct-pointer graph per region and express inter-region relationships through business values and protocols. | H-156/H-157/P-140; DL-03/DL-08/DL-15; D-10r25/D-13r1 |
| DD-055 | 2026-11-06 | Reserve ordinary keyword names; allow function and procedure overloads under parenthesized intrinsic/operator names. Intrinsics are compiler-internal operations, distinct from CoD symbols; a selected procedural operation uses current dynamic bindings. | H-153–H-155/H-158/H-159/P-136/P-137; DL-01/DL-02/DL-15/DL-16; D-10r30/D-11r12 |
| DD-056 | 2026-11-06 | Defer new user-required choices until a consolidated review after this active-work period; continue confirmed work autonomously. | H-160; DL-16 |
| DD-057 | 2026-11-06 | Reserve `type`, `regional`, `unit`, `array`, `using`, `import` and `conversion` against ordinary declarations/aliases; retain field and symbol meanings. | H-161; DL-01/DL-15; D-10r34 |
| DD-058 | 2026-11-06 | Use a distinct `(=)` family; indexed assignment supplies the original container, index and value to the separate `([]=)` family. | H-162–H-164; DL-01/DL-02/DL-16; D-10r34/D-11r13 |
| DD-059 | 2026-11-06 | Entry and dependency module top-level scopes end at process exit. | H-166; supersedes H-110’s teardown timing; D-10r35 |
| DD-060 | 2026-11-06 | Analyze effect-only whole-region holding and static post-send emptiness; intrinsic mailbox feedback covers local send errors, not delivery confirmation. | H-167/H-168; D-12r3/D-13r3; uniqueness and local-error ownership remain open |
| DD-061 | 2026-11-06 | Moving a whole-region owner with `effect @b = @a` empties the source binding. Analyze a returned-region sum for local send errors; exact sum and binding-presence syntax remain proposals. | H-169/H-170; D-13r4 |
| DD-062 | 2026-11-07 | Use only `effect @name <- rhs` for dynamic binding; preserve typed transfer/copy behavior, scope restoration and expression results. | H-172; D-10r37 |
| DD-063 | 2026-11-07 | Investigate limiting dynamic bindings to owning-region resources, bits values and statically known procedures; analyze remaining lexical/capture lifetimes and branch precision before adoption. | H-173/H-174; proposal pending-human; D-13r5 |

| DD-064 | 2026-11-07 | Procedures capture required dynamic-variable symbol identities, never definition-time values or binding frames; current bindings supply runtime values. H-175 proposes rejecting interior-pointer capture and deriving ambient views from the current owner. | H-175/H-176; D-10r38/D-13r6; H-97 value-capture semantics superseded, call-spelling/API details open |
| DD-065 | 2026-11-07 | Ordinary variables may own whole regions, preserving unique ownership and allowing runtime-created user regions to be constructed, edited and ultimately sent. | H-175/H-177; supersedes H-167’s effect-only placement premise; D-13r6; owning-return/root/mailbox interfaces not yet implemented |
| DD-066 | 2026-11-07 | Keep cross-process regional packets separate from region-backed dynamic resources. Capture used dynamic values at definition; perform shadows them temporarily without losing ownership or destroying hidden/captured regions, then restores caller lookup. | H-180; supersedes H-176 identity-only capture; D-10r39/D-13r9; ordinary owner installation remains consuming under H-179 |

| DD-067 | 2026-11-07 | Dynamic regional resources cannot be consumed out of symbolic bindings; shadowing preserves owners. WPO checks supplied bindings independently of value type. Direct region references need no implicit Maybe, while explicit Maybe<SomeRegion> remains valid domain modeling. | H-181; D-10r40/D-13r10; supersedes H-169 consuming effect moves, confirms H-178 boundary; typed symbol and sum syntax not yet implemented |

| DD-068 | 2026-11-07 | Analyze fresh regional construction or writable optional storage as effect initializer sources; all optionality forms need coherent semantics, and new region construction needs dedicated syntax/semantics. | H-182; proposal pending-human; D-14r1 |
| DD-069 | 2026-11-07 | Design ownership jointly across syntax, semantics and practical use; ordinary-owner terminology is not an already defined source construct or an implicit moved-out policy. | H-183; D-14r1; no source-category choice confirmed |
| DD-070 | 2026-11-07 | Permit lexical and symbolic procedure definitions and installing an existing lexical procedure as an effect. Require explicit types on each symbol definition, including procedure types, with grouped declarations. | H-184; D-10r41; supersedes H-103 symbol-only restriction; procedure-type grammar and complete direct-call effect contract remain open |

| DD-071 | 2026-11-07 | perform/behave target only symbols and make dynamic calls; direct procedure calls use lexical lookup. Do not admit perform p or behave p for ordinary names. | H-185; D-10r41; captured/current semantics of symbolic calls remain |

Dates above are recording dates. Exact source-message timestamps were unavailable; the registry records that provenance limit.

| DD-072 | 2026-11-07 | Typed/grouped symbols, explicit proc signatures and lexical/symbolic declarations are implemented under H-188. A missing symbolic proc identity is declared together with its signature. Direct lexical calls use the caller dynamic context; perform restores captures. Dynamic region extraction is rejected. | H-184–H-189; K-305; D-10r44; native evidence retains its exact scope. |

| DD-073 | 2026-11-07 | Whole-region ownership is intrinsic to CoD; intra-region allocation/reuse policies, including free lists and object pools, belong to user programs. Validate the missing ordinary-holder/constructor/result/optionality lifecycle and library sufficiency separately. | H-190; P-162; K-306; D-14r3; no new object ownership or allocator mechanism selected. |

| ID | Recorded | Direction / decision | Authority and limits |
| --- | --- | --- | --- |
| DD-074 | 2026-11-07 | User-defined type objects are region-internal and use `type`, without a `regional` modifier. References retain compile-time region provenance for escape/access analysis. Discuss post-send availability without implicit NPEs together with function/procedure parameter permissions and lifetimes. | H-193/H-194; P-164/P-165; K-310; D-14r7; make, named-source post-state, parameter-mode and sum syntax remain pending. |
| DD-075 | 2026-11-07 | Explore functions whose restriction is dynamic-effect-variable access, permitting explicit region allocation/editing and lexical cleanup. Analyze replacement of read-only snapshots, parameter permissions, returned owners and the explicit-intrinsic boundary. | H-195; P-167; K-310; proposed, not a confirmed change to function semantics. |
| DD-076 | 2026-11-07 | Any type containing a region owner must be intrinsic; user-defined types cannot embed or wrap owners directly or transitively. Intrinsic optional/result types are required if they carry returned ownership. | H-196; P-168; K-310; exact intrinsic outcome/optionality syntax remains open. |
| DD-077 | 2026-11-07 | Adopt ordinary functions with explicit memory operations and explicit-argument mailbox send while excluding dynamic-variable access; implement and validate the supported subset. Explore distinct owning/borrowed intrinsic region types. | H-197; P-167/P-169; K-311; exact ownership syntax and transport remain open. |
| DD-078 | 2026-11-07 | Ordinary functions have language-provided ST-like mutable-state semantics. Future read-only business requirements may use a separate const-function design; remove the read-only fallback plan. | H-198; P-170; K-311; no monadic syntax or full runST equivalence is implied. |
| DD-079 | 2026-11-07 | Prefer ownership modifiers such as owned region<RT> over the region_ref spelling; compare forms and discuss before selecting syntax. | H-199; P-169; K-311; explicit borrowed versus a borrowed default remains pending. |
| DD-080 | 2026-11-07 | Use unqualified types for borrowed access and owned for lifetime responsibility; no borrowed keyword. Explore Packet in place of ref<Packet> and make local for stack bits; non-bits objects remain region-only. | H-200; P-169/P-171; K-312; detailed scalar/local/result and field-layout rules are separately scoped. |
| DD-081 | 2026-11-07 | Const-qualified parameters/results are non-borrowing values; unqualified ordinary parameters/results borrow. For bits, this separates independent evaluated values from shared storage views. | H-201; P-172; K-312; const non-bits semantics and const function remain separate design work. |
| DD-082 | 2026-11-07 | Const non-bits variables use read-only borrowing: the holder cannot mutate the target, but other writable aliases may change observed contents. Track region/lexical storage to reject outliving access. | H-202; P-173/P-164; K-313; no snapshot, graph freeze or lifetime extension. |
| DD-083 | 2026-11-07 | References stored in another object must target storage with the same allocation owner, whether a region or lexical scope; otherwise reject at compile time. | H-203; P-174; K-313; ordinary borrowed bindings retain target provenance across calls. |
| DD-084 | 2026-11-07 | Develop = as reference assignment and <- as the ownership/bits transfer operator: bits copy their payload; owned regions transfer one responsibility. | H-204/H-205; P-175; K-313; replacement, expression results and overload migration remain pending. |
| DD-085 | 2026-11-07 | The result of a <- b is a const reference to a's resulting value. Support bits-field transfer; reference-bearing objects require explicit user graph conversion, with dispatch on both region root types including abstract memory-policy interfaces. | H-206; P-176/P-177; C-55/P-178/P-179; K-314; detailed abstraction/overload grammar remains pending. |
| DD-086 | 2026-11-07 | Ownership moves use explicit steps; non-consuming transfers may chain through the const destination result. | H-207; P-176; K-314; no consumption through a read-only result. |
| DD-087 | 2026-11-07 | Transfer function/procedure overloads receive actual references to both regions to invoke their memory-management methods. | H-208; P-180; K-314; owner pairing and permissions must remain checked. |
| DD-088 | 2026-11-07 | <- is right associative; dispatch ignores declaration order and uses a unique most-specific applicable method, otherwise errors. | H-209; P-182/P-183; K-315; owned chains still fail through const intermediate results. |
| DD-089 | 2026-11-07 | Use type for abstract types, data for product/sum representations, and <: to declare subtype relationships. | H-209/H-210; C-56/P-184–P-187; K-315; detailed conformance, hierarchy and constructor grammar remains pending. |
| DD-090 | 2026-11-07 | Defer multiple declared parents. Data-to-abstract <: is informational; satisfaction is structural without requiring all visible method implementations. Abstract-to-abstract inheritance remains nominal. | H-211/H-213; P-188; K-316; structural matching uses method signatures, not data structure. |
| DD-091 | 2026-11-07 | Selecting a type via using/import also brings its source-visible associated methods. Using permits use; import permits extension. | H-211/H-212; C-57/P-189–P-191; K-316; method membership and extension scope still need precise contracts. |
| DD-092 | 2026-11-07 | Mixin carries receiving-context source-body redefinition; import aligns with Julia-style identity-preserving selection and extension permission. | H-212; C-43/C-46/C-57; K-316; compiler syntax migration is pending. |
| DD-093 | 2026-11-07 | Structural data-to-abstract matching concerns method signatures only, not data structures. Bodies need not all be available in the current scope. | H-213 clarifies H-211; P-188; K-316; full signature/requirement syntax remains pending. |
| DD-094 | 2026-11-07 | Modules do not occupy local names; callable references use explicit source selections and local aliases, with no qualified function/procedure names. | H-214; C-45/C-57/P-192; retain this boundary unless a serious design obstacle warrants review. |
| DD-095 | 2026-11-07 | Review a separate import-as declaration for method-local aliases. | H-215; P-193; K-317; overload-family selection and detailed collision rules remain proposals. |
| DD-096 | 2026-11-07 | Nominal abstract subtyping inherits methods; data subtyping is an interface claim. There is no Interface body; methods may be distributed across modules, with missing implementations diagnosed on demanded calls. | H-216; C-56/P-188/P-194/P-195; K-317; no eager closed-interface completeness check. |
| DD-097 | 2026-11-07 | Implicitly brought method names occupy a lower-priority namespace that explicit names may shadow. | H-217; C-45/C-57/P-193; K-317; lookup/dispatch and unresolved-conflict details are proposed elaboration. |
| DD-098 | 2026-11-07 | Require explicit data-to-abstract conformance; allow multiple head-only declarations for the same data identity in different modules. No implicit structural conformance. | H-218; P-196; K-318; supersedes the informational/implicit matching part of DD-090/DD-093 and narrows the earlier multiple-parent deferral to abstract inheritance. |
| DD-099 | 2026-11-07 | Adopting another module's conformance requires explicit import; using and WPO inclusion are insufficient. | H-219; P-197; K-318; exact selector/bundle/re-export syntax remains pending. |
| DD-100 | 2026-11-07 | Adapter modules must import the ADT to declare conformance; consumers may use the adapter to obtain conformance without extension permission. | H-220 corrects DD-099/H-219's consumer restriction; P-197; K-318. |
| DD-101 | 2026-11-08 | = result and source access | H-222–H-224 requests right association, read-only RHS and a const destination result in the overload signature. Writable RHS access is not needed for chaining. Reference-binding permissions and const bits/non-bits result interpretation remain under discussion; no qualifier implementation is claimed. |
| DD-102 | 2026-11-08 | Type-associated source-visible methods include result-only factories, signature wrapper occurrences and inherited applicable methods. | H-225; C-57/P-190; D-07r61; K-340/K-341 native subset. |
| DD-103 | 2026-11-08 | Local declarations shadow using-only implicit method names and extend a unique family after importing the associated type; conflicting families require explicit callable selection. | H-226; C-57/P-193; D-07r61/D-10r62; K-341. |
| DD-104 | 2026-11-08 | Ordinary calls carry caller-selected implementation views under stable lexical callable identities; compiler operations/intrinsic fallback remain lexical in this increment. | H-227; P-191/P-195; D-07r61/D-10r62; K-341; no WPO-wide publication. |
| DD-105 | 2026-11-08 | Always dispatch on the concrete data type; broad abstract constraints preserve established explicit conformance. | H-229; P-195; K-348 native checkpoint. |
| DD-106 | 2026-11-08 | Callable extensions have module-scoped identities; distinct aliases preserve method sets and same-alias selection creates a local virtual union identity. The alias example must yield1,7,1. | H-230; P-202; helper dependency context remains open. |
| DD-107 | 2026-11-08 | Module resources are private unless explicitly exported; using/import/mixin cannot select private resources. | H-231; C-58/P-199–P-201; grammar and bundle details remain draft. |
| DD-108 | 2026-11-08 | A source-defined helper selected through using retains its source implementation context; import adapts its callable dependencies to the receiving module. The twice example returns2 versus14. | H-232; P-191/P-202/P-203; this refines callable context without general lexical body redefinition. |
| DD-109 | 2026-11-08 | Re-export must be explicit; export import performs the normal import and exports all resources it imports. | H-233; C-58/P-200/P-201; source privacy and consumer extension permission remain separate. |
| DD-110 | 2026-11-09 | Independent `unit m;` declarations, quantity types `f64(m)`, representation-preserving `m(value)` construction and starred quantity conversion parameters. | H-260; P-231; K-402 native acceptance; full fraction support deferred. |
| DD-111 | 2026-11-09 | Automatic widening preserves every source-type value; exact methods precede unique most-specific widened matches, and builtin mixed scalar arithmetic uses a unique least safe common representation. | H-261; P-231; K-402; narrowing, reference covariance and fraction-specific promotion are not implicit. |
| DD-112 | 2026-11-09 | Named ordinary owners use a compiler-tracked consumed state after transfer; no implicit null value or mandatory optional storage. | H-262; P-232; K-404; construction, reinitialization, local alias survival and consuming call/result syntax remain separate. |
| DD-113 | 2026-11-09 | Preserve borrowed views across local ownership transfer while the new owner remains alive and permits access; the consumed source name remains unavailable. | H-263; P-233; K-405; scope end or sending out revokes access; failed-send alias revival remains separate. |
| DD-114 | 2026-11-09 | Use `let packet :: owned region<Packet> <- make Packet(capacity = bytes) { sequence <- initial };`, with runtime capacity and direct regional root initialization. | H-264; P-234; K-405; allocation failure and complex graph initialization remain separate. |
| DD-115 | 2026-11-09 | Plan try/catch/finally control; ultimate out-of-memory is an unchecked exception. | H-265; C-63/P-240; D-17r1; K-406; detailed syntax, payload, catch/finally and lowering remain pending. |
| DD-116 | 2026-11-09 | Accept bounded E1–E5 exception/local-region lifecycle, explicit causes only, and retained causes beyond catch with fallible creation. | H-267/H-269; D-17r9; K-414 records focused native acceptance; general RTTI, transport and qualifiers remain separate. |
