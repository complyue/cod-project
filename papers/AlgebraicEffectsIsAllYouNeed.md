<a id="algebraic-effects-is-all-you-need"></a>

# Do Source and Graph Edits Agree on an Effectful Policy?

*CoD research proposal · revised 24 November 2026*

## Abstract

A fulfillment planner wants to remove a backup warehouse from an inventory policy. One editor deletes a branch in ordinary source. Another edits an edge in the published, typed program graph. Can an independent reader reopen either revision and establish that the two edits changed exactly the same behavior, even when a reservation may alter external state? We define the observations for a small request–reply language, derive a three-order example, and give a counterexample in which a duplicated graph node survives execution tests but changes the meaning of a later edit. The example also admits a conventional implementation with a persisted tagged graph and an explicit reservation service. Comparing it with CoD would identify what checked source projection, regional storage and dynamically installed effects each contribute. The appendix supplies planned CoD and its expected traces. The source-to-graph correspondence, prototype measurements and any relative engineering advantage remain research work.

## 1. A program that takes a program as its input

Warehouse B will be offline tomorrow. The planner asks which orders the company would backorder if its program stopped contacting B. Setting B's stock to zero still contacts the warehouse and may wait for a timeout. The distinction is visible at the service boundary even when the fulfillment decisions agree.

A disk-backed memory region (DBMR) can hold the typed policy, order data or decisions. In the example, the allocation executor accepts a policy and an order batch. A different executor accepts an edit instruction and a policy. A reporting client knows only the decisions schema. The proposed workflow is:

~~~text
R₀ = allocate(P₀, D)       fulfillment decisions under the published policy
P₁ = edit(E, P₀)           successor policy produced by an ordinary program
R₁ = allocate(P₁, D)       decisions on the same orders with fresh private inventory
S₁ = report(Q, R₁)         backorder count produced by an independent client
~~~

The research question concerns the second and third lines together. Can a policy published in one tool be inspected and edited by another, then executed with the control flow and effect requirements a source reviewer would infer? We call this form of program-as-data *semantic homoiconicity*. The evidence required here is a working correspondence over a stated fragment. A common file format for orders and programs establishes only the storage part. [CoD typed program and domain vocabulary design](../design/SymbolicDynamicScope.md#typed-dbmr-programs-and-domain-vocabularies)

## 2. One executor, one DSL

Three artifacts have different consumers. The data packages define orders and decisions for generators, executors and reporting clients. The allocation DSL defines an instruction vocabulary and the executor that interprets it. A merchant's policy is an instance of that vocabulary. A change of fallback warehouse can alter just the policy. A new instruction to wait for replenishment can require a new executor. A change to the meaning of a published decision field can affect independent readers as well. These differences determine what an editor must validate and which binaries a proposed revision can reuse.

### Making invalid states unrepresentable in the stored program

Suppose shipping must use the warehouse that just accepted the reservation. The DSL author can encode that rule in the vocabulary:

~~~text
Policy = Backorder | TryWarehouse(warehouse: WarehouseId, on_decline: Policy)

P₀ = TryWarehouse(A, TryWarehouse(B, Backorder))
P₁ = TryWarehouse(A, Backorder)
~~~

On acceptance, the executor ships from the warehouse in the current node. There is no separate success destination to disagree with it. On decline, the executor follows a required reference in the same region. Every generator and graph editor producing a well-formed value inherits this restriction. Execution also requires an interpreter that follows the rule and a reservation handler that reports its answer faithfully. A cycle of declines remains expressible. If termination is part of this DSL's contract, the validator needs an additional check.

The appendix uses the more general `Ask(w, yes, no) | Finish(destination)` vocabulary. It deliberately admits `Ask(A, Finish(B), Backorder)`. This gives an experiment two distinct questions. Can the editor preserve the behavior of the policy actually written? Can the chosen vocabulary preclude a forbidden policy altogether? An ordinary typed DSL can also enforce the narrower rule. Any claimed advantage for CoD must survive giving the comparison implementation that same type.

## 3. What the business programmer writes

The source policy uses a dynamically installed reservation operation. Warehouse identifiers 1 and 2 stand for A and B, and destination 0 means backorder.

```cod
symbol @reserve_one:proc(order_id:u64, sku:u64, warehouse:u64)->bool;

proc route(order_id:u64, sku:u64) -> u64 {
    if (behave @reserve_one(order_id, sku, 1u64)) { return 1u64; }
    if (behave @reserve_one(order_id, sku, 2u64)) { return 2u64; }
    return 0u64;
}
```

The normal successors of each call become explicit in the general program graph. The accepted edge of A leads to `Finish(A)`, and its declined edge leads to the request at B. B's respective successors are `Finish(B)` and `Finish(0)`. A batch driver can apply the policy to each order in an established sequence. For example, the following body assumes two application processors whose filters and ordering preserve the three orders in the fixture.

```cod
for { val id <- order_id; val sku <-; } := orders !> due_today |> by_priority {
    append_decision(output, id, route(id, sku));
}
```

The source fragment under study has a typed reservation request, branches on its normal reply, terminal decisions and a finite, acyclic reachable policy graph. Data order is fixed. Helpers that perform additional I/O, unbounded recursion and concurrent inventory changes would need further observations. The bounded rules for composing a traversal still apply to the batch driver. A lexical pipeline retaining a live callback or cursor must respect its owner. A library can instead construct a regional policy functor whose durable fields contain only admissible data and same-region edges. [Composite pipeline contract](../design/ProducersAndTransformers.md#composite-implementation-target) · [Callable storage and direct lexical access](../design/CallableStorageAndHereAccess.md#what-an-invocation-needs)

## 4. The program edit and the business result

In source, remove the second `if`. In the graph, redirect the decline edge of the unique A request to the existing backorder terminal. The editor operates on a private successor whose graph transfer preserves sharing. Both selected nodes must belong to that draft.

```cod
proc suspend_backup(region draft:RouteProgram) throws Exception {
    ref backorder = unique_finish(draft, 0u64);
    ref primary = unique_ask(draft, 1u64);
    assert primary isa Ask;
    primary.on_decline = backorder;
    return;
}
```

Give both policies orders 41, 42 and 43 for SKU 7, with one unit at each warehouse. For each run, start the handler from a fresh copy of the same stock counts. In each table cell the requests run from left to right.

| Order | Original policy P₀ | Revised policy P₁ |
| --- | --- | --- |
| 41 | A accepts. Ship from A. | A accepts. Ship from A. |
| 42 | A declines, B accepts. Ship from B. | A declines. Backorder. |
| 43 | A declines, B declines. Backorder. | A declines. Backorder. |
| Total | Five requests, one backorder | Three requests, two backorders |

Order 43 is a useful witness. Its decision is identical in the two results, yet only the old policy contacts B. The report needs the decision images to identify order 42's changed outcome. Reviewing the service calls requires the ordered traces as well. The [appendix](#appendix-the-warehouse-revision-in-cod) specifies both observations, including the exact stock replies. These numbers are derived expectations.

## 5. Why algebraic effects belong in this account

A reservation operation names a question. The policy gives the remaining computation for each normal answer. An installed handler interprets the question using an inventory table, a service, or a deterministic replay script. For fixed order and SKU, the normal case has the following small semantics. Here `s` is handler state.

~~~text
eval_H(Finish(d), s)   = (d, s)
eval_H(Ask(w,y,n), s)  = let (reply, s′) = H.reserve(order, sku, w, s)
                          in eval_H(if reply then y else n, s′)
~~~

Record the operation, its arguments, its reply or failure, and the final decision or exceptional exit. Record handler state where the fixture provides an observation of it. A reservation is executed for each visit, even if two paths name the same operation. A handler failure before a normal reply takes neither successor. A service might have committed before its reply was lost. The real stock change may then be unknown to the executor. A test for this case can use a simulated service that records the commit independently. This extended fixture must declare any checked service failure in the operation and caller contracts. The appendix's Boolean handler covers the normal paths.

Give an ordinary tagged-graph interpreter an explicitly passed `reserve` service. Both the CoD version and the explicit version can record the same requests, swap a simulator for a live service and execute a revised graph. The handler in this example returns a Boolean to ordinary control flow. It exercises a restricted interpretation of operations, with no need for a user handler to capture, discard or resume an arbitrary continuation. Plotkin and Pretnar give a more general account of operations and handlers. CoD's `behave` uses the current dynamic bindings for its scoped call. How much that binding discipline buys over passing a typed service is an empirical question. [Plotkin and Pretnar, *Handling Algebraic Effects*](https://arxiv.org/abs/1312.1399) · [CoD symbolic procedure selection and invocation](../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)

Let two entry branches point to one shared fallback node. A source projection can print two identical fallback clauses. Rebuilding that text into two distinct nodes preserves each present-day decision and request sequence. An editor that later modifies the shared fallback once will change both paths in the first image and one path in the second, or its unique-node selector will become ambiguous. A tool promising round-trip *editing* must preserve declared edit targets and relevant sharing, provide a checked correspondence for them, or reject this fragment. Execution equivalence alone warrants a smaller claim.

## 6. Native execution and interactive revision

The generic evaluator and a specialized native version can use the same reference trace. The edit protocol pins the input revision, builds and validates a private successor, and publishes it under a fresh name. A reader that already holds the old mapping can finish with it. Another reader can open the completed successor. The CoD DBMR contract governs shared readonly access and complete fresh-name publication, including a competing publisher. Atomic fresh-name installation alone does not compare an edit's expected base revision with a separately managed “latest” pointer. A tool claiming stale-edit protection must supply that precondition in its edit request or repository protocol. The appendix passes an explicit pinned base. [DBMR modes and publication](../design/ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr)

Replacing a graph edge and changing an installed handler are independent tests. First change the handler while retaining the graph revision. Then change only the decline edge while keeping handler semantics fixed. Any native artifact that depends on a specific edge, selected method, operation signature, layout or handler assumption needs corresponding validity evidence. The experiment may use a generic evaluator throughout. Specialization adds a second obligation to preserve the trace under revision or rebinding. Julia's world-age work supplies a comparison for stable running calls in the presence of new code. CoD's checked module worlds and published region revisions have their own rules. [Belyakova et al., *World Age in Julia*](https://arxiv.org/abs/2010.07516) · [CoD module worlds](../design/ModuleWorlds.md)

## 7. What we can show, and what to measure

Start with a **semantic comparison** that can fail on a small input. Implement a direct source interpreter and the generic graph evaluator for the fragment in §3. Export source to a typed graph, reopen it at a different address, apply the graph edit, project it back to source and check that source. Separately apply the corresponding textual edit and export that result. For each small graph, generate every Boolean reply path up to its longest reachable sequence of requests. Include failures at each reached operation, empty order batches, repeated warehouse IDs and shared tails. Drive all four paths from the same initial inventory or independently reset replay state. Compare ordered requests, replies, decision or failure, and observable state. A single mismatch defeats the claimed correspondence for that input. For the finite acyclic language, induction over the reachable successors can supplement these tests once the source, exporter, projection and edit relations are specified.

| Check | Discriminating observation |
| --- | --- |
| Wrong edge, changed request order or stale specialization | All final destinations can match while a warehouse observes an extra reservation. Ordered traces must distinguish it. |
| Shared target versus identical text | Apply a *second* edit using the published selector. Compare the targets found and then execute both previously separate entrance paths. |
| Installation, retirement and failure | Two handlers implement the same operation under distinct scopes. Check current binding and restoration on ordinary and exceptional exit. Compare an explicitly passed service with the same lifetime and behavior. |
| Invalid draft, wrong root and occupied output name | Reject before execution or publication as appropriate. The old published policy remains readable. Test stale-base protection separately by supplying an independently known expected base. |
| Independent reader and mapping | A process with only the compatible Decisions schema opens the result at a different address. It must recover both backorders and must need no authority to reserve inventory. |

For a single supported edit, write `L` for source-to-graph export, `e_s` for deleting the fallback
in source and `e_g` for redirecting its graph edge. The executable claim is equality of the observed
runs of `L(e_s(S))` and `e_g(L(S))`, for each admitted program `S` and independently reset
handler/input pair. Couple any external responses by the same per-request script and compare the
requests as they occur. For editable round trips, also compare the resolution of the *next* public
edit selector and the runs after applying it. This allows, for example, physically different
addresses and unreachable nodes while detecting loss of a shared editable target.

**Compare one mechanism at a time.** Implement the tagged graph, editable sharing, typed schemas, compatibility checks, immutable revision publication and trace logger in the comparison system too. Run CoD first with explicitly supplied service operations and then with its dynamically installed operation. Keep graph bytes, revision checks, kernel work, persistence guarantees and operation outcomes matched wherever the two modes admit this. Separately compare a handwritten source-to-plan adapter against the proposed checked CoD projection. Disclose formatter, validator and adapter work for both. If an explicit service and a conventional typed serializer provide the same checks and ease of independent edits at similar cost, the warehouse experiment supports the common approach and leaves a CoD-specific advantage unestablished.

**Measure a planner's wait and an executor's work separately.** In paired, fresh fixtures, time from submitting a branch edit to the first visibly correct checked result. Include checking, publication, invalidation and any compilation. Record individual stages as well as the median, p95 and uncertainty across independent runs. Test a value change, our branch edit, a public signature change and a cold start, reporting warm and cold cache states separately. For bulk execution, compare the direct native branch, tagged-graph interpreter and optional specialization on identical ordered inputs with the same in-process service. Vary batch size, graph size and proportion of edits between runs. Report requests per order, CPU time per order, data bytes read or copied, and end-to-end elapsed time. External service latency belongs in a separate run. Report which paths the pinned compiler and exporter support. For a disagreement, use the stipulated request trace, edit targets and publication contract to locate the first divergence. A reference evaluator can exercise the fragment before the exporter covers it. Predeclare practical latency and throughput targets with the intended users. A throughput gain does not compensate for an edit that fails its agreed latency budget.

Estimate authoring cost on three changes. First add a warehouse using existing instructions, then introduce a new operation, then extend the Decisions schema while an independently built reporter remains installed. Record which artifacts each change touches, who writes compatibility checks, whether independently produced valid graphs reopen, and which invalid edits reach an executor. A table of touched components can be reproduced without treating small differences in lines of code as general evidence of programmer productivity.

## 8. Related work and the scope of the claim

Hyper-code already proposed a common presentation for persistent source, executable code and data, including operations to edit and evaluate programs. The closest comparison preserves links into a persistent store and lets a user revisit the program through another tool. CoD's candidate contribution is a checked correspondence between a source edit and a graph edit over the admitted effectful fragment, with independently readable, typed results. A successful experiment would establish which observations and later edits survive that correspondence. [Zirintsis, Kirby and Morrison, *Hyper-code revisited*](https://research-portal.st-andrews.ac.uk/en/publications/hyper-code-revisited-unifying-program-source-executable-and-data/)

Defunctionalization helps explain why tagged first-order constructors and an evaluator can represent control. Substrait provides a working comparison for typed, serializable domain plans with multiple consumers. Neither precedent requires a novelty claim for the existence of a program graph. The distinguishing burden falls on checked editing correspondence, handler composition, and the measured cost of preserving both across revisions. [Danvy and Nielsen, *Defunctionalization at Work*](https://tidsskrift.dk/brics/article/view/21684) · [Substrait specification](https://substrait.io/)

<a id="conclusion"></a>

Trace preservation without preserved edit targets justifies a read-and-execute projection. Preserved targets and traces with equal conventional engineering cost justify a portable editing contract. A claim about convenient scoped handlers or faster checked revision requires its own comparison. Each outcome tells the planner what can safely be reviewed and changed before the next warehouse call.

## Appendix. The warehouse revision in CoD

These listings use the planned CoD language. The allocation, editing and reporting programs each have a typed DBMR as input and produce a fresh typed DBMR. The session at the end runs all three. The named file readers, graph-transfer and validation routines, row iterators and trace helpers are example package interfaces. Their contracts are given where used. The listings specify expected behavior for a future CoD run.

### The policy and its executor

The policy in §§3–5 has two instructions. `Ask` performs one reservation and branches on its reply. `Finish` returns a destination. Here both continuations are reference fields in the policy's own region. Section 2 gives the smaller vocabulary a DSL author can use when a successful reservation must always determine the shipping warehouse.

```cod
data RouteNode =
    Ask { warehouse:u64; ref on_accept:RouteNode; ref on_decline:RouteNode; }
  | Finish { destination:u64; };

region RouteProgram { ref entry:RouteNode; }

proc original_policy() -> own region :RouteProgram {
    return make region p:RouteProgram(4096u64 -> capacity) {
        ref ship_a = make Finish in p { destination <- 1u64; };
        ref ship_b = make Finish in p { destination <- 2u64; };
        ref backorder = make Finish in p { destination <- 0u64; };
        ref backup = make Ask in p {
            warehouse <- 2u64; on_accept = ship_b; on_decline = backorder;
        };
        entry = make Ask in p {
            warehouse <- 1u64; on_accept = ship_a; on_decline = backup;
        };
    };
}

symbol @reserve_one:proc(order_id:u64, sku:u64, warehouse:u64)->bool;

proc evaluate(n:const RouteNode in P, id:u64, sku:u64) -> u64
    where {const owner P} {
    return case n of {
        (Finish { val dest <- destination; }) -> { dest };
        (Ask { val w <- warehouse;
               ref yes:const RouteNode = on_accept;
               ref no:const RouteNode = on_decline; }) -> {
            if (behave @reserve_one(id, sku, w)) { return evaluate(yes, id, sku); }
            evaluate(no, id, sku)
        };
    };
}
```

`original_policy()` constructs the graph explicitly. The source-correspondence check in §7 starts separately from the ordinary `route` procedure in §3 and compares its exported graph with this one. In this example, `evaluate` invokes the reservation effect. The appendix uses its declared normal Boolean reply. The failure experiment in §5 requires the separately declared throwing variant.

### The stored editor

The editor DSL has one operation. Its two operands select a reservation by warehouse and a terminal by destination. The report DSL below likewise has one operation. In each case the root schema identifies the operation, and the selected executor supplies its meaning.

```cod
region RedirectDecline { warehouse:u64; destination:u64; }
region CountDestination { destination:u64; }
region Summary { count:u64; }

proc edit(region instruction:const RedirectDecline, region base:const RouteProgram)
    -> own region :RouteProgram throws Exception {
    own region draft <- fork_route(base);
    ref target = unique_finish(draft, instruction.destination);
    ref request = unique_ask(draft, instruction.warehouse);
    assert request isa Ask;
    request.on_decline = target;
    validate_revision(base, draft);
    return draft;
}
```

`fork_route` uses explicit graph transfer from the pinned base into a fresh writable owner and preserves sharing. Both lookups search the draft and require exactly one matching node. Both returned references belong to the draft. The single `=` changes the decline edge there. Validation checks the draft against the explicitly pinned base, root schema, edges, effect contract and dependencies before the result is eligible for publication. The instruction carries only two selectors. Rejecting a request prepared against a different base additionally requires the caller or revision store to supply the expected revision identity, as discussed in §6. A failed lookup or validation exits without returning the draft to the publisher. The original policy remains available.

### Allocation and independent reporting

The Orders package defines `Order` and `Orders`. The Decisions package defines `Decision` and `Decisions`. Their `orders_in` and `decisions_in` generators follow the regional list in order and yield the two indicated scalar fields per row. A typed reader checks the list and its root before exposing it. In this fixture every order is for SKU 7. The input includes the starting stock at warehouses A and B.

```cod
data Order { id:u64; sku:u64; ptr next:Order; }
region Orders { stock_a:u64; stock_b:u64; ptr first:Order; }

data Decision { id:u64; destination:u64; ptr next:Decision; }
region Decisions { ptr first:Decision; ptr last:Decision; }

proc empty_decisions() -> own region :Decisions {
    return make region Decisions(4096u64 -> capacity) { first = null; last = null; };
}

proc append_decision(region batch:Decisions, order_id:u64, dest:u64) {
    ref row = make Decision in batch { id <- order_id; destination <- dest; next = null; };
    ptr tail = batch.last;
    if (tail is null) { batch.first = row; } else { tail.next = row; }
    batch.last = row;
    return;
}

data Reply { order_id:u64; sku:u64; warehouse:u64; filled:bool; ptr next:Reply; }
region RequestLog { ptr first:Reply; ptr last:Reply; }

proc allocate(region policy:const RouteProgram, region input:const Orders,
              region trace:RequestLog) -> own region :Decisions {
    val left_a <- input.stock_a;
    val left_b <- input.stock_b;
    effect @reserve_one(order_id:u64, sku:u64, warehouse:u64) -> bool {
        assert sku == 7u64;
        val filled <- false;
        if (warehouse == 1u64) {
            if (left_a > 0u64) { left_a <- left_a - 1u64; filled <- true; }
        } else {
            assert warehouse == 2u64;
            if (left_b > 0u64) { left_b <- left_b - 1u64; filled <- true; }
        }
        record_reply(trace, order_id, sku, warehouse, filled);
        return filled;
    };

    own region batch <- empty_decisions();
    for { val id <-; val sku <-; } := orders_in(input) {
        append_decision(batch, id, evaluate(policy.entry, id, sku));
    }
    return batch;
}

proc report(region query:const CountDestination, region rows:const Decisions)
    -> own region :Summary {
    val total <- 0u64;
    for { val dest <- destination; } := decisions_in(rows) {
        if (dest == query.destination) { total <- total + 1u64; }
    }
    return make region Summary(4096u64 -> capacity) { count <- total; };
}

proc desk_batch(region rows:const Decisions) -> own region :Decisions {
    own region desk <- empty_decisions();
    for { val id <-; val dest <- destination; } := decisions_in(rows) {
        if (dest == 0u64) { append_decision(desk, id, dest); }
    }
    return desk;
}
```

Each `allocate` invocation starts with fresh lexical stock counters. Its definition-installed handler can update them during that invocation. `RequestLog` is a fixture-owned region whose `record_reply` appends the four scalar fields in call order. The desk client imports the Decisions package. It reads the published result and constructs its own rows under a new owner, using only order IDs and destinations.

### One session, two policy revisions

The fixture directory starts empty. Each `open_...` routine validates a published image against the named schema and returns an owned readonly mapping. `publish` consumes the private region and installs the complete image under a fresh filename. [DBMR construction and publication](../design/ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr)

```cod
proc sample_orders() -> own region :Orders {
    return make region d:Orders(4096u64 -> capacity) {
        stock_a <- 1u64;
        stock_b <- 1u64;
        ref third = make Order in d { id <- 43u64; sku <- 7u64; next = null; };
        ref second = make Order in d { id <- 42u64; sku <- 7u64; next = third; };
        first = make Order in d { id <- 41u64; sku <- 7u64; next = second; };
    };
}

proc compare_revisions() throws Exception {
    publish(sample_orders(), "orders.dbmr");
    publish(original_policy(), "policy-0.dbmr");
    publish(make region RedirectDecline(4096u64 -> capacity) {
        warehouse <- 1u64; destination <- 0u64;
    }, "edit.dbmr");
    publish(make region CountDestination(4096u64 -> capacity) {
        destination <- 0u64;
    }, "query.dbmr");

    own region inputs <- open_orders("orders.dbmr");
    own region before <- open_route("policy-0.dbmr");
    own region change <- open_editor("edit.dbmr");
    own region query <- open_query("query.dbmr");
    own region trace0 <- empty_log();
    own region trace1 <- empty_log();

    publish(allocate(before, inputs, trace0), "decisions-0.dbmr");
    publish(edit(change, before), "policy-1.dbmr");
    own region after <- open_route("policy-1.dbmr");
    publish(allocate(after, inputs, trace1), "decisions-1.dbmr");

    own region old_rows <- open_decisions("decisions-0.dbmr");
    own region new_rows <- open_decisions("decisions-1.dbmr");
    publish(report(query, old_rows), "summary-0.dbmr");
    publish(report(query, new_rows), "summary-1.dbmr");
    publish(desk_batch(new_rows), "desk.dbmr");

    own region old_summary <- open_summary("summary-0.dbmr");
    own region new_summary <- open_summary("summary-1.dbmr");
    own region desk <- open_decisions("desk.dbmr");
    assert old_summary.count == 1u64;
    assert new_summary.count == 2u64;
    assert destination_for(old_rows, 41u64) == 1u64;
    assert destination_for(new_rows, 41u64) == 1u64;
    assert destination_for(old_rows, 42u64) == 2u64;
    assert destination_for(new_rows, 42u64) == 0u64;
    assert destination_for(old_rows, 43u64) == 0u64;
    assert destination_for(new_rows, 43u64) == 0u64;
    assert row_count(desk) == 2u64;
    assert destination_for(desk, 42u64) == 0u64;
    assert destination_for(desk, 43u64) == 0u64;
    assert reply_count(trace0) == 5u64;
    assert reply_count(trace1) == 3u64;
    # reply_is(log, index, order_id, sku, warehouse, filled)
    assert reply_is(trace0, 0u64, 41u64, 7u64, 1u64, true);
    assert reply_is(trace0, 1u64, 42u64, 7u64, 1u64, false);
    assert reply_is(trace0, 2u64, 42u64, 7u64, 2u64, true);
    assert reply_is(trace0, 3u64, 43u64, 7u64, 1u64, false);
    assert reply_is(trace0, 4u64, 43u64, 7u64, 2u64, false);
    assert reply_is(trace1, 0u64, 41u64, 7u64, 1u64, true);
    assert reply_is(trace1, 1u64, 42u64, 7u64, 1u64, false);
    assert reply_is(trace1, 2u64, 43u64, 7u64, 1u64, false);
    return;
}
```

A wrong-root check uses the same inputs. Calling `open_route("orders.dbmr")` must fail before the editor can publish a successor. The two valid policy images can still be opened and run.
