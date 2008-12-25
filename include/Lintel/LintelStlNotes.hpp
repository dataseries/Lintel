/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** \file LintelStlNotes.hpp
    \brief see man LintelStlNotes
*/

/** \class LintelStlNotes
    \brief Notes on using the C++ STL
    
    Sorting:

    \code
    struct sortByFn {
    public:
        bool operator () (const T &a, const T &b) const {
  	    return a < b; // less than things go first after sort
       }
    };

    vector<T> sorted_vec;
    sort(sorted_vec.begin(), sorted_vec.end(), sortByFn());
    \endcode
*/
class LintelStlNotes {
}

// TODO: move this somewhere else
// TODO: complete the documentation
/** \namespace Lintel
    \brief a utility library 

    \section Data Structures

      - Double-ended queue: Deque
      - Priority queue: PriorityQueue
    
    \section Stats

      - Basic statistics: Stats
      - Quantile statistics: StatsQuantile

    \section Programs

      - Graphing: \b mercury-plot
      - Software checkout, building, committing, reviewing: \b deptool
      - Parallel command execute: \b batch-parallel
      - File locking: \b flock

    \section Misc

      - Notes on using the stl: LintelStlNotes

    \bug Missing references to lots of the other bits.
*/

namespace Lintel {
}
