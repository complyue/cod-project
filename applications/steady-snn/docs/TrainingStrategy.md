# Steady recurrent SNN: training strategy and activation-chain maintenance

Document-set revision: 202 · Recorded: 2026-10-28 · Topics: SNN-01/SNN-02/SNN-03/SNN-05/SNN-07.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** C-15/P-29 serves context-dependent sequence prediction through flow learning. C-22/P-45 serves the required recurrence through unified construction/repair; C-23/P-48 relates progression planning to that maintenance. C-24/P-54 and C-26/P-61 constrain repair by actual fading, semantic progress and useful context. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** fixed Flash 5,120 × (60+60) roles, static SoA connection storage, no heap topology, intrinsic polarity cancellation and no explicit inhibitory synapses in the early model. Preserve per-CC activity feedback, noise-filtered coverage, normal-state external steerability and requirements before auxiliary structures. No numerical chain parameters, neuron update law, controller, runtime group table, extra per-edge field or implementation is selected.
- **Acceptance:** flow assessment compares pre-input prediction with the observed sequence and checks narrowing where context disambiguates continuations; assisted training activity is not prediction evidence. Evaluate maintained recurrence under the actual integration, selected state/weight precision, cancellation and noise conditions; distinguish natural response from training assistance and intended progress from premature return or skipped steps. Assess token proportions over the required short horizon in the completed next-step response, while allowing its amplitude to grow or fade. Preserve future candidate/capacity feasibility, token fading, DMN behavior, later learning and external steerability. Topological cycle opportunity and harmful dynamical recurrence require different evidence.
- **Human baseline:** H-61—H-66 define construction/repair, epoch scope and progression/fidelity goals; H-67—H-71 add rate/fraction, natural fading, numerical care and useful loops; H-76/H-77 define entry/cancellation to evaluate; H-78 defers state choice. The linked registry retains exact evidence and exclusions. H-79—H-81 reserve Hebbian/HTM for flow and request its separate initial algorithm design. H-82 sets prediction matching as the direct flow-training criterion and separates generation design. H-83 expects predictive diversity to narrow as longer sequence context identifies the training sequence. H-84 excludes the explicit broad weak generation fallback from learning.
- **Current assessment:** [K-194](../../../design/DesignAuthorityRegistry.md#k-194) refines the propagation evidence: surviving activity, preserved graded input and cross-chain interference are distinct observations. Construction/repair goals and the separate [flow design](FlowLearning.md) remain unchanged; no neuron law, group width or training mechanism is selected. This is an agent assessment, not human confirmation or qualification. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; [progress](../research/modeling-refinement-progress.json) records its scope and substantive work.

## Training division

| Behavior | Proposed training role | Acceptance retained |
| --- | --- | --- |
| DMN | Maintain and initially construct chains within the epoch scope of all CC neurons, repairing deficient next-step participation. | Broad qualified temporal coverage, flexible activity regulation, slow negative-semantic drift and responsiveness to token input. |
| Isolated token | Once per epoch, select the concrete minicolumn cells proportionally to the token's signed vector; construct and maintain recurrence within that scope. | One stochastic token stimulus, meaningful recurrence and gradual return toward DMN with a loose half-life-like scale around 0.5 seconds and timely fading under the specified noise conditions; residual growth/peak/fade in the complementary-input case. |
| Flow from token sequences | Hebbian/HTM learning is the intended direction. Its initial algorithm remains under design. | Predictive preactivation closely matches the input sequence, with possibilities narrowing as longer matching context disambiguates it. Autonomous continuous generation is not a training acceptance condition. Preserve flow coverage, useful recurrence and steerability. |
| Immersion | Expected emergence from the resulting network, without a dedicated immersion training objective. | Meaningful multidimensional recurrence in a limited ensemble, permitted loop lengths and external steerability. Emergence is to be observed, not asserted from the construction. |

DMN and token training share a proposed maintenance process with different expected populations and response profiles. Initial construction is the case where existing connectivity supplies little of the desired next-step activity; later iterations repair the remaining shortfall. This does not require separate construction and maintenance algorithms, identical parameters, or separate networks. H-79 confines Hebbian/HTM learning to flow training with token sequences. H-80 makes clear that its algorithm is not established and may differ from DMN/token repair; H-81 requests an initial flow design with consequential choices brought to discussion early. DMN/token maintenance remains the construction/repair process, and immersion has no dedicated training objective. H-76 prediction-guided token entry is an activation rule and does not assign the same learning method to these tasks. Physical-distance preference is omitted under H-61.

## Flow learning: initial design scope

The separate [flow-learning design](FlowLearning.md) covers the preliminary Hebbian/HTM procedure, contextual assignment, source credit, prediction assessment, capacity and numerical comparisons. H-79—H-81 establish this distinct design task. H-82 requires predictive preactivation to match the externally supplied sequence; H-83 permits alternatives on shared fragments and expects narrowing as the observed context disambiguates them. Autonomous generation remains separate. No complete flow algorithm is selected, and the DMN/token repair process below is not its default.

### Generation fallback and sequence learning

H-84 excludes the explicit broad weak generation fallback from learning, including selection of its cells as teaching recipients. Learning an unfamiliar transition still needs its own representation and credit procedure. [FlowLearning](FlowLearning.md#generation-fallback-and-sequence-learning) preserves the HTM comparison and the distinction between teaching intervention and naturally available prediction.

### Predictive alternatives and signed cancellation

H-77 fixes active-rate cancellation. H-85 leaves predictive cancellation open, independently of H-78's state representation choice. Assess complete plausible continuations, since per-coordinate agreement alone can lose their relationships. [FlowLearning](FlowLearning.md#predictive-alternatives-and-signed-cancellation) records the alternatives and evidence. H-86 directs empirical choices to later real-data comparisons without repeated questions.

### Preceding source credit and sparse growth

Preceding context, unobserved predictive alternatives and the arriving observation have different learning roles. Existing-edge adaptation and absent-source discovery also have different requirements. [FlowLearning](FlowLearning.md#preceding-source-credit-and-sparse-growth) discusses these roles without importing generation fallback, DMN/token shortfall repair or an assumed history buffer.

### Capacity demand depends on contextual assignment

Flow learning must fit the retained source unions and class distribution of the shared static pool. A prediction failure can arise from assignment, integration, update precision or source discovery rather than capacity alone. The [capacity analysis](FlowLearning.md#capacity-demand-depends-on-contextual-assignment) and [joint training assessment](FlowLearning.md#joint-acceptance-across-training-stages) keep these concerns tied to actual behavior and work.

## Epoch scope, width and gain parameters

The maintenance objective determines the participating domain and its strategy parameters. H-65 specifies the following division. H-67/H-69 open a rate/fraction interpretation; the whole-neuron count and disjoint-traversal formulas below remain conditional reference cases, with their applicability discussed in [RateDynamics](RateDynamics.md#fractional-recruitment-and-rate-have-different-possible-meanings):

| Choice | DMN maintenance | Single-token maintenance |
| --- | --- | --- |
| Concrete scope for the epoch | All neurons in the CC; no additional random restriction is implied. | Cells selected once from the minicolumn populations in proportions determined by the token vector, respecting the existing signed-population encoding. |
| Approximate width | Goal-dependent number of cells activated at a step; no exact cardinality requirement. | The same parameter role, with a setting appropriate to the sampled token scope and response goal. |
| Body enhancement | Strengthening through the body to let weak circulating activation benefit from background noise. | The same role under the token's attention/noise and semantic-response conditions. |
| Tail-to-head attenuation | A weakening transition distributed over the selected tail/head stages. | Attenuation calibrated with body enhancement for the intended finite response and smooth return. |

Let `S_e` be the concrete neuron set for the current maintenance epoch and objective, and `n_e = |S_e|`. All stepwise candidate domains are subsets of `S_e`; sampling a next group does not resample the enclosing token scope. A later epoch may select another realization. What constitutes an epoch across multiple tokens or CCs, and how training conditions are scheduled, remain open.

Scope, traversal and training update are distinct. Several laps can use the same epoch scope. A new epoch is not an instruction to clear retained connections, allocate a private graph, or make cells outside its scope silent. Existing connections and background activity still affect the observed response. A sampled scope also does not by itself specify the first stimulus group or every later batch; H-76 defines the prediction-guided entry branches, while their numerical calibration and relationship to the epoch scope remain to be specified.

### Width, lap time and response duration

For an approximate pass through the scope without ordinary forward reuse, with typical width `W`:

```text
L ≈ n_e / W
T_lap ≈ L * delta_t
```

`L` is the rough number of group steps, and `delta_t` is the effective time per step if approximately uniform. Variable widths, residual final groups and the chosen closure timing affect the estimate. The width is a target, not a demand to borrow out-of-scope cells to complete an exact final batch. Numerical update timing is not selected.

The intended chain retains tail-to-head connections under H-66. H-67 permits approximately half a second as a loose half-life-like scale, with expected token differences and timely fading; this is not a deadline for completing a lap or returning fully to DMN. H-68 permits active propagation to cease naturally while learned weights remain. Width affects lap time and how often return attenuation is encountered. Persistence also depends on net gain, background drive and the response criterion, so it need not be an integer number of laps.

Connected return paths and successful reactivation are distinct. Nonzero return weights can remain while the returning signal is subthreshold, lost to quantization/cancellation or no longer carries a distinguishable token response. Consequently, observing only one effective pass would not establish that the chain is disconnected, and retaining the return wiring alone does not guarantee several visible laps. Fading does not require cutting the return connections or reducing the whole CC's activity to zero.

Distinguish per-step enhancement from aggregate enhancement through the body. In a single-wave scalar approximation:

```text
B = product of effective body-step gains
D = aggregate effective tail/head transmission, 0 < D < 1 for attenuation
G_lap = B * D
```

H-65 supplies the roles of enhancement and attenuation; these effective gains are an analytical interpretation, not raw FP8 weight multipliers. If a fixed per-body-step gain `g > 1` is used, `B = g^m` for `m` body transitions. Changing width changes the number of amplifying steps as well as the lap duration. For an illustrative `g=1.01` and `D=0.9`, eight body steps give `G_lap≈0.9746`, while sixteen give `G_lap≈1.0553`. These are arithmetic examples, not proposed parameter values or simulated SNN behavior.

The strategy therefore needs an explicit convention: body enhancement may be specified as a total effect over the body, or as a per-step effect calibrated jointly with chain length and return attenuation. Holding total `B` and `D` fixed separates that particular gain coupling from the width setting; it does not make the actual nonlinear response independent of width. Over many short laps, small effective gain errors accumulate: illustrative lap gains of 0.995 and 1.005 produce multipliers of about 0.606 and 1.647 after 100 laps with no additive drive. These are conditional arithmetic, not suggested settings or FP8 weight resolution requirements. No convention or calibration algorithm is selected yet.

For negligible additive drive and constant `0 < G_lap < 1`, an envelope sampled at the same phase each lap decays with approximate time constant `-T_lap / log(G_lap)`. A noise-supported effective model instead has:

```text
a_(r+1) ≈ G_lap * a_r + b_r
```

If the effective contribution `b_r` were a constant positive `b`, its linear steady amplitude would be `b / (1-G_lap)`: weaker amplitudes rise toward it and stronger amplitudes decline toward it. This illustrates how background support and net attenuation can coexist. It does not prove selective semantic amplification or the requested growth/peak/fade, and constant parameters alone do not yield an arbitrary peaked envelope. In the application, noise feedback, cancellation, thresholds, state/weight quantization effects and competing routes determine the actual trajectory. Absence of external token input does not remove the required background-noise mechanism.

### Entry conditions affect the growth and fading profile

Body enhancement can supply temporary growth without an additional fatigue or expiry mechanism. In a conditional ring of `L` reference groups, let `g_j > 0` be the effective transmission from group `j` to the next, and let `W` be that transition. Then `W^L = G_lap * I`: after a full lap, every initial phase mixture is scaled by the same product. With `G_lap < 1`, the repeated same-phase envelope fades, even though some body steps amplify. These are analytical reference groups, not a required runtime phase table.

The [entry arithmetic](../research/body-return-response-analysis.json) illustrates the distinction with twenty steps: sixteen body gains of 1.02, followed by four gains of about 0.862, give a lap gain of about 0.758. Activity starting at the body entrance first rises to about 1.373 times its initial amplitude. Starting at the return entrance, or uniformly across all twenty phases, gives no above-initial peak during the first lap in this example. At 10 ms per step the same-phase envelope has a 0.5-second half-life. These arbitrary values demonstrate dependence on entry, not recommended parameters or successful neural training.

H-76's focused predicted entry and broad weak entry may excite different parts of the learned recurrence. Evaluation of a trained response can include those generation input conditions, including the residual after H-77 cancellation, rather than only a chosen chain head. H-84 excludes learning the broad weak fallback itself; testing the resulting recurrence is a different operation. A reference ring cannot provide immediate total-amplitude growth from every possible phase while attenuating each lap: that would require every step gain to exceed one. This limited result does not rule out the desired response in the nonlinear, noise-regulated shared network, or justify adding an entry router.

Repeated small peaks can coexist with overall fading. The loose half-life target does not demand monotonic decline on every frame, nor preservation of a sharply separated reference wave. The relevant observations are the semantic response envelope, timely reduction of the dominant token response, retained useful context and external steerability. Changing width or frame duration changes both entry/progression timing and gain exposure; calibrating the lap product alone does not settle them.

### Token proportions apply to the epoch scope

Let `n_e,q` be the number of selected cells in coordinate/polarity population `q`. These population counts establish the token-proportional scope for the epoch. The magnitude-to-proportion normalization, integer rounding and any inclusion rule for very small coordinates remain encoding choices; H-65 does not give a numerical formula.

An arbitrary width-`W` group drawn uniformly from an initially unused scope has expected contribution `W * n_e,q / n_e` from population `q`; later draws depend on what remains. Actual per-step contributions fluctuate. In a no-reuse pass, a population with `n_e,q` selected cells can participate in at most `n_e,q` different steps. Thus epoch-wide proportions do not guarantee that every step reproduces the token vector.

H-66 prefers preserving the original token-vector distribution in each step, with at most a few steps allowed to maintain that distribution and no large short-term deviation. Matching only the whole epoch or traversal average is insufficient. The window size and tolerated error remain open; a temporal averaging decoder is not selected by allowing a short comparison window.

### Per-step targets and bounded short-window deviation

Let `p_q` describe the desired coordinate/polarity proportions under the eventual token encoder. Distinguish these original-vector targets from the integer epoch counts `n_e,q`, each step's intended and actual participants, and the decoded activation strengths. The target is not automatically redefined to equal whichever proportions the finite epoch sample happens to contain. Normalization, rounding and small-coordinate treatment must make the sampled scope compatible with the desired fidelity.

A candidate approach is balanced selection: determine population contributions for the next step, then randomly choose eligible unused identities within those populations. This preserves random cell selection while controlling semantic proportions. Uniform selection from the entire remaining scope preserves proportions only in expectation, which does not bound any particular short response. Precomputed groups and adaptive repair can both apply such balancing; neither a quota algorithm nor an auxiliary representation is adopted here.

For repair, the distribution applies to the **complete correct next-step group**, including naturally successful participation and the selected missing cells. The repair subset should compensate the population-specific deficits. If the target is 10 cells from population A and 10 from B, while 10 A and no B cells activate correctly, the missing contribution is 10 B cells. A fresh balanced repair sample of five A and five B would produce 15:5 rather than 10:10. Excess or semantically wrong activity cannot generally be corrected by adding positive contributions alone.

The allowance of a few steps should be assessed over rolling windows, including across the return boundary, rather than only disjoint blocks. For example, `[A,A,B,B][B,B,A,A]` balances each four-step block but contains a four-B window at their boundary. With variable widths, aggregate population counts or strengths over the window need appropriate weighting; averaging step percentages equally can misrepresent the contributions. The exact metric and permitted deviation, including very small coordinates, remain discussion items.

Token fidelity concerns relative signed contributions while overall response strength may grow or fade. A candidate readout check is closeness of the relevant response to `a_k * v`, for token vector `v` and a varying nonnegative amplitude `a_k`; this does not select a decoder or error norm. It also does not require all simultaneous CC activity, including background and other signals, to equal a pure token. The observable and low-amplitude boundary need definition. Correct target sampling alone is insufficient: coordinate-dependent gain, thresholds, cancellation and quantization can distort the actual response even when the chosen group counts are balanced.

### Finite populations constrain the short window

Under a no-reuse forward traversal with whole-cell membership, a population containing `n_e,q` selected cells can contribute to at most that many distinct steps. This bound concerns that assignment policy; it does not establish a universal limit for spike models or the proposed fractional/rate model. Suppose a required population must appear at least once in every rolling window of `H` steps on an `L`-step circular traversal, where `1 <= H <= L`. A necessary condition is:

```text
n_e,q >= ceil(L / H)
```

Consecutive participation steps must be no more than `H` steps apart, including the last-to-first gap. This is a count bound, not a sufficient condition for the correct proportions, strength or timing. For example, two selected cells cannot supply a required coordinate in every three-step window of a 20-step lap: that condition needs at least seven distinct participation steps. At best, two appearances can be spaced ten steps apart. Demanding several cells per window can tighten the bound further.

This does not require every arbitrarily small nonzero coordinate to appear in every window. The error contract must determine which omissions matter; meaningful coordinates omitted entirely by epoch sampling cannot be recovered by redistributing that same scope. A total-width budget alone misses these per-population constraints. Short-window fidelity may favor wider groups and fewer steps per lap, with persistence supplied by repeated laps. More laps in the response interval then require closer attention to effective net gain. Scope, width, window and tolerance must be examined together; ordinary early reuse or out-of-scope recruitment is not silently authorized to resolve an infeasible choice.

## Unified next-step repair

The conceptual training cycle below uses whole-cell selection as a reference. Fractional/rate maintenance can instead compare desired and realized effective contributions; accounting for correct natural activity remains necessary. A rate deficit may call for changing existing contribution strength as well as recruitment, so the exact repair operation remains open.

The conceptual training cycle is:

1. Select the concrete objective-specific neuron scope once for the epoch and set its width/enhancement/attenuation strategy. Within that scope, establish each next-step target under the progression, return and resource constraints.
2. Observe what the existing network produces under the specified token/noise/cancellation conditions, before any supplemental training activation.
3. If the relevant next-step activation is insufficient, identify eligible expected-but-inactive neurons. Exclude inadmissible earlier-step roles and preserve future-step feasibility; sample enough suitable candidates to address the shortfall.
4. Use the selected population as the additional next-step training group and repair the supporting transition. Adjusting existing connections, adding absent connections and supplying temporary training activation are distinct possible operations; their combination is not yet selected.
5. Assess subsequent response without supplemental training assistance, retaining the scenario's required noise/inputs. Check semantic progress and return behavior as well as activation quantity in the combined network. Repeat across training iterations as needed.

Steps 1–3 combine H-63's repair direction with H-64's progression concern. Steps 4–5 expose the learning operation and its acceptance: directly stimulating selected cells can continue a training episode, but does not alone show that connectivity has learned the required response or route. No teacher pulse, replay schedule, online-learning policy or runtime data structure is selected by this outline.

### Expected participation, observed response and shortfall

For a count-based illustration, use `q` for a coordinate/polarity population or another explicitly justified target partition:

```text
E_(k+1,q) = neurons permitted to supply the expected next-step response
A_(k+1,q) = naturally active neurons in that partition, before training assistance
w*_(k+1,q) = desired participant count for this response stage

C_(k+1,q) = E_(k+1,q) \ A_(k+1,q)
d_(k+1,q) = max(0, w*_(k+1,q) - |A_(k+1,q) intersect E_(k+1,q)|)
R_(k+1,q) subset of C_(k+1,q)
```

`R` is the selected repair group. Selecting `min(d, |C|)` cells illustrates filling a count shortfall; partial correction, robustness margins and activation-intensity targets are alternatives, not adopted formulas. If the eligible missing population cannot supply the target, that target/eligibility combination needs resolution; it does not authorize recruiting semantically unrelated cells. These sets describe the algorithm's meaning and imply no stored bitmaps or auxiliary arrays.

Under H-64, `E` includes progression eligibility: an earlier participant cannot be credited as a correct new forward-step target merely because it has the right coordinate/polarity. The global-plan discussion below separates forward selection from permitted return. Inadmissible natural reactivation remains a separate error rather than reducing the correct-response deficit.

For example, if the intended contribution is 20 cells, 12 eligible cells activate naturally and 40 eligible cells remain inactive, selecting eight missing cells fills the count deficit: 20% of the missing candidate pool. Selecting 20 additional cells would overfill the stated target. This example does not choose a universal repair fraction or equate cell count with graded activation strength.

The expected population must come from the training goal within the epoch scope, not be inferred solely by declaring whichever cells happened to fire correct. A precomputed reference group or a quota-defined subset of the remaining epoch cells can supply a next-step target. DMN's whole-CC scope still needs a rule for expected participation, qualified coverage and negative-semantic preference. Token targets must satisfy H-66's per-step or few-step fidelity preference, with naturally correct participation credited before selecting the repair contribution.

An adequate total active fraction can coexist with missing activation in required populations. The repair error is therefore distinct from the global activity regulator and from the regime detector; H-56's exclusion of total active fraction from classification remains unchanged. Incidental noise and meaningful noise-assisted recurrence must be distinguished according to the learning objective, without automatically importing the proposed detector-density filter into plasticity.

### Repair must respect maintenance, growth and fading

The target is appropriate next-step activation, not a constant minimum chain width for all time. A token response that is fading as intended should not be repeatedly restored to its initial strength. The expected response can differ during ordinary token fading, positive-residual growth/peak/fade, DMN maintenance and a legitimate input-driven transition. How these conditions supply a target envelope remains open; no fixed schedule or state classifier is selected.

A naturally successful step has no positive shortfall under this repair rule. Extra or incorrect activation, wrong semantic direction and insufficient external steerability are separate possible errors; absence of a deficit is not complete behavioral success. H-63 does not yet specify an excess-activation reduction rule, pruning policy or negative learning update.

Training-time corrective stimulation, if used, is distinct from the required single external token event during autonomous-response assessment. Similarly, repair must not force the network back into an old chain when an external token is supposed to redirect it. Initial construction and repair share the deficit criterion while retaining these scenario-specific goals.

## Global constraints and stepwise sampling

**Interim recommendation: define whole-chain constraints first, then sample concrete repair targets step by step within them, assessing the result in the existing CC graph.** A complete fixed list of every future neuron is one possible implementation of a plan, not a prerequisite. Width/strength matching alone is insufficient to establish the intended trajectory.

Here “whole-chain” refers to a training trajectory or reference cycle; the CC supplies shared neuron and connection resources. This is an analysis scope, not a chosen network-wide coordinator or one permanent phase assignment per neuron. Different tokens may use shared neurons in different contexts. Multi-CC training coordination remains unspecified.

### What a static plan would constrain

| Planning concern | Purpose for adaptive sampling |
| --- | --- |
| Semantic eligibility and expected proportions across stages | Keep a convenient replacement cell from changing the represented response. |
| Width and strength envelopes, including growth and fading | Define how much is missing without restoring a fading response to constant strength. |
| Ordinary progression, allowed skips/dwell if any, and return opportunities | Distinguish completing a path from closing it prematurely. |
| Head/tail groups and acceptable return lengths/timing | Make multi-step closure intentional rather than treating every earlier neuron as an admissible return. |
| Remaining eligible cells and cumulative connection capacity | Avoid completing the current step by exhausting the only candidates available later. |

H-65 specifies the epoch scope and width/body-enhancement/return-attenuation parameter roles within this plan. Exact envelopes, allowed transition pairs, a graph-search algorithm and persistent fields remain unselected.

### Useful small loops and main-chain progression

H-71 makes noise-supported small loops an intended source of longer-context influence. Avoid deliberately wiring a shortcut that defeats the main training progression; do not turn that construction preference into aggressive elimination of smaller cycles. Learned and emerging loops should be assessed for useful history-dependent effects, interference, coverage and external steerability. Neither recurrence nor cycle length alone establishes an error.

The whole-cell exclusion equations below remain a comparison for selecting a fresh forward target. They are not a requirement to suppress prior activity or reject every target reachable through a return path. In the fractional/rate proposal, persistent participation may carry context without being a new premature traversal. A small loop is harmful when its actual effect defeats the intended behavior; useful loops need not have been pre-enumerated as exceptions. [ContextLoopsAndNumerics](ContextLoopsAndNumerics.md#useful-small-loops-and-dominant-response-fading) separates dominant-response attenuation from retained context.

### Sampling needs a scoped participation history

Let `H_k` be the union of participant roles already admitted to the current traversal through step `k`, including naturally accepted participants and selected repair targets. It is not the union of every neuron touched incidentally by white noise. The criterion for admitting meaningful participation must be specified; a noise-origin label or detector-density rule is not presumed.

For semantic candidate domain `E_sem,k+1` within fixed epoch scope `S_e` and current natural activation `A_k+1`, a schematic rule is:

```text
ordinary forward candidates = (E_sem,k+1 \ H_k) \ A_k+1

permitted target domain = (E_sem,k+1 \ H_k) union B_k+1
repair candidates = permitted target domain \ A_k+1
```

`B_k+1` is a specifically permitted return subset of `E_sem,k+1 intersect H_k`; it is empty during ordinary forward progression. Future feasibility and existing-graph compatibility further restrict these domains. The equations express one candidate policy, not a selected sampler or memory representation.

The history analysis here concerns distinct forward-group roles. Persistence of a rate in the same neuron across numerical updates is not by itself reuse of a prior logical step. Fractional overlap still shares that neuron's outgoing connections, so unintended shortcuts remain a dynamics question.

History scope matters. Excluding all previously used neurons forever would prohibit recurrence and eventually exhaust the population. Excluding only the last few steps would still permit a premature return to an older step. A traversal boundary with explicit return permission is therefore worth discussing. It does not imply that every observed activation must follow a rigid global schedule or that later training iterations cannot reuse cells.

Crediting natural activity toward a forward target must respect the intended response role. Earlier activity can also remain useful context without being counted as a new successor. If a return actually displaces the required progression or defeats external steerability, adding forward cells may not correct that interference. Any response to such a demonstrated failure must account for other useful behaviors; H-71 excludes aggressive small-loop elimination as a default policy.

Excluding an earlier neuron from a new target role is not a command to silence all earlier activity. Residual or predictive activity can be legitimate under the neuron model. The response criterion must distinguish permitted carryover from a premature return that displaces the intended progression; a single repeated firing does not establish a shortcut loop.

### Width and strength can fail differently

For illustration, width counts meaningfully active cells, while strength could sum their decoded active-state magnitudes. Their exact definitions remain open. Ten strongly active cells and twenty weakly active cells can have the same total strength and different widths. Thus a single total-activation number can hide either a participation deficit or an intensity deficit.

If width is adequate but strength is low, adding more targets may miss the intended correction; changing existing support is an alternative. If strength is adequate but concentrated in too few cells, adding the missing participants without considering the existing drive can overshoot. This motivates evaluating both aspects of the intended response before selecting a repair. It prescribes neither two controllers nor new counters and leaves the global activity regulator and regime detector unchanged.

### Current-step feasibility does not ensure a completable chain

Consider two future steps requiring one cell each. The current step may choose `{a,b}`, but the later step may choose only `{a}`. Selecting `a` now exhausts the later step; selecting `b` leaves a feasible completion. Both local choices meet the current width target.

The 60-cell coordinate/polarity constraint remains important, but total remaining population alone is insufficient when future eligible sets differ. For any collection of future steps that must use distinct cells, the sum of their required counts cannot exceed the size of the union of their eligible sets. Reserved return participation is treated according to the permitted reuse rather than counted as a new distinct-cell requirement. Available incoming slots and existing support impose additional constraints.

A global plan can therefore reserve scarce options or use lookahead while leaving actual identities flexible where alternatives are plentiful. This motivates a feasibility check; it does not choose a matching solver, reservation array or exhaustive enumeration. When no completion remains, widening the candidate domain, shortening the planned traversal, changing widths or permitting reuse are different design changes, not silent sampler fallbacks.

### Fresh targets can still close unintended loops

Excluding earlier participants prevents direct reuse in the sampled sequence. It does not remove paths already present in the shared graph. A minimal example is:

```text
existing intended prefix:  a -> b
existing other path:       c -> x -> a
proposed new connection:   b -> c

combined cycle:            a -> b -> c -> x -> a
```

`c` is absent from the traversal history `{a,b}`, yet selecting it closes a four-edge cycle. In a directed graph, adding `u -> v` creates a cycle through that edge exactly when the old graph has a path `v -> ... -> u` (including the self-edge case). Individually nonrepeating chains can therefore form cycles when their connections are combined.

This is a graph statement, not proof that the cycle will carry harmful activity. Edge strengths, timing, thresholds, cancellation and noise determine whether it produces an early return. Conversely, strengthening existing edges can make a previously ineffective loop dynamically important without changing reachability. Both new-edge and weight-only repairs need behavioral assessment.

Rejecting every reachable return would be incompatible with a recurrent network and expected immersion, and could exclude almost every candidate in a strongly connected component. The relevant error is a return whose actual effect disrupts the intended progression, semantic behavior or steerability. H-71 explicitly permits useful smaller loops, including noise-supported context. A permanent increasing phase rank shared by all neurons could restrict many loops structurally, but it would also constrain context-dependent reuse and later learning; it is not adopted.

### Multi-step closure changes the permitted cycle lengths

Spreading head/tail connections can smooth strengths while also adding shorter paths. In an illustrative group-level graph with `L` groups and consecutive transitions, a return edge from tail step `i` to head step `j` closes a cycle of `i-j+1` group transitions. With `L=12`, three head groups `0,1,2` and three tail groups `9,10,11`, all-to-all tail/head return permits cycles of 8 through 12 group transitions.

These are exact edge counts for the illustrative group graph, not simulated activity periods or a guarantee that every sparse cellular realization conducts every route. If only the twelve-step route is intended, smoothing weights alone does not enforce it. The permitted return pairs and their timing must be chosen consistently with the intended response. Even once several return lengths are allowed, their combined gain and concurrent waves need assessment; a single scalar loop-gain calculation cannot describe every route.

### Practical scope of graph and trajectory checks

Full reachability for every proposed target can be costly and overly restrictive. One graph traversal can inspect up to the live neurons and edges; repeating it per candidate can dominate local repair work. Under the proposed target-owned incoming representation, reverse traversal from a proposed source follows stored incoming lists to find cells that already reach it. An outgoing index is not intrinsically required for that particular check, but traversal work and temporary visited-state representation still need a budget.

Bounded path screening can expose direct or short indirect returns, while full-context rollout can assess effective activation, phase order, fading and external redirection. Bounded screening does not prove absence of longer shortcuts, and a finite rollout does not prove all contexts correct. These are candidate checks with complementary purposes; no hop limit, effective-edge cutoff, training scratch array or execution schedule is selected.

The design comparison is consequently:

| Approach | What it controls | What remains unresolved |
| --- | --- | --- |
| Entire sequence fixed beforehand | Reference order and planned cell allocation are explicit. | Entry, repairs after interference and cycles through other retained edges still matter. |
| Local sampling with a recent-history exclusion | Cheap control of some direct near-term reuse. | Older-step returns, future candidate exhaustion and existing indirect paths. |
| Whole-chain constraints with adaptive sampling | Combines reference progress/resource goals with use of naturally successful activation. | Requires explicit history/return scope, future-feasibility policy and assessment in the combined graph. |

The third approach is the current agent recommendation for further design. It establishes a concrete purpose for global consideration while retaining H-63's reparative learning direction. All detailed policies remain open.

## Fixed groups and semantic eligibility

Let `G_k` be an intended or realized participant group at chain step `k`, with width `W_k = |G_k|`; which meaning is used must be stated in each analysis. For maintenance, naturally correct participation and selected repair candidates are accounted for separately. A group is a mathematical description, not a required stored runtime object. Actual firing can differ from reference membership because of inputs, noise, cancellation and neuron dynamics.

For DMN, all CC neurons form the epoch scope, with approximate width determining how many participate at a step. For a token, a concrete proportional scope is selected once per epoch under H-65. H-62's preliminary fixed-batch preference remains a possible way to arrange that scope; stepwise sampling within it is also compatible with H-64/H-65. Fixing the scope does not require ignoring naturally successful participation or fixing all batch identities in advance.

**Choosing groups beforehand does not by itself make them disjoint.** Disjoint construction would additionally sample without replacement across steps within each eligible coordinate/polarity population. This is a natural interpretation to compare under H-62, not a settled exclusion rule. Reusing the same group on a later lap is essential to a closed loop and is distinct from assigning one neuron to several different steps within that loop.

The selected epoch cells must be distributed across the traversal under the width and no-premature-reuse goals. H-66 prefers token proportions in every step, permitting at most a few steps of bounded deviation. A trajectory that recovers the right proportions only over a full lap does not satisfy that preference. The [short-window feasibility analysis](#finite-populations-constrain-the-short-window) applies to precomputed and adaptively sampled groups alike; fixed coordinate identities remain unchanged.

### Disjointness has a per-population limit

For each coordinate and polarity `q`, let `C_q` be the eligible cell count, at most 60, and let `b_kq` be its contribution to step `k`. Fully disjoint groups require:

```text
sum_k b_kq <= C_q                    for every q
L * b_q <= C_q                       for L equal-quota steps
L <= min[q with b_q > 0] floor(C_q / b_q)
```

The coordinate needing the largest fraction can be the bottleneck even when most CC neurons remain unused. For illustration, six cells per step from a 60-cell polarity population permit ten disjoint steps; twelve permit five; thirty permit two. These are arithmetic examples, not chosen quotas. The weaker global condition `sum_k W_k <= eligible_population` alone can miss this bottleneck.

This limits distinct groups, not the number of laps or the response duration. A five-step ring can circulate many times; no step duration is presently selected. It also does not imply that every token must allocate all 60 cells of one polarity in every dimension: strength-to-count conversion remains a separate encoding decision.

### Alternatives worth comparing

| Construction | Expected advantages | Expected limitations / decision needed |
| --- | --- | --- |
| Precomputed, disjoint steps within each chain | Clear intended order; avoids same-cell cross-step shortcuts in that chain; straightforward accounting of newly used cells. | Tight per-population length limit; does not prevent cross-chain interference or guarantee propagation. Entry from a fresh stochastic token input still needs analysis. |
| Precomputed steps with controlled reuse | More steps under the same 60-cell population limit; can support shared representations. | A shared neuron also exposes its outgoing edges from other steps; interference, branching and premature return become possible. Any exclusion gap, reuse cap or context dependence needs its own rationale. |
| Resampling between construction attempts, then fixing the resulting graph | Can explore several candidate realizations and sensitivity to initial cell choice. | Accumulating all attempts can consume slots and blur order. Replacing, merging or selecting attempts are distinct policies; none is adopted. |

These policies can use a fixed graph during autonomous assessment between training updates. Deficit-driven candidate selection during training is distinct from changing intended membership on every inference step. Its connection updates and any training assistance need explicit semantics; no inference-time routing or rewiring mechanism is presumed. Ordinary stochastic firing on fixed edges remains compatible with precomputed reference groups.

The current comparison concerns fixed reference populations versus quota-defined candidates within the [global constraints](#global-constraints-and-stepwise-sampling), including their effect on future feasibility, semantic fidelity and interference. Strictly disjoint reference groups retain the population limit above; a repair-driven trajectory is not automatically a fixed partition of all neurons into steps. Participation history has a concrete training purpose, but its representation and allocation still require design rather than a preset array.

## Fan-out, fan-in and slot arithmetic

The full-group construction arithmetic below remains a reference case. In repair, sources may already support some next-step cells, and selected targets may already have useful incoming edges. Existing input and the missing contribution must be distinguished; `M_in` is not automatically a count of new edges needed for every target.

For the newly added distinct edges only, `E_new = sum_source n_new = sum_target m_new`. Weight changes to existing edges consume no new endpoint slots, and targeting already present endpoint pairs requires a merge/update policy. Repairing a shortfall does not require rebuilding all edges between complete predecessor/successor groups. Whether to strengthen existing edges or allocate new ones first remains open.

For adjacent groups with sizes `W_k` and `W_(k+1)`, write `N_out,k` for the user's outgoing `N` and `M_in,k` for incoming `M`. These names avoid confusing them with total neuron count and total slot-set count in the capacity documents. For distinct edges between these groups:

```text
E_k = W_k * N_out,k
mean(M_in,k) = E_k / W_(k+1)

If every target has exactly M_in,k:
W_k * N_out,k = W_(k+1) * M_in,k
```

At equal width, average fan-in equals fan-out. They cannot both be chosen independently after widths are fixed. For a simple bipartite graph between disjoint groups, exact uniform degrees also need integer balance, `N_out,k <= W_(k+1)` and `M_in,k <= W_k`. Shared-group/self-edge restrictions and already present edges introduce further constraints.

If each source independently chooses `N_out,k` distinct targets uniformly, one target's fan-in has a binomial distribution:

```text
p = N_out,k / W_(k+1)
M_j ~ Binomial(W_k, p)
E[M_j] = W_k * p
Var(M_j) = W_k * p * (1-p)
```

Thus “each target receives M” is an expectation unless the construction explicitly balances degrees. With two width-1,000 groups and fan-out 20, there are 20,000 edges; mean fan-in is 20 with standard deviation about 4.43 under this independent selection scheme. A balanced random bipartite construction is a candidate when exact fan-in is desired; no generator is selected here.

The degree in these equations is the contribution from this transition, not the target's total live degree across all chains. New physical slots are consumed only for distinct retained endpoint pairs not already represented; repeated use needs an explicit weight-update/merge policy. Existing edges cannot be counted as newly allocated slots on every traversal.

At five bytes per retained edge, the example transition occupies 100,000 bytes in the source-ID and weight arrays if all edges are new, with no per-edge alignment padding. Allocation-class reservation and existing owner/current-set/state arrays remain separate. A 100-slot minimum class does not require 100 new edges from each individual chain stage.

For `L` equal-width transitions, the unreused edge count is `L * W * N_out`, plus any additional closure edges not included in those transitions. Multi-step closure must be counted explicitly. As a conditional scale check, if one CC represents all 127,997 ordinary token types, the 15-class candidate's 106,492,416 slots amount to about 832 slots per type before allocating DMN or later learning capacity. This is not an allocation rule; it shows that private, wide chains for every token cannot be presumed. Shared edges/groups, admitted vocabulary and per-CC responsibility need discussion, and sharing alone does not guarantee interference-free capacity.

## Propagation depends on the neuron transition law

H-77's active-rate cancellation is part of the response that maintenance must observe: opposite polarities are netted over a minicolumn and surviving rates are retained proportionally. Repair must not restore the canceled losing polarity merely to match an uncanceled raw target. Targets and observations need to refer to the same stage, while actual prediction-guided entry under H-76 must remain distinguishable from training assistance.

Cancellation couples the cells of a minicolumn. Changing one incoming drive changes its population total and can change other cells' retained rates. A repaired raw sum for one cell is therefore not necessarily the same correction in the completed response. Reobserve the shared post-cancellation behavior instead of assuming independently computed raw cell deficits remain independent; this requires no new connection index or global optimizer.

An intended next group must actually activate after the preceding group, while preceding activation can subside. Width, fan-in and weights do not alone define this sequence. Required model details include integration timing, activation threshold/transfer, activity persistence or decay, phase transitions, cancellation ordering and state/weight rounding. No explicit inhibitory synapses are introduced by asking these questions.

A limited analysis can write an effective transition gain:

```text
a_(k+1) ≈ g_k(a_k, context) * a_k + eta_k
```

Here `a_k` is a chosen response amplitude, not automatically the regulated active-neuron fraction; `eta_k` represents background drive and other inputs in this approximation. Gain depends on the currently active sources, their timing, target response and competing activity. `M_in * mean(weight)` describes summed input only under restrictive linear/equal-source assumptions, not a general firing guarantee.

Constructing edges gives a plausible route for propagating population activity, but a route that works in isolation may behave differently after other chains and noise feedback are present. Particularly important are unintended multi-step skipping, several simultaneous waves, lingering activation and loss of input steerability.

## Closure across several head and tail steps

The working expectation under H-66 retains tail-to-head connectivity. H-65 assigns enhancement to the chain body and attenuation to the return transition. H-61's multi-step head/tail groups distribute that transition smoothly. In schematic form:

```text
head:  G_0, ..., G_(h-1)  →  interior  →  G_(L-t), ..., G_(L-1) :tail
             ↑                            |
             +----- distributed return ---+
```

The return is a set of candidate weighted transitions between selected tail/head steps, not necessarily all-to-all connectivity. Logical step order is unrelated to physical distance. Which pairs participate, which delays they imply, and whether smoothing refers to a fixed profile over steps or gradual weight updates during construction remain open.

The body/return division has a useful conditional interpretation. For a single coherent wave, negligible additive drive and approximately linear scalar transfer over one lap:

```text
G_loop = product_k g_k
a_(r+1) ≈ G_loop * a_r
```

`G_loop < 1` gives attenuation measured at the same phase on successive laps, `G_loop > 1` gives initial amplification there, and `G_loop ≈ 1` gives approximate maintenance. The product includes both body enhancement and return attenuation, as `B * D` above. Once head/tail transitions overlap, the multi-group response is more general than this scalar product. Nonlinearity, saturation, noise, cancellation and other chains can change the result.

To illustrate smoothing only, a desired multiplier `q > 0` spread across `r` otherwise matched transitions could use per-transition multipliers `q^(1/r)`. This preserves their product without concentrating the change at one boundary. It is not an adopted weight formula; widening a return block also changes edge counts and concurrent input, which must not accidentally multiply the intended gain. Smoothing mathematical weights does not guarantee smooth quantized population activity near thresholds.

For a decaying scalar lap with period `T_loop`, constant `0 < G_loop < 1` and no additive drive, the envelope would reach amplitude `a_end` after approximately:

```text
t_end ≈ T_loop * log(a_end / a_start) / log(G_loop)
```

This makes the missing timing and response criteria explicit; it does not select a decay constant or equate amplitude with the DMN detector. The loose half-life-like token target cannot determine closure weights before those quantities are defined.

**Body enhancement and distributed return attenuation provide an explicit candidate for growth followed by reduction within a lap.** A within-lap peak is compatible with a net lap gain below one; the scalar lap formula compares matching phases across laps, not every intermediate step. Whether the complete semantic response grows preferentially from a weak residual, reaches the intended peak and then fades smoothly under background noise remains an assessment of the full dynamics. This candidate does not require adopting runtime weight changes, fatigue, an inhibitory circuit or new state fields.

## Entry, coexistence and regime acceptance

**Token inputs must reach useful recurrence through both H-76 entry branches.** A sufficient predictive population receives strong activation; otherwise a broad population receives weak activation. The response remains one token event, with coordinate strength and contextual selection both respected. Evaluate unfamiliar or conflicting generation inputs as well as predicted ones; a fixed head is not presumed. The explicit broad weak fallback is not a learning target under H-84. Maintenance concerns the learned recurrent response, with its teaching operations separately defined. For an independently sampled comparison only, input size `x_q` and group size `b_kq` within `C_q` cells overlap by `x_q*b_kq/C_q` on average. H-76 does not choose that unconditional sampler. No separate routing table or per-token chain ownership is implied.

**Overlaps across tokens are separate from overlaps within a chain.** The same 60 cells per polarity must serve many token contexts. Disjoint steps within one chain do not make different token chains disjoint. Shared neurons can activate several successor groups, and combined retained edges may shift gain or create shortcuts. Future flow learning must be assessed for its effect on the constructed behavior; neither frozen base weights nor a separate edge-class tag is presumed.

**DMN eligibility is not DMN coverage.** A fixed loop visits at most the union of its groups. If that union is small, repetition does not produce broad qualified coverage; a whole-CC candidate domain alone is insufficient. Conversely, visiting most cells once may still fail the proposed density-based noise filter. Width, group count, repeated visits and the observation window must be compatible with the existing coverage definition.

**Unbiased random construction supplies no negative-semantic direction.** Whole-CC access and recurrent maintenance do not identify the intended valence drift. The asymmetric semantic preference and its relation to group selection, dwell, weights or dynamics remain to be defined. Coordinate negativity cannot substitute for negative meaning; intrinsic opposite-polarity cancellation also affects the response.

**Immersion remains an outcome to examine.** Recurrent portions of the combined network may support limited-ensemble loops. Whether these loops carry useful semantics, fall in the intended length range and remain externally steerable is unresolved. Merely observing a loop does not establish immersion, and the absence of a separate immersion loss does not remove its behavioral acceptance conditions.

## Scientific comparison and read scope

The proposal is related to synfire-chain and synfire-ring models through successive population activation and recurrence. Strict spike synchrony is not selected for the individual-neuron rate proposal.

- [Zheng and Triesch, 2014](https://www.frontiersin.org/journals/computational-neuroscience/articles/10.3389/fncom.2014.00066/full), also LOC-071 in the completed research ledger: recurrent population rings emerge in their model, with switching involving inhibition and intrinsic plasticity. Current review refreshed ring behavior and the formation discussion; the existing ledger records the wider methods read. Their multiple plasticity mechanisms and inhibitory network are not this direct construction or our early architecture.
- [Kumar, Rotter and Aertsen, 2008](https://brainworks.biologie.uni-freiburg.de/2008/journal%20papers/kumar-jneurosci-2008.pdf), neuron/network/embedding methods and rate-propagation, overlap and discussion sections: activity can reach later groups while input-rate distinctions degrade. Their rate experiment supplies ongoing Poisson input for 500 ms, not a one-shot cue. Fully interconnected 300-cell groups in a conductance-based excitatory/inhibitory network differ from our proposal; its restricted rate-propagation regime is not a universal rejection of rate updates. Shared cells and induced surrounding activity can also ignite another chain while background activity remains stable. This supports checking graded fidelity and unintended cross-activation in the shared network, without selecting their width, firing rates, spatial layout or cellular mechanisms.

These studies motivate relevant questions; the construction arithmetic and overlap limits above are deductions from the proposed model. No author simulation, application training or project tests were executed.

## Next discussion

The immediate discussion combines [fractional recruitment and rates](RateDynamics.md) with [useful context loops, accumulated/scheduled decay and numeric choices](ContextLoopsAndNumerics.md). H-67 relaxes response timing to a half-life-like objective; H-68 retains weights when propagation ceases; H-69 proposes fractional selection and H-70 requires numerical care. The epoch scope and width/body-enhancement/return-attenuation roles must be reconciled with this proposal. Their open choices include per-step or aggregate enhancement, step timing, return profile and the desired response duration under the relevant noise conditions. Under the per-step preference and few-step allowance, define the short rolling window, error tolerance and actual-response observable. Check their feasibility against the finite neuron population, fractional/whole-cell participation semantics, progression and width before choosing a sampler.

Next specify how selected targets cause a durable repair without creating an unintended effective shortcut through existing connections: edge changes, training assistance, bounded structural checks and full-context response assessment. A minimal neuron transition/integration contract is still needed for gains and timing. Repair frequency, excess/incorrect-activation handling, resource budgets and interaction with future flow learning remain discussion items.
