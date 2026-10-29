# Steady recurrent SNN: input count and neuronal excitability

Document-set revision: 202 · Recorded: 2026-10-28 · Topics: SNN-02, SNN-03, SNN-05, SNN-07 · Research and proposals.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** P-26 relates bounded connection storage C-13 to application C-12; C-15/P-29 requires context-sensitive sequence prediction, with capacity dependency P-30. Separating stored capacity from neural integration prevents an array layout from silently defining behavior. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each scope.
- **Invariants / non-goals:** static pools, regional relocation/lifetimes, five-byte parallel source/weight slots, SoA by default and no heap topology. AoS needs an access-pattern-specific empirical reason; non-power-of-two bit widths need separate discussion, while FP4 is standard. Pool sizing targets slightly more sets than neurons with simple static indexing and a 100-slot minimum; final classes/quotas remain open. H-34 requires a demonstrated requirement before auxiliary structures. No physical-distance prior, explicit inhibitory synapses, biological equivalence, universal degree law or selected transition is implied.
- **Acceptance:** distinguish anatomy, electrophysiology, model predictions and engineering assumptions. Specify activity/readiness, timing, integration, intrinsic cancellation, noise control and precision; compare background response, contextual selectivity, fading and steerability. Justify any additional state and work before implementation.
- **Human baseline:** H-08/H-10 establish the SNN and HTM reference; H-13—H-39 supply scoped storage/layout constraints. H-22 requests excitability evidence, H-23 accepts compensating weight scale as a direction, and H-24 assigns it to training rather than capacity. H-53—H-59 constrain noise, cancellation, detection and steerability; H-67—H-70 cover rate, fraction, natural cessation and precision; H-73/H-75 permit wider state and propose two frames; H-77 defines active-rate cancellation. H-78/H-85 defer readiness choices, H-82/H-83 define flow prediction goals, and H-86 delegates obvious empirical alternatives to comparison. Exact mechanisms remain unconfirmed; the linked registry retains all evidence and exclusions.
- **Current assessment:** [K-196](../../../design/DesignAuthorityRegistry.md#k-196) distinguishes retained degree, weight concentration and correlated source drive when evaluating contextual integration. Existing population and integration comparisons remain open; no normalization, threshold, capacity parameter or auxiliary field is selected. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; [progress](../research/modeling-refinement-progress.json) records its scope and substantive work.

## Early-model interpretation of the evidence

The biological and mathematical evidence below remains reference material. Models that depend on explicit inhibitory neurons or inhibitory synaptic edges are not literal candidates for the current early architecture. Its required suppression functions are intrinsic cancellation within a semantic dimension and regulation of background-noise drive; these have different meanings and need separate operational definitions.

The complementary-input objective requires a rapid activity reduction, followed by increased noise when activity is low, preferential strengthening of the positive-semantic residual to a peak and then token-like fading. A bounded active fraction and a longer-lived meaningful residual are separate outcomes. Neither incoming degree, equal cancellation nor added white noise alone proves them. These targets guide the future integration/training discussion without adding a neuronal taxonomy, a storage array or an explicit inhibitory edge set.

H-56–H-58 explicitly exclude total active fraction from regime detection. It drives activity regulation; noise-qualified flexible-window coverage and activation-path behavior supply the separate criteria, with a small upper coverage bound for flow. C-19/P-39/P-40/P-41 record those detection boundaries; the activation-density filter remains a candidate. Noise support must therefore not relabel a low-activity residual as DMN solely from the regulated fraction.

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

With independent equal-statistic sources, nonuniform weights instead give `Var[drive]=Var[source]*sum(w*w)`. In an [exact 100-source comparison](../research/flow-context-assignment-analysis.json), weights `100 × 0.01` and `0.901 + 99 × 0.001` both sum to one, but the latter has **81.19 times** the input variance. The analytical averaging count `(sum w)^2/sum(w*w)` is 100 versus about 1.232; it is neither stored metadata nor contextual capacity. With one fully shared source fluctuation, both rows instead have the same variance. Degree and total weight therefore do not by themselves calibrate a contextual input score; useful strong associations must be judged by later behavior.

The denominator, if any, must refer to a defined live quantity rather than reserved capacity. Moving an unchanged connection set to a larger allocation class must not itself change activation behavior. Degree adaptation likewise needs an explicit update boundary when live connections change.

## Application choices to compare

| Candidate | Purpose | Costs and unresolved choices |
| --- | --- | --- |
| Point-neuron integration with explicit gain/threshold control | Keep a compact first model while controlling degree-dependent background activation. | Define the activity signal, integration interval, leak, inhibition, operating range and adaptation. Global normalization can dilute a small useful pattern in a large set. |
| Branch/segment integration followed by a neuron-level decision | Let a large incoming set represent several local input combinations; assess the connection with HTM context/prediction. | Choose grouping, nonlinearities, local state and how segment events affect prediction versus activation. More segments can increase false detections unless thresholds, inhibition or learning address that. |

The integration and activity-control contract must explain the intended behavior before a flat global sum can be evaluated as an implementation choice. A two-stage input model is a biologically motivated candidate, while a controlled point-neuron model remains a cheaper comparison. Neither is selected by H-22. The 100-slot allocation minimum does not define a dendritic segment size or a coincidence threshold, and HTM's learning/permanence semantics do not follow automatically from these experiments.

Both candidates can retain target-owned parallel source/weight arrays. Establish the required integration and activity-control behavior before discussing implementation algorithms and any auxiliary representation. No branch-membership or supporting-state layout is preset here. The worked 15-class Flash candidate uses **515.150806427001953125 MiB** for its named arrays; this is not a complete biological simulation budget. Any storage justified later must respect static allocation and regional lifetime constraints.

## Context selectivity is separate from connection capacity

H-82/H-83 require prediction to use the preceding sequence context. A large incoming set supplies many source weights; it does not by itself distinguish several combinations of those sources. [Conditional detection arithmetic](../research/context-detection-analysis.json) makes this distinction without selecting a neuron equation.

Consider four binary inputs and a receiver intended to detect either `(a AND b)` or `(c AND d)`. A single affine sum followed by a threshold cannot accept `1100` and `0011` while rejecting `1010` and `0101`: the two accepted sums require the total of all four weights to be at least twice the threshold, while the two rejected sums require it to be smaller. Bias and signed weights do not remove that contradiction. Two separately thresholded conjunctions followed by an OR can implement this example.

This is a fixed-state single-detector limitation. It does not establish failure of the complete nonlinear recurrent model, its other contextual cells, multi-frame computation or intrinsic cancellation. Different existing cells in one coordinate/polarity population could carry different contextual detections. That uses representational capacity and changes subsequent activity and learning; it is not automatically equivalent to several subunits inside one neuron. Compare those existing-cell assignments before assuming new subcellular storage.

[Poirazi, Brannon and Mel (2003)](https://www.cs.jhu.edu/~ayuille1/courses/Stat271-Fall13/Poirazi_etal03b.pdf) compare nonlinear subunit models with a point-neuron alternative against a detailed CA1 simulation. They distinguish overall input strength from spatial configuration and show why the stimulus set affects the comparison. This is a model of a biological model, with inhibitory inputs and imperfect rate predictions; it does not select our architecture or supply a human-CC parameter. Its useful methodological lesson is to compare contextual configurations at controlled overall drive, rather than infer selectivity from mean activity alone.

Pooling several remembered patterns can still be adequate when misleading mixtures are sufficiently rare in the actual inputs. [Ahmad and Hawkins (2016), §§2–3.3](https://arxiv.org/pdf/1601.00720v2), analyze binary sparse pattern detection and explicitly discuss this union trade-off. Their favorable random-pattern bounds cannot be transferred from total neuron count alone. Our semantic populations, graded strengths, repeated tokens and learned context assignment determine a different overlap distribution.

The scaling conditions matter even in the uniform binary reference. With a fixed number of sampled endpoints and a fixed active fraction, hypergeometric overlap approaches a binomial distribution as the total population grows; its false-match probability does not approach zero merely from adding neurons. In the [exact calculation](../research/context-detection-analysis.json), a 24-endpoint, threshold-12 detector at active fraction `1/32` has random-match probabilities about `1.02e-12` for 4,096 cells and `1.65e-12` for 614,400. Holding the active **count** at 128 instead gives about `1.06e-38` at the larger population. These are different scaling assumptions, not measured application errors or selected sparsity/threshold values. The real graded, structured context distribution remains the relevant comparison.

For illustration only, select three of sixty cells independently in one polarity of every dimension, and retain one source from each of 100 distinct coordinates. A fresh cell pattern for the **same** signed token matches each retained endpoint with probability `3/60`. A query with independently randomized fair signs matches with probability `3/120`. Requiring at least ten of those endpoints gives match probabilities about `0.02819` and `0.0002127`, respectively—a factor of about 133 despite equal total active-cell counts. These are diagnostic sampling laws, not measured false predictions or a recruitment rule. Whether the second contextual pattern should match is itself task-dependent.

The exact finite-union examples also distinguish a union's mean size from its recognition probability. Conditional on a union containing `u` IDs, uniform fixed-size query overlap has a hypergeometric distribution. Averaging that probability over possible unions is generally different from evaluating a formula at the expected union size. The evidence includes both random-query matches and matches conditional on the query not exactly equaling any stored pattern; neither automatically labels a semantically plausible alternative as wrong.

The resulting comparison is specific: retain a simple sum if its learned contextual assignments provide sufficient prediction selectivity; distribute detections across existing cells where that works; consider separate nonlinear input groups only if they resolve an observed deficit at justified memory, work and learning cost. Group sizes, branch metadata, permanence, traces and integration fields remain unspecified. The static capacity law and five-byte slot proposal are unchanged.

## Fractional activity and rate dynamics

H-67/H-69 propose discrete-time rate dynamics with fractional recruitment; H-68 clarifies natural cessation of active propagation with connection weights retained. This changes the possible units of the integration input, response and repair deficit. A rate-valued model can retain individual cells and sparse endpoints, but does not automatically preserve spike-timing behavior or eliminate shared-path interference. [RateDynamics](RateDynamics.md) compares the modeling choices and the relevant primary research.

H-70 requires precision to be considered across the computation. FP4 state and FP8 weight storage do not prescribe multiplication, accumulation, cancellation or update precision. A weak decay that repeatedly rounds back to the same stored value can prevent fading; a small signed residual after cancellation can be lost or change sign. These effects belong to behavioral acceptance. No extra trace, branch state, numeric scale array or new buffer is allocated by this analysis.

## Evidence needed before implementation

- Define the represented activity/readiness quantities and their integration window before choosing FP4, FP16, FP32 or integer formats. H-73 permits wider per-neuron state; H-78 leaves shared versus separate readiness open.
- Compare background and matching-pattern responses across live incoming degrees at matched input statistics. Record firing rate, false activations, selectivity and recovery from excessive drive; equal rates across all degrees are not assumed to be the goal.
- Vary temporal correlation, input strength and the candidate integration, intrinsic-cancellation and noise-control choices. Explicit inhibitory synapses remain outside the early model. Check contextual selectivity and stability after learning, degree changes and allocation-only migration.
- Measure task benefit, CPU work and extra state at the 16 GB node constraint. No neural experiment, code implementation, repository test or build is performed in this research revision.
