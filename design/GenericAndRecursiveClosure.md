# Generic and recursive closure for AOT micro-dispatch

Design revision: 8 · Recorded: 2026-11-13 America/Los_Angeles · Status: whole-file behavioral/native-coverage contract and value path confirmed; concrete mechanisms retain the separate review scopes below.

## Design authority

- **Semantic definitions and prepared implementations:** [H-409](DesignAuthorityRegistry.md#h-409)/[K-594](DesignAuthorityRegistry.md#k-594) clarifies callable → user method → compiler specialization, AOT call-site demand and local micro-dispatch. It preserves H-407’s coverage/correctness contract while ruling out a universal one-native-body interpretation. C-86/C-89 and their recorded value paths retain selected semantic meaning and prepared native coverage; no permanent implementation representation is selected.

- **Record and scope:** [D-26](DesignAuthorityRegistry.md#d-26), revision 8, all sections within the stated behavioral/native-coverage scope. [H-407](DesignAuthorityRegistry.md#h-407) records the user's conditional whole-file approval, fulfilled by the review of revision 6; [K-593](DesignAuthorityRegistry.md#k-593) continues that baseline through this administrative qualification update. H-400–H-402/H-404–H-406 supplies the type-object, same-T, generic, passage, throws and resource-stop decisions. H-398/H-399 governs source migration and its removal-first stage; H-408 reinforces practical progress with explicit capability gaps.
- **Confirmed value path:** C-89/P-371 derives from C-83/P-334 → C-02: checked finite native coverage makes abstract calls executable without mandatory whole-method tuple cloning. C-90/P-374 relates identity-preserving `<any>` selection and explicit failures to C-82/C-83 and their C-01/C-02 paths. P-372/P-375 organize those responsibilities here. Charter §§2–3, 5.1, 5.3–5.4, 6.3, 7.1, 8–9 and 12 require inspectable checked meaning, distinct identity/representation, explicit uncertainty and native evidence. H-407 confirms these relationships for the stated requirements and acceptance, without selecting the excluded mechanisms.
- **Governing contracts:** [D-21](AheadOfTimeMultipleDispatch.md) preserves unique-most-specific selection, shared method bodies, actual evidence and selected interpretations. P-373 records dependencies on D-07/D-10/D-11/D-14/D-15/D-16/D-17/D-18/D-21/D-24. Owners, conformance, complete captures, dynamic context and completion remain separate proof obligations. Dependency links confer no additional authority.
- **Invariants and exclusions:** ordinary runtime recursion remains supported; values, actual owner instances and distinct type arguments do not automatically require new native bodies. `<any>` has CoD type-object identity; no permanent discriminator encoding, universal payload layout, Julia `Any` semantics, runtime compiler, new method loading, result-inference policy, fraction implementation or toolchain change follows. This contract does not promise a universal proof solver.
- **Acceptance:** each claimed supported source-to-native path must establish semantics and finite native coverage with optional specialization disabled, including its specified runtime failures. A resource limit is a diagnostic safeguard, never proof. Stage completion follows the bounded criteria below; this document claims no implementation result.

## The question this contract answers

Before executing a selected artifact, the compiler must establish native coverage for its admitted methods and local operations, including required checks and failure paths. Recursion is allowed. The coverage graph concerns native behavior; it need not enumerate every precise type description that execution can construct. Runtime depth and the number of distinct type descriptions are not native body counts.

| Situation | Required treatment |
| --- | --- |
| Ordinary or mutual recursion | Represent calls as back edges to shared methods. Check the recursive component against its contracts; do not expand one body per activation. |
| Different generic arguments or recursive type descriptions | Preserve distinct type identities and relations while reusing method bodies and local operation paths. Named recursive references remain graph edges. A substitution is not automatically a native-instance key. |
| A runtime type absent from the static fused world | Use the CoD `<any>` selection boundary below. An operation unable to interpret that type raises checked `StaticallyUnknownType`; no hidden runtime compilation follows. |
| Detected analysis resource exhaustion or an identified unsupported operation | Stop compilation and report the concrete limit or implementation gap. Type growth alone establishes neither condition; finite guarded native behavior may cover it. |

Growing runtime data, captures, argument values and call depth belong to execution. They are not expanding generic instantiation. For example, recursive calls carrying freshly captured `@bias` values must reuse native code while retaining each complete capture and actual owner. An implementation that mistakes those values for new semantic contexts has not satisfied this contract.

## CoD `<any>` and concrete type identity

`<any>` denotes a real CoD type object with its own identity in the fused world. It gives the shared generic selection path a stable type-level referent. It is not Julia's built-in `Any`; this decision imports no top-type, subtyping, boxing or representation rules from another language. The spelling `<any>` is discussion notation here, not a selected source declaration syntax.

Concrete known types keep their own identities. In particular, `func() -> const u64` and `func() -> const bool` remain distinct callable types. A generic `func() -> const T` must have a stable type-level interpretation without relabelling both concrete signatures as the same actual type. The exact encoding of binders, type applications and persistent identities remains open.

Let K denote the fused world's complete static type catalog. The known possible concrete alternatives considered through `<any>` come from all statically known types, then ordinary applicability and proven facts narrow a particular operation. K is not a claim that no other runtime type can occur. For method selection, a type absent from K uses the `<any>` path; its actual identity must remain available wherever a later constraint or operation needs it. The projection used for selection does not prove two unknown types equal or give them the same payload layout.

The selection boundary does not define an operation accepting every type. Each method retains its signature, conformance, selected interpretation and other constraints. The applicable/best-method rules still govern candidates; a known missing or ambiguous method does not become an unknown-type failure merely because generic code is involved.

When execution encounters a type outside K that the required operation cannot interpret, it raises **checked `StaticallyUnknownType`**. Static analysis identifies the paths where this is possible and applies checked-exception handling obligations. Proven known-type paths need no unknown-type obligation. Existing D-17 checked propagation rules provide the surrounding model; the exact operation summaries and source examples for propagation remain to be specified. The name is provisional, selected for implementation rather than left unnamed.

An unknown type may pass through an operation that does not need to interpret its contents. Mere receipt or forwarding does not raise `StaticallyUnknownType` or create a checked obligation solely because the type is absent from K. The failure occurs when an operation requires an interpretation that the prepared artifact cannot supply. Actual type identity and owner evidence remain available; borrowing, access and lifetime obligations still apply.

For example, assuming `x` arrives through a valid borrow, this CoD review fragment can return `1u64` even if its actual type is outside K:

```cod
func marker(x: T) -> const u64 where T {
    return 1u64;
}
```

By contrast, evaluating `x + x` requires resolution and execution of `(+)`; if the required operation cannot interpret an outside type, that operation raises `StaticallyUnknownType`. Checked obligations follow genuine unknown-type interpretation paths and explicit exception contracts. The presence of `T` or a generic operation alone does not establish such a path or require a throws annotation. Exact analysis and attribution of real failing paths across calls remain implementation/design work.

The generic `twice` example below does **not** require `throws StaticallyUnknownType`. With a known supported `u64` addition, `twice(3u64)` acquires no unknown-type handling obligation merely because the method is generic. A programmer may voluntarily declare `throws StaticallyUnknownType`; that explicit declaration then retains D-17's fixed caller obligation, including for a known-type call whose body would not throw. No conditional throws syntax, optimizer-dependent declaration waiver or relaxation of declared checked exceptions is required for this example.

The permission to pass an uninterpreted value assumes a valid carrier/reference and established lifetime. It does not waive input validation, authorize an unknown-layout payload copy or allow opaque dereferencing or newly loaded method execution. Carrier protocols and operation-specific validation requirements remain separate design work. Fixed prepared method definitions, selected interpretations and actual owners constrain this boundary. D-15/D-21/D-25 share this input boundary; historical finite-catalog fixture evidence does not restrict it.

## Same-T and runtime soundness

Repeated use of one type parameter retains its relation. For example, this CoD review fragment requires the two arguments to satisfy the same `T`:

```cod
func first(x: T, y: T) -> const T where T {
    return x;
}
```

The compiler reports a type mismatch whenever it can establish one statically. Where runtime evidence is needed to check the established same-T contract, compare the actual type evidence, not merely the shared `<any>` selection identity. Two distinct runtime types do not satisfy same-T just because neither belongs to K. This rule does not authorize arbitrary heterogeneous calls as ordinary well-typed input.

If runtime checking detects a violation of typing that the accepted program was meant to satisfy, raise **unchecked `UnsoundRuntimeTyping`** before the dependent unsafe operation. Its role is a soundness backstop, not a recoverable business input alternative. It adds no mandatory catch or throws declaration at every guard or generic call. Unchecked classification does not remove the guard or D-17 cleanup behavior. The name is provisional.

The distinction is intentional: an uninterpretable type outside the selected static catalog is a modeled checked failure; a breached established typing contract is an unchecked failure. A compiler's inability to finish its analysis is neither one automatically. No explicit dynamic-cast API or general input-validation error taxonomy is selected here.

## Generic operations and deferred library contracts

CoD allows a generic body to express operations directly:

```cod
func twice(x: T) -> const T where T {
    return x + x;
}
```

This review fragment requires a suitable `(+)` method and a result satisfying the declared return contract. It does not imply a universal `(+)` accepting arbitrary types. Known unsupported demands receive the ordinary applicability/type diagnostic; the runtime unknown-type boundary has the separate behavior above. No additional implicit capability or trait mechanism is selected by this example. Review fragments describe intended semantics, not current source-compiler availability.

Formal type/operator contracts should progressively make generic requirements readable and navigable in the IDE. The selected direction is ordinary CoD library definitions distributed in the manner of a standard library, without introducing a special intrinsic mechanism for these contracts. The analogy to templates describes permissive source use, not mandatory C++-style native instantiation. Contract syntax, declaration semantics, library contents and IDE implementation are **deferred**, with no current migration prerequisite.

## Native coverage responsibility

The required boundary is finite native coverage over semantic methods, their demanded prepared implementations, local operation paths and required checks. One user method has one semantic definition but may have multiple AOT specializations. A type substitution does not universally require a new body; proven call-site facts should nevertheless demand and statically select an appropriate specialization for straightforward calls when the semantic method and interpretation are established. A broad declared parameter remains broad. Concrete evidence selects applicable methods or local operations, and the `<any>` path accounts for types outside K without claiming their concrete native implementations exist.

[D-21](AheadOfTimeMultipleDispatch.md#callable-method-and-native-implementation-hierarchy) owns the semantic-method/implementation hierarchy and phase/initiator table. A compiler specialization is not a user overload. Compiler analysis of a call or body is not execution of its runtime effects, and runtime installation/capture does not register methods or specializations. Each generated body can contain multiple independent local micro-dispatch points; choosing a local branch does not create a whole-body version.

Actual argument values, owner identities, current TLS contents, capture allocation addresses and activation depth are not automatically specialization keys. Owner roles, access requirements, capture schemas and required dynamic bindings still constrain checking and transport. Omitting runtime values from the key does not authorize joining incompatible owners or erasing a required interpretation.

The closure procedure must account for all admitted alternatives and branches, including callbacks and deferred actions. A currently observed input or optimizer-eliminated example path is not a coverage proof. Statically established impossibility may remove an obligation under the language's checking rules; a heuristic may not.

For recursion, publish the declared interface before checking the body, record recursive edges, and establish the mutually dependent obligations before accepting the component. Merely encountering an already-seen node does not validate its body, effects, result, ownership or completion. Analysis uses justified terminating procedures where available and operational resource bounds on searches without such a guarantee. It must not silently widen away a requirement or assume a general detector of future nontermination. The concrete analysis algorithm remains unselected.

Checking may retain distinct substitutions and relational facts without emitting a body for each one. The compiler must not identify types solely because layouts match or both select through `<any>`. Units, nominal conformance and selected interpretations remain meaningful; D-16's planned numeric representations cannot be collapsed into one type by this mechanism. The concrete analysis domain and checked representation remain unselected mechanisms. Representing the general implementation as a specialization with no extra narrowing assumptions is an engineering proposal only; the user has not selected that permanent representation.

This first contract does not define omitted-result inference. Explicitly declared recursive interfaces provide the initial acceptance anchor. [D-24](CallableResultInference.md) separately governs the pending source policy and unanchored result cycles. Existing inferred-result behavior is inventoried during migration; it is neither silently ratified nor globally removed by this contract.

## Analysis limits and concrete diagnostics

Native emission requires the necessary static checks and runtime-check paths to be established. Diagnosed failures identify a concrete reason:

1. **Semantic error:** a specific violation is established, such as incompatible ownership, missing applicability or incomparable best methods. Report that violation.
2. **Resource limit/exhaustion:** a measurable compiler limit is reached or resource exhaustion is detected. Report the exhausted resource or allowance and the active source obligation, then stop compilation. This does not prove that the source program is invalid or that analysis would never finish with more resources.
3. **Identified implementation gap:** a concrete required construct or operation is outside the current compiler's supported implementation. Report that gap under H-399's stage policy. This is an implementation-capability diagnostic, not a third semantic classification of programs or a claim of undecidability.

There is no general algorithm that decides in advance whether an arbitrary analysis will eventually finish. The compiler is not required to detect that it will never complete and report a universal `unproved` outcome. A supported bounded procedure can nevertheless stop with a specific failed obligation or implementation limitation; such a diagnostic must state its actual basis. Halting undecidability does not make known unsupported operations or exhausted allowances undetectable.

Runtime program termination is separate. A recursive call can reuse a checked interface and existing native body even if the program never returns:

```cod
func forever(x: u64) -> const u64 {
    return forever(x);
}
```

The compiler need not execute this function or prove its termination to check the declared result contract. Runtime stack/resource behavior remains a separate execution concern.

Exact analysis allowances and diagnostic formatting are engineering details; changes to semantic acceptance require review. A limit never justifies truncating coverage. There is no fallback to runtime compilation or mandatory specialization. Incomplete compiler reasoning cannot be replaced with an arbitrary `StaticallyUnknownType` or `UnsoundRuntimeTyping` throw and reported as successful coverage.

An example expansion `f<T> → f<Container<T>> → …` is schematic, not proposed source syntax. Its precise type sequence can be infinite while its shared native behavior is finite. The compiler must examine the actual obligations, including `<any>` selection, type relations, operations and owners. Whether a concrete example satisfies those obligations remains a proof question; neither automatic native cloning nor automatic rejection follows from the sequence alone.

## Migration and interface constraints

The preserved source compiler at the local `multi-dispatch` tag interleaves semantic checking, caller-context comparison and LLVM emission. `Value` contains LLVM values/places; `NativeCallDemand` combines checking state and native definitions; `compile()` resolves and emits through a shared refinement loop. `record_field()` needs a concrete product layout, and `LLVMEmission::type_of()` lacks a general abstract carrier. These are baseline observations, not requirements for the replacement; the removal-first branch does not retain those paths.

The migration must separate semantic method/operation obligations from emitted addresses and caller activation state. A checked boundary must carry sufficient result, owner, context and completion information for a thin emitter. This document fixes the closure responsibilities; the concrete checked representation, carrier and persistent identity encoding require their own review before being fixed.

H-399 requires removal of debt-producing historical checking/emission/context-specialization machinery first, followed by implementation on a coherent foundation. Temporary feature loss and known failing legacy cases are permitted during this restructuring stage. Keep a supported/missing-path inventory; preserve the confirmed language contracts as the target. The compiler must not retain compatibility shims, parallel semantics or an old-specialization fallback merely to preserve coverage. The preserved tag supplies recovery and comparison evidence.

## Required acceptance

The table specifies evidence for each claimed supported path. H-399 does not require all paths to be implemented or all legacy tests to pass in the current restructuring stage. Stage completion requires a coherent foundation, identified debt removed, correct retained/new paths and honest capability gaps. Missing implementation remains distinct from semantic invalidity or a proved coverage result.

| Case | Required observation |
| --- | --- |
| Direct and mutual recursion | Correct native result and cleanup; increasing runtime depth does not add native method bodies. |
| Generic calls with different type arguments | Preserve identities and relations through shared method bodies; declaration/traversal order does not change results or admitted coverage. Substitutions alone do not require body clones. |
| Known concrete callable types | `func() -> const u64` and `func() -> const bool` retain distinct identities through generic storage, selection and return. |
| A type outside the static catalog | Select through `<any>` while preserving actual evidence where needed; an uninterpretable required operation raises checked `StaticallyUnknownType`. Cover both a required checked obligation and a proven-known path without it. |
| Uninterpreted passage | With a valid borrow/carrier, `marker` returns `1u64` for an outside type without an unknown-type exception or mandatory catch. Forwarding preserves identity/owner evidence; a downstream operation requiring unavailable interpretation still raises the checked failure and contributes its obligation. |
| Same-T and violated typing evidence | A known mismatch is a compile error. Required runtime checks use actual identities; a typing breach raises unchecked `UnsoundRuntimeTyping`, without a mandatory catch obligation at every call. |
| Generic operator body | Only applicable `(+)` methods participate; unsupported known types do not acquire a universal operator or hidden conversion. Formal library contracts remain deferred. |
| Generic definition versus explicit throws | `twice` needs no StaticallyUnknownType declaration merely for its generic body; a known supported `u64` call has no such catch obligation. A voluntarily declared StaticallyUnknownType retains the caller's checked handling/propagation obligation even for that call. Genuine unknown interpretation paths remain checked. |
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

Use the existing source fixtures as behavioral evidence, not a substitute for their governing design decisions. The isolated authored-contract experiment is supporting evidence only. Focused compiler validation must include emitted method counts and actual native execution; the supported/missing inventory must account for displaced paths without weakening their assertions or claiming that a missing feature is forbidden by the language.

## Remaining implementation and separate review topics

The `<any>` type-object direction, distinct known identities, the two named failure classifications, same-T preservation, uninterpreted passage with failure at a required unavailable interpretation, no automatic throws obligation for the generic `twice` example, preserved voluntary throws obligations, stopping on detected resource exhaustion, and deferred library-contract direction are locally decided. Remaining topics are the concrete checking and recursive analysis procedures; operation-specific interpretation requirements and analysis of genuine unknown-type paths across calls; relational evidence through storage/calls; and the checked-method/value/emitter interfaces. Permanent representation/ABI, input-validation protocols and D-24 result inference retain their separate review scopes.

The confirmed contract adopts the finite native-coverage responsibility and concrete failure diagnostics, with the decisions above and explicit limits on what is still unselected. The compiler must preserve runtime recursion and abstract behavior while establishing native coverage for supported source paths. Neither a universal generic solver nor an analysis-nontermination detector is a migration prerequisite. Native emission depends on checked contracts for its supported paths.

Whole-file approval covers the stated value path, scope, initial proof boundary and acceptance criteria. It does not choose the permanent discriminator/ABI, omitted-result rules, a universal algorithm or any excluded language feature. Those remain explicit follow-up topics. Routine implementation within qualified scope proceeds without repeated confirmation; a new semantic boundary or material unresolved design choice retains its own review.
