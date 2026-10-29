# Steady recurrent SNN: context loops, decay scheduling and numeric representations

Document-set revision: 202 · Recorded: 2026-10-28 · Topics: SNN-01/SNN-02/SNN-03/SNN-05/SNN-07 · Research pass complete; modeling choices remain open.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** C-26/P-60 serves useful history-dependent responses; C-27/P-63 preserves meaningful temporal evolution; C-28/P-65 serves faithful, affordable numerics. C-18/P-36 and C-29/P-69 connect cancellation and consistent frames to those behaviors. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** fixed signed semantic roles and individual learned identities, static SoA connections, early intrinsic polarity cancellation without explicit inhibitory synapses, per-CC activity regulation and external steerability. No new runtime fields, topology indexes, precise decay law, numeric layout or output decoder is adopted. Application documentation remains separate from CoD language contracts.
- **Acceptance:** distinguish dominant-response fading from useful context retention; demonstrate effects of history on later responses under controlled conditions; compare numeric and timing methods by semantic error, lifetime, interference, steerability and actual resource requirements. Sources and arithmetic support proposals, not claims of biological equivalence or complete SNN validation.
- **Human baseline:** H-67—H-74 define loose fading, fractional/rate exploration, useful loops, numerical care and justified complexity; H-75—H-77 supply frame/input/cancellation directions; H-78 leaves state interpretation open. The linked registry retains exact evidence and exclusions. H-79—H-81 reserve Hebbian/HTM for flow and request its separate initial algorithm design.
- **Current assessment:** [K-205](../../../design/DesignAuthorityRegistry.md#k-205) makes the noise-only coverage example's independent, symmetric, fresh-frame assumptions explicit. K-204's distinction between numerical disappearance and faithful fading remains; no noise law, counting rule, rounding policy or extra state is selected. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; [progress](../research/modeling-refinement-progress.json) records its scope and substantive work.

## First-principles scope and a simple comparison baseline

The required value is faithful semantic behavior within the node budget: a regulated active CC, meaningful token-driven responses, useful influence from earlier context and continued external steerability. Low bit counts, event scheduling and additional state are possible means. They have no independent acceptance value.

The current economical **research baseline** is H-75's two alternating frames, incoming gather, FP32 reference arithmetic and the existing shared recurrent graph. The snapshot proposal has a concrete purpose: each destination sees the same preceding state, without traversal-order effects or persistent per-edge activity accumulation. It is not an adopted neuron equation or implementation. FP16 state and low-width weights can be compared against the same behavior without presuming extra runtime structures. Useful small loops can occur within the shared graph; they do not require a separate context subsystem. [RateDynamics](RateDynamics.md#two-alternating-frames-with-incoming-gather) specifies the proposal's scope.

| Required value | Concrete issue to examine | Condition for introducing more machinery |
| --- | --- | --- |
| Gradual fading and retained weak context | Small updates may disappear or change sign after rounding. | Show that the simple adequate-precision update fails an actual precision or resource requirement before adding residual state or scheduling. |
| Affordable execution | Connection access may dominate the update cost. | Demonstrate which work a proposed scheduler avoids, including its own maintenance and input invalidation; saving a scalar multiplication alone is insufficient evidence. |
| Useful history affects later output | Recurrent patterns may persist without retaining relevant content. | Establish a failed required behavior before adding a new memory mechanism; first assess the existing graph. |
| Faithful signed semantics | Encoding/cancellation may lose a small meaningful residual. | Identify its units, error and behavioral effect before adding scales, fields or coordinated allocation rules. |

The alternatives below delimit the comparison; they are not a menu of components to install together. H-86 directs obvious empirical alternatives to later real-data comparisons with meaningful observations, without repeated requests to choose now. A paper demonstrating a mechanism elsewhere does not establish its value here. If the baseline meets the requirement, an additional algorithm or data structure needs a measured benefit that justifies its cost and new semantics. H-74 applies to the cumulative design as well as each individual proposal.

H-78 leaves predictive readiness versus current firing representation open for future controlled comparison. The FP32 numerical reference is not a decision to merge those meanings or to store them independently. H-76's prediction-guided input and H-77's population-wide net/proportional active-rate cancellation provide concrete behaviors that each candidate must support. Their timing and state requirements are tracked in [RateDynamics](RateDynamics.md#what-a-frame-value-means).

## Useful small loops and dominant-response fading

H-71 requires a distinction between constructing the main activation progression and suppressing everything that recurs sooner. Main-chain construction should avoid deliberately creating a shortcut that defeats its intended progression. Existing or emerging small loops can nevertheless be useful: under background support they may retain aspects of earlier input and influence subsequent outputs. Cycle length or mere reachability is not a rejection criterion.

The isolated-token response can lose prominence on the loose half-life scale while weaker, context-dependent recurrent activity remains useful. Therefore fading must not be assessed as erasing every causal influence of that token. The dominant response envelope, retained context and later input-driven output are different observables. A new explicit state partition into these components is not presumed.

| Observed recurrence | Relevant assessment |
| --- | --- |
| Small loop retains history and changes a later response appropriately | Desired candidate context mechanism, even after the original prominent response has faded. |
| Return activity displaces an intended transition or prevents new input from redirecting the network | Harmful interference or stale behavior, to be assessed by its effect. |
| Loop carries activity but no identifiable history-dependent effect | Sustained activity alone does not establish useful context. |
| Background noise revives a useful weak pattern | Potential maintenance of context; distinguish it from arbitrary fluctuation or unwanted dominance. |

H-59's external steerability remains essential. H-71 does not require perpetual activity, a private memory loop per token, a separate context network, or actively constructing every possible small cycle. Hebbian/HTM learning belongs to flow training on token sequences (H-79). H-80/H-81 open its initial algorithm design separately from DMN/isolated-token construction and repair; no flow update rule is selected. Immersion remains an expected emergence without dedicated training. Small-loop participation is an acceptance concern rather than a new mandatory dedicated training stage.

[Goldman (2009)](https://pmc.ncbi.nlm.nih.gov/articles/PMC2674525/) distinguishes anatomical recurrence from functional propagation among activity patterns. Its linear constructions can retain a pulse over a finite interval without a slow individual eigenmode. Some output fits use optimized readout weights, and transformed networks may contain signed connections. This supports examining actual transient responses; it does not supply our fixed semantic readout, justify rotating cell identities or require removing the return connection.

[Rajan et al. (2016)](https://arxiv.org/pdf/1603.04687) retain cue identity during a delay when both cue conditions receive the same time-varying input. That input is a frozen filtered-noise waveform repeated across trials; separate fresh noise tests tolerance. The result supports memory under common ongoing drive, but does not establish our one-presentation response under regulated fresh noise. Their partially trained network is fully connected: the fraction of modified weights is not the fraction of allocated connections. Neither their training rule nor their reported sparsity supplies our slot budget or flow algorithm.

[Jaeger and Eck's cyclic-rehearsal model (2008)](https://www.ai.rug.nl/minds/uploads/2185_JaegerEck08.pdf) illustrates the difference between persistent cycling and faithful content. Its reported inaccurate delay recall can eventually settle into a shorter, unrelated pattern. The architecture uses a trained delay readout, accumulated prediction errors and voting feedback after repeated cueing. It is evidence to inspect content and presentation conditions, not a ready-made small-loop mechanism or proof of one-token retention here.

[Lau and Bi (2005)](https://pmc.ncbi.nlm.nih.gov/articles/PMC1177363/) evoked seconds of recurrent activity in cultures of 20–100 rat hippocampal neurons; blocking inhibitory transmission enhanced it. Their methods also report a post-episode rest requirement and use 30-second stimulus intervals to permit recovery. That chosen interval is not a measured minimum recovery time. The result supports examining persistent excitation, but does not demonstrate rapid repeated token responsiveness. Their threshold-defined episode duration is not our loose token-response half-life.

[Volman and Gerkin (2011)](https://rick.gerk.in/my-papers/volman-gerkin.pdf) model related reverberation with activity-dependent asynchronous release and fast/slow synaptic resource dynamics. Release bridges gaps between bursts, while accumulated depression eventually ends the episode. This is not externally regulated white background noise or fading from a fixed weight alone. The transferable comparison is whether a retained response remains responsive to subsequent inputs during its fading and afterward. It does not justify adding resource variables, a refractory period or a recovery timer to our two-frame model.

A useful proposed comparison supplies two different earlier histories followed by the same recent input sequence. With matched initial conditions, learned weights and controlled noise, ask whether later emitted responses retain a relevant difference due to the earlier history. Compare this with responses lacking that history and with intervening inputs that should override it. This distinguishes memory from a merely long output sequence. The task-specific meaning of a relevant difference and the readout remain open.

With activity-dependent noise amplitude, “controlled noise” needs a precise experimental meaning. Sharing the underlying random draws while each run retains its own regulator compares the complete feedback system; different histories may legitimately produce different noise amplitudes. Imposing identical applied forcing instead is a separate intervention that can isolate recurrence under matched drive. Neither result substitutes for the other, and shared draws do not guarantee lower variance in nonlinear comparisons. These are future comparison conditions, not a requirement to store noise histories or add state; the [conditional analysis](../research/temporal-coverage-analysis.json) records their scope.

Active fraction, qualified coverage and effective rate still have different roles. Several useful weak loops could coexist without all being counted as prominent instantaneous activation. A detector's exclusion of incidental noise must not automatically erase causally useful context from the model or forbid its effects. Conversely, preserving more loops is not unlimited storage: interference, saturation and the flexible flow-coverage constraint still matter.

### Noise support and information retention

Noise-supported persistence is plausible under particular nonlinear dynamics, but more activity is not automatically more memory. [Novikov et al. (2021)](https://www.frontiersin.org/journals/neural-circuits/articles/10.3389/fncir.2021.647944/full) find an intermediate noise range that prolongs a stimulus-initiated response while an unstimulated circuit stays near background; excessive noise activates both. This supports comparing cued and uncued behavior under the same noise. Their multi-circuit selectivity comparison instead assigns common noise to designated groups while every circuit receives the same stimulus. It does not demonstrate that unspecified background noise identifies which semantic history should return. The model uses excitatory/inhibitory populations, several synaptic timescales and short-term plasticity. Neither its circuitry nor its threshold-defined duration establishes our small-loop retention or token half-life.

[Burak and Fiete (2012)](https://fietelab.mit.edu/wp-content/uploads/2018/12/infodiff_PNAS.pdf) show memory diffusion despite persistent activity. Their analysis assumes weak fluctuations near a tuned continuum of steady states, with systematic drift eliminated. Continuous-valued rates alone do not imply that attractor structure, and a fading loop need not have it. Their reduction of diffusion with larger populations or higher firing also assumes preserved or scaled tuning curves; raising our background-noise level is a different intervention. Use this evidence to distinguish retained content from sustained activity, rather than to set a CC capacity, response half-life or noise-control rule.

For an elementary linear counterexample, let `h[k+1] = lambda*h[k] + eta[k]`, with independent zero-mean noise of variance `sigma^2` and `0 < lambda < 1`. An initial history difference `b` decays as `b*lambda^k`, while stationary noise variance is `sigma^2/(1-lambda^2)`. Thus stronger persistence also retains more noise. This derivation has no threshold, cancellation, regulation or learned selectivity: it does not rule out beneficial noise in our nonlinear model, but it shows why that benefit must be demonstrated.

The isolated-token fidelity target and a context-modulated response also need distinct experimental conditions. Earlier context is intended to affect later output; requiring that output always equal the latest isolated token would erase the desired effect by definition. This does not authorize arbitrary semantic drift. The controlled histories and expected later distinctions must define the relevant assessment.

Balanced additive noise followed only by H-77 cancellation does not preferentially amplify the mean signed residual. If the preexisting coordinate is `v`, the output remains exactly `v + noise_positive_total - noise_negative_total`; equal expected totals give expected output `v`. The summed absolute activity can nevertheless increase. The [conditional body/return analysis](../research/body-return-response-analysis.json) separates that activity increase from semantic amplification. The required growth–peak–fade must be evaluated in the actual learned recurrent and transfer response with its noise regulation; this does not by itself call for an extra timer, fatigue variable or memory subsystem.

### Noise can enable a response without ensuring useful amplification

The neuron response already under discussion is a possible source of noise-assisted growth; a separate amplification mechanism is not implied. Consider a conditional one-transition reference with sixty cells per polarity. Each positive cell receives residual drive `delta=0.01`, each negative cell receives zero, and every cell receives independent `U[0,sigma]` noise before `phi(x)=clip((x-0.1)/0.05,0,1)`. These are illustrative common rate units. Apply H-77 cancellation after this transfer, with no recurrence or feedback in the calculation.

The input's signed sum is 0.6. Cancellation preserves the provisional signed sum, so its expected output is `60*(E[phi(delta+U)]-E[phi(U)])`. [Exact piecewise integration](../research/noise-feedback-analysis.json) gives:

| Noise upper bound `sigma` | Mean signed output | Output standard deviation |
| --- | ---: | ---: |
| 0 | 0 | 0 |
| 0.10 | 0.6 | 0.2720 |
| 0.12 | 2.5 | 1.3889 |
| 0.15 | 4.0 | 3.5214 |
| 0.20 | 3.0 | 4.8708 |

Here noise enables a subthreshold residual and the steep response can amplify its mean. The greater mean at 0.15 comes with a lower mean-to-standard-deviation ratio than at 0.10. Neither statistic alone establishes meaningful token growth, retained context or an optimal noise setting. A unit-slope threshold-linear response is different: under identical draws it cannot expand an input difference above one. Learned recurrent gain can contribute separately. Iterating a conditional mean formula would not reproduce the stochastic shared network or its activity-dependent controller.

Noise can make a surviving distinction visible through an otherwise uninformative output. In a [conditional threshold calculation](../research/memory-gain-analysis.json), two equally likely histories leave exact drives zero and 0.05, both below a 0.1 threshold. The noiseless binary output is always zero. Independent uniform noise on `[0,0.1]` gives firing probabilities zero and one half, carrying about 0.311 bits about the history; doubling the noise bound reduces that value to about 0.049 bits. These are exact channel calculations, not recurrent-memory performance or a selected noise level. [Kosko and Mitaim (2003)](https://sipi.usc.edu/~kosko/NN2003.pdf) establish related noise benefits under explicit threshold/input assumptions; their result does not guarantee a practically large improvement or application-level memory.

This exposes information already present in the drive. If both histories have instead become identical in the **complete causally relevant state**, independent noise cannot reconstruct which occurred. A history-dependent regulator or learned parameters must be included when assessing that premise; identical semantic sums alone do not imply identical state. The artifact derives the conditional information bound. It does not prescribe estimating entropy during operation. Compare useful later responses and retained distinctions together, without treating renewed activity as proof that lost context returned.

[Knoll and Lindner (2021)](https://people.physik.hu-berlin.de/~lindner/PDF/Knoll-Lindner-JCNS-2021.pdf) compare recurrent signal encoding with controls matching firing rate and with controls approximating network noise. Their results separate better encoding from simply changing average firing. Their inhibitory LIF network, fixed random wiring and continuously supplied signal differ from our model. The study supports behavioral comparisons, not adopting its dynamics or inferring retained one-presentation context from signal encoding.

Future comparisons should observe residual growth, correct later prediction and fading together under the actual noise regulator. The transfer shape, its placement relative to cancellation and the noise law remain empirical alternatives under H-86. This reference selects no threshold, new state, inhibitory connections or specialized noise controller; H-78/H-85 remain independent open comparisons.

### Retention gain and usable context

Longer persistence does not improve every history timescale. In a [scalar analytical reference](../research/memory-gain-analysis.json), `x[t]=a*x[t-1]+b*u[t]+eta[t]`, let `u` and `eta` be independent, zero-mean, temporally independent processes, with `|a|<1`. At stationarity, squared correlation with the input `k` steps earlier is:

```text
R_k^2 = b^2*Var(u)*(1-a^2)*a^(2*k) / (b^2*Var(u)+Var(eta))
```

Increasing retention redistributes this scalar measure across lags: its sum over all nonnegative lags is independent of `a`, while its maximum at a particular positive lag occurs at `a^2=k/(k+1)`. State variance also grows as `a` approaches one. This is conditional probability arithmetic, not a capacity bound for this SNN or a proposed gain-setting algorithm. Adding observation noise changes the result; neither quantization error nor the adaptive background regulator is automatically independent additive noise.

A paired-history comparison and reliable single-run prediction answer different questions. In another scalar reference, two initial values differ by one, subsequent gain is 0.99, and independent Gaussian drive has standard deviation 0.1 per frame. After 100 frames their mean difference is about 0.366. Using identical applied noise in a paired calculation preserves that difference exactly, yet the conditional single-run distributions overlap enough to give about 39.1% error for the optimal equal-prior history classifier. The example does not propose a classifier; it shows why a causal influence can survive without yet supporting reliable contextual prediction. Actual state-dependent noise needs the full feedback comparison described above.

[Schuecker, Goedeke and Helias (2018), §VII](https://juser.fz-juelich.de/record/856457/files/PhysRevX.8.041029.pdf?subformat=pdfa&version=1) separately examine activity correlations, stability and reconstruction of past input. Their main memory target is the population mean of independent stochastic inputs; the remaining input components act as interference. Changing the common input amplitude therefore changes the remembered signal too. A separate comparison retains a white signal against colored background noise. Neither setup establishes that increasing unrelated background drive improves token memory. Their Gaussian signed couplings, tanh response and optimized linear reconstruction differ from this application, and optimal memory is not a universal critical-gain prescription. The transferable question is whether earlier token information improves the required later response under its actual background regulator.

### Individual identities and shared recurrence

H-75 identifies cells of one minicolumn with different contextual paths for the same semantic component. This gives their distinct learned connections a direct function. Numenta's Temporal Memory represents the same column-level input in different temporal contexts by activating different cells within a column. That modeling precedent supports the distinction; it does not select its segment, bursting or learning algorithms for this rate-based application. [Numenta's description](https://www.numenta.com/assets/pdf/temporal-memory-algorithm/Temporal-Memory-Algorithm-Details.pdf).

For an illustrative linear readout D and recurrent map W, two states can satisfy `D(a-b)=0` while `D*W*(a-b)` is nonzero. The same present vector can therefore lead to different subsequent responses through existing cell identities and weights. This is an algebraic possibility, not a demonstrated memory capacity or trained behavior. It does not allocate a new context field. A maintenance rule that replaces every acceptable cell pattern by one canonical pattern could remove this intended distinction; successful natural participation should be credited before repairing deficits.

Loops share neurons and edges. In a three-neuron linear example, one center connects reciprocally to two leaves, each edge having gain 0.8. Either isolated two-edge loop has gain 0.64, but the center's combined two-step return gain is 1.28. Consequently, individually attenuating loops do not establish attenuation of their combined response. The [conditional recurrence arithmetic](../research/recurrent-chain-arithmetic.json) illustrates why maintenance must observe the actual shared graph. It does not justify a global cycle detector or spectral constraint.

The same record shows how a leaky progression can spread activity across successive reference stages. Stage numbers describe traversal order, not physical distance. Spread is not automatically a failure when those stages preserve the same semantic proportions and remain steerable. A sharp moving group is only useful if an actual behavior requires it; no extra clock, refractory mechanism or inhibitory circuit is introduced merely to preserve a diagram's shape.

[Ganguli, Huh and Sompolinsky](https://ganguli-gang.stanford.edu/pdf/08.Memory.pdf) provide a useful conditional comparison: their linear noisy networks can exhibit transient amplification followed by decay, with feedback extending retention and input placement affecting retained information. Their finite-range analysis also distinguishes distributed amplification from unrestricted growth in individual units. These results motivate examining actual entry and body/return responses; they do not prescribe a chain architecture, physical positions, an eigenvector-based input router or a capacity estimate for this nonlinear model. The main memory analysis has been read; supplementary proofs have not.

### Cell composition can change while the semantic value stays fixed

For a nonzero coordinate after H-77 cancellation, write the surviving rates as `r_i=A*q_i`, where `A>0` is their sum and `q_i>=0` sums to one. These are **analysis coordinates derived from the existing rates**, not two fields or an alternative neuron representation. With sixty cells, a fixed total and fixed polarity leave up to fifty-nine continuous degrees of freedom in cell composition. Across 5,120 nonzero coordinates this is 302,080; it is not a count of independent contexts, reliable memory bits or learned capacity. The graph and numerical state must make any such distinctions useful in later responses.

If rates are bounded by a common `R`, those fifty-nine dimensions can remain locally available while the allowed variation shrinks: the largest L1 distance between two patterns of total `A` is `2*min(A,60R-A)`. At total `59R` it is only `2R`; at `60R` every cell equals `R`. The [bounded-composition arithmetic](../research/context-composition-analysis.json) also counts uniform four/eight-bit patterns at fixed totals. One code step from zero or full saturation permits only sixty patterns, whatever the bit width; the physical step differs. These are conditional representation bounds, not a chosen cap or learned context capacity.

[Hennig et al. (2018)](https://elifesciences.org/articles/36774) provide a related empirical caution: in a motor BCI, the distribution of activity invisible to the immediate readout depended on the activity producing that readout. Their within-manifold, short-session result concerns distributions, not per-step prediction or our semantic cells. It supports checking accessible contextual patterns under the required semantic response, without assuming either unrestricted freedom or a fixed biological capacity.

The [exact composition analysis](../research/context-composition-analysis.json) isolates what added drive can do. Starting with positive rates `A*q_i` and no negative active rates, add nonnegative drive `z_i` to positive cells and opposite drive totaling `Z_minus`. If the positive polarity survives, let `Z_plus=sum(z_i)` and `D=A+Z_plus-Z_minus`. H-77 gives:

```text
r_next_i = D * (A*q_i + z_i) / (A+Z_plus)
q_next_i = [A/(A+Z_plus)]*q_i + z_i/(A+Z_plus)
```

Cancellation preserves the composition of the **provisional** rates after drive was added. It need not preserve the composition before that addition. If both polarities receive equal total drive, the signed value stays exactly `A`, while the contextual distribution may change. For two equal-amplitude histories receiving identical realized added drive and retaining the same sign, their L1 composition difference shrinks by `A/(A+Z_plus)` in this isolated operation. This does not describe the subsequent learned recurrent response, which may amplify or restore useful distinctions.

For a concrete deterministic reference, add 0.01 to each of the sixty cells in both polarities, starting from a signed total of 1. The signed total remains 1, while composition is mixed with the uniform pattern with retention factor `1/(1+0.6)=0.625`. Repeating only this operation ten times leaves about 0.0091 of the original composition difference. Equal per-cell addition is not white-noise sampling; this calculation selects no noise distribution or physical time interval. It demonstrates why preserving a semantic sum alone cannot establish contextual preservation.

The same [closed-form analysis](../research/context-composition-analysis.json) connects this effect to frame duration. If each polarity receives total `kappa*Delta^alpha` per operation, the composition-difference ratio over horizon `T` is `[1+kappa*Delta^alpha/A]^(-T/Delta)`. With `alpha=1`, it tends to `exp(-kappa*T/A)` as frames shrink; with `alpha=1/2`, it tends to zero even though the signed total stays fixed. In normalized reference units with `A=kappa=T=1`, changing `Delta` from `1/4` to `1/256` changes square-root-scaled retention from about 0.198 to `1.82e-7`; linearly scaled retention changes from 0.4096 to about 0.369. This is deterministic balanced drive, not a reproduction of white-noise statistics. Compare contextual retention alongside the [chosen frame/noise units](RateDynamics.md#frame-duration-and-noise-units); learned recurrence and actual sampled noise can alter the result.

A precision comparison must also preserve useful differences within a population. In an [exact storage example](../research/context-composition-analysis.json), sixty positive rates are all `3/4` except that the first two are `3/4 ± 2^-13`; two histories swap those perturbations. Both signed totals are 45. FP16 rounds both patterns to the same sixty values, with zero semantic-sum error and only about 0.003% relative rate-vector error. FP32 retains them. A unit nonnegative connection from the first cell distinguishes their unrounded responses by `2^-12`, but cannot distinguish the identical FP16 states.

This does not establish that trained contexts use such small differences or that FP16 is inadequate. Distinct contextual cell identities may encode the required histories much more robustly. It shows why low present-vector error alone is insufficient: compare the later behavior that the differing cell patterns are supposed to support, using the actual learned graph and numerical path.

At zero active total, this composition is undefined and H-77 leaves no surviving active rates in that coordinate. A polarity reversal selects the other cell population without an index correspondence. Context may persist in other coordinates or in whatever predictive interpretation is eventually selected; H-78/H-85 remain open. Neither a scalar sum nor composition entropy alone measures the diversity of complete next-token predictions.

Future comparisons should examine later context-sensitive prediction together with signed residual, activity support and cell-level response under the actual noise and recurrence. The existing rates and edges supply the first candidate mechanism. A separate context store is warranted only if a demonstrated required behavior cannot be achieved adequately with that state; this decomposition introduces none.

### Matching response fading does not match contextual retention

Two-frame publication does not choose how much of each cell's previous state persists. In an [exact four-cell reference](../research/context-composition-analysis.json), let `a1,a2` belong to one positive semantic population and `b1,b2` to another. The only edges exchange `a1` with `b1` and `a2` with `b2`. Thus the example uses cross-minicolumn recurrence; it neither connects opposing cells by index nor requires internal minicolumn edges.

Let `P` perform those exchanges and use the conditional linear transition `T=(1-beta)*I+beta*g*P`. Two histories leave `(1,0,0,1)` and `(0,1,1,0)`. Both have semantic sum one in each population, but different contextual composition. Their semantic sums decay by `q=1-beta+beta*g`, while their cell-rate difference changes by `mu=1-beta-beta*g`.

Choose illustrative `q=63/64` and adjust `g` to preserve it. With `beta=1, g=63/64`, the two patterns alternate while retaining their contrast relative to the fading amplitude. With `beta=1/2, g=31/32`, the same semantic envelope accompanies a relative-contrast reduction by `1/63` each step. With `beta=65/128, g=63/65`, both histories reach exactly the same state after one transition. Every case has the same semantic half-life of about 44 frames; no frame duration is selected.

This isolates nonnegative linear dynamics without noise, thresholds, learning or rounding; absent opposite activity makes H-77 cancellation inactive. It does not show that direct updates outperform relaxation, or that the preserved difference is useful memory. It shows why equal fading curves do not make two update rules equivalent for contextual prediction. Compare their later responses as well as their envelopes before selecting self-retention; no extra state or diagnostic runtime structure follows.

### Active context under a fixed token input

Consider two histories with the same signed coordinate totals but different zero-sum rate distributions within their surviving cell populations. Hold the added token drive identical across histories and apply only additive input followed by H-77 cancellation. This isolates one operation: it omits recurrence, nonlinear transfer, saturation, noise, decay and readiness semantics.

For an old coordinate magnitude `A>0`, same-sign input leaves the **absolute** cell-rate difference unchanged, although its fraction of the larger total falls. Opposing input of magnitude `B` multiplies that difference by `max(0,1-B/A)`. At a polarity reversal, the old active population carries none of that difference; other coordinates can still distinguish the histories. These statements follow from proportional cancellation, without pairing opposite cells or storing a separate context value.

The [read-only Flash calculation](../research/context-composition-analysis.json) reuses 1,024 token types and 4,096 sampled pairs, evaluating both input directions. Each vector is normalized to L1 total one for this diagnostic; incoming strength is multiplied by `beta`. Assume the initial within-population differences have L1 magnitudes proportional to the old coordinate magnitudes. Then:

| Incoming strength `beta` | Median retained L1 cell-rate difference | Median fraction of coordinates retaining an old-polarity difference |
| ---: | ---: | ---: |
| 0.1 | 95.47% | 96.86% |
| 1 | 72.23% | 75.72% |
| 10 | 54.67% | 54.55% |

Keeping the sampled raw tensor rows at a **common scale**, instead of normalizing each row, gives median retained difference **72.51%**. Its 5th–95th percentile range is **65.79–78.84%**, compared with **70.42–74.92%** for equal-L1 inputs at `beta=1`. About **17.05%** of the ordered pairs differ by more than five percentage points between these two calculations. Similar medians therefore conceal effects of relative input strength. This compares the same isolated operation and hypothetical history differences; it does not select normalization or predict performance of the actual encoder.

These are conditional state differences, not fractions of learned context, prediction accuracy or a memory capacity. The sample is over token types, not observed sequence transitions. Actual H-76 selection may supply different input to different histories, and subsequent dynamics can preserve, amplify or erase distinctions. Do not multiply these pairwise factors to claim a sequence lifetime. H-78/H-85 remain open; no conclusion about predictive readiness follows from this active-rate calculation.

### Cancellation, noise and counted activity

H-77's selected operation preserves the signed remainder and the surviving population's relative rates. Its [derived properties](../research/cancellation-properties-analysis.json) include a 50% ceiling on instantaneous positive-active-rate support for equal polarity populations, and a dependence of contextual results on the cancellation boundary. These consequences should shape the ordinary frame update before considering extra machinery.

The same analysis separates a possible noise-model artifact from the required behavior. Consider fresh noise-only frames with no carried activity, recurrence or other input: each cell receives an independent sample from the same strictly positive continuous distribution, and any positive rate counts as active. Full cancellation leaves exactly one entire polarity population active per dimension, almost surely, so the counted fraction is 50% regardless of a common positive noise scale. Symmetry gives each polarity an equal chance to win; independent frames then give expected cumulative support of 87.5% after three frames. That coverage result does not describe a recurrent trajectory with retained state or biased polarity probabilities. These noise and counting assumptions remain unselected. The example shows why weak nonzero support cannot automatically stand for regulated activity or noise-filtered coverage; it does not justify a new detector or controller.

Cancellation alone does not amplify absolute **L1** perturbations, although relative errors near a zero net can be large. That bound is not Euclidean non-amplification. For two populations of `m` cells, the [derived L2 bound](../research/cancellation-properties-analysis.json) is `||C(x)-C(y)||_2 <= sqrt(2*m)*||x-y||_2`, a sharp worst-case factor of about **10.9545** for 60+60. Near balanced provisional totals, many small input changes can concentrate into a few surviving cells while their total absolute error remains unchanged.

For another exact example, both a focused and a uniform positive population have total 1 against negative total 0.5. Increasing each of sixty negative cells by 0.0001 reduces the signed total by 0.006 in either case. The cell-rate L2 error grows by `sqrt(60)` in the focused case and by one in the uniform case. Cancellation still preserves each positive population's composition. These are deterministic conditional ratios, not typical noise gains or evidence that focusing is undesirable. Match an error bound to the actual observable and norm before propagating it through the network.

[Conditional recurrence analysis](../research/cancellation-recurrence-analysis.json) separates these local properties from amplification, fading and history retention in the whole graph. It also illustrates signed effective responses using only nonnegative connections and the existing cancellation. No inhibitory-edge type, mirrored layout, new tolerance or stability algorithm follows.

### Noise strength in a 60+60 population

Equal noise means in the two polarity groups do not imply a small instantaneous signed disturbance. The [one-frame calculation](../research/noise-feedback-analysis.json) adds independent `U[0,sigma]` noise to every nonnegative provisional rate before exact H-77 cancellation. Let `d>=0` be the coordinate's pre-noise signed sum and `D` its remaining signed sum. In this restricted example, `E[D]=d` and `Var(D)=10*sigma^2`. Exact uniform-sum arithmetic gives:

| Pre-noise remainder `d/sigma` | Probability that the remaining coordinate is negative | Mean unsigned remainder `E[abs(D)]/sigma` |
| --- | --- | --- |
| 0 | 0.5 | 2.52419 |
| 1 | 0.37606 | 2.64915 |
| 5 | 0.05696 | 5.15320 |
| 10 | 0.000758 | 10.00128 |

Thus rising unsigned activity alone does not demonstrate the desired strengthening of semantic residuals. This additive operation leaves the expected signed remainder unchanged; the full recurrent response must be assessed. A weak coordinate changing sign also need not damage whole-vector meaning substantially. For 5,120 independent coordinates with this raw-sum readout, the expected squared vector perturbation is `51200*sigma^2`; fidelity depends on its scale relative to the signal, not a count of sign changes alone.

Signal and readout units matter. With `m` cells per polarity and fixed per-cell noise scale, raw-sum variance is `m*sigma^2/6`; dividing the coordinate readout by `m` gives variance `sigma^2/(6*m)`. A neuron-count comparison must specify which signal magnitude and noise quantity are held fixed. These are conditional moments, without rate clipping, recurrence, quantization or within-frame feedback. They do not select uniform noise, its amplitude, a readout normalization or a controller; retain those choices for real-data comparisons under H-86.

Temporal whiteness does not specify how simultaneous noise is shared among neurons. Global amplitude control likewise does not require a shared random draw. For equal per-cell variance `v`, within-polarity correlation `rho_w` and cross-polarity correlation `rho_b`, the [conditional covariance calculation](../research/noise-feedback-analysis.json) gives signed-sum variance `2*m*v*(1+(m-1)*rho_w-m*rho_b)`. At sixty cells per polarity this is `120*v` for independent cells, `7200*v` for independent draws each shared within one polarity, and zero for a single draw shared across both populations. Each pattern can be redrawn independently over time. These are additive, pre-transfer cases with valid covariance structures, not alternative noise settings already adopted.

Zero net perturbation does not mean unchanged context. Starting with one positive cell at1 and all other cells at zero, add `1/60` to every cell of both polarities. H-77 preserves signed sum1, but leaves the original cell at `61/120` and each other positive cell at `1/120`. Thus equal added totals can change contextual composition despite cancelling from the coordinate sum. Actual nonlinear transmission needs its own comparison.

[Moreno-Bote et al. (2014)](https://pmc.ncbi.nlm.nih.gov/articles/PMC4486057/) show why mean noise correlation alone does not determine task-relevant information. Their balanced spiking network and analytical limits differ from this application. Compare the required semantic and contextual outcomes under declared noise-sharing assumptions; no decorrelation objective, covariance estimator or shared-noise mechanism is implied.

## Accumulating decay instead of losing each small update

The numerical problem is repeated loss of a small decrement, not merely a short integration interval. Changing the interval at which a value is rounded can help only if the intervening evolution is retained or reconstructed. Simply freezing a value for several steps and applying a larger decrement later changes the signals seen by its neighbors.

| Candidate | What it retains | Main question to resolve |
| --- | --- | --- |
| Wider evolving state, lower-width stored weights or emitted values | Small changes directly | Whether its small per-neuron memory cost is preferable to extra scheduling/error machinery. |
| Accumulated residual/error | Updates below one stored quantum | Units, feedback placement, saturation and behavior under sign/cancellation changes. |
| Cumulative elapsed-time evaluation | A decay anchor or equivalent sufficient state | Every read/update must use the correct model-time value; elapsed time cannot be reset without preserving its effect. |
| Scheduled next representable change | The next code-boundary crossing | External input, noise and cancellation can invalidate the predicted time; a scheduler is not free or automatically exact. |
| Stochastic state transitions | The desired drift in expectation plus specified variability | Individual fading and coordinate fidelity can differ substantially from the average. |

These alternatives concern the represented dynamics as well as execution. A physical/model time step, a stored-code update and a deferred event need not coincide. User-visible or host wall-clock time must not silently replace model time; a slower worker should not give the same simulated neuron a different decay law.

Neuron leakage and network-response fading are also different. Recurrent input can replenish a leaking neuron; a pattern can disappear while the CC's total activity remains regulated. A local decay implementation therefore cannot, by itself, impose the desired token half-life or determine the lifetime of useful context.

### Isolated-decay arithmetic

Always rounding the updated positive value downward avoids a fixed code, but changes the fading law. For integer code `k` and `0<a<1`, `floor(a*k)=k-ceil((1-a)*k)`, so zero occurs within at most the initial code count in frames. In the [closed-form reference](../research/numeric-representation-arithmetic.json), a 0.5-second isolated half-life and 1 ms frames give a decrement below half a code throughout UINT4 and UINT8. Nearest rounding freezes every code; downward rounding instead removes exactly one code each frame. Full-scale UINT4 reaches half or below in 8 ms and zero in 15 ms; UINT8 does so in 128 ms and 255 ms. These scalar bounds omit input and recurrence. Reaching zero alone is not evidence of the intended token fading or natural cessation, and neither rounding policy is selected.

The [288-case arithmetic probe](../research/isolated-decay-probe.json) compares uniform unsigned state grids, update intervals, initial magnitudes and two decay scales. It contains no neural graph, input, noise or cancellation. A representative case starts at full scale, uses an intended half-life of 0.5 seconds and updates every 10 ms:

| State magnitude grid | Naive nearest rounding after each update | Retained latent decay progress |
| --- | --- | --- |
| Four bits, codes 0–15 | Remains at 15: every proposed decrement is lost. | Crosses to zero at the first grid observation after about 2.453 s. |
| Eight bits, codes 0–255 | Falls to 36 and then remains there, about 14.12% of full scale. | Crosses to zero at the first grid observation after about 4.497 s. |
| Sixteen bits, codes 0–65535 | Eventually stalls at 36, about 0.05493% of full scale. | Follows the isolated exponential to its quantization boundary, about 8.500 s. |

The sixteen-bit exact boundary lies beyond this case's six-second sampled horizon and is calculated analytically in the same record. These are **unsigned fixed-point arithmetic cases**, not FP16/FP32 results. Wider precision reduces the error but does not make deterministic repeated rounding immune to stagnation. The zero crossing is a representation boundary, not the chosen natural-cessation rule or the token's half-life.

Away from ties, the fixed-point condition is simply `q*(1-g) < 0.5`, where `g` is the per-update decay factor. Keeping the discarded component as part of the evolving latent quantity gives the isolated reference rounded at observation. Adding an un-decayed error after the next multiplication gives a different recurrence and slightly different crossing times. “Retain a residual” therefore needs an equation, not just a field name.

The [floating-state probe](../research/floating-state-decay-probe.json) separately evaluates 96 isolated FP16/FP32 cases. At a 0.5-second half-life and 10-ms update interval, both formats follow the reference closely over three half-lives; FP16 error also depends on coefficient precision. At a 50-second half-life with the same interval, a full-scale FP16 value remains unchanged because each decrement is too small, even when the coefficient is computed accurately. FP32 follows that case without a residual field. This supports comparing adequate precision before adding scheduling machinery; it is not a network-stability or universal FP32-accuracy result.

### Scheduled threshold crossings

H-72's intended event is the **next change that the chosen precision permits**. After applying that change, calculate the following one from the remaining decay progress. The scheduling horizon follows the current value, decay law and representation; it is not a separately prescribed token lifetime. With finer state precision, the next exact code transition may be much sooner, so the scheduler's useful work reduction must be checked for the actual format.

For an isolated positive amplitude following `x(t)=x0*exp(-(t-t0)/tau)`, a representable-code boundary `b` below `x0` is reached at:

```text
t_next = t0 + tau * log(x0 / b)
```

This is conditional arithmetic, not the complete noisy recurrent transition. It provides a concrete interpretation of a future decay trigger: apply the next representable change when the accumulated evolution reaches its boundary. It does not mean deleting a connection or setting a timer that erases a token at a fixed age.

The anchor matters. After a code transition, the true trajectory is at the boundary, not generally at the center of the new code. Restarting decay from that center loses fractional progress and changes the law. A timestamp, residual or other sufficient representation must retain the needed information; choosing among them is part of the design, with its storage cost included.

For the isolated 0.5-second half-life example, a four-bit magnitude starting at 15 reaches the final 0.5 boundary at about 2.453 seconds. Repeatedly restarting at each new integer code center instead reaches it at about 1.396 seconds. The eight-bit versions give about 4.497 and 2.412 seconds. These arithmetic differences concern how successive near-term events are calculated; no distant extinction timer is needed to produce the error.

Likewise, repeatedly postponing a timer whenever a value is read or receives a small update can make weak activity artificially immortal. New activity must change the future evolution consistently rather than simply renewing an old deadline. A delayed event must apply to the current valid state; it cannot blindly subtract from an unrelated newer activation. Required ordering at coincident input/decay events must be explicit.

Frequent noise, recurrent input and cancellation can require frequent rescheduling. Under those conditions a simple periodic accumulated update may cost less than individual event scheduling. The incoming-slot representation also does not automatically provide outgoing lists for notifying every affected target. This is a concrete execution dependency to examine before adopting an event-driven network.

Literal storage-code scheduling becomes particularly fine at higher precision. An isolated normal-range halving crosses 1024 adjacent FP16 intervals or 8388608 FP32 intervals; at a 0.5-second half-life these correspond to 2048 or 16777216 transitions per second for one value. Ordinary frames coalesce many such transitions. A larger behaviorally acceptable event quantum would be a separate approximation choice, not the same as the smallest storage increment. These counts are arithmetic, not an implemented event rate or hardware benchmark.

H-75's frame proposal strengthens the simpler comparison: evaluate decay within the next-state computation using enough precision, and introduce next-change scheduling only if a required slow evolution or measured work reduction warrants it. At the 15-class reserved-slot ceiling, scanning IDs and weights once reads 532462080 logical payload bytes, whereas reading and writing one FP32 state field costs 4915200 bytes. The ratio is about108.33 before indirect source reads and other work. [Work arithmetic](../research/execution-work-arithmetic.json) therefore separates saving scalar decay operations from avoiding connection traversal. No event index, queue or timer array follows from these numbers.

A precise next event can be predicted for an interval with a known evolution law. New sampled background input changes that law and may invalidate the event. The [original QSS formulation](https://www.fceia.unr.edu.ar/~kofman/files/qss.pdf) retains underlying continuous state and quantizes its communicated value with hysteresis. Its convergence result assumes Lipschitz dynamics in a non-saturated region and vanishing state-quantization error, including the hysteresis contribution. The proof's transient bound depends on the observation horizon; convergence as error shrinks is not a uniform accuracy guarantee for a fixed format over arbitrary durations. It does not establish that a fixed four-bit timer scheme is accurate for a noisy nonlinear SNN. Literal continuous white noise also falls outside a bounded piecewise-constant input assumption; the proposed discrete-time model must specify how its noise is sampled.

### Inputs and the numerical state carried by a timestamp

A stored code can remain unchanged while an input changes its next decay event. The [scalar timing arithmetic](../research/decay-input-timing-arithmetic.json) uses exact rational evolution with `x[n+1]=0.999*x[n]`, initial value 10 and unit-spaced nearest codes. With no input, code 10 first changes at frame 52. Adding 0.05 at frame 20 leaves code 10 unchanged but moves that event to frame 57; adding 0.2 moves it to frame 72. These are conditional scalar cases, not neural response measurements. A scheduler must retain the input's effect even if it does not immediately change the emitted code.

For constant drive, the affine recurrence `x[n+1]=a*x[n]+u`, with `0<a<1`, has `x[n]=x_eq+a^n*(x[0]-x_eq)` and `x_eq=u/(1-a)`. A boundary may never be crossed if the trajectory approaches an equilibrium inside the current code interval. With varying input, each contribution has its own age and decay factor. A sufficient running amplitude can accumulate those effects without retaining the entire input history; repeatedly rounding that amplitude can still lose progress. Equality at a quantizer boundary also needs a specified tie rule when converting a continuous crossing time into a discrete frame.

A timestamp can itself encode fractional state. Under the known unforced exponential law with finite next lower crossing `T` at boundary `b`, the present amplitude is `x(t)=b*exp(lambda*(T-t))`. Thus a precise timestamp can replace some amplitude information, rather than being merely administrative data. This representation is not always sufficient: latent values 0.2 and 0.4 both round to zero on a unit grid and have no further unforced code change, yet adding 0.2 makes their codes 0 and 1 respectively. Deliberately treating both as true zero is a distinct model alternative; it must be evaluated for natural cessation and later reactivation.

Timestamp precision and bytes therefore belong in the comparison. One packed four-bit magnitude plus one 32-bit timestamp costs 2,764,800 bytes for 614,400 neurons, versus 2,457,600 bytes for one FP32 field. These named-array cases exclude phase and dispatch information and prescribe no timer layout. An absolute FP32 time in seconds also becomes coarser as elapsed time grows: its spacing is 7.8125 ms near one day and 250 ms near thirty days. For a conditional 0.5-second scalar half-life, half-spacing timestamp error can produce reconstructed-amplitude relative errors up to about 0.543% and 18.9%, respectively. Relative time origins or integer ticks change the trade-off; no clock format is selected.

### Scheduling within the frame model

A local decay event and the recurrent input computation have different dependencies. With incoming gathers retained, a target's future decay estimate does not establish that its sources, background drive or cancellation remain unchanged. Avoiding a connection scan needs a justified way to know that the relevant input is unchanged; the estimate alone supplies none. This does not mandate a reverse index, dirty map or queue. Compare saved work with event maintenance under the actual input cadence before adding any such structure.

Sparse source changes do not imply equally sparse affected destinations. In a [conditional dependency count](../research/execution-work-arithmetic.json), independently changing each source with probability `f` gives probability `1-(1-f)^d` that a row with `d` distinct influential sources sees at least one change. At `f=0.001`, that probability is about 9.52% for `d=100`, 63.23% for `d=1000`, and almost 100% for `d=18432`. These are actual dependency counts, not reserved capacities; changing values is distinct from being active. This is neither a workload measurement nor a lower bound on recomputation: thresholds, compensating changes and an eventual justified incremental algorithm can change the work.

Cancellation can spread a provisional change within its dimension. In the same exact reference, sixty positive rates of 0.5 face negative total 15, leaving each positive rate at 0.25. Adding 0.05 to one positive cell changes the common retention factor and all sixty completed positive rates. A local decay event therefore does not isolate all downstream changes. Compare actual dependency overlap and completed-frame behavior before adopting event machinery; no reverse index, cached sums or change map is presumed.

H-75's completed-frame boundary remains the consistency rule. A scheduler may help identify a future update frame under a known local law, but immediate publication that changes what later destinations read would change the model. The [dependency analysis](../research/quantized-frame-dependency-analysis.json) also distinguishes exact local exponential timing from first-order QSS: the latter evolves its latent state using a quantized derivative input, which is constant between events. Reading quantized source values is an approximation choice even when local event times are calculated exactly.

Small local errors can accumulate through recurrence. In a conditional fixed transition that is `K`-Lipschitz, with at most `epsilon` next-frame error and equal initial states, the difference after `n` frames is bounded by `epsilon * sum(K^j, j=0..n-1)`. This is a worst-case bound under specified driving conditions, not a requirement for contraction or a prediction of learned error. Discontinuous phase rules, state-dependent noise regulation and changing learned weights require their own analysis. Exact timer arithmetic alone cannot establish faithful token fading or retained context.

An average retention factor is not a pointwise error bound. For the scalar reference `e_next=a*e+xi`, let independent zero-mean error innovations have variance one and let `a` be independently chosen as 0.1 or 0.9 with equal probability. The stationary variance is `1/(1-E[a^2])=100/59`, about 1.695; substituting the mean `E[a]=0.5` instead gives only `4/3`. [Exact periodic examples](../research/quantized-frame-dependency-analysis.json) also show that ordering the same coefficients changes phase-dependent accumulated error. Actual rounding need not satisfy these independence or zero-mean assumptions. Assess the actual transition and rounding behavior before transferring a scalar variance estimate; no runtime error estimator or contraction requirement is inferred.

### Deferred arithmetic must preserve cancellation

Accumulating decay progress does not permit delaying the frame's active-rate cancellation. In an [exact four-population reference](../research/quantized-frame-dependency-analysis.json), activity from one positive population sends strengths 2 and 1 into the positive and negative populations of another minicolumn. Cancellation leaves strength 1 on the positive side. If both receiving populations connect back with unit weight, normal publication gives returned activity 1; allowing the uncancelled intermediate values to transmit instead gives 3. All illustrated connections are nonnegative. A common multiplicative decay on each step scales both answers and does not remove the discrepancy.

Some attenuation can be factored algebraically under narrower conditions. Proportional cancellation commutes with a common nonnegative factor, including a factor shared by every cell and both signs of one minicolumn. A diagonal decay operator `D` commutes with a fixed linear transmission `W` only when `(D_ii-D_jj)*W_ij=0` on every edge. Even for a homogeneous map `F(x)=C(W*x)`, moving a common factor across repeated steps leaves `F^n`, including every cancellation; it does not replace those steps by `C(W^n*x)`. Additive drive, thresholds, quantization and feedback need their own treatment. This supplies conditions to check, not a selected decay placement or batching scheme.

A numerical remainder representing sub-precision evolution is also different from cancelled activity. H-77 makes cancelled active rates zero before publication; silently restoring them from a retained pre-cancellation value would change the dynamics. Predictive readiness remains open under H-85. This distinction neither mandates residual storage nor treats every weak contextual trace as disposable.

### Integer stochastic decay as a distinct model candidate

For a toy state containing `q` independent activity quanta, each surviving interval `Delta` with probability `p=exp(-lambda*Delta)`, the next count is binomial with mean `q*p` and variance `q*p*(1-p)`. This supplies exponential decay in expectation and eventual stochastic extinction without deleting learned connections. These quanta describe a possible numeric/activity model, not additional neurons or a claim about actual spikes.

At half survival, relative standard deviation is `1/sqrt(q)`: about 25.8% for `q=15`, 6.26% for `q=255`, and 100% for `q=1`. Thus four-bit and eight-bit versions can have very different single-response variability even when their mean decay is identical. Coordinate aggregation may reduce variability under suitable independence and weight assumptions; it is not guaranteed in the recurrent graph. Controlled background noise and this internal variability must not be conflated or counted twice.

### Matching the mean does not select a stochastic update law

The [closed-form comparison](../research/stochastic-decay-law-analysis.json) contrasts independent survival with independent Poisson reproduction. Both start with fifteen abstract activity quanta and use mean retention `m=2^(-0.01/0.5)` every 10 ms. At 0.5 seconds both have expected count 7.5, but their distributions differ:

| Conditional law | Count variance at 0.5 s | Probability of a nonzero count | Mean count among surviving cases |
| --- | ---: | ---: | ---: |
| Each quantum survives or disappears | 3.75 | 99.997% | 7.5002 |
| Each quantum produces Poisson(`m`) descendants | 272.38 | 32.485% | 23.09 |

The second law permits growth beyond fifteen, so it is **not a four-bit implementation** merely because its initial count is fifteen. The same average fade can hide frequent extinction together with large surviving bursts. These are analytical distributions, not trained neural responses, biological counts or selected randomness. Natural cessation and useful residual context must therefore be assessed at the required trajectory level as well as through an average envelope.

Frame cadence changes this comparison. At fixed 0.5-second duration and mean half-life, independent thinning composes to the same distribution. Repeating Poisson reproduction more frequently does not: changing its frame from 100 ms to 1 ms changes the survival probability from about 94.68% to 4.04%, while preserving the same expected count. Its per-quantum variance remains of order one at every small step. A consistent physical-time fluctuation law needs more than rescaling mean retention; in this reference, offspring variance proportional to the frame interval can avoid that divergence. No branching or continuous-time model is adopted.

Clipping is another part of the law. For one Poisson update from fifteen quanta at the same retention, the unclipped mean is about 14.7935. Capping the result at fifteen lowers it to about 13.3651; approximately 41.08% of draws exceed the cap. A narrow bounded representation therefore cannot inherit the unbounded mean-decay result unchanged. Compare the actual saturation and update rule, including its weak-response behavior, rather than only the nominal bit width.

Background drive can sustain counts while the original influence fades. In a conditional Poisson branching model with constant independent immigration, stationary mean is `h/(1-m)`, while the expected descendants of an initial group still decline as `q0*m^n`. Zero is no longer absorbing when new immigrants can arrive. This ancestry decomposition is mathematical; it requires no stored origin tags and is not the application's adaptive noise regulator or semantic readout. [Wilting and Priesemann (2019), Supplement 3](https://arxiv.org/html/1804.07864v2) gives the branching-with-immigration moment framework. It supplies a useful reference distinction, not a target branching ratio or evidence that a maintained count preserves useful semantic history.

## Integer formats are field-specific choices

“INT8” or “INT4” must identify signedness, scale, zero, saturation and interpretation. A nonnegative magnitude may use UINT8/UINT4, whereas a signed residual has different requirements. Fixed polarity identity supplies a coordinate sign, but does not provide an independent prediction/activation phase field for free.

Integer storage can represent fractional quantities through a scale. It does not eliminate quantization. Uniform steps may suit bounded rates or recruitment fractions; floating formats may better cover a wide range of magnitudes. Weight scale as a training policy remains separate from the representational scale used to interpret an integer code. Scale metadata and conversion work must be counted if selected.

The [exact arithmetic record](../research/numeric-representation-arithmetic.json) preserves the current 15- and 16-class baselines and conditional field substitutions. With 614,400 neurons, the state-array sizes are:

| Bits per neuron | Array bytes | Increase from four bits |
| ---: | ---: | ---: |
| 4 | 307,200 | 0 |
| 8 | 614,400 | 307,200 |
| 16 | 1,228,800 | 921,600 |
| 32 | 2,457,600 | 2,150,400 |

These are one-array arithmetic cases, not complete state layouts. A wider evolving quantity may therefore be inexpensive relative to hundreds of MiB of connection arrays. Whether it also meets cache/work requirements needs evidence; narrow storage is not automatically the best whole-system choice.

H-73 explicitly opens FP16 and FP32 for neuron state. One such field takes 1.171875 MiB or 2.34375 MiB respectively. They are serious candidates for direct evolution; low-width weights, emitted quantities and state need not use the same representation. Phase semantics, any independently needed fields and intermediate arithmetic still require discussion. A full-range unsigned four-bit magnitude comparison also does not supply the prediction/activation phase codes that must fit somewhere in a complete state representation.

H-75's two frames double only the evolving state fields: one FP16 field in each frame totals 2.34375 MiB, and FP32 totals 4.6875 MiB. The single connection pool dominates the current named-array budget. A sufficiently precise state also does not decide what it means: literal emitted rate, readiness and current activation need an explicit interpretation before adding fields.

Small state-array RAM cost does not imply negligible gather cost. As a hardware reference, the [BCM2712 specification](https://www.raspberrypi.com/documentation/computers/processors.html#bcm2712) lists a 2 MB shared L3 cache. One FP32 source field at the current population is 2.34375 MiB; FP16 is 1.171875 MiB. A narrower source working set could help irregular reads, but fitting by size does not prove residency or speed: streaming connection arrays, destination writes, other fields and concurrent CCs compete for cache. The [footprint arithmetic](../research/execution-work-arithmetic.json) motivates measurement alongside semantic error, without choosing narrow state or an extra readout buffer.

For the 15-class candidate, replacing one-byte weights by packed four-bit weights reduces the named-array sum from 540,174,772 to 486,928,564 bytes before scale metadata or other changes. Four-byte endpoint IDs remain, so connection-slot storage falls from five to four-and-a-half bytes: a 10% reduction, not a halving of the connectivity budget. INT8 weights occupy the same bytes as FP8 weights; their possible benefit concerns representation and arithmetic rather than those storage bytes.

Accumulator width is another decision. For the 16-class maximum capacity of 18,432, the raw bound `18432*255*255=1,198,540,800` fits signed 32 bits before other terms. Multiplying three unscaled eight-bit factors before summation instead gives `305,627,904,000`, which does not. A fraction, rate and weight must therefore have an explicit rescaling/accumulation order; eight-bit operands do not imply eight-bit or even universally 32-bit intermediates.

Integer substitutions also need exact boundary semantics. For integer `k`, a strict test `k > theta` is equivalent to `k >= floor(theta)+1`; a nonstrict test uses `k >= ceil(theta)`. At `theta=2`, substituting the latter for the former changes the result for `k=2`. Likewise, shifting nonnegative code 3 right by one gives 1, whereas nearest-even rounding of `3/2` gives 2. The [scalar arithmetic record](../research/numeric-representation-arithmetic.json) verifies these distinctions. A shift can implement a chosen rounding rule; matching the nominal half-strength coefficient alone does not establish the same decay or threshold behavior.

Opposite-polarity cancellation, small training changes, decay and threshold decisions need error analysis at their own units. Choosing integer arithmetic does not remove the earlier stagnation problem: discarding a sub-quantum decrement can still leave a stored value unchanged forever. Hardware speed cannot be inferred from bit width alone, especially for packed four-bit values and sparse, indirect source access.

### CPU operands and weight storage

Low-width weights can be combined with floating-point rates; narrowing the evolving state is not required. The [Arm intrinsic mappings](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html) distinguish byte-by-byte dot products with 32-bit accumulators from half-to-single conversion. A byte dot product needs byte source operands as well as byte weights. Our incoming IDs still require fetching and arranging the corresponding source values; packed four-bit weights also need interpretation. These costs accompany the storage saving. The [conditional operation record](../research/execution-work-arithmetic.json) compares these paths without prescribing a vector kernel or another source array.

Half-precision storage, half arithmetic and half-input FP32 accumulating instructions are separate capabilities. The [A76 r4p1 register descriptions](https://documentation-service.arm.com/static/5f562083235b3560a01e03bc) and inspected LLVM feature definitions do not advertise FP16FML, despite its listing in the older optimization table. [Linux exposes separate capability flags](https://docs.kernel.org/arch/arm64/elf_hwcaps.html). A comparison must use the actual target's supported operations; widening stored values before adequate-precision products remains a valid alternative. No Pi instruction throughput or sparse-gather speed was measured.

A common weight scale can be factored outside an incoming sum in exact arithmetic. One common source scale additionally permits an integer-code dot product; independently varying source scales do not reduce to that same operation. Conversion and reduction rounding can still change the result. Scale choice remains part of the numerical comparison, separate from capacity allocation and training weight-strength policy.

### Weight range, collective gain and training precision

State-encoding errors do not determine weight errors. The [weight arithmetic probe](../research/weight-precision-probe.json) uses a deliberately simple hypothesis: k equal contributing sources, ideal weights `1/k`, and a global maximum representable weight of 0.01. Here k is the contributing-input count, not the allocated capacity, and `1/k` is not a selected training policy. At k=18432, nearest global UINT8 coding yields aggregate gain about 0.723 instead of 1; scaled FP8 E4M3 gives about 1.029; the compared four-bit grids round each equal weight to zero. Changing the scale, distribution or training policy changes this result.

Collective precision can be finer than one edge's code spacing. In that UINT8 case, a mixture of 11364 code-1 edges and 7068 code-2 edges gives unit summed gain for equal source rates. It does not preserve every response when those sources carry different contextual activity. For the chosen global UINT4 grid, a unit gain contains only 1500 positive code units, so it cannot retain 18432 distinct nonzero weights. These examples identify questions about useful weak routes; they do not select a balancing allocator, row scale or connection policy.

The comparison must therefore include trained contextual responses and the distribution of needed weights, rather than just uniform row sums. Representational scaling remains distinct from training's control of effective weight strength under H-24. A per-row scale can be factored outside an incoming sum in exact arithmetic, but its actual need, precision and metadata must be established before selecting it. No scale array is preset.

Changing a shared representational range can alter associations that receive no learning update. In an [exact uniform-grid reference](../research/weight-update-precision-analysis.json), a four-bit range grows from `[0,1]` to `[0,2]` and returns, with nearest-even recoding each time. Unchanged weights initially `1/15`, `3/15` and `5/15` end as `0`, `4/15` and `4/15`. Returning the range does not recover discarded information. A zero weight is not deletion of its endpoint slot. Compare contextual responses and the cost of touching other retained entries when a scale changes; this is separate from intentional training normalization and allocation-only migration. Static scales, alternative formats and any justified higher-precision learning state retain different trade-offs.

Training storage also matters. [BinaryConnect](https://proceedings.neurips.cc/paper_files/paper/2015/file/3e15cc11f979ed25912dff5b0669f2cd-Paper.pdf) uses binary weights during propagation while retaining higher-precision weights for updates; its low-bit propagation result is not a low-bit-only training-memory guarantee. One extra FP32 value per reserved edge would cost 425969664 bytes (406.236328 MiB) in the 15-class candidate. Our deficit-maintenance rule has not selected that storage or an SGD optimizer. Direct code updates, higher-precision working arithmetic and retained update progress have different requirements; none follows automatically from choosing INT8/INT4 weights.

A binary effective weight can also conceal discrete learning state. [Nishi et al. (2021)](https://www.nature.com/articles/s41598-021-97583-y) model serial binary switching elements for potentiation and depression; their simulation retains the elements' intermediate states. Those states affect the next transition even while the effective weight is unchanged. Their retention measure counts cells keeping an assigned MNIST digit after further learning; it is not contextual sequence readiness. A CPU counterpart would need to preserve the relevant internal distinctions and account for their representation. The paper's synaptic law and additional state are not selected here.

[Gupta et al.](https://proceedings.mlr.press/v37/gupta15.pdf) likewise separate low-width stored values from a wider temporary sum before rounding. Their MNIST DNN result with eight fractional bits still uses **16-bit words**. Their successful fixed-point image-network training does not establish four-bit/eight-bit recurrent fidelity or CPU speed here. The practical first comparison remains adequately precise neuron state and local sums, with weight formats evaluated against the same learned behavior and their full costs.

A precision comparison has two distinct purposes:

| Comparison | What it establishes |
| --- | --- |
| Convert an already learned graph and assess with learning held fixed | Sensitivity of those acquired behaviors to the declared representation, arithmetic and any state conversion. Keep input timing and noise meaning comparable. |
| Construct or adapt the graph using the candidate arithmetic | Whether the candidate can acquire and jointly retain the required behaviors, with its actual updates and any additional learning state. Record training exposure and work as well as inference cost. |

Poor direct conversion is evidence about that learned graph, rather than a general impossibility result for the format. Conversely, recovery through additional training is an acquisition result with an associated cost; it does not show that conversion was harmless. Compare the same behavioral goals—fading, useful context, pre-input matching and steerability—even when learned weights or individual trajectories differ. A format that preserves token fading but destroys flow discrimination has not preserved the shared model.

Finer storage can also change behavior that a coarse-trained network relies on. The [Erbas et al. (2026) preprint](https://arxiv.org/html/2609.04490v1#S3.SS4) reports worse task error when its fixed four-bit-trained GRU receives wider state storage; matched training changes the comparison between memory allocations. This is evidence from a small fluorescence-inference task, not an SNN result. A wider arithmetic reference helps expose numerical error in a specified calculation; it is not a universal behavioral winner. State-change counts alone also do not establish useful memory.

[Jacob et al., author version §3](https://arxiv.org/html/1712.05877v1#S3) make quantization visible during training while retaining floating-point parameters for small updates. Their image-network result supports distinguishing these comparison purposes. It does not select backpropagation, a master-weight copy or a training schedule here; any such cost remains explicit.

[MINT (Yin et al., 2024), §§III–VI](https://arxiv.org/html/2305.09850v4), evaluates low-bit vision networks primarily over four or eight timesteps. Its multiplier reduction uses binary spikes, a half-strength leak and shared weight/state scales; training uses floating-point gradients, and hardware costs come from accelerator synthesis and simulation. Binary spikes permit conditional additions where graded rates still require weighted products. The inspected [VGG code](https://github.com/RuokaiYin/MINT_Quantization/blob/main/quant_net.py) resets state for each input and repeatedly supplies its static-image drive. Those conditions differ from one-presentation semantic recurrence. This evidence motivates a representation comparison but does not establish our long-context fidelity, whole-CC memory saving or CPU speed; deployed integer equivalence was not independently verified.

### Product, reduction and cancellation precision

The [floating-reduction arithmetic](../research/floating-reduction-analysis.json) separates stored operands, product precision, reduction order and final writeback. With 16,384 equal weights `2^-14` and unit source rates, the exact sum is one. Rounding to FP16 after every sequential addition stalls at **0.125**; the specified pairwise FP16 reduction and FP32 sequential sum both give one. With 18,432 copies of FP16-rounded `1/18432`, the exact sum of the stored operands is **0.999755859375**; sequential FP16 again gives 0.125, while sequential FP32 preserves that stored-operand sum. These are arithmetic references, not a choice of FP16 weights or a measured gather implementation.

Products can disappear before reaching a wide accumulator. The exactly represented FP16 operands `2^-10` and `2^-20` have product `2^-30`; rounding that product to FP16 gives zero, while computing it in FP32 retains it. Every finite binary16 product fits exactly in binary32, but a long binary32 sum can still round. The arithmetic contract must distinguish widening before multiplication from widening an already rounded product. Format conversion, runtime weight scales and saturation need the same attention.

H-75 snapshots fix which source frame is read; floating-point addition remains order-dependent within an incoming reduction. Summing a strong contribution before many tiny ones can lose them, while first combining the tiny terms may retain an observable increment. Reordering for traversal locality or parallel execution therefore needs a behavioral error assessment. The examples do not require bitwise reproducibility or a particular reduction tree. Under a common representational scale, a sufficiently wide integer accumulator can avoid this floating reduction error, but it cannot restore information lost when operands were quantized.

The minicolumn operation also needs suitable intermediate precision. Consider positive rates consisting of fifty-nine ones plus `2^-10`, against negative rates consisting of fifty-nine ones and zero. All operands fit FP16. FP16 sequential totals both round to 59 and erase the residual; FP32 totals retain it. Replacing `2^-10` by `2^-20` makes even the specified FP32 totals coincide, while wider reference arithmetic retains the difference. This concerns coordinate polarity, not semantic valence.

Even correct totals do not settle final state precision. For the second case, proportional retention gives each of the fifty-nine unit-rate cells about `1.6164e-8`. Writing those rates to FP16 erases the surviving population; FP32 preserves the aggregate to the reported arithmetic accuracy. Whether such a small residual matters is an empirical behavior question under H-86, evaluated against later amplification and context use. The example identifies where loss can occur without mandating its retention at every scale.

Evaluation order also matters after the totals are correct. H-77 leaves a one-sided population unchanged: `D/P=1`. In a [scalar rounding reference](../research/cancellation-properties-analysis.json), one positive FP16 rate `2^-14` and no negative activity should survive exactly. Computing and rounding `p*D` first gives zero, so the subsequent division cannot recover it; computing `D/P` first gives one and preserves the rate. FP32 has the same failure at `p=2^-80`. These checks use nearest-even rounding with gradual underflow. The mathematical formula does not prescribe a harmful instruction order. Other errors, final writeback and actual floating-point modes remain separate; no extra residual field or artificial activity cutoff follows.

Local wider products or totals can be compared without allocating a wider persistent field for every intermediate. FP32 remains a practical research candidate; sensitive cases should be assessed against an adequate wider reference. A compensated sum, residual array or specialized reduction is justified only if a required behavior or measured execution cost calls for it.

### Rounding before and after cancellation

Unbiased stochastic rounding of individual provisional rates does not commute with cancellation. In the [exact finite-distribution example](../research/quantized-frame-dependency-analysis.json), one positive and one negative rate both equal `p*q`, where `0<p<1` and `q` is the first nonzero quantization level. Exact cancellation followed by rounding gives zero. If each rate is independently rounded to zero or `q` first, each one-sided survivor has probability `p*(1-p)`. Expected signed remainder stays zero, but expected unsigned activity becomes `2*q*p*(1-p)`.

For sixty cells per polarity at the same provisional rate, the independently rounded population counts are two `Binomial(60,p)` variables. At `p=0.5`, exact enumeration gives a nonzero net in about **92.73%** of cases and mean unsigned remainder **4.3611*q**, although the unrounded provisional state cancels completely. These quantities are measured immediately after cancellation, before any further final-state rounding. They describe this independent-rounding law; deterministic or correlated rounding has different behavior.

The two-frame snapshot rule does not choose this numerical order. Storing narrow provisional values into the next frame before cancellation and narrowing only its final surviving rates can implement different transitions while both read the same previous frame. Compare the two orders against adequate reference arithmetic under the same modeled background input. Incidental rounding fluctuations are not automatically the intended controllable white noise. This calls for explicit conversion boundaries, without selecting stochastic rounding, a format, polarity pairing, another frame or a persistent correction field.

### Recurrent consequences of learning and rounding

The [shared-weight analysis](../research/shared-learning-recurrence-analysis.json) connects numerical updates to retained behavior. In a conditional linear two-unit loop, association increments 0.4375 and 0.46875 yield two-step gains 0.99 and 1.015, respectively. This illustrative grid is not a definition of FP8. It shows why a wider neuron state cannot compensate for a different stored connection graph, and why a quantized learner must be assessed with the weights it actually uses.

Unbiased coefficients do not imply an unbiased repeated response. If a positive scalar gain `G` is drawn once and held for `n` laps, the mean response is `E[G^n]`, generally different from `E[G]^n`. Independent resampling each lap instead gives the latter under independence. For equally likely gains 0.97 and 1.01, the mean gain is 0.99, but the mean fixed-realization response after 100 laps is about 1.376, versus 0.366 for gain 0.99. This is conditional scalar arithmetic, not a claim that stochastic learning follows that particular sampling scheme. It motivates examining individual learned trajectories and persistent weight realizations, rather than relying only on an unbiased one-step update or an ensemble mean.

The same distinction has a path interpretation in a nonnegative linear graph with independently rounded weights held fixed. A path that visits each edge once has the intended mean product; repeating an edge introduces a higher moment of that coefficient. Thus an unbiased linear acyclic-path calculation does not transfer automatically to recurrent walks. In the [exact two-edge reference](../research/shared-learning-recurrence-analysis.json), each desired weight `29/30` is rounded once to `14/15` or `1` on a uniform four-bit grid. A quarter of the stored graphs have both weights one and no lap attenuation, although the mean-weight graph fades. This is not a trained SNN or evidence of unsteerability; nonlinear response and dependent learning weights require separate assessment.

Low-bit recurrent computation remains a valid comparison direction. [Hubara et al.](https://jmlr.csail.mit.edu/papers/volume18/16-456/16-456.pdf) report low-bit RNN/LSTM results, while their described training method accumulates updates in real-valued weight variables. [Michaelis et al.](https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2022.1015624/full) report exact recurrent emulation without plasticity. Their learning comparison instead uses one plastic synapse with negligible postsynaptic influence and a separate strong spike-driving input. Its reported trace/weight agreement does not establish agreement during coupled recurrent learning. These are different training/numerical arrangements. Neither selects a master-weight array, a trace mechanism or an integer learning rule for this application.

### Repeated learning on a low-bit grid

Low-bit inference and low-cost persistent learning are separate claims. The [bounded-synapse review](FlowLearning.md#bounded-synapse-memory-is-a-separate-comparison) identifies evidence that depends on extra learning variables and ideal-observer retrieval. Such variables must earn their full storage and update cost through the required sequence behavior; they are not implicit additions to the connection slot.

One-step unbiasedness and an ideal stable learning rule do not settle the stored association. The [exact scalar analysis](../research/hebbian-update-law-analysis.json) uses `w_next=(1-eta)*w+eta*y`, with binary observations `y`, normalized weights in `[0,1]`, nearest-code writeback, ties to even and no retained residual. On a uniform grid with `K` intervals, every code freezes when `eta*K<1/2`. Thus the illustrative `eta=1/64` freezes all UINT4 codes; `eta=1/1024` freezes all UINT8 codes. These are particular arithmetic combinations, not a rejection of integer learning.

At UINT8 `eta=1/256`, code 128, about 0.502, is unchanged by either observation. Codes below it can only move upward, and codes above it can only move downward. With independent binary targets having any fixed probability strictly between zero and one, the weight eventually absorbs at this code, regardless of that probability. In a full-range E4M3 reference, with finite positive values divided by 448, repeated positive updates at `eta=1/64` stop at about 0.286. Other scales change the result; the artifact distinguishes this case from restricting raw E4M3 values to `[0,1]`. A plateau introduced by rounding is not a learned probability or a successful natural forgetting mechanism.

Unbiased stochastic rounding removes that particular frozen-update behavior, but has another conditional consequence. On the uniform grid, let `y` be independent Bernoulli observations with probability `p`, and assume `eta*K<=1`. The code `n` moves up with probability `p*eta*(K-n)` and down with probability `(1-p)*eta*n`. Detailed balance gives the stationary distribution `n~Binomial(K,p)`, hence:

```text
E[w] = p
Var(w) = p*(1-p)/K
```

In this regime, reducing `eta` slows the changes but does not reduce that stationary variance. The unquantized affine reference instead has variance `eta*p*(1-p)/(2-eta)`. At `p=0.01`, the UINT4 reference spends about 86.0% of its stationary time at zero, while the UINT8 reference spends about 7.71%. The mean is correct in both cases. These are fixed-assignment scalar results, not trained prediction rates; coordinated associations and recurrent dynamics can differ. A zero weight is also not automatically an unassigned slot.

Independent rounding draws do not make learned weights independent. Extend the same affine reference to `m` weights receiving the same independent Bernoulli observation on each exposure, but separate ideal rounding draws. The [exact moment analysis](../research/weight-update-precision-analysis.json) gives individual variance `v=p*(1-p)/K`, distinct-weight covariance `c=eta*p*(1-p)/(2-eta)`, and variance of their mean `c+(v-c)/m`. At `K=255`, `eta=1/256` and `m=100`, assuming independent weights understates that variance by about 50.4 times. The dependence comes from shared observations even though rounding is independent.

The unquantized affine learner under the same shared observations also has variance `c`. Thus the common component is not a quantization defect; averaging more weights only reduces the independent component in this reference. Actual source credits, target assignments and update laws change these relationships. The mean is a diagnostic statistic, not an adopted row normalization. Compare exposure-driven fluctuations and arithmetic effects separately before inferring collective precision from edge count; no additional averaging, noise or residual state is implied.

The rounding probability has finite precision too. In a [conditional counting reference](../research/weight-update-precision-analysis.json), an ideal positive update is `p` of one code step. A uniform integer `U` from `0..M-1` gives probability `ceil(M*p)/M` under `U/M<p`, while `U<floor(M*p)` gives `floor(M*p)/M`. With `p=1/18432` and six random bits, these become `1/64` and zero: respectively 288 times the intended probability and no update. Wider storage for the weight does not settle this comparison. The example selects neither a random-bit width nor either finite-grid rule.

Randomness shared across edges also changes collective updates. For 18,432 identical eligible one-code changes, each with ideal probability `1/18432`, the expected number changed is one with either independent draws or one shared draw. The count variance is about one under independence and 18,431 with the shared draw. This is a one-exposure reference without clipping, not a prediction of learned network behavior. Compare the actual probability resolution and correlations when assessing stochastic rounding; its arithmetic randomness is distinct from the application background-noise law.

Before adding retained update state, compare its full cost with simpler wider-weight references. With the 16-class pool and exactly two FP32 neuron-state frames, the [named-array arithmetic](../research/numeric-representation-arithmetic.json) is:

| Weight storage comparison | Total named-array bytes per CC |
| --- | ---: |
| One-byte weights | 573859800 |
| Two-byte weights | 686141400 |
| Four-byte weights | 910704600 |
| One-byte weights plus a four-byte master value per slot | 1022986200 |

The existing endpoint, reference and owner arrays are included; node-shared embeddings, region headers and unspecified workspace are excluded. This does not revise the current five-byte slot proposal or allocate a master array. Wider weights must earn their cost through learning quality or simpler adequate numerics; a master copy must additionally justify keeping both representations. Scan traffic, precision and actual CPU behavior remain separate measurements.

### Prediction-error updates on a quantized row

A stable ideal update can be lost at storage. For the [linear prediction-error reference](FlowLearning.md#prediction-error-modulation-as-a-comparison), let `e=y-y_hat` and use a uniform weight grid with spacing `q`. If every weight is initially on the grid and `abs(eta*e*c_i)<q/2` for every edge, nearest writeback changes none of them. This strict condition avoids tie cases and assumes no boundary clipping. With unit sources and the source-energy step `eta=1/d`, the unchanged-error band has half-width `d*q/2`.

The [exact writeback comparison](../research/hebbian-update-law-analysis.json) starts with `d=18,432` retained endpoints, zero equal weights, fixed unit source values and target one. Working arithmetic is exact. Each ideal weight after the update is `1/d`; repeating this identical input returns to that same ideal value before rounding. The first rounded row is therefore a fixed point of this restricted learning map, even with a remaining error:

| Stored weight representation | Raw row response after writeback; target is 1 |
| --- | ---: |
| UINT4, maximum 0.01 | 0 |
| UINT8, maximum 1 | 0 |
| UINT8, maximum 0.01 | 0.722824 |
| Unscaled E4M3 | 0 |
| E4M3 scaled to maximum 0.01 | 1.028571 |
| FP16, stored directly | 0.999755859 |
| FP32, stored directly | 1.000000007 |

These are diagnostic scales and fixed source patterns, not weight policies inferred from slot capacity. Different inputs, initial weights, growth and response laws can change the result. The integer/scaled-E4M3 values agree with the earlier static weight calculation; the new point is that this particular learning update does not remove the residual. Floating storage also needs a defined scale, intermediate precision and actual contextual assessment.

Increasing the step does not necessarily resolve the error. With the same UINT8 maximum-0.01 row and `eta=1.5/d`, the unquantized error multiplier is −0.5, but the stored row alternates between responses **0.722824 and 1.445647**. This is an exact cycle of weight updates across repeated exposures, distinct from the useful recurrent activity loops required by H-71.

Independent stochastic rounding gives a different first update. On the UINT8 maximum-one grid, the row response has mean one and standard deviation about **0.0622**, with an expected 255 nonzero weights among the retained endpoints. Using one shared rounding draw instead preserves that mean but raises the standard deviation to about **8.443**. Neither rounding policy is selected; the artifact states the exact distributions. A correct aggregate mean does not preserve every source-specific association or establish later stable learning. Compare actual stored updates and the existing wider-weight references before introducing persistent residuals or master weights.

Nonnegative bounds also affect an otherwise unbiased update. For the same 18,432 unit sources, target one, grid `q=1/255` and step `1/d`, compare rows containing either 256 weights of code one or two weights of code 128; other retained endpoints have zero weight. Both initially predict `256/255`. After clipping the negative update at zero and independently rounding, their expected total decreases differ by **128 times**: only positive weights can move downward. The mean waits for the first changed code are about 72.5 and 9,216 exposures, respectively; these are not convergence times. The arithmetic records the conditional law. It shows why source energy, current prediction error and unbiased rounding alone do not determine bounded learning behavior. No constraint-aware step rule or additional state is implied.


### Conditional input-magnitude measurements

The [embedding precision probe](../research/embedding-precision-probe.json) reads the existing SSD Flash-1017 table, using 2,048 deterministically sampled ordinary token types and eight separate examples. For this diagnostic only, each row's magnitudes are divided by its own largest magnitude; signs and row scale are preserved externally. No SNN input encoder, runtime per-token scale or phase representation is selected.

| Conditional representation | Median relative L2 error | Median coordinates rounded to zero |
| --- | ---: | ---: |
| Whole-cell count out of 60 | 1.831% | 2.539% |
| Uniform UINT4 magnitude | 7.321% | 10.137% |
| Uniform UINT8 magnitude | 0.431% | 0.586% |
| Scaled FP4 E2M1 magnitude | 11.871% | 12.656% |
| Scaled FP8 E4M3 magnitude | 2.647% | 0% |
| FP16 magnitude | 0.0207% | 0% |

These results compare a single encoding under the stated normalization, not learned weights or recurrent dynamics. The positive FP4/FP8 element values follow [OCP's format definitions](https://www.opencompute.org/documents/ocp-microscaling-formats-mx-v1-0-spec-final-pdf); the diagnostic real-valued row scale is not the MX block-scale format. Unsigned magnitudes use more nonnegative codes than ordinary signed floating elements. Changing scales, clipping, value distributions or phase-code allocation can change the comparison. No CPU speed or semantic-task result follows from the table.

Under this normalization the separate space-token example has about 32.96% UINT4 error and 1.92% UINT8 error, versus the smaller type-sample medians. Its largest coordinate sets a coarse common scale for the rest. This is a reason to assess representation against actual distributions; it does not prescribe a per-coordinate scale array.

Sixty cells can, arithmetically, offer finer aggregate resolution than a single cell. With uniform four-bit magnitudes their total has 901 levels, yielding mean increments of `1/(60*15)` if contributions can be balanced. The corresponding ideal sum probe has median relative L2 error about 0.122%. Identical rounding of sixty identical values does not achieve that resolution, and redistributing a quantum among distinct learned neurons changes their downstream effects. Thus the ideal aggregate result motivates a question about fractional recruitment; it does not justify adding a coordinated allocator or replacing individual cell identities.

## Primary-source checks and transfer limits

| Source and present read scope | Relevance and boundary |
| --- | --- |
| [Ganguli, Huh and Sompolinsky, 2008](https://ganguli-gang.stanford.edu/pdf/08.Memory.pdf), main memory analysis pp1–4, including input alignment, transient amplification and finite-range assumptions; supplementary proofs unread | Measures history information in noisy dynamical state, separating retained signal from sustained activity. Its linear/input assumptions do not establish the capacity of our nonlinear signed-population model. |
| [Kofman and Junco, 2001](https://www.fceia.unr.edu.ar/~kofman/files/qss.pdf), formulation, theorem assumptions and parsed Appendix B bound | Quantized-state simulation schedules changes in state rather than merely taking fixed time steps. Hysteresis, underlying state and stability assumptions matter; an arbitrary low-bit decay timer is not automatically QSS. |
| [Jacob et al., 2018](https://openaccess.thecvf.com/content_cvpr_2018/html/Jacob_Quantization_and_Training_CVPR_2018_paper.html), abstract/PDF inference section and author-version §§3–3.1 training | Integer-only inference uses defined scales and wider accumulators. Feedforward model accuracy/performance does not validate long-lived recurrent state in this application. |
| [Erbas, Intes and Pandey, 2026](https://arxiv.org/abs/2609.04490), methods/results and targeted matched-training supplement | Fixed-checkpoint conversion and matched training can rank memory allocations differently. A wider state is not automatically better for a coarse-trained model; auxiliary-memory remedies remain unselected. |
| [Haller, Laitenberger and Akbik, 2026](https://arxiv.org/html/2610.29545v1), recurrence, methods, setup, textual results/limitations and memory appendix | This preprint buffers recent inputs and retains wider scales/decays while rounding a gated-delta state less often. Intermediate outputs still account for those inputs; freezing neural activity is not equivalent. Its mean-decay/independent-error expression is not a bound for arbitrary time-varying nonlinear feedback. H100 throughput measurements cover the uniform four-bit variant, not the full mixed-head method or this CPU SNN. |
| [Hopkins et al., 2020](https://arxiv.org/pdf/1904.11263v5), rounding/PRNG methods, arithmetic-reference and comparator-precision sections, discussion | Their six-bit result concerns random/residual comparison precision in specified ODE tests, not six-bit neuronal state or universal update probabilities. The accurate cases principally use 32-bit fixed point; tested 16-bit arithmetic remains problematic. Preliminary ARM968 timing compares against software floating point, so it supplies no Cortex-A76 speed claim. |
| [Hubara et al., 2018](https://jmlr.csail.mit.edu/papers/volume18/16-456/16-456.pdf), §2.2 and §4.5/Table4 | Low-bit recurrent prediction results coexist with real-valued training accumulation. The inspected text does not establish every persistent intermediate's width or local Hebbian feasibility. |
| [Michaelis et al., 2022](https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2022.1015624/full), weight precision, plastic updates, traces and targeted learning comparisons | Static recurrent emulation and the isolated plastic-synapse comparison establish different scopes. Loihi traces, exponent fields and spike semantics are not adopted. |

The H-71 research pass is complete. The [progress record](../research/modeling-refinement-progress.json) accounts for at least twelve hours of substantive source review, conditional analysis and design integration, excluding idle gaps. Source read scopes and evidence limits remain explicit. Empirical alternatives stay open under H-86; numerical encoding probes do not establish SNN training success or qualify an implementation contract.
