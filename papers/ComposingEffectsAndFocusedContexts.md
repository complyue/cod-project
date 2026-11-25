<a id="extending-an-effect-vocabulary-and-refocusing-a-raster-computation"></a>

# Symbolic Effect Composition and Comonadic Refocusing in CoD

## Abstract

How much code must independent authors write when they can add both an effect and a scope
through which existing effects must pass? When can a computation that reads its surroundings
be reused at a different focus? We study these questions together for CoD's typed symbolic
bindings. In both cases, composition must preserve what a caller can observe.

For $M$ independently named operations and $N$ transparent scopes, we give a conditional
$O(M + N)$ bound on authored routing code. The construction uses nominal lookup and restoration,
bounded definitions and statically checked, live binding targets. With $K$ independent,
bounded interaction rules the bound is $O(M + N + K)$. Arbitrary policies can still require
a separate decision for each of the $M × N$ pairs. We relate this accounting to the interface-by-transformer
instances in `mtl` and compare two orders of retry and selective publication in CoD,
`mtl` and labeled `effectful`.

A defect viewer and a batch job share a detector that can read either raw pixels or a
horizontal filter's results. The detector must be able to read its input again at a neighbour.
We compare how its author expresses the rule and how the application connects the stages
in CoD, Haskell and C++20. A synchronous CoD `here` body can supply either input under a
locally changed focus. Caching the filter preserves scores on a stable frame with total,
observationally pure rules. It can change the report if evaluating a rule itself publishes
an event. The appendix proves the value equivalence in the Store model. These are design
results for the stated fragment. The interface for retaining a derived CoD view and the
conditions for reordering effectful rules remain open.

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
The same CoD leaf can be assembled as `retry_once() { stage() { inspect(k) } }` or
`stage() { retry_once() { inspect(k) } }`. Set the rejection threshold `k` to 1 and
initialize `(serial, credits, gain)` to `(0, 8, 3)`. The first acquisition is rejected
and the second accepted.

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

### Compare the operations the inspector can request

The Haskell leaf receives a `Ports m` record. The CoD leaf calls eight ordinary procedures.
`charge p` and `charge()` both debit the currently selected balance by two. `samples p g`
and `samples(g)` append the same gain reading to the currently selected sample journal.
The CoD provider and runner use the storage identities. The business module selects
only the named operations.

| Author's job | Haskell | CoD |
| --- | --- | --- |
| Write the inspection | Request the eight operations through `Ports m`. Both backends use the same leaf. | Call `next_serial()`, `charge()`, `take_gain()` and the five journal procedures. |
| Supply state and routing | Fill `Ports Stack` with actions on three `StateT` and five `WriterT` layers, or use eight labeled `effectful` handlers. | Define each procedure once. Bind three state cells and five journal destinations at application entry. |
| Stage defects and samples | The grouped stack filters two writer outputs. The labeled row installs two child writers and forwards accepted batches. | Bind those two destinations to temporary copies. On acceptance, write the completed journals to the restored parent targets. |

Here are two CoD operation definitions and the application's choice of scope order.
`appended` computes the new contents of the bounded journal. The same `charge()` reaches the
same balance in either order. The same `samples()` can reach a different destination.

```cod
ref symbol @credit_cell:u64 in O where {owner O}
ref symbol @samples_sink:u64 in O where {owner O}

proc charge() { @credit_cell <- @credit_cell - 2u64 }
proc samples(n:u64) { @samples_sink <- appended(@samples_sink, n) }

enum StageScope:u8 { PerAcquisition; PerTile; }

proc tile(scope:StageScope, reject_through:u64, prefix:bool) -> bool {
    if (prefix) {
        defects(7u64)
        samples(8u64)
    }
    return case scope : StageScope of {
        PerAcquisition -> retry_once() { stage() { inspect(reject_through) } };
        PerTile -> stage() { retry_once() { inspect(reject_through) } }
    }
}
```

In Haskell the application supplies `inspect` with a `Ports Stack` value. This is its
entire adapter for the `mtl` stack. The five outer layers are `WriterT` and the three
inner layers are `StateT`. `charge` reaches the second state and `samples` the second
writer. These are the routes used by Haskell's counterparts to `charge()` and `samples()`.

```haskell
fromStates :: States a -> Stack a
fromStates = lift . lift . lift . lift . lift

ports :: Ports Stack
ports = Ports
  { nextSerial = state advance
  , charge     = fromStates (lift (state pay))
  , takeGain   = fromStates (lift (lift (state takeOld)))
  , defects    = \n -> tell [n]
  , samples    = \n -> lift (tell [n])
  , acquired   = \n -> (lift . lift) (tell [n])
  , rejects    = \n -> (lift . lift . lift) (tell [n])
  , passes     = \n -> (lift . lift . lift . lift) (tell [n])
  }
```

The Haskell application then makes the same scope choice. `runStack` initializes the
three states and runs the five writers.

```haskell
run scope k prefix = runStack $ do
  when prefix $ defects ports 7 >> samples ports 8
  case scope of
    PerAcquisition -> retryOnce (stage (inspect ports k))
    PerTile        -> stage (retryOnce (inspect ports k))
```

With labeled `effectful` handlers the same choice is made in `tile`. The type fixes the
public row at the application boundary. Inside `stage`, `ports` can address the two
private writers.

```haskell
tile :: StageScope -> N -> Bool -> Eff Eight Bool
tile scope k prefix = do
  when prefix $ defects ports 7 >> samples ports 8
  case scope of
    PerAcquisition -> retryOnce (stage (inspect ports k))
    PerTile        -> stage (retryOnce (inspect ports k))

run scope k prefix = runEight (tile scope k prefix)
```

Take `k = 1` and start with public entries `[7]` and `[8]`. The first acquisition fails
at gain `3`, the second passes at gain `4`. A stage per acquisition publishes `[7,2]`
and `[8,4]`. A stage around the entire retry publishes `[7,1,2]` and `[8,3,4]`.
The [application entries and state setup](examples/ComposedTileReport.md#application-end-in-cod-and-haskell)
show all three calls. The [engineering checks](examples/ComposedTileReport.md#engineering-checks)
exercise twelve inputs and two nested-stage controls in each assembly. Each checks the verdict,
three final states and all five journals.

[CoD procedures, state and staging](examples/ComposedTileReport.md#cod) · [Haskell Ports and business rule](examples/ComposedTileReport.md#haskell-with-named-operations) · [Ports on the concrete transformer stack](examples/ComposedTileReport.md#wiring-ports-to-the-transformer-stack) · [Ports on the labeled effect row](examples/ComposedTileReport.md#wiring-ports-to-labeled-effectful-handlers)

At the application boundary, count the CoD procedure definitions and state installations,
and the Haskell record, its actions and the runners. Count a reusable definition once.
A Haskell `Ports m` value also lets a caller substitute an arbitrary operation. This tile
workload needs only to select state and scope. Runtime replacement of a method body and
selection of its invocation context are exercised in the [audit control](examples/ScopedTileAudit.md).
A lift can select the wrong same-typed writer. So can binding `@samples_sink` to the
wrong `u64` cell at entry. Both errors are visible in the named observations.
The [direct stack listings](examples/ComposedTileReport.md#haskell-with-mtl) expose how the
ports are routed. The [interleaved layout](examples/ComposedTileReport.md#an-interleaved-stack-using-transformers-directly)
places serial between the two staged writers. Its publication operation must preserve the
advanced serial on rejection. That cost belongs to the stack implementation and its layout.

These eight are *instances* of two familiar effect families. Three states and five writers
make errors in naming and routing visible; their product, fifteen, counts no obligation
in this experiment. The size argument below has different axes: independently extensible
interfaces and layer forms. Freeze the two wrappers, add a separately named observation
which must survive rejection, then add an independent scope which passes the other effects
through. Record changes in the business leaf, operation interfaces, adapters, handler installations
and observers. A reusable Haskell adapter and a reusable set of ordinary CoD procedures each count once. Compare this with deliberately extending the publication contract to a third
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

For a bits type `T`, `val symbol @x:T` explicitly declares a value symbol, and
`symbol @x:T` is its abbreviated form. In either spelling, installing a new value
saves its predecessor and restores it on scope exit. The examples use the explicit form
where the storage category matters. [Symbolic binding and restoration](../design/SymbolicDynamicScope.md#scoped-bindings-versus-object-transfer)

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

proc inspect_tile_cell(tile_id:u64) -> void {
    effect @tile <- tile_id
    inspect_cell()
}

proc inspect_cell() -> void {
    val score <- north_contrast()
    if (score >= 12i64) { behave @flag(@row, @col, score) }
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

<a id="a-focused-grid-gives-the-rules-a-reference-semantics"></a>

## Run the same detector on raw pixels and on a filtered image

An inspector clicks a flagged pixel in the defect viewer. The viewer must explain whether the
vertical rise is already visible in the raw acquisition or appears only after the horizontal
filter. Show both scores immediately for that pixel. Overnight, the batch job must use the
same detector on every tile and publish the filtered flags. The viewer has one requested
position. The batch job can afford to compute and keep an entire intermediate image.

The detector reads its input here, reads that same input one row north, and subtracts.
On the raw image the middle score is `8 − 2 = 6`. On the filtered image it is
`(8 + 13) − (2 + 4) = 15`. With the threshold at 12, only the filtered result flags that
pixel. Passing the detector the already-computed middle sum, 21, loses the northern sum. Several images can have 21 in the middle and different sums one row north. Giving it a way to sample
its input at either position lets the application change the input while keeping the detector.
The test for each implementation is whether another team can supply a new field without
editing the detector.

The three listings implement both choices on demand. Each starts with the same stable image,
zero-based (row, column) positions and signed 64-bit samples. All intermediate values fit.
`east` and `north` clamp at the physical edge of this image. In CoD each returns a new row or column, and the helper binds that coordinate for the sample.
In Haskell and C++ each returns the updated position. The application's
pixel reader supplies the same table in each listing.

<a id="recompute-the-northern-score-in-cod"></a>

### CoD lends each rule a sampling operation

A rule can ask for a value twice and choose the focus for its second request. The image library
offers two such moves. Each helper rebinds one coordinate for the duration of the supplied
`here` callback. The rule author can then write the arithmetic directly.

```cod
val symbol @row:u64
val symbol @col:u64
symbol @pixel:proc() -> i64

proc at_east(here sample() -> i64) -> i64 {
    effect @col <- east(@col)
    return sample()
}

proc at_north(here sample() -> i64) -> i64 {
    effect @row <- north(@row)
    return sample()
}

proc east_sum(here sample() -> i64) -> i64 {
    return sample() + at_east(sample)
}

proc north_difference(here sample() -> i64) -> i64 {
    return sample() - at_north(sample)
}
```

The viewer selects the source in two ordinary procedures. The report's `inspect_cell` above
calls the same `north_contrast` as the viewer.

```cod
proc raw_contrast() -> i64 {
    return north_difference() { behave @pixel() }
}

proc north_contrast() -> i64 {
    return north_difference() { east_sum() { behave @pixel() } }
}

effect @row <- 1u64
effect @col <- 1u64
effect @pixel() -> i64 { return fixture_at(@row, @col) }

val raw_score <- raw_contrast()
val filtered_score <- north_contrast()
```

For the detector's second sample, `at_north` invokes the body with the north row already
selected. The eastward read in `east_sum` then reaches the northeast pixel, 4.
Returning from both helpers leaves the viewer at (1,1). Forwarding `sample` preserves its
lexical origin and lets it see the dynamic bindings at invocation. The image owner must stay
live throughout. The application lends work to the pipeline for this call and can install
a different reader on the next inspection. [Synchronous calls and dynamic bindings](../design/CallableStorageAndHereAccess.md#an-effect-in-a-here-callback)
· [Single-brace callback results](../design/CallableStorageAndHereAccess.md#single-brace-bodies-for-a-here-callable)
· [Current and replayed bindings](../design/SymbolicDynamicScope.md#captured-values-and-call-behavior)

### Haskell passes a focused field

The `comonad` library already supplies the focused field. `extract` reads at its focus and
`peeks north` samples one row north. `extend eastSum` makes the horizontal filter's result
available at *any* focus, keeping the caller's current position. The two branches of
`scoresAt` show exactly which field the detector receives.

```haskell
import Control.Comonad (extend, extract)
import Control.Comonad.Trans.Store (Store, peeks, store)
import Data.Int (Int64)

type Pos = (Int, Int)

eastSum, northDifference :: Store Pos Int64 -> Int64
eastSum w         = extract w + peeks east w
northDifference w = extract w - peeks north w

scoresAt :: (Pos -> Int64) -> Pos -> (Int64, Int64)
scoresAt pixel p =
  let raw = store pixel p
  in (northDifference raw, northDifference (extend eastSum raw))
```

Calling `scoresAt pixel (1,1)` gives `(6,15)`. The type `Store Pos Int64` is the same on
both sides of the filter. A third author can name the filtered view, pass it to another
detector or focus it elsewhere using the existing library. The library leaves the choice of caching to the application. [Store and its sampling operations](https://hackage-content.haskell.org/package/comonad-5.0.10/docs/Control-Comonad-Trans-Store.html)
· [Control.Comonad](https://hackage-content.haskell.org/package/comonad-5.0.10/docs/Control-Comonad.html)

### C++20 passes the position to a sampler

A pixel reader that accepts a `Pos` is enough. The application can give the detector a
lambda that computes a horizontal sum at the position it receives. Both kernels accept
either kind of reader. Here `Pos` holds a row and a column.

```cpp
std::int64_t east_sum(auto const& sample, Pos p) {
    return sample(p) + sample(east(p));
}

std::int64_t north_difference(auto const& sample, Pos p) {
    return sample(p) - sample(north(p));
}

auto scores_at(auto const& pixel, Pos p) {
    auto sums = [&](Pos q) { return east_sum(pixel, q); };
    return std::pair{north_difference(pixel, p), north_difference(sums, p)};
}
```

`scores_at(pixel, Pos{1, 1})` also gives `(6,15)`. The lambda computes a horizontal sum at whichever `q` the detector requests. Its reference
to `pixel` remains live through both synchronous calls. The caller needs neither an
intermediate array nor a class hierarchy. [C++ lambda captures](https://eel.is/c++draft/expr.prim.lambda.capture)

### Which code would the next author want to read?

| Change in the inspection service | CoD | Haskell | C++20 |
| --- | --- | --- | --- |
| Review the detector's arithmetic | `sample() - at_north(sample)` reads like the local rule. Open `at_north` to see the scoped coordinate change. | `extract w - peeks north w` shows both reads on one named field. Learning `extract` and `peeks` buys that brevity. | `sample(p) - sample(north(p))` shows both positions on one line. Each rule carries `p`. |
| Put the filter before the detector | Nest the two trailing bodies. `behave @pixel()` identifies the leaf and asks it to read at the active focus. | `northDifference (extend eastSum raw)` names both stages. The library supplies the adapter. | Name `sums` with a one-line lambda. The forwarded position is visible at the connection. |
| Use a precomputed horizontal field | Supply `north_difference() { cached_sum() }`, where `cached_sum` reads at `@row, @col`. | Supply `northDifference (store cachedSum p)`. | Supply `north_difference(cached_sum, p)`. |

For this two-stage detector, Haskell has the clearest expression for inserting another filter. The
reader can follow the field through `extend` without inspecting a callback body. C++ has
the most transparent account of *which coordinate* reaches the pixel reader. CoD keeps those
coordinates out of both pieces of arithmetic, at the cost of following the nested bodies to
the active pixel binding. If the team needs a filtered view it can return today and inspect
later, Haskell already has a value for it. The CoD `here` body shown here is synchronous.
A CoD interface for a reusable, retained view would also have to express the backing owner's
lifetime and what happens when the image is replaced. [Callable lifetime](../design/CallableStorageAndHereAccess.md#what-an-invocation-needs)
· [Replacing a region owner](CoarseGrainedOwnership.md#owner-identity-target-identity-and-representation)

<a id="three-ways-to-get-the-right-looking-program-wrong"></a>

### What goes wrong in the viewer and the batch job?

| Change one condition | What the inspector sees | Consequence for the implementation |
| --- | --- | --- |
| Replay the pixel handler installed at (1,1) | Replacing `behave` with `perform` in the filtered entry reads 8 four times. The score becomes `16 − 16 = 0`. | The pixel reader must follow the focus at the time of each request. |
| Start a tile at the middle row | Clamping at the tile seam substitutes 8 and 13 for 2 and 4. The middle score becomes 0 and the flag disappears. | Read the northern border from the neighbouring tile of the same revision. If unavailable, report a fault or mark the result provisional under an explicit policy. |
| Change the physical boundary rule | At the top right, clamping gives `(4 + 4) − (4 + 4) = 0`. Wrapping instead gives `(4 + 1) − (19 + 7) = −21`. | Apply the boundary rule at each move. At the right edge, clamped east followed by west need not return to the initial position. |
| Write horizontal sums back into the source, right to left | In the middle row the eastern cell has already become 26, so the centre becomes 34. Its northern counterpart becomes 10. The reported score is 24. | Use a stable input revision for the pass. A read-only access path alone cannot prevent another admitted writer from changing the pixels. |

A cache also needs a correct account of *which* pixels can change a score. The middle output
depends on the middle, east, north and northeast pixels. Change northeast from 4 to 5 and the
score falls to 14. Change only southwest from 7 to 999 and it stays 15. For a composite rule,
take the union of the first stage's dependencies at each position the second stage samples.
That gives the tile team a border to prefetch and the viewer an invalidation test. Checking
that `@pixel` is installed answers a different question. Whether CoD should express such a
footprint in a library contract or infer it remains open. [Ypnos and structured grids](https://www.cs.kent.ac.uk/people/staff/dao7/publ/ypnos1.html)
· [Petricek, Orchard and Mycroft, *Coeffects*](https://tomasp.net/academic/papers/structural/)

### Can the batch job cache the first stage?

For a stable frame, calculating each horizontal sum once into a second buffer and computing
it afresh on request give the same scores at every output position. On this fixture the
filtered rows are `[0,0,0]`, `[8,15,18]` and `[7,9,12]` with either strategy.
The bottom-right score is exactly on the inclusive flag threshold. The comparison changes
if the first stage itself publishes an event every time it runs.

| Requested output | Materialize the entire horizontal field once | Recompute each requested horizontal sum |
| --- | ---: | ---: |
| Only the middle cell in the viewer | 9 evaluations | 2 evaluations |
| All nine cells in the batch, literal two-call detector | 9 evaluations | 18 evaluations |

These counts refer to calls of `east_sum`, including two calls at the top edge when clamping
selects the same position. An observer of first-stage visits can distinguish the schedules.
There is a separate failure mode for the defect report. If scoring emits a flag, asking twice
for the same flagged score can stage two copies on recomputation and one if the second request reuses the first result. Accepting the tile publishes whatever was staged. For one flag per defect,
publish at the report boundary with a key such as (revision, position, rule). Keep an audit of
physical evaluations separately if the service promises it. [Retry and selective publication](#composing-retry-and-publication)

The programming question that remains for CoD is which part of this contract a reusable
library can put in its interface. A synchronous supplier suffices for the viewer and can read
the batch job's cache. Allowing the library itself to retain a sampler, change its schedule
or suppress duplicate requests requires evidence about lifetime, stable inputs and observable
effects. The [derivation in Appendix A](#appendix-a) states exactly what justifies exchanging
the two numerical implementations.

## What would settle the design questions?

| Question for CoD | Evidence that would change the conclusion |
| --- | --- |
| Can unrelated effects pass a new scope without pair-specific source changes? | Give the flag author and the tiling author only the published interfaces. List changed declarations, generated forwarding, reachable binding checks, intentionally unsupported pairs and explicit policy adapters separately. A hidden central switch over all old effects counts as an edit. |
| Can a library expose a retained, reusable CoD field? | The synchronous `here` example already gives 6 and 15 with one detector. For a proposed retained interface, return the derived field, sample it later under the same live owner, and check that an invalidated old view cannot be used after owner replacement. Exercise a missing border and restoration after a checked fault. |
| Which effectful kernels can share the pure composition law? | Specify observations first. Count both stage evaluations and committed flags. An equality that preserves the score and changes a promised diagnostic trace fails for that interface. |
| What does the checker need to know about a neighbourhood? | Show that the footprint covers the northeast dependency, handles the tile seam and respects the boundary policy. Decide separately if conservative over-approximation is acceptable for prefetch, permission checking or invalidation. |

A scope can pass an independently named operation through when it has no policy for that
operation. The viewer shows another form of reuse. A local detector can read different fields through one
synchronous sampling contract. To justify new library machinery, a
proposal must say what further program it admits, which owner keeps its input valid and
which observations a new evaluation schedule preserves.

<a id="appendix-a"></a>

## Appendix A. Why the two schedules agree on values

### Model and observations

Let $P$ be the finite, nonempty set of valid positions in one image revision. Fix total
boundary-aware maps $e,n:P\to P$ for east and north. A focused field of values of type $A$
is a Store $W A=(P\to A)\times P$. Write $w=(v,p)$ with lookup $v$ and selected position
$p$. Two such fields are equal when they have the same focus and give equal values at every
position. Refocusing at a valid $q$ gives $(v,q)$. Each move is defined on $P$, including at its edge, so the model admits clamping.

For a total, extensional rule $f:W A\to B$, define

$$
\operatorname{extract}(v,p)=v(p),\qquad
E_f(v,p)=\bigl(q\mapsto f(v,q),\,p\bigr).
$$

Here $f(v,q)$ abbreviates $f((v,q))$ and $E_f$ is `extend f`. The observations in the
proof are the focus and the field's returned values. Repeated reads must see the same live
revision, the same boundary policy and the same answers. Assume defined arithmetic, normal
completion and no observable effects from evaluating the rules. [Comonad class and laws](https://hackage-content.haskell.org/package/comonad-5.0.10/docs/Control-Comonad.html)
· [Uustalu and Vene, *The Essence of Dataflow Programming*](https://doi.org/10.1007/11575467_2)

### Three laws, derived at a position

1. **Read the result at the starting focus.** Let $h(q)=f(v,q)$. Then
   $\operatorname{extract}(E_f(v,p))=\operatorname{extract}(h,p)=h(p)=f(v,p)$.
   An application asking for one filtered value can run the local rule at that position.
2. **Use the pixel read itself as the rule.** For every $q$,
   $\operatorname{extract}(v,q)=v(q)$. Thus
   $E_{\operatorname{extract}}(v,p)=(q\mapsto v(q),p)=(v,p)$ by pointwise equality.
   This is the identity stage for the viewer's raw mode.
3. **Regroup two successive rules.** Take $g:W B\to C$ and the same $h$ as in (1).
   Applying $g$ across the derived field gives $E_g(E_f(v,p))=(q\mapsto g(h,q),p)$.
   For the other grouping define $k(u)=g(E_f(u))$. At any $q$ we have $E_f(v,q)=(h,q)$,
   hence $k(v,q)=g(h,q)$. It follows that $E_k(v,p)=(q\mapsto g(h,q),p)$ as well.
   The lookups agree at every position and both results retain $p$.

### Corollary for the inspection service

Under the CoD installation in the listing, `behave @pixel()` at $q$ reads $v(q)$.
`at_east` and `at_north` invoke the supplied body at $e(q)$ and $n(q)$ respectively,
then restore the caller's focus. The horizontal field is therefore
$h(q)=v(q)+v(e(q))$. Applying the unchanged detector at $p$ gives $v(p)-v(n(p))$ for the
raw reader and $h(p)-h(n(p))$ for the filtered reader. Haskell's `peeks` and C++'s explicit
position arguments denote these same lookups.

Filling a separate table $H$ once sets $H(q)=h(q)$ for every $q$. A reader that recomputes
$h(q)$ answers identically. By pointwise equality, either field can be supplied to an
extensional second rule at any requested position. In particular their scores and the set
of positions passing the fixed threshold agree. This argument supplies the cache
substitution. The third law additionally justifies regrouping when another stage is added.
Choosing `perform` in the CoD sample changes the lookup premise. Changing the pixels in place removes the single $v$ on which the proof depends.

### What an effectful extension would have to prove

For an effectful kernel $f:W A\to M B$, lifting the kernel pointwise produces $W(M B)$.
A subsequent kernel $g:W B\to M C$ requires a field of *values*. One theoretical route is
a distributive law $W(M B)\to M(W B)$ compatible with the monad's unit and multiplication
and the comonad's counit and duplication. Its existence and the observations it preserves
must be established for the chosen pair and policy. In particular, a promised event for
each actual evaluation distinguishes the nine-visit table from the eighteen-visit full
recomputation. The three equations proved above concern returned values and supply no
equality of those histories. [Effectful context-dependent computation](https://doi.org/10.1007/11575467_2)
