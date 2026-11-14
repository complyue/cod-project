# CoD exception control and regional cleanup

Design revision: 57 · Recorded: 2026-11-14 · Status: confirmed bounded exception, lifecycle, scope, defer and return contracts; current implementation availability is recorded in the source capability reference.

## Design authority

- **Registry / current scope:** [D-17](DesignAuthorityRegistry.md#d-17); the semantic revision stated above and the independently qualified scopes in the current registry. **Confirmed/continued bounded exception, lifetime, scope, defer and return contract.** C-63/P-235/P-236; H-267/H-269/H-313–H-315/H-319/H-356–H-362/H-365/H-370/H-371; P-321–P-324/P-326/P-328/P-329. D-26r9/H-400/H-401/H-404/H-405/H-407 through C-90/P-374/P-375 governs the unknown-type failure categories and explicit declaration obligations.
- **Value path:** C-63/P-235 → C-10 and D-17/P-236 → C-63: recoverable business failure must preserve explicit regional responsibility, effects and inspectable lifetimes. H-267 confirms this bounded path and scope under charter §§2–4, 6.2–6.3 and 12; it does not qualify unrelated ownership proposals.
- **Contract boundary:** Acceptance: construction failure, exact release, explicit causes, owner/effect restoration, pre-entry versus callee failure, completion replacement and deferred queues. Complete integration/proof coverage remains work. No regional copy, lifetime extension, native-handle/out policy, RTTI, persistent transaction or broad D-04 guarantee follows.
- **Invariants / non-goals:** P-237 links D-14's ownership/borrowing, P-238 links D-10's scoped bindings, and P-239 links D-01's allocation, allocator symmetry and relocation. H-180/H-181 preserves hidden/captured owners; H-262/H-263 separates consumed names from valid local views. H-265/P-240 supplies the unchecked-OOM constraint. H-251 reserves RTTI decisions for human discussion. one cleanup responsibility per region; no per-object ownership, implicit owner resurrection, allocation-dependent OOM delivery, graph rollback, foreign-world editing or general RTTI decision. No C++ exception type or ABI becomes a CoD user type by implication.
- **Acceptance:** Nested normal and exceptional paths preserve values, cleanup and effect restoration; controlled construction failures neither leak nor double-release; invalid exceptional-path borrows reject. Deferred registration, execution and completion replacement preserve their specified ordering. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines remain the H records identified by the registry. [K-RESIZE-20261114](../CHANGELOG.md#k-resize-20261114) continues r56→57: H-431 retains E4 ordinary consuming-call cleanup for the H-432 resize proposal, with pre-entry preparation and post-cleanup result delivery kept distinct. H-428 allowed allocation effects and the qualified completion contract remain unchanged; no language allocation checkpoint or ownership recovery is added.

## Selected direction

CoD provides `try`, `catch` and `finally` control in the accepted contract. Ultimate out-of-memory failure is an **unchecked exception**: calling allocation does not require a checked-error result, a `throws` declaration or an enclosing catch merely to compile. A caller may catch the failure; otherwise it propagates toward the execution boundary. Unchecked does not mean ignoring ownership validity or turning invalid references into runtime exceptions.

`make` continues to produce its normal `owned region :Root` result when construction succeeds. OOM is an exceptional exit, not an implicit null owner or a mandatory optional/result wrapper. H-265 does not select automatic allocation retries, region growth, compaction or a process recovery policy. “Ultimate” failure describes exhaustion after any separately defined policy has failed; no such policy is introduced here.

The initial error taxonomy distinguishes `OutOfMemory` (the memory provider cannot supply storage), `RegionCapacityExceeded` (placement exceeds an existing region's fixed budget), and `InvalidCapacity` (an invalid or unrepresentable capacity request). All three are unchecked; none implies automatic growth or retry.

H-428 confirms allocation as an allowed initialization side effect. A failing initialization group does not rewind an existing region or undo earlier valid publication/effects. Its incomplete bindings are not successful results, while an already initialized target published into surviving same-owner state remains live with its region. Failed acquisition of a new whole region still releases its acquired resources exactly once. Application/library undo frameworks require their own contracts; the language supplies no allocation checkpoint or automatic rollback.

Compile-time type, moved-name, escape and binding errors remain compile errors. Other runtime checks are not all reclassified as catchable exceptions merely because OOM becomes catchable.

## Checked NullPointerException

H-313 accepts checked NPE: every potentially null dereference must be covered by a matching catch or declared by its immediately enclosing function/procedure with `throws NullPointerException`. Module top-level execution must handle it explicitly. A catch-all handles NPE when it covers the operation; no automatic boundary handler satisfies the source obligation. Null/default absence is not a substitute for definite payload initialization or valid owner lifetime.

```text
using "std:exceptions" NullPointerException;
function following(n:Node in O) -> const i64
  throws NullPointerException where {owner O} { n.next.count }
try { following(node) } catch (e : const NullPointerException) { 0 }
```

The exception name is selected and aliased through the normal type-identity rules. Throws may follow the result annotation or the where clause. It does not distinguish overloads. Bodyless contracts and symbolic procedure signatures retain the checked obligation; an implementation cannot expose more checked failure than its applicable declaration permits. A caller handles declared NPE even if a particular specialization does not throw. An outer caller's catch cannot excuse an undeclared callee body, and WPO inlining does not erase that boundary. Functions may throw/catch without gaining dynamic-symbol access.

Only a dereference creates the null-access obligation. `null` is a reserved literal, never a binding. `isnull(p)` evaluates its operand once and examines its reference without loading the target; merely rebinding a pointer with `=` does not dereference it. Ordinary payload argument evaluation, including copying bits into val, must account for null access in the caller. A valid non-null ref needs no NPE obligation. Stable pointer tests may establish a non-null view on the corresponding branch; mutations invalidate the fact, and checking a mutable field does not prove a later reload unchanged.

A handler's NPE or rethrow and NPE from finally must satisfy the enclosing callable's own contract. A cause containing NPE does not alter the primary exception's matching category. Existing E1–E5 cleanup, explicit causes and replacement semantics remain. OutOfMemory, RegionCapacityExceeded and InvalidCapacity remain unchecked; type, expired-owner and definite-initialization errors remain compile errors. Checked metadata introduces no payload RTTI, allocation on throw, general exception-effect polymorphism or runtime initialization registry.

Acceptance covers direct/module and callee-boundary rejection, explicit propagation, local handling and finally cleanup, bodyless and symbolic signatures, literal/test/rebind operations without target access, val argument evaluation, branch refinement and nullable loop paths. D-14 retains the reference initialization, same-owner, fixed-owner and current relative-encoding constraints. General exception payload graphs and unrelated deferred exception mechanisms remain outside this increment.

## Unknown-type and typing-contract failures

[D-26](GenericAndRecursiveClosure.md) classifies unavailable required type interpretation as checked `StaticallyUnknownType` and a detected violation of an established typing contract as unchecked `UnsoundRuntimeTyping`. Both use the ordinary exception control, explicit causes and cleanup rules. Static type errors remain compile errors; incomplete compiler reasoning cannot be substituted with an arbitrary runtime exception and claimed as successful native coverage.

Generic syntax or valid uninterpreted passage alone creates no unknown-type obligation. A voluntary `throws StaticallyUnknownType` declaration retains the fixed caller obligation, including known-type calls whose bodies succeed. Selected exception types, callable views, handlers and deferred invocations preserve this contract. Acceptance includes declaration coverage, retained categories and causes, ordered failure routing and exact release in specialized and shared native execution.

The source capability reference distinguishes category/declaration support from outside-catalog input and required-interpretation detection. This integration selects no new input validation protocol, carrier, permanent ABI, dynamic cast or runtime compilation mechanism; those D-26 boundaries remain separately scoped.

## Deferred work and explicit scopes

H-319 distinguishes expression grouping from scope creation. `{ ... }` groups expressions without starting a lexical or dynamic-effect scope. `{{ ... }}` starts a full scope: it owns its local storage lifetimes, restores its effects on exit and runs its deferred work. Calls and module top levels remain inherent full scopes. The [loop scoping rules](#loop-scoping-rules) specify loop-control bindings, iteration effect/resource restoration and enclosing-scope defer registration.

For resource-management idioms, `defer` is the explicit counterpart to C++ destructor-based scope cleanup. The cleanup obligation belongs to a registered action in a CoD scope; it is not an implicitly selected method on every object's type. Acquisition can supply the action with `defer init(...)()`. This analogy does not add independent destruction lifetimes to regional objects, replace intrinsic whole-region release, or make copied handles/callables acquire independent release authority.

H-356 selects two forms:

| Form | Registration | Scope exit |
| --- | --- | --- |
| `defer f(args...);` | Evaluate the callable and prepare its arguments once using normal call evaluation/conversion order. | Invoke the saved callable with the prepared arguments. |
| `defer { expressions... }` | Register the block without evaluating its body. | Evaluate the block. |

H-371 prohibits variable declarations and effect installation directly in a deferred block, including through ordinary grouping and control flow. Programs needing either operation must introduce an explicit nested `{{ ... }}` scope. The deferred action itself does not acquire an implicit invocation scope. Explicit nested scopes retain their normal local lifetime, effect restoration and deferred-work responsibilities; a return still targets the enclosing callable under H-362.

Outside the explicit block form, the deferred operand must be a function/procedure call. In `defer init(...)();`, the callable expression for the outer call is `init(...)`: it executes now, and its returned callable is retained for invocation at scope exit. In `defer factory(a())(b());`, factory acquisition and both argument computations run at registration in ordinary call order; only the final invocation waits. Parentheses have no special timing meaning. Arbitrary expressions require the explicit block form.

Saving an argument follows its declared passing mode. A bits value is prepared now; an externally borrowed argument retains its selected target and owner/access constraints, without copying the target's contents. A callable value retains its complete environment under D-18, so replacing its source holder later does not select different deferred code. Deferred direct procedure calls keep current-binding semantics when invoked; selecting their code at registration does not implicitly request `perform` or freeze unrelated TLS bindings. Deferred code retains the surrounding function/procedure permissions and explicit-only exception-chain rules.

Registration succeeds only after the callable and arguments have been prepared. An exception during preparation does not register a partial call; already completed preparations retain their ordinary cleanup responsibilities. Skipped control-flow paths register nothing. Registered work runs in reverse registration order within its owning scope. Borrowed values must remain valid until invocation; saving a view never extends its region or lexical owner's lifetime. H-370 keeps owning arguments under E4: registration identifies and saves the selected owning reference and corresponding actual region without consuming it. The original owner remains available for legal inspection and updates. Ownership transfers only when that registered call actually enters its callee at scope exit. Invocation uses the saved identity instead of resolving the source name again. Later operations must leave that ownership obligation valid; premature transfer/release and duplicate consumption must be identified by existing ownership checks. No owner copy, implicit lifetime extension, reservation protocol or new runtime exception follows. Preparation failure registers nothing and retains the existing preparation cleanup rules.

[Go's specification](https://go.dev/ref/spec#Defer_statements) similarly saves the function value and arguments before deferring the call. CoD retains its own explicit block scopes, checked exceptions, owner checks and delayed-block form. The earlier capture-list proposal is not needed for ordinary deferred calls; additional block-capture syntax is outside this bounded implementation item.

### Exceptions between deferred actions

H-357 confirms that an uncaught exception from an earlier-running deferred action is available to an exception handler of a later-running deferred block. The program can handle that failure or explicitly throw a new exception chained from it. Remaining registered actions still run in reverse registration order. An uncaught replacement exception supersedes the pending completion; no cause is attached automatically.

H-358 selects attached handlers and unconditional cleanup:

```text
defer { body(); } catch (e : const E) { recover(e); } finally { cleanup(); }
```

The finally block runs whether the deferred body/handler finishes normally or throws. While deferred actions execute, the scope retains its original normal result. If a deferred exception is handled and the remaining actions finish normally, scope exit resumes that result. A final unhandled exception abandons the retained result with its ordinary cleanup responsibility. This applies equally to bits, owned-region and complete callable results; retained views must still satisfy their owner lifetimes.

H-360 confirms that an already pending exception enters the matching catch directly, skipping the ordinary deferred body. With no pending exception, the deferred body runs and its own exceptions can enter its catches. Finally runs in either case. These handlers can also recover from an original scope-body exception. The recommended fallback form omits the normal body:

```text
defer catch (e : const E) { handle(e); } finally { cleanup(); }
```

The fallback is registered for its owning scope, with the same reverse-order and explicit-cause rules. H-361 requires explicit returns rather than silently treating a deferred catch's final expression as a missing callable result. H-362 gives the return-target and replacement rules below. No arbitrary exception graph or RTTI policy is introduced.

### Explicit callable returns

H-361 requires proc/func brace bodies to provide returned values with `return`. A value supplied from finally likewise requires explicit return syntax. A callable that finishes without a required explicit return raises an unchecked exception at runtime; the compiler should diagnose statically detectable missing-return paths through errors or warnings. The intrinsic vocabulary names this failure `MissingReturnException`; it carries the existing bounded system-exception details. Reachable required-value fallthrough produces `-Wmissing-return`, which `-Werror=missing-return` or `--warnings-as-errors` can reject. Static diagnosis does not turn this failure into a checked throws obligation.

H-362 confirms that `return value` inside a deferred catch or finally returns from the enclosing func/proc. It replaces a pending return or exception, while the remaining deferred actions still run. A handler without return preserves an existing pending normal result. If no required result is supplied before the callable completes, the missing-return rule applies. Void callables may complete normally without `return;`.

H-363 selects the `pure` declaration keyword for the const-function form whose body does not require explicit return: the final body expression supplies its result. Its boundary preserves readonly pre-existing external state, allows private mutation and constrains nested helper calls as described in [D-19](ConstFunctionsAndTailCalls.md). Pure-function implementation and TCO remain deferred by explicit user selection. The proposed `=>` form is not selected. Ordinary block-expression values remain separate from callable control-flow returns, and every return retains existing result-type, owner/access, storage and cleanup requirements. The executable proc/func fixtures use explicit returns; historical native checkpoint evidence retains its recorded source scope.

### Implementation availability

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Prepared values retain their independent snapshots; borrowed arguments retain the selected target and actual owner. Payload reads occur at invocation. Deferred registration contributes effects only after successful preparation; queue choices, failure paths and completion determine whether initialization is established. These are qualified behavioral obligations, not a mandated solver or runtime representation.

### Callable construction, installation and cleanup

H-380 cancels only affected dependency initialization TLS writes. Operand evaluation and callable construction retain their ordinary effects and exceptional exits; a produced value that is not installed follows existing temporary cleanup. No canceled binding is published or later restored. Procedure-local installations retain normal scoped cleanup. This supplies no permission to erase observable allocation failures or unrelated effects merely to shrink the final image.

H-372–H-376 distinguishes compile-time family registration from runtime callable materialization, TLS installation and invocation. A module definition is not a runtime resource acquisition to release during cleanup. Repeated combined effect execution creates separate materialization/installation events, not repeated method registration. Those events retain the existing preparation/publication boundary, restoration on every exit, captured-owner validity and exact release requirements.

Deferred calls retain the selected callable view and prepared operands; a family view must preserve its source interpretation and later actual-type dispatch. Deferred bodies remain delayed, with explicit nested scopes for declarations/effect installation. New abstract evidence and family dispatch must preserve checked NPE, pending normal/return/error completion and explicit causes. Missing/ambiguous method behavior not covered by existing static diagnostics needs its own decision; no new exception category follows from the AOT vision. [D-21](AheadOfTimeExecutionPreparation.md) tracks that architecture work while the bounded defer implementation continues.

### Immediate implementation and acceptance

H-356 authorizes implementing this language work independently of the LLVM binding pilot. The compiler distinguishes ordinary grouping from full scopes and iteration effect boundaries. Complete scope-owned deferred work and explicit returns while preserving the existing exception and regional invariants:

1. Parse deferred calls separately from deferred blocks. Validate the call shape without executing the final invocation or assigning timing significance to redundant parentheses.
2. Reuse typed call preparation for callee evaluation, ordered argument conversions, stored callable values and borrowed operands. Retain prepared state in the owning scope and register it only on successful preparation. Keep block bodies delayed.
3. Integrate deferred execution with normal and exceptional scope exit, pending results, effects and intrinsic owner cleanup. Use existing LLVM call/cleanup machinery and D-18 storage; no new general closure or persistence framework is implied.
4. Verify a compact set of composed cases: initializer-returned cleanup; saved values versus delayed block reads; later callable replacement; argument/conversion order and preparation failure; reverse-order cleanup on normal/exceptional paths; whole-loop versus per-iteration explicit scopes, direct loop defer accumulation and invalid borrowed-owner use. Record allocation and emitted-IR costs for repeated scoped use and accumulated registrations.

Before claiming complete coverage, verify H-370’s owning-argument boundary and resolve any uncovered integration choice involving checked exceptions from cleanup or cleanup failure while another completion is pending. Existing D-17 rules remain authoritative where they already apply. Neither a parser-only change nor execution of a standalone cleanup sample satisfies acceptance.

H-320 additionally proposes that an expected `func():void` or `proc():void` parameter can interpret `{{ ... }}` as an anonymous callable body instead of executing the block while evaluating the argument. The expected kind must retain function/effect restrictions; unresolved function/procedure overload ambiguity cannot be settled by silently executing the block. H-322 replaces the isolated inline-callback review with a unified owner-aware proposal for named and anonymous callables. Region-only reference captures, explicit copied bits and closure storage/lifetime requirements are reviewed in [D-14](RegionalOwnership.md#unified-callable-captures-and-region-owners); neither live lexical binding capture nor a blanket receiving-call-only restriction is selected. This is a language contract question, not permission to expose compiler callable-routing objects to source programs.

## Loop scoping rules

H-365 defines loops directly, without introducing a general-purpose scope abstraction. Ordinary grouping, explicit `{{ ... }}` scopes and inherent callable/module scopes retain their contracts. A loop combines loop-control bindings, per-iteration effects/resource lifetime and deferred work registered to a full scope; these responsibilities have distinct boundaries.

### Bindings and declarations

Loop-header index/accumulator bindings belong to the loop construct and follow its initialization/update rules. They are not fresh body variable declarations. An ordinary loop body forbids variable declarations, including storage, aliases, callable variables/named local callable definitions and region bindings. The restriction applies through ordinary nested groups and conditionals. A nested explicit `{{ ... }}` inside that body allows local declarations with the explicit scope's lifetime and cleanup. A full scope surrounding the whole loop does not authorize direct body declarations. Each nested loop needs its own per-iteration explicit boundary for body declarations; called functions/procedures retain their independent inherent scopes.

### Iteration effects and resources

Every iteration restores its entry effect bindings on every exit, including throw and return. A temporary binding cannot leak into the next iteration. Restoration changes bindings, not mutations already made to regional data. A fresh region installed through an effect, such as `effect @arena <- region(bytes)`, belongs to that iteration's resource lifetime: release it at iteration exit and restore the prior binding. Captured and hidden pre-existing owners remain alive; shadowing never frees the enclosing owner.

Existing access, transfer and provenance checks apply. An outer deferred action cannot borrow an iteration-owned region or a value dependent on it. Saving a view does not extend lifetime, and a dynamic binding does not permit ownership extraction. No allocator, regional representation or zero-cost-relocation rule changes.

### Deferred work and accumulation

Direct `defer` in an ordinary loop body is allowed. The iteration's effect restoration boundary does not own user deferred actions. Each successful dynamic registration goes to the nearest enclosing full scope (`{{ ... }}` or an inherent callable/module scope), skipping loops and ordinary grouping. Every registration remains distinct, and the owning scope runs all actions in global reverse registration order, including registrations before and after the loop. Accumulation is intentional for dynamic resource management; an implementation must not conceal its cost by cleaning up early.

```text
{{
  defer finish_batch();
  for (i in 0 .. count) { defer consume(i); }
  defer begin_cleanup();
}} # begin_cleanup, consume(count - 1) ... consume(0), finish_batch

for (i in 0 .. count) { {{ defer consume(i); }} } # one cleanup per iteration
```

Putting `{{ ... }}` around the whole loop forms a cleanup batch. Putting it inside the body creates per-iteration cleanup. Callee evaluation and argument preparation remain eager under H-356; partial preparation registers no action and preserves prior registrations and acquired cleanup obligations. D-17's pending return/error, deferred catches and finally rules apply. Outer deferred calls run after iteration effects have restored; direct procedure invocation sees its current bindings, without an implicit TLS snapshot.

### Loop-control snapshots and borrowing

A delayed defer block cannot capture loop-control variables, including the index and fold accumulator. `defer consume(i)` saves the current bits argument through ordinary call preparation; `defer { consume(i); }` is invalid. There is no implicit final-value capture, lexical boxing or lifetime extension. A borrowed argument saves the selected reference rather than a deep copy, so its owner must outlive eventual invocation. Call syntax does not make borrowing loop-control storage or iteration-owned resources valid for an outer deferred action.

### Zero-iteration else

Range `for ... else` executes its else branch only when the body executes zero times, not on exhaustion. The else branch follows the same rules: direct variable declarations are forbidden; its effects restore and fresh effect regions release at branch exit; direct defer registers to the enclosing full scope. A nested explicit `{{ ... }}` permits local declarations and local deferred work. The loop index is unavailable in else. Once-only bound evaluation, initialization joins and fold's initial accumulator are unchanged. This contract does not add `fold ... else`, break/continue or new loop forms.

### Acceptance

Composed programs must establish outer/iteration effect values, normal/throw/return restoration, exact releases with captured/hidden owner retention, eager snapshots and global LIFO over dynamic registrations. Validate whole-loop and per-iteration cleanup batches; reject declarations through ordinary body/else groups, delayed loop-control captures and outer deferred borrows of iteration resources; accept nested explicit and callable scopes. Cover zero iterations and bound evaluation. Measure live registrations, peak storage and execution/IR costs as registration count grows. These checks must compose with return/error replacement and remaining cleanup. D-16 syntax/comments, numeric promotion, bits layout, transfer and formatting remain unchanged; fraction implementation stays deferred.

## Expression-oriented control

`try` is an expression, consistent with CoD's existing expression model. Normal completion and handled completion provide its result; an unhandled exception provides no normal result. A `finally` block performs completion work without replacing the result merely through its final expression value. No public bottom type or general continuation API is needed to express this planning requirement.

The exception identities are explicitly selected into the module before use:

```text
try {
  owned region packet:Packet <-
    make Packet(capacity = bytes) { sequence <- initial };
  process_packet(packet)
} catch (error : const OutOfMemory) {
  handle_allocation_failure(error)
} finally {
  finish_attempt()
}
```

The same construct can occur in a function or a procedure. Each body's existing restrictions still apply: a function may operate on explicit memory but cannot access dynamic bindings or call a procedure. A lexical catch does not make dynamic lookup available inside a function. Exception capabilities do not create module exports, change callable identities or expose compiler call-routing machinery.

The control sequence is:

1. Execute the protected expression. On an exceptional exit, run cleanup for scopes being exited before entering the selected handler.
2. Execute at most one applicable catch. Unmatched exceptions continue outward. An exception raised by a catch is not caught again by that same try's sibling catches.
3. Execute `finally` once when leaving this try, after normal or handled completion, or while propagating an exception. Then deliver the pending result or propagate the pending exception.

This does not promise `finally` execution after forcible process termination or loss of the execution environment. Native resource release and TLS restoration are compiler obligations even when the program writes no `finally`; user finally code must not be required to free every intrinsic owner manually.

## Ownership and effects on exceptional paths

Exceptional control must preserve the actual ownership history; it is not a transaction that rewinds the program:

| Situation | Required preservation under the existing ownership/effect constraints |
| --- | --- |
| Region allocation fails before obtaining storage | No owner exists to expose, consume or release. |
| Allocation succeeds, then root initialization fails | The unfinished region still has one construction cleanup responsibility. Release it before propagation; never publish an uninitialized owner. |
| An effect's RHS construction fails | Do not install a new binding. Preserve the previous binding and owner; do not roll back unrelated edits made while evaluating the RHS. |
| A local owner transfers into an inner effect and that scope exits exceptionally | Release the region at its actual new owning scope. The original name stays consumed; old views cannot be used after that owner ends. |
| A transfer retains a live, accessible local owner | Preserve existing valid views under H-263, regardless of the old name's consumed state. |
| Perform temporarily shadows current bindings with captured values | Restore lookup on exceptional exit without freeing either pre-existing captured or hidden owner. Release only resources actually acquired by the exiting scope. |
| A caller-owned region was already edited before a later exception | Retain those edits unless the business algorithm separately implements rollback. |

Cleanup follows scope exit and reverse acquisition/restoration order within each scope. The handler's incoming ownership facts must include every throwing path that can reach it. If one such path transferred a region and another retained it, the handler cannot unconditionally use the old owning name. This extends the existing branch-validity requirement to exceptional edges; WPO does not make runtime failure positions statically known.

Pending expression results need explicit attention. If a try body produces an owning region but its finally subsequently throws, that not-yet-delivered result must still be released once. A borrowed result remains dependent on its actual owner through finally. Neither a stale pointer nor an abandoned owning result may escape by treating the normal result as delivered before finally completes. E4 below fixes the call/result transfer points; D-14 retains the wider ownership design.

## Exception data and low-memory operation

Raising OOM, routing it, restoring effects, releasing regions and entering a catch must not require a fresh allocation from the exhausted memory source. A handler can itself allocate and fail. Intrinsic system failures carry bounded scalar details; no borrowed payload may outlive its owner. General user exception payload graphs remain separate.

## Accepted first implementation contract

H-267 accepts E1–E5 together. Their value is a complete construction/edit/transfer/release workflow with usable failure handling and correct view validity, without per-object ownership or changes to regional pointer encoding. The explicit-cause amendment is part of E2/E3.

### E1 — Intrinsic exception categories and payload

The initial unchecked system failures are: `OutOfMemory`, `RegionCapacityExceeded` and `InvalidCapacity`. The first means the memory provider cannot satisfy allocation; the second means an existing region's fixed budget cannot satisfy placement; the third means an invalid/unrepresentable capacity request. No automatic growth or retry is implied.

Expose their type identities through an ordinary standard vocabulary module, with normal explicit selection and local aliases. Their intrinsic payload is bounded bits: category, a source-site identifier and relevant scalar size information. The source-site identifier is resolved against immutable metadata for the executing program; it contains no native address and does not promise meaning under a different program image. Error formatting must not turn it into a stored cross-owner pointer. Catch selection uses a small runtime category discriminator. General type reflection, arbitrary user exception payloads and native C++ type matching remain outside this increment. H-267 accepts this bounded discrimination under H-251; it does not approve general RTTI. Matching needs a type identity at the throw/handler boundary, not a runtime type descriptor embedded in each payload.

### E2 — Expression syntax, matching and completion

Use this source family, with exception identities already selected into the current module:

```text
try {
  operation()
} catch (error : const OutOfMemory) {
  recover(error)
} catch (error) {
  report_or_rethrow(error)
} finally {
  finish_attempt()
}
```

A typed catch matches its exact intrinsic category. Duplicate categories reject; an optional final catch-all matches the remaining supported categories. No subtype search or clause-order priority among distinct exact categories is needed. Catch parameters receive immutable bits values; the catch-all has the intrinsic common system-exception value type. This does not change ordinary parameter borrowing rules. Public field/accessor spelling is a routine API detail within the bounded payload, not authority to expose arbitrary runtime metadata.

Normal and handled arms must have the same result type under the existing conditional-expression rule; exceptional paths supply no normal value. Require `finally` to yield void. `try` may have catches, finally, or both, but at least one is required. `throw error` propagates a supported intrinsic exception value; bare `throw;` rethrows the active catch value. `throw error from cause` explicitly establishes the exception relationship. No throw, catch or finally automatically adds a cause. Source creation of arbitrary user exception values/graphs is deferred. Catch bodies are outside their own sibling catch coverage. After all relevant cleanups/finally blocks, an uncaught exception gets a minimal diagnostic and a nonzero exit status at the program boundary.

### E3 — Failure from finally

A new exception from finally replaces a pending exception or normal result. Outer cleanups/finally blocks still run; any abandoned owning result releases once. The replaced exception is not retained automatically. A program uses `throw error from cause` when it explicitly wants that relationship. The cause does not participate in matching the primary exception against catch clauses.

### E4 — Consuming calls and results

An admitted owning parameter transfers responsibility at callee entry, after argument evaluation and conversion succeed. An external borrowed parameter retains the original owner without acquiring cleanup responsibility. H-271/H-272 requires explicit owner-generic declarations for external borrowing; the native prototype's unqualified `region :Root` borrowing spelling is a migration boundary, not the new default. The call itself has not consumed an owning argument if a later argument fails before entry; explicit transfers performed inside earlier argument expressions retain their actual effects. Validate availability and provenance again at the call boundary, including duplicate ownership transfers and later-argument invalidation of earlier views.

After entry, an owning parameter's region belongs to the callee scope until returned or otherwise transferred; an exceptional callee exit releases what that scope still owns. The caller's original name remains consumed. An owning result transfers only after the callee's required finally work completes successfully; a result abandoned by finally failure remains subject to cleanup. This preserves the existing per-argument evaluation/conversion order without treating ownership transfer as a numerical conversion.

H-431 explicitly applies this rule to consuming replacement, now proposed as resize(r, new_cap) under H-432 in D-28. “Call transfers ownership” means callee entry, not the beginning of argument evaluation. “Only successful completion returns ownership” includes required finally/deferred cleanup. A return selected before throwing cleanup is not delivered; its abandoned owning resource is released. Catching an exception does not restore the caller's consumed input. A function may explicitly return the same region on success, but that is receipt of an owning result, not reversal of the input transfer. Resize's separate replacement contract ends old-region views rather than retargeting them. Its proposed capacity checks happen after entry: even an undersized request that throws does not restore the input. D-28's exact mapping of resize failures to existing categories remains a scoped proposal; E4/E5 is unchanged.

The borrowed and consuming call forms are:

```text
function edit(region packet:Packet) -> void { ... }
function transform(owned region packet:Packet) -> owned region :Packet { ... }

edit(packet); # Borrow; the caller still owns packet.
owned region next:Packet <- transform(packet);
# packet is consumed after callee entry; next owns the successful result.
```

The first increment does not select overloads whose only distinction is owning versus borrowed mode; reject such a competing pair with an explicit unsupported-mode-overload diagnostic. Otherwise adding an overload could silently change whether a call consumes its operand. General mode-based overload ordering remains a separate language decision.

### E5 — Bounded construction and acceptance scope

This increment completes typed make/root access, fresh owning declarations, factory results, borrowed/owning calls, effect installation, branch/exception ownership joins, and try/catch/finally together. The required capacity is a runtime byte budget for the root and subsequent payload allocations, including their internal alignment consumption but excluding the fixed region header. No automatic growth is introduced. The first root initializer names every declared field exactly once and evaluates initializers in written order; omitted/duplicate fields reject. More general initializer bodies and defaults remain separate.

Ordinary owner reassignment/reinitialization, occupied owner destinations, general reference-assignment/const-view overloads, mailbox transport, user exception graphs and general RTTI are outside this increment. Existing typed memory operations can demonstrate root edits without pretending that the complete assignment surface is implemented. Exceptions must not roll back prior graph edits or ownership transfers.

Implementation includes the actual allocation-to-publication cleanup repair, the direct LLVM control path, the required build and focused CoD-language/shilos-region validation. Native system-call failure translation is internal; the implementation must preserve the user semantics above without exposing an ABI or adding an alternative backend. Acceptance covers complete construction/edit/call/transfer/release, captured/hidden owner preservation, normal and exceptional control, pre-entry versus in-callee failure, invalid owner/view use and deterministic low-memory failure injection. Source, diagnostics and evidence should be reviewable before any broader feature is added.

Cancellation, asynchronous faults, restartable/resumable exceptions and cross-process exception delivery remain outside this first contract. The compiler/runtime route described below is an implementation recommendation subject to the accepted semantics, not another user-visible mechanism to approve.

## Compiler and runtime implementation

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Native exception handling must preserve checked obligations, explicit causes, scope cleanup, effect restoration and exact release. Intrinsic system-failure information is not general reflection or a mandated user-visible C++ exception ABI. Concrete payload and control-flow representation remain implementation concerns.

### Retained explicit-cause lifetime

H-269 requires the explicit cause chain to survive returning or storing a caught exception beyond the catch. Intrinsic runtime-managed storage retains the relationship; creating a chain may itself raise OutOfMemory. Basic OOM propagation does not allocate. A failed chain construction propagates the new OOM without automatically attaching either operand. Exception values and cause records are immutable: `throw x from e` supplies e as the explicit cause of the thrown value without mutating saved x or e values. It replaces the thrown copy's preceding direct cause; that preceding relationship remains in any unchanged copy of x.

The bounded runtime uses execution-local numeric handles for immutable cause records. Regional fields contain bits and handles rather than host addresses; copying a regional image within the same execution does not require pointer patching. Generic exception storage also preserves its bounded failure-category discriminator. These handles, like source-site IDs, are not a cross-process persistence or delivery format. General user exception graphs and general RTTI remain excluded.

Retained backing storage is released at execution end, including uncaught failure. The initial implementation retains discarded records until that boundary; repeated explicit chaining consumes that storage and may raise OOM. It does not add per-value destructors, graph ownership or eager collection. Public inspection uses a bounded cause count and checked indexed access; inspection and propagation allocate no cause records. Exact reclamation optimizations can preserve this source lifetime contract later.

The native cause table is created only by explicit chaining. Its TLS slot is a trivial pointer, with no lazy destructor registration on basic OOM propagation or cleanup. Failed table creation, first-record allocation or later growth leaves existing histories intact. The native fault harness fails the actual allocation at each of these three boundaries, verifies a fresh unchained OOM, retries successfully and accounts for all backing allocations/releases.

### Allocation-to-publication cleanup

The regional allocator must release its byte block if root construction throws, using the matching allocator and exact capacity. [The construction fixture](../tests/shilos/region_construction_tests.py) checks failure before root entry, both throwing-root overloads and successful retention/release. This allocation-to-publication boundary supplies no existing-region rollback or graph transaction.

## Bounded acceptance and remaining scope

Acceptance requires nested normal/exceptional completion, pre-entry versus callee failure, explicit cause retention, cleanup replacement, deferred queues, failed construction and invalid-borrow rejection. Check exact acquisition/release on supported paths.

Mailbox transport, general user exception graphs, RTTI, cross-process causes and remaining ownership choices retain separate scope. Current implementation limits are maintained in the source capability reference.

## Type routing and handler scope

A try scope supplies its accepted exception type identities and cleanup/handler destinations. At a statically typed throw, WPO can resolve the destination and emit a direct branch. The routing information is distinct from the exception payload; it does not require general RTTI in each exception value. Catch-all rethrow must preserve the arriving exception identity in the propagation context. Merely retaining the handler's accepted identity cannot distinguish otherwise identical incoming values.

The user's H-268 suggestion motivates this separation. A runtime handler-registration table is not required by the source semantics or selected merely by the analogy; compiler-local handler scopes and direct LLVM edges are the first implementation route. Explicit-cause lifetime remains a correctness obligation: no dangling catch-local references, silent truncation or allocation-dependent basic OOM delivery.
