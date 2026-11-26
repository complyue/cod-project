# Callable storage and direct lexical access

Design revision: 19 · 2026-11-24 · Confirmed source contract

## Design authority

[D-18-FUNCTOR-VALUE](DesignAuthorityRegistry.md#d-18-functor-value) r19 defines [function value and functor value](#function-value-and-functor-value). The [project user’s terminology and assignment decision](../CHANGELOG.md#h-functor-value-distinction-20261124), P-721/P-722, requires distinct types and preserves the receiver’s actual owner and access. Check both directions of assignment, symbol installation and owner-qualified reception. Synchronous here use, regional storage eligibility and native representation have their own contracts.

D-18-HERE-STORAGE r18 applies the [grouped installation decision](../CHANGELOG.md#h-grouped-local-proc-install-20261124) and the [local handler decision](../CHANGELOG.md#h-local-proc-handler-body-20261124) to the installed methods’ existing capture, owner and restoration obligations (P-611/P-612). Verify the active body’s own frame through nested restoration. See [local procedure bodies](SymbolicDynamicScope.md#installing-a-local-procedure-body).

[Confirmed symbolic procedure forms](../CHANGELOG.md#h-lexical-symbolic-proc-20261123) applies the existing D-18-HERE-STORAGE activation and owner checks to the lexical symbolic family. The ended-frame example below checks both operations. [Lexical and dynamic calls](SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)

[H-FUNCTOR-ARGUMENT-20261119](../CHANGELOG.md#h-functor-argument-20261119) applies ordinary argument preparation at r15 under D-18-HERE-STORAGE/P-611–P-612. The functor and any synchronous forward retain the selected receiver’s identity, access and lifetime. A later rebind of its source slot leaves that identity intact.

[H-CALLABLE-TAIL-20261119](../CHANGELOG.md#h-callable-tail-20261119) and [H-DEFER-RETURNING-20261119](../CHANGELOG.md#h-defer-returning-20261119) continue D-18-HERE-STORAGE at r14 for result ownership and deferred inspection. Compare the two brace forms and an independent full scope nested in a defer.

[H-GENERATOR-RECEPTION-20261119](../CHANGELOG.md#h-generator-reception-20261119) continues D-18-HERE-STORAGE/P-611–P-612 at r13 for direct access to a lexically bound here parameter inside an immediate full scope. The D-29 nested-yield example checks activation identity and the two return destinations.

[H-HERE-FUNCTOR-20261118](../CHANGELOG.md#h-here-functor-20261118) continues D-18-HERE-STORAGE/P-611–P-612 at r12 for the [functor as a bounded here use](CallableViewsAndCaptures.md#forms-of-here-callable). A receiver must remain valid through every invocation and synchronous forward. The data layout, receiver preparation and native representation have their own contracts.

[K-GENERALIZED-FOR-20261118](../CHANGELOG.md#k-generalized-for-20261118) updates r11’s trailing-group dependency. D-29-GENERALIZED-FOR/P-623–P-624 owns the generator call forms and paired acceptance. D-18 preserves the existing lexical, control and nonescape contract.

[H-CALLABLE-MODEL-20261118](../CHANGELOG.md#h-callable-model-20261118) confirms r9 for [D-18-HERE-STORAGE](DesignAuthorityRegistry.md#d-18-here-storage), D-18-HERE, FUNC-VALUES, HERE-PARAM, HERE-GROUP and HERE-BREAK. P-609–P-612/P-615–P-622 connect charter §§3.2–3.3/4.3/6.1, C-65/C-73 and D-12-LIFETIME to explicit state ownership and the lexical origin of procedural work. [D-10](SymbolicDynamicScope.md#definition-installed-effectful-procedures) governs symbolic replay, [D-17](ExceptionControl.md#full-scopes-as-immediate-ordinary-calls) governs local completion and [D-29](RangedForAndGenerators.md#lexical-scope-and-completion) governs the precedent for protected body exits.

[H-HERE-BREAK-CONTINUE-20261118](../CHANGELOG.md#h-here-break-continue-20261118) confirms the two typed exits in r10 under [D-18-HERE-BREAK](DesignAuthorityRegistry.md#d-18-here-break), P-621/P-622. The block spelling identifies the originating call and keeps the result and cleanup destination explicit through forwarding. The table and forwarding case below are source acceptance criteria.

Acceptance covers function selection without an enclosing activation, direct slot writes, synchronous forwarding across different owner counts, both brace forms, early feedback, reentrancy, cleanup and rejection of escape. Native layout, inlining and migration strategy are delegated to mainline. Name-only activation checks the required live defining slots of the selected family.

## Callable value kinds

### Function value and functor value

| Term | Type | What the value provides |
| --- | --- | --- |
| **Function value** | `func …` | A capture-free function selection and its checked signature. It carries no bound receiver. |
| **Functor value** | `func … in O` | A selected `func` call operator bound to a particular initialized data receiver under its actual owner `O` and checked access. |

The final `in O` here qualifies the callable value as a whole. An owner on an explicit parameter or result, or the storage owner of a borrowed callable holder, has its own meaning. **Stored functor** refers to a stored functor value. Use **receiver** for the data object and **call operator** for the selected operation. A supplied `here` body has its own synchronous use and control rules.

Type-check the kind at initialization, assignment, symbol installation, argument passing and result delivery. Assigning a function value to a functor-value destination or a functor value to a function-value destination is a type error, even with identical parameter and result types and a provably live owner. A functor-value destination also checks the actual owner, receiver access and lifetime. Rebinding the source receiver slot leaves the selected receiver unchanged. [Symbol installation and reception](SymbolicDynamicScope.md#installing-a-functor-value)

### Here use and procedure calls

Use [function value and functor value](#function-value-and-functor-value) for the two kinds written with `func`. Ordinary function values can be stored, passed or returned under their callable contract. A functor value also carries its receiver’s owner and access requirements. A `here` value may be invoked in its active scope or forwarded to another compatible `here` parameter in a synchronous call. It cannot escape. `proc` declares a procedure or a symbolic handler signature. Even a named proc with an empty body cannot initialize a callable-value holder.

```cod
func plus_one(n:u64) -> u64 { return n + 1u64; }
func transform = plus_one;
val result <- transform(4u64);    # 5

proc ping() -> void {}
ping();                          # Direct procedure call.
# proc later = ping;              # Reject at initialization, even if never called.
# func disguised = ping;          # Declared proc cannot supply a function value.
```

| Surface | Source consequence |
| --- | --- |
| Variables, ordinary parameters and results | Reject proc-valued locals, parameters, returns and structural result fields. An omitted type, a default argument, a conditional or a generic instantiation cannot produce a procedure value. References and pointers cannot designate a procedure holder. |
| Procedure names and signatures | Named, imported, recursive and overloaded procedures remain directly callable. A declaration or type query can describe a procedure signature; using that signature in an ordinary value slot rejects, including via `typeof`. A `func` view must satisfy its complete contract. An incompatible proc method cannot be silently dropped or reclassified to make a family fit. |
| Symbolic handlers and execution scopes | `symbol @send:proc()->void` declares the handler protocol. Definition activation installs the scoped implementation and its replay data. `perform` and `behave` use it. A standalone procedural `{{ ... }}`, a loop body, a module top level and a deferred call to a named proc can execute without constructing a user-visible procedure value. Their existing control, binding, owner and cleanup checks still apply. |
| Receiver-bound calls | A stored functor value selects a compatible `func` call operator. A data receiver offered to a synchronous here parameter follows that parameter’s call and lifetime rules. A direct call on data may select an eligible proc overload. |

**Retained work.** An application that chooses an operation now and invokes it later under symbolic effects loses a first-class proc as its event representation. For a known set of operations, enqueue a data constructor and dispatch to named procedures at delivery. An open integration can publish a symbolic operation and establish a handler in the delivery context. This does not preserve a producer's independently selected handler after its installation ends. If the application actually requires an open collection of independently selected procedure callbacks, that is a capability the restriction deliberately gives up.

**Synchronous work.** A retry library can invoke caller work and catch a failed attempt while the caller is live. This uses the [here callable parameter](#an-effect-in-a-here-callback). A retry library expressed as a generator cannot catch a failure thrown by the supplied loop body. [Generator catch boundary](RangedForAndGenerators.md#generator-catches-handle-generator-work)

**Engineering assessment.** Excluding procedures from the common value-construction and type-admissibility paths is a bounded checker change in principle. Coverage includes inference, argument/result matching, type queries, ordinary and native adapters, and pointers to holders. Direct procedure calls, overload selection, generator handoff and internal handler installation still need their signatures. The seed currently shares image construction between callable kinds and contains both proc-holder and handler probes. Retiring support specific to proc holders while retaining function and handler behavior is migration work. The presence of fixtures supplies evidence of implementation reach, never a reason to retain a language capability. [Seed image construction](../cod/lang_llvm.hh) · [Holder fixture](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/callable_slot_method_tests.py) · [Handler fixture](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/symbol_contract_tests.py)

The main additional work for a here formal is to bind the supplied definition per call site, validate all uses and lexical owners across zero or repeated calls, recursion, exceptions and cleanup. An outlined body can use a borrowed activation link with a checked synchronous lifetime. Different call sites can supply different owner counts; each site's body knows the slots it uses. A backend may share code when it preserves the source association and checks. This design needs neither universal inlining nor a public procedural value ABI. No performance figure or prototype is claimed.

## What an invocation needs

| Form | Information the invocation needs | Storage and responsibility |
| --- | --- | --- |
| Static definition or module-world family | Semantic identity, selected interpretation, dispatch and compatible signature | Code and checked metadata may be shared. Registration and type queries do not activate a procedure or read a slot. |
| Ordinary `func` value | Selected family and view contract, including any required type evidence | The function body has no outer runtime slots or symbolic replay data. A value may need only a discriminator, or no physical storage at a fixed call. Its CoD placement remains lexical. |
| Direct lexical `proc` call | Selected definition, arguments and required bindings | The two-kind rule excludes ordinary proc holders. Direct calls can check requirements without constructing a proc value. |
| Standalone/trailing `{{ ... }}` or `here` callback invocation | Body entry and access to the particular defining activation's slots it uses | The frame or lexical owner owns its slots. An outlined entry may borrow a stable activation reference and use slot locators. A direct call can pass the relevant access explicitly. Neither case needs a retained frame. |
| Installed local handler, including a [grouped installation](SymbolicDynamicScope.md#installing-a-local-family-with-several-methods) | The written methods, their required live lexical accesses and symbolic replay data | Definition activation establishes the replay data. The installed record may borrow lexical slots and internally refer to already active handlers. A saved TLS binding must remain restorable on normal and exceptional exit. |
| Loop body and module top level | Generator handoff to a particular caller, or the incorporated module's own context | A loop entry also preserves its established return and continue destinations, feedback and unwind. Module fragments share the live context through shutdown. Neither requires an ordinary first-class callable value merely to execute. |

A `here` **parameter** bounds the receiver's use. The procedural actual is a literal written in procedural lexical code. A named proc or an installed handler cannot supply this argument directly. A function-valued parameter remains available for APIs accepting ordinary function values. Borrowing a callable *holder* with `in O` and constraining a supplied body's use with `here` are independent contracts. The former refers to a storage place. Hidden lexical access in a here body supplies no new projectable owner or allocation permission. [Callable owner projection](CallableViewsAndCaptures.md#owner-projection)

The number of borrowed owners is fixed at each source call site. Different sites can supply different bodies to one receiving method. Their entries interpret their own slot layouts. The receiver checks the promised call signature and use boundary once. Each body still resolves every dereference using the current base of the correct actual owner. Recursive and reentrant executions of the same source block can have different live activations. A native slot address cached across lexical backing growth, or a lookup of the most recent activation of a method, can access the wrong storage. [Shared owner information](RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations)

**Native representation is a compiler decision.** For one known function signature and selected implementation, the value can be the compiled entry address. A family can use a dispatch entry and a compatible view can use a typed adapter. Direct calls may eliminate the value. The type checker retains the signature and the selected module world. An entry address grants neither source identity equality nor regional storage eligibility.

An outlined here body can take an entry and a borrowed activation context. At a given site the compiler knows which declarations are used. At runtime it must identify the instance supplying each slot, including through recursion and synchronous forwarding. A stable lexical owner plus slot offsets can suffice across backing growth. Inlined code or explicit access arguments can suffice elsewhere. The receiver does not require an owner count in its signature, an array of slot addresses or a heap allocation. Passing an escaping frame or caching an address across possible relocation would violate the source rule.

The call at depth zero below invokes a body supplied by the invocation where n is one. Its code location is shared by both recursive calls. Looking up the most recent activation of down would read zero.

```cod
func down(here previous() -> u64, n:u64) -> u64 {
    if n == 0u64 { return previous(); }
    return down(n - 1u64) {{ return n; }};
}
val result <- down(2u64) {{ return 99u64; }};  # 1
```

## Scoped replay without a first-class handler

```cod
symbol @quote:proc()->u64;
symbol @job:proc()->u64;
val fee <- 10u64;
effect @quote() -> u64 { return fee; };
effect @job() -> u64 { return behave @quote(); };
fee <- 12u64;
val observed <- perform @job();    # 12: the selected quote reads the live fee.
# proc escaped = @job;            # Reject, even at module lifetime.
```

A library can define @job to invoke @quote by symbol or call a lexical helper by its definition. In contrast, calling a saved proc whose body invokes @quote rejects even within the installing scope. Returning the installed @job itself would already be forbidden extraction. These checks address separate operations.

An activated @job may internally retain the binding selected for @quote. Its lexical slot stays in the activation that owns fee. Scoping establishes a useful ordering: an outer binding which is already active can be used during an inner installation, then restored when the inner scope exits. Re-entry creates distinct installation records even at the same definition site. A copied native frame pointer cannot establish owner liveness. An independently released regional target, a prepared deferred call in a longer-lived scope, or a module handler referring to an ended inner frame still needs a proof or rejection. A deferred block containing `behave @job()` needs a valid current binding when it runs. None of this requires exporting runtime installations as CoD callable values.

Mainline's native probe returns a lexical bridge containing a saved procedure-valued symbol and prints 1 under the implemented snapshot semantics. Its eventual invocation through the returned proc must reject under the source rule. This probe identifies a compiler migration case. Mainline's evidence about selected-value dependencies, TLS restoration and delayed use still applies to internal handler records. [Current runtime implementation boundary](CallableViewsAndCaptures.md#runtime-callable-contract)

### An effect in a here callback

**Control returns to the originating lexical scope.** The same source rule explains a generator reaching `yield` and a function reaching `body()` on a here formal. Each hands execution to caller-authored code with the live slots, owners and permissions of the particular activation that supplied it. Forwarding preserves that destination even through recursive calls. Forwarding itself executes no body. The functions between the invocation and the originating scope stay active, including their pending cleanup.

| At the handoff | Here callable supplied by `{{ ... }}` | Lexical `for` body supplied to a generator |
| --- | --- | --- |
| Normal completion | Each invocation opens a block-local scope. Its `return` and cleanup finish that invocation; the result goes to the site that invoked the formal. | Normal completion or `continue` supplies the declared feedback to the suspended yield. |
| Abrupt completion | The body’s declared failures propagate to the invocation site, where a surrounding catch may handle them. | A loop-body exception or enclosing return passes through the generator with its cleanup. |

The shared rule concerns who owns the executing code. Handoff alone does not rewind dynamic bindings to the time the literal was supplied. A permitted `behave` uses the currently installed binding; `perform` applies the selected handler’s own replay rule. [Generator effect boundary](RangedForAndGenerators.md#callable-kind-and-supplied-body-effects) · [Full scopes](ExceptionControl.md#full-scopes-as-immediate-ordinary-calls)

A single **here callable** formal describes the parameter modes, result and checked failures of a synchronous call. The receiver can invoke the body locally or pass it directly to a compatible here parameter of another func in a nested synchronous call. The parameter binds a scoped `here` value. Write `here body(...) -> T` in the declaration and attach the body directly after the call. No second keyword is needed. Both lexical kinds use the same formal and the usual trailing-block syntax.

```cod
func retry_once(here body() -> void throws RetryableException)
    -> void throws RetryableException {
    try { body(); }
    catch (e:RetryableException) { body(); }
}

func deliver(here send() -> void throws RetryableException)
    -> void throws RetryableException {
    retry_once(send);                # Forward the same body to another here formal.
}

proc send_order() throws RetryableException {
    deliver() {{ behave @send(); }};  # @send has the declared checked contract.
}
```

The function owns retry logic, including the decision to run the block again. The procedure owns the literal and its permission to invoke @send. A return in that block finishes only the current body invocation. A checked failure first runs the block's cleanup and reaches the function's ordinary catch; a second failure escapes under its declaration. Receiver arguments to `body(...)`, its catch, and its own cleanup obey the receiver’s func rules. A func cannot invoke a named proc or a symbolic procedure there. Execution enters the supplied definition with the dynamic bindings then active. Only an explicit perform inside the block requests the selected handler’s replay. [Trailing bodies](SymbolicDynamicScope.md#trailing-callback-arguments) · [Loop body catch boundary](RangedForAndGenerators.md#generator-catches-handle-generator-work)

```cod
func twice(here body() -> void) { body(); body(); }
proc notify() { behave @log(7u64); }

proc on_event() {
    twice() {{ notify(); }};       # Direct named proc call in on_event's block.
    # twice(notify);               # Reject: named proc supplied as an argument.
}
func compute() {
    twice() {{ val n <- 7u64; }};  # Function-authored block.
    # twice() {{ notify(); }};     # Reject even when a proc called compute.
}
```

Use the nearest **lexical** callable owning the literal. A nested named func establishes func permissions, even when declared or invoked inside a proc. Another nested procedural literal can retain only the authority its lexical site already has. A function's `body()` instruction can enter this supplied block; the instruction does not make any new `{{ ... }}` written in the function procedural. The literal-only path also prevents passing `@log` itself. Under the stated proc-only creation rule, a bare module top level obtains no extra eligibility to supply a **procedural argument** merely by being a here execution scope. A named proc called from module code can supply it. Existing top-level execution and standalone immediate-block permissions retain their own contracts.

The receiving activation may execute its body zero, once or several times. The block uses its own defining activation’s live slots, with the actual owners established at that call site. Check invalidation between calls, including exceptional exits and cleanup. At `retry_once(send)`, the second activation borrows the original body for its own, shorter call. Each further receiver checks the same here contract. Forwarding preserves lexical origin, slot access and the checked interface without creating a block or taking a symbolic snapshot. Each receiver remains responsible for uses and cleanup before its own return or unwind; neither success nor failure leaves an invocation right behind. The receiver can be checked once against the formal; the call site checks its supplied body. Native execution may pass a borrowed entry and activation reference. Neither one becomes a CoD procedure value.

**Storing the receiver.** The receiving `retry_once` is itself a function. A function view whose signature preserves this here parameter can still receive a fresh literal at a later call. Storing that function cannot preserve a previous caller's block. Every implementation admitted by its view must satisfy the same use, argument, result and checked-failure contract. Converting the parameter to an ordinary callable-value slot or omitting its here requirement from a compatible view would change that contract. Checking symbolic requirements at each possible body invocation still applies to indirect calls via such a function view.

A function taking a here callable may run caller-supplied effects. Optimizers must account for its calls even when the body result is unused; a func declaration alone supplies no purity, duplicability or reorderability guarantee. Future pure functions require separate whole-invocation evidence. The receiver's parameter type and argument/result checks are established before the body is checked. Which names the block uses cannot choose an overload or introduce a second overload distinguished by here-proc versus here-func.

### Single-brace bodies for a here callable

Write `visit() { ... }` to supply a group in its author’s lexical context. An ordinary receiver declares the parameter with `here`. A matching generator declares its body protocol with `yields`. Calling or forwarding the here parameter keeps track of both the body and its original control destinations. With `visit() {{ ... }}`, the supplied block has its own callable return and ordinary callback exception contract. Consider the same library definition with both spellings, assuming an installed @send that can throw RetryableException:

```cod
func retry_once(here attempt() -> void throws RetryableException)
    -> void throws RetryableException {
    try { attempt(); }
    catch (e:RetryableException) { attempt(); }
}
proc send_order() throws RetryableException {
    retry_once() {{ behave @send(); }};  # An initial failure gets a second attempt.
    retry_once() { behave @send(); };    # An initial failure leaves this call immediately.
}
```

One here formal accepts either spelling. Forwarding the formal preserves the supplied spelling’s control behavior. No overload or parameter modifier is needed to select it. A catch *inside* the supplied group gets its usual opportunity. An exception escaping the group reaches the lexical caller at the supplying call expression, after unwinding the receivers. Catches around `attempt()` in retry_once or in a forwarding helper cannot intercept it, even when the exception originated in a callee of the group. The receiver can still catch exceptions from its own work. For static checking, account for escaping group failures at the supplying call in addition to the receiver’s fixed throws obligations. A receiver catch cannot discharge the supplier’s obligation. The double-brace form still checks its body against the expected callback throws contract.

```cod
func visit(here step() -> void) {
    try { step(); }
    finally { release_visit_resources(); }
}
func relay(here step() -> void) { visit(step); }
proc first_match() -> u64 {
    relay() { if (matches()) { return 7u64; } };
    return 0u64;
}
```

Here the outward return belongs to first_match. After a match, visit’s finally runs, relay unwinds and first_match delivers 7 if cleanup succeeds. When the group falls through, visit resumes after step() and first_match can return 0. Every receiver runs the cleanup it actually established. A receiver-local return from cleanup cannot convert the pending caller transfer into success at that receiver. A new escaping cleanup failure follows the generator’s existing replacement/aggregation rules. A protected outward transfer must retain its target across nested calls; an ordinary catchable runtime exception would give retry_once the wrong behavior. [Generator cleanup](RangedForAndGenerators.md#cleanup-completion-at-the-for-boundary)

| What the source needs | Grouping behavior |
| --- | --- |
| Storage and defer | No new full-scope owner. `here` names the supplier’s existing lexical owner. A direct `defer` registers in the supplier’s nearest full scope each time the group runs. A delayed borrow must survive to that scope’s drain. A header name is bound for its invocation; a saved bits argument can outlive that binding, a borrow of an expired receiver slot cannot. |
| Normal result | For a non-void formal, use the ordinary group expression’s result on every normally completing path, as with generator feedback. A caller-targeted return or throw produces no value for step(). For instance, `{ 3u64 }` can supply 3 to a u64 formal; `{ return 3u64; }` exits the enclosing callable. For a void formal, discard the normal expression result. An explicit `continue` supplies feedback under the formal’s result contract. A `break` supplies the result of the call to which the body was attached. |
| Parsing | A trailing single-brace group or a for-loop body supplies the same first here parameter. The generator declaration names it `yield`. [Generalized for-loops](RangedForAndGenerators.md#generalized-for-loops) Use ordinary slot-one binding, with `(x) => { ... }` for a parameterized body. A `for` header’s body brace belongs to the loop. In expression position, `visit(); { ... }` separates a completed call and a group. Parsing must not attach a brace by trial overload resolution. |

For a u64-valued formal, let `plus_one` return `body() + 1u64`. Each line below can be the sole statement in a different u64-returning supplier.

```cod
return plus_one() {{ return 7u64; }};    # The local body returns 7; plus_one returns 8.
return plus_one() { 7u64 };              # Normal group result 7; plus_one returns 8.
return plus_one() { return 7u64; };      # The supplier returns 7; the addition is skipped.
```

**Library contracts.** A retry catches failures delivered by the double-brace body. A single-brace group lets the application leave the operation. This is useful for a caller abandoning a search or transaction even when a library catches errors from its own work. A transaction protecting an invariant puts rollback in finally or defer, guarded by whether commit succeeded. Code after `step()`, or a catch around it, only covers paths that actually reach it. An API that must route every attempted operation through its own recovery can expose a named symbolic operation in a proc, or accept an ordinary function value if explicit inputs suffice. There is no business requirement here for a second here formal or a per-parameter opt-in.

**Operational details borrowed from generators**

- On each actual invocation, bind header arguments afresh. For a form such as `(item) => { ... }`, names are visible only in that invocation. Reuse the generator-body restriction on direct local declarations: repeatedly invoking the group cannot publish or repeatedly initialize a new caller variable. Declare persistent state before the call or use a nested explicit full scope for fresh locals. Inside such a nested full scope, an explicit return is local again.
- Adopt the generator’s **per-invocation effect restoration** separately from full-scope ownership. Each invocation starts with the active dynamic bindings. Restore changes it installed, and release its fresh effect-owned resources, before resuming or unwinding the receiver. Persistent writes through valid targets remain observable. This avoids editing a caller binding underneath an active receiver shadow. It is an observable rule attached to invoking the group, including normal completion; ordinary unattached grouping keeps H-319. Direct defer and `here` still belong to the lexical supplier. [Loop scoping](ExceptionControl.md#loop-scoping-rules)
- For example, from a proc with @rate initially 10, `twice() { defer record(@rate); effect @rate <- 20u64; };` registers two prepared calls if twice invokes the group twice. Each prepares 10; both run at the supplier’s scope exit. The second invocation also starts at 10. Placing `defer record(@rate)` *after* the installation instead saves 20 on each invocation, assuming a bits parameter. A deferred block which reads @rate at drain time requires a valid binding then. A registration into a scope already draining rejects under D-17. No implicit per-invocation user-defer queue is needed.
- Fully prepare a normal result or outward return under its actual access and owner contracts. A borrowed reference into an effect-owned region being released cannot be delivered through either exit. For a non-void formal a group whose every path exits the supplier owes no normal value; a recovered body-local catch or fallthrough still owes the declared result. An early return is checked against the supplying callable, and the caller must still handle possible fallthrough if the receiver can invoke the group zero times. A receiver’s loops and yield are outside the body’s lexical context.
- The lexical destination includes the particular activation and the applicable handler at the supplying expression. For a single-brace body, a checked failure can pass outward even with a no-throws here formal: it is checked at the supplying call. A receiver catch that would cover the same type when supplied double braces does not count. Independently declared receiver failures remain caller obligations. Passing a here formal through another func preserves this distinction, including generic calls and calls through compatible function views. The compiler must keep the original catch/return destinations and referenced owners live for every invocation, including invocations during cleanup. Simply retaining a link to a live outer frame would not prove a particular local catch still active.
- Protect only a transfer that actually leaves the group. Called helpers keep their own local returns. A group-local try can recover its helper’s exception and resume normally. If a receiver’s finally calls an unrelated failing helper during unwind, that failure follows ordinary cleanup replacement; a normal receiver-local return cannot overwrite the supplier’s pending result. A caller-owned deferred failure stays with the caller’s own defer pass. A forwarding helper does not wrap or aggregate the same body exception again at each hop. These are acceptance cases for the same target-aware control paths required by generators.
- An implementation can use a private completion result for normal feedback, break with its originating call destination, outward return with its supplying callable destination, or a body failure crossing to the supplying call, and route each through normal cleanup. A receiver failure keeps its ordinary path. The double-brace entry instead uses ordinary call completion. A shared backend still needs proof and coverage for both brace forms, recursive forwarding, cleanup failure and exactly-once release of abandoned owning results. Existing generator design supplies a semantic contract; native implementation completeness is a separate matter.

<a id="early-result-from-a-here-body"></a>

### Break and continue in a single-brace here body

`continue expr;` completes the current invocation of the body. The code that called the here formal receives the feedback and resumes. Only another invocation runs the body again. `break expr;` completes the original call expression to which the body was supplied. The supplier receives the value at that expression and resumes there. An explicit `return expr;` exits the callable in which the group was written.

```cod
func plus_one(here body() -> u64) -> u64 {
    return body() + 1u64;
}
proc demo() -> u64 {
    val n <- plus_one() { break 7u64; };
    return n + 100u64;
}
```

| Body supplied by demo | What receives 7 | Result of demo |
| --- | --- | --- |
| `{ continue 7u64; }` | One invocation of body. Both additions run. | 108 |
| `{ break 7u64; }` | The call to plus_one. Only the addition in demo runs. | 107 |
| `{ return 7u64; }` | The caller of demo. Neither addition runs. | 7 |

A normal tail `{ 7u64 }` and either local spelling `{{ 7u64 }}` or `{{ return 7u64; }}` also yield 108. A nested ordinary statement group preserves the enclosing here body's destinations. A separately supplied here group establishes its own targets. Forwarding an existing formal preserves the original association, including the particular call activation under recursion. A nested full scope has its own callable boundary. Inside a generator loop body its own `continue` follows the [generator feedback contract](RangedForAndGenerators.md#explicit-continue-feedback).

```cod
func relay(here f() -> u64) -> u64 { return f() + 1u64; }
func matches(here f() -> u64) -> bool { return relay(f) == 8u64; }

val a <- matches() { continue 7u64; };  # relay gets 7, matches compares 8, a is true.
val b <- matches() { break false; };    # matches delivers false; the comparison is skipped.
```

Use the established body signature for `continue` and the established result contract of the original receiving call for `break`. In the second example `continue false;` and `break 7u64;` fail their respective contracts. For a void destination use the bare form. For a value destination supply the required payload, including its mode, fields and owner evidence. An exit operand provides no independent anchor for overload selection or result-type inference.

Prepare the operand once. Complete body-exit cleanup and restore per-invocation effects. A successful `continue` resumes at the active invocation. A `break` unwinds all active forwarding frames through the original receiving call, running the cleanup each frame established. User catches cannot intercept a successful control transfer. A defer registered on the supplier stays on its own list. An escaping cleanup failure follows the existing protected-exit rules. Deliver a result only after required cleanup succeeds and all selected result owners remain live. [Cleanup at a protected boundary](RangedForAndGenerators.md#cleanup-completion-at-the-for-boundary)

A delivered `break` counts as successful completion of the original call. Every promised `init` write must therefore be proved on that path. For example, if the receiver invokes the body and only then initializes an output, a breaking path which skips that write cannot report success. A `continue` permits the receiver to resume and perform the write. Intermediate calls abandoned by a break do not publish a normal result. [Initialization obligations](ConstructionAndAcquisition.md)

## Where stored values differ from live slots

| Operation | Observable consequence |
| --- | --- |
| Construct, assign and invoke | Activating an effectful definition creates its internal snapshot. For eligible ordinary callable values, preparing an argument, replacing a holder with `<-` or returning the value preserves the selection. An independent destination can be replaced without changing another holder. A nullable ptr to a holder observes its current contents. Reference `is` tests storage identity. Equal code, snapshots or native addresses do not establish that identity. |
| Replace an active callee | Prepare the ordinary callee before later arguments or nested calls can replace its holder. Its selected value stays usable until invocation and cleanup finish. A here body reading that holder by name observes the then-current contents at each execution of the read. |
| Prepare deferred work | In a scope with `func f = first;`, `defer f();`, `defer { f(); }` and then `f <- second;`, normal drain calls second and then first. The block resolves the live slot at exit. The plain call retains the value it prepared at registration. Any saved here access must fit the actual drain, including partial startup and shutdown. [Deferred execution](ExceptionControl.md#deferred-work-and-explicit-scopes) |
| Write through direct lexical access | A here body can change the outer scalar, rebind its mutable ref slot, consume an owner or invalidate a caller's type/nullability refinement. Check zero, one and repeated invocations on normal and exceptional paths. A callback that consumes its outer owner once cannot be silently run twice. A callee that may skip the callback cannot establish definite initialization merely by accepting it. |
| Return or receive | Ordinary callable results may be constructed into the caller's lexical destination, including structural fields. Prepare a replacement completely before publication. Unwind partial preparation, discarded fields and abandoned results exactly once. A new holder does not extend any borrowed target, here activation or default-provider owner. A here entry cannot be smuggled out through a result field, nested procedure snapshot or symbolic binding. [Result reception](ConstructionAndAcquisition.md#structural-result-types--return-only-proposal) |
| Prepare a functor | Select the target once when evaluating the argument. Invocations read that target’s current fields. A subsequent rebind of the source reference affects an explicit callback that reads the reference at invocation. A computed receiver is evaluated once even if the callee never invokes the predicate. [Functor arguments](DomainLanguageDesign.md#functor-receivers-and-callback-arguments) |

Direct forwarding requires a visible, compatible here formal. Passing `body` to an ordinary function-value parameter, hiding it in a returned function or registering it for use after the supplying call rejects. A helper may catch a declared failure or propagate it under its own throws contract. A recursive forwarding edge has the same nesting and cleanup rule per activation. The checker must include mutations and possible repeated invocations in the caller’s flow analysis; dropping the association at an intermediate func would make a later owner release unsound. Reusing the original checked entry and activation link is sufficient for forwarding. A new nested literal has its own lexical site: direct forwarding alone grants no right to capture a here formal into an escaping callable.

## Two dependencies beyond the callable body

**A callable type can need a live default provider.** In the D-10 example, `typeof origin` supplies the recipe `{ fallback }`, whose selected target can change before the argument is omitted. Even a context-free target can therefore be called through a type whose default needs owner evidence. H-461 puts that dependency on the originating type contract. Keep checking its lifetime separately from the target's body and avoid a new per-value snapshot of fallback. A caller-context default `{{ ... }}` needs the caller's live slots on that omission. Its writes reach those slots subject to ordinary access checks. Type-only inspection checks the contract without reading or mutating either environment. For example, adapting the capture-free `func identity(n:i64) -> i64 { return n; }` to `typeof origin` leaves `adapted()` dependent on fallback when the argument is omitted. Calling `adapted(7i64)` skips the default. A checked call must preserve this default-provider dependency. [Named defaults and their origin](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding)

**A family installed by name or by a grouped effect can include several methods.** A module world can know that a here-dependent method exists after its defining invocation has ended. For a fresh activation, the checker needs valid lexical access for each reachable method which can require it. A runtime restore uses the original handler record, interpretation and dependencies. Skipping an otherwise selected method, dispatching to a broader one, or using the most recent installation would change source semantics. A conservative baseline rejects a fresh activation if any required frame cannot be established. Direct calls to an independently selected ordinary method remain possible under ordinary selection. More precise reachability, and how methods with different defining frames can be activated together in one installed symbolic family, remain open. [Method registration and installation](SymbolicDynamicScope.md#callable-effects)

```cod
symbol @price:proc()->u64;
symbol @install_price:proc()->void;
effect @install_price() -> void {
    val cents <- 4u64;
    effect @price() -> u64 { return cents; };
    assert (behave @price()) == 4u64;
};
behave @install_price();
```

At a later `effect proc @price;` or direct lexical call, the checker must prove that the defining activation for cents remains live whenever this method can be used. In this example the inner activation has ended, so that later use rejects. A method over initialized module slots has a different lifetime proof.

## Control, storage and interoperability

- A standalone here block owns its local return and normal tail result, cleanup and checked failures. Only its own deferred entries can inspect that result with `if returning`. A nested immediate full scope can call an outer lexical here parameter such as `yield`. Its own return and implicit catch context remain local. An explicitly accessible exception variable has its ordinary ownership checks. A loop's lexical body keeps its distinctive nonlocal return and generator cleanup. Module defers drain in global reverse registration order with the original module contexts. A tail-call optimization can only retire a frame after its final live-slot use and cleanup. [Full scopes](ExceptionControl.md#full-scopes-as-immediate-ordinary-calls) · [Loop completion](RangedForAndGenerators.md#lexical-scope-and-completion)
- An installed here handler can execute through synchronous dynamic callees while its installation and defining slots are live. A later call can replace or release a regional target even while the module or lexical frame still exists. Snapshotting a regional reference freezes target selection, and a write to the live referent remains observable. A here read of a ref slot observes its current selection. Restoration on unwind cannot revive an ended owner. [Owner replacement](RegionalOwnership.md#replacing-an-occupied-region-owner)
- Ordinary lexical callable holders, including functions represented by one word, remain ineligible for regional fields, regional arrays, DBMR persistence and unrestricted bits transfer. A region can store a relocatable data receiver. A structural result can deliver an ordinary callable to an eligible lexical destination. Permission for nominal lexical ADTs with callable fields has its own design scope. Foreign adapters must establish synchronous use, owner validity and cleanup before accepting a here entry. A native system retaining a callback for a later event needs a separate explicit lifetime contract. [Placement](CallableViewsAndCaptures.md#callable-views-have-lexical-storage) · [Foreign interfaces](RegionalAndLexicalStorageRefactor.md#foreign-interfaces-adapt-to-cod-semantics)

## Implementation impact and verification

In the seed compiler, `CheckedProgram::captured_symbols` takes the union of symbol demands over the selected implementations. `create_callable_image` combines ordinary captures and those symbols. `cod_lang_callable_create` lays out nested images with relative offsets, and the current copy/free path owns the resulting image. A raw frame link placed in an owning image/child field would be interpreted as an image or freed. During migration keep the borrowed activation reference distinguishable from owned replay data throughout preparation, temporary cleanup and unwind. Existing zero-capture materialization already omits image allocation. A physical split into separate allocations, a fixed-size handle and a permanent ABI are all optional. [Checked family evidence](../cod/lang_checked_program.hh) · [Image construction and invocation](../cod/lang_llvm.hh) · [Native image ownership](../cod/runtime/cod_lang_runtime.c)

The costly migration is in source analysis. Replace ordinary outer-value acquisition with diagnostics for ordinary definitions and live-place inputs for here bodies. Carry writes, owner invalidation, hidden dependencies and actual control destinations through call summaries, defaults, query checking and native lowering. Preserve type-only declaration evidence, active-call protection, strong replacement on failure and complete symbolic replay. Saved immutable replay payloads may share backing if independent holder replacement, owner liveness, observable preparation and exactly-once resource release agree. Cost measurements should distinguish view construction, replay, direct invocation and scope teardown. A smaller environment does not establish any speedup by itself.

Acceptance should pair a safe and a failing case at each boundary: two activations of one proc with different symbolic snapshots, a live ref-slot rebind, a nested handler replay and rejection of handler extraction, definition-origin default evaluation, higher-order use of a here formal, incompatible family activation, later-argument replacement, deferred call versus block, recursive activations with identical code, backing growth, owner replacement, conditional/exceptional exits, and query versus native behavior. The existing independent snapshot probes are migration evidence for the old observable result. Retain their distinction as explicit changed-result or rejection cases during migration. [Current implementation coverage](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/tests/cod/language/README.md)
