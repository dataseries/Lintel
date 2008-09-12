/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Constant Strings are encoded much more efficiently than string,
    but are not freeable after being allocated
*/

#ifndef LINTEL_CONSTANT_STRING_HPP
#define LINTEL_CONSTANT_STRING_HPP

#include <sys/types.h>

#include <string>
#include <vector>
#include <cstring>

#include <boost/static_assert.hpp>

#include <Lintel/HashMap.hpp>

// TODO: think about how to make this thread safe; potentially one
// choice is to make the constructor the constantstring take a
// stringpool argument with a choice on the pool as to whether it is
// locked or not.

// Class is here merely to force type incompatibility
struct ConstantStringValue;

// These functions are stored out here rather than as static inline
// methods in the class because an old version of g++ doesn't seem to
// succeed in inlining them otherwise; TODO: recheck this
inline const uint32_t ConstantString_length(const ConstantStringValue *ptr) { 
    return *(reinterpret_cast<const uint32_t *>(ptr) - 1); }
inline const char *ConstantString_c_str(const ConstantStringValue *ptr) { 
    return reinterpret_cast<const char *>(ptr); 
}

class ConstantString {
public:
    ConstantString(const ConstantString &from) {
	myptr = from.myptr;
    }
    ConstantString(const std::string &str) { 
	init(str.data(), str.size());
    }
    ConstantString(const char *s) {
	init(s, strlen(s));
    }
    ConstantString(const void *s, uint32_t slen) {
	init(static_cast<const char *>(s), slen);
    }
    ConstantString() { init("", 0); }
    void init(const void *s, uint32_t slen);

    const char *c_str() const { return ConstantString_c_str(myptr); }
    const char *data() const { return ConstantString_c_str(myptr); }
    const uint32_t size() const { return ConstantString_length(myptr); }
    const uint32_t length() const { return ConstantString_length(myptr); }
    std::string str() const { return std::string(c_str(), size()); }

    const uint32_t hash() const {
	BOOST_STATIC_ASSERT(sizeof(ConstantStringValue *) == 4 ||
			    sizeof(ConstantStringValue *) == 8);
	if (sizeof(ConstantStringValue *) == 4) {
	    // guaranteed to be unique!
	    // Cast through size_t because on 64bit gcc 3.4.6
	    // complains about the cast losing precision even though
	    // this branch of the if is dead.
	    size_t tmp = reinterpret_cast<size_t>(myptr);
	    return static_cast<uint32_t>(tmp);
	} else if (sizeof(ConstantStringValue *) == 8) {
	    // likely to be unique
	    uint64_t v = reinterpret_cast<uint64_t>(myptr);
	    return static_cast<uint32_t>((v & 0xFFFFFFFFULL) ^ (v >> 32));
	} 
    }
    int compare(const ConstantString &rhs) const {
	int cmplen = size() < rhs.size() ? size() : rhs.size();
	cmplen += 1; // include null byte of shorter string
	return memcmp(c_str(),rhs.c_str(),cmplen);
    }
    int compare(const std::string &rhs) const {
	int cmplen = size() < rhs.size() ? size() : rhs.size();
	cmplen += 1; // include null byte of shorter string
	return memcmp(c_str(),rhs.c_str(),cmplen);
    }

    struct buffer {
	ConstantStringValue *data;
	int size;
	int amt_used;
    };
    static void dumpInfo();
    typedef std::vector<buffer> bufvect;

    // Comparisons are done with char *'s here so that when doing a
    // hash check to see if we already have a string, we don't have to
    // create a constant-string formed string before it can do the
    // hash lookup; note that this substantially embeds the idea that
    // the c_str() pointer of a thing is the same as the pointer
    class hteHash {
    public:
	unsigned int operator()(const ConstantStringValue *k) const {
	    return HashTable_hashbytes(ConstantString_c_str(k),ConstantString_length(k));
	}
    };
    class hteEqual {
    public:
	bool operator()(const ConstantStringValue *a, const ConstantStringValue *b) const {
	    uint32_t len_a = ConstantString_length(a);
	    uint32_t len_b = ConstantString_length(b);
	    if (len_a != len_b) {
		return false;
	    } else {
		return memcmp(ConstantString_c_str(a),ConstantString_c_str(b), len_a) == 0;
	    }
	}
    };
    bool equal(const ConstantString &to) const {
	return myptr == to.myptr ? true : false;
    }
private:
    const ConstantStringValue *myptr;

    static int nstrings;
    static int string_bytes;
    static const int buffer_size = 512*1024;
    static bufvect *buffers;
    typedef HashTable<const ConstantStringValue *, hteHash, hteEqual> CS_hashtable;

    static CS_hashtable *hashtable;
};

inline bool
operator<(const ConstantString &lhs, const ConstantString &rhs) {
    return lhs.compare(rhs) < 0;
}

inline bool
operator<=(const ConstantString &lhs, const ConstantString &rhs) {
    return lhs.compare(rhs) <= 0;
}

inline bool
operator==(const ConstantString &lhs, const ConstantString &rhs) {
    return lhs.equal(rhs);
}

inline bool
operator!=(const ConstantString &lhs, const ConstantString &rhs) {
    return !lhs.equal(rhs);
}

inline bool
operator>(const ConstantString &lhs, const ConstantString &rhs) {
    return lhs.compare(rhs) > 0;
}

inline bool
operator>=(const ConstantString &lhs, const ConstantString &rhs) {
    return lhs.compare(rhs) >= 0;
}

inline bool
operator!=(const ConstantString &lhs, const std::string &rhs) {
    return lhs.compare(rhs) != 0;
}

inline bool
operator!=(const ConstantString &lhs, const char *rhs) {
    return strcmp(lhs.c_str(),rhs) != 0;
}

inline std::ostream &
operator<< (std::ostream&o, const ConstantString&s) {
    o << s.c_str();
    return o;
}

template <>
struct HashMap_hash<const ConstantString> {
    unsigned operator()(const ConstantString &a) const {
	return a.hash();
    }
};

#endif
