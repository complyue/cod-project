# Building Reusable Domain Operations in CoD

*Developer guide · first edition*

A booking application already finds the first affordable seat. Now operations wants an audit of **every** affordable seat, including two seats at the same price. Tomorrow the auditor will read recorded inventory. Keep one definition of “affordable,” give each caller its own terminal, and give the live and recorded sources the same offer-and-reply protocol. This guide walks through those changes and the additional ownership decision when another caller needs the rule after its builder returns.

## Write the caller's contract first

For the example, a route source enumerates route IDs with void feedback. The inventory service enumerates that route's fares with Boolean feedback. True requests another seat on the current route. False ends that route's seat cursor. The next route can still open. The application treats zero as “no match” and uses positive fares. A recorder writes one audit row per offer, including duplicates.

Choose the meaning of a changing policy before choosing its representation. Suppose a booking form selects a cap of 100, its selected policy is later edited to 70, and the caller's current policy allows 90. On fares 80 and 95, a quoted cap should offer both, a live link to the edited policy should offer neither, and a search under the current caller's policy should offer just 80. Put the desired answer in the API example or acceptance check. It decides what the builder must keep.

| When a developer asks for… | Put this in the prepared operation | Supply this when driving it |
| --- | --- | --- |
| A short-lived connection used twice inside a live call | A named `here` connection with valid receiver and argument dependencies | A fresh traversal and the terminal for each use |
| A quote that keeps its original cap | The scalar cap, in the live connection or in a regional field if the quote must outlive its builder | The new inputs and terminal |
| A reusable rule that observes edits to a specific tariff | A typed link to the selected tariff, subject to the plan's owner and relocation rules | Access to the same live tariff at the promised read point |
| A rule that follows the current customer or run | The operation that uses the policy | A typed policy argument or a declared dynamic service in the current procedural scope |
| A catalog that another caller opens later | A regional root and data nodes sufficient to reconstruct the operation | A compatible reader, fresh run state and callbacks from this call |

## Add an audit without cloning the search

The example below uses the producer arguments and pipeline vocabulary from the fare-search design. `fares_for()` expands routes using the inventory service. `affordable(100u64)` prepares the cap once, replies true for each rejected seat, and forwards the current terminal's Boolean reply for each accepted seat. `record(fare)` writes one audit entry. These snippets express the design target. The direct nested-loop counterpart and the implementation status are linked below. [Fare-stage definitions](../design/ProducersAndTransformers.md#one-fare-search-three-spellings)

```cod
proc quoted_fares(here routes() yields u64) yields u64 -> bool {
    here stages = fares_for() |> affordable(100u64)
    for val fare <- routes !> stages {
        continue yield(fare)
    }
}

proc audit_affordable(here routes() yields u64) -> void {
    for val fare <- quoted_fares(routes) {
        record(fare)
        continue true
    }
}

proc first_affordable(here routes() yields u64) -> u64 {
    for val fare <- quoted_fares(routes) {
        return fare
    }
    return 0u64
}
```

`quoted_fares` names the composition and prepares its cap. To add auditing to the first-match application, add `audit_affordable` and its call site. It writes one row and asks for the next seat. Both callers use the same predicate, cap and route expansion. A single-brace `for` body gives `first_affordable` its outward return. An ordinary Boolean false travels only as far as the seat producer's contract specifies.

To replay the **whole** search, replace both inputs that can reach live inventory. Supply recorded route IDs through `routes` and provide recorded per-route fares through the inventory service used by `fares_for`. A route list alone would still query the live seat source. Match the order, duplicate seats, feedback and declared failures at those ports. The search and audit terminal then work against the same contract. The same pattern lets a unit-sized acceptance example run from a short sequence with a recording terminal.

## Keep a rule after the builder returns

The next requirement is to build a fare rule in a catalog and use it from separate requests. Give the catalog an actual owner G. For the snapshot contract the entire policy for this example is one integer. A new data type can hold it, and a method can run the search with the current caller's route source. The following is a domain specialization of the project's regional invoice-rule example. Assume `fares_on` is the named procedural inventory operation described above. [Regional invoice rule and its constructor](../design/ProducersAndTransformers.md#a-regional-functor-returned-by-an-overload)

```cod
data FareRule { maximum:u64 }

func FareRule(limit:u64) {
    maximum <- limit
}

proc (const FareRule in G).(())(here routes() yields u64)
    yields u64 -> bool where {const owner G} {
    for val route <- routes() {
        for val fare <- fares_on(route) {
            if (fare > self.maximum) { continue true }
            continue yield(fare)
        }
    }
}
```

For a product that needs both short-lived composition and stored rules, factor the fare predicate into one shared definition or have the scoped API use the regional method. The snippet shows the complete behavior that such a method must preserve. The owner clause follows the ADT receiver. `proc` fits the call to the procedural inventory service. A method that reads a scalar field and calls only permitted supplied bodies may qualify as `func` under its own contract. An ADT with a procedural call operator suffices for this example. It requires no language-wide first-class bound procedure representation.

A builder with write access can construct `make FareRule(100u64) in G` and install the result in the catalog's typed regional root. Return a `ref FareRule in G` when the caller already keeps G alive. If the builder creates the owner, return its ownership with a reachable entry under the result-owner rules. Once the builder exits, a reader reaches the rule through the catalog. The new caller supplies route enumeration and a receiving body at invocation. Its cursor, early-return destination and per-search count all belong to this drive. The audit destination can outlive the traversal under its own owner. Keep the plan in an ordinary in-memory region when requests only share it in process. Choose a published DBMR when independent reopening or toolchain delivery is a product requirement. [Result-bound owners](../design/ConstructionAndAcquisition.md#result-bound-owners-and-ordinary-liveness) · [Owned parser plans and publication](../design/ParserCombinatorLibSupport.md#lexical-composition-or-an-independently-owned-plan)

When a new field is proposed, give it a sentence a reviewer can test. “An edit by this tariff owner takes effect on the next invocation” requires a live referent and an explicit read point. “An approved quotation keeps its ceiling” requires a copy or an immutable revision. “Limit this search to two offers” puts the configured limit in the rule and the remaining count in the invocation. “Limit this customer to two bookings in total” needs separately owned shared state, with an explicit update and concurrency policy.

## Review the change with a small inventory

Use A = [120, 80, 80] and B = [90] with cap 100. Have each source log open, offer, normal reply and close. Give duplicate seats separate IDs in the log.

| Exercise | What to look for |
| --- | --- |
| Audit all seats | Three rows in order, 80, 80, 90. The rejected 120 gets a true reply. |
| Reply false on the first accepted seat, then true | The audit sees 80, 90. A closes after its first 80 and B opens. |
| Return the first match from the caller | The caller gets 80. The current seat gets no Boolean reply. A closes, then the route cursor closes. B never opens. |
| Fail from the audit terminal | Under the fixture's successful close operations, the same active cursors close once and the declared failure reaches the caller's eligible handler. |
| Drive a saved two-offer rule during another drive | With three eligible inputs per independent source and always-true terminals, each drive gets its own first two offers. |
| Switch to recorded inventory | With the same input and policy schedule, compare offers, replies, closes and terminal result. Observe both inventory ports to catch accidental live reads. |

For the productivity claim, keep the feature diff. Adding a terminal should require the terminal and its wiring. Switching inventory should require the source or service adapter and its installation. Check whether either change also edited the predicate, duplicated an early-stop loop, or touched a regional layout. Record the affected declarations, new adapters and effort if comparing implementations. This gives the team evidence about its own application and makes unexpected coupling easy to locate.

## Apply the same decisions in other domains

**Parser-library authors.** In the production plan `pcl.cod` is a parsing HLVM, and the CoD parser runs on it as one application. The toolchain ships that application's grammar program as `cod.grammar` in DBMR format. Keep the grammar's typed rules in G. Give each parse fresh session storage and callbacks for consumed success, consumed failure, empty success and empty failure. Allocate retained syntax in a result owner N distinct from the actual grammar owner. Different parameter names alone do not establish that separation. To check reuse, parse once, release that result region, and parse different input into another result region using the same grammar. For `left <|> right` also exercise a left empty success and a left empty failure. Only the latter should start the right rule. The general callable interface, answer types and rollback details are active pre-research. [PCL working design](../design/ParserCombinatorLibSupport.md#the-rule-interface) · [HLVM and application](../papers/AlgebraicEffectsIsAllYouNeed.md#the-cod-parser-as-an-application-of-the-parsing-hlvm)

**Actor and event-library authors.** Use bounded composition for synchronous decode and normalize stages. At admission, route the event to the selected site's durable queue or store the admitted payload and destination identity through the Actor library's permitted transfer. Supply fresh execution context when the receiving Actor runs. Keep a pending window with the site that owns the window. A gateway acknowledgment observes admission. An alarm is a later receiver-side observation. Exercise live and trial routing with identical readings so a mistaken site association is visible. [Event-pipeline case study](../papers/OpenEventPipelines.md#one-ingress-plan-for-live-and-replay)

## Design basis and implementation coverage

Edition 1 applies the [product documentation scope](../design/DesignAuthorityRegistry.md#d-doc-idiomatics) to the current paper and worked examples. Its review criteria are the observable traces above and the changes needed to add a terminal and replace an input. The source conventions come from the project's [idiomatics guidance](IdiomaticCoD.md#choose-a-callable-by-its-dependencies), [pipeline design](../design/ProducersAndTransformers.md#lexical-variables-for-open-pipelines) and [callable storage rules](../design/CallableStorageAndHereAccess.md#callable-value-kinds). The [working paper](../papers/WhatShouldAComposedOperationRetain.md) gives the preservation argument and experiments that could disprove it. The [recorded data-call-operator probes](../tests/cod/language/README.md#data-call-operator-coverage) found that the tested seed and booter images rejected the OO receiver heads. The calls above illustrate the intended developer interface. End-to-end compilation of those snippets, parser-library qualification and measured productivity improvements remain open.
