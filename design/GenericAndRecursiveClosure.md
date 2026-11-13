# Generic and recursive closure for AOT micro-dispatch

Discussion revision: 2 · Recorded: 2026-11-13 America/Los_Angeles · Status: pending topic discussion and whole-file approval.

## Design authority

- **Record and scope:** [D-26](DesignAuthorityRegistry.md#d-26), revision 2, all sections. [H-398](DesignAuthorityRegistry.md#h-398) authorizes the source-compiler migration and baseline preservation, not a new generic acceptance rule. There is no human-confirmed baseline for this proposed closure contract. [K-586](DesignAuthorityRegistry.md#k-586) records the draft and unaffected scopes.
- **Candidate value path:** C-89/P-371 derives from C-83/P-334 → C-02: proving a closed native implementation graph makes abstract calls executable before they run, without requiring whole-method tuple cloning. P-372 organizes that value in this contract. Charter §§2–3, 5.3–5.4, 6.3 and 12 require inspectable checked meaning, explicit uncertainty and end-to-end native evidence. These proposed relationships require human confirmation.
- **Governing contracts:** [D-21](AheadOfTimeMultipleDispatch.md) preserves unique-most-specific selection, shared method bodies, actual evidence and selected interpretations. P-373 records dependencies on D-07/D-10/D-11/D-14/D-15/D-16/D-17/D-18/D-21/D-24. Owners, conformance, complete captures, dynamic context and completion remain separate proof obligations. Dependency links confer no additional authority.
- **Invariants and exclusions:** ordinary runtime recursion remains supported; values and actual owner instances do not themselves require new native bodies. No hidden old-compiler fallback, runtime compiler, blanket ban on generic recursion, new type identity, permanent ABI, result-inference policy, fraction implementation or toolchain change follows. This contract does not promise a universal termination solver.
- **Acceptance:** the source-to-native cases below must establish both semantics and a finite implementation graph with optional specialization disabled. A resource limit is a diagnostic safeguard, never evidence that a graph is closed.

## The question this contract answers

Before executing a selected artifact, the compiler must know that every admitted method and local operation has a checked native implementation. Recursion is allowed. The relevant question is whether preparation needs finitely many distinct semantic implementations and whether their contracts can be established, not whether execution has a fixed depth or terminates.

| Situation | Proposed treatment |
| --- | --- |
| Ordinary or mutual recursion over fixed method/type instances | Represent calls as back edges to existing method instances. Check the recursive component against its declared contracts; do not expand one body per activation. |
| Finite generic instantiation or finite recursive type descriptions | Instantiate only distinct required semantic instances. Named recursive references remain graph edges, not endlessly expanded trees. Prove obligations for each admitted instance and close the resulting finite graph. |
| Successively expanding type applications or required semantic interpretations | No closure claim follows merely from a finite set of nominal declarations. Establish a sound finite representation/proof if supported; otherwise report that native closure is unproved, with the originating demand chain. |

Growing runtime data, captures, argument values and call depth belong to execution. They are not expanding generic instantiation. For example, recursive calls carrying freshly captured `@bias` values must reuse native code while retaining each complete capture and actual owner. An implementation that mistakes those values for new semantic contexts has not satisfied this contract.

## Proposed initial admission boundary

The initial migration accepts a reachable finite graph of required method instances and local operation obligations whose complete contracts have been checked. An instance is distinguished by the selected method definition, explicit type substitution and required semantic interpretation. A broad declared parameter remains broad; concrete runtime alternatives select methods or local operations without automatically cloning the whole body.

Actual argument values, owner identities, current TLS contents, capture allocation addresses and activation depth are not native-instance keys. Owner roles, access requirements, capture schemas and required dynamic bindings still constrain checking and transport. Omitting runtime values from the key does not authorize joining incompatible owners or erasing a required interpretation.

The closure procedure must account for all admitted alternatives and branches, including callbacks and deferred actions. A currently observed input or optimizer-eliminated example path is not a coverage proof. Statically established impossibility may remove an obligation under the language's checking rules; a heuristic may not.

For recursion, publish the declared interface before checking the body, record recursive edges, and establish the mutually dependent obligations before accepting the component. Merely encountering an already-seen node does not validate its body, effects, result, ownership or completion. Summary refinement must be sound and converge over a justified finite domain, or finish as unproved. It must not silently widen away a requirement.

For finite generic calls, different explicit type substitutions may need different general native instances. This is semantic instantiation, not optional call-site specialization. The compiler must not identify substitutions solely because layouts match. Units, nominal conformance and selected interpretations remain meaningful; D-16's planned numeric representations cannot be collapsed into one type by this mechanism.

This first contract does not define omitted-result inference. Explicitly declared recursive interfaces provide the initial acceptance anchor. [D-24](CallableResultInference.md) separately governs the pending source policy and unanchored result cycles. Existing inferred-result behavior is inventoried during migration; it is neither silently ratified nor globally removed by this contract.

## Unproved is distinct from invalid

The compiler distinguishes three outcomes:

1. **Established:** all required definitions, local alternatives and checking obligations close; native emission may proceed.
2. **Invalid:** a concrete semantic violation is established, such as incompatible ownership, missing applicability or incomparable best methods. Report that violation.
3. **Unproved:** preparation cannot establish closure within its supported analysis or operational resources. Do not emit an executable purporting to cover the unresolved path. Report the unresolved obligation and a bounded source demand chain; do not label the program intrinsically ill-typed or claim that recursive generics are forbidden.

Proposed initial behavior for expanding instances is the third outcome unless a supported proof closes them. A work limit can make the compiler responsive, but reaching it does not prove divergence or justify truncating the set of generated methods. Exact limits and diagnostic formatting are engineering details; changes to the semantic acceptance boundary require review. There is no fallback to runtime compilation or mandatory specialization.

An example expansion `f<T> → f<Container<T>> → …` is schematic, not proposed source syntax. A finite constructor set does not make this sequence finite. A future finite representation, structurally decreasing proof or other justified method can enlarge the proven subset without making the initial implementation's limitations into language restrictions.

## Migration and interface constraints

The preserved source compiler at the local `multi-dispatch` tag interleaves semantic checking, caller-context comparison and LLVM emission. `Value` contains LLVM values/places; `NativeCallDemand` combines checking state and native definitions; `compile()` resolves and emits through a shared refinement loop. `record_field()` needs a concrete product layout, and `LLVMEmission::type_of()` lacks a general abstract carrier. These are baseline observations, not requirements for the replacement; the removal-first branch does not retain those paths.

The migration must separate semantic method/operation obligations from emitted addresses and caller activation state. A checked boundary must carry sufficient result, owner, context and completion information for a thin emitter. This document fixes the closure responsibilities only after approval; the concrete checked representation, carrier and persistent identity encoding require their own review before being fixed.

H-399 requires removal of debt-producing historical checking/emission/context-specialization machinery first, followed by implementation on a coherent foundation. Temporary feature loss and known failing legacy cases are permitted during this restructuring stage. Keep a supported/missing-path inventory; preserve the confirmed language contracts as the target. The compiler must not retain compatibility shims, parallel semantics or an old-specialization fallback merely to preserve coverage. The preserved tag supplies recovery and comparison evidence.

## Required acceptance

The table specifies evidence for each claimed supported path. H-399 does not require all paths to be implemented or all legacy tests to pass in the current restructuring stage. Stage completion requires a coherent foundation, identified debt removed, correct retained/new paths and honest capability gaps. Missing implementation remains distinct from semantic invalidity or a proved coverage result.

| Case | Required observation |
| --- | --- |
| Direct and mutual fixed-instance recursion | Correct native result and cleanup; increasing runtime depth does not add native method bodies. |
| Finite generic calls, including recursion at a fixed substitution | Required substitutions are checked once per semantic instance; declaration/traversal order does not change results or admitted coverage. |
| Recursive type descriptions | Finite named graph and valid regional references; no by-value expansion or regional copy/move. Invalid layout/reference cycles remain separate errors. |
| Abstract arguments and results | A real CoD source program preserves actual concrete evidence through calls, storage and return, selecting the unique best method at runtime. |
| Independent polymorphic fields | Each field retains its own alternatives; different enclosing layouts use local access; a joint operation retains the checks it actually needs. Body count is not the field Cartesian product. |
| Owners and changing captures/context | Native recursion preserves fresh captures, selected/current interpretation, actual borrowed owners and access; expired or incompatible owners reject. |
| Exceptions and deferred actions | All reachable actions contribute obligations; ordered cleanup, recovery, explicit returns and exceptional release preserve the accepted contract. |
| Expanding or unresolved demands | A bounded useful unproved diagnostic; no truncated-success result, hidden runtime compiler or automatic new language prohibition. |
| Deliberately inadequate work allowance | Unproved rather than accepted or falsely diagnosed semantic invalidity. Increasing an allowance cannot change the meaning of an established program. |
| Independent call-site simplification | With simplification disabled, the same general method bodies remain correct and complete. Proven simplification may reduce checks without supplying missing semantic coverage. |

Use the existing source fixtures as behavioral evidence, not a substitute for their governing design decisions. The isolated authored-contract experiment is supporting evidence only. Focused compiler validation must include emitted method counts and actual native execution; the supported/missing inventory must account for displaced paths without weakening their assertions or claiming that a missing feature is forbidden by the language.

## Whole-file review

The proposed decision is to adopt this finite-obligation admission contract and its established/invalid/unproved distinction for the source migration, while retaining ordinary recursion and developing stronger closure proofs alongside real source slices. Alternatives are to demand a complete generic solver before migration, or to start emitting before defining the closure boundary. The former delays validated basic paths; the latter leaves recursive acceptance and native coverage dependent on incidental emitter behavior.

Approval of this whole file would confirm its stated value path, scope, initial proof boundary and acceptance criteria. It would not choose the permanent discriminator/ABI, omitted-result rules, a universal algorithm or any excluded language feature. Those remain explicit follow-up topics.
