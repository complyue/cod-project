# Open Event Pipelines with Dynamically Scoped Event Sinks

## Abstract

Two sensors report vibration on one pump. Their adapters run at different times. The application wants one alert when both sensors report a high reading in the same event-time window. A call stack can account for the submission of each reading, but neither adapter owns the lifetime of the correlation. We examine an asynchronous producer built from a typed symbolic procedure, an explicitly owned inbox and a worker that installs its own processing context. An issuer calls `@push` under a live binding. What survives that call is the accepted event. This gives a concrete question for complex event processing: can dynamically selected ingress preserve the same routing, lifetime and admission behavior as explicitly passing each adapter its destination? We specify a bounded reference experiment, including a trace that would expose premature window closure. Queueing, delivery and recovery policies here belong to the experiment.

## Where asynchrony enters

A synchronous generator may use many inputs for one output or expand one input into many. This changes the rate of offers within one traversal. We call a pipeline *open* when independent invocations can contribute inputs to processing whose lifetime exceeds any one submission. A later worker turn or a time frontier can trigger output without a suspended upstream caller. One event loop suffices to demonstrate this difference. Parallel execution is an additional engineering choice. [Companion paper on closed traversals](ComposingEnumerativeChoice.md)

## A dynamically scoped event sink

A **dynamically scoped event sink** is a typed symbolic operation through which a producer submits an event to the implementation installed for that dynamic extent. In `symbol @push:proc(evt:Event)->void;`, the symbol identity and event type are known statically. The installation selects the destination. One producer can use an inline correlator during replay, a recording sink in a test, or an inbox when delivery must outlive ingress. At each call, `behave` selects the installed definition and uses the current dynamic bindings. Changing the installation preserves the type contract. Its delivery, ordering and failure semantics still have to suit the application. [Definition-installed procedures](../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)

The following two turns use the same symbol. `Event` is a pure-bits reading in this example. The named helpers stand for application operations with an explicitly established, live inbox and correlation state.

```cod
symbol @push:proc(evt:Event)->void;

proc submit(evt:Event) {
    behave @push(evt);
}

proc ingress_turn(evt:Event) {
    effect @push(evt:Event) { offer_or_count_drop(evt); };
    submit(evt);
}

proc worker_turn() {
    effect @push(evt:Event) { correlate(evt); };
    drain_batch();  # For each removed event, this named helper calls submit(evt).
}
```

The symbolic call runs synchronously. In the ingress turn, the installed body tries to transfer the event's bits into a bounded inbox that outlives the turn. In the worker turn, a different installation delivers the event to the correlator. Even if the ingress installation has retired, an accepted event can be processed. Queueing the installation itself, a here callable, or a reference into the finished ingress frame would violate CoD's lifetime rules. A payload containing a graph needs an explicit transfer into sufficiently long-lived storage or a supported ownership handoff. Worker entry establishes its own bindings, including on another thread. [Scoped handler access](../design/CallableStorageAndHereAccess.md#scoped-replay-without-a-first-class-handler)

Routing also matters within a live scope. If `correlate` calls `submit` to publish an alert, the worker's still-active `@push` leads straight back to `correlate` for a compatible event. In this example alerts use a distinct `@alert` symbol. A producer that temporarily installs a test recorder for `@push` shadows the enclosing sink only during that installation. A useful test submits one reading before the recorder is installed, one inside it, and one after it retires. Only the middle reading belongs in the recording.

A successful submission return means the configured ingress policy has run. In this void example, a full inbox drops the incoming event and increments a counter. A caller that must distinguish acceptance from rejection needs a different contract, such as a returned admission result. A failure during later correlation belongs to the worker's execution. A submitting caller's `catch` cannot receive it after that caller has returned. Acknowledgments or retries need a return path identified by event or submission identity.

## A correlation with an observable answer

Give each sensor an identity and each reading a per-sensor sequence number, pump key and event timestamp. For this experiment, one serial worker groups accepted readings into ten-second event-time windows. It emits one alert per pump and window when both distinct sensors reported high vibration. Duplicate sequence numbers contribute at most once within a window. Each sensor supplies a monotone watermark promising that subsequent on-time readings have timestamps at or above that watermark. Both input streams are known to the correlator, and each stream's watermark follows its earlier readings. The smaller watermark closes windows whose end it has reached. A missing watermark keeps the window open.

| Worker input for pump P | Expected observation |
| --- | --- |
| Sensor A reports high at 8. Its watermark advances to 12. | Keep A's contribution to [0, 10). B can still contribute. |
| B reports high at 6, repeats the same sequence number, then advances its watermark to 9. | The pair qualifies. Emit nothing yet under this final-only policy. |
| B advances its watermark to 10. | Emit exactly one alert for P and retire the window's state. |
| A reading with timestamp 7 arrives after closure. | Count it as late. The finalized alert stays as published. |

The worker may drain in batches, but changing the batch size must preserve this trace for the given input order. Transporting the two watermarks reliably and in order matters as much as transporting the readings. For the experiment, control records use an ordered per-source barrier that retries admission under pressure. An ingress data drop counts as loss and cannot be repaired by a watermark. Replay after process failure, wall-clock timeouts for idle inputs and durable exactly-once delivery would need additional policies. Even a bounded inbox can feed an unbounded set of open keys or windows if a watermark stalls.

## Prior work and evaluation

SEDA connects event-driven stages with queues and explicitly studies batching and overload. Reactive Streams specifies downstream demand as a means of bounding asynchronous delivery. The Dataflow Model distinguishes event time, processing time and output triggers. Ahman and Pretnar give asynchronous effects a formal treatment through signals and interrupts. These are independent comparisons for stage scheduling, flow control, temporal results and effect semantics. [Welsh et al., *SEDA*](https://www.cs.cmu.edu/afs/cs/academic/class/15712-f08/www/readings/Welsh01.pdf) · [Reactive Streams specification](https://github.com/reactive-streams/reactive-streams-jvm) · [Akidau et al., *The Dataflow Model*](https://research.google.com/pubs/archive/43864.pdf) · [Ahman and Pretnar, *Asynchronous Effects*](https://arxiv.org/abs/2003.02110)

Run the same adapters, inbox, worker and policy twice. Give adapters an explicit destination in one run and establish it by symbolic installation in the other. Compare admissions, drops, alerts, late counts and cleanup under bursts, two interleaved pump deployments, nested installation and retirement of the original ingress frame. Deliberately omit the worker binding and attempt to enqueue a borrowed ingress-local payload as negative cases. Measure submit latency, queue occupancy, time to a finalized alert and retained window state separately. A result in favor of symbolic routing would show where the checker rules out a misrouted or dangling submission, or where it removes destination plumbing while preserving equivalent checks. Measurements and a proof of routing under the stated bindings remain future work.
