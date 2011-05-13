#ifndef LINTEL_BASE_64_HPP
#define LINTEL_BASE_64_HPP

/** @file
    \brief Header for the ASCIIbeticalB64 class
*/

#include <sys/types.h>
#include <stdint.h>

#include <string>

// Consider replacing with: http://code.google.com/p/stringencoders/
// TODO: add test cases, probably make a lot of this not inline.
namespace lintel {
    /** \brief A class for converting strings to an alphabetical base64 variant
        
        The default Base64 encoding does not preserve ordering, i.e. if I have a binary string x
        and y such that x < y; then it is possible that Base64(x) > Base64(y).  The ASCIIBeticalB64
        class preserves the ordering for all string x and y. */

    class ASCIIbeticalB64 {
    public:
	static const char * translate() {
	    static char table[] = 
	"+,0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	    return table;
	}

	static const char * detranslate() {
	    static char * inverse = 0;
	    if (inverse) {
		return inverse;
	    }
	    inverse = new char[256];
	    for (int8_t i=0; i<64; ++i) {
		inverse[static_cast<int32_t>(translate()[i])] = 
		    static_cast<char>(i);
	    }
	    return inverse;
	}
	
	static std::string decode(std::string &data) {
	    return decode(data.data(), data.size());
	}

	static std::string decode(const char *data, size_t size) {
	    const char *table = detranslate();
	    int32_t ret_size = ((size*6)/8);
	    std::string ret(ret_size, translate()[0]);
	    int32_t out_off = 0;
	    uint32_t trans_buf = 0;
	    uint32_t bits = 0;
	    for(size_t in_off = 0;
		in_off < size && out_off < ret_size; ++in_off) {
		trans_buf += ((uint32_t)table[(uint32_t)data[in_off]]) << (32-6-bits);
		bits += 6;
		if (bits >= 8) {
		    ret[out_off] = trans_buf >> (32-8);
		    trans_buf <<= 8;
		    ++out_off;
		    bits -= 8;
		}
	    }
	    return ret;
	}
	
	static std::string encode(uint32_t number, int32_t digits) {
	    if (digits==0) {
		digits = 6;
	    }
	    std::string ret(digits, translate()[0]);
	    encode(number, digits, &ret[0]);
	    return ret;
	}

	static void encode(uint32_t number, int32_t digits, 
			   char * ret) {
	    for(int32_t i=digits-1; i>=0; --i) {
		ret[i] = translate()[number & 0x3F];
		number >>= 6;
	    }
	}

	static std::string encode(int32_t number, int32_t digits) {
	    return encode(static_cast<uint32_t>(number), digits);
	}

	static void encode(int32_t number, int32_t digits,
			   char * ret) {
	    encode(static_cast<uint32_t>(number), digits, ret);
	}

	static std::string encode(uint64_t number, int32_t digits) {
	    if (digits==0) {
		digits = 11;
	    }
	    std::string ret(digits, translate()[0]);
	    encode(number, digits, &ret[0]);
	    return ret;
	}

	static void encode(uint64_t number, int32_t digits,
			   char * ret) {
	    for(int32_t i=digits-1; i>=0; --i) {
		ret[i] = translate()[number & 0x3F];
		number >>= 6;
	    }
	}

	static std::string encode(int64_t number, int32_t digits) {
	    return encode(static_cast<uint64_t>(number), digits);
	}

	static void encode(int64_t number, int32_t digits,
			   char * ret) {
	    encode(static_cast<uint64_t>(number), digits, ret);
	}

	static std::string encode(const void * data, size_t size, 
				  int32_t digits) {
	    if (digits==0) {
		digits = size * 8;
		if (digits%6==0) {
		    digits/=6;
		} else {
		    digits = digits / 6 + 1;
		}
	    }
	    std::string ret(digits, translate()[0]);
	    encode(data, size, digits, &ret[0]);
	    return ret;
	}

	static void encode(const void * data, size_t size, int32_t digits,
			   char * ret) {
	    const unsigned char * cdata = 
		reinterpret_cast<const unsigned char *>(data);
	    size_t in_off;
	    int32_t out_off;
	    uint32_t trans_buf = 0;
	    uint32_t bits = 0;
	    for(out_off=0, in_off=0;
		in_off < size && out_off < digits; ++out_off) {
		if (bits < 6) {
		    trans_buf+= ((uint32_t)cdata[in_off]) << (32-8-bits);
		    ++in_off;
		    bits+=8;
		}
		ret[out_off] = translate()[trans_buf >> (32-6)];
		trans_buf <<= 6;
		bits-=6;
	    }
	    // For the spare bits..
	    for(; bits>0 && out_off < digits; ++out_off) {
		ret[out_off] = translate()[trans_buf >> (32-6)];
		trans_buf <<=6;
		bits-=6;
	    }
	}
	
	// This is the same as encode, but instead of adding zero bits at the end,
	// it adds zero bits at the beginning; that is, it treats the input
	// string as a big-endian binary integer.
	static void encodeBackward(const void * data, size_t size, int32_t digits,
				  char * ret) {
	    const unsigned char * cdata = 
		reinterpret_cast<const unsigned char *>(data);
	    size_t in_off;
	    int32_t out_off;
	    uint32_t trans_buf = 0;
	    uint32_t bits = 0;
	    for(out_off=digits-1, in_off=size-1; 
		in_off >= 0 && out_off >= 0; --out_off) {
		if (bits < 6) {
		    trans_buf += ((uint32_t)cdata[in_off]) << bits;
		    --in_off;
		    bits += 8;
		}
		ret[out_off] = translate()[trans_buf & 0x3F];
		trans_buf >>= 6;
		bits -= 6;
	    }
	    // There may be a few spare bits...
	    for(; bits > 0 && out_off >= 0; --out_off) {
		ret[out_off] = translate()[trans_buf & 0x3F];
		trans_buf >>= 6;
		bits -= 6;
	    }	
	}	
	
	static std::string encode(std::string data, int32_t digits) {
	    return encode(data.data(), data.size(), digits);
	}

	static void encode(std::string data, int32_t digits,
			   char * ret) {
	    encode(data.data(), data.size(), digits, ret);
	}
    };
}

#endif
