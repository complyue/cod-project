# Ahead-of-time micro-dispatch

Design revision: 7 · Recorded: 2026-11-13 America/Los_Angeles · Status: shared-method AOT candidate and bounded native implementation; established language behavior continues; permanent mechanisms remain pending-human.

CoD selects a method using the actual argument types, declared conformance and the callable's selected family/interpretation. Each reachable method has a general native implementation for its required semantic context. Within that body, operations discriminate locally only when their behavior or access requires concrete type information. Correct baseline execution requires neither whole-method argument-tuple specialization nor BBV, ORC or a runtime compiler.

## Design authority

- **Record / covered scope:** [D-21](DesignAuthorityRegistry.md#d-21), all sections, revision 7. [H-396](DesignAuthorityRegistry.md#h-396)/[H-397](DesignAuthorityRegistry.md#h-397) authorizes this architecture replacement, isolated implementation, persisted discriminator validation and conditional merge under external package revision 5. [K-585](DesignAuthorityRegistry.md#k-585) records alignment and the preserved behavioral scope. The former exploration is isolated at `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8`; it supplies no final architecture selection.
- **Human baseline and value:** H-372–H-377/H-379–H-382 preserves family selection, stages, explicit installation, operand effects, nearest lookup and inferred-owner bindings. H-378 confirms C-82/P-333 → C-01, C-83/P-334 → C-02 and D-21/P-337: abstract business intent and selected interpretation, prepared native execution and organized coverage obligations. That behavioral path continues in this revision. C-86/C-87/C-88 and P-363–P-370 record the new shared-body/concrete-evidence/root-compatibility candidate paths; detailed production mechanisms remain pending whole-file review. Charter §§1–3, 4.3, 5.3–5.4, 6.3, 8 and 12 governs.
- **Source / scope of authorization:** current project user, [micro-dispatch side chat](codex://threads/01a1d51f-3ef4-7101-a86c-94666675f5b6), authorized handoff received in the active mainline. H-396 retains actual statements and the distinction between confirmed task direction and an unapproved permanent ABI. Exact message timestamps and personal identity are unavailable. Conditional merge concerns the validated bounded scope; it does not qualify unsupported full-language machinery.
- **Dependencies / invariants:** P-338/P-361/P-362; D-01 regional protocols, D-07/D-11 nominal conformance and unique-most-specific selection, D-10/D-14 owners/access, D-15 dynamic bindings, D-17 completion, D-18 complete captures and D-16 representation-independent identity/UoM. Preserve actual owners, selected interpretations, source isolation, ordered effects and zero-cost relocation. No runtime world mutation or implicit owner union follows.
- **Acceptance / exclusions:** an explained AOT route, unspecialized shared method bodies, genuine family selection and local operations, independent field evidence, focused positive/negative native observations and traceable scope. Permanent discriminator/ABI, arbitrary generic closure, a complete CoD checker/parser, full module composer, reflection/persistence admission, concurrency, ORC/tracing/OSR/deoptimization and fractions are excluded from the implemented subset. The [experiment](../experiments/micro-dispatch/README.md) owns detailed evidence and limits; success there does not silently narrow the language.

## Responsibilities and native path

| Responsibility | Required behavior |
| --- | --- |
| Selected semantic artifact | Construct reachable identities, declarations, conformances and parallel interpretations before preparing code. Runtime input supplies values of known ADTs; it adds no definitions or methods. |
| Checking and coverage | Check applicability, unique-most-specific selection, bodies, result/effect/owner contracts and required native operations for every admitted use. Keep semantic proof distinct from optimization. |
| Method selection | Use actual concrete type identity/evidence, conformance and the selected family/interpretation. Multiple arguments may jointly select a method. |
| Method implementation | Emit a general native body for each reachable method in its required context, accepting its abstract declared parameters. Explicit generic instantiations or semantic interpretation differences may require additional bodies. |
| Local operations | Retain concrete evidence through forwarding and dispatch locally when field access, arithmetic, conversion, writing, construction or another operation needs it. |
| Optional optimization | Prove a direct target, eliminate redundant tests or specialize when easy and worthwhile. Removing these optimizations must preserve correctness and native coverage. |

The candidate path is selected definitions → checked semantic operations → temporary native control/operation representation → method selectors, shared bodies and local operation paths → AOT emission/linking → native execution over runtime values. There is no assumed existing checker, generic backend or fallback. The bounded implementation explicitly supplies its finite checks, operation templates, generated C++20 and trusted primitives using the existing Clang/LLVM toolchain. A production checked representation/emitter interface remains to be designed.

Method selection remains observable even if the body reads no fields: two admitted signatures may return different opcode categories or constants. Replacing ordinary method selection with field access alone would lose that behavior. Conversely, merely forwarding a value or reading a field with a statically common offset/storage contract needs no redundant discriminator check. Sharing an access path does not make distinct signatures interchangeable.

## Concrete-type evidence in abstract values

An abstract value or polymorphic field carries the discriminator of its actual concrete type, like a sum value. Reuse CoD type identity directly when it is compact and suitable. Otherwise a compact code can resolve unambiguously to that identity across compatible selected artifacts; this is an encoding, not another semantic type system. A naked build-order ordinal is insufficient for stored values.

```text
Example.x : A | B       carries TypeId(A) or TypeId(B)
Example.y : C | D       carries TypeId(C) or TypeId(D)
Holder.value : Example | Other
                       carries TypeId(Example) or TypeId(Other)
```

Example needs no additional enclosing-object discriminator merely because x/y vary. Updating x changes x's evidence/payload, not Example's identity. Independent fields remain independent until an operation actually depends on both. Analyze possible types as finite sets, not numeric ranges or a flattened Cartesian object tag.

An abstract type has its own identity for signatures and conformance. A concrete value does not acquire that abstract identity as its actual runtime type. The same concrete evidence can flow through different abstractions and selected method interpretations. Forwarding a field value retains its evidence with the payload/reference and actual owner where still needed. A second runtime identity tag and a header on every regional object are not required.

### Call-site facts and observable distinctions

Use **proven facts at the call site**, not just the method definition's broad parameter annotation. An exact known type supplies constant identity evidence. A finite possible set can narrow selection. If every remaining possibility selects the same method under the selected interpretation, call that existing method body directly; no method cloning is required. A dynamic selected family can still affect the target even when argument type is known. An unchecked hint is not proof.

Runtime distinctions serve admitted behavior. Same-layout IntegerLiteral/BooleanLiteral values select different no-field compiler-opcode methods in the experiment. Their one concrete discriminator preserves that difference. If an operation only reads their common value offset, the generated access code is shared and reads no tag. Existing unit/conformance rules likewise do not justify additional metadata where a discriminator or static context already supplies the needed evidence.

Concrete identity and externally selected conformance are separate. Selecting an adapter relation must not relabel an existing value; equal layout does not create conformance. The fixture assigns explicit durable 64-bit IDs to its semantic definitions and generates selectors from the corresponding nominal relation graph. It does not define a permanent ID allocation policy, cross-artifact ABI or replacement identity subsystem. The existing compiler's `Type.identity` is a `size_t` component of a richer Type record; its suitability as a complete universal runtime key has not been established. This isolated model makes its mapping explicit instead of assuming an existing checker/runtime service.

### Carrier and admission limits

The tested polymorphic field contains an 8-byte concrete discriminator and a relative payload link. A runtime borrowed view also retains the actual region through `global_ptr`. The field happens to occupy 16 bytes in this target layout; neither that width nor the earlier checksum width is a permanent ABI. Statically typed policy fields have no runtime discriminator.

Prepare an injective mapping before code generation. The fixture decoder rejects unknown, mismatched and abstract-as-concrete evidence before constructing regional payloads. Typed access still depends on checked storage, ownership and native-operation contracts; a numeric tag alone is no memory-safety proof. General untrusted DBMR/mailbox validation, forged native views, general cross-artifact compatibility, inline bits carriers and permanent borrowed-value ABI remain open. Regional payloads are constructed in regions with relative links; no regional copy/move, raw persistent native/code address, hidden user heap or owner union is introduced.

### Persistent meaning across fused worlds

A discriminator inside a region is persistent payload. A compatible newer program must recognize and compare the same concrete type even when unrelated types or discovery order change. Prefer a small stable definition identity. If an encoding needs context, that context must be durable and validated before access; independently numbered indices are not semantic equality. Do not rewrite stored tags or reconstruct the graph to match the reader's numbering.

The prototype uses explicit definition-owned 64-bit IDs, independent of hashes, enumeration order or inventory size. A small catalog stored once in its fixture root binds each relevant ID to a qualified semantic key and the supported size/alignment. The reader validates those bindings before payload access. Conflicting meanings reject; there is no silent global uniqueness assumption, central registry service or remapping pass. The catalog is ordinary fixture data using the existing DBMR API, not a production header/loader change or another tag on every value.

An independently compiled older writer and newer reader, with reordered definitions and an unrelated concrete type added, interpret the same saved bytes with identical results. A reader that swaps Example/Other ID meanings rejects before accessing their payloads. File bytes remain unchanged. This establishes stable discrimination for the documented root layouts; it does not prove arbitrary schema evolution, global identifier allocation or full file validity. Stable identity never authorizes an incompatible layout.

## DBMR root-type compatibility planning

Type checksums are **root-associated metadata planned for storage in DBMR**, to help relate a saved root/type version to the current program's fused-world types. They are not runtime method-selection or field micro-dispatch tags. A root checksum concerns relevant declared type structure/dependencies, not current object values, every byte in a file or every unrelated type in the program.

Checksum equality is only one compatibility input and is not sufficient proof. A different checksum also does not decide whether a supported compatible evolution exists. Root/type identity and schema versions, relevant dependencies, stored discriminator interpretation, target layout/ABI, file format and bounds/reference validation are candidate additional inputs. Canonicalization, recursive descriptions, dependency closure, algorithm/width/placement and migration rules remain separate design questions.

A small planning observation hashes a finite named recursive/shared description graph. Adding an unrelated type preserves both the root checksum and the chosen stable IDs; a naive build-order encoding would change. Equal root checksums are also possible under a deliberately wrong ID binding. The native fixture rejects that binding through its separate root catalog. These observations show why root-checksum equality cannot repair a wrongly interpreted discriminator or serve as the sole compatibility predicate.

This package changes no production DBMR header, format, loader or migration mechanism. Existing DBMR validation remains in force. The checksum observation illustrates separation of responsibilities, not a finalized compatibility predicate or storage design.

## Mutable fields and aliases

Every local access must see evidence and payload that belong together. Prepare/check a replacement before publishing it. The tested sequential update has a nonthrowing, callback-free commit; a failure before commit preserves the old pair. An alias read reloads the field after a mutation and observes the new alternative. The baseline keeps no speculative field-type cache to invalidate.

This establishes neither atomic concurrent publication nor a general rollback allocator. Abandoned region allocations retain their ordinary region lifetime. Future redundant-check elimination must account for aliases, calls, effect rebinding and cleanup; a stable slot name is not a stable concrete-type fact. Existing owner/access rules govern which aliases may write.

<a id="actual-type-dispatch-and-wpo-coverage"></a>
## AOT coverage without mandatory tuple specialization

Before execution of the supported selected artifact, prepare its method selectors, reachable method bodies and every required local native operation path. Semantic checking completeness, native-code availability, selection and optional specialization are separate obligations. Several argument combinations can enter one body; the body handles their necessary local distinctions. Coverage need not be a dense table and does not require one native body per concrete tuple.

The finite experiment checks coverage and ambiguity by subtracting symbolic rectangles from declared domains. Specificity uses nominal relationships, not equal extents. An explicit intersection resolves crossed methods; an unproved or ambiguous call rejects before emission. Its separate work limit is an inconclusive compile-time result, never permission to execute an uncovered value. Generated selectors use applicability and absence of an applicable more-specific method, independently of declaration order.

Local code generation is driven by methods and operation contracts. Independent x/y access paths scale with their relevant alternatives; a genuinely joint conversion retains its joint checks. Larger finite sets can increase selector comparisons, descriptor preparation, repeated checks and coverage fragmentation. No claim makes all combination complexity disappear. The scoped artifact prepares 13 method bodies, including both interpretations and no-field methods, with optional specialization disabled; symbol inspection verifies one emitted body per generated method.

Runtime recursion can call an existing native body. Compilation must separately close explicit generic instances and required semantic contexts. A fixed nominal universe alone does not prove bounded generic instantiation: growth such as successively nested type applications needs a sound rejection/proof strategy. Implicit type generators, general recursive inference and variadic closure are not implemented by the finite fixture. These remain full-language obligations, not language restrictions inferred from a successful prototype.

## Selected worlds and required dynamic context

The fixed semantic boundary is relative to one selected artifact; preparing another composition does not silently mutate an in-flight one. Independent module worlds retain their definitions, interpretations and visibility. They do not mutate a source world or create a process-wide latest method set. Preserve the established base/extended/indirect `1,7,1` behavior and existing alias/using/import/mixin rules. Fusing a selected artifact includes reachable dependency-local interpretations and procedure-local effects; it does not reduce every call to the entry module's view.

Conceptually invoke a method with explicit arguments and its required implicit dynamic-context projection. Include dependencies through admissible callees, touch and deferred/cleanup actions. An empty projection needs no dynamic-context input; lexical captures and explicitly passed memory remain separate. Functions retain their existing restrictions on dynamic effects.

Current/behave calls use current required bindings. Perform replays complete captured values for its captured bindings and obtains remaining requirements from the invocation context, with scoped restoration. Copying a callable preserves its complete environment; replacing its source holder cannot change that copy. Borrowed referents retain their actual owners and permitted mutation remains observable. Touch can require capture/dependency evidence without a current-value read. Context projection does not freeze the whole TLS world or put every owner/binding into a specialization key.

TLS slots, explicit machine arguments, registers and elimination are physical choices. The fixture uses explicit projected family values plus TLS adapters and conservative supplied dependency contracts. General effect inference, nested capture replay, touch owner validation and arbitrary complete environment schemas remain open; sharing native bodies does not remove these checks.

## Definition, publication and installation

Bare `other` denotes a symbol object; `@other` reads its installed TLS value. Symbol lookup, semantic family construction, callable materialization, capture, installation and invocation remain distinct. Definitions/extensions require the existing permissions and do not install runtime values. Runtime installation targets TLS and requires `effect`. A combined declaration/installation adds or extends definitions at compilation and installs the applicable family when executed.

```text
effect @destination = source;   # Materialize the selected source family view.
effect @destination = @source;  # Retain the existing complete callable value.
effect @destination;            # Install this module's composed family.
```

Source/destination symbol identities may differ; installation does not merge them or redirect the source view to the destination's family. Export publishes the whole method set, not a private subset chosen by a view contract. A skipped runtime branch installs nothing. Repeated execution creates installation events, not repeated method registration. Selecting a dependency retains its required once-only initialization without implicitly installing a receiver binding.

### Entry installation and unused dependency code

An entry module extending a dependency's callable must explicitly install it; omission is a compile-time error. With the required entry installation, suppress only affected dependency-module-initialization TLS writes. Retain operand evaluation, callable construction, effects, failures and temporary cleanup in their normal order. Suppressed writes create no installation/restoration event. Procedure-local installations and explicit dependency views remain usable.

CoD must expose correct roots to LLVM so genuinely unused dependency-world code can be excluded from the final image. Removing a write alone does not prove code elimination. Operand effects or retained source views can keep code reachable. Native reachability/linkage and final-image verification remain production work; the fixture checks the installation boundary, not a complete module composer/linker implementation.

**D21-Q-entry-composition:** alias/extension identity matching, conditional entry-installation coverage, binding availability and dependency encounter order still require concrete mechanisms. Do not treat a future entry write as already executed or erase operands to make the code graph smaller.

### Nearest bindings and inferred owners

Bare names and `@` follow the same nearest lexical binding. A non-symbol parameter/local named `p` makes `@p` ill-typed; lookup does not bypass it for an outer symbol. This applies to effects, touch, call modes and type inspection. Definition-site lexical bindings retain their meaning under captures/mixins; a caller-local shadow does not reinterpret a separately defined callee.

Each inferred owner binder is an implicit const lexical binding, including generic region-or-scope owners. It participates in the same lookup rule. Its constness grants neither region capability, content immutability, ownership nor lifetime extension. Preserve actual owner provenance; no runtime owner object or owner-union ABI is selected.

## Completion, validation and remaining work

Native body sharing must preserve pending return/error state and ordered cleanup. Early deferred failures may enter later handlers; a completing handler preserves an existing return, while an explicit return in catch/finally replaces pending completion and remaining cleanup still runs. Recovery without a required return follows the established missing-return rule. The fixture checks finite completion categories with real LLVM resource acquisition, verifier failure and ordered release. General exception payload graphs and full deferred-handler matching remain unimplemented.

The [focused evidence](../experiments/micro-dispatch/README.md) covers runtime-only field combinations, abstract forwarding/storage/results, no-field method selection, crossed specificity and rejection, independent layouts, call-site narrowing, invalid/mismatched discriminator rejection and separate recursive/shared root-checksum planning, alias updates, actual owners/remapping, unchanged saved data across separately compiled worlds, selected contexts and native cleanup. It supplies a bounded native implementation from explicit finite contracts, not a full source compiler.

The remaining delivery topics retain their stable identifiers:

| Topic | Required production contract |
| --- | --- |
| DD-AOT-01 / DD-AOT-08 | Keep behavioral qualification and new candidate paths distinct; align dependent design summaries. |
| DD-AOT-02 / D21-Q-entry-composition | Selected family/composition identity, revision dependency and conditional installation coverage. |
| DD-AOT-03 | General semantic/generic/recursive closure and actionable unproved-coverage diagnostics without mandatory tuple expansion. |
| DD-AOT-05 | Permanent evidence/carrier/ABI, admission, bits and borrowed-value transport with actual owners. |
| DD-AOT-07 | Namespace/staging, grammar migration and established missing/ambiguous-call failure contracts. |
| DD-AOT-04 / DD-AOT-06 | Complete family/capture values and a thin native emitter consuming checked operation contracts. |
| DD-AOT-09 | Broader source-language integration, complete context/owner/completion checking and final-image reachability. |
| DD-AOT-10 | Evaluate costs when workload evidence makes a trade-off consequential; no benchmark matrix is a prerequisite here. |

Preserve method/callable/source associations for later profiling. ORC is reserved for future tracing-JIT hot paths; no profiler, trace compiler, guards, OSR, deoptimization, code-lifetime protocol or runtime code generation belongs to this baseline. No performance class follows from correctness traces or branch/body counts. Fraction arithmetic/promotion and its open design questions remain deferred.
