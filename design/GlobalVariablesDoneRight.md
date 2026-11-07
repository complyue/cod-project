# CoD Dynamic Bindings Vision and Implementation Tracker

Design revision 1 · 2026-11-06

## Design authority

- **Registry and scope** [D-12](DesignAuthorityRegistry.md#d-12), revision 1, all sections. This is a vision and evidence tracker. The candidate value path and open designs are `pending-human`.
- **Value lineage** P-134 proposes that inspectable, typed execution context serves C-10’s explicit language semantics and the charter’s separation of program, execution instance and environment. P-135 depends on D-10 for binding, call and lifetime semantics. C-52/H-151 supplies the actor direction. [Charter](RegionalSemanticPlatformCharter.md) · [D-10](SymbolicDynamicScope.md)
- **Human baseline** H-96–H-104 and H-113/H-114 cover nominal symbols, scoped effects, callable kinds, capture and whole-program binding checks. H-127/H-129/H-133 cover regional snapshot access and caller lifetimes. H-151/H-152 cover the actor direction, the `Mainbox<WorkPkg>` example and region holding and transfer as a design priority. [H-96](DesignAuthorityRegistry.md#h-96) · [H-127](DesignAuthorityRegistry.md#h-127) · [H-151](DesignAuthorityRegistry.md#h-151) · [H-152](DesignAuthorityRegistry.md#h-152)
- **Invariants and acceptance** Preserve symbol identity, restoration, function purity and regional lifetimes. Use native traces and rejection cases to track the synchronous prototype. Treat the actor type, isolation and region transfer cases below as acceptance targets for open design work. [K-278](DesignAuthorityRegistry.md#k-278)

## Scoped execution context

“Global Variables Done Right” is CoD’s vision for dynamically scoped execution context. At the language level, a symbol such as `@gain` works like a named register. Its identity is fixed, and its value comes from the active execution context. `effect @gain = value` installs a scoped binding and yields the value. Leaving that scope restores the prior binding. Procedures can use these registers across calls, while pure functions compute from explicit inputs and lexical constants. [Symbol identity and bindings](SymbolicDynamicScope.md#symbol-identity-variable-identity-and-compiled-location) · [Callable kinds](SymbolicDynamicScope.md#functions-and-procedures)

`perform` uses the selected procedure’s captured bindings. `behave` runs it with the current bindings. If a reader is defined with `@gain = 1` and the caller later binds `@gain = 7`, `perform` reads `1` and `behave` reads `7`. Both leave the caller’s binding at `7`. A regional reference capture preserves the reference and its owning region. Later reads see the referent’s current contents, and writes through it survive binding restoration. [Capture semantics](SymbolicDynamicScope.md#captured-values-and-call-behavior) · [Regional lifetimes](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime)

This gives user programs an ISA-like way to design execution protocols. In the supported whole-program model, CoD resolves the selected callee and checks the required bindings, types and live regional references along the demanded call paths. The program defines what those values mean and what sequence of operations is valid for the domain. For example, a simulation step can bind its source and destination frame references. The program is responsible for reading frame A, writing frame B and swapping their roles only after the update completes. [Whole-program checking](SymbolicDynamicScope.md#call-path-binding-checks) · [Application validation plan](../applications/steady-snn/docs/LanguageCoDesign.md)

## Synchronous implementation

The current prototype lowers known calls directly to LLVM. Typed thread-local slots carry active runtime effect values. The compiler specializes procedure identities and regional owners along known paths. These are implementation choices for the synchronous entry model. [Frontend](../cod/lang_frontend.hh) · [LLVM lowering](../cod/lang_llvm.hh) · [Prototype reference](../tests/cod/language/README.md)

| Contract | Recorded native evidence |
| --- | --- |
| Nominal symbol identity and scoped rebinding | Aliases preserve identity, nested bindings restore, and ordinary symbolic assignment is rejected. [Identity restoration](../tests/cod/language/identity-restore.cod) · [Scope trace](../tests/cod/language/scopes.cod) · [Assignment rejection](../tests/cod/language/plain-assignment.cod) |
| Captured and current calls | Scalar captures, nested procedure rebinding and transitive `behave` calls observe the selected context. Missing definition-time and caller bindings are rejected. [Capture and restoration](../tests/cod/language/capture-scope-restore.cod) · [Missing capture](../tests/cod/language/missing-capture.cod) · [Missing caller binding](../tests/cod/language/behave-without-caller.cod) |
| Module composition | `using` retains the source definition and `import` redefines the body in the receiving context. The diamond trace exercises once-only initialization and first-encounter capture. [Using](../tests/cod/language/using.cod) · [Import](../tests/cod/language/import.cod) · [Diamond](../tests/cod/language/module-diamond.cod) |
| Pure functions and regional lifetimes | Functions reject ambient effects. Regional captures observe referent writes, and calls requiring expired regions are rejected. [Purity rejection](../tests/cod/language/impure-function.cod) · [Regional capture](../tests/cod/language/region-capture.cod) · [Expired capture rejection](../tests/cod/language/region-module-perform.cod) |
| Specialization across deep paths | The recorded stress suite includes 1,000 procedure specializations and a 96-module regional chain with procedure redefinition. [Composition evidence](../tests/cod/language/stress-validation.json) |

The recorded language run on 2026-11-06 passed 134 of 134 checks. The evidence files retain their own source hashes, traces and recording times. Update this tracker when the supported boundary, a relevant failure or its acceptance evidence changes. [Language validation](../tests/cod/language/validation.json) · [Regional layout and relocation](../tests/cod/language/regional-validation.json)

## Actor direction and region holding

`Mainbox<WorkPkg>` illustrates a process’s main mailbox with a work package as its payload type. A regional work package could carry a user-defined root, inputs and a typed description of computation. The receiver supplies the interpretation and the execution context. The mailbox type answers which work packages it accepts. Region holding and transfer determine who can access the storage and who releases it from construction through send, queueing, receipt and disposal. [Actor design](SymbolicDynamicScope.md#actor-concurrency-direction)

| Open design | Acceptance target |
| --- | --- |
| Process and mailbox semantics | Deliver a supported payload, reject an incompatible payload and demonstrate independent dynamic bindings in two processes. |
| Region holding and transfer | Specify legal holders and references at each delivery stage and account for rejected and abandoned deliveries. An exclusive handoff must preserve graph sharing, reject stale sender access and allow receipt after the sender’s scope exits. |
| Work package interpretation | Define how a user-specified root selects an admitted protocol and how the receiver provides its effects. Exercise the same work package with two receiver contexts. |
