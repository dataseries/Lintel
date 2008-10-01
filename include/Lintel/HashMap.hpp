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

#include <boost/static_assert.hpp>

#include <Lintel/HashFns.hpp>
#include <Lintel/HashTable.hpp>

// propagate things into the global namespace

template <class K> struct HashMap_hash : lintel::Hash<K> {
    //    uint32_t operator()(const K &a) const;
};

template<typename T> struct PointerHashMapHash : lintel::PointerHash<T> { };
template<typename T> struct PointerHashMapEqual : lintel::PointerEqual<T> { };

/// HashMap class; in our testing almost as fast as the google dense
/// map, but uses almost as little memory as the sparse map.  Note
/// that class K can't be const, see src/tests/hashmap.cpp for
/// details.  
/// 
/// To create a hashmap on a separate type, you need to define two
/// additional operations.  For example, with:
/// \code
/// struct example { 
///    int32_t x, y;
///    std::string str;
///    bool operator==(const example &rhs) const;
///    uint32_t hash() const;
/// }
/// \endcode
/// you would need to define a hash function and operator == for
/// struct example.  The operator definition can be seen above.  There are
/// two options for the hash function.  Either it is a hash method on the
/// object itself, with the signature shown above.  Or it is a 
/// hashType(const Example &v) function defined in the same namespace as
/// the underlying type or the lintel namespace.  So you could write either:
/// \code
/// uint32_t example::hash() const {
///     uint32_t partial_hash = BobJenkinsHashMix3(a.x, a.y, 2001);
///     return HasHTable_hashbytes(a.str().data, a.str.size(), partial_hash);
/// }
/// \endcode
/// or
/// \code
/// uint32_t hashType(const example &v) {
///     uint32_t partial_hash = BobJenkinsHashMix3(a.x, a.y, 2001);
///     return HasHTable_hashbytes(a.str().data, a.str.size(), partial_hash);
/// }
/// \endcode
/// If both are defined, the hash function on the object takes precedence.
///
/// To define operator ==, you can either define it as part of the
/// class as shown above in the structure example (normally you would
/// define it inline), or you can define an external operator == as in:
/// \code
/// bool operator==(const example &a, const example &b) {
///    return a.x == b.x && a.y == b.y && a.str == b.str;
/// }
/// \endcode
///
/// Then you can define your hash map:
/// HashMap<example, int> example_to_int;

template <class K, class V, 
          class KHash = HashMap_hash<const K>, 
          class KEqual = std::equal_to<const K> >
class HashMap {
public:
    typedef std::pair<K,V> value_type;
    struct value_typeHash {
	KHash khash;
	uint32_t operator()(const value_type &hmv) const {
	    return khash(hmv.first);
	}
    };
    struct value_typeEqual {
	KEqual kequal;
	bool operator()(const value_type &a, const value_type &b) const {
	    return kequal(a.first,b.first);
	}
    };

    // FUTURE: Make this function const. May have to const the returned V*. This
    // may affect much code, so Jay held off on doing this change for now (July
    // 28, 2008). --Jay
    V *lookup(const K &k) {
	value_type fullval; fullval.first = k;
	value_type *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return NULL;
	} else {
	    return &v->second;
	}
    }

    /// Returns the value associated with the key, if it exists. Otherwise,
    /// creates an entry initialized with the default value.
    
    V &operator[] (const K &k) {
	value_type fullval; fullval.first = k;
	value_type *v = hashtable.lookup(fullval);
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
	value_type fullval; fullval.first = k;
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

    typedef HashTable<value_type,value_typeHash,value_typeEqual> HashTableT;
    typedef typename HashTableT::iterator iterator;
    typedef typename HashTableT::const_iterator const_iterator;

    iterator begin() {
	return hashtable.begin();
    }
    iterator end() {
	return hashtable.end();
    }

    const_iterator begin() const {
	return hashtable.begin();
    }
    const_iterator end() const {
	return hashtable.end();
    }

    iterator find(const K &k) {
	value_type fullval; fullval.first = k;
	return hashtable.find(fullval);
    }
    
    const_iterator find(const K &k) const {
	value_type fullval; fullval.first = k;
	return hashtable.find(fullval);
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

    size_t memoryUsage() const {
	return hashtable.memoryUsage();
    }

    /// Get statistics for the chain lengths of all the chains in the
    /// underlying hash table.  Useful for detecting a bad hash
    /// function.
    void chainLengthStats(Stats &stats) {
	return hashtable.chainLengthStats(stats);
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
