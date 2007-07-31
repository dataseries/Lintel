/* -*-C++-*- */
/*
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    A "map" using the HashTable class
*/

#ifndef __HASH_MAP
#define __HASH_MAP

#if __GNUG__ == 3 && __GNUC_MINOR == 3
#include <ext/stl_hash_fun.h>
#endif
#if __GNUG__ == 3 && __GNUC_MINOR == 4
#include <ext/hash_fun.h>
#endif
#include <functional>

#include <Lintel/HashTable.H>

template <class K, class V>
struct HashMap_val {
    K first;
    V second;
    HashMap_val(K &k, V &v) : first(k), second(v) { }
    HashMap_val(const K &k) : first(k), second() { }
};

template <class K>
struct HashMap_hash {
    //    unsigned operator()(const K &a) const;
};

template <>
struct HashMap_hash<const std::string> {
    unsigned operator()(const std::string &a) const {
	return HashTable_hashbytes(a.data(),a.size());
    }
};

template <>
struct HashMap_hash<const int> {
    unsigned operator()(const int _a) const {
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

template <>
struct HashMap_hash<const unsigned> {
    unsigned operator()(const unsigned _a) const {
	return _a;
    }
};

template <>
struct HashMap_hash<const long long> {
    unsigned operator()(const long long _a) const {
	return BobJenkinsHashMixULL(_a);
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

    void remove(const K &k, bool must_exist = true) {
	hmval fullval(k);
	hashtable.remove(fullval, must_exist);
    }

    void clear() {
	hashtable.clear();
    }

    uint32_t size() {
	return hashtable.size();
    }

    typedef typename HashTable<hmval,hmvalHash,hmvalEqual>::iterator iterator;

    iterator begin() {
	return hashtable.begin();
    }
    iterator end() {
	return hashtable.end();
    }

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
private:
    HashTable<hmval,hmvalHash,hmvalEqual> hashtable;
};

#endif
