#include <Lintel/AssertBoost.hpp>
#include <Lintel/BoyerMooreHorspool.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>

// For memmem
#ifndef _GNU_SOURCE
#error need gnu source for memmem
#endif
#include <string>
using namespace lintel;

size_t other_impl(const void * hay, size_t hay_len,
		  const void * needle, size_t needle_len) {
    void * res = memmem(hay, hay_len, needle, needle_len);
    char * res_c = reinterpret_cast<char *>(res);
    const char * hay_c = reinterpret_cast<const char *>(hay);
    if (res_c == 0) {
	return BoyerMooreHorspool::npos;
    } else {
	return res_c - hay_c; //Convert to offset
    }
}

void simple_tests(){
    const char hay1[] = "Does this algorithm really algorithm?";
    const char needle1[] = "algorithm";

    BoyerMooreHorspool bmh1(needle1);
    SINVARIANT(other_impl(hay1, strlen(hay1), needle1, strlen(needle1)) ==
	       bmh1.find(hay1, strlen(hay1)));
    SINVARIANT(bmh1.matches(hay1, strlen(hay1)));

    const char hay2[] = "Does this algorithm really algorithm?";
    const char needle2[] = "Algorithm";

    BoyerMooreHorspool bmh2(needle2);
    SINVARIANT(other_impl(hay2, strlen(hay2), needle2, strlen(needle2)) ==
	       bmh2.find(hay2, strlen(hay2)));
    SINVARIANT(!bmh2.matches(hay2, strlen(hay2)));

    const char hay3[] = "xxxxxxxxxxihm?";
    const char needle3[] = "algorithm";

    BoyerMooreHorspool bmh3(needle3);
    SINVARIANT(other_impl(hay3, strlen(hay3), needle3, strlen(needle3)) ==
	       bmh3.find(hay3, strlen(hay3)));
    SINVARIANT(!bmh3.matches(hay3, strlen(hay3)));
}

void simple_random(MersenneTwisterRandom & rgen) {
    size_t haylen = rgen.randInt(1000000);
    size_t nlen = rgen.randInt(1000000);

    char * hay = new char[haylen];
    char * needle = new char[nlen];
    memset(hay, '\0', haylen);
    memset(needle, '\0', nlen);
    
    for(uint32_t i = 0; i<haylen; ++i) {
	hay[i] = rgen.randInt('~' - ' ') + ' '; //Only printables.
    }

    for(uint32_t i = 0; i<nlen; ++i) {
	needle[i] = rgen.randInt('~' - ' ') + ' '; //Only printables.
    }
    
    BoyerMooreHorspool bmh(needle, nlen);
    SINVARIANT(other_impl(hay, haylen, needle, nlen) ==
	       bmh.find(hay, haylen));

    size_t offset = rgen.randInt(haylen-1);
    char * other_n = hay + offset;
    size_t other_len = rgen.randInt(haylen-offset-1)+1;
    
    BoyerMooreHorspool bmh2(other_n, other_len);
    SINVARIANT(other_impl(hay, haylen, other_n, other_len) ==
	       bmh2.find(hay, haylen));

    delete [] hay;
    delete [] needle;
}

// Verify we can work on binary
void hard_random(MersenneTwisterRandom & rgen) {
    size_t haylen = rgen.randInt(1000000);
    size_t nlen = rgen.randInt(1000000);

    char * hay = new char[haylen];
    char * needle = new char[nlen];

    for(uint32_t i = 0; i<haylen; ++i) {
	hay[i] = rgen.randInt(256);
    }

    for(uint32_t i = 0; i<nlen; ++i) {
	needle[i] = rgen.randInt(256);
    }

    BoyerMooreHorspool bmh(needle, nlen);
    SINVARIANT(other_impl(hay, haylen, needle, nlen) ==
	       bmh.find(hay, haylen));

    size_t offset = rgen.randInt(haylen-1);
    char * other_n = hay + offset;
    size_t other_len = rgen.randInt(haylen-offset-1)+1;
    
    BoyerMooreHorspool bmh2(other_n, other_len);
    SINVARIANT(other_impl(hay, haylen, other_n, other_len) ==
	       bmh2.find(hay, haylen));

    delete [] hay;
    delete [] needle;
}

int main() {
    uint32_t nreps = 100;
    simple_tests();

    MersenneTwisterRandom rgen;
    for(uint32_t i = 0; i<nreps; ++i) {
	simple_random(rgen);
	hard_random(rgen);
    }
}
