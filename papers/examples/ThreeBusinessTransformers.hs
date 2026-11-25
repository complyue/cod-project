{-# LANGUAGE GeneralizedNewtypeDeriving #-}

module Main where

import Control.Monad (forM_, unless)
import Control.Monad.Trans.Class (MonadTrans (lift))
import Control.Monad.Trans.Reader (ReaderT, ask, local, mapReaderT, runReaderT)
import Control.Monad.Trans.State.Strict (StateT (..), mapStateT, state)
import Control.Monad.Trans.Writer.Strict (WriterT, mapWriterT, pass, runWriterT, tell)
import Data.Functor.Identity (Identity, runIdentity)

type Flag = (Int, Int)  -- Acquisition serial and sampled gain.

data CameraState = CameraState {nextSerial :: Int, credits :: Int}
  deriving (Eq, Show)

class Monad m => MonadCamera m where
  acquire :: m (Maybe Int)

class Monad m => MonadGain m where
  gain :: m Int
  withGain :: Int -> m a -> m a

class Monad m => MonadReport m where
  flag :: Flag -> m ()
  publishIf :: (a -> Bool) -> m a -> m a

newtype CameraT m a = CameraT (StateT CameraState m a)
  deriving (Functor, Applicative, Monad)

newtype GainT m a = GainT (ReaderT Int m a)
  deriving (Functor, Applicative, Monad)

newtype ReportT m a = ReportT (WriterT [Flag] m a)
  deriving (Functor, Applicative, Monad)

instance MonadTrans CameraT where lift = CameraT . lift
instance MonadTrans GainT where lift = GainT . lift
instance MonadTrans ReportT where lift = ReportT . lift

-- Each package supplies the native instance for its own contract.
instance Monad m => MonadCamera (CameraT m) where
  acquire = CameraT $ state $ \s ->
    if credits s <= 0
      then (Nothing, s)
      else (Just (nextSerial s), CameraState (nextSerial s + 1) (credits s - 1))

instance Monad m => MonadGain (GainT m) where
  gain = GainT ask
  withGain n (GainT action) = GainT (local (const n) action)

instance Monad m => MonadReport (ReportT m) where
  flag entry = ReportT (tell [entry])
  publishIf ok (ReportT action) = ReportT $ pass $ do
    result <- action
    pure (result, if ok result then id else const [])

-- These three crossed instances suffice for CameraT (GainT (ReportT Identity)).
instance MonadGain m => MonadGain (CameraT m) where
  gain = lift gain
  withGain n (CameraT action) = CameraT (mapStateT (withGain n) action)

instance MonadReport m => MonadReport (CameraT m) where
  flag = lift . flag
  publishIf ok (CameraT action) = CameraT $ StateT $ \s ->
    publishIf (ok . fst) (runStateT action s)

instance MonadReport m => MonadReport (GainT m) where
  flag = lift . flag
  publishIf ok (GainT action) = GainT (mapReaderT (publishIf ok) action)

-- Supporting the other orders adds the three converse crossings.
instance MonadCamera m => MonadCamera (GainT m) where
  acquire = lift acquire

instance MonadCamera m => MonadCamera (ReportT m) where
  acquire = lift acquire

instance MonadGain m => MonadGain (ReportT m) where
  gain = lift gain
  withGain n (ReportT action) = ReportT (mapWriterT (withGain n) action)

attempt :: (MonadCamera m, MonadGain m, MonadReport m) => Bool -> m Bool
attempt accepted = publishIf id $ do
  acquired <- acquire
  case acquired of
    Nothing -> pure False
    Just serial -> do
      sampled <- gain
      flag (serial, sampled)
      pure accepted

type Outcome = ((Bool, Bool), Int)

inspection :: (MonadCamera m, MonadGain m, MonadReport m) => (Bool, Bool) -> m Outcome
inspection (acceptFirst, acceptSecond) = do
  first <- withGain 7 (attempt acceptFirst)
  second <- attempt acceptSecond
  restored <- gain
  pure ((first, second), restored)

runCamera :: CameraState -> CameraT m a -> m (a, CameraState)
runCamera s (CameraT action) = runStateT action s

runGain :: Int -> GainT m a -> m a
runGain n (GainT action) = runReaderT action n

runReport :: ReportT m a -> m (a, [Flag])
runReport (ReportT action) = runWriterT action

type Application = CameraT (GainT (ReportT Identity))

runApplication :: CameraState -> Application a -> ((a, CameraState), [Flag])
runApplication s = runIdentity . runReport . runGain 3 . runCamera s

data Observation = Observation Outcome CameraState [Flag]
  deriving (Eq, Show)

cameraFirst :: ((Outcome, CameraState), [Flag]) -> Observation
cameraFirst ((outcome, finalCamera), published) = Observation outcome finalCamera published

reportFirst :: ((Outcome, [Flag]), CameraState) -> Observation
reportFirst ((outcome, published), finalCamera) = Observation outcome finalCamera published

-- Stack order is outermost first. Each interpreter handles its own layer.
orders :: (Bool, Bool) -> CameraState -> [(String, Observation)]
orders verdicts s =
  [ ("Camera/Gain/Report", cameraFirst $ runApplication s (inspection verdicts))
  , ("Camera/Report/Gain",
     cameraFirst $ runIdentity $ runGain 3 $ runReport $ runCamera s (inspection verdicts))
  , ("Gain/Camera/Report",
     cameraFirst $ runIdentity $ runReport $ runCamera s $ runGain 3 (inspection verdicts))
  , ("Gain/Report/Camera",
     reportFirst $ runIdentity $ runCamera s $ runReport $ runGain 3 (inspection verdicts))
  , ("Report/Camera/Gain",
     reportFirst $ runIdentity $ runGain 3 $ runCamera s $ runReport (inspection verdicts))
  , ("Report/Gain/Camera",
     reportFirst $ runIdentity $ runCamera s $ runGain 3 $ runReport (inspection verdicts))
  ]

main :: IO ()
main = do
  let cases =
        [ ((False, True), 2, Observation ((False, True), 3) (CameraState 3 0) [(2, 3)])
        , ((True, True), 2, Observation ((True, True), 3) (CameraState 3 0) [(1, 7), (2, 3)])
        , ((True, False), 2, Observation ((True, False), 3) (CameraState 3 0) [(1, 7)])
        , ((False, True), 1, Observation ((False, False), 3) (CameraState 2 0) [])
        , ((False, True), 0, Observation ((False, False), 3) (CameraState 1 0) [])
        ]
  forM_ cases $ \(verdicts, budget, expected) ->
    forM_ (orders verdicts (CameraState 1 budget)) $ \(order, observed) ->
      unless (observed == expected) $
        fail (order ++ " with " ++ show budget ++ " credits: " ++ show observed)
  putStrLn "Five inspection cases agree in all six transformer orders."
