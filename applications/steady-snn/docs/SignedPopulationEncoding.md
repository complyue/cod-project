# Steady recurrent SNN: separate positive and negative populations

Document-set revision: 202 · Recorded: 2026-10-28 · Topics: SNN-01–SNN-05, SNN-07 · Proposal under discussion.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** C-14/P-27 expresses signed semantic input through contextual cell identities; C-18/P-36 reduces opposing activity while retaining the signed residual. P-28/P-37/P-53 constrain capacity and phase/encoding compatibility without granting authority. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** retain the provisional capacity-class rule, static pools, parallel five-byte slots, regional relocation/lifetimes and training ownership of weight-scale policy. This analysis selects no biological equivalence, neural transition, four-bit format or implementation. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** signed reconstruction and contextual discrimination must coexist; measure zero/sign transitions, prediction versus current activation, recurrence, resource costs and message information loss under the eventual selected contract. Criteria are detailed below; numerical tolerances remain open.
- **Human baseline:** H-27/H-35 fix the current Flash 60+60 sizing case; H-43 fixes roles; H-54/H-55 set the residual behavior; H-75 clarifies contextual paths, H-76 input branches, H-77 proportional net cancellation; H-78 defers state representation. The linked registry retains exact evidence and exclusions. H-84 excludes the explicit broad weak generation fallback from learning.
- **Current assessment:** [K-199](../../../design/DesignAuthorityRegistry.md#k-199) connects prediction-guided entry to the existing contextual assessment under background activity. Branch occupancy does not establish learned readiness; H-78/H-85 alternatives remain open. No threshold, gate, noise-origin label or additional state is selected. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; empirical choices remain open.

## Current planning case: Flash 60+60

The current planning case uses DeepSeek-V4.1-Flash and **60 positive + 60 negative cells per semantic dimension**. The verified text embedding width is 5,120, giving **614,400 neurons**, including 307,200 of each polarity. There are 5,120 dimension groups and 10,240 polarity populations. The worked 15-class [capacity calculation](CapacityPlanning.md#flash-6060-calculation) uses A=311296 sets in the 100-slot class and gives **515.150806427001953125 MiB** of named arrays. A is a working quota for this calculation; final pool provisioning remains a choice.

## Population structure and identity

Each dimension group has separate positive and negative 60-cell populations. The number of cells does not establish an equal number of independent contexts; contextual capacity also depends on connectivity and learning. Whether each polarity population or the whole dimension group is called a minicolumn remains terminology to settle.

A fixed-ID illustration is `id=120*d+60*s+c`, where `s=0` for positive, `s=1` for negative and `c=0..59`. This is not an adopted ABI. IDs fit in uint32; polarity can be derived without another sign field. Learned connectivity belongs to each persistent cell, including while its polarity is inactive.

H-43 makes the semantic response role a static assignment. The current per-coordinate positive/negative population identity is part of that assignment; activity, predictive state and learned connections can vary within it. Approximate-replica CCs do not imply identical weights or simultaneous activation. H-42 rejects internal positions as a source of cross-minicolumn distance bias, so the ID illustration supplies no geometry. [Semantic cortical mapping research](SemanticCorticalMappingResearch.md) keeps topology separate from learned semantic reassignment, which is outside the current baseline. Exact response curves and signed decoding remain open.

## Signed input and output

Let `g_d>0` be a specified coordinate normalization and `u_d=x_d/g_d`, with the represented range and overflow behavior defined by the encoder. The following illustration assumes `u_d` is in [-1,1]:

```text
u_plus  = max(u_d, 0)
u_minus = max(-u_d, 0)
x_hat_d = g_d * (A_plus - A_minus)
```

Here A_plus and A_minus are decoded current-activation magnitudes, not yet defined as raw counts or means. The coordinate normalization is an encoding/readout choice, separate from synaptic weight-scale training policy. Its representation and any storage need an explicit budget if selected.

For normalized input -0.6, the external encoder drives the negative population with nonnegative magnitude 0.6. Negative output arises from the decoder's subtraction. **Semantic polarity is distinct from synaptic excitation/inhibition:** a negative-population cell can excite another negative-population cell, reinforcing a negative coordinate. Treating every negative cell as an inhibitory source would select a materially different recurrent model. Any requirement that a negative input suppress particular targets must be expressed through their integration/connection rules.

Signed population decoding has a modeling precedent in the [NEF implementation example](https://www.nengo.ai/nengo/v4.0.0/examples/advanced/nef-algorithm.html), which uses positive/negative encoders and learned readout weights. It does not validate this particular equal-sized split, FP4 state or HTM-inspired recurrence. The equations here are application proposals.

## Magnitude and contextual cell selection

The HTM reference distinguishes cells sharing a column input by temporal context. Predicted input can select a subset of cells; an unpredicted active column can activate all its cells. This motivates separating input magnitude from the number of contextual hypotheses represented. [Hawkins and Ahmad, §2.2.1](https://arxiv.org/pdf/1511.00083).

H-75 makes this role explicit for our model: different neurons within a minicolumn represent different contextual paths for the same semantic component. Different cell patterns with the same aggregate signed value are therefore meaningful. Amplitude-preserving redistribution must not assume that all cells are interchangeable, and a training repair should retain successful contextual participation rather than canonicalize every response to one pattern.

At fixed nonzero signed total, different relative rates among the surviving cells can retain contextual distinctions. [Cell-composition arithmetic](ContextLoopsAndNumerics.md#cell-composition-can-change-while-the-semantic-value-stays-fixed) shows how added drive can change those distinctions while H-77 preserves the same aggregate. The normalized proportions are analytical descriptions of existing rates, not additional stored state; their usefulness is assessed through later context-sensitive behavior.

For this application, a raw mean over 60 cells gives 1/60 (about 0.01667) when one cell is fully active and 1 when all 60 are fully active. If both patterns encode the same observed coordinate under different contextual certainty, raw averaging would turn surprise into a sixtyfold amplitude change. Splitting polarity does not resolve that ambiguity.

| Encoding direction | What the cells do | Design consequence |
| --- | --- | --- |
| Count/probability code | Activation fraction represents magnitude. | Context selection must preserve a calibrated magnitude estimate; independently selecting fewer cells can bias it. |
| Context selection with graded magnitude | Cell identity/subset represents context; selected cells carry magnitude through graded activity. | Define a decoder calibrated for the selected subset and bursting rule, or explicit column-level amplitude. Extra amplitude state is not automatically allocated. |

For the first direction, an illustrative independent binary sampler with 60 cells and activation probability p has normalized standard deviation `sqrt(p*(1-p)/60)`, about **0.06455 at p=0.5**. Count resolution is 1/60 (about 0.01667), which is not the same as that sampling error. This is arithmetic for an ideal sampler, not a measured FP4/SNN accuracy claim. Correlated firing, context selection, graded states and temporal filtering change the result.

H-53 specifies one presentation according to token-coordinate strengths. H-76 defines its contextual selection: when a sufficient proportion of relevant cells is already predictively activated, strongly activate those cells; otherwise weakly activate a broad part of the minicolumn. Randomness, where used, remains part of that single presentation, not repeated token forcing.

### Prediction-guided token input

| Existing prediction at token arrival | Required input response |
| --- | --- |
| A sufficient proportion is predictively activated | Strong activation of those predictive cells. |
| Predictive participation is insufficient | Low-intensity activation across a broad part of the minicolumn. |

H-84 locates the broad weak fallback in generation mode. It is an explicit activation operation, not transmission produced by ordinary learned connections, and is excluded from learning. Flow training must not treat that fallback population as a teaching pattern or infer learning recipients from it. This boundary does not specify the separate algorithm for learning unfamiliar sequence transitions.

The common minicolumn role carries the semantic component; selection among its cells carries the contextual path. The broad response provides multiple weak candidate paths when a concentrated prediction is unavailable. Numenta's Temporal Memory supplies the selective-predicted versus broad-unpredicted distinction; H-76's sufficient-proportion criterion and graded strengths are the application's rule, not parameters measured or prescribed by that source. [Primary algorithm description](https://www.numenta.com/assets/pdf/temporal-memory-algorithm/Temporal-Memory-Algorithm-Details.pdf).

The prediction is read from the state before applying this token. In the two-frame proposal, its result contributes to the next frame, with new recurrent propagation using that completed frame on the following transition. Prediction must be compatible with the coordinate/polarity being stimulated; opposite-sign readiness does not by itself predict the arriving signed component. The exact predictive criterion, fraction denominator, sufficiency threshold, broad coverage and high/low strength functions remain to define.

Coordinate strength and contextual confidence have distinct roles. If `m` predictive cells receive rate `h`, while `b` broadly selected cells receive rate `l`, their summed rates are `m*h` and `b*l`. Low per-cell strength alone does not make a broad response weak in aggregate. For illustration, six cells at 1 and sixty at 0.1 both sum to 6. These are not chosen settings. Preserving an intended coordinate total by distributing it among selected cells is one candidate; confidence-dependent aggregate gain is another material choice because differing gains across coordinates can change the vector. H-66's short-horizon semantic objective must be assessed rather than assuming either normalization automatically.

The rule gives predictive readiness a concrete job: it directs the next input toward contextual cells. A state representation that maps every inactive and predictive cell to the same zero value cannot perform that job. This does not automatically require a new field; the existing state semantics must expose the distinction. Strong current firing and readiness for a later input still need a coherent interpretation.

H-78 leaves one activity value with predictive/active ranges versus distinct readiness/current firing open for future controlled comparison. Neither a final phase encoding nor separate arrays are selected. The input branches and H-77 cancellation are requirements that each candidate must interpret coherently.

Background activity makes this comparison consequential: a value falling in a proposed predictive range need not carry contextual evidence. The fraction of inputs taking the focused branch measures branch use, not prediction quality. Compare pre-input selectivity and the resulting contextual response on genuine shared and distinguishing histories under the declared noise conditions, using the [shorter-context assessment](FlowLearning.md#assessing-contextual-prediction-on-actual-sequences). Spontaneous readiness is not automatically erroneous; it can reflect learned recurrence. Separate readiness can avoid conflating two numerical ranges, but does not itself establish selectivity. This comparison requires no noise-origin tag, extra gate or new persistent field.

Faithful token encoding and the combined network response also require separate assessment. Prior context and cancellation intentionally change what follows an input; the entire resulting state need not equal the latest isolated token. Thus the calibration question does not justify projecting every post-cancellation state back onto that token vector. [Conditional input analysis](../research/input-context-calibration.json) distinguishes a common gain from coordinate-dependent gains and separates weak nonzero support from the qualified coverage used for regime detection.

## Token distribution during recurrent progression

The epoch's selected cells and the per-step participants serve different roles. H-65 selects the enclosing token-proportional scope once per epoch. H-66 prefers maintaining the token vector's distribution in each step, allowing at most a few steps of bounded deviation. A whole-epoch or whole-lap average alone does not satisfy that direction. Relative semantic proportions can remain near the token while their common response amplitude grows or fades; the response is not required to hold its original absolute magnitude throughout recurrence.

Keep the original signed-vector proportions, finite sampled population counts, intended next-step participants and actual state/readout strengths distinct. Rounding or sampling a meaningful coordinate out of the epoch scope cannot be repaired by later rearrangement of those same cells. Under a no-reuse forward traversal, a small selected population also limits how often that coordinate can appear. [TrainingStrategy](TrainingStrategy.md#finite-populations-constrain-the-short-window) analyzes this constraint and its coupling to width, lap length and repeated circulation.

Balanced per-population sampling is a proposal for meeting the short-horizon target. In repair, balancing must account for eligible cells that activated correctly before choosing the missing contribution; the repair subset need not itself have the token's full proportions. Balanced membership still does not guarantee balanced realized activity after thresholds, noise, cancellation and quantization. The readout metric, allowed short window and numerical error bound remain open, as does separating the relevant token response from concurrent background or other semantic activity. This requirement introduces no temporal-decoder array or additional per-cell field.

### Fractional recruitment and rate-valued contributions

H-71 opens INT8/INT4 comparison for modeled quantities. A nonnegative magnitude can use an unsigned scaled integer, while signed residuals and phase information need their own representation semantics. The current FP4/FP8 array budgets are conditional baselines, not proof that these formats are optimal. [ContextLoopsAndNumerics](ContextLoopsAndNumerics.md#integer-formats-are-field-specific-choices) keeps weight storage, state storage, accumulation and scaling separate.

H-69 proposes fractional selection together with H-67's discrete-time rate modeling. A coordinate contribution can then depend on summed effective activity rather than only integer cell counts. This can preserve a small magnitude without repeatedly recruiting a whole new neuron, provided the numerical format represents it. [RateDynamics](RateDynamics.md#fractional-recruitment-and-rate-have-different-possible-meanings) distinguishes fractional target membership, probability/duty interpretations and conditional versus effective rate.

Neither two independently stored factors nor a population collapse is selected. Fixed cell identities and their shared learned outputs remain; overlap can still cause interference. H-70 requires examining fractional encoding, accumulation, cancellation residuals and slow decay together. FP4 storage alone does not guarantee sufficient effective precision; H-73 permits wider state as a direct candidate; any additional fields still require a defined purpose and accounting. H-66's short-horizon signed proportions remain an objective while amplitude fades, with a low-signal boundary still to define.

## Coactivation, prediction and transitions through zero

For one observed scalar, rectified external drive activates only the matching polarity; at exact zero, both external drives are zero. Before intrinsic cancellation, provisional population totals `(A_plus,A_minus)=(0,0)` and `(0.8,0.8)` have the same signed net and different unsigned drive. H-77 maps both to zero active rates at the completed cancellation boundary. Individual contextual patterns can still differ at a nonzero surviving total; predictive readiness has its own open handling under H-85.

H-54 requires intrinsic mutual inhibition/cancellation between the two populations. H-77 specifies its active-rate operation: compute the minicolumn-wide net remainder and proportionally retain active rates in the surviving direction. Cancellation affects internal activity; subtracting two unchanged populations only at readout would not satisfy that purpose. Timing and treatment of predictive readiness remain open. This function needs neither index pairing nor learned inhibitory edges between the 60+60 cells.

H-85 explicitly defers whether opposite-sign predictive readiness should coexist or undergo net cancellation to future controlled comparison. H-86 directs obvious empirical alternatives to be recorded with meaningful comparison observations without repeated questions. This leaves H-77 active-rate cancellation and H-78 state-format comparison intact.

Current activation and predictive preactivation also need separate readout semantics. Both polarities might predict alternative next states even when only one is observed now. A rule forbidding all activity in the opposite polarity could destroy those alternatives. Specify which phase contributes to the emitted instantaneous vector and what happens when the coordinate changes sign. Zero input does not, by itself, require erasing recurrent state or learned connections.

Fixed polarity removes the need for a cell's semantic sign to change. It does not automatically provide another usable bit in standard FP4, whose encoding is fixed. The 16-code phase/intensity allocation, simultaneous current/predictive state and any extra buffers remain the [SNN-02](Model.md#snn-02) design question.

H-73 permits FP16/FP32 state; the phase/readout question survives at those widths. H-75's two temporal frames give a consistent previous/next boundary, not automatically two independent current/prediction fields. Cancellation can be organized around both polarity populations' provisional next values before publishing the completed frame. Its effect on contextual identities and predictive readiness still needs an explicit rule; no persistent third frame or extra phase array is presumed.

### Cancellation arithmetic and the semantic residual

An illustrative calculation separates signed readout from total activity. Let `P,N >= 0` measure the two populations in common units, and let `c` be an equal amount removed from each, with `0 <= c <= min(P,N)`:

```text
P_after = P - c                    N_after = N - c
P_after - N_after = P - N
P_after + N_after = P + N - 2*c
```

H-77 selects full net cancellation of active rates: `c=min(P,N)`. For positive-cell rates `p_i`, negative-cell rates `n_j`, and `D=P-N`, the proportional operation is:

```text
if D > 0:  p_i_after = p_i * D/P;       n_j_after = 0
if D < 0:  p_i_after = 0;               n_j_after = n_j * (-D)/N
if D = 0:  all active rates after cancellation are zero
```

Each division occurs only in a branch with a positive denominator. The signed aggregate remains `D`, the unsigned aggregate becomes `abs(D)`, and ratios among surviving-direction active cells are preserved. The operation is independent of array-index pairing. It removes opposing common activity while retaining the surviving population's contextual pattern in proportional form; it does not establish that a particular pattern is useful.

The operation is local to one minicolumn and needs aggregate arithmetic over its two populations. In the frame proposal, provisional next-frame rates can be canceled before that frame becomes visible to recurrent reads; a third full frame or persistent cancellation array is not required. H-77 specifies active spiking rates, not automatic erasure of all predictive readiness or learned weights. The phase interpretation and placement relative to the transfer function remain open.

Lower unsigned strength does not imply an equal proportional drop in the counted active neurons. The losing polarity becomes silent in active-rate terms; cells on the surviving side may or may not cross the eventual activity threshold after scaling. Active-fraction regulation and semantic readout therefore retain distinct meanings.

The equal 60+60 populations give an immediate bound: after full cancellation, no more than 60 cells per dimension can have positive active rate. Across a CC this is at most 307200 of 614400 cells, or **50% instantaneous active support**. This says nothing about predictive readiness or coverage across a time window; both polarities can be visited at different times. A future activity target must use a counting rule compatible with this bound.

The cancellation boundary matters for context. Suppose provisional positive rates are `(1,0)` and negative total is 0.8, then another positive contribution `(0,1)` arrives. Combining first and canceling once leaves positive rates `(0.6,0.6)`. Canceling the first contribution before adding the second leaves `(0.2,1)`. Both have signed total 1.2, but different contextual identities. Consequently, an optimization must not replace the defined combined cancellation with arbitrary per-contribution cancellation.

The [derived-properties record](../research/cancellation-properties-analysis.json) also establishes idempotence, positive homogeneity and an absolute L1 non-amplification bound for this operation on nonnegative rates. These are properties of cancellation alone. They do not guarantee network stability or small relative error when the signed remainder is near zero, and they introduce no new runtime calculation.

The [conditional precision arithmetic](../research/cancellation-frame-arithmetic.json) also shows why adequate state precision matters. With sixty positive cells at 0.50001 and sixty negative cells at 0.5, the exact summed residual is about 0.0006. Rounding both input populations to FP16 first erases that difference; FP32 retains about 0.000600815 in this example. This does not set a meaningful-signal threshold or prove all FP32 residuals accurate. Precision cannot recover information already removed by an earlier state write-back.

Semantic valence belongs to the whole pattern. A positive-semantic token can carry both coordinate signs, and need not be the negative of a negative-semantic state's vector. H-54's scenario specifically concerns a complementary positive input. Whether it produces opposing population activity, a reduction in aggregate activation and a semantically positive residual must be assessed together. The calculation alone proves neither the residual's meaning nor its lifetime.

H-55 requires the activity deficit to increase background noise and the positive residual to strengthen preferentially before reaching a peak and following the token-like fading objective. That growth/decline trajectory is a learned-behavior target. It does not follow from equal cancellation or from the semantic neutrality of white noise alone.

### Lexical contrast does not identify a complementary state

A [read-only Flash probe](../research/context-composition-analysis.json) examines eleven declared English contrasts, using exact leading-space vocabulary entries and only their 22 local embedding rows. All pairs have positive signed cosine, ranging from **0.2270 to 0.7255**. The sample is illustrative, not a representative valence dataset. Older [distributional-embedding research](https://aclanthology.org/P16-2074/) also distinguishes lexical contrast from distributional relatedness; its training intervention is not adopted here.

For a conditional cancellation comparison, give the prior negative-labeled row and positive-labeled input equal L1 magnitude. Add the input once and apply H-77 to the resulting coordinate net, excluding other transfer, clipping, noise and recurrence:

| Illustrative token entries | Signed cosine | Remaining net L1 / prior L1 |
| --- | ---: | ---: |
| ` good` / ` bad` | 0.5706 | 1.7692 |
| ` happy` / ` sad` | 0.3868 | 1.6715 |
| ` calm` / ` anxious` | 0.2270 | 1.5646 |
| ` positive` / ` negative` | 0.7255 | 1.8641 |

All eleven comparisons increase this rate-magnitude measure. Partial coordinate cancellation still occurs: the comparison is with the prior alone, not with the uncanceled sum of both contributions. Changing only input strength does not reverse this result in the stated reference. If `f(g)=sum_i abs(n_i+g*p_i)` and `sum_i abs(p_i)=1`, its right derivative at zero is `1-2*a`, where `a` is input L1 mass on coordinates opposing the prior's sign. Here `a` lies between 0.1332 and 0.3827. Convexity then gives `f(g)>=f(0)` for every nonnegative gain.

A broader diagnostic uses public word-rating supplements from [Warriner et al. (English)](https://link.springer.com/article/10.3758/s13428-012-0314-x) and [Xu et al. (simplified Chinese)](https://link.springer.com/article/10.3758/s13428-021-01607-4). Eligibility requires one actual ordinary token: lowercase leading-space English or a standalone two-to-four-character Chinese word, without pooling multiple tokens. Before vector access, a deterministic hash selects 64 eligible words in each of three bands. English bands are `<=3`, `>=7` and `4.5–5.5`; Chinese bands are `<=−1.5`, `>=1.5` and `−0.375–0.375` in the reported rating units. These analyst-selected bands are not established psychometric equivalents across languages.

Each language supplies 4,096 positive-input/negative-prior pairs plus neutral and within-band controls. The [artifact](../research/context-composition-analysis.json) preserves protocols, selections, ratings, row fingerprints and all aggregate results:

| Rated-word sample | Eligible words | Signed cosine: range (median) | Net L1 / prior L1: range (median) | Negative initial derivative |
| --- | ---: | --- | --- | ---: |
| English | 8,500 / 13,915 | −0.0535–0.4007 (0.0979) | 1.3824–1.6727 (1.4826) | 41 / 4,096 |
| Chinese | 5,960 / 11,310 | −0.0344–0.4707 (0.0820) | 1.3896–1.7235 (1.4704) | 47 / 4,096 |

Every equal-unit-L1 cross-valence sum increases this magnitude, despite partial cancellation; the listed exceptions permit an initial decrease at smaller gains. Pair observations share words and are dependent. The samples are neither matched translations nor representative arbitrary tokens or contexts; no significance, population frequency, language difference or valence decoder is inferred.

Net-vector magnitude can rise while the active count falls. In an [exact two-coordinate reference](../research/cancellation-properties-analysis.json), prior totals `(1/4,1/4)` occupy all sixty positive cells in each coordinate. Input totals `(1/2,-1/2)` give net `(3/4,-1/4)`, doubling L1 magnitude. Distributing that input over one cell per coordinate leaves 61 participating cells instead of 120; distributing it over sixty leaves 120. Both outcomes obey H-77 and have the same signed totals. A range of positive counting thresholds preserves these counts, but no activity threshold or recruitment quota is selected. The reference supplies no semantic labels or recurrent behavior.

The lexical geometry is therefore neither an active-neuron-count result nor a test of a learned DMN state. That state need not equal a negative-labeled token row, and the actual neuron response need not be simple addition. H-54's complementary relation needs evidence from the actual state/input pair, its regulated active fraction, residual meaning and later persistence. Lexical labels alone cannot supply it. The observation selects no vector negation, centering, rotation, valence mapping or replacement of the fixed semantic roles.

## Connectivity and learning

The incoming-set proposal remains compatible: each target cell owns an incoming set; each slot holds a source ID and an FP8 weight. Source IDs cover both polarities. H-54 excludes explicit inhibitory synaptic connections from the early model; this is a separate modeling choice from the sign assigned by the decoder. Intrinsic opposition within a dimension supplies the selected cancellation function. Cross-polarity and within-polarity associations can be learned without mirroring weights or adding dense local blocks. Exact weight encoding and update rules remain open.

Excitation of an opposite-polarity population can, through the required cancellation, reduce the signed contribution of the other population. This is a conditional network effect, not a new inhibitory-edge class or an assumption that negative-coordinate cells are biologically inhibitory. The useful dynamics and stability of the complete learned network still need assessment.

Capacity classes need not change. A single fixed pool can serve both populations; equal neuron counts do not require reserving exactly half of every capacity class for each polarity. Any partition, sharing rule and degree-demand statistics remain choices. The scale-law template is a provisioning prior, not a consequence of adding a second polarity. Synaptic weight scaling stays with training under H-24.

## Static memory and work

The worked 15-class candidate uses **N=614400**, **A=311296** sets in the 100-slot class, **M=622573** total sets and **S=106492416** reserved connection slots. It has 8,173 surplus sets and 19 sets at the maximum capacity of 12,800. The [array formula](CapacityPlanning.md#exact-static-arrays) is `5*S+8*N+4*M+ceil(N/2)`.

| Named array | Bytes |
| --- | ---: |
| Four-byte endpoint IDs | 425,969,664 |
| One-byte FP8 weights | 106,492,416 |
| Eight-byte current-set references | 4,915,200 |
| Four-byte set-owner IDs | 2,490,292 |
| Packed FP4 states | 307,200 |
| **Total** | **540,174,772 = 515.150806427001953125 MiB** |

The arrays meet their element-alignment requirements without inter-array padding. Both polarities share this capacity pool; inactivity does not erase a neuron's learned connectivity or make its set available to another cell. Under incoming ownership the largest-set count does not cap outgoing hubs. Reserved slots are not necessarily live learned parameters.

Scanning every reserved slot implies **532,462,080 bytes of logical endpoint/weight reads per update**, before source-state reads, integration and learning. This is access-volume arithmetic, not measured bandwidth or an update-rate target. Actual work depends on the algorithm chosen for the business requirement. The 16 GB node budget also covers shared embeddings and runtime; no auxiliary structures are preset.

## Messages and persistent state

Decoded vectors contain 5,120 signed coordinates: **10,240 payload bytes** at two bytes per coordinate before framing. For a completed H-77 active state, the signed net `D` determines the two aggregate active magnitudes: `max(D,0)` and `max(-D,0)`, before message quantization and with the agreed coordinate scaling. Sending both totals separately therefore adds no independent aggregate active information. Uncancelled provisional drive or predictive readiness would carry different semantics; their transmission is not selected. Aggregate values still omit individual contextual cell identity, so the eventual message contract must state which information is intended.

The changed cell identity/population layout would need an explicit checkpoint/schema version if implemented. Historical images cannot be reinterpreted as the new layout solely because both use uint32 IDs. One resident CC region, static connection topology and regional relocation/lifetime rules remain applicable. No runtime or serialization change is implemented by this analysis.

## Human cortical-column scale reference

Garcia-Marin et al. (2024) sampled 13 cortical tissue columns from seven humans. They measured neuronal density, then estimated populations using defined module dimensions. [Corrected Table 7](https://academic.oup.com/cercor/article/34/2/bhad512/7511994) reports:

| Human V1 quantity | Result |
| --- | ---: |
| Measured mean neuronal density | 79,221 neurons/mm³ |
| Defined ocular-dominance module dimensions | 0.863 × 0.863 × 2.70 mm |
| Estimated neurons per module | 159,303 |
| Estimated neurons per minicolumn | 130 |
| Estimated minicolumns per module | 1,221 |

These are V1-specific estimates, not a census of every neuron in an entire functional column or a universal human CC size. Use the corrected article table; the [correction notice](https://doi.org/10.1093/cercor/bhae286) also reproduces the old table.

The **614,400-cell case is about 3.86 times** this module's population. This is an arithmetic comparison, not a functional equivalence. A complete embedding vector per software CC is an engineering choice. The evidence does not demonstrate a biological one-dimension/one-minicolumn mapping, paired polarity populations, or equivalent computation at equal neuron counts. H-26 requested the comparison; H-27 separately requests sizing the smaller model and populations.

## Decisions and evidence before implementation

1. Assess the current 5,120-dimensional 60+60 case, including how magnitude and context share their representational role. Final population and pool size are not selected by requesting the calculation.
2. Define the signed encoder/decoder, range, saturation and current/predictive phase readout. Preserve coordinate magnitude across supported contextual subsets and burst behavior.
3. Apply H-77's net/proportional active-rate cancellation; specify its timing, predictive-phase behavior and interaction with transfer, zero/sign transitions and retained context. Evaluate the complementary positive-input residual scenario without equating coordinate sign and semantic valence.
4. If adopted, size the static pool explicitly. Establish per-class demand, compute and bandwidth requirements; discuss any additional representation only when the business requirement and algorithm justify it.
5. Eventually assess signed reconstruction, zero/sign-crossing behavior, context discrimination, prediction accuracy, bounded recurrence and repeated inter-CC encode/decode. Document information intentionally lost on the wire.

This revision contains design analysis, public-source review and exact arithmetic only. It runs no SNN experiment, repository test or build.
