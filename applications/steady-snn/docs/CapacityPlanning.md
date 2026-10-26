# Steady recurrent SNN: capacity planning

Document-set revision: 24 · Recorded: 2026-10-26 · Topic: SNN-04.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 24, all capacity calculations and proposals here; `pending-human`.
- **Value lineage:** P-25 → C-12 and candidate P-22 → C-01 → V-01/V-02. Explicit resource costs help choose application semantics that can execute on the intended hardware; a smaller footprint alone does not prove semantic equivalence. Candidate P-26 adds C-13 variable connection capacity.
- **Invariants / non-goals:** preserve the vector width unless separately decided; retain region/lifetime constraints. A per-node ceiling is not a per-CC allocation or a throughput guarantee. No benchmark, dependency, precision format or topology is selected by these calculations. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** see [Acceptance evidence](#acceptance); calculations are conditional planning estimates, not measured RSS or latency.
- **Human baseline / continuity:** H-09 requires capacity first and Raspberry Pi-sized CCs; H-10 directs attention to HTM; H-12 sets 16 GB per node. H-08 supplies the application. H-13 introduces unequal capacity and migration; H-14 requires static pools and current-set pointers with no heap allocation. H-15 considers rare outgoing degrees around 10,000 as a rare high-degree case. H-16/H-17 fix five payload bytes per slot, the source-owner array and parallel target/weight arrays. H-18 permits extra sets and prioritizes arithmetic indexing. H-19 fixes the current minimum at 100 slots, permits omitting weakly connected outgoing sources and specifies when to reconsider that minimum. K-03–K-13 record the pending revisions. Additional limits remain proposals. H-20 requests biological evidence. H-21 provisionally retains the alpha-near-3 template and asks to evaluate target-owned incoming sets; execution details remain proposals under K-11. H-22 requests anatomical and physiological evidence about incoming degree and excitability; K-12 records this assessment without selecting an integration or activity-control mechanism. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.

## Budget and exactness boundary

The node RAM ceiling is **16 GB**. H-27's current planning case uses DeepSeek-V4.1-Flash's 5,120-dimensional token embeddings and 60 positive plus 60 negative cells per dimension: **N=614400 neurons**. Model metadata and the boundary between token embeddings and contextual model computation are recorded in [Model](Model.md#snn-05).

H-38 makes the sizing objective explicit: total sets M should be slightly greater than neurons N, with simple static indexing. H-39 excludes preserving a previous scenario's reserved connection capacity. The provisional capacity/count geometry remains; final J and quotas are open. The [alpha-near-3 geometry](ConnectivityScalingResearch.md#candidate-static-layout), with the worked 15-class Flash 60+60 candidate, gives M=622573, S=106492416 and **540,174,772 bytes = 515.150806427001953125 MiB** for the named arrays. Target-owned incoming sets preserve this budget. [IncomingConnectivity](IncomingConnectivity.md#memory-and-event-driven-trade-off) records execution-specific additions. MiB means 2^20 bytes and GiB means 2^30 bytes; this is array sizing, not measured RSS.

## Flash 60+60 calculation

For the worked 15-class candidate, the provisional geometry is `c_j=(100 if j is even else 144)<<floor(j/2)`, `B_j=A>>j`, for j=0..14. Capacity doubles every two classes and set count halves every class. The minimum set is still **100 slots**, independently of the 60 cells in each polarity population; maximum capacity is **12,800**.

**A is the number of sets in the smallest, 100-slot class.** For J=15, the smallest exact-halving pool with M>N uses A=311296; it is not the total number of sets or slots. Each successive class has half as many sets:

```text
A = 311296 = 19 * 2^14
N = 614400
M = sum(A >> j, j=0..14) = 2*A - (A >> 14) = 622573
S = sum((A >> j)*c_j, j=0..14) = 106492416
```

This working value provides 8,173 surplus sets if all neurons have one and 173.3275 reserved slots per neuron. Divisibility by 2^14 makes all 15 class counts exact, ending with 19 largest sets. This is a worked candidate under the small-surplus objective, not a finalized quota; overall surplus does not guarantee availability in each class.

| Class | Capacity | Set count |
| ---: | ---: | ---: |
| 0 | 100 | 311,296 |
| 1 | 144 | 155,648 |
| 2 | 200 | 77,824 |
| 3 | 288 | 38,912 |
| 4 | 400 | 19,456 |
| 5 | 576 | 9,728 |
| 6 | 800 | 4,864 |
| 7 | 1,152 | 2,432 |
| 8 | 1,600 | 1,216 |
| 9 | 2,304 | 608 |
| 10 | 3,200 | 304 |
| 11 | 4,608 | 152 |
| 12 | 6,400 | 76 |
| 13 | 9,216 | 38 |
| 14 | 12,800 | 19 |

| Array | Bytes |
| --- | ---: |
| Source neuron IDs, four bytes per slot | 425,969,664 |
| FP8 weights, one byte per slot | 106,492,416 |
| Neuron-to-set references, eight bytes per neuron | 4,915,200 |
| Set-to-owner IDs, four bytes per set | 2,490,292 |
| Four-bit state, assuming two packed values per byte | 307,200 |
| **Named-array sum** | **540,174,772** |

Under the two-FP4-values-per-byte sizing assumption, the per-CC named-array payload is **540,174,772 bytes = 515.150806427001953125 MiB**. Source and owner IDs need four-byte alignment, the assumed eight-byte references need eight-byte alignment, and weight bytes and the assumed packed state byte array need only byte alignment. With an eight-aligned base, the listed order satisfies these requirements without inter-array padding: the reference array starts at offset 532,462,080, the owner array at 537,377,280, and packed state at 539,867,572. This demonstrates alignment for that byte-array representation without selecting a complete FP4 access/update policy. FP4 is a standard width under H-32 and does not trigger the nonstandard-width discussion requirement. Region headers, any selected allocation/page rounding, and additional metadata/state are separate from this array payload. Four-byte source IDs and the five-byte parallel-slot representation remain sufficient; no per-edge sign or reverse-index field is added. The 106,492,416 weight slots are reserved capacity, not a measured live parameter count.

The [class-count comparison](ConnectivityScalingResearch.md#sizing-objective-and-class-count-options) also calculates J=16 under the same objective: A=327680, M=655350, S=112281600 and **569,251,800 bytes = 542.88082122802734375 MiB**. Its 40,950 surplus sets are 6.665039% of N; the 15-class candidate has 8,173 surplus sets, or 1.330241%. Sixteen classes extend the maximum to 18,432 slots. Neither class count is mandatory; these are the closest counts above N under exact halving. Pool sizing happens before execution; the configured arrays remain static.

At fixed mean live degree, gather work scales with neuron count; actual runtime remains unmeasured. Visiting every reserved slot implies 532,462,080 bytes of logical source-ID/weight reads per update, before source states and learning.

A two-byte signed-coordinate message has **10,240 bytes = 10 KiB** of payload.

### Shared node budget

Under H-28, CCs on one compute node share the semantic-vector table. At vocabulary size 129,280, width 5,120 and two bytes/value, its payload is **1,323,827,200 bytes = 1.23291015625 GiB**, counted once per node using that table. It is excluded from the per-CC total above.

For k CCs using the worked 15-class candidate:

```text
node_bytes = k * 540174772
           + 1323827200                  # one shared embedding table
           + sum(extra_bytes_for_CC_i)
           + node_runtime_bytes
```

Per-CC extras include selected neural/learning state, private queues, histories and region metadata. Node runtime includes shared services and system overhead. These categories are budgeted separately under the 16 GB node ceiling; the shared table is never multiplied by k.

<a id="exact-static-arrays"></a>
## Exact static arrays

H-29 establishes **SoA as the default for multi-field data**. Field arrays follow their own element-alignment requirements. This applies to connection fields and any additional neuron/set metadata. An AoS exception must identify a specific access pattern and provide measurements demonstrating its cache benefit; a conjectured locality benefit or alignment alone is insufficient.

H-30 requires **a separate field-level packing and alignment discussion for nonstandard bit widths**. Under H-31/H-32, a standard width is **`8 * 2^n` bits, with integer n and a positive whole-number bit count**: 1, 2, 4, 8, 16, 32, 64, 128, and so on. Equivalently, standard widths are powers of two in bits. **FP4 is standard: `4 = 8 * 2^-1`.** Widths such as 3, 12, 24 and 48 bits are nonstandard; whole-byte widths such as 24 and 48 therefore still require separate discussion. That discussion should specify the packing unit, element addressing, byte/word boundary crossings, update granularity and any resulting padding for the affected field.

Width classification and storage/access layout are distinct. The classification does not prescribe an ABI alignment or imply native/atomic instruction support. SoA separates fields; it does not determine the storage unit or spacing within a field array. The current FP4 budget retains two values per byte (`ceil(N/2)` bytes); FP4 does not trigger H-30's nonstandard-width discussion requirement, and its standard classification adds no padding. The budget remains a representation assumption, without selecting a complete access/update protocol.

Let M be the set count and `S=sum(B_j*c_j)` total slots. Each participating neuron has one assigned set. The current sizing case provides more sets than N; participation and unassigned-reference semantics remain separate decisions. Unassigned sets can be free or temporarily reserved.

| Array | Representation | Bytes |
| --- | --- | ---: |
| Opposite-endpoint IDs | `uint32_t[S]` | `4*S` |
| Weights | `uint8_t[S]`, FP8 bits | `S` |
| Neuron → current set | Eight-byte regional reference on the current 64-bit target | `8*N` |
| Set → owner neuron | `uint32_t[M]` | `4*M` |
| Neuron state | Packed four-bit codes | `ceil(N/2)` |
| **Total specified arrays** | | **`5*S + 8*N + 4*M + ceil(N/2)`** |

No per-edge reverse index is included. The set-owner map has M entries; only neuron references and state arrays have N entries. Endpoint IDs and weights occupy separate global arrays divided into J capacity-class ranges. Capacity and set addresses follow formulas, without per-set offsets, headers or capacity fields.

## Incoming-degree sensitivity

H-25 keeps the capacity allocation provisional and moves the active discussion to signed populations. The alternatives below remain sensitivity references, not an immediate decision to reopen the chosen class geometry.

The [fixed-capacity comparison](ConnectivityScalingResearch.md#fixed-capacity-sensitivity-calculation) holds neuron count, total set count and capacities constant while changing the class-population exponent. Counts are engineering projections from a literature-fitted exponent and selected binning/normalization, not measured class quotas. At the current N=614400 and M=622573, the alpha=3.15 alternative has 99,973,624 slots and 10 largest sets; its named arrays use 507,580,812 bytes = 484.066783905029296875 MiB. Placing references and ID arrays before the byte arrays satisfies element alignment without inter-array padding. It remains an unadopted comparison; the current alpha=3 sizing case uses 515.150806427001953125 MiB. Weight-scale policy belongs to training strategy under H-24; any selected training state needs a separate budget.

## Direct indexing and named-array total

The [current geometry and formulas](ConnectivityScalingResearch.md#formula-based-indexing) derive class and set locations from A=311296 and the 100/144 capacity sequence. The two connection arrays occupy **532,462,080 bytes** in total. Adding neuron references, set owners and packed FP4 state gives **540,174,772 bytes = 515.150806427001953125 MiB** per CC. The current placement meets element-alignment requirements without inter-array padding; additional physical format choices remain unspecified.

## Principle: requirements before auxiliary structures

Clarify the business requirement before discussing its implementation algorithm. Consider auxiliary data structures only in that algorithm discussion, when their purpose and necessity are established. Do not preselect their representation or add speculative memory allowances during capacity planning. Once justified and selected, account for their semantics, lifetime and storage under the static-allocation, SoA and alignment rules. The current totals cover the specified connection, reference, owner and FP4 state arrays; algorithm-dependent storage is unspecified.

## Migration cost and static topology

With a suitable free destination, a neuron can transfer its retained d connections and release the old set after the ownership switch. The transferred payload is `5*d` bytes; source reads plus destination writes are approximately `10*d` bytes before other fields, initialization and cache effects. No second neuron's data needs to be exchanged just to obtain a destination. Migration still copies retained data and uses only preexisting sets.

Global surplus does not guarantee free capacity in every class. Admission/reservation policy remains necessary, but pool geometry and addresses stay fixed. The way an algorithm finds and reserves a destination is deferred until its business requirements are clear; no auxiliary structure is prescribed here.

## CPU, network and shared data

For the source-owned event-delivery reference, runtime cost depends on activity and retained outgoing degree, plus any scan of unused capacity. The incoming gather proposal instead traverses all retained inputs each step; sparse firing does not by itself avoid those reads. No Pi throughput has been measured. For context, 100 sources with 10,000 connections each firing 100 times/s generate 100 million edge deliveries/s; that is workload arithmetic only.

The shared Flash embedding table costs **1,323,827,200 bytes = 1.23291015625 GiB** at two bytes/value. A 5,120-coordinate two-byte vector has **10,240 payload bytes**; q remote recipients and f emissions/s require `10240*q*f` bytes/s before framing.

At 100 / 1,000 / 10,000 CCs the named per-CC arrays total about **50.31 / 503.08 / 5030.77 GiB**, before shared/runtime/history costs. Node placement must satisfy compute and bandwidth constraints as well as RAM.

<a id="acceptance"></a>
## Acceptance evidence

Before implementation, clarify the business semantics for migration, training and source admission/omission, then discuss the algorithms and any justified auxiliary structures and state. Choose J and class quotas against the small-surplus objective, and confirm the geometric distribution and complete storage budget against those requirements. The 100-slot minimum should be reconsidered if useful underfilled sets prove common; low degree alone is not measured evidence of negligible task impact. Eventual validation must establish:

1. Agreement between forward/inverse indexes at all class boundaries, the stated M/S counts, array byte lengths and alignment.
2. Owner/link consistency for participating and omitted sources and assigned, free and reserved sets, without hidden per-edge metadata or uncounted storage.
3. Preserved connection/state meaning and fixed storage throughout migration, with no heap or fresh regional allocation.
4. Measured migration bytes, class occupancy, useful learning and RAM/latency, including separately declared dynamics/history buffers.

Only integer arithmetic and layout reasoning support the current totals; no implementation or runtime benchmark has been run.
