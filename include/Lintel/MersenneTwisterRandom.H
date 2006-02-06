/*
 * derived from code at http://www.math.keio.ac.jp/matumoto/emt.html,
 * see copyright/license in MersenneTwisterRandom.C
 */

/** @file
    Mersenne Twister Randum Number Generator
*/

#ifndef __MERSENNE_TWISTER_RANDOM_H
#define __MERSENNE_TWISTER_RANDOM_H

#include <vector>

// A possibly faster version may be adaptable from:
// http://www.math.keio.ac.jp/matumoto/MT2002/emt19937ar.html

class MersenneTwisterRandom {
public:
    typedef unsigned int uint32;
    
    // 0=Defaults to seeding with various system parameters
    MersenneTwisterRandom(uint32 seed = 0);
    MersenneTwisterRandom(std::vector<uint32> seed_array);

    void init(uint32 seed); 
    // seed_array should be N bytes long for full randomness; if not, we will
    // seed with seed_array repeated enough times.
    void initArray(std::vector<uint32> seed_array);

    inline uint32 randInt() {
	uint32 y;

	if (mti >= N) {
	    reloadArray();
	}
	y = mt[mti++];
	y ^= (y >> TEMPERING_SHIFT_U);
	y ^= (y << TEMPERING_SHIFT_S) & TEMPERING_MASK_B;
	y ^= (y << TEMPERING_SHIFT_T) & TEMPERING_MASK_C;
	y ^= (y >> TEMPERING_SHIFT_L);
	return y;
    }

    inline unsigned long long randLongLong() {
	unsigned long long ret;
	ret = randInt();
	ret = ret << 32;
	ret = ret | randInt();
	return ret;
    }
    inline uint32 randInt(uint32 max) {
	return randInt() % max;
    }
    // randDouble() gives you doubles with 32 bits of randomness.
    // randLongDouble() gives you doubles with ~53 bits of randomness
    // *Open() gives values in [0,1)
    // *Closed() gives values in [0,1]
    
    // HP-UX aCC won't let me define these as const double;
    // const double foo = 5.0 reports error 481.
#define MTR_int_to_open (1.0/4294967296.0) 
#define MTR_int_to_closed (1.0/4294967295.0) 
#define MTR_AMult (67108864.0) // 2^(32-6)
// 9007199254740992 = 2^53, 9007199254740991 = 2^53 -1
#define MTR_53bits_to_open(a,b) ((a * MTR_AMult + b)/9007199254740992.0)
#define MTR_53bits_to_closed(a,b) ((a * MTR_AMult + b)/9007199254740991.0)
    inline double randDoubleOpen() { // in [0,1), 32 bits of randomness
	return (double)randInt() * MTR_int_to_open;
    }
    inline double randDoubleClosed() {  // in [0,1], 32 bits of randomness
	return (double)randInt() * MTR_int_to_closed;
    }
    inline double randDouble() { // in [0,1), 32 bits of randomness
	return randDoubleOpen();
    }

    // casting a long long to double is unbelievably slow on pa2.0 aCC C.03.30
    inline double randDoubleOpen53() { // in [0,1), 53 bits of randomness
	uint32 a=randInt()>>5, b=randInt()>>6;
	return MTR_53bits_to_open(a,b);
    }
    inline double randDoubleClosed53() { // in [0,1]
	uint32 a=randInt()>>5, b=randInt()>>6;
	return MTR_53bits_to_closed(a,b);
    }

    uint32 seed_used; 

    static void selfTest();
private:
    static const int N = 624; // state vector size
    static const int M = 397; 
    static const uint32 MATRIX_A = 0x9908b0dfUL;   /* constant vector a */
    static const uint32 UPPER_MASK = 0x80000000UL; /* most significant w-r bits */
    static const uint32 LOWER_MASK = 0x7fffffffUL; /* least significant r bits */
    static const uint32 TEMPERING_MASK_B = 0x9d2c5680UL;
    static const uint32 TEMPERING_MASK_C = 0xefc60000UL;
    static const int TEMPERING_SHIFT_U = 11;
    static const int TEMPERING_SHIFT_S = 7;
    static const int TEMPERING_SHIFT_T = 15;
    static const int TEMPERING_SHIFT_L = 18;

    void reloadArray();
    uint32 mt[N]; 
    int mti;
};

extern MersenneTwisterRandom MTRandom; 

template <class RandomAccessIter>
inline void
MT_random_shuffle(RandomAccessIter first, RandomAccessIter last, MersenneTwisterRandom &rng = MTRandom)
{
  if (first == last)
    return;
  for(RandomAccessIter i = first + 1; i != last; ++i) {
    int rval = rng.randInt((i-first)+1);
    iter_swap(i,first + rval);
  }
}
#endif
