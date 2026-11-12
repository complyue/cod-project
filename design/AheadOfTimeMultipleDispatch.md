# Ahead-of-Time Multiple Dispatch

Design revision: 3 · Recorded: 2026-11-12 · Status: behavioral contract, value path and initialization-cancellation boundary confirmed; detailed mechanisms pending-human.

**Prepare as much as makes sense, ahead of time.**

CoD callable values preserve a selected method family and its module interpretation. Calls select methods using actual argument types. Whole-program compilation prepares the admissible leaf-ADT combinations and their dispatch before execution. Independent module worlds coexist; runtime installation chooses a compatible callable view without changing those worlds.

## Design authority

- **Registry / scope:** [D-21](DesignAuthorityRegistry.md#d-21), revision3. H-372–H-377 records the stage, symbol, publication, dispatch, AOT and vision decisions. [H-378](DesignAuthorityRegistry.md#h-378) qualifies revision1's behavioral contract and P-333/P-334/P-337 value path, retained under K-535/[K-536](DesignAuthorityRegistry.md#k-536). H-379/H-380 adds the entry-installation rule and confirms initialization-only cancellation with operand evaluation preserved. Concrete type-evidence/ABI, namespace, coverage algorithms, uncovered diagnostics and entry-composition mechanisms remain pending-human; the qualification does not cover those mechanisms.
- **Confirmed value lineage:** C-82/P-333 → C-01 preserves abstract business operations and their selected interpretations independently of optimization. C-83/P-334 → C-02 prepares native execution for the admissible type combinations without compilation during covered calls. D-21/P-337 organizes those responsibilities before implementation. H-378 explicitly confirms this scoped path under charter §§2–3/§5.2/§6/§8/§12.
- **Confirmed constraints:** P-330–P-332/P-335/P-336 separate module construction from explicit TLS installation, preserve symbol identity and whole-family publication, require actual-type dispatch, and specify WPO/AOT coverage. P-339–P-341 govern entry installation and initialization-only cancellation while preserving operands. C-35/C-36/C-37/C-57/C-59/C-60 and H-230 retain aliases, call modes, permissions, context and source isolation. P-338 depends on D-10/D-14/D-15/D-18; dependency does not confer authority. D-16 remains a compatibility constraint.
- **Human baseline:** current project user, side chat [01a1cfc8-0910-7ce2-8e67-a0a037e31c95](codex://threads/01a1cfc8-0910-7ce2-8e67-a0a037e31c95) and active mainline [01a1cefe-fe25-7251-b25b-b3908d803ff6](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6), recorded 2026-11-12 America/Los_Angeles; exact message timestamps and personal identity unavailable. H-372–H-380 preserves the behavioral decisions, revision1 value-path confirmation and entry-installation/cancellation rules. External review revision10 and vision revision3 retain provenance only.
- **Invariants / non-goals:** preserve selected family/context, module isolation, explicit effects, complete callable values, captures, owner/access/lifetime checks and zero-cost regional relocation. No JIT implementation, runtime method-table mutation, global aging world, general reflection API, object-header format, persistence ABI, owner union or fraction implementation is selected.
- **Acceptance:** observable family dispatch through abstract arguments/results, equivalent specialized calls, complete coverage of the selected composition, context/capture preservation, exact cleanup and measured compilation/code growth. These are required outcomes; implementation acceptance remains outstanding.

## Separate semantic responsibilities

| Question | Responsibility |
| --- | --- |
| Which definitions and methods are available? | Identity-based construction in the selected module world. |
| Which callable is installed now? | An explicit effect supplies a compatible view to a typed TLS slot. |
| Which method handles these arguments? | The view's family dispatcher uses their actual types and its selected interpretation. |
| When is machine code generated? | WPO/AOT prepares the required code before that composition executes. |

Bare `other` denotes a symbol object carrying identity and semantics. Its module information can include callable contracts and methods. `@other` reads the currently installed TLS value. Symbol lookup, callable materialization, capture, installation and invocation are distinct operations; the model does not require a separate runtime allocation for each responsibility.

Module-world definition/extension needs no `effect` and follows existing import/extension permissions. Runtime installation always targets TLS and requires `effect`. The combined `effect @sym(...) { ... }` defines **or extends** at compilation and installs a compatible family view when executed. It installs the applicable family, not just its newly added method.

```text
effect @destination = source;   # Materialize the source symbol's selected family view.
effect @destination = @source;  # Install an existing TLS value, retaining its captures.
effect @destination;            # Install this module's composed family without adding a method.
```

Source and destination identities may differ. Installation does not merge their identities, add methods under the destination or redirect invocation to the destination's family. Exporting a symbol-identified callable publishes its entire method set; a compatible view's type contract is not a private-method filter. [Module-world contract](ModuleWorlds.md) · [Source operations](SymbolicDynamicScope.md#callable-effects)

A skipped runtime branch performs no installation. Repeated execution creates separate installation events, not repeated module-method registration. Existing declaration/scope restrictions still apply. Selecting a dependency can retain its once-only executable initialization, including explicit effects; selection itself does not implicitly install a receiver binding.

### Entry installation and unused dependency code

If the entry module extends a dependency's symbolic callable, it must explicitly install that callable with `effect`; omission is a compilation error. With the required entry installation, CoD cancels dependency module-initialization installations of that callable. Procedure-local installations remain active. A dependency installation cannot silently supply the entry's extended interpretation. H-379/H-380/P-339 selects this composition requirement; it does not remove the underlying method definitions or merge source worlds.

Cancellation omits the TLS write while still evaluating the installation operands and constructing their values in the ordinary order. Preserve their effects, failures and temporary cleanup. The canceled operation publishes no binding and creates no corresponding installation/restoration event. A combined effect declaration still contributes its compile-time definition/extension. [H-380](DesignAuthorityRegistry.md#h-380)/P-341 confirms this boundary.

CoD must expose the resulting reachability to LLVM so unused dependency-world code is absent from the final program image. Canceled TLS writes must not retain obsolete family views through installation roots. LLVM performs native code elimination; CoD supplies the correct semantic installation and reference graph. Code still reached through preserved operand effects, an explicit source-world view, a procedure-local installation or an ordinary call remains used and must retain its behavior. P-340 does not promise that every dependency method disappears; removing a write alone is not evidence of final-image elimination.

**D21-Q-entry-composition:** specify identity matching across aliases/extension versions and conditional entry-installation coverage against dependency encounter order. Operand evaluation still obeys ordinary binding availability; a future entry write cannot be treated as already executed. The cancellation boundary above is settled. Concrete composition and lowering mechanisms must preserve it without inventing broader effect deletion or new evaluation order.

## Static parallel module worlds

**Static** is relative to the selected compilation/composition. It does not freeze runtime values or prevent explicit integration and compilation of a new domain inside a running host. **Parallel** means independent coexistence, not concurrent execution.

Definitions, imports and extensions construct the receiver's world. They do not modify source worlds or create a process-wide latest method set. A callable keeps its selected family and interpretation. H-230's base/extended/indirect `1,7,1` behavior is a compatibility requirement. Aliases, selected contexts and intentional identity unions retain their existing rules.

Julia's world-age model tracks changes to global method/binding state and which changes an execution can observe. CoD's selected model uses independent module interpretations. This difference is separate from AOT versus JIT. [Julia world age](https://docs.julialang.org/en/v1/manual/worldage/)

| Dimension | Julia reference | CoD direction |
| --- | --- | --- |
| Organization | Evolving global method/binding state with task-local visibility. | Independently composed module worlds. |
| Coexistence | Executions can observe different world ages. | Base, extended and composed interpretations coexist. |
| Compiler obligation | Respect world visibility and compiled-code validity. | Distinguish selected family/context and composition dependencies. |

The concrete revision protocol still needs artifact identity, dependency invalidation, code lifetime and installation rules for revised compositions. This vision selects no hot-reload API and permits no silent reinterpretation of an in-flight callable.

## Actual-type dispatch and WPO coverage

An `Entity` parameter or result can carry `Node` or `Edge`. A family containing methods for those concrete types must preserve their behavior through abstract declarations, storage and callable boundaries. A declared abstract type cannot justify erasing the actual type or choosing a single method permanently. Direct calls and inlining are valid when the compiler proves equivalent behavior for the selected family/context and possible arguments.

For two such parameters, the initial domain contains `Node/Node`, `Node/Edge`, `Edge/Node` and `Edge/Edge`. WPO resolves and compiles the applicable targets and dispatch before execution; execution supplies the pair that occurs. Several combinations can share a method body. Proven correlations can eliminate impossible combinations. Coverage does not require a dense table or independent machine code for every tuple.

Before pruning and sharing, leaf counts n₁ through nₖ permit up to n₁ × … × nₖ tuples. Module interpretations and generic instantiations can add contexts. This is a planning bound, not a measured code-size prediction. Coverage must account for recursive/generic closure and variadic calls without treating each runtime invocation as a new specialization. An incomplete analysis must retain possible cases.

WPO knows possible concrete types but may still need runtime evidence of which type a value carries. Abstract arguments, results, borrowed references, TLS and stored views must retain that evidence with their existing lifetime and relocation guarantees. Physical encoding, transport and ABI are design tasks. The selected behavior neither mandates per-object tags nor prohibits a discriminator carried by a view.

Applicability, specificity, conversions, owner/access and effect rules remain constraints. Missing/ambiguous cases cannot silently select the first method. Static diagnostics remain valid where proved; any uncovered failure-timing or exception contract must be decided before implementation. Fractions remain deferred; intrinsic dispatch and representation-independent UoM cannot acquire accidental new promotion or return-type-dispatch rules.

## Comparison with Julia compilation

Julia supports AOT compilation for system/package images as well as JIT. Its documented image-building path uses execution traces to discover specializations, and PackageCompiler supports explicit precompile statements. CoD's proposed distinction is coverage of the admitted WPO domain before execution, not exclusive possession of AOT. [Julia AOT](https://docs.julialang.org/en/v1/devdocs/aot/) · [PackageCompiler sysimages](https://julialang.github.io/PackageCompiler.jl/stable/sysimages.html)

| Concern | CoD WPO/AOT direction | Julia reference and tradeoff |
| --- | --- | --- |
| Code availability | Covered runtime cases select prepared code. | Precompiled code can be reused; additional cases may compile during execution. [JIT](https://docs.julialang.org/en/v1/devdocs/jit/) |
| First use | Remove method compilation from covered calls; loading, initialization and dispatch still cost time. | Precompiled images reduce first-use work but may omit specializations. [AOT](https://docs.julialang.org/en/v1/devdocs/aot/) |
| Abstract interfaces | Actual-type dispatch is the baseline; specialization can remove dispatch. | Generic functions contain methods, and specialization can resolve calls before execution. [Functions](https://docs.julialang.org/en/v1/devdocs/functions/) |
| Compilation volume | May compile admissible combinations rarely encountered in one run. | Demand-driven compilation can defer cases not encountered. This is an inference from the compilation model, not a benchmark. [JIT](https://docs.julialang.org/en/v1/devdocs/jit/) |

The intended benefits are predictable code availability and inspectable dispatch coverage. Costs include compilation work, possible code growth and composition-closure requirements. No universal throughput, startup, size or memory advantage is asserted. Regional storage, ownership and relocation are separate dimensions; AOT does not eliminate runtime environments, type evidence, TLS or cleanup.

JIT remains a future option if measured combination growth makes compilation or generated programs impractical. Assess sharing and pruning first. Any later JIT design must preserve family identity, interpretation and observable behavior; no threshold or automatic fallback is selected here.

## Delivery sequence and acceptance

The backlog belongs to the active mainline. The confirmed behavior is not reopened by the remaining mechanism work. A pending item is not an imposed language restriction.

| Item | Deliverable and gate |
| --- | --- |
| DD-AOT-01 | Behavioral value-path qualification is recorded under H-378. Keep detailed mechanisms separately qualified before implementation. |
| DD-AOT-08 | Reconcile principles, detailed contracts, examples and authority summaries in D-12/D-15/D-10/D-07 and affected D-18/D-14/D-17/D-16 scopes. |
| DD-AOT-02 | Specify selected composition/family/context identity, dependencies and revision boundaries; preserve H-230. |
| DD-AOT-03 | Define WPO closure for leaf alternatives, generics, recursion and variable arity, with coverage diagnostics. |
| DD-AOT-05 | Design concrete-type evidence through abstract arguments/results, storage, borrowed views, TLS and callable returns under relocation/owner constraints. |
| DD-AOT-07 | Close namespace/staging, grammar migration, entry-installation cancellation and uncovered missing/ambiguous-call diagnostics; prepare concrete alternatives for human decisions where required. |
| DD-AOT-04 | Replace one-method value construction with family dispatcher values, preserving context, capture and complete-value semantics. |
| DD-AOT-06 | Lower precompiled targets/dispatch, shared implementations, surviving installation roots and proven direct calls; expose unused dependency-world code to LLVM elimination. |
| DD-AOT-09 | Verify composed behavior, including source isolation, cross-identity installation, abstract multi-dispatch and cleanup. |
| DD-AOT-10 | Measure growth and assess JIT only if later evidence warrants a separate design. |

Acceptance scenarios must cover:

1. Receiver-local extension without effects, whole-family export, shared aliases and unbound TLS after definition alone; reject unauthorized extension and conflicting declarations.
2. Explicit family installation, TLS-value reinstallation and installation without extension, including different destination identities and an unbound source TLS slot. Preserve source context and existing value captures.
3. Combined definition/extension with skipped and repeated runtime paths. Distinguish method registration from installation and capture events; preserve scope restoration on return/exception and failed preparation.
4. `Node` and `Edge` flowing through abstract parameters and callable results, with more than one dispatch argument and several selected module views. Specialization and remaining dispatch must agree on values, effects, errors and owners.
5. Precompiled coverage of every admitted type combination in the fixture, exercised through runtime input. A one-method guard or one-concrete-result implementation limit cannot redefine the accepted language.
6. Entry extension without explicit installation rejects. A valid entry installation omits the applicable dependency initialization TLS writes, preserves operand effects/failures and leaves procedure-local installations active. Final-image inspection demonstrates removal of genuinely unused code, while explicitly retained source-world views and code needed by preserved operand effects still work. Conditional coverage follows the specified entry-composition mechanism.

Measure compiler wall time and peak memory, admitted combinations, unique emitted bodies, dispatcher and executable size, startup/first-call latency and steady-state throughput. Vary arity, leaf counts, sparse/dense method coverage, module-view count and generic instantiations. A Julia comparison must include a representative precompiled image and distinguish first-call from warmed execution on the same workload and target. No benchmark result is implied by this document.
