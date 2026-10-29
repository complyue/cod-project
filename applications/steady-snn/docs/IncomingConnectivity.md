# Steady recurrent SNN: target-owned incoming connection sets

Document-set revision: 202 · Recorded: 2026-10-28 · Topic: SNN-07 · Proposal under discussion.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** C-13/P-26 serves bounded adaptive storage; C-29/P-69 gives incoming gather an order-independent frame transition. P-70 is its storage dependency, not implementation authority. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** fixed pools, parallel four-byte endpoint IDs and one-byte weights, per-neuron regional references, per-set owner IDs, regional relocation and lifetime rules. No heap growth, selected learning rule, complete HTM model or measured CPU performance follows from this proposal. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** preserve the intended directed edges and weights; account for incoming class demand, update ordering, packed-state writes, migration and any routing index. Evaluate CPU cost and useful task behavior before implementation.
- **Human baseline:** H-13—H-39 supply the scoped static pool, layout, SoA and sizing constraints; H-21 requests incoming ownership evaluation. H-75 proposes two frames; H-73 allows wider state; H-76/H-77 specify input/cancellation. Exact layout and execution remain unqualified. The linked registry retains exact evidence and exclusions.
- **Current assessment:** [K-108](../../../design/DesignAuthorityRegistry.md#k-108) derives retained-prefix comparison candidates from gather, learning and migration requirements. Counts and endpoint markers remain unselected; no auxiliary allowance enters the current budget. This is an agent assessment, not human confirmation or qualification. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; [progress](../research/modeling-refinement-progress.json) records its scope and substantive work.

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

In the 15-class candidate, the 19 sets with 12,800 slots would accommodate high **incoming** degree. A source may still influence 10,000 targets by appearing in 10,000 different targets' sets. Its outgoing degree is not capped by the capacity of its own incoming set, nor is the number of high-outgoing-degree neurons limited to 19 by this table.

Incoming and outgoing degree parameters are assessed separately in the [population comparison](ConnectivityScalingResearch.md#incoming-and-outgoing-degree-parameters). Its class-count ratios are calculated from fitted exponents and chosen spacing, not measured allocation quotas. H-23 accepts a compensating incoming weight scale as a reasonable model direction; H-24 places that policy within training strategy, potentially as a hyperparameter or dynamically planned/adapted quantity. It is not a capacity-allocation parameter and does not determine the degree exponent. The alpha=3 allocation remains provisional; the alpha=3.15 table is a sensitivity comparison.

### Static capacity feasibility

For one assigned set per participating target, let `d_i` be retained incoming degrees and `c_i` the available set capacities, each sorted in descending order. With interchangeable sets and no other placement restrictions, a static assignment exists exactly when there are enough sets and `d_i <= c_i` at every demand index. The necessity follows because the largest `i` demands need `i` sufficiently large sets; matching in sorted order supplies the sufficiency. Equivalently, at every degree threshold the number of targets requiring at least that degree cannot exceed the number of sufficiently large sets. This is a feasibility statement, not a prescribed runtime sort, histogram or migration algorithm.

The [pool arithmetic](../research/flow-slot-capacity-analysis.json) gives 311,277 sets above 100 slots for the 15-class candidate and 327,670 for the 16-class candidate. If all 614,400 neurons participate, at least 303,123 or 286,730, respectively, must need no more than 100 inputs. Total slot count does not remove this constraint. Neurons requiring no assigned incoming set still retain their other model roles; this is not a new omission rule.

Static feasibility does not guarantee that a suitable destination is currently free during a particular online migration. Final demand, current assignments and permitted publication order are separate questions. The existing extra sets help provide destinations, but no reservation or exchange machinery is inferred. [Flow learning](TrainingStrategy.md#capacity-demand-depends-on-contextual-assignment) must measure distinct retained sources and class demand after contextual assignment; a fixed distribution of storage capacities is not evidence for the learned degree distribution.

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

H-75 proposes implementing this semantic update with **two alternating state frames**. Read only the completed frame while filling the other, then exchange their roles. The incoming sum is a temporary local quantity, not a persistent per-edge accumulator. H-73 permits FP16/FP32 state, avoiding packed-state write concerns for those candidates. [RateDynamics](RateDynamics.md#two-alternating-frames-with-incoming-gather) records consistent polarity cancellation, one-shot input and publication boundaries. H-76 consults preceding predictive state for token recruitment; H-77 specifies minicolumn-wide net cancellation and proportional retention before publication. No third whole-CC frame, duplicate connection pool, outgoing index or global inter-CC barrier is implied.

Source-state reads are indirect and may be scattered. FP4/FP8 decoding, accumulation precision and the transition remain explicit costs and semantic choices. An immutable snapshot does not alone guarantee bit-identical floating-point reductions across implementations; summation order and precision must also be defined if required.

This gather structure has a simulation precedent in postsynaptic summed variables: Brian describes summing synaptic contributions for each neuron at each time step. Its separate presynaptic event mechanism illustrates the distinct execution alternative. This is a semantic reference, not a proposed dependency. [Brian documentation](https://brian2.readthedocs.io/en/stable/user/synapses.html#summed-variables).

Target ownership can also group local learning updates and future dendritic segments around the receiving neuron. It does not by itself specify segment boundaries, timing traces or HTM coincidence detection; a simple weighted sum is only the execution sketch above.

## Identifying retained inputs without preset bookkeeping

Gather, adaptation, growth and migration must distinguish retained edges from unused capacity. This is an actual requirement of the proposed algorithms; it does not yet justify a particular auxiliary array. In particular, a retained association can have zero weight and remain eligible for future learning. Using zero weight as an empty-slot marker would silently choose a topology-removal policy.

A simple candidate keeps retained source/weight pairs in a consecutive prefix of the assigned set. Two ways to identify its end can be compared:

| Candidate | End detection | Additional persistent storage | Potential value to establish |
| --- | --- | ---: | --- |
| Invalid endpoint at the first unused position, plus the class capacity bound | Stop at that endpoint; a full set stops at its capacity | None | Gather already discovers the end, which a colocated learning operation can reuse temporarily |
| Separate `uint16` retained count per neuron | Read the count before traversing the prefix | 1,228,800 bytes per CC | Repeated end queries or execution improvements may justify retaining the count |

At 614,400 neurons, the four-byte intra-CC endpoint namespace has values outside the valid neuron-ID range; no particular marker code is chosen. The marker occupies an already unused endpoint element, so a full set does not lose a usable slot. For the count candidate, both current maximum capacities—12,800 and 18,432—fit 16 bits. It is a separate SoA candidate field, not a reinstated bookkeeping allowance or part of the current named-array budget. Neither representation resolves source discovery, duplicate checking, free-set selection or the learning criterion for removal.

Let `E` be the number of retained edges, `N_a` the number of assigned targets and `U` the number of their underfilled sets. The [conditional traversal arithmetic](../research/incoming-retained-prefix-analysis.json) gives `5*E + 4*U` logical endpoint/weight/stop-check bytes for the marker loop, or `5*E + 2*N_a` endpoint/weight/count bytes for the count loop. This counts the specified reads once, not cache-line traffic or complete frame cost. Source-state gathers, references, decoding, arithmetic, branches and writes remain additional.

For a feasible example with 32 retained inputs on each of 614,400 targets, those totals are 100,761,600 and 99,532,800 bytes per frame. Both are far below the 16-class pool's 561,408,000-byte full reserved-array scan; neither is a throughput result. At 100 retained inputs per target, assigning all 327,680 minimum-capacity sets makes them full, so marker reads are unnecessary for those rows. The respective totals become 308,346,880 and 308,428,800 bytes. Even the small logical-byte difference between the candidates depends on occupancy; measured execution, not that difference alone, should decide whether the extra count is useful.

Growth establishes new pairs and the new end before publication. Migration copies the retained prefix and establishes its destination end while preserving endpoint identities and effective weights. If a later justified removal algorithm fills a hole with the last pair, the mathematical edge multiset is preserved but floating-point reduction order changes. A required numerical tolerance or reproducibility property must guide that choice; no pruning, sorting or deterministic-reduction scheme is implied.

These operations can be discussed at the existing complete-frame boundary. No heap allocation, free list, reverse index, new connection pool or branch-membership layout is prescribed. Compare the same retained graph and update schedule, favoring a count only when its actual algorithmic or measured execution value pays for the additional state.

## Memory and event-driven trade-off

For the worked 15-class Flash 60+60 candidate, incoming ownership preserves the [540,174,772-byte array budget](ConnectivityScalingResearch.md#per-cc-array-payload) at N=614400, M=622573 and S=106492416:

```text
4*S source IDs + S weights + 8*N references + 4*M target owners + N/2 state
= 540174772 bytes = 515.150806427001953125 MiB
```

This includes reserved sets and slots. Region headers and algorithm-dependent storage are outside the named-array sum; no auxiliary structures or storage allowances are prescribed.

The formula above assumes one FP4 state array. For H-75's two-frame proposal, replacing only `N/2` by `2*N*sizeof(state_value)` gives these exact conditional named-array totals:

| Candidate pool | Two FP16 state frames | Two FP32 state frames |
| --- | ---: | ---: |
| 15 classes | 542325172 bytes / 517.201588 MiB | 544782772 bytes / 519.545338 MiB |
| 16 classes | 571402200 bytes / 544.931602 MiB | 573859800 bytes / 547.275352 MiB |

Each case has one scalar state value per neuron per frame, unchanged one-byte weights and the existing single ID/reference/owner arrays. These are not complete layouts or a selected precision. The node-shared embedding table is excluded from every per-CC total.

An incoming sweep must process the retained edges needed by the chosen update semantics, even when few sources fire. The traversal algorithm determines whether unused slots are also visited; the representation of retained versus unused slots is not selected here. At the current pool's reserved-slot ceiling, a complete ID/weight-array scan covers 532,462,080 bytes; multiplying that by a step rate is workload arithmetic, not a throughput result.

Incoming ownership alone does not provide efficient enumeration of a firing source's targets. Establish whether the business workload needs event-driven propagation before discussing its implementation algorithm or auxiliary structures. No routing representation or additional routing memory is preset.

## Minimum capacity and participation

The minimum physical set can remain 100 slots. Its new meaning would be **100 incoming slots**, potentially underfilled. H-19's permission to omit low-outgoing-degree influence cannot silently become permission to omit low-incoming-degree neurons: a neuron with few inputs may have a large outgoing influence.

An unassigned incoming set can represent no retained internal inputs; it does not imply that the neuron is absent, receives no external drive, or cannot be a source in other sets. Incoming admission, output suppression and learning-driven re-entry need separate semantics. If the outgoing omission policy is retained, first define the business criterion for suppressing influence; discuss how to evaluate it when choosing the algorithm, without preset tracking structures.

## Decisions still open

- Finalize H-75's two-frame incoming-gather proposal and its state/transition semantics. Event-driven propagation would need a demonstrated unmet requirement before adding its supporting structures.
- Define incoming-set admission and useful underfilling separately from outgoing influence and suppression.
- Specify point-neuron versus branch/segment integration, input timing, inhibition and gain/threshold regulation; compare degree-dependent background activation and task selectivity. These are open under H-22. Define precision, step boundaries, learning and migration publication, and budget any additional state.
- Validate per-class incoming demand, outgoing hub formation, migration traffic and CPU bandwidth under an explicit task and update rate.

These questions affect D-08's model, capacity and migration scope. They introduce no change to CoD's language contract or regional pointer semantics.
