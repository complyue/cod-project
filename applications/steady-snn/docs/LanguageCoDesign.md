# SNN-driven CoD language development and validation

Document-set baseline: 213 · Document revision: 213 · Recorded: 2026-11-06.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), this document revision 213, all sections; complete implementation contract `pending-human`.
- **Value lineage:** C-38/P-87 uses a real SNN workload to develop useful language features and assess their cost. P-25/P-22 connect the application to the charter's typed semantic programs. P-91 depends on the confirmed C-29 pull architecture; P-93 depends on the separately specified [D-10 language contract](../../../design/SymbolicDynamicScope.md). Dependencies do not grant qualification.
- **Human baseline:** H-93/H-94 govern continuous source state and target-owned two-frame pull; H-95 ties model mechanisms to learning/generalization. H-96 requests joint development. The individually scoped H-97–H-121 records specify capture, purity, storage, demand-checked module/call behavior and symbol selectors/import maps. H-123 limits the current increment to programming facilities and runtime support. H-140 makes module incorporation once-only after scanning; H-143–H-145 require one LLVM path, focused tests and simplification. H-148 requires each argument’s conversion to finish before the next argument is evaluated.
- **Invariants / non-goals:** retain regional lifetimes, static SoA connectivity, non-consuming source reads and common source-time views. No user heap lifetime or implicit heap closure is introduced. No push queue, new neuron equation, selected training rule, frame rate or auxiliary per-edge state follows from language development.
- **Acceptance:** each increment demonstrates both a language property and a simulator property using the same stated semantics. Diagnostic arithmetic is distinct from evidence of learned neural behavior.
- **Current assessment:** [K-252](../../../design/DesignAuthorityRegistry.md#k-252) reconciles revision 213 and the semantic catalog with confirmed module/call rules and actual evidence. The native records below identify their own implementation scope; new mechanisms still require qualification within their bounded increment.

## The first useful execution path

The first path is one complete CC update with destination-owned incoming traversal. It is useful before choosing a final neuron law: it can verify context binding, typed numerical calls, frame isolation and publication using a clearly labeled arithmetic fixture. The same path then accepts the selected continuous-neuron/AP model as that model's contract becomes ready.

| Simulator responsibility | Language form and reason |
| --- | --- |
| Evaluate a source's output from its A snapshot, model time and model parameters | A function: repeated reads have the same result and do not advance or consume the source. |
| Integrate a target's preceding state and gathered inputs | A function returning the computed next-state values under the selected numerical contract. |
| Read incoming IDs/weights, establish target/frame context and invoke model operations | A procedure using symbolic effects and explicit iteration. |
| Sample/update a runtime RNG when the selected model requires it | A procedure, or a function with explicit RNG state in its input/output; never a hidden draw in a source accessor. |
| Write destination state, apply group operations and publish a full frame | A procedure respecting the A/B boundary. |

Functions may compute an AP output over several frames from a source's continuous state. This does not require a separate queue of independently evolving AP tails. The language must still permit additional justified state and timing mechanisms; it should not encode an unjustified restriction into the neuron type.

## Vocabulary and model composition

A vocabulary module can declare `@read_frame`, `@write_frame`, `@model_time`, `@tgt_neuron` and model-operation symbols with comments. Their exact names are illustrative; sharing the declarations gives model modules the same identities without making a default equation part of the vocabulary.

Use `using` when the simulator needs a procedure as defined by its source model module. Use `import` when its body is intentionally redefined against the receiving module's lexical helpers and dynamic definition context. Either resulting procedure can be invoked with `perform` or `behave`; the latter can supply the current target/frame without replacing its lexical definition. Redefinition must still pass whole-program checking and the same immutable-A/B-publication contract. This gives a concrete comparison use without selecting a new SNN equation, plugin system or hot-loading mechanism.

All simulator entry dependencies are scanned/loaded before execution semantics are compiled. Scanning a model's `procedure @update_target {}` only registers its body; it neither captures a neuron/frame nor invokes a model operation. Entry compilation incorporates each dependency’s top-level execution semantics once, at its first encounter after the complete scan. Repeated acyclic encounters reuse that incorporated module; compiler encounters do not execute user code. The resulting definition/call paths must supply the intended contexts; scanning the vocabulary or model first does not establish those dynamic values.

Module-level bits model constants can use literal or pure-function initializers. The module can also establish defaults through `effect` and invoke setup operations through `perform`/`behave` during its execution. The initial dependency scan retains those expressions without running model setup or capturing a target/frame. Their scopes must still supply the bindings checked on the demanded paths.

Every cross-module reference triggers source execution semantics, including vocabulary references and `import`. The source top-level scope closes after its execution, so its effects do not remain ambient in the simulator entry. Operations must be brought into the receiver through `using`/`import`; `using` preserves source captures, while `import` defines against receiving context.

## Binding context in the target loop

The following describes expression evaluation order and scope lifetime; it is semantic pseudocode, not a parser grammar, a statement syntax or a neuronal equation:

```text
procedure @step_cc(cc, model_time):
    begin scope
        effect @read_frame  = immutable view of A
        effect @write_frame = exclusive destination view of B
        effect @model_time  = model_time

        for each target neuron:
            begin scope
                effect @tgt_neuron = target identity/view
                behave @update_target
            end scope

        finish the selected minicolumn-wide operations in B
    end scope
    publish the completed frame and swap A/B
```

`behave` deliberately gives `@update_target` the current target and frame. Its numerical functions receive the relevant snapshots and parameters explicitly. An accidental `perform` could restore the target/frame captured when that procedure was defined. The demanded whole-program call paths must supply the required bindings: `behave` uses the active target/frame context, while `perform` requires the appropriate captured values. Missing required values are compilation errors. Module initialization is part of those paths, not an independently compiled module with an unknown caller.

`@step_cc` and `@update_target` are dynamic procedure bindings; numerical helpers such as `source_effect` are lexical functions. Reads use `@name`, writes use `effect @name = value`, and operations participate in expression evaluation. A frame-publication operation can have a `void` result when it intentionally returns no meaningful value. Publishing and the write operation on regional arrays remain explicit effectful operations; symbolic rebinding alone does not define regional-object mutation.

For an operation needing fixed model configuration **and** the current target, lexical capture or explicit arguments can carry that configuration while `behave` supplies the dynamic execution context. `perform` is appropriate when the operation intentionally requires its captured dynamic values. It must not silently mix captured and current values according to convenience.

Capture payloads contain only bits or regional pointers. Frame/target references must not escape their valid scope or survive frame reuse without a specified lifetime. The first implementation can establish non-escaping stack values and same-region captures. User closures cannot gain a native-heap lifetime automatically; cross-region references remain an explicit placeholder for later increments. The system's per-thread binding context does not replace the CC's regional state or create another user allocation carrier.

The [first native prototype milestone](../../../design/SymbolicDynamicScope.md#first-native-prototype-boundary) covers modules, scalar semantics and actual native compile/run before general regional type construction or neural training. The exact-integer gather is an application data-flow fixture, not successful SNN behavior.

## Paired increments

| Increment | CoD language work | SNN work | Evidence required |
| --- | --- | --- | --- |
| 1. Identity and call meaning | Module/symbol identity, complete declaration scan followed by entry-driven execution composition, call-path checking, lexical functions, dynamic procedures, expression results, scalar capture and both call modes | A target-context example using a reusable operation | Expected traces in the case file; reject function effect access and alias/version collisions |
| 2. One complete frame | Typed products/sums, bounded array views, loops, pure numerical calls and procedure writes | Immutable-A incoming gather, B completion and publication | Forward/reverse destination orders agree; repeated source reads are non-consuming; an A write is rejected |
| 3. Inspectable typed programs | A minimal GADT core for the operations actually used, with type/effect indexes checked before execution | Express the same update as program data, retaining selected model parameters | Reference execution and the chosen native path agree under the declared numerical semantics |
| 4. Native specialization | Resolve symbol locations, specialize known procedures, preserve capture/restoration, track layout dependencies | Measure the actual gather loop with realistic state/edge counts | Generated accesses and measured context cost; compare outputs before claiming speedup |
| 5. Joint model validation | Extend the same typed program/primitive contracts as justified model features arrive | Continuous AP/rate alternatives, cancellation, regulation, DMN/token maintenance and separate flow learning | Behavioral criteria from existing application documents on the same stored network |
| 6. Composition boundaries | Multiple package versions, two execution contexts/threads, foreign entries and qualified persistence | Multiple CCs and application-selected transport/reload paths | Isolation, explicit binding installation and preserved identities/lifetimes |

These increments form a proposed implementation order, not a request to settle the entire language first. Each bounded contract should become concrete and reviewable before its implementation; implementation and validation then feed the next language and simulator decisions. The current C++ build-and-run driver can supply facilities without making C++ semantics the language specification. Its system-side heap use does not authorize heap-resident user data. Cross-region references are a named dependency of the later composition increment, not a detail silently completed by a native pointer.

## Concrete checks

The [semantic case file](../research/cod-language-scope-cases.json) records these initial expectations:

- A procedure created with `@x = 1` still observes 1 through `perform` after the caller evaluates `effect @x = 2`. `behave` observes 2.
- With a captured scalar 1 and caller scalar 7, a procedure evaluating `effect @x = @x + 1` then reading `@x` observes 2 on each `perform` and 8 on each `behave`. Its local effect restores on return; the caller remains 7 throughout.
- An imported alias accesses the original symbol. An identically spelled declaration from a distinct package origin has another binding.
- A demanded call path missing a required binding is rejected during compilation; no module is independently compiled. Procedure definitions use symbols, while lexical function lookup remains independent of caller spellings.
- Scanning all model/vocabulary dependencies causes no procedure activation or capture. Dependency execution-semantics incorporation follows first encounters during entry compilation, once per module, even when their order differs from scan order.
- Functions cannot access `@symbol`, establish `effect`, invoke either procedure mode, or conceal such access through a helper.
- Calls evaluate each argument once and complete its automatic conversion before the next argument. A failed conversion prevents later argument effects; functions, procedures and overloaded operator forms follow this same order.
- Given A = `[2, 5]`, target 0 gathering source 1 with weight 3 and target 1 gathering source 0 with weight 7 produce B = `[15, 14]` in either destination order. This exact-integer fixture tests data flow, not a neural model or learning outcome.

Subsequent native validation must distinguish language errors from numerical changes and from an unsuccessful neuronal model. A fast kernel with the wrong captured target fails language semantics; a correct frame implementation does not establish the desired DMN/flow/immersion behavior.

## Current native evidence

The [prototype reference](../../../tests/cod/language/README.md) and [validation record](../../../tests/cod/language/validation.json) cover symbols/modules, products/UoMs, iteration, static regional arrays/records and input-reachable pure snapshots. Checked CoD emits LLVM IR directly and executes through the existing toolchain with small native system services. The validation record identifies the latest source-corpus run and the earlier four-group component-suite run by commit. Each linked evidence record retains its own source hashes, timestamp and scope. The reference maps coverage to language contracts; full catalog and SNN acceptance require their own evidence.

[region-gather.cod](../../../tests/cod/language/region-gather.cod) combines separate type, layout/index, pure-kernel and procedure modules. It allocates source IDs, weights and A/B states as regional SoA arrays, gathers from A through target-owned incoming sets, and writes B before the next update reads it. Independent rounded FP32 calculations check the resulting sequence. The [full-pool record](../../../tests/cod/language/gather-full-pool-validation.json) exercises 112,281,600 arithmetic slots using 566,650,928 region bytes. Its rows are slot-set positions, not a selected CC neuron count. [Relocation evidence](../../../tests/cod/language/regional-validation.json) verifies that objects created by LLVM retain sharing, cycles and array access after copying the whole region and releasing the original.

These are programming/storage capabilities and arithmetic diagnostics. The H-123 increment excludes SNN dynamics/training; it establishes no learned behavior, neuronal update law, package-version semantics, cross-region representation or concurrent snapshot protocol. The broader application case catalog remains a specification with selected evidence links.

## Requirements that stay with the application

The language should support separate arrays for IDs, weights and state fields, explicit scalar formats, and specified reductions. It should not bake in 60+60 cells, a particular connection-class law, one AP curve, a 10 kHz frame rate or a fixed noise distribution. Those are application parameters or open model choices.

The confirmed two-frame pull architecture remains the simulator's execution contract. Language-level purity helps make non-consuming source reads checkable; it does not by itself establish correct neuron dynamics. Training publication, cancellation placement and any history needed by a selected mechanism still follow their application contracts.

Region-backed program/state storage and eventual C/C++ interoperability retain the charter's goals. The first arithmetic fixture does not require a new compiler toolchain, bootstrap path, persistence format or generic experiment framework. Such mechanisms enter when a named increment demonstrates their need.
