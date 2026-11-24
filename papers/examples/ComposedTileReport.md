<a id="composing-retry-and-publication-in-a-tile-inspector"></a>

# Composing three states and five writers in a tile inspector

The [CoD inspector](#cod) and the [Haskell inspector with `Ports m`](#haskell-with-named-operations)
request the same eight operations. Their implementations below show where the state lives
and which two journal destinations staging redirects. The [application comparison](#application-end-in-cod-and-haskell)
shows all three assemblies. The [engineering checks](#engineering-checks) exercise their observations.
The [direct transformer](#haskell-with-mtl) and [labeled effect](#haskell-with-effectful)
listings expose the underlying routing.

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
val restored <- @cutoff
```

The first order tests against `2 × (3 + 4)`. The second tests against
`(2 × 3) + 4`. Neither policy had to know which other policy enclosed its test.
`at_fourteen` is false, `at_ten` is true and `restored` is 3.

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

<a id="haskell-business-code-shared-by-both-libraries"></a>

## Haskell with named operations

A camera inspection needs eight operations. It can give them business names in a
record and run each field in a chosen monad `m`. The field `nextSerial` supplies an
action that obtains a new number each time it runs. A journal field accepts an entry
and produces an append action. The three state cells and five accumulated journals
belong to the backend. Here is the complete business interface and its one consumer,
shared by the two Haskell libraries.

```haskell
module Inspector where

import Data.Word (Word64)

type N = Word64
type Trace = [N]

data StageScope = PerAcquisition | PerTile deriving (Eq, Show)

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
```

The CoD leaf requests the same eight operations, in the same order. Compare `charge p`
with `charge()` and `samples p g` with `samples(gain)`. In each case the inspection
can state the business rule without knowing where the balance or journal is kept.

## CoD

The business module selects eight ordinary procedures from the provider. Their definitions
are fixed. A call reads the state bindings active at that point, so `stage` can redirect
both provisional journals while an unchanged `inspect` is running.

```cod
proc inspect(reject_through:u64) -> bool {
    val id <- next_serial()
    charge()
    val gain <- take_gain()
    acquired(id)
    defects(id)
    samples(gain)
    if (id <= reject_through) {
        rejects(id)
        return false
    }
    passes(id)
    return true
}

func retry_once(here body() -> bool) -> bool { body() or body() }
```

The provider declares a separate identity for each mutable target. In each declaration,
`O` is established from the target of that installation. These eight declarations can
therefore select cells with different owners. `observe` supplies the public cells, and each
active `stage` owns its two temporary cells. The business module only needs the procedure
names. For this fixture a journal stores its ordered, single-digit entries as a `u64`.
A zero denotes the empty journal and appending `n` computes `10 * log + n`.

```cod
ref symbol @serial_cell:u64 in O where {owner O}
ref symbol @credit_cell:u64 in O where {owner O}
ref symbol @gain_cell:u64 in O where {owner O}
ref symbol @acquired_sink:u64 in O where {owner O}
ref symbol @rejects_sink:u64 in O where {owner O}
ref symbol @passes_sink:u64 in O where {owner O}
ref symbol @defects_sink:u64 in O where {owner O}
ref symbol @samples_sink:u64 in O where {owner O}

proc next_serial() -> u64 {
    @serial_cell <- @serial_cell + 1u64
    return @serial_cell
}

proc charge() { @credit_cell <- @credit_cell - 2u64 }

proc take_gain() -> u64 {
    val previous <- @gain_cell
    @gain_cell <- previous + 1u64
    return previous
}

func appended(log:u64, n:u64) -> u64 { 10u64 * log + n }

proc acquired(n:u64) { @acquired_sink <- appended(@acquired_sink, n) }
proc rejects(n:u64) { @rejects_sink <- appended(@rejects_sink, n) }
proc passes(n:u64) { @passes_sink <- appended(@passes_sink, n) }
proc defects(n:u64) { @defects_sink <- appended(@defects_sink, n) }
proc samples(n:u64) { @samples_sink <- appended(@samples_sink, n) }
```

`effect @credit_cell = credits` borrows the caller's existing cell. The assignment in
`charge()` writes through that binding, so a rejected acquisition still costs two credits.
The earlier calibration example installs a scoped bits value with `effect @cutoff <- ...`.
A direct call to an ordinary `proc` uses the current bindings without installing a callable
for each operation. The separate [audit experiment](ScopedTileAudit.md) uses an installed
procedure to compare invocation under saved and current bindings.
[Ordinary procedure calls](../../design/SymbolicDynamicScope.md#functions-and-procedures) ·
[Owner-parameter reference symbols](../../design/SymbolicDynamicScope.md#owner-parameter-reference-symbols-and-implementation-cost)

On entry `stage` copies the two current journals, including any existing prefix. It
redirects exactly those two destinations for the duration of `body()`. All appends in
this synchronous example go through the selected destinations. An accepted stage can
therefore publish the two completed values to its parent after the bindings are restored.

```cod
proc stage(here body() -> bool) -> bool {
    val child_defects <- @defects_sink
    val child_samples <- @samples_sink
    val accepted <- {{
        effect @defects_sink = child_defects, @samples_sink = child_samples
        body()
    }}
    if (accepted) {
        @defects_sink <- child_defects
        @samples_sink <- child_samples
    }
    return accepted
}
```

A rejected stage discards its two local copies. The other three journal destinations
and the three state cells stay active throughout. The observations are the ordered journal
contents on return. This copy-and-publish implementation assumes those contents fit in
the fixture's bounded representation and no other path writes the parent during the call.

A nested stage exposes an important consequence. Start with public entries `[7]` and `[8]`.
An inner stage accepts acquisition 1 and its sample 3. If the enclosing stage rejects, the
public journals stay `[7]` and `[8]`. If it accepts, they become `[7,1]` and `[8,3]`.
In both cases serial, credits and gain end at `(1,6,4)`. The [engineering checks](#engineering-checks)
exercise both outer verdicts in all three assemblies.

## Application end in CoD and Haskell

`PerAcquisition` gives each attempt its own stage. `PerTile` gives the whole retry one stage.
All three application entries take the scope, rejection threshold and prefix flag in that order.
The public prefix consists of defect ID `7` and its gain sample `8`.

**CoD.** `tile` selects the order. Its eight ordinary operations use the current state and journals.

```cod
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

**Haskell with `mtl`.** `ports` routes the business operations to the three states and five writers.

```haskell
run scope k prefix = runStack $ do
  when prefix $ defects ports 7 >> samples ports 8
  case scope of
    PerAcquisition -> retryOnce (stage (inspect ports k))
    PerTile        -> stage (retryOnce (inspect ports k))
```

**Haskell with labeled `effectful`.** `tile` uses the same `Inspector` interface on its effect row.

```haskell
tile :: StageScope -> N -> Bool -> Eff Eight Bool
tile scope k prefix = do
  when prefix $ defects ports 7 >> samples ports 8
  case scope of
    PerAcquisition -> retryOnce (stage (inspect ports k))
    PerTile        -> stage (retryOnce (inspect ports k))

run scope k prefix = runEight (tile scope k prefix)
```

Here are the calls for a failed acquisition followed by an accepted retry, with a public prefix.
The Haskell names below qualify the two adapter modules. CoD's `observe` runs a body with fresh cells.

| Application | Business call | Defect IDs | Gain samples |
| --- | --- | --- | --- |
| CoD inside `observe()` | `tile(PerAcquisition:StageScope, 1u64, true -> prefix)` | `[7,2]` | `[8,4]` |
| `mtl` | `PortStacked.run PerAcquisition 1 True` | `[7,2]` | `[8,4]` |
| labeled `effectful` | `PortLabeled.run PerAcquisition 1 True` | `[7,2]` | `[8,4]` |

Selecting `PerTile` on the same inputs publishes `[7,1,2]` and `[8,3,4]` in all three.

### Installing the CoD state

`observe` owns the eight public cells for one run and returns a snapshot of their final
values with the business verdict. The named result fields are produced after the action.
All eight cells can be selected independently while it runs. The Haskell runners deliver
these observations as the nested pairs produced by the real transformer or handler stack.

```cod
proc observe(here action() -> bool)
    -> { accepted:bool; serial:u64; credits:u64; gain:u64;
         acquired_log:u64; rejects_log:u64; passes_log:u64; defects_log:u64; samples_log:u64; }
{
    val serial <- 0u64
    val credits <- 8u64
    val gain <- 3u64
    val acquired_log <- 0u64
    val rejects_log <- 0u64
    val passes_log <- 0u64
    val defects_log <- 0u64
    val samples_log <- 0u64

    effect @serial_cell = serial, @credit_cell = credits, @gain_cell = gain,
           @acquired_sink = acquired_log, @rejects_sink = rejects_log, @passes_sink = passes_log,
           @defects_sink = defects_log, @samples_sink = samples_log

    val accepted <- action()
    return { accepted -> accepted, serial -> serial, credits -> credits, gain -> gain,
             acquired_log -> acquired_log, rejects_log -> rejects_log, passes_log -> passes_log,
             defects_log -> defects_log, samples_log -> samples_log }
}
```

A caller can select just the fields it needs from that result.

```cod
{ val ids <- defects_log; val gains <- samples_log; } := observe() {
    tile(PerAcquisition:StageScope, 1u64, true -> prefix)
}
```

The two Haskell adapters below both use the `Inspector` module, including its state
transitions and business rule.

<a id="supplying-the-haskell-operations"></a>

### Wiring Ports to the transformer stack

The [`Stacked` module below](#haskell-with-mtl) defines the concrete types and
publication policy used by this module. `Stack` has five strict `WriterT [Word64]`
layers, followed by three strict `StateT Word64` layers over `Identity`.
The three states, from the outside in, are serial (initially 0), credits (8) and
gain (3). Their roles come from the transitions at these positions. The two fields
`nextSerial` and `takeGain` even have the same action type. An adapter that switched
their two implementations could still type-check. The component observations detect it.

```haskell
module PortStacked where

import Control.Monad (when)
import Control.Monad.State.Strict (state)
import Control.Monad.Trans.Class (lift)
import Control.Monad.Writer.Strict (tell)
import Inspector
import Stacked (Stack, States, retryOnce, runStack, stage)

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

run scope k prefix = runStack $ do
  when prefix $ defects ports 7 >> samples ports 8
  case scope of
    PerAcquisition -> retryOnce (stage (inspect ports k))
    PerTile        -> stage (retryOnce (inspect ports k))
```

The first available `MonadState` instance reaches serial through all five writers.
`charge` crosses the writer prefix and serial to reach credits. The gain action goes
one layer deeper and returns the old reading. The writer lifts select defects,
samples, acquired, rejects and passes in order of depth. Both assemblies reuse this
one adapter. `stage` filters the contribution of the two outer writer transformers
on rejection. The other six layers keep their normal meanings.

### Wiring Ports to labeled effectful handlers

The [`Labeled` module below](#haskell-with-effectful) defines `Eight`, its eight-role
row, and the same retry and stage policies that its direct inspector uses. These ports
work in any row providing all eight labels. Inside `stage`, the port record is
instantiated for the extended row containing the two private writers. Its defects and
samples actions consequently address the nearest matching handlers. The shared `runEight` runner
installs each of the eight public handlers.

```haskell
{-# LANGUAGE ConstraintKinds, DataKinds, FlexibleContexts, TypeApplications, TypeOperators #-}
module PortLabeled where

import Control.Monad (when)
import Effectful
import Effectful.Labeled (labeled)
import qualified Effectful.State.Static.Local as S
import qualified Effectful.Writer.Static.Local as W
import Inspector
import Labeled (Eight, Journal, StateRole, retryOnce, runEight, stage)

type Roles es =
  (StateRole "serial" :> es, StateRole "credits" :> es, StateRole "gain" :> es,
   Journal "acquired" :> es, Journal "rejects" :> es, Journal "passes" :> es,
   Journal "defects" :> es, Journal "samples" :> es)

ports :: Roles es => Ports (Eff es)
ports = Ports
  { nextSerial = labeled @"serial" @(S.State N) (S.state advance)
  , charge     = labeled @"credits" @(S.State N) (S.state pay)
  , takeGain   = labeled @"gain" @(S.State N) (S.state takeOld)
  , acquired   = \n -> labeled @"acquired" @(W.Writer Trace) (W.tell [n])
  , rejects    = \n -> labeled @"rejects"  @(W.Writer Trace) (W.tell [n])
  , passes     = \n -> labeled @"passes"  @(W.Writer Trace) (W.tell [n])
  , defects    = \n -> labeled @"defects" @(W.Writer Trace) (W.tell [n])
  , samples    = \n -> labeled @"samples" @(W.Writer Trace) (W.tell [n])
  }

tile :: StageScope -> N -> Bool -> Eff Eight Bool
tile scope k prefix = do
  when prefix $ defects ports 7 >> samples ports 8
  case scope of
    PerAcquisition -> retryOnce (stage (inspect ports k))
    PerTile        -> stage (retryOnce (inspect ports k))

run scope k prefix = runEight (tile scope k prefix)
```

For either adapter, `run PerAcquisition 1 False` publishes defects `[2]` and samples `[4]`.
With the same leaf and `run PerTile 1 False`, the outputs are `[1,2]` and `[3,4]`.
In both cases the final states are `(2,4,5)`, the acquired log is `[1,2]`, rejects
is `[1]`, and passes is `[2]`. Start with a public prefix via `run PerAcquisition 1 True` and
the first pair becomes `[7,2]`, `[8,4]`. With `run PerTile 2 True`, rejection leaves
`[7]` and `[8]` intact. The nested result pairs are the runners' ordinary output.
These predictions can also be checked by inlining each field in the inspector. Both backends share the exact same runner between their port-based and
direct versions. The listings below make that routing visible and let a reader change one lift or
label without changing the operation interface.

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
import Inspector (StageScope(..))
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

runStack action = runIdentity
  $ flip runStateT 3 $ flip runStateT 8 $ flip runStateT 0
  $ runWriterT $ runWriterT $ runWriterT $ runWriterT $ runWriterT action

run scope k prefix = runStack $ do
  when prefix $ do
    tell [7]
    lift (tell [8])
  case scope of
    PerAcquisition -> retryOnce (stage (inspect k))
    PerTile        -> stage (retryOnce (inspect k))
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
import Inspector (StageScope(..))
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

run scope k prefix = runIdentity
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
    case scope of
      PerAcquisition -> retryOnce (stage (inspect k))
      PerTile        -> stage (retryOnce (inspect k))
```

For `run PerAcquisition 1 False` the eight native runners successively expose acceptance, defects
`[2]`, serial `2`, samples `[4]`, credits `4`, acquired `[1,2]`, gain `5`, rejects `[1]`
and passes `[2]`. For `run PerTile 1 False` only defects and samples change, to `[1,2]` and
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
import Inspector (StageScope(..))
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

tile :: StageScope -> Word64 -> Bool -> Eff Eight Bool
tile scope k prefix = do
  when prefix $ do
    labeled @"defects" @(W.Writer Entries) (W.tell ([7] :: Entries))
    labeled @"samples" @(W.Writer Entries) (W.tell ([8] :: Entries))
  case scope of
    PerAcquisition -> retryOnce (stage (inspect k))
    PerTile        -> stage (retryOnce (inspect k))

runEight action = runPureEff
  $ runLabeled @"gain"    (S.runState (3 :: Word64))
  $ runLabeled @"credits" (S.runState (8 :: Word64))
  $ runLabeled @"serial"  (S.runState (0 :: Word64))
  $ runLabeled @"passes"   (W.runWriter @Entries)
  $ runLabeled @"rejects"  (W.runWriter @Entries)
  $ runLabeled @"acquired" (W.runWriter @Entries)
  $ runLabeled @"samples"  (W.runWriter @Entries)
  $ runLabeled @"defects"  (W.runWriter @Entries)
  $ action

run scope k prefix = runEight (tile scope k prefix)
```

During `stage`, two fresh local writer handlers temporarily shadow the public defects
and samples handlers of the same labels. After eliminating the locals, the two final
`tell` operations address the immediately enclosing handlers. An outer stage may still buffer them. The local static writer supplies
`runWriter` and `tell` and deliberately omits `pass` and `censor`. The code uses the
library's actual API to express selective publication. Each public state is still
handled once by its own `runState` and each public journal once by its own `runWriter`.
[effectful labels](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Labeled.html) · [local state](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-State-Static-Local.html) · [local writer](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Writer-Static-Local.html)

## Engineering checks

The business verdict alone cannot reveal a refunded exposure or a write to the wrong journal.
Both test programs check the verdict, three state values and five ordered journals. They run
six combinations of scope and threshold, each with and without a public prefix. Two further
cases accept an inner stage and vary only the outer verdict. A failing CoD assertion identifies
the role at its source line. The Haskell checker reports the backend, case, role and values.

<a id="cod-installation-and-check"></a>

### CoD fixtures and checks

`trace` spells expected journals in entry order. The fixture encodes at most three entries,
each from 1 to 9, in one `u64`. `trace()` is empty. `trace(7u64, 1u64, 2u64)` represents
`[7,1,2]`. `with_prefix` accepts an unprefixed batch of at most two entries. The two `for`
loops below visit the same rows and prefix choices as Haskell's two inner `forM_` loops.

```cod
func trace() -> u64 { 0u64 }
func trace(a:u64) -> u64 { a }
func trace(a:u64, b:u64) -> u64 { 10u64 * a + b }
func trace(a:u64, b:u64, c:u64) -> u64 { 10u64 * trace(a, b) + c }

func with_prefix(enabled:bool, first:u64, entries:u64) -> u64 {
    if (not enabled) return entries
    if (entries == 0u64) return first
    if (entries < 10u64) return trace(first, entries)
    100u64 * first + entries
}

func reports() yields { scope:StageScope; limit:u64; ids:u64; gains:u64; } {
    val per_acquisition <- PerAcquisition:StageScope
    val per_tile <- PerTile:StageScope
    yield(per_acquisition -> scope, 0u64 -> limit, trace(1u64) -> ids, trace(3u64) -> gains)
    yield(per_tile -> scope, 0u64 -> limit, trace(1u64) -> ids, trace(3u64) -> gains)
    yield(per_acquisition -> scope, 1u64 -> limit, trace(2u64) -> ids, trace(4u64) -> gains)
    yield(per_tile -> scope, 1u64 -> limit, trace(1u64, 2u64) -> ids, trace(3u64, 4u64) -> gains)
    yield(per_acquisition -> scope, 2u64 -> limit, trace() -> ids, trace() -> gains)
    yield(per_tile -> scope, 2u64 -> limit, trace() -> ids, trace() -> gains)
}

func prefixes() yields bool { yield(false); yield(true) }
```

```cod
proc verify(here action() -> bool, limit:u64, want_ok:bool, want_ids:u64, want_gains:u64) {
    { val accepted <-; val serial <-; val credits <-; val gain <-;
      val acquired_log <-; val rejects_log <-; val passes_log <-;
      val defects_log <-; val samples_log <-; } := observe() { action() }

    assert accepted == want_ok
    assert defects_log == want_ids
    assert samples_log == want_gains
    if (limit == 0u64) {
        assert serial == 1u64
        assert credits == 6u64
        assert gain == 4u64
        assert acquired_log == trace(1u64)
        assert rejects_log == trace()
        assert passes_log == trace(1u64)
    } else {
        assert serial == 2u64
        assert credits == 4u64
        assert gain == 5u64
        assert acquired_log == trace(1u64, 2u64)
        if (limit == 1u64) {
            assert rejects_log == trace(1u64)
            assert passes_log == trace(2u64)
        } else {
            assert rejects_log == trace(1u64, 2u64)
            assert passes_log == trace()
        }
    }
}

proc check_all() {
    for { val scope <-; val limit <-; val ids <-; val gains <-; } := reports() {
        for val prefix <- prefixes() {
            val want_ids <- with_prefix(prefix, 7u64, ids)
            val want_gains <- with_prefix(prefix, 8u64, gains)
            verify(limit, limit < 2u64, want_ids, want_gains) { tile(scope, limit, prefix) }
        }
    }
}

check_all()
```

The nested control uses the same verifier. The inner inspection succeeds and charges once,
including when the enclosing stage subsequently rejects the journals.

```cod
proc nested(allow_outer:bool) -> bool {
    defects(7u64)
    samples(8u64)
    return stage() { stage() { inspect(0u64) } and allow_outer }
}

verify(0u64, false, trace(7u64), trace(8u64)) { nested(false) }
verify(0u64, true, trace(7u64, 1u64), trace(8u64, 3u64)) { nested(true) }
```

<a id="haskell-check-with-both-backends"></a>

### Haskell fixtures and checks

Both backends are checked against the same expectations. `verify` decodes the native
runner's result only after the action finishes. A swapped route between same-typed writers
can compile and still fail one of these named comparisons.

```haskell
module Main where

import Control.Monad (forM_, unless)
import Effectful (Eff)
import Inspector (StageScope(..))
import qualified Inspector as I
import qualified Labeled as L
import qualified PortLabeled as EffPorts
import qualified PortStacked as Mtl
import qualified Stacked as S

verify label limit wantOK wantIDs wantGains result = do
  let ((((((((ok, ids), gains), acquired), rejects), passes), serial), credits), gain) = result
      (wantSerial, wantCredits, wantGain, wantAcquired, wantRejects, wantPasses) =
        case limit of
          0 -> (1, 6, 4, [1],   [],    [1])
          1 -> (2, 4, 5, [1,2], [1],   [2])
          _ -> (2, 4, 5, [1,2], [1,2], [])
      same field want got =
        unless (got == want) $
          fail (label ++ " / " ++ field ++ ": expected " ++ show want ++ ", got " ++ show got)
  same "verdict"  wantOK       ok
  same "serial"   wantSerial   serial
  same "credits"  wantCredits  credits
  same "gain"     wantGain     gain
  same "acquired" wantAcquired acquired
  same "rejects"  wantRejects  rejects
  same "passes"   wantPasses   passes
  same "defects"  wantIDs      ids
  same "samples"  wantGains    gains

nestedMtl allowOuter = S.runStack $ do
  I.defects Mtl.ports 7 >> I.samples Mtl.ports 8
  S.stage $ do
    inner <- S.stage (I.inspect Mtl.ports 0)
    pure (inner && allowOuter)

nestedEff :: Bool -> Eff L.Eight Bool
nestedEff allowOuter = do
  I.defects EffPorts.ports 7 >> I.samples EffPorts.ports 8
  L.stage $ do
    inner <- L.stage (I.inspect EffPorts.ports 0)
    pure (inner && allowOuter)

main :: IO ()
main = do
  let reports =
        [ (PerAcquisition, 0, [1], [3]), (PerTile, 0, [1],   [3])
        , (PerAcquisition, 1, [2], [4]), (PerTile, 1, [1,2], [3,4])
        , (PerAcquisition, 2, [],  []),  (PerTile, 2, [],    [])
        ]
      backends = [("mtl", Mtl.run, nestedMtl), ("effectful", EffPorts.run, L.runEight . nestedEff)]
  forM_ backends $ \(backend, run, nested) -> do
    forM_ reports $ \(scope, limit, ids, gains) ->
      forM_ [False, True] $ \prefix -> do
        let wantIDs   = (if prefix then [7] else []) ++ ids
            wantGains = (if prefix then [8] else []) ++ gains
            label     = backend ++ " " ++ show (scope, limit, prefix)
        verify label limit (limit < 2) wantIDs wantGains (run scope limit prefix)
    verify (backend ++ " / outer rejects") (0 :: I.N) False [7]   [8]   (nested False)
    verify (backend ++ " / outer accepts") (0 :: I.N) True  [7,1] [8,3] (nested True)
```

<a id="what-changes-when-another-author-extends-the-program"></a>
<a id="a-pre-existing-report-and-an-independently-added-journal"></a>

### Observing a routed write and testing an extension

In `PortStacked`, `Stacked`, `PortLabeled` and `Labeled`, each writer runner returns its result and its own output.
Every state runner returns its result and its own final state. The natural nesting gives
the following results, in elimination order (verdict, defects, samples, acquired, rejects,
passes, serial, credits, gain). These are the return values of the eight eliminations.
The acquisition itself communicates through the individual effects shown above.

```haskell
-- Empty report. Reject the first acquisition, accept the second.
run PerAcquisition 1 False == ((((((((True,[2]),[4]),[1,2]),[1]),[2]),2),4),5)
run PerTile 1 False == ((((((((True,[1,2]),[3,4]),[1,2]),[1]),[2]),2),4),5)

-- An earlier defect 7 has a matching sample 8.
run PerAcquisition 1 True == ((((((((True,[7,2]),[8,4]),[1,2]),[1]),[2]),2),4),5)
run PerTile 2 True == ((((((((False,[7]),[8]),[1,2]),[1,2]),[]),2),4),5)
```

For all twelve inputs, combine both values of `prefix` with each staging scope with
`k = 0, 1, 2`. The [table above](#which-report-is-correct) gives the verdict and all eight effect outputs
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
In the Ports version, the original leaf can keep its existing record; a wrapper may request
the new telemetry operation separately. If the published record grows a field, count the
change at each record construction. In CoD, add a journal identity, bind it to a cell at
entry, and have the wrapper call an ordinary append procedure for it. The original eight
operations and both scopes keep their definitions. The observer also checks the new journal.
An independent lot-number binding tests the other axis by enclosing the unchanged
acquisition in a new scope. If the business later
includes telemetry in the provisional evidence, the stage must also implement its new
publication contract.

These are costs of routing repeated instances of two effect families. The paper's
[interface/transformer matrix](../ComposingEffectsAndFocusedContexts.md#where-the-m--n-obligations-arise)
counts independently extensible *interfaces and layer constructors*. Three states and
five writers by themselves establish neither fifteen adapters nor an asymptotic bound.
