/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

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

