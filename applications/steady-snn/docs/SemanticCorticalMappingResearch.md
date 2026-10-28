# Steady recurrent SNN: semantic representations and cortical topology

Document-set revision: 32 · Recorded: 2026-10-27 · Topics: SNN-01/SNN-03/SNN-05/SNN-07.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 32, all sections; `pending-human`, a research assessment under confirmed local constraints, without a qualified implementation contract.
- **Value lineage:** candidate C-16/P-32 examines anatomical compatibility so that physical modeling assumptions do not silently distort C-12's assigned semantic roles or recurrence. P-25/P-22 connect the document set and application to C-01 and V-01/V-02. P-33 depends on C-14's encoding/readout boundaries; P-30 retains connectivity's bounded-storage dependency. No dependency grants authority.
- **Human baseline / continuity:** H-43 / AD-037 specifies CC as the semantic presentation unit, approximate replicas across CCs and statically assigned semantic roles. H-44 / AD-038 prioritizes resolving conflicts between semantic-assigned positions and anatomical topology ahead of choosing 1D/2D. H-41 dimension neutrality and H-42 / AD-035 rejection of internal-position bias remain. H-46–H-48 clarify distributed features and broad per-CC activity bounds for three mental regimes. H-50 clarifies that semantic positions are already assigned and no natural physical positions exist; H-51 requests at least 12 hours of deeper research after correction. [K-37](../../../design/DesignAuthorityRegistry.md#k-37) withdraws the premature application locality recommendation. No re-encoding, interleaving, morphology model or placement rule is adopted.
- **Invariants / non-goals:** retain current Flash 5120 × (60+60) planning, regional relocation, static SoA capacity, weight-scale ownership by training and requirements before auxiliary structures. Current per-dimension signed input/output remains the design baseline. No anatomical equivalence, universal CC definition, lossless 2D projection, extra dense matrix or implementation is asserted.
- **Acceptance:** identify species, spatial scale, measurement and inferential limits; separate semantic role, soma position, neurite contact opportunity and storage order; make any impact on readout, dimension neutrality and the meaning of minicolumn explicit. A candidate must state which conflict it removes and which assumptions it changes. A future algorithm needs a task, resource calculation and observable acceptance, beyond a visually plausible map.

## Finding and scope

**The reviewed literature does not provide a measured mapping from a general semantic embedding to a resolved 2D arrangement inside one anatomically identified human cortical column.** The closest evidence concerns column-scale human functional modules, semantic responses of individual cells, and maps of feature preferences spanning larger visual or language areas. Computational models offer mechanisms to study, not a ready-made biological specification.

The application CC is a region containing a complete current semantic state and is the basic presentation unit under H-43. CCs are approximate replicas, rather than an assumed division of semantic dimensions among specialized CCs. Their exact correspondence, learned histories and cooperation protocol are not specified by that premise. Neither a biological macrocolumn, hypercolumn, functional module nor an imaging patch is automatically equivalent to this application unit.

H-46 clarifies that biological features need not correspond one-to-one with the model's minicolumns. A semantic pattern can span many coordinate populations, and a population can participate in many features. The absence of a biological feature/minicolumn bijection therefore does not itself conflict with the fixed-coordinate model. Mixed feature responses alone do not establish a need to scatter coordinate populations or learn replacement semantic roles.

H-44's anatomical question concerns additional spatial assumptions: for example, compact coordinate parcels combined with a simple distance prior. The conditional analysis below assesses those assumptions separately from feature coding. The operational target is the [three activation regimes](ActivationRegimes.md), with broad per-CC activity bounds and changing semantic trajectories, rather than precise single-coordinate activation.

A 2D sheet is a spatial abstraction. An actual column extends through cortical depth, so a proposed sheet must state whether it represents one layer, a surface projection or an aggregation across layers. Maps across a cortical area do not automatically describe the interior of one column.

## Fixed semantic assignments and learned behavior

H-43 supplies a modeling premise, not a requirement to prove that biology uses this exact code. Minicolumn/neuron semantic response roles are assigned statically. Learning operates within that representation; discovering a new semantic vocabulary or changing which semantic coordinate a cell represents is outside the current baseline.

| Quantity | Current status |
| --- | --- |
| CC role | Basic semantic presentation unit; approximate replicas need not have identical current states or learned graphs. |
| Minicolumn/cell semantic role | Assigned by the model; current planning uses a coordinate group with positive/negative populations. Exact numerical response and readout formulas still need discussion. |
| Connections and weights | Subject to the coactivity-learning direction, static capacity constraints and an eventual selected update rule. |
| Instantaneous phase/activity and contextual selection | Dynamic quantities whose transition semantics remain open; they do not redefine assigned semantic identity. |
| Physical or logical positions | No accepted geometry. Fixed roles do not automatically determine positions, or vice versa. |

“Response properties across semantic space” can describe an assigned function without implying it was learned by this network. An observed biological response map likewise does not, by itself, establish how that map developed. The static-role premise does not freeze neural activity or synaptic learning, and it does not settle all numerical response parameters.

### What HTM contributes to this distinction

[Hawkins and Ahmad, 2016](https://www.frontiersin.org/journals/neural-circuits/articles/10.3389/fncir.2016.00023/full) separates shared feedforward recognition within a minicolumn from learned temporal-context recognition through basal connections. This supports keeping assigned input meaning distinct from learned associations; it does not specify our signed embedding code.

[Hawkins, Ahmad and Cui, 2017](https://www.frontiersin.org/journals/neural-circuits/articles/10.3389/fncir.2017.00081/full) models whole objects within columns and cooperation across columns. Its simulations supply feature and location codes as sparse binary inputs; learning associates features with locations and objects. Thus supplied input codes and associative learning can coexist. This is a theoretical model, not anatomical proof of an LLM-coordinate layout.

The [Thousand Brains Project white paper](https://arxiv.org/html/2412.18354v1) explicitly uses repeated, semi-independent learning modules that can model whole objects. That is a closer architectural analogy to approximate replicas than a map dividing concepts among specialized cortical areas. Its represented object/reference-frame coordinates are not the physical coordinates of neurons on a cortical sheet. Neither theory determines our internal 1D/2D arrangement or requires identical weights across CCs.

## Evidence by measurement and scale

### Closest human column-scale evidence

[Chapeton et al., 2022, *Micro-scale functional modules in the human temporal lobe*](https://www.nature.com/articles/s41467-022-34018-w.pdf) studied eight participants with microelectrode arrays in anterior temporal cortex. They identified persistent, compact functional modules about 1.3 mm across. Cells within a module shared more image-category information than cells across modules, without being fully redundant. Module boundaries were comparatively sharp. Connectivity was inferred from field potentials, not reconstructed synapses; the authors state that recordings across layers are needed to establish columnarity. This is evidence for local functional grouping, not a continuous semantic-coordinate chart inside a confirmed CC.

### Semantic tuning and physical proximity

| Study | Observation | Scale and limit |
| --- | --- | --- |
| [Jamali et al., 2024, *Semantic encoding during language comprehension at single-cell resolution*](https://www.nature.com/articles/s41586-024-07643-2) | A population analysis fits 300-dimensional word embeddings to 133 neurons' responses, linking tuning to semantic structure and context. | Human single cells; the fitted weights and low-dimensional plots describe response relationships. They do not resolve a physical 2D map within a CC. |
| [Mickiewicz et al., 2025, *A semantotopic map in human hippocampus*](https://pubmed.ncbi.nlm.nih.gov/41279861/) | Dense recordings along one axis in three anesthetized patients associate recording-site separation with semantic-tuning differences, with additional shorter-scale structure. | **Preprint** in the sources checked. Hippocampus, essentially an axial sample, and individual-specific effects: neither neocortical column anatomy nor a 2D CC map. This is a closer semantic/physical-distance observation, with those limitations. |
| [Bao et al., 2020, *A map of object space in primate inferotemporal cortex*](https://www.nature.com/articles/s41586-020-2350-5) | Macaque neurons project visual objects onto preferred feature-space axes; their anatomical organization relates to components of those preferences across repeated stages. | A broad visual-cortex organization supported by imaging, stimulation and recordings. It maps neuronal tuning preferences rather than assigning raw vector axes to neighboring minicolumns; it is not human language or one CC. |
| [Yao et al., 2023, *High-dimensional topographic organization of visual features in the primate temporal lobe*](https://www.nature.com/articles/s41467-023-41584-0) | A 25-dimensional visual-feature model supports maps of feature preference in human and monkey temporal cortex, including gradients at different spatial scales. | Primarily regional fMRI, with additional monkey electrophysiology. Physical 2D organization can coexist with multidimensional feature descriptions; this does not resolve the internal organization of one human CC. |
| [Hübener et al., 1997, *Spatial relationships among three columnar systems in cat area 17*](https://pubmed.ncbi.nlm.nih.gov/9364073/) | Orientation, ocular-dominance and spatial-frequency maps have coordinated spatial relationships. | Local visual map organization in cats. Multiple functional variables can coexist on a cortical sheet, but these specific sensory variables do not supply an arrangement for thousands of arbitrary semantic coordinates. |

These studies describe relations between response properties and physical location. Under H-43 they can inform the organization of assigned properties, but cannot establish a requirement to learn those properties. Regional specialization does not demonstrate that each local unit contains a comparable semantic repertoire; that distinction is essential for the approximate-replica premise.

### Computational models outside the current semantic-assignment baseline

| Model | Mechanism | Relevance and limit |
| --- | --- | --- |
| [Doshi and Konkle, 2023, *Cortical topographic motifs emerge in a self-organized map of object space*](https://konklab.fas.harvard.edu/Papers/Doshi_2023_SciAdvances.pdf) | A 20 × 20 self-organizing map learns 4,096-dimensional tuning vectors from pretrained visual-network representations, producing several cortical-like motifs. | A concrete example of arranging high-dimensional preference vectors on a 2D sheet. Some cortical selectivities are not reproduced. It models visual population organization, not a measured human CC or lossless semantic-vector storage. |
| [Rathi et al., ICLR 2025, *TopoLM*](https://arxiv.org/html/2410.11516v3) | Task learning plus spatial smoothness produces language-related functional clusters among units placed on 2D grids. | Demonstrates a language-model route to learned organization. Full connectivity and a spatial objective differ from our sparse coactivity rule; input embeddings alone are not its learned contextual representations. |
| [Hyvärinen, Hoyer and Inki, 2001, *Topographic independent component analysis*](https://pubmed.ncbi.nlm.nih.gov/11440596/) | Residual statistical dependencies among components supply an ordering principle, rather than treating component numbers as distances. | A useful theoretical distinction between coordinate labels and data-dependent organization. The model does not establish semantic anatomy, and its dependence assumptions cannot simply be assigned to the current embedding. |

SOM and TopoLM learn feature preferences or internal representations. Importing that mechanism to redefine minicolumn semantics would change the current premise. They remain comparisons for topographic organization, not active recommendations to learn our semantic assignments. Using a frozen assignment and optimizing only its placement would be a different algorithm requiring its own justification; it is not selected here.

The [H01 human temporal-cortex reconstruction](https://h01-release.storage.googleapis.com/landing.html) supplies detailed anatomy and connectivity in a small tissue volume. Its anatomical data do not themselves attach measured semantic response functions to the reconstructed cells. A connectome and a semantic activity dataset would need a justified alignment; neither alone closes the evidence gap.

The relevant Chapeton and Doshi paper sections and TopoLM methods were inspected in full-text sources. The other observations above were checked against primary publisher/author abstracts, indexed methods or the original study text available in this research session and the preceding discussion. The hippocampal finding is retained as a preprint, not represented as peer-reviewed validation. No large dataset or model weights were downloaded.

## A 2D sheet is not a two-number semantic code

Three different objects are involved:

```text
x                 : the semantic state vector in D dimensions
w_i / f_i         : neuron i's feature preference / response function
p_i = (u_i, v_i)  : neuron i's location on a sheet
```

An illustrative response could depend on a projection `w_i dot x`, or more generally on `f_i(x, context, prior_state)`. The location has two coordinates; the tuning vector may still have D components. An activity pattern across N positions has N scalar values before any further constraints. Putting those positions in a plane does not reduce the whole pattern to two scalar degrees of freedom.

Conversely, assigning each complete token vector to a single winning point is a form of quantization; assigning it only two projection coordinates generally loses information. Neither scheme automatically provides the required continuous signed-vector readout or recurrent semantics.

Under H-43, the semantic role represented by `w_i / f_i` is assigned. The notation does not require a learned feature projection, a dense matrix or complete specification of dynamic response curves. A 2D placement could be fixed independently; its usefulness would still need an explicit relation between position and the assigned roles.

## Why the current raw-axis mapping supplies no spatial order

The abstract coordinate directions `e_d` are equally separated under the Euclidean metric on normalized basis vectors:

```text
norm(e_d - e_k)^2 = 2,  d != k
```

Consequently the fixed input assignment alone provides no preferred cross-dimension neighborhood. Orthogonal coordinate axes need not have statistically independent values, but a correlation-derived neighborhood would be an additional prior, not a consequence of coordinate identity. H-41's dimension-neutrality constraint remains: assigning meanings statically does not authorize giving some dimension pairs a higher connection prior.

This geometry does not say that a token activates only one dimension. For an embedding matrix `E`, a token vector is a row `E[token, :]`; a coordinate's response across tokens is a column `E[:, d]`; the basis vector `e_d` merely labels an axis. Dense token rows, correlated response columns and orthogonal coordinate axes can coexist. Input coefficients also do not directly specify which SNN cells count as active: the encoding and phase rules still need definition.

The [actual Flash-1017 table](TokenEmbeddingStatistics.md) confirms that distinction: all 127,997 ordinary types have 5,120 nonzero coordinates, and the median token needs 2,278 coordinates for 90% of its squared magnitude. Some coordinate pairs are correlated across the vocabulary, and some coordinates have strong sign biases. These facts support distributed input patterns; they neither select a spatial prior nor establish a correspondence to anatomy.

Token similarity and coordinate similarity are also different objects. Two token vectors can be close in the imported embedding without defining which of its 5,120 coordinate groups should be physically adjacent. A fixed semantic assignment therefore resolves who represents what, while leaving the justification for distance unresolved.

Capacity-set migration remains a storage operation: it cannot change a cell's semantic role, identity or any eventual logical position.

## Which biological analogy is useful under the fixed-role premise?

The most relevant comparison is **repeated local coverage of a representational repertoire**, together with learning among its assigned roles. A map of mutually specialized distant regions is a weaker match. Whole-object HTM columns provide an architectural hypothesis; they do not establish a particular physical map inside one CC.

Visual cortical feature maps offer a more specific but limited analogy. Hübener et al. measured coordinated orientation, eye-dominance and spatial-frequency maps, and suggested that their arrangement helps provide local coverage of feature combinations. Our inference is that topology can organize coverage of defined features without making discovery of their identities the application task. This does not prove that biological maps are innate or static. Orientation and visual position also have meaningful sensory neighborhoods that arbitrary embedding-coordinate labels lack. A visual hypercolumn is not thereby a measured implementation of this application's CC.

Chapeton's local human modules and the single-cell semantic studies constrain plausible organization, but neither demonstrates repeated copies of a complete semantic coordinate system. The evidence gap remains even after semantic roles are supplied by design.

## Anatomical compatibility of assigned semantic populations

### Direct evidence that changes the modeling question

The following are primary experimental studies. Their results constrain particular species, areas and measurements; none supplies a complete physical map for a human CC carrying a general language embedding.

| Study | Evidence relevant to the conflict | Modeling implication and limit |
| --- | --- | --- |
| [Chang and Tsao, 2017, *The Code for Facial Identity in the Primate Brain*](https://authors.library.caltech.edu/records/znzhp-4j547) | Macaque face-cell responses were modeled by projection onto preferred axes in a 50-dimensional face space, supporting population decoding. | Coordinate-like response roles have an experimental analogue. Preferred axes are not necessarily the supplied basis axes or mutually orthogonal; this does not demonstrate one compact minicolumn per axis, static developmental assignment or a human language code. |
| [Ohki et al., 2005](https://www.med.harvard.edu/bss/neuro/bornlab/nb204/papers/OhkiReidNature2005.pdf) | Cellular imaging found neighboring rat V1 neurons with different orientation preferences, while cat area 18 exhibited sharply organized direction domains, with borders one to two cells wide. | There is no universal rule that physically adjacent cortical neurons must have similar tuning. Conversely, genuine compact functional domains cannot be dismissed. Rat mixing does not establish the arrangement of human association cortex. |
| [Kondo, Yoshida and Ohki, 2016](https://www.nature.com/articles/ncomms13210) | Mouse V1 contained weak orientation clustering in some short vertical minicolumn segments, roughly 60 µm in a 100 µm observation depth; other minicolumns were heterogeneous. Spatial-frequency preference was intermingled, while retinotopic clustering occurred at a larger scale. | Anatomical minicolumn membership does not imply one homogeneous functional identity for every response variable. Pure random mixing is also too strong a universal claim. The measured organization depends on the variable and spatial scale. |
| [Ko et al., 2011](https://www.nature.com/articles/nature09880) | Nearby mouse V1 layer 2/3 pyramidal neurons with similar visual responses were more likely to connect; matching orientation preference was associated with approximately twice the connection rate of orthogonal preferences. | Being nearby does not determine the learned graph. The result is an association between function and connectivity, not a measured Hebbian update rule. |
| [Ding et al., 2025, MICrONS](https://www.nature.com/articles/s41586-025-08840-3) | Matched mouse visual-cortex function and EM anatomy allowed controls for axon–dendrite proximity. Feature similarity predicted fine synaptic connectivity beyond anatomical opportunity; receptive-field location similarity did not add the same residual prediction. | Separate contact opportunity from functional selection. The analysis used 148 proofread presynaptic cells and 4,811 functional postsynaptic partners; it spans layers/areas, not one human CC. The study's 5 µm proximity criterion concerns axons and dendrites, not a soma-distance cutoff or exclusion zone. |
| [Schneider-Mizell et al., 2025](https://www.nature.com/articles/s41586-024-07780-8) | An EM census across mouse visual-cortex layers found inhibitory targeting of particular, spatially intermingled cell populations and compartments. | Layer, cell type, projection and compartment can matter alongside location. One isotropic distance kernel is a simplification, not a full account of anatomical connectivity. This evidence does not select new cell classes or fields for our model. |
| [Kremkow et al., 2016](https://www.nature.com/articles/nature17936) | In cat V1, organization of ON/OFF thalamic afferents and retinotopy was related to orientation, direction and disparity maps. | A functional map may be anchored by structured input pathways, not just by semantic similarity. ON/OFF visual contrast is not equivalent to an arbitrary embedding coordinate's sign, and neither is equivalent to excitatory/inhibitory cell identity. |

The relevant results and methods of Kondo and Ding, the Ohki and Chang papers, and the targeting analysis of Schneider-Mizell were inspected in primary full-text sources. Ko and Kremkow's claims here are limited to their primary abstracts. These anatomical conclusions are literature assessments, not measurements from a downloaded connectome.

### Four objects that must remain distinct

```text
semantic role        which coordinate/polarity a cell represents
anatomical position  where its soma is, including cortical depth
contact opportunity  where its axon and another cell's dendrites can meet
storage order        where the simulator keeps its state and connection slots
```

The assigned semantic role is fixed under H-43. H-50 clarifies that this semantic arrangement is already our starting organization; there is no natural physical arrangement to measure or complete. Contiguous IDs or SoA array entries do not establish anatomical adjacency. Cell bodies need not be near each other for their neurites to meet, and nearby cell bodies do not ensure a connection. Capacity migration must preserve semantic identity and any adopted physical model.

Our inference from the studies is to distinguish anatomical opportunity from activity-dependent selection among opportunities. This is a conceptual distinction, not an adopted factorized probability law or a request to store neurite geometry. Anatomy and function can themselves be correlated; treating them as statistically independent would be another unproven assumption.

The earlier H01 discussion concerns an anatomical dataset lacking matched semantic responses. MICrONS provides a joint function/anatomy example for mouse vision, closing that methodological gap in a different system; it does not close the human semantic-CC gap.

### Conditional conflicts if anatomical assumptions are added

| Assumption or combination | Assessment |
| --- | --- |
| A cell/population has an assigned coordinate response role | Compatible with coordinate-like coding as an engineering abstraction. Biology does not establish our particular basis, population count or static assignment mechanism. |
| Measured semantic features and coordinate minicolumns are not in one-to-one correspondence | Compatible with the model under H-46. Distributed patterns can span minicolumns; heterogeneity of a measured feature alone does not refute coordinate grouping. |
| All 120 cells for a coordinate must occupy one compact anatomical column | An additional spatial interpretation, not established by the coordinate assignment. Some functional domains are compact, but anatomical minicolumns are not universally homogeneous. |
| Compact coordinate parcels + distance-dependent partner preference + exactly equal prior probability for every other coordinate | These cannot generally be guaranteed together. Parcel placement introduces unequal access to different dimensions, even if labels were randomly assigned to parcels. |
| A universal scalar soma-distance rule explains connectivity | Omits neurite overlap, layers, types, compartments and functional selection. It may be a useful approximation only after its intended scope is justified. |
| Positive/negative populations directly correspond to biological excitatory/inhibitory populations or ON/OFF maps | Unsupported identification. Signed readout, visual contrast polarity and synaptic effect are separate meanings. |
| The tiny very-near exclusion is an anatomical universal | Not established by these studies. H-40 retains it as a modeling direction; its biological or computational justification remains separate. |

The compact-parcel conflict can be made explicit without a biological assumption. For a source cell `i`, suppose a candidate is proposed using only a distance kernel over eligible cells:

```text
Z_i(d) = sum over eligible cells j in semantic group d of K(distance(i,j))
P(target group = d | i) = Z_i(d) / sum over eligible groups k of Z_i(k)
```

Equal population sizes do not make the `Z_i(d)` equal. In the illustrative limit where each compact group is a point and the kernel strictly decreases over all relevant distances, equal pairwise group preferences require equal pairwise distances. Euclidean space of dimension `q` admits at most `q+1` distinct mutually equidistant points. This is a conditional mathematical obstruction, not a claim about every extended arbor or every finite population arrangement. It explains why replacing a line by a sheet cannot by itself remove the conflict.

An exclusion zone adds another source of unequal eligibility; it does not automatically restore dimension neutrality. Uniform proposal across dimensions also differs from equal retained degree after learning, which has never been required by the current model.

### Conditional anatomical interpretations

These comparisons concern extra anatomical assumptions, not a current design choice or a defect in the existing semantic arrangement. H-50 supplies no physical metric, and none of these interpretations is the default next step. They remain conditional reference material within C-16, without an implementation contract.

| Alternative | What it preserves | What changes or remains unresolved |
| --- | --- | --- |
| Keep semantic groups logical and assign their cells to spatially interleaved locations | Fixed coordinate/polarity roles, 60+60 population and signed readout ownership; physical locality can act on actual positions. | Relaxes identification of one semantic group with one compact anatomical minicolumn. Salt-and-pepper evidence makes this plausible in some circuits, not validated for human semantic cortex. Random interleaving gives label symmetry in expectation, not exact dimension neutrality for every cell in one finite layout. |
| Retain compact semantic groups and study selective axonal/dendritic reach | Compact membership and fixed semantic roles. | Replaces the idea that soma separation alone determines opportunity. Cross-group access, direction/layer effects and preservation of the dimension-neutral prior need explicit justification; drawing long-range arbors does not solve these automatically. |
| Represent effective connection opportunity without assigning explicit physical positions yet | Fixed semantics and learning can be discussed without declaring an unsupported physical embedding. | This is a functional abstraction. It leaves the anatomical embedding unresolved and cannot claim to have reproduced cortical spatial topology. |

No alternative is required merely because measured features and minicolumns are non-bijective. Interleaving would need an explicit decision about the meaning of our term `minicolumn` and a demonstrated benefit for the intended dynamics; the name alone cannot approve scattering its members. This candidate does not use matching internal indices or shared within-minicolumn coordinates to favor cross-minicolumn connections. Independent placement is also not the same as independent semantic learning.

Under randomized placement, the expectation is over layouts. A specific CC can still have persistent dimension biases and missing locally accessible partners, especially under a short-range cutoff. Different layouts across approximate-replica CCs would not guarantee that those biases cancel in network behavior. Exact per-cell group neutrality would require another mechanism or constraint, which would need its own analysis and could alter the global distance preference. No such mechanism is selected.

H-41 establishes that dimension labels must not imply proximity. Equal cross-minicolumn opportunity was a proposed way to express that aim; the record does not select whether equality applies to proposals or retained edges. Exact per-source proposal equality is used here as a strong comparison condition, not a newly confirmed requirement. Whether symmetry only over layouts meets the intended neutrality remains an open modeling question.

None of these alternatives changes the existing static capacity arithmetic or adds a position array, cell-type field, morphology structure, index or auxiliary budget. Those costs belong to a later selected algorithm.

## Research priorities and acceptance boundaries

1. Keep assigned semantic roles fixed and distinguish coordinate populations from distributed semantic features. Evaluate topology against the three requested regimes and their broad per-CC activity envelope; non-bijective feature tuning alone is not a conflict to repair.
2. Check dimension neutrality at the relevant level: label symmetry over layouts, per-source proposal probabilities, and learned connectivity are different properties. H-41 does not select a complete probability contract. Make any residual spatial bias explicit instead of treating expected symmetry as exact equality.
3. Identify computational benefits attributed to biological locality and inspect whether they require material anatomy or follow from graph/learning properties. Establish relevance within the existing semantic organization before proposing a metric, layout or contact representation. The conditional `Z_i(d)` calculation diagnoses a possible added assumption; it does not justify adding it.
4. Preserve signed readout, contextual discrimination, recurrence, static capacity and the rejected shared-position boundary. Any detailed mapping, acceptance thresholds or experiment remains a separate decision. The research need not conclude that geometry should be introduced.

For direct biological confirmation, the missing combination is semantic stimulation/encoding, dense physical localization, within-module measurements and sufficient laminar information to identify the column. Evidence distributed across separate datasets cannot be treated as if all four were measured in one preparation.

The current conclusion is that **fixed coordinate assignment and distributed semantic features are compatible**. The literature does not establish the application's physical layout, and any additional identification of semantic grouping, anatomical compactness and distance-based opportunity needs its own justification. Intermingling and selective reach remain alternatives for a specific spatial problem, not compulsory repairs for feature heterogeneity. Geometry should serve the requested activity and semantic-trajectory behavior; no 1D/2D layout or change of semantic roles is selected.

[Physical distance preference research](PhysicalDistancePreferenceResearch.md) retains the biological evidence while making the H-50 boundary explicit: no current physical metric exists. The research asks what biological locality contributes computationally and whether that contribution is useful within our semantic organization. Neither Flash coefficient correlations nor graph adjacency automatically supplies a physical placement, and no positions-versus-contact-graph decision is currently required.
