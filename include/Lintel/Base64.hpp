#ifndef LINTEL_BASE_64_HPP
#define LINTEL_BASE_64_HPP

#include <sys/types.h>
#include <stdint.h>

#include <string>

namespace lintel {
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
	
	// TODO - decode... don't need it yet.  But I did make the
	// inverse translation table for you.
	
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
