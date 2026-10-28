# Steady recurrent SNN: spatial connectivity research and boundaries

Document-set revision: 32 · Recorded: 2026-10-27 · Topics: SNN-03/SNN-05/SNN-07.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 32, all sections; `pending-human`, general research rather than an implementation contract.
- **Value lineage:** candidate P-29 relates C-15's search for a justified proximity rule to C-12's contextual recurrence; P-25/P-22 connect the documents and application to C-01 and V-01/V-02. P-30 is a capacity dependency. P-31's shared-position scope is rejected under H-42 and grants no authority.
- **Human baseline / continuity:** H-40 supplies coactivity/proximity research; H-41 excludes dimension-index distance; H-42 / AD-035 rejects cross-minicolumn internal-position bias. H-43 / AD-037 fixes semantic assignments and approximate-replica CCs. H-46–H-48 specify three regimes with broad per-CC activity bounds. H-50 clarifies that our semantic arrangement supplies no natural physical positions; H-51 requests at least 12 hours of deeper research after correction. [K-37](../../../design/DesignAuthorityRegistry.md#k-37) withdraws the premature application recommendation. No geometry or learning algorithm is selected.
- **Invariants / non-goals:** preserve Flash 60+60 planning, static SoA capacity, regional relocation, training ownership of weight scale and requirements before auxiliary structures. Storage migration does not change learned semantics. No anatomical equivalence or measured learning advantage is asserted.
- **Acceptance:** distinguish source findings from deductions, state the premises of each geometric claim, and establish a useful meaning for distance before selecting a learning kernel. The application contract remains under discussion.

## Current status

**Using internal minicolumn positions to create cross-minicolumn distance preference is rejected.** It must not remain a candidate under a renamed shared-coordinate, corresponding-index or soft-alignment rule. H-41's proposal remains in the historical decision record; H-42 is the current decision.

The existing organization follows statically assigned semantic roles inside each approximate-replica CC; it supplies no natural physical positions. The active question is which computational benefits of biological locality, if any, can apply within that organization. [SemanticCorticalMappingResearch.md](SemanticCorticalMappingResearch.md) owns the semantic/anatomical comparison. Current per-coordinate input/output ownership remains; neither new semantic roles nor an extra layout is presumed.

This note retains general network geometry as reference material. Its 1D/2D and exclusion-zone deductions are conditional results, not a recommendation to impose positions on arbitrary dimension labels.

[Physical distance preference research](PhysicalDistancePreferenceResearch.md) describes biological locality, selection beyond anatomical contact, heterogeneous live degrees and recurrent dynamics. Those findings do not yet justify an application distance prior. The formulas below are conditional tools requiring a meaningful metric; our semantic arrangement does not provide that metric.

## General spatial-network research

These results concern conventional spatial embeddings. They explain conditional geometry effects; their applicability depends on a justified relation between neural response properties and positions. No distance based on semantic-dimension labels or corresponding internal cell indexes is adopted.

| Study | Relevant result | Transfer boundary |
| --- | --- | --- |
| [Watts and Strogatz, 1998, *Collective dynamics of small-world networks*](https://www.cs.cornell.edu/courses/cs6241/2019sp/readings/Watts-1998-smallworld.pdf) | Rewiring a locally connected ring can combine high clustering with short graph paths. A 1D starting geometry is sufficient for this property. | This is not a learned, weighted SNN, and its local backbone does not implement our exclusion zone. It proves no learning superiority of either dimension. |
| [Kleinberg, 2000, *The small-world phenomenon: an algorithmic perspective*](https://www.cs.cornell.edu/home/kleinber/swn.d/swn.html) | In its lattice model with local edges and distance-biased long-range contacts, the exponent enabling efficient decentralized routing depends on spatial dimension. | Routing to a known destination differs from recurrent neural computation. The theorem assumes a local backbone; it does not select our kernel or override the near exclusion. |
| [Dall and Christensen, 2002, *Random geometric graphs*](https://arxiv.org/pdf/cond-mat/0203026) | For uniformly positioned nodes connected within a Euclidean radius, the bulk clustering coefficients are 3/4 in 1D and `1 - 3*sqrt(3)/(4*pi) ≈ 0.5865` in 2D. | With no inner exclusion, 1D is actually more clustered in this model. Dimension alone does not guarantee more triangles; our annular-neighborhood deduction below has different premises. |
| [Barnett, Di Paolo and Bullock, 2007, *Spatially embedded random networks*](https://users.sussex.ac.uk/~ezequiel/sern_physreve.pdf) | Their homogeneous spatial ensemble has a Poisson degree limit under bounded expected degree. Spatial bias alone need not produce a heavy-tailed degree distribution. | Their independence/homogeneity assumptions need not survive learning. Our capacity-class law and a distance kernel govern different quantities. |
| [Ercsey-Ravasz et al., 2013, *A predictive network model of cerebral cortical connectivity based on a distance rule*](https://doi.org/10.1016/j.neuron.2013.07.036) | Macaque interareal tracing supports a strong distance dependence and an exponential distance-rule network model. | Interareal projection statistics are not single-neuron connection probabilities within a CC. A distribution over connection lengths must not be mistaken for a per-pair probability. This does not compare learned 1D and 2D SNNs. |
| [Cossell et al., 2015, *Functional organization of excitatory synaptic strength in primary visual cortex*](https://www.nature.com/articles/nature14182) | Mouse V1 layer 2/3 recordings associate sparse strong excitatory connections with correlated visual responses; uncorrelated responses have weak connections. | Functional association matters alongside geometry. These observations neither establish our causal learning rule nor justify a universal hard exclusion zone. |

The first four papers were consulted in full text; the last two findings were checked against indexed publisher/PMC abstracts and publisher material. No unavailable full text is represented as inspected. Cortical surface organization motivates considering a sheet, but a uniform 2D sheet omits depth, cell types and layered circuitry; it is an engineering abstraction.

## Geometry, graph and storage are separate

1. **Geometry:** stable positions and a distance measure determine which eligible partners receive greater proposal probability.
2. **Learned graph:** actual directed connections and their weights result from activity, learning and capacity limits. A 2D embedding does not require a planar graph: edges may cross and long connections may exist.
3. **Storage:** neuron states and connection endpoints remain flat arrays. A ring drawn on a page is intrinsically 1D; a flat array indexed by `(x, y)` can implement genuinely 2D distances.

Serializing a grid in row order, or along a space-filling curve, preserves neither all 2D distances nor all neighborhoods when distance is then measured only by index difference. Conversely, calculating 2D distance from a scalar ID is entirely compatible with SoA storage.

## Consequences of dimension

For uniform spatial density, away from boundaries and at scales larger than lattice spacing:

| Property | 1D line / locally 1D ring | 2D sheet |
| --- | --- | --- |
| Candidates within radius r | Proportional to r | Proportional to r² |
| Candidates in a thin shell dr | Proportional to dr | Proportional to r dr |
| Neighborhood outside a small forbidden center | Two separated intervals locally | An annulus with angular freedom |
| Boundary of a large compact group, fixed local interaction range | A constant number of interfaces | Interface grows with the group's perimeter |
| Typical hop distance on a fixed-range lattice of N sites | Order N | Order sqrt(N) |

The last two rows describe fixed-range local lattices, not the learned graph. Shortcuts, degree heterogeneity and interaction ranges growing with N can change these scalings. A 2D annulus being spatially connected also does not guarantee that its induced neural graph is connected.

### A specific interaction with the near exclusion

This is our geometric deduction, not an anatomical measurement. It applies only if the near zone forbids partners at `r <= r0` in the very coordinate used for partner preference. Consider the particularly favored local band `r0 < r <= R`, with `R <= 2*r0`.

For three ordered positions on a line, `x1 < x2 < x3`, their outer separation is the sum of the two inner separations. If both inner pairs are allowed, then:

```text
d13 = d12 + d23 > 2*r0 >= R
```

Consequently all three pairs cannot lie in that favored band. Locally the same holds on a large ring; requiring `R < circumference/3` excludes a triangle wrapping around the whole ring. In a Euclidean plane, an equilateral triangle with side in `(r0, R]` is possible. A sufficiently fine grid can approximate such arrangements.

For example, with `r0=10` and `R=15`, three collinear cells separated successively by 12 have an outer pair at distance 24. Three cells at the corners of a triangle can all be 12 apart. Thus 2D can support a mutually nearby triplet without paying the lower probability of a longer third connection.

**The user has not requested a hard outer cutoff.** With a soft distance tail, 1D triangles remain possible; one connection pays a distance penalty. If the excluded zone is tiny compared with the decay length, this distinction may be weak. More local recurrent motifs can aid assembly formation or amplify unwanted synchrony; geometry alone cannot decide the learning outcome. If “exemption” instead means that very close partners remain allowed under another rule, this deduction does not apply to those partners.

### Per-partner probability versus connection-length distribution

Let `K(r)` be the preference assigned to each equally eligible partner at distance r. In d dimensions, the number of candidates in a shell contributes another factor:

```text
radial selection density(r) ∝ r^(d-1) * K(r),  r > r0
```

Therefore copying `K(r)=exp(-r/ell)` from 1D into 2D does not preserve the distribution of selected lengths: the respective radial densities are proportional to `exp(-r/ell)` and `r*exp(-r/ell)`. The nearer individual remains more likely in both cases, while a more distant shell can receive more total selections because it contains more cells.

For `K(r) ∝ r^(-s)`, mass per logarithmic distance interval is proportional to `r^(d-s)`. Equal mass per distance octave occurs at `s=d`: 1 in 1D and 2 in 2D. This is a geometric calculation, not a recommended neural exponent. Kleinberg's routing result is consistent with the distinction but cannot be transferred as an optimal-learning theorem. None of these distance exponents is the approximately 3 degree-distribution exponent used in capacity planning.

## Relationship to coactivity learning and capacity

For a generic spatial graph, an eligible set `E_i` can combine coactivity and exclusion before applying distance preference:

```text
Q(j | i, E_i) = K(distance(i,j)) / sum[k in E_i] K(distance(i,k))
```

This formula illustrates a generic preference conditional on eligibility; it establishes no semantic or anatomical justification for the distance. The rejected shared-position factorization supplies no current learning contract. Neither expression selects a sampling algorithm, trace representation or weight update. Time windows, prediction versus actual activation, causal order, edge direction, pruning and the role of unequal coactivity evidence remain open. Distance preference is not an instruction to make all nearer learned weights stronger.

A symmetric distance measure does not imply reciprocal learned connections. With target-owned incoming sets, it biases which source is considered for that target; incoming degree and outgoing influence still differ. Uniform distance preference alone supplies no reason to expect the provisional heavy-tailed degree population. Learning and capacity assignment must account for whatever heterogeneity is useful.

Capacity migration must preserve a neuron's logical position along with its identity and learned edges. Deriving position from its current pool class or slot-set address would make a storage reassignment silently change the learning rule. The numerical weight-scale policy remains a training decision, separate from both capacity geometry and spatial dimension.

## Application placement and memory

The population remains `5120 * (60 + 60) = 614400`. An illustrative ID layout `id=120*d+60*s+c` supplies identity and grouping, not a spatial metric. Neither the dimension index nor a matching internal cell index across minicolumns supplies a justified proximity prior.

The four-byte endpoints, one-byte weights, owner IDs, current-set references and FP4 state remain the current named arrays. Topology research introduces no new allocation, learned input projection or runtime structure. If a useful representation or algorithm is selected, its storage and work must be calculated then, under the static-allocation and SoA constraints.

## Current research boundary

[Semantic cortical mapping research](SemanticCorticalMappingResearch.md) separates assigned semantic response roles, learned connections, instantaneous state and physical layout. Repeated local representation is the relevant research comparison; large-area specialization and learned feature maps do not determine the internal map of an approximate-replica CC. Shared internal indexes remain rejected.

Future comparison requires a task, selected response representation, live-edge budget and specified learning semantics. A ring, sheet or contact abstraction is not the default next decision. The meaning and benefit of any added metric would need justification first; the geometric formulas do not supply it. No learning experiment or implementation is selected by this reference note.
