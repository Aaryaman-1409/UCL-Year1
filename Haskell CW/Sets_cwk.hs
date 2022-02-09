module Coursework where

{-
  Your task is to design a datatype that represents the mathematical concept of a (finite) set of elements (of the same type).
  We have provided you with an interface (do not change this!) but you will need to design the datatype and also 
  support the required functions over sets.
  Any functions you write should maintain the following invariant: no duplication of set elements.

  There are lots of different ways to implement a set. The easiest is to use a list
  (as in the example below). Alternatively, one could use an algebraic data type, or 
  wrap a binary search tree.
  Extra marks will be awarded for efficient implementations if appropriate.

  You are NOT allowed to import anything from the standard library or other libraries.
  Your edit of this file should be completely self-contained.

  DO NOT change the type signatures of the functions below: if you do,
  we will not be able to test them and you will get 0% for that part. While sets are unordered collections,
  we have included the Ord constraint on most signatures: this is to make testing easier.

  You may write as many auxiliary functions as you need. Please include everything in this file.
-}

{-
   PART 1.
   You need to define a Set datatype. Below is an example which uses lists internally.
   It is here as a guide, but also to stop ghci complaining when you load the file.
   Free free to change it.
-}

-- you may change this to your own data type
newtype Set a = Set { unSet :: [a] } deriving(Show)

{-
   PART 2.
   If you do nothing else, at least get the following two functions working. They
   are required for testing purposes.
-}

quicksort :: Ord a => [a] -> [a]
quicksort [] = []
quicksort (h:t) = quicksort [y | y<-t, y<h] ++ [h] ++ quicksort [y | y<-t, y>=h]

removedup :: Ord a => [a] -> [a]
removedup [] = []
removedup (x:xs) = x : removedup [y | y<-xs, y/=x]

extractdup :: Ord a => [a] -> [a]
extractdup xs = removedup [x | x<-xs, count x xs >=2]
   where count x xs = sum[1 | element<-xs, element==x]


-- toList {2,1,4,3} => [1,2,3,4]
-- the output must be sorted.
toList :: Set a -> [a]
toList (Set xs) = xs

-- fromList [2,1,1,4,5] => {2,1,4,5}
fromList :: Ord a => [a] -> Set a
fromList xs = Set (quicksort(removedup xs))

{-
   PART 3.
   Your Set should contain the following functions.
   DO NOT CHANGE THE TYPE SIGNATURES.
-}

-- test if two sets have the same elements.
instance (Ord a) => Eq (Set a) where
  s1 == s2 = toList s1 == toList s2

-- the empty set
empty :: Set a
empty = Set []


-- Set with one element
singleton :: a -> Set a
singleton a = Set [a]


-- insert an element of type a into a Set
-- make sure there are no duplicates!
insert :: (Ord a) => a -> Set a -> Set a
insert a (Set xs) = fromList (a:xs)


-- join two Sets together
-- be careful not to introduce duplicates.
union :: (Ord a) => Set a -> Set a -> Set a
union s1 s2 = fromList (toList s1 ++ toList s2)


-- return the common elements between two Sets
intersection :: (Ord a) => Set a -> Set a -> Set a
intersection s1 s2 = fromList (extractdup (toList s1 ++ toList s2))


-- all the elements in Set A *not* in Set B,
-- {1,2,3,4} `difference` {3,4} => {1,2}
-- {} `difference` {0} => {}
difference :: (Ord a) => Set a -> Set a -> Set a
difference s1 s2 = fromList [x | x<-toList s1, x `notElem` shared]
   where shared = extractdup (toList s1 ++ toList s2)


-- is element *a* in the Set?
member :: (Ord a) => a -> Set a -> Bool
member a s1 = a `elem` toList s1

-- how many elements are there in the Set?
cardinality :: Set a -> Int
cardinality s1 = sum[1 | element<-toList s1]


setmap :: (Ord b) => (a -> b) -> Set a -> Set b
setmap f s= fromList [f x | x<-toList s]


setfoldr :: (a -> b -> b) -> Set a -> b -> b
setfoldr f (Set []) b = b
setfoldr f (Set (x:xs)) b = f x (setfoldr f (Set xs) b)


powerHelper :: [a] -> [[a]]
powerHelper [] = [[]]
powerHelper (x:xs) = [x:ps | ps<-powerHelper xs] ++ powerHelper xs


-- powerset of a set
-- powerset {1,2} => { {}, {1}, {2}, {1,2} }
powerSet :: Set a -> Set (Set a)
powerSet (Set []) = Set [Set []]
powerSet s1 = Set [Set y | y<-ys]
   where ys = powerHelper (toList s1)

-- cartesian product of two sets
cartesian :: Set a -> Set b -> Set (a, b)
cartesian (Set xs) (Set ys)= Set [(x,y)| x <- xs, y <- ys]


-- partition the set into two sets, with
-- all elements that satisfy the predicate on the left,
-- and the rest on the right
partition :: (a -> Bool) -> Set a -> (Set a, Set a)
partition f s1 = (Set [x | x<-l, f x], Set [x | x<-l, not (f x)])
   where l = toList s1



{-
   On Marking:
   Be careful! This coursework will be marked using QuickCheck, against Haskell's own
   Data.Set implementation. Each function will be tested for multiple properties.
   Even one failing test means 0 marks for that function.

   Marks will be lost for too much similarity to the Data.Set implementation.

   Pass: creating the Set type and implementing toList and fromList is enough for a
   passing mark of 40%.

-}
