# Steady recurrent SNN: separate positive and negative populations

Document-set revision: 28 · Recorded: 2026-10-27 · Topics: SNN-01–SNN-05, SNN-07 · Proposal under discussion.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 28, all sections here; `pending-human`. K-29 records formatting-only maintenance of revision 24.
- **Value lineage / dependencies:** candidate P-27 derives C-14 signed population encoding from application C-12, preserving interpretable signed semantic coordinates through nonnegative cell activity. P-22 connects C-12 to C-01 and V-01/V-02; P-25 covers this application document set. P-28 is a dependency on C-13 bounded connection storage, not an authority grant.
- **Invariants / non-goals:** retain the provisional capacity-class rule, static pools, parallel five-byte slots, regional relocation/lifetimes and training ownership of weight-scale policy. This analysis selects no biological equivalence, neural transition, four-bit format or implementation. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** signed reconstruction and contextual discrimination must coexist; measure zero/sign transitions, prediction versus current activation, recurrence, resource costs and message information loss under the eventual selected contract. Criteria are detailed below; numerical tolerances remain open.
- **Human baseline / continuity:** H-08–H-24 supply the application and storage constraints. [H-25](../../../design/DesignAuthorityRegistry.md#h-25) keeps capacity allocation provisional and requests analysis of 100 positive plus 100 additional negative cells per semantic dimension. H-26 asks for measured human cortical-column neuron counts. These requests do not adopt the decoder, doubled pool, biological correspondence or learning policy below. K-16 records this draft and its dependencies; there is no qualified implementation baseline. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.
- **Current discussion:** H-43 / AD-037 fixes CC as the basic semantic presentation unit, with approximate replicas across CCs and statically assigned minicolumn/neuron semantic roles. H-41 dimension neutrality and H-42 rejection of internal-position bias remain. [K-33](../../../design/DesignAuthorityRegistry.md#k-33) records the reassessment; learned semantic reassignment is outside the current baseline.

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

For this application, a raw mean over 60 cells gives 1/60 (about 0.01667) when one cell is fully active and 1 when all 60 are fully active. If both patterns encode the same observed coordinate under different contextual certainty, raw averaging would turn surprise into a sixtyfold amplitude change. Splitting polarity does not resolve that ambiguity.

| Encoding direction | What the cells do | Design consequence |
| --- | --- | --- |
| Count/probability code | Activation fraction represents magnitude. | Context selection must preserve a calibrated magnitude estimate; independently selecting fewer cells can bias it. |
| Context selection with graded magnitude | Cell identity/subset represents context; selected cells carry magnitude through graded activity. | Define a decoder calibrated for the selected subset and bursting rule, or explicit column-level amplitude. Extra amplitude state is not automatically allocated. |

For the first direction, an illustrative independent binary sampler with 60 cells and activation probability p has normalized standard deviation `sqrt(p*(1-p)/60)`, about **0.06455 at p=0.5**. Count resolution is 1/60 (about 0.01667), which is not the same as that sampling error. This is arithmetic for an ideal sampler, not a measured FP4/SNN accuracy claim. Correlated firing, context selection, graded states and temporal filtering change the result.

The recommended next discussion is whether the cells primarily represent magnitude samples or temporal contexts with graded magnitude. If HTM-style context discrimination is the priority, assess the second direction first. Its decoder and state cost remain open; population size alone does not determine precision or sparsity.

## Coactivation, prediction and transitions through zero

For one observed scalar, rectified external drive activates only the matching polarity; at exact zero, both external drives are zero. Recurrent activity and prediction can behave differently. Current activation `(A_plus,A_minus)=(0,0)` and `(0.8,0.8)` both decode to zero, while carrying different internal activity and possibly different evidence. A signed scalar preserves the difference, but discards the sum and individual contextual patterns.

Possible contracts include competing current-activation populations, or allowing both and interpreting their common activity separately. A competition rule can use group summaries and cell selection; it does not require a learned dense local matrix. No competition, threshold, hysteresis or confidence interpretation is selected here.

Current activation and predictive preactivation also need separate readout semantics. Both polarities might predict alternative next states even when only one is observed now. A rule forbidding all activity in the opposite polarity could destroy those alternatives. Specify which phase contributes to the emitted instantaneous vector and what happens when the coordinate changes sign. Zero input does not, by itself, require erasing recurrent state or learned connections.

Fixed polarity removes the need for a cell's semantic sign to change. It does not automatically provide another usable bit in standard FP4, whose encoding is fixed. The 16-code phase/intensity allocation, simultaneous current/predictive state and any extra buffers remain the [SNN-02](Model.md#snn-02) design question.

## Connectivity and learning

The incoming-set proposal remains compatible: each target cell owns an incoming set; each slot holds a source ID and an FP8 weight. Source IDs cover both polarities, and the decoder's sign does not dictate the slot weight's sign. Cross-polarity and within-polarity associations can be learned without mirroring weights or adding dense local blocks. Learning needs to address sign transitions and any desired transfer of context across the two populations.

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

Decoded vectors contain 5,120 signed coordinates: **10,240 payload bytes** at two bytes per coordinate before framing. Internal polarity populations do not force wire size to double. Sending both polarity magnitudes separately would double those payloads and preserve their common component, but would still lose cell-context identity. Message meaning must decide whether that information is needed.

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
3. Define whether both polarities can be active together, what cancellation means and how zero/sign transitions affect recurrent state and learning.
4. If adopted, size the static pool explicitly. Establish per-class demand, compute and bandwidth requirements; discuss any additional representation only when the business requirement and algorithm justify it.
5. Eventually assess signed reconstruction, zero/sign-crossing behavior, context discrimination, prediction accuracy, bounded recurrence and repeated inter-CC encode/decode. Document information intentionally lost on the wire.

This revision contains design analysis, public-source review and exact arithmetic only. It runs no SNN experiment, repository test or build.
