/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief A class for substring matching based on the Boyer-Moore-Horspool algorithm.
*/

#ifndef LINTEL_BOYERMOOREHORSPOOL_HPP
#define LINTEL_BOYERMOOREHORSPOOL_HPP

/* -*-C++-*-
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <sys/types.h>
#include <stdint.h>

#include <cstddef>
#include <limits>
#include <string>

namespace lintel {
    /** \brief A class that encapsulates the BoyerMooreHorspool string matching algorithm */
    class BoyerMooreHorspool {
    public:
	/** Constructs a new @c BoyerMooreHorspool that is capable of searching for the specified
	    @param needle within a given string. The needle is fixed for the object, whereas the
	    haystack is provided to the matches method. @param needle_length is the length of the
	    needle. 
	    
	    @param needle the search data for this class
	    @param needle_length the length of the search data
	*/
	BoyerMooreHorspool(const void *needle, size_t needle_length);

	/** Constructs a new @c BoyerMooreHorspool that is capable of searching for the specified
	    @param needle within a given string. The needle is fixed for the object, whereas the
	    haystack is provided to the matches method. 
	    
	    @param needle the search data for this class
	*/
	BoyerMooreHorspool(const std::string &needle);
	~BoyerMooreHorspool();
	
	/** Returns true if and only if needle is a substring of @param haystack\. 
	    
	@param haystack the data to match against
	@param hay_len the length of the data to match against
	*/
	bool matches(const void *haystack, size_t hay_len) const{
	    return find(haystack, hay_len) != npos;
	}
	
	/** Returns offset in haystack where needle can be found in @param haystack
	    and npos if the needle cannot be found.
	    
	    @param haystack the data to match against
	    @param hay_len the length of the data to match against
	*/

	size_t find(const void *haystack, size_t hay_len) const;
	
	/// value returned for no match, same as for string::find
	static const size_t npos = static_cast<size_t>(-1);
    private:
	void init(size_t needle_len);
	static const uint32_t uint8_max = 255;

	size_t bad_char_shift[uint8_max + 1];
	const uint8_t * const needle;
	size_t needle_length;
	size_t last;
    };
}
#endif
