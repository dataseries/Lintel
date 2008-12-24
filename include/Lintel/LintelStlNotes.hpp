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

    \list Double-ended queue: Deque

    \list Priority queue: PriorityQueue
    
    \section Stats

    \list Basic statistics: Stats

    \list Quantile statistics: StatsQuantile

    \section Programs

    \list Graphing: \b mercury-plot

    \list Software checkout, building, committing, reviewing: \b deptool

    \list Parallel command execute: \b batch-parallel

    \list File locking: \b flock

    \section Misc

    \list Notes on using the stl: LintelStlNotes

    \bug Missing references to lots of the other bits.
*/

namespace Lintel {
}
