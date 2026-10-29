# Steady recurrent SNN: activation regimes

Document-set revision: 202 · Recorded: 2026-10-28 · Topics: SNN-01/SNN-02/SNN-03/SNN-05/SNN-07.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** C-15/P-29 relates learning to semantic behavior; C-17/P-34 provides attention/activity support; C-18/P-36 cancellation, C-19/P-39 classification and C-20/P-42 steerability serve distinct required behaviors. Dependencies do not merge these observables. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** preserve per-coordinate roles, current Flash 5120 × (60+60) planning, static SoA capacity and regional relocation. Coordinate polarity, semantic valence, neuronal phase and synaptic effect are distinct. The approximate response duration and noise-control sequence are behavioral requirements; activity bounds, loop-length bounds, update/decay laws, noise levels, valence estimator, detector/control algorithm and state representation remain open.
- **Acceptance:** one token event according to coordinate strengths and H-76 prediction-guided selection evokes meaningful recurrence and gradual fading under low noise, with a loose half-life-like scale and timely return toward DMN under H-67. DMN detection then raises white noise to maintain activity and permits slow negative-semantic convergence. Also assess the three regimes' convergence from the intended entry conditions and the broad per-CC activity envelope. Exact encoding, detector observables, timing tolerances and noise levels remain open; these goals do not assert biological equivalence.
- **Human baseline:** H-43/H-46—H-59 define semantic roles, regimes, broad per-CC activity, noise, detection and steerability; H-67/H-71 distinguish dominant fading from useful context; H-76/H-77 define input/cancellation and H-78 defers state interpretation. The linked registry retains exact evidence and exclusions. H-79—H-81 reserve Hebbian/HTM for flow and request its separate initial algorithm design. H-82 sets prediction matching as the direct flow-training criterion and separates generation design. H-83 expects predictive diversity to narrow as longer sequence context identifies the training sequence. H-84 excludes the explicit broad weak generation fallback from learning.
- **Current assessment:** [K-186](../../../design/DesignAuthorityRegistry.md#k-186) broadens the rated-word input comparison to Chinese while preserving the distinction from learned-state meaning and regulated active fraction. Drift, steering and detector requirements remain; no evaluator, classifier, threshold or additional state is selected. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; empirical choices remain open.

## Common target: bounded activity with changing semantic state

The model assigns semantic-coordinate roles to minicolumn populations. It does not require a token or a meaningful feature to activate exactly one coordinate. A multidimensional activation pattern can express a feature, and the same coordinate population can participate in many such patterns. Non-bijective correspondence between measured biological features and minicolumns therefore does not by itself contradict the model.

Across the three regimes, the active-neuron fraction **within each CC** should remain within a permissive range. A compact statement of the intended observable is:

```text
a_c(t) = number of active neurons in CC c at time t / N_c
L <= a_c(t) <= U
```

Here `N_c` is the neuron population of CC `c`; the current sizing case has 614,400 neurons per CC. This defines an observable, not a stored counter or an implementation. H-47 selects the per-CC scope. The activity definition, observation window, treatment of predictive cells, transient tolerance and numerical bounds remain open.

H-48 requires a broad operating allowance rather than a narrow clamp or fixed set point. DMN, flow and immersion may have different typical activity levels, ranges and fluctuations within that allowance. Their ordering and numbers are not selected. There is no requirement for the three regimes to have identical activity distributions, or for inactive, predictive and active neuronal phases to contain equal populations. Those neuronal phases and the mental operating regimes are separate classifications.

Under H-77's full net cancellation, at most one 60-cell polarity population per dimension has positive active rate after cancellation. Thus instantaneous positive-active-rate support is at most **50% of the CC**. Predictive readiness and qualified temporal coverage are separate: visiting both polarities at different times can still support H-56's broad DMN coverage. This is a consequence of the selected operation, not a newly chosen activity target.

The decoded semantic state can evolve while the activity fraction stays bounded. Stable activity occupancy therefore does not require a fixed semantic vector or a stationary distribution of semantic content. Neither dense input coefficients nor nonzero FP4 values automatically define the neurons counted as active.

### Whole-network activity follows from the per-CC bounds

Using the same activity definition and observation time/window for every CC:

```text
A(t) = sum_c N_c * a_c(t) / sum_c N_c
L <= a_c(t) <= U for every c  =>  L <= A(t) <= U
```

The whole-network active fraction is a neuron-count-weighted average, so it cannot escape the common envelope while every CC respects it. Equal-size CCs give an ordinary average. If local bounds differ, the network fraction lies between the corresponding weighted-average lower and upper bounds. A separate whole-network fraction constraint or controller is not needed to obtain this arithmetic consequence.

This is stability of activity occupancy. Which neurons are active, their strengths and the represented semantic trajectory remain separate observables. Bounded occupancy permits the intended drift and recurrence; it does not by itself establish their quality or a dynamical-stability theorem. Nor do the bounds specify how coupled CCs maintain them: the eventual transition and learning rules must achieve the selected local behavior under cross-CC input. H-53 separately requires global modulation of background noise for attention and DMN activity support. That behavioral control requirement does not follow from, or replace, the weighted-average identity.

## Desired regimes

| Regime | Human-specified behavior | Details still to define |
| --- | --- | --- |
| No-task / DMN | White-noise drive sustains recurrent activity within a proportion band. Semantically negative information has a slight activation advantage, with the semantic trajectory gradually drifting toward an extreme of composite negative content. | Noise channels, amplitude and temporal definition; what the activation advantage changes; semantic-valence criterion and meaning of the endpoint; allowable drift rate and active-fraction bounds. |
| Task / flow | Token-associated patterns activate sequentially, with the overall active fraction remaining within a band. | Sequence source and pacing, how successive vectors interact, retention versus replacement, noise during task drive, and task-level success. |
| Immersion | Multidimensional semantic patterns recurrently self-activate in loop chains whose lengths vary within a specified range. The overall active fraction stays within a band. | Definition of a loop and its length, semantic similarity tolerance, time units, coexistence of loops, perturbation behavior and permitted external drive. |

These are operating regimes for the SNN discussion. Their activity levels may differ under H-48. H-52 sets their steady attractor behavior and convergence from varied entry conditions as the overall objective. H-61/H-63 distinguish DMN/token construction and repair from flow learning and immersion. H-79 assigns the Hebbian/HTM direction specifically to token-sequence flow; H-80/H-81 request initial design of its still-open algorithm, without presuming it is a repair variant. Immersion remains an expected emergence without dedicated training. The common activity envelope must leave room for their different semantic dynamics. This does not prescribe three separate networks or a mode-switching state machine.

## Learning objective: regime attraction and token transients

Connections should jointly produce **the intended regime and convergence trajectory for the entry conditions**, including the applicable input context. “Three phases” here refers to DMN, flow and immersion; the inactive/predictive/active cellular phases remain a separate classification. Steady regime behavior permits ongoing circulation and semantic change. It does not require convergence to one fixed neuron vector.

The entry conditions and their intended outcomes must eventually be stated together. A prior CC state, its input history and current drive can affect which regime is expected. The objective is robust convergence across the intended cases; it does not imply that every possible initial state and input must converge to all three regimes, or that a scalar active fraction identifies the correct regime. H-53 supplies a behavioral role for noise modulation; its control algorithm and representation remain open.

### Acquisition routes and emergent immersion

The [training proposal](TrainingStrategy.md#unified-next-step-repair) uses one maintenance approach for DMN and isolated tokens, including initial construction: observe natural next-step activation and select sufficient eligible expected-but-inactive cells when it falls short. Existing successful participation and missing participation are distinguished. Hebbian/HTM is the intended direction for token-sequence flow, whose initial algorithm remains under design. H-82 requires predictive preactivation to closely match the incoming sequence. H-83 expects multiple possibilities on shared matching fragments and narrowing toward the training sequence as context grows. Autonomous continuous generation is a separate design problem, not this direct training criterion; immersion remains a desired, externally steerable emergence without a dedicated training objective.

H-62's preliminary fixed groups can supply a reference for H-63's repair process; quota-defined eligible populations remain an alternative. Group membership, disjointness, fresh stochastic token entry and later semantic profiles remain distinct questions. Repair targets must allow the intended fading and input-driven redirection, rather than maintain one constant width. A whole-CC DMN candidate domain does not guarantee broad qualified coverage; any temporary training activation is distinct from successful autonomous recurrence.

H-65 specifies the enclosing scope once per epoch: all CC neurons for DMN, and cells proportionally selected from the token's minicolumn populations for single-token training. Width is an approximate per-step count; scope size divided by width estimates a pass's step count. Body enhancement and distributed return attenuation aim to support weak recurrence with noise while allowing smooth fading without external signals. The working expectation under H-66 retains tail-to-head wiring. Lap duration and net gain jointly affect persistence across recurrence; a half-second token response does not specify one pass, and fading does not require severing return edges. H-66 prefers token-vector proportions in each step, allowing at most a few steps of bounded deviation. Compare the completed recurrent response with that objective while permitting overall amplitude to grow or fade; an epoch/lap average alone is insufficient. The exact observable, short window, tolerance and low-amplitude boundary remain open.

### Intermediate objective for every token

Each token, under its own isolated input, should evoke self-recurrent semantic activity followed by timely fading and return toward DMN. H-67 permits a **loose half-life-like quantitative target**, with approximately **0.5 seconds** as a provisional scale, broad tolerance and expected token differences. It does not require the entire response to end at half a second.

H-53 specifies one presentation according to semantic-coordinate strengths, without repeated token forcing. H-76 directs strong activation toward a sufficient predictive population; otherwise it supplies broad low-intensity activation. H-84 specifies that the broad weak fallback is an explicit generation-mode operation, outside ordinary neuronal connection transmission and excluded from learning. H-69's fractional recruitment and H-67's rate proposal require the two branches' intensity and semantic calibration to be explicit. Low background noise during attention remains required; zero noise is not specified.

The intended response has these observable parts:

1. One token event recruits a semantic response under the eventual input encoder.
2. Recurrent interactions sustain its content while allowing a changing amplitude.
3. The prominent token response fades, and active propagation can cease naturally through insufficient support or stochastic failure. H-68 preserves learned weights; H-71 allows useful smaller recurrent loops to retain history and influence later outputs.
4. The CC returns toward DMN; its detector and activity regulator support the background, with slow negative-semantic convergence. Low activity may already have increased noise before DMN detection.

DMN remains active. Measure the prominent token-specific envelope relative to the appropriate background, separately from useful context retention, total CC activity and the coverage-based regime detector. The response does not need to restore the exact pre-input neuron configuration. A half-life measured from the response peak is a candidate for cases with initial amplification; the reference point, observable, tolerance and tail criterion remain open.

A conditional exponential envelope with a 0.5-second half-life would retain 50% after 0.5 seconds, 25% after one second and 6.25% after two seconds. This is arithmetic, not a selected decay law or acceptance schedule. An average across abrupt stochastic endings can show the same decay while individual trajectories do not fade smoothly. Evaluate token/trial variation, dominant-response attenuation, useful retained context, unwanted dominance and revival separately. A weak history-dependent effect on later output is not by itself an overlong dominant token response.

The half-life target selects no hard shutoff timer, membrane time constant, input rate or immersion lifetime. A subsequent token can interact with an unfinished response. [RateDynamics](RateDynamics.md#a-loose-half-life-objective) relates this objective to natural cessation, noise, fractional activity and numerical precision.

### Attention, DMN detection and global background-noise control

H-53 requires a global suppression mechanism that substantially reduces background white noise when external conditions demand concentration. The ordinary isolated-token response is learned under low noise. DMN detection calls for increased background noise to support its active fraction and slow semantic convergence. H-55 also requires feedback that maintains overall activity fraction within a flexible target range: a low-activity condition caused by cancellation raises noise even while a token-specific residual remains. **DMN detection is therefore not a prerequisite for every noise increase.**

The global modulation is a required behavioral function, with both attention-dependent suppression and activity-dependent support. Whether its scope is a whole CC, a coordinated set of CCs or the entire deployed network, and how it is implemented, remain to be specified; H-47's per-CC activity requirement remains. No precise noise amplitude, suppression ratio, feedback gain, ramp shape, detector, mode bit or auxiliary storage is selected. Continuing attention demands, an activity deficit, a new token and DMN detection may coexist; their quantitative combination and transition behavior need discussion.

H-56 excludes total active fraction from regime detection; it remains the noise regulator's activity variable. Detection uses temporal neuron coverage and activation-path behavior as specified below. The loose half-life-like scale is an evaluation target; detector windows and timing tolerances are separate decisions. Detecting DMN does not require that its slow negative-semantic convergence has reached its endpoint.

### Activity response and feedback timing

Noise amplitude is an input to the activity regulator, but its effect on **active-neuron fraction** needs measurement under the actual transition. It cannot be inferred from its effect on summed firing strength. In a [two-cell cancellation reference](../research/noise-feedback-analysis.json), independent uniform positive drive is added to both polarities before taking the net. With initial signed drive 0.2 and an activity threshold 0.1, increasing the noise scale from 0 to 0.4 lowers the expected active fraction from 0.5 to 0.375, while increasing expected unsigned rate from 0.2 to approximately 0.2167. At larger noise scales the fraction rises again. This is a counterexample to universal monotonicity, not a proposed noise law or the response of a 614400-cell CC. A strict positive-value count also fails to distinguish noise amplitudes in the dense positive-drive case described above. Preserve the requested activity-fraction observable; do not substitute summed rate to obtain an easier feedback signal.

There is a useful restricted case: H-77 cancellation is positively homogeneous, so `C(sigma*x)=sigma*C(x)`. If the **whole** provisional input scales with noise and no fixed drive remains, support above a fixed positive threshold is nondecreasing with scale. Actual recurrent and token contributions need not satisfy that condition. Defining activity as a fixed fraction of the largest current rates would instead enforce the count by definition, and would not demonstrate successful activity regulation. Any adaptive qualification needs an explicit meaning and sensitivity to fading.

Feedback must identify which completed frame supplies its activity observation. Consider only a local linear reference, in deviation coordinates:

```text
a[t+1] = rho*a[t] + b*sigma[t],  0 <= rho < 1, b > 0

lagged observation:     sigma[t+1] = sigma[t] - k*a[t]
completed observation:  sigma[t+1] = sigma[t] - k*a[t+1]
```

For this illustrative integral correction, strict local stability requires `0 < b*k < 1-rho` in the first case and `0 < b*k < 2*(1+rho)` in the second. At `rho=0.98, b=1, k=0.1`, their spectral radii are approximately 1.0392 and 0.98995 respectively. Reading completed B to select the **following** transition's noise is compatible with H-75; changing noise while still constructing B is a different operation. The reference fixes no controller, point target, gain, activity window or neuron equation. Nonlinearity, bounded noise, changing learned weights and observation filtering remain outside its proof.

Primary research supports examining those interactions rather than importing a universal homeostatic timescale. [Harnack et al. (2015)](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1004357) analyze intrinsic-excitability feedback and show how recurrence and intermediate filtering can make feedback oscillatory. [Zenke et al. (2013)](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1003330) instead examine compensating plasticity with activity detection in balanced spiking networks, where overly slow compensation can lose stability. The regulated quantities and models differ. Neither study specifies our noise controller or transfers its biological timing values to this application.

Noise has a lower bound, and attention may restrict its allowed range. A simple stable linear reference already has a limited range of reachable activity levels under bounded noise: a low cap can prevent recovery to the band, while excessive external/recurrent drive can keep activity above it even at zero added noise. The actual comparison must establish this reachability as well as recovery time. It does not authorize a new inhibitory mechanism, hard activity clamp or priority rule.

Under H-86, compare meaningful activity definitions, feedback observation timing and simple bounded corrections on actual trajectories. Observe excursions and sustained oscillations, cancellation recovery, residual growth and fading, context prediction and external steerability. Include both frozen-connection behavior and behavior after learning changes the response. Prefer the simpler adequate candidate; add filtering or other state only when a demonstrated failure and its remedy justify it. The flexible regime-dependent envelope, temporal classifier and attention requirements remain separate constraints.

## External steerability, stale states and recovery

H-59 makes **external-token steerability a requirement of every intended normal state**. Autonomous recurrence, repeated use of an ensemble and long-lived immersion can all be normal. The requirement is that external token input can redirect the ongoing dynamics when appropriate.

Stale denotes a state in which no external token input can break or redirect the ongoing recurrence. That is a loss of responsiveness, rather than a fourth mental regime or a judgment based on activity level, coverage, cycle length or time spent in immersion. Noise feedback can maintain activity without establishing this input-response property.

The requirement does not say that every arbitrary token must force a transition. It also does not require permanent departure from DMN: a meaningful token-dependent recurrent response followed by the intended return can demonstrate responsiveness. The admissible input protocol and what counts as effective redirection remain to be specified. A practical assessment needs to distinguish the injected activation itself from its effect on subsequent dynamics; one unsuccessful input or trial does not establish the universal absence of a steering input.

Training and validation should therefore assess both the intended autonomous behavior and responses to external inputs from the intended normal states. This adds a responsiveness criterion alongside the three-regime window classifier and the activity regulator. No separate runtime watchdog, probe schedule, intervention timer or response threshold is selected.

State convergence under identical continuing input is a different property from useful response to an arriving token. [Yildiz, Jaeger and Kiebel (2012)](https://www.ai.rug.nl/minds/uploads/2519_Yildizetal12.pdf) examine convergence in sigmoid reservoirs, including both general counterexamples to a spectral-radius shortcut and sufficient conditions for nonnegative weights. Those conditions omit our population cancellation and adaptive noise. [Manjunath and Jaeger (2013)](https://www.ai.rug.nl/minds/uploads/ESP_NC.pdf) explicitly relate convergence to the driving input. Neither result establishes H-59 merely from a connection-matrix statistic or a test under unrelated forcing.

A [scalar analytical example](../research/minimal-dynamics-analysis.json) separates persistence from cue strength without choosing a neuron model. For `x_next=tanh(2*x+u)`, zero input has stable states near ±0.9575. Starting at the negative state, one `u=1` presentation leaves a negative state and subsequent zero-input evolution returns to that branch; one `u=2` presentation produces a positive state and subsequent evolution approaches the positive branch. Persistent input and a single presentation are therefore different interventions. This is not a stale-state diagnosis or an input-gain prescription. Actual comparisons must use the admissible token encoder, duration and attention/noise conditions, observe the response after presentation, and check retained context alongside redirection.

H-59 raises **system reset as a possible recovery operation** when external input cannot restore responsiveness. Reset remains a design candidate. Its operational trigger, scope (a CC, a group or the network), affected activity/predictive/learned state, retained information and post-reset entry condition need discussion before an implementation or automatic policy is chosen. Whether learned connections or persisted state participate in recovery remains unresolved.

## Regime detection and training observables

The fractional/rate proposal must retain the distinction between active-neuron fraction, total rate and qualified temporal coverage. Tiny positive rates across all cells must not automatically count as broad meaningful coverage, and total rate cannot silently replace the existing activity-fraction regulator. The activity/qualification semantics remain to be designed; total active fraction remains excluded from regime classification.

H-56–H-58 specify a flexible observation window and coverage that discounts added white-noise activity. The qualitative criteria are:

| Regime | Activation pattern over the window |
| --- | --- |
| DMN | Qualified activity covers most neurons; 80% or more is illustrative. |
| Immersion | Qualified activity remains within a small subset of neurons. Circulation within that subset is compatible with the criterion. |
| Task / flow | The path keeps passing through different groups while short-window total coverage stays small. Exceeding an upper coverage bound, illustratively20%, rules out flow. |

**Total active fraction is a regulation target and is not an input to regime classification.** Temporal coverage is a different observable: it asks which distinct neurons have participated across the window. A CC can keep a modest instantaneous active fraction while visiting most neurons over time. The raw union of all active-neuron sets would count every touched neuron, including incidental noise events; H-58 requires qualifying participation before applying coverage criteria. No hard-window implementation, soft weighting, activity threshold or stored set is selected.

The descriptions supply both training goals and detection directions. The80% and20% examples are not fixed numerical cutoffs. A numerical immersion fraction, window duration/weighting, group-identification algorithm and complete decision rule remain open. The ordinary token-response half-life-like target does not fix the window length. Per-CC notation follows the existing application unit; coordination of regime labels across CCs remains unspecified.

H-57 excludes broad short-window coverage from flow in this model. Coverage above the flow cap implies non-flow, not automatically DMN; DMN has its own broad-coverage criterion. At low coverage, immersion and flow still require a temporal distinction between remaining in a limited ensemble and progressing through different groups within the allowed footprint. A small footprint can also describe a brief token response. Window/path definitions must resolve these cases without assigning physical distance to neuron IDs or minicolumns; no extra mode or complete label precedence is selected.

A path can satisfy both descriptions: in a constructed sixteen-frame record, repeatedly visiting four disjoint groups of 15,360 cells gives 10% coverage and per-cell time density 1/4. Consecutive groups do not overlap, yet the path remains within one small recurring population. These are observation counts, not a demonstrated neural trajectory or an assigned regime; the [conditional calculation](../research/temporal-coverage-analysis.json) specifies its density cutoff assumption. Thus path order alone does not guarantee distinct flow and immersion labels. If cases supply identical allowed observations, a classifier cannot infer different causal origins from those observations alone. Future comparisons should establish which observable differences justify the intended distinctions under their stated task conditions. This does not require classification by causal origin, adding input-history fields, changing the flow cap or suppressing useful cycles. The complete rule remains an empirical design question under H-86.

### Coverage across opposite polarities

If coverage counts post-cancellation active cells, H-77 imposes a useful consistency bound. Let `D=5120` and let `B` dimensions have qualifying cells from **both** polarities during the observation window. Each other dimension can contribute at most sixty covered cells; each of those `B` dimensions can contribute at most 120. Thus, for `K` covered neurons:

```text
K <= 60*(D+B)
covered_fraction C = K/(120*D)
B/D >= max(0, 2*C-1)       [with integer rounding]
```

The [coverage arithmetic](../research/temporal-coverage-analysis.json) therefore requires at least **3,072 dimensions, or 60%**, to have qualified participation in both polarities if coverage reaches the illustrative 80%. The reversals occur somewhere within the window; they need not happen simultaneously. Density qualification, activity bounds and actual paths can be more restrictive, so the bound is necessary rather than sufficient. Predictive readiness is outside this active-cell calculation.

A trajectory with fixed coordinate signs throughout the window cannot exceed 50% such coverage, even if it circulates among all cells of the surviving populations. DMN's negative-semantic convergence and broad qualified coverage must therefore be assessed together. This does not equate coordinate reversals with positive or negative semantic content, or require every dimension to preserve its own private contextual state.

A separate geometric illustration uses the existing sample of 1,024 ordinary Flash token rows. Reversing the smallest-magnitude 20% of coordinates while preserving their magnitudes gives median cosine **0.9915** with the original row; reversing 60% gives **0.7420**. These are specific simultaneous vector transformations, not DMN trajectories, semantic-valence measurements or bounds on the actual temporal process. Their purpose is to keep sign-change count, vector geometry and semantic behavior distinct. No sign-selection algorithm or new metric is prescribed.

### Attractor terminology and the required exit

The three regimes are the desired attracting behaviors. The isolated token response is required to be finite-lived within that regime-level design. Calling it a permanent token attractor would omit its required exit: under unchanged autonomous dynamics, an invariant attracting state does not itself prescribe a timed departure. A transient recurrent response, or a metastable description with a specified escape process, can express the intended behavior. The mechanism and mathematical model remain open.

[Miller and Katz (2010)](https://pmc.ncbi.nlm.nih.gov/articles/PMC2851230/), in the indexed main-text passages inspected, model rapid state changes at trial-variable times while trial averages vary more slowly. Their decision network also shows conditional benefits and limits of added noise. This supports examining individual trajectories alongside average envelopes. It does not establish smooth single-trial fading, select noise as our decay mechanism or supply H-67's half-life target. The desired observation scale remains open.

### Candidate noise filter: per-neuron activation density

H-58 requires added white-noise activity to be discounted in coverage. Its suggested method is to omit neurons whose activation density over the window is too low. This selects the purpose of filtering; “density,” the window and cutoff remain proposals to define.

One illustrative definition uses `I_i(s)=1` when neuron `i` counts as active and a nonnegative window weight `w_t(s)` with integral one:

```text
d_i(t) = integral w_t(s) * I_i(s) ds
Q(t) = { i : d_i(t) >= density_cutoff }
coverage(t) = |Q(t)| / N
```

This is a candidate mathematical interpretation, not a chosen estimator, field or data structure. Event frequency, active-time fraction and strength-weighted density are different definitions. The same qualified-coverage meaning must govern the DMN/flow examples. Window weights, cutoff and treatment of current versus predictive activity remain open.

The intended separation is incidental noise activity versus meaningful recurrent participation. A residual that noise helps to sustain is not automatically removed because noise contributed to it. A density gate would not label the causal origin of every activation, and cannot guarantee perfect separation: sustained noise can pass, while brief genuine flow participation can fail. Its suitability must be judged against the desired trajectories.

For the illustrative active-time definition above, average density across neurons equals window-averaged active fraction. Consequently `coverage * density_cutoff <= window_average(active_fraction)`. This is an offline consistency condition for compatible window/threshold/training targets, not a runtime classifier feature or a relaxation of H-56. In particular, broad DMN coverage after filtering must be supported by enough genuine repeated participation; raw noise touching many cells does not establish it.

### Temporal dependence, window length and path information

White forcing does not generally produce white observed activity. Even a stationary scalar reference `x[n+1]=a*x[n]+epsilon[n]`, with independent zero-mean drive and `|a|<1`, has correlation `a^k` at lag `k`. Recurrence, cancellation and adaptive noise regulation can therefore affect a density filter's behavior. This linear reference does not select a neuron law or imply that thresholded activity has the same correlation function.

The [conditional counting analysis](../research/temporal-coverage-analysis.json) gives a concrete example. With a 100-frame window, a 5% density cutoff and an illustrative 1% incidental active probability per cell, independent frames pass the cutoff about 0.343% of the time. Holding each independent draw for five frames changes that probability to about 18.209%, despite unchanged marginal activity. This constructed block process is not the application's white-noise generator. It shows why the distribution of temporal participation matters, and why a cutoff cannot be calibrated from average activity alone. The effect need not vary monotonically with correlation length because a finite-window threshold changes discretely.

Window length also interacts with input pacing and genuine participation. If each event independently touches a given cell with probability `p`, the expected raw union over `m` events is `1-(1-p)^m`; for `p=0.05`, five events already give about 22.62%. This is an unfiltered independence reference, not a prediction of the contextual SNN or a violation of H-57 by itself. Actual reuse, graded activity and qualification matter. A physical-time window includes more events at faster pacing, while a fixed-event window spans different physical durations. A genuine visit lasting `b` of `n` equal frames has active-time density `b/n` and is rejected by any larger cutoff. These relationships belong in a joint window/qualification comparison.

Density also discards order. Permuting the frames leaves every cell's active-time density and union support unchanged while changing the activation path. The symbolic sequences `ABCDABCD` and `AABBCCDD` illustrate equal group densities with different transitions; they are not assigned regime labels. An order-dependent distinction cannot be recovered from coverage alone. This does not require a transition matrix or a new runtime history structure: first establish which path information the intended distinction actually needs.

Known injected noise is not a general cell-level decomposition of the final state. With the nonlinear minicolumn operation `C`, `C(a+noise)-C(noise)` can differ from `C(a)` even when the signed population sum remains additive. Thus filtering coverage must be evaluated as a useful participation criterion, rather than claimed to identify an exact noise-free component. Cells omitted from coverage may still carry useful weak context into later responses; omission from an observable does not authorize erasing that state.

### History needed by the observation window

Two complete neural frames make the next state depend on a consistent snapshot; they do not generally recover earlier participation. For example, binary observation histories `0010` and `1110` have identical last two observations but four-frame densities of one-quarter and three-quarters. Even a running count plus the latest two observations cannot always identify which contribution must expire from a rolling window. These are information counterexamples for arbitrary observation histories, not asserted SNN trajectories. H-75's neural update does not forbid observation history when a required detector demonstrably needs it.

The [window arithmetic](../research/temporal-coverage-analysis.json) compares three simple candidates. The illustrative `W=100` below concerns binary active-time observations of 614,400 neurons, not predictive readiness or per-edge activity. The payloads are conditional algorithm costs, **not reserved additions to the CC budget**.

| Observation choice | Direct candidate payload | Behavioral consequence |
| --- | --- | --- |
| Counts within nonoverlapping blocks | 614,400 bytes for one `uint8` count per neuron | Reports complete aligned blocks; results and delay depend on block phase. |
| Exact rolling 100-frame window | 7,680,000 bytes of packed observation bits | Retains finite-window membership. Recounting avoids cached counts; optional `uint8` counts would add 614,400 bytes to reduce repeated arithmetic. |
| Exponentially fading history | 2,457,600 bytes for one FP32 density per neuron | Approximates a recurrence with an unbounded fading tail; it does not reproduce a finite rolling window. |

These are direct representations, not universal memory lower bounds. Shared position/time metadata and implementation overhead are excluded. Initialization, indicator definition, precision and physical-time weighting still need specification. Any retained multi-field data follows SoA; no history log, heap topology or additional neural frame is selected.

For a rolling mean of width `W`, the mean observation age is `(W-1)/2` frames. The exact-arithmetic recurrence `d[n]=(1-lambda)*I[n]+lambda*d[n-1]` has mean age `lambda/(1-lambda)`. Choosing `lambda=(W-1)/(W+1)` matches both this age and the variance under independent identically distributed indicators. It does **not** match transient or correlated activity. At `W=100`, the fading kernel still assigns about 13.53% of its weight to observations older than the latest hundred frames. After ten active frames preceded by inactivity, followed by quiet frames, an illustrative density cutoff of 0.05 first fails after 65 quiet frames for that fading kernel and 96 for the rolling window. Neither timing is selected.

Compare the windows by incidental-activity rejection, retention of meaningful brief participation and regime-transition timing, including the consequences for DMN-dependent noise control. A cheaper observation rule changes behavior if it changes the window. Density history still cannot recover all path order, and its filtering rule does not automatically become flow-learning source credit.

### Comparisons on actual trajectories

H-86 directs empirical choices to controlled comparison without repeated requests for a parameter decision. The immediate value is a detector that distinguishes the required trajectories while preserving useful context; the following alternatives remain open until such trajectories exist.

| Comparison | Evidence that would favor an option |
| --- | --- |
| Active-time density and strength-qualified participation | Excludes incidental participation while retaining genuine brief flow activity and meaningful noise-supported recurrence. Prefer the simpler adequate definition; strength and time occupancy are different observables. |
| Physical-time and event-relative windows | Labels track the intended behavior across input pacing and recurrence times without hiding broad coverage by shortening the observation. Window and qualification are compared together. |
| Block, rolling and fading windows | Compare meaningful participation and transition timing, including block phase or fading tails, against the actual retained-state and computation costs. Matching an average age alone is insufficient. |
| Coverage alone and an additional path observable | Comparable qualified footprints require different labels because of their actual progression. Introduce an ordered observable only to resolve a demonstrated distinction, then derive its necessary state. |
| Alternative numerical precisions and activity thresholds | Regime distinctions and weak context survive representation changes; changed counts caused solely by numerical dust or threshold crossings are exposed. |

Primary analysis research supports care in interpreting these comparisons. [Louis et al. (2010)](https://www.frontiersin.org/journals/computational-neuroscience/articles/10.3389/fncom.2010.00127/full) show that surrogate spike manipulation can distort rate profiles and interval statistics, changing synchrony conclusions. [Zeraati et al. (2022)](https://www.nature.com/articles/s43588-022-00214-3) examine finite-record bias in timescale estimation. Neither supplies this model's detector, window or half-life. If shuffled or fitted summaries are later used, report what they preserve and what they change; no surrogate generator, Bayesian fitting package or extra array is selected.

Total active fraction remains excluded from regime classification. Predictive diversity under H-83 is also separate: reducing readiness amplitude can reduce a thresholded cell count without making the possible continuations more specific. Flow learning is assessed against context-compatible observed continuations before input, alongside its temporal coverage requirement.

## Complementary positive input from a negative-semantic DMN state

H-54 specifies an early-model scope with **no explicit inhibitory synaptic connections**. H-77 defines this intrinsic interaction by a minicolumn-wide signed net: active rates in the surviving polarity are scaled proportionally to retain that net, while opposing active rates are canceled. There is no index-based cell pairing. This is separate from H-53's global background-noise suppression. Neither function introduces a modeled inhibitory-cell population or a prescribed set of inhibitory edges.

The additional desired trajectory is:

1. DMN activity has moved into a relatively negative semantic state.
2. An external positive-semantic token complementary to that state supplies its single prediction-guided activation.
3. Opposite-polarity activity cancels within dimensions, rapidly lowering overall activation for a short interval.
4. The low overall active fraction causes the system to increase background white noise, supporting its flexible activity target before DMN detection.
5. The small positive-semantic residual preferentially strengthens under that drive and reaches a peak.
6. It then begins the token-like fading/return behavior, eventually returning to DMN and its slow negative-semantic convergence.

The intended reduction concerns internal activity, not only a small signed-vector readout. H-55/H-56 identify total active fraction as the regulation variable: the activity deficit must be visible to that regulator to trigger increased noise. Summed strength and semantic-response magnitude remain distinct observables. The transient reduction must be reconciled with the flexible per-CC envelope; no lower bound or permitted excursion is chosen here.

“Complementary positive” describes a semantic input/state relation whose actual opposing-population activation needs assessment. Semantic valence is not coordinate sign. Intrinsic cancellation can reduce unsigned activity while leaving a signed residual, but it does not by itself establish that the residual is semantically positive or longer-lived; [signed population encoding](SignedPopulationEncoding.md#cancellation-arithmetic-and-the-semantic-residual) gives the conditional arithmetic. Residual fidelity and extended lifetime are additional learning/behavioral targets.

The actual regulated quantity matters: [input-row magnitude and active participation can move in opposite directions](SignedPopulationEncoding.md#lexical-contrast-does-not-identify-a-complementary-state). A larger net-vector L1 norm does not disprove the requested drop in active fraction, and a smaller active fraction does not establish positive residual meaning. Compare these observations under the same input selection, cancellation and noise-control conditions; no new runtime statistic is prescribed by this distinction.

H-55 specifies an additional growth-to-peak stage before the residual's token-like decline. The longer overall response can include that stage; the desired post-peak behavior follows the isolated-token training objective. No growth time, peak magnitude, precise lifetime extension or revised numerical clock origin is selected. A fall in overall activity triggers support but does not by itself establish DMN, and increased noise does not by itself identify a semantic phase.

This separates **activity regulation** from **semantic selectivity**. White noise supports activity without carrying a positive semantic instruction. Preferential growth of the surviving positive pattern, followed by the required decline, is a target for the learned dynamics in that context. Merely raising unrelated activity to the permitted fraction would not meet it. The role of any additional state, saturation, adaptation or timing mechanism remains to be discussed; no implementation is inferred.

## Semantic negativity is not coordinate negativity

A vector with many negative coordinates is not thereby semantically negative. The positive/negative populations in the current encoding describe the sign of a coordinate, while semantic valence concerns the meaning of the whole represented pattern in context. Neither notion identifies excitatory versus inhibitory synapses.

The requested DMN drift is an explicit behavioral target. A valence criterion would need to identify the direction and endpoint independently of raw coordinate signs. The basis, labels, interpretation context and any evaluator remain to be discussed; no additional classifier or data structure is presumed.

White noise has no inherent negative semantic direction. In a model with a semantically neutral drive, the specified drift would have to come from an asymmetry in the learned dynamics or another explicitly selected part of the model. H-53 maintains this distinction: raising noise supports activity after DMN detection, while DMN slowly converges toward negative meaning. The user's slight activation advantage supplies a desired bias, but does not yet specify whether it changes activation probability, strength, persistence or transitions. A bounded active fraction must coexist with that drift; the requested endpoint is not permission for numerical amplitudes or connection weights to diverge.

Published lexical norms offer candidate stimulus evidence, not a definition of arbitrary network-state meaning. Public supplements from [Warriner, Kuperman and Brysbaert (2013)](https://link.springer.com/article/10.3758/s13428-012-0314-x) and [Xu, Li and Chen (2022; online 2021)](https://link.springer.com/article/10.3758/s13428-021-01607-4) supply the English and Chinese ratings used in the [token-matched Flash geometry comparison](SignedPopulationEncoding.md#lexical-contrast-does-not-identify-a-complementary-state). Word norms do not label arbitrary token fragments, contextual compositions or autonomous CC states. A word-based comparison must state its language, presentation context and actual tokenization; the two samples do not establish matched cross-language valence scales.

Stronger negative-labeled responses, longer negative-state residence and increasingly negative interpreted meaning are different outcomes. A [conditional episode calculation](../research/minimal-dynamics-analysis.json) gives negative time occupancy `q*tau_minus / (q*tau_minus+(1-q)*tau_plus)`, where `q` is the fraction of negative-labeled episodes and the `tau` values are their mean durations. Equal episode counts with a ten-percent duration advantage yield about 52.38% negative occupancy, not an extreme endpoint. Different occurrence/duration combinations can give the same occupancy; the same stationary occupancy can also approach at different speeds. This is not a relation between neural weights and duration, a two-state DMN model or a proposed runtime label.

The future comparison should therefore assess semantic change within DMN, its extent and timescale, response strength, residence/return behavior and continued token responsiveness separately. An observed negative occupancy bias alone does not establish H-46/H-53's slow drift target. Keep the interpretation and exposure conditions independent of the mechanism being evaluated; measuring negative-coordinate count or using the intended training bias itself as the score would not demonstrate the requested meaning. No valence field, learned classifier, directed noise, special training stage or precise endpoint is prescribed.

## Learning and recurrence questions

H-40's coactivity direction leaves the temporal update law open. H-52 supplies the purpose of that learning: the three regimes' attracting behavior and appropriate convergence, with token-evoked recurrence and return to DMN as an intermediate objective. Reinforcing joint activity alone does not specify the duration or exit of a response. The eventual rule must achieve both useful recurrent support and the required termination under the specified input conditions.

The operational goals require two separately observable outcomes:

1. **Activity behavior:** each CC's active-neuron fraction remains within the permissive bounds, allowing regime-dependent levels and whatever transient allowance is selected.
2. **Semantic dynamics:** the network converges into the intended regime and exhibits its required semantic behavior. In the isolated-token case, content is recurrently supported and gradually returns to DMN around the target time; unrelated activity at the correct fraction is insufficient.

Global background-noise suppression and DMN-dependent restoration have an explicit business role under H-53. Their implementation, and any other inhibition, homeostatic rule, synaptic timing mechanism, threshold adaptation or trace, remain open. Storage is considered when the corresponding algorithm is discussed.

## Acceptance to make concrete

| Question | Why it is needed |
| --- | --- |
| What counts as active within each CC, over what time window, with what broad bounds and transient allowance? | Makes the local steady-activity target measurable while allowing different mental states and separating current activation from prediction. |
| What external inputs and response observables demonstrate that each normal state remains steerable, over what horizon? | Makes H-59 operational while preserving intentional recurrence and distinguishing a failed trial from loss of responsiveness. |
| If reset is adopted, what evidence triggers it, what state is affected or retained, and where does the network resume? | Defines the candidate recovery operation without treating it as an automatic or already authorized implementation. |
| Which entry conditions should converge into which regime, under what continuing drive? | Makes the primary attractor/convergence objective assessable without presuming a controller or reducing all three regimes to fixed points. |
| How do coordinate strengths determine the fraction and random selection of cells in the single activation event? | Makes the confirmed one-event stimulus precise without choosing an encoder or repeated forcing. |
| What demonstrates token-specific recurrence, gradual fading and return to DMN, with what timing tolerance and token/trial coverage? | Makes the intermediate objective observable while distinguishing the response from DMN activity and an average curve from individual trajectories. |
| What flexible window and noise-qualification rule distinguish broad DMN coverage, small-footprint immersion and bounded-coverage flow progression? | Makes H-56–H-58 operational while retaining the flow exclusion bound and keeping total active fraction out of classification. |
| What is the scope of global noise control, and how do attention suppression, activity support and new tokens combine? | Specifies the required feedback behavior before choosing a control formula. |
| What measures cancellation, preferential growth of the positive residual, its peak and subsequent token-like fading? | Makes H-54/H-55's complementary-input objective assessable without confusing coordinate polarity, valence, activity level and operating regime. |
| How is semantic valence recognized, and what precisely is the small negative advantage? | Prevents coordinate signs from substituting for meaning and defines the intended DMN drift. |
| Which pre-input observables demonstrate matching and context-dependent narrowing during flow learning? | Makes H-82/H-83 measurable while keeping continuous generation as a separate design task. |
| What constitutes recurrence of a semantic pattern, and how is loop length measured? | Makes immersion distinguishable from a fixed point, unrelated noise or an unbounded trajectory. |
| How do inputs and dynamics change across regimes? | Defines transitions, persistence of noise and behavior after task input ends without assuming a controller design. |

The half-life-like timing direction is specified with broad tolerance and token variation; experimental protocols, tail criteria and quantitative pass/fail thresholds remain open. [Model](Model.md#snn-03) tracks encoding and state semantics; [synaptic integration research](SynapticIntegrationResearch.md) tracks activity-control candidates; [semantic cortical mapping research](SemanticCorticalMappingResearch.md) keeps anatomical correspondence separate from the operational goals.
