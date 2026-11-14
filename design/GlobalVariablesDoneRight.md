# Global Variables Done Right: Module Worlds and TLS Bindings

*显式组合，动态上下文 · Explicit composition and dynamic context*

Design revision 26 · 2026-11-13

## Design authority

- **Registry / current scope:** [D-12](DesignAuthorityRegistry.md#d-12); the semantic revision stated above and the independently qualified scopes in the current registry. **Whole file pending-human.** C-52/C-59/P-131–P-135/P-213; H-241 places independent worlds in the vision, D-15 owns the operational proposal.
- **Value path:** H-241/P-213 explicitly places independent module worlds (C-59) in this vision: semantic integration is locally controlled instead of editing one ambient world shared by every module. [D-15](ModuleWorlds.md) maintains the concrete inventory and links back here. This local relationship does not qualify the full operational design. P-134 relates inspectable typed execution context to C-10 and the charter's separation of program, execution instance and interpretation (§§1, 3.2 and 6.1). P-135 depends on [D-10](SymbolicDynamicScope.md) for binding, call and lifetime rules. C-52/H-151 supplies the Actor direction; P-131/P-132 and regional dependency P-133 remain candidate relationships. [Charter](RegionalSemanticPlatformCharter.md) · [D-13](GraphGrainedMemoryManagement.md) · [D-14](RegionalOwnership.md)
- **Contract boundary:** Actor scheduling/delivery/shutdown, optional outcomes and packet lifecycle need their own contract. Distinguish packet ownership from effect-resource replay; synchronous evidence is not Actor acceptance.
- **Invariants / non-goals:** Preserve symbol identity, scope restoration, function independence from dynamic effects, region-only user objects, zero-cost relocation and live owner associations. A captured replay neither consumes nor disposes of a pre-existing resource. No per-object owner, implicit optional resource, scheduler, runtime closure, mailbox transport or SNN model is introduced here.
- **Acceptance:** Validate symbol identity, publication, isolated worlds and scoped TLS restoration. Actor scheduling, delivery, shutdown and packet lifecycle require their separate contracts. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## Two complementary worlds

“Global Variables Done Right” includes both **independent module worlds** and **typed dynamic bindings across modules**. The module-world architecture is part of the vision itself, not merely an implementation detail of TLS.

| Part of the vision | Semantic contents and responsibility |
| --- | --- |
| Each module's dedicated world copy | Types, functions/procedures, overloaded methods, module bindings and related semantic information/values. A module explicitly exports what receivers may integrate. Information about identical imported identities merges in the receiver's own world; source worlds remain independent. |
| Each thread's flattened TLS collection | Typed symbolic bindings. Bits payloads live directly in TLS; non-bits object references point into regions supplied by region symbols and retain their lifetime dependencies. Symbol identity S and declared type identity T remain distinct; a compatible value may originate from another module-world copy without importing that world into TLS. |

The worlds do not model the same set of entities. A type or callable in a module world is distinct from a typed symbolic binding. Symbol identity, type identity, the value's identity/address and its originating world are separate facts. Different local names for the same symbol reach the same TLS entry; different symbol identities do not collapse because their names match.

For example, worlds A and B may each supply a value whose type has identity T. A symbol S declared with type T may point to A's value, then be rebound to B's compatible value. Its type obligation is checked by identity compatibility, not by comparing the addresses of the worlds' type information. This rebinding merges neither world and transfers no region ownership merely by changing visibility. Existing lifetime, capture and effect-transfer rules still apply.

The module world is closer to Julia's semantic runtime environment than to a declaration index alone. The essential CoD distinction is independent module world copies. Julia's documented global method/binding state and task-local world-age visibility do not provide that same architecture, and its world-age mechanism is not CoD's TLS pointer set. [Julia world-age model](https://docs.julialang.org/en/v1/manual/worldage/) · [Detailed CoD inventory](ModuleWorlds.md)

Module integration and dynamic rebinding have different combination laws. Integration composes selected semantic information by identity and explicit export. Effects change the current bits payload or reference under a symbol and restore preceding bindings as scopes end. Captured procedure values and referenced regional payloads keep their own semantics; TLS is not another type/method catalog or a copy of every payload it reaches.

A module constructs only its own module world. Cross-module world editing is impossible at compile time and runtime: source exports are read to construct receiver-local information, without rewriting the source. Compilation determines the flattened TLS shape and declared type constraints. Runtime effects change and restore the typed bindings within that shape, using `<-` for bits/ownership transfer and `=` for object-reference or procedure rebinding. Binding a value from another module-world copy does not permit editing that world (H-245).

H-236 requires the architecture before local lookup repairs. Receiver-owned construction, identity-based composition and runtime binding must remain coherent. No cross-module world editor or TLS editor-target protocol belongs to this vision; ordinary CoD calls do not expose internal construction or invocation machinery.

## Definitions, values and execution

A symbol object carries identity and meaning in the selected module world. For a callable, that meaning includes its contracts and methods. Defining or extending those methods is semantic construction; it does not capture current execution state or install a TLS binding. Explicit effects install compatible callable views. A view retains the source family and interpretation even when installed under a different destination symbol.

```text
proc @reader() -> const u64 { return @gain; }  # Register a method.
effect @reader;                               # Construct/install the family view.
```

These examples state the selected language contract; the compiler’s older implicit installation paths still require migration. `effect @target = reader` constructs a view from the symbol’s module semantics, while `effect @target = @reader` installs an existing TLS value with its captures. `effect @reader(...) { ... }` combines definition/extension with installation and preserves the separate stages. Using/import/mixin supplies semantic selection or redefinition, not an implicit receiver installation.

Callable views preserve families and use actual argument types to select methods, including through abstract interfaces. Whole-program compilation prepares the admitted alternatives before execution; direct method selection is an optimization. This lets business code retain abstraction and module interpretation without depending on whether the compiler can specialize a particular call. [Canonical world model](ModuleWorlds.md) · [AOT execution preparation](AheadOfTimeExecutionPreparation.md)

An entry module that extends a dependency callable must explicitly install its chosen family. The accepted entry installation cancels dependency installations, allowing LLVM to exclude otherwise unused world code. This is a composition rule, not automatic mutation of source methods. [D-21](AheadOfTimeExecutionPreparation.md#entry-installation-and-unused-dependency-code) records the required compiler error, final-image acceptance and preserved operand effects; cancellation applies only to dependency initialization.

## Shared vocabularies, independent worlds

H-242 makes vocabulary modules the common identity origin for symbols, types and their method collections. Types are primarily abstract, with a small number of supporting ADTs as needed. Shared method/callable identities and signatures make the vocabulary a semantic contract, rather than only a list of dynamic variable names.

Modules selecting the same resolved vocabulary retain those identities in their separate world copies. The common identities support both module-local information merging and TLS type compatibility across values originating in different worlds. Aliases do not change them; separate definitions or package versions remain distinguishable. Shared identities do not imply a shared mutable method table or world.

Vocabularies follow explicit export and re-export rules. Declaring a typed symbol supplies its identity/type, not a default dynamic binding. The detailed [vocabulary inventory](ModuleWorlds.md#vocabulary-modules-establish-shared-identities) separates these identity declarations from method-head implementation choices, which remain part of the architecture review.

## Scoped execution context

The TLS side of the vision supplies dynamically scoped execution context. At the language level, a symbol such as `@gain` works like a named register. Its identity is fixed, and its value comes from the active execution context. `effect @gain <- value` transfers bits into its TLS payload, installs a scoped binding and yields void. Object-reference symbols use `effect @target = object` and may rebind across live regions supplied by region symbols. Leaving that scope restores the prior binding; an established dynamic region resource cannot be consumed or emptied by extracting its ownership. Procedures can use these registers across calls, while functions use explicit inputs and lexical constants and may edit reachable memory without accessing those registers. [Symbol identity and bindings](SymbolicDynamicScope.md#symbol-identity-variable-identity-and-compiled-location) · [Callable kinds](SymbolicDynamicScope.md#functions-and-procedures)

`perform` uses the selected procedure’s captured bindings. `behave` runs it with the current bindings. If a reader value is materialized with `@gain` equal to 1 and the caller later binds it to 7, `perform` reads `1` and `behave` reads `7`. Both leave the caller’s binding at `7`. A regional reference capture preserves the reference value and its owner association; it neither freezes the referent nor extends the region’s lifetime. Later reads see the referent’s current contents, and writes through it survive binding restoration. [Capture semantics](SymbolicDynamicScope.md#captured-values-and-call-behavior) · [Regional lifetimes](SymbolicDynamicScope.md#storage-carriers-capture-types-and-lifetime)

For a captured whole-region resource, temporary visibility does not transfer ownership. If the caller has R2 under an identity and `perform` exposes captured R1 under it, R2 keeps its owner throughout. Removing the shadow exposes R2 again and does not release R1. Destruction belongs to actual owning-scope exit. [Resource shadowing](SymbolicDynamicScope.md#regional-resource-capture-preserves-owners)

This gives user programs an ISA-like way to design execution protocols. In the supported whole-program model, CoD resolves the selected callee and checks the required bindings, types and live regional references along the demanded call paths. The program defines what those values mean and what sequence of operations is valid for the domain. For example, a simulation step can bind its source and destination frame references. The program is responsible for reading frame A, writing frame B and swapping their roles only after the update completes. [Whole-program checking](SymbolicDynamicScope.md#call-path-binding-checks) · [Application validation plan](../applications/steady-snn/docs/LanguageCoDesign.md)

## Binding presence is not business optionality

A resource symbol can declare a direct region-reference type. H-181 prohibits moving ownership out and WPO rejects an access without a supplied binding; the language need not insert Maybe. The design also permits a deliberate `symbol @x :: Maybe<SomeRegion>`: bound absence is a business value, whereas an unbound @x is still a compile error. General sum construction/matching and this illustrative type spelling are not implemented. Declaring the symbol does not establish either a region or an absent value. Shadowing and captured perform preserve the underlying resource owners in both cases. [Typed binding contract](SymbolicDynamicScope.md#binding-availability-and-optional-resource-values)

## Synchronous acceptance

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Acceptance must preserve nominal identities, explicit publication, independent receiving worlds, captured/current call contexts, exact cleanup and scoped TLS restoration. Module construction determines definitions; runtime binding changes do not edit method sets. Synchronous acceptance does not establish Actor scheduling or packet lifecycle.

## Actor direction and region holding

`Mainbox<WorkPkg>` illustrates a process’s main mailbox with a work package as its payload type. A regional work package could carry a user-defined root, inputs and a typed description of computation. The receiver supplies the interpretation and the execution context. The mailbox type answers which work packages it accepts. Region holding and transfer determine who can access the storage and who releases it from construction through send, queueing, receipt and disposal. [Actor design](SymbolicDynamicScope.md#actor-concurrency-direction)

| Open design | Acceptance target |
| --- | --- |
| Process and mailbox semantics | Deliver a supported payload, reject an incompatible payload and demonstrate independent dynamic bindings in two processes. |
| Region holding and transfer | Specify legal holders and references at each delivery stage and account for rejected and abandoned deliveries. An exclusive handoff must preserve graph sharing, reject stale sender access and allow receipt after the sender’s scope exits. |
| Work package interpretation | Define how a user-specified root selects an admitted protocol and how the receiver provides its effects. Exercise the same work package with two receiver contexts. |

## Actor implementation readiness

Packet send/receive and region-backed dynamic resources are separate use cases under H-180. A receiver may interpret a payload using its own effects; that does not make sending a payload equivalent to moving an effect resource or replaying a sender capture. The candidate below addresses packet execution/transport only, while resource shadowing follows the synchronous contract above.

The synchronous implementation provides regional allocation, nominal payload types, typed effect bindings, non-extractable dynamic resources, explicit-input memory operations and direct LLVM lowering. The CoD frontend/runtime has no Actor process, mailbox, send/receive or scheduler implementation. The following gaps determine the work; adding a queue alone would not close them.

| Existing boundary | Consequence for an Actor increment |
| --- | --- |
| `Type` and `Form` in `lang_syntax.hh` have no process/mailbox forms. | `Mainbox<WorkPkg>` is an illustrative design spelling. A built-in mailbox type constructor could provide the required payload parameter without first implementing a general GADT/generics system; that is a proposal. |
| `Compiler::Region` tracks liveness and acquisition provenance; holding records and native cleanup slots account for effect-owned allocations. Module owners survive initialization. Dynamic-owner extraction is rejected. | Ordinary packet ownership and send-time invalidation need a distinct intrinsic transition and result/match contract. Captured shadowing cannot serve as packet transfer. |
| `Procedure::captures` is compiler metadata; demanded bodies are inlined. LLVM emission currently starts in one `main` entry. | An asynchronously invoked receiver needs a checked entry boundary. WPO can check a statically selected receiver body with its declared inputs/effects without knowing the runtime message order. Existing synchronous inlining does not itself supply that boundary. |
| Effect storage uses native TLS. | Two Actors sharing a worker do not automatically get separate effect contexts. A receiver entry must establish its own context and restore the worker’s previous context; suspending inside a handler would require preserving additional state. |
| `cod_lang_fail` exits the native program with status 70. | Actor-local failure, supervision and recovery are not present. The first milestone must state its error boundary rather than imply that one failed handler leaves other Actors running. |

### Proposed first executable milestone

The smallest useful candidate is two logical Actors in one native process, driven by one worker, with queued delivery and handlers that run to completion. This exercises asynchronous delivery without requiring mid-handler suspension, worker migration or a general runtime closure representation. A language process is not thereby defined to be an OS process or a permanently assigned native thread.

Use one declared regional work-package protocol, with receiver handlers known to WPO. The mailbox carries the graph and its typed entry reference; the receiving handler supplies the dynamic bindings. Merely placing a sender procedure value in a queue cannot transfer the compiler’s captured environment or its region lifetimes. A runtime envelope can hold system bookkeeping separately from the regional payload; this is not permission to add physical cross-region pointers to user records. Concrete syntax and envelope layout remain open.

Exclusive whole-region handoff to the mailbox runtime is the recommended first transport candidate because it preserves internal graph sharing without concurrent mutation by a sender. It can reuse the existing offset encoding and allocator pairing. In one address space, moving a system owner need not move any payload bytes. Wire transport remains a separate packet/ABI task; zero pointer repair does not mean zero transport cost. The [holding analysis](GraphGrainedMemoryManagement.md#actor-holding-and-transfer-review) defines the candidate’s lifecycle and alternatives.

Exclusive handoff and serial handlers can prevent a different Actor from concurrently mutating the transferred graph; they do not make nested function calls read-only. H-197/H-198 permits their explicit writes. Shared-memory concurrency would need its own contract, and no actor scheduler or cross-process handoff is implemented by the synchronous function increment.

### Work that can proceed and decisions before implementation

H-166/H-180/H-181 and H-188 establish the implemented module-lifetime, captured-shadow and non-extractable dynamic-resource foundation. H-168 supplies the send feedback boundary; H-170 proposes returning the unsent region. Ordinary packet creation/transfer, result matching, receiver context and delivery defaults require a bounded contract under `AGENTS.md`. The synchronous evidence makes those distinctions reviewable without selecting a scheduler or a named-source post-transfer state.

| Decision | Recommended initial scope | Material alternative |
| --- | --- | --- |
| Region handoff and local errors | Use an ordinary whole-region packet owner, separate from non-extractable dynamic resources. H-170 proposes returning ownership on local error; H-196 requires owner-bearing carriers to be intrinsic. | Decide construction, named-source post-state, typed error detail and matching syntax. An implicit moved-null state is not selected. |
| Execution and dynamic context | One worker; one handler completes before another starts; receiver-selected handlers and bindings; no ambient sender-context inheritance. | Suspension within a handler or transferred procedure captures require state materialization and additional lifetime rules. |
| Delivery and shutdown | H-168 supplies intrinsic mailbox send with local-system errors only, without delivery acknowledgement. Runtime-held payloads still require disposal when not delivered. | Blocking behavior, ordering, capacity and close/drain policy remain open. Local send acceptance cannot promise remote queue admission or handler completion. |

H-166 preserves module lifetimes; H-180/H-181 preserves captured/hidden dynamic owners and prohibits extracting them. Ordinary ownership and consuming installation into a dynamic resource remain separate from resource replay. H-170's returned-packet proposal does not select whether a named source becomes statically unusable or explicitly absent; [D-14](RegionalOwnership.md) tracks that open storage contract. Queue capacity is a resource parameter, while delivery order, failure results, cleanup responsibility and context inheritance are observable contracts. The [effect-held ownership analysis](GraphGrainedMemoryManagement.md#effect-held-region-ownership) separates packet consumption from dynamic restoration. P-131/P-132/P-134 remain candidate value paths, with P-133/P-135 and C-53 as dependencies/constraints.

After those choices are confirmed, a bounded implementation can cover typed mailbox construction, enqueue/dequeue, receiver entry lowering, dynamic-context installation and region transfer/release, followed by native validation. The acceptance trace should send a shared/cyclic graph, end the sender scope before receipt, run the same protocol under two receiver bindings, and account for one release. A local-error trace should cover the chosen ownership/disposal behavior; a later dropped message must not be reported as delivery acknowledgement or resurrect sender ownership. Static rejection cases should cover a wrong payload type and stale aliases/captures, including conditional paths. These few scenarios cover the central contracts without a test-count target. Parallel workers, cross-process/network transport, supervision, persistence and arbitrary suspended computations are later increments. No SNN dynamics or learning changes are required.

## Dynamic value categories and lifetime

H-277–H-281 admits region-symbol-backed interior object references with explicit lifetime checking, alongside inline TLS bits. Whole-region resources and statically selected procedures keep their respective ownership/capture contracts. The [regional analysis](GraphGrainedMemoryManagement.md#proposed-dynamic-value-boundary-and-soundness) distinguishes rebinding a TLS reference from changing regional graph edges or owning the region. A known procedure body does not make its captured resource instance immortal.

Borrowed-result branches and folds require a live actual-owner association; escaping a nested region or joining incompatible owners is rejected. A new dynamic resource may shadow another, while each owner stays tied to its acquisition scope. Ordinary packet-owner outcomes require their own contract.
