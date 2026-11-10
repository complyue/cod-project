# CoD symbols, dynamic scope and callable kinds

Design revision: 94 · Recorded: 2026-11-09 · Phase: language/application co-design.

## Design authority

- **Bounded exception/lifecycle contract:** [H-267](DesignAuthorityRegistry.md#h-267)/[K-408](DesignAuthorityRegistry.md#k-408) accepts [D-17r3 E1–E5](ExceptionControl.md#accepted-first-implementation-contract), including explicit throw-from only, cleanup, fresh owners and call/result transfer. This overrides pending labels for that enumerated scope; wider qualifier, assignment, mailbox and user exception graph choices remain open. [K-414](DesignAuthorityRegistry.md#k-414) records completed H-267/H-269 E1–E5 acceptance and [current native evidence](../tests/cod/language/exception-ownership-validation.json). Wider unconfirmed scopes retain their status.

- **Registry / scope:** [D-10](DesignAuthorityRegistry.md#d-10), revision94, all sections; [H-265](DesignAuthorityRegistry.md#h-265)/[K-406](DesignAuthorityRegistry.md#k-406) adds exception-control planning in [D-17](ExceptionControl.md); [H-263/H-264](DesignAuthorityRegistry.md#h-263)/[K-405](DesignAuthorityRegistry.md#k-405) records permitted local-view survival and the first typed construction form; [H-262](DesignAuthorityRegistry.md#h-262)/[K-404](DesignAuthorityRegistry.md#k-404) records compiler-tracked consumption of named ordinary owners without changing dynamic resource capture/shadowing; H-257/H-258 and [K-399](DesignAuthorityRegistry.md#k-399) cover lexical callee selection, typed callable declarations and the compiler value/place distinction; [K-400](DesignAuthorityRegistry.md#k-400) records successful build repair and28 focused compiler/native/IR checks. [K-396](DesignAuthorityRegistry.md#k-396) records the completed architecture audit on the K-395 compiler, including H-253 conflict rejection, H-252 warning controls and cross-world TLS. Earlier checkpoints retain their actual scopes. H-234/H-237–H-249 supplies independent module worlds, separate typed TLS, vocabulary identities, receiver-local composition and identity-based publication; H-245 supersedes cross-module editing. H-251 reserves RTTI for human discussion. Earlier decisions/evidence retain their scopes in the registry. Full contracts and candidate value paths remain pending-human; this revision does not qualify unresolved mechanisms. [K-401](DesignAuthorityRegistry.md#k-401) records H-255/H-256 comment reservation and H-260/H-261 quantity/widening decisions; full fraction execution remains deferred.
- **Value lineage:** C-10's inspectable typed execution motivates C-35/C-43/C-45/C-46 identity/composition through P-84/P-100/P-102/P-109–P-111; C-56/C-57/P-182–P-197 covers abstractions, dispatch and adapter visibility. C-39/C-54/P-94/P-143/P-155 retains regional lifetime boundaries; C-58/P-199–P-203 covers publication and selected contexts. These candidate paths serve charter §§2–4 without borrowing authority from technical dependencies.
- **Human baseline:** H-96–H-141 supplies symbols/modules/units, subject to supersession. H-143–H-146 selects LLVM, lean checks and the zero-offset limitation. H-149/H-150 bounds value shadowing/reserved names. H-166/H-180/H-181 governs module lifetime, capture/shadow and non-extraction; H-184–H-189 governs typed lexical/symbolic procedures. H-193/H-196–H-210 governs regional data, intrinsic owners, explicit-memory functions, borrowing/const and transfer requirements. H-211–H-233 governs source selection, mixin, explicit interfaces, concrete dispatch, aliases/implementation contexts and publication, with the scope/exclusions recorded in the registry.
- **Invariants / non-goals:** preserve region-only non-bits storage, zero-cost relocation, no regional copy/move, same-owner stored edges, preserved hidden/captured owners and no dynamic access or procedure calls from functions. No module namespace, runtime interface/borrow registry, immutable snapshot, allocator/toolchain change or SNN model is selected. Compound views, higher-order overload values, full qualifiers/ownership construction/transfer and mailbox remain bounded or pending.
- **Acceptance / evidence:** H-245 authorizes the corrected architecture goal, whose implementation audit is recorded by K-396; H-221 records the completed ten-hour period. Native checks must distinguish source/caller contexts, conformance visibility, using/import permission, unique/ambiguous dispatch, demanded/unused missing bodies and owner lifetimes. The [prototype contract table](../tests/cod/language/README.md#current-bounded-acceptance) and [evidence below](#runnable-prototype-evidence) identify implemented subsets and actual source revisions; qualifier, transfer and mailbox acceptance requires its own evidence.

## Bounded native pointer bindings

The compiler keeps each module's selected semantic information in its own ModuleWorld, distinct from the flat dynamic-binding environment and source-loading state. For native scalar/reference effects, LLVM TLS entries contain pointers to lexical value cells. Rebinding saves the preceding pointer and scope exit restores it; WPO checks the symbol type and required binding. Definition-time captures retain their values separately. One entry-block cell per emitted binding site supports repeated loop execution without repeated stack allocation. Region/procedure effect values retain their specialized WPO paths.

[K-395](../tests/cod/language/world-conflict-validation.json) checks H-253 conflicting imported dependencies and cross-world TLS behavior. [K-394](../tests/cod/language/world-context-validation.json) retains the preceding interpretation checkpoint, with its conflicting-sibling silent-choice expectation superseded. H-252's multi-alias extension warning reports local declarations that share one callable identity; its opt-in error category is independent of unit-conversion warnings. These are compile-time mechanisms and leave the TLS/capture contract unchanged.

[K-396's architecture audit](../tests/cod/language/world-architecture-validation.json) combines current source inspection with 42 direct checks, covering receiver-local identity information, exports, source isolation, flattened data-pointer TLS, binding/type rejection, captured/current execution and owner lifetimes. Region/procedure associations are specialized by WPO without a generic runtime callable ABI. [D-15](ModuleWorlds.md#identity-records-aliases-and-publication) owns the construction/identity boundary. Earlier evidence retains its actual compiler revision; complete ownership/qualifier and Actor workflows remain separate.

## Module construction and runtime TLS editing

H-245 forbids cross-module module-world editing at compile time and runtime. Each module constructs its own information about types, callables and overloaded methods from explicit source exports; information about identical imported identities merges locally. The source world is not edited, and local integration does not implicitly re-export its contents. Compilation determines the flattened TLS shape and type obligations; runtime dynamically scoped effects change and restore its typed symbolic pointers. [D-15](ModuleWorlds.md) owns the architecture review.

H-238 requires different concrete entity inventories. H-239 specifies one flattened TLS collection of typed symbolic pointers: S identifies the symbol, T independently identifies its required type, and the pointer dynamically refers to a compatible value. That value may originate in another module-world copy. H-240 gives every module its own semantic runtime world; TLS does not duplicate those worlds or their method tables. Identity-compatible rebinding and module information merge are different operations. Capture-by-value, reference lifetime and ownership rules are not replaced by choosing pointers as the carrier. The complete inventory belongs in D-15 and the shared vision in [D-12](GlobalVariablesDoneRight.md), as required by H-241. H-235 requires ordinary CoD calls to avoid exposing the compiler's routing machinery. In particular, manually installing a procedure into a symbol is not a source-language remedy for an internal lookup defect. Existing perform/behave and deliberate effect binding retain their own language purposes.

H-236 requires architecture before local lookup fixes. H-247/H-248 makes that architecture concrete: source exports map public aliases to identities; receiver information is keyed by identity and can have multiple local aliases. Module construction reads source worlds and merges locally. H-246/H-249 directs removal of obsolete technical helpers and autonomous application of these principles within their confirmed scope. Callable-context machinery remains internal; no public world symbol, invocation protocol or special capture exception follows.

## Semantic core supplied by the user

| Construct | Meaning |
| --- | --- |
| `symbol tgt_neuron :: T` | Introduce a first-class symbol with strict identity and an explicitly declared value type. The runtime use is `@tgt_neuron`. |
| Import a symbol, optionally under a local alias | Refer to the same exported symbol identity. An alias does not create another symbol. This is distinct from redefining a procedure body with mixin. |
| Vocabulary module | Define typed symbols, primarily abstract types with some supporting ADTs, and their method collections; document their meaning and publish intended shared identities explicitly. A symbol declaration does not supply a default TLS binding. |
| `using` an external procedure | Refer to the procedure defined in the source module's lexical/dynamic scopes, preserving that definition and its captures. |
| `import` an external definition | Select an exported source definition with extension permission. An imported helper uses the receiving module's callable implementation context under H-232; source body/non-callable lexical bindings remain. |
| `mixin` an external procedure body | Define it again in the current module's lexical/dynamic scopes; capture effects used in that new definition context. |
| `@var_name` | Read the value under the declared symbol identity; WPO requires a supplied dynamic binding. |
| `effect @s <- value` | The only symbolic-variable write form; establish the value for the current program scope and restore the preceding binding when that scope ends. It evaluates to the newly bound value. |
| Define a procedure value | Retain its definition's lexical environment and capture by value the symbolic effects it uses; capture payloads are bits or regional pointers. |
| `perform @p` | Resolve the procedure, temporarily install its captured effect values in its execution scope, execute its body, and restore the caller's bindings. |
| `behave @p` | Resolve and execute the procedure with its definition's lexical environment and the current dynamic bindings. Its stored dynamic captures are not installed. |
| Function | Lexically defined; compute from explicit inputs to outputs, with no symbolic effect access, `perform` or `behave`. |
| Procedure | May have an ordinary lexical name or a symbolic dynamic binding; may access effects, invoke procedures and call functions in either case. |
| Module top level | Declare symbols, functions and bits globals initialized by literals or function calls; register procedure definitions. Top-level `effect`, `perform` and `behave` expressions are also permitted. Scanning runs none of their execution semantics. |
| Module execution composition | Scan/load every entry dependency first; then compile from the entry, incorporating dependency top-level execution semantics in encounter order. The compiled initialization supplies the module dynamic scope. |
| Required bindings | Checked on the effective caller paths during on-demand whole-program compilation; absence is a compilation error. |
| Expression / `void` | Evaluation yields a result; `void` explicitly indicates no meaningful returned value. The language direction uses expressions without a separate statement category. |

A procedure declaration/body can be registered before its definition event has execution semantics. Here capture at “definition” means the event that creates the procedure value in the resulting execution, not scanning the declaration. Compiling its code or binding an existing procedure value to another symbol with `effect` does not, by itself, recreate its captures. Named procedure declaration forms are described below; additional procedure-value expression syntax remains open.

The supplied `effect` spelling operates within the enclosing scope. It need not introduce a separate brace construct. Repeated/nested bindings restore in the corresponding unwind order. The procedure value used by a call is resolved in the caller's current context **before** its captures are installed; otherwise changing the dispatch symbol could change which procedure is being called.

## Typed symbol definitions

H-184 requires a type for every newly defined symbol, including those used to bind procedures:

```text
symbol a :: T1, b :: T2;
```

The names in this declaration introduce symbol identities in the module’s compile-time vocabulary. They do not declare runtime lexical values named `a` and `b`; symbolic reads remain `@a` and `@b`. Each identity has its declared value type before binding. A standalone `symbol` declaration does not supply a runtime value, even for an optional type. The compiler checks uses and installations against that type instead of choosing a permanent type from the first effect value.

A combined `proc @p(...) -> RT { ... }` declaration also satisfies the explicit-type requirement: when `@p` has no prior declaration or import, its signature declares the new symbol's procedure type. The symbol identity is established at compile time; the procedure value and dynamic binding are established at the definition's execution event. No separate `symbol p :: ...` declaration is required.

Identity-preserving aliases and vocabulary selections also preserve the declared type. Procedure selection/installation must respect the destination symbol’s declared procedure type. Mixin body maps translate identities at compile time, with typed uses checked in the receiving context. The implemented procedure-type notation is `proc(T1, T2) -> RT`; parameters/results retain their nominal type identities. Aliasing does not silently retype an identity. Broader callable subtyping remains outside this subset.

The executable requires a type on each `symbol` definition. Both `symbol x :: T` and `symbol @x :: T` are accepted, including comma-separated definitions. A procedure signature supplies the type in a combined declaration. The compiler-provided `std:io` print family remains scalar-polymorphic; binding it to an explicitly typed procedure symbol checks and restricts its signature.

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

The alias environment is a compiler name-resolution relation, not a set of lexical symbolic variables. A module can call S `@x` and another module can call the same S `@target`; both access the same thread-local dynamic binding. `effect @x <- 2` binds S and yields 2; `@x = 2` is invalid. Functions' ordinary lexical parameters/constants remain non-`@` values and do not gain permission to read effects.

In declaration/selection syntax, `@name` identifies which symbol is meant. In an ordinary expression, `@name` reads the runtime value bound under that symbol. Making identities compile-time values does not make those runtime reads compile-time constants. Any later syntax for passing symbol values to compile-time computations must preserve this distinction; no new runtime symbol-object type or user allocation carrier is implied.

### Identity introduction versus procedure installation

Vocabulary selection and procedure installation have different jobs:

```text
symbol @step :: proc() -> i64;
symbol from "snn/vocabulary.cod" @tgt_neuron as @tgt;

using "models/lif.cod" @update as @step;
# Alternative definition route:
mixin "models/lif.cod" @update as @step;
```

`symbol @step :: proc() -> i64` creates a local identity. The `symbol from ...` form introduces a local compile-time alias for an identity declared by the vocabulary; omitting `as` keeps its spelling. It does not retain an incidental runtime value bound to that symbol in the source module.

In the procedure forms, `@step` must already resolve to a symbol identity T, either from `symbol @step` or a vocabulary selection. The source module's `@update` selects the source procedure resource under some identity S. The operation installs the resulting procedure value under T. **S and T may be different identities.** `as @step` does not rename S, make S equal T, or create the missing destination symbol. Ordinary `using` without `as` requires an existing same-spelling destination with the source identity under H-120. Explicit `as` selects an existing destination even when its identity differs and its spelling is unchanged. The combined `using symbol` form below can introduce the source identity when the local name is absent. The corresponding outer `mixin` shorthand policy is not settled by this using-specific clarification.

| Form | Compile-time action | Resulting execution |
| --- | --- | --- |
| `symbol @local :: T` | Introduce a fresh nominal identity, its local name and value type | No initial runtime binding |
| `symbol from "vocab" @src as @local` | Make `@local` denote the source identity | Source execution still follows H-109; no selected runtime value is retained |
| `using "model" @src as @local` for a procedure | Resolve the source resource and existing destination identity | Install the source-defined procedure value under the destination identity, retaining its lexical context and captured values |
| `mixin "model" @src as @local` for a procedure | Resolve destination and reinstantiate the body with receiving lexical names and symbol-alias translation | Create the receiving-context procedure value/captures and install it under the destination identity |
| `effect @local <- value` | Resolve the local name to its identity and check the value | Evaluate the value, establish the dynamic binding, return that value and restore the old binding on scope exit |

Procedure declaration and using/import/mixin are explicit selection/creation/installation forms; they are not exceptions permitting ordinary `@local = value` assignment. A source's declaration origin and the module providing a procedure can differ: a vocabulary may declare the symbol that several models bind. The destination identity is a further independent choice. Tooling should distinguish all three and retain source provenance for diagnostics.

Two aliases of one identity cannot hold two independent procedure values simultaneously. If `@a` and `@b` both denote S, installing a procedure under `@b` changes the binding read through `@a` in that scope. Two independently usable model operations need two destination identities, which H-116's prior-declaration rule makes explicit.

### Combined using and identity-checked abbreviation

`using symbol "std:io" @print;` combines selecting the source identity with installing its source-defined procedure. H-120 permits an existing same-name local alias when it already denotes that identity. It rejects an accidental collision with an independent identity. This comparison uses resolved nominal identity, including aliases through a common vocabulary; matching spelling, type, procedure value or body is insufficient.

Let S be the identity selected by the source module and T the identity already bound to the receiving name:

| Form | Receiving name absent | T = S | T differs from S |
| --- | --- | --- | --- |
| `using symbol "m" @p;` | Introduce the local alias for S and install the procedure | Reuse the identity and install the procedure | Compilation error; require explicit destination selection |
| `using "m" @p;` | Error: destination is not declared | Install the procedure | Compilation error; require explicit `as` |
| `using "m" @p as @p;` | Error: destination is not declared | Install the procedure | Install the procedure under T, preserving S and T as distinct identities |

For example, when both modules select `@print` from the same vocabulary, the receiving module can write `using "std:io" @print;`. When it independently declares `symbol @print :: proc(i64) -> void;`, it must write:

```text
symbol @print :: proc(i64) -> void;
using "std:io" @print as @print;
```

Here `as` explicitly selects the destination; it need not change the printed name. It does not replace the receiving compile-time identity or merge it with the source identity. Usual type and required-binding checks still apply. The combined form likewise does not overwrite an existing alias's identity; adapting to a different destination uses ordinary explicit `using`.

The source-identity-preserving renamed combined form, `using symbol "m" @p as @out;`, follows the same rule: it introduces a name for S, with same-identity reuse consistent with H-120; a different existing identity requires ordinary `using "m" @p as @out;`. This generalization is not a new identity-changing declaration form.

The combined form represents one cross-module encounter: incorporate source initialization, retain its process-lifetime top-level scope and select the procedure under the receiving identity. Its two conceptual tasks must not be expanded into two source executions. Reusing a compile-time identity does not skip procedure installation, and H-140 makes incorporation once per module; a repeated selection reuses its incorporated resources. Definition captures, `perform`/`behave`, dynamic scope restoration and the scan-before-execution boundary remain unchanged.

This rule concerns the outer using destination. H-115's implicit mappings inside a mixed-in body remain receiving-name resolution, including independently declared identities; the outer identity guard does not silently change those mappings. Extending the outer guard to mixin or specifying import extension declarations remains separate design scope.

Acceptance for this refinement: a missing local name is introduced by the combined form; a shared vocabulary identity is accepted with or without `symbol`; a same-name distinct identity is rejected without `as`; explicit self-`as` installs under the existing destination without merging identities; renaming does not manufacture identity; and the combined form retains one source execution and the existing capture/restoration behavior. These cases are covered by the [native fixture results](../tests/cod/language/validation.json). The parser preserves whether `as` appeared, because normalizing an omitted alias to its source spelling would erase a semantic distinction. Mismatch diagnostics identify both resolved origins and show the explicit destination form.

<a id="import-time-alias-translation"></a>
### Mixin-time alias translation

H-115 adds a compile-time correspondence between source-body aliases and receiving-module symbols. Following H-117/H-118, the design spelling presents the supplied local symbol first and the source-body role second:

```text
symbol @step :: proc() -> i64;
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

The mapping is compiler input and creates no runtime alias relation. `using` keeps its source body/identities and captures; `mixin` translates the body and obtains captures in the receiving dynamic context. `perform` installs whichever values that resulting procedure captured; `behave` reads current dynamic bindings under that procedure's resolved identities. Capture values can be computed at runtime even though the set of identities and access locations is determined during compilation.

The outer `as @step` chooses the procedure's destination binding. The inner `@target as @tgt_neuron` supplies an existing local symbol for a role in the mixed-in body. Neither should be treated as an implicit text-wide rename. Translation must cover symbolic reads, effect targets and procedure-call symbols consistently. Source initialization and other already-created procedures are not silently rewritten by this body's map; H-109's source execution and its own binding checks remain separate.

The prototype requires each explicit source role to occur in the selected body, rejects duplicate source-role entries, and diagnoses attempts to split aliases of one source identity across different receiving identities. Mapping multiple such aliases to one receiving identity works. These are bounded diagnostic policies; the complete language policy for broader mappings remains open. No general symbol-mapping framework or runtime registry is required.

### Syntax and practical use

The `using/mixin "module" @source as @local` spelling keeps the requested operation, origin, source resource and destination visible. `symbol @local :: T` introduces a new identity; `symbol from "module" @source as @local` names an existing identity. This keeps compile-time symbol declarations together and uses using/import for source-preserving selection and mixin for receiving-context definition. Neither declaration permits `@local = ...` assignment. H-214 excludes a module-prefix namespace and qualified callable names. Grouped selections may be considered without binding a module name; callable use still goes through selected local names.

Keep source provenance at declarations/selections and short names in procedure bodies. A local alias rename accompanied by the corresponding explicit mixin-map update should preserve the selected identities and runtime behavior. Auto-matching source-body spellings is convenient, but a change to one side alone can change or break the correspondence; navigation and diagnostics should show the resolved identities, not just matching text. Ambiguous aliases should require explicit disambiguation instead of depending on selection order.

Symbol selection still triggers source execution under H-109, after complete scanning and during entry-driven composition. The source top-level scope remains alive until process exit under H-166; identity-only selection still does not select a bound resource under H-110’s resource-selection rule. H-140 makes repeated encounters reuse the module incorporated at its first encounter. Grouped selection syntax remains open; direct module-qualified callable expressions are excluded by H-214. A local alias read uses an already selected identity rather than performing another module selection.

Rust's `use path as name` illustrates local aliases for explicit source paths. Racket's `only-in`, `rename-in` and `prefix-in` illustrate selection and spelling controls. These are useful naming comparisons; they do not supply CoD's dynamic-variable, destination-binding, capture or body-mixin semantics. [Rust Reference](https://doc.rust-lang.org/stable/reference/items/use-declarations.html), [Racket Guide](https://docs.racket-lang.org/guide/module-require.html).

### Module origins without local module names

H-214 requires source modules to remain origins of selections rather than local namespace bindings. Using/import/mixin names the source at the selection site; calls use local lexical names, selected operator/intrinsic names or symbolic perform/behave. A single alias preserves the selected source identity; H-230 separately creates identities for extension-module callables and same-alias unions. Diagnostics can display package/module provenance without admitting Module.f or Module.@p as expressions. There is no qualified-call or qualified-definition escape from extension permissions. H-215 proposes a separate explicit import-as declaration for selected method names. H-217 puts implicitly brought method names in a shadowable lower-priority lookup tier; this does not create a module namespace. H-226 resolves explicit versus implicit names before typed method dispatch, preserves operation identity through aliases and rejects a demanded unresolved implicit-name conflict. Detailed signature selectors and alias availability remain design work. [The method-selection review](DomainLanguageDesign.md#type-associated-method-visibility-and-extension) records H-225–H-227 and the remaining boundaries. Preserve this boundary unless an actual serious design obstacle is identified.

### Explicit method aliases and implicit method names

The form `import "module" operation as local_operation;` selects a source-preserving callable alias with extension permission. An alias names the selected callable in its import context. H-230/H-232 gives receiving-context callable adaptation and same-alias unions distinct identities while retaining source method definitions; this is not body redefinition as mixin does. Lexical aliases must not be confused with a symbolic procedure selection that installs a value under a predeclared destination identity. The local spelling is not the signature's identity.

H-217 gives automatically brought method names lower priority than explicit bindings in the same receiving scope. Name lookup first resolves the local binding, then argument-type dispatch uses that operation's visible method set. An explicit binding with no applicable method does not fall back to a hidden implicit name. Ordinary nested-scope shadowing and duplicate explicit-declaration rules remain; no new method is authorized merely by a using selection. Separate import-as can both supply an explicit alias and grant extension permission.

Implicit collisions remain unresolved until a demanded name or imported-family extension needs one identity, so explicit aliases can resolve the conflict without encounter-order winners. H-226 selects local shadowing after using a type and unique-family extension after import; it does not authorize use of an explicit alias before its selection event or new initialization effects. Source bodies retain definition context under using/import; H-232 adapts an imported helper's callable implementations to the receiving module; this must not silently become general lexical rebinding. Methods may be distributed across modules and inherited through abstract <: without an Interface body; only demanded missing implementations are errors. The full signature and visibility rules are in D-07.

A private helper remains a source lexical definition even when its enclosing imported helper adapts public callable dependencies. That selected implementation context propagates through the private body; an unrelated same-spelled receiving definition does not replace it. Direct using/import/mixin of the private name still requires an export. [Native private-helper evidence](../tests/cod/language/private-helper-validation.json) covers these boundaries and preserves typed procedure captures through the resulting call chain.

### Captured callees and selected implementations

A symbolic call uses the selected procedure value in its dynamic binding. Importing the containing helper does not rewrite that stored value into a different implementation. `perform` restores the helper’s captured values; `behave` uses the current bindings. An explicit effect installation can supply an imported procedure whose own ordinary callable dependencies use an adapted implementation context. These are separate selections, with no recapture or runtime method dictionary.

[Native composition evidence](../tests/cod/language/captured-callee-validation.json) uses a source inner procedure computing score(x) + @bias. The source score returns1, the adapter score returns7, the definition-time bias is1 and the current bias is5. A routing procedure invokes the procedure-valued @task; the caller replaces that binding with the adapter-selected inner procedure.

| Call | Native result |
| --- | --- |
| Direct source-preserved helper | 6 |
| Direct import-adapted helper | 12 |
| Perform the source-preserved or import-adapted router | 2 for each; both restore the original captured @task value. |
| Behave through either router after explicit @task rebinding | 12 for each; both use the selected current procedure value. |
| Perform the explicitly adapted inner procedure | 8; adapted score7 with its retained captured bias1. |
| Read caller @bias after the calls | 5; temporary captures did not replace its binding. |

The complete generated sources and output are preserved in the record. This checks composition of the existing capture/context contracts; it does not select a policy for competing dependency views or establish lexical-call acceptance; H-257 and K-400 supply that separate contract and evidence.

### Callee selection and argument rebinding: native observation

[The argument-rebinding record](../tests/cod/language/call-argument-observations.json) checks the current lowering when an argument changes the symbolic callee binding. The prototype resolves the procedure value first, evaluates arguments in the caller environment, then invokes that selected value. Perform uses its stored captures; behave uses the environment after argument evaluation. Argument-side effects belong to the enclosing scope and remain after the call; restoring the procedure body does not undo them.

The example selects an exported read procedure with captured bias10 and defines an alternate with captured bias20. Passing an argument that binds bias31 gives41 under perform and leaves caller bias31; binding32 gives64 under behave and leaves32. Rebinding the callee symbol to the alternate inside an argument does not change the in-flight selected read: it returns43 under perform or68 under behave. Subsequent calls use the alternate, producing3301/3401 with current bindings or2001 with its capture.

This is actual prototype behavior, not a newly confirmed language-wide call-order rule. H-148 fixes each argument's conversion before the next argument; it does not expressly place symbolic callee resolution relative to effectful arguments. That timing distinction can be reviewed with the local value/call namespace question. The probe adds no callback-call syntax, runtime dispatch or changed capture mechanism.

### Explicit conformance: declaration permission and consumer visibility

H-218 permits `data D <: A;` to add conformance for an existing data identity, including declarations in different modules and more than one abstraction. Available signatures never infer conformance. H-220 requires an adapter to import the ADT before declaring such a relation. Using the ADT is insufficient to authorize the declaration; using the adapter is sufficient for consumers to obtain that declared relation. Visibility to use and permission to extend are separate facts.

Source-preserving functions/procedures retain their source body, non-callable lexical bindings and captured values. The native implementation retains source-family provenance while selecting ordinary methods under distinct module/context identities; it does not accumulate every caller-visible method. H-230 requires distinct extension/union identities; H-232 gives using the preserved selected context and import the receiving implementation context; compound alias/context conflicts remain to be specified. H-229 preserves established conformance of concrete arguments through broad constraints and nested traversal, without publishing unrelated caller type relations. Operator/intrinsic candidate lookup remains lexical. Mixin checks the redefined body in its receiving context: interface use needs the corresponding adapter view, and declaring new conformance needs import permission on the ADT. This is independent of perform/behave's dynamic-value selection. No new effect binding, body capture, region permission or runtime type field follows from conformance.

D-07 illustrates selecting the data type through an adapter with using. Explicit export/re-export and export import have a bounded implementation; compound publication choices remain pending. Neither requires module-qualified calls or a runtime interface registry. A source body may use its own declared relations without exporting those facts through every callable selection. Once-only incorporation and explicit alias availability retain their existing rules.

### Explicit publication and extension identities

H-231 makes module resources private unless declared with export. Using/import/mixin, symbol selection and implicit associated-method selection must respect that boundary; an alias or selected type cannot bypass it. Public source bodies can use their own private dependencies without making them directly selectable. H-233 requires a receiving alias to have its own explicit export before it can be selected again. The confirmed shorthand export import performs the normal import and publishes everything that statement selects, including its associated method selections, within source privacy constraints. The proposed export list and outstanding conformance/conversion bundle details are in [D-07](DomainLanguageDesign.md#explicit-exports-and-private-module-resources); the prototype supports single-name export declarations and export import, while grouped-list and broader bundle syntax remain outside it.

H-230 requires distinct callable identities for extension modules. Different local aliases preserve their selected method sets; same-alias composition of versions of the same semantic callable creates a virtual union identity in the selecting module. The required base/extended/indirect example is1,7,1. The [helper-context example](DomainLanguageDesign.md#extension-module-callable-identities-and-local-unions) has a confirmed result under H-232: using base twice gives2, while import base twice gives14 through the receiving score implementation. A later using preserves the selected exported context. This adaptation does not change symbol identity, dynamic bindings, capture mode, source globals or general lexical body resolution; mixin remains body redefinition. H-253 requires conflicting inherited and receiver-selected overloads to reject during import adaptation; no alias/identity priority silently selects a winner. Explicit arbitration syntax remains future design.

### Implementation boundary and acceptance

The native prototype selects existing definitions with using/import and redefines symbolic procedure bodies with mixin. Source-preserving selections retain types, lexical resolution and captures; symbolic destinations must be declared before selection. Omitting as requires matching source/destination symbol identities, while explicit as can install the source-defined procedure under a different predeclared destination. Combined identity introduction remains using symbol. Direct named-callable extension requires import under the extended local alias. The native implementation keeps semantic-family provenance separate from extension/local-union identities and checks exports for every cross-module selector. Original methods with no ordinary callable dependencies deduplicate across contexts; distinct observable contexts are not silently erased. Import stores a receiving implementation context, using preserves the selected context, and scope restoration prevents cross-alias leakage. Selecting a type also brings its source-visible associated methods at lower name priority, including result-only factories and inherited applicable signatures. Local ordinary declarations shadow using-only implicit names or extend a uniquely imported family. Original methods deduplicate across paths. Cross-alias grants and extension of unrelated merged callable families remain outside the prototype. These limits do not change the separate declaration rules for compiler operations.

Mixin translates source-body symbol roles to receiving identities before checking required bindings and collecting captures. Repeated mixing uses the current provider's aliases; it does not rewrite the source initialization or existing definitions. Body maps are rejected on using/import, and built-in procedures have no body to mix. Lexical procedures may also be mixed under ordinary local names and subsequently installed into symbolic bindings. Operation-overload mixin remains outside this subset; mixin does not copy type definitions, function definitions or globals. UoM selection retains its separate using-units contract.

[The module-selection checkpoint](../tests/cod/language/module-selection-validation.json) records source-preserving and receiver-context calls, captured/current values, type aliases and lexical procedure values in one native case; existing mapping and region-factory cases now use mixin. The full batch passed 153/154 checks, with the sole mismatch an obsolete expected diagnostic for using-with-map; updating that expectation and rerunning the affected case plus frontend-only validation passed 2/2 without another compiler change. All five composition cases passed, including the 96-module regional mixed-body chain. This is evidence for the selected subset, not abstract conformance, implicit method names or extension permissions.


## Vocabulary modules and procedure reuse

The customary vocabulary module defines typed symbols, types (primarily abstract types, with a small number of supporting ADTs) and their method collections, with comments explaining their intended meanings (H-242). These declarations establish common symbol, type and callable identities for independent module worlds. Different modules can deliberately bind the same vocabulary symbol to compatible values from their own world copies. Selecting a symbol declaration does not install a default value; similarly spelled declarations from distinct origins remain distinct. Comments aid understanding; they do not substitute for the compiler's type and required-binding checks. The [vocabulary inventory](ModuleWorlds.md#vocabulary-modules-establish-shared-identities) keeps method-head and implementation choices within the architecture review.

Definition reuse, callable implementation selection and capture are separate:

| Operation | Body and non-callable lexical bindings | Ordinary callable implementations | Dynamic captures |
| --- | --- | --- | --- |
| `using` | Source definition | Preserve the selected context | Retain the source value's captures |
| `import` | Source definition | Select the receiving context; permit extension | Retain the source value's captures |
| `mixin` | Redefine the body in the receiving module | Resolve in that new definition | Capture at the receiving definition event |

`mixin` is not merely a local alias for the existing procedure value. The body must be checked as a definition in the receiving module, including its lexical references, symbol identities, types and reachable call paths. It need not imply textual file copying or duplicated machine code: equivalent definitions can share generated code while preserving their semantics. The bounded selector/map syntax is described above; module-qualified callable names are excluded by H-214; private-by-default selection is required by H-231; explicit single-name export and export import are implemented; grouped selections remain outside this prototype.

This distinction is independent of the invocation mode:

| Definition route | `perform` | `behave` |
| --- | --- | --- |
| `using` | Preserved implementation context and source captured values | Preserved implementation context and current caller bindings |
| `import` | Receiving callable implementation context and source captured values | Receiving callable implementation context and current caller bindings |
| `mixin` | Receiving definition and newly captured values | Receiving definition and current caller bindings |

For example, a body's unqualified lexical helper can compute 4 in its source module and 9 in the receiving module. With the same vocabulary symbol `@x`, source capture 1, receiving definition capture 7 and later caller binding 8, the conceptual `(helper_result, @x)` results are: using/perform `(4,1)`, using/behave `(4,8)`, mixin/perform `(9,7)`, mixin/behave `(9,8)`. Import changes the helper implementation only through the receiving module's selection of that callable family; an unrelated helper with the same spelling does not replace the source helper. All three routes retain the selected procedure value when invoking perform/behave. These distinctions require no new pointer or runtime capture representation.

H-116 requires the destination symbol to exist beforehand, either from a local declaration or an imported vocabulary identity. The outer `as @local` selects that destination for the resulting procedure value; it does not alias the source symbol identity. A compile-time alias alone never creates a fresh symbol identity. H-115 permits a separate source-body alias map during mixin. The compiler must retain enough source/semantic body information for redefinition; compiled machine code alone cannot supply the receiving lexical meaning. The entry-driven compilation phase incorporates module initialization semantics that determine these definition events and their required bindings. The earlier scan only registers declarations; none of `using`, `import` or `mixin` makes that scan an effectful definition event.

## Functions and procedures

H-197/H-198 establishes functions as lexical callables that may operate on explicit memory and explicit-argument system intrinsics without reading or writing dynamic bindings. Procedures may additionally use dynamic context. The ordinary function contract is mutable; a future const-function form would be a separate design.

A function definition binds a lexical name and cannot access dynamic effect variables. Under H-184, a procedure may be defined with an ordinary name in lexical scope or with a symbol name in dynamic scope. Both procedure forms may use effects; lexical naming does not make a procedure a function. A function cannot call either procedure form, directly or indirectly.

A lexical procedure keeps its lexical definition context and H-180’s captured values of the dynamic variables it uses. Installing that existing procedure with `effect @operation <- local_procedure` preserves the selected definition and captures; assignment is not another definition event. The destination identity must have an explicitly declared compatible procedure type. Procedure `mixin` is the distinct operation that redefines a source body in receiving context.

H-185 specifies the call forms: `p(...)` resolves a lexical procedure; `perform @p(...)` and `behave @p(...)` resolve a procedure through its symbolic binding. H-189 makes direct lexical calls use the caller's current dynamic bindings. `perform` temporarily installs definition-time captured values; `behave` uses current values. `perform p(...)`, `behave p(...)` and direct `@p(...)` calls are invalid. Pure functions cannot invoke procedures through any form.


| Call form | Callee lookup | Established invocation rule |
| --- | --- | --- |
| `p(...)` | Ordinary lexical procedure name | Use current dynamic bindings; no symbolic callee selection. |
| `perform @p(...)` | Procedure value currently bound to the symbol | Temporarily install that procedure’s captured dynamic values. |
| `behave @p(...)` | Procedure value currently bound to the symbol | Use current dynamic values. |

### Lexical calls and shadowing

H-257 requires ordinary call syntax to respect the nearest lexical value binding. A procedure parameter or local procedure value supplies the callee; a same-spelled module callable does not bypass it. A non-callable local value makes the call ill-typed. Direct calls retain H-189's current dynamic bindings. Symbol-only `perform`/`behave` and the function/procedure effect boundary are unchanged.

```text
proc selected(x::i64) -> i64 { x + 20 }
proc callback(x::i64) -> i64 { x + 1 }
proc apply(callback::proc(i64) -> i64, x::i64) -> i64 { callback(x) }
# apply(selected, 3) must yield 23.
```

A parameter or local declaration that shadows an outer local, module callable or module value/type binding emits `[-Wshadow]`. `-Werror=shadow` or the general warnings-as-errors option makes it an error. Same-scope duplicate declarations remain errors. No user-visible call protocol, escaping closure representation, general function-value type or runtime callable dispatch is introduced.

### Callable declarations and methods

A callable identity can exist without any executable methods. Its signature declarations and its methods are separate module-world information. Using/import, explicit exports, aliases and receiver-local merging apply to both; selecting or exporting an unimplemented declaration does not execute or fabricate a body. A demanded call requires an applicable implementation. A function declaration promises a function implementation with a compatible result for its applicable inputs; a procedure declaration supplies the corresponding procedure contract. These promises are checked on demanded calls, alongside the selected method's own signature.

H-258 requires parameters and an explicit return type on a bodyless declaration; an untyped identity-only `function score;` is not supported. Generic declarations and methods use compile-time type parameters, including abstract upper bounds, and a final variadic parameter:

```text
function score(x::T)::i64 where T <: Metric;
function combine(first::T, rest::T...)::T where T;
proc dispatch(first::T, rest::T...) -> void where T;
function pair(x::T, y::U)::i64 where {T, U};
```

Type parameters are inferred from concrete argument types at a demanded call. Repeated occurrences constrain those types to agree; bounds use the caller's selected conformance information. A type parameter that cannot be inferred from inputs is diagnosed. Unique-most-specific dispatch compares signature constraints, including concrete types, bounds, repeated-variable relationships and fixed versus variadic arity, independently of declaration order. Equally specific candidates remain ambiguous. This bounded prototype does not supply generic ADT constructors, explicit type-argument calls or a complete higher-rank type system.

Within a method, `len(rest)` gives the tail count, `rest[i]` reads an element and `other(first, rest...)` forwards it. The finite argument pack is compiler metadata with each argument's existing provenance; it is not a user heap object. A dynamic index into regional arguments requires a common live owner. A pack cannot escape its call scope or become a TLS value. Empty tails are valid when their element type can be inferred from another argument or is explicitly concrete. Automatic conversion still completes for each argument before the next argument's effects.

### Evaluated bits and storage places

H-257 directs the compiler model toward LLVM register optimization. An evaluated bits value is an SSA value; a storage place denotes a typed address in a tracked owner. Reading a place produces the value at that point. Writing the place does not retroactively change that SSA value. A retained regional reference permits a subsequent read to observe intervening writes. Const non-bits access therefore does not imply immutable underlying memory.

LLVM lowering uses typed places for referent, field and element access, with common read/write operations. Regional aggregate views retain their offsets and owners; relative reference fields keep their existing encoding. Bits operations and variadic argument selection do not require artificial memory cells. This internal separation does not introduce `owned`/`const`/`make` source syntax, change the assignment-result contract, extend owner lifetime or select RTTI. The complete ordinary-region lifecycle is the separately deferred fourth work package.

### Direct symbolic procedure declaration

H-186 permits defining the symbolic procedure directly:

```text
proc @p(arg::T) -> RT {
    ...
}

perform @p(argument);
behave @p(argument);
```

`T`, `RT`, the argument and body stand for the chosen parameter type, result type, value and implementation. The declaration defines a procedure under the symbolic name `@p`; no intermediate lexical procedure or separate assignment of its value is required. H-187 also permits it without a preceding symbol declaration or import:

| Symbol availability | Meaning of the declaration |
| --- | --- |
| No declared or imported `@p` | Introduce a new nominal symbol identity, declare its procedure type from the explicit signature, and define the procedure under that identity. |
| `@p` already resolves to a declared or imported identity | Retain that identity and check the procedure signature against its declared type; do not silently create a replacement identity or change its type. |

This is a combined declaration, not inference of a symbol type from an arbitrary runtime value. It does not introduce an ordinary lexical callable named `p`.

Definition-time capture and scoped installation occur under the existing execution rules; scanning registers the declaration without executing it. Calls still use `perform @p(...)` or `behave @p(...)`, with their captured/current dynamic contexts respectively. This form does not permit direct `@p(...)` calls or dynamic-context access in functions.

The implemented signature type is `proc(T1, T2) -> RT`. `proc` and `procedure` introduce the same callable kind; declarations require an explicit result type, including `void`. Definitions are currently module-level. Named lexical procedures participate in typed method dispatch; obtaining a procedure value by name requires one unambiguous declaration and its definition event. General procedure-valued scope results and persistent closures remain outside the prototype.

### Callable context and memory access

The procedure’s name binding, its body’s lexical environment and its use of dynamic effects are separate facts. Registering a body during scanning does not perform captures; its runtime definition semantics still follow the existing scan/incorporation boundary. Nested-definition placement, arbitrary lexical captures and ordinary-name overload collision policy require their own bounded design. No heap closure or new capture lifetime follows merely from adding lexical procedure names.

A function uses explicit inputs and may read/edit their reachable memory. It may call functions and use permitted lexical definitions/constants, but cannot read or rebind an ambient @symbol, invoke a procedure through another spelling, or obtain hidden dynamic context. Explicit-argument intrinsics can have external consequences under H-197. No referential-transparency or read-only guarantee follows from the function keyword. Lexical captures must not smuggle dynamic access or an invalid storage lifetime into a function.

Procedure signatures may have explicit arguments and results as well as effect requirements. “Procedure” does not imply a void return. Function-valued arguments require functions; passing a procedure there cannot bypass the effect restriction. Methods need the same callable-kind distinction.

The checker should retain each procedure's required symbol identities and read/write/binding use through calls. This is a proposed implementation of the user's distinction, not a commitment to a general row-polymorphic effect calculus. A small finite requirement set is sufficient for the first examples. Foreign calls need a trusted declaration of their effects; an opaque native call is not presumed pure.

Constructing a description and executing its requested operation remain distinct. A function may construct/edit regional data through explicit resources once the source allocation contract exists. Looking up @region, or creating a procedure that captures current effects, accesses dynamic context and therefore cannot occur inside a function. Context exclusion applies transitively through helpers and foreign intrinsics.

The SNN support layer may use functions for numerical work and explicit array edits, with procedures supplying ambient target/frame resources. This callable boundary selects no neural transition or learning algorithm. H-200/H-201's borrowing, local-bits and const-value contract needs its own source implementation; K-311 validates the existing explicit-reference memory subset only.

## Captured values and call behavior

Let `D` be the caller's current symbolic bindings, and let a procedure value `p` contain a body, lexical environment `L`, and captured values `C` indexed by its required symbols. These are semantic components, not a prescribed heap object layout.

```text
perform @p:
    save caller bindings for the identities in C
    install C in the procedure's execution scope
    evaluate body under L and those temporary bindings
    restore the saved caller bindings on scope exit

behave @p:
    evaluate body under L and the current dynamic bindings
```

Only the captured identities are temporarily installed. `behave` does not erase lexical captures or turn lexical names into dynamic names. The callee can establish further `effect` bindings normally in either call mode.

For a scalar example, evaluate `effect @x <- 1`, define `@p` whose body reads `@x`, then evaluate `effect @x <- 2` in the caller scope:

| Observation | Result |
| --- | --- |
| `perform @p` | 1 |
| `@x` after that call | 2 |
| `behave @p` | 2 |

If the body evaluates `effect @x <- @x + 1` and then reads `@x`, its local binding is restored on exit in **both** call modes. With capture 1 and caller value 7, two `perform @p` calls each observe 2; two `behave @p` calls each observe 8. The caller remains 7 after every call. `behave` omits capture installation; it does not make the body's own scoped writes permanent. Installing a captured scalar does not mutate the stored capture itself.

Scope exit includes the selected language's structured returns and failure/unwind paths. Lowering must arrange restoration on each such path. It does not promise to run cleanup after process termination, or roll back effects performed through referenced objects.

### Regional resource capture preserves owners

Three different facts must remain distinct: the symbol identity used for lookup, the value captured at procedure definition, and the region’s ownership/lifetime. A procedure captures the then-current value of each required dynamic variable under its resolved identity. It does not reread an old variable’s later value as a substitute for that snapshot. For a region-backed resource, the captured value identifies the existing resource; it is not a copy of the object graph or a new independent owner.

Suppose `@work` denotes R1 when `@p` is defined. The caller subsequently shadows `@work` with R2 while both owning scopes remain alive:

| Point | Region reached through `@work` | Ownership and lifetime |
| --- | --- | --- |
| Before `perform @p` | R2 | R1 and R2 retain their respective owners. |
| Inside `perform @p` | Captured R1 | R2 is hidden through this variable only. Its ownership and cleanup responsibility are untouched. R1 also retains its existing owner. |
| After `perform @p` | R2 | The caller binding is restored. Neither region was released by capture replay or shadow removal. |
| Inside `behave @p` from that caller | R2 | The procedure uses current values; its saved R1 is not installed. |

A previous reference or another valid binding to R2 is not invalidated merely because `@work` is shadowed. Mutating R1 through the captured resource changes the live graph; leaving the call restores lookup, not graph contents. Removing the temporary shadow does not free R1 either. Only actual ownership/scope events release resources, and a region newly acquired within the call follows its own owning scope.

The supported typed-reference prototype also retains compile-time interface facts associated with the selected reference value. Captured/current lookup must not erase those facts, and rebinding must not accumulate the previous value's guarantees. [Native evidence](../tests/cod/language/effect-conformance-validation.json) distinguishes Weighted captured dispatch from Alternate current dispatch without publishing either adapter relation globally. This is compiler evidence propagation; it adds no runtime tag, reference permission, graph snapshot or lifetime extension.

Captured resources must be valid for their uses. H-180 supplies owner preservation during temporary shadowing; it does not choose a reference-counting system, an implicit lifetime extension for an escaped procedure, or a region-copying snapshot. Definition-time capture, temporary replay and explicit ownership transfer must have different checking/lowering paths. H-181’s restriction on moving a resource out of a dynamic owner must not reject legitimate captured replay.

### What “uses” includes

Capturing only the symbol spellings appearing directly in a body is insufficient. If `p` executes `behave @q` and `q` reads `@gain`, then `@gain` can be a dynamic dependency of `p` even when the name never appears in `p`'s source. Conversely, a `perform` call can supply a callee's dependencies from that callee's own captures.

The required-symbol analysis must account for dispatch, transitive calls, higher-order arguments, reads/writes and bindings that already supply a value inside the procedure. Captures are taken before later control-flow branches are known; a conditional read is still a potential use unless checking proves it unreachable. A procedure that provides its own binding before every access need not capture an irrelevant outer value.

CoD checks the demanded whole-program execution with its caller paths available, rather than compiling a module against an unknowable future caller. A finite summary or fixed-point analysis is a candidate way to handle recursion and branches; the semantic requirement is not to enumerate an unbounded runtime stack. Missing requirements cannot be hidden behind an empty summary or deferred to an ordinary runtime lookup failure.

### Call-path binding checks

H-102 locates checking at compilation of the demanded call paths. The compiler follows module initialization, callers, scoped bindings, callable selection and invocation mode; declaration text in an isolated module is not the compilation unit.

- For `behave @p`, required effects must be available in the current effective dynamic scope along each reachable path to their use, accounting for bindings established inside the body.
- For `perform @p`, the values it must install must have been obtainable at the applicable definition event. Missing required capture values are compilation errors, with no fallback to the caller's values.
- Branches and possible callees are checked in their corresponding contexts. Knowing caller paths does not require knowing runtime branch values; a reachable path lacking a required binding cannot be excused by another path that supplies it.

The reusable-operation case is therefore checked through its complete definition/call paths and chosen mode. This does not select a runtime missing-value sentinel, implicit zero value, or a separate-module rejection policy. The precise analysis must preserve by-value captures while avoiding requirements for values that the checked path does not use.

### Module scanning and execution composition

H-106 separates registration of the complete dependency set from compilation of execution semantics. The module top level admits `symbol`, `function` and **bits global-variable definitions whose initial values are bits literals or function calls**. It also permits `effect @name <- rhs`, `perform` and `behave` expressions under H-107. A top-level `procedure @proc_name {}` registers its definition body during scanning. Global initializers and executable top-level expressions belong to module execution; the scanner records them without invoking functions/procedures, installing effects or capturing runtime values.

| Phase | Required work | Meaning of a top-level procedure definition |
| --- | --- | --- |
| 1. Scan/load | Scan the entry and all its dependency modules, registering the permitted definitions and retaining initializers, executable expressions, procedure bodies and their origins. Finish this phase before compiling entry execution semantics. | Available to resolve and compile; no procedure invocation, dynamic binding installation or value capture occurs merely from scanning it. |
| 2. Compile execution | Start from the entry module's execution semantics. As this compilation encounters dependency modules, incorporate their top-level execution semantics into the resulting program in that encounter order. | Its definition/binding semantics join global initialization and top-level effect/call expressions in the resulting program's module execution scope, with the appropriate source-preserving or mixin definition context. |

For a non-recursive example, suppose scanning loads dependencies A, B and C in that order, but compiling the entry encounters B and then A. The dependency execution-semantics incorporation order is **B, A**, independent of the earlier scan order. C's scan alone does not cause its execution semantics to be incorporated. Any cross-module reference qualifies as a trigger under H-109, including a vocabulary symbol, `using`, `import`, `mixin`, or a function/global reference. H-140 incorporates each module only once. These are compilation encounters after scanning, not immediate execution of user code by the compiler. A diamond dependency reuses the incorporated module; its initialization effects are not duplicated.

The native diamond fixture uses export import to publish a selected symbolic procedure and later selects it under another alias after changing the caller’s binding. Its shared initializer runs once and perform still sees the original capture. [Recorded order evidence](../tests/cod/language/module-order-validation.json) separates the native output from the compiler’s verbose scan/incorporation report; --check emits no user-program output. The same record permutes local method declarations while keeping imported/type/conformance visibility fixed, without changing encounter availability.

“Compile execution semantics” means constructing the resulting program, not running user initialization effects in the compiler. Capture values follow the definition event in that program's dynamic execution; they are not snapshots of the scanner's state. Required bindings are still checked at compile time over the resulting caller/initialization paths. Declaration availability therefore does not prove that a required dynamic value is available.

The entry module and every incorporated dependency retain their top-level scopes until process exit under H-166. Finishing a dependency’s initialization body does not end that scope or release its still-held regions. Ordinary nested procedure/block scopes keep their own exit boundaries. A region explicitly transferred out of a top-level holder need not remain allocated until process exit.

Scope lifetime and active binding lookup must be distinguished. H-110’s resource-selection boundary remains: `using`/`import` selects the source-defined procedure/resources and `mixin` defines the selected body in the receiving context. Merely selecting a vocabulary identity does not select its bound resource. H-166 changes lifetime; it does not explicitly replace these selection rules with automatic ambient exposure of every dependency binding. The implementation must retain module contexts without treating initialization return as their destruction. The precise retained-context representation and shutdown ordering remain open.

A source-defined procedure can therefore refer to a region still held by its module top level after initialization completes. Its captured references remain subject to the owning region’s lifetime, and a capture itself does not retain an independently ended nested region. Source versus receiving contexts, capture-by-value and current-binding `behave` remain distinct.

**Implemented boundary:** module-owned regions remain live after initialization and are released at normal program exit. Returning to the incorporating module restores its active binding lookup; the retained source context and its owners remain available through explicitly selected resources. Ordinary nested blocks and calls still release their local owners on exit. The source-module capture fixture reads its region after initialization; the nested-region escape fixture remains rejected. [K-295](DesignAuthorityRegistry.md#k-295) records the implementation and native evidence. This synchronous prototype has no user shutdown hooks or Actor lifecycle policy.

The [guarded-owner fixture](../tests/cod/language/short-circuit-owner-validation.json) uses an explicit scoped right operand of && to acquire each inner region and a skipped right operand of || to verify that no extra acquisition occurs. The independent harness observes release order1,2,3,0 and eighteen reads against live owners. Unscoped conditional rebinding remains rejected; short-circuiting does not change captured/current lookup or extend a nested owner's lifetime. This is expression control flow within the existing module initialization, not a rule for conditional module incorporation.

Any kind of cross-module reference encountered during execution compilation triggers incorporation of that module's execution semantics, including `import` and `mixin`; definition reuse does not exempt source execution. H-140 requires once-only incorporation in first-encounter order after scanning. Repeated encounters reuse the selected resources and do not produce another initialization body or recapture under a different caller. Cyclic initialization and branch handling remain separate questions; the prototype diagnoses a demanded initialization cycle. This limit does not select permanent cyclic-initialization semantics.

For a bits global, a bits literal and a function call producing the required bits value are permitted initializer forms. A procedure invocation is not thereby a function-call initializer; top-level `perform`/`behave` remains a separate allowed expression form. Function initializers retain the no-dynamic-context contract, even though their evaluation belongs to module execution. This permits computed model constants without introducing scan-time user execution.

Top-level `effect` establishes bindings in the module's execution scope, while `perform` and `behave` follow their ordinary capture/current-binding semantics there. Compiling these expressions places their behavior into the resulting program; it does not run them during dependency discovery. The registration/execution distinction applies to expressions as well as procedure declarations.

The exact literal grammar, global mutability/sharing and physical realization remain to be specified alongside H-99's storage boundary and H-197/H-198's function boundary. Permitting bits globals or effectful module execution does not by itself introduce native-heap user objects or ambient mutable global reads in functions.

### Whole-region owning bindings

H-190 confines intrinsic ownership to whole regions. Intra-region free lists, object pools and slot reuse are user-library policies under ordinary typed access; they do not introduce per-object language owners or change region release/transfer. A still-live region does not by itself validate a reused slot's business identity. [D-14](RegionalOwnership.md#remaining-syntax-semantics-and-idiom-validation) records the missing whole-region lifecycle and library-sufficiency validation.

The following lifetime constraints are confirmed; the source-language meaning of an ordinary owner is still incomplete under H-183. [D-14](RegionalOwnership.md) designs the required declaration, access, construction, optionality and transfer forms together. H-262 confirms that an ordinary named owning source becomes statically unavailable after transfer; it need not use optional storage. H-182’s optional-source extraction remains a separate proposal. H-263 preserves borrowed views while the new local owner remains alive and permits access; ending that scope or sending the region out revokes access. H-264 selects the first typed let/make form. H-265 selects unchecked ultimate OOM and requests try/catch/finally planning. D-17 fixes consuming call/result boundaries and exception control; K-414 records the completed bounded local lifecycle. General qualifiers, owner reassignment and transport remain separate.

H-177 permits ordinary owners, and H-179 requires an ordinary variable to relinquish ownership when it installs a region-backed dynamic resource. H-181 prohibits consuming ownership out of a region-valued symbolic binding. Such a binding cannot be moved into an empty state; permitted shadowing and H-180 captured replay preserve its existing owner. The prohibition also applies when an explicitly optional dynamic value contains a region: matching or accessing the present case must not become a hidden ownership extraction.

This is distinct from ordinary packet ownership. An ordinary owner may be consumed by its separate move/send/result protocol. A dynamic resource remains held for its owning scope, including while hidden by another value. Ending an actual owning scope releases its own resource; removing a perform shadow does not release either pre-existing resource.

**Implemented boundary:** `effect @b <- @a` and `effect @a <- @a` reject whole-region values already held by a dynamic binding. Captured replay uses its own non-consuming path and preserves both captured and shadowed owners. The compiler no longer clears source owner slots during effect installation.

The executable requires fresh region acquisition in an effect initializer and does not yet support ordinary lexical owners/parameters. Its lifetime checks still reject escaping local resources and unsupported distinct-region joins. General ordinary-owner construction, optional extraction and packet send/result paths await D-14; they are not inferred from the dynamic-resource implementation.

### Exceptional control and scoped effects

[Exception control](ExceptionControl.md) supplies H-267/H-269's accepted try/catch/finally, unchecked failures and retained explicit causes. Leaving an effect scope exceptionally must restore its preceding binding and release only resources owned by that exiting scope. Perform replay must preserve pre-existing captured and hidden owners. Neither catching an exception nor restoring an environment snapshot can resurrect a consumed ordinary owner. Functions retain their exclusion from dynamic bindings/procedure calls; explicit-memory failure propagation does not grant new effect access. K-414 supplies focused exceptional-path evidence, including captured/hidden resource preservation; it does not qualify wider closure or concurrency contracts.

### Binding availability and optional resource values

The declaration’s value type and the availability of its dynamic binding are separate facts. H-181 permits a direct region-reference declaration and an explicitly optional one:

```text
symbol @required :: ResRegion;
symbol @optional :: Maybe<SomeRegion>;
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

A symbolic read is `@var_name`; a symbolic write is always `effect @var_name <- rhs`. Plain assignment, compound assignment and increment syntax must not provide alternate symbolic writes. Reads and writes participate in expression evaluation; procedures may also return meaningful results. `void` explicitly indicates no meaningful result, rather than a missing required binding, an implicit nullable value or another allocation carrier.

H-172 makes `<-` the only separator in an effect binding; `effect @x = rhs` and bare `@x <- rhs` remain invalid. H-204/H-205 additionally develops an ordinary ownership/bits transfer operator with the same glyph. Prefix effect retains scoped installation/restoration and its typed procedure-binding case; ordinary bits stores persist, and ordinary owner moves require a separate admitted owning destination. Captured replay remains non-consuming. The prototype supports `<-` in effect bindings, fresh owning declarations and make root initializers; parameter conversion-route arrows retain their independent grammar. General transfer overloads, owner reassignment and reference-assignment migration remain unimplemented.

An `effect @x <- 3` expression evaluates to 3, the newly bound value, under H-111. This result can be consumed by another expression while the binding lasts until its enclosing scope exits. The current language direction needs no separate statement category. Scope-result selection, compound-expression evaluation order and final grammar remain to be specified at the prototype boundary; unsupported combinations must not inherit arbitrary host-language semantics. Binding lifetime follows the enclosing scope, not merely evaluation of the write expression.

H-148 specifies callable argument order: evaluate an argument exactly once and finish its selected automatic conversions before evaluating the next argument. A failed conversion prevents later argument effects. Functions, procedures and overloaded operator spellings share this rule; compile-time method/route selection does not execute those effects. The callable body begins after all arguments and conversions finish. [D-11](ScalarUnitsAndIndexTypes.md#argument-evaluation-and-automatic-conversions) records the conversion-specific scope. Other open expression conventions are not settled by this decision.

### Local value shadowing

H-149 permits a nested lexical scope to shadow an outer local value, while duplicate declarations within one scope are compilation errors. A `let` initializer is evaluated before the new binding becomes visible: `{ let n = n + 1; n }` reads an outer `n` and keeps the new binding inside the block. Leaving the scope restores lexical lookup of the outer value. Local names remain distinct from symbol variables written through `effect`.

Function parameters belong to the function body's scope. A loop's index and fold accumulator belong to its body scope; their names may shadow surrounding locals but may not be redeclared in that body. Nested blocks and conditional branches establish their own scopes. The existing conditional right operand of `&&`/`||` keeps its local declarations inside that operand and applies the same nested-scope rule. Scope-local name tracking is compiler metadata, with no runtime binding table, storage carrier or change to regional lifetime.

Acceptance covers an initializer reading the outer value, restoration after nested blocks/branches/loops, parameter and loop binding uniqueness, and rejection of repeated declarations in one scope before program output. H-149 resolves K-253's naming alternatives; global declaration, function/type namespaces, dynamic symbols and broader scope-result rules retain their existing boundaries.

### Lexical declaration keywords

H-150 rejects reserved expression names in lexical value declarations: function/procedure parameters, `let` declarations, loop indices and fold accumulators. A bare name that the expression parser treats as a literal or control/declaration form cannot introduce a local binding. For example, `function identity(true::bool)::bool { true }` is a compilation error rather than a function with an inaccessible parameter.

The bounded grammar reserves `true`, `false`, `null`, `alloc`, `for`, `fold`, `let`, `effect`, `perform`, `behave`, `if`, `procedure`, `function`, `global` and `symbol` in those declaration positions. Contextual names with ordinary value lookup, such as `in` and `else`, remain available. This aligns accepted declarations with their subsequent lexical lookup, serving C-37/C-41’s readable composition through the existing candidate P-86/P-97 value paths. It does not change H-149’s visibility/uniqueness rule or dynamic symbol identity.

Field selectors retain contextual identifier meaning: `record.true` and `setfield(record, true, value)` are valid. Symbol names, module resource aliases, callable names, module globals, a general reserved-word policy and escaped identifiers retain their separate boundaries; this local decision supplies no escaping syntax or wider namespace rule.

Acceptance checks inaccessible parameter/local/loop names before execution, including during dependency scanning, while preserving ordinary contextual names and field selectors. Rejection names the offending spelling and its source location. P-130 records the confirmed local constraint; the complete language contract remains pending-human.

### Primary module-form keywords

The implemented H-161 keyword set reserves `type`, `data`, `regional`, `unit`, `array`, `using`, `import`, `mixin`, `export` and `conversion` against ordinary declarations and aliases. These names cannot introduce lexical functions, values, types or selected UoM aliases. A declaration such as `function type(...)` is rejected before execution, independent of where it would be called. Field selectors such as `record.type` and symbolic names such as `@using` retain their contextual meaning. H-149 shadowing and usable nonreserved contextual names remain unchanged. Data is the concrete-representation declaration keyword; regional remains reserved although the legacy regional type form is rejected. C-41/P-97's readable expression composition is the existing candidate value rationale; this local keyword decision does not qualify the complete grammar.

### Operator and intrinsic overloads

H-153 reserves a set of keywords against ordinary-name declarations and uses parenthesized operation names to declare overloads. An intrinsic spelling such as `len` must not silently conceal an accepted ordinary function declaration. `(len)` is an explicit overload name, following H-131's operator naming direction. The [prototype reference](../tests/cod/language/README.md#intrinsic-and-procedural-operation-overloads) lists its reserved expression, intrinsic and built-in type names. H-150's usable contextual names and field selectors retain their grammar; parentheses do not turn arbitrary control forms into eager calls. The complete future grammar remains open.

Operation spelling is separate from callable kind. H-155 permits function and procedure overloads under one operation, distinguished by parameter types. Under H-197/H-198, a function overload may write explicitly supplied regional storage; an overload accessing dynamic resources must be a procedure. Selected procedure overloads remain forbidden within functions. Conversion formulas are functions without dynamic-context access; the keyword itself supplies no automatic read-only guarantee.

The bounded implementation selects methods by argument types across both kinds, then checks whether the selected callable is legal in the caller. In a function, selecting a procedure would be an error rather than silently selecting a different pure overload. Two equally applicable definitions remain ambiguous; callable kind is not a proposed tie-breaker. Argument effects and automatic conversions must still obey H-148's left-to-right order and execute once.

H-158 requires intrinsics to use their compiler-internal representation without conflating it with CoD's symbol mechanism. A declaration such as `procedure (store)(...)` is an intrinsic procedure overload; it does not introduce an `@store` identity, a vocabulary alias or a symbolic-variable binding. Ordinary symbol-named procedures retain H-103's rules. Parentheses preserve token categories: `(@len)` groups a symbolic read, while `(len)` names an intrinsic operation. A symbol token or quoted string cannot declare an intrinsic overload. The intrinsic method's callable kind governs its permitted body operations.

H-159 specifies current-binding invocation for an operation-selected procedure, like `behave`. Its body sees the caller's dynamic context. An enclosing ordinary `perform @p` still establishes its own captured context; an intrinsic procedure invoked within that body sees that context. Capture analysis must therefore follow demanded typed calls without requiring effects used only by unselected overload candidates. Ordinary procedure definitions still resolve directly referenced symbol identities at their definition point; an alias requires its introducing module encounter first. This identity check does not require the corresponding dynamic value to be bound yet. No runtime method registry or user heap follows from operation syntax.

The implementation uses the compile-time method table and exact-before-conversion selection. Broader method replacement, explicit `perform` syntax for intrinsic operations and extensions to body-mixin syntax are separate work; none is implied by assigning intrinsics CoD symbol identities. H-160 defers new design decisions to a consolidated review at the end of the current work period, while already confirmed implementation work continues.

H-162 selects a distinct assignment operation `(=)`. Explicit `function (=)(...)` and `procedure (=)(...)` definitions, `using "module" (=);` and `(=)(...)` calls follow the same type dispatch, callable-context and current-binding rules as other eager operation methods. `(store)` remains independent. H-163/H-164 define `a[i] = value` as `([]=)(a, i, value)`, a separate family receiving the original container/index/value types. The receiver and index are evaluated once without loading the destination element; argument conversions finish before the next argument. The RHS is a full expression, and the result is the selected method’s result. Explicit `([]=)` declarations, source selections and calls use the same method rules. A visible method supplies behavior; there is no implicit forwarding to `(store)` or `(=)`. Record-field assignment and field-selector typing remain separate work; no first-class lvalue object or pointer-to-pointer representation is introduced.

H-222–H-224 adds right association and the proposed signature `(=)(lhs::XX, rhs::const YY) -> const XX`. Its read-only result can feed another read-only RHS parameter; requiring source mutation permission is unnecessary. Intrinsic reference-rebinding permissions and qualifier lowering remain under discussion. The H-204/H-205 target distinguishes reference assignment from bits/ownership transfer. Existing `(=)`/`([]=)` validation describes the prototype's unconstrained typed operation bodies, not that new distinction. A proposed `([]<-)` family would retain container/index/value types for bits transfer into arrays; its name, result and permission contract remain pending. No overload may duplicate a region owner, drop const access restrictions, or create a cross-owner stored reference.

H-206/H-208 extends transfer selection with both owner/root types and actual owner-reference arguments. The result is a const destination view, including when a user function/procedure supplies the graph algorithm; H-207 prevents consuming that view in a second owning move. Methods can use explicit source/destination region-policy interfaces without ambient lookup. Abstract root matching must retain concrete type/owner identity and reject unauthorized object/owner pairings. The two-operand source expansion and method header/result grammar remain proposed in D-14; this does not silently extend the existing exact-before-UoM-conversion matcher.

H-209 fixes <- as right associative and requires a unique most-specific applicable method regardless of declaration order. H-210 gives declarations the subtype relation <:; type names denote abstractions and data names concrete product/sum definitions. The keyword split changes scan/type-registration rules as well as constructor and method checking; it is not a backward-compatible alias for the current product parser. Detailed subtype/interface grammar is tracked in D-07. Operator association does not alter the existing argument-evaluation or function/procedure permission rules.

Field selection and resource ownership retain their own boundaries. `setfield` currently has a compile-time field selector, not three ordinary value arguments; assignment must identify a valid regional storage location without inventing an extra pointer layer. Returning a region created inside an overload cannot bypass its existing scope lifetime. Symbol-variable rebinding still requires `effect @x <- value`; regional assignment does not authorize bare `@x = value`.

The value rationale is readable operation syntax with inspectable type, effect and environment dependencies: C-41/P-97 and C-50/P-121 support readable typed composition, while C-37/P-86 and C-36/P-85 preserve explicit-input versus dynamic-context boundaries and contextual interpretation. These candidate value paths do not qualify the complete contract. Acceptance must distinguish function and procedure overloads of one spelling, reject a selected procedure in a function call tree, preserve current versus captured values and argument order, and diagnose ordinary keyword declarations. Existing intrinsic behavior and contextual field names must remain covered. The [prototype reference](../tests/cod/language/README.md#intrinsic-and-procedural-operation-overloads) describes the implemented subset. The earlier effect-arrow checkpoint passed 147 source checks, five composition cases and the regional roundtrip/relocation harness, with its source/compiler hashes in the [effect-arrow record](../tests/cod/language/effect-arrow-validation.json). The [assignment record](../tests/cod/language/assignment-validation.json) retains the focused purity, nominal-index and reserved-alias checks at their recorded source revision. Numeric and gather records retain their own run scope. These are language/runtime checks, not SNN acceptance. General lvalues, record-field assignment, field-selector overloads and intrinsic body-mixin extensions remain deferred; passing this subset does not qualify the full design.

### Storage carriers, capture types and lifetime

User-program objects have two carriers: stack and regions. H-177 permits ordinary variables to own region handles; H-179 permits consuming installation into a dynamic resource owner. Neither copies a regional object onto the stack. Host-native heap allocation belongs to system code such as compiler/runtime infrastructure. It cannot serve as a hidden way to extend the lifetime of a user closure, captured object or environment. No user `malloc`/`free` lifetime, automatic heap box, user GC object or reference-counted heap capture is introduced.

| Captured value | Meaning and current boundary |
| --- | --- |
| Bits | Preserve the value at procedure definition. Later scalar bindings do not change it. |
| Whole-region effect resource | Preserve the selected resource value and its owner/lifetime dependency. Replay does not move ownership, copy the graph or freeze its contents. |
| Interior regional reference | The current executable permits it. H-173/H-175’s proposed exclusion remains an independent allowed-type question; H-180 does not settle that proposal by reaffirming value capture. |

Reinterpreting an address as an integer must not bypass carrier/lifetime checks. A native heap pointer, stack pointer or arbitrary handle is not implicitly an admitted capture. System-resolved addresses are execution artifacts, not independently owned user objects.

Mutation through a valid resource reference changes its referent. Restoring a symbol binding does not undo that mutation. A read-only view prevents mutation through that view but still requires valid lifetime. Capturing a regional object by C++ copy/move is forbidden.

An initial SNN step can use non-escaping stack values and region-backed effect resources. Captured whole-resource values and the proposed restriction on captured interior pointers are separate concerns. A regional pointer's owning-region association must remain valid; a saved offset cannot silently be interpreted relative to the caller's different `@region`. A procedure retained across frame reuse, region release or remapping needs a justified lifetime. No stack closure is automatically promoted to the native heap. The concrete procedure descriptor and compiler-generated stack temporaries remain representation questions. H-193 makes user-defined data objects regional. Scalar UoM identities retain scalar bits representations; they do not provide arbitrary record storage on the stack.

TLS is the system's mechanism for accessing current bindings, not a third user-object allocation carrier. Values/bindings realized there must respect stack-scope or regional ownership; saved values and environment storage cannot gain an unrequested lifetime through a runtime implementation detail. `@region` allocation is available to procedures, while functions use their declared inputs/outputs and legal stack temporaries.

#### Function region-state access

H-197/H-198 permits functions to read and edit explicitly supplied memory and, once their source contracts exist, create regions, return ownership and send through an explicit mailbox. Functions cannot obtain regions through dynamic-variable lookup or call procedures to bypass that restriction. A procedure can supply a valid explicit view to a function while retaining its dynamic owner; the function does not acquire or rebind that owner's symbol.

References preserve the actual input-region association through fields, nested calls and returned values. A function may return a view into a still-live caller region. A view into a local region released at function exit is rejected; returning an intrinsic whole-region owner would instead transfer cleanup responsibility. Regional objects are never copied onto the stack, and function return does not itself end an input region.

There is no automatic read-only snapshot spanning nested functions. An inner function's writes are observable by subsequent reads in its caller and through aliased parameters. Native lowering must preserve source sequencing and actual memory dependencies, without inferring referential transparency from the keyword. No copied snapshot, version counter or per-object ownership system is introduced. Caller-visible edits are not automatically rolled back when local storage is released.

The implemented subset permits store, setfield and mutating function overloads over explicit regional inputs. A bare region local, parameter or result borrows an already held resource and retains its owner provenance. `alloc arena T(...)` accepts that named lexical borrow, so a function can construct objects and arrays in a caller-supplied region without reading a dynamic symbol. Functions still reject symbolic reads/writes, perform/behave and all selected procedure calls. A borrow cannot be installed as a new owning effect or escape the region's acquiring scope. [K-411](DesignAuthorityRegistry.md#k-411) records typed roots, make, ordinary owned locals/calls/results and preserved dynamic captures. Scalar-slot borrowing, general const propagation and mailbox operations remain unimplemented. Concrete data uses regional allocation; implicit object-field borrowing and qualifier syntax remain separate migration work.

Acceptance uses repeated mutating calls, alias-sensitive reads, returned views, cross-region store rejection, transitive dynamic-access rejection and the CoD free-list functions. The existing record/cycle and capture-owner native checks preserve their lifetime/relocation role. Historical snapshot reports identify earlier source scopes; they do not restore a read-only guarantee. Future const-function syntax, reachable-data restrictions and external-effect rules require their own business-driven design.

Persistent code identity, captured regional values and transient native access paths remain separate. Existing C++ pointer layouts do not settle their language representation. A TLS offset or native procedure address is not a portable stored program reference.

### Accepted temporary self-reference limitation

H-146 retains the current field-relative pointer encoding. When the first pointer field has the same address as its enclosing object and is assigned that object, its displacement is zero and it reads back as null. This known issue is accepted for the current increment; no new marker or store diagnostic is introduced. The expected future resolution is the introduction of intrinsic object fields. Their contents, layout, representation and compatibility consequences remain to be designed; the current increment adds no such fields. Nonzero-distance references and cycles retain their existing behavior.

### Logical relationships between regions

H-156 identifies each region with one graph of directly linked objects. H-157 confines relationships between regions to logical business associations. A CoD physical pointer edge must stay inside its region. A business ID or key can describe an association with an entity in another region. Its resolution, identity and version policy, permissions and behavior under reload belong to the business protocol.

The runtime can resolve a native address for a region whose lifetime is established. Each language-level reference still belongs to one region, including when a procedure uses bindings or arguments from several regions. The Shilos C++ `global_ptr` facility belongs to the host storage layer. [Regional memory vision and implementation tracker](GraphGrainedMemoryManagement.md)

## Whole-program layout and specialization

The compilation input is the demanded program composition: package/module identities, module initialization, typed definitions and reachable caller/callee paths. Modules supply definitions and are not independently compiled. Foreign entries must establish a CoD entry context whose binding requirements can be checked; this does not choose an FFI ABI.

A proposed lowering sequence is:

1. Scan/load the complete entry dependency set; register allowed declarations and retain initializers, procedure bodies and executable top-level expressions without running their semantics. Resolve symbol identities and preserve module origins.
2. Compile execution semantics from the entry, incorporating dependency top-level execution in encounter order. Follow these initialization/caller paths and check required symbols, types and captured/current bindings for each call mode. Compute reusable summaries where useful.
3. Assign typed locations in the program's per-thread context, then lower known accesses to a base plus a constant offset.
4. Lower `effect` and `perform` to scoped save/install/restore operations; `behave` retains the current context.
5. Specialize/in-line known calls and promote bindings to ordinary values where their observations, addresses and foreign uses permit it. Retain the context representation where it remains observable.

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
| `effect`, `perform`, `behave` | Supply current execution context or restore definition-time values deliberately | Nested scopes, capture, mutation and restoration traces |
| Regional ownership plus bounded read/write views | Keep state relocatable and distinguish source-frame reads from destination writes | Borrow lifetime and mutation checks; no copy/move of regional objects |
| Typed arrays, loops and reductions | Execute incoming sparse gathers and frame computations | Bounds, reduction/numeric semantics and complete-frame publication |
| Native primitive boundary | Reuse runtime facilities and eventually compile numerical kernels | Same contract under reference and native interpretations |

The SNN owns the neuron equations, AP/rate interpretation, training rules and data layout choices. The language owns identity, callable/effect semantics, types, lifetimes and execution meaning. An application-specific neuron or slot class does not need to become a built-in language construct.

## Validation and unresolved contract scope

The [case records](../applications/steady-snn/research/cod-language-scope-cases.json) contain expected semantic traces and proposed checking cases. They remain specifications, with a link to selected native fixture evidence; the complete catalog has not been executed. They cover alias/version identity, scoped restoration, by-value capture, both invocation modes, transitive dependencies, function restrictions, thread/program isolation, scan-only registration, entry-driven dependency incorporation and the SNN's immutable-A gather.

Typed symbols, bits globals, explicit result checks, ordered argument conversion and known-call binding/lifetime checks have bounded native implementations described in the prototype reference. Remaining semantic work includes module cycles, compound implementation views, local value/call lookup, standalone method heads and the coherent owned/const/place/construction/transfer contract. Missing required dynamic bindings remain compilation errors. These boundaries affect observable behavior or valid programs and require design decisions; equivalent lowering layouts and inlining choices can instead be evaluated on actual programs.

[Racket's parameter reference](https://docs.racket-lang.org/reference/parameters.html) is a useful primary comparison for scoped bindings, restoration and thread-sensitive behavior. Its continuation/thread-cell model is not CoD's capture-by-value rule. No continuation system, general dynamic dictionary or scheduler follows from that comparison.

## First native prototype boundary

H-108 authorizes autonomous development toward a first prototype that compiles and runs on demand once the immediate review is resolved. H-109–H-111 settle that review's reference trigger, module-scope lifetime and effect-result questions. The first milestone exercises the confirmed language semantics rather than selecting final SNN dynamics.

The initial acceptance is a CoD source entry with dependency modules that is scanned, checked as a demanded whole program, lowered to a native executable using the existing build toolchain, and run with observable scalar results. It must demonstrate nominal symbols/aliases, lexical functions, dynamically named procedures, both reuse forms and call modes, module execution boundaries, scoped value capture/restoration and missing-binding/function-effect diagnostics before execution. A small exact-integer incoming-gather example ties the language path to the confirmed two-frame application architecture without asserting a neuron equation or learned behavior.

The implementation can reuse the current driver's compiler/link/run facilities; it must not merely pass the new source through as C++ or claim interpreter-only output as native compilation. Compiler scratch state belongs to system code; prototype user values and captures preserve the stated storage boundaries. Begin with scalar bits and non-escaping values. Cross-region references, persistent closures, general GADT/ADT syntax, unrestricted higher-order recursion, concurrency and final numerical/model choices remain outside this first acceptance milestone. Unsupported forms receive an explicit diagnostic rather than a silently chosen permanent semantics.

Source syntax and lowering remain reviewable implementation work under these boundaries. The complete language contract is not retroactively qualified by this milestone. The authority registry records the direct implementation request and the exact local decisions separately from unconfirmed full-language mechanisms.

## Runnable prototype evidence

The [native prototype reference](../tests/cod/language/README.md) describes the implemented typed/module/regional subset and its experimental syntax. The parser, demanded-call checker and lowering are in `cod/lang_syntax.hh` and `cod/lang_frontend.hh`; `cod/main.cc` provides `--run`, `--check`, `--emit-llvm` and `--emit-bc`. Checked CoD lowers directly to LLVM IR, which the existing toolchain compiles and links with small C++ system services. H-143 selects this sole lowering path. LLVM artifacts are derived execution representations, not the language definition or a persistent semantic graph.

[Recorded validation](../tests/cod/language/validation.json) identifies a full language checkpoint; the prototype reference identifies subsequent focused corrections separately. Each linked record retains its own timestamp, source hashes and execution scope. H-144 requires meaningful semantic coverage rather than a growing case count. It exercises captured/current calls, using/import combinations and identity checks, restoration, once-only module incorporation, named product/UoM methods, pure conversions, typed SoA arrays, regional records, alias-sensitive memory operations, returned references and mutation through nested calls. Compilation errors prevent preceding top-level effects from running. A multi-module A/B gather operates on real regional arrays without selecting a neural transition or learning rule.

Known-call specialization uses fixed typed binding slots and saved values, with owning-region information specialized along demanded paths. User regional records stay in regions and cannot copy/move. The [stress record](../tests/cod/language/stress-validation.json) includes 512 modules, 1,000 procedure specializations and a 96-module regional borrow chain. [Relocation evidence](../tests/cod/language/regional-validation.json) checks the emitted layouts after a whole-region byte relocation. [Kernel measurements](../tests/cod/language/gather-full-pool-validation.json) cover the existing J16,q10 arithmetic layout, separately from any SNN acceptance.

Historical evidence remains scoped to its source revision: [abstract/data checks](../tests/cod/language/abstract-validation.json), [regional representation](../tests/cod/language/data-validation.json), [lexical mixin](../tests/cod/language/lexical-mixin-validation.json), [module integration](../tests/cod/language/module-integration-validation.json), [associated methods](../tests/cod/language/associated-method-validation.json), [context/export checks](../tests/cod/language/module-context-export-validation.json) and [borrowed regions](../tests/cod/language/region-borrow-validation.json). These records establish neither general ownership/qualifier syntax nor complete module/type acceptance.

Source conventions and the full language contract remain reviewable. General recursion, dynamic higher-order dispatch, persistent closures, business relationship resolution across regions, package-version identity protocols, compound UoMs, general GADT construction and concurrency are outside this evidence. Cross-platform parity and a general reference interpreter are not claimed.

## Complex composition and initial lowering

H-139 requires deliberate multi-module, complex-dependency and deep-call validation. Acceptance includes diamond graphs, repeated selections, shared vocabulary and nominal-type aliases, capture restoration across long call chains, and source initialization order without scan-time execution. H-140 requires each module to be incorporated once; emitted initialization is part of the resulting program. H-166 keeps module top-level scopes alive until process exit; initialization return restores active lookup.

H-141 accepts procedure expansion/inlining for the initial WPO implementation while program size remains reasonable. LLVM artifact size, native size and compile/run cost must be measured on representative deep and branching graphs. Reusing machine-code fragments is a medium/long-term optimization rather than an immediate architecture requirement. WPO call-path checking remains mandatory whether the eventual native lowering inlines or shares code.

H-144/H-145 require keeping the implementation and validation lean. Coverage is assessed by whether module incorporation, identity, capture/restoration, dynamic-context exclusion, typed conversion, control flow and regional lifetime/relocation behave as specified. Named numeric boundaries and an independent SoA gather reference address the relevant arithmetic risks. Duplicate syntax variants and large sample counts do not establish additional semantic coverage. The prototype introduces no backend abstraction, custom optimizer or code-sharing layer merely to support this validation.
