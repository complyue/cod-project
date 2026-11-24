# Composing Enumerative Choice with Producers and Transformers

## Abstract

A booking service can stop at the first affordable fare while its route and seat cursors are open.
What should a higher-kinded transformer signature tell a library author who wants to reuse that
search with a different inventory source? We compare two choices. One abstracts over the family
that carries the offered items. The other makes the source's normal feedback visible in that
family. Both can type the composition of route expansion and price filtering. Only the second
exposes the difference between a route that expects void feedback and a seat source that expects a
Boolean. Even a correctly typed filter can answer the seat source incorrectly or open a cursor it
never needs. We derive the missing behavioral obligations from explicit CoD loops and trace which
ones a future type language could check at an adapter boundary.

## Two ways to type a transformer

The running inventory offers routes A and B. A offers seats priced 120, 80 and 80, in that order.
B offers one at 90. The filter accepts fares at most 100. A searcher can collect every accepted
seat or return as soon as the first one appears.

In the *paper notation* below, `Type` stands for a completed type or protocol signature. The kind
`Type → Type` classifies a family that still needs one type argument. `F`, `G` and `H` range over
such families. A type synonym abbreviates a type expression. These equations propose ways to write
and compare contracts in a future CoD type language. The CoD listings later in the paper provide
the independent source semantics. The project specifically identifies higher-kinded type synonyms
and a formal producer/transformer/consumer language as an endpoint-design requirement.
[Design direction](../design/ActorChannels.md#independent-chan-endpoints)

### Candidate A. Abstract over the item carrier

```text
F, G, H                         : Type → Type
Producer[F, A]                  ≔ F[A]
Transformer[F, G, A, B]         ≔ F[A] → G[B]
Stage[F, A, B]                  ≔ Transformer[F, F, A, B]
Consumer[F, A, Z]               ≔ F[A] → Z
```

`Consumer` here names a terminal that drives a **whole traversal** and produces a result `Z`. One
invocation of the loop body receives an individual item. Keeping those roles distinct makes it
possible for the terminal to return a fare while its source finishes void. The companion paper can
instantiate the same family notation for events with an entirely different way of consuming them.

The higher-kinded step is passing `F` itself to `Stage`, then applying it at two potentially
different types, `A` and `B`. A parameterized name like `Fares[A]` with a fixed definition would
let us vary the item type alone. The composition rule also admits a change of carrier.

```text
p : Transformer[F, G, A, B]       q : Transformer[G, H, B, C]
q ∘ p : Transformer[F, H, A, C]

c : Consumer[H, C, Z]
c ∘ q ∘ p : Consumer[F, A, Z]
```

Read composition here as connection of already prepared stages. Operand evaluation, opening a
source and cleanup follow their own contracts. A bare family parameter supplies no operation for
iterating an arbitrary `F[A]`. A generic stage that needs to drive it must receive that operation
or require it through an admitted protocol.

Let `Ordered[A]` denote a finite sequence of A. With `F = G = H = Ordered`, a route expander has
type `Ordered[Route] → Ordered[Fare]`, a filter has type `Ordered[Fare] → Ordered[Fare]`, and their
composition maps routes to fares. `Route` and `Fare` are domain labels in the paper notation. The
CoD fixture uses scalar IDs and prices. This instantiation captures the full-enumeration result
80, 80, 90. Its terminal can select the first of those answers. A version that materializes all
seats first can have exactly the same types and answer while opening an extra inventory cursor.
For a replay over immutable data, item order and multiplicity may be all its caller needs.

### Candidate B. Index a closed traversal by the reply it requires

Keep the outer composition rule. This second design specializes the first by making one index
part of the producer family. It changes the contract that library interfaces must expose while
retaining the same arity for `Transformer`. Introduce a family that includes a synchronous
driver. For the scalar fragment, the driver takes a live single-brace consumer, may offer an
`A` to it, receives an `R` when the offer completes normally, and finishes void. In the following
notation `Here` stands for this bounded invocation with CoD lexical control. It carries no right
to retain the supplied body.

```text
Visit                           : Type → (Type → Type)
Visit[R][A]                     ≔ traversal driven by Here(A → R), with void completion
Routes                          ≔ Visit[void]       : Type → Type
Seats                           ≔ Visit[bool]       : Type → Type

expand                          : Transformer[Routes, Seats, Route, Fare]
price_filter                    : Stage[Seats, Fare, Fare]
first                           : Consumer[Seats, Fare, Fare]

price_filter ∘ expand           : Visit[void][Route] → Visit[bool][Fare]
first ∘ price_filter ∘ expand   : Visit[void][Route] → Fare
```

The partial applications `Visit[void]` and `Visit[bool]` leave their item argument open. Substitution
now does real work. Expansion drives a route source using a body that finishes void, and it drives
each selected seat source using a body that returns bool. The filter must supply bool even on an
upstream seat for which it makes **zero** downstream offers. Forwarding `bool` to the route source,
or attaching a filter requiring Boolean feedback to a fare source expecting void, fails the
contract under exact protocol matching. A separately authored adapter could supply the missing
policy. The same `Visit` constructor applied to the proper indices gives the signatures on either
side of that adapter.

Two filters can nevertheless return opposite Boolean values for a skipped seat. Both satisfy this
signature. Likewise, a body may return a value from its lexical supplier or throw, in which case
the pending offer receives no `R`. Encoding an outward return as an ordinary reply such as
`false` would change the caller's meaning. This candidate therefore relies on the language rules
for bounded `here` invocation, checked failures and unwinding of live scopes. If an optimizer
wants to exchange one implementation for another while preserving resource use, it also needs
evidence about which inputs the stage drives, how it handles skipped offers and which cursors it
opens. The kinds and synonym expansions alone do not provide that evidence.

## A closed traversal

Prices in the inventory are positive, so the booking procedure can reserve
zero for an unsuccessful search. A seat source takes a Boolean reply at every normally completed
handoff. True asks it to advance, while false finishes the current seat cursor. The route source
takes void feedback. These are the contracts of the [CoD fare example](../design/ProducersAndTransformers.md#one-fare-search-three-spellings).

Here are finite producers for that example. Route IDs 1 and 2 stand for A and B. The four ordinary
library procedures named `open_…` and `close_…` record a cursor acquisition or release and
complete normally. The caller can observe their order. Each seat price is copied into a fresh
scalar loop control.

```cod
proc itineraries(day:u64) yields u64 {
    open_routes(day)
    try {
        yield(1u64)
        yield(2u64)
    } finally { close_routes(day) }
    return
}

proc fares_on(route:u64) yields u64 -> bool {
    open_seats(route)
    try {
        if (route == 1u64) {
            if (not yield(120u64)) { return }
            if (not yield(80u64)) { return }
            yield(80u64) : void
        } else {
            yield(90u64) : void
        }
    } finally { close_seats(route) }
    return
}
```

The source explicitly tests the feedback before each remaining seat on A. At the last seat there
is no remaining work and it discards the normal reply. The generator's own final completion is
void in both procedures. In the following reference search the single-brace consumer instead
has permission to return a fare from `first_affordable`. [Independent handoff and final results](../design/RangedForAndGenerators.md#void-final-completion)

```cod
proc first_affordable(day:u64) -> u64 {
    for val route <- itineraries(day) {
        for val fare <- fares_on(route) {
            if (fare <= 100u64) { return fare }
            continue true
        }
    }
    return 0u64
}
```

A *closed traversal* uses each supplied body while its enclosing invocation is still active. The
same producers accept explicit single-brace calls. The receiving headers below select the same
fresh scalar controls as the loops above. In particular, the inner `return` still belongs to
the booking procedure. [Generalized loops](../design/RangedForAndGenerators.md#generalized-for-loops)
· [Receiving headers in both forms](../design/RangedForAndGenerators.md#one-receiving-header-for-both-call-forms)

```cod
proc first_by_call(day:u64) -> u64 {
    itineraries(day) (val route <-) => {
        fares_on(route) (val fare <-) => {
            if (fare <= 100u64) { return fare }
            continue true
        }
    }
    return 0u64
}
```

### Extracting the two stages in CoD

Start with a stage whose source is known by name. `available_fares` expands each route into its
seat offers. `affordable_fares` filters that sequence. Both are ordinary named generators that
receive explicit data arguments. Each invocation of `yield` below refers to the consumer supplied
to the lexically enclosing generator. [Lexical use of yield](../design/RangedForAndGenerators.md#a-generator-declaration-introduces-a-here-parameter-named-yield)

```cod
proc available_fares(day:u64) yields u64 -> bool {
    for val route <- itineraries(day) {
        for val fare <- fares_on(route) {
            continue yield(fare)
        }
    }
    return
}

proc affordable_fares(day:u64, limit:u64) yields u64 -> bool {
    for val fare <- available_fares(day) {
        if (fare > limit) { continue true }
        continue yield(fare)
    }
    return
}

proc first_factored(day:u64) -> u64 {
    for val fare <- affordable_fares(day, 100u64) {
        return fare
    }
    return 0u64
}
```

The filtered branch contains the design choice. The filter has received 120 from its source.
It owes a Boolean, even though the booking procedure never sees 120. Returning true here lets
inventory reach the first 80. On an accepted seat the filter forwards the terminal consumer's
normal reply. The expansion stage passes that reply to the particular seat cursor that offered
the fare. When the terminal returns outward, the pending `yield(fare)` expressions never produce
ordinary feedback. [Feedback on normal paths](../design/RangedForAndGenerators.md#non-void-feedback-checks-all-normal-exits)

## What an equivalence must preserve

For these procedures, observe the opens and closes, each seat offered by its route and position,
each visit of an accepted fare, the feedback reaching its seat source, and the value or failure
reaching the booking caller. An outside inventory service can observe the cursor operations.
A source can distinguish true, false and an offer that never resumes. Price equality does not
erase seat identity in those observations.

Both searches open the route cursor, enter A and open its seat cursor. A offers 120. The explicit
loop answers true in its final `continue`; in the factored program the filter answers true and
expansion relays it. A offers its first 80. The explicit body returns from the search. In the
factored program the same offer passes through both stages before the final body returns from
its search. In each run A closes, the route cursor closes, then the caller receives 80. No Boolean
reaches that second offer. Neither the second seat priced at 80 nor route B is visited.

### Three continuations at the first acceptable seat

The terminal can give ordinary Boolean feedback, return from the caller or raise an escaping
failure. Change only that decision in the two programs. The table gives the
rest of the paper derivation from the first accepted 80. An ordinary false below applies once.
On later visits that consumer replies true.

| Terminal decision | Seat cursor and route behavior | Observable result |
| --- | --- | --- |
| Always give ordinary true feedback | A offers its second 80, then closes. B opens, offers 90 and closes. The route cursor closes after B. | The terminal visits 80, 80, 90, preserving order and multiplicity. |
| Give ordinary false feedback at the first 80 | A receives false and closes. The route body completes normally, so B still opens. Its only seat receives true. B and then the route cursor close. | The terminal visits 80, 90. |
| Return 80 from the booking procedure | A's active offer receives no reply. A closes, then the route cursor closes. | The caller receives 80 after cleanup. |
| Raise a checked failure that escapes the terminal | A's active offer receives no reply. Both cursor cleanups run. Assuming they complete normally, the failure reaches the supplying search's loop site. | A caller-side handler or a declared checked obligation is required. |

A generator catch enclosing a `yield` may handle a failure while preparing its own seat. A
failure of the supplied body bypasses that generator's catch. An intervening handler written
in a consuming body has its ordinary opportunity at its own loop site. The example has no such
handler. If a close itself fails, apply the cleanup-failure rules before deciding what reaches
the caller. [Nested failure boundaries](../design/RangedForAndGenerators.md#execution-and-nested-loop-boundaries)
· [Cleanup at the loop boundary](../design/RangedForAndGenerators.md#cleanup-completion-at-the-for-boundary)

**Paper result.** For finite ordered inventories, a fixed total price predicate, the same terminal
policy and initial state, and successful cleanup, the displayed extraction preserves these
observations. At the beginning of a seat offer the runs have the same live cursors, seat position,
and history of terminal visits. A rejected fare elicits the same true reply. An accepted fare
enters the same terminal once. On a normal reply the same seat source receives the same value and
either advances or finishes. On an outward completion both runs leave the same live cursor scopes
in the same order. Empty routes contribute an open and close without a seat offer. Exhaustion
completes the final route and closes its parent. Applying these cases to the remaining seats and
routes establishes the claim. Extra stage calls are internal to this observation. The argument
assumes identical input evaluation and no extra effects in the stage procedures.

Two small counterexamples show why the observation matters. With A offering 120 then 80, returning
false for the rejected 120 makes the service miss A's affordable seat. A different faulty filter
can cache the first 80, enumerate B and eventually return the cached fare. A caller comparing
only the final number will accept that implementation even though it made an extra inventory call.

## Distinguishing the two candidates

Three workloads expose different demands on the type language.

| Workload | What the carrier families express | Contract needed to decide the trace |
| --- | --- | --- |
| Filter every seat out | Candidate A connects equal item carriers. Candidate B additionally requires a Boolean on every normal seat handoff. | The filter's skip policy must keep this seat cursor open until its remaining offers have been examined. |
| Take zero routes | Both can assign an endomorphism `F[Route] → F[Route]` to the stage. | A zero cap should leave the upstream body unopened while preserving effects from preparing its ordinary arguments. |
| Merge two ordered sources | A binary transformer can require two independently typed inputs and produce one output. Candidate B can additionally state each input's normal feedback type. | Producing ordered output with bounded lookahead needs access to a retained head and a way to advance the corresponding source. |

For the merge, a useful paper signature is
`Merge[F, G, H, A] ≔ (F[A], G[A]) → H[A]`. The parentheses stand for two supplied arguments. They
grant no permission to store either argument. Substitute `Visit[void]` on both sides of the input
and ask for an ordered merge of 10, 30 on the left with 20, 40 on the right. After emitting 10 and
20, the consumer needs 30 without losing the position preceding 40. The source protocol permits
nested calls but offers no operation to resume a traversal that has returned. One-shot sources
make restarting an observable operation. A library can distinguish a right-hand
`Cursor[A]` with documented peek, advance and owner lifetime, or introduce a separate resumable
protocol. Merely replacing `G` by a new name with the same kind adds no such capability.
[CoD's ordered-journal case](../design/ProducersAndTransformers.md#replay-two-sorted-journals-in-order)

### Binding arguments without entering the source

The extraction already works because each stage names the operation it drives. A library author
may also want to supply a fresh inventory traversal or a recorded traversal to the same filter.
CoD's confirmed composite scope includes bounded lexical pipeline connections. For example,
`here selected = src !> stages` prepares a compatible connection, and `selected $> report` drives
it with a named terminal. Preparing a source's ordinary arguments and entering its body occur
at different points. Its actual owners must stay live through use.
[Confirmed composite scope](../design/ProducersAndTransformers.md#composite-implementation-target)
· [Three bounded connection forms](../docs/IdiomaticCoD.md#name-reusable-pipeline-connections)

The zero-cap case distinguishes two general binding choices even before the first seat is offered.
Let `cap` be an already prepared processor that takes zero items, and let `select_day()` record
one observable call. Drive the resulting traversal with a consumer. An explicit connection
`itineraries(select_day()) !> cap` prepares the day once. Because the cap never asks for its input,
the route cursor never opens and the consumer sees no item.

Should a caller also be able to write `cap(itineraries(select_day()))`? Under contextual producer
binding, the expected producer signature could retain the prepared day and give the same trace.
Under a rule requiring the explicit connector, that nested spelling would reject in a context
with no applicable ordinary value call. If both interpretations were viable, the rule would also
need a way to select one. The distinguishing observations are the acceptance of the spelling,
one day selection and zero cursor opens. Named, parameterized signatures and their general
binding rules are a future language-design question. The independent chan endpoint workload
adds a further requirement for higher-kinded type synonyms. Its local consumer feedback remains
within its own traversal. [Deferred signature design](../docs/DesignTopics.md#named-signatures-and-pipelines)
· [Independent chan endpoints](../design/ActorChannels.md#independent-chan-endpoints)

## Prior work and evaluation

The Haskell Report translates dependent list generators using `concatMap` and a failing guard
using an empty branch. For a finite, pure, fully consumed inventory this predicts 80, 80, 90.
Go's range-over-function protocol gives its iterator a Boolean continuation signal. CoD lets
each producer declare the type and interpretation of its normal feedback. In this example,
false ends one route's seat cursor and the route traversal can continue. Haskell's
`Pipes.Core.respond` already represents an output and a separately typed reply. The additional
comparison for CoD is where a lexical return goes and which active resource scopes it leaves.
[Haskell 2010 Report, §3.11](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html#x8-420003.11)
· [Go specification, range clauses](https://go.dev/ref/spec#For_range)
· [Pipes.Core](https://hackage-content.haskell.org/package/pipes-4.3.16/docs/Pipes-Core.html#v:respond)

<a id="validation-against-a-cod-implementation"></a>

### Reviewing future source proposals

Future producer or transformer proposals can be reviewed directly against the CoD source above.
Substitute an empty route, two equal fares, a filter rejection before a match, a terminal that
answers false, and a caller that returns or fails. For each offer, identify the supplier of its
feedback or the destination of its outward exit. For each acquired cursor, place its close in
that same trace. For a proposed general binding, also record every preparation effect before
opening a cursor. Under candidate A, the result of this review is a library contract for each
family admitted as `F`. Candidate B moves the normal reply obligation into the type of the
connection and leaves the stage's interpretation of that reply to the library. A proposal for
more expressive types can be judged by which pair of filters, caps or merges it can distinguish
and what the caller can consequently rely on.
