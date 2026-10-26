# Steady recurrent SNN: input count and neuronal excitability

Document-set revision: 24 · Recorded: 2026-10-26 · Topics: SNN-02, SNN-03, SNN-05, SNN-07 · Research and proposals.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 24, all sections here; `pending-human`.
- **Value lineage:** candidate P-25/P-22 connect D-08/application C-12 to C-01 and V-01/V-02; P-26 relates bounded connection storage C-13 to C-12. Distinguishing stored connectivity from the neural transition prevents a storage convenience from silently defining application behavior.
- **Invariants / non-goals:** preserve static pools, regional relocation/lifetimes, the provisional capacity template and separate application documentation. No biological equivalence, universal human degree law, selected neural transition or implementation authorization is inferred from the studies. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** distinguish anatomy, electrophysiology, model predictions and engineering assumptions; specify input timing, integration, inhibition, gain/threshold adaptation and their memory costs before implementing the corresponding scope. Future acceptance is described below.
- **Human baseline / continuity:** H-08/H-10 establish the SNN and HTM discussion; H-13–H-21 establish storage constraints and provisional capacity. [H-22](../../../design/DesignAuthorityRegistry.md#h-22) asks whether excessive incoming connectivity causes implausibly easy activation and requests biological evidence. It selects no remedy. K-12 records the research revision; no qualified application implementation baseline exists. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.

## Finding

The concern is valid for a point neuron that adds comparable positive inputs and uses the same fixed threshold regardless of incoming degree. It does not establish that large anatomical input counts are implausible. Human neurons can have thousands to tens of thousands of dendritic spines, while their output depends on which inputs coincide, their strength and location, dendritic integration, inhibition and activity-dependent regulation.

Incoming ownership remains a storage proposal. Neither that ownership nor alpha near 3 determines excitability. The global weighted-sum sketch in [IncomingConnectivity](IncomingConnectivity.md#gather-based-updates) is an execution illustration, not an accepted biological neuron model.

## Anatomical measurements and their units

Sources were checked on 2026-10-26. These are primary studies; the ranges and averages below retain their authors' measurement or estimation scope.

| Study | Anatomical result | Scope and limit |
| --- | --- | --- |
| Benavides-Piccione et al., *Cerebral Cortex*, 2024, [Table 2, row 28; Figure 9](https://doi.org/10.1093/cercor/bhae180) | Reported estimated basal-arbor spine ranges: human visual BA17 **590–2,230**; temporal BA20 **5,073–19,220**; temporal BA21 **3,930–14,890**. | Basal spines, not whole-neuron synapses or distinct presynaptic cells. Reconstruction and density estimates include a correction assuming sampled basal arbors represent two-thirds of their total extent. These are the paper's estimation ranges, not a universal degree distribution or measured firing thresholds. |
| Elston, Benavides-Piccione and DeFelipe, *Journal of Neuroscience*, 2001, [Figure 3](https://www.jneurosci.org/content/jneuro/21/17/RC163.full.pdf) | Estimated basal spines per average human layer-III pyramidal cell: **2,417** in visual V2, **12,700** in temporal cortex and **15,138** in prefrontal cortex. | Derived from arbor branching and spine-density measurements. Human tissue came from one 48-year-old donor; these are illustrative regional estimates, not population-wide constants. V2 is not the BA17 region in the 2024 study. |

Keep four counts separate: reserved connection slots, retained source-target pairs, anatomical synaptic contacts and contacts active in the relevant integration interval. One presynaptic neuron can make multiple contacts on a postsynaptic neuron. Anatomical whole-arbor input also includes projections outside the modeled CC. Consequently, these measurements support large anatomical input capacity but do not validate 12,800 distinct intra-CC sources, the selected exponent, or the number of large sets.

## Physiological and computational evidence

| Primary study | Finding | Implication and transfer limit |
| --- | --- | --- |
| Polsky, Mel and Schiller, *Nature Neuroscience*, 2004, [paper](https://doi.org/10.1038/nn1253) | In rat neocortical pyramidal cells, nearby inputs on one thin branch summed sigmoidally; widely separated inputs or inputs on different branches summed approximately linearly. | Input location and branch nonlinearities matter. Local concentration can amplify inputs as well as saturate their combined effect; segmentation is not an automatic anti-excitation mechanism. |
| Beaulieu-Laroche et al., *Cell*, 2018, [human dendritic recordings](https://pmc.ncbi.nlm.nih.gov/articles/PMC6197488/) | Human layer-5 neurons showed stronger electrical compartmentalization over their extended dendritic trees; distal dendritic spikes could fail to propagate effectively to the soma. | Direct ex vivo human physiology supports separating local dendritic events from output spikes. It does not establish the same attenuation or excitability for every cortical layer or cell class. |
| Eyal et al., *Frontiers in Cellular Neuroscience*, 2018, [experiment-constrained models](https://doi.org/10.3389/fncel.2018.00181) | Six modeled human L2/L3 cells predicted **134 ± 28 simultaneously activated excitatory synapses** for a 50% chance of a somatic spike; **20 ± 10 clustered inputs** could trigger a local NMDA spike. | These are model predictions informed by anatomy and recordings, not direct universal thresholds. A local dendritic spike and a somatic output spike are different events. The result also shows that many stored contacts do not require all contacts to activate together. |
| Okun and Lampl, *Nature Neuroscience*, 2008, [in vivo recordings](https://doi.org/10.1038/nn.2105) | Excitatory and inhibitory inputs in rat somatosensory cortex were correlated in timing and strength during spontaneous and sensory-evoked activity. | Firing depends on interacting excitation and inhibition, not the excitatory contact count alone. This does not prescribe a fixed E/I count ratio for our CC. |
| Turrigiano et al., *Nature*, 1998, [cortical culture experiment](https://doi.org/10.1038/36103) | Blocking inhibition initially increased firing; over **48 hours**, excitatory miniature-current amplitudes decreased and firing returned near control levels. | Activity-dependent synaptic scaling can counter persistent overactivity. This slow adaptation is distinct from fast inhibition and does not prove an exact inverse-degree weight rule. |

These studies address different layers, species, preparations and observables. They support mechanisms to examine, not a single measured mapping from incoming degree to firing probability. Large dendritic trees can provide more separately processed input combinations; their size alone does not imply either greater or lower excitability.

## Training strategy: incoming weight scale

H-24 places weight scale within training strategy, not capacity allocation. A strategy may consider it as a hyperparameter or plan/adapt it dynamically using learned connectivity, activity or task feedback. This responsibility boundary is confirmed; the particular objective, algorithm, schedule and state remain open.

H-23 regards lower typical incoming weights for more connected neurons as a reasonable way to avoid excessive activation. Record this as an application modeling direction. The available homeostatic evidence does not establish that every higher-degree biological neuron has weaker inputs, or that every individual weight must shrink.

For the positive inputs of neuron i, let `K_i+` be the live excitatory degree and `W_i+=sum_j w_ij+` its total positive weight. Then `mean(w_i+)=W_i+/K_i+`. Comparable W_i+ across neurons gives an inverse-degree mean; nonuniform weights and a few strong connections remain possible. More generally, a candidate conditional scale could vary as `K^(-gamma)`, with gamma distinct from the degree-distribution exponent alpha. H-23 chooses no gamma, fixed sum or automatic rescaling operation.

Expected drive also depends on presynaptic activity: `E[drive_i+]=sum_j p_j*w_ij+` in a binary-activity illustration. Correlations affect fluctuations and coincidence. Separate excitatory/inhibitory or magnitude budgets avoid interpreting a nearly cancelled signed sum as a small total input resource. Lowering mean weights can compensate for larger degree, but does not by itself prove recurrent stability or preserve a small matching pattern's effect.

A related primary modeling study derives degree distributions from bounded or fixed total synaptic strength and compares them with reconstructed Drosophila mushroom-body connectivity. Its fixed-strength model performs best overall among the tested alternatives. This supports studying joint degree/weight constraints; anatomical synapse counts are used as a strength proxy, and the work does not establish a universal human inverse-degree law. [Ocker and Buice, 2020](https://doi.org/10.1371/journal.pcbi.1008080).

The [incoming/outgoing comparison](ConnectivityScalingResearch.md#incoming-and-outgoing-degree-parameters) addresses degree distributions and allocation geometry. Weight-scale policy belongs here, with training. Training can change live-degree demand and request migration within the fixed pool; allocation-only migration preserves unchanged live weights and their effective contribution. If degree is used by a training rule, it refers to defined live connectivity, not reserved class capacity. FP8 representation and the resolution of small effective weights remain to be specified. Any selected scale or adaptation state needs a budget, but budgeting it does not turn the training policy into an allocation parameter.

## Why the simple sum needs a degree-aware specification

As an analytical illustration, let K retained excitatory sources independently be active with probability p in one integration interval, with equal positive weight w. Then:

```text
E[drive]   = K*p*w
Var[drive] = K*p*(1-p)*w*w
```

For p=0.01, increasing K from 100 to 10,000 changes expected active inputs from 1 to 100. This is a deliberately simplified calculation, not a biological rate estimate. It explains why retaining the same weight scale and threshold can create the bias the user identified.

Scaling weights by 1/K holds this simplified mean constant, but reduces sensitivity to a fixed-size matching group as K grows. Scaling by 1/sqrt(K) holds variance constant under the independence assumptions, while its positive mean still grows as sqrt(K). Correlated inputs add covariance terms; matching input statistics and inhibitory drive also matter. Neither normalization is a universal biological law or automatically sufficient for the recurrent network.

The denominator, if any, must refer to a defined live quantity rather than reserved capacity. Moving an unchanged connection set to a larger allocation class must not itself change activation behavior. Degree adaptation likewise needs an explicit update boundary when live connections change.

## Application choices to compare

| Candidate | Purpose | Costs and unresolved choices |
| --- | --- | --- |
| Point-neuron integration with explicit gain/threshold control | Keep a compact first model while controlling degree-dependent background activation. | Define the activity signal, integration interval, leak, inhibition, operating range and adaptation. Global normalization can dilute a small useful pattern in a large set. |
| Branch/segment integration followed by a neuron-level decision | Let a large incoming set represent several local input combinations; assess the connection with HTM context/prediction. | Choose grouping, nonlinearities, local state and how segment events affect prediction versus activation. More segments can increase false detections unless thresholds, inhibition or learning address that. |

The assistant recommends discussing the integration and activity-control contract before adopting a flat global sum. A two-stage input model is a biologically motivated candidate, while a controlled point-neuron model remains a cheaper comparison. Neither is selected by H-22. The 100-slot allocation minimum does not define a dendritic segment size or a coincidence threshold, and HTM's learning/permanence semantics do not follow automatically from these experiments.

Both candidates can retain target-owned parallel source/weight arrays. Establish the required integration and activity-control behavior before discussing implementation algorithms and any auxiliary representation. No branch-membership or supporting-state layout is preset here. The worked 15-class Flash candidate uses **515.150806427001953125 MiB** for its named arrays; this is not a complete biological simulation budget. Any storage justified later must respect static allocation and regional lifetime constraints.

## Evidence needed before implementation

- Define whether an FP4 value represents instantaneous drive, a spike/event, prediction or a persistent state. Specify its integration window and update semantics.
- Compare background and matching-pattern responses across live incoming degrees at matched input statistics. Record firing rate, false activations, selectivity and recovery from excessive drive; equal rates across all degrees are not assumed to be the goal.
- Vary temporal correlation, branch grouping, inhibitory drive and input strength. Check stability after learning, degree changes and allocation-only migration.
- Measure task benefit, CPU work and extra state at the 16 GB node constraint. No neural experiment, code implementation, repository test or build is performed in this research revision.
