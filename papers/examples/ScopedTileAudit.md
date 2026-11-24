# One scoped audit, written three ways

This control isolates how one installed method selects its environment. For independent
policies whose nesting changes the public report, see [Composing retry and publication](ComposedTileReport.md).

A tile records one inspection attempt before it is validated. A caller can request an
audit addressed to the installation-time destination, or to the destination active in the
tile. Either attempt remains observable if validation fails. This is the small source
comparison for the [additive-extension claim](../ComposingEffectsAndFocusedContexts.md#additive-source-extension).

## Contract and observations

Install the recorder at destination **1**, change the caller's destination to **9**, then
enter a tile which selects **2**. Emit one attempt. Read the destination *after* emitting
and return it on success, or attach it to a typed fault on failure. Inspect the caller's
restored destination after the normal return or catch. Start with an empty audit in each run.

| Invocation | Tile succeeds | Tile fails | Audit left in either run | Restored caller |
| --- | --- | --- | --- | ---: |
| Installation-time | returns 2 | fault carries 2 | one attempt at 1 | 9 |
| Current | returns 2 | fault carries 2 | one attempt at 2 | 9 |

Saving 9 at tile entry instead of 1 at installation fails the first row. Wrapping the
entire tile in the saved environment can get the log right but return or throw the wrong
destination. Discarding the append on failure fails the last two audit observations.
A fixed numerical answer per mode cannot pass an independent observation of the write.
In CoD a counter and the last destination in the recorder's enclosing lexical scope represent
the whole observable trace of this one-append protocol. The Haskell observers check that a
writer outside the error handler produced exactly the expected singleton list.

## CoD

The destination is a scalar. We use the explicit paper spelling `val symbol` for the
independent bits binding that the current design text spells `symbol @destination:u64`.
Installing the procedure with a body gives it access to two live locals in `check`.
Both invocation modes write to these same locals. Only the choice of destination changes.
A checked failure occurs after the write, which lets the observer distinguish a restored
binding from a lost audit entry. [Value and reference symbols](../../design/SymbolicDynamicScope.md#explicit-ref-and-ptr-symbols-backed-by-a-region-symbol)

```cod
using "std:exceptions" Exception;

data TileFault <: Exception { destination:u64; }
val symbol @destination:u64
symbol @attempt:proc() -> void

proc tile(replay:bool, bad:bool) -> u64 throws TileFault {
    effect @destination <- 2u64
    if (replay) { perform @attempt() } else { behave @attempt() }
    if (bad) { throw TileFault { destination <- @destination } }
    return @destination
}

proc check(replay:bool, bad:bool) -> void {
    val count <- 0u64
    val last <- 0u64
    effect @destination <- 1u64
    effect @attempt() -> void {
        count <- count + 1u64
        last <- @destination
        return
    }
    effect @destination <- 9u64
    try {
        val where_seen <- tile(replay, bad)
        assert bad == false
        assert where_seen == 2u64
    } catch (e:const TileFault) {
        assert bad
        assert e.destination == 2u64
    }
    assert @destination == 9u64
    assert count == 1u64
    if (replay) { assert last == 1u64 } else { assert last == 2u64 }
    return
}
```

The recorder is activated at destination 1. A captured call replays that value. A current
call reads 2 inside the tile. Neither invocation replaces the storage of `count` or
`last`. This is why the example can test both modes and observe a write after unwinding
with just ordinary locals and a value symbol.
[Installing a local procedure body](../../design/SymbolicDynamicScope.md#installing-a-local-procedure-body)
· [Captured values and lexical access](../../design/SymbolicDynamicScope.md#captured-values-and-call-behavior)
· [Exception construction and catch](../../design/ExceptionControl.md#exception-construction-throw-and-rethrow)

## Haskell with mtl

Build an ordinary recorder that accepts a mode, and pass it to the tile. The tile chooses the mode at the point of use. A dedicated `MonadFlag`
class would add little for this closed program. `saved <- ask` is the installation event.
Only the recorder runs under `local (const saved)` in replay mode. The ordinary
`ExceptT` action still validates under the tile's current reader. Here the entire reader
is one destination. In a record with unrelated fields, save and restore the intended
projection, or put the relevant configuration in its own component.

```haskell
module TileMtl where

import Control.Monad.Except
import Control.Monad.Reader
import Control.Monad.Writer.Strict
import Data.Word (Word64)

data TileFault = TileFault Word64 deriving (Eq, Show)
type Run = ReaderT Word64 (Writer [Word64])

tile :: (Bool -> Run ()) -> Bool -> Bool -> ExceptT TileFault Run Word64
tile emit replay bad = local (const 2) $ do
  lift (emit replay)
  d <- ask
  if bad then throwError (TileFault d) else pure d

run :: Bool -> Bool -> ((Either TileFault Word64, Word64), [Word64])
run replay bad = runWriter $ flip runReaderT 1 $ do
  saved <- ask
  let mark :: Run ()
      mark = ask >>= \d -> tell [d]
      emit pinned = if pinned then local (const saved) mark else mark
  local (const 9) $ do
    result <- runExceptT (tile emit replay bad)
    after <- ask
    pure (result, after)

check :: Bool -> Bool -> Bool
check replay bad =
  run replay bad == ((if bad then Left (TileFault 2) else Right 2, 9),
                     [if replay then 1 else 2])
```

This version uses the stock instances and adds **zero** transformer instances. The
explicit action parameter makes its dependency visible in the tile signature. A client
needing a reusable nominal operation across many modules could put an audit service in a
reader record or define a capability class. Those are additional design choices. For this
fixture the smaller explicit action is a sufficient control.
[mtl Reader](https://hackage-content.haskell.org/package/mtl-2.3.1/docs/Control-Monad-Reader-Class.html)
· [ExceptT](https://hackage-content.haskell.org/package/transformers-0.6.2.0/docs/Control-Monad-Trans-Except.html)

## Haskell with a dynamically interpreted effectful operation

Here `Flag` is an independently interpretable effect. `Replay` asks its interpreter
to use the destination saved at installation. `At` carries the destination resolved at
the call. Sampling before `send` makes the latter rule explicit. `audit` installs one
interpretation of both requests. Replacing that interpretation at its installation site
requires no change to the tile. The static reader, error and writer effects use their
stock handlers. The code uses the `effectful-core` 2.7.1.2 interfaces.

```haskell
{-# LANGUAGE DataKinds, FlexibleContexts, GADTs, LambdaCase, TypeApplications #-}
{-# LANGUAGE TypeFamilies, TypeOperators, KindSignatures #-}
module TileEff where

import Data.Word (Word64)
import Effectful
import Effectful.Dispatch.Dynamic
import qualified Effectful.Error.Static as E
import qualified Effectful.Reader.Static as R
import qualified Effectful.Writer.Static.Local as W

data TileFault = TileFault Word64 deriving (Eq, Show)
data Flag :: Effect where
  Replay  :: Flag m ()
  At      :: Word64 -> Flag m ()
type instance DispatchOf Flag = 'Dynamic

audit :: (R.Reader Word64 :> es, W.Writer [Word64] :> es)
      => Eff (Flag ': es) a -> Eff es a
audit body = do
  saved <- R.ask @Word64
  interpret_ (\case
    Replay -> W.tell [saved]
    At d   -> W.tell [d]) body

tile :: (Flag :> es, R.Reader Word64 :> es, E.Error TileFault :> es)
     => Bool -> Bool -> Eff es Word64
tile replay bad = R.local @Word64 (const 2) $ do
  if replay then send Replay else R.ask @Word64 >>= send . At
  d <- R.ask @Word64
  if bad then E.throwError (TileFault d) else pure d

run :: Bool -> Bool -> ((Either TileFault Word64, Word64), [Word64])
run replay bad = runPureEff $ W.runWriter @[Word64] $ R.runReader (1 :: Word64) $
  audit $ R.local @Word64 (const 9) $ do
    result <- E.runErrorNoCallStack @TileFault (tile replay bad)
    after <- R.ask @Word64
    pure (result, after)

check :: Bool -> Bool -> Bool
check replay bad =
  run replay bad == ((if bad then Left (TileFault 2) else Right 2, 9),
                     [if replay then 1 else 2])
```

The row constraint names the effects used by the tile. The GADT and interpreter have value
when another module can supply a genuinely different interpretation. A reusable library
could factor out the two request modes, and a concrete application could instead pass an
action directly, as in the mtl version. Charging the shared library once and each use once
is essential when moving from this one-operation example to M interfaces.
[Dynamic dispatch](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Dispatch-Dynamic.html)
· [Reader](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Reader-Static.html)
· [Writer](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Writer-Static-Local.html)
· [Error](https://hackage-content.haskell.org/package/effectful-core-2.7.1.2/docs/Effectful-Error-Static.html)

## Adding a second independently installed operation

The previous listing includes the cost of the failure observer and the audit journal.
Here is a smaller positive example about *extending the vocabulary*. A threshold policy
is published independently of the audit operation. Clients can query that installed policy
with a fresh score under either invocation mode. The run sets the threshold to 12, installs
the policy, and later raises the tile threshold to 18. For score 15 the two answers must be
True and False. Replacing the predicate at installation should require no edit to the calls.
No audit sink, exception or image implementation is needed for this separate question.

```cod
val symbol @cutoff:i64
proc @defect(score:i64) -> bool { return score >= @cutoff }

effect @cutoff <- 12i64
effect proc @defect;
effect @cutoff <- 18i64
assert perform @defect(15i64)
assert (behave @defect(15i64)) == false
```

The first two lines are the new vocabulary and its application method. The installation,
rebinding and both observations take five more statements. A tile already using some other
operation needs no boilerplate to admit this one. For the Haskell comparisons below use
`Int64` from `Data.Int`, and the imports of the corresponding complete listing above.

**mtl.** A compact baseline explicitly gives clients a query action. This helper accepts the
predicate as an argument so its interpretation can be replaced. For this scalar setting it
can be shared as written by other predicates with the same interface.

```haskell
type Query = Bool -> Int64 -> Reader Int64 Bool

install :: (Int64 -> Int64 -> Bool) -> (Query -> Reader Int64 a) -> Reader Int64 a
install accepts client = do
  saved <- ask
  client $ \replay score -> do
    cutoff <- if replay then pure saved else ask
    pure (accepts score cutoff)

answers :: (Bool, Bool)
answers = flip runReader 12 $ install (>=) $ \query ->
  local (const 18) $ (,) <$> query True 15 <*> query False 15
```

**effectful.** For a nominal, independently interpretable request, both modes can share one
constructor. `Nothing` requests the interpretation's installation-time threshold; `Just n`
supplies a call-time threshold. A client that wants the current value reads it where it
makes the request. The installer accepts the same replaceable predicate as the mtl version.

```haskell
data Defect :: Effect where
  Test :: Maybe Int64 -> Int64 -> Defect m Bool
type instance DispatchOf Defect = 'Dynamic

policy :: R.Reader Int64 :> es => (Int64 -> Int64 -> Bool)
       -> Eff (Defect ': es) a -> Eff es a
policy accepts body = do
  saved <- R.ask @Int64
  interpret_ (\case Test setting n -> pure (accepts n (maybe saved id setting))) body

answers :: (Bool, Bool)
answers = runPureEff $ R.runReader (12 :: Int64) $ policy (>=) $
  R.local @Int64 (const 18) $ do
    pinned <- send (Test Nothing 15)
    current <- R.ask @Int64 >>= \n -> send (Test (Just n) 15)
    pure (pinned, current)
```

The policy-only listings have **7, 10 and 14 nonblank lines**, respectively, including the
published operation (or query type), installer, client and both observations. Standard
library imports are shared with the preceding examples. These counts describe these
particular readable spellings. More importantly, there is a small argument about what
they express. Fix any shared total predicate P on a score and a threshold, an installed
threshold a, a later threshold b and a fresh score x. The CoD captured call and each
Haskell saved branch evaluate P(x,a). The CoD current call and each Haskell current
branch evaluate P(x,b). Thus all three yield the same pair for every x, a and b representable
by the fixture, including a=b and threshold equality. The argument is scoped to this
pure predicate protocol and admitted callers. A Haskell library can hide the raw request
constructor if its client interface should permit only these two choices.

The three versions give the same two answers. Changing the installed predicate to strict
greater-than and testing a score equal to the cutoff distinguishes a real rule replacement
from a baked-in comparison. In CoD that variation changes the selected method at installation;
in Haskell it changes the argument passed to `install` or `policy`. The Haskell versions can
be factored further when this pattern recurs; count those helpers once, exactly as the CoD
dispatcher is counted once. A static reader plus an explicit saved value is also sufficient
for this one concrete policy. It does not provide a separately named dynamic operation
until the application supplies one. This positive example shows why a whole application
might value the built-in modes even when its complete audit harness is longer.

## Where is the saving?

Compare the same decisions in each program, including the integration and observer.

| Responsibility | CoD | mtl | effectful, dynamic `Flag` |
| --- | --- | --- | --- |
| Publish an audit operation | A named symbolic method. | An ordinary action passed to its consumer. | A request family and an interpreter. |
| Retain the installation-time choice | Activation snapshots the method's required bindings. | Save the destination and construct the replay action. | Save the destination in the interpreter. |
| Choose current or saved behavior | Two built-in invocation forms on the same symbol. | Choose an action with or without the saved reader context. | Choose a request; the current request carries the call-site destination. |
| Supply the trace sink and typed failure | Two captured lexical cells and a checked exception. | Writer and an ExceptT action under ReaderT. | Built-in writer and error handlers with explicit installation. |

For a length check, count all nonblank lines of each complete tile-audit listing in the first three language sections, including
language pragmas, imports, setup and the observation check. The counts are **33 CoD,
26 mtl and 37 effectful**. Excluding only pragmas, module headers and imports (including
CoD's `using` line), the counts are **32, 21 and 28**. All signatures and all
harness code remain in the second count. CoD checks the individual fields along the two exit
paths; the Haskell check compares a tuple and a list. Counting lines also rewards layout.
The substantive saving is narrower: installation and invocation in CoD carry the capture
protocol once for all symbolic methods, and this tile needs no explicit forwarding parameter.
The explicit mtl encoding is particularly short for a single reader and a single audit.
No claim of a shortest possible implementation follows from these listings.

A plain change of focus provides a control where little separates the libraries. Suppose
`readPixel` is already supplied and the problem only asks for its value under a changed
coordinate. CoD can use a scoped binding, and either Haskell library can use its existing
reader operation. No saved/current distinction or custom effect is needed in this control.

```cod
val eastern <- {{
    effect @col <- east(@col)
    behave @pixel()
}}
```

```haskell
-- mtl, with the column carried by this reader
eastern = local east readPixel

-- effectful, with the same column reader
eastern = R.local east readPixel
```

For the full audit fixture, call `check` with both Boolean arguments independently set
to true and false. Each CoD call has four groups of assertions: success or fault carries
the tile destination, caller restoration, write count and selected target. The Haskell
functions return True on the same four inputs. These are paper expectations checked against
the source contracts. They establish the observations listed above, without establishing
general equivalence for arbitrary effect rows, resumptions, escaping capabilities or owners.
