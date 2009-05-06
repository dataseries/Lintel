#include <Lintel/BoyerMooreHorspool.hpp>

#include <string.h>

BoyerMooreHorspool::BoyerMooreHorspool(const char *needle, int32_t needleLength) :
        needleLength(needleLength), last(needleLength - 1) {
    // initialize the bad character shift array
    for (int32_t i = 0; i <= CHAR_MAX; ++i) {
        badCharShift[i] = needleLength;
    }

    for (int32_t i = 0; i < last; i++) {
        badCharShift[(int8_t)needle[i]] = last - i;
    }

    this->needle = new char[needleLength];
    memcpy(this->needle, needle, needleLength);
}

BoyerMooreHorspool::~BoyerMooreHorspool() {
    delete [] needle;
}

bool BoyerMooreHorspool::matches(const char *haystack, int32_t haystackLength) {
    while (haystackLength >= needleLength) {
        int32_t i;
        for (i = last; haystack[i] == needle[i]; --i) {
            if (i == 0) { // first char matches so it's a match!
                return true;
            }
        }

        int32_t skip = badCharShift[(int8_t)haystack[last]];
        haystackLength -= skip;
        haystack += skip;
    }
    return false;
}
