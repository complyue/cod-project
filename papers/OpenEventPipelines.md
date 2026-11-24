# Open event pipelines with dynamically scoped event sinks

## Abstract

A pump monitor sees a high reading from sensor A in production and one from sensor B in a trial.
Pooling the readings would raise an alarm at a pump where neither monitor saw a pair. Both
deployments use the same adapter code, event type and sensor names. Can that code select its sink
from the caller's dynamic scope and still give the same answers as an adapter passed its destination
explicitly, after the caller has returned? We identify what the queue must preserve about each
submission and what the sources must establish before a worker can finalize a window. Two histories
with identical untagged records but opposite correct answers establish a limit on late destination
selection. A finite model compares the two routing disciplines over 360 legal input orders. It also
shows how often a wrong route escapes detection if the observer checks only the final alarms.

<a id="where-asynchrony-enters"></a>

## Which decision can a submission leave to its worker?

An independently invoked adapter can publish a reading and return before another invocation
supplies the reading that completes the pair. The monitor owns the partial match across both calls.
This is an *open* pipeline in the sense used here. A closed traversal hands each offer to a consumer
while its traversal is still active. A queue and a later worker turn on one event loop suffice to
expose the difference. [Closed traversals](ComposingEnumerativeChoice.md)

<a id="a-dynamically-scoped-event-sink"></a>

The operation under study is ingress. Consider two versions of the same adapter. In the first, an
installation site passes a destination to the adapter and any named helpers on the path to its
final submit. In the second, a named helper submits through a typed procedure symbol. Here `Input`
is an application record made entirely of bits. It can represent a reading or a source-progress
record. The owner of the symbol can supply this ingress definition for a live deployment. The named
application helper copies an admitted record into the live monitor's queue.

```cod
symbol @ingress:proc(r:Input)->void;

proc submit(r:Input) -> void {
    behave @ingress(r)
}

proc live_turn(r:Input) -> void {
    effect @ingress(r:Input) { offer_live(r); };
    submit(r)
}
```

A trial scope can install the corresponding trial definition for the same symbol. `behave` selects
the active definition and uses current dynamic bindings. Scope exit restores the previous
installation. The call to `submit` finishes synchronously in either deployment. The queue owner
can subsequently retain the copied record, and the installation can retire. A worker drains the
queue and calls the correlator with its own state. Alert output has a separate destination. The
installed procedure and the adapter's live activation are unavailable as queue elements under CoD's
callable and lifetime rules. [Procedure installation and restoration](../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)
· [Scoped handler lifetime](../design/CallableStorageAndHereAccess.md#scoped-replay-without-a-first-class-handler)

**Claim.** Replace explicit destination passing along the synchronous adapter call path by a scoped
ingress operation. For executions with matching route selection at the installation sites, this
replacement preserves each deployment's admitted records, drops and finalized alarms provided that:

1. Every data or progress submission resolves its destination before admission. The accepted copy
   retains that decision until consumption, either by membership in a destination-owned queue or
   through an immutable route identity in a shared queue. The destination remains live throughout
   its pending work.
2. The explicit and scoped versions use the same admission rule, initial state and schedule of
   submissions and drain opportunities. The record includes the same source identity at both
   interfaces. All nested installations restore the prior route on exit.
3. Each input source makes progress promises within its own deployment. For that source and
   deployment, an admitted progress record follows every admitted reading that it covers.
   The worker uses the same deterministic transition for each consumed record.

The first condition fixes where to record a decision. The third fixes what a progress record may
assert. They need independent checks. A properly typed `Input` can still be sent to the wrong
perfectly well-typed monitor. A correctly routed progress record can still overtake an accepted
reading.

A direct argument explains the claimed equivalence. Couple the explicit destination at each
installation site to the corresponding stack entry in the scoped program. Entering and exiting a
nested trial changes both choices in the same way. An ingress call then selects the same queue and
applies the same admission rule to the same record. A drain uses only queue contents and the
worker's state, so equal histories remain equal at the next drain. These are all the transitions in
the model. The obligations for a compiler or library implementation are the choice actually made
at a call, the restoration on all exits and the absence of any later dependency on that installation.
An implementation can share the underlying queue code between the two interfaces.

<a id="a-correlation-with-an-observable-answer"></a>

## Two histories that a worker cannot distinguish without a route

Use one pump, two isolated deployments named *live* and *trial*, and two known sensors A and B in
each. The pump and sensor names are intentionally identical across deployments. A high A sample
at event time 8 and a high B sample at time 6 qualify for the same half-open window [0, 10).
Each source has a monotone watermark. In this experiment a watermark of 10 is a binding promise
that the source has finished its on-time contributions earlier than 10 in that deployment. The
serial worker finalizes the window when both sources reach 10. It emits once if both reported
high, then releases the window state. With the final-only policy, the second high reading alone
cannot publish an alarm. The eligibility rule uses a set of distinct sensors, so a redelivery of
the same high reading within this window cannot supply the other sensor.

| Submission in order | Split history | Pair history |
| --- | --- | --- |
| A reports high at 8 | live | live |
| B reports high at 6 | trial, inside a nested trial installation | live |
| A advances to 10 | live | live |
| B advances to 10 | live | live |
| A advances to 10 | trial | trial |
| B advances to 10 | trial | trial |
| Final alarms | none | one live alarm |

Both histories permit the worker to wait until every ingress frame has retired. Erasing the
destination makes their pending input identical, down to the order of their data and controls.
A deterministic worker in the same initial state and with the same current bindings must then
produce the same output for both. At least one result must be wrong. Separate owned queues carry
the distinguishing information through queue membership alone. A shared log can instead store the
resolved route with each entry. A deployment-owned source identity can also encode the distinction,
provided its mapping stays valid for the lifetime of queued work. For these two histories, the retained state must distinguish at least two alternatives. Queue
identity can supply that distinction without adding any bytes to the submitted record.

There is a second way to lose the alarm. Keep every destination correct in the pair history but
allow the two live watermarks to bypass the pending live readings on a priority control path. The
worker legitimately finalizes an empty window given the order it sees. Delivering the two readings
one turn later cannot repair an irrevocable final-only result. Fixing this error requires preserving
the relation between progress and earlier accepted readings on the way to the worker.

<a id="prior-work-and-evaluation"></a>

## A bounded comparison

The executable model below varies the interleaving of four input lanes, one for each
(deployment, sensor). In each history two lanes contain a high reading followed by their own
watermark, and two contain only a watermark. This yields `6! / (2! × 2!) = 180` legal orders
per history. The experiment has room for all six submissions and drains after the last one.
The explicit adapter queues its argument's destination, while the scoped adapter resolves the
currently installed route at each synchronous call. A trial call executes inside a nested
installation and returns to live. After admission the worker has no access to the adapter.

| Model and observation | Split, 180 orders | Pair, 180 orders |
| --- | ---: | ---: |
| Explicit and scoped ingress disagree on an admitted record and its route | 0 | 0 |
| Correct scoped ingress gives the wrong alarms or fails to finalize both deployments | 0 | 0 |
| Choose the drain-time route (live) for every record. Wrong final alarm set | 120 | 60 |
| Deliver all watermarks before all readings, keeping their routes. Wrong final alarm set | 0 | 180 |

The last two rows are deliberate negative controls that violate the claim's premises. The third
row corrupts the input history in **every** order in both columns. In 60 split orders it
nevertheless emits the expected empty alarm set. In 120 pair orders it happens to emit the expected
live alarm. The fourth row similarly passes every split alarm check. A test that compares just
the public alarms systematically misses routing and progress errors. The per-record admission log,
which deployments finalized, and the resulting alarms give separate observations. The counts are
exact for the finite model and were obtained by running the appendix. They establish fixture
sensitivity and the model result. Native CoD execution, saturation and timing need separate evidence.

### What a native reproduction must hold fixed

Run the explicit and scoped adapters against one instrumented queue implementation and the same
application worker. For each compiler revision, record coverage of the binding, restoration and
lifetime cases. Derive routing expectations from the declared installations and the input schedule.
Record the offered source, selected destination, admission decision, dequeue, window finalization,
alert and state release with a logical step number. A mismatch for any matched schedule refutes
the implementation's equivalence for that input, even if its alarms match. Record actual time
separately if measuring latency. Vary batch size and drain position without violating any
source's order. A changed trace of per-route admissions or final alarms, after accounting for the
specified interleaving and admission decisions, identifies a semantic difference. Different wall
times under the same logical schedule identify an implementation cost.

Three further traces exercise obligations the appendix leaves open.

* **Pressure.** Give the queue one slot. Admit live A's high reading, then offer live B's high
  reading before a drain. Specify an ingress policy that drops this incoming reading and records
  the drop. Hold a full-queue progress submission at its source until it can be admitted, and
  prevent later submissions on that same lane from overtaking it. Drain and advance both sources
  to 10. Expect one B data drop, no alarm and release of the live window state. In the paired
  control, drain after A and admit B before the progress records. Expect one live alarm. The void
  submit operation reports neither observation to the adapter. The ingress owner supplies the log.
* **Scope and lifetime.** Route a valid submission to trial, then take an exceptional exit from
  the nested scope. Catch at live and submit a second record. Drain only after both frames end.
  Expect one record per intended destination. An ordinary call path reaching `behave @ingress`
  without an installation is a separate checker-negative fixture. Attempting to enqueue the
  installed handler as a callable is another. Those fixtures ask whether a path is admissible.
  They cannot diagnose a live but wrongly selected destination of the same type.
  [Call-path binding checks](../design/SymbolicDynamicScope.md#call-path-binding-checks)
* **Event-time boundary.** Replay the live pair twice with identical submission and drain steps.
  Move B's timestamp from 6 to exactly 10 on the second replay. After both watermarks reach 10,
  expect the first replay to produce a live alarm and the second to close [0, 10) without one.
  B now belongs to [10, 20), which remains open. A processing-time implementation that consults
  only dequeue order cannot satisfy both observations.

Event timestamps, watermark values and queue dwell time have different units of meaning here.
The timestamp selects a window. A route-specific progress assertion permits finalization.
Elapsed processing time measures delay and conveys no independent evidence of logical completion.
If B stays silent in one deployment, that window can remain open indefinitely. A finite queue
therefore gives no general bound on retained correlation state over many keys or windows. An idle
source policy, replay after process failure, and acceptance of events behind an already finalized
frontier each change the application's contract. The small model makes no claim about those policies.

An inter-Actor version adds a real transport obligation. Each CoD Actor has its own dynamic context.
Sending through a statically typed chan is normally fire-and-forget. A systemic fault that makes
later sends likely to fail must be reported. An individual send gives the adapter no evidence of
peer admission. An application needing the strict frontier used above must arrange a per-source
prefix protocol spanning admission and receipt, or postpone finalization while a prefix remains
unknown. An ordinary progress message that happens to arrive cannot establish this property by
itself. The paper's one-event-loop fixture makes it possible to study the ingress question without
assuming a stronger chan service. [Actor and chan contract](../design/SymbolicDynamicScope.md#actor-concurrency-direction)

## Relation to prior work

Lewis, Shields, Meijer and Launchbury give statically typed implicit parameters a dynamic
binding discipline and show how a caller-supplied parameter can travel through intermediate
definitions. Their pretty-printer controls a choice deep inside one computation. The added issue
here is which result of that choice must survive into separately scheduled work. CoD's
definition-installed procedure supplies a particular source-language boundary at which to test it.
[Lewis et al., *Implicit Parameters: Dynamic Scoping with Static Types*, POPL 2000](https://launchbury.blog/wp-content/uploads/2019/01/implicit-paramters.pdf)

Tucker, Maier, Sheard and Fegaras treat punctuation as a promise that a described part of a stream
has ended and use it to justify emitting results or releasing state. Akidau and colleagues analyze
how stream processors communicate event-time progress and the practical limits of watermark
estimation. Our final-only oracle deliberately uses a strict, per-source promise over the records
admitted to a particular destination. Applying an estimated watermark would require an explicit
late-data and correction policy before these same alarm checks could be used.
[Tucker et al., *Exploiting Punctuation Semantics in Continuous Data Streams*, TKDE 2003](https://dsf.berkeley.edu/cs286/papers/punctuations-tkde2003.pdf)
· [Akidau et al., *Watermarks in Stream Processing Systems*, PVLDB 2021](https://www.vldb.org/pvldb/vol14/p3135-begoli.pdf)

The strongest comparison supported here concerns when a route choice must take effect. A programmer
can make that choice correctly through either interface. The explicit version exposes the route at
each forwarding boundary. The scoped version can leave those signatures alone, at the price of a
binding obligation and a need to inspect installation sites when explaining a route. Code-change
scope across real adapter libraries, diagnostic quality and runtime costs would decide whether
that trade pays for a particular deployment.

## Executable finite model

Run this block with Python 3. `H` denotes the sole high reading of its lane at the event time above.
`W` advances that lane to 10. The model retains only the state needed for one pump and one window.
High readings at a finalized destination cannot change its published result. The three output
counts per case are legal orders, wrong alarms with drain-time routing, and wrong alarms with
watermarks first.

<details>
<summary>Python 3 enumeration and alarm oracle</summary>


```python
def interleave(lanes):
    if not any(lanes):
        yield ()
    for i, lane in enumerate(lanes):
        if lane:
            rest = lanes[:i] + (lane[1:],) + lanes[i + 1:]
            for tail in interleave(rest):
                yield (lane[0],) + tail


def ingest(order, dynamic):
    queue, bindings = [], ["live"]

    def adapter(sensor, kind):
        queue.append((bindings[-1], sensor, kind))

    for route, sensor, kind in order:
        if not dynamic:
            queue.append((route, sensor, kind))
        elif route == "trial":
            bindings.append("trial")
            try:
                adapter(sensor, kind)
            finally:
                bindings.pop()
        else:
            adapter(sensor, kind)
    return queue


def finalize(queue):
    marks = {r: set() for r in ("live", "trial")}
    highs = {r: set() for r in marks}
    closed, alerts = set(), set()
    for r, s, kind in queue:
        if kind == "H":
            if r not in closed:
                highs[r].add(s)
        else:
            marks[r].add(s)
            if len(marks[r]) == 2 and r not in closed:
                if len(highs[r]) == 2:
                    alerts.add(r)
                closed.add(r)
                highs[r].clear()
    return alerts, closed


for case, b_route in (("split", "trial"), ("pair", "live")):
    lanes = tuple(
        tuple((r, s, k) for k in
              ("HW" if (r, s) in {("live", "A"), (b_route, "B")} else "W"))
        for r in ("live", "trial") for s in ("A", "B")
    )
    expected = {"live"} if case == "pair" else set()
    total = late_route = early_mark = 0
    for order in interleave(lanes):
        explicit, bound = ingest(order, False), ingest(order, True)
        assert explicit == bound == list(order)
        assert finalize(bound) == (expected, {"live", "trial"})
        wrong = [("live", s, k) for _, s, k in bound]
        first = sorted(bound, key=lambda row: row[2] != "W")
        late_route += finalize(wrong)[0] != expected
        early_mark += finalize(first)[0] != expected
        total += 1
    print(case, total, late_route, early_mark)
```

</details>

