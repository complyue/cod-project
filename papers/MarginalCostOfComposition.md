<a id="extending-an-effect-vocabulary-and-refocusing-a-raster-computation"></a>
<a id="symbolic-effect-composition-and-comonadic-refocusing-in-cod"></a>

# The Marginal Cost of Composition — Independent Extension and Local Reasoning in CoD

*Working paper · companion developer guide: [Facing Complexity with Scoped Effect Composition](../docs/ScopedEffectComposition.md).*

## Abstract

A production inspection line already works. What must its application author change, and whose code must they understand, to run a candidate detector beside it? We study the *marginal cost* of adding an independently developed feature to a functioning composition. We count changes to domain rules, scopes and integration code, and identify the facts an author needs to predict an observation. CoD offers shared vocabulary modules, lexically identified operations, dynamically scoped interpretations and statically checked requirements. For bounded definitions of $M$ operations and $N$ transparent scopes, the routing argument gives an $O(M+N)$ bound on authored source. $K$ independent, bounded interaction rules add $O(K)$. A separate Store argument establishes when a detector can reuse or recompute a sampled field without changing its values. Neither calculation measures developer effort. A shadow rollout below gives both arguments a common job and specifies what an author study would have to measure.

## The change request

> Run the candidate on the same exposure as the production detector. Record disagreements. Only the production detector may cause equipment to reject a part.

The detector team knows the image rule. The deployment team owns the equipment and the shadow log. Both select the same nominal request from an inspection vocabulary. Give the detectors one acquired sample of 80 for part 42. A production threshold of 90 passes it. A candidate threshold of 70 requests rejection. With a recording adapter at each exit, the expected observations are **one acquisition, zero production reject commands, one shadow “would reject” entry**. The [developer exercise](../docs/ScopedEffectComposition.md#wire-production-and-shadow-to-one-acquisition) derives that trace. Removing the shadow installation alone leaves a compatible production handler and changes the trace to **1 / 1 / 0**. This is a small test of a large promise: type-correct wiring can still implement the wrong deployment.

Two teams now make independent changes. One adds a neighbourhood detector that explains a finding and can request manual inspection. The other adds shadow execution to an existing pipeline. Sharing an acquired frame and its calibration lets each model read the same evidence. An extra lookup across a tile boundary must not spend another exposure. A physical retry *does* spend one. Sending a manual-inspection request from the shadow run to a real work-order system requires a product decision. Charge that decision to the composition. A scheme that makes every unrelated tile or camera helper forward the new request charges the feature for considerably more.

## What the application author has to know

| Author | A sufficient interface for this change | Evidence that would undermine the claim |
| --- | --- | --- |
| Detector author | The sampling contract identifies the frame revision, focus and boundary policy. The vocabulary states what a request means. | To implement a new rule, the author must inspect the camera retry stack, the device driver or the list of shadow deployments. |
| Application integrator | The production and shadow entries state who supplies each effect, what it may reach, and which observations survive failure. | To predict whether a work order opens, the integrator must read unadvertised dependencies inside an otherwise compatible handler. |
| Library author | A transparent layer admits its body once, preserves required resources and restores relevant bindings. Exceptional policies identify their affected operations. | A new operation requires adapters in every old transparent layer or equivalent edits to a central registry. |

The [routing argument](#counting-the-things-authors-must-supply) explains why some of that knowledge can remain local. Fix an operation's nominal identity and the bindings its interpretation actually depends on. Passing it through a once-only scope that preserves those bindings, target lifetimes and observable resources gives its author the same interpretation. The shared lookup rule covers another transparent scope by the same reasoning. A retry that invokes the body twice, a handler that writes to shared state, and a deployment that changes the meaning of rejection each require their own account. Counting their independent policies explicitly prevents a short client program from hiding the cost in its assembler.

The [raster fixture](#run-the-same-detector-on-raw-pixels-and-on-a-filtered-image) asks what the detector author can assume at a neighbour. The middle cell scores 6 on the raw field and 15 on the horizontally filtered field. On a stable frame with pure sampling, a cached and a recomputed filtered field agree on those scores. An audit promised for each actual evaluation can distinguish the schedules. Thus an application may share the sample contract across models while keeping physical exposure, rule evaluation, proposed rejection and issued command as separate observations. The [Store proof](#appendix-a) justifies the value substitution for its stated fragment.

## Lexical identity, dynamic interpretation, static checking

A dedicated vocabulary module can publish a typed procedure symbol before any implementation is installed. The detector can select that identity with `symbol from`. The integration can select or supply a method family and install it for an execution scope. Independently declaring a symbol with the same spelling and signature gives a different identity. [Shared identities](../design/ModuleWorlds.md#vocabulary-modules-establish-shared-identities) · [Selecting a procedure family](../design/ModuleWorlds.md#selecting-a-symbolic-procedure-family)

For a **procedure symbol**, a bare `@op(x)` calls the lexically, statically selected family, with ordinary method dispatch inside it. A dynamic replacement for that symbol cannot intercept the bare call. Both `behave @op(x)` and `perform @op(x)` call the currently installed handler. `behave` uses the dynamic bindings effective at the call. `perform` replays the required bindings captured **at that installation**. The same source definition installed twice can capture different destinations. Restoring a binding on exit preserves writes already made through it. A captured reference still depends on its actual owner. The pre-2.0 checking contract uses the union of outstanding requirements in the selected family after local supply is accounted for. A missing required installation is grounds for rejection. The choice between two available, compatible business interpretations remains visible in the trace. [Call forms](../design/SymbolicDynamicScope.md#lexical-selection-installation-and-invocation) · [Checking boundary](../design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20) · [Audit trace](examples/ScopedTileAudit.md#contract-and-observations)

## Dependency injection, inversion of control and aspects

A composition root in a dependency-injection design can supply a camera, a reporting capability and a device adapter. Inversion of control also covers the framework's decision about *when* to call the detector. The engineering comparison concerns how a deployment changes one interpretation down a call chain, how the program checks the supply, and how much wiring each author must inspect. Compile-time DI and statically typed dynamic binding both have precedents. Use an idiomatic explicit-capability or labeled-effect implementation with scoped override as a baseline. Include its setup, and count CoD's registrations and required installations on the same terms. [Fowler on dependency injection and IoC](https://martinfowler.com/articles/injection.html) · [Dagger compile-time validation](https://dagger.dev/dev-guide/basic-usage) · [Lewis et al., *Implicit Parameters: Dynamic Scoping with Static Types*](https://launchbury.blog/wp-content/uploads/2019/01/implicit-paramters.pdf)

For aspect-oriented programming, the vocabulary offers an explicit **domain join point**. “Reject requested” belongs at the detector's policy boundary. “Reject command issued” belongs at the device adapter. An audit can interpret the first operation across independently written detectors and see one request in the shadow fixture. An audit of the second sees zero commands. A direct driver call bypasses the request boundary. Claiming coverage of every device command therefore requires control over the actual driver entry points. Wrapping audit and retry also requires a delegation and ordering contract. A bounded `here` body can supply the next step. Calling `behave @op` again from a replacement can re-enter it, and `perform` replays installation-time bindings. Neither verb implicitly advances an advice chain. Compare the cost of providing and maintaining this explicit boundary with AspectJ pointcuts and with typed join point interfaces that support explicit announcement. [AspectJ join points and advice](https://eclipse.dev/aspectj/doc/latest/progguide/semantics.html) · [Bodden, Tanter and Inostroza, *Join Point Interfaces*](https://www.bodden.de/pubs/bti13jpi.pdf)

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
three. The [three business transformers](#ship-three-business-transformers) measure the cost
of integration. The retry experiment below measures the observable effect of changing a scope.
[Warehouse policy and its executor](AlgebraicEffectsIsAllYouNeed.md#two-entries-share-one-fallback)

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

enum StageScope:u8 { PerAcquisition, PerTile }

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

### Ship three business transformers

The camera driver, calibration library and defect reporter are released by different teams.
Each team supplies a capability that client code can request, a transformer that implements
it, and a runner. The application must use all three capabilities in one inspection. What
must the authors supply between the first working package and the assembled application?

| Team | Public methods | What the team's own transformer does |
| --- | --- | --- |
| Camera | `acquire :: m (Maybe Int)` | `CameraT` spends one exposure credit and issues the next serial together. An empty budget yields `Nothing` and leaves the serial unchanged. |
| Calibration | `gain :: m Int` and `withGain :: Int -> m a -> m a` | `GainT` supplies a calibration and overrides it for the duration of a caller's computation. |
| Defect report | `flag :: Flag -> m ()` and `publishIf :: (a -> Bool) -> m a -> m a` | `ReportT` collects ordered (serial, gain) pairs. A protected computation returns its result in either case. Its flags reach the enclosing report only if the predicate accepts that result. |

The three public types are new transformers. They reuse strict `StateT`, `ReaderT` and
strict `WriterT` privately for storage, environment and ordered output. That reuse pays
for the underlying monad machinery. The assembled client must also be able to
ask for `MonadReport` through `CameraT`, even though a camera by itself knows nothing
about publishing a report. `mtl` supplies no instances for these domain classes. The listing uses its class-and-instance
pattern. Its imports need only `base` and `transformers` because the teams define all three classes. [Complete Haskell program](examples/ThreeBusinessTransformers.hs)

Begin with serial 1, two exposure credits and normal gain 3. Make one acquisition at gain 7,
reject it, then accept an acquisition at normal gain. The inspection has one definition for
every stack. Here is the body at the centre of the comparison.

```haskell
attempt :: (MonadCamera m, MonadGain m, MonadReport m) => Bool -> m Bool
attempt accepted = publishIf id $ do
  acquired <- acquire
  case acquired of
    Nothing -> pure False
    Just serial -> do
      sampled <- gain
      flag (serial, sampled)
      pure accepted
```

The caller runs `withGain 7 (attempt False)` followed by `attempt True` and reads `gain`
again. Both credits have been spent, the next serial is 3, the visible gain is 3, and the
published report contains only `[(2,3)]`. Accept the first attempt as a control and the report
must contain `[(1,7),(2,3)]` in order. Without this control a broken `withGain` that ignored
the override could pass the rejection example. Reverse the decisions and the final report must
keep `[(1,7)]`, even though the second attempt staged `(2,3)` before it was rejected. The full
source also covers exhaustion before the second shot and before the first.

### What one application has to write

Fix the order `CameraT (GainT (ReportT Identity))`, outermost first. Start at the innermost
layer and ship outward. Each row gives the *additional* source in the direct `mtl`-style
encoding. A native instance implements the layer's own class. A crossing exposes an inner
capability on the new outer type.

| Release | New native instance | New crossings required by the client | New instance heads | New method bindings |
| --- | --- | --- | ---: | ---: |
| Report on its own | `MonadReport (ReportT m)` | None | 1 | 2 |
| Put calibration over report | `MonadGain (GainT m)` | Report through `GainT` | 2 | 4 |
| Put camera over both | `MonadCamera (CameraT m)` | Gain through `CameraT`; report through `CameraT` | 3 | 5 |
| **One delivered stack** | **3 native** | **3 crossings** | **6** | **11 = 5 native + 6 forwarding** |

At the final step a call to `flag` travels through camera and calibration to reach the reporter.
The app keeps the calibration team's existing report bridge. It adds one bridge from report to camera, irrespective of
how many layers separate the two. Omit that last bridge and `inspection` still has a valid
polymorphic definition. Specializing it to `Application` now lacks `MonadReport (CameraT …)`.
The application author can point to the call and the missing instance in the following assembly.

```haskell
type Application = CameraT (GainT (ReportT Identity))

runApplication :: CameraState -> Application a -> ((a, CameraState), [Flag])
runApplication s = runIdentity . runReport . runGain 3 . runCamera s

-- Reject the first report, then accept the second.
example = runApplication (CameraState 1 2) (inspection (False, True))
```

One of the new camera instances illustrates the difference between forwarding a request
and forwarding a scope.

```haskell
instance MonadReport m => MonadReport (CameraT m) where
  flag = lift . flag
  publishIf ok (CameraT action) = CameraT $ StateT $ \s ->
    publishIf (ok . fst) (runStateT action s)
```

`flag` lifts an action that already runs in the inner monad. The body handed to `publishIf`
still needs an initial camera state. Running it returns `(result, finalCamera)`.
The inner reporter must apply the approval predicate to `result` and pass the whole pair back.
Resetting the camera on rejection would let the next shot reuse serial 1. Keeping all the
flags would leak `(1,7)`. The supplied body needs more than `lift`. The gain crossing can reuse
`mapStateT (withGain n)` to carry its scoped action through the camera state.
The fixed stack's six forwarding methods consist of three ordinary lifts, two library
mappings and the result projection above.

### What changes when a second application wants another order?

Each cell below names an implementation in the companion source. **F** marks a crossing
needed by the first application. **R** marks one added for the remaining orders.
Rows count three independently authored interfaces ($M$). Columns count three public
transformer constructors ($N$). Counting individual method bindings gives a different unit.

| Capability requested | Outer `CameraT` | Outer `GainT` | Outer `ReportT` |
| --- | --- | --- | --- |
| `MonadCamera` | Native acquisition | R · lift `acquire` | R · lift `acquire` |
| `MonadGain` | F · lift `gain`, map `withGain` over state | Native calibration | R · lift `gain`, map `withGain` over writer output |
| `MonadReport` | F · lift `flag`, project `fst` for `publishIf` | F · lift `flag`, map `publishIf` over the reader | Native publication |

Supporting all six orders adds **three** instance heads and **four** method bindings to the
fixed stack. Across the library the total is **nine heads, fifteen method bindings**.
Of those, six heads and ten bindings provide crossings. Six of the ten bindings are plain
lifts, three reuse a carrier mapping, and one adjusts the result predicate. The six application
orders share these definitions. The fixture peels each stack with the three existing runners
and normalizes the placement of final camera state and report so that every order faces the
same observer. Agreement is a business requirement for *these* packages. The earlier
retry/publication experiment deliberately requires different observations for its two scope nestings.

| Authored work beyond the method bodies | One production order | Order-independent library and its example clients |
| --- | --- | --- |
| Public contracts and package machinery | 3 capability classes with 5 signatures; 3 newtypes; 3 `MonadTrans` lifts; 3 domain runners. Each newtype derives `Functor`, `Applicative` and `Monad`. | Reuse the same contracts, types, lift definitions and runners. |
| Application and verification | One stack alias and runner composition for the production order; one acquisition rule, one two-attempt inspection and its expected observations. | Exercise the same inspection through 6 runner compositions; normalize the 2 result shapes. |
| Package integration | Deliver 3 cross-layer instances along with the 3 native instances. | Deliver all 6 cross-layer instances with the same 3 natives. An integration module can supply the bridge source when the domain packages are frozen. |

The companion's `main` specifies five business cases across all six orderings. Its entry point is
`runghc papers/examples/ThreeBusinessTransformers.hs` with `transformers` available. The
source keeps the declarations together so readers can inspect each implementation. In separately released Haskell packages, putting a cross-instance in a module owning
neither class nor outer type makes it an orphan. Publish one consistent instance set or
introduce an integrating newtype. A library can also reuse default methods, deriving or
a generic lifting construction wherever the required semantics fit. Credit the shared
definition once and count any remaining opt-ins. A single generic `MonadReport (StateT s m)` instance, for example, could let
multiple domain newtypes derive their report bridge from the same representation, where that
policy fits. A `Ports m` record is a legitimate
application-specific alternative. Count its construction and the scoped operation adapters
that the application actually needs. [Haskell program and runners](examples/ThreeBusinessTransformers.hs)
· [MonadTrans and lifting](https://hackage-content.haskell.org/package/transformers-0.6.2.0/docs/Control-Monad-Trans-Class.html)
· [Jaskelioff, *Modular Monad Transformers*](https://www.fceia.unr.edu.ar/~mauro/pubs/mmt/mmt.pdf)
· [Affeldt and Nowak on the hypotheses of modular lifting](https://doi.org/10.4230/LIPIcs.TYPES.2020.2)

With $n$ such packages, a fixed stack whose leaf uses all capabilities has $n$ native
instances and $n(n-1)/2$ distinct crossings in the direct pairwise encoding. Supporting
every order covers $n(n-1)$ directed crossings plus the $n$ natives. These counts describe
the published interface and coding choice. An application using standard `ReaderT`, `StateT`
and `ExceptT` already receives their `mtl` instances from the library. An application whose requirements fit those existing interfaces can ship against them
with zero new instances. The library sources show the work that was amortized.
[Reader instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/src/Control.Monad.Reader.Class.html)
· [State instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/src/Control.Monad.State.Class.html)
· [Error instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/src/Control.Monad.Error.Class.html)

### What CoD pays for the same inspection

In CoD the camera provider can implement `acquire` against the current serial and credit
cells. Calibration supplies the current gain and rebinds that value for a call. The reporter
appends to the current journal and changes the journal destination while a result is provisional.
For the Boolean verdicts in this inspection the two scope bodies have the following shapes.
The enclosing caller supplies the business leaf and the initial bindings.

```cod
proc with_gain(here body() -> bool, value:u64) -> bool {
    effect @gain <- value
    return body()
}

proc publish_if_accepted(here body() -> bool) -> bool {
    val draft <- @report
    val accepted <- {{
        effect @report = draft
        body()
    }}
    if (accepted) { @report <- draft }
    return accepted
}
```

Here `@gain` is a bits value symbol. `@report` is a writable reference symbol bound to an
owned cell holding the fixture's bounded journal as bits. The draft copies the parent's
current contents, so acceptance retains the prefix. Camera cells remain live and writable
under both scopes. The caller can run `with_gain(7u64) { publish_if_accepted() { shot(false) } }`,
then `publish_if_accepted() { shot(true) }` at restored gain 3. Here `shot` acquires if credit
is available, appends (serial, gain) on acquisition and returns its supplied verdict, or
returns false on exhaustion. These steps give the same camera, verdict and journal trace.
The Haskell `publishIf` accepts a predicate on arbitrary results. This CoD excerpt covers
the Boolean protocol used by the inspection. [Concrete CoD journal cells and scoped publication](examples/ComposedTileReport.md#cod)

CoD still needs three business providers, five operations including the two scopes, live
storage, domain initializers and an application entry. The scoped bindings carry the other
providers through both scope bodies with **zero per-pair forwarding declarations** for this
trace. This count compares routing source. It assigns real work to credit accounting, gain
restoration and conditional publication on both sides. Refunding a rejected exposure, for
example, would change the camera contract and require explicit coordination. The
[additive bound](#additive-source-extension) specifies which transparent policies generalize.

### Add a row, then a column

The three-package experiment grows both axes together. To vary them independently, freeze the
nine-cell library. Add only a `MonadAudit` interface with an operation recording each physical
exposure in a base audit sink. Trace it through the three existing outer constructors. Next
add only an outer `TileT` execution layer. Check which of the four interfaces it must expose,
including whether an acquisition in a rejected tile still charges credit, survives in the
audit, and disappears from the public defect report.

The fully supported rectangle has grown from three by three to four by four. It adds three
candidate routes for the new audit interface, three for existing interfaces through `TileT`,
and their new intersection, **seven cells**. In general one row and one column add $M+N+1$
cells to a full $M\times N$ rectangle. Record the provider, shared default, explicit instance
or deliberately unsupported combination in each. One reusable rule can cover several cells.
An independently chosen policy for every cell still costs information somewhere.

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
| `mtl` with standard transformers | Use the supplied error and state instances for the caught-fault control. For the three business packages above, also account for derivation and projecting services out of one reader environment. Record which functions still need a scoped interpretation and whether the supplied failure semantics match the requested report. |
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

The first application study should freeze the detector interface, then give one author the neighbourhood rule and another the shadow deployment. Combine their changes in both orders. For each role, record initial setup, incremental edits, provider installations, policy decisions, generated inputs and the old modules opened to understand the change. Run the same release sequence with explicit capabilities and a labeled-effect implementation; for the audit boundary also compare typed join point interfaces. An ordinary detector may cost the same in each. Give a reader the public interfaces and wiring, ask for the three expected counts, and time a correct diagnosis of a deliberately misrouted request. Report prediction errors and rework separately from source counts.

The shadow observations in this draft are derived for the stated fixture. Comparable implementations, participant measurements and a full evaluation of aspect coverage belong to the next revision. The conditional routing bound and the pure Store derivation below establish narrower properties that those experiments can use as controls.

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
