# SNN cognitive model design progress

Revision 2 · 2026-11-05 · Discussion and research synthesis

[中文版](CognitiveModelProgress.zh-CN.md)

The working premise is that each cortical column (CC) carries an approximate replica of the learned conceptual dynamics. Concepts acquire their content through experience and prediction; token vectors provide a shared interface for accessing and expressing that content. The main unresolved question is how a finite-precision rate model can support abstraction and coordinated reasoning while retaining the useful effects of spike timing.

The rectangle scenario connects these questions to perception, bodily orientation, and self-observation. The four-aspect account in the *Cheng Weishi Lun* supplies a conceptual framework for this investigation. It is not evidence that the network has consciousness or that the proposed CC groups are biological counterparts of the four aspects.

## Design authority

- **Registry and coverage:** [D-09](../../../design/DesignAuthorityRegistry.md#d-09), revision 2, both language versions in full; `pending-human`. Shared section, question, and reference IDs support comparison; neither language has independent design authority.
- **Value lineage:** C-30–C-33 and P-71–P-81 connect this investigation to C-12 and C-24, then through the existing candidate application paths to C-01/V-01/V-02. The value is to make conceptual learning, capacity limits, and temporal cooperation explicit enough to evaluate. The new formal relationships remain pending confirmation.
- **Human baseline:** [H-87–H-92](../../../CHANGELOG.md#h-87) record the user's scenario, modeling premises, corrections, research request, and authorization to produce this tracker. H-43's fixed coordinate roles and H-75's contextual cell identities remain relevant. These statements do not approve the complete document; the additional scoped confirmations are listed below.
- **Constraints and non-goals:** preserve individual contextual cells, fixed semantic coordinates, one resident CC per region, regional relocation, static connection storage, active-rate cancellation, and external steerability. No new physical-distance prior, inhibitory synapses, persistent state fields, encoder, update rule, or implementation is selected.
- **Acceptance for this revision:** preserve the scoped human premises; separate them from provisional reasoning and published evidence; give each unresolved question an observable comparison or decision requirement. Section S8 tracks these obligations.
- **Confirmed local scope:** H-93/H-94 establish continuous source dynamics and two-frame incoming pull; H-95 states the capability-preservation objective. C-29/P-69/P-70 and C-34/P-82/P-83 link these constraints to temporal modeling. They do not settle equations, numerical parameters or state layout. [K-209](../../../CHANGELOG.md#k-209) records this revision; S5.1 and S8 distinguish the confirmed boundaries from open choices.
- **Initial assessment:** [K-208](../../../CHANGELOG.md#k-208) registers new research scope. It does not continue a previously qualified cognitive-model contract. D-08's technical baseline remains pending-human.

<a id="s1"></a>
## S1 Current premises and conclusions

| ID | Proposition | Status |
| --- | --- | --- |
| CM-01 | Assume the multi-CC network can serve as the complete carrier of the four aspects; investigate the rectangle scenario under that assumption. | User's exploratory premise, H-87; not an established result. |
| CM-02 | A concept's experiential and predictive content must be learned by the SNN and expressed in its connection-supported activation dynamics. Neither token identity nor imported LLM weights directly supplies that content. | User's modeling premise, H-88. |
| CM-03 | Every CC contains the full conceptual attractor repertoire, approximately rather than identically. Sensory CCs favor patterns associated with their own bodily or visual-field location. | User's modeling premise, H-89; acquisition and coverage remain open. |
| CM-04 | Evaluate 5,120 dimensions together with sparsity, usable range, practical resolution, and temporal reuse. | User's research direction, H-89/H-90; sufficiency remains open. |
| CM-05 | Each neuron has a finite-precision firing rate. Binary firing/silent states are not the model. Rate precision must be considered together with the cooperative effects of spike timing. | User's correction and requirement, H-91. |
| CM-06 | Capacity concerns stored repertoire, current expression, and coordinated reasoning separately. A semantic readout need not expose the full contextual state. | Provisional analytical conclusion. |
| CM-07 | Identical average rates can conceal different causal timing relationships. More rate bits cannot recover discarded timing. | Analytical conclusion; relevant biological evidence appears in S7. |
| CM-08 | A rate model retaining selected temporal structure is worth comparing with a simpler rate baseline. | Research candidate; no filter, delay, phase, trace, or learning rule selected. |

## S2 The rectangle as a shared cognitive event
<a id="s2"></a>

The CC groups below describe roles in the current event. Under CM-03, they retain a common conceptual repertoire rather than owning disjoint concept libraries.

| Aspect | Assigned inputs and role | Example content |
| --- | --- | --- |
| 相分 — presented content | CCs assigned to positions in a two-dimensional visual field represent local pattern elements, rather than raw retinal color pixels. | At the lower-left: a corner pointing lower-left, with two extending sides. Elsewhere: blank space, horizontal or vertical edges, and other corners. |
| 见分 — apprehension | Higher visual CCs integrate the field patterns into an object description. | A medium-sized horizontal rectangle near the center of the visual field. |
| 自证分 — embodied self-registration | Spatial CCs combine the visual result with bodily and muscular input, including head and eye orientation; binocular input could add distance. | Body seated upright; head turned left 45 degrees; gaze level; a horizontal rectangle. |
| 证自证分 — reflective awareness | Further CCs receive the preceding information and represent the ongoing observation and inquiry. | I am looking at a rectangular frame to my front-left. Its edges look straight and its corners sharp. I am curious about its material. |

Feedback from reflective activity to the embodied group can support the description “subject: me; activity: observing a rectangle with curiosity.” A proposed functional criterion is that this description both influences subsequent observation and can be corrected by what the system actually does and receives. Mutual agreement alone would not establish that relationship.

The scene needs explicit reference frames. A corner's visual-field location, its position within an object, and its orientation are different attributes. “Gaze level” does not specify eye yaw; head yaw alone therefore does not determine the object's body-relative bearing. The spatial transformation and attribute binding remain learning problems.

The philosophical account concerns interrelated aspects of cognition, not a mandatory four-stage anatomical pipeline [R01]. Our assignment is an application hypothesis. A verbal self-description and subjective experience are also distinct claims.

## S3 Concepts are learned predictive organization
<a id="s3"></a>

For this design, assume that the static token table supplies essentially none of the experiential model that constitutes a concept. This is a modeling boundary, not a claim that pretrained embeddings contain no statistical associations. A token can become an access cue; it does not define what a rectangle will imply in an unfamiliar situation.

The learned content includes expectations about edge continuation, corners, viewpoint changes, and the observations or actions that distinguish an open frame from a solid panel. Shape alone does not settle material or solidity. LLM descriptions may supply teaching material, but the SNN must acquire the relevant dynamics itself. Importing a full LLM is not a direct transfer of those dynamics into this architecture.

The proposed unit of conceptual organization is a family of recurrent activation paths and their input-dependent transitions. Persistent activity alone is insufficient: the model must also account for how a context starts a path, changes its continuation, and ends or redirects it. Different CCs should instantiate approximately corresponding families despite differences in weights, experience, and current input.

Abstraction would let experience with some rectangles improve prediction about others through shared organization. The present state could then carry location, orientation, and task-relevant exceptions, while learned connections supply familiar relationships. Memorizing separate episodes under the same token does not by itself demonstrate this generalization.

Fixed semantic coordinate assignments remain compatible with learning these dynamics. A new per-concept coordinate system, physical geometry inside a CC, or hard partition of concepts among CCs is not implied. Preserving the full repertoire in CCs exposed to different sensory streams is an explicit training obligation, not a consequence of sharing a token table.

## S4 Capacity under sparse graded activity
<a id="s4"></a>

Distinguish the **stored repertoire**, the **precision of current expression**, and the **number of relations that remain usable during coordinated reasoning**. The 5,120-dimensional vector is a candidate common semantic representation; it is not a measured bound on the whole network's cognition.

In the current 5,120 × (60 + 60) sizing case, one participating cell per coordinate uses 5,120 of 614,400 neurons, about 0.83%. This counting example can leave every coordinate nonzero. It proves neither adequate precision nor stable propagation, but shows why neuronal sparsity cannot be converted directly into a percentage of unavailable semantic dimensions.

Activity support size, rate magnitude, and sparsity of the semantic readout are separate observables. Cell identity and graded rate both matter. Sparse codes can support many possible patterns while simultaneous expression remains constrained by interference and reliable discrimination [R05]. Energy arguments motivate examining activity budgets, but provide no universal active-fraction threshold for this model [R04].

For coordinate i, a rough uniformly spaced level count is `Q_i ≈ 1 + R_i / δ_i`, where R_i is usable range width and δ_i is the separation needed for a specified downstream discrimination. Practical δ_i depends on arithmetic, noise, cancellation, recurrence, and observation time. It need not equal one neuron's numerical step size. Correlated coordinates and activity constraints prevent treating all nominal levels as independently available information.

“7 ± 2” motivates a capacity question, not a chosen item limit. Miller distinguished kinds of capacity; Cowan examined approximately four chunks under more restrictive conditions [R02]. Population-noise and rate–distortion models offer more direct ways to relate load, precision, and the cost of errors [R03]. No cited study establishes that human working memory exhausts 5,120 semantic dimensions.

Temporal reuse could spread processing across successive activations while weak activity or another justified state preserves context. It must preserve object identity, relations, and reasoning progress between visits. More time is beneficial only when the dynamics retain or accumulate useful information. Oscillatory-subcycle models and intermittent working-memory recordings provide references, not a selected clock or retention mechanism [R06].

With neural state s, readout x = D(s), and transition s' = F_W(s, u), equal x can accompany different contextual cells and different futures. Thus, “5k is sufficient as an interface” is weaker than “5k completely specifies the predictive state.” Multiple CCs add state and interaction capacity; a global reasoning bottleneck would need its own explanation. Approximate replicas can also share errors, so their count is not a precision multiplier [R07].

## S5 Temporal cooperation in a rate model
<a id="s5"></a>

Each neuron carries a value from a finite set of rates. Activity/inactivity is a derived classification, not the full state. The design question is which effects of individual spike timing must remain visible in the graded dynamics.

An illustrative counterexample uses two inputs, each firing once in a 20 ms window. Both window-average rates are 50 Hz. With unit input effects, a 5 ms exponential decay, and threshold 1.5, coincident arrivals reach 2; arrivals 10 ms apart reach about `1 + exp(-2) = 1.14`. Equal mean rates can therefore correspond to different outcomes. The example is analytical and does not select biological parameters or replace the SNN with a spike simulator.

Relevant timing relationships include overlap at the target, input order, transmission delay, recent synaptic use, and the receiver's state. Coincident arrival does not require simultaneous emission. Timing affects both immediate propagation and learning; success at reproducing one does not establish the other [R08–R13].

A useful candidate is causal filtering of rate histories followed by a nonlinear target response. Such a model can retain changes and overlap at its resolved timescale. It cannot distinguish spike patterns already mapped to identical retained rate histories. A phase or correlation summary would also be an approximation with its own missing cases.

For a simplified current-based input, `Var(Σ w_i a_i) = Σ w_i² Var(a_i) + 2Σ_{i<k} w_i w_k Cov(a_i,a_k)`. Individual means do not determine the joint terms, and generally `E[F(I)] ≠ F(E[I])`. This explains one source of loss during averaging; it does not prescribe a covariance matrix or establish that second-order statistics are sufficient.

The choices must be assessed jointly: rate meaning and averaging window, update interval, integration and recovery times, retained history, delays, numerical formats, and learning credit. Rate-model reductions have operating limits, including low-noise regimes with spike locking [R14]. That boundary matters to the intended low-noise flow regime. The confirmed two-frame architecture determines snapshot consistency; it does not establish temporal coincidence or retain arbitrary delays.

### S5.1 Continuous neurons and the confirmed pull architecture
<a id="s5-1"></a>

The objective is to preserve physiological mechanisms that may enable knowledge learning, concept generalization and coordinated computation. Reconstructing a single neuron's physiology in full is not an independent goal (H-95). Complexity needs a reason tied to those capabilities.

H-93 confirms that successive APs arise from one continuously evolving source-neuron state. A global curve can approximate the output during firing without making successive APs independent. The last AP completion time may summarize recovery; any additional history must be justified by the selected mechanism. There is no general requirement to construct separate AP objects, retain their tails in queues or add those tails linearly.

H-94 confirms the execution architecture: each destination traverses its own incoming connections, pulls source effects from immutable frame A, computes its next state into B, and publishes B only when the step is complete. Source and destination are roles on edges, not different cell categories. Reading an effect does not advance or consume source state. Derived and cached effects must describe the same source-time view for every reader. Single-frame and multi-frame AP output share this architecture.

Upstream scatter, outgoing traversal and push queues are not default transmission mechanisms; adopting them would change the confirmed design. Additional conduction delay could, if later selected, be supported by downstream reads of bounded source history. This compatibility observation selects no history layout or extra network frames. The 10 kHz scan, refractory formula, AP duration, curve normalization, neuron equation and learning rules remain open. Implementation must preserve the chosen source dynamics and downstream response without accidentally changing them through timestamp replacement or buffer reuse.

## S6 Candidate mechanisms and their costs
<a id="s6"></a>

| Candidate | Potential contribution | Boundary or cost |
| --- | --- | --- |
| Current graded rate with nonlinear integration | Baseline for assessing which behaviors require more information. | Identical retained states cannot distinguish discarded histories. |
| Causally filtered rates | Sensitivity to recent changes and resolved temporal overlap. | Filter timescales and discretization require validation; sub-window timing remains absent. |
| Selected adaptation or transmission history | Different responses after repeated use or recovery. | Additional state needs a demonstrated role and explicit ownership. |
| Effective delays or phase relationships | Selective cooperation between arrivals or CC groups. | Requires temporal reference and retained information; no per-edge delay field or global oscillator is selected. |
| Arrival-concentration or correlation summaries | Distinguish some equal-mean input conditions. | Summaries can miss ordered or multimodal patterns; per-input-pair storage can be expensive. |
| Timing-sensitive learning traces | Preserve credit for relevant temporal relationships. | Does not follow from naming a rule “Hebbian” or “STDP”; rate, order, and cooperative context must be specified. |

Keep individual contextual cells. Population reductions are methodological references, not authorization to replace each minicolumn with one rate [R15]. Shared per-source history and independent per-connection history have different costs. Selected cooperation must remain compatible with semantic diversity, ordinary fading, and external redirection; global synchronization is not the objective.

## S7 Research evidence and its scope
<a id="s7"></a>

The references below support particular observations or modeling methods. They do not establish our complete architecture or select its parameters.

| ID | Reference | Relevant result and boundary |
| --- | --- | --- |
| R01 | [Cheng Weishi Lun, fascicle 2](https://zh.wikisource.org/zh/成唯識論/02) | Four aspects of cognition, including mutual witnessing of the third and fourth. Does not identify CCs or define a neural implementation. |
| R02 | [Miller 1956](https://labs.la.utexas.edu/gilden/files/2016/04/MagicNumberSeven-Miller1956.pdf); [Cowan 2001](https://memory.psych.missouri.edu/assets/doc/articles/2001/cowan-bbs-2001.pdf) | Chunking and task conditions matter to capacity estimates. Neither determines semantic dimensionality. |
| R03 | [Bays 2014](https://pmc.ncbi.nlm.nih.gov/articles/PMC3942580/); [Sims, Jacobs and Knill 2012](https://pmc.ncbi.nlm.nih.gov/articles/PMC3646905/) | Population noise and rate–distortion offer accounts of memory errors and precision. Their particular noise laws and task costs are not adopted. |
| R04 | [Lennie 2003](https://www.cns.nyu.edu/~pl/pubs/Lennie03a.pdf) | Energy analysis constrains sustained substantial firing under stated assumptions; not a universal sparsity percentage. |
| R05 | [Tsodyks and Feigel'man 1988](https://weizmann.esploro.exlibrisgroup.com/esploro/outputs/journalArticle/The-Enhanced-Storage-Capacity-in-Neural/993263100803596); [Olshausen and Field 1996](https://www.cns.nyu.edu/~tony/vns/readings/olshausen-field-1996.pdf) | Sparse associative storage and learned image coding illustrate different benefits of sparsity. Hopfield assumptions and a learned sensory basis do not automatically transfer to this model. |
| R06 | [Lisman and Idiart 1995](https://www.ucl.ac.uk/brain-sciences/sites/brain_sciences/files/lisman95-oscillatory-memory.pdf); [Lundqvist et al. 2016](https://pmc.ncbi.nlm.nih.gov/articles/PMC5220584/) | A temporal-subcycle model and monkey recordings of intermittent memory-related bursts motivate temporal reuse; neither mandates a rigid seven-slot clock. |
| R07 | [Moreno-Bote et al. 2014](https://www.nature.com/articles/nn.3807) | Specific correlation structures limit population information. More replicas or lower average correlation alone does not guarantee higher precision. |
| R08 | [Usrey, Alonso and Reid 2000](https://pmc.ncbi.nlm.nih.gov/articles/PMC6772311/) | Cat thalamocortical recordings distinguish same-axon and cross-axon cooperation windows, roughly 15 and 7 ms. These are preparation-specific scales. |
| R09 | [Tsodyks and Markram 1997](https://pmc.ncbi.nlm.nih.gov/articles/PMC19580/) | Recordings and modeling connect synaptic depression to the relative transmission of rate and temporal coherence. Short-term efficacy is distinct from long-term weight. |
| R10 | [Branco, Clark and Häusser 2010](https://pmc.ncbi.nlm.nih.gov/articles/PMC6354899/) | Patterned stimulation reveals dendritic sensitivity to input order. The result depends on biophysical mechanisms absent from a flat weighted sum. |
| R11 | [Sjöström, Turrigiano and Nelson 2001](https://plasticity.muhc.mcgill.ca/DataPage/PDFs/Sjostrom_Neuron_2001.pdf); [Pfister and Gerstner 2006](https://pmc.ncbi.nlm.nih.gov/articles/PMC6674434/) | Cortical plasticity jointly depends on rate, timing, and cooperation; triplet models connect timing and rate learning under stated assumptions. No universal plasticity rule follows. |
| R12 | [Womelsdorf et al. 2007](https://pubmed.ncbi.nlm.nih.gov/17569862/); [Zandvakili and Kohn 2015](https://pmc.ncbi.nlm.nih.gov/articles/PMC4545497/) | Phase relationships and coordinated activity are associated with effective communication. V1 coordination did not propagate unchanged through every downstream stage examined. |
| R13 | [Izhikevich 2006](https://www.izhikevich.org/publications/spnet.htm) | A model with delays and plasticity produces time-locked but nonsynchronous groups. Group counts do not establish usable concept capacity here. |
| R14 | [Ostojic and Brunel 2011](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1001056) | Filter/nonlinearity reductions approximate firing-rate responses in specified regimes; weak-noise locking and fuller spike statistics expose limits. |
| R15 | [Schwalger, Deger and Gerstner 2017](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1005507) | Mesoscopic equations retain history and finite-size fluctuations under population assumptions. This is not a license to erase our contextual cell identities. |

## S8 Progress and open decisions
<a id="s8"></a>

| Work item | Current stage | Evidence required to advance |
| --- | --- | --- |
| Four-aspect scenario and replica premise | Human premises recorded. | Specify how a shared cognitive event is tracked across CC roles. |
| Concept content and abstraction | Learning responsibility established; acquisition mechanism open. | Generalization to new situations and distinguishable practical predictions. |
| Capacity | Variables and analytical distinctions identified. | A load–precision–latency comparison with a declared task and error measure. |
| Temporal cooperation | Relevant literature and candidate reductions identified. | Matched-rate comparisons that expose what each retained state can and cannot distinguish. |
| Source continuity and execution | H-93/H-94 confirmed: continuous neuron state and two-frame incoming pull. | Specify equations and justified state within these boundaries; keep all source reads consistent. |
| Training and implementation | No mechanism qualified; no SNN validation results in this tracker. | A scoped design decision, justified state/cost choices, and the applicable execution authorization. |

| Question | Decision or comparison needed | Affected scope |
| --- | --- | --- |
| CQ-01 | Is the 5k vector an interface, or intended to be sufficient for future prediction? Specify the permitted hidden contextual state. | Model; SignedPopulationEncoding. |
| CQ-02 | How are object identity, reference frame, and attributes bound across CCs? Compare pose changes and exchanged attribute assignments. | Cognitive scenario; CC messages. |
| CQ-03 | How does training establish abstraction and preserve the full repertoire in differently driven replicas? Compare held-out combinations and transfer across sensory contexts. | TrainingStrategy; FlowLearning. |
| CQ-04 | What prediction errors and latency are acceptable as load, rate range, resolution, and sparsity vary? | CapacityPlanning; RateDynamics; ContextLoopsAndNumerics. |
| CQ-05 | Which timing distinctions must survive, and over what averaging/update windows? Compare coincidence, dispersion, ordering, and delay-compensated arrivals at equal average rates. | RateDynamics; SynapticIntegrationResearch. |
| CQ-06 | What is the smallest retained history that preserves those distinctions? Compare filtered-rate, adaptation, and coordination candidates; account for state and traversal costs. | RateDynamics; IncomingConnectivity; CapacityPlanning. |
| CQ-07 | Which temporal relationships receive learning credit? Evaluate both immediate response and learning direction, including repeated use and recovery. | FlowLearning; TrainingStrategy. |
| CQ-08 | Does cooperation remain selective and externally steerable under coupled CC operation, including low-noise flow? | ActivationRegimes; CC communication. |

Proposed comparison order: start with small timing-sensitive motifs, then learned sequence distinctions, then the rectangle task, and finally coupled replicas. Preserve identical mean rates where timing is the variable of interest; shift inputs across frame boundaries to expose discretization artifacts. Candidate models may use only information retained by their declared representations, not hidden reference spike times or future labels.

Record quantitative results separately from this synthesis, with units, assumptions, numerical formats, and source conditions. Changes to fields, filters, delays, or learning rules require their own design scope. This tracker does not authorize a bootstrap, build, test run, or simulation.

## S9 Related application contracts
<a id="s9"></a>

[Model](Model.md) defines semantic coordinates and contextual identities; [Rate dynamics](RateDynamics.md) covers graded activity and frame semantics; [Synaptic integration](SynapticIntegrationResearch.md) covers incoming drive and nonlinear alternatives; [Flow learning](FlowLearning.md) and [Training strategy](TrainingStrategy.md) retain their distinct training scopes. [Activation regimes](ActivationRegimes.md), [Capacity planning](CapacityPlanning.md), and [Context loops and numerics](ContextLoopsAndNumerics.md) constrain behavioral and resource comparisons. [Token embedding measurements](TokenEmbeddingStatistics.md) describes the measured input table, not acquired concept knowledge.

The present tracker adds a separately versioned research scope. It does not replace the existing documents, resolve their open mechanisms, or extend the completed H-71 research pass.
