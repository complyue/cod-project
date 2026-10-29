# Steady recurrent SNN: adaptive connection capacity

Document-set revision: 202 · Recorded: 2026-10-28 · Topic: SNN-07.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. No qualified implementation baseline exists.
- **Value lineage:** C-13/P-26 concentrates bounded connection storage where learned context needs it. P-25/P-22 connect this application document through C-12 to C-01 and V-01/V-02. Capacity dependencies do not approve learning or migration mechanisms.
- **Invariants / non-goals:** fixed pools, a current regional set reference per participating neuron, one owner per set, and separate four-byte endpoint/one-byte weight arrays. Capacity-only migration preserves effective connections, semantic cell identity and state, with no heap topology growth. Source-owned sets are the reference; target-owned incoming sets remain a proposal. The 100-slot minimum does not turn low outgoing participation into an incoming-degree omission rule. SoA is the default; power-of-two bit widths, including FP4, are standard; other widths need explicit alignment analysis. Auxiliary storage needs a demonstrated requirement. Final class count, quotas and migration protocol remain open.
- **Acceptance:** [Decision and measurement requirements](#acceptance) covers index/ownership consistency, fixed storage, preserved behavior and measured learning/migration costs. Representation-only movement differs from topology or weight changes.
- **Human baseline:** H-13—H-19 supply scoped storage constraints; H-21/H-25 retain provisional geometry; H-24 leaves weight scale with training. H-27/H-35 limit current sizing to Flash 60+60; H-28 shares the embedding table. H-29—H-34 govern layout and justified structures; H-38/H-39 specify a small set surplus without preserving older capacity. H-43 fixes semantic roles; H-61—H-64 define training divisions and progression concerns. Exact statements and exclusions remain in the registry.
- **Current assessment:** [K-202](../../../design/DesignAuthorityRegistry.md#k-202) condenses this summary while preserving the body and prior authority evidence. K-108's retained-prefix alternatives remain unselected. No mechanism or implementation qualification is added. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; [progress](../research/modeling-refinement-progress.json) records its scope and substantive work.

## Current scope

The current [alpha-near-3 geometry](ConnectivityScalingResearch.md#candidate-static-layout) uses the worked 15-class Flash 60+60 candidate: 614,400 neurons, 622,573 sets, 106,492,416 slots and 515.150806427001953125 MiB of named arrays. [IncomingConnectivity](IncomingConnectivity.md) evaluates target ownership for gather-based updates. The source-owned examples here explain H-19's outgoing participation constraint using that same population and class geometry; changing ownership does not change the byte counts or automatically transfer the omission rule to incoming degree.

## Fixed topology, variable assignment

Let `N=614400` be the neuron count and `M=622573` the current statically provisioned set count. H-38 sets the sizing objective `M>N` with a small surplus; the pool geometry should make scale-based capacities and indexes easy to calculate. Connection capacity follows from that sizing, with no requirement to preserve a previous scenario's total slots (H-39). Final class count and quotas remain open. No heap allocation maintains the pool's shape. Training changes values and assignments inside the fixed arrays.

Each participating source neuron points to one current set. A candidate representation gives an omitted source a null current-set reference. `source_of_set[set_id]` contains the source neuron ID for an assigned set, or a distinguished free value. In a stable state, participating sources and assigned sets correspond one-to-one; unused sets provide migration destinations. The owner map has M entries, while neuron links have N entries.

Each outgoing slot contains only a target ID and a weight, in separate arrays:

```text
neuron[i].current_set ---> &targets[p]
source_of_set[set_id]  = i
slot p                = (targets[p], weights[p])

targets[S] : uint32_t       4*S bytes
weights[S] : uint8_t        S bytes, FP8 bit representation
```

The source-owner array is per set, not an incoming-edge index. The current same-region reference budget is eight bytes per neuron. Regional objects and the arrays retain their locations throughout set reassignment; regional references preserve whole-region relocation.

## Minimum capacity and omitted outgoing participation

The current minimum set capacity is **100 slots** (H-19). Do not provision classes below 100. The user permits ignoring the outgoing contribution of neurons with too few connections; this is a modeling approximation that concentrates the connection budget, not a measured biological result.

A candidate omitted source has no assigned set and emits no deliveries through this outgoing pool. Its neuron ID, input eligibility and state remain represented; the effects on column-local selection and vector readout are not silently removed. The N-entry reference/state arrays therefore remain in the budget. Omitting an assignment does not release static pool memory.

Capacity and live degree are different: a 100-slot set can be partially occupied during learning. A rule that permanently rejects every source with fewer than 100 current edges would need a way to discover or initialize enough useful edges to participate. Initialization, admission, omission and re-entry rules remain open; ordinary capacity migration preserves retained connections, while deliberately suppressing a source changes model behavior.

Revisit the 100-slot minimum if underfilled minimum-size sets are both widespread and useful to the task. Measure occupancy and the task effect of including or omitting those sources; no numerical revision trigger has been selected.

## Current capacity classes

The worked candidate uses 15 classes; 16 and other counts are possible, as assessed in the [class-count comparison](ConnectivityScalingResearch.md#sizing-objective-and-class-count-options). **A is the set count in the smallest, 100-slot class**; it is currently 311,296. Capacity doubles every two classes while counts halve every class:

```text
A = 311296 = 19 * 2^14
j = 0..14
capacity(j) = (100 if j%2==0 else 144) << (j//2)
sets(j) = A >> j
M = 622573; S = 106492416
```

This gives 8,173 more sets than neurons and 19 sets at the maximum capacity of 12,800. Global spare capacity does not guarantee availability in the class a neuron needs. The [scaling research](ConnectivityScalingResearch.md#candidate-static-layout) gives all class counts and separates this engineering assumption from biological degree measurements.

## Direct address and inverse-index formulas

Parallel endpoint-ID and weight arrays share the same slot index. With the current class rule, let `e=j//2` and `t=j%2`:

```text
set_prefix(j) = 2*A - ((2*A) >> j)
slot_prefix(j) = 344*A - (((344 - 100*t)*A) >> e)
set_id = set_prefix(j) + k
p = slot_prefix(j) + k*capacity(j) + l
endpoint location = endpoint_base + 4*p bytes
weight location = weights_base + p bytes
```

The [inverse formulas](ConnectivityScalingResearch.md#formula-based-indexing) use A=19*2^14 and integer floor_log2. These are arithmetic address calculations, not stored prefix arrays or a prescribed free-set selection algorithm. The endpoint field holds targets in the source-owned example and sources in the target-owned proposal.

Endpoint IDs have four-byte alignment and weights use one-byte elements. A 100-slot set occupies 400 ID bytes and 100 weight bytes. The capacity sequence preserves the required alignment without rounding 100 to 128 or padding every slot; there is no five-byte struct stride.

## Available sets and migration

The owner array and current-set references are part of the specified representation. Free-set selection and reservation semantics remain open. Auxiliary structures are considered only after clear business requirements lead to an implementation-algorithm discussion; no such structure or extra memory allowance is prescribed here.

At an agreed update boundary, a candidate migration protocol is:

1. Obtain a free preallocated set of sufficient capacity; the selection algorithm remains unspecified.
2. Transfer this neuron's retained target IDs and weights to the destination, preserving any selected learning state.
3. Switch its regional current-set reference and publish the destination owner consistently.
4. Once no readers or queued work use the old set, mark it free again.

Transferring d retained connections copies **5*d payload bytes**, implying about **10*d bytes of source-read plus destination-write traffic**, before other fields and cache effects. Unlike a forced pair exchange, another neuron's connections need not move. This is not zero-copy migration: the destination must receive the retained connections. The [retained-prefix comparison](IncomingConnectivity.md#identifying-retained-inputs-without-preset-bookkeeping) examines an endpoint end marker and a separate count; neither is selected. Migration must establish the destination's chosen end representation before publishing it. An underfilled destination needs one additional four-byte marker write in that candidate, beyond the copied pairs and reference/owner updates.

The extra destination storage is already part of M/S. Migration adds no new set or heap allocation. A full requested class can still require deferral, a larger available class or a separately defined exchange policy; class reservations and learning admission remain design choices.

Bits values transfer between existing arrays; the neuron and set storage themselves do not move. Self-relative references are assigned using their normal protocol. Whole-region relocation stays zero-cost, while individual connection transfer has the cost above. See the [regional type reference](../../../design/MemoryRegionAndRegionalTypes.md).

Preservation includes the interpretation of stored weights. If a code value 0.5 is decoded with scale 2 before migration and scale 4 afterward, keeping the bits changes its effective weight from 1 to 2. A destination capacity class therefore cannot silently redefine weight scale or neural gain. H-24 keeps scaling within training policy. This condition does not select a scale field or recoding method; any such representation needs its own semantic and precision justification.

## Learning and HTM context

H-63 unifies initial DMN/token chain construction with ongoing repair of missing expected next-step activation. H-61 retains later Hebbian flow and immersion without dedicated training; H-62's fixed groups remain a preliminary reference option. [TrainingStrategy](TrainingStrategy.md) separates a repair's missing contribution from existing input and each neuron's total retained degree. Strengthening existing edges, adding absent edges and revisiting shared edges have different slot costs; repair does not imply rebuilding a complete transition or allocating a private pool per token. Update ordering, capacity admission and interference assessment remain open.

H-64's [global planning question](TrainingStrategy.md#global-constraints-and-stepwise-sampling) adds future eligibility and shortcut concerns. Enough total free slots or neurons does not ensure that later semantic stages retain usable candidates. Current-step sampling should be assessed against those future needs and the existing graph. Target-owned incoming lists, if adopted, permit reverse reachability traversal from source groups; this does not make full-graph checks cheap or authorize a visited array, outgoing index or reservation structure. Training scratch requirements need an explicit algorithm and budget.

H-43 fixes the semantic roles within which construction and learning operate. H-61 accepts omitting physical-distance preference for now; dimension-index distance and cross-minicolumn internal-position alignment remain excluded. Capacity-set addresses must not change semantic assignment. Capacity degree distribution and numerical weight scale remain separate choices.

H-24 assigns weight-scale policy to training strategy, including possible hyperparameters or dynamic planning/adaptation. The allocator manages fixed capacity classes, ownership and migration; it does not choose numerical weight scaling. Training may change demand for those classes without changing the statically provisioned topology.

The [input-count research](SynapticIntegrationResearch.md) evaluates branch integration and activity control under H-22. The allocated class size must not set excitability: moving unchanged live connections into a larger reserved set should preserve neural behavior. Changes to live degree, grouping or adaptive gain require their own learning and publication semantics.

Weight changes, connection growth/pruning and set reassignment are distinct operations. A large weight does not itself demand more slots; a small or zero weight is not automatically disposable. The criterion for allocating more capacity, the discovery of useful absent edges and the training rule remain open. [RigL](https://proceedings.mlr.press/v119/evci20a.html) provides a precedent for changing sparse topology during training, not an adopted SNN rule.

An endpoint slot addresses a neuron, not a dendritic segment. HTM's distinction between common feedforward input and different temporal contexts remains useful background. Any segment-level behavior first needs clear business semantics; its algorithm and representation would be discussed from those requirements. Source-owned outgoing sets are the H-16 reference; target-owned incoming sets are the current gather proposal under evaluation.

The separate [flow-learning design](FlowLearning.md#preceding-source-credit-and-sparse-growth) distinguishes source credit, existing-edge adaptation and missing-edge discovery. Its [capacity comparison](IncomingConnectivity.md#static-capacity-feasibility) concerns the distribution of retained incoming degrees, not just total edges. Wrong contextual assignment or insufficient selectivity must not be labeled capacity exhaustion merely because a prediction failed. No auxiliary topology, usage counters or capacity changes are prescribed by that distinction.

Live assignment changes do not automatically produce DBMR history. If old weights/topology are retained, history has its own representation and compaction budget; a mutable set undergoing reassignment cannot also serve as an immutable historical snapshot.

## Capacity example and peak accounting

The [exact static-array calculation](CapacityPlanning.md#exact-static-arrays) counts M owner IDs and N neuron links/states. The current endpoint IDs occupy 425,969,664 bytes and weights 106,492,416 bytes. Including references, owners and packed FP4 states gives **540,174,772 bytes = 515.150806427001953125 MiB**. These figures are exact for the stated arrays, not measured process RSS.

<a id="rare-high-fan-out"></a>
### Rare high fan-out

The largest class has 19 sets of 12,800 slots. Each set has 62.5 KiB of endpoint/weight payload; the entire class has 1,216,000 bytes, already included in the connection arrays. Under incoming ownership this limits the largest incoming sets, not the number of outgoing hubs. Not every such set must be assigned to a currently high-degree neuron. The geometric distribution intentionally reserves capacity in every class; its efficiency and class occupancy need measurement once the model exists.

Source-event propagation depends on activity-weighted outgoing degree. In the incoming-frame proposal, retained incoming edges determine the gather work even when few sources are active; visits to unused capacity depend on the traversal representation. Neither firing rate nor Pi throughput has been established.

<a id="acceptance"></a>
## Decision and measurement requirements

Fixed directions are static topology, arithmetic indexing as a priority, a small surplus of sets over neurons, a 100-slot minimum with smaller classes omitted, permission to omit weakly connected outgoing sources, a current-set reference per neuron, one source-owner entry per set, parallel target/weight arrays and no heap allocation for connection management. The exact geometric rule, total count, free-set search, migration publication, retained-connection representation, source admission/re-entry and learning policy remain proposals.

Eventual validation should establish direct/inverse index agreement across class boundaries, aligned array access, owner/link consistency, fixed storage during migration and preserved target/weight/state semantics. Account for all storage justified by the selected algorithms, and measure class occupancy, the usefulness of underfilled minimum sets, migration bytes and useful runtime throughput.
