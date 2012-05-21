#ifndef LINTEL_MARSAGLIA_RANDOM_HPP
#define LINTEL_MARSAGLIA_RANDOM_HPP

#include <stdint.h>
#include <Lintel/HashFns.hpp>
#include <Lintel/RandomBase.hpp>

namespace lintel {

    class MicroRandInner {
        // Another from George Marsaglia
    public:
        
        uint32_t x, y, z, w;  // fits in a cache line likely.
        
        MicroRandInner(uint32_t seed=0) {
            // Need to handle seed==0 as choose own entropy.

            // For a nothing-up-my-sleeve number, we'll use the hexadecimal digits of pi.
            static uint32_t pi[] = 
                { 0x243F6A88, 0x85A308D3,
                  0x13198A2E, 0x03707344,
                  0xA4093822, 0x299F31D0,
                  0x082EFA98, 0xEC4E6C89 };

            x = lintel::BobJenkinsHashMix3(pi[0], pi[1], seed);
            y = lintel::BobJenkinsHashMix3(pi[2], pi[3], seed);
            z = lintel::BobJenkinsHashMix3(pi[4], pi[5], seed);
            w = lintel::BobJenkinsHashMix3(pi[6], pi[7], seed);
        }
        
        uint32_t randInt() {
            uint32_t t;
            // Choices of A,B,C given in http://www.jstatsoft.org/v08/i14/paper
            // Good ones being (5, 14, 1), (15, 4, 21), (23, 24, 3), and (5, 12, 29)
            static const uint32_t a = 15;
            static const uint32_t b = 4;
            static const uint32_t c = 21;
            t=(x^(x<<a));
            x=y; y=z; x=w;  // Shift 'em down.
            return w=(t^(t>>b))^(w^(w>>c));
            //return w=(w^(w>>c))^(t^(t>>b));
        }
    };

    typedef RandomTempl<MicroRandInner> MarsagliaRNGMicro;

    template <int32_t SIZE, uint64_t A>
    class MarsagliaInner {
        //From Marsaglia, in May 2003 CACM.
        //  The recurrence is
        //  X_n = (B-1) - (A * X_n-1024 + C_n-1 % B)
        //  C_n = floor(A * X_n-1024 + C_n-1 / B)
        //
        //  Where B is 2^32 - 1, C and X are internal state, and A is by recommended value
        //
        //  Any bits are fine for X, initial C must be 0 <= C < A.
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
        MarsagliaInner(uint32_t seed=0) {        
            MarsagliaRNGMicro r(seed);
            for(int32_t i=0; i<SIZE; ++i) {
                x[i] = r.randInt();
            }
            c = r.randIntUnbiased(A);
        }
        
        inline uint32_t randInt() {
            uint64_t temp;
            uint32_t ax_plus_c;
            ++i;
            i &= (SIZE-1);
            
            temp = A * x[i] + c;
            c = temp >> 32;
            ax_plus_c = temp + c;
            
            if (ax_plus_c<c) {
                ++ax_plus_c;
                ++c;
            }
            x[i] = (B-1)-ax_plus_c;
            return x[i];
        }
    };
    
    template <int32_t SIZE>
    class Marsaglia {
    public:
        Marsaglia(uint32_t seed=0) { }
        uint32_t randInt();
    };
    
    template <>
    class Marsaglia<4096> : public MarsagliaInner<4096, 18782LL> {
    public:
        Marsaglia(uint32_t seed=0) : MarsagliaInner<4096, 18782LL>(seed) { }

    };
    
    template <>
    class Marsaglia<2048> : public MarsagliaInner<2048, 1047570LL> {
    };
    
    
    template <>
    class Marsaglia<1024> : public MarsagliaInner<1024, 123471786LL> {
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
    public:
        Marsaglia(uint32_t seed=0) : MarsagliaInner<8,716514398LL>(seed) { }
    };

    typedef RandomTempl<Marsaglia<4096> > MarsagliaRNGHuge;
    typedef RandomTempl<Marsaglia<1024> > MarsagliaRNGBig;
    typedef RandomTempl<Marsaglia<256> > MarsagliaRNGMedium;
    typedef RandomTempl<Marsaglia<64> > MarsagliaRNGSmall;
    typedef RandomTempl<Marsaglia<8> > MarsagliaRNGTiny;
};

#endif
