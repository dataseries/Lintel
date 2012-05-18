#ifndef LINTEL_MARSAGLIA_RANDOM_HPP
#define LINTEL_MARSAGLIA_RANDOM_HPP

#include <stdint.h>
#include <Lintel/RandomBase.hpp>
namespace Lintel {

    template <int32_t SIZE, uint64_t A>
    class MarsagliaInner : public RandomBase{
        //From Marsaglia, in May 2003 CACM.
        //  The recurrence is
        //  X_n = (B-1) - (A * X_n-1024 + C_n-1 % B)
        //  C_n = floor(A * X_n-1024 + C_n-1 / B)
        //
        //  Where B is 2^32 - 1, C and X are internal state, and A is by recommended value
        //
        //  Any bits are fine for X, inital C must be 0 <= C < A.
        //
        //  Also, there are variants where the X state is less than 1024 words.
        
        // From Marsaglia in jmasm 2003 (http://www.jmasm.com/journal/2003_vol2_no1.pdf)
        // we get recommended values for A based on the size of the state:
        // 2048,   1030770
        // 2048,   1047570
        // 1024,   5555698
        // 1024, 987769338
        // 512,  123462658
        // 512,  123484214
        // 256,  987662290
        // 256,  987665442
        // 128,  987688302
        // 128,  987689614
        // 64,   987651206
        // 64,   987657110
        // 32,   987655670
        // 32,   987655878
        // 16,   987651178
        // 16,   987651386
        // 8,    987651386
        // 8,    987651670
        // 4,    987654366
        // 4,    987654978
        // We also have a note "Those wanting even more pairs r,a will need to find primes of the
        // form p = (ab^r)+1 for which b=(2^32)-1 is a primitive root."
        // Further, this family of generator "have periods of ab^r", or "roughtly 2^(32r+30)"
        //
        // In Marsaglia's Jul 13 2005 posting to sci.crypt, he also gives
        // 4096,     18782
    private:
        uint32_t x[SIZE];
        uint32_t c; //Must be 0<=c<A
        uint32_t i; //Must be 0<=i<SIZE
        static const uint32_t B = 0xffffffff; 
    public:
        MarsagliaInner() {        
            MersenneTwisterRandom r;
            for(int32_t i=0; i<SIZE; ++i) {
                x[i] = r.randInt();
            }
            c = r.randInt() % A;
        }
        
        virtual inline uint32_t randInt() {
            uint64_t temp;
            uint32_t ax_plus_c;
            ++i;
            i &= (SIZE-1);
            
            temp = A * x[i] + c;
            c = temp >> 32;
            ax_plus_c = temp +c;
            
            if (ax_plus_c<c) {
                ++ax_plus_c;
                ++c;
            }
            x[i] = (B-1)-ax_plus_c;
            return x[i];
        }

        using RandomBase::randInt;
    };
    
    template <int32_t SIZE>
    class Marsaglia {
    public:
        Marsaglia();
        uint32_t randInt();
    };
    
    template <>
    class Marsaglia<4096> : 
        public MarsagliaInner<4096, 18782LL> {
    };
    
    template <>
    class Marsaglia<2048> :
        public MarsagliaInner<2048, 1047570LL> {
    };
    
    
    template <>
    class Marsaglia<1024> : 
        public MarsagliaInner<1024, 123471786LL> {
    };
    
    template <>
    class Marsaglia<512> : public MarsagliaInner<512,123554632LL> {
    };
    
    template <>
    class Marsaglia<256> : public MarsagliaInner<256,8001634LL> {
    };
    
    template <>
    class Marsaglia<128> : public MarsagliaInner<128,8007626LL> {
    };
    
    template <>
    class Marsaglia<64> : public MarsagliaInner<64,647535442LL> {
    };
    
    template <>
    class Marsaglia<32> : public MarsagliaInner<32,547416522LL> {
    };
    
    template <>
    class Marsaglia<16> : public MarsagliaInner<16,487198574LL> {
    };
    
    template <>
    class Marsaglia<8> : public MarsagliaInner<8,716514398LL> {
    };
};

#endif
