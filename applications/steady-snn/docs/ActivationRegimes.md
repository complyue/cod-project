# Steady recurrent SNN: activation regimes

Document-set revision: 30 · Recorded: 2026-10-27 · Topics: SNN-01/SNN-02/SNN-03/SNN-05/SNN-07.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 30, all sections; `pending-human`. The operating goals below are scoped human directions, not a qualified implementation contract.
- **Value lineage / dependencies:** candidate P-25/P-22 connect the application to C-12/C-01 and V-01/V-02: make intended recurrent behavior and its observable acceptance explicit. C-14/P-27 concerns signed coordinate representation; C-15/P-29 concerns learning; C-13/P-26 bounds connection storage. Dependencies do not supply approval of transition or control algorithms.
- **Human baseline / continuity:** H-43 fixes semantic roles and approximate-replica CCs. H-46 / AD-040 specifies the three operating regimes and states that measured biological features need not correspond one-to-one with minicolumns. H-47 / AD-041 places the activity bound within each CC. H-48 / AD-042 requires latitude for different mental states to have different activity fractions. [K-35](../../../design/DesignAuthorityRegistry.md#k-35) records the scoped update. H-45's embedding investigation concerns input representation, not a learned SNN result.
- **Invariants / non-goals:** preserve per-coordinate roles, current Flash 5120 × (60+60) planning, static SoA capacity and regional relocation. Coordinate polarity, semantic valence, neuronal phase and synaptic effect are distinct. No activation percentage, loop-length bound, timing rule, noise distribution, valence estimator, control circuit, new state field or implementation is selected.
- **Acceptance:** each regime must eventually have observable activity bounds and semantic-trajectory criteria. The definitions and missing parameters below must be resolved before implementation; these names describe intended application behavior, not established equivalence to human DMN, flow or immersion.

## Common target: bounded activity with changing semantic state

The model assigns semantic-coordinate roles to minicolumn populations. It does not require a token or a meaningful feature to activate exactly one coordinate. A multidimensional activation pattern can express a feature, and the same coordinate population can participate in many such patterns. Non-bijective correspondence between measured biological features and minicolumns therefore does not by itself contradict the model.

Across the three regimes, the active-neuron fraction **within each CC** should remain within a permissive range. A compact statement of the intended observable is:

```text
a_c(t) = number of active neurons in CC c at time t / N_c
L <= a_c(t) <= U
```

Here `N_c` is the neuron population of CC `c`; the current sizing case has 614,400 neurons per CC. This defines an observable, not a stored counter or an implementation. H-47 selects the per-CC scope. The activity definition, observation window, treatment of predictive cells, transient tolerance and numerical bounds remain open.

H-48 requires a broad operating allowance rather than a narrow clamp or fixed set point. DMN, flow and immersion may have different typical activity levels, ranges and fluctuations within that allowance. Their ordering and numbers are not selected. There is no requirement for the three regimes to have identical activity distributions, or for inactive, predictive and active neuronal phases to contain equal populations. Those neuronal phases and the mental operating regimes are separate classifications.

The decoded semantic state can evolve while the activity fraction stays bounded. Stable activity occupancy therefore does not require a fixed semantic vector or a stationary distribution of semantic content. Neither dense input coefficients nor nonzero FP4 values automatically define the neurons counted as active.

### Whole-network activity follows from the per-CC bounds

Using the same activity definition and observation time/window for every CC:

```text
A(t) = sum_c N_c * a_c(t) / sum_c N_c
L <= a_c(t) <= U for every c  =>  L <= A(t) <= U
```

The whole-network active fraction is a neuron-count-weighted average, so it cannot escape the common envelope while every CC respects it. Equal-size CCs give an ordinary average. If local bounds differ, the network fraction lies between the corresponding weighted-average lower and upper bounds. A separate whole-network fraction constraint or controller is not needed to obtain this arithmetic consequence.

This is stability of activity occupancy. Which neurons are active, their strengths and the represented semantic trajectory remain separate observables. Bounded occupancy permits the intended drift and recurrence; it does not by itself establish their quality or a dynamical-stability theorem. Nor do the bounds specify how coupled CCs maintain them: the eventual transition and learning rules must achieve the selected local behavior under cross-CC input. No extra global mechanism is proposed here.

## Desired regimes

| Regime | Human-specified behavior | Details still to define |
| --- | --- | --- |
| No-task / DMN | White-noise drive sustains recurrent activity within a proportion band. Semantically negative information has a slight activation advantage, with the semantic trajectory gradually drifting toward an extreme of composite negative content. | Noise channels, amplitude and temporal definition; what the activation advantage changes; semantic-valence criterion and meaning of the endpoint; allowable drift rate and active-fraction bounds. |
| Task / flow | Token-associated patterns activate sequentially, with the overall active fraction remaining within a band. | Sequence source and pacing, how successive vectors interact, retention versus replacement, noise during task drive, and task-level success. |
| Immersion | Multidimensional semantic patterns recurrently self-activate in loop chains whose lengths vary within a specified range. The overall active fraction stays within a band. | Definition of a loop and its length, semantic similarity tolerance, time units, coexistence of loops, perturbation behavior and permitted external drive. |

These are operating regimes for the SNN discussion. They do not prescribe three separate networks, a mode-switching state machine or a particular training curriculum. Their activity levels may differ under H-48. Transition behavior and any quantitative regime-specific bands remain open; the common envelope must leave room for those differences.

## Semantic negativity is not coordinate negativity

A vector with many negative coordinates is not thereby semantically negative. The positive/negative populations in the current encoding describe the sign of a coordinate, while semantic valence concerns the meaning of the whole represented pattern in context. Neither notion identifies excitatory versus inhibitory synapses.

The requested DMN drift is an explicit behavioral target. A valence criterion would need to identify the direction and endpoint independently of raw coordinate signs. The basis, labels, interpretation context and any evaluator remain to be discussed; no additional classifier or data structure is presumed.

White noise has no inherent negative semantic direction. In a model with a semantically neutral drive, the specified drift would have to come from an asymmetry in the learned dynamics or another explicitly selected part of the model. The user's slight activation advantage supplies a desired bias, but does not yet specify whether it changes activation probability, strength, persistence or transitions. A bounded active fraction must coexist with that drift; the requested endpoint is not permission for numerical amplitudes or connection weights to diverge.

## Learning and recurrence questions

H-40's coactivity direction leaves the temporal update law open. Reinforcing joint activity alone does not specify the direction or duration of transitions in a loop. The eventual rule must distinguish sustaining an assembly, following a sequence and repeatedly traversing a chain; none of those behaviors follows solely from having sparse connections or a bounded firing fraction.

The operational goals require two separately observable outcomes:

1. **Activity behavior:** each CC's active-neuron fraction remains within the permissive bounds, allowing regime-dependent levels and whatever transient allowance is selected.
2. **Semantic dynamics:** the network exhibits the required drift, sequence response or self-sustaining recurrence, rather than satisfying the fraction using unrelated activity.

No particular inhibition, homeostatic rule, synaptic timing mechanism, threshold adaptation or trace is selected by that distinction. Storage for an algorithm is considered only after its business role and algorithm have been agreed.

## Acceptance to make concrete

| Question | Why it is needed |
| --- | --- |
| What counts as active within each CC, over what time window, with what broad bounds and transient allowance? | Makes the local steady-activity target measurable while allowing different mental states and separating current activation from prediction. |
| How is semantic valence recognized, and what precisely is the small negative advantage? | Prevents coordinate signs from substituting for meaning and defines the intended DMN drift. |
| What sequence behavior demonstrates task flow? | Determines whether token patterns are followed, retained, completed or transformed. |
| What constitutes recurrence of a semantic pattern, and how is loop length measured? | Makes immersion distinguishable from a fixed point, unrelated noise or an unbounded trajectory. |
| How do inputs and dynamics change across regimes? | Defines transitions, persistence of noise and behavior after task input ends without assuming a controller design. |

No experiments or acceptance thresholds have been selected. [Model](Model.md#snn-03) tracks encoding and state semantics; [synaptic integration research](SynapticIntegrationResearch.md) tracks activity-control candidates; [semantic cortical mapping research](SemanticCorticalMappingResearch.md) keeps anatomical correspondence separate from the operational goals.
