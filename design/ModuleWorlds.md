# Module worlds and flat TLS bindings

Design revision: 12 · Recorded: 2026-11-09 · Status: architecture implemented and audited in the WPO native prototype.

## Design authority

- **Registry / scope:** [D-15](DesignAuthorityRegistry.md#d-15), revision12, all sections; [K-396](DesignAuthorityRegistry.md#k-396) records the completed architecture audit on the K-395 compiler, including H-253 conflict rejection, H-252 warning controls and cross-world TLS. Earlier checkpoints retain their actual scopes. H-234/H-237–H-249 supplies independent module worlds, separate typed TLS, vocabulary identities, receiver-local composition and identity-based publication; H-245 supersedes cross-module editing. H-251 reserves RTTI for human discussion. Earlier decisions/evidence retain their scopes in the registry. Full contracts and candidate value paths remain pending-human; this revision does not qualify unresolved mechanisms.
- **Value lineage:** C-59/P-204/P-209 relates receiver-owned semantic construction to C-10 and charter §§1–4. P-205 preserves explicit publication and local identity merging. H-245/P-216 governs world isolation and the compile-layout/runtime-binding distinction; P-206 is superseded. P-210–P-212 distinguishes the entity sets and typed-pointer roles. H-241/P-213 places module worlds in D-12's vision; H-242/C-60/P-214 supplies common vocabulary identities. H-252/P-219 requires a diagnostic for multi-alias extension while retaining shared identity; H-253/P-220 requires conflicting inherited/selected overloads to reject during import adaptation. Full document value paths remain pending-human. P-207/P-208 are dependencies, not authority.
- **Human baseline:** H-96/H-97/H-104/H-111/H-184–H-189 supplies typed symbol identities, aliases, scoped bindings, procedure captures and call modes. H-180/H-181 preserves hidden/captured owners and forbids extraction from dynamic resources. H-234–H-242 supplies the two-world inventory and vocabulary idiom, subject to H-245's explicit correction of cross-module editing. H-245 also requests the replacement active goal. H-235 keeps invocation machinery internal; H-236 requires architecture before local fixes.
- **Invariants / non-goals:** A module constructs its own world; it cannot edit another world at either stage. Source selection reads published information and constructs receiver-local information. Symbol identity, type identity, a module-world copy and a referenced value are distinct. TLS is a flat typed-pointer collection, not another type/method catalog or permission to edit a source world. Preserve capture and regional lifetime/ownership contracts. No cross-module world editor, editor-target effect, live method-table mutation protocol, public pointer ABI or transaction scheme is introduced.
- **Acceptance / evidence:** [K-396's audit](../tests/cod/language/world-architecture-validation.json) establishes receiver-owned construction, source isolation, explicit publication, identity-local composition, flattened TLS/type checking, native rebinding/restoration, captures and owner preservation. It combines source inspection with 42 direct checks on one compiler revision. This implements H-245's architecture in the WPO native prototype; it does not qualify the full document or establish a persistent world ABI, general runtime callable representation or Actor lifecycle.

## The two sets of entities

A module world is a module's own semantic runtime environment, containing its available types, callable values, methods and related module information. It is more than a source-name or declaration index. Different modules have independent world copies and explicitly select what to publish and integrate.

A TLS world is the thread's **flattened set of typed symbolic pointers**. Each symbol has its own identity and a required type identity. At runtime its entry dynamically points to a value of a compatible type. The value may originate from any of the module-world copies; it is not copied into a second type/method catalog in TLS merely because the pointer refers to it.

These worlds do not model the same set of entities. The relevant connection is **typed reference to a value**, not a pair of matching world records representing that value.

## Module-world inventory

| Entity or information | What belongs to the module's world | What must stay distinct |
| --- | --- | --- |
| Type | Type identity and its world-local type information: abstract relationships, data representation and other admitted type semantics. | Type identity versus the address of one copy of its type information; a type versus a particular value of that type. |
| Function/procedure | Callable identity, definition/body and the callable information/value available in this world. | Callable definition, an overload's definition, a captured procedure value and an individual invocation. |
| Overloaded method | Signature, body, origin and membership in the callable's available method set. | A world-local method set versus a process-global shared mutable method table. |
| Explicit type relation | Selected abstract inheritance and data-interface declarations. | Declared conformance versus inference from similar fields or a value's storage address. |
| Symbol declaration | Strict symbol identity and declared type; the source aliases used to refer to that identity. | The declaration versus the pointer currently installed under that identity in a thread's TLS. |
| UoM/conversion | Unit identities and the selected pure conversion definitions. | A unit type versus a particular measured amount/index value. |
| Module globals and initialization | Permitted global declarations/values and the initialization semantics that establish module resources. | Module state versus an automatic thread-local copy of every global; exact sharing/copying details cannot be inferred from a host compiler container. |
| Names, selection permissions and exports | Source-local access to the world's definitions, extension permission and explicit publication. | Aliases versus identity; local availability versus onward export. |

Types, functions/procedures and their methods are the core entities in H-234. The additional rows locate existing language facilities; they do not add new categories of heap object or public syntax.

A procedure definition can establish a procedure value with captures when its definition executes. A compact form such as proc @p can also declare the symbol if needed. That source form therefore has definition, initialization and binding roles; their proximity in syntax does not make a callable definition a TLS entry.

Each receiver integrates exported information for identical identities in its own module world. The operation does not silently mutate the source worlds. Physical copying, sharing of immutable definitions and regional representation must preserve that independence; this inventory does not choose their implementation.

## Identity records, aliases and publication

Identity is the key of semantic information. A module can have several local aliases and several public aliases for that identity. Aliases provide access; they neither own separate copies of the method set nor redefine the object. Selecting more information for the same identity merges into the receiving world's record, so every alias of that identity sees the locally combined information (H-234/H-248).

Import selects a receiving implementation context for a source callable. That receiver records one imported identity per source callable identity; different aliases of that import share its information. Scanning may register local overloads before their import is encountered. When an explicitly imported alias identifies their callable, those declarations join that receiver record. A second imported alias does not create another interpretation context. Distinct source/extension identities and explicit same-alias unions retain their own identities. This is compiler construction metadata, not RTTI or a runtime identity layout.

If one module declares overloads through several local aliases of the same callable identity, the compiler warns with `[-Wmulti-alias-extension]` and identifies the aliases and declaration locations (H-252). All aliases still access the shared method information; the diagnostic makes that non-obvious consequence visible. Prefer one alias for declarations within a module. Merely selecting several aliases, or declaring several overloads through one alias, does not trigger this warning. `-Werror=multi-alias-extension` promotes this category to an error; `--warnings-as-errors` promotes all language warnings. The unit-conversion warning remains separately selectable.

An imported helper retains the source-to-receiver sequence of its selected implementation contexts. Each receiving interpretation considers inherited definitions and the source-related callable information selected in that world, by identity. Several aliases of one identity contribute its information once. A repeated view of a declaration supplies its selected interpretation only through the same identity or its explicit receiver import. Sibling identities remain distinct even when they share a source body, so nested helper interpretations cannot silently replace one another. A distinct overload definition remains a dispatch candidate. Ordinary typed dispatch must have a unique best candidate. In H-253's case, A's score(Sample)=7 and B's score(Sample)=11 conflict when B is selected while importing A's helper, so compilation rejects and identifies both definitions.

This candidate comparison does not merge the published identities or edit any source world. Direct calls through distinct aliases still select those identities, while `using` preserves the helper's existing interpretation. `import` adds the receiving interpretation without erasing inherited overloads. Explicit syntax for informed conflict arbitration is a future design possibility, not a current feature. Source globals, non-callable names, procedure captures and runtime data layout retain their separate contracts. No arbitrary choice by alias order, original ancestor or preferred source identity resolves a conflicting overload.


Export means **make an identity importable under a public alias** (H-247). Public aliases are a separate mapping from local lookup names. An ordinary `export name;` publishes the identity selected by that local name. `export import` publishes the identities selected by the import, including admitted associated methods. A competing local name does not retarget that publication. Two public aliases may expose the same identity; a single public alias cannot denote two conflicting identities. Existing selection-as followed by export supplies renamed publication without a new grammar form.

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

This role does not decide whether a particular method collection contains only signatures or also useful default bodies. Standalone method-head semantics and the detailed identity rules for newly added implementations still follow the architecture review; the vocabulary idiom does not silently approve those local mechanisms.

## TLS-world inventory

The semantic entry is described schematically as:

```text
symbol identity S
    declared type identity T
    current typed pointer -> value v, whose type identity is compatible with T
```

This is explanatory notation, not a public struct, boxed-value ABI or requirement for a runtime tag check. WPO may prove compatibility and resolve identities before execution. The contract still uses identities even when their checks have been discharged statically.

| TLS concern | Role |
| --- | --- |
| Symbol identity S | Selects the symbolic pointer entry. Different module-local aliases of S refer to the same entry within the thread. Different symbol identities remain different entries even when their spellings match. |
| Declared type identity T | Constrains which values the entry may reference. T is not the symbol identity and is not replaced by a module-local type name or type-record address. |
| Current typed pointer | Dynamically associates S with a value. Rebinding can select a different value originating in another module-world copy when its type identity is compatible. |
| Dynamic scope behavior | Effects shadow the current association and restore the preceding association on scope exit, following the binding/capture contracts. The flattened active collection does not require a dictionary or a separate TLS namespace per module. |

The referenced value can be bits, a procedure value or a permitted regional/resource reference. The final admissible dynamic-value categories retain their separate design status. The TLS entry is a typed pointer to that value; the value's payload, procedure captures or region contents do not thereby become extra TLS-world declarations.

The flat collection describes active entries. Saved values/pointers needed for scope restoration and procedure captures are supporting execution semantics, not a second catalog of module definitions. This does not prescribe a heap-allocated scope frame or runtime closure.

The pointer carrier alone does not redefine capture by value, bits transfer or resource ownership. Capturing a bits value must still preserve the required value; retaining a pointer to mutable storage is not automatically equivalent. Capturing a regional reference preserves its existing referent/lifetime relationship, without deep-copying the region. Representation must satisfy these existing semantics rather than replacing them.

## Values from different module-world copies

```text
Module world A                      Module world B
  type information with ID T         type information with ID T
  value a : T                        value b : T
            \                         /
             \                       /
TLS for one thread:  S :: T  -> a
                     S :: T  -> b     after a scoped rebinding
```

The two values need not have the same value identity or address. Their originating module-world copies remain distinct. A is not merged into B, and neither world is imported into TLS by the rebinding. The symbol's type requirement is satisfied through type identity compatibility.

Conversely, a same-spelled type with a different identity is not interchangeable merely because its fields or representation look similar. The precise compatibility admitted for abstract/sum/conversion cases follows the type contracts; this inventory does not invent implicit conversions or erase nominal distinctions.

Type compatibility answers whether a value can be associated with S. It does not by itself choose a method set, redefine a callable's source context, grant ownership or extend the value's lifetime. Those are distinct obligations. In particular, origin in a different module world is not a reason to mint a new type identity or create a second per-module TLS entry.

The [cross-world TLS probe in K-395](../tests/cod/language/world-conflict-validation.json) establishes shared typed data bindings across a vocabulary, two source modules and a receiver. Captured/current reads, loop shadowing/restoration and nominal-type/missing-binding rejection are exercised. Its emitted IR has two pointer-valued TLS entries for two shared data symbols, including their aliases. Captured regional references preserve the referent, so authorized changes to that referent remain observable; capture does not deep-copy the region. Region/procedure specialization remains separate from the two native data slots.

## RTTI: current implementation and decision boundary

The current CoD source-language prototype has compile-time type identities and concrete-type method selection, but no general runtime type-information facility. Compiler Type/Value metadata retains identity and provenance; demanded calls are checked and lowered with the known concrete types. User data layouts contain their declared fields, without an automatically inserted type tag, vtable or method-world pointer. Native scalar/reference TLS slots carry a pointer to the value, without a runtime type descriptor. This is a statement of the current bounded implementation, not a decision to omit RTTI from the language.

The shilos storage layer separately records a region-root type UUID. The source prototype uses one native CoDRegionRoot UUID for its allocated regions. That header does not identify each CoD object’s concrete type, supply dynamic method dispatch or expose a CoD reflection API. [Compiler types](../cod/lang_syntax.hh) · [Method selection](../cod/lang_frontend.hh) · [Native object/TLS layout](../cod/lang_llvm.hh) · [Root wrapper](../cod/lang_native_runtime.hh) · [Storage header](../include/shilos/region.hh)

H-251 requires discussion and a human decision before RTTI design or implementation. Questions to bring to that discussion include which business operations need runtime type identity, whether method selection can require a runtime type, and which information survives region transport or persistence. A stable type identity, a particular module world's methods/conformance and a value's physical layout are separate concerns. No type-header format, dynamic dispatch mechanism, descriptor registry, reflection API or RTTI prohibition is selected here. Compiler identity-centered storage and flat TLS do not themselves authorize any such choice.

## Julia comparison and the CoD boundary

Julia provides the useful reference point of a runtime containing types, generic functions, methods and module bindings. Its current documentation describes global method/binding state and task-local world-age visibility; it does not describe independent semantic worlds owned by each module. [Julia world age](https://docs.julialang.org/en/v1/manual/worldage/) · [Julia functions and method tables](https://docs.julialang.org/en/v1/devdocs/functions/)

H-240 makes CoD's distinction explicit: **each module has its own dedicated world copy**. H-234 supplies explicit export/integration and local merging by identity. Julia's global world counter, method-table implementation, task world age and public namespace syntax are not selected by this comparison. CoD's TLS pointer set is not Julia's task-local world-age view.

This module-world independence belongs to the [Global Variables Done Right vision](GlobalVariablesDoneRight.md#two-complementary-worlds). It prevents selected semantic information from becoming one ambient mutable world shared by every module. The TLS side supplies explicit typed dynamic associations across those module boundaries.

## Storage and execution stages

A referenced value still needs valid storage. Regional graphs, packets and application state reside in their regions; lexical bits storage follows its lexical lifetime. TLS does not become a third user allocation carrier. A module-world value referenced through TLS must remain valid for its permitted use, just as any other bound regional value must. Scoped rebinding and captured replay do not transfer or destroy hidden region owners.

Compilation determines the flattened TLS shape: the admitted symbolic identities, their declared type constraints and the binding layout used by generated code. Runtime effects change the typed pointers occupying that layout and restore preceding bindings as scopes end. The compiler's analysis of those effects is not execution of a source-program TLS-world edit at compile time.

Cross-module module-world editing is impossible at compile time and runtime (H-245). A module constructs only its own world. Reading another module's exports and merging their information locally is receiver construction, not an edit of the source world. TLS binding to a compatible value from another copy does not grant a way to rewrite that copy's semantic information. The runtime use of referenced values retains the established memory-access, capture and ownership contracts.

## Architecture implementation boundaries

H-245's implementation goal covers independent module worlds and flat runtime TLS, including H-242's vocabulary identities. The requirement audit covers construction, composition, isolation, compile-time TLS layout and native binding behavior together. Its evidence is organized by those requirements, including rejection paths and source inspection; successful examples alone cannot establish world isolation.

The architecture separates three operations:

1. **Receiver-owned module construction:** read explicitly exported source information and merge information for identical identities in the receiver's own world. Local aliases provide access; distinct identities remain distinct. The operation never edits a source world and does not automatically re-export imported information.
2. **Compile-time TLS layout:** determine the flattened symbolic-pointer collection and its declared type constraints, then check demanded paths and captures against that model.
3. **Runtime TLS editing:** change a symbol's current typed pointer, potentially selecting a compatible value from another module-world copy, and restore the preceding binding on scope exit. Binding changes do not edit module-world definitions or method sets.

Canonical identity-based composition must follow this ownership boundary. Compiler-internal construction and call machinery stays internal. No cross-module world-edit primitive, TLS editor-target protocol or runtime method-table update mechanism is required or permitted by this goal. Native lowering may specialize statically known semantic information without inventing runtime world objects solely to host an editor.

## Current implementation evidence and deferred work

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

The WPO implementation preserves the semantic flat binding set while specializing statically known procedure and region associations. Procedure calls are inlined with their selected captures/current bindings; region values retain native pointers and checked owner provenance. Native data bindings use real thread-local pointer cells. This does not supply a generic runtime closure, reflection registry or mutable runtime method table. Module owners survive initialization and hidden/captured use until program exit; ordinary scoped acquisitions retain their own cleanup points.

H-253 resolves conflicting imported dependencies through typed dispatch, including distinct helper interpretations sharing source code. Explicit conflict arbitration, extension of unrelated callable unions, standalone method heads, cyclic initialization and broader owner/qualifier syntax retain their separate design scope. The audit proves this architecture implementation, not the full language or Actor lifecycle. No cross-module world editor is a pending deliverable.
