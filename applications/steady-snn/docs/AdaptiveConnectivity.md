# Steady recurrent SNN: adaptive connection capacity

Document-set revision: 28 · Recorded: 2026-10-27 · Topic: SNN-07.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 28, all sections here; detailed mechanisms remain `pending-human`.
- **Value lineage:** H-13 introduces C-13, unequal connection capacity and training-driven migration. Candidate P-26 derives C-13 from application C-12: concentrate bounded storage where learned context needs connections. P-25/P-22 connect the document set and application to C-01 and V-01/V-02; those candidate paths are not qualified implementation authority.
- **Invariants / non-goals:** stable neuron identity, explicit segment semantics, bounded total storage and preservation of regional relocation/lifetimes. Capacity migration alone must preserve the learned graph and state. Connection sets are statically provisioned, and each participating source points to its current out-of-line set; no heap allocation serves connection growth or migration. H-16/H-17 specify one source-owner ID per set and separate 32-bit target / 8-bit weight arrays. H-18 permits extra sets and prioritizes formula-based indexing of the fixed pool. H-19 sets the minimum capacity at 100 and permits omitting weakly connected outgoing participation. Exact class counts, reassignment protocol and learning rules remain open. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** see [Decision and measurement requirements](#acceptance); topology changes and representation-only migration require different evidence.
- **Human baseline / continuity:** H-08–H-12 supply the application/resource constraints; [H-13](../../../design/DesignAuthorityRegistry.md#h-13) supplies adaptive capacity; [H-14](../../../design/DesignAuthorityRegistry.md#h-14) specifies static pools, a current-set pointer and no heap allocation. [H-15](../../../design/DesignAuthorityRegistry.md#h-15) asks for rare neurons with outgoing degree around 10,000 as a rare high-degree case. H-16/H-17 specify the minimal outgoing layout and parallel arrays. H-18 relaxes the exact set count to simplify indexing and migration. H-19 excludes smaller sets and makes later reduction conditional on evidence of widespread, useful underfilling. K-11 records the revision as a pending design, not a confirmed implementation contract. H-20 requests biological evidence. H-21 provisionally retains the alpha-near-3 template and asks to evaluate target-owned incoming sets; execution details remain proposals under K-11. H-22 requests anatomical and physiological evidence about incoming degree and excitability; K-12 records this assessment without selecting an integration or activity-control mechanism. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.
- **Current discussion:** H-43 / AD-037 fixes CC as the basic semantic presentation unit, with approximate replicas across CCs and statically assigned minicolumn/neuron semantic roles. H-41 dimension neutrality and H-42 rejection of internal-position bias remain. [K-33](../../../design/DesignAuthorityRegistry.md#k-33) records the reassessment; learned semantic reassignment is outside the current baseline.

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

Transferring d retained connections copies **5*d payload bytes**, implying about **10*d bytes of source-read plus destination-write traffic**, before other fields and cache effects. Unlike a forced pair exchange, another neuron's connections need not move. This is not zero-copy migration: the destination must receive the retained connections. How the migration algorithm identifies retained connections and handles unused slots remains unspecified.

The extra destination storage is already part of M/S. Migration adds no new set or heap allocation. A full requested class can still require deferral, a larger available class or a separately defined exchange policy; class reservations and learning admission remain design choices.

Bits values transfer between existing arrays; the neuron and set storage themselves do not move. Self-relative references are assigned using their normal protocol. Whole-region relocation stays zero-cost, while individual connection transfer has the cost above. See the [regional type reference](../../../design/MemoryRegionAndRegionalTypes.md).

## Learning and HTM context

H-40 retains coactivity as the connection-learning direction; H-43 fixes the semantic roles within which it operates. H-41 excludes dimension-index distance, and H-42 rejects internal-position alignment across minicolumns. [Semantic cortical mapping research](SemanticCorticalMappingResearch.md) evaluates topology for those assigned roles and approximate-replica CCs. No spatial partner-selection rule is active. Capacity-set addresses must not change semantic assignment or any eventual logical positions. Distance, capacity degree distribution and numerical weight scale remain separate choices.

H-24 assigns weight-scale policy to training strategy, including possible hyperparameters or dynamic planning/adaptation. The allocator manages fixed capacity classes, ownership and migration; it does not choose numerical weight scaling. Training may change demand for those classes without changing the statically provisioned topology.

The [input-count research](SynapticIntegrationResearch.md) evaluates branch integration and activity control under H-22. The allocated class size must not set excitability: moving unchanged live connections into a larger reserved set should preserve neural behavior. Changes to live degree, grouping or adaptive gain require their own learning and publication semantics.

Weight changes, connection growth/pruning and set reassignment are distinct operations. A large weight does not itself demand more slots; a small or zero weight is not automatically disposable. The criterion for allocating more capacity, the discovery of useful absent edges and the training rule remain open. [RigL](https://proceedings.mlr.press/v119/evci20a.html) provides a precedent for changing sparse topology during training, not an adopted SNN rule.

The specified slot layout addresses a target neuron, not a target dendritic segment. HTM's distinction between common feedforward input and different temporal contexts remains useful background. Any segment-level behavior first needs clear business semantics; its algorithm and representation would be discussed from those requirements. H-16 settles source-owned outgoing sets for the current design.

Live assignment changes do not automatically produce DBMR history. If old weights/topology are retained, history has its own representation and compaction budget; a mutable set undergoing reassignment cannot also serve as an immutable historical snapshot.

## Capacity example and peak accounting

The [exact static-array calculation](CapacityPlanning.md#exact-static-arrays) counts M owner IDs and N neuron links/states. The current endpoint IDs occupy 425,969,664 bytes and weights 106,492,416 bytes. Including references, owners and packed FP4 states gives **540,174,772 bytes = 515.150806427001953125 MiB**. These figures are exact for the stated arrays, not measured process RSS.

<a id="rare-high-fan-out"></a>
### Rare high fan-out

The largest class has 19 sets of 12,800 slots. Each set has 62.5 KiB of endpoint/weight payload; the entire class has 1,216,000 bytes, already included in the connection arrays. Under incoming ownership this limits the largest incoming sets, not the number of outgoing hubs. Not every such set must be assigned to a currently high-degree neuron. The geometric distribution intentionally reserves capacity in every class; its efficiency and class occupancy need measurement once the model exists.

CPU work still depends on the activity-weighted outgoing degree. The number of visited slots depends on the traversal algorithm, which remains unspecified. No firing rate or Pi throughput has been established.

<a id="acceptance"></a>
## Decision and measurement requirements

Fixed directions are static topology, arithmetic indexing as a priority, a small surplus of sets over neurons, a 100-slot minimum with smaller classes omitted, permission to omit weakly connected outgoing sources, a current-set reference per neuron, one source-owner entry per set, parallel target/weight arrays and no heap allocation for connection management. The exact geometric rule, total count, free-set search, migration publication, retained-connection representation, source admission/re-entry and learning policy remain proposals.

Eventual validation should establish direct/inverse index agreement across class boundaries, aligned array access, owner/link consistency, fixed storage during migration and preserved target/weight/state semantics. Account for all storage justified by the selected algorithms, and measure class occupancy, the usefulness of underfilled minimum sets, migration bytes and useful runtime throughput.
