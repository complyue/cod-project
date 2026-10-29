# Steady recurrent SNN: application design

Document-set revision: 202 · Recorded: 2026-10-28 · Phase: discussion, implementation deferred.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** P-25 derives this document set from application C-12; P-22 connects the application to C-01 and V-01/V-02 through explicit semantic behavior. C-13/P-26 serves bounded storage, C-14/P-27 contextual semantic encoding, C-15/P-29 learning, C-17/P-34 activity support, and C-26—C-29 useful context, numerical evolution and consistent frame execution. Dependencies do not grant authority.
- **Invariants / non-goals:** one resident CC per region; fixed semantic roles and individual contextual identities; regional relocation/lifetime rules; static SoA connections and no heap topology. No physical-distance prior, explicit inhibitory synapses, complete neural transition, language syntax, implementation or measured performance is selected.
- **Acceptance:** keep human constraints, conditional arithmetic, evidence and proposals distinguishable. Assess the required semantic behavior, useful context, fading, prediction, activity envelope, steerability and actual resource costs under a subsequently selected contract.
- **Human baseline:** D-08 and H-08—H-86 retain the exact scoped statements. H-11 separates application documents; H-34/H-74 require a demonstrated value for additional structures; H-75—H-77 give contextual-cell, frame, input and active-cancellation directions. H-79—H-84 distinguish flow learning, prediction assessment and generation. H-78/H-85 defer readiness choices; H-86 delegates obvious empirical alternatives to real-data comparisons without repeated questions.
- **Current assessment:** [K-190](../../../design/DesignAuthorityRegistry.md#k-190) makes the existing empirical comparisons incremental: identify a behavioral or resource question, compare relevant alternatives, and distinguish direct numerical conversion from acquisition under candidate arithmetic. No alternative is selected; D-08 stays pending-human. [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; [progress](../research/modeling-refinement-progress.json) records its scope and substantive work.

This document set owns the application's model, resource planning and decisions. CoD's [language tracker](../../../design/DomainLanguageDesign.md) owns reusable language semantics. An application proposal becomes a language requirement only through an explicit design decision.

## Model and operating goals

One cortical column (CC) represents an instantaneous semantic vector. CCs are approximate replicas and exchange vector sequences; the overall planning scale is hundreds to tens of thousands of CCs. The current input-table investigation uses `deepseek-ai/DeepSeek-V4.1-Flash-1017`: 5,120 dimensions, each assigned sixty positive and sixty negative cells, giving **614,400 neurons per CC**. Final checkpoint deployment, input normalization and numerical encoding remain open.

Cells within a minicolumn share a semantic component but represent different contextual paths. They retain distinct identities and connections. Coordinate sign, semantic valence, predictive readiness and current firing are different concepts. In particular, negative semantic content is not the collection of negative-valued coordinates.

[Activation regimes](ActivationRegimes.md) supplies the behavioral targets:

| Regime | Intended behavior |
| --- | --- |
| DMN | Background white noise supports broad recurring participation and slow drift toward negative semantic content. |
| Flow | Token sequences evoke context-sensitive responses; pre-input prediction increasingly matches the sequence as distinguishing context accumulates. |
| Immersion | Multidimensional activity circulates autonomously within a limited participating population, with useful loops of varying length. |

The active-neuron fraction **within each CC** stays in a broad, flexible envelope. Regimes may have different typical levels and fluctuations. Whole-network occupancy inherits comparable local bounds as a weighted average; this fact does not establish semantic stability or useful memory.

Regime classification instead uses qualified temporal coverage and path behavior. Broad DMN coverage, limited immersion, and progression through different groups within a small flow footprint are the current directions. The illustrative 80% DMN and 20% flow figures do not fix thresholds or windows. Total active fraction is a regulation variable and is excluded from classification. Incidental noise participation must be discounted without automatically erasing weak useful context.

Normal recurrence remains steerable by external tokens. Repetition alone is not stale; loss of effective external redirection is the concern. Reset is a possible recovery operation with no selected trigger, scope or policy.

## Dynamics and training boundaries

The simple execution proposal is [two alternating state frames](RateDynamics.md#two-alternating-frames-with-incoming-gather): incoming gathers read a complete preceding snapshot, write the next frame, and publish it when finished. Each cell retains its own rate/activity value. The noise level stays consistent within the transition. This selects neither a frame duration nor a whole-network lockstep schedule, and does not require a second weight pool.

H-77 specifies minicolumn-wide active-rate cancellation: calculate the positive/negative total difference, remove the losing polarity's active rates, and proportionally retain the surviving cells' rates. There is no positive/negative cell-index pairing. Cancellation preserves the provisional surviving composition; added drive can change that composition even when the aggregate signed value remains fixed.

H-76 directs a token toward sufficiently predicted cells with strong activation; otherwise it gives broad, weak activation. **H-84 places that broad weak fallback in generation mode and excludes it from learning.** Flow teaching responsibility must be established independently. Readiness as a range of one activity value versus separate readiness/current firing, and coexistence versus cancellation of opposing predictions, remain experimental alternatives.

The [training strategy](TrainingStrategy.md) keeps the acquisition routes distinct:

| Scope | Direction and assessment |
| --- | --- |
| DMN and isolated tokens | Initial construction and ongoing repair address insufficient expected next-step participation. DMN's epoch scope is the whole CC; each token epoch selects its semantic-proportional scope once. Actual response is assessed without corrective training assistance. |
| Token-sequence flow | A provisional Hebbian/HTM procedure assesses preceding prediction, independently assigns the observed token's contextual responsibility, adapts eligible associations and considers absent-source discovery. Its rule is not presumed to be DMN/token repair. |
| Immersion | Expected emergence, with no dedicated training objective introduced. |
| Continuous generation | Separate algorithm design. Autonomous long-sequence production is not required to pass the direct flow-learning criterion. |

For DMN/token maintenance, width, body enhancement and distributed tail/head attenuation shape the intended recurrence. Tail-to-head connections remain; token proportions should be maintained each step or within a few steps. The prominent isolated-token response has a loose half-life-like scale around half a second, with variation and timely fading. This is not one lap, a hard deadline or an instruction to erase all earlier context.

Useful smaller loops may retain context after the dominant response fades. Avoid deliberately harmful shortcuts during main-chain construction without aggressively removing all small cycles. Shared edges and cells make joint behavioral assessment necessary: individually adequate loops or successful next-step counts do not prove correct combined dynamics.

Attention substantially reduces background noise. Low activity can trigger increased support before DMN is detected. After complementary positive-semantic input cancels a negative-semantic state, the intended residual grows to a peak under support and then fades. The regulator's timing and activity definition remain open; neither added white noise nor a preserved activity fraction alone proves selective residual growth or contextual retention.

## Resource planning and numerical comparison

The per-node RAM ceiling is **16 GB**. A CC should fit a Raspberry Pi-sized resource budget; the CC count per node and update rate remain unspecified. The embedding table occupies 1,323,827,200 bytes shared once per node, and is excluded from per-CC totals.

Connections use static capacity classes with formula-friendly indexing. Each participating neuron references its current set; each set has an owner ID. Each slot contains a four-byte endpoint ID and a one-byte weight in separate arrays. Target-owned incoming sets are the preferred gather proposal under evaluation. Training may migrate a neuron to another preallocated class; no heap allocation maintains the topology. Weight scale belongs to training and representation semantics, not the capacity-distribution law.

The provisional alpha-near-3 template has a 100-slot minimum, doubles capacity every other class, and halves set population at each adjacent class. Total sets should slightly exceed neuron count. Neither class count is mandatory, and no older total connection capacity must be preserved.

| Worked pool | Sets | Reserved slots | Largest set | Named arrays, one FP4 state array | Named arrays, one FP32 value per frame, two frames |
| --- | ---: | ---: | ---: | ---: | ---: |
| 15 classes | 622,573 | 106,492,416 | 12,800 | 540,174,772 bytes | 544,782,772 bytes |
| 16 classes | 655,350 | 112,281,600 | 18,432 | 569,251,800 bytes | 573,859,800 bytes |

These are conditional named-array totals, with one-byte weights, existing references/owners and no preset auxiliary allowance. The FP32 column budgets one scalar per neuron in each of two temporal frames: 4,915,200 state bytes (4.6875 MiB). It does not budget separate readiness and firing values in both frames. Final activity/readiness fields and weight precision remain open. Reserved capacity, retained edges and per-class demand are distinct: enough total slots do not guarantee an available set of every needed size.

[Context loops and numerics](ContextLoopsAndNumerics.md) uses adequate-precision reference arithmetic to assess FP16/FP32 state and narrower weight/state alternatives. Product precision, reductions, cancellation, writeback and repeated learning all matter. A low bit count is useful only if the required behavior survives its actual arithmetic and work cost.

Scheduled decay means determining the next meaningful representable change, including intervening input effects. It is a comparison candidate, not a default timer array. First assess decay in the ordinary frame computation; connection traversal can dominate the saved scalar work. Delaying cancellation or losing subprecision inputs is a model change, not simply a cheaper schedule.

## Requirements before auxiliary structures

Every added concept, algorithm and data structure needs a real application value and a demonstrated reason the simpler approach is insufficient. Do not preselect supporting representations or reserve memory for hypothetical ones. SoA is the default; AoS needs an access-pattern-specific empirical reason. Standard widths are powers of two in bits, including FP4; other widths need their own alignment discussion.

Algorithm requirements can justify comparisons without selecting their storage. For example, traversing retained incoming edges requires an end convention: a marker in an unused endpoint and a separate count are both candidates. Likewise, normalized cell composition is an analytical description of existing rates, not an extra context array. Migration, source discovery and any later auxiliary storage need explicit semantics and cost when selected.

## Choices reserved for real-data comparison

H-86 directs these empirical alternatives to be recorded and compared without repeatedly asking the user to choose parameters. Keep the alternatives, deciding observations and resource consequences explicit. A deferral does not approve a mechanism or authorize implementation. Questions about changed goals, scope or material value trade-offs remain distinct.

| Comparison | Evidence that matters |
| --- | --- |
| Readiness representation and opposing predictive signs | Pre-input matching, narrowing on distinguishing context, cancellation behavior and required state cost. |
| Flow contextual assignment, source credit and bounded reinforcement | Learning unfamiliar transitions, retaining legitimate alternatives, shared-fragment discrimination and interference with DMN/token recurrence. |
| Rate/state/weight precision and update arithmetic | Semantic fidelity, weak contextual distinctions, effective learning, fading and measured execution/storage cost. |
| Activity qualification, noise allocation and feedback timing | Flexible-envelope recovery, selective residual growth, useful context and steerability; avoid counting numerical dust or enforcing success by definition. |
| Ordinary frame decay and next-change scheduling | Preserved intervening-input effects and recurrent semantics, plus actual work saved after scheduling costs. |
| Retained-edge representation and static class assignment | Real degree demand, traversal/growth/migration cost and preservation of effective weights. |

Future comparisons should start with a concrete behavioral or resource question and the simplest adequate-precision reference that expresses it. Compare the relevant alternative, then examine coupled choices where their interaction changes the conclusion—for example, predictive cancellation with the chosen readiness meaning, or a frame interval with its noise units. Reassess the shared graph against the same DMN/token and flow goals. For numerical formats, distinguish direct conversion of learned behavior from acquisition or recovery using candidate arithmetic, including the cost of that training. This is a proposed comparison order, not a selected neuron equation or implementation plan.

Prediction assessment must occur before the observed token changes state or weights. Shared sequence fragments can permit several predictions; longer distinguishing context should narrow them. Readiness is not automatically a probability, and high aggregate overlap does not alone demonstrate contextual prediction. H-84's generation fallback supplies no learning credit.

## Document map

| Document | Scope |
| --- | --- |
| [Model](Model.md) | CC/minicolumn roles, signed coding, phases, embedding/messages, regions and persistence. |
| [Activation regimes](ActivationRegimes.md) | Per-CC activity, DMN/flow/immersion, detection, noise feedback, residual behavior and steerability. |
| [Training strategy](TrainingStrategy.md) | Training division and DMN/token activation-chain construction and repair. |
| [Flow learning](FlowLearning.md) | Provisional Hebbian/HTM procedure, contextual prediction, temporal credit and shared-graph preservation. |
| [Rate dynamics](RateDynamics.md) | Fractional participation, two-frame semantics, timing, natural cessation and precision. |
| [Context loops and numerics](ContextLoopsAndNumerics.md) | Useful recurrent context, cell composition, accumulated/scheduled decay and numeric comparisons. |
| [Signed population encoding](SignedPopulationEncoding.md) | Polarity, magnitude, contextual identity, input selection, readiness and cancellation. |
| [Capacity planning](CapacityPlanning.md) | Dense/sparse budgets, node limits, CPU/network costs and named-array formulas. |
| [Incoming connectivity](IncomingConnectivity.md) | Target ownership, gather updates, class feasibility and retained-edge traversal. |
| [Adaptive connectivity](AdaptiveConnectivity.md) | Static classes, training-driven migration, ownership and lifetimes. |
| [Connectivity scaling research](ConnectivityScalingResearch.md) | Biological degree evidence, capacity geometry and detailed 15/16-class arithmetic. |
| [Synaptic integration research](SynapticIntegrationResearch.md) | Input count, excitability and context-sensitive integration; capacity is not a firing rule. |
| [Token embedding measurements](TokenEmbeddingStatistics.md) | Actual Flash tensor, density/concentration, dimension statistics and limits. |
| [Physical distance preference](PhysicalDistancePreferenceResearch.md) | Biological locality and computational effects; no physical-distance prior is currently adopted. |
| [Semantic cortical mapping](SemanticCorticalMappingResearch.md) | Anatomical compatibility of assigned semantic roles and limits of biological correspondence. |
| [Spatial connectivity research](SpatialConnectivityResearch.md) | Conditional 1D/2D analysis and the rejected internal-position distance proposal. |
| [Decisions](Decisions.md) | Scoped human directions and open proposals. |

The H-71 research pass is complete; its substantive intervals and requirement coverage are in the [progress record](../research/modeling-refinement-progress.json). Research evidence and read scopes are in the [source ledger](../research/modeling-source-ledger.json); mathematical and numerical records state their own assumptions. They support design discussion and do not establish successful SNN training, biological equivalence or implementation qualification.
