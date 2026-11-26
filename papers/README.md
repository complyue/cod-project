# CoD working papers

These papers accompany the long-term design of CoD. Each begins with a programming decision
someone has to make: how a domain team builds a language its users can program visually, which allocations should retire together,
how to stop an inventory search, which state belongs to a completed fuel sale, or which tile flags to publish. The worked examples let a
reader compare possible language and library designs on paper. The [design registry](../design/DesignAuthorityRegistry.md)
records settled language contracts; the [design topics](../docs/DesignTopics.md) track decisions
still open.

| Paper | Design question | Case that separates the alternatives |
| --- | --- | --- |
| [Algebraic Effects Is All You Need — Software Defined HLVMs](AlgebraicEffectsIsAllYouNeed.md) | Can independent application authors define domain HLVMs and graphical languages in which practitioners write business programs with AI assistance? | A planner links warehouse fallbacks. An editor-in-chief inserts a legal approval before release in a different team's language. Both machines can import one ontology package and consult the [same historical facsimile catalog](AlgebraicEffectsIsAllYouNeed.md#shared-ontology-packages-and-dbmr-databases) through long-lived, shared read-only DBMR opens while using their own domain types, programs and private databases. |
| [Domain-Specific Database Solutions in CoD — Direct Access and Native DML with DBMR](DSDS.md) | Can a domain team reuse indexed data and typed DML across applications while choosing which operations commit together and which observations may lag? | Two server processes compete for the last foldout. One complete reservation commits while a warehouse display still shows one copy available. The [application guide](../docs/BuildingADatabaseServiceInCoD.md) exercises recovery, convergence and a catalog correction. |
| [Memory Management at the Right Granularity — Coarse-Grained Ownership Tracking](CoarseGrainedOwnership.md) | How can a long-running service choose domain lifetimes and use data across several owners? | An importer with 1 GB free cannot get a 10 MB buffer from 125 separate 8 MB holes. Grouping the same allocations by retirement time makes a 1,000 MB span reusable. Returning a workspace reference after its owner ends must be rejected. An independently owned audit summary can survive. |
| [Composing Operations with Different Lifetimes — Scoped Callbacks and Region-Owned Plans](ComposingAcrossLifetimes.md) | When can a scoped connection and a region-owned plan implement the same caller contract? | Mutate a fare cap after preparation, then drive the same inputs. A snapshot, a live receiver and the current caller's policy yield three distinct answers. The [developer guide](../docs/BuildingReusableDomainOperations.md) applies the result to audit and replay. |
| [Closed Event Pipelines — Typed Feedback and Nonlocal Return](ClosedEventPipelines.md) | What can a higher-kinded synonym express about a synchronous traversal, its feedback and the stages that compose with it? | A seat filter must tell the upstream cursor to continue after skipping a fare. A caller returning the first affordable fare exits two live traversals and receives its result after cleanup. |
| [A Vocabulary for Propagating Event Effects — Composing Domain Rules with Open Event Sinks](EventEffectPropagation.md) | How can independently authored domain rules give an event a new business consequence through an open sink? | A monitor offers the same [pump alarm](EventEffectPropagation.md#from-a-pump-alarm-to-an-inspection-request) to an alarm recorder or a maintenance rule. Only the maintenance rule requests an inspection. A [fuel sale](EventEffectPropagation.md#a-reflex-style-network-for-a-fuel-sale) shows why the chosen frame also matters. |
| [The Marginal Cost of Composition — Independent Extension and Local Reasoning in CoD](MarginalCostOfComposition.md) | When an application adds a detector and a shadow deployment independently, what must each author edit or know? | One simulated acquisition yields zero production reject commands and one shadow record. The [developer guide](../docs/ScopedEffectComposition.md) changes only the installation and obtains a different trace. The paper compares [routing costs](MarginalCostOfComposition.md#counting-the-things-authors-must-supply) and derives the [conditions for reusing sampled values](MarginalCostOfComposition.md#appendix-a). |

### Read the two pipeline papers together

Start with [Composing operations with different lifetimes](ComposingAcrossLifetimes.md). It separates a policy's binding time, the owner of a reusable plan, and the live control supplied by each invocation. The two pipeline papers use those results to develop [Event Processing Systems](../design/EventProcessingSystems.md), a planned CoD library in `e-wrks/eps.cod`. The [closed-pipeline study](ClosedEventPipelines.md#where-a-closed-event-pipeline-ends) follows one live drive through feedback, outward exit and cleanup. The [event-effects study](EventEffectPropagation.md#from-a-pump-alarm-to-an-inspection-request) lets one team define a pump alarm and another decide what it triggers. It then asks what each rule can observe across a queue, an Actor turn or a reactive frame.

Both papers show how to name a composition before choosing either endpoint. The library user can
[expand routes and filter fares](ClosedEventPipelines.md#an-itinerary-plan-with-both-ends-open), or
[decode and normalize device frames](EventEffectPropagation.md#one-ingress-plan-for-live-and-replay).
At the protocol level, a composition operator can connect stages `P → Q` and `Q → R` to
produce `P → R`. The letters stand for complete protocols, including feedback and borrowing.
The same CoD mechanisms let [rules of the parsing HLVM `pcl.cod`](../design/ParserCombinatorLibSupport.md#reusable-rules-and-the-scope-of-language-support)
compose before a caller supplies a parse session and output arena. A bounded lexical
connection can serve synchronous uses. A reusable regional program needs typed data edges
and valid owners. Naming families of such signatures, or passing a family itself as a type
argument, adds separate questions to those concrete examples.

Both ask what changes when a library can pass a type constructor such as `F : Type → Type`
and use it at several item types. They share candidate notation for producer, transformer and
consumer. In the booking example a suspended seat source can receive a Boolean from its active
consumer. At a chan boundary the sending traversal can finish before a receiving traversal
begins. A reactive timeline introduces a third question: which changes are simultaneous for
a local network? Each paper gives examples a type checker could distinguish and examples whose
answers depend on an operation's behavioral contract. The proposed notation and those
contracts can be discussed independently. [Higher-kinded type synonym topic](../docs/DesignTopics.md#higher-kinded-type-synonyms)

### How to read the examples

CoD listings exercise the identified language rules under the stated application assumptions.
Declarations labeled *paper notation* describe candidate future interfaces. A worked trace
records what a particular choice would imply: a service request, access rejection, feedback
to one active cursor, or price sampled at a particular instant. Changing one assumption and
working out the first differing observation is a way to challenge a proposal before choosing
syntax or implementing it. A seed or booter run can later be compared with that result. Its
acceptance or rejection alone does not settle the design.
