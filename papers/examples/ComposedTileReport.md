# Composing retry and publication in a tile inspector

One tile can require two physical acquisitions. Each acquisition gets a fresh number,
records a provisional defect flag bearing that number, then returns the tile validator's
verdict. Two independently authored library operations wrap that body.

* `retryOnce(body)` runs the body again after its first rejection. It returns the second
  verdict, even on rejection. It knows nothing about flags.
* `stage(body)` directs flags to a fresh private report. On acceptance it appends the
  batch to the enclosing report in order. On rejection it drops the batch. It knows
  nothing about retries or how a tile is validated.

The inspector, driver of physical acquisitions, retry policy and publication policy remain
identical in both assemblies. Only their nesting changes. A Boolean is the complete
business verdict for this experiment. Unexpected failures, concurrent observers and
external publication are outside the experiment. The paper's
[scoped audit](ScopedTileAudit.md) separately tests checked failure and restoration.

## Which report is correct?

Let `R` mean retry, `S` mean stage, and `I` mean inspect once. Start each run with a
fresh counter and empty public report. `rejectThrough = 1` rejects acquisition 1 and
accepts acquisition 2. These observations include both successful and exhausted retries.

| Acquisition verdicts | Acquisitions in either assembly | Final verdict | Public report from R(S(I)) | Public report from S(R(I)) |
| --- | --- | --- | --- | --- |
| Accept immediately | 1 | Accepted | [1] | [1] |
| Reject, then accept | 1, 2 | Accepted | **[2]** | **[1, 2]** |
| Reject twice | 1, 2 | Rejected | [] | [] |

In the middle row, the inner stage in `R(S(I))` drops its first private report and the
second stage starts empty. In `S(R(I))`, retry observes the first rejection while the
same private report is still open. Its second acquisition accepts, so the outer stage
commits both flags. A report of accepted *acquisitions* needs the first assembly. A
report of every acquisition contributing to an ultimately accepted *tile* needs the second.

## CoD

`@acquire` numbers physical acquisitions in a live outer region. `@flag` appends to
the currently selected report. The inspector invokes the installed `@flag` with
`behave` so a private report supplied by a surrounding stage takes effect. A helper that
replayed the original public destination here would leak the rejected flag.

For a run of at most two single-digit acquisitions, the CoD observer represents a list
by its decimal digits and scale. Thus [1,2] is (12,100), [2] is (2,10), and [] is (0,1).
Appending B to A gives (A.digits × B.scale + B.digits, A.scale × B.scale). This
preserves the actual order, allows an empty successful batch and needs no list library.
The small bound keeps all arithmetic in range.

```cod
region Source { calls:u64; }
region Report { digits:u64; scale:u64; }
region symbol @source:Source
region symbol @report:Report

proc @acquire() -> u64 {
    @source.calls <- @source.calls + 1u64
    return @source.calls
}

proc @flag(id:u64) -> void {
    @report.digits <- 10u64 * @report.digits + id
    @report.scale <- 10u64 * @report.scale
    return
}

proc inspect(reject_through:u64) -> bool {
    val id <- behave @acquire()
    behave @flag(id)
    return id > reject_through
}

func retry_once(here body() -> bool) -> bool {
    if (body()) { return true }
    return body()
}

proc stage(here body() -> bool) -> bool {
    val digits <- 0u64
    val scale <- 1u64
    val accepted <- {{
        effect @report <- make region Report(256u64 -> capacity) { digits <- 0u64; scale <- 1u64; };
        val ok <- body()
        digits <- @report.digits
        scale <- @report.scale
        return ok
    }}
    if (accepted) {
        @report.digits <- @report.digits * scale + digits
        @report.scale <- @report.scale * scale
    }
    return accepted
}

proc check(per_acquisition:bool, reject_through:u64, ok:bool, calls:u64, encoded:u64) -> void {
    effect @source <- make region Source(256u64 -> capacity) { calls <- 0u64; }
    effect @report <- make region Report(256u64 -> capacity) { digits <- 0u64; scale <- 1u64; }
    effect proc @acquire;
    effect proc @flag;
    val accepted <- false
    if (per_acquisition) {
        accepted <- retry_once() { stage() { inspect(reject_through) } }
    } else {
        accepted <- stage() { retry_once() { inspect(reject_through) } }
    }
    assert accepted == ok
    assert @source.calls == calls
    assert @report.digits == encoded
    return
}

check(true,  0u64, true,  1u64, 1u64)
check(false, 0u64, true,  1u64, 1u64)
check(true,  1u64, true,  2u64, 2u64)
check(false, 1u64, true,  2u64, 12u64)
check(true,  2u64, false, 2u64, 0u64)
check(false, 2u64, false, 2u64, 0u64)
```

The two assemblies use single-brace callbacks whose final expression supplies the Boolean
verdict. The immediate double-brace scope inside `stage` runs the body with a fresh report owner.
It copies two integers out before restoring the parent binding and retiring the child
owner. No reference to that owner escapes. The source counter stays live across both
attempts. Staging that counter along with the flags would make a retry acquire 1 again.
The effect bindings are installed once at the application boundary.
[Here callback results](../../design/CallableStorageAndHereAccess.md#single-brace-bodies-for-a-here-callable) ·
[Symbolic invocation and regional capture](../../design/SymbolicDynamicScope.md#regional-resource-capture-preserves-owners)

## Haskell with mtl

`WriterT [Int]` gives each stage a private report, and the underlying strict state
supplies acquisition numbers. Running the child writer removes it before `tell` appends
the accepted suffix to its parent. The retry wrapper only requires a monad. The
inspector's two constraints are satisfied at both nesting depths by existing instances.

```haskell
{-# LANGUAGE FlexibleContexts #-}
module ComposedMtl where

import Control.Monad (when)
import Control.Monad.State.Strict (MonadState, get, modify', runState)
import Control.Monad.Writer.Strict (MonadWriter, WriterT, runWriterT, tell)

inspect :: (MonadState Int m, MonadWriter [Int] m) => Int -> m Bool
inspect rejectThrough = do
  modify' (+ 1)
  n <- get
  tell [n]
  pure (n > rejectThrough)

retryOnce :: Monad m => m Bool -> m Bool
retryOnce body = do
  ok <- body
  if ok then pure True else body

stage :: MonadWriter [Int] m => WriterT [Int] m Bool -> m Bool
stage body = do
  (ok, batch) <- runWriterT body
  when ok (tell batch)
  pure ok

run :: Bool -> Int -> ((Bool, [Int]), Int)
run perAcquisition rejectThrough = flip runState 0 $ runWriterT $
  if perAcquisition
    then retryOnce (stage (inspect rejectThrough))
    else stage (retryOnce (inspect rejectThrough))
```

This program needs no new transformer instance. The higher-order stage makes the
private writer explicit in its input type. The state operation crosses it through the
stock `MonadState` instance. A library author adding a new capability class or
transformer has a different extension task. The paper's
[instance matrix](../ComposingEffectsAndFocusedContexts.md#where-the-m--n-obligations-arise)
identifies those cells and the costs already paid by the library.
[mtl state](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/Control-Monad-State-Class.html) ·
[strict writer](https://hackage-content.haskell.org/package/transformers-0.6.2.0/docs/Control-Monad-Trans-Writer-Strict.html)

## Haskell with effectful

The static writer and state effects suffice for this contract. The repeated writer at
the head of the effect row is the private writer; after `runWriter`, `tell` goes to the
enclosing one. An open tail carries effects unrelated to staging. The same `retryOnce`
definition from the mtl listing is reproduced so the modules can be read independently.

```haskell
{-# LANGUAGE DataKinds, FlexibleContexts, TypeApplications, TypeOperators #-}
module ComposedEff where

import Control.Monad (when)
import Effectful
import qualified Effectful.State.Static.Local as S
import qualified Effectful.Writer.Static.Local as W

inspect :: (S.State Int :> es, W.Writer [Int] :> es) => Int -> Eff es Bool
inspect rejectThrough = do
  S.modify @Int (+ 1)
  n <- S.get @Int
  W.tell [n]
  pure (n > rejectThrough)

retryOnce :: Monad m => m Bool -> m Bool
retryOnce body = do
  ok <- body
  if ok then pure True else body

stage :: W.Writer [Int] :> es => Eff (W.Writer [Int] ': es) Bool -> Eff es Bool
stage body = do
  (ok, batch) <- W.runWriter @[Int] body
  when ok (W.tell batch)
  pure ok

run :: Bool -> Int -> ((Bool, [Int]), Int)
run perAcquisition rejectThrough = runPureEff $ S.runState (0 :: Int) $ W.runWriter @[Int] $
  if perAcquisition
    then retryOnce (stage (inspect rejectThrough))
    else stage (retryOnce (inspect rejectThrough))
```

For either Haskell module the inputs `True 1` and `False 1` to `run` give
`((True,[2]),2)` and `((True,[1,2]),2)`. Inputs with 0 and 2 give the other two
rows of the table. The signatures target `effectful-core` 2.7.1.2.
[Local writer operations](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Writer-Static-Local.html) ·
[Local state operations](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-State-Static-Local.html)

## What changes when another author extends the program?

Freeze the inspector and both wrappers. Add a probe that records **every** acquisition
in a separate counter that stays live across both attempts. A new inspector variant can call it once before delegating
to the old inspector. Installing the counter outside staging keeps the probe count at
two in the disputed row under either nesting. In CoD the additional module and its
client can express the extension as follows.

```cod
region Probe { count:u64; }
region symbol @probe:Probe

proc @observed() -> void {
    @probe.count <- @probe.count + 1u64
    return
}

proc inspected_with_probe(reject_through:u64) -> bool {
    behave @observed()
    return inspect(reject_through)
}
```

The assembly adds `effect @probe <- make region Probe(256u64 -> capacity) { count <- 0u64; }`
and `effect proc @observed;`, invokes the new inspector at the leaf, and checks
`@probe.count == @source.calls`. Neither wrapper mentions the probe. The same experiment
can add a distinct reader-provided probe service to the mtl program or a distinct
probe operation to the effectful row. Record any forwarding definitions each version
actually needs. An existing library instance or shared open-row implementation counts once.

Independently, add a layer that runs a body once under the acquisition station's lot
number. A consumer can check it sees lot 42 inside and its original lot 7 on exit.

```cod
symbol @lot:u64
proc in_lot(here body() -> bool, lot:u64) -> bool {
    effect @lot <- lot
    return body()
}
```

Install 7 at the application boundary and surround either assembly with
`in_lot(42u64) {{ ... }}`. The original inspector, retry, stage and new probe pass
through this layer unchanged. A lot-reading leaf and a read after the call make its own
semantics observable. Then combine the independently added row and column. The probe
still counts both acquisitions, the public report still distinguishes the orders, and
the lot still restores. Try the additions in either order, and count edits to old code,
new definitions, application wiring and any generated registration input separately.

The differing public reports are the interaction that must be decided: whether to
stage each acquisition or the whole retry. No forwarding rule can select a business
answer. Both assemblies express it by nesting two existing wrappers. The CoD saving
here is that the inspector and wrappers share nominal operations without an explicit
effect-row or transformer-stack signature. Both Haskell implementations are compact and
also reuse both wrappers unchanged. The public trace witnesses noncommutativity, `R ∘ S ≠ S ∘ R`, for these two scopes.
The probe and lot examples supply controls for transparent extension. The
[additive-source argument](../ComposingEffectsAndFocusedContexts.md#additive-source-extension)
addresses the separate scaling claim. All six observations above are paper derivations.
