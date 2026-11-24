# Extending an Effect Vocabulary and Refocusing a Raster Computation

## Abstract

A surface-inspection application scores a camera image by comparing neighbouring pixels.
One library supplies the inspection rules. Other libraries choose how to tile the image,
retain tentative results and recover from a bad tile. Can the author of a new rule add a
capability without teaching every existing layer how to forward it? Can the author of a
new layer preserve calculations that read their neighbours without knowing each calculation?

We study both questions on one raster. For the first we construct a small instance matrix
from `mtl` and extend it along both axes. For the second we derive a two-stage inspection
from a focused grid, using a Store comonad as its value semantics. CoD's symbolic bindings
offer a way to write a rule once and run it at different foci. We identify which forwarding
obligations can disappear, which pairs still require a business decision, and the exact
assumptions under which the two ways of computing a score agree. We give a conditional
M + N source-size bound for transparent composition, locate the independent policy cost,
and compare retry and selective publication on a stack of **three State and five Writer
instances**. With the same rejected acquisition followed by an accepted one, two of five
journals change when the scopes exchange places. All three states and the other three
journals must agree. The [eight-role programs](examples/ComposedTileReport.md) give the
observable contract, the concrete `mtl` stack and a labeled `effectful` comparison. A
[one-handler control](examples/ScopedTileAudit.md) isolates invocation. All results below
are paper derivations.

## The inspection that both extensions must preserve

Inspect the same acquired image throughout a pass. A cell holds a signed intensity. The first
stage sums a pixel and its eastern neighbour. The second takes that horizontal sum here and
subtracts the horizontal sum immediately north. A high positive score flags a rise from the
previous row. Use clamped coordinates at the physical image boundary. An internal *tile* edge,
introduced to divide the work, is still inside the same image.

| Row | West | Centre | East |
| --- | ---: | ---: | ---: |
| North | 1 | 2 | 4 |
| Middle | 3 | 8 | 13 |
| South | 7 | 11 | 19 |

The middle cell's horizontal sum is 21. Its northern neighbour's horizontal sum is 6. An
inspection threshold of 12 therefore flags the middle cell: its score is **15**. The inspection
team now wants to record explanations for flags. Independently, the image team wants to execute
the same rules one tile at a time. Each change is useful without the other. Their combination
must still produce 15 and must attribute the flag to the middle cell in this revision.

The warehouse policy paper can compare two handlers for a fixed operation. For this study we
need authors who can independently add *kinds of operation* and *execution layers*, and an
old rule that must still work on a newly focused part of a result. The inspection supplies all
three. [Warehouse policy and its executor](AlgebraicEffectsIsAllYouNeed.md#two-entries-share-one-fallback)

<a id="composing-retry-and-publication"></a>

## Compose the inspection, retry and publication

A physical camera acquisition changes three pieces of state. Its serial advances from zero,
it spends two of an initial eight exposure credits, and it samples the current gain before
advancing it from its initial value of three. Each role holds the same unsigned integer
type. Five writers also share one output type, an ordered sequence of integers. They have
distinct business audiences: equipment operators need all acquisition IDs, diagnostics
needs rejected IDs, the defect report needs provisional IDs *and the corresponding gain
samples*, and the validator records passing IDs. The defect and sample streams must
commit together to keep an ID paired with its sample.

The tile team supplies the inspection. A retry wrapper calls its body again after a first
rejection. A separate stage wrapper buffers only the defect IDs and their gain samples,
then appends both batches to their parents if its body returns acceptance. Neither wrapper
names the other. Choosing where to put the stage determines what “approved report” means.
The same CoD leaf can be assembled as `retry_once() { stage() { inspect() } }` or
`stage() { retry_once() { inspect() } }`. With initial state `(serial, credits, gain) = (0, 8, 3)`, let the first acquisition be
rejected and the second accepted.

| Observer | Retry around stage (per acquisition) | Stage around retry (per tile) |
| --- | --- | --- |
| Verdict and final (serial, credits, gain) | Accepted, (2, 4, 5) | Accepted, (2, 4, 5) |
| Acquired / rejected / passing IDs | [1, 2] / [1] / [2] | [1, 2] / [1] / [2] |
| Defect IDs | **[2]** | **[1, 2]** |
| Gain samples paired with those IDs | **[4]** | **[3, 4]** |

Both orders charge the camera twice. Moving the wrong boundary could refund the first
exposure, reuse its serial or publish a defect without its matching gain. Each would give
a distinguishable result even if the tile was ultimately accepted. The
[worked example](examples/ComposedTileReport.md) also gives immediate-success, exhausted-retry
and routing controls.

The `mtl` case really stacks five `WriterT`s on three `StateT`s. The application's adapter
names the routes to the separate layers. A plain `tell [n]` selects the nearest writer.
A scalar type alone cannot tell `state` which of three counters the author means. Peeling
both provisional writers at the stage boundary handles them together; the base states
and the other journals continue through the body. A labeled `effectful` row gives the
eight roles names even with identical payloads. In CoD the source gives the roles nominal
symbol identities. A scoped value such as a threshold can use `val symbol`. Independently
mutable cells that must retain writes across calls can be local lexical storage selected
by reference symbols; temporarily selecting private cells stages just the chosen outputs.
[Value binding and target updates](../design/SymbolicDynamicScope.md#scoped-bindings-versus-object-transfer)

These eight are *instances* of two familiar effect families. Three states and five writers
make errors in naming and routing visible; their product, fifteen, counts no obligation
in this experiment. The size argument below has different axes: independently extensible
interfaces and layer forms. Freeze the two wrappers, add a separately named observation
which must survive rejection, then add an independent scope which passes the other effects
through. Record what changes in the old inspector, the adapters, the handler assembly and
the observers. Compare this with deliberately extending the publication contract to a third
staged output, which requires a change to the stage's policy.
[Same-typed instances and extension](examples/ComposedTileReport.md#what-changes-when-another-author-extends-the-program)

## Where the M × N obligations arise

Let M count independent effect interfaces and N count outer transformer constructors through
which clients want to use them. A cell asks how all methods of one interface work on that
outer layer. Count supported pairs for this workload. A built-in instance, a derived forwarding
instance, a new semantic implementation and a rejected pairing have different costs.
Permutations of a complete stack and the number of domain algorithms are different quantities.

Here is a baseline that can be checked against `mtl` 2.3.1. The application uses the standard
classes to expose focus, mutable run state and tile faults. The columns are three constructors
applied to an appropriate base monad. For each non-native cell the base supplies the interface
being forwarded. The state transformer is the strict variant.

| Interface available to the client | `ReaderT Focus` | `StateT RunState` | `ExceptT TileFault` |
| --- | --- | --- | --- |
| `MonadReader` — inspect focus or run a body under a changed focus | Supplies its own environment and `local`. | Lifts the read; `local` maps over the underlying state computation. | Lifts the read; `local` maps over the underlying error computation. |
| `MonadState` — read or update run state | Lifts the state operation. | Supplies its own state. | Lifts the state operation. |
| `MonadError` — raise a fault or protect a computation with a handler | Lifts a throw; runs the protected action and handler with the reader environment. | Lifts a throw; runs the catch handler with the state from entry to the protected action. | Supplies its own throw and catch. |

These are three native cells and six inherited cells already supplied by the library.
In these nine instances the source spells out 24 method bindings: eight native methods,
twelve ordinary lifts, two mappings for `local` and two specialized liftings for `catchError`.
A user of this stack writes none of the nine instances. The table explains the maintenance
question faced by someone shipping a *new* interface or transformer. Lifting a single base
action suffices for many operations. `local` and `catchError` also receive computations that
must run in the right context. The library's transformer-specific mappings and catch helpers
make those choices inspectable. Jaskelioff identified this product of axes and gave a modular
lifting construction under additional structure. It would be misleading to turn the example
into a lower bound on all Haskell encodings. [Reader instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/src/Control.Monad.Reader.Class.html)
· [State instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/src/Control.Monad.State.Class.html)
· [Error instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/src/Control.Monad.Error.Class.html)
· [Jaskelioff, *Modular Monad Transformers*](https://www.fceia.unr.edu.ar/~mauro/pubs/mmt/mmt.pdf)
· [Affeldt and Nowak, formalization of modular transformations and their hypotheses](https://doi.org/10.4230/LIPIcs.TYPES.2020.2)

### Add a row, then a column

1. **Add the inspection team's `Flag` interface.** Its operation records the coordinate and score
   when the contrast threshold is reached. If it lives in the base, the three existing layers each need
   a route to it. An implementation can reuse an appropriate lift for this first-order operation.
   Native dispatch, reusable generic definitions and generated forwarding each count as reuse.
2. **Add the image team's `TileT` layer.** Existing focus, state and error operations must
   still be available where the tiled calculation uses them. The new layer must distinguish
   the physical image boundary from a tile's private storage boundary. A scoped change of focus
   may read a neighbouring tile. A tile with no access to the required border pixels must
   obtain them or report a fault.
3. **Install both extensions.** Where does a flag written during a tile attempt go if the tile
   subsequently fails verification? An audit of every attempt keeps it. A published defect
   report containing only verified tiles drops it. Either contract is useful, but a transparent
   lift to an append-only destination implements only the first. For the second, stage the flag
   until tile acceptance.

Going from three by three to four by four adds **seven candidate pairings**: three old layers
for the new interface, three old interfaces for the new layer, and the new intersection.
In general, adding one row and one column to a full M × N table adds M + N + 1 cells.
The seven are a reproducible count of questions, not seven required hand-written instances.
For each, record who had to change code, what default applied, what evidence the checker
required and what an unchanged client observed. A single conditional over every supported
layer, relocated into a registry, would still be pair-specific maintenance.

### One cell where a generic answer changes the result

Start a local tentative flag count at zero. In the protected body, increment the count and then
raise a typed tile fault. In the catch handler, read the count. With the standard instances,
`StateT Count (Except TileFault)` yields **0** at that read. `ExceptT TileFault (State Count)`
yields **1**. A successful increment with no fault yields 1 in both. The two failing
calculations request the same operations. The accepted observation depends on whether run
state survives the failure. An independently persisted audit append would require a separate
observation; restoration of a pure state value could not undo it.

| Comparison | What to try on this same workload |
| --- | --- |
| `mtl` with standard transformers | Use the table above, then allow reasonable baselines such as deriving through a newtype or projecting two services out of one reader environment. Record which functions still need a scoped interpretation and whether the supplied failure semantics match the requested report. |
| `fused-effects` | Make `Flag` a member of the effect signature; let a carrier handle its own operations and delegate the remaining signature. Check whether the tile carrier can delegate unknown first-order flags unchanged, and where it must take responsibility for provisional publication. |
| Polysemy | Interpret the domain operations into a pure state and a typed error. In the documented pure interpreters, eliminating `Error` before `State` preserves the increment in the catch; the opposite order discards it. Record the chosen interpreters and order alongside the result. |
| `effectful` | Compare its `Error.Static` and `State.Static.Local` handlers. In this single-threaded caught-fault case both orders keep the increment. To publish verified flags only, express the commit boundary explicitly. |
| CoD symbolic effects | Trace nominal symbol lookup, the bindings saved at activation, required reachable bindings and the owner of the flag destination. A write into a still-live outer audit sink remains visible after unwind. A tile-owned buffer merged on success gives the alternative observation. |

The first two rows address how much forwarding an author needs. The final three also
supply controls for what such forwarding *means*. None of the counts here measures runtime
dispatch cost. [fused-effects, comparison to mtl](https://github.com/fused-effects/fused-effects#comparison-to-mtl)
· [A fused carrier delegating its remaining signature](https://github.com/fused-effects/fused-effects/blob/82f59e1ea83b97aaa4c7a3fe2f9a1fa350564f33/src/Control/Algebra.hs#L302-L307)
· [Polysemy, the two interpreter orders](https://github.com/polysemy-research/polysemy/blob/e41ef55ab3c28edb7d750664d583f2737237140f/src/Polysemy/Internal.hs#L108-L150)
· [effectful, error/state order](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Error-Static.html)
· [Schrijvers et al., *Monad Transformers and Modular Algebraic Effects*](https://people.cs.kuleuven.be/~tom.schrijvers/portfolio/haskell2019a.html)

<a id="additive-source-extension"></a>

## When M × N becomes M + N

**Claim to test.** For independently supplied operations and transparent scopes in the
fragment below, the amount of authored routing code grows additively in the two
vocabularies. An old scope can admit a new operation without an operation-specific adapter.
An old operation can be used inside a new transparent scope without a scope-specific adapter.
The claim concerns source changes when *both* populations can grow. A fixed small program
with a short spelling supplies a witness, not the asymptotic argument.

### Counting the things authors must supply

Take M independently named operation interfaces and N independently written scope forms.
Use the same required operations, supported programs, admissible owners, failure policy and
observations in every encoding. A scope can introduce bindings, call its body and restore
those bindings on exit. For an operation it promises to pass transparently, it invokes its
body once, preserves the required underlying resources and capabilities, and supplies the
agreed bindings. A layer may deliberately rebind a dependency. The client can then request
the installation-time or current interpretation under the operation's contract. The
comparison includes the type checker, standard dispatcher and reusable adapters as fixed
infrastructure with their assumptions disclosed.

Let each operation declare its identity, bounded signature and required interpretation once.
Let each scope install a bounded number of bindings and use the same scoped-lookup and
restoration protocol. The cost for these definitions is bounded by `aM + bN + c`. Here
is a construction for that bound. Lookup selects a binding by nominal identity. Pushing a
frame for a different identity leaves lookup of the old identity available by the same rule.
Induction over nested frames permits any number of these transparent crossings without a
new source declaration for the crossed pair. Popping frames in reverse order restores the
previous bindings on normal and checked exceptional exits. For a selected handler, replay
uses its required bindings saved at activation, whereas current invocation uses the effective
bindings at the call. The actual targets must stay alive. The argument assumes the admissible
call paths pass CoD's existing availability and owner checks. It does not establish how much
work the checker needs to discharge them. [Symbolic installation and invocation](../design/SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)
· [Replay and resource lifetime](../design/SymbolicDynamicScope.md#regional-resource-capture-preserves-owners)

The more general bound is the **total source size of the operations, scopes and independent
interaction rules**, plus shared infrastructure. Under bounded-size definitions, K independent
exception rules give `O(M + N + K)`. Count both registration and wiring at application entry;
the additive fragment assumes these can be supplied once per independently chosen unit.
Specify the rules before measuring. A generated M-by-N table whose input independently selects
every cell still encodes M-by-N decisions. In fact, arbitrary, independent keep/discard choices
for the cells allow `2^(M × N)` policies: selecting one in the worst case needs at least
`M × N` bits somewhere. One rule such as “stage every public flag until this tile passes”
can cover a whole family. Charge its description once. For policies involving three or more
components, charge the actual multi-party rules and their size. The number of semantic
outcomes, test obligations, compiled specializations and dynamic calls have separate costs.

In the [two report compositions](#composing-retry-and-publication), retry and staging each have one definition. The business selects their order. This deliberate interaction sits outside the once-only transparent-body assumption in the bound. It adds an explicit choice at assembly. A transparent pass through a new symbol also cannot make a previously released owner accessible.

### What would constitute an advance?

Reduction in hand-written forwarding would be a material engineering result for CoD and a
well-defined comparison with `mtl`'s explicit instance matrix. Several mechanisms already
achieve modularity in relevant fragments. Modular lifting abstracts over classes of
transformers and operations. Extensible effects use open requests, and fused carriers can
delegate the remainder of a signature. The closest comparison for capture is the work on
typed implicit functions that select a binding dynamically and evaluate under the binding
environment. Work on lexical handlers also gives typed, semantics-preserving translations
using second-class capabilities and region evidence. These references answer different
parts of the proposed claim. [Modular lifting](https://www.fceia.unr.edu.ar/~mauro/pubs/mmt/mmt.pdf)
· [Extensible effects](https://homes.luddy.indiana.edu/sabry/files/exteff.pdf)
· [Implicit functions and dynamic binding](https://www.microsoft.com/en-us/research/publication/programming-with-implicit-values-functions-and-control-or-implicit-functions-dynamic-binding-with-lexical-scoping/)
· [Müller et al., *From Capabilities to Regions*](https://pl.cs.uni-tuebingen.de/publications/mueller23capabilities-preprint.pdf)

The stronger CoD research target is **additive source extension with the same observable
behavior, two invocation modes on the same installed operation, and checked lifetimes for
the actual application data its handlers can reach**. Freeze the old modules. Have independent
authors supply a new operation and a new transparent scope, in either order. Compare to an
explicit-capability reference with the same sequence of destinations, requests, results,
faults and cleanup. Change only the backing owner and require an invalid old use to reject;
retain a control where the same owner stays live. Repeat with a deliberately interacting
layer, writing down the extra policy once. Count source edits in the application, libraries,
generated inputs and any central dispatcher. A per-pair requirement hiding in any of those
places defeats the additive claim for that fragment. A translation preserving accepted
observations and the relevant rejections would go beyond a collection of passing examples.

### Control experiment: selecting the environment of one installed method

The [scoped audit and threshold programs](examples/ScopedTileAudit.md) isolate `perform`
and `behave` on a single installed operation. An auditor installed at destination 1 runs
under a caller at 9 and a tile at 2. The two choices record at 1 and 2 respectively, and
the caller gets 9 back on return or checked failure. A short threshold variant isolates
the same choice on a pure predicate. Their source counts describe just that protocol.
The retry/publication programs above supply the separate evidence about composing
independently authored effects.

## What named effects remove in CoD

The listings spell an independent bits binding `val symbol @x:T`, making its value role
explicit. The current design specifies the same scoped value as `symbol @x:T`. The examples
assume that established save-and-restore rule. [Symbol categories](../design/SymbolicDynamicScope.md#explicit-ref-and-ptr-symbols-backed-by-a-region-symbol)

Consider a tile scope that binds only its tile identity. An inspection procedure directly
calls an installed `@flag` operation with `behave`. Provided the required outer flag handler
and its dependencies are available, the tile scope need not declare an adapter for that symbol.
Conversely, adding a flag handler need not change old focus and pixel operations. The caller
still has to install the intended interpretations. A missing required binding on a reachable
path is an error. Two independently declared symbols with the same spelling and signature do
not acquire a common identity by coincidence.

The two authors can exercise this contract with the following CoD fragment. The image
library already calls the named inspection entry under a selected tile. The inspection
library adds the flag operation; the application installs its interpretation at their common
caller. The tile procedure can retain its existing body. The new inspection path still
has to pass the binding check.

```cod
val symbol @tile:u64
symbol @flag:proc(row:u64, col:u64, score:i64) -> void

proc inspect_tile_cell(tile:u64) -> void {
    effect @tile <- tile
    inspect_cell()
    return
}

proc inspect_cell() -> void {
    val score <- north_contrast()
    if (score >= 12i64) { behave @flag(@row, @col, score) }
    return
}
```

Here `north_contrast` and the named coordinates are defined below. With the complete
image and the middle focus, the flagger receives (1,1,15) once. A clipped tile that computes
0 sends no flag. Observing an empty report alone cannot distinguish a wrong score from
misrouted output. Check the score and the selected destination independently.

For a provisional report, deliberately shadow the flag destination with storage held for the
tile. Calls intended to follow that selection must use the effective destination. A
`perform` on a handler whose snapshot includes the outer sink can write to the outer sink
instead. That is a distinct implementation with a distinguishing trace. The authors of the
flag vocabulary and tiling layer need an agreed publication protocol at this intersection.
Thus a new symbol can pass an unrelated scope without a new declaration, even as the
combination of flagging and tile rejection needs a policy. The claim would fail if unrelated
scopes had to enumerate every new symbol, or if the only way to implement the policy were
to edit every old inspection rule. [Installation and invocation](../design/SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)
· [Call-path binding checks](../design/SymbolicDynamicScope.md#call-path-binding-checks)
· [Ownership of captured resources](../design/SymbolicDynamicScope.md#regional-resource-capture-preserves-owners)

## A focused grid gives the rules a reference semantics

One concrete meaning of the *comonad question* is whether ordinary CoD can express and
compose a rule that consumes a focused context. In a two-stage inspection the second rule
needs the first rule's result at the current focus *and* at another focus. A current-coordinate
variable can answer “where am I?” by itself. To answer the second question, the program must
be able to evaluate the same rule over the same image at another legitimate position.
Structured-grid languages such as Ypnos and work on comonadic dataflow give independent
precedents. [Orchard, Bolingbroke and Mycroft, *Ypnos*](https://www.cs.kent.ac.uk/people/staff/dao7/publ/ypnos1.html)
· [Uustalu and Vene, *The Essence of Dataflow Programming*](https://cs.ioc.ee/~tarmo/papers/uustalu-vene-aplas05.pdf)

Fix a nonempty finite set of valid image positions P, an immutable image and a particular
boundary rule for relative moves. In paper notation the focused view of values of type A is
a Store, `W[A] = (P → A, P)`. Write a view as `(v,p)`. The following definitions and laws
compare complete views pointwise, including their focus.

```text
refocus((v,p), q) = (v,q)                  for q in P
extract(v,p)      = v(p)
extend(f)(v,p)    = (q ↦ f(v,q), p)       for f : W[A] → B

extract(extend(f)(w))      = f(w)
extend(extract)(w)         = w
extend(g)(extend(f)(w))    = extend(u ↦ g(extend(f)(u)))(w)
```

The first law reads the result at the original position. The second recovers each value by
reading it at its own position. For the third, inspect either side at any position q:
both give g the field whose value at r is f applied to the original image focused at r,
with q as g's focus. This is why materializing the complete horizontal-sum field and
recomputing the needed horizontal sums on demand agree for total, read-only rules. A
straightforward Haskell `Store` or an explicit image-and-coordinate pair is a fair baseline
for this part of the study. A reader environment carrying that pair can also run a rule
under a changed focus with `local`. Across all three presentations, the extra work in
`extend` is to supply a *new field of results* on which the next rule can focus.
A mixed computation `W[A] → M[B]` that also logs, fails or reads
a changing service needs further interaction rules before the same equation can be claimed
for its observable trace. [Comonad operations and laws](https://hackage.haskell.org/package/comonad/docs/Control-Comonad.html)

The obstruction can be located precisely. After refocusing the input at every position and
applying the first effectful kernel there, the intermediate shape is `W[M[B]]`. The next
kernel expects a `W[B]`. One theoretical route supplies a compatible distributive law
`W[M[B]] → M[W[B]]` for the chosen comonad and monad. For an audit effect, evaluating a
whole tile eagerly, evaluating only the requested positions, and memoizing repeated reads
can each produce a different audit history. CoD's ability to name both operations gives a
programmer places to implement those policies; it does not select a law for that pair.
[Uustalu and Vene on effectful context-dependent computation](https://doi.org/10.1007/11575467_2)

## Recompute the northern score in CoD

The following CoD design example specializes the two rules. `east` and `north` are ordinary
total helpers that clamp at the image boundary. `fixture_at` is the application's read of
the table above. The real acquisition must keep its selected image alive and stable through
both stages. Both coordinate helpers map `u64` to `u64`; `fixture_at` takes two `u64`
coordinates and returns an `i64`. All intermediate values in this fixture fit in `i64`.
The code uses zero-based row and column positions. The scalar binding names
and the pixel operation belong to one shared vocabulary.

```cod
val symbol @row:u64
val symbol @col:u64
symbol @pixel:proc() -> i64

proc east_sum() -> i64 {
    val here_value <- behave @pixel()
    val east_value <- {{
        effect @col <- east(@col)
        behave @pixel()
    }}
    return here_value + east_value
}

proc north_contrast() -> i64 {
    val middle <- east_sum()
    val northern <- {{
        effect @row <- north(@row)
        east_sum()
    }}
    return middle - northern
}

effect @row <- 1u64
effect @col <- 1u64
effect @pixel() -> i64 { return fixture_at(@row, @col) }
assert north_contrast() == 15i64
```

Inside the northward scope, `east_sum` reads 2, shifts one column, and reads 4. When both
scopes exit, the focus is again the middle cell. Replace **both** `behave @pixel()` calls
with `perform @pixel()` and keep the same installation. The pixel handler replays (1,1);
every read returns 8. Both sums become 16 and their difference becomes **0**. This change
isolates refocusing from the choice of arithmetic. A second positive control is to direct
both subcomputations to an ordinary function taking the same image, row and column explicitly:
that version must recover 15. [Captured values and call behavior](../design/SymbolicDynamicScope.md#captured-values-and-call-behavior)

The listing uses named procedures and immediate nested scopes on purpose. A general library
may want an operation that applies a caller-supplied rule at each focus. Such an interface
must respect CoD's admitted callable kinds, lexical control and owner lifetime. The existence
of this specialized computation supplies no first-class value containing an installed handler.
Keeping a handler's snapshot does not copy the image or prolong its owner's lifetime.
If an actual image owner is replaced, later reads must obtain fresh access under the new owner.
[Calls through lexical definitions](../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)
· [Owner replacement](CoarseGrainedOwnership.md#owner-identity-target-identity-and-representation)

## Three ways to get the right-looking program wrong

**Tile seam.** Put a tile boundary immediately above the middle cell. The physical northern
values are 2 and 4 in the neighbouring tile. Treating that seam as an image boundary
reuses 8 and 13 and incorrectly returns 0. Providing the required border values restores
15. If those values are unavailable, a checked fault or an explicitly provisional output is
an application choice. A kernel that only needs the current pixel can still be used on the
same tile without acquiring that border.

**An edge is part of the calculation.** At the top right, clamping gives
`(4 + 4) − (4 + 4) = 0`. Switching both axes to periodic boundaries gives
`(4 + 1) − (19 + 7) = −21`. On the middle row, starting at its right edge and moving east
then west under clamping reaches the middle value 8. Cancelling the two offsets first
would leave the original value 13. A footprint or optimization must compose the actual
boundary-aware focus changes.

**Observing the evaluation.** On this 3 × 3 image, materializing the first stage once visits
nine foci. Computing only the middle output on demand visits two first-stage foci. Even when
all nine outputs are requested in both versions, a literal two-call evaluation of every output
without memoization visits eighteen first-stage foci. If every
first-stage invocation appends a flag or increments an observable counter, the same final
15 now comes with different histories. Even value equality can fail if the pass overwrites
the source it is still reading. With rows updated from right to left, the middle row's
eastern value becomes 26 before its centre sum is written as 34. The northern equivalents
are 8 and 10. The second stage then returns 24. A read-only view alone cannot establish
stability against mutation through an independent admitted access path.

There is also a case at the intersection of both experiments. Let a second inspection stage
request the same scored focus twice in a successful tile. If scoring emits the public flag,
a cache may suppress the second flag, while recomputation stages two copies. Buffering until
tile acceptance publishes both copies together; it only settles what happens on rejection.
If the report promises one flag per defect, emit at a separate commit step keyed by image
revision, position and rule, or explicitly define equivalent deduplication. A kernel whose
only result is the score permits either evaluation strategy without that side effect.

A coeffect analysis would address a further question: which part of the input can the
middle output depend on? Its four positions are middle, east, north and northeast. Changing
the northeast value from 4 to 5 must give 14. Changing only the southwest value from 7 to
999 must still give 15. A conservative footprint for a composite rule takes the union of
the first rule's footprints at all positions that the second rule may inspect. Proving that
`@pixel` is installed establishes no such spatial bound. Whether to infer it, ask for it in
a library contract or inspect it at runtime is open. [Petricek, Orchard and Mycroft, *Coeffects*](https://tomasp.net/academic/papers/structural/)

## What would settle the design questions?

| Question for CoD | Evidence that would change the conclusion |
| --- | --- |
| Can unrelated effects pass a new scope without pair-specific source changes? | Give the flag author and the tiling author only the published interfaces. List changed declarations, generated forwarding, reachable binding checks, intentionally unsupported pairs and explicit policy adapters separately. A hidden central switch over all old effects counts as an edit. |
| Can a reusable library express the two-stage rule with lawful refocusing? | Supply the specialized named-procedure version and an explicit-context baseline. Propose an admissible interface for arbitrary eligible kernels. Compare 15 in the interior, the boundary cases, missing border data and restoration after an exceptional exit. Keep the stable-image precondition explicit. |
| Which effectful kernels can share the pure composition law? | Specify observations first. Count both stage evaluations and committed flags. An equality that preserves the score and changes a promised diagnostic trace fails for that interface. |
| What does the checker need to know about a neighbourhood? | Show that the footprint covers the northeast dependency, handles the tile seam and respects the boundary policy. Decide separately if conservative over-approximation is acceptable for prefetch, permission checking or invalidation. |

These cases justify two limited claims. Independent named operations can be usable through a
scope that has no business interpreting them. A stable, focused region with well-defined
refocusing can support a comonadic fragment of a CoD library. General composition of arbitrary
comonads, an inferred spatial coeffect system, and a law for combining arbitrary effects
with refocusing each require their own problem definition and evidence.
