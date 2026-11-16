# Module worlds and flat TLS bindings

Design revision: 28 · Recorded: 2026-11-16 · Status: module-world contract; implementation evidence refers to the preserved source-compiler baseline.

## Design authority

- **Implicit source lookup:** [D-15-TYPE-METHODS](DesignAuthorityRegistry.md#d-15-type-methods)/[D-15-LOOKUP](DesignAuthorityRegistry.md#d-15-lookup), r28's type-selection and fallback sections, records H-483's shadowing and [H-484](../CHANGELOG.md#h-484)'s source-module fallback direction. [H-485](../CHANGELOG.md#h-485)/[D-15-LOOKUP-INFO](DesignAuthorityRegistry.md#d-15-lookup-info) reserves informational diagnostics at implicit-use sites for later implementation. Candidate C-35/C-45/C-57/C-59 via P-425–P-433 serves charter §§3.1/3.2/4.2/6.3/8/9 through reusable public definitions, explicit identity/context and inspectable source provenance. Preserve privacy, source-world isolation, explicit extension/publication, name-before-dispatch, captures and unique interpretations. Acceptance and remaining engineering boundaries are below. Human confirmation covers the stated directions, not full value-path/operational qualification, a general distance algorithm or current compiler support. [K-SOURCE-FALLBACK-20261116](../CHANGELOG.md#k-source-fallback-20261116) records the r26→27 integration; existing identity/helper contracts continue. [H-487](../CHANGELOG.md#h-487)/[D-15-LOOKUP-TRANSITIVE](DesignAuthorityRegistry.md#d-15-lookup-transitive) additionally confirms the intermediate private-binding barrier in the multilevel model requested by H-486; P-434–P-436 records that scope. No runtime module object or qualified-access syntax is introduced.

- **Explicit callable bundles and re-export:** [H-488](../CHANGELOG.md#h-488) permits an upstream public re-export alongside a private local namesake and directs origin-based information suggestions. H-489/H-490/[D-15-METHODS-CLAUSE](DesignAuthorityRegistry.md#d-15-methods-clause) selects optional with methods and whole-callable selection. H-491/[D-15-REEXPORT](DesignAuthorityRegistry.md#d-15-reexport) selects publication-only export from, including public renaming. P-437–P-442 extends the candidate C-35/C-45/C-57/C-59 path to charter §§3.1/3.2/4.2/8 through explicit identities, selectable behavior and independent local/public intent. Preserve source contexts, public-source access, collision rules, owners/captures and D-16. Acceptance and implementation boundaries are below; [K-REEXPORT-METHODS-20261116](../CHANGELOG.md#k-reexport-methods-20261116) integrates r27→28. Direction/surface confirmation does not qualify a general module graph, physical representation or current compiler support.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-15r23→24 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Registry / current scope:** [D-15](DesignAuthorityRegistry.md#d-15); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole operational contract pending-human.** C-59/C-60/P-204–P-220; H-234/H-237–H-249 specifies receiver-owned identity construction, explicit publication and typed TLS. H-387/K-573 covers the fused artifact; K-593 aligns its input boundary.
- **Value path:** C-59/P-204/P-209 relates receiver-owned semantic construction to C-10 and charter §§1–4. P-205 preserves explicit publication and local identity merging. H-245/P-216 governs world isolation and the compile-layout/runtime-binding distinction; P-206 is superseded. P-210–P-212 distinguishes the entity sets and typed-pointer roles. H-241/P-213 places module worlds in D-12's vision; H-242/C-60/P-214 supplies common vocabulary identities. H-252/P-219 requires a diagnostic for multi-alias extension while retaining shared identity; H-253/P-220 requires conflicting inherited/selected overloads to reject during import adaptation. Full document value paths remain pending-human. P-207/P-208 are dependencies, not authority.
- **Contract boundary:** No foreign-world editing; H-253 rejects conflicting interpretations. Persistent identity/layout, general composition/namespace and physical RTTI remain unselected. H-387 preserves source-world isolation/owners; H-400–H-406/H-407/D-26 replaces the known-ADT-only input restriction without authorizing new executable methods.
- **Invariants / non-goals:** A module constructs its own world; it cannot edit another world at either stage. Source selection reads published information and constructs receiver-local information. Symbol identity, type identity, a module-world copy and a referenced value are distinct. TLS is a flat typed-binding collection, not another type/method catalog or permission to edit a source world. Preserve capture and regional lifetime/ownership contracts. No cross-module world editor, editor-target effect, live method-table mutation protocol, public pointer ABI or transaction scheme is introduced.
- **Acceptance:** Validate receiver-owned identity, explicit publication, source-world isolation and typed TLS without foreign-world edits. General composition and persistent representation remain separate review scopes. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## The two sets of entities

A module world is a module's own semantic runtime environment, containing registered symbol objects, types, callable contracts, methods and related semantic information. Runtime values and module initialization state have their own construction events. It is more than a source-name or declaration index. Different modules have independent world copies and explicitly select what to publish and integrate.

A TLS world is the thread's **flattened set of typed symbolic bindings**. Each symbol has its own identity and required type identity. Independent bits payloads are stored directly in TLS; H-482 separately admits explicit ref/ptr symbols borrowing scalar targets from region-symbol-backed storage. Non-bits object bindings reference values in regions supplied by region symbols, with actual region lifetime checks; scoped rebinding may select a different region. Values may originate from different module-world copies without importing those worlds into TLS. Region-resource and procedure-valued bindings retain their distinct ownership/capture contracts.

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

Import selects a receiving implementation context for a source callable. That receiver records one imported identity per source callable identity; different aliases of that import share its information. Scanning may register local overloads before their import is encountered. When an explicitly imported callable alias identifies their callable, those declarations join that receiver record. A family made available only implicitly by type selection has the lower-priority lookup behavior below; a same-named local definition does not automatically join that family. A second imported alias does not create another interpretation context. Distinct source/extension identities and explicit same-alias unions retain their own identities. This is compiler construction metadata, not RTTI or a runtime identity layout.

If one module declares overloads through several local aliases of the same callable identity, the compiler warns with `[-Wmulti-alias-extension]` and identifies the aliases and declaration locations (H-252). All aliases still access the shared method information; the diagnostic makes that non-obvious consequence visible. Prefer one alias for declarations within a module. Merely selecting several aliases, or declaring several overloads through one alias, does not trigger this warning. `-Werror=multi-alias-extension` promotes this category to an error; `--warnings-as-errors` promotes all language warnings. The unit-conversion warning remains separately selectable.

An imported helper retains the source-to-receiver sequence of its selected implementation contexts. Each receiving interpretation considers inherited definitions and the source-related callable information selected in that world, by identity. Several aliases of one identity contribute its information once. A repeated view of a declaration supplies its selected interpretation only through the same identity or its explicit receiver import. Sibling identities remain distinct even when they share a source body, so nested helper interpretations cannot silently replace one another. A distinct overload definition remains a dispatch candidate. Ordinary typed dispatch must have a unique best candidate. In H-253's case, A's score(Sample)=7 and B's score(Sample)=11 conflict when B is selected while importing A's helper, so compilation rejects and identifies both definitions.

This candidate comparison does not merge the published identities or edit any source world. Direct calls through distinct aliases still select those identities, while `using` preserves the helper's existing interpretation. `import` adds the receiving interpretation without erasing inherited overloads. Explicit syntax for informed conflict arbitration is a future design possibility, not a current feature. Source globals, non-callable names, procedure captures and runtime data layout retain their separate contracts. No arbitrary choice by alias order, original ancestor or preferred source identity resolves a conflicting overload.

Export means **make an identity importable under a public alias** (H-247). For a symbol-identified callable it publishes the entire method set available in that module (H-374); no per-method private subset is hidden inside the published family. A bodyless contract can genuinely have no methods. View compatibility constrains usable calls, not publication privacy. Public aliases are a separate mapping from local lookup names. An ordinary `export name;` publishes the identity selected by that local name. `export from "source.cod" name [as public_name];` instead publishes the explicitly selected upstream object without creating an own local alias or extension target; a private local namesake remains private. `export import` publishes the identities selected by the import, including admitted associated callables. A competing local name does not retarget that publication. Two public aliases may expose the same identity; a single public alias cannot denote two conflicting identities. Selection-as followed by export can publish that local alias; export from also permits an independently chosen public alias.

The compiler represents this distinction directly:

| Information | Authoritative record and access |
| --- | --- |
| Callable information | Receiver-owned identity records contain methods and origins. Explicit aliases and resolved source-fallback lookups identify those records; fallback registration need not copy every source name. A virtual union refers to its member identities in the receiver. |
| Type information | Definitions have compiler identities; local type names resolve to identity-bearing types. Selected conformance belongs to the receiving world. |
| Symbols | Local aliases resolve to symbol IDs; declared types belong to those IDs. Pending source selectors are scanning state, not runtime bindings. |
| Globals | Values are indexed by declaration identity; local aliases point to them. |
| Publication | Public alias maps to a typed identity: symbol, type, callable or global. Consumers resolve that identity directly instead of looking up the exporter’s local spelling again. |

Source selection receives a read-only source module. It reads the source's published identity and information, merges permitted information into the receiver, binds a local alias and optionally publishes that identity. Mutable receiver construction and runtime TLS changes remain separate. Shared immutable definitions are compatible with independent worlds; shared mutable selection tables are not.

These operations give implementation helpers their purpose: declare an identity, select source information, merge a local record, bind an alias, combine a local union, or publish an identity. H-246/H-249 requires deriving such operations from the architectural contract and removing obsolete bookkeeping, rather than preserving helpers merely because earlier code used them. This is an engineering rule within confirmed semantics; it does not settle unrelated dispatch, ownership or syntax choices.

## Type selection and shadowable associated methods

Using/import of a type makes its public source-visible behavior available without selecting every operation separately. H-484 supplies the lookup model: register the selected source module as a fallback below the receiving module's own namespace, retaining its publication information and its own source-fallback relationships. If ordinary lexical/local lookup finds no binding, lookup may consult that source view. Symbol from establishes the same fallback for its source. This is a compile-time relationship between lookup environments, not a module-valued expression, runtime dictionary or global method registration.

Fallback resolves each visited module's own binding before following that module's fallback relationships. Downstream lookup first consults that module's explicit public alias at the spelling; without one, an unexported own binding is a shadowing barrier, not an absent name. Public aliases include explicit re-exports. It can therefore find public names unrelated to the initially selected type; it is not a recursive traversal of that type's fields or method signatures. A resolved callable name denotes its published callable identity and available family, rather than a permanently type-filtered name binding. H-225's signature association identifies related callables where required, including H-233's associated publication; it does not limit which public source names fallback can find. Receiver-local information still merges by identity under H-234/H-248. Do not expose private names or search arbitrary loaded modules.

A direct local definition at the same spelling shadows the implicitly brought name after either using or import of a type. It creates/selects the local definition's own identity according to ordinary declaration rules; it neither collides merely with that implicit name nor automatically extends its family. Name lookup selects the binding before argument dispatch. The hidden family is not a fallback when the local binding lacks a matching overload. Explicit callable aliases also take priority; independent explicitly declared names retain their ordinary collision rules.

```cod
# model.cod exports Node and score; score(Node in O) returns 1.
import "model.cod" Node;              # score is available implicitly.
func score(x:Node in O) -> u64 where {owner O} {
    return 7u64;                     # Local family shadows implicit score.
}
proc use(x:Node in O) -> u64 where {owner O} {
    return score(x);                  # 7; same rule after using Node.
}
```

To extend a source family, explicitly select that callable with import under the intended local alias, then use the established extension rules. Importing a type still grants its existing type-level permissions; this name rule does not remove permitted explicit conformance declarations. Explicit callable import, using's preserved implementation context, import's receiving interpretation, mixin body redefinition and explicit same-alias unions remain separate operations. Local shadowing alone is not explicit overload-conflict arbitration inside a composed family.

### Types carried by symbol selection

Selecting a symbol with `symbol from` retains its declared type information and registers its source for fallback under the same publication and shadowing rules. The explicitly selected symbol retains its declared nominal type identity, regardless of local aliases or later shadows. Public type/method names found through that source remain implicit; directly selecting a type remains an explicit selection.

```cod
# vocabulary.cod exports ticket, its declared type Ticket and Ticket's id method.
symbol from "vocabulary.cod" @ticket;
# Ticket and id are implicitly available; no TLS value was installed.
data Ticket { code:u64; };
# This local Ticket shadows the implicit type name.
# The declared type of @ticket is still vocabulary.cod's original type identity.
```

This does not redefine the selected symbol, retag values, install an effect, construct a callable view or capture dynamic state. Private dependencies can remain required semantic information without becoming selectable local names. Only published resources gain imported names through this rule. Imported symbol aliases retain identity and share their existing TLS slot; name shadowing cannot replace its declared contract. H-481's definition/view materialization and H-482's reference-symbol owner rules remain unchanged.

### Source fallback, proximity and publication

The initial policy uses a conservative priority relation:

- Ordinary lexical/local bindings and explicit selections take priority over fallback. Finding an unsuitable binding is still finding a binding: no deeper search is triggered by a wrong kind, missing overload or failed type check.
- Source views registered by peer explicit selections have equal priority. In a simple chain, each additional fallback edge is farther from the receiving selection; count lookup relationships, not the original definition's provenance or type/field depth. Distinct candidates at that level are ambiguous when the name is demanded; source order, module spelling and argument types do not break the tie. A local or explicit binding can shadow an unused fallback conflict.
- Prefer a candidate when its proximity to the explicit selection is clearly established. If paths cannot be ordered without inventing a semantic-distance rule, reject the demanded lookup and request explicit using/import. No arbitrary shortest-path, field-depth or inheritance-distance metric is selected. More precise proven proximity may be supported later.
- Repeated paths can collapse only when they select equivalent identities and interpretations. Shared origin alone does not erase a distinction between source-preserving using and receiver-adapted import. If those contexts compete without an established priority, require explicit selection.

Lookup edges retain their selection context and publication/shadowing evidence. Using preserves source interpretation; import retains its established receiving interpretation; symbol from preserves the selected identity/source context. Discoverability does not grant extension permission for every name in the source: explicitly importing the callable, by name or through with methods, selects the family to extend. A local shadow does not substitute itself into a source helper merely by spelling.

A source's own fallback relationships can be followed recursively when it has no own binding for the demanded spelling. Only a public terminal binding may be used downstream. H-487 makes an intermediate unexported binding a barrier when no public alias at that spelling is provided. H-488 permits an explicit re-export to supply that public alias with a different upstream identity, leaving the local binding private; this is an explicit publication decision, not a skipped private shadow. Following a dependency path does not put that dependency's aliases into the intermediate module's explicit export map. Thus implicit lookup can reach core's public helper through facade, while `using "facade.cod" helper` still requires facade to explicitly export helper. Source worlds remain read-only.

Ordinary export publishes its resolved identity; export import retains its existing explicit selected/associated publication contract rather than becoming a wildcard export of every reachable namespace. A cached implicit resolution or receiver identity record is not an own declaration/alias and cannot create a new private shadowing barrier.

```cod
# vocabulary.cod exports packet, Packet, Detail and detail_code.
symbol from "vocabulary.cod" @packet;
# Packet, Detail and detail_code can be found in the source's public view,
# without traversing Packet's fields. No TLS value is installed.

# a.cod and b.cod each export their selected type and a distinct render.
using "a.cod" Node;
using "b.cod" Packet;
# A demanded render(...) is ambiguous: these source views are peers.
using "a.cod" render as render;
# render now resolves explicitly; dispatch then uses that selected family.
```

### Identity, visibility and acceptance

Repeated selection paths preserve identities and do not duplicate implementations. Different implicit families sharing a spelling do not become one family merely by name. Export publishes selected identities according to the existing explicit-publication rules, not whichever later spelling happens to hide them. No module-qualified access syntax or automatic re-export is added.

Source-preserved helper bodies and already selected callable values retain their established definitions/contexts and captures. H-253 still rejects distinct equally applicable definitions admitted to an import-adapted helper; lookup shadowing cannot silently arbitrate that conflict. A new local family shadowing an implicit name has its own identity; spelling alone cannot make it replace a helper's source dependency. Existing explicit family adaptation remains valid. Connecting an imported family through an explicit callable import permits its ordinary extension rules; competing applicable implementations still obey H-253. Selection and declaration registration are compile-time operations; this rule does not hoist runtime initialization or define a live name-replacement event.

Acceptance must cover using/import type selection, result-only/inherited associations, public-source filtering, unrelated public-name fallback, local shadowing after both reuse forms, no typed fallback to a hidden family, explicit callable extension, repeated-path identity, demanded peer-source/context ambiguity, multilevel public-name lookup, private shadow barriers, explicit publication, symbol-carried type/method names and unchanged symbol type identity after local type shadowing. Preserve source-world independence, selected view/capture semantics, runtime initialization and fraction/UoM compatibility. These are design acceptance obligations, not a claim that current source composition supports the complete surface.

### Follow-up decisions for type-carried names

Identity merging and helper interpretation remain independent of namespace traversal. Resolving a public callable through fallback accesses its published family in the selected context; additional selections of the same receiver identity merge information for all its aliases. A local shadow creates/selects its own identity. Explicitly importing the source callable provides an extension target, subject to H-232 adaptation and H-253 conflicts.

```cod
# base.cod exports Scorable, Node, score and twice.
# Node conforms to Scorable. The base score is deliberately broader.
func score(x:Scorable in O) -> u64 where {owner O} { return 1u64; }
func twice(x:Node in O) -> u64 where {owner O} {
    return score(x) + score(x);
}

# receiver.cod
import "base.cod" Node;
import "base.cod" twice;
func score(x:Node in O) -> u64 where {owner O} { return 7u64; }
# For a Node n: local score(n) is 7; imported twice(n) is 2.
# The local family has no source-identity connection merely from its spelling.
```

In a separate receiver, explicitly importing base's score under the local declaration alias makes score(Node)=7 an extension of that imported family. It is more specific than score(Scorable)=1, so the established import-adapted twice uses it and returns 14. Equal-specificity conflicts still reject. These are consequences of existing identity/context contracts, not new source implementation evidence.

Source fallback avoids a recursive dependent-type-name closure algorithm. More elaborate proximity proofs, source-graph representation/caching and complete source implementation remain engineering work. They may not resolve an unclear ordering arbitrarily or weaken publication, interpretation, initialization or ownership checks. Existing signature-association information remains relevant to its separately specified uses, including associated publication.

### Multilevel fallback and private shadow barriers

H-486 requests multilevel source fallback; H-487/H-488 establishes its public/private boundary. Inside a module, ordinary local lookup retains its own binding. For a downstream implicit lookup at each visited module:

1. If the module explicitly publishes the requested alias, use that public identity and context, even if its private local binding at the same spelling is different.
2. Otherwise, an own binding is private and blocks deeper fallback. Diagnose the module/declaration and require an explicit same-named public choice there: publish the local object, or explicitly re-export an upstream object while retaining the local private binding.
3. Only when neither an own binding nor a public alias settles the name may lookup continue through that module's source fallbacks.

The check uses published identity directly, without resolving the module's private local spelling again. Source-body lookup still uses its private bindings normally.

```cod
# core.cod
func answer() -> u64 { return 1u64; }
export answer;
# Core is also exported.

# facade.cod
using "core.cod" Core;
func answer() -> u64 { return 2u64; }  # Private; shadows core's answer.
# Facade is exported, but answer is not.

# app.cod
using "facade.cod" Facade;
val n:u64 <- answer();
# Error: facade's private answer blocks implicit lookup.
# facade may add export answer; to publish its local answer (result 2),
# or export from "core.cod" answer; to publish core's answer (result 1).
# Without an explicit public choice, this call must fail.
```

If facade has no own answer binding, app can find core's public answer and the result is 1. If facade explicitly selects `using "core.cod" answer;` without exporting it, that private alias is also an own binding and blocks downstream implicit lookup unless an explicit public alias resolves the name, even though it identifies the same underlying definition. The rule concerns the selected name's publication, not merely the original object's export status. A local declaration of another kind also blocks the name; wrong kind or an inapplicable overload cannot restart fallback. Explicit names in app retain their higher priority and can choose a public resource directly.

The following consequences preserve deterministic lookup without a richer arbitration mechanism:

- **Nearest permitted lookup tier:** for ordinary acyclic chains, an own binding is nearer than any fallback beneath it; a direct source binding is nearer than one reached through an extra source edge. An explicit re-export belongs to the exporting module's own public bindings, regardless of the definition's older origin. Distinct peer bindings, including conflicting interpretation paths, require explicit selection. General contextual orderings that cannot be proved remain rejected.
- **Private barriers participate in lookup:** do not discard an inaccessible own binding as though the name were absent. Without an explicit public alias at that spelling, a winning private barrier is an error; a private barrier competing at the same priority cannot be silently removed in favor of a public peer. A strictly higher-priority explicit/public binding may already settle lookup without traversing the blocked lower path.
- **Diamond paths:** repeated paths to the same selected identity and equivalent interpretation do not duplicate a candidate or initialize a module again. Same definition with observably different using/import contexts is not equivalent merely because the source body is shared. Preserve the selected context sequence; unresolved equivalence or competing contexts reject.
- **Definitions keep their context:** app-level shadowing controls app's mention. It does not re-resolve a helper body's lexical names at app or turn an independent family into an imported helper dependency. Existing explicit adaptation and H-253 conflict checks continue.
- **Checking order cannot change names:** implicit lookup results may be cached, and selected information may merge by identity, but neither operation declares a private alias. A source body mentioning a fallback name does not newly block downstream access to it. Visibility comes from declarations/publication and the lookup graph, not which body was checked first.
- **Graph traversal does not activate modules:** retain existing once-only initialization and owner/capture rules. The current acyclic-module boundary remains; termination/caching of name lookup does not authorize cyclic module construction or initialization. All private barriers and ambiguous winners remain compile-time diagnostics.

This permits an intermediate dependency change or a new nearer binding to change a downstream implicit resolution. That is a consequence of implicit lookup; H-485's future use-site information can expose the path, and explicit source selection can state the intended dependency. No stability against arbitrary dependency edits is promised. Acceptance includes all four facade cases (absent, private without publication, local public, upstream public alongside local private), private aliases of the same original identity, peer barriers, diamond contexts, declaration-kind shadows, check-order independence and explicit-export/implicit-reachability distinction. Full graph representation, general distance proofs and source implementation remain separate; these rules add no runtime pointer or method-table protocol.

### Publication-only re-export

H-491 selects:

```cod
# facade.cod; core.cod publicly exports answer.
func answer() -> u64 { return 2u64; }  # Private local object.
export from "core.cod" answer;
# Local answer() still returns 2.
# Downstream answer resolves to core's exported object.

export from "core.cod" answer as original_answer;
# A second public alias for that same upstream object.
```

Export from selects the source's explicitly published object and preserves its identity and selected interpretation. It adds a public alias, not an own local alias, extension target, method definition, callable capture or TLS installation. The source remains a normal construction dependency; source fallback can retain that upstream relationship without promoting the selected public alias into a local declaration. Existing once-only initialization rules apply. Local lookup still follows local bindings before fallback; it does not consult the publication map as a replacement for those bindings.

The forms express different operations: export name publishes an existing local resolution; export from publishes an explicit source object; export import performs its established receiving-context import and publication. Export from does not silently perform import adaptation. A pure re-export is not a new callable definition. Public aliases of distinct objects cannot collide merely because one was declared later; repeated identical identity/context publication is harmless. Private upstream names cannot be selected. Publishing a type does not itself publish every name in its source namespace; associated publication through export import remains its separately specified contract.

Acceptance distinguishes local answer=2 from downstream answer=1, verifies both public aliases and unchanged private status, rejects conflicting public identities/private source access, and preserves source interpretation and initialization across multiple re-export levels. This surface is planned source behavior; current compiler support is not claimed.

### Explicit associated callables with with methods

H-489/H-490 makes with methods an optional suffix on using/import/symbol-from selections of a type or a symbol with an associated value type:

```cod
using "model.cod" Node with methods;
import "model.cod" Node with methods;
symbol from "vocabulary.cod" @node with methods;
```

These are separate selection examples. The clause explicitly brings in every associated **callable**, including its complete public method family. Signature association under H-225 identifies related callables; it never creates a callable containing only the matching methods. H-490 makes the selection unit general: using/import selects callables, not individual methods, whether the callable is named directly or selected through this clause. There is no source signature-selector or alias-specific method subset.

Resolve association within the admitted source view, respecting public identities, shadows, fallback priority and context. Do not scan callable bodies, recursively import every type in newly selected signatures, bypass a private barrier or filter overload candidates to disguise an ambiguous name. Repeated family identities deduplicate information. Public aliases for the associated callables retain their source spellings; renaming the initially selected type/symbol renames only that selection, not all its method names. Ordinary explicit aliases can be written separately when a bulk selection would conflict. No exclusion-list syntax is selected.

```cod
# model.cod exports Node and score, with both definitions:
func score(x:Node in O) -> u64 where {owner O} { return 1u64; }
func score(x:u64) -> u64 { return 2u64; }

# consumer.cod
using "model.cod" Node with methods;
val n:u64 <- score(7u64);  # 2: the whole score callable was selected.
```

The explicit binding has the same lookup/permission status as selecting that callable by name. Using keeps its source context and grants no extension permission. Import supplies its established receiving context and extension permission; a local declaration through that explicitly selected alias participates in that family instead of creating H-483's independent implicit-name shadow. Symbol from retains identity/source-preserving behavior and supplies no callable extension grant. Initializing/capturing a symbolic value remains a separate operation.

Explicit names obey existing collision and intentional composition rules. The clause does not silently union unrelated same-spelled families or make an explicit conflict harmless because a name is unused. Without the clause, local definitions can shadow implicit associated names. With it, the same spelling is explicitly selected, so ordinary explicit-selection/declaration rules apply. Repeated equivalent selections remain valid; declarations through multiple aliases still follow H-252's diagnostic. If a bulk selection conflicts, omit the clause and select the desired callables/aliases individually.

A mention resolved through one of these explicit bindings no longer qualifies for H-485's implicit-use information; other names reached through fallback still do. With methods does not automatically re-export its local bindings: private explicit aliases can form H-487 barriers downstream. An explicit export decision remains necessary, subject to export import's existing selected-associated-publication contract. It does not grant unrelated type conformance, UoM conversion registration, intrinsic dispatch permission or new fraction semantics.

Acceptance covers complete-family selection, result-only/inherited associations, using/import/symbol-from modes, explicit priority, extension versus independent shadow, identical-path deduplication, private barriers, export separation, collisions and diagnostic provenance. Full source grammar/lowering and general graph/proof coverage remain implementation work; no executable support is claimed.

### Deferred information diagnostics for implicit lookup

H-485 reserves a later compiler/tooling capability: emit an **information-level diagnostic at a source mention whose successful resolution depends on implicit fallback**, recommending an explicit using/import selection. This is planned capability, not an implemented diagnostic or an immediate prerequisite for supporting fallback.

The diagnostic should identify the use site, resolved public name, actual defining module and selection path sufficiently to explain the dependency. Under H-488, a transparent re-export chain should recommend explicit selection from the actual defining module, using an available public alias and preserving any local renaming, rather than merely recommending the nearest forwarding module. Recommend an explicit selection preserving the resolved identity and using/import interpretation, with an alias when needed. Do not blindly replace a source-preserving selection with import or imply that the suggestion changes the symbol's declared type or installs TLS. An imported/adapted callable identity is distinct from an older source body; "actual defining module" must identify the selected semantic object, not discard its interpretation by jumping to that body's ancestor. An unsafe or unproved textual replacement must not be offered as an automatic fix.

For the example above, a use of detail_code can report: `information: detail_code resolved implicitly through vocabulary.cod; consider selecting it explicitly with using.` A use already covered by an equivalent explicit selection no longer depends on that fallback. Ambiguous lookup remains a rejection, rather than a successful implicit resolution accompanied only by information. Informational output does not change name priority, overload selection or program validity. Diagnostic category spelling, controls, repeated-use grouping and automatic fixes are deferred; no warning/error escalation policy is selected.

Future acceptance covers the actual mention location, correct source/identity/context provenance, a meaning-preserving explicit-selection suggestion, no implicit-use report after equivalent explicit selection, and unchanged ambiguity/privacy rules. Shared or specialized checking must not manufacture a new source mention for every generated instance. D-16 fraction and UoM identity/dispatch/storage rules are unaffected.

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

The [symbolic contract](SymbolicDynamicScope.md#symbolic-storage-and-regional-provenance) distinguishes inline bits, regional object references, whole-region resources and procedure bindings. Independent bits bindings require `effect @x <- rhs`; regional reference bindings, including explicit scalar ref/ptr symbols under H-482, use `effect @x = rhs`. These storage categories do not put type definitions, method sets or region contents into TLS. No raw symbolic assignment is permitted.

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

Captured regional references preserve their referents: authorized referent edits remain observable, and capture does not deep-copy a region. Shared typed symbols and aliases retain their identity and scope-restoration requirements; concrete TLS representation is an implementation concern.

<a id="rtti-current-implementation-and-decision-boundary"></a>
## Actual-type dispatch and concrete-type evidence

The [source capability reference](../tests/cod/language/README.md) owns current concrete-type checking and native coverage. A source metadata representation or fixture does not select a general runtime type-information facility, reflection API or persistent layout.

The shilos storage layer separately records a region-root type UUID. The source prototype uses one native CoDRegionRoot UUID for its allocated regions. That header does not identify each CoD object’s concrete type, supply dynamic method dispatch or expose a CoD reflection API. [Compiler types](../cod/lang_syntax.hh) · [Method selection](../cod/lang_frontend.hh) · [Native object/TLS layout](../cod/lang_llvm.hh) · [Root wrapper](../cod/lang_native_runtime.hh) · [Storage header](../include/shilos/region.hh)

H-375 settles the behavioral requirement: family views dispatch on actual argument types, including abstract parameters/results. A selected Node/Edge result must retain enough evidence for a subsequent Entity-typed call to choose the appropriate method. A one-method or one-concrete-result implementation guard cannot define the language. Direct selection is permitted when equivalent for the selected family/context.

The fused artifact fixes the executable method definitions and interpretations prepared for execution, not every possible runtime type. Under [D-26](GenericAndRecursiveClosure.md), an outside-catalog type selects through CoD `<any>` without losing actual identity or required owner evidence. Valid uninterpreted passage does not add methods, edit a module world or require an unknown-type catch. An operation requiring unavailable interpretation raises checked `StaticallyUnknownTypeException`; same-T still uses actual evidence and breached established typing raises unchecked `UnsoundRuntimeTypingError`. Validation, access and lifetime remain required. This permits neither arbitrary opaque payload access nor executable method loading/runtime compilation.

The remaining H-251 boundary concerns concrete evidence/ABI, reflection and persistence, not whether dynamic dispatch is allowed. Type identity, selected world interpretation, value layout and actual owner remain separate. H-396 adopts actual concrete-type discriminators for abstract/polymorphic values and call-site facts that can narrow or omit selection. This does not require an object header or a second identity tag; physical transport and a permanent ABI remain separate. Root-type checksums are planned DBMR compatibility metadata, not dispatch discriminators. [AOT execution preparation](AheadOfTimeExecutionPreparation.md) records WPO/AOT coverage and that design backlog.

## Julia comparison and the CoD boundary

Julia provides the useful reference point of a runtime containing types, generic functions, methods and module bindings. Its current documentation describes global method/binding state and task-local world-age visibility; it does not describe independent semantic worlds owned by each module. [Julia world age](https://docs.julialang.org/en/v1/manual/worldage/) · [Julia functions and method tables](https://docs.julialang.org/en/v1/devdocs/functions/)

H-240 makes CoD's distinction explicit: **each module has its own dedicated world copy**. H-234 supplies explicit export/integration and local merging by identity. Julia's global world counter, method-table implementation, task world age and public namespace syntax are not selected by this comparison. CoD's TLS pointer set is not Julia's task-local world-age view.

This module-world independence belongs to the [Global Variables Done Right vision](GlobalVariablesDoneRight.md#two-complementary-worlds). It prevents selected semantic information from becoming one ambient mutable world shared by every module. The TLS side supplies explicit typed dynamic associations across those module boundaries.

## Callable families and explicit installation

Bare `other` denotes a symbol object carrying identity and semantics; `@other` reads its current typed TLS binding. H-381 requires nearest-binding lookup: a non-symbol local shadowing other makes @other ill-typed. Reads, effects, calls, touch and type inspection use that same rule. Same-scope module-category collisions and staging still need their complete contract. No general runtime Symbol ABI or implicit conversion in every value context is selected.

| Operation | Compilation/integration | Runtime |
| --- | --- | --- |
| Symbolic callable definition without effect | Define or extend the receiver’s family under existing permissions. | No implicit installation or capture. |
| `effect @sym(...) { ... }` | Define or extend and check the compatible family view. | Materialize/install that view when reached. |
| `effect @sym;` | Select the current composed family; add no method. | Materialize/install its compatible view. |
| `effect @sym = other;` | Resolve the source symbol’s selected family and destination contract. | Materialize that source view and install it under the destination. |
| `effect @sym = @other;` | Check the destination against the source TLS value’s contract. | Install the existing value, preserving its captures. |

The installed view preserves family/context and environment. A different destination identity does not redirect dispatch, merge identities or edit either world. A family with several methods can be a value. Export, using/import/mixin and extension remain module construction; none implicitly installs a receiver TLS value. Dependency initialization still follows its once-only incorporation contract and can execute its own explicit effects.

Skipped runtime paths do not install; repeated runtime execution does not repeat method registration. Existing scope/declaration rules constrain where combined forms may occur. Whole-program compilation prepares admissible actual-type combinations and dispatch. Independent module interpretations remain fixed for that composition and can coexist; a new explicit integration stage is separate from invocation.

Entry-module composition additionally follows H-379: extending a dependency callable requires an explicit entry installation, and that installation cancels dependency installations of the callable. Module definitions and explicitly retained source interpretations remain available. CoD must expose unused installation roots to LLVM elimination; the [cancellation contract](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) preserves initialization operand effects and all procedure-local installations. Once-only dependency initialization does not override this rule.

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

## Implementation availability and open work

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Conflict arbitration, extension of unrelated callable unions, cyclic initialization, general composition and persistent representation retain separate review scope. No cross-module world editor is a pending deliverable.
