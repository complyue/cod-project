<a id="composing-retry-and-publication-in-a-tile-inspector"></a>

# Composing three states and five writers in a tile inspector

## A scalar setting is enough for a small composition

Two calibration policies modify the cutoff for work carried out in their scope. One
adds a margin of four. The other doubles the cutoff. A reading of 12 clears a cutoff
of 10 but misses a cutoff of 14. One scoped scalar suffices to express the difference.
For a bits payload, `val symbol @cutoff:u64` declares a value symbol explicitly.
`symbol @cutoff:u64` is its abbreviated form. Both install a scoped copy of the bits and
restore the previous value on exit.
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

<a id="haskell-business-code-shared-by-both-libraries"></a>
<a id="haskell-with-mtl"></a>

## Three StateT layers and five WriterT layers

Read the concrete stack from outside in. The five strict writers are defects, samples,
acquired, rejects and passes. The three strict states are serial, credits and gain.
The inspector performs each operation directly in this stack. In particular, the three
states all hold `Word64`, and every writer outputs `[Word64]`. Reaching the wrong
layer can therefore type-check.

```haskell
module Stacked where

import Control.Monad (when)
import Control.Monad.State.Strict (StateT, modify', runStateT, state)
import Control.Monad.Trans.Class (lift)
import Control.Monad.Writer.Strict (WriterT(..), tell)
import Data.Functor.Identity (Identity, runIdentity)
import Data.Word (Word64)

type Entries = [Word64]
type States  = StateT Word64 (StateT Word64 (StateT Word64 Identity))
type Stack   = WriterT Entries (WriterT Entries (WriterT Entries
                 (WriterT Entries (WriterT Entries States))))

inspect :: Word64 -> Stack Bool
inspect rejectThrough = do
  n <- state (\s -> (s + 1, s + 1))                  -- serial, through all five writers
  (lift . lift . lift . lift . lift . lift) $        -- credits, past serial
    modify' (subtract 2)
  g <- (lift . lift . lift . lift . lift . lift . lift) $
    state (\s -> (s, s + 1))                         -- gain, past serial and credits
  (lift . lift) (tell [n])                           -- acquired
  tell [n]                                           -- defects
  lift (tell [g])                                    -- samples
  let ok = n > rejectThrough
  if ok then (lift . lift . lift . lift) (tell [n])  -- passes
        else (lift . lift . lift) (tell [n])         -- rejects
  pure ok

retryOnce :: Monad m => m Bool -> m Bool
retryOnce body = do
  ok <- body
  if ok then pure True else body

stage :: Stack Bool -> Stack Bool
stage body = WriterT $ WriterT $ do
  ((ok, ds), ss) <- runWriterT (runWriterT body)
  let keep xs = if ok then xs else []
  pure ((ok, keep ds), keep ss)

run prefix perAcquisition k = runIdentity
  $ flip runStateT 3 $ flip runStateT 8 $ flip runStateT 0
  $ runWriterT $ runWriterT $ runWriterT $ runWriterT $ runWriterT
  $ do
      when prefix $ do
        tell [7]
        lift (tell [8])
      if perAcquisition then retryOnce (stage (inspect k))
                        else stage (retryOnce (inspect k))
```

At the first line of `inspect`, the stock `MonadState` instance for `WriterT` forwards
the action through all five writers to serial. The six explicit lifts for credits cross
those writers and the first `StateT`. Gains sit one layer deeper. The writer operations
use zero through four explicit lifts. The two `runWriterT` calls in `stage` expose just
the two provisional outputs of this action. Its Boolean verdict filters both. The other
three writers and all states execute in the underlying six-layer stack. Earlier parent
output has already accumulated outside the filtered action. This program takes the
favorable layout in which both provisional writers are outermost. It defines no new
typeclass instance. [mtl strict state](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/Control-Monad-State-Strict.html) · [mtl strict writer](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/Control-Monad-Writer-Strict.html)

## An interleaved stack using transformers directly

Keep the same business protocol and move serial state **between** the two writers that
stage must control. This second physical stack uses the primitive `StateT` and `WriterT`
operations from `transformers`. From the outside in its slots are defects, serial,
samples, credits, acquired, gain, rejects, passes. Each `lift` crosses one actual layer.
With this layout the publication function must return the new serial even when it
discards both writer outputs. Returning the incoming serial on rejection would cause
the next acquisition to reuse an ID.

```haskell
module Interleaved where

import Control.Monad (when)
import Control.Monad.Trans.Class (lift)
import qualified Control.Monad.Trans.State.Strict as S
import qualified Control.Monad.Trans.Writer.Strict as W
import Data.Functor.Identity (Identity, runIdentity)
import Data.Word (Word64)

type N = Word64
type Journal = W.WriterT [N]
type Cell = S.StateT N
type Stack = Journal (Cell (Journal (Cell (Journal (Cell (Journal (Journal Identity)))))))

inspect :: N -> Stack Bool
inspect rejectThrough = do
  n <- lift $ S.state (\s -> (s + 1, s + 1))                    -- serial
  (lift . lift . lift) $ S.modify' (subtract 2)                 -- credits
  g <- (lift . lift . lift . lift . lift) $ S.state (\x -> (x, x + 1)) -- gain
  (lift . lift . lift . lift) $ W.tell [n]                      -- acquired
  W.tell [n]                                                    -- defects
  (lift . lift) $ W.tell [g]                                    -- samples
  let ok = n > rejectThrough
  if ok then (lift . lift . lift . lift . lift . lift . lift) (W.tell [n]) -- passes
        else (lift . lift . lift . lift . lift . lift) (W.tell [n])        -- rejects
  pure ok

retryOnce :: Monad m => m Bool -> m Bool
retryOnce body = do
  ok <- body
  if ok then pure True else body

stage :: Stack Bool -> Stack Bool
stage body = W.WriterT $ S.StateT $ \serialIn -> W.WriterT $ do
  (((ok, ds), serialOut), ss) <- W.runWriterT (S.runStateT (W.runWriterT body) serialIn)
  let publish xs = if ok then xs else []
  pure (((ok, publish ds), serialOut), publish ss)

run prefix perAcquisition k = runIdentity
  $ W.runWriterT                 -- passes
  $ W.runWriterT                 -- rejects
  $ flip S.runStateT 3           -- gain
  $ W.runWriterT                 -- acquired
  $ flip S.runStateT 8           -- credits
  $ W.runWriterT                 -- samples
  $ flip S.runStateT 0           -- serial
  $ W.runWriterT                 -- defects
  $ do
    when prefix $ do
      W.tell [7]
      (lift . lift) $ W.tell [8]
    if perAcquisition then retryOnce (stage (inspect k))
                      else stage (retryOnce (inspect k))
```

For `run False True 1` the eight native runners successively expose acceptance, defects
`[2]`, serial `2`, samples `[4]`, credits `4`, acquired `[1,2]`, gain `5`, rejects `[1]`
and passes `[2]`. For `run False False 1` only defects and samples change, to `[1,2]` and
`[3,4]`. An observer must decode each result pair in this stack's own order. The predicate,
retry and publication contract are the same as in the grouped stack. The altered source
of `stage` and the changed paths to `tell` show exactly where this layout costs the author.
[StateT operations](https://hackage-content.haskell.org/package/transformers-0.6.2.0/docs/Control-Monad-Trans-State-Strict.html) · [WriterT operations](https://hackage-content.haskell.org/package/transformers-0.6.2.0/docs/Control-Monad-Trans-Writer-Strict.html)

<a id="haskell-with-effectful"></a>
<a id="the-same-inspector-with-labeled-effectful-handlers"></a>

## Direct operations on a labeled effectful row

Here is a separate implementation for `effectful-core` 2.7.1.2. `Eff Eight` has an
effect row. Each of its three labeled state handlers and five labeled writer handlers
has its own installation. Type-level strings distinguish equal payload types. The
inspector invokes the library's `labeled`, `state`, `modify` and `tell` at their actual
use sites. Its constraints also admit temporary writers ahead of the outer handlers.

```haskell
{-# LANGUAGE DataKinds, FlexibleContexts, KindSignatures, TypeApplications, TypeOperators #-}
module Labeled where

import Control.Monad (when)
import Data.Word (Word64)
import Effectful
import Effectful.Labeled (Labeled, labeled, runLabeled)
import qualified Effectful.State.Static.Local as S
import qualified Effectful.Writer.Static.Local as W
import GHC.TypeLits (Symbol)

type Entries = [Word64]
type StateRole (name :: Symbol) = Labeled name (S.State Word64)
type Journal   (name :: Symbol) = Labeled name (W.Writer Entries)
type Eight = '[Journal "defects", Journal "samples", Journal "acquired",
               Journal "rejects", Journal "passes",
               StateRole "serial", StateRole "credits", StateRole "gain"]

inspect :: (StateRole "serial" :> es, StateRole "credits" :> es, StateRole "gain" :> es,
            Journal "acquired" :> es, Journal "defects" :> es, Journal "samples" :> es,
            Journal "rejects" :> es, Journal "passes" :> es)
        => Word64 -> Eff es Bool
inspect rejectThrough = do
  n <- labeled @"serial" @(S.State Word64) $ S.state @Word64 (\s -> (s + 1, s + 1))
  labeled @"credits" @(S.State Word64) $ S.modify @Word64 (subtract 2)
  g <- labeled @"gain" @(S.State Word64) $ S.state @Word64 (\s -> (s, s + 1))
  labeled @"acquired" @(W.Writer Entries) $ W.tell [n]
  labeled @"defects"  @(W.Writer Entries) $ W.tell [n]
  labeled @"samples"  @(W.Writer Entries) $ W.tell [g]
  let ok = n > rejectThrough
  if ok then labeled @"passes" @(W.Writer Entries) (W.tell [n])
        else labeled @"rejects" @(W.Writer Entries) (W.tell [n])
  pure ok

retryOnce :: Monad m => m Bool -> m Bool
retryOnce body = do
  ok <- body
  if ok then pure True else body

stage :: (Journal "defects" :> es, Journal "samples" :> es)
      => Eff (Journal "defects" ': Journal "samples" ': es) Bool -> Eff es Bool
stage body = do
  ((ok, ds), ss) <- runLabeled @"samples" (W.runWriter @Entries)
                  $ runLabeled @"defects" (W.runWriter @Entries) body
  when ok $ do
    labeled @"defects" @(W.Writer Entries) (W.tell ds)
    labeled @"samples" @(W.Writer Entries) (W.tell ss)
  pure ok

tile :: Bool -> Bool -> Word64 -> Eff Eight Bool
tile prefix perAcquisition k = do
  when prefix $ do
    labeled @"defects" @(W.Writer Entries) (W.tell ([7] :: Entries))
    labeled @"samples" @(W.Writer Entries) (W.tell ([8] :: Entries))
  if perAcquisition then retryOnce (stage (inspect k))
                    else stage (retryOnce (inspect k))

run prefix perAcquisition k = runPureEff
  $ runLabeled @"gain"    (S.runState (3 :: Word64))
  $ runLabeled @"credits" (S.runState (8 :: Word64))
  $ runLabeled @"serial"  (S.runState (0 :: Word64))
  $ runLabeled @"passes"   (W.runWriter @Entries)
  $ runLabeled @"rejects"  (W.runWriter @Entries)
  $ runLabeled @"acquired" (W.runWriter @Entries)
  $ runLabeled @"samples"  (W.runWriter @Entries)
  $ runLabeled @"defects"  (W.runWriter @Entries)
  $ tile prefix perAcquisition k
```

During `stage`, two fresh local writer handlers temporarily shadow the public defects
and samples handlers of the same labels. After eliminating the locals, the two final
`tell` operations address the public handlers. The local static writer supplies
`runWriter` and `tell` and deliberately omits `pass` and `censor`. The code uses the
library's actual API to express selective publication. Each public state is still
handled once by its own `runState` and each public journal once by its own `runWriter`.
[effectful labels](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Labeled.html) · [local state](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-State-Static-Local.html) · [local writer](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Writer-Static-Local.html)

<a id="what-changes-when-another-author-extends-the-program"></a>
<a id="a-pre-existing-report-and-an-independently-added-journal"></a>

### Observing a routed write and testing an extension

In `Stacked` and `Labeled`, every writer runner returns its result and its own output.
Every state runner returns its result and its own final state. The natural nesting gives
the following results, in elimination order (verdict, defects, samples, acquired, rejects,
passes, serial, credits, gain). These are the return values of the eight eliminations.
The acquisition itself communicates through the individual effects shown above.

```haskell
-- Empty report. Reject the first acquisition, accept the second.
run False True  1 == ((((((((True,[2]),[4]),[1,2]),[1]),[2]),2),4),5)
run False False 1 == ((((((((True,[1,2]),[3,4]),[1,2]),[1]),[2]),2),4),5)

-- An earlier defect 7 has a matching sample 8.
run True True  1 == ((((((((True,[7,2]),[8,4]),[1,2]),[1]),[2]),2),4),5)
run True False 2 == ((((((((False,[7]),[8]),[1,2]),[1,2]),[]),2),4),5)
```

For all twelve inputs, combine both values of `prefix` and `perAcquisition` with
`k = 0, 1, 2`. The [table above](#which-report-is-correct) gives all eight observations
for empty prefixes. With prefixes, prepend 7 to defects and 8 to samples for each
corresponding result. Try a wrong route that still type-checks: use three lifts for the
acquired append in `Stacked`, or change just its label to `"rejects"` in `Labeled`. For
`k = 1` both still accept with state (2,4,5); acquired becomes empty, and rejects
records [1,1,2]. Checking the role-specific outputs catches the error.

Freeze the old business requirement and insert a ninth telemetry writer between acquired
and rejects in the concrete stack. A wrapped inspector can record the new event. The
positional routes to rejects, passes, credits and gain each cross an additional layer;
the stock state lifting to serial still works. The application must also eliminate the
new writer. With a fresh label in the effectful row, the old operation sites keep their
labels and the application still owes a handler and an observer for the ninth output.
In CoD a separate symbol and lexical cell can supply the extra journal while the two
publication scopes retain their definitions. An independent lot-number binding tests the
other axis by enclosing the unchanged acquisition in a new scope. If the business later
includes telemetry in the provisional evidence, the stage must also implement its new
publication contract.

These are costs of routing repeated instances of two effect families. The paper's
[interface/transformer matrix](../ComposingEffectsAndFocusedContexts.md#where-the-m--n-obligations-arise)
counts independently extensible *interfaces and layer constructors*. Three states and
five writers by themselves establish neither fifteen adapters nor an asymptotic bound.
