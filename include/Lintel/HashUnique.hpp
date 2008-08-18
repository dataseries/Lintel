/* -*-C++-*- */
/*
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    hash implementation of uniqueifying a set of values
*/

#ifndef LINTEL_HASH_UNIQUE_HPP
#define LINTEL_HASH_UNIQUE_HPP

#include <Lintel/HashMap.hpp>

template <class K,
          class KHash = HashMap_hash<const K>,
          class KEqualFn = std::equal_to<const K> >
class HashUnique {
public:
    struct KEqual {
	KEqualFn fn;
	bool operator()(const K &a, const K &b) {
	    return fn(a,b);
	}
    };
    
    bool exists(const K &k) {
	K *v = hashtable.lookup(k);
	return v != NULL;
    }
    
    bool add(const K &k) {
	K *v = hashtable.lookup(k);
	if (v == NULL) {
	    hashtable.add(k);
	    return true;
	} else {
	    return false;
	}
    }
    
    void remove(const K &k) {
	hashtable.remove(k);
    }

    typedef HashTable<K, KHash, KEqual> HashTableT;

    typedef typename HashTableT::iterator iterator;

    iterator begin() {
	return hashtable.begin();
    }
    
    iterator end() {
	return hashtable.end();
    }

    HashUnique() {
    }

    HashUnique(const HashUnique &__in) {
	hashtable = __in.hashtable;
    }

    HashUnique &
    operator=(const HashUnique &__in) {
	hashtable = __in.hashtable;
	return *this;
    }

    void clear() {
	hashtable.clear();
    }
    
    uint32_t size() {
	return hashtable.size();
    }

    void reserve(unsigned expected_entries) {
	hashtable.reserve(expected_entries);
    }

    /// Get statistics for the chain lengths of all the chains in the
    /// underlying hash table.  Useful for detecting a bad hash
    /// function.
    void chainLengthStats(Stats &stats) {
	return hashtable.chainLengthStats(stats);
    }
private:
    HashTableT hashtable;
};

#endif
