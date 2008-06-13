/* -*-C++-*- */
/*
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    A "map" using the HashTable class.  You may want to read the warning in
    HashTable.H about the types of values that you can safely use.
*/

#ifndef LINTEL_HASH_MAP_HPP
#define LINTEL_HASH_MAP_HPP

#if __GNUG__ == 3 && __GNUC_MINOR == 3
#include <ext/stl_hash_fun.h>
#endif
#if __GNUG__ == 3 && __GNUC_MINOR == 4
#include <ext/hash_fun.h>
#endif
#include <functional>

#include <Lintel/HashTable.hpp>

template <class K, class V>
struct HashMap_val {
    K first;
    V second;
    HashMap_val(K &k, V &v) : first(k), second(v) { }
    HashMap_val(const K &k) : first(k), second() { }
};

// TODO: make the return value uint32_t
template <class K>
struct HashMap_hash {
    //    unsigned operator()(const K &a) const;
};

template <>
struct HashMap_hash<const std::string> {
    uint32_t operator()(const std::string &a) const {
	return HashTable_hashbytes(a.data(),a.size());
    }
};

template <>
struct HashMap_hash<const char * const> {
    uint32_t operator()(const char * const a) const {
	return HashTable_hashbytes(a,strlen(a));
    }
};

template <>
struct HashMap_hash<const int> {
    uint32_t operator()(const int _a) const {
	// This turns out to be slow, so turn it back into just using
	// the underlying integer; if someone does put "bad" integers
	// into the system then they can make their own hash function

	// doesn't increase the entropy, but shuffles the entropy across
	// the entire integer, meaning the low bits get shuffled even if they
	// are constant for some reason
//	int a = _a;
//	int b = 0xBEEFCAFE;
//	int ret = 1972;
//	BobJenkinsHashMix(a,b,ret);
//	return ret;
	return static_cast<unsigned>(_a);
    }
};

// cygwin didn't use the above for a const int32_t, so...
#ifdef __CYGWIN__
template <>
struct HashMap_hash<const int32_t> {
    uint32_t operator()(const int32_t _a) const {
	return static_cast<unsigned>(_a);
    }
};

template <>
struct HashMap_hash<const uint32_t> {
    uint32_t operator()(const uint32_t _a) const {
	return static_cast<unsigned>(_a);
    }
};
#endif

template <>
struct HashMap_hash<const unsigned> {
    uint32_t operator()(const unsigned _a) const {
	return static_cast<uint32_t>(_a);
    }
};

// Can't have both long long and int64_t instantiations of this
// template because 32bit x86 Debian etch thinks those types are the
// same, so it complains about duplicate templates, but 64bit x86_64
// RHEL4 thinks the types are different so requires both of them.  We
// choose to go with just the standard, size-specific types.

template <>
struct HashMap_hash<const int64_t> {
    uint32_t operator()(const int64_t _a) const {
	return BobJenkinsHashMixULL(_a);
    }
};

template <>
struct HashMap_hash<const uint64_t> {
    uint32_t operator()(const uint64_t _a) const {
	return _a;
    }
};

template <class K, class V, 
          class KHash = HashMap_hash<const K>, 
          class KEqual = std::equal_to<const K> >
class HashMap {
public:
    typedef HashMap_val<K,V> hmval;
    struct hmvalHash {
	KHash khash;
	uint32_t operator()(const hmval &hmv) {
	    return khash(hmv.first);
	}
    };
    struct hmvalEqual {
	KEqual kequal;
	bool operator()(const hmval &a, const hmval &b) {
	    return kequal(a.first,b.first);
	}
    };

    V *lookup(const K &k) {
	hmval fullval(k);
	hmval *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return NULL;
	} else {
	    return &v->second;
	}
    }

    V &operator[] (const K &k) {
	hmval fullval(k);
	hmval *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return hashtable.add(fullval)->second;
	} else {
	    return v->second;
	}
    }

    V &operator[] (K &k) {
	hmval fullval(k);
	hmval *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return hashtable.add(fullval)->second;
	} else {
	    return v->second;
	}
    }

    bool exists(const K &k) {
	return lookup(k) != NULL;
    }

    /** returns true if something was removed */
    bool remove(const K &k, bool must_exist = true) {
	hmval fullval(k);
	return hashtable.remove(fullval, must_exist);
    }

    void clear() {
	hashtable.clear();
    }

    uint32_t size() const {
	return hashtable.size();
    }

    bool empty() const {
	return hashtable.empty();
    }

    typedef HashTable<hmval,hmvalHash,hmvalEqual> HashTableT;
    typedef typename HashTableT::iterator iterator;

    iterator begin() {
	return hashtable.begin();
    }
    iterator end() {
	return hashtable.end();
    }

    explicit HashMap(double target_chain_length) 
	: hashtable(target_chain_length)
    { }

    HashMap() {
    }

    HashMap(const HashMap &__in) {
	hashtable = __in.hashtable;
    }

    HashMap &
    operator=(const HashMap &__in) {
	hashtable = __in.hashtable;
	return *this;
    }

    void reserve(uint32_t nentries) {
	hashtable.reserve(nentries);
    }
    
    uint32_t available() {
	return hashtable.available();
    }

    size_t memoryUsage() {
	return hashtable.memoryUsage();
    }

    // primiarily here so that you can get at unsafeGetRawDataVector, with
    // all the caveats that go with that function.
    HashTableT &getHashTable() {
	return hashtable;
    }
private:
    HashTableT hashtable;
};

#endif
