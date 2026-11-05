# CoD symbols, dynamic scope and callable kinds

Design revision: 5 · Recorded: 2026-11-05 · Phase: language/application co-design.

## Design authority

- **Registry / scope:** [D-10](DesignAuthorityRegistry.md#d-10), revision 5, all sections. The complete implementation contract is `pending-human`; the local propositions identified below are supplied directly by the user.
- **Value lineage:** C-35/P-84 connects strict symbol identity to unambiguous language composition; C-36/P-85 connects scoped execution bindings to context-dependent interpretation; C-37/P-86 connects pure computation and effectful execution to inspectable typed programs. D-10/P-90 derives from language design C-10. C-39/P-94 connects explicit storage to language semantics; C-41/P-97 connects expression results to typed composition; C-42/P-99 and C-43/P-100 connect vocabulary identities and definition reuse to composable semantics. C-44/P-103 connects declaration registration and entry-driven composition to explicit staged execution; P-104 records the human-specified phase/order constraint. These formal value paths remain proposals through the charter's C-01/C-02 goals, rather than qualification inherited from a dependency.
- **Human baseline:** [H-96](DesignAuthorityRegistry.md#h-96) supplies symbols, imports, thread scope, `effect`, `perform`, `behave`, WPO and joint development with the SNN. [H-97](DesignAuthorityRegistry.md#h-97) specifies capture by value of used effects and temporary installation for `perform`. [H-98](DesignAuthorityRegistry.md#h-98) restricts effects to procedures. [H-99](DesignAuthorityRegistry.md#h-99) limits user storage to stack and `@region`; [H-100](DesignAuthorityRegistry.md#h-100) limits captures to bits and regional pointers. [H-101](DesignAuthorityRegistry.md#h-101) reserves cross-region references for progressive design. [H-102](DesignAuthorityRegistry.md#h-102) requires compile-time binding checks on the demanded whole-program caller paths, without independent module compilation. [H-103](DesignAuthorityRegistry.md#h-103) defines functions lexically, procedures dynamically under symbol names, and module initialization as the top-level dynamic scope. [H-104](DesignAuthorityRegistry.md#h-104) specifies direct symbolic reads, effect-only writes, expression evaluation and explicit `void`. [H-105](DesignAuthorityRegistry.md#h-105) introduces symbol-only vocabulary modules and distinguishes source-definition `using` from current-context redefinition through `import`. [H-106](DesignAuthorityRegistry.md#h-106) restricts module-top-level definitions, separates scanning/registration from execution semantics, and specifies dependency incorporation in entry-driven encounter order. [H-107](DesignAuthorityRegistry.md#h-107) permits literal/function-call global initializers and top-level `effect`/`perform`/`behave`, with execution deferred to module execution. [H-108](DesignAuthorityRegistry.md#h-108) requests the design checkpoint and autonomous first runnable prototype after immediate choices are resolved. [H-109](DesignAuthorityRegistry.md#h-109) makes every kind of cross-module reference an execution trigger; [H-110](DesignAuthorityRegistry.md#h-110) ends the source module scope after its top-level execution and permits only `using`/`import` to bring its effective resources into the current module. [H-111](DesignAuthorityRegistry.md#h-111) makes `effect` return its newly bound value.
- **Invariants / non-goals:** preserve nominal identities across aliases and package versions, thread isolation, scope restoration, lexical binding, callable kinds and regional lifetimes. User objects have no native-heap/malloc lifetime; only system code uses the host heap. Capture payloads contain only bits or regional pointers. Optimization preserves these rules. Scanning/loading never establishes procedure bindings or captures; execution semantics are compiled only after all entry-module dependencies have been scanned. Symbol bytes, complete typing, closure layout, late loading and persistence ABI are not selected.
- **Acceptance:** the semantic cases below and the independent [SNN validation plan](../applications/steady-snn/docs/LanguageCoDesign.md) must agree across reference and native execution. The bounded native prototype evidence below covers scalar/module behavior; full reference/native agreement and performance remain unvalidated.
- **Current assessment:** [K-214](DesignAuthorityRegistry.md#k-214) records revision 5 and the first native prototype evidence under H-108. It does not qualify the complete document.

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

“First-class” permits passing and storing a symbol value. It does not automatically require a runtime facility to create arbitrarily many new symbol identities. Statically declared symbols can still be selected dynamically; such a selection needs a typed indirect access or specialization over its possible identities. Only a statically resolved symbol access can be promised a compile-time constant offset. Runtime symbol creation and open-ended loading need a separate extension contract.

“Use declares” removes a separate variable-definition ceremony; it does not settle the type or initial value. Proposed initial checking unifies all uses of one symbol variable to one value type, inferring local uses and requiring enough information at module boundaries. Conflicting uses are errors. Required binding availability is checked on the demanded whole-program call paths; a missing required variable is a compilation error under H-102. No zero/default value, implicit `Any`, or last-writer-selected type is adopted here.

## Vocabulary modules and procedure reuse

The customary vocabulary module contains symbol declarations with comments explaining their intended meaning. It supplies shared nominal identities, rather than choosing a model implementation or installing default values. Different modules can bind the same vocabulary symbol deliberately; similarly spelled symbols from distinct declarations remain distinct. Comments aid understanding; they do not substitute for the compiler's type and required-binding checks.

External procedure reuse has two different definition semantics:

| Operation | Lexical definition context | Dynamic definition/capture context |
| --- | --- | --- |
| `using` | Source module | Source definition event; retain existing captures |
| `import` | Current module, reusing the source body | Current definition event; collect captures in that context |

`import` is not merely a local alias for the existing procedure value. The body must be checked as a definition in the receiving module, including its lexical references, symbol identities, types and reachable call paths. It need not imply textual file copying or duplicated machine code: equivalent definitions can share generated code while preserving their semantics. Exact syntax and treatment of explicit qualifications/private declarations remain open.

This distinction is independent of the invocation mode:

| Definition route | `perform` | `behave` |
| --- | --- | --- |
| `using` | Source lexical context plus source captured effect values | Source lexical context plus current caller bindings |
| `import` | Receiving lexical context plus newly captured effect values | Receiving lexical context plus current caller bindings |

For example, a body's unqualified lexical helper can compute 4 in its source module and 9 in the receiving module. With the same vocabulary symbol `@x`, source capture 1, receiving definition capture 7 and later caller binding 8, the conceptual `(helper_result, @x)` results are: using/perform `(4,1)`, using/behave `(4,8)`, import/perform `(9,7)`, import/behave `(9,8)`. This specifies the context distinction, not declaration grammar or a new pointer/capture representation.

Importing the body does not by itself decide whether its destination symbol is an existing vocabulary identity or another explicitly declared symbol; that binding syntax remains open. A local alias alone never creates a fresh symbol identity. The compiler must retain enough source/semantic body information for redefinition; compiled machine code alone cannot supply the receiving lexical meaning. The entry-driven compilation phase incorporates module initialization semantics that determine these definition events and their required bindings. The earlier scan only registers declarations; neither `using` nor `import` makes that scan an effectful definition event.

## Functions and procedures

A function definition binds a lexical name. A procedure definition binds a symbol in a dynamic scope. Pseudocode therefore uses `function source_effect(...)` and `procedure @update_target(...)`; the declaration grammar remains to be designed. A bare-name procedure declaration or call must not create a second lexical procedure mechanism. Importing its symbol preserves identity; it does not freeze the procedure currently bound to it. Procedure `using`/`import` follows the separate definition rules above.

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

For a non-recursive example, suppose scanning loads dependencies A, B and C in that order, but compiling the entry encounters B and then A. The dependency execution-semantics incorporation order is **B, A**, independent of the earlier scan order. C's scan alone does not cause its execution semantics to be incorporated. Any cross-module reference qualifies as a trigger under H-109, including a vocabulary symbol, `using`, `import`, or a function/global reference. Repeated/cyclic encounter behavior is a separate open detail.

“Compile execution semantics” means constructing the resulting program, not running user initialization effects in the compiler. Capture values follow the definition event in that program's dynamic execution; they are not snapshots of the scanner's state. Required bindings are still checked at compile time over the resulting caller/initialization paths. Declaration availability therefore does not prove that a required dynamic value is available.

The module's top-level dynamic scope is its initialization procedure. That source scope ends after its top-level execution completes, restoring its scoped effects. It does not enclose the rest of the entry program. Only `using` and `import` bring the source module's effective resources into the current module under H-110.

`using` must retain the selected source-defined procedure and its captured values after the source scope ends; it does not retain the entire active source scope. `import` establishes the selected body as a definition in the receiving lexical/dynamic context. Neither route keeps every source `effect` binding active in the receiver. A vocabulary-symbol reference also triggers source execution, but knowing that symbol's identity does not itself transfer the source's bound value.

For example, a caller has `@x = 7`; a source module temporarily binds `@x = 1` and defines a procedure reading it. After the source module completes, the caller again sees 7. A procedure brought in through `using` observes 1 under `perform` and 7 under `behave`, provided the latter binding is available. An imported redefinition captures the receiving context instead. Retained captures must be bits or valid regional references; a destroyed source stack address cannot stand in for a retained value.

Any kind of cross-module reference encountered during execution compilation triggers incorporation of that module's execution semantics, including `import`; definition reuse does not exempt source execution. Precise repeated/cyclic encounter and branch handling remain to be specified. The first prototype can limit supported programs to an acyclic, unambiguous module-execution order and diagnose unsupported cases. That limit does not select a permanent repetition or cycle policy.

For a bits global, a bits literal and a function call producing the required bits value are permitted initializer forms. A procedure invocation is not thereby a function-call initializer; top-level `perform`/`behave` remains a separate allowed expression form. Function initializers retain the pure function contract, even though their evaluation belongs to module execution. This permits computed model constants without introducing scan-time user execution.

Top-level `effect` establishes bindings in the module's execution scope, while `perform` and `behave` follow their ordinary capture/current-binding semantics there. Compiling these expressions places their behavior into the resulting program; it does not run them during dependency discovery. The registration/execution distinction applies to expressions as well as procedure declarations.

The exact literal grammar, global mutability/sharing and physical realization remain to be specified alongside H-99's storage boundary and H-98's function purity. Permitting bits globals or effectful module execution does not by itself introduce native-heap user objects or ambient mutable global reads in functions.

### Expressions and `void`

A symbolic read is `@var_name`; a symbolic write is always `effect @var_name = rhs`. Plain assignment, compound assignment and increment syntax must not provide alternate symbolic writes. Reads and writes participate in expression evaluation; procedures may also return meaningful results. `void` explicitly indicates no meaningful result, rather than a missing required binding, an implicit nullable value or another allocation carrier.

An `effect @x = 3` expression evaluates to 3, the newly bound value, under H-111. This result can be consumed by another expression while the binding lasts until its enclosing scope exits. The current language direction needs no separate statement category. Scope-result selection, compound-expression evaluation order and final grammar remain to be specified at the prototype boundary; unsupported combinations must not inherit arbitrary host-language semantics. Binding lifetime follows the enclosing scope, not merely evaluation of the write expression.

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

Persistent code identity, captured regional values and transient native access paths remain separate. Existing C++ pointer layouts do not settle their language representation. A TLS offset or native procedure address is not a portable stored program reference.

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

The immediate design work is to specify symbol-value typing, the remaining repetition/cycle rules for module composition, bits-global realization, scope results/evaluation order, the call-path checker, capture lifetimes and the first checked program representation. Missing required bindings are already specified as compilation errors. These affect observable semantics or valid programs; they cannot be replaced by a benchmark choice. In contrast, equivalent context layouts, inlining thresholds and numeric-format implementations can be compared using actual programs once their semantics are fixed.

[Racket's parameter reference](https://docs.racket-lang.org/reference/parameters.html) is a useful primary comparison for scoped bindings, restoration and thread-sensitive behavior. Its continuation/thread-cell model is not CoD's capture-by-value rule. No continuation system, general dynamic dictionary or scheduler follows from that comparison.

## First native prototype boundary

H-108 authorizes autonomous development toward a first prototype that compiles and runs on demand once the immediate review is resolved. H-109–H-111 settle that review's reference trigger, module-scope lifetime and effect-result questions. The first milestone exercises the confirmed language semantics rather than selecting final SNN dynamics.

The initial acceptance is a CoD source entry with dependency modules that is scanned, checked as a demanded whole program, lowered to a native executable using the existing build toolchain, and run with observable scalar results. It must demonstrate nominal symbols/aliases, lexical functions, dynamically named procedures, both reuse forms and call modes, module execution boundaries, scoped value capture/restoration and missing-binding/function-effect diagnostics before execution. A small exact-integer incoming-gather example ties the language path to the confirmed two-frame application architecture without asserting a neuron equation or learned behavior.

The implementation can reuse the current driver's compiler/link/run facilities; it must not merely pass the new source through as C++ or claim interpreter-only output as native compilation. Compiler scratch state belongs to system code; prototype user values and captures preserve the stated storage boundaries. Begin with scalar bits and non-escaping values. Cross-region references, persistent closures, general GADT/ADT syntax, unrestricted higher-order recursion, concurrency and final numerical/model choices remain outside this first acceptance milestone. Unsupported forms receive an explicit diagnostic rather than a silently chosen permanent semantics.

Source syntax and lowering remain reviewable implementation work under these boundaries. The complete language contract is not retroactively qualified by this milestone. The authority registry records the direct implementation request and the exact local decisions separately from unconfirmed full-language mechanisms.

## Runnable prototype evidence

The [native prototype reference](../tests/cod/language/README.md) describes the implemented scalar/module subset and its experimental syntax. The parser, demanded-call checker and native lowering are in `cod/lang_syntax.hh` and `cod/lang_frontend.hh`; `cod/main.cc` provides `--run`, `--check` and `--emit-cpp` using the existing compiler/linker facilities. Source is parsed and checked as CoD before lowering. Native C++ is a derived execution artifact, not the language definition or a persistent semantic graph.

[Recorded validation](../tests/cod/language/validation.json) contains 22 direct checks, source/executable hashes, compiler/linker/runner process evidence and a successful existing C++ evaluation. The checks cover captured/current calls, all four using/import invocation combinations, scoped restoration, alias versus distinct-file identity, transitive requirements, module execution/resource transfer, missing-binding and invalid-function diagnostics, expression results and an exact-integer incoming gather. Compilation errors prevent preceding top-level effects from running. The gather stores both B values before publication and checks reverse traversal against the same immutable source values.

The implementation uses known-call specialization, fixed TLS scalar binding slots and stack-based saved bits. It accepts `i64`/`bool`/`void`, immutable globals and acyclic unambiguous local module composition. Last-expression block results, left-to-right evaluation and checked integer arithmetic are prototype conventions, not permanent language decisions. General module repetition, recursion, regional-pointer syntax/lifetimes, first-class runtime symbols, package-version identity, GADT/ADT construction, persistent closures and concurrency remain outside this evidence. The two-target arithmetic fixture is not a complete simulator or trained SNN. No reference interpreter, cross-platform result or performance comparison is claimed.
