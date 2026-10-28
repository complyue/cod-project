# Steady recurrent SNN: Flash-1017 token embedding measurements

Document-set revision: 30 · Recorded: 2026-10-27 · Topics: SNN-01/SNN-03/SNN-05.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 30, all sections; `pending-human`. Measurements inform the design discussion and do not qualify an implementation contract.
- **Value lineage / dependencies:** candidate P-25/P-22 connect C-12 to C-01 and V-01/V-02: inspect the actual input representation before choosing an encoding or topology. C-14/P-27 concerns signed population representation; C-16/P-32 concerns mapping assumptions. Dependencies do not authorize an embedding-derived distance prior.
- **Human baseline / continuity:** H-45 / AD-039 requests actual ordinary-token statistics for the named checkpoint and checks SSD capacity. H-43 fixes coordinate roles; H-46 permits distributed semantic features; H-47/H-48 specify broad per-CC activity bounds. [K-35](../../../design/DesignAuthorityRegistry.md#k-35) records the measurement and interpretation scope.
- **Invariants / non-goals:** preserve Flash 5120 × (60+60), shared-node table accounting, static SoA capacity and regional relocation. No contextual transformer execution, output-head analysis, SNN training, activation threshold, spatial layout, decoder or implementation is selected.
- **Acceptance:** identify the immutable source, tensor, dtype, dimensions, token populations, measurement formulas and sampling limits; retain compact evidence and distinguish numerical density from semantic meaning and neuronal activation.

## Measured finding

**The input table is distributed across dimensions, not dominated by one coordinate per token.** All 127,997 ordinary vocabulary entries have 5,120 nonzero coordinates. The median largest-coordinate share of squared magnitude is **0.283%**, and the maximum is **5.721%**. The median token needs **2,278 coordinates** to account for 90% of its squared magnitude; even the minimum over ordinary entries is 2,043.

These measurements support multidimensional token input under H-46. They do not imply equal contributions, statistically independent coordinates, equal positive/negative usage in every dimension, or activation of every SNN cell.

## Source and storage

The requested model is [deepseek-ai/DeepSeek-V4.1-Flash-1017](https://huggingface.co/deepseek-ai/DeepSeek-V4.1-Flash-1017), pinned to revision `1bae0e1088993b9b82de08f59de5be5a34e42170` for this investigation. The [pinned configuration](https://huggingface.co/deepseek-ai/DeepSeek-V4.1-Flash-1017/raw/1bae0e1088993b9b82de08f59de5be5a34e42170/config.json) identifies vocabulary 129,280, hidden width 5,120 and untied input/output embeddings. The [weight index](https://huggingface.co/deepseek-ai/DeepSeek-V4.1-Flash-1017/raw/1bae0e1088993b9b82de08f59de5be5a34e42170/model.safetensors.index.json) places `embed.weight` in `model-00002-of-00048.safetensors`.

The safetensors header identifies the actual input tensor as **BF16 `[129280, 5120]`**. The header length is 344 bytes, so tensor payload begins at byte 352; its tensor-relative data offsets are `[0, 1323827200]`. The extracted inclusive file range is `352–1323827551`.

```text
129280 * 5120 * 2 = 1,323,827,200 bytes
                   = 1,262.5 MiB
                   = 1.23291015625 GiB
```

This table fits the local SSD, which had approximately 192 GiB available at the initial check. The data resides under `/ws/EvtWorks/model-data/DeepSeek-V4.1-Flash-1017/1bae0e1088993b9b82de08f59de5be5a34e42170/`, outside the Git checkout and NFS home. Only the input tensor and small metadata files are needed; the complete model, output head and Engram tables are outside this analysis. H-28 counts this table once per node sharing it, not once per CC. The measurement checkpoint is not a selection of the final SNN encoding or deployment.

## Populations and methods

The tokenizer's base BPE vocabulary has 128,000 entries. Excluding its three base special tokens gives **127,997 ordinary vocabulary types**, IDs 3–127999. Another 1,280 IDs are added entries. Some controls, including reasoning and role markers, are marked `special: false`, so filtering that flag alone would mix control codes into the ordinary population. Results keep ordinary base vocabulary, added controls not marked special, tokenizer specials and all rows separate.

Every ordinary type has equal weight. These are vocabulary-table statistics, not frequencies in a language corpus, contextual hidden states or firing statistics from a trained SNN. A vocabulary item may be a word fragment, byte fragment, whitespace or punctuation rather than a complete word.

For a row vector `x`, define `E = sum_d x[d]^2`. Report:

- Nonzero, positive and negative coordinate counts.
- The largest 1, 10 and 100 coordinates' shares of `E`, sorted by squared magnitude.
- The minimum number of coordinates covering 50%, 90% and 95% of `E`.
- Effective energy dimension `E^2 / sum_d x[d]^4`, a concentration statistic rather than semantic rank.
- Counts exceeding diagnostic fractions of the row RMS; these are not selected SNN activation thresholds.

BF16 values are decoded exactly to float32, with row energy reductions in float64. Scalar BF16 decoding and independent summation check selected ordinary/control rows. Zero-energy rows have undefined energy fractions and are counted separately.

Per-coordinate means, RMS, signs and total energy use all ordinary rows. Pearson correlation discovery uses all coordinate pairs on a deterministic sample of 4,096 ordinary rows, seed 20261027. The 20 largest sampled pairs and 2,048 random distinct coordinate pairs are then evaluated on the full ordinary vocabulary. The selected-pair checks do not constitute a full-vocabulary scan of all pair correlations or prove which pair is globally largest.

## Full ordinary-vocabulary results

All 127,997 rows are finite and have nonzero norm. No ordinary token places even 10% of its squared magnitude in one coordinate. Quantiles below are over equally weighted token types.

| Metric | Median | 5th percentile | 95th percentile | Minimum | Maximum |
| --- | ---: | ---: | ---: | ---: | ---: |
| Nonzero coordinates | 5,120 | 5,120 | 5,120 | 5,120 | 5,120 |
| Positive coordinates | 2,568 | 2,512 | 2,624 | 2,428 | 2,727 |
| Negative coordinates | 2,552 | 2,496 | 2,608 | 2,393 | 2,692 |
| Largest coordinate: energy share | 0.283% | 0.230% | 0.386% | 0.182% | 5.721% |
| Largest 10 coordinates: energy share | 2.200% | 1.973% | 2.492% | 1.711% | 13.772% |
| Largest 100 coordinates: energy share | 14.088% | 13.435% | 14.809% | 12.495% | 27.203% |
| Coordinates for 50% energy | 637 | 615 | 658 | 424 | 690 |
| Coordinates for 90% energy | 2,278 | 2,240 | 2,317 | 2,043 | 2,378 |
| Coordinates for 95% energy | 2,834 | 2,794 | 2,873 | 2,626 | 2,952 |
| Effective energy dimension | 1,712.1 | 1,644.1 | 1,775.0 | 213.1 | 1,862.5 |
| L2 norm | 8.443 | 5.588 | 10.605 | 0.422 | 36.368 |

A few frequent-looking token types are more concentrated than the type-level median, while still spreading most energy over many dimensions. The largest single-coordinate share occurs for the space token, ID 223: 5.721%, with 2,118 coordinates needed for 90% energy. No corpus was used, so these are examples, not frequency-weighted estimates.

| Ordinary token text | ID | Largest coordinate's energy | Coordinates for 90% energy |
| --- | ---: | ---: | ---: |
| `" the"` | 270 | 5.477% | 2,138 |
| `"的"` | 301 | 2.278% | 2,214 |
| `"人"` | 470 | 0.357% | 2,317 |
| `"中国"` | 2069 | 0.267% | 2,260 |
| `"return"` | 3916 | 0.299% | 2,256 |
| `"world"` | 29616 | 0.242% | 2,325 |
| `"hello"` | 33310 | 0.511% | 2,263 |

The separate populations contain 54 added controls not marked special and 1,229 tokenizer-special entries, the latter including the three base special tokens. None has zero norm or a top-coordinate share reaching 10%. Ordinary rows and non-special controls have exactly 5,120 nonzero coordinates; tokenizer-special rows have 5,119–5,120. Control-code rows are not evidence about ordinary semantic usage.

## Dimensions are not statistically identical

Across all ordinary token types:

- The largest aggregate-energy dimension contributes **0.06655%** of total table energy; the top 10 together contribute **0.40233%**. This is well short of a single dominant axis, but not exact isotropy.
- The most frequent largest-magnitude coordinate is dimension **1233**, winning for **1.5633%** of ordinary types. Dimension indices are zero-based labels, not distances.
- Positive coefficients average **50.1515%** of entries, yet individual coordinates have strong sign preferences: dimension **3757** is positive for **99.5508%** of ordinary types, while dimension **5071** is positive for only **0.8321%**.
- The squared norm of the vocabulary mean vector is **4.6327%** of the mean row squared norm. This reports a common offset, not a selected centering operation or a semantic-valence direction.
- Of 2,048 randomly selected distinct coordinate pairs evaluated on every ordinary row, median absolute Pearson correlation is **0.02374**, the 95th percentile **0.07011**, and the maximum **0.15531**. These are statistics of that pair sample.
- The separate search over all 13,104,640 pairs on 4,096 sampled rows found stronger candidates. Full-vocabulary evaluation gives **r = 0.50147** for coordinates **3757/5071**, and **r = −0.35191** for **1233/3136**. These validate particular correlations, not a claim to have found the globally strongest full-vocabulary pair.

Orthogonality of abstract coordinate axes does not entail independence of their coefficients across token rows. Conversely, measured correlations do not provide an approved semantic-distance metric or prove a biological spatial map. Any use in connectivity would be a separate design choice under H-41/H-42.

The equal 60+60 cell allocation is a representation capacity assumption. It does not require equal positive/negative input probabilities in every dimension. None of the observed sign asymmetries identifies negative semantic meaning: a valence direction requires a separate semantic interpretation.

## Interpretation for the SNN

A token supplies a distributed signed pattern. A measured biological feature can likewise depend on a pattern across many coordinate populations, while one population participates in many features. There is no need to demand one measured feature per minicolumn.

Numerical density does not determine neuronal occupancy. The encoder must still map magnitude, polarity and context into cell phases and strengths while meeting the broad per-CC activity envelope in [ActivationRegimes](ActivationRegimes.md). No threshold, normalization, sparsification, centering or dimension removal is selected by this measurement. Energy concentration is not a semantic reconstruction guarantee, and effective energy dimension is not the intrinsic rank of the represented concepts.

The measurements concern the pretrained input table. They do not establish contextual transformer behavior, the learning performance of the proposed SNN, its three semantic regimes, or an anatomical placement rule.

## Evidence and reproduction

[Compact measurement record](../research/flash-1017-embedding-statistics.json) contains full metric quantiles, population definitions, token examples, dimension statistics, correlation samples, metadata hashes and source integrity evidence. The complete BF16 tensor, metadata and standalone `analyze_embeddings.py` are retained at the SSD directory above; the tensor is outside Git. Analysis used NumPy 2.5.4, with no model runtime or project build.

- Tensor SHA-256: `aa1ac979edab508950a871c370fa645a875203a1a35c20ba9c1d45bb1ed46e3c`.
- Analysis script SHA-256: `3bc90fc66064d8507905efb91119426ee8328b58fd945fc1c937db9223462948`.
- The original shard's SHA-256 was reconstructed by hashing its 352-byte header, the local tensor and its 30,720-byte tail in file order. It matches the pinned Hugging Face LFS object: `43616eebd88dd4f1f1b642772356fc160478d1d9d04edd240431e7d065d2eda7`. This checks the extracted bytes against the published source without retaining another large shard copy.
- Extraction completed at 2026-10-27T15:47:44Z; analysis completed at 2026-10-27T15:48:17Z. Independent scalar decoding checked six rows, including ordinary tokens, controls and the concentration outlier.
