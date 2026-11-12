# CoD exception control and regional cleanup

Design revision: 23 · Recorded: 2026-11-12 · Status: accepted exception/lifecycle, checked-NPE and defer requirements; explicit-return details pending; explicit loop declarations checked, grouping/defer implementation outstanding.

## Design authority

- **Pending completion and explicit returns:** [H-360/H-361](DesignAuthorityRegistry.md#h-360)/[K-519](DesignAuthorityRegistry.md#k-519), C-63/P-323–P-324, confirms direct incoming-error catch dispatch, bodyless defer catch/finally, recovery from original scope-body failures and explicit value returns from callable bodies. Missing a required explicit return raises an unchecked exception, with static diagnostics where analysis permits. This serves visible business control and failure responsibility under charter §§2–3/§6/§12. Return targeting/replacement in deferred handlers, void fallthrough and concise expression-function syntax remain pending-human; no implicit recovered-result rule is inferred. Acceptance must compose return, throw, handlers, finally, deferred actions and exact retained-result cleanup.

- **Deferred handlers and loop declarations:** [H-358/H-359](DesignAuthorityRegistry.md#h-358)/[K-518](DesignAuthorityRegistry.md#k-518) qualifies this revision’s catch/finally surface, retained normal completion and explicit per-iteration declaration boundary. C-63/P-286/P-323 serves explicit storage responsibility and recoverable business cleanup under charter §§2–3/§6/§12. Existing D-14/D-18 owner and value contracts remain dependencies. Acceptance requires result retention through handled cleanup failure, unconditional finally execution and rejection of loop-body declarations outside an iteration’s explicit scope. H-360 confirms incoming-error dispatch and original body-failure handling. H-361 requires explicit callable returns; return-target, void and concise-function choices remain pending. K-519 records bounded loop-declaration evidence; grouping/defer evidence is outstanding.

- **Deferred cleanup failures:** [H-357](DesignAuthorityRegistry.md#h-357)/[K-517](DesignAuthorityRegistry.md#k-517) confirms continuing remaining actions in reverse registration order, replacement of an unhandled pending completion without automatic causes, and delivery of an earlier-running action's uncaught exception to a later-running deferred block's handler. This extends C-63/P-235–P-236's explicit failure/resource responsibility; D-14 ownership and D-18 retained values remain dependencies. H-358 confirms attached catch/finally clauses and resumption of the original normal result after recovery. H-360 confirms direct catch dispatch, skipping the deferred body when an exception is pending, and handling of original scope-body failures. H-361 supplies the explicit-return direction; its remaining integration choices are `pending-human`. Acceptance requires composed handled/rethrown/chained cleanup paths and exact cleanup of retained or abandoned results.

- **Defer call/block contract:** [H-356](DesignAuthorityRegistry.md#h-356)/[K-515](DesignAuthorityRegistry.md#k-515), C-63/P-321–P-322, confirms this revision's deferred-work section: explicit delayed blocks, otherwise function/procedure calls with registration-time evaluation of callable and arguments. Preparing acquisition now and invoking its cleanup later serves explicit lifetime responsibility and readable effect ordering under charter §§1–3/§6/§8/§12. H-319's scopes and the existing owner/exception contracts govern; acceptance requires observable timing, saved callables/arguments and correct normal/exceptional cleanup. Immediate implementation is authorized; native evidence is still required. Native handles, `out` parameters and other D-20 proposals remain outside this confirmed scope.

- **Callable dependency:** [D-18](RegionalClosures.md) and its current registry entry govern retained callable values, captures and owner associations. Deferred work must preserve those contracts and D-14's target-owner/access checks. It introduces no automatic lexical boxing, owner lifetime extension, regional callable storage or RTTI. The contextual block-to-callback proposal remains separate from an ordinary deferred block.

- **Explicit scope boundary:** [H-319](DesignAuthorityRegistry.md#h-319)/[K-444](DesignAuthorityRegistry.md#k-444) confirms ordinary grouping braces and explicit double-brace lexical/effect scopes, plus defer only within an explicit scoped block when inside a loop. C-63/P-286–P-287 preserves C-65 lifetime and C-36 effect requirements. This scope change is not implemented by the colon/nullable checkpoint; acceptance must cover name visibility, initialization, cleanup, conditional paths and loops. H-320's contextual callable/capture boundary remains pending; no escaping closure or wider qualification is implied.

- **Nullable calls and uniform annotations:** [H-317/H-318](DesignAuthorityRegistry.md#h-317)/[K-443](DesignAuthorityRegistry.md#k-443) accepts explicit nullable ptr call boundaries, retains ptr edges for cycles and uses single-colon annotations with arrow results. C-72/P-284–P-285 preserves C-65 owners/access and C-63 checked failures; charter §§1–3 and D-16 compatibility were reviewed. Acceptance requires nullable forwarding without payload access, checked dereference, owner/result rejection and migrated grammar. H-319 establishes the revised scope boundary; native colon/nullable acceptance is recorded by K-444, while scope/defer implementation remains separate.


- **Declaration cleanup:** [H-316](DesignAuthorityRegistry.md#h-316)/[K-442](DesignAuthorityRegistry.md#k-442) applies C-72/P-283 to the current ref/ptr/val surface. Ordinary views name their target type; allocation and element selection preserve owner/access information directly. Let/var declarations and angle-bracket reference types are rejected. [Native evidence](../tests/cod/language/declaration-cleanup-validation.json) covers migrated examples, copied bits versus aliases, checked NPE, lifetime/relocation and multi-module composition. H-317 covers nullable call boundaries; automatic reference-target allocation and staged required-reference cycles remain outside that acceptance. Existing pending qualifications remain pending.

- **Native nullable-storage acceptance:** [K-441](DesignAuthorityRegistry.md#k-441) continues H-309–H-315 for the bounded field/val/checked-NPE implementation. [Evidence](../tests/cod/language/nullable-storage-validation.json) records the actual compiler, language cases, LLVM verification, relocated scalar/object reference fields and exceptional region cleanup. Nullability does not weaken definite initialization, fixed owners or readonly access. Automatic reference-target allocation and general cyclic ref publication remain outside this acceptance; H-316/K-442 covers declaration cleanup.

- **Checked NPE and explicit bits storage:** [H-313–H-315](DesignAuthorityRegistry.md#h-313)/[K-440](DesignAuthorityRegistry.md#k-440) accepts checked NullPointerException, mandatory module-top-level handling and val locals/parameters; null is a reserved literal and isnull inspects a pointer without dereferencing it. C-72/P-280–P-282 preserves storage/access/lifetime rules, with a null-initialized local acquiring its fixed owner at the first non-null assignment. D-17 owns exception/signature obligations and D-14 owns ordinary storage/owner checks. No checked OOM, payload RTTI, implicit regional-object copy or per-object owner is introduced. Acceptance requires native execution plus rejection at uncovered calls, unsafe reads and inconsistent owner paths; evidence is recorded separately from this confirmed contract.

- **Region-source continuity:** [H-307](DesignAuthorityRegistry.md#h-307)/[K-435](DesignAuthorityRegistry.md#k-435) updates region examples to the declaration-keyword surface in [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language). C-70/P-270–P-271 preserves the intrinsic borrowed/owning distinction and existing lifecycle constraints. This revision changes source notation and evidence navigation only; it does not qualify pending transport, optional-owner, numeric or general graph contracts. Acceptance is coherent examples plus the migrated compiler/lifecycle checks recorded by K-435.

- **Resource declarations:** H-275/H-307 and [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language) use region declarations for all resource references, retaining owned for one cleanup responsibility. Local, parameter, result and procedure-signature forms are implemented. Exceptional transfer timing, cleanup and retained causes are unchanged; historical evidence retains its original source revision.

- **Parameter migration:** [H-271/H-272](DesignAuthorityRegistry.md#h-271) replaces the default-borrowed parameter source rule with `ptr` and callee-local default targets; external borrowing requires owner-generic declarations in [D-14](RegionalOwnership.md#pointer-variables-and-owner-generic-parameters). [K-417](DesignAuthorityRegistry.md#k-417) records this scoped supersession. E4's transfer timing, exceptional cleanup and single-owner invariants continue unchanged. K-435 supplies fresh cleanup/relocation evidence using the region declaration surface; older E1–E5 reports retain their recorded source/compiler scope.

- **Current continuity:** [K-414](DesignAuthorityRegistry.md#k-414) continues H-267/H-269 with [the completed E1–E5 audit](../tests/cod/language/exception-ownership-validation.json). Retained causes, fallible explicit creation, storage, cleanup and regional lifetimes have focused native evidence; no general RTTI or wider qualifier choice is made.

- **Registry / scope:** [D-17](DesignAuthorityRegistry.md#d-17), revision 23, E1–E5, checked NPE, deferred-work and the compiler/runtime, lifetime and acceptance sections. H-267 accepts the bounded value path, ownership/effect preservation and explicit chaining only; [H-269](DesignAuthorityRegistry.md#h-269) confirms retained causes beyond catch exit and permits OOM during explicit creation. H-319/H-356 supplies the explicit scope and defer call/block contracts. K-515 records their integration without claiming implementation; H-357–H-359/K-517–K-518 adds deferred-error handling, retained normal completion and the loop declaration boundary; H-360 confirms deferred entry/body-failure handling and H-361 requires explicit returns, with detailed return integration pending. K-408–K-413 retains the earlier acceptance and intermediate evidence history. General user exception graphs, RTTI and the other stated non-goals remain outside this contract.
- **Value path:** C-63/P-235 → C-10 and D-17/P-236 → C-63: recoverable business failure must preserve explicit regional responsibility, effects and inspectable lifetimes. H-267 confirms this bounded path and scope under charter §§2–4, 6.2–6.3 and 12; it does not qualify unrelated ownership proposals.
- **Dependencies / constraints:** P-237 links D-14's ownership/borrowing, P-238 links D-10's scoped bindings, and P-239 links D-01's allocation, allocator symmetry and relocation. H-180/H-181 preserves hidden/captured owners; H-262/H-263 separates consumed names from valid local views. H-265/P-240 supplies the unchecked-OOM constraint. H-251 reserves RTTI decisions for human discussion.
- **Invariants / non-goals:** one cleanup responsibility per region; no per-object ownership, implicit owner resurrection, allocation-dependent OOM delivery, graph rollback, foreign-world editing or general RTTI decision. No C++ exception type or ABI becomes a CoD user type by implication.
- **Acceptance:** nested normal/exceptional paths preserve values, cleanup and effect restoration; controlled construction failures neither leak nor double-release; invalid exceptional-path borrows reject. K-441 supplies current compiler/source hashes and full language plus focused native acceptance; the earlier E1–E5 audit retains its recorded scope.

## Selected direction

CoD provides `try`, `catch` and `finally` control in the accepted contract. Ultimate out-of-memory failure is an **unchecked exception**: calling allocation does not require a checked-error result, a `throws` declaration or an enclosing catch merely to compile. A caller may catch the failure; otherwise it propagates toward the execution boundary. Unchecked does not mean ignoring ownership validity or turning invalid references into runtime exceptions.

`make` continues to produce its normal `owned region :Root` result when construction succeeds. OOM is an exceptional exit, not an implicit null owner or a mandatory optional/result wrapper. H-265 does not select automatic allocation retries, region growth, compaction or a process recovery policy. “Ultimate” failure describes exhaustion after any separately defined policy has failed; no such policy is introduced here.

The initial error taxonomy distinguishes `OutOfMemory` (the memory provider cannot supply storage), `RegionCapacityExceeded` (placement exceeds an existing region's fixed budget), and `InvalidCapacity` (an invalid or unrepresentable capacity request). All three are unchecked; none implies automatic growth or retry.

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

## Deferred work and explicit scopes

H-319 distinguishes expression grouping from scope creation. `{ ... }` groups expressions without starting a lexical or dynamic-effect scope. `{{ ... }}` starts both: it owns its local storage lifetimes, restores its effects on exit and runs its deferred work. Calls and module top levels remain inherent scopes. H-359 requires variable declarations in a loop body to be inside a nested per-iteration `{{ ... }}` scope. This includes val/ref/ptr, callable, alias and region declarations; loop-header bindings belong to the loop construct. Direct defer in a loop body is rejected; `for (...) { {{ defer ...; ... }} }` is permitted because the explicit scope finishes on every iteration rather than accumulating actions or storage in an outer scope. An explicit scope surrounding the loop does not provide that per-iteration boundary; nested loops require their own nested scope for body declarations.

For resource-management idioms, `defer` is the explicit counterpart to C++ destructor-based scope cleanup. The cleanup obligation belongs to a registered action in a CoD scope; it is not an implicitly selected method on every object's type. Acquisition can supply the action with `defer init(...)()`. This analogy does not add independent destruction lifetimes to regional objects, replace intrinsic whole-region release, or make copied handles/callables acquire independent release authority.

H-356 selects two forms:

| Form | Registration | Scope exit |
| --- | --- | --- |
| `defer f(args...);` | Evaluate the callable and prepare its arguments once using normal call evaluation/conversion order. | Invoke the saved callable with the prepared arguments. |
| `defer { expressions... }` | Register the block without evaluating its body. | Evaluate the block. |

Outside the explicit block form, the deferred operand must be a function/procedure call. In `defer init(...)();`, the callable expression for the outer call is `init(...)`: it executes now, and its returned callable is retained for invocation at scope exit. In `defer factory(a())(b());`, factory acquisition and both argument computations run at registration in ordinary call order; only the final invocation waits. Parentheses have no special timing meaning. Arbitrary expressions require the explicit block form.

Saving an argument follows its declared passing mode. A bits value is prepared now; an externally borrowed argument retains its selected target and owner/access constraints, without copying the target's contents. A callable value retains its complete environment under D-18, so replacing its source holder later does not select different deferred code. Deferred direct procedure calls keep current-binding semantics when invoked; selecting their code at registration does not implicitly request `perform` or freeze unrelated TLS bindings. Deferred code retains the surrounding function/procedure permissions and explicit-only exception-chain rules.

Registration succeeds only after the callable and arguments have been prepared. An exception during preparation does not register a partial call; already completed preparations retain their ordinary cleanup responsibilities. Skipped control-flow paths register nothing. Registered work runs in reverse registration order within its owning scope. Borrowed values must remain valid until invocation; saving a view never extends its region or lexical owner's lifetime. Owned-argument consumption across registration and invocation needs explicit integration with E4 before such paths can claim acceptance, rather than an implicit move hidden in argument saving.

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

The fallback is registered for its owning scope, with the same reverse-order and explicit-cause rules. H-361 requires explicit returns rather than silently treating a deferred catch's final expression as a missing callable result. The remaining result/return questions below must be resolved before activating their affected lowering. No arbitrary exception graph or RTTI policy is introduced.

### Explicit callable returns

H-361 requires proc/func brace bodies to provide returned values with `return`. A value supplied from finally likewise requires explicit return syntax. A callable that finishes without a required explicit return raises an unchecked exception at runtime; the compiler should diagnose statically detectable missing-return paths through errors or warnings. Static diagnosis does not turn this failure into a checked throws obligation.

The user also requests consideration of a concise function form with an implicit expression result. A proposed `func add(x:u64, y:u64) -> const u64 => x + y;` form is pending-human. Whether void callables need `return;`, and whether return from deferred catches/finally targets the enclosing callable and replaces a pending completion while continuing cleanup, are also pending-human. Existing block-expression values and callable result storage/owner requirements remain distinct from these control-flow decisions. D-10/D-18 implicit callable-body result examples are superseded for future implementation, while their native checkpoint evidence retains its original source/compiler scope.

### Bounded implementation evidence

K-519 records explicit-scope AST representation and enforcement of H-359's loop-body declaration rule. The checker covers nested loops, declarations under conditional/grouping expressions, explicit iteration scopes and independent callable-body scopes. Existing loop fixtures spell out their intended lifetime boundaries. Native validation covers 386 language checks, 27 callable accounting cases and focused ownership/relocation checks. Ordinary grouping still uses the prototype's implicit-scope lowering; deferred registration/execution and explicit callable returns are outstanding. These results do not establish full H-319/H-356 acceptance.

### Immediate implementation and acceptance

H-356 authorizes implementing this language work independently of the LLVM binding pilot. The inspected prototype has no defer expression/lowering path and still gives ordinary blocks scope behavior. Implement H-319's scope distinction with the following work, preserving existing exception and regional invariants:

1. Parse deferred calls separately from deferred blocks. Validate the call shape without executing the final invocation or assigning timing significance to redundant parentheses.
2. Reuse typed call preparation for callee evaluation, ordered argument conversions, stored callable values and borrowed operands. Retain prepared state in the owning scope and register it only on successful preparation. Keep block bodies delayed.
3. Integrate deferred execution with normal and exceptional scope exit, pending results, effects and intrinsic owner cleanup. Use existing LLVM call/cleanup machinery and D-18 storage; no new general closure or persistence framework is implied.
4. Verify a compact set of composed cases: initializer-returned cleanup; saved values versus delayed block reads; later callable replacement; argument/conversion order and preparation failure; reverse-order cleanup on normal/exceptional paths; explicit loop scopes and rejection of direct loop defer; invalid borrowed-owner use. Record allocation and emitted-IR costs for repeated scoped use.

Before claiming complete coverage, resolve any uncovered integration choice involving owned arguments, checked exceptions from cleanup, or cleanup failure while another completion is pending. Existing D-17 rules remain authoritative where they already apply. Neither a parser-only change nor execution of a standalone cleanup sample satisfies acceptance.

H-320 additionally proposes that an expected `func():void` or `proc():void` parameter can interpret `{{ ... }}` as an anonymous callable body instead of executing the block while evaluating the argument. The expected kind must retain function/effect restrictions; unresolved function/procedure overload ambiguity cannot be settled by silently executing the block. H-322 replaces the isolated inline-callback review with a unified owner-aware proposal for named and anonymous callables. Region-only reference captures, explicit copied bits and closure storage/lifetime requirements are reviewed in [D-14](RegionalOwnership.md#unified-callable-captures-and-region-owners); neither live lexical binding capture nor a blanket receiving-call-only restriction is selected. This is a language contract question, not permission to expose compiler callable-routing objects to source programs.

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

An admitted owning parameter transfers responsibility at callee entry, after argument evaluation and conversion succeed. An external borrowed parameter retains the original owner without acquiring cleanup responsibility. H-271/H-272 requires explicit owner-generic declarations for external borrowing; the native prototype's unqualified `region :Root` borrowing spelling is a migration boundary, not the new default. The call itself has not consumed an owning argument if a later argument fails before entry; explicit transfers performed inside earlier argument expressions retain their actual effects. Validate availability and provenance again at the call boundary, including duplicate moves and later-argument invalidation of earlier views.

After entry, an owning parameter's region belongs to the callee scope until returned or otherwise transferred; an exceptional callee exit releases what that scope still owns. The caller's original name remains consumed. An owning result transfers only after the callee's required finally work completes successfully; a result abandoned by finally failure remains subject to cleanup. This preserves the existing per-argument evaluation/conversion order without treating ownership transfer as a numerical conversion.

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

The current direct LLVM path lowers protected scopes, exact catches, catch-all/rethrow and finally into basic blocks. System-service wrappers report bounded allocation status; generated edges perform scope cleanup and enter the matching handler. The service protocol is internal and does not expose a mandatory Result type or native C++ exception ABI to user code. Handler registrations are compiler metadata. Catch matching uses the source type identity at a known throw and preserved propagation information after a catch-all; the scalar payload contains no type descriptor or native address.

The standard vocabulary `std:exceptions` exports `OutOfMemory`, `RegionCapacityExceeded`, `InvalidCapacity` and the common `SystemException` catch-all value type. Normal using/import alias rules apply. A caught value exposes read-only scalar details: `site`, `requested`, `available`, the diagnostic `category` code and `cause_count`. `cause_at(error, index)` returns the indexed cause as a `SystemException`, starting at zero for the direct cause; an out-of-range index is a terminating runtime check. The code is bounded system-failure information, not general type reflection. Source sites are resolved against complete immutable program metadata when reporting an uncaught failure, including causes originating on preceding loop iterations.

[Native evidence](../tests/cod/language/exception-validation.json) covers normal/handled expression results, nested finally, rethrow, replacement without automatic chaining, exceptional function calls, short-circuit and loop paths, and restoration of captured/hidden dynamic resources. A real `std::bad_alloc` injected at the provider boundary checks finally before inner-owner release, release before catch, and preservation of the outer owner until exit. LLVM assembly/bitcode verification and existing graph/pool relocation also pass. This is bounded evidence, not completion of E1–E5.

Typed `make`, root initialization/access, ordinary owning locals, borrowed/owning calls/results and branch/exception joins have [focused native evidence](../tests/cod/language/ownership-lifecycle-validation.json). The LLVM path keeps acquisition identity separate from owning-holder availability. A consumed holder becomes unavailable to source code; the region can remain live under another local holder. Pending owning results stay protected through finally. Argument-conversion failure uses the ownership and cleanup state at that argument, before later argument effects.

[The ownership/control audit](../tests/cod/language/ownership-control-audit.json) additionally covers choosing one of two consumed input owners, releasing the unselected region, generic forwarding, captured/current symbolic calls returning ownership, and caller-visible edits retained after a borrowed call fails. Reference comparisons revalidate both operands after evaluation, because a later operand can end an earlier borrow's owner.

`throw x from e` executes with the retained-chain lifetime below. Exact and common exception fields support make, allocation, reads and field updates. Common `SystemException` storage retains its bounded category alongside the payload, so rethrow after reload selects the original category. The current scalar payload uses four u64 fields (site, requested, available, cause handle); common stored carriers add one u64 category. A native type descriptor is not embedded. [Current evidence](../tests/cod/language/exception-ownership-validation.json) covers function return, storage beyond catch and original-region release, regional relocation, dynamic argument selection and uncaught formatting.

### Retained explicit-cause lifetime

H-269 requires the explicit cause chain to survive returning or storing a caught exception beyond the catch. Intrinsic runtime-managed storage retains the relationship; creating a chain may itself raise OutOfMemory. Basic OOM propagation does not allocate. A failed chain construction propagates the new OOM without automatically attaching either operand. Exception values and cause records are immutable: `throw x from e` supplies e as the explicit cause of the thrown value without mutating saved x or e values. It replaces the thrown copy's preceding direct cause; that preceding relationship remains in any unchanged copy of x.

The bounded runtime uses execution-local numeric handles for immutable cause records. Regional fields contain bits and handles rather than host addresses; copying a regional image within the same execution does not require pointer patching. Generic exception storage also preserves its bounded failure-category discriminator. These handles, like source-site IDs, are not a cross-process persistence or delivery format. General user exception graphs and general RTTI remain excluded.

Retained backing storage is released at execution end, including uncaught failure. The initial implementation retains discarded records until that boundary; repeated explicit chaining consumes that storage and may raise OOM. It does not add per-value destructors, graph ownership or eager collection. Public inspection uses a bounded cause count and checked indexed access; inspection and propagation allocate no cause records. Exact reclamation optimizations can preserve this source lifetime contract later.

The native cause table is created only by explicit chaining. Its TLS slot is a trivial pointer, with no lazy destructor registration on basic OOM propagation or cleanup. Failed table creation, first-record allocation or later growth leaves existing histories intact. The native fault harness fails the actual allocation at each of these three boundaries, verifies a fresh unchained OOM, retries successfully and accounts for all backing allocations/releases.

### Allocation-to-publication cleanup

The regional allocator releases its byte block if root construction throws, using the same allocator and exact capacity. Both overloads share that guard. [Focused native evidence](../tests/shilos/region-construction-validation.json) checks failure before root entry, both throwing-root forms and successful retention/release; a published-source negative control fails the release assertion. Existing CoD relocation/pool/capture checks pass. This repair protects the backend construction boundary; the evidence above separately covers plain language exception control. Typed construction has separate owning-lifecycle evidence above. No existing-region slot rollback or graph transaction is implied.

## Bounded acceptance and remaining scope

[The E1–E5 audit](../tests/cod/language/exception-ownership-validation.json) records the required build, five selected language/LLVM checks, native provider/chain failures, 19 exact owning-region acquisitions/releases, 15 invalid-owner/view cases, six invalid-exception cases, graph/pool/capture regression and the shilos throwing-root check. It maps each accepted requirement to observed behavior on one compiler revision. Earlier reports above retain their original compiler/source checkpoints.

Owner reassignment, the full const/place/assignment surface, mailbox transport, general user exception graphs and RTTI remain separate work. Cause handles are execution-local, and discarded histories occupy storage until execution ends. No full-suite or cross-process exception acceptance is claimed.

## Type routing and handler scope

A try scope supplies its accepted exception type identities and cleanup/handler destinations. At a statically typed throw, WPO can resolve the destination and emit a direct branch. The routing information is distinct from the exception payload; it does not require general RTTI in each exception value. Catch-all rethrow must preserve the arriving exception identity in the propagation context. Merely retaining the handler's accepted identity cannot distinguish otherwise identical incoming values.

The user's H-268 suggestion motivates this separation. A runtime handler-registration table is not required by the source semantics or selected merely by the analogy; compiler-local handler scopes and direct LLVM edges are the first implementation route. Explicit-cause lifetime remains a correctness obligation: no dangling catch-local references, silent truncation or allocation-dependent basic OOM delivery.
