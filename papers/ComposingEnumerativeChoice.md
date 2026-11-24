# Composing Enumerative Choice with Producers and Transformers

## Abstract

A booking service searches the seats on each eligible route. It should return the first affordable
fare, close the two cursors it has already opened, and leave the next route untouched. Can a library
factor that search into reusable expansion and filtering stages while preserving what the service
does? We give a trace equivalence for finite, synchronous traversals with typed feedback and
successful cursor cleanup. Its decisive case is a discarded seat. Although the price filter makes
no downstream call, it must still decide what answer to send to the seat source. A normal negative
answer ends the current route. An outward return from the booking service supplies no answer and
ends the search. A case argument establishes the equivalence for the stated fragment. Two
independently written executable models agree on all 399 cases of a bounded comparison. One-seat
inputs distinguish two faulty translations even when their final fares agree with the reference.

## A closed traversal

A producer offers an item by entering its consumer synchronously. If the consumer completes that
handoff normally, it supplies the feedback type declared by the producer. A transformer accepts an
upstream traversal and can offer items to a downstream consumer of its own. A traversal is *closed*
here when every handoff returns or transfers control while the invocation that supplied the
consumer is still active. This lets a worker search a live inventory cursor without first reading
all the seats. Processing an event after the submitting call returns is a different lifetime problem
studied in the [companion paper](OpenEventPipelines.md).

Consider these contracts. `itineraries(day)` opens an ordered route cursor and offers route
identifiers with void feedback. `fares_on(route)` opens that route's seat cursor. It offers a
positive integer fare and accepts a Boolean after each normal handoff. In this application, true
requests the next seat and false finishes the current seat cursor. Each successfully opened cursor
logs and executes one close when its invocation ends. For this experiment, acquisition and cleanup
succeed. The first-fare operation returns zero if it reaches the end without taking a fare.

The explicit CoD loops give the operational reference. The single-brace `return` targets
`first_affordable`. The `continue true` supplies feedback to the active seat cursor. The terminal
consumer can qualify an offer and return without ever asking inventory for another seat.
[Generator feedback](../design/RangedForAndGenerators.md#explicit-continue-feedback) ·
[Lexical completion](../design/RangedForAndGenerators.md#lexical-scope-and-completion)

```cod
proc first_affordable(day:u64) -> u64 {
    for val route <- itineraries(day) {
        for val fare <- fares_on(route) {
            if (fare <= 100u64) { return fare; }
            continue true;
        }
    }
    return 0u64;
}
```

The factorization gives each stage one decision. Expansion opens one seat cursor per route and
forwards each seat's normal reply. The filter takes responsibility for the reply to a rejected
seat. Each `yield` below invokes that stage's own downstream consumer.

```cod
proc fares_for(here routes() yields u64) yields u64 -> bool {
    for val route <- routes() {
        for val fare <- fares_on(route) {
            continue yield(fare);
        }
    }
}

proc affordable(here fares() yields u64 -> bool, limit:u64) yields u64 -> bool {
    for val fare <- fares() {
        if (fare > limit) { continue true; }
        continue yield(fare);
    }
}

proc first_factored(day:u64) -> u64 {
    for val fare <- itineraries(day) !> fares_for() |> affordable(100u64) {
        return fare;
    }
    return 0u64;
}
```

The second listing uses the fare study's **candidate** reusable producer declarations. The selected
CoD composite contract covers a particular set of operand, binding, feedback and lifetime cases.
General signatures and contextual producer binding await a language for named signatures. The result
below concerns the behavior that such a factoring must preserve. It can be checked before choosing
the general syntax. [Fare study](../design/ProducersAndTransformers.md#one-fare-search-three-spellings)
· [Design scope](../design/ProducersAndTransformers.md#design-authority)

## What an equivalence must preserve

**Proposition.** Fix a finite ordered sequence of routes, a finite ordered sequence of seats for
each route, a pure total price predicate and the same initial consumer state. Run the explicit loops and
the factored traversal with the same terminal policy. On each accepted seat the policy may give
ordinary Boolean feedback, return from the enclosing search, or raise a failure that escapes the
body. With successful cleanup and stable, already prepared operands, the two runs have the same
trace at the source and consumer boundaries, including the final control destination.

The observation records cursor opens, route offers, seat offers with route and position, consumer
visits, normal feedback received by each seat offer, normal resumption of a route offer, cursor
closes and the outward outcome. A terminal policy may choose its next action using its prior
visits. Its next action must be determined by that state and offer, or both runs must receive
the same external answers in the same order. Record other visible terminal actions in that case. Internal calls between stages are hidden. Requiring their names or their
stack depths to match would distinguish an extracted helper even in a pure mapping.

**Argument.** At the start of a route, pair the two route cursors at the same position. At the start
of each seat offer, the corresponding runs have opened the same cursors, advanced to the same
seat, and produced the same observation prefix. The final consumer has the same state in both.
There are four continuations.

1. A seat fails the predicate. Both forms send true to the seat source without invoking the
   terminal. They advance together.
2. A seat passes and the terminal returns ordinary feedback. Both have visited it once. The
   factored stages forward that feedback unchanged. True advances the same cursor. False closes
   that cursor, completes the route offer normally and lets the route producer advance.
3. A seat passes and the terminal returns from the enclosing search. Both leave the active seat
   offer without a normal Boolean. They close the seat cursor and then the route cursor, and
   deliver the same value to the search's caller.
4. A terminal failure escapes. The same active cursors close in the same order. An ordinary catch
   surrounding the handoff *inside a generator* cannot intercept this body-originated failure.
   Handling resumes at the caller's traversal boundary. A handler explicitly installed in an
   intermediate consuming body would change the program and must be present on both paths.
   [Checked body failures and nested loops](../design/RangedForAndGenerators.md#execution-and-nested-loop-boundaries)

An empty seat cursor opens and closes without a handoff. Exhausting the route source closes it
before the caller's normal continuation. Those cases supply the base cases. On a path that
continues, either a seat or a route advances. Induction over the remaining finite itinerary
establishes equality of the stated observations. An implementation can use a different control
representation while preserving these steps. The argument relies on real lexical destinations,
exact forwarding of ordinary replies and one cleanup for every live cursor.

### Three continuations at the first acceptable seat

Route A has fares 120, 80, 80. Route B has one fare of 90. The common prefix opens the route cursor,
offers A, opens A, offers 120 and receives true, then offers the first 80 to the terminal. The
following suffixes start *after* that terminal visit. Seat positions make the two 80s distinct.

| Terminal action | Remaining source and consumer observations |
| --- | --- |
| Reply false once, then true | A receives false and closes. The route source resumes, offers B and opens its seat cursor. B offers 90, the terminal visits it and B receives true. B closes, the route source resumes and closes, and the caller finishes normally. |
| Return the first fare | A closes, the route cursor closes, and the caller receives 80. There is no Boolean for that 80 and no offer of B. |
| Raise an uncaught booking failure | A closes, the route cursor closes, and the same failure reaches the caller. There is no Boolean for that 80. |

A fourth control run always replies true. Its terminal sees 80, 80, 90 in that order. The first row
instead sees 80, 90 because false stops only the current seat cursor. An implementation that caches
the first result, drains B and eventually returns the cached 80 passes a result-only test. Its extra
cursor open is observable. If opening B makes a billed request, that execution also incurs a charge.

## Prior work and evaluation

The Haskell 2010 Report translates a comprehension generator through `concatMap` and a guard through
a conditional empty branch. For finite, pure inputs with full consumption, these rules give the
ordered answers of the fare example, including the duplicate 80. They provide an independent
answer-list oracle. That projection omits which source received feedback and when a live inventory
cursor was closed. [Haskell 2010 Report, §3.11](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html#x8-420003.11)

Go's range-over-function rule supplies a synthesized `yield`. When a loop body terminates, the
function receives false and may issue no further yields. The booking example uses Boolean feedback
at a *different* boundary. A terminal can feed false to A as an ordinary application reply and
still go on to inspect B. A Go encoding can use separate iterator boundaries or an explicit
protocol to preserve that distinction. The false-versus-return rows above test the encoding more
precisely than a first-result example alone. [Go specification, range clauses](https://go.dev/ref/spec#For_range)

Haskell's `Pipes.Core.respond` already offers a value downstream and resumes with a reply whose type
is a separate parameter. Its response-substitution operator can replace individual offers with a
handler. Typed bidirectional handoff therefore has a direct precedent. A comparison with CoD must
also account for the return destination of a consumer written in a lexical loop and for cleanup
while transporting that exit. Strymonas, in turn, shows how staged filtering and nested flat-map
can remove intermediate stream overhead. That work gives a meaningful performance comparator once
candidate implementations agree on the trace. No performance result follows from our equivalence.
[Pipes.Core](https://hackage-content.haskell.org/package/pipes-4.3.16/docs/Pipes-Core.html#v:respond)
· [Kiselyov et al., *Stream Fusion, to Completeness*, POPL 2017](https://yanniss.github.io/streams-popl17.pdf)

### A bounded executable comparison

We compared two Python models. The reference walks route and seat indices with ordinary nested
loops and an explicit outward-exit flag. The second implementation passes consumers to sources,
splits expansion from filtering, and unwinds an explicit control token. They share input data and
an event vocabulary. Each computes its own effects and control transitions.

There are seven possible seat sequences of length zero, one or two over prices 80 and 120. With
zero, one or two routes, this gives `1 + 7 + 49 = 57` inventories. Seven terminal policies cover
always-continue plus ordinary false, outward return or escaping failure at accepted visit one or
two. For ordinary false the policy gives true on subsequent visits. The comparison found **zero
trace differences in 399 cases**. Two independent changes to the factored model test what this
observation can distinguish.

| Change to the factored traversal | Differing cases | A shortest distinguishing input |
| --- | ---: | --- |
| Reply false to a filtered seat | 258 / 399 | One route, one fare of 120, always continue. Neither terminal sees a seat. The reference delivers true to the source and the changed version delivers false. |
| Omit the seat close when a terminal exit escapes | 134 / 399 | One route, one fare of 80, return at the first visit. Both runs deliver 80. Only the reference closes the seat cursor. |

Empty inputs take neither changed branch. The one-seat witnesses are therefore minimal by number of
offered seats. The executable appendix reproduces the counts using only the Python standard library.
The experiment checks model agreement on this domain. The case argument above covers longer finite
inputs in the fragment.

### Validation against a CoD implementation

Derive each expected trace from the generator contract and record which operand forms the pinned
seed or booter accepts. A disagreement between the two compilers is a diagnostic input. Use the
first differing handoff and its governing clause to assess each implementation independently.
Drive the two CoD spellings from independently instrumented table and cursor producers. Log at the
source and terminal boundaries, before final delivery to the caller. First compare the four fare
traces above. Add a catch for the same declared exception around a source's offer and outside the
caller's loop. Inject one failure while preparing the fare and one in the terminal. The source
catch should see the former, the caller the latter. Introduce a failing close in a separate run
and use CoD's cleanup-failure rules as the oracle. Successful cleanup was an explicit premise of
the proposition. [Failure phase and cleanup rules](../design/RangedForAndGenerators.md#cleanup-completion-at-the-for-boundary)

Two further checks delimit the result. A source that requires Boolean feedback must reject a
consumer path that completes normally without providing it. A stage that retains a borrowed offer
beyond its proven owner must also reject. Then run a zero-input prefix stage with an observable
source-argument preparation step. Argument preparation should occur once in written order even
when the traversal never opens the source. These checks exercise typing, owner validity and
preparation beyond the scalar, prebound model. Infinite search fairness, suspension and Actor
delivery require their own observations. [Preparation and control](../design/ProducersAndTransformers.md#preparation-control-and-storage)

<details>
<summary>Executable appendix · Python 3 differential model</summary>

Copy this block to a file and run it with Python 3. A failure outcome carries the fare as a stable
test label. A normally exhausted traversal has outcome (return, 0) in both models, even for a consumer that
visited every fare. The model uses a host exception as its private control token. Generator catches, native
cleanup failures and CoD exception identity are checked separately by the validation plan above.

```python
from itertools import product


def reference(routes, policy):
    events = [("open", "routes")]
    visits = 0
    outcome = ("return", 0)
    escaping = False
    for r, seats in enumerate(routes):
        events.extend([("route", r), ("open", r)])
        for i, fare in enumerate(seats):
            events.append(("offer", r, i, fare))
            action = "more"
            if fare <= 100:
                visits += 1
                events.append(("visit", fare))
                if visits == policy[1]:
                    action = policy[0]
            if action in ("return", "fail"):
                escaping = True
                outcome = (action, fare)
                break
            events.append(("feedback", r, i, action == "more"))
            if action == "end":
                break
        events.append(("close", r))
        if escaping:
            break
        events.append(("resume-route", r))
    events.extend([("close", "routes"), outcome])
    return events


class Escape(Exception):
    pass


def composed(routes, policy, mutant=None):
    events = []
    visits = 0

    def route_source(k):
        events.append(("open", "routes"))
        try:
            for r in range(len(routes)):
                events.append(("route", r))
                k(r)
                events.append(("resume-route", r))
        finally:
            events.append(("close", "routes"))

    def expand(consume):
        def visit_route(r):
            events.append(("open", r))
            try:
                for i, fare in enumerate(routes[r]):
                    events.append(("offer", r, i, fare))
                    reply = consume(fare)
                    events.append(("feedback", r, i, reply))
                    if not reply:
                        break
            except Escape:
                if mutant != "leak":
                    events.append(("close", r))
                raise
            events.append(("close", r))
        route_source(visit_route)

    def filter_price(consume):
        def offer(fare):
            if fare > 100:
                return mutant != "stop-on-skip"
            return consume(fare)
        expand(offer)

    def terminal(fare):
        nonlocal visits
        visits += 1
        events.append(("visit", fare))
        if visits != policy[1]:
            return True
        if policy[0] == "end":
            return False
        raise Escape(policy[0], fare)

    try:
        filter_price(terminal)
        outcome = ("return", 0)
    except Escape as transfer:
        outcome = transfer.args
    events.append(outcome)
    return events


seats = [s for n in range(3) for s in product((80, 120), repeat=n)]
policies = [("more", 0)] + list(product(("end", "return", "fail"), (1, 2)))
differences = dict.fromkeys((None, "stop-on-skip", "leak"), 0)
cases = 0
for n in range(3):
    for routes in product(seats, repeat=n):
        for policy in policies:
            cases += 1
            expected = reference(routes, policy)
            for mutant in differences:
                differences[mutant] += composed(routes, policy, mutant) != expected
print(cases, differences)
```

</details>
