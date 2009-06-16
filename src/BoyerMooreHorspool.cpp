/* -*-C++-*-
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <string.h>
#include <limits>

#include <boost/static_assert.hpp>

#include <Lintel/AssertBoost.hpp>

#include <Lintel/BoyerMooreHorspool.hpp>

// TODO-tomer: put in reference to textbook describing this algorithm.

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

bool BoyerMooreHorspool::matches(const void *haystack_v, size_t haystack_length) {
    const uint8_t *haystack = reinterpret_cast<const uint8_t *>(haystack_v);

    while (haystack_length >= needle_length) {
        size_t i;
        for (i = last; haystack[i] == needle[i]; --i) {
            if (i == 0) { // first char matches so it's a match!
                return true;
            }
        }

        ssize_t skip = bad_char_shift[haystack[last]];
	DEBUG_SINVARIANT(haystack_length >= static_cast<size_t>(skip));
        haystack_length -= skip;
        haystack += skip;
    }
    return false;
}
