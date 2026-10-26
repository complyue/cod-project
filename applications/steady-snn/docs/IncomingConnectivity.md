# Steady recurrent SNN: target-owned incoming connection sets

Document-set revision: 24 · Recorded: 2026-10-26 · Topic: SNN-07 · Proposal under discussion.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 24, all sections here; `pending-human`.
- **Value lineage:** candidate P-26 derives C-13 bounded adaptive connection storage from application C-12; P-25/P-22 connect D-08/C-12 to C-01 and V-01/V-02. Explicit edge ownership and update ordering make the same application transition inspectable across execution environments.
- **Invariants / non-goals:** fixed pools, parallel four-byte endpoint IDs and one-byte weights, per-neuron regional references, per-set owner IDs, regional relocation and lifetime rules. No heap growth, selected learning rule, complete HTM model or measured CPU performance follows from this proposal. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** preserve the intended directed edges and weights; account for incoming class demand, update ordering, packed-state writes, migration and any routing index. Evaluate CPU cost and useful task behavior before implementation.
- **Human baseline / continuity:** H-13–H-19 supply the storage constraints. [H-21](../../../design/DesignAuthorityRegistry.md#h-21) provisionally retains the alpha-near-3 template and requests evaluation of incoming ownership. It does not approve all execution details. K-11 records this draft; no qualified application implementation baseline exists. H-22 requests anatomical and physiological evidence about incoming degree and excitability; K-12 records this assessment without selecting an integration or activity-control mechanism. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.

## Recommendation and physical layout

Use target-owned incoming sets as the preferred proposal for a synchronous, gather-based simulation. A target owns its incoming weights; each slot identifies the presynaptic source. This changes how a directed edge is stored, not its direction or meaning.

| Item | Source-owned reference | Target-owned proposal |
| --- | --- | --- |
| Neuron's current-set reference | Its outgoing set | Its incoming set |
| Four-byte slot array | Target neuron IDs | Source neuron IDs |
| One-byte slot array | FP8 weight bits | FP8 weight bits |
| Four-byte set-owner array | Source neuron ID | Target neuron ID |
| Capacity class constrains | Outgoing degree | Incoming degree |

For an assigned set, its owner ID supplies the common endpoint. The opposite endpoint is stored once per slot. The target-owned representation therefore still uses **five payload bytes per reserved slot**, in two parallel arrays. No target ID is added to individual slots. A single intra-CC neuron ID identifies each endpoint; CC-to-CC vector messages remain a separate interface.

The current regional reference can point into the fixed source-ID array. The [provisional geometry and inverse formulas](ConnectivityScalingResearch.md#candidate-static-layout) still locate the corresponding weight range and set owner. Reassignment uses existing sets and preserves neuron identity. The reference is not a persistent raw process address.

## Compatibility with the capacity law

The provisionally selected template has capacities `100,144,200,288,...,12800`, with counts halving each class. Capacity doubles and count quarters across every pair of classes. Its logarithmic-bin interpretation remains degree-density exponent alpha near 3, whether the counted degree is incoming or outgoing.

The relevant biological comparison becomes **in-degree**. Yadav, Shinde and Singh report adult-fly tail exponents **3.15 incoming** and **2.96 outgoing**. This supports retaining approximately 3 as a coarse engineering prior for the incoming proposal; it does not establish a universal human law. With ideal sqrt(2) spacing, alpha=3.15 implies a successive-bin count ratio about 0.475, versus 0.5 at alpha=3. The convenient integer template remains a provisional application choice. [Primary study](https://doi.org/10.1162/NETN.a.26).

For the same directed intra-CC graph, `sum(in_degree) = sum(out_degree) = E`. The individual distributions need not match. Regrouping existing edges by target preserves both distributions, but an allocation table that fits the outgoing distribution need not have enough large sets for the incoming distribution. Total free capacity alone cannot guarantee a fit.

An incoming power-law capacity template also does not generate an outgoing power law. Choosing sources uniformly can yield a relatively concentrated outgoing distribution even with highly unequal incoming capacities. If the model needs influential outgoing hubs, source selection and learning must express that separately; no rule is selected here.

The 19 sets with 12,800 slots would accommodate high **incoming** degree. A source may still influence 10,000 targets by appearing in 10,000 different targets' sets. Its outgoing degree is not capped by the capacity of its own incoming set, nor is the number of high-outgoing-degree neurons limited to 19 by this table.

Incoming and outgoing degree parameters are assessed separately in the [population comparison](ConnectivityScalingResearch.md#incoming-and-outgoing-degree-parameters). Its class-count ratios are calculated from fitted exponents and chosen spacing, not measured allocation quotas. H-23 accepts a compensating incoming weight scale as a reasonable model direction; H-24 places that policy within training strategy, potentially as a hyperparameter or dynamically planned/adapted quantity. It is not a capacity-allocation parameter and does not determine the degree exponent. The alpha=3 allocation remains provisional; the alpha=3.15 table is a sensitivity comparison.

## Gather-based updates

The [input-count research](SynapticIntegrationResearch.md) distinguishes storage from excitability. A global sum with comparable positive weights and a fixed threshold can overactivate high-degree neurons. Target ownership also permits per-branch integration; choosing the neural transition, input timing, inhibition and activity control is separate from arranging its arrays.

An execution sketch for the point-neuron alternative, without selecting it as the biological model, is:

```text
drive_i(t) = external_i(t)
             + sum(weight[p] * signal(state_t[source[p]]), p in incoming(i))
state_next[i] = transition(state_t[i], drive_i(t))
```

Each target's source IDs and weights are traversed consecutively. Its accumulator can stay local to the worker, with one final state write. Assigning each target to one worker avoids concurrent scatter updates to that target. Targets with 100 and 12,800 inputs require different amounts of work; scheduling should account for retained edges, not just neuron count.

All workers read the same prior-state snapshot. Publishing next state at the step boundary avoids accidental dependence on traversal order. Packed FP4 requires an additional detail: two neurons share a byte, so write partitions must respect byte/word boundaries or use a separate packing phase. Exclusive ownership of a neuron alone does not prevent packed writes from racing.

Source-state reads are indirect and may be scattered. FP4/FP8 decoding, accumulation precision and the transition remain explicit costs and semantic choices. An immutable snapshot does not alone guarantee bit-identical floating-point reductions across implementations; summation order and precision must also be defined if required.

This gather structure has a simulation precedent in postsynaptic summed variables: Brian describes summing synaptic contributions for each neuron at each time step. Its separate presynaptic event mechanism illustrates the distinct execution alternative. This is a semantic reference, not a proposed dependency. [Brian documentation](https://brian2.readthedocs.io/en/stable/user/synapses.html#summed-variables).

Target ownership can also group local learning updates and future dendritic segments around the receiving neuron. It does not by itself specify segment boundaries, timing traces or HTM coincidence detection; a simple weighted sum is only the execution sketch above.

## Memory and event-driven trade-off

For the worked 15-class Flash 60+60 candidate, incoming ownership preserves the [540,174,772-byte array budget](ConnectivityScalingResearch.md#per-cc-array-payload) at N=614400, M=622573 and S=106492416:

```text
4*S source IDs + S weights + 8*N references + 4*M target owners + N/2 state
= 540174772 bytes = 515.150806427001953125 MiB
```

This includes reserved sets and slots. Region headers and algorithm-dependent storage are outside the named-array sum; no auxiliary structures or storage allowances are prescribed.

An incoming sweep must process the retained edges needed by the chosen update semantics, even when few sources fire. The traversal algorithm determines whether unused slots are also visited; the representation of retained versus unused slots is not selected here. At the current pool's reserved-slot ceiling, a complete ID/weight-array scan covers 532,462,080 bytes; multiplying that by a step rate is workload arithmetic, not a throughput result.

Incoming ownership alone does not provide efficient enumeration of a firing source's targets. Establish whether the business workload needs event-driven propagation before discussing its implementation algorithm or auxiliary structures. No routing representation or additional routing memory is preset.

## Minimum capacity and participation

The minimum physical set can remain 100 slots. Its new meaning would be **100 incoming slots**, potentially underfilled. H-19's permission to omit low-outgoing-degree influence cannot silently become permission to omit low-incoming-degree neurons: a neuron with few inputs may have a large outgoing influence.

An unassigned incoming set can represent no retained internal inputs; it does not imply that the neuron is absent, receives no external drive, or cannot be a source in other sets. Incoming admission, output suppression and learning-driven re-entry need separate semantics. If the outgoing omission policy is retained, first define the business criterion for suppressing influence; discuss how to evaluate it when choosing the algorithm, without preset tracking structures.

## Decisions still open

- Adopt target ownership and choose synchronous gather updates, event-driven propagation, or a defined combination. Gather is the current recommendation.
- Define incoming-set admission and useful underfilling separately from outgoing influence and suppression.
- Specify point-neuron versus branch/segment integration, input timing, inhibition and gain/threshold regulation; compare degree-dependent background activation and task selectivity. These are open under H-22. Define precision, step boundaries, learning and migration publication, and budget any additional state.
- Validate per-class incoming demand, outgoing hub formation, migration traffic and CPU bandwidth under an explicit task and update rate.

These questions affect D-08's model, capacity and migration scope. They introduce no change to CoD's language contract or regional pointer semantics.
