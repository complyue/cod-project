# CoD symbols, dynamic scope and callable kinds

Design revision: 17 · Recorded: 2026-11-05 · Phase: language/application co-design.

## Design authority

- **Registry / scope:** [D-10](DesignAuthorityRegistry.md#d-10), revision 17, all sections. The complete implementation contract is `pending-human`; the local propositions identified below are supplied directly by the user.
- **Value lineage:** C-35/P-84 connects strict symbol identity to unambiguous language composition; C-36/P-85 connects scoped execution bindings to context-dependent interpretation; C-37/P-86 connects pure computation and effectful execution to inspectable typed programs. D-10/P-90 derives from language design C-10. C-39/P-94 connects explicit storage to language semantics; C-41/P-97 connects expression results to typed composition; C-42/P-99 and C-43/P-100 connect vocabulary identities and definition reuse to composable semantics. C-44/P-103 connects declaration registration and entry-driven composition to explicit staged execution; P-104 records the human-specified phase/order constraint. C-45/P-106–P-108 tracks explicit origin/category selection; C-46/P-109–P-111 tracks compile-time body alias translation and the destination-binding boundary. These formal value paths remain proposals through the charter's C-01/C-02 goals, rather than qualification inherited from a dependency.
- **Human baseline:** [H-96](DesignAuthorityRegistry.md#h-96) supplies symbols, imports, thread scope, `effect`, `perform`, `behave`, WPO and joint development with the SNN. [H-97](DesignAuthorityRegistry.md#h-97) specifies capture by value of used effects and temporary installation for `perform`. [H-98](DesignAuthorityRegistry.md#h-98) restricts effects to procedures. [H-99](DesignAuthorityRegistry.md#h-99) limits user storage to stack and `@region`; [H-100](DesignAuthorityRegistry.md#h-100) limits captures to bits and regional pointers. [H-101](DesignAuthorityRegistry.md#h-101) reserves cross-region references for progressive design. [H-102](DesignAuthorityRegistry.md#h-102) requires compile-time binding checks on the demanded whole-program caller paths, without independent module compilation. [H-103](DesignAuthorityRegistry.md#h-103) defines functions lexically, procedures dynamically under symbol names, and module initialization as the top-level dynamic scope. [H-104](DesignAuthorityRegistry.md#h-104) specifies direct symbolic reads, effect-only writes, expression evaluation and explicit `void`. [H-105](DesignAuthorityRegistry.md#h-105) introduces symbol-only vocabulary modules and distinguishes source-definition `using` from current-context redefinition through `import`. [H-106](DesignAuthorityRegistry.md#h-106) restricts module-top-level definitions, separates scanning/registration from execution semantics, and specifies dependency incorporation in entry-driven encounter order. [H-107](DesignAuthorityRegistry.md#h-107) permits literal/function-call global initializers and top-level `effect`/`perform`/`behave`, with execution deferred to module execution. [H-108](DesignAuthorityRegistry.md#h-108) requests the design checkpoint and autonomous first runnable prototype after immediate choices are resolved. [H-109](DesignAuthorityRegistry.md#h-109) makes every kind of cross-module reference an execution trigger; [H-110](DesignAuthorityRegistry.md#h-110) ends the source module scope after its top-level execution and permits only `using`/`import` to bring its effective resources into the current module. [H-111](DesignAuthorityRegistry.md#h-111) makes `effect` return its newly bound value. [H-112](DesignAuthorityRegistry.md#h-112) requests explicit origins and local aliases; [H-113](DesignAuthorityRegistry.md#h-113) confirms that symbolic variables are exclusively dynamic and reject ordinary assignment. [H-114](DesignAuthorityRegistry.md#h-114) makes symbol identities and aliases compile-time entities. [H-115](DesignAuthorityRegistry.md#h-115) permits import-time alias maps with matching-name defaults. [H-116](DesignAuthorityRegistry.md#h-116) requires a preexisting destination symbol for procedure using/import with local renaming. [H-117](DesignAuthorityRegistry.md#h-117) proposes `symbol from` for vocabulary selection and a receiving-symbol-first `with` clause; [H-118](DesignAuthorityRegistry.md#h-118) authorizes implementing the discussed forms, testing, committing and pushing the bounded increment. [H-119](DesignAuthorityRegistry.md#h-119) proposes the combined `using symbol` shorthand. [H-120](DesignAuthorityRegistry.md#h-120) requires existing same-name symbols to share the source identity for abbreviated using; selecting a different identity requires explicit `as`, including `as @print` with unchanged spelling. [H-121](DesignAuthorityRegistry.md#h-121) authorizes implementing, validating, committing and pushing this refinement.
- **Regional-access baseline:** H-124 makes regions dynamic resources available to procedures. H-127 permits pure traversal of the complete input-reachable regional structure; H-129 defines one snapshot for the outermost function call and all nested functions, expiring at the outer return. H-133 confirms that calls occur within a procedure scope, including module top level, with inputs alive around the call; returning an existing input reference needs no additional snapshot-lifetime mechanism. C-48/P-114/P-115/P-117 record the scoped rationale and boundaries. H-146 accepts the first-field self-reference-to-null behavior as a temporary known limitation; future intrinsic fields are the expected resolution, with their layout still open.
- **Invariants / non-goals:** preserve nominal identities across aliases and package versions, thread isolation, scope restoration, lexical binding, callable kinds and regional lifetimes. User objects have no native-heap/malloc lifetime; only system code uses the host heap. Capture payloads contain only bits or regional pointers. Optimization preserves these rules. Scanning/loading never establishes procedure bindings or captures; execution semantics are compiled only after all entry-module dependencies have been scanned. Symbol bytes, complete typing, closure layout, late loading and persistence ABI are not selected.
- **Acceptance:** the semantic cases below and the independent [SNN validation plan](../applications/steady-snn/docs/LanguageCoDesign.md) must agree across reference and native execution. The bounded native evidence below covers typed values, modules, regional snapshots and measured arithmetic kernels; general interpreter agreement, concurrency and SNN behavior remain unvalidated.
- **Current assessment:** [K-247](DesignAuthorityRegistry.md#k-247) records revision 17 and H-148’s argument/conversion sequencing rule. [K-242](DesignAuthorityRegistry.md#k-242) records revision 16 and H-146’s accepted temporary pointer limitation. [K-231](DesignAuthorityRegistry.md#k-231) records revision 15 and the H-143 sole LLVM path with H-144/H-145 focused validation and simplification. [K-230](DesignAuthorityRegistry.md#k-230) records revision 14, updating implementation evidence without changing the scoped language rules. [K-224](DesignAuthorityRegistry.md#k-224) records revision 13: H-139 requires complex-module/deep-call acceptance, H-140 requires once-only incorporation in encounter order after scanning, and H-141 retains initial procedure inlining with measured size. [K-223](DesignAuthorityRegistry.md#k-223) records revision 12 and H-133 caller-scope lifetime clarification; K-220 records H-124/H-127/H-129 regional-access semantics. K-219 retains the passing implementation/validation of the H-119/H-120 shorthand and identity rule under H-121. Prototype reference 9 documents the implemented subset. The complete document and unrelated mapping edge policies retain their qualification limits.

## Semantic core supplied by the user

| Construct | Meaning |
| --- | --- |
| `symbol @tgt_neuron` | Introduce a first-class symbol with identity independent of its spelling. |
| Import a symbol, optionally under a local alias | Refer to the same exported symbol identity. An alias does not create another symbol. This is distinct from importing a procedure body. |
| Vocabulary module | Define symbols and explanatory comments, without procedure implementations or default effect values. |
| `using` an external procedure | Refer to the procedure defined in the source module's lexical/dynamic scopes, preserving that definition and its captures. |
| `import` an external procedure | Define its source body again in the current module's lexical/dynamic scopes; capture the effects used in that new definition context. |
| `@var_name` | Read the symbol variable directly; use declares the variable in the current thread's program context. |
| `effect @s = value` | The only symbolic-variable write form; establish the value for the current program scope and restore the preceding binding when that scope ends. It evaluates to the newly bound value. |
| Define a procedure value | Retain its definition's lexical environment and capture by value the symbolic effects it uses; capture payloads are bits or regional pointers. |
| `perform @p` | Resolve the procedure, temporarily install its captured effect values in its execution scope, execute its body, and restore the caller's bindings. |
| `behave @p` | Resolve and execute the procedure with its definition's lexical environment and the current dynamic bindings. Its stored dynamic captures are not installed. |
| Function | Lexically defined; compute from explicit inputs to outputs, with no symbolic effect access, `perform` or `behave`. |
| Procedure | Dynamically defined under a symbol name; may access effects, invoke procedures and call functions. |
| Module top level | Declare symbols, functions and bits globals initialized by literals or function calls; register procedure definitions. Top-level `effect`, `perform` and `behave` expressions are also permitted. Scanning runs none of their execution semantics. |
| Module execution composition | Scan/load every entry dependency first; then compile from the entry, incorporating dependency top-level execution semantics in encounter order. The compiled initialization supplies the module dynamic scope. |
| Required bindings | Checked on the effective caller paths during on-demand whole-program compilation; absence is a compilation error. |
| Expression / `void` | Evaluation yields a result; `void` explicitly indicates no meaningful returned value. The language direction uses expressions without a separate statement category. |

A procedure declaration/body can be registered before its definition event has execution semantics. Here capture at “definition” means the event that creates the procedure value in the resulting execution, not scanning the declaration. Compiling its code or binding an existing procedure value to another symbol with `effect` does not, by itself, recreate its captures. The exact syntax for forming procedure values remains open.

The supplied `effect` spelling operates within the enclosing scope. It need not introduce a separate brace construct. Repeated/nested bindings restore in the corresponding unwind order. The procedure value used by a call is resolved in the caller's current context **before** its captures are installed; otherwise changing the dispatch symbol could change which procedure is being called.

## Symbol identity, variable identity and compiled location

Keep three identities distinct:

1. **Symbol declaration identity:** identifies the exported declaration and its resolved package/module origin. Re-exports and local aliases preserve it. Distinct package versions or independently selected origins remain distinguishable even when every textual name agrees.
2. **Thread binding:** associates that symbol with its current value for this program execution in a thread. Different threads do not share the binding merely because they import the same symbol.
3. **Compiled location:** an offset or other specialized access path chosen for a particular compiled program layout. It is an execution artifact, not the symbol's persistent identity.

A candidate durable identity combines a resolved package-definition identity with a declaration identity. A human-readable version string alone does not distinguish every source/configuration origin; a raw address cannot survive reload. The concrete identity encoding and equivalence of repeated instantiations of the same package remain design choices. Imported aliases must already resolve to the same declaration before layout is assigned.

Under H-114, “first-class” symbol values belong to compilation; aliases resolve before runtime accesses are lowered. The current prototype supports declared identities and their aliases, without runtime symbol objects or dynamically selected identities. Any future runtime symbol-selection or open-ended loading facility needs a separate contract and cannot be inferred from first-class compile-time values.

“Use declares” removes a separate variable-definition ceremony; it does not settle the type or initial value. Proposed initial checking unifies all uses of one symbol variable to one value type, inferring local uses and requiring enough information at module boundaries. Conflicting uses are errors. Required binding availability is checked on the demanded whole-program call paths; a missing required variable is a compilation error under H-102. No zero/default value, implicit `Any`, or last-writer-selected type is adopted here.

## Symbol origins, aliases and dynamic values

H-112 requests explicit origins and local aliases. H-113 excludes lexical symbolic variables and ordinary symbolic assignment. H-114 places symbol identities and module-local aliases at compile time; runtime values bind under symbol identities. H-115 permits an alias-mapping table when importing a procedure body, with matching local spellings needing no explicit entry. H-116 specifies `using/import "<modu>" @src_name as @local_name` and requires the destination symbol to exist beforehand. These human rules determine the relationships below. H-117 supplies `symbol from` and the receiving-symbol-first `with` direction. H-118 authorizes their bounded implementation. The prototype also implements the H-119/H-120 combined using and identity-checked abbreviation under H-121.

### Compilation and execution have different bindings

| Layer | Binding / value | Consequence |
| --- | --- | --- |
| Module resolution | Source selector resolves to a particular module/package origin | Same spelling from distinct selected origins need not denote the same symbol |
| Compile-time symbol value | `symbol @x` creates a nominal identity S | S has no default runtime value and is not a runtime string/name lookup |
| Compile-time alias environment | A module-local spelling such as `@target` denotes S | Aliases disappear from runtime access; multiple aliases of S denote the same dynamic variable |
| Runtime dynamic environment | S has a current runtime value in the thread's active binding context | Reads access that value; `effect` installs a scoped binding and restores its predecessor on exit |

Conceptually, compilation resolves `local spelling -> symbol identity S -> program-specific TLS location`. Runtime `@x` accesses the value at that location. WPO can lower a known identity directly to a slot or specialize away the access; this requires no runtime alias map, string interning or symbol dictionary. S's semantic identity is distinct from the physical offset chosen for one compiled program. No particular identity encoding is selected.

The alias environment is a compiler name-resolution relation, not a set of lexical symbolic variables. A module can call S `@x` and another module can call the same S `@target`; both access the same thread-local dynamic binding. `effect @x = 2` binds S and yields 2; `@x = 2` is invalid. Functions' ordinary lexical parameters/constants remain non-`@` values and do not gain permission to read effects.

In declaration/selection syntax, `@name` identifies which symbol is meant. In an ordinary expression, `@name` reads the runtime value bound under that symbol. Making identities compile-time values does not make those runtime reads compile-time constants. Any later syntax for passing symbol values to compile-time computations must preserve this distinction; no new runtime symbol-object type or user allocation carrier is implied.

### Identity introduction versus procedure installation

Vocabulary selection and procedure installation have different jobs:

```text
symbol @step;
symbol from "snn/vocabulary.cod" @tgt_neuron as @tgt;

using "models/lif.cod" @update as @step;
// Alternative definition route:
import "models/lif.cod" @update as @step;
```

`symbol @step` creates a local identity. The `symbol from ...` form introduces a local compile-time alias for an identity declared by the vocabulary; omitting `as` keeps its spelling. It does not retain an incidental runtime value bound to that symbol in the source module.

In the procedure forms, `@step` must already resolve to a symbol identity T, either from `symbol @step` or a vocabulary selection. The source module's `@update` selects the source procedure resource under some identity S. The operation installs the resulting procedure value under T. **S and T may be different identities.** `as @step` does not rename S, make S equal T, or create the missing destination symbol. Ordinary `using` without `as` requires an existing same-spelling destination with the source identity under H-120. Explicit `as` selects an existing destination even when its identity differs and its spelling is unchanged. The combined `using symbol` form below can introduce the source identity when the local name is absent. The corresponding outer `import` shorthand policy is not settled by this using-specific clarification.

| Form | Compile-time action | Resulting execution |
| --- | --- | --- |
| `symbol @local` | Introduce a fresh nominal identity and its local name | No initial runtime binding |
| `symbol from "vocab" @src as @local` | Make `@local` denote the source identity | Source execution still follows H-109; no selected runtime value is retained |
| `using "model" @src as @local` for a procedure | Resolve the source resource and existing destination identity | Install the source-defined procedure value under the destination identity, retaining its lexical context and captured values |
| `import "model" @src as @local` for a procedure | Resolve destination and reinstantiate the body with receiving lexical names and symbol-alias translation | Create the receiving-context procedure value/captures and install it under the destination identity |
| `effect @local = value` | Resolve the local name to its identity and check the value | Evaluate the value, establish the dynamic binding, return that value and restore the old binding on scope exit |

Procedure declaration and using/import are explicit creation/installation forms; they are not exceptions permitting ordinary `@local = value` assignment. A source's declaration origin and the module providing a procedure can differ: a vocabulary may declare the symbol that several models bind. The destination identity is a further independent choice. Tooling should distinguish all three and retain source provenance for diagnostics.

Two aliases of one identity cannot hold two independent procedure values simultaneously. If `@a` and `@b` both denote S, installing a procedure under `@b` changes the binding read through `@a` in that scope. Two independently usable model operations need two destination identities, which H-116's prior-declaration rule makes explicit.

### Combined using and identity-checked abbreviation

`using symbol "std:io" @print;` combines selecting the source identity with installing its source-defined procedure. H-120 permits an existing same-name local alias when it already denotes that identity. It rejects an accidental collision with an independent identity. This comparison uses resolved nominal identity, including aliases through a common vocabulary; matching spelling, type, procedure value or body is insufficient.

Let S be the identity selected by the source module and T the identity already bound to the receiving name:

| Form | Receiving name absent | T = S | T differs from S |
| --- | --- | --- | --- |
| `using symbol "m" @p;` | Introduce the local alias for S and install the procedure | Reuse the identity and install the procedure | Compilation error; require explicit destination selection |
| `using "m" @p;` | Error: destination is not declared | Install the procedure | Compilation error; require explicit `as` |
| `using "m" @p as @p;` | Error: destination is not declared | Install the procedure | Install the procedure under T, preserving S and T as distinct identities |

For example, when both modules select `@print` from the same vocabulary, the receiving module can write `using "std:io" @print;`. When it independently declares `symbol @print;`, it must write:

```text
symbol @print;
using "std:io" @print as @print;
```

Here `as` explicitly selects the destination; it need not change the printed name. It does not replace the receiving compile-time identity or merge it with the source identity. Usual type and required-binding checks still apply. The combined form likewise does not overwrite an existing alias's identity; adapting to a different destination uses ordinary explicit `using`.

The source-identity-preserving renamed combined form, `using symbol "m" @p as @out;`, follows the same rule: it introduces a name for S, with same-identity reuse consistent with H-120; a different existing identity requires ordinary `using "m" @p as @out;`. This generalization is not a new identity-changing declaration form.

The combined form represents one cross-module encounter: execute the source, close its scope, and retain the selected procedure under the receiving identity. Its two conceptual tasks must not be expanded into two source executions. Reusing a compile-time identity does not skip procedure installation, and H-140 makes incorporation once per module; a repeated selection reuses its incorporated resources. Definition captures, `perform`/`behave`, dynamic scope restoration and the scan-before-execution boundary remain unchanged.

This rule concerns the outer using destination. H-115's implicit mappings inside an imported body remain receiving-name resolution, including independently declared identities; the outer identity guard does not silently change those mappings. Extending the outer guard to `import`, or adding `import symbol`, remains separate design scope.

Acceptance for this refinement: a missing local name is introduced by the combined form; a shared vocabulary identity is accepted with or without `symbol`; a same-name distinct identity is rejected without `as`; explicit self-`as` installs under the existing destination without merging identities; renaming does not manufacture identity; and the combined form retains one source execution and the existing capture/restoration behavior. These cases are covered by the [native fixture results](../tests/cod/language/validation.json). The parser preserves whether `as` appeared, because normalizing an omitted alias to its source spelling would erase a semantic distinction. Mismatch diagnostics identify both resolved origins and show the explicit destination form.

### Import-time alias translation

H-115 adds a compile-time correspondence between source-body aliases and receiving-module symbols. Following H-117/H-118, the implemented spelling presents the supplied local symbol first and the source-body role second:

```text
symbol @step;
symbol from "application-vocabulary.cod" @cell as @target;
symbol from "model-vocabulary.cod" @gain;

effect @target = 2;
effect @gain = 3;

import "model.cod" @update as @step
    with { @target as @tgt_neuron; };
```

Read `@target as @tgt_neuron` as “use my `@target` symbol as the imported body’s `@tgt_neuron`.” The left name resolves in the receiving module; the right name identifies a source-body alias to be satisfied. The clause does not introduce a receiving-module alias called `@tgt_neuron` or read either symbol’s runtime value.

Conceptually, `resolve_imported(@tgt_neuron) = resolve_local(@target)`. An implementation can index the correspondence by the right-hand source alias even though the written clause starts with the supplied local symbol. Resolve this correspondence before checking body requirements, computing captures and lowering runtime accesses. If the body also uses `@gain` and the receiver supplies that spelling, the implicit correspondence is `@gain as @gain`; no explicit entry is necessary.

The consistent reading of `A as B` is “use A in the role/name B.” For an outer `using/import`, A is a supplied source procedure and B is its receiving destination. Inside `with`, A is a supplied receiving symbol and B is the source-body role. The scopes differ explicitly; `as` need not mean a universal source-module-to-receiving-module arrow. The table is compile-time input under H-115; its supplier/role direction is implemented under H-118.

The coherent interpretation with H-105/H-115 is that an omitted entry uses the same receiving spelling. A same-spelling local alias may denote a different identity from the source alias: redefinition then uses the receiving identity; it does not equate the two original symbols. The prototype checks this case with independently declared source and receiving identities; spellings themselves do not determine identity. There is no fallback to a source binding when the intended receiving name or required value is unavailable; the missing requirement is diagnosed on the demanded whole-program path.

The mapping is compiler input and creates no runtime alias relation. `using` keeps its source body/identities and captures; `import` translates the body and obtains captures in the receiving dynamic context. `perform` installs whichever values that resulting procedure captured; `behave` reads current dynamic bindings under that procedure's resolved identities. Capture values can be computed at runtime even though the set of identities and access locations is determined during compilation.

The outer `as @step` chooses the procedure's destination binding. The inner `@target as @tgt_neuron` supplies an existing local symbol for a role in the imported body. Neither should be treated as an implicit text-wide rename. Translation must cover symbolic reads, effect targets and procedure-call symbols consistently. Source initialization and other already-created procedures are not silently rewritten by this body's map; H-109's source execution and its own binding checks remain separate.

The prototype requires each explicit source role to occur in the selected body, rejects duplicate source-role entries, and diagnoses attempts to split aliases of one source identity across different receiving identities. Mapping multiple such aliases to one receiving identity works. These are bounded diagnostic policies; the complete language policy for broader mappings remains open. No general symbol-mapping framework or runtime registry is required.

### Syntax and practical use

The existing `using/import "module" @source as @local` spelling keeps the requested operation, origin, source resource and destination visible. `symbol @local` introduces a new identity; `symbol from "module" @source as @local` names an existing identity. This keeps compile-time symbol declarations together and reserves using/import for effective-resource selection. Neither declaration permits `@local = ...` assignment. A module-prefix abbreviation or grouped selection can be considered if repeated source paths become a practical burden; neither is needed to establish these relationships.

Keep source provenance at declarations/selections and short names in procedure bodies. A local alias rename accompanied by the corresponding explicit import-map update should preserve the selected identities and runtime behavior. Auto-matching source-body spellings is convenient, but a change to one side alone can change or break the correspondence; navigation and diagnostics should show the resolved identities, not just matching text. Ambiguous aliases should require explicit disambiguation instead of depending on selection order.

Symbol selection still triggers source execution under H-109, after complete scanning and during entry-driven composition. H-110 closes the source scope; an identity-only selection does not keep its values ambient. H-140 makes repeated encounters reuse the module incorporated at its first encounter. Grouped selection and direct qualified-expression syntax retain their separate syntax scope. A local alias read uses an already selected identity rather than performing another module selection.

Rust's `use path as name` illustrates local aliases for explicit source paths. Racket's `only-in`, `rename-in` and `prefix-in` illustrate selection and spelling controls. These are useful naming comparisons; they do not supply CoD's dynamic-variable, destination-binding, capture or body-import semantics. [Rust Reference](https://doc.rust-lang.org/stable/reference/items/use-declarations.html), [Racket Guide](https://docs.racket-lang.org/guide/module-require.html).

### Implementation boundary and acceptance

The native prototype resolves `symbol from` to the source identity without retaining an incidental runtime value. Ordinary symbolic `using`/`import` selects a procedure value and requires a destination declaration/selection before the encounter. `using symbol` can introduce a source-identity alias and selects the procedure in that single encounter. Ordinary using without `as`, and all combined using forms, require matching source/destination identities. Import instantiates its own body with receiving symbol identities before checking requirements and capturing values; source definitions remain unchanged. Reads, effect targets and both procedure-call forms use the same translation. Reimporting an instantiated body translates its current provider aliases into the next receiving context.

[Recorded checks](../tests/cod/language/validation.json) exercise identity-only selection/restoration, distinct procedure destinations, same-name defaults with distinct identities, explicit maps, mapped reads/writes/calls, transitive requirements, reimport and captured/current calls. Invalid destinations, map names, map direction, source resources and premature alias use are rejected before execution. A later binding can satisfy `behave`; it cannot manufacture a missing definition-time capture for `perform`.

Validation covers combined source selection, same-identity reuse through a vocabulary/re-export, same-name mismatch rejection, explicit self-`as` with distinct identities, alias-shared dynamic values, source execution and captured/current calls. Import-body translation and outer import destination rules retain their existing semantics; `import symbol` is unsupported. General regional representation, new SNN algorithms, module-cycle policy and runtime symbol dispatch remain outside this increment. Built-in procedures have no importable source body; they use `using`. The complete language contract is not inferred from these checks.

## Vocabulary modules and procedure reuse

The customary vocabulary module contains symbol declarations with comments explaining their intended meaning. It supplies shared nominal identities, rather than choosing a model implementation or installing default values. Different modules can bind the same vocabulary symbol deliberately; similarly spelled symbols from distinct declarations remain distinct. Comments aid understanding; they do not substitute for the compiler's type and required-binding checks.

External procedure reuse has two different definition semantics:

| Operation | Lexical definition context | Dynamic definition/capture context |
| --- | --- | --- |
| `using` | Source module | Source definition event; retain existing captures |
| `import` | Current module, reusing the source body | Current definition event; collect captures in that context |

`import` is not merely a local alias for the existing procedure value. The body must be checked as a definition in the receiving module, including its lexical references, symbol identities, types and reachable call paths. It need not imply textual file copying or duplicated machine code: equivalent definitions can share generated code while preserving their semantics. The bounded selector/map syntax is described above; explicit qualifications and private-declaration policies remain open.

This distinction is independent of the invocation mode:

| Definition route | `perform` | `behave` |
| --- | --- | --- |
| `using` | Source lexical context plus source captured effect values | Source lexical context plus current caller bindings |
| `import` | Receiving lexical context plus newly captured effect values | Receiving lexical context plus current caller bindings |

For example, a body's unqualified lexical helper can compute 4 in its source module and 9 in the receiving module. With the same vocabulary symbol `@x`, source capture 1, receiving definition capture 7 and later caller binding 8, the conceptual `(helper_result, @x)` results are: using/perform `(4,1)`, using/behave `(4,8)`, import/perform `(9,7)`, import/behave `(9,8)`. This specifies the context distinction, not declaration grammar or a new pointer/capture representation.

H-116 requires the destination symbol to exist beforehand, either from a local declaration or an imported vocabulary identity. The outer `as @local` selects that destination for the resulting procedure value; it does not alias the source symbol identity. A compile-time alias alone never creates a fresh symbol identity. H-115 permits a separate source-body alias map during import. The compiler must retain enough source/semantic body information for redefinition; compiled machine code alone cannot supply the receiving lexical meaning. The entry-driven compilation phase incorporates module initialization semantics that determine these definition events and their required bindings. The earlier scan only registers declarations; neither `using` nor `import` makes that scan an effectful definition event.

## Functions and procedures

A function definition binds a lexical name. A procedure definition binds a symbol in a dynamic scope. The prototype therefore uses `function source_effect(...)` and `procedure @update_target(...)`; broader callable syntax remains open. A bare-name procedure declaration or call must not create a second lexical procedure mechanism. Importing its symbol preserves identity; it does not freeze the procedure currently bound to it. Procedure `using`/`import` follows the separate definition rules above.

This definition distinction coexists with H-96's definition-site lexical environment for a procedure body: lexical functions and constants referenced by the body retain their lexical meaning in both invocation modes. The procedure's own binding and its symbolic dependencies are dynamic. It does not require every local argument or temporary to become a symbol variable.

A function's result must be determined by its inputs under the declared numerical semantics. It can call functions and use lexical definitions and constants, but cannot read an ambient `@symbol`, rebind an effect, call a procedure through another spelling, obtain hidden randomness/time, or mutate externally visible state. An immutable lexical capture can be represented as part of a function value's fixed input; a mutable external capture cannot silently become an additional input.

Procedure signatures may have explicit arguments and results as well as effect requirements. “Procedure” does not imply a void return. Function-valued arguments require functions; passing a procedure there cannot bypass the effect restriction. Methods need the same callable-kind distinction.

The checker should retain each procedure's required symbol identities and read/write/binding use through calls. This is a proposed implementation of the user's distinction, not a commitment to a general row-polymorphic effect calculus. A small finite requirement set is sufficient for the first examples. Foreign calls need a trusted declaration of their effects; an opaque native call is not presumed pure.

Pure construction of a value describing a future effect is different from executing that effect. A function can construct permitted stack values or descriptions from its inputs; a procedure or handler performs regional materialization and execution. Allocating a regional GADT through `@region` is itself effectful under this boundary. Forming a procedure value that captures current effects also reads that context and requires an effectful creation site. No hidden effect access may enter a function through closure construction.

For the initial simulator example, functions return computed values; procedures write the next-frame arrays and advance runtime state. A future treatment of region allocation or uniqueness-based in-place realization of pure outputs must preserve the same observable boundary. The first slice does not need to solve that entire language problem.

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

For a scalar example, evaluate `effect @x = 1`, define `@p` whose body reads `@x`, then evaluate `effect @x = 2` in the caller scope:

| Observation | Result |
| --- | --- |
| `perform @p` | 1 |
| `@x` after that call | 2 |
| `behave @p` | 2 |

If the body evaluates `effect @x = @x + 1` and then reads `@x`, its local binding is restored on exit in **both** call modes. With capture 1 and caller value 7, two `perform @p` calls each observe 2; two `behave @p` calls each observe 8. The caller remains 7 after every call. `behave` omits capture installation; it does not make the body's own scoped writes permanent. Installing a captured scalar does not mutate the stored capture itself.

Scope exit includes the selected language's structured returns and failure/unwind paths. Lowering must arrange restoration on each such path. It does not promise to run cleanup after process termination, or roll back effects performed through referenced objects.

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

H-106 separates registration of the complete dependency set from compilation of execution semantics. The module top level admits `symbol`, `function` and **bits global-variable definitions whose initial values are bits literals or function calls**. It also permits `effect @name = rhs`, `perform` and `behave` expressions under H-107. A top-level `procedure @proc_name {}` registers its definition body during scanning. Global initializers and executable top-level expressions belong to module execution; the scanner records them without invoking functions/procedures, installing effects or capturing runtime values.

| Phase | Required work | Meaning of a top-level procedure definition |
| --- | --- | --- |
| 1. Scan/load | Scan the entry and all its dependency modules, registering the permitted definitions and retaining initializers, executable expressions, procedure bodies and their origins. Finish this phase before compiling entry execution semantics. | Available to resolve and compile; no procedure invocation, dynamic binding installation or value capture occurs merely from scanning it. |
| 2. Compile execution | Start from the entry module's execution semantics. As this compilation encounters dependency modules, incorporate their top-level execution semantics into the resulting program in that encounter order. | Its definition/binding semantics join global initialization and top-level effect/call expressions in the resulting program's module execution scope, with the appropriate `using`/`import` definition context. |

For a non-recursive example, suppose scanning loads dependencies A, B and C in that order, but compiling the entry encounters B and then A. The dependency execution-semantics incorporation order is **B, A**, independent of the earlier scan order. C's scan alone does not cause its execution semantics to be incorporated. Any cross-module reference qualifies as a trigger under H-109, including a vocabulary symbol, `using`, `import`, or a function/global reference. H-140 incorporates each module only once. These are compilation encounters after scanning, not immediate execution of user code by the compiler. A diamond dependency reuses the incorporated module; its initialization effects are not duplicated.

“Compile execution semantics” means constructing the resulting program, not running user initialization effects in the compiler. Capture values follow the definition event in that program's dynamic execution; they are not snapshots of the scanner's state. Required bindings are still checked at compile time over the resulting caller/initialization paths. Declaration availability therefore does not prove that a required dynamic value is available.

The module's top-level dynamic scope is its initialization procedure. That source scope ends after its top-level execution completes, restoring its scoped effects. It does not enclose the rest of the entry program. Only `using` and `import` bring the source module's effective resources into the current module under H-110.

`using` must retain the selected source-defined procedure and its captured values after the source scope ends; it does not retain the entire active source scope. `import` establishes the selected body as a definition in the receiving lexical/dynamic context. Neither route keeps every source `effect` binding active in the receiver. A vocabulary-symbol reference also triggers source execution, but knowing that symbol's identity does not itself transfer the source's bound value.

For example, a caller has `@x = 7`; a source module temporarily binds `@x = 1` and defines a procedure reading it. After the source module completes, the caller again sees 7. A procedure brought in through `using` observes 1 under `perform` and 7 under `behave`, provided the latter binding is available. An imported redefinition captures the receiving context instead. Retained captures must be bits or valid regional references; a destroyed source stack address cannot stand in for a retained value.

Any kind of cross-module reference encountered during execution compilation triggers incorporation of that module's execution semantics, including `import`; definition reuse does not exempt source execution. H-140 requires once-only incorporation in first-encounter order after scanning. Repeated encounters reuse the selected resources and do not produce another initialization body or recapture under a different caller. Cyclic initialization and branch handling remain separate questions; the prototype diagnoses a demanded initialization cycle. This limit does not select permanent cyclic-initialization semantics.

For a bits global, a bits literal and a function call producing the required bits value are permitted initializer forms. A procedure invocation is not thereby a function-call initializer; top-level `perform`/`behave` remains a separate allowed expression form. Function initializers retain the pure function contract, even though their evaluation belongs to module execution. This permits computed model constants without introducing scan-time user execution.

Top-level `effect` establishes bindings in the module's execution scope, while `perform` and `behave` follow their ordinary capture/current-binding semantics there. Compiling these expressions places their behavior into the resulting program; it does not run them during dependency discovery. The registration/execution distinction applies to expressions as well as procedure declarations.

The exact literal grammar, global mutability/sharing and physical realization remain to be specified alongside H-99's storage boundary and H-98's function purity. Permitting bits globals or effectful module execution does not by itself introduce native-heap user objects or ambient mutable global reads in functions.

### Expressions and `void`

A symbolic read is `@var_name`; a symbolic write is always `effect @var_name = rhs`. Plain assignment, compound assignment and increment syntax must not provide alternate symbolic writes. Reads and writes participate in expression evaluation; procedures may also return meaningful results. `void` explicitly indicates no meaningful result, rather than a missing required binding, an implicit nullable value or another allocation carrier.

An `effect @x = 3` expression evaluates to 3, the newly bound value, under H-111. This result can be consumed by another expression while the binding lasts until its enclosing scope exits. The current language direction needs no separate statement category. Scope-result selection, compound-expression evaluation order and final grammar remain to be specified at the prototype boundary; unsupported combinations must not inherit arbitrary host-language semantics. Binding lifetime follows the enclosing scope, not merely evaluation of the write expression.

H-148 specifies callable argument order: evaluate an argument exactly once and finish its selected automatic conversions before evaluating the next argument. A failed conversion prevents later argument effects. Functions, procedures and overloaded operator spellings share this rule; compile-time method/route selection does not execute those effects. The callable body begins after all arguments and conversions finish. [D-11](ScalarUnitsAndIndexTypes.md#argument-evaluation-and-automatic-conversions) records the conversion-specific scope. Other open expression conventions are not settled by this decision.

### Storage carriers, capture types and lifetime

User-program objects have two carriers: stack and `@region`. Host-native heap allocation belongs to system code such as compiler/runtime infrastructure. It cannot serve as a hidden way to extend the lifetime of a user closure, captured object or environment. No user `malloc`/`free` lifetime, automatic heap box, user GC object or reference-counted heap capture is introduced.

| Capture payload | Meaning |
| --- | --- |
| Bits | Preserve the value at procedure creation. Changes to the original scalar do not change that capture. |
| Regional pointer | Preserve a reference to an object in its valid regional context. This does not copy that object or freeze its later contents. |

A native heap pointer, stack pointer or arbitrary runtime handle is not a third capture kind. Reinterpreting an address as an integer must not provide a language-level escape from the carrier/lifetime rule. System-internal resolved addresses are execution artifacts, with no independently owned user-object lifetime.

Later mutation through a captured regional pointer can change its referent. Restoring a symbol binding does not undo that mutation. A read-only view prevents mutation through that view, but its temporal validity must also be enforced. Capturing an entire regional object by C++ copy/move is not permitted.

An initial SNN step can use non-escaping stack values and same-region procedure/capture storage. A regional pointer's owning-region association must remain valid; a saved offset cannot silently be interpreted relative to the caller's different `@region`. A procedure retained across frame reuse, region release or remapping needs a justified lifetime. No stack closure is automatically promoted to the native heap. The concrete procedure descriptor and whether a particular stack aggregate is a bits value remain representation/type questions, rather than new capture categories.

TLS is the system's mechanism for accessing current bindings, not a third user-object allocation carrier. Values/bindings realized there must respect stack-scope or regional ownership; saved values and environment storage cannot gain an unrequested lifetime through a runtime implementation detail. `@region` allocation is available to procedures, while functions use their declared inputs/outputs and legal stack temporaries.

#### Function snapshot access

H-124 specifies `@region` as a dynamically scoped resource: procedures may operate on available regions explicitly or implicitly, subject to their actual bindings and regional lifetimes. Passing a region handle as an ordinary argument does not grant a function access to that resource. Functions receive values obtained by dereferencing regional objects; a regional object remains in its region rather than being copied onto the stack.

H-127 permits a function to read the complete structure reachable through those supplied values, including further pointer dereferences. This is read-only graph access within the pure context, not an ambient region lookup or a procedure call. H-129 specifies its lifetime: the outermost function call establishes one read-only snapshot of input-reachable external objects, every nested function shares it, and the snapshot expires when the outer call returns. A procedure may call a function; no function may call a procedure, directly or transitively.

In the single-threaded model, the procedure is suspended while its function call tree executes, and that tree cannot perform effects that mutate the region. The semantic snapshot therefore need not allocate a copy, freeze the graph or introduce version counters, locks or a runtime snapshot object. Such mechanisms are not selected. Object sharing remains sharing, and legal intra-region references retain their relative representation. Read access neither copies a self-relative pointer field to a different storage base nor grants allocation, mutation or dynamic binding access.

H-133 places function calls within a procedure scope, including module top level, with their inputs alive before and after the call. A function may return a reference to an existing object obtained from those inputs; the caller continues to use it under the same ordinary regional lifetime. The snapshot ends on the outer return as H-129 requires, without destroying the object or the reference. No copied snapshot, new permission token or additional lifetime mechanism is needed for this case. Cross-region representation remains separately open under C-40.

A subsequent procedure write can change the value observed by a later function invocation at the same native address. The semantic input includes the supplied snapshot's contents, not just pointer bits. Native optimization must respect intervening writes and aliasing; purity does not authorize memoization or common-subexpression reuse across those writes using only address equality. No per-object version field follows from this requirement.

Acceptance must include traversal of pointer-bearing input records, nested functions sharing the same contents, rejection of direct/transitive procedure calls or writes from functions, and a procedure mutation between two outer calls producing the appropriately changed observation. It must include an existing input reference returned to the calling procedure and distinguish snapshot expiry from destruction of its referent. Concurrent or external writers require their own coherence contract; the single-threaded claim does not supply one.

Persistent code identity, captured regional values and transient native access paths remain separate. Existing C++ pointer layouts do not settle their language representation. A TLS offset or native procedure address is not a portable stored program reference.

### Accepted temporary self-reference limitation

H-146 retains the current field-relative pointer encoding. When the first pointer field has the same address as its enclosing object and is assigned that object, its displacement is zero and it reads back as null. This known issue is accepted for the current increment; no new marker or store diagnostic is introduced. The expected future resolution is the introduction of intrinsic object fields. Their contents, layout, representation and compatibility consequences remain to be designed; the current increment adds no such fields. Nonzero-distance references and cycles retain their existing behavior.

### Cross-region reference placeholder

H-101 explicitly keeps cross-region references in the design space. C-40 names the unresolved concept: a reference whose target belongs to a different independently managed region. Its identity, region/version resolution, lifetime, permissions and behavior under reload need a later contract.

The first same-region capture slice does not adopt `global_ptr` as an additional capture kind or encode a foreign address inside bits. A later design must explain how cross-region access relates to the two permitted capture forms, including a procedure stored in one region and invoked while another is current. Existing C++ `global_ptr` is implementation evidence, not an automatically selected language representation. This placeholder supplies no field layout, reference-counting scheme, region registry or per-neuron metadata.

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

## Initial language features and their purpose

| Feature | Immediate purpose | Initial boundary to validate |
| --- | --- | --- |
| Two-phase module processing | Make declarations available before composing demanded execution | No scan-time captures/effects; dependency execution incorporation follows entry compilation encounters |
| Nominal modules and vocabulary symbols | Compose model modules and multiple versions without binding collisions | Alias identity and distinct-origin cases |
| Procedure `using` versus `import` | Reuse a source definition or instantiate its body in a new lexical/dynamic context | Four combinations with `perform`/`behave`, checked against their actual binding paths |
| Scalar types, products, sums and exhaustive matching | Describe parameters, state alternatives and explicit outcomes | Type-correct construction and rejected invalid cases |
| A small checked GADT program core | Keep input/output/effect relationships in inspectable program data | Correct type indexes through pure and effectful operations; no full theorem solver required |
| Functions versus procedures | Separate lexical pure computation from dynamically bound operations | Symbol-only procedure names and transitive effect checking |
| Expressions and `void` | Compose value-producing and effectful computation consistently | Explicit scope results, sequencing, effect-only writes and no meaningful-result use of `void` |
| `effect`, `perform`, `behave` | Supply current execution context or restore definition-time values deliberately | Nested scopes, capture, mutation and restoration traces |
| Regional ownership plus bounded read/write views | Keep state relocatable and distinguish source-frame reads from destination writes | Borrow lifetime and mutation checks; no copy/move of regional objects |
| Typed arrays, loops and reductions | Execute incoming sparse gathers and frame computations | Bounds, reduction/numeric semantics and complete-frame publication |
| Native primitive boundary | Reuse runtime facilities and eventually compile numerical kernels | Same contract under reference and native interpretations |

The SNN owns the neuron equations, AP/rate interpretation, training rules and data layout choices. The language owns identity, callable/effect semantics, types, lifetimes and execution meaning. An application-specific neuron or slot class does not need to become a built-in language construct.

## Validation and unresolved contract scope

The [case records](../applications/steady-snn/research/cod-language-scope-cases.json) contain expected semantic traces and proposed checking cases. They remain specifications, with a link to selected native fixture evidence; the complete catalog has not been executed. They cover alias/version identity, scoped restoration, by-value capture, both invocation modes, transitive dependencies, function restrictions, thread/program isolation, scan-only registration, entry-driven dependency incorporation and the SNN's immutable-A gather.

The immediate design work is to specify symbol-value typing, the remaining cycle rules for module composition, bits-global realization, scope results/evaluation order, the call-path checker, capture lifetimes and the first checked program representation. Missing required bindings are already specified as compilation errors. These affect observable semantics or valid programs; they cannot be replaced by a benchmark choice. In contrast, equivalent context layouts, inlining thresholds and numeric-format implementations can be compared using actual programs once their semantics are fixed.

[Racket's parameter reference](https://docs.racket-lang.org/reference/parameters.html) is a useful primary comparison for scoped bindings, restoration and thread-sensitive behavior. Its continuation/thread-cell model is not CoD's capture-by-value rule. No continuation system, general dynamic dictionary or scheduler follows from that comparison.

## First native prototype boundary

H-108 authorizes autonomous development toward a first prototype that compiles and runs on demand once the immediate review is resolved. H-109–H-111 settle that review's reference trigger, module-scope lifetime and effect-result questions. The first milestone exercises the confirmed language semantics rather than selecting final SNN dynamics.

The initial acceptance is a CoD source entry with dependency modules that is scanned, checked as a demanded whole program, lowered to a native executable using the existing build toolchain, and run with observable scalar results. It must demonstrate nominal symbols/aliases, lexical functions, dynamically named procedures, both reuse forms and call modes, module execution boundaries, scoped value capture/restoration and missing-binding/function-effect diagnostics before execution. A small exact-integer incoming-gather example ties the language path to the confirmed two-frame application architecture without asserting a neuron equation or learned behavior.

The implementation can reuse the current driver's compiler/link/run facilities; it must not merely pass the new source through as C++ or claim interpreter-only output as native compilation. Compiler scratch state belongs to system code; prototype user values and captures preserve the stated storage boundaries. Begin with scalar bits and non-escaping values. Cross-region references, persistent closures, general GADT/ADT syntax, unrestricted higher-order recursion, concurrency and final numerical/model choices remain outside this first acceptance milestone. Unsupported forms receive an explicit diagnostic rather than a silently chosen permanent semantics.

Source syntax and lowering remain reviewable implementation work under these boundaries. The complete language contract is not retroactively qualified by this milestone. The authority registry records the direct implementation request and the exact local decisions separately from unconfirmed full-language mechanisms.

## Runnable prototype evidence

The [native prototype reference](../tests/cod/language/README.md) describes the implemented typed/module/regional subset and its experimental syntax. The parser, demanded-call checker and lowering are in `cod/lang_syntax.hh` and `cod/lang_frontend.hh`; `cod/main.cc` provides `--run`, `--check`, `--emit-llvm` and `--emit-bc`. Checked CoD lowers directly to LLVM IR, which the existing toolchain compiles and links with small C++ system services. H-143 selects this sole lowering path. LLVM artifacts are derived execution representations, not the language definition or a persistent semantic graph.

[Recorded validation](../tests/cod/language/validation.json) covers the contract-focused source corpus and the full CoD component suite. H-144 requires meaningful semantic coverage rather than a growing case count. It exercises captured/current calls, using/import combinations and identity checks, restoration, once-only module incorporation, named product/UoM methods, pure conversions, typed SoA arrays, regional records, input-reachable snapshots, returned references and mutation between calls. Compilation errors prevent preceding top-level effects from running. A multi-module A/B gather operates on real regional arrays without selecting a neural transition or learning rule.

Known-call specialization uses fixed typed binding slots and saved values, with owning-region information specialized along demanded paths. User regional records stay in regions and cannot copy/move. The [stress record](../tests/cod/language/stress-validation.json) includes 512 modules, 1,000 procedure specializations and a 96-module regional snapshot chain. [Relocation evidence](../tests/cod/language/regional-validation.json) checks the emitted layouts after a whole-region byte relocation. [Kernel measurements](../tests/cod/language/gather-full-pool-validation.json) cover the existing J16,q10 arithmetic layout, separately from any SNN acceptance.

Source conventions and the full language contract remain reviewable. General recursion, dynamic higher-order dispatch, persistent closures, cross-region representation, package-version identity protocols, compound UoMs, general GADT construction and concurrency are outside this evidence. Cross-platform parity and a general reference interpreter are not claimed.

## Complex composition and initial lowering

H-139 requires deliberate multi-module, complex-dependency and deep-call validation. Acceptance includes diamond graphs, repeated selections, shared vocabulary and nominal-type aliases, capture restoration across long call chains, and source initialization order without scan-time execution. H-140 requires each module to be incorporated once; emitted initialization is part of the resulting program. The source scope still closes under H-110.

H-141 accepts procedure expansion/inlining for the initial WPO implementation while program size remains reasonable. LLVM artifact size, native size and compile/run cost must be measured on representative deep and branching graphs. Reusing machine-code fragments is a medium/long-term optimization rather than an immediate architecture requirement. WPO call-path checking remains mandatory whether the eventual native lowering inlines or shares code.

H-144/H-145 require keeping the implementation and validation lean. Coverage is assessed by whether module incorporation, identity, capture/restoration, purity, typed conversion, control flow and regional lifetime/relocation behave as specified. Named numeric boundaries and an independent SoA gather reference address the relevant arithmetic risks. Duplicate syntax variants and large sample counts do not establish additional semantic coverage. The prototype introduces no backend abstraction, custom optimizer or code-sharing layer merely to support this validation.
