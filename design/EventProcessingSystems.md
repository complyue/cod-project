# Event Processing Systems

**CoD library research plan · WIP r2**  
**Planned repository** `e-wrks/eps.cod`

A booking application can examine seats in order, stop on the first match and release its inventory cursors. An auditor can process the same seats after the booking call returns. Both need typed event processing. They need different answers to “who can stop the source?” This plan brings the [closed event pipeline](../papers/ClosedEventPipelines.md#where-a-closed-event-pipeline-ends) and [open event network](../papers/EventEffectPropagation.md#connecting-two-closed-drives) studies into one CoD library, **Event Processing Systems** (EPS).

## Design authority

The [project direction](../CHANGELOG.md#h-eps-library-direction-20261126) establishes the two research tracks, their common basis in the [retention working paper](../papers/ComposingAcrossLifetimes.md), and one intended library repository. [D-EPS-RESEARCH](DesignAuthorityRegistry.md#d-eps-research) records the original scope. Revision 2 carries it forward with an [open-sink composition case](../papers/EventEffectPropagation.md#from-a-pump-alarm-to-an-inspection-request) for independently authored monitoring and maintenance rules. The proposed interfaces, workload order and evaluation criteria below are research choices. The relevant language contracts cover [bounded callbacks](CallableStorageAndHereAccess.md#callable-value-kinds), [typed pipeline composition](ProducersAndTransformers.md#lexical-variables-for-open-pipelines), [regional ownership](RegionalOwnership.md#owner-bearing-types-are-intrinsic) and [independent chan ends](ActorChannels.md#independent-chan-endpoints). EPS applies them to reusable business operations whose caller, source or deployment can change. The fare, alarm and receipt traces are paper derivations. They provide expected observations for a future implementation.

## What the two papers contribute

| Question | Closed event pipeline | Open event network |
| --- | --- | --- |
| Who acts next? | A source, stage or terminal in the live drive. A normal handoff returns the declared feedback. | Components may have independent drivers. A receiver can have an empty turn. A local reactive host determines the next frame. |
| What does composition check? | Offered type, labels, access, owner, normal reply and the terminal's local result or failure. | Each local composition checks those same obligations. Joining components also requires compatible send and receive roles, an admissible payload and the chosen delivery or timeline contract. |
| What survives? | A prepared policy can outlive its builder in a regional ADT. Each drive supplies its own valid control destinations and traversal-local state. | Admitted work owns or validly acquires its payload. The selected destination, worker state and any event-time progress survive as long as the application's observations require. |
| What can a caller observe? | Which offers appeared, which replies returned, which cursor closed and what reached the caller. | Local submission, receiver observations in subsequent turns and local frame results. A remote acknowledgment, if required, is an application event. |

A pipeline can have two open *ports* while an author is composing it, then run as a closed drive once attached. A deployed network can run indefinitely even when each component processes its current frame synchronously. These descriptions concern different boundaries. They allow EPS to support an in-memory traversal, an independently drained queue and an Actor deployment without forcing one feedback contract onto all three.

## Candidate library surface

Start with a middle that a domain author can prepare and reuse. Its left and right ports describe complete protocols. For two adjacent stages, the first stage's output must satisfy the second's input, including the response due on a normal handoff. At drive time the application supplies the missing endpoints. The following relationships are paper notation. The names expose which information the proposed API must preserve.

```text
compose     (stage P → Q, stage Q → R)                  → stage P → R
drive       (source for P, stage P → R, terminal for R) → local result
sender      (sending end for M, local admission policy) → local terminal
ready       (receiving end for M, turn budget)          → local source
```

The terminal can use a local admission policy to decide whether to continue its own producer. The paired receiving end may run under a different owner, with a different item protocol. A later receiver decision can reach a submitter through an explicit application protocol with its own message, identity and lifetime. The types of the four operations above supply no implicit path for it.

For a local composition such as `fares_for() |> affordable(100u64)`, a named `here` connection suffices while its dependencies are live. For a policy kept in a catalog, put the selected data and typed edges in an actual owner G. The ADT's call operator tracks G and can be a `proc` when it uses a dynamic service. At invocation the method receives fresh callbacks, a source and access to the current run's state. To specify a configurable rule, say whether a field snapshots a value, tracks a particular receiver, or selects a provider at invocation. For example, changing a live tariff can affect future drives. Editing that tariff must leave an already quoted scalar unchanged. Reentry must give a per-traversal counter a fresh initial value. A per-site window persists across turns under the site's owner. These are different reasons to retain state. [Binding-time experiment](../papers/ComposingAcrossLifetimes.md#one-mutation-three-answers) · [Developer guide](../docs/BuildingReusableDomainOperations.md#keep-a-rule-after-the-builder-returns)

Concrete, monomorphic domain types are sufficient for the first comparisons. The papers also investigate naming families of producers, transformers, terminals, independent send/receive endpoints and signals indexed by timeline. A generalized spelling should demonstrate that it accepts the independently authored examples and rejects a fare stage with the wrong reply, a sending end used as a receiver, and an unbridged signal from another timeline. Publication of a plan as DBMR is relevant when an application needs independent reopening. The first in-process workload can use ordinary owned data.

## First library workload

Use one seat adapter with A1 = 120, A2 = 80, A3 = 80, B1 = 90 and a quoted cap of 100. Its payload includes seat identity. Begin with a direct nested loop as a reference. Compare it with a prepared two-stage search driven by the same source and terminal. Use a destination-owned local queue that copies accepted records. Hold its consumer idle until the source has finished.

| Drive | Local obligation | Available for a later drain |
| --- | --- | --- |
| Append every match and always continue | Rejection of A1 supplies true. The source reaches both routes. | A2, A3, B1 in order |
| Append, give false once at A2, then give true | Only A's seat cursor stops at A2. B still opens. | A2, B1 |
| Append A2 and return its price from the caller | A2 gets no normal reply. Close A then the route cursor. Return 80. | A2 |

Repeat the last case across an Actor boundary using the permitted whole-region payload transfer. Pause delivery long enough for an empty receiver turn. Then deliver A2 and run the worker again. Compare sender and receiver logs at their own observation points. The local queue fixture guarantees an append on success. A successful ordinary chan send supplies no reception guarantee. A second schedule with no delivery must therefore admit the same sender-side completion. Exercise resource failure and reentry separately, so neither can pass unnoticed behind an identical list of prices.

## Evidence for an open network

| Case | What the application must preserve | Observation that separates the candidates |
| --- | --- | --- |
| Pump alarm | Site selected on admission, site-owned window, per-source progress after the accepted readings it covers | Split A and B across live and trial and both stay quiet. Pair them at live and live emits one alert. Let progress overtake the readings and the alert can disappear even with correct destinations. |
| Independently authored alarm and maintenance rules | The alarm sink's typed domain protocol and the site, pump and window selected by the monitor | With the same qualifying window, an alarm recorder observes one alarm. Connecting the maintenance rule also offers one inspection request. The request recorder observes the offer under the local fixture's successful calls. |
| Fuel-sale receipt | Timeline identity, frame membership, price immediately before start, final meter increment in the close frame | Publish one local receipt for 510 cents. Choosing the newly posted price gives 540. Omitting the final litre gives 340. |
| Console switch | Which dispenser is selected in a frame where both complete a sale | Switching after the frame reports sale 17. Switching in the frame reports sale 29. |
| Live and replay ingress | A recording at the observation being compared | Equal decoded frames can reproduce normalized input. Reproducing an alarm also needs site and progress order. Reproducing a receipt also needs the host's frame decisions. |

These examples suggest local traversal and admission adapters, a bounded receiver drive, and timeline-specific sampling and switching operations as candidate library areas. Endpoint pairing, drop or retry policy, progress and late data, and same-frame selection belong to the adapters or applications that promise them. An application that needs settlement after a sale must define how receipt identities, duplicates and reconciliation work. [Window histories](../papers/EventEffectPropagation.md#a-worker-that-can-distinguish-the-histories) · [Sale and switch histories](../papers/EventEffectPropagation.md#a-receipt-that-distinguishes-the-sampling-rules)

## Path to the library repository

1. **Establish the closed reference.** Bring the route/seat source, filter, first-match and audit callers together with a trace collector. Compare direct, scoped and region-owned forms with the same binding contract. Include duplicate seats, early exit, checked failure, cleanup and reentry. The developer should be able to add the second terminal without editing the price predicate.
2. **Connect independent drives.** Give the same domain stages a local queue adapter and independently held Actor endpoints. Record local submission and receiver turns separately. Use the delayed audit to test ownership after the sender returns, then run the split/pair pump histories to test routing and progress. A replay adapter should state which observations its recording can reconstruct.
3. **Exercise a local timeline.** Give the fuel-sale example a host that admits explicit frames. Run both sampling policies and both switch cutovers. Compare locally published receipts and console output before adding a transport to a till. This shows which operations need a timeline index or a separate host contract.
4. **Extract the public CoD library.** Bring the supported operations, examples, adapters and their contract checks into `e-wrks/eps.cod`. Share an operation where the earlier workloads agree on its types, ownership and observations. Keep protocol-specific operations visible where the workloads return different answers. Record frontend support alongside the source examples and report the declarations that actually change when a new source or terminal is added.

The first design decision after those comparisons is how much of the typed middle can have one public signature across the supported workloads. A one-shot visit, for example, cannot supply the retained head or explicit resumability a bounded-lookahead merge needs merely by receiving a family name. A failed generalization should identify the missing operation and the workload that needs it. Public module names, a general type encoding, implementation timing and distribution remain open in this plan. [Merge counterexample](../papers/ClosedEventPipelines.md#distinguishing-the-two-candidates)
