# Steady recurrent SNN: fractional recruitment, rate dynamics and natural fading

Document-set revision: 202 · Recorded: 2026-10-28 · Topics: SNN-01/SNN-02/SNN-03/SNN-05/SNN-07 · Design discussion.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** C-24/P-52 makes temporal semantic behavior explicit; C-25/P-55 expresses fractional maintenance targets; C-29/P-69 makes transitions independent of destination traversal order. P-53/P-56/P-57/P-59/P-70 are dependencies, not authority. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** preserve fixed signed semantic roles, the current Flash 60+60 population, static SoA connection storage, intrinsic polarity cancellation, the early absence of explicit inhibitory synapses, per-CC activity regulation and external steerability. No population collapse, per-token runtime mask, additional persistent field, heap topology or biological equivalence is selected. Requirements precede auxiliary structures.
- **Acceptance:** assess semantic proportions and amplitude separately; timely token fading and natural cessation in individual responses; controlled noise and later reactivation; correct coverage/flow behavior; sensitivity to time discretization and numerical precision; retained connection semantics and actual resource costs. Numerical tolerances and protocols remain open.
- **Human baseline:** H-65—H-70 supply maintenance, near-step fidelity, loose timing, natural cessation and fractional/numeric scope; H-75 proposes frames, H-76/H-77 specify input/cancellation, H-78 defers state representation. The linked registry retains exact evidence and exclusions. H-79—H-81 reserve Hebbian/HTM for flow and request its separate initial algorithm design. H-84 excludes the explicit broad weak generation fallback from learning.
- **Current assessment:** [K-185](../../../design/DesignAuthorityRegistry.md#k-185) distinguishes positive-kick mean and variance under frame refinement. Existing reduction, numeric and contextual-retention comparisons remain applicable. No noise law, transition or additional field is selected; [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; empirical choices remain open.

## Working direction and the distinction it makes

The leading proposal is to keep individual neuron identities and their learned sparse connections, model their activity over discrete time intervals, and allow fractional participation in the intended activation groups. A neuron's contribution need not be a whole binary event or exhaust its eligibility after one numerical update. This changes the modeled dynamics, not just the integration step of a fixed spike simulator.

| Representation | Quantity carried through an update | Consequence for this application |
| --- | --- | --- |
| Explicit spike events | Individual event times | Retains timing information; binary membership and no-reuse chains are additional choices, not requirements of all spiking models. |
| Binned realized spike counts | Integer events within a time bin | Removes event timestamps but retains small-count granularity; dividing by bin width does not remove it. |
| Individual-neuron rate/activity | Expected or effective output intensity, potentially stochastic | Permits graded contributions and activity across multiple updates while preserving learned identity; rate and noise semantics must be specified. |
| One aggregate rate per minicolumn/polarity | Population-average output | Discards individual contextual identities unless more state restores them; not the present recommendation. |

Calling a fractional expected count a rate does not assert that a physical neuron emitted a fractional spike. It chooses the level of description. Conversely, sampling a Bernoulli event from that rate on every small step would restore much of the fluctuation that the rate representation was intended to avoid. A coarse stochastic rate model can retain selected fluctuations without reconstructing every spike.

The earlier population bound came from distinct, whole-cell assignments to distinct forward steps. It was not an unavoidable limitation of spike models. Relaxing that assignment is what changes the bound. The need to account for the response already present before repairing a deficit survives either representation.

## Fractional recruitment and rate have different possible meanings

For discussion, let `f_i,k` be the selected fraction of neuron `i` for target stage `k`, and `r_i,k` its activity rate conditional on that participation. An effective contribution can be written:

```text
0 <= f_i,k <= 1
z_i,k = f_i,k * r_i,k
effective recruited width = sum_i f_i,k
total rate contribution = sum_i z_i,k
```

These are candidate semantic quantities, not allocated arrays. Fractional membership, rate strength, and the number of physically active neurons are different observables. The width objective may need both a fractional mass and a support-size description; replacing the existing active-neuron fraction regulator by total rate is a separate decision.

One useful division to explore is **fraction as a training allocation target, effective rate as the observed evolving activity**. It allows a target contribution from a weak coordinate without rounding recruitment immediately to zero or one cell. If only the product affects all future transitions, storing both factors as independent runtime variables is redundant: `0.5 * 0.8` and `0.8 * 0.5` both contribute `0.4`. Separate factors become necessary only if a defined operation distinguishes participation, conditional rate, prediction, availability or future response. The choice must precede a storage budget.

Fraction may instead mean a probability of selecting a whole neuron, or its duty fraction within a time interval. Those interpretations give different variability and temporal behavior. A probability followed immediately by a binary draw does not guarantee a smooth realized contribution. Multiplying a duty fraction into a rate that already averages over the full interval would count the same attenuation twice. Units and conditional meaning must be explicit.

H-65's once-per-epoch scope can be compared with an epoch-fixed fractional allocation and stepwise fractional targets inside its support. This is a candidate extension, not permission to silently replace the existing input encoder. H-53's one token presentation remains, with H-76's prediction-guided strong/focused versus weak/broad selection; its fractional rates, predictive threshold and graded intensity calibration still need discussion. There is no repeated external forcing to keep the known token vector present.

Fractional recruitment does not create new independent neurons. Splitting one identity among several stages exposes all of that neuron's learned outputs to its effective activity. It can soften amplitude granularity while leaving shared-edge interference and unintended shortcuts. A restriction such as `sum_k f_i,k <= 1` would be a separate participation budget, not an automatic law of rate dynamics. The old `L ≈ scope_size / width` formula applies only when its traversal assumptions hold; it is not a general rate-response lifetime formula.

## Preserving the vector while allowing a response envelope

For each coordinate/polarity population, compare its aggregate **effective contribution**, not only a count of chosen cells. A weak coordinate may receive small fractional contributions from selected cells across several updates. This removes the requirement to assign an additional whole cell every time that coordinate contributes. It does not guarantee fidelity after limited-precision arithmetic, cancellation, gain differences or stochastic failure.

H-66 still favors per-step fidelity with at most a few steps of bounded deviation. Express the timing window in model time as well as update count; changing the integration step must not silently change the allowed semantic distortion. Relative signed proportions can remain near the token while overall amplitude varies. A low-amplitude boundary is needed because relative errors and direction become unreliable as the token-specific signal disappears into the ongoing background.

Repair compares the desired contribution with the eligible response already produced. Illustratively, if desired rates in two populations are `(0.4, 0.2)` and the observed eligible rates are `(0.35, 0.05)`, the positive deficits are `(0.05, 0.15)`. The missing contribution does not itself have the token's 2:1 ratio. Targets, excess activity and noise-assisted meaningful participation still need distinct treatment; fractional targets change the units, not this accounting principle.

Training can use the known token to define a target or evaluate fidelity. Runtime projection of every network state back onto that token vector would be an additional forcing mechanism and would obscure whether recurrence was learned. No token label, private chain instance or per-token mask is assumed in the shared recurrent graph.

## A loose half-life objective

The timing objective is timely fading of the prominent isolated-token response, with broad tolerance and expected differences between tokens. H-71 allows weaker noise-supported loops to retain context that influences subsequent outputs; fading does not require erasing all such influence. Approximately half a second can serve as a provisional half-life scale, not a deadline for completed return to DMN. Define a token-specific response envelope `a(t)` relative to the relevant background; total CC activity remains regulated and need not decay with it.

A conditional exponential reference after the response peak is:

```text
a(t_peak + t_half) ≈ a(t_peak) / 2
a(t_peak + t) ≈ a(t_peak) * 2^(-t / t_half)
```

The response need not be exactly exponential. Measuring from the input, onset of decay or peak changes the meaning when there is initial amplification; a peak-referenced measure is a proposal, particularly for the complementary-positive-residual case. A half-life alone does not bound a persistent low tail or recurrent revival. The eventual evaluation needs decay of the dominant response together with useful context retention and external steerability, without inventing a precise deadline or forcing every trace to disappear.

For illustration only, a half-life of 0.5 seconds leaves 50% after 0.5 seconds, 25% after one second and 6.25% after two seconds under the exponential reference. These percentages are not acceptance thresholds. An average across abruptly terminating trials can also have a half-life; that does not establish gradual fading in each response. Token/trial variability, smoothness and survival duration therefore need separate observation.

## Natural cessation with learned connections retained

H-68 uses “断链” for the active propagation losing continuity. The learned weights remain available for later inputs and background-driven activity. A response may cease because its combined incoming support is inadequate, or because fluctuations fail to recruit enough downstream activity. These effects can act together and need not occur at a predesignated tail.

A deterministic rate law with positive linear gain below one approaches zero asymptotically. Finite-time cessation requires some actual dynamic feature, such as a transfer region with no output, cancellation that removes supporting activity, or a stochastic realization with no adequate successor. Numerical rounding to zero must not be mistaken for a justified biological or application rule. Conversely, a sigmoid with strictly positive output everywhere plus persistent noise does not create a globally absorbing silent state.

Natural extinction can end a propagation episode while the CC remains active and can be driven again. H-71 also permits surviving or reactivated small loops to carry useful history; complete loss of token-related causal influence is not required. Background support may restart fragments; whether they reconstruct the token or disperse into DMN is a property of the shared dynamics. Simply observing a zero update is insufficient evidence of fading, and later useful context effects are not automatically failures of fading. A runtime procedure that identifies and deletes a particular token's active chain would require additional attribution semantics and is not assumed.

State-dependent stochastic continuation can be examined without adopting independent dropout. For a constant extinction hazard `h`, an illustrative probability of extinction over interval `Delta` is `1 - exp(-h*Delta)`. Keeping a per-step failure probability fixed while changing `Delta` changes the lifetime. This hazard example describes survival statistics, not the amplitude half-life of a gradually fading individual response, and is not a selected mechanism.

Independent loss of a few carriers can disproportionately distort weak coordinates. Fully coordinated termination can preserve proportions until an abrupt end but requires a coordination mechanism and does not supply gradual fading. A candidate to explore is smooth graded attenuation while the signal is meaningful, with naturally increasing failure as support weakens. Whether the actual local transfer and noise can produce that behavior is a design question, not an added whole-chain gate.

## Time discretization and a candidate transition vocabulary

Separate numerical update interval, activity integration/response time, short semantic comparison window and token-response half-life. A rate variable does not require averaging actual spikes over the entire half-second interval. No numerical step is selected; it must resolve the behavior we intend to retain and fit the CPU budget. Coarse steps can merge intended ordering, conceal a shortcut or create numerical oscillations.

### Frame duration and noise units

A direct frame map and a numerical approximation to an underlying continuous-time law have different comparison meanings. If `x[k+1] = W*x[k]` advances one learned transition per frame, using the same `W` twice as often advances two transitions in the former interval. A smaller frame interval is therefore not automatically a more accurate version of the same dynamics. The [time arithmetic](../research/frame-time-noise-analysis.json) gives a simple counterexample: the two-cell swap matrix has no real two-dimensional linear half-step whose square equals that swap. A fixed discrete modeling interval remains a valid candidate; no continuous-time law or additional delay state is required by this observation.

Noise units also matter. In a scalar additive reference with independent unit-variance samples, fixed-size kicks every frame accumulate variance proportional to `1/Delta` over a fixed duration. Multiplying each sample by `Delta` instead gives variance proportional to `Delta`; scaling by `sqrt(Delta)` preserves a diffusion variance per unit time. [Higham's primary exposition](https://www.maths.ed.ac.uk/~dhigham/Publications/P42.pdf), §§2 and4, separates Brownian increments from time-scaled drift. This is a reference for consistent units, not a choice of Brownian noise or an SDE for this application.

Positive kicks also carry a mean. For independent `U[0,A_Delta]` increments over duration `T`, the [exact moment reference](../research/frame-time-noise-analysis.json) gives accumulated mean `T*A_Delta/(2*Delta)` and variance `T*A_Delta^2/(12*Delta)`. Scaling amplitude with `Delta` preserves the mean but makes variance vanish as frames shrink; scaling with `sqrt(Delta)` preserves variance but makes the mean grow. Thus the zero-mean diffusion scaling cannot silently define positive activation input. This concerns the stated additive uniform family, without leak, cancellation or feedback; other noise processes have different limits. A fixed discrete interval remains a valid model choice, and no offset, new noise channel or event mechanism follows.

Temporal whiteness alone does not specify spatial correlation, positive activation kicks versus signed drive fluctuations, or the controller's cadence. A shared CC-wide noise-amplitude setting need not mean one shared random draw for every cell. Future comparisons must identify the intended law and retain physical-time meanings for token input, response duration, regulation and coverage windows. Matching only noise variance does not establish equivalent threshold or cancellation behavior. These conditions introduce no sampling algorithm, extra state or experiment framework.

A rate response may already average over some variability. [Ostojic and Brunel (2011)](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1001056) approximate trial-averaged firing under a specified background; both their transfer curve and temporal response depend on that background. Their averaged response is not the stochastic trajectory of one of our differently connected contextual cells. A phenomenological rate curve need not have this averaged meaning, but any borrowed reduction must state what variability it already includes.

In an [independent threshold reference](../research/frame-time-noise-analysis.json), averaging Gaussian variability of standard deviation `sigma` gives `f_sigma(mu)=Phi((mu-theta)/sigma)`. Adding another independent Gaussian input of standard deviation `tau` and averaging again gives `Phi((mu-theta)/sqrt(sigma^2+tau^2))`. At one standard deviation below threshold, the response changes from about 0.159 to 0.240 when `tau=sigma`; above threshold it changes from about 0.841 to 0.760. These are conditional probabilities, not proposed neuron rates. Two genuinely different noise sources can be intentional; representing one intended source in both an averaged response and explicit kicks changes its strength. This calls for clear noise semantics, not additional noise channels, an adaptive transfer function or a selected distribution.

Averaging also need not commute with H-77 cancellation. In the same [arithmetic record](../research/frame-time-noise-analysis.json), one candidate cell in each polarity independently receives zero or one with equal probability; all other cells are zero. Canceling each realization and then averaging gives rates `(0.25,0.25)`, while canceling the provisional means `(0.5,0.5)` gives `(0,0)`. Both signed means are zero, but unsigned activity differs. The first average is not an instantaneous frame: every actual canceled realization has at most one surviving polarity. Thus a chosen effective-rate trajectory cannot automatically be identified with the average of a stochastic microscopic model. This does not require spike simulation, distribution fields or a change to H-77; it specifies which behavior a later comparison can claim to preserve.

Holding noise fixed within a transition does not decide the feedback observation's age. [Activity response and feedback timing](ActivationRegimes.md#activity-response-and-feedback-timing) compares choosing the next noise level from the latest completed frame with an additional observation delay. It also shows why the sign of the activity-fraction response to noise cannot be assumed from total-rate behavior. These are requirements for interpreting later comparisons, not a selected controller or additional state.

### Token order, model time and input boundaries

Token observation order, modeled elapsed time, neural frames, learning exposures and CPU execution time are distinct. Offline presentation can run faster or slower than modeled time. H-75 does not equate one token with one neural frame; the transition must explain how an observed prefix becomes available before predicting its next token.

Commutative aggregation can remove the distinction being learned. In an [exact affine reference](../research/token-frame-timing-analysis.json), a half-strength two-cell swap followed by additive input gives final states `(0,1.5)` for ordered inputs `A=(1,0), B=(0,1)`, and `(1.5,0)` for the reverse order. Combining either pair as one sum gives `(1,1)`. If that aggregate and all other causal state, parameters and future forcing coincide, the model cannot subsequently recover the lost order. This does not claim that all batching loses order: ordered operations or retained order-dependent state change the premise.

For `x_next=W*x+u`, an exact block of ordered inputs is `W^m*x_0 + sum_j W^(m-1-j)*u_j`, rather than an unweighted input sum. This affine identity is not permission to collapse the actual nonlinear cancellation and transfer operations into a block.

Aligning an isolated event at time `tau` to `ceil(tau/Delta)*Delta` adds less than one frame of delay, but distinct events can land on the same boundary. An exponential reference with half-life `H` changes amplitude by at most a fraction `1-2^(-Delta/H)` for such an age shift: about 1.38% for `Delta=10 ms, H=0.5 s`. This bounds isolated decay only, without queued events, thresholds or recurrent interactions. It does not select an adequate frame interval.

Likewise, a mode retaining a fixed factor per **token** has a physical-time duration that changes with token spacing. One learning exposure per token does not make the neural dynamics event-clocked. Pure scalar decay permits a time-scaled coefficient; an arbitrary learned discrete transition need not have an equivalent fractional-time update.

Under H-86, compare frame-aligned ordered observations with explicitly ordered input handling within a modeled interval, using actual pre-input prediction, pacing sensitivity and work. Preserve the chosen within-frame causal rules. Neither a token queue, position encoding, one-token-per-frame policy nor extra history state is prescribed. Sequence-order prediction, learning exact element durations and autonomous replay remain separate capabilities.

### Two alternating frames with incoming gather

H-75 proposes two alternating frames: every next-frame activation is computed from the same previous-frame snapshot. This is the preferred execution direction for discussion. Its value chain is direct: incoming slots already gather each neuron's causes; an immutable source frame makes that transition independent of which destination is visited first; a separate destination frame costs little compared with connectivity. No per-edge accumulated activity or event queue is required by this scheme.

```text
read frame A; write frame B
for each destination i:
    incoming drive = sum(weight[j -> i] * signal(A[j]))
    B[i] = transition(A[i], incoming drive, this-frame input/noise)
finish the frame's intrinsic cancellation and other defined local operations
publish B; exchange the roles of A and B
```

This is a transition outline, not a chosen equation, instruction schedule or implementation. A destination's sum is temporary arithmetic, not persistent network state. The next frame is fully defined, including inactive values, so stale contents of the recycled buffer do not become state. One owner per destination avoids concurrent scatter additions; no new reverse connectivity index is needed.

The distinction is observable even in two neurons. With unit reciprocal edges and initial `(1,0)`, a snapshot update gives `(0,1)`. Updating the first neuron in place and then the second gives `(0,0)`; reversing that order gives `(1,1)`. The snapshot preserves the intended one-transition causality. It does not assert bit-identical sums across different reduction orders.

Intrinsic positive/negative cancellation must use a consistent stage. H-77 specifies computing the minicolumn-wide signed net and proportionally retaining active rates in the surviving direction. Both polarity populations' provisional next rates can be processed together before finishing those destination values. This can be organized per minicolumn without a third whole-CC frame. The active-rate cancellation law is specified in [SignedPopulationEncoding](SignedPopulationEncoding.md#cancellation-arithmetic-and-the-semantic-residual); its phase handling and placement relative to the transfer function remain open; recurrent gathers must not accidentally read a partly finished B. H-76 token input consults the prior predictive state: sufficient predictive participation gets strong focused activation, otherwise activation is broad and weak. H-84 identifies the broad weak branch as an explicit generation-mode operation excluded from learning; it must not be treated as ordinary synaptic transmission or as a flow teaching population. The token is presented once at its defined boundary, and the noise level for a transition is held consistent rather than being revised after each visited neuron. Learning and slot migration can likewise publish between complete transitions; double-buffering the weight pool is not implied.

Canceling each partial incoming contribution is not equivalent to combining them before proportional cancellation: it can change the surviving contextual pattern even when the net coordinate agrees. The chosen transition must define when recurrent drive, token input and background contributions are combined and canceled. Two-frame publication supplies a consistent boundary; it does not make arbitrary rearrangements within that transition equivalent.

For N=614400, two frames of one FP16 value cost **2457600 bytes (2.34375 MiB)**; FP32 costs **4915200 bytes (4.6875 MiB)**. These are exactly two state arrays, with the existing connection arrays single. They do not include independently required fields that have not yet been specified. A per-CC frame boundary also does not require all CCs in the distributed network to run in global lockstep.

### What a frame value means

H-75 clarifies that cells within a minicolumn represent different contextual paths for the same semantic component. Their individual identities and connections therefore remain meaningful even when their summed semantic output is equal. Replacing their states by one population average would lose the information that the user intends them to carry. The fixed coordinate/polarity assignment does not prescribe a distinct extra semantic axis or one exclusive stored path for every cell.

The minimal state must retain distinctions needed by the next transition. For example, if predictive readiness is modeled as subthreshold drive, a near-threshold neuron and a fully inactive neuron can both emit zero current rate but react differently to the same next input. One sufficiently precise drive value with derived output is a candidate; it does not require a separate field automatically. Alternatively, a phase-aware encoding can be evaluated. Two temporal frames solve read/write ordering, but they do not by themselves define how current activity and predictive readiness coexist within each frame.

H-78 explicitly leaves the state interpretation undecided so that future implementation can compare **one activity value with predictive/active ranges** against **readiness distinct from current firing**. The design must retain that comparison opportunity. It does not require implementing a generic model-selection framework or reserving both layouts now.

H-85 also leaves predictive-phase cancellation for controlled comparison. H-86 permits recording obvious empirical alternatives directly rather than repeatedly requesting a choice. Predictive cancellation and state representation are distinct questions: neither behavior requires a particular field layout by definition. Compare pre-input alternative retention and later disambiguation while preserving the specified active-rate operation; a net semantic average alone cannot expose all ambiguity.

Separate semantic quantities do not automatically require independent stored fields. Readiness that is completely determined by the existing snapshot and learned graph can be derived; readiness with its own required history needs a representation that preserves that history. Inspecting a stored phase and computing a new prediction from incoming activity have different timing semantics, however, and cannot be interchanged silently. Numenta's [reference implementation](https://raw.githubusercontent.com/numenta/nupic/master/src/nupic/algorithms/temporal_memory.py) activates cells using existing active segments, then computes new segment activity for the next prediction. This is a timing reference, not a requirement to add its segment structures.

The gather's `signal(A[j])` also needs its own meaning. A stored predictive value may be transmitted as a weak recurrent rate, or may represent preparation that changes the response to input without itself emitting that rate. One field versus separate fields does not decide this operation. In a [single-gather reference](../research/flow-learning-design-analysis.json), a predictively ready source of strength `1/4` and a unit edge supply drive `1/4` under the first interpretation and zero under the second. Neither result is selected. The inspected NuPIC reference computes segment activity from `activeCells`, while deriving predictive cells from active segments; prediction alone is not an emitted fractional activity in that implementation. Its binary and segment rules are not adopted.

Compare how predictive alternatives evolve between token observations as well as how they respond when input arrives. Transmitted readiness can influence later cells even without a new observation; that may be useful or may impair next-input specificity. Keeping readiness nonemitting can preserve a distinction from current firing, but is not evidence that useful weak recurrence survives. Distinguish these effects from storage precision and from which activity receives learning credit. Weak context-bearing firing must not automatically be classified as predictive-only preparation, or excluded by a coverage filter. No output gate, phase threshold, credit mask or extra state follows from this comparison.

The future comparison should use the same behavioral goals and account for each candidate's actual state and work. Relevant observations are prediction-guided versus broad token entry, the effects of cancellation on the next response, isolated-token fading with useful retained context, and external steerability under the specified noise conditions. Parameters need comparable meanings rather than merely identical numeric values. No extra persistent field is justified solely by naming a candidate.

With a first-order discrete transition, all persistent neuronal information needed for the next step resides in A. Any chosen self-retention can read A[i] directly; there is no need to reconstruct intervening spike-by-spike accumulation. A direct frame-to-frame map and a leaky relaxation are different modeling choices, however. The rate interpretation alone does not require adding an independent membrane leak, timer or filtered state. Below is one illustrative relaxation vocabulary, not an additional mandatory layer on top of the two-frame proposal.

An illustrative rate-relaxation vocabulary is:

```text
u_i[k] = sum_j w_ij * z_j[k] + external_i[k] + background_i[k]
z_trial_i[k+1] = (1 - beta_i) * z_i[k] + beta_i * Phi_i(u_i[k])
z[k+1] = Encode(Cancel(z_trial[k+1]))
```

`Phi` is a nonnegative response function; `Cancel` represents the required within-dimension opposite-polarity interaction. Their ordering, units, noise semantics and prediction handling remain open. A fraction that already contributes to `z` is not multiplied in again by default. For frozen input to a simple first-order relaxation, `beta = 1 - exp(-Delta/tau)` is a possible step-consistent coefficient; it does not establish stability of the recurrent network. This sketch adopts no time constant, function, code format or auxiliary buffer. A [matched-envelope example](ContextLoopsAndNumerics.md#matching-response-fading-does-not-match-contextual-retention) gives identical semantic fading but very different retention of contextual cell composition. Two-frame publication and a fitted half-life therefore do not determine the self-retention rule.

Rate dynamics can preserve sequencing if its coupling and time response support traveling or changing activity. It can also converge to static coactivation, oscillate in an unintended way or revive a nominally faded token. Persistence in the same neuron across numerical updates is not automatically a shortcut. The meaningful progression constraint is preserving intended order, causal response and steerability, with any identity exclusion justified at that level. Rate coactivity also does not choose a Hebbian/HTM flow-learning rule or preserve millisecond spike-timing information. H-79 reserves that learning direction for token-sequence flow; H-80/H-81 leave its initial algorithm design open and distinct from DMN/isolated-token repair. H-76 token-entry activation does not itself select a learning algorithm.

## Numerical precision is part of the behavior

H-70 requires attention throughout the computation, beyond selecting storage widths. FP16/FP32 state, low-width weights and their intermediate arithmetic are separate choices; storage labels do not guarantee fractional-rate fidelity.

[Specified reduction examples](ContextLoopsAndNumerics.md#product-reduction-and-cancellation-precision) show that summing many correctly stored positive contributions in FP16 can lose most of their total, and that a small minicolumn net can disappear during population summation or narrow writeback. Compare the full path from stored operands through products, totals, cancellation and next-state storage. The two-frame rule fixes source-state timing; it does not make arithmetic associative or require a particular reduction algorithm.

| Operation | Behavioral failure to examine | Design question |
| --- | --- | --- |
| Encode fractional targets and rate values | Weak coordinates round to zero; a product quantized repeatedly loses meaningful contributions. | Which quantity is stored, its scale/range, and how much error is tolerable near zero? |
| Sum many FP8-weighted contributions | Small inputs disappear beside large terms; overflow, reduction order or cancellation changes effective gain. | Accumulation precision, range, reduction semantics and reproducibility tolerance. |
| Opposite-polarity cancellation | Two large nearly equal activities produce an inaccurate small residual or wrong sign. | Error in the signed residual and the surviving unsigned activity must both be assessed. |
| Repeated weak attenuation or repair | Updates below a quantization boundary repeatedly round away, leaving a stuck response or ineffective learning. | Representation and update policy must allow the required changes; higher precision, error retention or stochastic rounding are alternatives with different costs and noise. |
| Threshold and stochastic continuation | Small rounding differences cause early cessation, repeated revival or biased lifetimes. | Define model thresholds independently of accidental numeric underflow, and assess time-step dependence. |

For a scalar exponential reference, per-update retention is `g = 2^(-Delta/t_half)`. With illustrative `Delta=10 ms` and `t_half=0.5 s`, `g≈0.98623`. A quantizer that maps `0.98623` back to `1` will keep a unit value unchanged if this operation repeats without retained error. This is a conditional numerical counterexample, not a selected FP4 code or recommendation for a 10 ms step. Keeping temporary arithmetic accurate alone does not solve it if every stored update loses the change.

Preserving two fractional factors separately only helps if their meanings and quantization provide useful information; it is not automatically better than one effective rate. Predictive preactivation also remains distinct from current firing intensity. Neither interpretation defines how phase and magnitude coexist in a stored value; four bits are not a required state limit. Extra state or wider fields require their own requirements, layout and memory accounting; the named-array capacity calculations remain conditional on their stated formats.

## Accumulated decay and integer-format comparison

H-71 requests practical accumulation periods and possible future decay triggers, together with INT8/INT4 assessment. [ContextLoopsAndNumerics](ContextLoopsAndNumerics.md#accumulating-decay-instead-of-losing-each-small-update) compares wider evolving state, retained error, cumulative-time evaluation, scheduled code-boundary crossings and stochastic integer transitions. These must preserve intermediate effects or explicitly define a different model. Integer storage requires scale, signedness, accumulator and update semantics; bit width alone does not settle fidelity or cost.

H-72 clarifies that scheduling concerns the next minimum-precision-permitted change, calculated successively. H-73 permits wider evolving state, including FP16/FP32, so tiny-state storage is not a prerequisite. Under H-74, the initial numerical comparison can use straightforward FP32 updates; retained-error fields, timers or other machinery need a demonstrated behavioral or resource benefit beyond what that baseline supplies. Scheduling scalar decay does not automatically avoid recurrent connection access.

[Deferred arithmetic](ContextLoopsAndNumerics.md#deferred-arithmetic-must-preserve-cancellation) must retain each frame's effective cancellation and input boundary. Holding uncancelled activity until a later batch can let it transmit before removal; retaining numerical decay progress does not authorize that behavior. Common attenuation has limited homogeneity identities, but arbitrary per-neuron decay, thresholding and feedback cannot be moved across recurrence without checking the complete transition.

A stochastic integer interpretation also needs its own variability and saturation law. [Conditional survival/branching arithmetic](ContextLoopsAndNumerics.md#matching-the-mean-does-not-select-a-stochastic-update-law) gives equal average half-lives with very different individual extinction and burst behavior. Shrinking a frame while preserving its mean retention need not preserve those distributions. Clipping an unbounded update to a narrow code range changes the conditional mean. These findings favor evaluating the actual update law, frame duration and representation together; they select neither a stochastic model nor a return to spike-by-spike simulation.

A next-change timestamp can encode fractional decay progress only under a law and boundary that make it sufficient. Intervening input can change that timestamp while leaving the visible code unchanged; no-future-event cases can hide different residuals with different later reactivation. [Timestamp/input analysis](ContextLoopsAndNumerics.md#inputs-and-the-numerical-state-carried-by-a-timestamp) keeps these alternatives explicit. Any schedule also retains the completed-frame read/write boundary and must identify the work it actually avoids; local decay timing alone does not eliminate incoming gathers or prove a token-response half-life.

## What the research supports

| Primary source and read scope | Useful evidence | Boundary for this design |
| --- | --- | --- |
| [Ostojic and Brunel, 2011](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1001056), abstract, response/noise results and discussion | Derived linear–nonlinear and adaptive-timescale descriptions can approximate firing-rate responses of specified spiking models; background noise affects response gain and timing. | Accuracy deteriorates in relevant low-noise/strong-input conditions. This supports explicit rate/noise semantics, not automatic equivalence in our attention regime. |
| [Schwalger, Deger and Gerstner, 2017](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1005507), abstract, finite-size/switching results and discussion/limitations | Stochastic population equations retain finite-size fluctuations and history effects; switching statistics depend on noise. | Homogeneous-population and mean-field assumptions matter, especially with influential outliers. Aggregating our heterogeneous, differently connected cells would need separate justification. |
| [Augustin et al., 2017](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1005545), reduction assumptions, numerical integration, performance and limitations | Population reductions use many weak inputs, Poisson/mean-field approximations and precomputed response quantities. Simpler filters can be more robust under rapidly varying drive; finer integration can be needed to avoid numerical oscillations. | Their exchangeable-population speedups do not transfer to our retained individual contextual cells. Neither a comparable cost reduction nor preservation of learned identities follows from adopting a rate variable. |

These sources establish useful modeling precedents and limitations. Fractional recruitment, the proposed application-level variable division and the arithmetic examples above are our design analysis. No scientific simulation, benchmark, project test or implementation was run for this discussion.

## Consequences and next decisions

The recommended next design comparison is individual-neuron stochastic/effective rate dynamics with fractional training targets, preserving learned identities and allowing activity to cease naturally. A simple deterministic rate model is a useful comparison, but it cannot claim the same finite-size extinction statistics merely by matching an average curve.

Resolve the following together: whether fractional selection is a target, probability, duty fraction or independently persistent quantity; what rate/current/predictive state means; which local response conditions allow natural cessation; the semantic error window and low-signal boundary; numerical update/accumulation formats; and how activity fraction and temporal coverage remain meaningful. Near-zero rates across all cells must not automatically count as full coverage, nor may total rate replace the selected per-CC active fraction without a decision.

The capacity pool and endpoint ownership can survive this change, while integration, training error, timing, state precision and acceptance may change substantially. Removing explicit spike events does not prove a CPU speedup: a rate update may still visit many edges. Storage and work claims require a chosen algorithm; no new indexes, buffers or performance numbers are presumed. A fractional/rate model should be judged by its useful semantic trajectories and resource costs, without claiming that smoothing alone fixes interference, guarantees fading or reproduces biological spikes.
