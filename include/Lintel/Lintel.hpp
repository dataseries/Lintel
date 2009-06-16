/* -*-C++-*- */
/*
   (c) Copyright 2008-2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

// TODO: complete the documentation
/** \namespace Lintel
    \brief a utility library 

    \section Data Structures

      - Double-ended queue: Deque
      - Priority queue: PriorityQueue
      - Constant/interned strings: ConstantString
      - Hashing:
         - map-like interface: HashMap
         - Hashing functions: HashFns
         - Basic hash table: HashTable
	 - quickly prunable table: RotatingHashMap
	 - hash operations on a tuple: HashTupleStats
	 - unique presence hash: HashUnique
    
    \section Stats

      - Basic statistics: Stats
      - Quantile statistics: StatsQuantile

    \section Program development

      - invariants that use boost for messages: AssertBoost
      - assertions that use exceptions: AssertException
      - interactions with the clock, including using the cycle counter: Clock
      - macros for annotating functions: CompilerMarkup
      - safely comparing doubles, special values: Double
      - calculate least squares: LeastSquares

    \section Programs

      - Graphing: \b mercury-plot
      - Software checkout, building, committing, reviewing: \b deptool
      - Parallel command execute: \b batch-parallel
      - File locking: \b flock

    \section Misc

      - Notes on using the stl: LintelStlNotes

    \section Obsolete or being removed
    
      - LintelAssert

    \bug Missing references to lots of the other bits.
*/

namespace Lintel {
}
