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
| [Memory Management at the Right Granularity — Coarse-Grained Ownership Tracking](CoarseGrainedOwnership.md) | How can a long-running service choose domain lifetimes and use data across several owners? | An importer with 1 GB free cannot get a 10 MB buffer from 125 separate 8 MB holes. Grouping the same allocations by retirement time makes a 1,000 MB span reusable. Returning a workspace reference after its owner ends must be rejected. An independently owned audit summary can survive. |
| [What should a composed operation retain?](WhatShouldAComposedOperationRetain.md) | When can a scoped connection and a region-owned plan implement the same caller contract? | Mutate a fare cap after preparation, then drive the same inputs. A snapshot, a live receiver and the current caller's policy yield three distinct answers. The [developer guide](../docs/BuildingReusableDomainOperations.md) applies the result to audit and replay. |
| [Composing enumerative choice with producers and transformers](ComposingEnumerativeChoice.md) | What can a higher-kinded synonym express about a synchronous traversal, its feedback and the stages that compose with it? | A seat filter must tell the upstream cursor to continue after skipping a fare. A caller returning the first affordable fare exits two live traversals and receives its result after cleanup. |
| [Typing open event pipelines across Actors and reactive timelines](OpenEventPipelines.md) | How can a library abstract over independent send and receive families, and over the signal families of a local reactive network? | Pump monitoring separates submission from a later worker turn. A [fuel sale](OpenEventPipelines.md#a-reflex-style-network-for-a-fuel-sale) distinguishes same-frame sampling rules by the amount on a receipt, and a dispenser switch distinguishes which completion reaches the console. |
| [Symbolic Effect Composition and Comonadic Refocusing in CoD](ComposingEffectsAndFocusedContexts.md) | When can independently extensible operations and scopes use additive routing code? Under what observations does refocusing agree with Store composition? | [Three states and five writers](examples/ComposedTileReport.md) expose the same business operations through Haskell `Ports m` and ordinary CoD procedures over dynamically selected state. The adapters show the actual transformer layers and labeled handlers. Moving staging past retry changes the published defect IDs and gain samples while preserving all the other observations. [Three business transformers](ComposingEffectsAndFocusedContexts.md#ship-three-business-transformers) expose the author's integration work. One fixed stack needs three cross-layer instances, and supporting all six orders needs six. A two-stage score is 15 with the correct focus, 0 with captured coordinates and 24 with an in-place update. |

### Read the two pipeline papers together

Start with [What should a composed operation retain?](WhatShouldAComposedOperationRetain.md). It separates a policy's binding time, the owner of a reusable plan, and the live control supplied by each invocation. The two pipeline papers use those results to develop [Event Processing Systems](../design/EventProcessingSystems.md), a planned CoD library in `e-wrks/eps.cod`. The [closed-pipeline study](ComposingEnumerativeChoice.md#where-a-closed-event-pipeline-ends) follows one live drive through feedback, outward exit and cleanup. The [open-network study](OpenEventPipelines.md#connecting-two-closed-drives) starts two independently driven computations and asks which observations survive admission, delivery or a change of reactive frame.

Both papers show how to name a composition before choosing either endpoint. The library user can
[expand routes and filter fares](ComposingEnumerativeChoice.md#an-itinerary-plan-with-both-ends-open), or
[decode and normalize device frames](OpenEventPipelines.md#one-ingress-plan-for-live-and-replay).
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
