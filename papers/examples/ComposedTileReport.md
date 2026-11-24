<a id="composing-retry-and-publication-in-a-tile-inspector"></a>

# Composing three states and five writers in a tile inspector

## A scalar setting is enough for a small composition

Two calibration policies modify the cutoff for work carried out in their scope. One
adds a margin of four. The other doubles the cutoff. A reading of 12 clears a cutoff
of 10 but misses a cutoff of 14. One scoped scalar suffices to express the difference.
The listings use the project's requested explicit `val symbol` spelling for a bits
binding. The current design text spells this same binding `symbol @cutoff:u64`.
Both spellings here mean a copied value with scoped installation and restoration.
[Symbolic storage contract](../../design/SymbolicDynamicScope.md#symbolic-storage-and-regional-provenance)

```cod
val symbol @cutoff:u64

proc add_margin(here test() -> bool) -> bool {
    effect @cutoff <- @cutoff + 4u64
    return test()
}

proc double_cutoff(here test() -> bool) -> bool {
    effect @cutoff <- 2u64 * @cutoff
    return test()
}

effect @cutoff <- 3u64
val at_fourteen <- add_margin() { double_cutoff() { 12u64 >= @cutoff } }
val at_ten <- double_cutoff() { add_margin() { 12u64 >= @cutoff } }
assert at_fourteen == false
assert at_ten
assert @cutoff == 3u64
```

The first order tests against `2 × (3 + 4)`. The second tests against
`(2 × 3) + 4`. Neither policy had to know which other policy enclosed its test.
Both leave the caller's cutoff at 3.

## Which report is correct?

A tile may require a second physical acquisition. Three separately named State
channels govern capture. Five separately named Writer channels record different
business observations. All three states have the **same** unsigned scalar type.
All five writers have the **same** ordered-sequence meaning and numeric item type.
This forces the programs to show how an operation selects its intended channel.

| Channel | Initial value | Contract of one physical acquisition |
| --- | --- | --- |
| State `serial` | 0 | Increment, then use the new value as the acquisition ID. |
| State `credits` | 8 | Charge two credits. The charge survives a failed validation. |
| State `gain` | 3 | Sample the current gain, then advance it by one for the next acquisition. |
| Writer `acquired` | [] | Append the acquisition ID before validation. |
| Writer `rejects` | [] | Append the ID on rejection. |
| Writer `passes` | [] | Append the ID on acceptance. |
| Writer `defects` | [] | Append a provisional defect with the acquisition ID. Its publication follows the staging policy. |
| Writer `samples` | [] | Append the sampled gain as the evidence for that provisional defect. Stage it in the same batch. |

The validator rejects IDs up to and including `rejectThrough`. `retryOnce` retries an
initial rejection once. `stage` commits the two provisional outputs together only on
acceptance. The other three writers and all three states survive rejection. Compare
R(S(I)), a fresh stage around each acquisition, with S(R(I)), one stage around the
entire tile. Each row starts a fresh run. The state tuple is (serial, credits, gain).
The two rightmost cells each show (defects, samples).

| Rejected through | Final verdict and state | acquired | rejects | passes | R(S(I)) | S(R(I)) |
| ---: | --- | --- | --- | --- | --- | --- |
| 0 | Accept, (1, 6, 4) | [1] | [] | [1] | ([1], [3]) | ([1], [3]) |
| 1 | Accept, (2, 4, 5) | [1, 2] | [1] | [2] | **([2], [4])** | **([1, 2], [3, 4])** |
| 2 | Reject, (2, 4, 5) | [1, 2] | [1, 2] | [] | ([], []) | ([], []) |

The middle row distinguishes “evidence from accepted acquisitions” from “all evidence
that led to an accepted tile.” Checking only the verdict misses the difference. Checking
only the two provisional writers misses an implementation that rolls back the charge or
reissues the same acquisition number. The fixture models synchronous, normally returning
business verdicts on at most two acquisitions.

## CoD

The rejection threshold is a scoped value. The eight other channels need writes to
survive a returning procedure. Under the confirmed symbolic storage contract, a new
`effect` on an independent bits value restores the prior value on exit. A writable
`ref symbol` can instead select an ordinary lexical `val` as its target. A target write
survives the procedure's return. Staging needs just two extra lexical values with the
same representation. The fixture needs no owning region symbol or whole-region acquisition.
[Borrowing a lexical scalar through a symbol](../../design/SymbolicDynamicScope.md#owner-parameter-reference-symbols-and-implementation-cost)

For this small observation range, a sequence of digits from 1 through 9 can be encoded
in a `u64`: zero denotes empty, and appending digit `d` to `w` gives `10*w + d`.
The fixture emits at most two items per channel, plus one optional prefix digit. Thus
34 records [3, 4], and 43 records the opposite order. The business program only appends. The publication policy
can additionally seed and commit a private prefix. Real unbounded streams would need a
different representation.

```cod
val symbol @reject_through:u64
ref symbol @serial:u64 in O where {owner O}
ref symbol @credits:u64 in O where {owner O}
ref symbol @gain:u64 in O where {owner O}
ref symbol @acquired:u64 in O where {owner O}
ref symbol @rejects:u64 in O where {owner O}
ref symbol @passes:u64 in O where {owner O}
ref symbol @defects:u64 in O where {owner O}
ref symbol @samples:u64 in O where {owner O}

func tell(ref output:u64 in O, digit:u64) where {owner O} {
    output <- 10u64 * output + digit
}

proc inspect() -> bool {
    @serial <- @serial + 1u64
    @credits <- @credits - 2u64
    val id <- @serial
    val used_gain <- @gain
    @gain <- used_gain + 1u64
    tell(@acquired, id)
    tell(@defects, id)
    tell(@samples, used_gain)
    if (id <= @reject_through) {
        tell(@rejects, id)
        return false
    }
    tell(@passes, id)
    return true
}

func retry_once(here body() -> bool) -> bool {
    if (body()) { return true }
    return body()
}

proc stage(here body() -> bool) -> bool {
    val defects <- @defects
    val samples <- @samples
    val ok <- {{
        effect @defects = defects
        effect @samples = samples
        return body()
    }}
    if (ok) {
        @defects <- defects
        @samples <- samples
    }
    return ok
}

data Observation {
    accepted:bool; serial:u64; credits:u64; gain:u64;
    acquired:u64; rejects:u64; passes:u64; defects:u64; samples:u64;
}

proc run(ref out:Observation in O, per_acquisition:bool, reject_through:u64) where {owner O} {
    effect @reject_through <- reject_through
    effect @serial = out.serial
    effect @credits = out.credits
    effect @gain = out.gain
    effect @acquired = out.acquired
    effect @rejects = out.rejects
    effect @passes = out.passes
    effect @defects = out.defects
    effect @samples = out.samples
    if (per_acquisition) {
        out.accepted <- retry_once() { stage() { inspect() } }
    } else {
        out.accepted <- stage() { retry_once() { inspect() } }
    }
}

proc check_accepted_acquisition() {
    ref out = make Observation in here {
        accepted <- false; serial <- 0u64; credits <- 8u64; gain <- 3u64;
        acquired <- 0u64; rejects <- 0u64; passes <- 0u64;
        defects <- 0u64; samples <- 0u64;
    }
    run(out, true, 1u64)
    assert out.accepted
    assert out.serial == 2u64
    assert out.credits == 4u64
    assert out.gain == 5u64
    assert out.acquired == 12u64
    assert out.rejects == 1u64
    assert out.passes == 2u64
    assert out.defects == 2u64
    assert out.samples == 4u64
}
```

Each declaration of `O` binds a fresh owner parameter. The selected lexical target
must remain live wherever its binding can be used. The two local targets in `stage`
start with the enclosing writer prefixes. During the callback, the only admitted
business writes to these streams target those locals. Leaving the inner scope restores
the preceding reference bindings. On acceptance, replacing their contents publishes
exactly the accumulated suffixes after the original prefixes. On rejection, both
outer prefixes survive intact. Acquisition, diagnostics and pass counting continue
through their own bindings under either nesting.

For example, `run(out, true, 1u64)` records acceptance, state (2, 4, 5) and the five
encoded writer values (12, 1, 2, 2, 4). On a fresh observation, `run(out, false, 1u64)`
changes only the last two to (12, 34). As a prefix control, start the public defect and sample logs at 7 and 8.
The two accepted reports must then contain (72, 84) and (712, 834), respectively.
After two rejections both must still contain the original (7, 8).
This detects a commit that accidentally replaces an existing report with only the
latest acquisition.

## Haskell business code shared by both libraries

The two Haskell programs use the same inspector. Its ports give each of the eight roles
a name. Only the adapter for a library knows how to reach its chosen layer. The observer
names every returned state and journal, including the ones that survive a rejection.

```haskell
module Inspector where

import Data.Word (Word64)

type N = Word64
type Trace = [N]

data Ports m = Ports
  { nextSerial :: m N
  , charge :: m ()
  , takeGain :: m N
  , acquired, rejects, defects, samples, passes :: N -> m ()
  }

inspect :: Monad m => Ports m -> N -> m Bool
inspect p rejectThrough = do
  n <- nextSerial p
  charge p
  g <- takeGain p
  acquired p n
  defects p n
  samples p g
  let ok = n > rejectThrough
  if ok then passes p n else rejects p n
  pure ok

advance, takeOld :: N -> (N, N)
advance n = (n + 1, n + 1)
takeOld g = (g, g + 1)

pay :: N -> ((), N)
pay c = ((), c - 2)

retryOnce :: Monad m => m Bool -> m Bool
retryOnce body = do
  ok <- body
  if ok then pure True else body

data Observation = Observation
  { accepted :: Bool
  , serialCreditsGain :: (N, N, N)
  , acquiredRejectedPassesDefectsSamples :: (Trace, Trace, Trace, Trace, Trace)
  } deriving (Eq, Show)

-- Each of the eight eliminators below returns a pair with its own output.
observe ((((((((ok, ds), ss), as), rs), ps), n), c), g) =
  Observation ok (n, c, g) (as, rs, ps, ds, ss)
```

<a id="haskell-with-mtl"></a>

## Three StateT layers and five WriterT layers

Here is a concrete `mtl` stack. From outside in, the writers collect **defects, samples,
acquired, rejects, passes**. The states then hold **serial, credits, gain**. All three
states store the same scalar type. All five writers append to the same list type. An
unqualified `tell [n]` at the top chooses defects. Its argument cannot identify any of
the other four journals. Similarly, `mtl` can lift an unqualified state operation through
the writers to the serial state. Accessing the two further state instances needs a route
past that first instance.

```haskell
module Stacked where

import Control.Monad (when)
import Control.Monad.State.Strict (StateT, runStateT, state)
import Control.Monad.Trans.Class (lift)
import Control.Monad.Writer.Strict (WriterT(..), tell)
import Data.Functor.Identity (Identity, runIdentity)
import Inspector

type W = WriterT Trace
type S = StateT N
type States = S (S (S Identity))
type Stack = W (W (W (W (W States))))

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

stage :: Stack Bool -> Stack Bool
stage body = WriterT $ WriterT $ do
  ((ok, ds), ss) <- runWriterT (runWriterT body)
  let keep xs = if ok then xs else []
  pure ((ok, keep ds), keep ss)

run :: Bool -> N -> Observation
run = runWithPrefix False

runWithPrefix :: Bool -> Bool -> N -> Observation
runWithPrefix prefix perAcquisition k = observe $ runIdentity
  $ flip runStateT 3 $ flip runStateT 8 $ flip runStateT 0
  $ runWriterT $ runWriterT $ runWriterT $ runWriterT $ runWriterT
  $ do
    when prefix $ defects ports 7 >> samples ports 8
    if perAcquisition then retryOnce (stage (inspect ports k))
                      else stage (retryOnce (inspect ports k))
```

`stage` handles only the outer two writer outputs. Reading their complete batches and
retaining both or neither preserves earlier output in the enclosing scopes. The other
six operations run in the underlying stack throughout. Grouping the selected writers at
the top is a favorable layout for this policy. With the same underlying journal type,
interchanging positions can preserve type correctness and change the report. Keeping
the lift paths in `ports` confines that maintenance to the adapter. This fixture requires
no new `mtl` instances. An application needing just one combined state and output can
put three fields in one state and five fields in a product monoid; the eight-layer
workload here additionally asks how independently installed, same-typed layers are named
and selectively interpreted. [mtl state instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/Control-Monad-State-Strict.html) · [mtl writer instances](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/Control-Monad-Writer-Strict.html)

<a id="haskell-with-effectful"></a>

## The same inspector with labeled effectful handlers

`effectful-core` already supports multiple instances of one effect through `Labeled`.
A concrete label selects a service even when its payload matches every other service.
Here the stage accepts an action with two fresh labeled writers in front of an open
tail. It runs both child interpreters and forwards their output to the outer writers
of the corresponding names only on acceptance. The other handlers stay live.

```haskell
{-# LANGUAGE AllowAmbiguousTypes, ConstraintKinds, DataKinds, FlexibleContexts #-}
{-# LANGUAGE KindSignatures, ScopedTypeVariables, TypeApplications, TypeOperators #-}
module LabeledInspector where

import Control.Monad (when)
import Effectful
import Effectful.Labeled (Labeled, labeled, runLabeled)
import qualified Effectful.State.Static.Local as S
import qualified Effectful.Writer.Static.Local as W
import GHC.TypeLits (Symbol)
import Inspector

type Cell name = Labeled (name :: Symbol) (S.State N)
type Log name = Labeled (name :: Symbol) (W.Writer Trace)

update :: forall name es a. Cell name :> es => (N -> (a, N)) -> Eff es a
update f = labeled @name @(S.State N) (S.state f)

emit :: forall name es. Log name :> es => Trace -> Eff es ()
emit ns = labeled @name @(W.Writer Trace) (W.tell ns)

type Roles es =
  (Cell "serial" :> es, Cell "credits" :> es, Cell "gain" :> es,
   Log "acquired" :> es, Log "rejects" :> es, Log "defects" :> es,
   Log "samples" :> es, Log "passes" :> es)

ports :: Roles es => Ports (Eff es)
ports = Ports
  { nextSerial = update @"serial" advance
  , charge     = update @"credits" pay
  , takeGain   = update @"gain" takeOld
  , acquired   = \n -> emit @"acquired" [n]
  , rejects    = \n -> emit @"rejects" [n]
  , defects    = \n -> emit @"defects" [n]
  , samples    = \n -> emit @"samples" [n]
  , passes     = \n -> emit @"passes" [n]
  }

stage :: (Log "defects" :> es, Log "samples" :> es)
      => Eff (Log "defects" ': Log "samples" ': es) Bool -> Eff es Bool
stage body = do
  ((ok, ds), ss) <- runLabeled @"samples" (W.runWriter @Trace)
                  $ runLabeled @"defects" (W.runWriter @Trace) body
  when ok $ do
    emit @"defects" ds
    emit @"samples" ss
  pure ok

run :: Bool -> N -> Observation
run = runWithPrefix False

runWithPrefix :: Bool -> Bool -> N -> Observation
runWithPrefix prefix perAcquisition k = observe $ runPureEff
  $ runLabeled @"gain"    (S.runState (3 :: N))
  $ runLabeled @"credits" (S.runState (8 :: N))
  $ runLabeled @"serial"  (S.runState (0 :: N))
  $ runLabeled @"passes"   (W.runWriter @Trace)
  $ runLabeled @"rejects"  (W.runWriter @Trace)
  $ runLabeled @"acquired" (W.runWriter @Trace)
  $ runLabeled @"samples"  (W.runWriter @Trace)
  $ runLabeled @"defects"  (W.runWriter @Trace)
  $ do
    when prefix $ defects ports 7 >> samples ports 8
    if perAcquisition then retryOnce (stage (inspect ports k))
                      else stage (retryOnce (inspect ports k))
```

For both Haskell modules, `run True 1` gives
`Observation True (2,4,5) ([1,2],[1],[2],[2],[4])`.
`run False 1` gives `Observation True (2,4,5) ([1,2],[1],[2],[1,2],[3,4])`.
Labeling already removes the positional lift paths in the transformer adapter. The
business code and its two short compositions are shared. The remaining source-level
comparison concerns how a role is declared, activated, rebound and checked. The cost
of implementing reusable library machinery is charged once.
[effectful labels](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Labeled.html) · [local state](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-State-Static-Local.html) · [local writer](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Writer-Static-Local.html)

<a id="what-changes-when-another-author-extends-the-program"></a>

### A pre-existing report and an independently added journal

Start with defects `[7]` and samples `[8]` in the enclosing report. Call
`runWithPrefix True True 1` in either Haskell module. Its two staged outputs are
`[7,2]` and `[8,4]`. With `runWithPrefix True False 2`, both attempts fail and the
outputs remain `[7]` and `[8]`. A handler that clears the parent journals on rejection
fails this test even if it passes every empty-report case.

Now freeze the inspector and both policy bodies. Insert an independently authored
telemetry writer between acquired and rejects in the concrete stack. A new instrumented
client can emit telemetry and call the old inspector. The writer order in `Stack`, the
paths to rejects, passes and the inner states, and the runner/observer for the extra
output need attention. Changing the shared `fromStates` helper updates its clients together. The two staged writers stay adjacent at the top so the existing
publication body still works. With a fresh label in `effectful`, the existing eight
label selections and the old stage keep their meanings. The application installs the
ninth handler and extracts its output. Charge both versions for new wiring and include
an old trace as a control against accidentally rerouting a same-typed journal.

In CoD, a ninth telemetry role can have its own reference symbol and lexical cell, with its
installation at the run boundary. An instrumented leaf calls the existing inspector and
writes telemetry for both acquisitions. Neither retry nor stage reads that symbol. A
different request, to admit telemetry to the jointly published evidence, would require
changing the stage's policy in every version. The unrelated-lot scope provides a second
axis: bind a fresh scalar lot number while invoking the unchanged composition; a lot-aware
leaf must see the temporary lot and the caller must recover its previous lot on return.

This extension exercises repeated *instances* of two effect families and the cost of
selecting them by identity. The paper's [three-by-three matrix](../ComposingEffectsAndFocusedContexts.md#where-the-m--n-obligations-arise)
studies a different extension: adding an interface and a transformer constructor to
independently authored vocabularies. Five writer instances and three state instances
alone do not establish fifteen required adapters or an asymptotic lower bound.
