# Ahead-of-time execution preparation and runtime micro-dispatch

Design revision: 36 · Recorded: 2026-11-24 America/Los_Angeles · Status: AOT execution-preparation behavior and native-coverage contract; bounded experimental evidence; permanent mechanisms retain their separate review scopes.

During AOT compilation, CoD checks the required behavior and prepares native implementations, semantic method selectors and local micro-dispatch paths. Proven call-site facts can resolve selections, demand or reuse specializations and permit direct-call emission. Preparing selection code does not execute that selection or the source program’s runtime effects. At runtime, execution follows the prepared code and performs remaining selections using actual runtime evidence.

Semantic method selection uses actual argument types, declared conformance and the callable’s selected family/interpretation. A method has one semantic definition and may have a general native implementation and several compiler-prepared specializations. Within a generated body, runtime micro-dispatch selects individual operations whose choice remains unresolved after compilation. Selecting a user method and selecting a local operation within its implementation are separate responsibilities. Correct general execution remains available without exhaustive whole-method tuple specialization, BBV, ORC or runtime compilation.

Function preparation selects code under a static signature. Here invocation resolves live access to the actual defining activation. Maintain current owner-plus-locator resolution across nested calls and backing growth. Internal symbolic handler replay data and borrowed activation references have separate lifetime and cleanup responsibilities.

## Design authority

At r35 the [symbol identity decision](../CHANGELOG.md#h-runtime-symbol-identity-20261124) applies [D-10’s protected lexical name](SymbolicDynamicScope.md#the-bare-symbol-name-at-runtime) to lexical lookup and specialization. Distinct spellings may denote the same identity. A binding that would hide a visible symbol fails at its declaration.

D-21 r34 applies the [grouped installation decision](../CHANGELOG.md#h-grouped-local-proc-install-20261124) and the [local handler decision](../CHANGELOG.md#h-local-proc-handler-body-20261124) under P-330/P-337. Entry installation must select the extended family. Validate the independent scoped override and the ordinary name-only entry activation. See [local procedure bodies](SymbolicDynamicScope.md#installing-a-local-procedure-body).

[Confirmed symbolic procedure forms](../CHANGELOG.md#h-lexical-symbolic-proc-20261123) applies D-10-HANDLER-INSTALL to the explicit installation required by H-379. Check installation ordering and the selected family in the entry module. [Lexical and dynamic calls](SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)

- **Callable lowering.** [H-CALLABLE-MODEL-20261118](../CHANGELOG.md#h-callable-model-20261118), D-18-HERE-STORAGE and D-21 preserve actual-type dispatch and prepared target selection. Mainline chooses the representation. Across inlining and separate compilation, repeated or recursive here calls must reach the correct original slots.

- **D-21r30, entry installation.** [D-10-HANDLER-INSTALL](DesignAuthorityRegistry.md#d-10-handler-install) governs the source forms for effectful procedures. [H-EFFECTFUL-PROC-INSTALL-20261118](../CHANGELOG.md#h-effectful-proc-install-20261118) excludes assignment installation. H-379 still requires the entry to establish the handler for its extended family. Acceptance requires this installation without a first-class handler operand. The scoped name-only form is `effect proc @p;`.

- **Full-scope dependency continuity:** [K-FULL-SCOPE-IMPLEMENTATION-20261118](../CHANGELOG.md#k-full-scope-implementation-20261118) updates only the referenced r92 qualification. This document’s semantic revision, own qualification, acceptance and exclusions are unchanged.

- **Prepared callable targets.** Preserve the selected callee across argument evaluation and the destination on failed preparation under its ordinary replacement contract.

- **Consolidated native-coverage contract:** [D-26](DesignAuthorityRegistry.md#d-26), now the [generic/recursive coverage section](#generic-and-recursive-native-coverage), retains H-407/P-371/P-372/P-374/P-375 in semantic revision 22 within D-21r27. [H-559](../CHANGELOG.md#h-559)/[K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) removes the standalone resolved-issue tracker and migration narrative while preserving current coverage, type, failure, diagnostics and acceptance requirements. Its authority block and review exclusions remain scoped to that section; no semantic, implementation or wider qualification change. D-16 continues.

- **Callable-reference continuity:** [K-ALIAS-GLOSSARY-20261117](../CHANGELOG.md#k-alias-glossary-20261117) preserves this revision's existing D-18 dependency under the canonical CallableViewsAndCaptures.md path. Only dependency links and this continuity summary change; existing contracts, qualification, acceptance and D-16 remain unchanged.

- **Lifetime dependency.** D-18-HERE and D-28-OWNERS require an activation-specific proof through synchronous forwarding, unwind and restoration.

- **Full scopes.** D-17 retains local returns and separate cleanup for double braces. Outer names access live lexical slots under D-18.

- **Checked deferred coverage:** [H-550/H-551](../CHANGELOG.md#h-550)/[K-DEFER-CHECKED-COVERAGE-20261116](../CHANGELOG.md#k-defer-checked-coverage-20261116), [D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch)/P-461–P-463, requires checked failures to be handled before their owning defer pass completes. A covering defer catch counts; enclosing throws alone cannot authorize residual checked members. Preserve actual registration/FIFO order, H-525 declared obligations, retained ordinary body_exception, owners and D-16. D-17r86 owns acceptance and conservative proof boundaries. Candidate C-36/C-44/C-63/C-65 and charter §§2/3.1–3.3/4.3/6.3/8/12 serve explicit recoverability and valid ordered cleanup. This dependency alignment continues unrelated contracts; it does not qualify the whole mechanism/value path or assert compiler support.

- **Return-state dependency:** [D-17-RETURN-STATE](DesignAuthorityRegistry.md#d-17-return-state)/[H-542–H-547](../CHANGELOG.md#h-542), [H-548](../CHANGELOG.md#h-548)/[K-RETURNING-PROFILE-20261116](../CHANGELOG.md#k-returning-profile-20261116), aligns this revision’s prepared return evidence under P-484. Candidate C-54/C-63/C-65 and charter §§2/3.2–3.3/4.3/6.3/8/12 preserve author-controlled completion, result/owner validity and observable cleanup. D-17/D-23 own acceptance: scoped cancellation, unbound inspection under the [normal-completion decision](../CHANGELOG.md#h-normal-completion-20261124), ordinary binding grammar/copy/borrow lifetimes and owning inspection without capture. Module cleanup has no callable return target. Preserve unrelated qualified contracts, FIFO failures and D-16. The H-548 agent selections do not add implementation support or full value-path/physical qualification; owning capture and future pure-callable rules are excluded.

- **FIFO cleanup dependency:** [H-541](../CHANGELOG.md#h-541)/[K-DEFER-FIFO-20261116](../CHANGELOG.md#k-defer-fifo-20261116) updates this revision’s native completion section under D-17-DEFER-DISPATCH/P-463. C-36/C-63/C-65 and charter §§2/3.3/4.3/6.3/12 require preserving observable FIFO handling, tail insertion, incoming-only visits and existing owner/context boundaries. D-17 owns acceptance and remaining policies; unchanged lookup, capture and native-coverage contracts continue. No new runtime support, whole-design qualification or D-16 change follows.

- **Disjoint declarations:** [D-10-DISJOINT](DesignAuthorityRegistry.md#d-10-disjoint), [H-527–H-535](../CHANGELOG.md#h-527) and [K-DISJOINT-ENDPOINTS-20261116](../CHANGELOG.md#k-disjoint-endpoints-20261116) cover this revision's constraint-related sections. Existing listed-endpoint ancestry permits inherited overlap; a second independent listed-parent relationship rejects, while prior redundant relationships warn about the path and intermediate methods. Preserve complete public records, per-pair definition/import authority, frozen validation, declared checkedness, demanded checking, owners/captures and D-16. Candidate C-56/C-10/P-476–P-478 serves explicit nominal intent and stable interpretation under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12. [D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) owns acceptance; finite generic/cyclic proof procedures, source implementation and full value-path qualification remain separate. No separate exemption-path store or runtime inheritance mutation.

- **Shared-information candidate:** [D-15-DISJOINT-POOL](DesignAuthorityRegistry.md#d-15-disjoint-pool)/P-479–P-481 under C-59/C-56/C-10 records [H-533's](../CHANGELOG.md#h-533) requested feasibility review. [D-15](ModuleWorlds.md#shared-disjoint-information-per-module-world) owns identity, upstream retention, applicable local/source checks, publication filtering and frozen-backing acceptance. Physical representation and the complete value path remain pending-human; no compiler support is inferred.

- **Checked and unchecked roots:** [H-524–H-526](../CHANGELOG.md#h-524)/[K-ERROR-ROOTS-20261116](../CHANGELOG.md#k-error-roots-20261116), [D-17-ERROR-ROOTS](DesignAuthorityRegistry.md#d-17-error-roots), adds Error <: Exception and preserves explicit Exception handling obligations even for values proven unchecked. Checkedness-changing adapter conformance rejects. Candidate P-473–P-475 serves explicit typed failure contracts and user-defined errors under charter §§2/3.1/3.3/4.3/6.3/8/12. Existing actual-type dispatch, concrete failure categories, owner/capture and D-16 contracts continue. New roots/declaration enforcement and arbitrary exception payload support are not established by current source tests; general disjointness and remaining wrapper/throws/inspection scope remain separate.

- **Wrapper-aware catch dependency:** [H-518–H-522](../CHANGELOG.md#h-518)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), [K-WRAPPER-CATCH-20261116](../CHANGELOG.md#k-wrapper-catch-20261116), records source-order wrapper-prefix matching and same-exception-identity selection of the hidden tip within the lexical catch context under candidate P-470–P-472/charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve ordinary payload type evidence, complete incoming exception history for selected exception operations, actual owners and D-16. [H-523](../CHANGELOG.md#h-523)/[K-RETHROW-FROM-20261116](../CHANGELOG.md#k-rethrow-from-20261116) requires standard-wrapper cause/checkedness agreement and adds automatic rethrow from cause, with a direct diagnostic suggestion on mismatch. Inspection and transparent checked coverage remain pending in D-17; no copied graph, general value ABI or new source support is inferred.

- **Exception construction provenance:** [H-510–H-517](../CHANGELOG.md#h-510)/[K-EXCEPTION-CONSTRUCTION-20261116](../CHANGELOG.md#k-exception-construction-20261116) continues this dependency in r16→17. [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve), candidate P-464–P-469 under charter §§2/3.2–3.3/4.3/6.3/8/12, separates unchanged throw propagation from constructor-time trace/cause creation, lexically supplied rethrow defaults and the bounded process OOM diagnostic window. Preserve semantic checkedness, module/callable contexts, captures, actual owners, independent deferred-queue obligations and D-16. D-17 owns acceptance and remaining representation/aggregate/ordering questions; no compiler support or whole-scope qualification follows.

- **Deferred-queue dependency:** [H-503/H-505–H-509](../CHANGELOG.md#h-503)/[D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch), [K-DEFER-STACK-20261116](../CHANGELOG.md#k-defer-stack-20261116), updates r15→16's completion dependency under P-463 and charter §§3.2–3.3/6.3/8/12. Lowering must distinguish local attached catches from a bodyless entry's queued-error scan and once-only finally, preserving actual owners/contexts and excluding ordinary body errors. D-17 owns pending rethrow/aggregate/return/aggregate storage policies and acceptance. Existing AOT family, staging and D-16 contracts continue; finite single-completion evidence does not validate the new queue model. H-509 requires a single unchecked UnhandledDeferError retaining all residual deferred exceptions; H-510–H-515 supplies the current construction/rethrow contract; FIFO order follows H-541; H-550/H-551 requires checked coverage before pass completion; aggregate/body-field representation remains separately scoped.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-21r14→15 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Whole callable-contract compatibility:** [H-462](../CHANGELOG.md#h-462)/[K-CALLABLE-FAMILY-CONTRACT-20261115](../CHANGELOG.md#k-callable-family-contract-20261115), P-409, integrates [D-10-ARGUMENTS](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding) without changing the method/native hierarchy. A typeof-family parameter requires every origin signature, independently of current call reachability. C-57/C-89 and the existing D-21/D-26 paths preserve checked semantic coverage and code sharing under charter §§3.1/6.3/12. Acceptance distinguishes full source compatibility from demanded native emission and rejects missing/ambiguous/incompatible required cases. Concrete proof algorithms, local family grammar and default transport remain separate; no new ABI or implementation claim.

- **Registry / current scope:** [D-21](DesignAuthorityRegistry.md#d-21); the semantic revision stated above and the independently qualified scopes in the current registry. **Behavioral path continued** H-378/P-333/P-334/P-337 with H-372–H-382. **Replacement design/prototype and conditional merge authorized** H-396/H-397. K-593 aligns input/coverage to H-407/D-26; H-409 clarifies call-site specialization; H-410/K-596 makes AOT preparation and runtime selection explicit. C-86/C-87/C-88/P-363–P-370 detailed production paths remain pending-human.
- **Value path:** The registry’s named value relationships explain the charter benefit and retain their individual confirmation states; dependency relationships confer no additional authority.
- **Contract boundary:** H-409/K-594 separates one semantic method from its general/specialized implementations and local dispatch points; unique-most-specific selection by actual concrete identity/conformance; local operations and proven call-site facts. Preserve installation/operand effects, captures, owners and completion. D-26 supplies the qualified coverage responsibility; concrete analysis, permanent ABI, module composition and compatibility protocol remain open. ORC future-only.
- **Acceptance:** The completion and validation section defines composed family/context, installation, capture, native coverage and cleanup acceptance; prepared implementations preserve actual owners and effects. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-682](../CHANGELOG.md#k-682) covers r11→12’s callable-view access clarification under H-366–H-369/P-327 with H-375/H-378, preserving actual type selection and the existing coverage responsibility. [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## Responsibilities and native path

| Responsibility | AOT compilation | Runtime execution |
| --- | --- | --- |
| Selected semantic artifact | Prepare the selected method definitions, conformances and parallel interpretations. | Use the prepared artifact. Actual types may be outside the static catalog under D-26’s `<any>` boundary; values do not install new executable methods or trigger compilation. |
| Checking and coverage | Check applicability, unique-most-specific selection, bodies, result/effect/owner contracts and required native operations for every admitted use. Keep semantic proof distinct from optimization. | Execute the established native paths and required runtime checks; their preparation is not repeated as runtime compilation. |
| Semantic method selection | Resolve the target when proven facts suffice; otherwise prepare a selector using actual type identity, conformance and the selected family/interpretation. | Invoke the resolved target or execute that prepared selector with actual argument evidence. Multiple arguments may jointly select a method. |
| Method implementation | Preserve one semantic definition per user method; prepare a general implementation and demand-driven specializations, reusing compatible implementations. There is no universal one-native-body limit. | Invoke an applicable prepared implementation of the selected semantic method. Execution does not register another method or specialization. |
| Local operations | Resolve known operations and prepare local micro-dispatch paths for unresolved accesses, arithmetic, conversions, writes and other operations. Preserve the required evidence through forwarding. | Execute the prepared local selections and operations, then rejoin shared control flow. These branches are not whole-body specializations. |
| Call-site native preparation | Demand/reuse an appropriate specialization and emit a direct call when the semantic method and interpretation are established. | Execute the emitted call. A proven direct call needs no runtime selector merely because a general implementation also exists. |

The candidate path separates preparation from execution:

```text
AOT compilation:
  selected definitions → checked semantic operations → temporary native control/operation representation
  → prepared method selectors, general/specialized bodies and local micro-dispatch paths → emission/linking
Runtime execution:
  emitted calls and remaining prepared selections → native operations over runtime values
```

Generating a selector supplies executable decision code; it does not choose a branch using future runtime values. Proven facts may instead establish the target during compilation and remove that runtime decision. There is no assumed existing checker, generic backend or fallback. The bounded experiment explicitly supplies its finite checks, operation templates, generated C++20 and trusted primitives using the existing Clang/LLVM toolchain. A production checked representation/emitter interface remains to be designed.

Method selection remains observable even if the body reads no fields: two admitted signatures may return different opcode categories or constants. Replacing ordinary method selection with field access alone would lose that behavior. Conversely, merely forwarding a value or reading a field with a statically common offset/storage contract needs no redundant discriminator check. Sharing an access path does not make distinct signatures interchangeable.

## Callable, method and native implementation hierarchy

A callable family contains user-defined methods. Each method has one semantic definition/body and can have multiple compiler-statically-registered implementations: general, partially specialized or more narrowly specialized. These implementations realize that method; they are not additional overloads.

```text
callable family
  user method f(T): one semantic definition
    general implementation
    specialization for established u64 facts
    partially specialized implementation with remaining local dispatch
  user method f(u64): a distinct semantic method
    its prepared implementation(s)
```

The family’s applicability and specificity rules first establish the semantic method; implementation selection then chooses a valid prepared implementation of that method. This is a semantic dependency order, not a requirement that both decisions execute at runtime: compilation may resolve either decision when the relevant facts are proven and prepares code for any remaining runtime choice. A specialization of `f(T)` for `u64` cannot compete with or override the user-defined `f(u64)`. Complete callable values preserve the selected family/interpretation and captures independently of this compiler implementation inventory. Runtime owner instances and capture values are not automatically specialization keys.

H-462 makes `parameter:typeof origin` a whole-family contract: the supplied callable must implement every origin method signature, even if the receiving body currently calls only one. Compatibility uses the selected family/world and existing applicability/specificity rules, including actual selected-method obligations when domains overlap. A caller-local named adapter may provide the required methods; extra target methods cannot make a required case ambiguous or incompatible. A generic method may satisfy several required signatures with a valid proof. This source admission rule neither counts native bodies nor forces emission of unreachable native code; optimization cannot turn partial source coverage into compatibility.

The callable view constrains parameter target access as specified by [the runtime callable contract](CallableViewsAndCaptures.md#runtime-callable-contract). Coverage checks admitted actual types and owner relations under that access contract. Readonly views preserve readonly borrowing; independent value parameters receive their declared local access. Actual type identity, selected interpretation, complete captures and ordered operand preparation remain distinct from those access permissions. Coverage and native selection must agree on that same view contract.

## Phase and initiator

| Initiator and activity | Phase | Meaning |
| --- | --- | --- |
| User-authored definition code | Its definition/compilation stage | Constructs/registers semantic callable and method definitions. |
| Compiler processing a call site | AOT compilation | Checks the call, resolves semantic/implementation choices supported by proven facts, requests/reuses specializations and emits calls or prepared selectors for unresolved choices. It does not execute the runtime invocation. |
| Compiler checking/lowering a method body | AOT compilation | Checks and prepares the body’s operations, including local micro-dispatch code where needed; it does not execute those selections or the body’s runtime effects. |
| Explicit user-authored compile-time computation | Its explicitly designated compile-time stage | Executes that computation under its contract; it is distinct from compiler-triggered analysis or specialization. |
| User-written runtime call | Program execution | Follows the emitted call path, executes any remaining prepared method/implementation selector, invokes the selected native body and executes its prepared local selections as encountered. It creates/registers no methods or specializations. |
| Runtime effect installation/capture | Program execution | Changes or retains complete bindings/values, not semantic method or specialization registration. |

These distinctions identify both who initiates work and when it occurs. A source call expression can be inspected during compilation and later execute at runtime; those are different activities. Here AOT denotes preparation during compilation of the selected artifact, before that artifact’s execution. The distinction does not collapse the charter’s separate semantic levels or stage-relative computation model into one process-wide stage.

## Local micro-dispatch points

AOT prepares the code and required alternatives for each local micro-dispatch point; the actual local selection occurs when runtime execution reaches that point with concrete evidence. One generated general or partially specialized body may contain several independent points. For a plan with polymorphic x/y fields, reading x can choose its access path, reading y can choose another, and a genuinely joint conversion can select using both. Runtime executes each applicable local branch and rejoins the shared control flow. These are local operation choices within a method implementation, distinct from selecting the user method. None of these branches creates a specialization of the whole method.

Call-site specialization may statically resolve some or all of those points. Unresolved points remain local selections inside that implementation. Independent points do not require a Cartesian product of whole-body versions; actual joint dependencies remain explicit. Implementation counts reflect deliberate compiler-prepared implementations, not the number of runtime branch paths or activations.

## Concrete-type evidence in abstract values

An abstract value or polymorphic field carries the discriminator of its actual concrete type, like a sum value. Reuse CoD type identity directly when it is compact and suitable. Otherwise a compact code can resolve unambiguously to that identity across compatible selected artifacts; this is an encoding, not another semantic type system. A naked build-order ordinal is insufficient for stored values.

```text
Example.x : A | B       carries TypeId(A) or TypeId(B)
Example.y : C | D       carries TypeId(C) or TypeId(D)
Holder.value : Example | Other
                       carries TypeId(Example) or TypeId(Other)
```

Example needs no additional enclosing-object discriminator merely because x/y vary. Updating x changes x's evidence/payload, not Example's identity. Independent fields remain independent until an operation actually depends on both. Analyze statically known alternatives as finite sets, retaining the outside-catalog path where admitted; do not flatten independent fields into a Cartesian object tag.

An abstract type has its own identity for signatures and conformance. A concrete value does not acquire that abstract identity as its actual runtime type. The same concrete evidence can flow through different abstractions and selected method interpretations. Forwarding a field value retains its evidence with the payload/reference and actual owner where still needed. A second runtime identity tag and a header on every regional object are not required.

### Call-site facts and observable distinctions

Use **proven facts at the call site**, not just the method definition’s broad parameter annotation. For a straightforward call such as `twice(3u64)`, compilation establishes the semantic method and required interpretation, demands or reuses an appropriate AOT specialization, and emits a direct call to it. Runtime executes that call; compiler inspection does not execute `twice`. A finite possible set can narrow selection; a proven common target may also permit a direct call to an existing suitable implementation. The general correctness path does not force this known call through runtime discrimination. A dynamic selected family can still affect the semantic target even when the argument type is known. An unchecked hint is not proof.

Runtime distinctions serve admitted behavior. Same-layout IntegerLiteral/BooleanLiteral values select different no-field compiler-opcode methods in the experiment. Their one concrete discriminator preserves that difference. If an operation only reads their common value offset, the generated access code is shared and reads no tag. Existing unit/conformance rules likewise do not justify additional metadata where a discriminator or static context already supplies the needed evidence.

Concrete identity and externally selected conformance are separate. Selecting an adapter relation must not relabel an existing value; equal layout does not create conformance. The fixture assigns explicit durable 64-bit IDs to its semantic definitions and generates selectors from the corresponding nominal relation graph. It does not define a permanent ID allocation policy, cross-artifact ABI or replacement identity subsystem. The existing compiler's `Type.identity` is a `size_t` component of a richer Type record; its suitability as a complete universal runtime key has not been established. This isolated model makes its mapping explicit instead of assuming an existing checker/runtime service.

### Types outside the static catalog

The static catalog is not the entire runtime type universe. A type outside it selects through the identified CoD `<any>` type object while retaining its actual identity and required owner evidence. A valid uninterpreted borrow may pass through a method that needs no interpretation. An operation requiring an unavailable interpretation raises checked `StaticallyUnknownTypeException`; receipt or forwarding alone does not. Same-T checks compare actual identities, and a detected breach of established typing raises unchecked `UnsoundRuntimeTypingError` before unsafe dependent work. Known missing/ambiguous methods retain their ordinary diagnostics.

[D-26](AheadOfTimeExecutionPreparation.md#generic-and-recursive-native-coverage) governs checked obligations across genuine unknown-type paths and voluntary throws declarations. Generic syntax alone does not impose a throws declaration. The boundary supplies neither a universal payload layout nor permission to copy unknown-layout objects, bypass validation, load executable methods or compile at runtime. Physical carriers and input validation remain separate contracts.

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

<a id="mutable-fields-and-aliases"></a>
## Mutable fields and runtime target aliasing

Every local access must see evidence and payload that belong together. Prepare/check a replacement before publishing it. The tested sequential update has a nonthrowing, callback-free commit; a failure before commit preserves the old pair. A read through another reference to the same target reloads the field after a mutation and observes the new alternative. The baseline keeps no speculative field-type cache to invalidate.

This establishes neither atomic concurrent publication nor a general rollback allocator. Abandoned region allocations retain their ordinary region lifetime. Future redundant-check elimination must account for runtime target overlap, calls, effect rebinding and cleanup; a stable slot name is not a stable concrete-type fact. Existing owner/access rules govern which reference access paths may write.

<a id="actual-type-dispatch-and-wpo-coverage"></a>
## AOT coverage without mandatory tuple specialization

During AOT compilation, prepare the selected artifact’s semantic method selectors, demanded general/specialized implementations and every required local native operation path. Coverage establishes that admitted runtime behavior has prepared executable paths and required checks; it does not require choosing each future runtime branch during compilation. Semantic checking completeness, native-code availability, semantic method selection, implementation selection and local operation selection are distinct responsibilities. Several argument combinations can enter one body; runtime execution uses the body’s prepared local paths to handle their necessary distinctions. Coverage need not be a dense table and does not require one native body per concrete tuple.

The finite experiment checks coverage and ambiguity by subtracting symbolic rectangles from declared domains. Specificity uses nominal relationships, not equal extents. An explicit intersection resolves crossed methods; an unproved or ambiguous call rejects before emission. Its separate work limit is an inconclusive compile-time result, never permission to execute an uncovered value. Generated selectors use applicability and absence of an applicable more-specific method, independently of declaration order.

Local code generation is driven by methods and operation contracts. Independent x/y access paths scale with their relevant alternatives; a genuinely joint conversion retains its joint checks. Larger finite sets can increase selector comparisons, descriptor preparation, repeated checks and coverage fragmentation. No claim makes all combination complexity disappear. The scoped artifact prepares 13 method bodies, including both interpretations and no-field methods, with optional specialization disabled; symbol inspection verifies one emitted body per generated method.

Runtime recursion calls existing native bodies. Native coverage concerns shared methods, local operations and required checks; it need not enumerate every precise type description or emit a body per substitution. Growing type applications alone neither force cloning nor establish an error. The [generic and recursive coverage contract](#generic-and-recursive-native-coverage) governs the actual obligations and concrete semantic-error, detected-resource-limit and implementation-gap diagnostics. No universal analysis-nontermination detector is assumed. The finite fixture does not establish full generic analysis, recursive inference or variadic coverage; its limits are not language restrictions.

## Generic and recursive native coverage

### Design authority for generic and recursive coverage

[D-26](DesignAuthorityRegistry.md#d-26), semantic revision 22 within D-21r27, retains the H-407-confirmed behavioral/native-coverage contract and value path under C-89/C-90/P-371/P-372/P-374/P-375. P-373/P-376 records dependencies only. H-400–H-402/H-404–H-406 fixes the input, failure and diagnostic boundaries; H-409/H-410 fixes the hierarchy and preparation/execution distinction. [K-AOT-CONTRACT-CONSOLIDATION-20261117](../CHANGELOG.md#k-aot-contract-consolidation-20261117) continues the previous r21 requirements here under H-559's documentation cleanup direction. Charter §§2/3.1–3.3/4.3/5.3/6.3/8/12 preserves inspectable identities and finite prepared execution without mandatory tuple cloning. The acceptance table below and exclusions govern this scope; no permanent solver, carrier/ABI, runtime compilation or new language feature is selected. D-16 and all independently qualified dependencies remain unchanged. Current source support is recorded separately; neither consolidation nor historical tests establish an unsupported path.

### Type interpretation and generic contracts

`<any>` is discussion notation for a real CoD type object in the fused world, not a selected declaration spelling or an imported top-type/boxing convention. Concrete known types retain their own identities: `func()->const u64` and `func()->const bool` remain distinct. Generic callable types require stable type-level interpretations; binders, type applications and persistent identity encodings retain their separate design scope.

Let K be the selected fused world's complete static type catalog. Possible known alternatives come from all types in K and are narrowed by applicability and proven facts. Types outside K select through `<any>` while retaining actual type identity and owner evidence wherever needed. This projection implies neither type equality nor a common payload layout. Every method still requires its signature, conformance and selected interpretation; a known missing/ambiguous method retains its ordinary diagnostic.

An operation requiring an interpretation absent from the prepared artifact raises checked `StaticallyUnknownTypeException`. A valid borrow may be received or forwarded without interpreting its contents and without an unknown-type obligation solely because its type is outside K. Required downstream interpretation still carries that obligation. Generic syntax alone creates no throws declaration; a voluntary explicit throws declaration retains its caller obligation even at a proven known-type call. The current D-17/H-525 declared-contract rule remains binding. Valid uninterpreted passage does not authorize unknown-layout payload copying, opaque dereference, invalid input, new executable methods or runtime compilation.

Repeated T means the same actual type, including outside K. A statically established mismatch is a compile error. A required runtime check compares actual evidence, not the common `<any>` selection identity; a breach of established typing raises unchecked `UnsoundRuntimeTypingError` before unsafe dependent work. This is distinct from a recoverable unavailable interpretation and from explicit author assertions under D-17. It does not admit arbitrary heterogeneous inputs or prescribe a new dynamic-cast/input-validation API.

```cod
func marker(x: T) -> const u64 where T { return 1u64; }
func first(x: T, y: T) -> const T where T { return x; }
func twice(x: T) -> const T where T { return x + x; }
```

These semantic examples do not claim current parser/carrier coverage. With a valid outside-catalog borrow, marker needs no interpretation; first preserves the actual same-T and lifetime contract. Twice requires an applicable (+) and a compatible result, without inventing a universal operator or trait mechanism. A known supported `twice(3u64)` needs no unknown-type catch merely because the definition is generic. Ordinary CoD library definitions should progressively express readable generic type/operator requirements; their syntax, library contents and IDE integration remain deferred and are not migration prerequisites.

### Recursive coverage obligations

Before executing an artifact, establish finite native coverage for every admitted semantic method, demanded general/specialized implementation, local operation and required check/failure path. Runtime data, argument values, activation depth, owner instances, current TLS contents and capture allocation addresses do not automatically create specializations. Required owner roles, access, capture schemas and selected interpretations still constrain each path. Runtime recursion with fresh captures must preserve each complete value/actual owner while reusing suitable native bodies.

Ordinary and mutual recursion use checked call-graph back edges rather than one body per activation. Preserve distinct generic types, substitutions, units, nominal relations and named recursive graph edges without equating equal-layout types or emitting a native body for each substitution. Type descriptions can grow while shared native behavior remains finite; growth alone implies neither acceptance, rejection nor cloning. A compiler may still demand/reuse a specialization from proven call-site facts. The general path must remain correct without mandatory specialization; the [method hierarchy](#callable-method-and-native-implementation-hierarchy) and [phase table](#phase-and-initiator) distinguish semantic methods, prepared implementations and runtime local selections.

Establish declared or independently inferred interfaces under [D-24](CallableResultInference.md) before accepting recursive components. An already-seen node does not prove its body, effects, result, ownership or completion. Independently supplied contracts and body anchors are valid; unresolved recursive result cycles need an explicit contract. A unique least common result uses existing permitted conversions, without circular guesses or owner unions. Cover every admitted callback, branch and deferred action; only proved impossibility can remove an obligation, not an observed input or a heuristic optimization.

[D-28](RegionalAndLexicalStorageRefactor.md#global-contract-and-source-impact) requires sufficient actual layout and shared owner-resolution evidence for every admitted lexical/regional ADT path, including calls, returns and cleanup that may relocate backing. Layouts do not become semantic types; owner contexts do not become concrete-type identifiers. H-439/H-440 target identity and null comparison do not interpret payloads and add no unknown-type obligation solely for that comparison; operand evaluation retains its ordinary obligations. This grants no new placement, graph copying or physical carrier.

The checked/emitter boundary must carry sufficient result, owner, selected context and completion evidence independently of native addresses or caller activation state. Algorithms, recursive analysis domains and permanent checked representations remain separate review scopes. No alternative unqualified semantics or fallback to runtime compilation can stand in for established coverage.

### Analysis limits and concrete diagnostics

Failures to establish coverage must identify their actual basis:

1. **Semantic error:** an established violation such as incompatible ownership, unavailable applicability or incomparable best methods.
2. **Detected resource limit/exhaustion:** stop compilation and report the exhausted resource/allowance and active source obligation. This is not proof of invalid source or inevitable analysis divergence.
3. **Identified implementation gap:** a concrete required construct/operation is unsupported. Report the capability gap, not a new semantic category or undecidability claim.

Use justified terminating procedures where available and operational resource bounds where a search lacks that guarantee. No universal detector of arbitrary analysis nontermination is required. A limit cannot justify truncating coverage, silently weakening a contract, or emitting an arbitrary StaticallyUnknownTypeException/UnsoundRuntimeTypingError as successful coverage. Resource allowances and formatting are engineering details; semantic acceptance changes require design review.

Runtime program termination is separate: an explicitly typed `forever` calling itself can reuse its checked interface/native body without executing it during compilation or proving it will return. Runtime stack/resource behavior remains an execution concern. A schematic `f<T> → f<Container<T>> → …` type sequence can be infinite while native behavior is finite; the actual operation, type-relation and owner obligations decide coverage.

### Acceptance for generic and recursive coverage

Each claimed supported path must meet the following observations. Concrete unsupported paths remain diagnosed and recorded in the source capability reference; this contract does not require every path to be implemented at once. Use source programs and actual native execution/method counts as evidence, with the isolated finite experiment only supporting its stated scope.

| Case | Required observation |
| --- | --- |
| Direct and mutual recursion | Correct native result and cleanup; increasing runtime depth does not add native method bodies. |
| Generic calls with different type arguments | Preserve identities and relations through shared method bodies; declaration/traversal order does not change results or admitted coverage. Substitutions alone do not require body clones. |
| Known concrete callable types | `func() -> const u64` and `func() -> const bool` retain distinct identities through generic storage, selection and return. |
| A type outside the static catalog | Select through `<any>` while preserving actual evidence where needed; an uninterpretable required operation raises checked `StaticallyUnknownTypeException`. Cover both a required checked obligation and a proven-known path without it. |
| Uninterpreted passage | With a valid borrow/carrier, `marker` returns `1u64` for an outside type without an unknown-type exception or mandatory catch. Forwarding preserves identity/owner evidence; a downstream operation requiring unavailable interpretation still raises the checked failure and contributes its obligation. |
| Same-T and violated typing evidence | A known mismatch is a compile error. Required runtime checks use actual identities; a typing breach raises unchecked `UnsoundRuntimeTypingError`, without a mandatory catch obligation at every call. |
| Generic operator body | Only applicable `(+)` methods participate; unsupported known types do not acquire a universal operator or hidden conversion. Formal library contracts remain deferred. |
| Generic definition versus explicit throws | `twice` needs no StaticallyUnknownTypeException declaration merely for its generic body; a known supported `u64` call has no such catch obligation. A voluntarily declared StaticallyUnknownTypeException retains the caller's checked handling/propagation obligation even for that call. Genuine unknown interpretation paths remain checked. |
| Recursive type descriptions | Finite named graph and valid regional references; no by-value expansion or regional copy/move. Invalid layout/reference cycles remain separate errors. |
| Abstract arguments and results | A real CoD source program preserves actual concrete evidence through calls, storage and return, selecting the unique best method at runtime. |
| Independent polymorphic fields | Each field retains its own alternatives; different enclosing layouts use local access; a joint operation retains the checks it actually needs. Body count is not the field Cartesian product. |
| Owners and changing captures/context | Native recursion preserves fresh captures, selected/current interpretation, actual borrowed owners and access; expired or incompatible owners reject. |
| Exceptions and deferred actions | All reachable actions contribute obligations; ordered cleanup, recovery, explicit returns and exceptional release preserve the accepted contract. |
| Growing type descriptions | Shared native coverage where its obligations can be established; growth alone neither clones methods nor rejects the program. A failure identifies a semantic violation, a detected resource limit or a concrete implementation gap. |
| Deliberately inadequate work allowance | A resource-limit diagnostic and stopped compilation, without a claim of semantic invalidity or inevitable divergence. Increasing an allowance cannot change the meaning of an established program. |
| Runtime nontermination | Check an explicitly typed recursive call graph without proving runtime termination or expanding one body per activation. No universal analysis-nontermination detector is assumed. |
| Call-site specialization and general-path control | A proven call such as `twice(3u64)` demands/reuses an appropriate AOT specialization and selects it directly. General-path execution with specialization disabled remains correct and complete. A specialization of a generic method does not override a more-specific user method. |
| Multiple local dispatch points | Independent x/y accesses and a genuinely joint operation select locally within a generated body. Partial specialization can resolve some points and retain others without mandatory whole-body Cartesian cloning. |
| Definition, compilation and runtime initiators | Distinguish user definition/compile-time computation from compiler-driven checking/specialization and runtime invocation/binding changes. Runtime effects are neither executed by body analysis nor registered as specializations. |

### Separate implementation and review boundaries

The permanent analysis procedure/domain, operation-specific interpretation summaries, relational evidence transport, checked-method/value/emitter representation, carrier/ABI and input-validation protocols remain separately reviewed. D-24 owns result inference; D-17 owns checked declarations and failure behavior. General implementations need not be represented permanently as “specializations with no extra assumptions.” Generic library contracts/IDE support remain deferred. No universal generic solver, analysis-nontermination detector, per-type native cloning or runtime code generation is required. Correctness and honest capability limits remain mandatory for claimed supported source paths.

## Selected worlds and required dynamic context

The prepared method/interpretation boundary is relative to one selected artifact; preparing another composition does not silently mutate an in-flight one. Values with outside-catalog types do not extend its executable method set. Independent module worlds retain their definitions, interpretations and visibility. They do not mutate a source world or create a process-wide latest method set. Preserve the established base/extended/indirect `1,7,1` behavior and existing name/identity-alias, using/import and mixin rules. Fusing a selected artifact includes reachable dependency-local interpretations and procedure-local effects; it does not reduce every call to the entry module's view.

Conceptually invoke a method with explicit arguments and its required implicit dynamic-context projection. Include dependencies through admissible callees, touch and deferred/cleanup actions. An empty projection needs no dynamic-context input; lexical captures and explicitly passed memory remain separate. Functions retain their existing restrictions on dynamic effects.

Current/behave calls use current required bindings. Perform replays complete captured values for its captured bindings and obtains remaining requirements from the invocation context, with scoped restoration. Copying a callable preserves its complete environment; replacing its source holder cannot change that copy. Borrowed referents retain their actual owners and permitted mutation remains observable. Touch can require capture/dependency evidence without a current-value read. Context projection does not freeze the whole TLS world or put every owner/binding into a specialization key.

TLS slots, explicit machine arguments, registers and elimination are physical choices. The fixture uses explicit projected family values plus TLS adapters and conservative supplied dependency contracts. General effect inference, nested capture replay, touch owner validation and arbitrary complete environment schemas remain open; sharing native bodies does not remove these checks.

## Definition, publication and installation

Selecting a symbol or publishing its method family has no runtime installation effect. An effect with a local method or a [grouped installation](SymbolicDynamicScope.md#installing-a-local-family-with-several-methods) activates its own anonymous family. Check the reachable methods’ lexical dependencies and full coverage of the symbol’s callable contract. Assignment-installing a separate definition, lexical callable or active handler rejects. The symbol remains available for static family selection and type checking. An entry can execute `effect proc @p;` to activate its composed definitions. A direct lexical `@p(...)` leaves the entry’s installation obligation intact. [Definition-installed effectful procedures](SymbolicDynamicScope.md#definition-installed-effectful-procedures)

Export publishes the whole method set. A skipped branch installs nothing. Repeated execution creates distinct activations without registering methods again. Selecting a dependency retains its required once-only initialization.

### Entry installation and unused dependency code

An entry module extending a dependency’s callable must explicitly install the extended family with `effect proc @p;`. A local body at the same symbol supplies a different family and cannot discharge that obligation. With the required entry installation, suppress only affected dependency-module-initialization TLS writes. Retain operand evaluation, callable construction, effects, failures and temporary cleanup in their normal order. Suppressed writes create no installation/restoration event. Procedure-local definition installations and permitted dependency calls remain usable.

CoD must expose correct roots to LLVM so genuinely unused dependency-world code can be excluded from the final image. Removing a write alone does not prove code elimination. Operand effects or retained source views can keep code reachable. Native reachability/linkage and final-image verification remain production work; the fixture checks the installation boundary, not a complete module composer/linker implementation.

**D21-Q-entry-composition:** alias/extension identity matching, conditional entry-installation coverage, binding availability and dependency encounter order still require concrete mechanisms. Do not treat a future entry write as already executed or erase operands to make the code graph smaller.

### Nearest bindings and inferred owners

Resolve the declared symbol once in its lexical context. Bare `p` provides its nominal identity at runtime. A supported `@p` value read uses that declaration’s active binding. Reject a parameter or local that would hide the symbol when the name is introduced. The same identity governs effects, touch, calls and type inspection. A separately defined callee keeps its definition-site resolution.

Each inferred owner binder is an implicit const lexical binding, including generic region-or-scope owners. It participates in the same lookup rule. Its constness grants neither region capability, content immutability, ownership nor lifetime extension. Preserve actual owner provenance; no runtime owner object or owner-union ABI is selected.

## Frozen inheritance constraints

H-528 freezes inheritance before execution. H-534's disjoint exemptions are existing ancestry among listed endpoints, already represented by the retained type graph; no copied per-declaration path graph or grandfathering of arbitrary old common conformers is required. H-535 rejects a subtype's second independent listed-parent relationship; H-530 accepts only already established redundant relationships with a warning. A new relation cannot justify its own admission or retroactively make an earlier independent relationship redundant. Validate relevant local/selected-source records and relations during static preparation, retaining their existing semantic dependencies. An ancestor exemption is not an empty-intersection proof. H-533's per-world shared pool is a candidate storage organization, not a complete applicability rule or new runtime lookup requirement. H-530/H-531 shorter declarations warn without removing intermediate method contracts. [D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) and [D-15](ModuleWorlds.md#shared-disjoint-information-per-module-world) own acceptance and remaining proof/representation boundaries; D-26's outside-catalog interpretation contract remains separate.

## Completion, validation and remaining work

Native body sharing must preserve return/error state and ordered cleanup. Under D-17's H-503/H-505/H-507–H-509/H-541 direction, only bodyless deferred handlers inspect incoming errors FIFO at their own list position; outgoing failures join the tail for later entries. Attached catches handle only their own body; ordinary body/startup errors are excluded from the deferred queue. Optional finally runs once per entry, even when its catch handles several errors. Explicit returns still target the enclosing callable and remaining cleanup must run; later returns replace retained results without erasing unrelated queued failures. D-17-RETURN-STATE owns explicit body-error recovery, ordinary cancellation and locally preserved finally completion. Remaining ownership/pattern and queue integration retain their separate scopes. Recovery without a required return retains missing-return checking. The fixture checks bounded finite completion categories with real LLVM resource acquisition, verifier failure and ordered release; it does not establish the new queue policy. General exception payload graphs and full deferred-handler integration remain unimplemented. H-510–H-515 additionally requires traces to be captured at new exception construction and preserved by ordinary propagation/copies; moving capture to a later throw is invalid. Frame detail/reconstruction and OOM diagnostic storage remain separately reviewed, and current source-site records do not prove this trace contract.

The [focused evidence](../experiments/micro-dispatch/README.md) covers runtime-only field combinations, abstract forwarding/storage/results, no-field method selection, crossed specificity and rejection, independent layouts, call-site narrowing, invalid/mismatched discriminator rejection and separate recursive/shared root-checksum planning, updates through shared target references, actual owners/remapping, unchanged saved data across separately compiled worlds, selected contexts and native cleanup. It supplies a bounded native implementation from explicit finite contracts, not a full source compiler.

Phase-sensitive acceptance distinguishes an emitted direct call for proven facts from a prepared selector whose remaining choice uses runtime evidence. Independent local selections rejoin shared control flow without registering native implementations. Compilation of source call sites and bodies must not execute their runtime effects; explicit user compile-time computation retains its separate contract. These observations apply to claimed supported paths and do not expand the bounded experiment’s implementation claims.

The remaining delivery topics retain their stable identifiers:

| Topic | Required production contract |
| --- | --- |
| DD-AOT-01 / DD-AOT-08 | Keep behavioral qualification and new candidate paths distinct; align dependent design summaries. |
| DD-AOT-02 / D21-Q-entry-composition | Selected family/composition identity, revision dependency and conditional installation coverage. |
| DD-AOT-03 | Generic and recursive native coverage with concrete semantic-error, resource-limit and implementation-gap diagnostics without mandatory tuple expansion. |
| DD-AOT-05 | Permanent evidence/carrier/ABI, admission, bits and borrowed-value transport with actual owners. |
| DD-AOT-07 | Namespace/staging, grammar migration and established missing/ambiguous-call failure contracts. |
| DD-AOT-04 / DD-AOT-06 | Complete family/capture values and a thin native emitter consuming checked operation contracts. |
| DD-AOT-09 | Broader source-language integration, complete context/owner/completion checking and final-image reachability. |
| DD-AOT-10 | Evaluate costs when workload evidence makes a trade-off consequential; no benchmark matrix is a prerequisite here. |

Preserve method/callable/source associations for later profiling. ORC is reserved for future tracing-JIT hot paths; no profiler, trace compiler, guards, OSR, deoptimization, code-lifetime protocol or runtime code generation belongs to this baseline. No performance class follows from correctness traces or branch/body counts. Fraction arithmetic/promotion and its open design questions remain deferred.
