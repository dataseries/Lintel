/* -*-C++-*- */
/*
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief header file for HashUnique class
*/

#ifndef LINTEL_HASH_UNIQUE_HPP
#define LINTEL_HASH_UNIQUE_HPP

#include <Lintel/HashMap.hpp>

/// \brief hash implementation of uniqueifying a set of values; no excess space used.
template <class K,
          class KHash = HashMap_hash<const K>,
          class KEqual = std::equal_to<const K> >
class HashUnique {
public:
    HashUnique() { }

    HashUnique(const HashUnique &__in) {
	hashtable = __in.hashtable;
    }

    HashUnique &
    operator=(const HashUnique &__in) {
	hashtable = __in.hashtable;
	return *this;
    }

    bool exists(const K &k) const {
	const K *v = hashtable.lookup(k);
	return v != NULL;
    }
    
    // add the key to the table if it does not exist.  if
    // it does exist, nothing happens.  returns true iff
    // an add occurred.
    bool add(const K &k) {
	K *v = hashtable.lookup(k);
	if (v == NULL) {
	    hashtable.add(k);
	    return true;
	} else {
	    return false;
	}
    }

    // add the key or replace it if it exists.  returns true
    // iff a replacement occurred.
    bool addOrReplace(const K &k) {
	bool replaced;
	hashtable.addOrReplace(k, replaced);
	return replaced;
    }

    void remove(const K &k, bool must_exist = true) {
	hashtable.remove(k, must_exist);
    }

    typedef HashTable<K, KHash, KEqual> HashTableT;

    typedef typename HashTableT::iterator iterator;
    
    iterator begin() {
	return hashtable.begin();
    }
    
    iterator end() {
	return hashtable.end();
    }

    typedef typename HashTableT::const_iterator const_iterator;

    const_iterator begin() const {
	return hashtable.begin();
    }

    const_iterator end() const {
	return hashtable.end();
    }

    void clear() {
	hashtable.clear();
    }
    
    uint32_t size() const {
	return hashtable.size();
    }

    void reserve(unsigned expected_entries) {
	hashtable.reserve(expected_entries);
    }

    /// Get statistics for the chain lengths of all the chains in the
    /// underlying hash table.  Useful for detecting a bad hash
    /// function.
    void chainLengthStats(Stats &stats) const {
	return hashtable.chainLengthStats(stats);
    }
private:
    HashTableT hashtable;
};

#endif
