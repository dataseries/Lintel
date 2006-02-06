/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Constant Strings are encoded much more efficiently than string,
    but are not freeable after being allocated
*/

#ifndef __CONSTANT_STRING
#define __CONSTANT_STRING

#include <string>
#include <vector>
#include <GC_Alloc.H>

#include <HashTable.H>

// These are stored out here rather than as static inline methods in the 
// class because g++ doesn't seem to succeed in inlining them otherwise

inline const unsigned int ConstantString_length(const unsigned int *ptr) { return *(ptr - 1); }
inline const char *ConstantString_c_str(const unsigned int *ptr) { return (const char *)ptr; }

class ConstantString {
public:
    typedef unsigned int uint32;
    ConstantString(const std::string &str);
    ConstantString(const gcstring &str);
    ConstantString(const char *s);

    const char *c_str() const { return ConstantString_c_str(myptr); }
    inline const uint32 size() const { return ConstantString_length(myptr); }
    inline const uint32 length() const { return ConstantString_length(myptr); }
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

    int compare(const gcstring &rhs) const {
	int cmplen = size() < rhs.size() ? size() : rhs.size();
	cmplen += 1; // include null byte of shorter string
	return memcmp(c_str(),rhs.c_str(),cmplen);
    }

    struct buffer {
	char *data;
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
	unsigned int operator()(const char *k) {
	    return HashTable_hashbytes(k,strlen(k));
	}
    };
    class hteEqual {
    public:
	bool operator()(const char *a, const char *b) {
	    return strcmp(a,b) == 0;
	}
    };
private:
    void init(const char *s,uint32 slen);
    uint32 *myptr;

    static int nstrings;
    static int string_bytes;
    static const int buffer_size = 512*1024;
    static bufvect *buffers;
    // safe to use the ignore off page allocator here because the
    // buffer pool pointer to the strings will always be the
    // authoritative pointer, so the char * pointer here is a
    // duplicate.
    typedef HashTable<const char *, hteHash, hteEqual,
	GCAtomicIgnoreOffPageAllocator<HashTable_hte<const char *> >,
	GCAtomicIgnoreOffPageAllocator<int> > CS_hashtable; 
    static CS_hashtable hashtable;
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
    return lhs.compare(rhs) == 0;
}

inline bool
operator!=(const ConstantString &lhs, const ConstantString &rhs) {
    return lhs.compare(rhs) != 0;
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

#endif
