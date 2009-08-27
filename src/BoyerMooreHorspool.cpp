/* -*-C++-*-
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <string.h>
#include <limits>

#include <boost/static_assert.hpp>

#include <Lintel/AssertBoost.hpp>

#include <Lintel/BoyerMooreHorspool.hpp>

// TODO-done: put in reference to textbook describing this algorithm.
//
// See wikipedia article for description of algorithm
// http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
//
// May want to consider ordinary Boyer-Moore or a variant that consumes 2 bytes
// at a time

namespace lintel {
namespace {
    uint8_t *memDup(const void *in, size_t len) {
	uint8_t *ret = new uint8_t[len];
	memcpy(ret, in, len);
	return ret;
    }
}

BoyerMooreHorspool::BoyerMooreHorspool(const void *needle_v, size_t needle_length) 
    : needle(memDup(needle_v, needle_length)), needle_length(needle_length), 
      last(needle_length - 1) 
{
    INVARIANT(needle_length > 0, "invalid to search for zero length string");

    // initialize the bad character shift array
    for (uint32_t i = 0; i <= uint8_max; ++i) {
        bad_char_shift[i] = needle_length;
    }

    for (size_t i = 0; i < last; ++i) {
        bad_char_shift[needle[i]] = last - i;
    }
}

BoyerMooreHorspool::~BoyerMooreHorspool() {
    delete [] needle;
}


ssize_t BoyerMooreHorspool::find(const void *hay_v, size_t hay_len) const {
    const uint8_t *haystack = reinterpret_cast<const uint8_t *>(hay_v);

    while (hay_len >= needle_length) {
        for (size_t i = last; haystack[i] == needle[i]; --i) {
            if (i == 0) { // first char matches so it's a match!
                return (haystack - reinterpret_cast<const uint8_t *>(hay_v));
            }
        }
	
        size_t skip = bad_char_shift[haystack[last]];
	//TODO-code-review: invariant was wrong.
        hay_len -= skip;
        haystack += skip;
    }
    return -1;
}
}
