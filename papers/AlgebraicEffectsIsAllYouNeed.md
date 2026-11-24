<a id="algebraic-effects-is-all-you-need"></a>
<a id="do-source-and-graph-edits-agree-on-an-effectful-policy"></a>

# Algebraic Effects Is All You Need

**A Software Defined HLVM over relocatable functor graphs**

## Abstract

What would a high-level virtual machine use for instructions if its programmer worked in
fulfillment rather than machine code? A warehouse policy can be a directed graph admitting
cycles (DCG) in a relocatable region. Each instruction is a *functor* in the C++ sense:
an object with fields and a call operation. Its fields describe this particular instruction.
Its type provides behavior through the compatible executor. Calling a warehouse node can
issue a typed reservation operation and use its reply to choose the next edge. Software
defines both the instruction vocabulary and how the environment interprets those operations.
We call this interpreter a **Software Defined HLVM** (High Level Virtual Machine).

We ask how far a fixed vocabulary, typed effect requests and their interpretations suffice
to run independently authored business programs. We derive the execution of a shared
warehouse fallback, change its handler, and reopen its program graph at a different base
address. A small BASIC loop tests the account on a cycle and an observable output effect.
The economic proposition is that a change expressible in the vocabulary can be delivered
as a new program image and run by an existing native executor. The performance proposition
is that coarse domain instructions can amortize graph dispatch, with LLVM specialization
available for stable workloads. Both have explicit comparisons below. A CoD program that
edits a source graph with AI assistance is one authoring application of this machine.

## The instruction is a regional functor

Consider one instruction, `TryWarehouse(B, fallback)`. A warehouse programmer can inspect
its operand B and follow its decline edge. At run time, invoking the node means asking for
a reservation of the current order at B. Acceptance yields a shipping decision for B.
Decline transfers control along the edge. A service fault follows the handler's failure
contract. The inventory call is the effect. The decision about what to do with its answer
belongs to this instruction. `Backorder` is a second instruction which finishes without
requesting inventory. The two alternatives deliberately provide no way to reserve at B and
then ship from a different warehouse.

| What has to survive | What supplies it in the warehouse machine |
| --- | --- |
| The program, across opens and executions | A typed disk-backed memory region (DBMR) contains entry roots, node alternatives, their operands and same-region references. Multiple incoming edges can name the same receiver. |
| The meaning of an instruction | The executor's selected module world provides the nominal receiver's call-operator methods. A method specialized on the active sum alternative can implement one instruction. |
| An execution of an order | The active invocation supplies the order, position or call stack, and selected bindings. Thousands of orders can enter one read-only image. |
| The authority and outcome of `reserve` | An installed handler in that execution context may call an inventory service, consult a simulator, or return scripted replies and record requests. Each interpretation owes its caller its declared contract. |

The functor is ordinary initialized regional **data that admits a call**. In CoD, `r(args)`
selects the visible `(())` family of the nominal receiver at the call site. Within that
family an alternative-specific method can read its receiver's fields. An effectful method
has `proc` permissions. No Haskell `Functor` instance or `fmap` occurs in this definition.
The bytes of the receiver do not include a process-local method address, a C++ vtable
pointer, a lexical callable view or an installed handler. Persisting the object in this
sense makes no claim about copying arbitrary C++ object representations. [CoD call-operator lookup](../design/DomainLanguageDesign.md#type-level-call-operator-lookup) · [Callable storage and access](../design/CallableStorageAndHereAccess.md#what-an-invocation-needs) · [C++ function objects](https://eel.is/c++draft/function.objects)

Here is a small CoD rendering of the instruction set for a service that returns a normal
Boolean answer. In this fixture the three valid warehouse IDs are positive, and zero denotes
backorder in the result. A domain package could give that result its own sum type.

```cod
data RouteNode =
    TryWarehouse { warehouse:u64; ref on_decline:RouteNode; }
  | Backorder {}

region Routing { ref retail:RouteNode; ref urgent:RouteNode; }

symbol @reserve:proc(order:u64, warehouse:u64) -> bool

proc (())(self:const TryWarehouse in O, order:u64) -> u64 where {const owner O} {
    if (behave @reserve(order, self.warehouse)) { return self.warehouse }
    ref fallback = self.on_decline
    return fallback(order)
}

proc (())(self:const Backorder in O, order:u64) -> u64 where {const owner O} {
    return 0u64
}
```

Calling the retail root starts the program. The active alternative supplies the method.
Only the decline case calls the next receiver. An executor may implement the tail call by
updating a cursor. Acceptance returns the warehouse already present in the node. A live
service that can report a fault needs a corresponding failure contract and a propagation
path from the instruction. Fabricating a normal `false` reply would issue an extra request
on the decline route. The listing isolates the two normal replies so that this extension
can be tested independently. [Functor method permissions](../design/DomainLanguageDesign.md#functor-receivers-and-callback-arguments) · [Calling an installed operation](../design/SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)

## What the HLVM shares with LLVM

LLVM gives producers of low-level typed SSA programs a common representation and machinery
for optimization and native code generation. A domain-specific HLVM draws a corresponding
boundary at the level of a business instruction. A generator, an editor or a compatible
image loader can supply a warehouse program. The executor can consume any checked program
from that vocabulary. `TryWarehouse` retains its warehouse, effect contract and decline
edge at this boundary. Lowering it earlier would oblige every tool interested in the
business meaning to recover those relationships. MLIR's dialects already provide extensible
typed operations and lowering. The particular question for CoD is what follows when the
*executable domain program itself* remains a relocatable regional graph with owned data
receivers and execution-scoped effects. [LLVM Language Reference](https://llvm.org/docs/LangRef.html) · [MLIR Language Reference](https://mlir.llvm.org/docs/LangRef/) · [CoD architecture, ``3 and 6](../design/RegionalSemanticPlatformCharter.md)

In CoD's planned architecture, LLVM supports native handlers and execution machinery. A
generic native executor can already consume a new graph in its vocabulary. A stable
combination of graph revision, mappings, handler contracts and target can also justify a
specialized path. A specialization which inlines a recorded interpretation must be
reselected or invalidated when the execution changes to live inventory. Every path owes
the business program the same requests, in order, for the same inputs and replies.
[Typed DBMR programs and native executors](../design/SymbolicDynamicScope.md#typed-dbmr-programs-and-domain-vocabularies)

| Change requested by the business | What must be supplied again |
| --- | --- |
| Keep B in reserve for urgent orders | Edit one edge, check and publish the new graph. The vocabulary, generic executor and inventory handler can all be reused. |
| Run the same published program in an inventory simulator | Bind a compatible simulator for this run. The program image and generic executor can be reused. |
| Add a timed hold with a distinct expiry reply | Extend the vocabulary, its interpretations and the affected builders and validators. Establish a compatible executor before admitting such an image. |
| Retarget native execution | Prepare methods and handlers for the target. Reopening an existing image requires compatible regional layout, schema and semantics on that target. |

“All you need” makes a sufficiency claim at a specified boundary. For this vocabulary an
order, the selected entry, compatible instruction methods and a reservation interpretation
determine the observable execution. We can test that claim by supplying two implementations
with the same sequence of service replies and comparing requests and results. An instruction
that also reads a hidden clock could distinguish the implementations. The clock would have
to appear as an input or an effect before the claim could cover it. The choice of inventory
consistency, retry semantics and permission to reserve still belongs to the application.
Interpretations can themselves use a lower-level effects vocabulary. For example, a live
warehouse adapter can interpret `reserve` by issuing an atomic conditional stock claim,
while a simulation interprets the request by updating a private inventory. Reading a count
and decrementing later would implement a different contract under competing orders.
This is where authors must specify atomicity and faults. A persistent continuation or a
multi-shot resumption would need its own representation and laws; neither is needed to
choose a decline edge. The operations-and-interpretations account follows the algebraic effects literature. The
regional representation and its costs are additional hypotheses. [Plotkin and Pretnar, *Handling Algebraic Effects*](https://arxiv.org/abs/1312.1399)

<a id="2-one-executor-one-dsl"></a>
<a id="making-invalid-states-unrepresentable-in-the-stored-program"></a>
<a id="the-policy-and-its-executor"></a>

## Two entries share one fallback

The application graph has two named entry roots. The names `R`, `U`, `F` and `Z` below identify nodes in the example revision. The view shows both entries and every decline edge reachable from them. Acceptance is terminal at each warehouse and is omitted from the drawing.

```mermaid
flowchart TD
    R["Retail entry · R · try A"] -->|decline| F["F · try backup B"]
    U["Urgent entry · U · try C"] -->|decline| F
    F -->|decline| Z["Z · backorder"]
```

There is one F with two incoming edges. If a retail order and an urgent order both reach it, each execution issues its own reservation for B. Node sharing preserves program structure. Each invocation brings its own order, current position and effect occurrence. A second warehouse adapter or a simulator can supply a different interpretation of the reservation operation for a separate run. [Program, instance and environment](../design/RegionalSemanticPlatformCharter.md#32-程序执行实例与环境分离) · [Typed DBMR programs](../design/SymbolicDynamicScope.md#typed-dbmr-programs-and-domain-vocabularies)

The planner asks, **“Stop retail orders from trying B. Keep B available to urgent orders if C declines.”** The intended edit redirects R's decline edge from F to Z. U still points to the original fallback within the successor, and F still points to Z. This small case is acyclic. A source viewer should be able to display a back edge once and navigate it by identity. Whether a particular executor admits a cycle and what execution means in that DSL require separate rules. The warehouse executor considered here accepts acyclic reachable routes.

<a id="5-why-algebraic-effects-belong-in-this-account"></a>

## Run and reopen the same program

Take an urgent order for which C declines and B accepts. Invoking U emits the request
`reserve(order,C)`. Its reply selects F. Invoking F emits `reserve(order,B)` and returns B.
Starting a second order at R can reach the same F and issue a fresh reservation. An
optimization that memoizes F's outcome solely by node identity changes inventory behavior.
Install a recording handler with fixed replies at demonstration time. At deployment the
live handler receives the same operation and arguments at each visit. Equal reply prefixes
give equal request prefixes. Different inventories can legitimately produce different routes.

For the acyclic fixture this can be shown by induction on the remaining path length.
`Backorder` finishes immediately with an empty request trace. A warehouse node emits
one request. A positive reply determines the final result. A negative reply reduces the
claim to its stored successor. A declared fault propagates at the visited node according
to the same contract. This reasoning covers equal results with *unequal* request traces
as a detectable error. It makes no assumption that the live handler rolls back an external
reservation after a lost reply.

Now publish the checked image and map it at another base address in a compatible reader.
The retail and urgent decline edges still denote the same regional target F. Relative
references preserve that fact without writing a fixup into every node. The new reader
supplies a matching executor, opens its own authority for the handler, and starts a fresh
invocation. Comparing traces for both entries at two base addresses tests execution as
well as graph preservation. Check shared-target identity directly too: duplicating F
could pass every current execution test while changing the meaning of a later graph edit.
A view obtained from a consumed owner cannot be used to access the graph under its new
owner. A fresh validated view belongs to the current owner. Page faults, validation,
format compatibility and native dispatch still have measurable costs. [Regional ownership](../design/RegionalOwnership.md#local-view-validity-after-ownership-transfer) · [Region representation and relocation](../design/GraphGrainedMemoryManagement.md#one-region-per-pointer-graph)

## A second instruction set with a cycle

The proposed `hlvm-basic` application supplies a small independent check. Let its frontend
resolve BASIC line numbers to instruction edges once. A counter lives in the execution
instance. `PRINT` emits an output request. A conditional branch selects an internal edge.
Consider this written input for the proposed fixture.

```basic
10 LET N = 2
20 PRINT N
30 LET N = N - 1
40 IF N > 0 THEN GOTO 20
50 END
```

A recording handler must see `[2, 1]` in order. Launch two instances from the same image
and each must produce that trace under its own counter and handler. Reopen the image at a
different base in a fresh, compatible process with the BASIC executor and without the
frontend. It should visit the shared print node twice and give the same trace. A console
handler can instead display the two values. Replacing the taken back edge with the edge
to `END` yields only `[2]`. Here the equivalent textual BASIC edit supplies an independent
control. This example needs only instance-local state, branches and a typed output effect.
Saving a running instance across a crash would additionally require an explicit account of
the remaining computation, its state and the status of external effects.
[Deferred BASIC application](../applications/README.md#deferred-application-hlvm-basic) · [Execution instances and persistent continuations](../design/RegionalSemanticPlatformCharter.md)

<a id="7-what-we-can-show-and-what-to-measure"></a>

## What low cost and high performance would mean

Compare (A) a directly compiled CoD implementation of the same warehouse decisions, (B) a
native Software Defined HLVM executing the regional graph, and, if built, (C) an LLVM
specialization of B for a pinned configuration. Give all three the same inputs, handler,
reply stream, validation assumptions and observable trace. The generic executor is useful
even when specializing each revision would cost more than that revision's entire run.

| Workload | Quantities to report and failure it can reveal |
| --- | --- |
| Change only the retail decline edge | List which artifacts, packages and native components actually changed. Measure edit to first correct result, including validation, publication, loading and any recompilation. Rebuilding the generic executor for every such edit defeats this cost argument. |
| Fresh open of the same checked image | Record cold I/O, page faults, validation and first completed order separately. Test another base address and count per-edge repairs. A linear validation pass and a linear pointer-rewriting pass are different costs. |
| Warm execution with a deterministic in-memory handler | Measure time per visited node, requests per second, memory and code size. Vary path length, branch predictability, layout and instruction granularity. The cheap `PRINT` fixture can expose dispatch and locality costs that an inventory call hides. |
| Service-dominated execution | Supply the same service latency distribution to each path and report end-to-end latency separately. Record the ordered calls so that skipping a call cannot masquerade as speed. |
| Optional specialization | Charge compilation, extra code and invalidation as well as warm savings. At a saving of `d` time per order and a one-time cost `c`, a stable revision needs more than `c/d` orders to pay back compilation when `d > 0`. |

A low-cost, high-performance result would combine reuse across policy revisions with
competitive warm execution on stated workloads. The crossover between cheap branching and
coarse native operations matters more than a single peak throughput figure. These are
measurement designs. The traces in this paper are derived from the instruction contracts.

<a id="1-a-program-that-takes-a-program-as-its-input"></a>
<a id="what-it-means-to-edit-the-source"></a>
<a id="editing-the-source-graph-of-an-effectful-cod-program"></a>
<a id="3-what-the-business-programmer-writes"></a>
<a id="what-a-two-dimensional-view-can-establish"></a>

## Editing the source graph with CoD

The warehouse DCG can itself be the complete program source. There may be no ordinary
textual CoD program with equivalent source identities, shared nodes, cycles and bindings.
A readable projection can omit those details if it marks its boundary. A complete
exchange format would have to recover them. In paper notation the editor takes and
returns a `Program<Orders → Decisions>`. The executor subsequently runs the successor on
orders. Separating these two roles lets an AI help write a CoD editor without giving it
inventory authority. [Readable views and complete exchange](../design/RegionalSemanticPlatformCharter.md#82-可读视图与完整交换形式)

The planner selects R's decline edge in a bounded two-dimensional view. The viewer should
show the field, old and new targets, and both incoming edges at F. If a retail-only filter
hides U, the reviewer must be able to discover that other user or see that the claimed
impact is limited to the visible graph. A drag can propose an edge update. A request such
as “change only retail policies that still share their fallback with urgent” needs a graph
predicate. An AI-assisted CoD editor can check it before updating a private successor.
The drawing's layout supplies no persistent node identity.

<a id="4-the-program-edit-and-the-business-result"></a>
<a id="appendix-the-warehouse-revision-in-cod"></a>
<a id="the-stored-editor"></a>

### A CoD program that changes one edge

The editor uses the instruction set defined above, checking the relevant identities before the write.
[Reference identity](../design/RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator)

```cod
proc reserve_backup_for_urgent(region draft:Routing) {
    ref retail = draft.retail;
    ref urgent = draft.urgent;
    assert retail is not urgent;
    assert retail isa TryWarehouse;
    assert urgent isa TryWarehouse;
    assert retail.warehouse == 1u64;
    assert urgent.warehouse == 3u64;

    ref backup = retail.on_decline;
    assert backup is urgent.on_decline;
    assert backup isa TryWarehouse;
    assert backup.warehouse == 2u64;
    ref stop = backup.on_decline;
    assert stop isa Backorder;

    retail.on_decline = stop;
}
```

The assignment changes a stored reference field in the selected retail node. All the comparisons of target identity take place within the private draft. The equality check on `backup` would fail for two separately allocated, equal-looking B nodes. The checks also fail if retail and urgent have become the same entry target: changing that record would affect both. The routine assumes a validated input conforming to the two-entry fixture, including the acyclic routing rule. The checks in the routine cover the requested edit. The surrounding workflow must check the permitted *write set*, including changes outside those two paths if the source contains additional data.

The caller pins the published base revision and forks its reachable graph into a fresh writable region through a domain graph-transfer operation that preserves sharing. A borrowed target in the published base cannot become a stored physical edge in the draft. Between revisions, selected nodes must be related by the editor's recorded correspondence or a domain identity whose scope is declared. Two independently opened regions have distinct actual owners even when they represent corresponding policy nodes. The rectangular labels in the figure illustrate this correspondence without fixing an ID format. [Explicit graph transfer](../design/DomainLanguageDesign.md#dl-03) · [Checked owner and locator identity](../design/RegionalAndLexicalStorageRefactor.md#reference-identity-and-the-is-operator)

An AI assistant can generate the CoD routine from the planner's request and the inspected node relationships. Before it runs, the workbench supplies only the pinned input, a writable draft and the declared editing operations required for the task. The separately maintained checker should verify the resulting graph and compute its changes against the base. A preview can evaluate the old and new policy with a trace-recording reservation fixture. These are proposed workbench capabilities. Being able to write CoD source does not confer inventory credentials. The review record must say which capabilities the editor and preview actually received. [Authority and review](../design/RegionalSemanticPlatformCharter.md#103-权限审阅与执行分离)

<a id="the-program-edit-and-the-business-result"></a>
<a id="allocation-and-independent-reporting"></a>

### What the planner can check

The graph edit can be checked before either policy runs. A reviewer then needs independent evidence connecting the change in source to the requested change in fulfillment. A trace fixture supplies replies by order and warehouse. The table below gives **expected observations** for fresh runs of each revision. “Backorder” is a terminal decision. Each listed warehouse letter denotes one actual request in that run.

| Entry and fixture replies | Published base | Proposed successor |
| --- | --- | --- |
| Retail. A accepts. | Ask A. Ship from A. | Ask A. Ship from A. |
| Retail. A declines and B would accept. | Ask A, then B. Ship from B. | Ask A. Backorder. |
| Retail. A and B would both decline. | Ask A, then B. Backorder. | Ask A. Backorder. |
| Urgent. C declines and B accepts. | Ask C, then B. Ship from B. | Ask C, then B. Ship from B. |
| Urgent. C and B decline. | Ask C, then B. Backorder. | Ask C, then B. Backorder. |
| Urgent. C accepts. | Ask C. Ship from C. | Ask C. Ship from C. |

The two retail-decline rows distinguish a changed output from an eliminated service call. The urgent rows distinguish “reserve B for urgent work” from deleting B for everyone. A reporting client can read order IDs and shipping decisions without permission to reserve stock. To review the eliminated request even where both policies backorder, the reviewer also needs the effect trace. A reservation failure before a normal reply takes neither the acceptance action nor the decline edge. If the service committed before a reply was lost, the true inventory change can be unknown to the planner. The graph edit makes no promise to undo it.

<a id="6-native-execution-and-interactive-revision"></a>
<a id="one-session-two-policy-revisions"></a>

### Review and publish the successor

A proposed review record can be short enough to read during a change review. The values below describe the fixture and the requested outcome. A real review must derive them independently from the draft produced by the editor.

| Evidence | What the review should show for this edit |
| --- | --- |
| Request and base | The planner's retail-only instruction, the pinned policy revision and the two entry roots considered. Record which imports, callers and graph portions the impact check covers. |
| Graph change | R's `on_decline` changes from F to Z. U's target is still F. F still names B and declines to Z. The original source graph and the candidate have an explicit node correspondence. |
| Type, identity and authority | Every stored edge selects a valid same-region target of the declared type. The draft retains both roots. The edit used draft write access. The preview used a fixture handler with no live reservation capability. |
| Business consequences | For urgent orders, the ordered reservation requests and outcomes agree. For retail orders that A declines, the B request disappears. Include both rows where the final decision changes and rows where it agrees. |
| Conflict and publication | Compare the request's expected revision with the selected current base if the application maintains such a selection. Publish the validated private draft under a fresh name. Record the resulting revision and the base that a still-running execution uses. |

CoD's confirmed DBMR publication contract installs a complete new image only when its chosen final name is absent. Existing readers can retain their old version. Detecting that somebody changed the application's selection of the current policy requires an additional revision precondition. A proposed edit prepared against yesterday's base must be rechecked against today's selection before it can become the merchant's active policy. An occupied publication name, an ambiguous correspondence or an unacceptable capability leaves the reviewed operation incomplete. The reviewer should still be able to identify the previous policy and the reason publication or activation failed. [Fresh-name DBMR publication](../design/ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr) · [DBMR version completion](../design/ConstructionAndAcquisition.md#building-and-maintaining-dbmr-versions)

<a id="8-related-work-and-the-scope-of-the-claim"></a>
<a id="where-the-proposal-can-fail"></a>
<a id="conclusion"></a>

### How an editor could pass the wrong test

A candidate that redirects the urgent edge as well can appear correct in a retail-only
view. A candidate prepared from a revision that already split the fallback may type-check
while failing the sharing precondition. A decoder that duplicates F can pass every
input/output trace on the original program. Inspecting both roots and applying a subsequent
edit addressed to their one shared target distinguishes all three cases. Run the same
review against a conventional typed domain editor with identical inputs and permissions.
Hyper-code previously explored explicit links across persistent program source, data and
execution. The authoring question here is whether ordinary CoD graph transformations
and bounded visual review retain that precision as new domain vocabularies are added.
[Zirintsis, Kirby and Morrison, *Hyper-code revisited*](https://research-portal.st-andrews.ac.uk/en/publications/hyper-code-revisited-unifying-program-source-executable-and-data/)
