# Steady recurrent SNN: decisions and open questions

Document-set revision: 202 · Recorded: 2026-10-28.

## Design authority

- **Registry / scope:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), document revision 202, all sections; `pending-human`. There is no qualified application implementation baseline.
- **Value lineage:** P-25 derives D-08 from C-12; recording propositions and their value/dependency relationships preserves application intent while choices evolve. The history distinguishes human direction, arithmetic and agent proposals. P-25/P-22 connect the application through C-12 to C-01 and V-01/V-02; the registry controls each relationship's scope.
- **Invariants / non-goals:** distinguish human constraints from assistant proposals and calculations. Source quotes are evidence for their stated scope, not approval of every generated document or downstream mechanism. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** each selected mechanism needs its proposition, trade-off, scope, evidence and remaining dependencies recorded before implementation. The registry remains authoritative for qualification.
- **Human baseline:** AD-001—AD-092 preserve the propositions sourced to H-08—H-86. A direction or comparison request does not approve every detailed mechanism; H-78 expressly leaves the state-model choice open. The linked registry retains exact evidence and exclusions.
- **Current assessment:** [K-96](../../../design/DesignAuthorityRegistry.md#k-96) records H-85/H-86: predictive cancellation stays open for controlled comparison, and obvious empirical alternatives should be recorded directly for later real-data comparison without repeated questions. No candidate becomes a default by deferral. H-78 remains open; D-08 is unqualified; [K-207](../../../design/DesignAuthorityRegistry.md#k-207) records completion of the H-71 research pass; empirical choices remain open.

## Human decisions

The speaker is the user in the [CoD project conversation](codex://threads/01a14005-1edf-70d1-adcf-c091e8652c75); no independently verified name is available. Exact message timestamps are unavailable. Dates here are recording dates, not invented message times.

| Entry | Direction and scope | Evidence |
| --- | --- | --- |
| AD-001 | First application: steady recurrent SNN, one CC per region, semantic-vector exchange, hundreds to tens of thousands of CCs, intended DeepSeek V4 Pro input embeddings. The initial 100-cell/FP4/FP8 dense model starts the discussion; sign encoding and dynamics are open. | [H-08](../../../design/DesignAuthorityRegistry.md#h-08), original statement preserved there; recorded 2026-10-23 |
| AD-002 | Prioritize parameter and memory feasibility; each CC must fit a Raspberry Pi-sized budget. Reopen the 100-cell population size. No specific board, throughput or topology was chosen. | H-09; quotes below |
| AD-003 | Within-minicolumn connections are less necessary; consult HTM. This redirects connectivity analysis but does not select exact sparsity, segment limits or a complete Temporal Memory algorithm. | H-10; quote below |
| AD-004 | Maintain a separate document set for this first business application. CoD's language tracker retains the example link; the application model and capacity decisions live here. | H-11; quote below |
| AD-005 | Set the per-node RAM ceiling to 16 GB. Preserve the separate per-CC Raspberry Pi constraint; CCs per node and update rate remain open. | H-12; quote below |
| AD-006 | Consider unequal sparse connection capacities: highly connected neurons receive more weight slots, lightly connected neurons fewer, with training-driven migration between preallocated connection sets. A degree scaling law is a consideration, not a specified exponent or distribution. | H-13; quote below; SNN-07 |
| AD-007 | Statically provision the connection-weight slot sets according to the scale-law plan. Each neuron has an out-of-line pointer to its current set. Connection management uses no heap allocation. Exact class counts, pointer layout and reassignment protocol remain open. | H-14; quote below; SNN-07 |
| AD-008 | Evaluate a sufficiently large outgoing-degree allowance: around 10,000 connections for a very small minority among 716,800 neurons. The assistant’s 4,096-slot example is not a design ceiling. Exact maximum, class sizes, high-degree population and rate remain open. | H-15; quote below; SNN-04/SNN-07 |
| AD-009 | Count only a target ID and one-byte weight per slot; total set count equals neuron count. Add one array mapping each set back to its source neuron. Recalculate exact static memory. No per-edge reverse index or spare sets are implied. | H-16; quote below |
| AD-010 | Store targets and weights in parallel arrays of four-byte and one-byte elements for CPU alignment. Five bytes is the aggregate per-slot payload, not a packed struct requirement. | H-17; quote below |
| AD-011 | Permit more sets than neurons to assist migration. Prioritize class counts that make scale-based indexes easy to calculate; keep pool topology static rather than maintaining it through heap allocations. This supersedes only AD-009’s exact-N count requirement. | H-18; quote below |
| AD-012 | Set the current minimum set capacity to 100; omit smaller classes and permit ignoring insufficiently connected outgoing sources. Revisit the minimum if underfilled 100-slot sets prove both common and useful. Exact admission/re-entry rules, population limits and larger class counts remain open. | H-19; quote below |
| AD-013 | Investigate whether human-brain research supports a better scaling exponent than the doubling proposal. This requests research and comparison; it does not approve a particular exponent, class ratio or pool size. | H-20; quote below |
| AD-014 | Provisionally retain the preceding alpha-near-3 template for planning: 100/144 times powers of two, halving counts, 786,408 sets and 134,516,736 slots. This is a provisional choice, not measured suitability or a full implementation contract. | H-21; first sentence below; revision 8 template carried into revision 9 |
| AD-015 | Evaluate target-owned incoming sets for simulation convenience and compatibility with the scale law. Ownership and execution details remain under discussion; no automatic reinterpretation of the low-outgoing-degree omission rule. | H-21; second sentence below |
| AD-016 | Investigate whether neurons with many incoming connections activate too easily, using neuroscience and anatomical evidence. This is a request to assess the model, not adoption of a new degree cap, normalization rule, dendritic model or experiment. | H-22; quote below; recorded 2026-10-26 |
| AD-017 | Regard lower typical incoming weights at greater incoming degree as a reasonable compensation for excessive activation. This accepts a modeling direction; it does not establish a biological law or choose inverse-degree normalization, a fixed weight budget or a learning rule. | H-23; first sentence below |
| AD-018 | Assess incoming-degree allocation parameters separately from outgoing-degree parameters. The question does not select alpha=3.15 or replace AD-014’s provisional integer table. | H-23; second sentence below |
| AD-019 | Weight scale belongs to training strategy, as a possible hyperparameter or dynamically planned/adapted quantity; it is not a capacity-allocation design parameter. No particular training algorithm or scale rule is selected. | H-24; first sentence below |
| AD-020 | Retain the current capacity allocation provisionally and move discussion to semantic-dimension encoding. The two-class capacity doubling and per-class count halving remain the planning geometry; resizing for a different neuron population is a separate calculation. | H-25; first sentence below |
| AD-021 | Analyze 100 cells for positive values plus 100 additional cells for negative values per semantic dimension. This proposes 200 cells per dimension, not a 50+50 division of the old population; it does not yet select the decoder, training rule or resized pool. | H-25; second sentence below |
| AD-022 | Check measured human cortical-column neuron counts and compare the unit of measurement with the modeled CC. This requests anatomical evidence, without selecting biological equivalence or a smaller CC. | H-26; quote below |
| AD-023 | Calculate a smaller planning case using DeepSeek V4.1 Flash token vectors and 60 positive + 60 negative cells per semantic dimension. This makes that case the current sizing discussion; it does not select a final checkpoint, resized pool quota or neural decoder. | H-27; quote below |
| AD-024 | Account for a semantic-vector table once per compute node, shared by its CCs, and exclude it from per-CC storage. The user also questions the padding entry; K-18 removes the assistant-imposed eight-byte start requirement for byte arrays. No complete physical ABI is selected. | H-28; quote below |
| AD-025 | Default to SoA for multi-field data. An AoS exception needs a specific cache-friendly access-pattern justification and empirical evidence. This extends the explicit slot layout to the general layout policy without approving an implementation contract. | H-29; quote below |
| AD-026 | Discuss packing and alignment separately for each field with a nonstandard bit width. SoA does not automatically select tight packing, widening or padding; FP4 byte packing remains a sizing assumption pending that discussion. | H-30; quote below |
| AD-027 | Define standard field widths as exactly `8 * 2^n` bits for integer n >= 0. All other widths require AD-026’s separate packing/alignment discussion, including whole-byte widths such as 24 and 48 bits. | H-31; quote below |
| AD-028 | Include FP4 as standard: `4 = 8 * 2^-1`. Negative integer exponents are allowed where the resulting bit count is a positive integer, giving 1, 2, 4, 8, 16, ... standard widths. This supersedes AD-027’s n >= 0 restriction and excludes FP4 from AD-026’s nonstandard-width trigger. | H-32; quote below |
| AD-029 | Establish business requirements before discussing implementation algorithms; consider auxiliary data structures only within that discussion. Remove preset auxiliary structures and their hypothetical memory allowances from current planning. | H-34; quote below |
| AD-030 | Set aside the V4 Pro 7,168-dimensional case for now; keep current planning focused on Flash 5,120 dimensions and 60+60 cells. Clarify A as the set count in the 100-slot class; explaining the existing value does not approve a final quota. | H-35; quote below |
| AD-031 | Size the static pool so total slot sets M are slightly greater than total neurons N, while retaining convenient arithmetic indexing. No exact surplus percentage or final class count is selected. | [H-38](../../../design/DesignAuthorityRegistry.md#h-38); quote below |
| AD-032 | Do not preserve connection capacity from an older sizing scenario as a constraint. Derive reserved slots from the current population, class geometry and small-surplus objective. | [H-39](../../../design/DesignAuthorityRegistry.md#h-39); quote below |
| AD-033 | Use coactivity as the basic connection-learning direction, favoring nearer eligible partners outside a small very-near exclusion. Research 1D versus 2D; no dimension, metric, placement, kernel, exclusion radius or detailed weight update is selected. | [H-40](../../../design/DesignAuthorityRegistry.md#h-40); quote below; recorded 2026-10-27 |
| AD-034 | Keep semantic-dimension/minicolumn labels free of distance preference. Evaluate equal cross-minicolumn connection opportunity with a preference for corresponding internal cell positions; research semantic representations versus biological locations. Shared-role meaning, exact equality semantics, geometry, polarity and learning mechanisms remain open. | [H-41](../../../design/DesignAuthorityRegistry.md#h-41); quote below; recorded 2026-10-27 |
| AD-035 | Reject cross-minicolumn distance preference derived from internal minicolumn positions. The shared-position/aligned-index proposal is not an active design direction. | [H-42](../../../design/DesignAuthorityRegistry.md#h-42); first sentence below; recorded 2026-10-27 |
| AD-036 | Deepen research into semantic-vector mappings and 2D topology within a human CC, seeking design inspiration. This selects a research scope, not an alternative encoding, biological equivalence or implementation. | H-42; second sentence below |
| AD-037 | Treat CC as the basic semantic presentation unit, with approximate/non-exact replicas across CCs. Minicolumn/neuron semantic response roles are assigned statically; learning those roles is not required. Reassess topology research within that premise. Exact replication, response curves, learned dynamics and spatial layout are not selected. | [H-43](../../../design/DesignAuthorityRegistry.md#h-43); quote below; recorded 2026-10-27 |
| AD-038 | Prioritize reconciling neuron positions implied by the fixed semantic-vector assignment with biological anatomical topology. Continue deeper research; 1D versus 2D is secondary. This does not select interleaving, anatomical compactness, an opportunity model or a relaxation of dimension neutrality. | [H-44](../../../design/DesignAuthorityRegistry.md#h-44); quote below; recorded 2026-10-27 |
| AD-039 | Investigate whether actual ordinary-token embeddings use many dimensions, using `deepseek-ai/DeepSeek-V4.1-Flash-1017`; check local SSD capacity for its semantic table. The checkpoint is identified for this investigation, without selecting the final SNN encoder or deployment. | [H-45](../../../design/DesignAuthorityRegistry.md#h-45); quotes below; recorded 2026-10-27 |
| AD-040 | Biological features need not correspond one-to-one with minicolumns. Train for bounded recurrent activity with three semantic regimes: noise-supported DMN with a slight negative-semantic activation advantage and gradual negative drift; sequential token-associated flow; and immersion through autonomous multidimensional semantic loops with varying bounded lengths. Exact single-coordinate activation is not the goal. | [H-46](../../../design/DesignAuthorityRegistry.md#h-46); quote below; recorded 2026-10-27 |
| AD-041 | Apply the steady active-neuron fraction bound within each CC. | [H-47](../../../design/DesignAuthorityRegistry.md#h-47); scoped reply below; recorded 2026-10-27 |
| AD-042 | Keep the per-CC fraction allowance broad; different mental states may have different activity levels. Do not require equal fractions for every phase/regime. Numerical bounds and counting rules remain open. | [H-48](../../../design/DesignAuthorityRegistry.md#h-48); quote below; recorded 2026-10-27 |
| AD-043 | Continue deeper research to find scientific support for how the model should handle neuronal physical-distance preference. This requests an evidence-based assessment, without adopting a distance kernel, physical layout or learning mechanism. | [H-49](../../../design/DesignAuthorityRegistry.md#h-49); quote below; recorded 2026-10-27 |
| AD-044 | The neurons already have a semantic-vector arrangement and no natural physical positions. Correct the premise of the physical-distance assessment; this does not select another topology. | [H-50](../../../design/DesignAuthorityRegistry.md#h-50); quote below; recorded 2026-10-27 |
| AD-045 | Correct the documents, then continue substantive deeper research for at least 12 hours. Track evidence and actual research time; implementation remains deferred. | [H-51](../../../design/DesignAuthorityRegistry.md#h-51); quote below; recorded 2026-10-27 |
| AD-046 | Make the three mental regimes' steady attractor behavior and expected convergence from varied entry conditions the primary learning objective. An intermediate objective is that every token, under its own isolated stimulus, evokes self-recurrent activation that gradually fades back to DMN, ideally in about 0.5 seconds. AD-047 specifies the stimulus/noise sequence; numerical tolerances and mechanisms remain open. | [H-52](../../../design/DesignAuthorityRegistry.md#h-52); quote below; recorded 2026-10-28 |
| AD-047 | Under external attention demands, substantially suppress background white noise globally. Present a token by randomly activating the corresponding fraction of cells according to each coordinate's strength once, then let recurrent activity fade to DMN under low noise. After DMN detection, raise white noise to maintain activity; DMN slowly converges toward negative semantic content, not negative coordinate values. Exact encoder, noise levels, global scope, detector and feedback algorithm remain open. | [H-53](../../../design/DesignAuthorityRegistry.md#h-53); clarification below; recorded 2026-10-28 |
| AD-048 | In the early model, omit explicit inhibitory synaptic connections and use intrinsic mutual inhibition/cancellation between opposite-polarity populations within each semantic dimension. From a negative-semantic DMN state, a complementary positive token should briefly lower overall activation and leave a positive-semantic residual with longer noise-supported persistence. Detailed cancellation and lifetime criteria remain open. | [H-54](../../../design/DesignAuthorityRegistry.md#h-54); quote below; recorded 2026-10-28 |
| AD-049 | Maintain total active fraction within a flexible target range. Low activity after cancellation increases white noise before DMN detection if needed. The small positive residual should strengthen preferentially to a peak, then begin token-like fading/return. Exact control law, growth time, peak and timing tolerances remain open. | [H-55](../../../design/DesignAuthorityRegistry.md#h-55); clarification below; recorded 2026-10-28 |
| AD-050 | Exclude total active fraction from regime detection because it is a regulation target. Over a flexible window, DMN covers most neurons (over80% is an example), immersion remains within a small subset, and flow follows an activation path through different groups. These are detection/training directions; exact window, thresholds, trajectory criteria and overlap handling remain open. | [H-56](../../../design/DesignAuthorityRegistry.md#h-56); quote below; recorded 2026-10-28 |
| AD-051 | Flow cannot cover a large neuron population within the short observation window. Coverage above an upper bound, illustratively20%, must be classified as non-flow. Progression across groups still occurs within the bounded footprint; this is not an instantaneous activity cap or an automatic DMN classification. | [H-57](../../../design/DesignAuthorityRegistry.md#h-57); quote below; recorded 2026-10-28 |
| AD-052 | Coverage must discount added white-noise activity. A possible method is to omit neurons with insufficient activation density over the window. The filtering requirement is explicit; the density definition, window, threshold and algorithm remain proposals. | [H-58](../../../design/DesignAuthorityRegistry.md#h-58); quote below; recorded 2026-10-28 |
| AD-053 | Autonomous recurrence is an expected state. Every intended normal state must remain steerable by external token input; if no external token input can break or redirect the recurrence, it is stale. A system-reset operation may be needed as recovery. Input-response criteria and reset trigger, scope, affected/retained state and policy remain open. | [H-59](../../../design/DesignAuthorityRegistry.md#h-59); quote below; recorded 2026-10-28 |
| AD-054 | Assess positional preferences against the training objectives, identify an interim conclusion and judge whether deeper discussion is worthwhile. This authorizes reassessment; it does not adopt the resulting recommendation, choose a metric or authorize implementation. | [H-60](../../../design/DesignAuthorityRegistry.md#h-60); quote below; recorded 2026-10-28 |
| AD-055 | Omit physical-distance preference for now so SNN design can proceed without it. This accepts the staged omission, not permanent exclusion of geometry or an otherwise fully qualified implementation contract. | [H-61](../../../design/DesignAuthorityRegistry.md#h-61); quote below; recorded 2026-10-28 |
| AD-056 | Initial training proposal: use Hebbian learning for later flow training; expect immersion to emerge without directly training it; construct DMN and isolated-token activation chains directly. Behavioral acceptance remains applicable to every regime. | [H-61](../../../design/DesignAuthorityRegistry.md#h-61); quote below; recorded 2026-10-28 |
| AD-057 | Choose an initial activation width and eligible domain, randomly choose successive target groups, and connect each predecessor neuron to N targets. Design incoming M, outgoing N and strengths for self-maintaining propagation. Exact degree balance, randomness, neuron law and parameters remain to be analyzed. | [H-61](../../../design/DesignAuthorityRegistry.md#h-61); quote below; recorded 2026-10-28 |
| AD-058 | Propose stronger/weaker tail-to-head return for amplification/attenuation and extend head/tail to several activation steps, smoothing strength to avoid abrupt changes. This is an initial dynamical hypothesis; whole-loop gain, timing, smoothing and finite-response behavior remain open. | [H-61](../../../design/DesignAuthorityRegistry.md#h-61); quote below; recorded 2026-10-28 |
| AD-059 | Preliminarily favor determining all activation groups before execution to avoid overlap caused by repeated sampling. Request deeper comparison and alternative-effect analysis. Precomputation and disjointness remain distinct; no final overlap policy or per-step semantic-proportion rule is selected. | [H-62](../../../design/DesignAuthorityRegistry.md#h-62); quote below; recorded 2026-10-28 |
| AD-060 | DMN/token training includes ongoing reparative chain maintenance, not only initial construction. Seek a unified algorithm: when next-step activation is insufficient, select a sufficient proportion from the population that theoretically should activate but has not. Expected-target, selection, teaching and connection-update details remain open. | [H-63](../../../design/DesignAuthorityRegistry.md#h-63); quote below; recorded 2026-10-28 |
| AD-061 | Consider global/static planning combined with per-step sampling of next activation targets. Width/strength is one goal; avoid selecting earlier-step neurons as later targets because this can create unintended shortcut loops. Request deeper analysis of the resulting complexity. Scope of history, deliberate returns, whole-chain planning and checks remain open. | [H-64](../../../design/DesignAuthorityRegistry.md#h-64); quote below; recorded 2026-10-28 |
| AD-062 | The maintenance objective determines the participating scope. Select the concrete scope once per epoch: all CC neurons for DMN; for a token, select cells from each minicolumn in proportions determined by its semantic-vector values. This fixes scope selection frequency, not every step's batch, runtime readout or an epoch-wide reset of connections. | [H-65](../../../design/DesignAuthorityRegistry.md#h-65); quote below; recorded 2026-10-28 |
| AD-063 | Chain width is an approximate number of neurons activated per step, without an exact-count requirement. Scope size divided by width gives the rough number of steps per pass, related to maintenance duration. Step timing, closure details and gain also affect persistence; no numerical setting is chosen. | [H-65](../../../design/DesignAuthorityRegistry.md#h-65); quote below; recorded 2026-10-28 |
| AD-064 | Use objective-dependent body enhancement and tail-to-head attenuation as strategy parameters, allowing noise to assist weak recurrent activation and smooth fading without external signals. Exact gain convention, weight mapping, return profile and calibration remain open. | [H-65](../../../design/DesignAuthorityRegistry.md#h-65); quote below; recorded 2026-10-28 |
| AD-065 | Preliminary expectation: activation chains retain tail-to-head connections; a completely disconnected return is not the intended case. Analyze the half-second token response across recurrence. The accompanying agent analysis distinguishes connection presence from successful reactivation; it does not infer duration or observable lap count from wiring alone. | [H-66](../../../design/DesignAuthorityRegistry.md#h-66); quote below; recorded 2026-10-28 |
| AD-066 | Reinforcement and weakening strategies are subtle and require continuing attention to detail throughout design. The accompanying analysis examines gain definitions, coupling, thresholds, quantization, noise and realized semantic response; no numerical strategy is adopted. | [H-66](../../../design/DesignAuthorityRegistry.md#h-66); quote below; recorded 2026-10-28 |
| AD-067 | Preliminary preference: preserve the token vector's distribution in each step's sample, allowing at most a few steps to maintain it; short-term coordinate distribution must not deviate too far. Whole-epoch or whole-lap averages alone are insufficient. Exact window, tolerance, integer sampling and readout remain open. | [H-66](../../../design/DesignAuthorityRegistry.md#h-66); quote below; recorded 2026-10-28 |
| AD-068 | Permit a loose half-life-like quantitative description of token fading instead of requiring complete return at half a second. Exact precision is unnecessary; differences between tokens are expected, and timely fade-out is the objective. The observable, reference point, tolerance and significant-tail criterion remain open. | [H-67](../../../design/DesignAuthorityRegistry.md#h-67); quote below; recorded 2026-10-28 |
| AD-069 | Propose modeling spiking rate at a suitable discrete time scale and allowing activation-chain cessation; request deeper discussion as a major modeling decision. A particular rate variable, time step, update law or population aggregation is not selected. | [H-67](../../../design/DesignAuthorityRegistry.md#h-67); quote below; recorded 2026-10-28 |
| AD-070 | “断链” means natural cessation of active propagation, not eliminating connection weights. Both insufficient ongoing support and probabilistic propagation failure are intended possibilities. This does not select a timer, independent dropout rule or pruning operation. | [H-68](../../../design/DesignAuthorityRegistry.md#h-68); clarification below; recorded 2026-10-28 |
| AD-071 | Propose expressing neuron selection as fractional participation together with spike rate, addressing the remaining granularity of whole-neuron selection. Fraction semantics, interaction with rate, epoch scope and storage remain under discussion. | [H-69](../../../design/DesignAuthorityRegistry.md#h-69); quote below; recorded 2026-10-28 |
| AD-072 | Floating-point precision requires careful treatment across multiple stages of the design and cannot be ignored. Storage formats alone do not settle arithmetic/update behavior; exact precision, scaling and rounding policies remain open. | [H-70](../../../design/DesignAuthorityRegistry.md#h-70); quote below; recorded 2026-10-28 |
| AD-073 | While constructing large recurrent activation chains, avoid deliberately introducing intermediate shortcut loops, but do not aggressively eliminate small loops. Noise-supported small loops are desired when they let longer context influence later network outputs rather than relying solely on the main chain for long output. Dominant response fading need not erase all later history effects. | [H-71](../../../design/DesignAuthorityRegistry.md#h-71); quote below; recorded 2026-10-28 |
| AD-074 | Examine decay over practical accumulation periods rather than settling every small update independently; consider future scheduled triggers for applying the decay law. The exact accumulated quantity, model-time semantics, trigger invalidation and representation remain open. | [H-71](../../../design/DesignAuthorityRegistry.md#h-71); quote below; recorded 2026-10-28 |
| AD-075 | Investigate whether INT8 or INT4 is preferable for some modeled quantities. This requests field-specific analysis, not a blanket format replacement or selected scale, signedness, accumulator, rounding or packing policy. | [H-71](../../../design/DesignAuthorityRegistry.md#h-71); quote below; recorded 2026-10-28 |
| AD-076 | Autonomously spend twelve hours integrating these ideas with established directions and refining the modeling details. Record substantive research and design work; implementation remains deferred. | [H-71](../../../design/DesignAuthorityRegistry.md#h-71); quote below; recorded 2026-10-28 |
| AD-077 | Scheduled decay can precompute when the next minimum-precision-permitted change occurs, rather than choose a distant time target. This clarifies the successive local event direction; rounding boundaries, progress retention, input handling and execution remain open. | [H-72](../../../design/DesignAuthorityRegistry.md#h-72); quote below; recorded 2026-10-28 |
| AD-078 | Per-neuron state values need not be tightly restricted in precision: FP16 and FP32 may be considered because the array is relatively small. This opens the comparison; it does not select a format, phase layout or uniform precision for weights. | [H-73](../../../design/DesignAuthorityRegistry.md#h-73); quote below; recorded 2026-10-28 |
| AD-079 | Avoid overengineering. Introducing technical concepts, algorithms or data structures requires a real value-dependency chain derived from first principles. This governs cumulative modeling choices as well as individual mechanisms; literature precedent and technical possibility alone do not supply application value. | [H-74](../../../design/DesignAuthorityRegistry.md#h-74); quote below; recorded 2026-10-28 |
| AD-080 | Different neurons within a minicolumn represent different contextual paths for its shared semantic component, drawing on HTM. Individual learned identities have a purpose beyond aggregate magnitude; this does not select HTM segments, bursting or a new semantic-coordinate mapping. | [H-75](../../../design/DesignAuthorityRegistry.md#h-75); quote below; recorded 2026-10-28 |
| AD-081 | Propose alternating two state frames: next-frame activations read the previous snapshot through target-owned incoming slots. This simplifies computation and avoids persistent intermediate accumulation. Exact state meaning, transition, cancellation, time step and implementation remain under discussion; temporary incoming sums still exist. | [H-75](../../../design/DesignAuthorityRegistry.md#h-75); quote below; recorded 2026-10-28 |
| AD-082 | At token input, strongly activate cells when a sufficient proportion is already predictively activated; otherwise activate a broad part of the minicolumn at low intensity. This specifies context-dependent entry, while predictive criteria, proportions, strength functions and coordinate calibration remain open. | [H-76](../../../design/DesignAuthorityRegistry.md#h-76); quote below; recorded 2026-10-28 |
| AD-083 | Do not pair positive/negative cells by index. Compute the net over the minicolumn, then proportionally retain the spiking rates of active neurons in the surviving direction. This specifies full active-rate net cancellation and preserves surviving within-population ratios; prediction handling and transition timing remain open. | [H-77](../../../design/DesignAuthorityRegistry.md#h-77); quote below; recorded 2026-10-28 |
| AD-084 | Leave the neuron-state interpretation undecided and preserve room for future controlled implementation comparisons: one activity value with predictive/active ranges versus readiness distinct from current firing. This selects neither layout nor simultaneous storage, and does not request an experiment framework now. | [H-78](../../../design/DesignAuthorityRegistry.md#h-78); question and answer below; recorded 2026-10-28 |
| AD-085 | Hebbian and HTM learning apply specifically to flow training with token sequences. DMN and isolated-token construction/repair must remain a distinct training method. The prediction-guided token-entry rule does not make these methods identical. | [H-79](../../../design/DesignAuthorityRegistry.md#h-79); quote below; recorded 2026-10-28 |
| AD-086 | The flow-training algorithm has not yet been designed. The user expects it may differ from DMN/isolated-token repair; no common algorithm or particular alternative is selected. | [H-80](../../../design/DesignAuthorityRegistry.md#h-80); quote below; recorded 2026-10-28 |
| AD-087 | Include preliminary flow-training algorithm design in the ongoing goal and raise discussion points early. This authorizes exploration, not adoption of an algorithm or implementation. | [H-81](../../../design/DesignAuthorityRegistry.md#h-81); quote below; recorded 2026-10-28 |
| AD-088 | The direct ideal result of sequence training is predictive preactivation closely matching the input sequence. Preactivation need not sustain long autonomous output; matching suffices as the direct training criterion. Continuous output generation will have a separately designed mechanism/algorithm. | [H-82](../../../design/DesignAuthorityRegistry.md#h-82); question and answer below; recorded 2026-10-28 |
| AD-089 | Validate flow prediction as sequence context grows: multiple possibilities on matching shared fragments are expected, then predictive diversity should contract toward the training input sequence. A fixed single-continuation requirement at every step is not selected. | [H-83](../../../design/DesignAuthorityRegistry.md#h-83); question and answer below; recorded 2026-10-28 |
| AD-090 | Broad weak activation is a generation-mode behavior supplied explicitly rather than through ordinary neuronal connection transmission, and it is not learned. Refine the model using HTM details without treating this fallback as a flow teaching population. | [H-84](../../../design/DesignAuthorityRegistry.md#h-84); question and correction below; recorded 2026-10-28 |
| AD-091 | Leave opposite-sign predictive readiness versus predictive net cancellation open for later controlled comparison; do not decide now. | [H-85](../../../design/DesignAuthorityRegistry.md#h-85); structured reply below; recorded 2026-10-28 |
| AD-092 | Handle choices that are clearly suitable for later real-data comparisons directly as open empirical alternatives, without asking the user each time. | [H-86](../../../design/DesignAuthorityRegistry.md#h-86); explicit instruction below; recorded 2026-10-28 |

AD-002 / H-09:

> 参数量和内存需求要先讨论，确定一个目前分布式CPU集群可承载的规划，每minicolumn 100 神经元也许太大了，目前不现实。

In response to a question about cluster capacity:

> 每个CC的参数量，控制在一个树莓派可以承载的范围

AD-003 / H-10:

> minicolumn内部反而不很需要连接，参考 HTM 的理论阐述

AD-004 / H-11:

> 第一个业务应用单独编写文档集，不要与cod文档混

AD-005 / H-12:

> 单节点ram上限放宽到16GB

AD-006 / H-13:

> 代替dense matrix的sparse方案，考虑scale law，为多链神经元分配更多连接权重槽位，少连神经元分配少槽位，随着神经元连接权重的训练，特定神经元可以迁移到更多或更少槽位的预分配连接集上去

AD-007 / H-14:

> 基于scale law的连接权重槽位集合采用静态分配，神经元通过一个外链指针指向它的当前槽位集，不做堆内存分配。

AD-008 / H-15:

> 允许的最大外连数不能太小，总共 716800 个神经元，允许极少数外连10000个的量级不过分吧？

AD-009 / H-16:

> 你仔细检查连接槽集分配，每槽只需目标神经元编号加一个权重，5字节够了吧？ 然后槽集总数应该等于总神经元数，另有一个槽集指回源神经元的数组。 总的静态内存需求算准确

AD-010 / H-17:

> 5字节元素这种应该分成两个数组，一个4字节元素的，另一个1字节元素，这样CPU内存对齐友好。

AD-011 / H-18:

> 槽集个数比总神经元个数多些问题不大，迁移时可能节省内存搬运复制吧。槽集个数按方便基于scale law直接算各种下标的方便优先，避免依靠堆内存分配维护所有槽的拓扑形状。

AD-012 / H-19:

> 外连少于100的槽集可以直接不分配，现实中对外影响太小的神经元可以忽略其作用。将来发现100槽用不满的情况普遍且合理的话再调整这个最小槽集容量。

AD-013 / H-20:

> 翻倍这个scale可能不是最好的，调研看有没有人脑相关研究可以现在就给出一个更好的指数值的

AD-014 / AD-015 / H-21:

> 这个就这样暂定。然后从模拟计算方便的角度，槽集承载一个目标神经元的连入权重比较好吧，看看跟scale law会不会产生冲突？

AD-016 / H-22:

> 输入太多的神经元，太容易激活吧，现实意义感觉不合理，这方面有神经学、生物解剖方面的研究数据吗？

AD-017 / AD-018 / H-23:

> 所以连入多的神经元，其入连权重普遍偏低，也就可以不表现为过于容易激活，这个合理。 然后根据入连数分配槽集，适用的 scale law 参数可能还是跟连出的建模参数有所区别吧

AD-019 / H-24:

> weight scale 不应该是容量分配的设计参数，应该是训练策略考虑采用的超参数，甚至是动态规划的。你说的 “Next-class population” 这个是来自实验测量结果还是根据我们当前设计反推的？

The provenance question requests clarification. The fitted exponents are study results; adjacent-bin population ratios are our calculations using those exponents and selected spacing. The provisional halving rule is chosen geometry, with alpha near 3 inferred from it. These distinctions are documented in [the provenance analysis](ConnectivityScalingResearch.md#provenance-of-adjacent-class-population-ratios); they are an agent clarification, not a human adoption of the calculated quotas.

AD-020 / AD-021 / H-25:

> 容量分配就先这样暂定。 接下来看minicolumn对语义维的建模，我想到说用 100 个神经元表示正值，另外再分配100 个神经元表示负值，你分析一下这个方向会产生哪些设计变化？

AD-022 / H-26:

> 人脑每CC的神经元个数，有研究实测的是多少？

AD-023 / H-27:

> 这样看，我们考虑用更少维度的 Deepseek V4.1 Flash 语义向量，然后每 MiniColumn 60 positive + 60 negative 神经元这样，你按这个计算一下

AD-024 / H-28:

> 怎么还出 “Alignment padding” 了？ 一个计算节点负责多个CC时，语义向量表可以共享，不应该往单CC的内存需求上加。

AD-025 / H-29:

> 对于多字段，除非有特殊理由（访问模式决定的缓存友好性）且实证，否则默认做 SoA 而不是 AoS。

AD-026 / H-30:

> 一个字段如果bit数不整齐，要单独讨论对齐问题。

AD-027 / H-31:

> nonstandard bit width 应该只要不是 8 乘以 1,2,4,8… 就算

AD-028 / H-32:

> FP4 是 8*2^-1 ，也算 standard bit width

AD-029 / H-34:

> 这些辅助数据结构，先不引入预设，只有明确了业务需求，讨论实现算法时才考虑。 更新文档去掉这些内容，并增加原则说明段落。

AD-030 / H-35:

> 这个 Pool A是啥？ 可以先不考虑 V4Pro 的7168维语义向量了。

Class-count question / H-37:

> 目前总共15个class是硬性的吗？ 其它个数，比如16个，行不行？

This requests assessment; it does not select 16 classes.

AD-031 / H-38:

> 目标是总槽集数略微大于总神经元数吧，这个作为明确目标没有？

AD-032 / H-39:

> 没必要 preserve “connection capacity from the older sizing case,” 吧？


AD-033 / H-40:

> 接下来讨论神经元连接学习算法中的邻近优先原则，其拓扑采用一维还是二维有没有本质区别。 基本的学习还是 fire-together->wire-together，只是对方节点选择上，肯定要按（排除一小片极近的豁免区之后）越近则选择的概率越高的原则。 你从拓扑学相关研究的调研入手，看看简单的一维邻近拓扑就够，还是用更接近生物现实的二维拓扑有本质性优势。

AD-034 / H-41:

> 对，我们的模型首先会把神经元下标域安排给 minicolumn 布局使用，而我们刻意让 minicolumn 代表语义向量的一维，鉴于语义向量的维度之间不该有距离远近概念，我们不能让距离偏好影响各 维度/minicolumn 的正交性。 也许可以考虑 “距离” 概念映射到 minicolumn 内部神经元之间的拓扑关系？ 也就是一个神经元与其它 minicolumn 的连接概率相等，但是选择对方 minicolumn 内部个体神经元时，偏好等位距离更近的。 这个设计方向有多大程度上成立，值得看看有没有语义向量与人脑神经元物理位置对应关系的研究成果。

AD-035 / AD-036 / H-42:

> 那 minicolumn 内部位置去产生跨 minicolumn 的距离偏好确实没有说得通的理由，这个方向不该走。 接下来值得深挖语义向量与人脑的单个CC范围内二维拓扑的映射关系的相关研究成果，找找启发思路。

AD-037 / H-43:

> 我们的设计，有一个源自 HTM 的前提假定是 CC 是语义呈现的基本单元，CC与CC之间是非精确replica关系，minicolumn代表一定的 概念/语义，所以目前为止神经元的 “**response properties across semantic space**” 我们是静态分配的而不是要求网络去学出来。 基于这个澄清再看看

AD-038 / H-44:

> 是啊，现在 1D vs 2D 已经不是主要问题了，首先要面对的是神经元之间的相对位置，我们建模基于语义向量分配的物理位置，跟生物解剖现实中的的神经元物理位置拓扑，如何消除明显冲突。 继续深挖相关研究成果吧

AD-039 / H-45:

> LLM的语义向量，应该没有单维激活倾向吧？ 普通token应该是不同程度激活多个维度的吧，这个有统计数据可以查的吧，看看 DS  V4.1F 的真实情况

> 看 deepseek-ai/DeepSeek-V4.1-Flash-1017 ，本地ssd是否能存下语义表？

AD-040 / H-46:

> 所以人脑测量 “feature” 不跟 minicolumn 一一对应并不跟我们的建模冲突。 我们要训练的 SNN，并非精确语义单维激活，而是： 无任务 DMN 状态 —— 白噪音维持一定比例范围的稳态循环激活，语义上负面信息比正面信息后活跃稍强，所以稳态会慢慢滑向综合负面状态的极端；任务心流状态 —— token序列式轮番激活，整体仍维持一定比例范围的稳态激活；沉浸状态 —— 多维度语义向量循环式自我激活，循环链条长短不一，在一个特定范围内，整体神经元激活比例也维持一定的比例范围。

AD-041 / H-47, answer to whether the activity-fraction bound applies within each CC, across the whole network, or both:

> Within each CC

AD-042 / H-48:

> 每个CC内部激活比例是稳的，整体就不会失稳吧？ 另外这个比例不要求每个相态都一样，也不要太窄，应该允许不同心智状态在激活比例上也有所区别。

The whole-network weighted-average bound in [ActivationRegimes](ActivationRegimes.md#whole-network-activity-follows-from-the-per-cc-bounds) is an agent mathematical consequence under consistent counting/window definitions. H-48 asks about that consequence; it does not approve an unrestricted dynamical-stability theorem. Semantic valence, coordinate sign, neuronal phases and mental regimes remain separate meanings.

AD-043 / H-49:

> 继续深挖科研成果，找到我们的建模应该如何处理神经元物理距离偏好的支持依据

AD-044 / H-50:

> 我们已经按语义向量安排神经元位置了，没有天然物理位置，你自己晕了？

AD-045 / H-51:

> 修正文档以后继续深挖，花足够成本，至少12小时以上

AD-046 / H-52:

> **what relationships the connections learn**？ 主要是三相的稳态吸引子，力求各种入口条件都按预期收敛。 中间学习目标还包括每个token都在唯一性输入刺激下，自我循环激活，整体逐渐淡出回归DMN相态，理想的淡出时间是每token持续半秒左右回归 DMN 相态。

AD-047 / H-53, in response to clarification of the isolated-token stimulus and background noise:

> DMN 背景白噪音应有一定的全局抑制机制，在外部要求专注的场景下有相当幅度的降低。 token 唯一性输入应当只需要按语义向量各维强度随机激活相应比例的神经元一次，并在低噪音环境下稳态淡出，回归 DMN。 系统检测到 DMN 相态后提升背景白噪音维持激活比例，DMN则应当缓慢向负面语义（而非负值）方向收敛。

AD-048 / H-54:

> 我们早期阶段先不建模神经元的抑制性连接，固有的让一个语义维度中代表正负值的两组神经元相互抑制，其激活相互抵消。 预期是 DMN 滑到较负面语义状态时，外部与之互补的正面语义token刺激，短期会快速降低整体激活水平，让输入的正面语义激活在对冲后留下的残值，在背景噪音辅助下，能够维持更长时间。

AD-049 / H-55, clarifying noise support before DMN detection:

> 系统有维持整体激活比例在柔性目标范围的机制，正负语义抵消以后的整体低激活水平应导致系统提高背景噪音水平；但轻微的正面语义残留，应在背景白噪音下首先加强，达到峰值以后才按单token训练目标那样开始回落回归

AD-050 / H-56:

> 训练目标和检测条件：
>
> 总激活比例是系统调节目标，所以不参与检测。
>
> 检测逻辑大概是，在一个柔性窗口内：
>
> DMN - 激活覆盖大部分（比如80%以上）神经元
> 沉浸 - 激活范围维持在少部分神经元
> 任务心流 - 激活路径保持扫过不同神经元群

AD-051 / H-57:

> 心流状态不可能短时间扫过大的神经元覆盖面，总覆盖面如果超过（比如20%）就应该判非心流状态

AD-052 / H-58:

> 覆盖面判断有必要剔除系统附加的白噪音，可能的办法是特定神经元的激活密度小于一定程度就不算

AD-053 / H-59:

> 自动循环激活属于预期状态，如果任何外部token输入都不能打破，那么确实属于stale，可能需要系统性重置操作机制。 预期的正常状态，都可以由外部token输入驱使网络转向

AD-054 / H-60:

> 基于这些训练目标，看看神经元位置选择偏好方面，可以有什么阶段性结论，是否值得继续深入探讨

AD-055–AD-058 / H-61:

> 确实，物理距离倾向性可以暂不加入，也能先设计实现我们的 SNN。 另外需要专门提出训练方面我的初步想法，你分析记录： Hebbian 训练是后期 flow 训练的方法；immersion是预期的涌现行为不主动训练；DMN以及单token训练，采用直接创造激活链的方式 —— 决定一个初始激活规模，或者说激活链宽度，第一步激活这么多个范围内（DMN来说是全部，token来说是语义向量决定的各维比例选出神经元范围）的神经元；然后随机选出第二部激活的目标神经元，继而让第一步的每个神经元建立到 N 个第二步神经元的连接，理论上导致每个第二步神经元新建 M 个连入连接。 设计 M、N 以及各新连接的强度，让激活链足够自我维持。 链尾往链头的连接强度如果更大，则产生增强激活效果，如果更小，则产生减弱效果。 且链尾和链头的概念要拓宽到多步激活群，平滑更新强度避免断崖式变化。

AD-059 / H-62:

> 我倾向于开始之前静态把各批激活神经元确定下来，也就是避免每一步重新取样造成的重叠。 但这个想法比较初步，你还需深入分析比较与替代方法的效果预期差别，保持讨论这个话题。

AD-060 / H-63:

> 我又想了一下，DMN和token训练不止是初始建造激活链，其实持续训练迭代中更多的会是修补性激活链维护，这可能更好的是设计一个统一训练算法，下一步激活量不足时，从（理论上）该激活而未激活的群体中选足够比例作为下一步激活的神经元群，你这样调整一下

AD-061 / H-64:

> 可能要静态考虑全局+每一步采样选取下一步的激活目标，激活链 宽度/强度 是一个目标，前序步骤的神经元也要避免选为后续的下一步目标，否则有可能建立预期外的捷径循环了，整体应该还是有复杂度，你深入思考分析

AD-062–AD-064 / H-65:

> 激活链如何走一遍，涉及的神经元群体，由 维护（训练/建造）目标决定，并应景设计不同的策略参数，主要包括：
>
> - 激活链宽度 —— 一步激活神经元的（大致）个数（不要求精确），范围内神经元总个数除以这个宽度，就是大体的一次循环步数，与维持时间直接相关
> - 链主体过程的增强程度 和 链尾衔接链头过渡过程的削弱程度 —— 让背景噪音能够辅助增强弱激活循环，以及没有外部信号时平稳淡出
>
> DMN 训练时是全部神经元作为范围；单 token 训练时是按语义向量值作为比例选出各 minicolumn 中的神经元作为范围。 每个epoch选一次具体神经元范围。

AD-065–AD-067 / H-66:

> “the half-second token response” 只有在链尾不接链头的情况下才等于一个lap，我初步预期中不应该有链尾完全不接链头的情形，你分析看看呢
>
> 强化与削弱策略的确会非常微妙，需要仔细对待，设计过程要一直注意细节。
>
> 我初步判断还是应该每一步采样都保持 token 向量的分布，最多是放宽到少数几步以内维持分布，不能短时间内与token vector原本的维度分布偏离太多。

AD-068/AD-069 / H-67:

> 半秒目标可以改为半衰期性质的定量要求，总之不需要严格精确，不同token有差异也是预期的，只要及时fade-out就行，具体时间可以宽松。
>
> 你提的两个设计问题，我感觉本质上是严格 spike-by-spike 建模模拟 SNN 的局限，我觉得可以转而在合适的离散时间尺度上建模 spiking-rate ，引入一定的断链条件，这样语义vector各维的数值就不容易被明显偏少的神经元个数搞得过于割裂。 你深入想下去，这是个挺大的设计决定和讨论话题。

AD-070 / H-68:

In response to the question distinguishing (1) stopping weak ongoing propagation while retaining learned connections, (2) probabilistic propagation failure with connections retained, and (3) removing/zeroing learned connections, the user answered:

> 1 和 2 都是

The user then clarified:

> 我说的 “断链” 是指激活链自然断掉，不是连接权重消除。

AD-071 / H-69:

> 选神经元使用离散整个形态确实仍是问题的一部分，我觉得可以用分数表示选中比例，与spike-rate配合，大概效果就对了

AD-072 / H-70:

> 对，浮点数精度需要在很多环节仔细处理，不容忽视。

AD-073–AD-076 / H-71:

> 我们构建大（循环）激活链的同时，虽然避免主动制造中途的短路小循环，但是也不应该强力消除小循环，运行在一定背景噪音加持下能够维持的小循环也是期待的目标，这样长上下文才能给后续网络输出施加影响，而不是仅仅依靠直接激活链的主干产生连续的长输出。
>
> 衰减确实有必要考虑上实用的累积周期，而不是单步结算就完了，也许可以考虑对衰减律做未来触发定时器的方式生效之类的，你仔细深入考虑这个方向。
>
> 更极端一些，你深入考虑如果采用 INT8 甚至 INT4 来表达一些建模量，是否更优呢？
>
> 你自主推进12小时，综合以上思路和既定方向，完善打磨建模细节。

AD-077 / H-72 — user clarification, recorded 2026-10-28:

> scheduled decay 可以是预计算下一次最小精度允许的变化发生在什么时候，而不是安排过远的时间目标

AD-078 / H-73 — user direction, recorded 2026-10-28:

> 每神经元的状态数值确实不用太限制，FP16 甚至 FP32 都可以考虑，因为规模确实不大。

AD-079 / H-74 — user constraint, recorded 2026-10-28:

> 切忌朝过度工程方向走，技术概念、算法、数据结构的引入，都必须有真实第一性原则沿袭下来的价值依赖链支持。

AD-080/AD-081 / H-75 — user clarification and proposal, recorded 2026-10-28:

> 一个minicolumn内的不同神经元代表同一概念的不同细分路径，正式 Numenta 的 HTM 相关理论中的重要模型之一。
>
> 我们的 SNN 更新可以采用 2 frame 轮换更新的算法吧？ 下一frame的神经元激活值，从上一frame的快照中取，配合对连入的槽集组织，计算简便很多，不必考虑中间累积状态。

AD-082 / H-76 — user direction, recorded 2026-10-28:

> 输入token生效时，遵循 Numenta 的 minicolumn 预测规则： 已经有足够比例预测性激活的神经元时，高度激活这些神经元；没有时，低强度激活 minicolumn 所属内大范围的神经元。

AD-083 / H-77 — user direction, recorded 2026-10-28:

> 确实 minicolumn 内的正负抵消不该按下标对应，没有合理对应理由，应该是在 minicolumn 范围整体计算净余，然后按比例保留相同方向活跃神经元的 spiking-rate

AD-084 / H-78 — response to a state-model comparison question, recorded 2026-10-28:

The question contrasted one activity value with predictive/active ranges and separate predictive readiness/current firing. The user answered:

> 设计上留将来实现时能做对照实验的余地，目前不决定

AD-085 / H-79 — user clarification, recorded 2026-10-28:

> 注意使用token序列训练flow相态时才是 Hebbian 与 HTM 学习模式，不要跟 DMN 和 单token 训练混淆。

AD-086 / H-80 — user clarification, recorded 2026-10-28:

> flow学习训练算法还没开始设计，我感觉很可能不是 DMN 和 单token 那种修复算法。

AD-087 / H-81 — user design instruction, recorded 2026-10-28:

> 你的目标包含做 flow 训练算法的初步设计，有待讨论的点尽早提出来跟我讨论

AD-088 / H-82 — response to the flow-learning objective question, recorded 2026-10-28:

The question asked whether sequence learning should preactivate cells for the next external token, produce the next semantic vector before its arrival, or pursue both objectives. The user answered:

> 序列训练的直接理想结果是预测性预激活的神经元群与输入序列高度匹配。 预激活强度有可能不足以维持长序列自主连续生成输出，作为训练的检验条件，高度匹配就够了。 持续输出的生成机制算法另行设计。

AD-089 / H-83 — response to the ambiguous-continuation question, recorded 2026-10-28:

The question contrasted retaining multiple plausible predictive populations with favoring one continuation. The user answered:

> 这个验证时是随着序列长度变化的，匹配的序列段的时候预测可能性多是预期的，随着序列变长，预期是预测预激活的多样性收缩，向训练输入token序列收敛

AD-090 / H-84 — correction of the flow-learning-recipient question, recorded 2026-10-28:

The question proposed choosing learning recipients from broad weak activation after an unexpected token. The user corrected that premise:

> “keeps activation broad and weak” 是生成模式下的表现，并非依赖正常神经元连接的传到实现，自然也不予学习。 你结合 HTM 细节打磨我们的相关细节。

AD-091 / H-85 — predictive cancellation comparison, recorded 2026-10-28:

Question: When plausible next tokens require opposite signs on the same semantic coordinate, should both neuron groups be allowed to remain predictively ready until input resolves the ambiguity? This concerns prediction semantics, not the deferred choice of state fields.

> 留设计空间到时候做对照实验吧，当前不做决定

AD-092 / H-86 — handling empirical design alternatives, recorded 2026-10-28:

> 有些明显可以留作真实数据做对照实验的，你直接按这个处理，不用问我了

## Proposal history and current status

| Proposal | Status / reason |
| --- | --- |
| Two-frame incoming gather | AD-081 proposes the preferred simple execution direction: immutable preceding state, destination-local sums and a completed next frame. FP32 scalar state in both frames costs 4.6875 MiB; weights remain single. The state and cancellation semantics remain open. |
| Next-precision-change decay scheduling | AD-077 identifies the intended near-term prediction. Exact code boundaries and accumulated progress need explicit arithmetic; extra event structures require demonstrated benefit under AD-079. A straightforward FP32 discrete-time reference is an agent-proposed comparison baseline, not an adopted implementation. |
| Wider neuron state | AD-078 allows FP16/FP32 consideration. One field costs 1.171875/2.34375 MiB at the current population; weights and intermediates need separate comparisons. Existing four-bit named-array totals remain conditional baselines. |
| Physical-distance preference at this stage | AD-055 accepts omitting it for now. Geometry is not a prerequisite for the current [training discussion](TrainingStrategy.md); a future proposal still needs a concrete benefit. |
| Unified DMN/token construction and maintenance | AD-060 emphasizes repeated repair of deficient expected next-step activation. Naturally achieved participation and selected missing participants are distinct; initial construction uses the same direction. Target semantics, selection fraction, teaching support, edge updates and assessment remain open. AD-085 reserves Hebbian/HTM for token-sequence flow; AD-086/AD-087 open its separate initial algorithm design. Immersion has no dedicated training objective. |
| Epoch scope and strategy parameters | AD-062–AD-064 fix one scope per epoch, approximate width, body enhancement and return attenuation roles. Scope selection and stepwise sampling are distinct. AD-067 prefers per-step token fidelity with only a few-step allowance; its exact tolerance/window, gain convention, physical step time and numerical parameters remain open. |
| Fractional recruitment and rate dynamics | AD-069/AD-071 open a major model change. Individual-neuron effective rates with fractional training targets are an agent recommendation; binary counts, population aggregation and explicit spikes remain distinct comparison levels. A no-reuse whole-cell bound is conditional on that assignment policy. AD-070 retains learned weights; AD-072 requires numerical care. No update law, new field or implementation is selected. |
| Whole-chain constraints and adaptive target sampling | AD-061 requests global/static consideration and avoidance of earlier-step targets that can create unintended shortcuts. [Analysis](TrainingStrategy.md#global-constraints-and-stepwise-sampling) recommends constraining semantic/quantity profiles, future resources and permitted returns while sampling concrete targets adaptively. A fresh target can still have an existing return path; structural opportunity and harmful activation are distinct. The detailed plan/history/check algorithm is unselected. |
| Activation-chain repair and closure | AD-064 assigns enhancement to the body and attenuation to the distributed tail/head transition; AD-057/AD-058's group and edge analysis remains relevant. Existing support and missing contributions are counted separately. AD-065 expects retained return connections. Within-lap growth can coexist with net attenuation across repeated laps; repair still follows the intended response. Connectivity does not alone guarantee successful reactivation or a particular visible lap count. |
| Precomputed activation groups | AD-062 fixes the enclosing neuron scope once per epoch; it does not fix every batch. AD-059's precomputed batches and stepwise choices within the scope remain [alternatives](TrainingStrategy.md#alternatives-worth-comparing). Cross-token interference and fresh-input entry remain open; no runtime group structure is chosen. |
| External-token steerability | AD-053 requires every normal state to remain externally steerable. Repetition, long immersion or normal active fraction does not establish stale; loss of effective input response does. Admissible inputs, response observables and the practical assessment horizon remain open. |
| System reset for stale recovery | AD-053 raises reset as a possible recovery mechanism. Trigger, scope, retained/affected state, restart condition and automatic policy are unselected. This is a candidate, not a required reset on a single failed input. |
| Physical distance as an opportunity prior | Withdrawn agent recommendation after AD-044 / H-50, as recorded in K-37. The existing semantic arrangement supplies no physical metric. Biological findings remain references in [PhysicalDistancePreferenceResearch](PhysicalDistancePreferenceResearch.md); no choice between explicit positions and a contact abstraction is currently required. |
| Steady activity and three mental regimes | AD-040–AD-042 specify broad per-CC occupancy bounds and distinct semantic trajectories. AD-046 makes their steady attractor behavior and convergence from varied entry conditions the primary learning objective. No exact activity set point, equal phase populations, controller implementation, valence estimator or transition/learning formula is selected. |
| Token-evoked recurrence and return to DMN | AD-068 permits a loose half-life-like timing scale around half a second with expected variation and timely fading. AD-047 retains one stochastic presentation and the noise conditions; AD-070 allows active propagation to cease naturally while learned weights remain. AD-073 permits useful small-loop context after the dominant response fades. Its envelope, individual-trial behavior, reference point and distinction from retained context remain open. |
| Attention/activity/DMN background-noise control | AD-047 requires attention-dependent suppression and DMN activity support. AD-049 also raises noise in response to a low active fraction before DMN detection; the positive residual grows to a peak and then fades. AD-050 excludes the regulated fraction from classification. Global scope, noise levels and control/detector algorithms remain open. |
| Intrinsic opposite-polarity cancellation | AD-048 requires within-dimension mutual inhibition/cancellation and excludes explicit inhibitory synaptic connections in the early model. The function changes activity, rather than only subtracting unchanged populations at readout; cell/phase/update details remain open. |
| Regime detection | AD-050–AD-052 use qualified window coverage and paths: broad coverage for DMN, a small persistent footprint for immersion and progression within a limited footprint for flow. Exceeding an illustrative20% cap rules out flow. Noise must be discounted; a density gate is proposed. Total active fraction is excluded. Window/threshold definitions and low-coverage temporal distinctions remain open. |
| Biological feature / minicolumn correspondence | AD-040 permits non-bijective correspondence. Distributed semantic features are compatible with fixed coordinate roles; feature heterogeneity alone does not require interleaving or re-encoding. |
| Proximity-biased coactivity learning | AD-033 remains the direction, with AD-034's dimension-index neutrality. AD-035 rejects the internal-position mechanism; no geometry or distance kernel is selected. [Semantic cortical mapping research](SemanticCorticalMappingResearch.md) assesses AD-036's research scope. |
| Shared internal positions across minicolumns | Rejected by AD-035 / H-42. Equal cell index or an imposed shared internal coordinate supplies no accepted cross-minicolumn distance preference. |
| Spatial interleaving of fixed semantic roles / selective neurite reach | Alternatives examined under AD-038, not adopted. Interleaving relaxes anatomical compactness and alone provides only expected label symmetry, not exact equal proposals for every source; H-41 did not select that full probability contract. Selective reach requires a justified opportunity model; neither adds preset structures or changes capacity budgets. |
| Spatially organized neuronal response functions | AD-037 fixes semantic assignments and approximate-replica CCs; AD-038 prioritizes anatomical compatibility. Research distinguishes semantic group, compact anatomical population, neurite opportunity and storage order. Learning replacement roles remains outside the baseline. |
| Dense local minicolumn blocks plus sparse or low-rank global coupling | Earlier assistant capacity sketch, not approved. AD-003 redirects the active proposal to contextual segments without assumed dense local blocks. Low-rank coupling is not equivalent to arbitrary sparse context matching. |
| Pi 5 with 4 GB as a provisional reference | Earlier assistant assumption, replaced for current planning by the explicit AD-005 16 GB node ceiling. Pi 5 remains a reference CPU, not a confirmed fleet inventory. |
| Flash pool size | AD-031 makes M slightly above N the objective; AD-032 excludes preserving earlier connection capacity. The [comparison](ConnectivityScalingResearch.md#sizing-objective-and-class-count-options) uses the smallest exact-halving pools above N: 15 classes give M=622573, 1.330241% surplus and 515.15 MiB; 16 give M=655350, 6.665039% surplus and 542.88 MiB. Final J and quotas remain open; execution uses a static pool. |
| Equal positive/negative cell populations | AD-023 makes Flash 5,120 × (60+60) the current sizing case: 614,400 neurons. The worked 15-class A=311296 candidate gives about 515.15 MiB of per-CC named arrays. Current planning uses the Flash 60+60 case under AD-030. Signed readout, magnitude/context separation, coactivation, prediction and training remain open. |
| Field width classification | AD-028 supersedes AD-027’s exponent restriction: standard widths are 1, 2, 4, 8, 16, 32, ... bits. FP4 is standard; 12/24/48-bit fields remain nonstandard and require AD-026’s separate discussion. Current FP4 storage figures retain two values per byte; classification adds no padding and does not select an access/update protocol. |
| Multi-field data layout | AD-025 establishes SoA as the default. AoS requires an explicit access-pattern/cache rationale supported by measurements; no exception is currently established. |
| Shared embedding storage | AD-024 excludes the embedding table from per-CC storage. The current two-byte table uses 1,323,827,200 bytes once per node; node totals add k CC payloads, the shared table and separately budgeted runtime/state. |
| Biological column size | AD-022 requests measured evidence. Human V1 tissue measurements support a study-specific ocular-dominance module estimate of about 159,000 neurons. The current 614,400-neuron case is about 3.86 times that population. This does not establish a universal human CC count or functional equivalence. |
| Sparse contextual segments, `m=8/16`, `s=4`, `k=32` | Uniform arithmetic reference. AD-006 makes variable capacity the active direction; four segments and 32 synapses do not constrain every neuron. |
| 512 MiB private planning allowance for the 16-cell candidate | Proposal with explicit storage arithmetic; no measured peak or throughput guarantee. Thirty-two and 100 cells remain comparison points. |
| Retaining every old live set until DBMR compaction | Earlier assistant interpretation applied history retention too broadly. AD-007 fixes a static live pool; assignment changes need not create history. Reuse of unchanged typed set objects is the current proposal, with its release boundary still open. |
| Extra sets and per-edge reverse indexes | AD-009 excluded both from its exact-N calculation. AD-011 subsequently permits extra sets for indexing/migration; per-edge reverse indexes remain absent from the base budget. AD-031 explicitly targets a small surplus; the worked 15-class candidate provides 8,173 sets (1.330241%) above N. |
| Auxiliary data structures | AD-029 defers consideration until business requirements are clear and implementation algorithms are discussed. Current planning prescribes no auxiliary representation or speculative memory allowance. |
| Geometric classes and occupancy thresholds | Assistant proposals. AD-009/AD-010 fix outgoing slot contents and layout; AD-014 provisionally retains the alpha-near-3 class counts; migration publication remains open; auxiliary representations are deferred under AD-029 until requirements and algorithm discussion justify them. |
| Maximum connection-set capacity | The 15-class candidate reaches 12,800 slots with 19 largest sets; the 16-class candidate reaches 18,432 with 10. Class count is open. H-15’s approximately 10,000 outgoing-degree case remains distinct from the incoming-capacity limit. |
| Equal-slot geometric classes | Earlier comparison, outside the current Flash-only planning scope. The worked candidate uses 15 alpha-near-3 classes; 16 is also feasible. |
| Exponent near 3 with finer capacity classes | Assistant evaluation candidate under AD-013. Adult-fly evidence reports outgoing exponent 2.96; human evidence does not establish a universal value. Capacities alternate 100/144 times powers of two, while counts halve. The current Flash case has A=311296 sets in the 100-slot class, 622,573 total sets and 515.150806427001953125 MiB. AD-014 provisionally retains the class geometry; lognormal remains a comparison, not the chosen planning layout. |
| Target-owned incoming sets | AD-015 requests assessment. Recommended for synchronous gather: source IDs per slot, target ID per set, unchanged base storage. Incoming degree now determines required capacity; outgoing distribution, minimum-input admission and sparse-event routing are separate questions. |
| Input count versus excitability | AD-016 requests biological evidence. A flat positive sum with unchanged weight scale/threshold has degree bias; anatomy, human/animal recordings and constrained models motivate explicit timing, dendritic integration, inhibition and adaptation. Point-neuron control and branch-based integration remain alternatives; no numerical firing threshold or biological degree law is adopted. |
| Conditional incoming weight scale | AD-017 accepts the compensating direction as reasonable. AD-019 assigns weight-scale policy to training strategy, including possible hyperparameters or dynamic planning/adaptation. A fixed total or specific normalization rule remains unselected. |
| Separate incoming population exponent | AD-018 requests assessment. Adult-fly fitted point estimates are alpha_in=3.15 and alpha_out=2.96; adjacent-class ratios are calculated projections using our geometry. At the current Flash N=614400 and M=622573, the same-capacity 3.15 sensitivity calculation gives 10 largest sets and about 484.07 MiB, not an experimentally measured requirement. The existing alpha=3 allocation remains provisional. |
| Omitted outgoing sources | AD-012 permits excluding low-connectivity contributions. A null current-set reference with retained neuron identity/state is a source-owned representation proposal. Under incoming ownership, a null input set does not suppress that source’s outputs in other sets. Actual admission, suppression, discovery and re-entry rules remain open; automatic permanent removal below 100 live edges is not selected. |
| Four-bit phase/intensity, eight-bit synapse storage | User starting direction, encoding unresolved. Standard floats, custom codes and quantized permanence have different semantics. |

## Next decisions

| Topic | Exact open decision | Consequence |
| --- | --- | --- |
| [SNN-03/SNN-05](TrainingStrategy.md#generation-fallback-and-sequence-learning) | How should an unfamiliar observed training transition establish a contextual learning representation and associations, independently of the generation fallback? | AD-090 excludes the explicit broad weak generation response from learning. HTM active cells, learning winners and connection updates are distinct reference roles; no imported structures or cold-start rule are selected. |
| [SNN-01/SNN-02/SNN-05](TrainingStrategy.md#predictive-alternatives-and-signed-cancellation) | May opposite-sign populations retain simultaneous predictive readiness until input resolves the alternatives, or should readiness also undergo net cancellation? | AD-091 explicitly reserves this for controlled comparison. AD-092 directs obvious empirical alternatives to be handled without repeated questions. H-77 active-rate cancellation remains, and H-78 state fields remain undecided. |
| [SNN-03/SNN-05](TrainingStrategy.md#flow-learning-initial-design-scope) | How should pre-input matching and contraction of plausible predictions be measured as sequence context grows? | AD-088/AD-089 require close matching and context-dependent narrowing; continuous output generation is separate. Distinguish plausible continuation diversity from cell count and avoid a self-confirming score. |
| [SNN-03/SNN-05](ContextLoopsAndNumerics.md#useful-small-loops-and-dominant-response-fading) | Which later response differences establish useful small-loop context, and which interference actually defeats progression or steerability? | AD-073 rejects aggressive small-cycle elimination; a main-chain construction preference is not a whole-network acyclicity requirement. |
| [SNN-02/SNN-03/SNN-07](ContextLoopsAndNumerics.md#accumulating-decay-instead-of-losing-each-small-update) | What accumulated information and model-time rule preserve decay between representable updates, and when is scheduling worthwhile? | AD-074/AD-077 favor preserving accumulated progress and predicting the next permitted change; AD-079 requires demonstrated benefit before extra scheduling machinery. |
| [SNN-02/SNN-04/SNN-07](ContextLoopsAndNumerics.md#integer-formats-are-field-specific-choices) | Which quantities benefit from INT8/INT4, with which scale, accumulation and temporal error costs? | AD-075/AD-078 call for per-field comparison including FP16/FP32 state; AD-079 requires a real value chain for extra mechanisms. |
| [SNN-01/SNN-02/SNN-03/SNN-05](RateDynamics.md#fractional-recruitment-and-rate-have-different-possible-meanings) | What does fractional selection mean, what does rate measure, and which quantity must persist? | Avoids duplicating one effective activity as two unspecified factors or reintroducing whole-event granularity through immediate binary sampling. |
| [SNN-02/SNN-03/SNN-05](RateDynamics.md#natural-cessation-with-learned-connections-retained) | Which local transfer/noise conditions give natural cessation and later reactivation while preserving meaningful token proportions? | Makes AD-070 operational without weight deletion, token-specific runtime gates or assuming that an ensemble survival half-life proves gradual individual fading. |
| [SNN-02/SNN-03/SNN-07](RateDynamics.md#numerical-precision-is-part-of-the-behavior) | Which representation and arithmetic preserve weak contributions, cancellation residuals and gradual updates across useful time steps? | Makes AD-072 assessable; FP4/FP8 storage alone does not determine accumulation precision, error control or actual fading. |
| [SNN-03/SNN-05/SNN-06](ActivationRegimes.md#external-steerability-stale-states-and-recovery) | Which inputs and response criteria establish steerability, and if reset is adopted, what triggers it and what state is retained? | Makes AD-053 assessable without treating intended recurrence as failure or selecting a recovery implementation. |
| [SNN-02/SNN-03](ActivationRegimes.md) | What counts as active within a CC, over what time window, with what broad envelope and transient allowance? | Makes the selected local bound measurable while allowing different mental states to occupy different activity levels. |
| [SNN-03/SNN-05](ActivationRegimes.md#learning-objective-regime-attraction-and-token-transients) | Which entry conditions should reach which regime, and what demonstrates dominant token fading while useful history still affects later outputs? | Makes AD-046/AD-047 and AD-068 assessable while separating individual response envelopes, stochastic survival and the coverage-based return to DMN. |
| [SNN-03/SNN-05](ActivationRegimes.md#attention-dmn-detection-and-global-background-noise-control) | What flexible-window coverage/path criteria distinguish the regimes, and how do noise suppression, activity support and new token arrivals combine? | Specifies the required noise-control behavior before choosing its detector, feedback algorithm or representation. |
| [SNN-01/SNN-03/SNN-05](ActivationRegimes.md) | What defines negative semantic valence and its slight activation advantage, task-sequence success and an immersion loop's length? | Specifies the three semantic trajectories without substituting coordinate signs for meaning or choosing a learning rule prematurely. |
| [SNN-01/SNN-03/SNN-05/SNN-07](TrainingStrategy.md#epoch-scope-width-and-gain-parameters) | How do approximate width, step time, body enhancement and return attenuation jointly produce the intended persistence under noise? | AD-062–AD-064 fix their roles and epoch scope. Per-step versus aggregate enhancement changes the coupling to chain length; numerical calibration remains open. |
| [SNN-01/SNN-03/SNN-05](TrainingStrategy.md#token-proportions-apply-to-the-epoch-scope) | Which short window and error tolerance implement the per-step preference with at most a few-step allowance? | AD-067 excludes reliance on the whole-lap average alone. Integer epoch populations, nonrepeating progression and width must support short-window fidelity; repair balances the completed next-step group, including correct natural activity. |
| [SNN-01/SNN-03/SNN-05/SNN-07](TrainingStrategy.md#global-constraints-and-stepwise-sampling) | Which whole-chain constraints govern ordinary progression, history exclusion, permitted return and future candidate feasibility? | AD-061 requires deeper analysis; neither a lifetime no-reuse rule nor only a short recent-history exclusion resolves intended recurrence and shortcuts. |
| [SNN-02/SNN-03/SNN-05/SNN-07](TrainingStrategy.md#fresh-targets-can-still-close-unintended-loops) | What structural and response evidence distinguishes an unintended shortcut from useful recurrence in the shared graph? | Prior-participant exclusion alone is insufficient; full cycle rejection conflicts with recurrence, while bounded screening has explicit limits. Representation and work budgets remain open. |
| [SNN-02/SNN-03/SNN-05/SNN-07](TrainingStrategy.md#unified-next-step-repair) | How do selected missing cells lead to durable connection repair, and how is response assessed without extra teaching assistance? | Distinguishes target selection, temporary activation, existing-edge updates and new allocation; neither an update rule nor an extra runtime structure is selected. |
| [SNN-02/SNN-03/SNN-05/SNN-07](TrainingStrategy.md#propagation-depends-on-the-neuron-transition-law) | Which minimal neuron transition law makes constructed propagation, distributed closure and finite fading assessable? | Required before choosing M, N, weights or time per step; stronger return alone does not establish growth followed by fading. |
| [SNN-02/SNN-03/SNN-07](SynapticIntegrationResearch.md) | Which integration, input-timing and activity-control contract achieves the targets with intrinsic polarity cancellation and no explicit inhibitory synaptic edges? | Determines the meaning of activation, any branch or adaptation state, and acceptance beyond storage arithmetic. |
| [SNN-01/SNN-04](ConnectivityScalingResearch.md#sizing-objective-and-class-count-options) | Which class count and quotas best satisfy the small-surplus objective for Flash 60+60? | With N=614400 and exact halving, 15 classes give 1.330241% surplus and a 12,800-slot maximum; 16 give 6.665039% and 18,432. No old-capacity preservation requirement applies. |
| [SNN-05](SynapticIntegrationResearch.md#training-strategy-incoming-weight-scale) | Which training strategy, hyperparameters or dynamic planning/adaptation should govern weight scale? | Determines learned effective weights and any adaptation state; pure allocation migration preserves those weights and their effect. |
| [SNN-07](IncomingConnectivity.md) | Adopt incoming ownership and define gather/event-driven timing, incoming admission and the relationship to outgoing influence. | Determines update work, any routing index and the meaning of the 100-slot minimum. The alpha-near-3 layout is provisional. |
| [SNN-07](AdaptiveConnectivity.md) | What determines useful growth/pruning, incoming class shortages, migration/availability and source participation/re-entry? | Determines learned topology, fixed pool utilization and migration work; useful underfilling and output influence need separate evidence. History retention is separate. |
| [SNN-04](CapacityPlanning.md) | What context capacity, population and segment/synapse caps should the first model explore? What update/emission rate matters? | Determines memory, CPU demand and useful placement. |
| [SNN-03](Model.md#snn-03) | How do dense signed coordinates generate sparse column/cell activity while retaining required magnitude and context? | Determines both representational fidelity and activity-driven CPU cost. |
| [SNN-01](SignedPopulationEncoding.md#magnitude-and-contextual-cell-selection) | In the proposed positive/negative populations, do cells encode magnitude samples or select context with graded magnitude, and how is each population decoded? | Prevents contextual bursting from silently changing the represented coordinate. |
| [SNN-01/SNN-02](SignedPopulationEncoding.md#coactivation-prediction-and-transitions-through-zero) | How does the required intrinsic cancellation act on current and predictive activity, and what residual information survives? | Determines zero/sign transitions, hidden activity and information preserved in signed-vector messages. |
| [SNN-02](Model.md#snn-02) | How do current activation and future prediction coexist, and what does the four-bit code mean? | Determines state transitions, extra fields and arithmetic. |
| [SNN-06](Model.md#snn-06) | Which state is durable and how much history is retained? | Determines DBMR growth and compaction headroom. |

Detailed choices and candidate value relationships remain open. Implementation is deferred under the user's language-design instruction H-07; this document set supports the ongoing discussion.
