/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    A class for substring matching based on the Boyer-Moore-Horspool algorithm.
*/

#ifndef LINTEL_BOYERMOOREHORSPOOL_HPP
#define LINTEL_BOYERMOOREHORSPOOL_HPP

#include <inttypes.h>
#include <limits.h>

class BoyerMooreHorspool {
public:
    /** Constructs a new @c BoyerMooreHorspool that is capable of searching for the specified
        @param needle within a given string. The needle is fixed for the object, whereas the
        haystack is provided to the matches method. @param needleLength is the length of the
        needle. */
    BoyerMooreHorspool(const char *needle, int32_t needleLength);
    ~BoyerMooreHorspool();

    /** Returns true if and only if needle is a substring of @param haystack\. */
    bool matches(const char *haystack, int32_t haystackLength);

private:
    int32_t badCharShift[CHAR_MAX + 1];
    char *needle;
    int32_t needleLength;
    int32_t last;
};

#endif
