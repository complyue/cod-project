# CoD working papers

A program can return the expected answer while contacting an extra warehouse, leaking a cursor,
or sending a sensor reading to the wrong installation. An unchanged address can conceal a change
of owner. These papers use small examples in which a plausible implementation gives the right
answer to an inadequate check. Each identifies the additional observation needed to distinguish
the two executions or representations.

| Working paper | Research question | Evidence in the draft |
| --- | --- | --- |
| [Source and graph edits to an effectful policy](AlgebraicEffectsIsAllYouNeed.md) | When do a textual edit and an edit to a published program graph agree, including for the next editor? | A warehouse example has identical decisions on one order and different service requests. A second edit distinguishes a shared target from two identical copies. The draft specifies a comparison against a typed graph interpreter with an explicit service. |
| [Composing enumerative choice](ComposingEnumerativeChoice.md) | Does extracting a filter and a dependent traversal preserve what a live source observes at each offer and exit? | A case argument for finite synchronous traversals, a reproducible comparison with explicit loops, and one-seat counterexamples for wrong feedback and cleanup. |
| [Open event pipelines](OpenEventPipelines.md) | Which routing and progress information must survive a synchronous submission for later processing to give the correct answer? | A pair of indistinguishable untagged histories, an explicit-destination control, and a reproducible interleaving model with two deployments. |
| [Coarse-grained ownership](CoarseGrainedOwnership.md) | When can several references share owner information while retaining distinct identities and permissions at each use? | Two representation arguments, a backing-growth versus owner-replacement counterexample, and a cost study that separates metadata, image relocation and retained memory. |

The synchronous and open-pipeline studies meet at the point where an invocation ends. In the
former, the original cursor is still waiting for a particular reply or exit. In the latter, the
submitting frame can have retired before the next observation. The ownership study supplies a
question for both: which actual owner makes each later access admissible? The graph-edit study
adds a further horizon. Even agreement on every execution of the current policy can leave two
editors with different targets for a subsequent change.

The Python appendices in the two pipeline papers operate on finite models with stated inputs.
The other results are worked examples or conditional arguments. Proposed measurements concern
native execution, checking and resource costs under matched workloads. The individual papers
give the controls and failure criteria for interpreting those measurements.

### Reading compiler evidence

Derive the expected observation from the scoped language contract or an explicitly stated research
assumption before running a fixture. Record the seed or booter revision and the portion of the
workload it accepts. A disagreement is evidence to diagnose. Check the fixture and model against
the contract, then distinguish an implementation defect, missing coverage or an open design choice.
Agreement between two compiler versions is still one implementation observation for each version.
The paper's claims about the language rest on its contracts and arguments. This convention also
allows a useful counterexample to be published before either compiler can run the complete study.
