/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    This utility function compares the iterator range
*/

#ifndef LINTEL_STLUTILITY_HPP
#define LINTEL_STLUTILITY_HPP


namespace lintel {

template <class InputIterator1, class InputIterator2>
bool iteratorRangeEqual(InputIterator1 first1, InputIterator1 last1,
			InputIterator2 first2, InputIterator2 last2) {
    while (first1 != last1) {
	if (first2 == last2) { 
	    return false;
	}
	if (*first1 != *first2) { 
	    return false;
	}
	++first1; 
	++first2;
    }
    return first2 == last2;
}

}

#endif
