# Module worlds and flat TLS bindings

Design revision: 18 · Recorded: 2026-11-12 · Status: module-world architecture audited; bounded typed symbolic bindings implemented.

## Design authority

- **Initialization cancellation:** [H-380](DesignAuthorityRegistry.md#h-380)/[K-536](DesignAuthorityRegistry.md#k-536), D-15r18, confirms P-339–P-341: omit the affected dependency initialization TLS writes, preserve operand evaluation/effects/failures and procedure-local installations. The H-378 value path and existing owner/cleanup constraints continue under charter §§2–3/§5.2/§6.3/§8/§12. Acceptance distinguishes those observations and final-image removal of genuinely unused code; identity/conditional-coverage and detailed native mechanisms remain design work.

- **Callable-world and AOT scope:** [H-372–H-377](DesignAuthorityRegistry.md#h-372)/[K-534](DesignAuthorityRegistry.md#k-534), D-15r17, records module registration, symbol objects, whole-family publication and dispatcher views separately from runtime TLS installation. C-35/C-36/C-57/C-59/C-60 and P-330–P-332/P-335/P-336 preserve explicit semantic construction, typed execution and inspectable business dispatch under charter §§2–3/§5.2/§6/§8/§12. [H-378](DesignAuthorityRegistry.md#h-378)/[K-535](DesignAuthorityRegistry.md#k-535) confirms [D-21](AheadOfTimeMultipleDispatch.md)'s P-333/P-334/P-337 value path for the stated behavioral scope. Detailed namespace, type-evidence/ABI, coverage and diagnostic mechanisms remain pending-human. H-379's entry-installation requirement is bounded by H-380: initialization-only suppression with operand evaluation preserved. Composition identity/conditional coverage and detailed native mechanisms remain open. Existing qualified invariants continue only within their unchanged scopes. Acceptance requires the composed family/context, installation/capture and cleanup scenarios in D-21; current implementation evidence does not establish that new behavior. No root-charter change or whole-document qualification is inferred.

- **Symbolic implementation scope:** [H-287–H-291](DesignAuthorityRegistry.md#h-287)/[K-426](DesignAuthorityRegistry.md#k-426) records the requested implementation and callable/result decisions: every effect yields void; procedure rebinding uses `=`, while the recorded inline `effect proc` implementation introduces or signature-checks its symbol and constructs captured values. H-372/H-373 supersedes its implicit-registration/installation and no-extension interpretation; the compiler migration remains outstanding. C-36/C-54/C-65/C-66 and P-248–P-254 preserve typed dynamic context, actual-region lifetime and explicit capture. [Native evidence](../tests/cod/language/symbolic-context-validation.json) covers typed TLS payloads, staged groups, touch/replay checks and overload-backed object transfer. This bounded scope does not qualify whole-document drafts or implement ordinary ptr migration, transport, runtime closures or fractions.

- **Registry / scope:** [D-15](DesignAuthorityRegistry.md#d-15), revision18, all sections; H-257/H-258 and [K-399](DesignAuthorityRegistry.md#k-399) separate typed callable declarations from executable methods; [K-400](DesignAuthorityRegistry.md#k-400) records its bounded native acceptance. [K-396](DesignAuthorityRegistry.md#k-396) records the completed architecture audit on the K-395 compiler, including H-253 conflict rejection, H-252 warning controls and cross-world TLS. Earlier checkpoints retain their actual scopes. H-234/H-237–H-249 supplies independent module worlds, separate typed TLS, vocabulary identities, receiver-local composition and identity-based publication; H-245 supersedes cross-module editing. H-375 answers H-251’s actual-type-dispatch behavioral question; physical type evidence and reflection remain separate design work. Earlier decisions/evidence retain their scopes in the registry. Full contracts and candidate value paths remain pending-human; this revision does not qualify unresolved mechanisms.
- **Value lineage:** C-59/P-204/P-209 relates receiver-owned semantic construction to C-10 and charter §§1–4. P-205 preserves explicit publication and local identity merging. H-245/P-216 governs world isolation and the compile-layout/runtime-binding distinction; P-206 is superseded. P-210–P-212 distinguishes the entity sets and typed-pointer roles. H-241/P-213 places module worlds in D-12's vision; H-242/C-60/P-214 supplies common vocabulary identities. H-252/P-219 requires a diagnostic for multi-alias extension while retaining shared identity; H-253/P-220 requires conflicting inherited/selected overloads to reject during import adaptation. Full document value paths remain pending-human. P-207/P-208 are dependencies, not authority.
- **Human baseline:** H-96/H-97/H-104/H-111/H-184–H-189 supplies typed symbol identities, aliases, scoped bindings, procedure captures and call modes. H-180/H-181 preserves hidden/captured owners and forbids extraction from dynamic resources. H-234–H-242 supplies the two-world inventory and vocabulary idiom, subject to H-245's explicit correction of cross-module editing. H-245 also requests the replacement active goal. H-235 keeps invocation machinery internal; H-236 requires architecture before local fixes.
- **Invariants / non-goals:** A module constructs its own world; it cannot edit another world at either stage. Source selection reads published information and constructs receiver-local information. Symbol identity, type identity, a module-world copy and a referenced value are distinct. TLS is a flat typed-binding collection, not another type/method catalog or permission to edit a source world. Preserve capture and regional lifetime/ownership contracts. No cross-module world editor, editor-target effect, live method-table mutation protocol, public pointer ABI or transaction scheme is introduced.
- **Acceptance / evidence:** [K-396's audit](../tests/cod/language/world-architecture-validation.json) establishes receiver-owned construction, source isolation, explicit publication, identity-local composition, flattened TLS/type checking, native rebinding/restoration, captures and owner preservation. It combines source inspection with 42 direct checks on one compiler revision. This implements H-245's architecture in the WPO native prototype; it does not qualify the full document or establish a persistent world ABI, general runtime callable representation or Actor lifecycle.

## The two sets of entities

A module world is a module's own semantic runtime environment, containing registered symbol objects, types, callable contracts, methods and related semantic information. Runtime values and module initialization state have their own construction events. It is more than a source-name or declaration index. Different modules have independent world copies and explicitly select what to publish and integrate.

A TLS world is the thread's **flattened set of typed symbolic bindings**. Each symbol has its own identity and required type identity. Bits payloads are stored directly in TLS. Non-bits object bindings reference values in regions supplied by region symbols, with actual region lifetime checks; scoped rebinding may select a different region. Values may originate from different module-world copies without importing those worlds into TLS. Region-resource and procedure-valued bindings retain their distinct ownership/capture contracts.

These worlds do not model the same set of entities. The relevant connection is **typed reference to a value**, not a pair of matching world records representing that value.

## Module-world inventory

| Entity or information | What belongs to the module's world | What must stay distinct |
| --- | --- | --- |
| Type | Type identity and its world-local type information: abstract relationships, data representation and other admitted type semantics. | Type identity versus the address of one copy of its type information; a type versus a particular value of that type. |
| Function/procedure | A callable identity’s declarations, methods and selected interpretation. | A family, one method, a materialized dispatcher value and an invocation. |
| Overloaded method | Signature, body, origin and membership in the callable's available method set. | A world-local method set versus a process-global shared mutable method table. |
| Explicit type relation | Selected abstract inheritance and data-interface declarations. | Declared conformance versus inference from similar fields or a value's storage address. |
| Symbol object | Identity, declared TLS value contract and module-world semantics, including callable methods where applicable; aliases name this object. | Bare symbol lookup versus reading its current TLS value with `@`; no second callable alias is introduced. |
| UoM/conversion | Unit identities and the selected pure conversion definitions. | A unit type versus a particular measured amount/index value. |
| Module globals and initialization | Permitted global declarations/values and the initialization semantics that establish module resources. | Module state versus an automatic thread-local copy of every global; exact sharing/copying details cannot be inferred from a host compiler container. |
| Names, selection permissions and exports | Source-local access to the world's definitions, extension permission and explicit publication. | Aliases versus identity; local availability versus onward export. |

Types, functions/procedures and their methods are the core entities in H-234. The additional rows locate existing language facilities; they do not add new categories of heap object or public syntax.

A symbolic procedure definition registers or extends module-world method information during compilation; it does not install TLS or capture runtime bindings. A lexical callable-value construction still creates a runtime value. Explicit installation from a symbol object materializes its compatible family view; installation of an existing value retains that value’s environment. The combined effect form requests both semantic registration and runtime materialization/installation without merging those responsibilities.

Each receiver integrates exported information for identical identities in its own module world. The operation does not silently mutate the source worlds. Physical copying, sharing of immutable definitions and regional representation must preserve that independence; this inventory does not choose their implementation.

## Identity records, aliases and publication

Identity is the key of semantic information. A module can have several local aliases and several public aliases for that identity. Aliases provide access; they neither own separate copies of the method set nor redefine the object. Selecting more information for the same identity merges into the receiving world's record, so every alias of that identity sees the locally combined information (H-234/H-248).

Import selects a receiving implementation context for a source callable. That receiver records one imported identity per source callable identity; different aliases of that import share its information. Scanning may register local overloads before their import is encountered. When an explicitly imported alias identifies their callable, those declarations join that receiver record. A second imported alias does not create another interpretation context. Distinct source/extension identities and explicit same-alias unions retain their own identities. This is compiler construction metadata, not RTTI or a runtime identity layout.

If one module declares overloads through several local aliases of the same callable identity, the compiler warns with `[-Wmulti-alias-extension]` and identifies the aliases and declaration locations (H-252). All aliases still access the shared method information; the diagnostic makes that non-obvious consequence visible. Prefer one alias for declarations within a module. Merely selecting several aliases, or declaring several overloads through one alias, does not trigger this warning. `-Werror=multi-alias-extension` promotes this category to an error; `--warnings-as-errors` promotes all language warnings. The unit-conversion warning remains separately selectable.

An imported helper retains the source-to-receiver sequence of its selected implementation contexts. Each receiving interpretation considers inherited definitions and the source-related callable information selected in that world, by identity. Several aliases of one identity contribute its information once. A repeated view of a declaration supplies its selected interpretation only through the same identity or its explicit receiver import. Sibling identities remain distinct even when they share a source body, so nested helper interpretations cannot silently replace one another. A distinct overload definition remains a dispatch candidate. Ordinary typed dispatch must have a unique best candidate. In H-253's case, A's score(Sample)=7 and B's score(Sample)=11 conflict when B is selected while importing A's helper, so compilation rejects and identifies both definitions.

This candidate comparison does not merge the published identities or edit any source world. Direct calls through distinct aliases still select those identities, while `using` preserves the helper's existing interpretation. `import` adds the receiving interpretation without erasing inherited overloads. Explicit syntax for informed conflict arbitration is a future design possibility, not a current feature. Source globals, non-callable names, procedure captures and runtime data layout retain their separate contracts. No arbitrary choice by alias order, original ancestor or preferred source identity resolves a conflicting overload.

Export means **make an identity importable under a public alias** (H-247). For a symbol-identified callable it publishes the entire method set available in that module (H-374); no per-method private subset is hidden inside the published family. A bodyless contract can genuinely have no methods. View compatibility constrains usable calls, not publication privacy. Public aliases are a separate mapping from local lookup names. An ordinary `export name;` publishes the identity selected by that local name. `export import` publishes the identities selected by the import, including admitted associated methods. A competing local name does not retarget that publication. Two public aliases may expose the same identity; a single public alias cannot denote two conflicting identities. Existing selection-as followed by export supplies renamed publication without a new grammar form.

The compiler represents this distinction directly:

| Information | Authoritative record and access |
| --- | --- |
| Callable information | Receiver-owned identity records contain methods and origins. Explicit and low-priority local alias maps point to those records. A virtual union refers to its member identities in the receiver. |
| Type information | Definitions have compiler identities; local type names resolve to identity-bearing types. Selected conformance belongs to the receiving world. |
| Symbols | Local aliases resolve to symbol IDs; declared types belong to those IDs. Pending source selectors are scanning state, not runtime bindings. |
| Globals | Values are indexed by declaration identity; local aliases point to them. |
| Publication | Public alias maps to a typed identity: symbol, type, callable or global. Consumers resolve that identity directly instead of looking up the exporter’s local spelling again. |

Source selection receives a read-only source module. It reads the source's published identity and information, merges permitted information into the receiver, binds a local alias and optionally publishes that identity. Mutable receiver construction and runtime TLS changes remain separate. Shared immutable definitions are compatible with independent worlds; shared mutable selection tables are not.

These operations give implementation helpers their purpose: declare an identity, select source information, merge a local record, bind an alias, combine a local union, or publish an identity. H-246/H-249 requires deriving such operations from the architectural contract and removing obsolete bookkeeping, rather than preserving helpers merely because earlier code used them. This is an engineering rule within confirmed semantics; it does not settle unrelated dispatch, ownership or syntax choices.

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

A vocabulary can publish a callable with typed signature declarations and no implementations. H-257/H-258 separates its identity, declared contracts and executable methods. Each receiving world merges those distinct information sets by identity. Missing implementations matter when calls are demanded; importing the vocabulary does not fabricate a body or install a dynamic procedure value. [D-10](SymbolicDynamicScope.md#callable-declarations-and-methods) specifies generic/variadic signatures and the bounded calling rules.

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
| Current typed binding | Contains bits directly, or a permitted reference. Object-reference rebinding may select another region-symbol-backed region when type and lifetime checks hold. |
| Dynamic scope behavior | Effects shadow the current association and restore the preceding association on scope exit, following the binding/capture contracts. The flattened active collection does not require a dictionary or a separate TLS namespace per module. |

The [symbolic contract](SymbolicDynamicScope.md#symbolic-storage-and-regional-provenance) distinguishes inline bits, regional object references, whole-region resources and procedure bindings. Bits require `effect @x <- rhs`; object references use `effect @x = rhs`. These storage categories do not put type definitions, method sets or region contents into TLS. No raw symbolic assignment is permitted.

The flat collection describes active entries. Saved values/pointers needed for scope restoration and procedure captures are supporting execution semantics, not a second catalog of module definitions. This does not prescribe a heap-allocated scope frame or runtime closure.

The pointer carrier alone does not redefine capture by value, bits transfer or resource ownership. Capturing a bits value must still preserve the required value; retaining a pointer to mutable storage is not automatically equivalent. Capturing a regional reference preserves its existing referent/lifetime relationship, without deep-copying the region. Representation must satisfy these existing semantics rather than replacing them.

## Values from different module-world copies

```text
Module world A                      Module world B
  type information with ID T         type information with ID T
  value a : T                        value b : T
            \                         /
             \                       /
TLS for one thread:  S :: T  -> a   # Object reference; region-symbol backing required.
                     S :: T  -> b     after a scoped rebinding
```

The two values need not have the same value identity or address. Their originating module-world copies remain distinct. A is not merged into B, and neither world is imported into TLS by the rebinding. The symbol's type requirement is satisfied through type identity compatibility.

Conversely, a same-spelled type with a different identity is not interchangeable merely because its fields or representation look similar. The precise compatibility admitted for abstract/sum/conversion cases follows the type contracts; this inventory does not invent implicit conversions or erase nominal distinctions.

Type compatibility answers whether a value can be associated with S. It does not by itself choose a method set, redefine a callable's source context, grant ownership or extend the value's lifetime. Those are distinct obligations. In particular, origin in a different module world is not a reason to mint a new type identity or create a second per-module TLS entry.

The [cross-world TLS probe in K-395](../tests/cod/language/world-conflict-validation.json) establishes shared typed data bindings across a vocabulary, two source modules and a receiver. Captured/current reads, loop shadowing/restoration and nominal-type/missing-binding rejection are exercised. Its emitted IR has two pointer-valued TLS entries for two shared data symbols, including their aliases. Captured regional references preserve the referent, so authorized changes to that referent remain observable; capture does not deep-copy the region. Region/procedure specialization remains separate from the two native data slots.

<a id="rtti-current-implementation-and-decision-boundary"></a>
## Actual-type dispatch and concrete-type evidence

The current CoD source-language prototype has compile-time type identities and concrete-type method selection, but no general runtime type-information facility. Compiler Type/Value metadata retains identity and provenance; demanded calls are checked and lowered with the known concrete types. User data layouts contain their declared fields, without an automatically inserted type tag, vtable or method-world pointer. Native scalar/reference TLS slots carry a pointer to the value, without a runtime type descriptor. This is a statement of the current bounded implementation, not a decision to omit RTTI from the language.

The shilos storage layer separately records a region-root type UUID. The source prototype uses one native CoDRegionRoot UUID for its allocated regions. That header does not identify each CoD object’s concrete type, supply dynamic method dispatch or expose a CoD reflection API. [Compiler types](../cod/lang_syntax.hh) · [Method selection](../cod/lang_frontend.hh) · [Native object/TLS layout](../cod/lang_llvm.hh) · [Root wrapper](../cod/lang_native_runtime.hh) · [Storage header](../include/shilos/region.hh)

H-375 settles the behavioral requirement: family views dispatch on actual argument types, including abstract parameters/results. A selected Node/Edge result must retain enough evidence for a subsequent Entity-typed call to choose the appropriate method. A one-method or one-concrete-result implementation guard cannot define the language. Direct selection is permitted when equivalent for the selected family/context.

The remaining H-251 boundary concerns concrete evidence/ABI, reflection and persistence, not whether dynamic dispatch is allowed. Type identity, selected world interpretation, value layout and actual owner remain separate. Per-object tags, lexical-view discriminators, descriptor catalogs and physical transport require a qualified mechanism design. [Ahead-of-Time Multiple Dispatch](AheadOfTimeMultipleDispatch.md) records WPO/AOT coverage and that design backlog.

## Julia comparison and the CoD boundary

Julia provides the useful reference point of a runtime containing types, generic functions, methods and module bindings. Its current documentation describes global method/binding state and task-local world-age visibility; it does not describe independent semantic worlds owned by each module. [Julia world age](https://docs.julialang.org/en/v1/manual/worldage/) · [Julia functions and method tables](https://docs.julialang.org/en/v1/devdocs/functions/)

H-240 makes CoD's distinction explicit: **each module has its own dedicated world copy**. H-234 supplies explicit export/integration and local merging by identity. Julia's global world counter, method-table implementation, task world age and public namespace syntax are not selected by this comparison. CoD's TLS pointer set is not Julia's task-local world-age view.

This module-world independence belongs to the [Global Variables Done Right vision](GlobalVariablesDoneRight.md#two-complementary-worlds). It prevents selected semantic information from becoming one ambient mutable world shared by every module. The TLS side supplies explicit typed dynamic associations across those module boundaries.

## Callable families and explicit installation

Bare `other` denotes a symbol object carrying identity and semantics; `@other` reads its current typed TLS binding. Local name collisions, shadowing and staging need one coherent lookup rule; an independent `@` namespace is not assumed to bypass ordinary shadowing. No general runtime Symbol ABI or implicit conversion in every value context is selected.

| Operation | Compilation/integration | Runtime |
| --- | --- | --- |
| Symbolic callable definition without effect | Define or extend the receiver’s family under existing permissions. | No implicit installation or capture. |
| `effect @sym(...) { ... }` | Define or extend and check the compatible family view. | Materialize/install that view when reached. |
| `effect @sym;` | Select the current composed family; add no method. | Materialize/install its compatible view. |
| `effect @sym = other;` | Resolve the source symbol’s selected family and destination contract. | Materialize that source view and install it under the destination. |
| `effect @sym = @other;` | Check the destination against the source TLS value’s contract. | Install the existing value, preserving its captures. |

The installed view preserves family/context and environment. A different destination identity does not redirect dispatch, merge identities or edit either world. A family with several methods can be a value. Export, using/import/mixin and extension remain module construction; none implicitly installs a receiver TLS value. Dependency initialization still follows its once-only incorporation contract and can execute its own explicit effects.

Skipped runtime paths do not install; repeated runtime execution does not repeat method registration. Existing scope/declaration rules constrain where combined forms may occur. Whole-program compilation prepares admissible actual-type combinations and dispatch. Independent module interpretations remain fixed for that composition and can coexist; a new explicit integration stage is separate from invocation.

Entry-module composition additionally follows H-379: extending a dependency callable requires an explicit entry installation, and that installation cancels dependency installations of the callable. Module definitions and explicitly retained source interpretations remain available. CoD must expose unused installation roots to LLVM elimination; the [cancellation contract](AheadOfTimeMultipleDispatch.md#entry-installation-and-unused-dependency-code) preserves initialization operand effects and all procedure-local installations. Once-only dependency initialization does not override this rule.

## Storage and execution stages

A referenced object still needs a live region supplied by a region symbol. Regional graphs, packets and application state reside in their regions. Symbolic bits payloads reside directly in TLS; lexical bits retain lexical storage. TLS is not a non-bits allocation arena, and its thread lifetime does not extend the lifetime of a referenced object. Scoped rebinding and captured replay do not transfer or destroy hidden region owners.

Compilation determines the flattened TLS shape: the admitted symbolic identities, their declared type constraints and the binding layout used by generated code. Runtime effects change the bits payloads or permitted references occupying that layout and restore preceding bindings as scopes end. The compiler's analysis of those effects is not execution of a source-program TLS-world edit at compile time.

Cross-module module-world editing is impossible at compile time and runtime (H-245). A module constructs only its own world. Reading another module's exports and merging their information locally is receiver construction, not an edit of the source world. TLS binding to a compatible value from another copy does not grant a way to rewrite that copy's semantic information. The runtime use of referenced values retains the established memory-access, capture and ownership contracts.

## Architecture implementation boundaries

H-245's implementation goal covers independent module worlds and flat runtime TLS, including H-242's vocabulary identities. The requirement audit covers construction, composition, isolation, compile-time TLS layout and native binding behavior together. Its evidence is organized by those requirements, including rejection paths and source inspection; successful examples alone cannot establish world isolation.

The architecture separates three operations:

1. **Receiver-owned module construction:** read explicitly exported source information and merge information for identical identities in the receiver's own world. Local aliases provide access; distinct identities remain distinct. The operation never edits a source world and does not automatically re-export imported information.
2. **Compile-time TLS layout:** determine the flattened typed-binding collection and its declared type constraints, then check demanded paths and captures against that model.
3. **Runtime TLS editing:** change a symbol's current bits payload or permitted reference, potentially selecting a compatible value from another module-world copy, and restore the preceding binding on scope exit. Binding changes do not edit module-world definitions or method sets.

Canonical identity-based composition must follow this ownership boundary. Compiler-internal construction and call machinery stays internal. No cross-module world-edit primitive, TLS editor-target protocol or runtime method-table update mechanism is required or permitted by this goal. Native lowering may specialize statically known semantic information without inventing runtime world objects solely to host an editor.

## Current implementation evidence and deferred work

The current typed TLS implementation is recorded in [symbolic-context evidence](../tests/cod/language/symbolic-context-validation.json); older pointer-cell reports retain their original source hashes.

| Source | Observed representation and limit |
| --- | --- |
| [Frontend](../cod/lang_frontend.hh), Module / ModuleWorld | Each module owns a ModuleWorld containing its selected symbols, types, methods, relations, publication information and lowered resource values. Source forms, dependency loading and execution-composition state remain in Module. Identity-bearing definitions can be shared; selection containers belong to the receiver. These system compiler containers are not a persistent/runtime world ABI. |
| Frontend, Environment / bind / read_symbol | Tracks current/captured bindings by symbol identity, checks declared types and binding availability, and retains resource provenance. This analysis determines the native bindings or their WPO specialization; it is not runtime execution of the source effects. |
| Frontend, define_procedure | Retains body/lexical references and captured binding values in compiler metadata. The host shared_ptr is not a user-visible callable or heap-closure protocol. |
| [LLVM lowering](../cod/lang_llvm.hh), slot / effect / restore | Native scalar/reference bindings use pointer-valued TLS entries indexed by symbol identity. A binding stores its payload in a separate lexical value cell; scope exit restores the preceding pointer. WPO checks type compatibility and binding availability. Region/procedure effects retain specialized metadata/native paths. |
| [Capture fixture](../tests/cod/language/capture-scope-restore.cod) | Records current/captured routing and nested restoration at its compiler revision; it does not establish the module-world architecture. |

Each emitted native binding site has one value cell in the entry block of the current inlined, non-recursive program. Repeated loop execution reuses that site's storage; the payload is written at binding time. Saved pointers restore visibility, while definition-time captures retain their separately checked values. This native storage choice does not create a third user allocation carrier or extend source-level borrow/owner validity.

[K-395 evidence](../tests/cod/language/world-conflict-validation.json) records H-253 overload-conflict rejection, direct-alias and using-preserved contexts, a 64-module chain, native pool reuse and cross-world TLS checks. [K-394](../tests/cod/language/world-context-validation.json) retains its prior warning/context checkpoint; H-253 supersedes the silent-choice expectation for conflicting sibling overloads. Full language and relocation scripts were not rerun. Earlier evidence retains its actual compiler and scope.

[K-391 evidence](../tests/cod/language/world-composition-validation.json) retains eleven direct examples plus six supplemental TLS/type/lifetime checks at its preceding compiler. Its IR inspection finds one pointer-valued TLS slot for a shared symbol. [K-389 evidence](../tests/cod/language/world-foundation-validation.json) retains the pointer-cell foundation checkpoint. These records are historical evidence at their actual source/executable hashes.

The [architecture audit](../tests/cod/language/world-architecture-validation.json) maps every goal requirement to current source and evidence. Its 15 supplemental checks cover private/non-re-exported resources, separate symbol/type identities, unavailable bindings, escape/extraction rejection, same-type conformance information merged through aliases, source isolation and a saved alias beside a local union. The compiler/source hashes match K-395's 27 checks, including conflict rejection through a 64-module chain and the cross-world TLS probe.

The WPO implementation preserves the semantic flat binding set while specializing statically known procedure and region associations. Procedure calls are inlined with their selected captures/current bindings; region values retain native pointers and checked owner provenance. Native data bindings use typed thread-local payloads. This does not supply a generic runtime closure, reflection registry or mutable runtime method table. Module owners survive initialization and hidden/captured use until program exit; ordinary scoped acquisitions retain their own cleanup points.

H-253 resolves conflicting imported dependencies through typed dispatch, including distinct helper interpretations sharing source code. Explicit conflict arbitration, extension of unrelated callable unions, cyclic initialization and broader owner/qualifier syntax retain their separate design scope. The audit proves this architecture implementation, not the full language or Actor lifecycle. No cross-module world editor is a pending deliverable.
