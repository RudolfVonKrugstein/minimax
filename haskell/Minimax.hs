import Maybe
import Data.List

data Player = Player1 | Player2

class Position a where
	winscore     :: a -> Int
	score        :: a -> Int
	gameOver     :: a -> Bool
	moves        :: a -> [Move a]
	activePlayer :: a -> Player

data Position a => Move a = Move {
  usedDepth    :: Int,
  newPos       :: a	
}

data Position a => GameTree a = Node {
  depth       :: Int,
  pos         :: a,
  children    :: [GameTree a]
}

gameTree :: Position a => a -> GameTree a
gameTree pos = moveToGameTree (Move 0 pos)
  where
    moveToGameTree :: Position b => Move b -> GameTree b
    moveToGameTree m = Node (usedDepth m) (newPos m) (map moveToGameTree (moves (newPos m)))



prune :: Position a => Int -> GameTree a -> Maybe (GameTree a)
prune d node
  | d > 0     = Just $ Node (depth node) (pos node) $ catMaybes $ map (prune (d - depth node)) (children node)
  | otherwise = Nothing
   

minimax :: Position a => GameTree a -> Int
minimax (Node _ pos [])    = score pos
minimax (Node _ _ children) = maximum $ map (negate . minimax) $ children

alphaBeta :: Position a => GameTree a -> Int
alphaBeta n = alphaBeta' (negate ws) (ws) n
  where
    ws = winscore (pos n)
    alphaBeta' :: Position a => Int -> Int -> GameTree a -> Int
    alphaBeta' _     _    (Node _ pos [])       = score pos
    alphaBeta' alpha beta (Node _ _ children) = fst $ foldl' sub (alpha, beta) children
      where
        sub :: Position a => (Int,Int) -> GameTree a -> (Int, Int)
        sub (alpha,beta) n
          | alpha >= beta = (alpha, beta)
          | otherwise     = (max alpha $ negate $ alphaBeta' (negate beta) (negate alpha) n, beta)

main = undefined
