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

#include <Lintel/HashTable.hpp>

#include <boost/type_traits/is_integral.hpp>

/// template to hash integral type things; this lets us handle all of
/// the combinations of int64_t, long long, etc., without having to
/// have different variants for all of the sub-types, which is
/// currently impossible because we can't have both long long and
/// int64_t instantiations of the HashMap_hash template.  32bit x86
/// Debian etch thinks those types are the same, so it complains about
/// duplicate templates, but 64bit x86_64 RHEL4 thinks the types are
/// different so requires both of them.  
template<bool isIntegral, int size> struct HashMap_hash_int {
    //    uint32_t operator()(const K &a) const;
};

template <class K> 
struct HashMap_hash 
    : HashMap_hash_int<boost::is_integral<K>::value, sizeof(K)> {
    //    uint32_t operator()(const K &a) const;
};

/// Specialization for std::string
template <> struct HashMap_hash<const std::string> {
    uint32_t operator()(const std::string &a) const {
	return HashTable_hashbytes(a.data(),a.size());
    }
};

/// Specialization for char *
template <> struct HashMap_hash<const char * const> {
    uint32_t operator()(const char * const a) const {
	return HashTable_hashbytes(a,strlen(a));
    }
};

/// To achieve the HashMap_hash_int thing, need to have
/// specializations for the different sizes; this is 4 byte size.
template <> struct HashMap_hash<const uint32_t> {
    uint32_t operator()(const uint32_t _a) const {
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
	return static_cast<uint32_t>(_a);
    }
};

/// specialization for 8 byte integers
template <> struct HashMap_hash<const uint64_t> {
    uint32_t operator()(const uint64_t _a) const {
	return BobJenkinsHashMixULL(_a);
    }
};

/// general 4 byte integer hashing
template<> struct HashMap_hash_int<true, 4> : HashMap_hash<const uint32_t> {
    BOOST_STATIC_ASSERT(sizeof(uint32_t) == 4);
};

// general 8 byte integer hashing
template<> struct HashMap_hash_int<true, 8> : HashMap_hash<const uint64_t> {
    BOOST_STATIC_ASSERT(sizeof(uint64_t) == 8);
};

/// Object comparison by pointer, useful for making a hash structure
/// on objects where each object instance is separate.  Instantiated
/// separately rather than as a variant of HashMap_hash because having
/// this as the default behavior doesn't seem safe; people could
/// reasonably expect the comparison to be done as hash(*a) also.
/// Used by HashMap<K, V, PointerHashMapHash<K>,
/// PointerHashMapEqual<K> >
template<typename T> struct PointerHashMapHash {
    uint32_t operator()(const T *a) const {
	BOOST_STATIC_ASSERT(sizeof(a) == 4 || sizeof(a) == 8);
	if (sizeof(a) == 4) {
	    // RHEL4 64bit requires two stage cast even though this
	    // branch should never be executed.
	    return static_cast<uint32_t>(reinterpret_cast<size_t>(a));
	} else if (sizeof(a) == 8) {
	    return BobJenkinsHashMixULL(reinterpret_cast<uint64_t>(a));
	}
    }
};

/// Object equality check -- unnecessary unless operators have been
/// defined.
template<typename T> struct PointerHashMapEqual {
    bool operator()(const T *a, const T *b) const {
	return a == b;
    }
};

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
/// }
/// \endcode
/// you would need to define the HashMap_hash<example> and operator ==
/// on struct example.  To define HashMap_hash<example>>, you would write:
/// \code
/// template<> struct HashMap_hash<const example> {
///     uint32_t operator()(const example &a) const {
///         uint32_t partial_hash = BobJenkinsHashMix3(a.x, a.y, 2001);
///         return HasHTable_hashbytes(a.str().data, a.str.size(), partial_hash);
///     }
/// } 
/// \endcode
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
