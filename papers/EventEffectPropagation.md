<a id="open-event-pipelines-with-dynamically-scoped-event-sinks"></a>

<a id="typing-open-event-pipelines-across-actors-and-reactive-timelines"></a>

# A Vocabulary for Propagating Event Effects — Composing Domain Rules with Open Event Sinks

## Abstract

A monitoring team can specify when a pump alarm takes effect. A maintenance team can specify what an alarm means for its work. How can the teams compose those rules when the monitor was written before the maintenance application existed? We study a vocabulary in which a domain rule can offer a named event to a typed, open sink. The application connecting the rules supplies the interpretation of that offer. A pump monitor and a fuel-sale controller expose the obligations behind this vocabulary. Routing determines which site can raise an alarm. Progress determines when the rule can decide. The grouping of events into a local frame determines which price goes on a receipt. We use the [composition paper](ComposingAcrossLifetimes.md#two-tests-beyond-fare-search) to distinguish a reusable rule from the live callback, state and delivery evidence required for a particular interpretation. This is the open-network study for [Event Processing Systems](../design/EventProcessingSystems.md).

## From a pump alarm to an inspection request

Two sensors monitor a pump. At each site, the monitoring rule observes a window until both sensors have reported progress through its end. If both reported a high reading for that pump within the window, the rule offers a `PumpAlarm`. A maintenance team uses that domain event to request an inspection. Each author can describe the effect they intend in the terms of their own application. In paper notation:

```text
Monitoring rule
    When the site's window closes with high readings from both A and B,
    offer PumpAlarm(site, pump, window).

Maintenance rule
    On PumpAlarm(site, pump, window),
    offer InspectionRequested(site, pump, window).
```

The monitoring module publishes the alarm protocol. The maintenance module accepts that protocol and introduces the inspection-request protocol. An integration can supply the maintenance rule at the monitor's alarm sink and a work-order adapter at the maintenance rule's request sink. A replay author can instead supply an alarm recorder at the first sink. Neither rule needs a list of downstream applications. Here *open* means that another author can supply a compatible interpretation at the point of composition. The domain names above illustrate the proposed vocabulary. In the [CoD ingress example](#a-dynamically-scoped-event-sink), a declared procedure symbol and its installed implementation give a concrete way to select a sink.

The [worker](#a-worker-that-can-distinguish-the-histories) produces the local decision `Alert` when its window qualifies. The monitor interprets that decision by offering one `PumpAlarm` for the site, pump and window. Compare the same readings with one synchronous handler at each connected sink, successful calls, and an initially empty observation log.

| Window history and sink selected | Alarms offered | Inspection requests offered |
| --- | --- | --- |
| Split the high readings across live and trial. Connect maintenance. | None | None |
| Pair both high readings at live. Connect an alarm recorder. | One live alarm, recorded | None |
| Pair both high readings at live. Connect maintenance and a request recorder. | One live alarm | One request for the same site, pump and window |

The last two rows give the same monitor an observably different business interpretation. They also locate the domain boundary precisely. A recorded request establishes that maintenance asked for an inspection. A work-order service that must confirm acceptance or completion has to describe that further event. The following endpoint and timeline cases ask what must be typed, retained and observed to honor such rules when the next handler runs elsewhere or later.

## Which constructor belongs to each endpoint?

An alarm interpreter may run locally or be reached through a chan. To give independent authors only the authority their rules require, the higher-kinded question is whether each component can type its half of the connection. An instrument adapter needs authority to *send* a measurement. A monitor needs
authority to *receive* its admitted input. Neither needs the other component's endpoint. We
would like to reuse the typing of local producer, transformer and consumer composition on each
side while allowing the two sides to use different traversal and endpoint families. CoD identifies
independent typed chan ends and higher-kinded type synonyms as the language direction. The following
signatures are **paper notation for exploring the design**. An arrow names a typing relation for
an operation. Its storage, calling convention, owner and dynamic requirements need their own CoD
contracts. [Independent chan endpoints](../design/ActorChannels.md#independent-chan-endpoints)
· [Producer and transformer contracts](../design/ProducersAndTransformers.md#a-transformer-takes-a-producer-as-an-input)

Use the notation of the [closed traversal paper](ClosedEventPipelines.md). A family `F` of
kind `Type → Type` takes an element type `A` and produces a completed local producer signature
`F[A]`. A transformer changes the element type and, if its contract calls for it, the family.
A terminal consumer drives a whole producer and yields a local result `Z`.

```text
F, G, H, Tx, Rx             : Type → Type
Producer[F, A]              ≔ F[A]
Transformer[F, G, A, B]     ≔ F[A] → G[B]
Consumer[F, A, Z]           ≔ F[A] → Z
```

This makes the direction of an endpoint precise. A sending end of type `Tx[A]` accepts each local
item. A terminal can use that end to drive a producer of such items. A receiving end of type
`Rx[A]` supplies items that the local worker can offer downstream. Give it a bounded turn so the
paper can name its local producer without suspending a submitter until the next message arrives.
The following are required adapter operations, parameterized over the admissible message types
and the particular constructor families for which an implementation supplies them.

```text
sender[Tx, F, A]       : Tx[A] → Consumer[F, A, LocalExit]
ready[Rx, G, A]        : (Rx[A], TurnBudget) → Producer[G, A]
```

`sender(tx)` is a terminal requiring only the sending end. On a normal local visit it offers the
item and applies the local sending policy. `LocalExit` describes the local traversal's completion.
`ready(rx, budget)` supplies only locally admitted input and can produce an empty turn. A chan
can have outgoing traffic while the other Actor has nothing ready. These adapters also require
an actual way to traverse `F` and `G`, an admissible payload, and permission to use the selected
endpoint. The kind of a constructor by itself supplies none of those operations. In particular,
`Consumer` in this notation consumes a *producer*. The per-item operation hidden inside the
sending terminal consumes a *value*. Substituting one for the other changes the interface.

### One ingress plan for live and replay

An instrument team can prepare the work that a live adapter and a replay audit have in common.
In this paper example `decode_frames` offers one raw record per well-formed device frame.
`normalize` converts each such record to the application's reading or progress input, preserving
sensor identity, timestamp and per-source order. An `Advance` record that overtakes an earlier
accepted reading can close the window too soon.

```cod
here ingress = decode_frames() |> normalize()
```

If decoding connects `Wire[Frame]` to `F[Raw]` and normalization connects `F[Raw]` to
`G[Input]`, then `ingress` still has both ends open at `Wire[Frame] → G[Input]`. The
intermediate protocol must match, including any normal reply. Constructing this expression
prepares the stages once without reading a device, selecting a queue or admitting a record.
The two local drives below use the CoD connection spellings with illustrative adapters.

```cod
live_frames()   !> ingress $> admit_live
replay_frames() !> ingress $> record_input
```

Here the normalized traversal takes void feedback. The per-item terminal `admit_live` sends
each input through the live site's admission policy. The per-item terminal `record_input` logs
the normalized fields for an audit. Both return void. The earlier `sender(tx)` adapter has a
different calling convention. It drives a whole producer, so its corresponding attachment in
the paper notation is `sender(txLive)(ingress(liveFrames))`. On a recording of the same
well-formed frames, compare the normalized records **before** the different terminal actions.
On the live path, queue admission and the selected site still determine what survives.

As in the design for `pcl.cod`, the library can compose typed operations before a caller
supplies the concrete input and output. ADT receivers can carry configuration. The driver
supplies fresh synchronous callbacks when the plan runs. In the
proposed parser protocol one of four callbacks completes a rule. In this event application
the local plan can finish while the receiving Actor has an empty turn. What crosses the chan
is an admissible message. A later `ready(rx, budget)` drives a new local computation with
that site's window state and its own terminal. Retaining the sender's `here` until an alarm
arrives would violate its lifetime and give send a remote acknowledgement it does not promise.
[Lexical connections](../design/ProducersAndTransformers.md#lexical-variables-for-open-pipelines) · [PCL rule interface](../design/ParserCombinatorLibSupport.md#the-rule-interface) · [Companion fare plan](ClosedEventPipelines.md#an-itinerary-plan-with-both-ends-open)

### Two compositions and a missing arrow

An adapter reading a sensor journal can normalize raw equipment records into the monitor's
`Input`, including source progress. On another turn the monitor uses a window state `w` to
produce alert records. Let `F` denote the local journal traversal, `G` its normalized traversal,
`H` the worker's admitted batch and `J` the traversal of alarms. All four have kind `Type → Type`.
`Input` below denotes the reading or progress fields used in the CoD examples. Fix a compatible
sending endpoint `tx : Tx[Input]` and receiving endpoint `rx : Rx[Input]`.

```text
normalize                    : Transformer[F, G, Raw, Input]
sender(tx)                   : Consumer[G, Input, LocalExit]
sender(tx) ∘ normalize       : Consumer[F, Raw, LocalExit]

ready(rx, budget)            : Producer[H, Input]
decide(w)                    : Transformer[H, J, Input, Alarm]
publish                      : Consumer[J, Alarm, LocalExit]
publish ∘ decide(w)          : Consumer[H, Input, LocalExit]
```

The first composition checks because `normalize` ends with exactly `G[Input]`. The second checks
because `decide(w)` takes `H[Input]` and produces `J[Alarm]`. Its terminal can run on
`ready(rx, budget)`. Passing a sending end for `Alarm` into the first composition at `Input` is
a type error. An adapter that can use two different `Tx[Input]` values for live and trial must still
select the right one. Type correctness distinguishes the message families and endpoint roles.
Source identity, route selection, owner lifetime and the watermark's meaning require more evidence.
Even matching the payload on both sides leaves an operational question: which receiving endpoint
does this particular sending endpoint reach? The application fixes the pairing in the examples.

Here are two concrete instantiations of the *families*, keeping the same business types. They name
hypothetical application signatures, rather than introducing built-in containers or endpoint APIs.

| Context | Sender's substitution | Receiver's substitution | Where a value can outlive submission |
| --- | --- | --- | --- |
| One event loop | `Tx = QueueWriter`, `G = JournalVisit` | `Rx = QueueReader`, `H = ReadyBatch` | A destination-owned queue copies a complete input record. `ReadyBatch[Input]` borrows a later drain turn. |
| Two Actors | `Tx = RegionSend`, `G = JournalVisit` | `Rx = RegionReceive`, `H = ReadyBatch` | The sender constructs an admissible regional representation. A receiving Actor offers the input only after the whole region has been received and validated. |

For example, `RegionSend[Input]` and `RegionReceive[Input]` can share a payload type while a
sensor module receives only the former. The higher-kinded abstraction is the ability to pass
`QueueWriter` or `RegionSend` *before* applying it to a payload type, then reuse the adapter
signature at both `Input` and `Alarm` when their admission evidence is available. An individual
method taking ordinary generic `A` can type a fixed `QueueWriter[A]`. It cannot thereby abstract
over whether the chosen writer family is `QueueWriter` or `RegionSend`.

A tempting shorter signature gives both roles one family and calls both ends `Port[A]`. With no
role parameter or restricted views, interchanging the two endpoints of the same payload type passes
the shape check. Requiring one shared constructor `K` for both arguments instead excludes the
`RegionSend` / `RegionReceive` substitution above. Independent `Tx` and `Rx` solve this part.
A role-indexed family, partially applied at `Send` and `Receive` to yield two distinct unary
families, is another candidate. Its useful test is whether a component holding only
`Port[Send, Input]` can type its entire sender path without acquiring receive or close authority.
Variance of the endpoint, close authority, multiple receivers and how local failures appear in
`LocalExit` remain design choices.

There is deliberately no composition equation identifying the sending terminal's result with the
receiving turn's input. Consider a single submitted reading. The local traversal finishes, the
region goes missing and the next receiving turn is empty. Alternatively, a receiver stops
draining, retained items consume capacity, further admission or transmission slows, and work can
resume as capacity becomes available. Both behaviors fit these endpoint families. Transport
backpressure constrains resource use through time. Equating `LocalExit` with remote receipt, or
using synchronous producer feedback to acknowledge the remote worker's computation, would exclude
valid executions. A stronger acknowledged stream requires an application protocol and a return
path. [Region publication and capacity](../design/ActorChannels.md#regional-transport-over-akcp)

The pump example below asks what the constructor still leaves for the program to establish.
The [fuel-sale example](#a-reflex-style-network-for-a-fuel-sale) tests a different family
parameter, the timeline within which a price change and a sale can be observed together.

<a id="where-asynchrony-enters"></a>

## What outlives a submission?

A sensor adapter can return immediately after submitting its reading. The next reading may come
from an independent invocation. A worker can emit an alarm after *both* adapters have returned.
We call this an *open* pipeline. In a closed traversal, the producer that offered the current
item can still receive feedback or a control exit from its active consumer. Here the pump monitor
owns the unfinished correlation across calls. The distinction already arises on one event loop
when ingestion and processing run on separate turns. [Closed traversal and feedback](ClosedEventPipelines.md)

The application has two queues, one per deployment, and an ordinary named procedure
`admit(site:Site, s:Sensor, k:Kind, stamp:u64) -> void`. For the worked histories there is room
for every record. Before `admit` returns, the selected queue holds a copy of the three input
fields. The queue owner lives through the later worker turn. Its per-source ordering applies to
the control records as well as the readings. These are contracts of this particular monitor.
A shared queue storing a stable site with each entry could meet the same contract.

The two implementations below use the same application procedure and types. Timestamps and progress
values are nonnegative seconds. `High` reports a high reading at `stamp`. `Advance` announces that
this source has finished its on-time contributions strictly earlier than `stamp` at this site.
Each site has exactly the two sensors shown.

```cod
enum Site:u8 { Live; Trial; }
enum Sensor:u8 { A; B; }
enum Kind:u8 { High; Advance; }
```

<a id="a-dynamically-scoped-event-sink"></a>

## Two CoD ingress procedures

**Explicit destination.** An intermediate adapter that forwards a reading can inspect the `site`
argument or pass it through unchanged. The last call names the destination that receives the copy.

```cod
proc submit_to(site:Site, s:Sensor, k:Kind, stamp:u64) -> void {
    admit(site, s, k, stamp)
    return
}
```

**Dynamically scoped destination.** The application owns the `@ingress` symbol and its procedure
family. Its one method reads a typed `@site` binding. The adapter calls the installed family with
`perform`, which replays the bindings that the activated method needs. An integration site can
therefore select a destination once for a sequence of submissions.

```cod
symbol @site:Site;
symbol @ingress:proc(s:Sensor, k:Kind, stamp:u64)->void;

proc @ingress(s:Sensor, k:Kind, stamp:u64) -> void {
    admit(@site, s, k, stamp)
    return
}

proc submit(s:Sensor, k:Kind, stamp:u64) -> void {
    perform @ingress(s, k, stamp)
    return
}
```

The symbol declaration gives the operation its identity and signature. The method definition
makes its behavior available. Executing `effect proc @ingress;` below installs it with the active
site binding. `submit` is a named procedure reached while that binding is installed. Queued records
contain only bits. Neither the handler nor its invoking activation has to survive until the worker
runs. CoD requires a suitable installation on each reachable invocation path and prevents an
installed handler from escaping as a stored callable.
[Direct symbolic procedure declaration](../design/SymbolicDynamicScope.md#direct-symbolic-procedure-declaration)
· [Installation, invocation and restoration](../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)
· [Call-path checks](../design/SymbolicDynamicScope.md#call-path-binding-checks)

### A trial that can end in two ways

Suppose an operator sometimes ends a trial as soon as its reading has been recorded. To make the
unwind observable, both versions call the same helper after admitting the trial reading.

```cod
data TrialEnded <: Exception { stamp:u64; }

proc finish_trial(stop:bool) -> void throws TrialEnded {
    if (stop) { throw TrialEnded { stamp <- 6u64 }; }
    return
}
```

With explicit routing, the outer caller selects live at both of its submit sites. Its local catch
handles the trial's declared exception and execution proceeds to the second site.

```cod
proc explicit_trial(stop:bool) -> void throws TrialEnded {
    submit_to(Trial:Site, B:Sensor, High:Kind, 6u64)
    finish_trial(stop)
    return
}

proc explicit_prefix(stop:bool) -> void {
    submit_to(Live:Site, A:Sensor, High:Kind, 8u64)
    try { explicit_trial(stop) }
    catch (e:const TrialEnded) {}
    submit_to(Live:Site, A:Sensor, Advance:Kind, 10u64)
    return
}
```

The scoped caller activates live. Its nested call activates the same procedure family again under
trial. Each activation takes its own snapshot. The final `submit` in `scoped_prefix` uses whichever
installation is active *after* the inner procedure has exited.

```cod
proc scoped_trial(stop:bool) -> void throws TrialEnded {
    effect @site <- Trial:Site;
    effect proc @ingress;
    submit(B:Sensor, High:Kind, 6u64)
    finish_trial(stop)
    return
}

proc scoped_prefix(stop:bool) -> void {
    effect @site <- Live:Site;
    effect proc @ingress;
    submit(A:Sensor, High:Kind, 8u64)
    try { scoped_trial(stop) }
    catch (e:const TrialEnded) {}
    submit(A:Sensor, Advance:Kind, 10u64)
    return
}
```

Read these calls with the worker paused. The first accepted record belongs to live. The second
belongs to trial. With `stop` false, the inner return restores the outer installation. With `stop`
true, unwinding restores it before the catch completes. The trial queue still owns its accepted B
record in both cases. Restoration changes the current binding and leaves completed writes intact.
The final record advances **live A**. The two prefixes therefore leave identical queue contents
after their own frames retire. A worker that later fails while consuming B cannot return that
failure to either of these completed catches.
[Scope restoration](../design/SymbolicDynamicScope.md#method-registration-and-scoped-installation)
· [Exception construction and propagation](../design/ExceptionControl.md#exception-construction-throw-and-rethrow)

There is a source-level distinction between selecting an installed procedure and choosing the
environment in which its method runs. Call the following helper while the outer live installation
is active. The helper rebinds the site and leaves the installed procedure in place.

```cod
proc compare_call_modes() -> void {
    effect @site <- Trial:Site;
    perform @ingress(A:Sensor, High:Kind, 8u64)
    behave @ingress(B:Sensor, High:Kind, 6u64)
    return
}
```

The first reading goes to live, the captured site. The second goes to trial, the current site. A
program that wants the most recent site rebinding can consistently choose `behave`. To reproduce
the explicit prefixes with the `perform` convention used here, the trial must activate the handler
under its own site. Simply changing `@site` and omitting the inner activation sends B to live.
Every value in that erroneous program still has the required static type.
[Procedure snapshots and current bindings](../design/SymbolicDynamicScope.md#captured-values-and-call-behavior)

<a id="a-correlation-with-an-observable-answer"></a>

## A worker that can distinguish the histories

A worker consumes accepted records after the adapter calls have returned. For one pump and one
site it tracks A and B's high readings in [0, 10), plus each source's latest progress. The
following CoD function handles that particular window. The application supplies a live, distinct
`FirstWindow` per site, initialized with false flags and both marks at zero. It supplies valid
monotone progress within each source and consumes records in their per-source order. The function
mutates storage supplied explicitly by its caller. A worker publishes an alarm only when the
function returns `Alert` and can retire the closed window's partial match.

```cod
enum Decision:u8 { NoOutput; Quiet; Alert; }

data FirstWindow {
    a_high:bool; b_high:bool;
    a_mark:u64; b_mark:u64;
    closed:bool;
}

func step(w:FirstWindow in O, s:Sensor, k:Kind, stamp:u64) -> Decision
    where {const owner O} {
    if (w.closed) return NoOutput:Decision
    if (k == (High:Kind)) {
        if (stamp < 10u64) {
            if (s == (A:Sensor)) { w.a_high <- true } else { w.b_high <- true }
        }
    } else {
        if (s == (A:Sensor)) { w.a_mark <- stamp } else { w.b_mark <- stamp }
    }
    if (w.a_mark < 10u64) return NoOutput:Decision
    if (w.b_mark < 10u64) return NoOutput:Decision

    val result <- Quiet:Decision
    if (w.a_high) {
        if (w.b_high) { result <- Alert:Decision }
    }
    w.closed <- true
    w.a_high <- false
    w.b_high <- false
    return result
}
```

`NoOutput` leaves the public alarm log untouched. `Quiet` finalizes an unqualified window. `Alert`
finalizes a qualified one. Repeating a high reading from A only sets A's flag again. A distinct
high from B is required. The example discards later input for this closed window. A monitor for
successive windows must separately route a reading at time 10 into [10, 20).
[Access through an explicit target](../docs/IdiomaticCoD.md#ask-an-operation-for-the-access-it-needs)

Consider two submission histories. **Split** starts with the prefixes above. **Pair** changes
only the destination of B's high reading to live. Read each column from top to bottom. The last
three control records complete the histories after the prefix. They use precisely the same ingress
operation as readings in the proper deployment.

| Accepted source record | Split destination | Pair destination |
| --- | --- | --- |
| A high at 8 | live | live |
| B high at 6 | trial | live |
| A advances to 10 | live | live |
| B advances to 10 | live | live |
| A advances to 10 | trial | trial |
| B advances to 10 | trial | trial |

In **Split**, live A sets `a_high`. When live B advances, both marks are at 10 but `b_high` is
false, so live returns `Quiet`. Trial has only `b_high` and also returns `Quiet`. In **Pair**, both
flags are set at live before B advances. Live returns `Alert` once. Trial returns `Quiet`. These
steps remain valid if all consumption takes place after all six submissions. Each step still uses
the state owned by the destination chosen for that record. With the [maintenance rule connected](#from-a-pump-alarm-to-an-inspection-request), that one live alert in **Pair** yields a request for the same pump and window. Changing only the alarm sink to a recorder preserves the window decision and changes the downstream effect.

Erase the destinations and put the records into a common queue. The worker now receives exactly
the same six inputs in both histories, in the same order. Starting with the same state, it has
no observation from which to derive the different required alarms. This proves that routing
information must survive until it can no longer influence processing. The queue's identity,
a stored site field, or a stable mapping from a deployment-specific source identity all suffice.
The names A and B alone cannot encode that distinction here.

### A correct route can carry a false frontier

Take **Pair** again. Admit both high readings to live, then allow live A and B's progress records
to take a faster control path. At the worker the two marks are now processed first. `step` closes
an empty live window with `Quiet`. Each high subsequently meets `w.closed` and yields `NoOutput`.
The site, sensor, timestamp and procedure binding of every record are correct. The missed alarm
comes from letting a progress promise overtake the very data that makes it valid.

The three observations have different meanings. A sample's *event time* selects its window. A source's
*progress* supplies evidence that a particular prefix is complete. *Processing time* measures how
long the worker has waited. Finishing both adapter calls, waiting ten seconds of wall time, or
draining a batch supplies no new source promise. If trial B never advances, the trial window stays
open under this policy. Moving B's high timestamp from 6 to exactly 10 makes it ineligible for the
first window even if every enqueue and dequeue occurs at the same wall time as before.

## When the two programs agree

The comparison fixes the inputs, their intended deployments, the schedule of admissions and
drains, and each source's progress promises. An application may choose bounded admission. In that
case both versions must apply the same capacity policy to the same destination, and progress
must follow every *accepted* earlier reading it covers. A dropped reading cannot be recovered
from a watermark.

An argument by cases covers the permitted transitions. Entering live or trial couples the
explicit destination to its scoped handler. A submit in either program adds the same value to
the same queue, or takes the same capacity decision. Returning or unwinding restores the outer
choice without changing an accepted copy. A worker step consults only queued data and its
destination's state. These facts preserve the equivalence at the next transition. They also
locate the first disagreement when one version departs from the contract. Observing the selected
queue, its admitted prefix and the window's terminal decision can expose that disagreement even
when both public alarm logs happen to be empty.

If ingress and processing reside in separate CoD Actors, the receiver establishes its own dynamic
context. An ordinary statically typed chan send is fire-and-forget. A systemic communication fault
that makes subsequent sends likely to fail must be reported. An individual send supplies no
receipt. An application that wants final-only windows across that boundary must establish which
prefix its receiver may treat as complete, or continue waiting when that information is missing.
The strict progress, ordering and admission policy used in this paper belong to the application.
The argument selects no queue representation or endpoint syntax.
[Actor context and chan guarantees](../design/SymbolicDynamicScope.md#actor-concurrency-direction)

## A Reflex-style network for a fuel sale

A customer lifts a nozzle as the posted price changes. At the end of the sale the last meter
increment and the return of the nozzle reach the dispenser's sale controller together. Which price
and quantity belong on the receipt? A queue can deliver both inputs to an Actor. The order of
callbacks alone is a poor place to hide a pricing rule. This case examines the type language a
CoD library would need to express a rule that the operator can inspect.

Blackheath and Jones develop a petrol dispenser, its meter and its point-of-sale interface as
an industrial FRP example. We borrow the business setting. The coincidence rules, prices and
traces below are a new CoD paper exercise. For the comparison we use Reflex's distinction
between an event occurrence, a sampleable value and a changing value that also exposes its
updates. Its timeline parameter ties a network's signals together. Reflex distinguishes sampling
the value before a simultaneous update from sampling the newly updated value. The difference is
observable on the receipt. [Blackheath and Jones, chapter 4](https://livebook.manning.com/book/functional-reactive-programming/chapter-4)
· [Reflex concepts](https://docs.reflex-frp.org/en/latest/overview.html)
· [Reflex sampling and switching operations](https://hackage-content.haskell.org/package/reflex-0.9.3.4/docs/Reflex-Dynamic.html)

### What the library would need to name

Fix one local sale timeline `T`. Use `E_T[A]` for an event carrying an `A` when it occurs,
`B_T[A]` for a value of type `A` that can be sampled, and `D_T[A]` for a changing value with
both a current value and update occurrences. Each is a unary type constructor after fixing
`T`. The following is **candidate type-level notation for CoD**. It gives a library author two
constructor arguments to one synonym. For each use, `A` is the value sampled, `X` is the
trigger payload and `(A, X)` carries both pieces of information.

```text
F, E                  : Type → Type
Snapshot[F,E,A,X]     ≔ (F[A], E[X]) → E[(A, X)]
before[T,A,X]         : Snapshot[B_T,E_T,A,X]
includingUpdate[T,A,X]: Snapshot[D_T,E_T,A,X]
```

`Snapshot[B_T,E_T,UnitPrice,SaleId]` expands to a function taking a price behavior and a start
event and producing a price together with the sale identity on that timeline. The first
operation can sample the price shown at the beginning of the frame. The second can include
a price update in the frame. The corresponding Reflex operations are `attach (current d)`
and `attachPromptlyDyn d` for a dynamic value `d`. Take `d` and the trigger as the two inputs
to each operation. Projecting `current d` inside the first one gives both the same expanded
shape, `Snapshot[D_T,E_T,UnitPrice,SaleId]`. They still select different prices on coincidence.
The type records the families, payloads and shared timeline. The sampling rule belongs to the
operation's contract.

One possible wiring for a single dispenser is short enough to derive by hand. These equations
are paper notation for the candidate library. The two named sale operations are domain code.

```text
pricedStarts ≔ before(current(postedPrice), start)
saleTotals   ≔ meterBySale(pricedStarts, litresAdded)
receipts     ≔ matchClosingSale(includingUpdate(saleTotals, close))
```

`meterBySale` starts from idle. Each priced start establishes the sale ID, fixes the unit
price and resets the volume. A later posted price has no path into its accumulated total.
`matchClosingSale` emits a receipt for a matching close, at most once per sale ID.
The total sampled at close includes its last increment. The next start resets the volume
after that observation. Passing `current(saleTotals)` to the earlier-value sampler instead
changes only one connection and yields a different receipt in the history below.

The dispenser also illustrates a reusable operator with several inputs. In paper notation,
`receiptPolicy(before, includingUpdate)` fixes the two sampling operations in the wiring
above. A dispenser later supplies its own posted-price, start, meter and close signals on
one timeline `T`, and connects the receipt output to a display or an audit log. Each
installation has its own sale state. The same wiring should yield the 510-cent receipt
below for either terminal. Attaching a close from another timeline requires an explicit
bridge. The four input signals and their common timeline remain visible in the interface.

A potential CoD library might offer one network-building context per local timeline, with a
way to hold state with an initial value and derive current values and update events. The
generality worth testing is whether a library can give the sampling pattern a name and export
its lawful operations while retaining the same timeline on both arguments and the result.
`E_T` and `E_U` have the same kind. Passing a start event from U to a sampling operation
specialized to T must still fail under this candidate when T and U are distinct. An explicit
input bridge can deliver data to T as a new local occurrence. An admitted regional value must
be copied as permitted or carried under an appropriate ownership contract before a retained
signal can use it. A timeline index by itself supplies no region lifetime.

### A receipt that distinguishes the sampling rules

For this exercise the sale controller promises the price displayed immediately *before* the start frame.
That price is fixed for the sale. At completion the controller includes the final meter increment
admitted in the close frame. A frame is one logical propagation step chosen by the local host.
The table stipulates which inputs share a frame. Equal timestamps on two packets would not be
enough to establish that fact. Prices are whole cents per litre, and the example uses whole litres.

| Local frame | Inputs admitted together | Observation required by the sale policy |
| --- | --- | --- |
| Before start | Idle, zero volume, display at 170 cents/litre | A new sale can start at the posted price. |
| 1 | Start sale S and post 180 cents/litre | Capture 170 for S. The posted price for future sales becomes 180. |
| 2 | Add two litres to S | Display two litres and 340 cents for the active sale. |
| 3 | Add the last litre and close S | Emit one local completion for S with three litres at 170, total 510 cents. Retain that completed value before resetting the meter. |
| 4 | No input | The dispenser can display the completed sale. A redraw emits no additional completion. |

Using the newly posted price at startup produces **540** cents. Sampling the preceding meter
value at closure produces **340** cents. Publishing the total as each dependency arrives can
also emit the partial result before the final one. A customer can observe the extra completion
even if a later screen eventually displays 510. A proposed library must let the programmer
name both sampling choices and arrange for the locally published completion to see one
consistent sale. It also needs a causal account of stateful feedback: a completed value can
trigger a reset for the next sale without feeding the reset's zero back into the receipt that
caused it. Whether that needs a distinct delay operator, phased combinators or a checked
construction rule is an open design question.

### A network whose selected input can change

A service console can follow one of two dispenser-specific completion feeds. A possible source
for the console is `D_T[E_T[Sale]]`: the current value selects an event source, and that
selection can itself change. A candidate `switch : D_T[E_T[A]] → E_T[A]` expresses this nested
structure, but leaves a real policy choice. Begin with dispenser A selected. In one frame select
B while A completes sale 17 and B completes sale 29. A switch that takes effect *after* the
frame reports 17, as Reflex's `switchDyn` does. A switch that takes effect *in* the frame
can report 29, as `switchPromptlyDyn` does for these coincident inputs. This gives CoD a second
paper test that first-order mapping over `Sale` alone cannot express. A design that accepts the nested type
but offers no way to specify the cutover still leaves a visible ambiguity.

The host may first admit independent dispenser inputs to the console's own timeline. Neither
that admission nor switching the displayed feed transfers sales settlement responsibility.
The once-per-sale occurrence in the table is local to the stated input history. Sending a
completed sale region to a point-of-sale Actor via a typed `chan` gives the sender no delivery
receipt. A settlement protocol that requires recovery must define durable sale identities,
deduplication and reconciliation at the application level. Two histories with the same local
receipt can therefore differ in what the remote till has recorded. This is where a Reflex-style
library, an input adapter and the CoD Actor transport each have a distinct observable contract.
[CoD actor channels](../design/ActorChannels.md)

## Connecting two closed drives

An open network connects components whose work and observations can extend beyond the submitting call. A source-normalization-terminal chain on the sender can finish as one closed drive. A worker's bounded drain, window step and publication can form another. A local reactive graph can also remain open to future host frames while processing each admitted frame synchronously. At each boundary the network author must say who gets the next turn and which observations that turn can make.

Start with the [booking paper's delayed audit](ClosedEventPipelines.md#where-a-closed-event-pipeline-ends). In its outward-return case the queue owns A2, and the caller has received 80 after both cursors closed. With an independently driven Actor receiver, use this controlled schedule. Assume a valid transfer that eventually succeeds for A2. Record the local send-side observation appropriate to the chosen adapter, retire the sender's activation, run the receiver once before delivery, then once after delivery.

| Observation point | Evidence available at that point |
| --- | --- |
| Sender completes its bounded drive | The local result and cleanup trace are known. The fixture can record submission of A2. |
| First receiver turn has no ready item | The worker completes an empty turn. The prior local result is the same. |
| A2 is delivered before a second turn | The worker may consume the validated record, update its own state, stop its drain or report its own failure. |

A run in which transport never delivers A2 can have the same local completion as the successful-transfer run. Remote processing or settlement that a sender needs to learn about requires a separate application message and an identity to correlate it. An in-process queue can promise a successful local append. The Actor adapter has the [chan send contract](../design/ActorChannels.md). EPS needs to name the observation each adapter exposes. An implementation that suspends the original seat offer until a worker decides would give a different result even in the empty-turn experiment.

The retention question now applies separately to configuration, admitted work and worker state. A shared normalization plan may retain calibration data or a typed policy receiver. A live ingress drive supplies fresh callbacks and its chosen admission service. An admitted record must preserve the selected site, either by its queue or by explicit routing evidence. The worker owns its per-site match and progress independently of that plan. At a timeline boundary the host also supplies the frame membership and the sampling or switching rule. Storing a timestamp alone cannot recover them.

This gives replay a precise contract. To compare decoders, replay the recorded frames and compare normalized records. To reproduce the [pump alarm](#a-worker-that-can-distinguish-the-histories), reproduce the selected destinations and the admitted per-source order, including progress. To reproduce the [receipt](#a-receipt-that-distinguishes-the-sampling-rules), reproduce the local frame grouping and sampling rule. Replaying identical normalized payloads can pass the first comparison and fail either of the other two. These are distinct acceptance cases for the [joint library plan](../design/EventProcessingSystems.md#evidence-for-an-open-network).

<a id="prior-work-and-evaluation"></a>

## Relation to prior work

Jones gives a type system in which constructors can be abstracted over and constrained by
operations they support. The endpoint example uses that distinction twice, once to drive a
producer into the selected sending end and once to obtain a local producer from the receiving end.
Which obligations travel in a constructor index and which require separate witnesses also arises in
the [fuel-sale timeline](#a-reflex-style-network-for-a-fuel-sale).
[Jones, *A System of Constructor Classes*, FPCA 1993](https://web.cecs.pdx.edu/~mpj/pubs/fpca93.pdf)

Lewis, Shields, Meijer and Launchbury showed how a statically typed implicit parameter can carry a
caller's choice through intermediate functions. CoD's installed procedure raises a further question
for a pipeline whose processing outlasts its caller. The choice must leave evidence in an owned
record or destination before the call ends. The explicit version makes the choice visible at every
forwarding interface. The scoped version keeps the adapter interface independent of deployment and
moves responsibility for selecting it to the installation sites.
[Lewis et al., *Implicit Parameters: Dynamic Scoping with Static Types*, POPL 2000](https://launchbury.blog/wp-content/uploads/2019/01/implicit-paramters.pdf)

Tucker, Maier, Sheard and Fegaras use punctuation to express that a described part of a stream
has ended, allowing an operator to emit a result or release state. Akidau and colleagues analyze
how stream processors communicate and estimate event-time progress. The strict promise used for
the two-sensor monitor supplies a precise boundary for irrevocable publication. A deployment
using estimated watermarks would have to specify what happens to late readings and when an earlier
alarm may be corrected. Selecting a different CoD ingress binding cannot settle those temporal
questions.
[Tucker et al., *Exploiting Punctuation Semantics in Continuous Data Streams*, TKDE 2003](https://dsf.berkeley.edu/cs286/papers/punctuations-tkde2003.pdf)
· [Akidau et al., *Watermarks in Stream Processing Systems*, PVLDB 2021](https://www.vldb.org/pvldb/vol14/p3135-begoli.pdf)

