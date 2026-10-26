# Steady recurrent SNN: evidence for connectivity scaling

Document-set revision: 24 · Document revision: 24.1 · Recorded: 2026-10-26 · Topic: SNN-07 · Research and provisional capacity choice.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document-set revision 24 with this document at revision 24.1, all sections; `pending-human`. K-28 records the detailed 16-class table and array breakdown; K-27 covers the small-surplus objective and 15/16-class calculations; K-25 clarifies Flash-only scope and A; K-24 supplies the requirements-before-auxiliary-structures principle; the baseline, constraints and acceptance below apply.
- **Value lineage:** candidate P-26 relates C-13 bounded adaptive connection storage to application C-12; P-25/P-22 connect D-08/C-12 to C-01 and V-01/V-02. Explicit statistical assumptions make resource choices and their semantic consequences reviewable.
- **Invariants / non-goals:** retain the 100-slot minimum, static pools, separate four-byte endpoint IDs and one-byte weights, per-set owner map, regional references and no heap growth. Endpoint roles are evaluated in IncomingConnectivity. No biological equivalence, empirical human exponent, learning benefit or implementation contract is asserted. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** see [Evidence required for selection](#acceptance). A literature prior and an arithmetic layout are distinct from a fitted application model.
- **Human baseline / continuity:** H-13–H-19 supply the capacity constraints; H-20 requests research. H-21 provisionally retains the alpha-near-3 template and asks to evaluate incoming ownership. K-11 records this pending design revision; the provisional choice does not qualify the whole application implementation contract. H-22 requests anatomical and physiological evidence about incoming degree and excitability; K-12 records this assessment without selecting an integration or activity-control mechanism. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.

## Finding and recommendation

The inspected human evidence does not establish a transferable, universal exponent for a cortical neuron's number of distinct outgoing targets. An actionable comparison is **degree-density exponent alpha near 3 versus a lognormal model**, with allocation granularity chosen independently. The numerical power-law lead comes from an adult-fruit-fly study; it is an animal analogue, not a human estimate. H-21 provisionally retains the alpha-near-3 integer template below. Keep the equal-slot reference and lognormal alternative available for comparison.

## What each number measures

| Quantity | Meaning for this application |
| --- | --- |
| k_out / k_in, unweighted degrees | Distinct targets / distinct sources. Either matches required slots under the corresponding ownership if one retained slot represents one source-target pair. Multiple contacts per pair require separate semantics. |
| Synapse multiplicity or edge weight | Number/strength of contacts between a particular pair; does not specify how many different targets need slots. |
| Node strength | Sum of incident edge weights or synapse counts; not unweighted outgoing degree. |
| r, capacity-class ratio | Engineering spacing between adjacent capacities. The reference uses r=2. |
| alpha, degree-density exponent | Parameter of a candidate distribution p(K) proportional to K^(-alpha). |
| beta, class-count exponent | If counts in equal logarithmic intervals scale as capacity^(-beta), then beta=alpha-1 under a continuous power-law interpretation. |

For logarithmic bins [c,r*c), integrating K^(-alpha) gives a count proportional to c^(1-alpha). Thus doubling capacity and halving counts corresponds to **beta=1, alpha=2**; doubling capacity and quartering counts corresponds to **beta=2, alpha=3**. A ratio of 1.4 is not a degree-density exponent of 1.4.

These relations interpret the interior scaling of a capacity-count template. They do not prove that live degrees occupy every slot, or supply the admission rule and mass at the 100-slot boundary. A minimum allocated capacity and a fitted statistical lower cutoff are different parameters.

## Primary evidence and transfer limits

Sources were checked on 2026-10-23. Publication dates below identify the actual studies, not search-engine crawl dates.

| Study | Relevant result | What it supports here |
| --- | --- | --- |
| Shapson-Coe et al., *Science*, 2024, [human H01 reconstruction](https://pmc.ncbi.nlm.nih.gov/articles/PMC11718559/) | A roughly 1 mm³ temporal-cortex sample contains about 150 million synapses; approximately 57,000 cells includes glia, not just neurons. The paper describes segmentation errors and rare strong multisynaptic pairs. | Human synaptic evidence, but a finite slab and incomplete reconstructions cannot supply a universal whole-axon outgoing-degree law. Pair strength cannot be substituted for target count. |
| Cirunay et al., *Physical Review Research* 7, 013134, 2025, [paper](https://journals.aps.org/prresearch/abstract/10.1103/PhysRevResearch.7.013134), [full author manuscript](https://arxiv.org/html/2407.17220v2) | Edge-weight tails have exponents around 3. H01 node strengths instead receive exponentially truncated power-law fits; other datasets favor lognormal or stretched-exponential fits. | The approximately 3 edge-weight exponent is not an outgoing-target exponent. Their H01 graph has 13,579 nodes and 76,004 edges, a selected network rather than the full tissue's synapse population. |
| Piazza et al., [bioRxiv preprint](https://www.biorxiv.org/content/10.1101/2025.02.27.640551v2), version 2 dated 2025-11-18 | The abstract reports eight mapped connectomes whose degree and strength distributions are better described by lognormal models than random or scale-free models. | A lognormal alternative deserves comparison. This remains a preprint; the [bioRxiv metadata](https://api.biorxiv.org/details/biorxiv/10.1101/2025.02.27.640551) lists no published replacement. Metadata/abstract and author code were inspected; the full revised manuscript was unavailable in this review. |
| Yadav, Shinde and Singh, *Network Neuroscience* 9(4), 1299–1322, 2025-11-20, [paper](https://doi.org/10.1162/NETN.a.26) | Analysis of adult Drosophila FlyWire v630 reports power-law out-degree alpha=2.96 and in-degree alpha=3.15. Larval degrees favor Weibull fits. The reported fitting uses an estimated lower cutoff and maximum likelihood. | Directly relevant observable for an alpha-near-3 comparison. Species, developmental stage, sample filtering and fitted range prevent treating 2.96 as a human constant or extending it automatically to all degrees. |
| Dorkenwald et al., *Nature* 634, 124–138, 2024-10-02, [FlyWire reconstruction](https://doi.org/10.1038/s41586-024-07558-y) | With a five-synapse connection threshold, intrinsic neurons have median outgoing degree 13; a CT1 neuron has 6,399 outgoing partners. | The long tail can be substantial, and threshold choice changes the counted graph. These fly numbers do not set the human minimum or determine this CC's quotas. |

The lognormal and power-law findings are not a settled universal consensus. They use different observables and dataset treatments. Treating either family as an application prior requires an explicit transfer assumption.

For the human portion of the comparative preprint, the author's [H01 degree-processing code](https://github.com/bdanubius/ComparativeConnectomics/blob/84fb26bf29111151fb47a44dfc71441560eebee7/Processing/4-Degree_Processing/H01_Degree_Processing.m) symmetrizes pairs rather than exporting a separate outgoing-degree column. Its [aggregate table](https://github.com/bdanubius/ComparativeConnectomics/blob/84fb26bf29111151fb47a44dfc71441560eebee7/ProcessedData/H01_104/H01_AggregatedData.csv) is a small H01 subset with missing degree entries. No human outgoing mu/sigma or alpha was fitted from it here. These observations limit reuse of these files; they are not a reanalysis or rejection of the paper.

## Lognormal comparison

A candidate lognormal uses ln(K) distributed as Normal(mu,sigma²), with application-specific admission and upper-capacity handling. Its local density slope is alpha_eff(K)=1+(ln(K)-mu)/sigma², so there is no single fixed exponent over the whole range.

Choose mu from the desired typical degree and sigma from the spread and tolerated high-degree population, then assess them against data. No numerical human mu/sigma has been established here. Integrate the selected distribution over explicit degree intervals to obtain per-class populations; define how underfilled 100-slot sets and omitted sources enter that calculation. If this alternative is pursued, discuss its addressing algorithm after establishing the business requirements; no auxiliary representation is prescribed here.

<a id="candidate-static-layout"></a>
## Provisional static layout for alpha near 3

The current case uses **5,120 semantic dimensions × (60 positive + 60 negative cells) = 614,400 neurons per CC**. H-21/H-25 retain the capacity geometry: two classes per capacity doubling, with set counts halving at each step.

**The sizing objective is a total set count M slightly above the neuron count N, while preserving simple static indexing (H-38).** Total reserved connection capacity is derived from the selected class geometry and set counts; preserving a previous scenario's connection capacity is not a requirement (H-39). No percentage reserve has been fixed.

**A means the number of sets in class 0, whose capacity is 100 slots per set.** J is the number of classes and is a sizing parameter, not a fixed value of 15. Capacity doubles every two classes: `c_(j+2)=2*c_j`, starting from `c_0=100` and `c_1=144`. Advancing two classes quarters the set count under the provisional exact-halving rule, matching alpha=3 logarithmic-bin scaling over that interval. The 144 starting capacity is an engineering choice, not a biological measurement.

### Sizing objective and class-count options

For J classes with positive integer counts that halve exactly, let q be the count in the largest class:

```text
N = 614400
j = 0..J-1
capacity(j) = (100 if j%2==0 else 144) << (j//2)
A = q * 2^(J-1)
sets(j) = A >> j
M = q * (2^J - 1)
S = sum(sets(j)*capacity(j), j=0..J-1)
q_min = floor(N / (2^J - 1)) + 1       # smallest q with M > N
```

Using q_min is a calculation candidate for the stated small-surplus objective, not a selected migration-reserve policy. Both 15 and 16 classes are valid; the extra class extends the maximum from 12,800 to 18,432 slots. The minimum remains 100 slots, independently of the 60 cells per polarity population.

<a id="flash-only-pool-size-comparison"></a>

| Classes J | Largest-class count q | Smallest-class count A | Total sets M | Surplus M−N | Surplus / N | Maximum slots/set | Total slots S | Per-CC array bytes | MiB |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 15 | 19 | 311,296 | 622,573 | 8,173 | 1.330241% | 12,800 | 106,492,416 | 540,174,772 | 515.150806 |
| 16 | 10 | 327,680 | 655,350 | 40,950 | 6.665039% | 18,432 | 112,281,600 | 569,251,800 | 542.880821 |

Under exact halving, M changes in steps of `2^J−1`: 32,767 sets for J=15 and 65,535 for J=16. These integer steps explain the different surplus percentages; 16 classes do not inherently require 6.665039% surplus under every possible count rule. Altering that rule would require a separate indexing assessment. With the current rule, 15 classes are closer to the small-surplus objective; 16 provide a higher capacity ceiling. Final J and quotas remain open.

“Resizing the pool” in this discussion means choosing its preallocated class counts during planning. Each configured pool is static during execution; migration uses existing sets. No old-capacity preservation constraint, heap topology or auxiliary structure is introduced by this comparison.

### Detailed 15-class calculation

The following worked candidate uses `J=15`, `q=19`, and `A=311296=19*2^14`. It gives `M=622573` and `S=106492416`; it does not select the final class count or reserve policy.

| Class | Slots/set | Sets | Total slots |
| ---: | ---: | ---: | ---: |
| 0 | 100 | 311,296 | 31,129,600 |
| 1 | 144 | 155,648 | 22,413,312 |
| 2 | 200 | 77,824 | 15,564,800 |
| 3 | 288 | 38,912 | 11,206,656 |
| 4 | 400 | 19,456 | 7,782,400 |
| 5 | 576 | 9,728 | 5,603,328 |
| 6 | 800 | 4,864 | 3,891,200 |
| 7 | 1,152 | 2,432 | 2,801,664 |
| 8 | 1,600 | 1,216 | 1,945,600 |
| 9 | 2,304 | 608 | 1,400,832 |
| 10 | 3,200 | 304 | 972,800 |
| 11 | 4,608 | 152 | 700,416 |
| 12 | 6,400 | 76 | 486,400 |
| 13 | 9,216 | 38 | 350,208 |
| 14 | 12,800 | 19 | 243,200 |
| **Total** | | **622,573** | **106,492,416** |

There are **8,173 surplus sets = approximately 1.330241% of N** if each neuron owns one set, and **173.3275 reserved slots per neuron**. The highest class has **19 sets**. In the target-owned incoming proposal these accommodate 19 owners with incoming capacity 12,800; they do not cap the number of outgoing hubs at 19. Free sets in other classes cannot replace a full highest class. [Incoming ownership](IncomingConnectivity.md#compatibility-with-the-capacity-law) explains the degree distinction.

#### Per-CC array payload

With target-owned incoming sets, each slot stores a source ID and a weight; the set-owner array identifies the target. All fields use SoA. Swapping endpoint roles for the source-owned interpretation preserves the byte counts. The [array formula](CapacityPlanning.md#exact-static-arrays) is:

```text
CC_array_bytes = 4*S + S + 8*N + 4*M + ceil(N/2)
               = 5*S + 8*N + 4*M + ceil(N/2)
```

| Named array | Bytes | MiB |
| --- | ---: | ---: |
| Source neuron IDs, `4*S` | 425,969,664 | 406.236328 |
| FP8 weights, `S` | 106,492,416 | 101.559082 |
| Neuron-to-set regional references, `8*N` | 4,915,200 | 4.687500 |
| Set-to-target-owner IDs, `4*M` | 2,490,292 | 2.374928 |
| FP4 states, two values per byte, `ceil(N/2)` | 307,200 | 0.292969 |
| **Total per CC** | **540,174,772** | **515.150806** |

The exact total is **515.150806427001953125 MiB**. Each connection slot contributes exactly **five bytes across the ID and weight arrays**; connections alone occupy 532,462,080 bytes. FP4 is a standard bit width under H-32, with two values per byte in this budget. The listed array order meets the assumed reference/ID/byte alignment requirements without inter-array padding. Region headers, additional neural/learning state, histories and queues are separate costs. Reserved weight slots are capacity, not a measured live parameter count.

### Detailed 16-class calculation

This worked candidate uses the same **N=614400 neurons**, capacity sequence and exact-halving count rule, with `J=16`. The smallest integer q giving M>N is 10:

```text
q = floor(614400 / (2^16 - 1)) + 1 = 10
A = q * 2^15 = 327680
j = 0..15
capacity(j) = (100 if j%2==0 else 144) << (j//2)
sets(j) = A >> j
M = 10 * (2^16 - 1) = 655350
S = sum(sets(j)*capacity(j), j=0..15) = 112281600
```

| Class | Slots/set | Sets | Total slots |
| ---: | ---: | ---: | ---: |
| 0 | 100 | 327,680 | 32,768,000 |
| 1 | 144 | 163,840 | 23,592,960 |
| 2 | 200 | 81,920 | 16,384,000 |
| 3 | 288 | 40,960 | 11,796,480 |
| 4 | 400 | 20,480 | 8,192,000 |
| 5 | 576 | 10,240 | 5,898,240 |
| 6 | 800 | 5,120 | 4,096,000 |
| 7 | 1,152 | 2,560 | 2,949,120 |
| 8 | 1,600 | 1,280 | 2,048,000 |
| 9 | 2,304 | 640 | 1,474,560 |
| 10 | 3,200 | 320 | 1,024,000 |
| 11 | 4,608 | 160 | 737,280 |
| 12 | 6,400 | 80 | 512,000 |
| 13 | 9,216 | 40 | 368,640 |
| 14 | 12,800 | 20 | 256,000 |
| 15 | 18,432 | 10 | 184,320 |
| **Total** | | **655,350** | **112,281,600** |

There are **40,950 surplus sets = 6.6650390625% of N** if each neuron owns one set, and **182.75 reserved slots per neuron**. The highest class has **10 sets of 18,432 slots**; under incoming ownership these limit the corresponding owners' incoming capacity. The preceding class provides another 20 sets of 12,800 slots. Availability in a particular class still depends on assignment. Final class count and quotas remain open.

#### Per-CC array payload for 16 classes

The same SoA arrays and representation assumptions give:

| Named array | Bytes | MiB |
| --- | ---: | ---: |
| Source neuron IDs, `4*S` | 449,126,400 | 428.320312 |
| FP8 weights, `S` | 112,281,600 | 107.080078 |
| Neuron-to-set regional references, `8*N` | 4,915,200 | 4.687500 |
| Set-to-target-owner IDs, `4*M` | 2,621,400 | 2.499962 |
| FP4 states, two values per byte, `ceil(N/2)` | 307,200 | 0.292969 |
| **Total per CC** | **569,251,800** | **542.880821** |

The exact total is **542.88082122802734375 MiB**, from `5*S+8*N+4*M+ceil(N/2)`. Connections alone occupy **561,408,000 bytes**. With an eight-aligned base and the listed array order, references start at offset 561,408,000, owner IDs at 566,323,200, and packed states at 568,944,600. Each array meets its element-alignment requirement without inter-array padding. The node-shared embedding table is excluded; auxiliary structures have no preset allowance.

Compared with the 15-class candidate, this candidate has **32,777 more sets**, **5,789,184 more slots** and **29,077,028 more array bytes = 27.730014801025390625 MiB**. These differences include the recalculated counts in all classes. The [indexing formulas](#formula-based-indexing) apply with `A=327680`, `q=10` and `b=15`.

### Principle: requirements before auxiliary structures

Establish the business requirement first, then discuss the implementation algorithm. Consider an auxiliary data structure only within that discussion, when its purpose and necessity follow from the requirement and algorithm. Capacity planning does not preselect auxiliary structures or reserve memory for hypothetical ones. When a structure is justified and selected, specify its semantics, lifetime and storage cost, following the existing static-allocation, SoA and alignment rules. The current per-CC budget covers only the named arrays above; algorithm-dependent storage remains unspecified.

### Shared node memory

The semantic-vector table is **shared once per node**, excluded from every per-CC figure here. At 129,280 vocabulary entries × 5,120 dimensions × two bytes/value, it uses **1,323,827,200 bytes = 1.23291015625 GiB**. For k CCs using one candidate, the node budget is `k*CC_array_bytes + 1323827200 + per_CC_extras + node_runtime`, where `CC_array_bytes` is 540,174,772 for 15 classes or 569,251,800 for 16 classes, and per_CC_extras is summed over the k CCs. The 16 GB ceiling applies to that full node budget.

These counts are a provisioning assumption, not evidence of task capacity. For an admitted degree rounded to the next class, the 100/144 geometry bounds unused capacity relative to degree below 44%, provided a suitable class is available and the degree lies between 100 and the selected maximum (12,800 for 15 classes; 18,432 for 16). Minimum-class underfilling is outside that bound; finer spacing trades unused slots against migration frequency and class availability.

### Formula-based indexing

The pool needs no heap-built topology or per-set address directory. For valid j, local set k and local slot l:

```text
A = 311296; q = 19; b = 14       # A = q * 2^b
e = j // 2; t = j % 2
c = (100 if t==0 else 144) << e
set_prefix(j)  = 2*A - ((2*A) >> j)
slot_prefix(j) = 344*A - (((344 - 100*t)*A) >> e)
set_id = set_prefix(j) + k
p = slot_prefix(j) + k*c + l
```

The 344 factor sums the 100/144 pair's slot contributions over geometric levels. Source-ID and weight byte offsets are `4*p` and `p`. For a valid flat set ID:

```text
j = b - floor_log2((2*A - 1 - set_id) // q)
k = set_id - set_prefix(j)
```

For a valid global slot index:

```text
e = b + 2 - floor_log2((344*A - 1 - p) // (43*q))
even_start = 344*A - ((344*A) >> e)
t = 1 if p >= even_start + ((100*A) >> e) else 0
j = 2*e + t
k, l = divmod(p - slot_prefix(j), capacity(j))
```

Here `344*A = 43*q*2^(b+3)`, giving the integer inverse. For this pool the set inverse uses `b=14, q=19`, and the slot inverse uses `b+2=16, 43*q=817`. For J classes, the formulas require `A=q*2^b` with positive integer q and `b>=J-1`, so every class count is an exact integer. The 16-class candidate uses `b=15, q=10`; the same formulas apply within its larger ID ranges. All arguments to floor_log2 must be positive and IDs within the stated pool. Regional references still point into fixed ID arrays; region lifetime and relocation semantics are unchanged.

## Incoming degree and ownership

[Synaptic integration research](SynapticIntegrationResearch.md) addresses H-22’s excitability concern. The degree law allocates connection capacity; it does not set synaptic gain, temporal coincidence, dendritic integration or firing threshold. Human spine counts and model spike thresholds do not directly validate this distinct-source capacity distribution.

The same template can describe incoming capacities without changing its indexing or byte counts. The relevant reported exponent is then 3.15 rather than 2.96; both motivate a coarse alpha-near-3 comparison. Incoming and outgoing degree sums agree for the same closed directed graph, but their distributions and required class populations need not. Storage regrouping preserves edges, whereas provisioning must fit the chosen owner degree. [IncomingConnectivity](IncomingConnectivity.md) analyzes gather updates, outgoing hubs, the 100-slot threshold and optional event routing.

## Incoming and outgoing degree parameters

H-23 asks whether incoming capacity needs different scale-law parameters. H-24 assigns weight scale to training strategy, outside capacity allocation. The allocation discussion distinguishes these quantities:

| Parameter | What it describes | Current status |
| --- | --- | --- |
| alpha_in, alpha_out | Tails of distinct-source and distinct-target degree distributions | Fit or choose separately; neither is a weight exponent. |
| r | Spacing of allocated capacity classes | Engineering granularity; the provisional 100/144 alternation stays available independently of the degree exponent. |
| Lower cutoff/body, upper cutoff and mixture | Low-degree participation, rare hub allowance and neuron-class composition | Need separate input/output assessment; they are not fixed by alpha alone. |

Weight scale is a [training-strategy consideration](SynapticIntegrationResearch.md#training-strategy-incoming-weight-scale): it may be a hyperparameter or planned/adapted dynamically. Training determines learned weights and connectivity demand; allocation supplies bounded storage and migration between existing sets. Neither the capacity class nor its population exponent prescribes a weight-scale rule. FP8 storage format and the cost of any selected training state remain representation/budget questions.

The adult-fly study reports point estimates **alpha_out=2.96** and **alpha_in=3.15** for fitted tails. This is evidence for distinguishing the two observables, not a demonstrated universal or statistically established human difference of exactly 0.19. No uncertainty analysis comparing the exponents was performed here. [Yadav, Shinde and Singh](https://doi.org/10.1162/NETN.a.26), rechecked 2026-10-26.

### Provenance of adjacent-class population ratios

The adjacent-class percentages below are calculated model projections, not populations measured in the study:

| Quantity | Provenance |
| --- | --- |
| alpha_out=2.96, alpha_in=3.15 | The study's fitted adult-fly degree-tail exponents. |
| Ideal class spacing r=sqrt(2) | An engineering reference for our capacity geometry. |
| 50.6980% and 47.4671% adjacent-bin ratios | Calculated from those fitted exponents and the selected ideal spacing. No raw connectome observations were rebinned here. |
| Halving set counts in the provisional integer table | A directly chosen allocation rule. Interpreting the two-step geometry as a power law gives alpha near 3; this interpretation is inferred from the design. |

For the continuous approximation `f(k)=C*k^(-alpha)`, a bin `[a_j,a_(j+1))` contains mass proportional to its integral. If `a_(j+1)=r*a_j`, equal-ratio logarithmic bins within the modeled tail satisfy `B_next/B = r^(1-alpha)`. This assumes the same density and no truncation within the compared bins; integer degrees and terminal cutoffs require their own treatment. With the same lower-end normalization, a larger alpha gives fewer high-degree neurons:

| Degree reference | alpha | Calculated adjacent-bin count ratio, r=sqrt(2) | Calculated count ratio when capacity doubles |
| --- | ---: | ---: | ---: |
| Reported outgoing point estimate | 2.96 | 0.506980 | 0.257028 |
| Provisional engineering template | 3.00 | 0.500000 | 0.250000 |
| Reported incoming point estimate | 3.15 | 0.474671 | 0.225313 |

The actual capacities follow the exact rule **capacity doubles every two classes**, starting from 100 and 144. The sqrt(2) spacing used above is an idealized per-class reference; the recurrence does not require equal adjacent ratios. Thus 47.4671% is an idealized comparison, not an exact adjacent-class prediction for that integer layout. An empirical allocation estimate would require defined live-degree admission intervals, integrals or discrete sums over those intervals, cutoff treatment and evidence from the intended network. Reserved migration sets also need a provisioning policy; measured neuron counts alone would not fix their quotas.

If counts must halve at every step, ideal capacity spacing would instead be `r=2^(1/(alpha-1))`: about **1.424252** for 2.96 and **1.380429** for 3.15. Biology does not require this coupling; the allocator may keep its convenient capacities and adjust class populations. Merely relabeling the existing half-count, approximately sqrt(2) layout as alpha=3.15 would be inaccurate.

### Fixed-capacity sensitivity calculation

For the worked 15-class Flash 60+60 candidate, keep `N=614400`, `M=622573`, and all 15 capacities `c_j=(100 if j is even else 144)<<floor(j/2)`. Define `q=2^((1-alpha)/2)`, assign real counts `M*q^j/sum(q^h,h=0..14)`, round down, then give remaining sets to the largest fractional remainders, breaking ties by lower class index. This is a reproducible engineering sensitivity calculation, not experimentally measured set populations or allocation requirements. The alternating actual ratios are not exactly sqrt(2), so it is not an exact fit of a continuous distribution to those capacity boundaries. No live-degree graph is synthesized.

| Template alpha | Total slots S | Sets at 12,800 | Named arrays, bytes | Named arrays, MiB |
| ---: | ---: | ---: | ---: | ---: |
| 2.96 | 108,537,236 | 23 | 550,398,872 | 524.90126800537109375 |
| 3.00 | 106,492,416 | 19 | 540,174,772 | 515.150806427001953125 |
| 3.15 | 99,973,624 | 10 | 507,580,812 | 484.066783905029296875 |

The 3.15 class counts, in ascending-capacity order, are:

```text
327060, 155246, 73691, 34979, 16603,
7881, 3741, 1776, 843, 400,
190, 90, 43, 20, 10
```

The formula remains `5*S+8*N+4*M+ceil(N/2)`. The 3.15 case uses **507,580,812 bytes = 484.066783905029296875 MiB** for the named arrays. An eight-aligned base followed by references, source IDs, owner IDs, weight bytes and packed state bytes satisfies element alignment without inter-array padding for all three cases. These are per-CC arrays; the node-shared embedding table is excluded.

The 3.15 counts do not retain the halving population rule, so the current inverse-index formulas do not apply. Its addressing algorithm remains unspecified; this sensitivity calculation introduces no auxiliary representation or storage allowance. A smaller footprint buys a smaller large-set allowance; it is not evidence of equivalent task capacity or measured optimization.

Incoming and outgoing live degrees in the same closed CC must satisfy `sum(k_in)=sum(k_out)=E`, even when their tail shapes differ. Independently fitting tails therefore does not permit independently assigning incompatible total edge counts. The low-degree body, cutoffs and degree correlations must also be consistent. Reserved slots are capacity bounds and need not equal E; the three rows are alternatives, not simultaneous incoming and outgoing edge budgets.

**Current recommendation:** track alpha_in separately, use 3.15 as a literature-informed comparison, and retain alpha=3's capacity/count geometry provisionally, with J and quotas subject to the small-surplus objective. Selecting a different population requires an explicit decision about high-degree demand and reserves. Lower average incoming weights do not mathematically require a larger alpha_in or determine its value.

<a id="acceptance"></a>
## Evidence required for selection

H-21 provisionally retains the alpha-near-3 template. Lognormal and alpha=2 remain comparison models. Incoming ownership and its participation/update semantics remain under discussion; the provisional population and granularity are planning inputs, not a validated implementation contract.

Before implementation, choose J and quotas against the small-surplus objective and define the retained-edge observable, treatment of repeated targets, admission at the 100-slot boundary, maximum degree and class reserves. For empirical fitting, report sampling boundaries, neuron classes, connection threshold and fit range; use likelihood-based fitting, goodness-of-fit and alternative-distribution comparison rather than a straight line on a log plot. [Clauset, Shalizi and Newman, 2009](https://arxiv.org/abs/0706.1062).

Eventual application evidence must measure useful task behavior, occupied-degree distributions, high-class shortages, underfill, migration traffic and CPU work at matched resource budgets. The literature assessment and arithmetic are separate evidence; the current memory update checks the specified arrays and indexing formulas without re-reviewing the literature. This work downloads no connectome volume and runs no SNN experiment or repository test suite.
