# Algebraic Effects Is All You Need

Working paper for author review · revised 19 November 2026

## Abstract

In CoD, the source data, the program that processes it, and the resulting data can each be a disk-backed memory region (DBMR). The interesting case occurs when the input is itself a program and the result can be executed. We call this *semantic homoiconicity*: ordinary CoD programs can inspect, construct and transform the typed program graphs that CoD checks and executes. A programmer can express the original computation with procedures, branches and functional composition. A second programmer can change the same computation through a typed graph edit. Algebraic effects give a published program operations whose answers determine its control flow and whose interpretation can change between runs. The architecture separates long-lived data type packages, a moderately stable native DSL executor and its authoring tools, and frequently revised application graphs. A DSL designer can encode a business invariant in the type of the persistent program itself, so a generator, a graph editor and an executor share the same rule. We edit an inventory policy's fallback branch as data, run the new policy in the same session and query its decisions with an independent tool. A small executable model demonstrates this cycle for a closed language fragment. The CoD research agenda is to give the full cycle a checked source correspondence, approach native throughput in steady execution, and approach the edit-to-result latency of interactive languages. Relocatable storage makes the graphs shareable across processes.

## 1. A program that takes a program as its input

A distribution center reports that its backup warehouse is unavailable. The fulfillment planner wants an answer before publishing a policy change: *which orders would we backorder if we stopped trying that warehouse?* The planner has yesterday's program, today's orders and a starting inventory snapshot. The useful edit changes one branch of the program. Changing today's inventory to zero at the backup gives a different answer to an operational question: the program would still contact the warehouse. If the warehouse is offline, that distinction can mean a timeout on every affected order.

Consider four invocations in one running session. The notation below describes artifacts and execution; it introduces no CoD API. `P₀`, `E`, `P₁` and `Q` are program DBMRs. `D` is an input DBMR holding orders and starting inventory. `R₀`, `R₁` and `S₁` are result DBMRs.

```text
R₀ = run_allocation(P₀, D)       fulfill under the published policy
P₁ = run_edit(E, P₀)             revise the policy's decline branch
R₁ = run_allocation(P₁, D)       fulfill under the revised policy
S₁ = run_reporting(Q, R₁)        count backorders in the decisions
```

The second line is the decisive one. A result produced as data becomes the executable operand of the next invocation after checking. The fourth line exercises the other direction: a result becomes input to further business processing. Orders, decisions and programs have different schemas. All three use the same regional facilities for allocation, graph access, inspection, checked editing and publication. A file extension or common serializer alone would establish none of these operations on the program's control flow.

The subscripts identify the executor required by each application graph. The editor understands the policy's published program schema. A reporting executor needs only the decisions schema to read the allocation output.

At the level of types, `P₀` and `P₁` have the business contract `Orders → Decisions` under a reservation effect. The editor has the metaprogram contract `Program<Orders → Decisions> → Program<Orders → Decisions>`; the query has the contract `Decisions → Summary`. These are explanatory signatures. In particular, producing a well-typed program graph and having authority to execute it against live inventory are distinct capabilities.

## 2. One executor, one DSL

The architecture assigns artifacts different expected rates of change. These are design expectations about ownership and reuse.

| Artifact | Typical change | Who can use it |
| --- | --- | --- |
| CoD type packages for source and result data | Longest lived. Order records and allocation decisions must remain useful across applications and versions. | Any CoD program that depends on the compatible defining package can read these values and write them under ordinary authority and publication rules. The source and result schemas may belong to different packages. |
| CoD executor binary, its DSL contract and companion generators | Medium lived. A new vocabulary, interpretation, optimization or capability may merit a release. | The DSL designer implements an executor for a typed vocabulary and supplies tools that generate valid app DBMRs for it. One binary runs many such applications. A compatible rebuild can preserve a DSL version. |
| Application DBMR | Ordinarily changes fastest. The merchant can revise warehouse preference or fallback while retaining a compatible executor. | A domain author generates it, writes CoD that constructs it, or edits an existing program graph. The executor checks compatibility and runs it. |

For example, an Orders package defines the input root and a Decisions package defines the result root. An independent reporting program can import Decisions, read R₁ and publish a summary. A correction tool authorized to produce decisions can also write a successor decisions image using that package. Neither tool needs the allocation binary, app graph or reservation handler. Updating a published read-only DBMR means constructing and publishing a new revision. Package compatibility, ownership and access still apply.

**An executor is a language implementation.** The allocation executor defines the meanings of conditional reservation, terminal decisions and the other operations its DSL admits. The companion tool can expose a `primary |> backup` constructor, an interactive form, or a textual domain syntax. All can generate the same typed app DBMR. Adding a third warehouse using existing operations changes the app. Adding an operation that waits for replenishment changes the DSL and can require an executor and generator release. Changing the public structure of a decision affects its defining type package and potentially every reader. This division explains why DSL design and use could become routine: the DSL author supplies the vocabulary, its interpretation and useful constructors; CoD supplies common data access, checking and publication facilities.

### Making invalid states unrepresentable in the stored program

Suppose the fulfillment team adopts one rule: **an order may ship only from the warehouse that just accepted its reservation**. In the general graph below, `Ask(A, ShipFrom(B), Backorder)` is well-typed. Its author supplied both continuations and named a shipping warehouse, so the graph can express a reservation at A followed by shipment from B. Giving the merchant's form a dropdown that hides B would leave a graph editor free to write this policy.

The DSL designer can instead publish the following sum type as the root vocabulary of the app DBMR. This is algebraic notation. The decline field is a required reference to another policy node in the same region.

```text
Policy = Backorder
       | TryWarehouse(warehouse: WarehouseId, on_decline: Policy)

P₀ = TryWarehouse(A, TryWarehouse(B, Backorder))
P₁ = TryWarehouse(A, Backorder)
```

On acceptance, the executor emits a shipment from the current node's warehouse. On decline, it follows the edge. The only other constructor produces a backorder. Every shipment in a successful execution therefore uses the warehouse from the immediately preceding successful reservation, assuming the handler reports the reservation truthfully and the executor implements the operation as specified. The successful destination occupies **no field** in the app DBMR. Even an editor with permission to rewrite every field of a well-formed policy has nowhere to put a conflicting B. To suspend the backup it changes just A's decline edge. That edit still preserves the rule.

The cost advantage comes from where the type lives. A form, a batch generator and a program that edits a published DBMR all construct the same `Policy`. Each can offer a different authoring interface. Encoding the rule once in the persistent vocabulary saves each tool from implementing a cross-field consistency check. The narrower vocabulary deliberately gives up routing a successful reservation through an arbitrary action. A domain that needs that freedom can choose the general operation graph used in §§3–5 and in the appendix.

The guarantee applies to well-formed values of the declared type. At the file boundary, an unknown constructor tag or a dangling edge still requires rejection before exposing a typed root. A cycle of declines is expressible with same-region references and needs a separate check if the DSL promises termination. Whether A currently has stock is answered when the handler runs. A DSL designer can therefore adopt “make invalid states unrepresentable” one business rule at a time and name exactly which rules the representation enforces. [Yaron Minsky, *Effective ML Revisited*](https://blog.janestreet.com/effective-ml-revisited/)

## 3. What the business programmer writes

The program prefers warehouse A (1) over B (2). Zero denotes backorder. A direct procedure expresses both the ordering and the fact that a failed request, rather than a successful one, reaches the second branch.

```cod
symbol @reserve_one:proc(order_id:u64, sku:u64, warehouse:u64)->bool;

proc route(order_id:u64, sku:u64) -> u64 {
    if (behave @reserve_one(order_id, sku, 1u64)) { return 1u64; }
    if (behave @reserve_one(order_id, sku, 2u64)) { return 2u64; }
    return 0u64;
}
```

The same application can use functional composition for the incoming orders and ordinary procedural calls for effects and output. For example, an application with a deadline filter and a priority-order processor can write the following batch body under CoD's proposed pipeline notation. The example below assumes those processors preserve the order of the three orders used here.

```cod
for { val id <- order_id; val sku <-; } := orders !> due_today |> by_priority {
    record_decision(output, id, route(id, sku));
}
```

The human-facing language has names, function and procedure declarations, branches, composition and lexical scopes. The persistent representation exposes the meaning of these constructs to another program. For the first procedure, its five reachable operations reduce to the graph below. The enclosing batch driver applies its root to each order. Lines in source and byte offsets in an image are both unsuitable as public identities for an edit.

| Operation | Successor on success | Successor on decline |
| --- | --- | --- |
| Reserve one unit at A | Return A | Reserve one unit at B |
| Reserve one unit at B | Return B | Return backorder |

This paper proposes that ordinary CoD text and a graph editor can address the **same checked business computation**. A source projection should recover a readable equivalent procedure, with an exchange representation capable of preserving semantically relevant sharing, binding and dependencies. Acceptance is behavioral and structural. For example, parsing the projection must preserve the condition on the request to B. Reproducing a particular indentation style is a separate tooling property.

A library has another way to produce a publishable program. An overload of processor composition can construct a nominal `RoutingPolicy` functor in an explicit writable region, with a same-region edge to the root of the operation graph. Its type supplies the call operator in the compatible executable. When an overload snapshots the configured warehouse identifiers, that decision belongs to its contract. An open pipeline that still borrows a live callback or cursor remains a bounded lexical value. The language must preserve this distinction even when optimization inlines every call. The functor idiom is one entry into the regional program representation; the broader language goal includes lowering ordinary procedural and functional source into inspectable graphs.

## 4. The program edit and the business result

The editor `E` selects the reservation at A and redirects its *decline* edge to the existing backorder outcome. An operator editing text could instead remove the second `if` from `route`. In each case, the important review question is the same: *does A's success still return A, and is B now unreachable after A declines?* A text edit may omit the unreachable B node entirely. A graph edit may preserve that node and its identity for other uses. The two root computations still agree on which requests they can make.

An ordinary CoD procedure could express the edit as follows. The named operations are illustrative typed library interfaces. The draft destination has its own writable owner. The graph transfer establishes draft-owned nodes and preserves sharing before either destination lookup runs.

```cod
proc suspend_backup(base:const RouteProgram in P, draft:RouteProgram in D)
    where {const owner P, owner D} {
    transfer_policy_graph(base, draft);
    ref primary = unique_reservation(draft, 1u64);
    ref backorder = unique_backorder(draft);
    primary.on_decline = backorder;
    return;
}
```

A publisher checks the candidate graph's root schema, edges, owner relationships, effect contracts, required dependencies and base version before exposing the revision. A selector with two matching primary reservations needs a more precise identity. A draft edge into the old mapping violates the same-owner rule. Neither edit should become the next executable policy. Editing and previewing a policy use editing authority; they perform zero inventory reservations.

Give `D` three orders for SKU 7 and one unit at each warehouse. Run each policy against a **fresh private handler state initialized from the same D**. Otherwise the first run will consume the inventory needed for the comparison.

| Order | Original policy P₀ | Revised policy P₁ |
| --- | --- | --- |
| 41 | A accepts → ship from A | A accepts → ship from A |
| 42 | A declines, B accepts → ship from B | A declines → backorder |
| 43 | A declines, B declines → backorder | A declines → backorder |
| Backorders in the result | **1** | **2** |

A query over the two decision images can identify order 42 as the order whose disposition changes. The effect traces show a second benefit of the edit: it suppresses both requests to B, including the unsuccessful one for order 43. Counting only changed outputs would miss that fact.

## 5. Why algebraic effects belong in this account

The graph records the business question and the continuations for its possible replies. A handler supplies an answer and can update execution-local or external state. In this fragment, a reservation has two ordinary replies and two successor edges. For a fixed order and SKU, its interpretation satisfies

```text
eval_H(ShipFrom(w), s)   = (Ship(w), s)
eval_H(Backorder, s)     = (Backorder, s)
eval_H(Ask(w, y, n), s)  = let (reply, s′) = H.reserve(order, sku, w, s)
                           in eval_H(if reply then y else n, s′)
```

Production can install an inventory-service handler. A simulation can decrement a private table. A trace-checking handler can validate each request and return its recorded reply. Reaching one graph node twice performs the operation twice. A timeout supplies no Boolean reply and therefore takes neither edge. Treating a timeout as a decline could reserve an order twice after a lost acknowledgment. Retry and idempotency policy remain obligations of the operation and its interpreter.

The title's “all” is a hypothesis about the interface to the outside world. In this request–reply fragment, the business computation can express all its interactions as typed operations. Homoiconicity gives another CoD program access to the questions and their branches. Handlers give those questions an execution-time meaning. Changing the decline edge changes the stored policy. Changing the handler lets the same policy run against a snapshot or a live service. The title adopts a familiar research-paper joke. The account of operations and handlers draws on Plotkin and Pretnar. [Prabhu and Teehan's title collection](https://github.com/vinayprabhu/X-is-all-you-need) · [Vaswani et al.](https://arxiv.org/abs/1706.03762) · [Plotkin and Pretnar](https://arxiv.org/abs/1312.1399)

## 6. Native execution and interactive revision

“Compiled or interpreted?” becomes a staging decision for the executor. One policy revision can first run through already compiled generic handlers, then run as a specialized native path. A tool can compile a known policy ahead of deployment, while an interactive session can execute a newly checked branch immediately. These executions must realize the same DSL operations in the same observable order. If this works, domain authors can stop choosing a language paradigm on the basis of that binary distinction. Compilation cost, dispatch cost and the conditions for reusing native code remain measurable engineering choices.

A semantic graph can stay authoritative while execution uses derived native code. The proposed path is to resolve the program's types, operation identities and compatible module world, then execute with native handlers or specialize stable graph fragments using LLVM. A native artifact depends on the graph revision and the assumptions made about layouts, operation contracts, handler bindings and target. Reusing specialized code requires those assumptions to hold. A specialization that always selects A would fail immediately against the trace for order 42.

After publishing `P₁`, the same session can request a checked run of that revision. A run already pinned to `P₀` finishes under its original interpretation. The order schema, inventory operation signature and output writer are candidates for reuse across the edit; the branch and code that inlined it need revalidation. A change to a public type or the fused module world can invalidate a much larger set. A fresh checked world can be activated for new work. This supplies a precise meaning for interactive updates without assuming mutable type relationships within an executing world.

Relocatability supports this workflow by letting multiple workers map a published graph without rewriting its internal edges for each base address. For a finite graph, if resolving each reachable locator gives the same node and successor locators under two mappings, equal request histories and equal handler replies inductively lead to equal decisions. This lemma pays for a **storage** guarantee. The central language test remains the edit cycle: CoD itself must be able to read a program as typed data, publish a valid changed program and execute it.

## 7. What we can show, and what to measure

The executable model in the appendix runs a stored editor on a stored policy, executes the resulting policy, and runs a stored query on both outputs. The change affects order 42 alone. Backorders rise from one to two and reservation requests fall from five to three. A separate client reads the revised decisions and publishes a new decision batch containing orders 42 and 43 for the backorder desk. That client uses only the Decisions schema and ordinary image I/O. Allocation, editing and reporting run as distinct Python entry points in one process over a small file format. These results exercise the artifact cycle. Source correspondence, production DBMR support and CoD execution speed need separate evidence.

**Semantic acceptance for CoD.** Start from the ordinary source in §3. Export its checked program DBMR, inspect the reachable operations using CoD, apply the edit from §4 and execute both revisions. Independently edit the text and compare the resulting reachable operations, decisions and ordered effect traces. Check both Boolean outcomes, a handler failure, empty orders, a shared successor and reopening at another mapping base. Reject a stale-base edit and a result with the wrong program root. A failed edit must leave the previous policy executable. Changing the inventory handler alone must retain the policy identity.

**Two performance claims require two measurements.** For steady throughput compare a direct native branch implementation, the CoD regional graph with native specialization, a graph interpreter, and an equivalent interpreter-based application such as CPython. Include Julia as a compiled interactive baseline. Run identical ordered inputs, policies and in-process effect responses. Verify equal traces before timing. Report orders per second, CPU time per order, warm-up, and separate end-to-end measurements that include reading and publishing DBMRs. External service latency deserves its own experiment. It can dominate every language comparison.

For interactivity measure elapsed time **from submitting an edit until the first checked result visibly reflects it**, in an already running session. Include publication, invalidation, checking, compilation as needed and first execution. Report median and p95 for a constant edit, a branch edit, and a public signature change, plus a cold start. Compare with editing and running the same policy in Python and Julia, reporting data I/O both separately and under equivalent durability requirements. Throughput cannot establish responsive editing; a fast prompt cannot establish fast execution. A result showing high native throughput but multi-second pauses for a two-edge edit would satisfy only half the design ambition.

**Cost of building and changing a DSL.** Give two teams the same Orders and Decisions packages. Ask each to build an allocation vocabulary, an app generator and a reporting client. Then request three changes: add a third warehouse using existing operations, introduce a new inventory operation, and extend the decisions schema. Report which packages, generators, binaries and app images changed, how much adapter code the teams wrote, and whether an independently built reporting client can still read a compatible result. For the reservation rule in §2, attempt the A-reserves/B-ships policy through both a generator and an independent typed graph editor. Record whether the policy can be constructed, whether a later validator rejects it, and how many tools implement a special check for it. Give the comparison embedded DSL the same sum types and count the additional work each system needs to preserve the rule across persistent editing and reopening. This tests the claim of lower engineering cost directly.

## 8. Related work and the scope of the claim

Lisp established that programs can operate on representations of their own programs. Julia demonstrates that familiar function and control syntax can coexist with programmatically manipulated expressions. Lisp implementations also include native compilers, so surface syntax is a poor guide to execution speed. CoD locates its proposed point of uniformity at the **typed, persistent semantic graph**. Imperative and functional text are projections and constructors of that representation. The relevant comparison concerns what survives checking, publication and reopening, and which changes an ordinary program can make after reopening. [McCarthy, *Recursive Functions of Symbolic Expressions*](https://www-formal.stanford.edu/jmc/recursive/recursive.html) · [Julia metaprogramming](https://docs.julialang.org/en/v1/manual/metaprogramming/) · [Steel Bank Common Lisp](https://www.sbcl.org/)

Persistent hyper-programming is a particularly close precedent. The St Andrews work describes type-safe links between programs and objects in a persistent store, and its hyper-code work explicitly investigates a common presentation for source, executable code and data. Julia's world-age work explains how interactive code introduction can coexist with optimization and stable dispatch for existing executions. A defensible CoD contribution would demonstrate **the combination** of a checked DBMR-to-DBMR program transformation, effect rebinding, conventional source projection and measured native execution with low edit latency. The individual ingredients all have prior art. [Persistent hyper-programming](https://research-portal.st-andrews.ac.uk/en/publications/a-persistent-hyper-programming-system/) · [Hyper-code revisited](https://research-portal.st-andrews.ac.uk/en/publications/hyper-code-revisited-unifying-program-source-executable-and-data/) · [Belyakova et al., *World Age in Julia*](https://arxiv.org/abs/2010.07516)

Tagged instructions with executable interpretation have precedents in defunctionalization. Relative pointers have established in-place encodings, and Substrait provides a cross-language vocabulary for serializable computational plans. A stored domain-specific plan is consequently a useful baseline. The harder demonstration for CoD is to write the editor in the same human-facing language, inspect its program image with the same facilities, and use its checked output in the existing session. [Danvy and Nielsen](https://tidsskrift.dk/brics/article/view/21684) · [Cap’n Proto encoding](https://capnproto.org/encoding.html) · [Substrait](https://substrait.io/)

## Conclusion

The merchant changes a business rule by running a program whose input **and output are programs**. The merchant then applies the result to the original orders. The backorder desk uses the same Decisions package to publish its own batch. The policy author needs a new app revision, the desk needs its data types, and the executor can serve both policy revisions. This is what persistent program-as-data buys the people doing the work. If one checked app revision can run immediately after an edit and specialize for repeated execution, “compiled or interpreted?” becomes a question about when to compile that revision. The answer can change during the life of a single application.

## Appendix. Executable artifact-cycle model

Run the following block with Python 3. It creates temporary binary files for each published revision. The small schemas use little-endian unsigned integers. Program edges are indices within their own image. The edit resolves nodes by operation and domain argument. Each executor accepts only its declared program and data schemas. The single-process fixture uses fresh filenames, write-once outputs and a private inventory per allocation run. Production DBMR layout, concurrency, native lowering and source projection are outside this model.

```python
from pathlib import Path
from struct import Struct
from tempfile import TemporaryDirectory

HEADER = Struct("<4sII")          # format, schema, number of records
ROW = Struct("<4I")               # tag and three fields
ORDERS, ALLOC_APP, EDIT_APP, REPORT_APP, DECISIONS, SUMMARY = range(1, 7)
STOCK, ORDER, ASK, CHOICE, REDIRECT_NO, COUNT, RESULT, TOTAL = range(1, 9)


def publish(path, schema, rows):
    payload = b"".join(ROW.pack(*row) for row in rows)
    with path.open("xb") as writer:
        writer.write(HEADER.pack(b"REG1", schema, len(rows)) + payload)


def read(path, schema):
    data = path.read_bytes()
    magic, actual, size = HEADER.unpack_from(data)
    if (magic, actual, len(data)) != (b"REG1", schema, HEADER.size + size * ROW.size):
        raise ValueError("incompatible image")
    return list(ROW.iter_unpack(data[HEADER.size:]))


def policy(path):
    nodes = read(path, ALLOC_APP)
    if not nodes:
        raise ValueError("missing policy root")
    for i, (op, arg, yes, no) in enumerate(nodes):
        if op == ASK and arg > 0 and i < yes < len(nodes) and i < no < len(nodes):
            continue              # this fixture numbers successors after parents
        if op != CHOICE or (yes, no) != (0, 0):
            raise ValueError("invalid instruction or edge")
    return nodes


def exactly_one(indices):
    first, = indices
    return first


def edit(app, base, output):
    (instruction,) = read(app, EDIT_APP)
    op, warehouse, outcome, reserved = instruction
    if op != REDIRECT_NO or reserved:
        raise ValueError("unknown edit")
    nodes = policy(base)
    src = exactly_one(i for i, n in enumerate(nodes) if n[:2] == (ASK, warehouse))
    dst = exactly_one(i for i, n in enumerate(nodes) if n[:2] == (CHOICE, outcome))
    if dst <= src:
        raise ValueError("edge violates forward layout")
    tag, arg, yes, _ = nodes[src]
    nodes[src] = (tag, arg, yes, dst)
    publish(output, ALLOC_APP, nodes)


def simulate_inventory(stock):
    def reserve(order, sku, warehouse):
        key = (warehouse, sku)
        if stock.get(key, 0) == 0:
            return False
        stock[key] -= 1
        return True
    return reserve


def allocate(app, inputs, output, install):
    nodes = policy(app)
    stock, orders = {}, []
    for tag, a, b, c in read(inputs, ORDERS):
        if tag == STOCK:
            stock[a, b] = c
        elif tag == ORDER:
            orders.append((a, b))
        else:
            raise ValueError("unknown input")
    reserve = install(stock)
    results, trace = [], []
    for order, sku in orders:
        pc = 0
        while True:
            tag, arg, yes, no = nodes[pc]
            if tag == CHOICE:
                results.append((RESULT, order, arg, 0))
                break
            reply = reserve(order, sku, arg)
            trace.append((order, arg, reply))
            pc = yes if reply else no
    publish(output, DECISIONS, results)
    return trace


def decisions(path):               # client of the result schema alone
    items = []
    for tag, order, destination, reserved in read(path, DECISIONS):
        if tag != RESULT or reserved:
            raise ValueError("invalid decision record")
        items.append((order, destination))
    return items


def report(app, inputs, output):
    ((op, selected, b, c),) = read(app, REPORT_APP)
    if op != COUNT or b or c:
        raise ValueError("unknown report")
    count = sum(destination == selected for _, destination in decisions(inputs))
    publish(output, SUMMARY, [(TOTAL, count, 0, 0)])


with TemporaryDirectory() as tmp:
    root = Path(tmp)
    d, p0, e, p1, q, r0, r1, s0, s1 = [
        root / name for name in "D P0 E P1 Q R0 R1 S0 S1".split()
    ]
    publish(d, ORDERS, [(STOCK, 1, 7, 1), (STOCK, 2, 7, 1)]
            + [(ORDER, order, 7, 0) for order in (41, 42, 43)])
    publish(p0, ALLOC_APP, [(ASK, 1, 2, 1), (ASK, 2, 3, 4),
                            (CHOICE, 1, 0, 0), (CHOICE, 2, 0, 0),
                            (CHOICE, 0, 0, 0)])
    publish(e, EDIT_APP, [(REDIRECT_NO, 1, 0, 0)])
    publish(q, REPORT_APP, [(COUNT, 0, 0, 0)])
    originals = d.read_bytes(), p0.read_bytes()

    t0 = allocate(p0, d, r0, simulate_inventory)
    edit(e, p0, p1)
    t1 = allocate(p1, d, r1, simulate_inventory)
    report(q, r0, s0)
    report(q, r1, s1)

    assert originals == (d.read_bytes(), p0.read_bytes())
    assert decisions(r0) == [(41, 1), (42, 2), (43, 0)]
    assert decisions(r1) == [(41, 1), (42, 0), (43, 0)]
    assert t0 == [(41, 1, True), (42, 1, False), (42, 2, True),
                  (43, 1, False), (43, 2, False)]
    assert t1 == [(41, 1, True), (42, 1, False), (43, 1, False)]
    assert read(s0, SUMMARY) == [(TOTAL, 1, 0, 0)]
    assert read(s1, SUMMARY) == [(TOTAL, 2, 0, 0)]
    desk = root / "backorder-desk"
    publish(desk, DECISIONS, [(RESULT, order, dst, 0)
                              for order, dst in decisions(r1) if dst == 0])
    assert decisions(desk) == [(42, 0), (43, 0)]
    try:
        edit(e, d, root / "bad")
    except ValueError:
        assert not (root / "bad").exists()
    else:
        raise AssertionError("orders accepted as a program")
    print("backorders 1 -> 2; reservation requests 5 -> 3; changed order 42")
```
