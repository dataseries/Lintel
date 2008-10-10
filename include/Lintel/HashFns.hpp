/* -*-C++-*- */
/*
   (c) Copyright 2003-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#ifndef LINTEL_HASHFNS_HPP
#define LINTEL_HASHFNS_HPP

/*! \file HashFns.hpp
    \brief File documentation explains how to extend hashing to new types.
    
 * To add a hash function to a new type T, you can do one of two things:
 *   -# add a uint32_t hash() const { ... } function to T's class definition.
 *   -# add a uint32_t hashType(const T) { ... } function; either in the lintel namespace 
 *      or the same namespace as the underlying type.
 * 
 * When you write the hash function, you probably want to use
 * lintel_BobJenkinsHashMix(a,b,c), and/or lintel::bobJenkinsHash()
 * in order to make your hash function.
 *
 * We make the global function named hashType because if it was named
 * hash and put in the wrong namespace, the Hash class and the hash
 * function could recurse infinitely.
 */

#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_integral.hpp>

/// A fast way of doing an inline mix of three integers; used in the
/// BobJenkinsHash as a core operation; this is placed here so that
/// hashing on a bunch of small integers can be done quickly without a
/// function call and all of the variable length overhead inherent in
/// the general purpose hash.  c is what is nominally returned from
/// mixing.
#define lintel_BobJenkinsHashMix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

namespace lintel { 
    /// prev_hash allows you to hash data where the key is in separate pieces
    /// it may not get the same hash as concatenating the key would give
    /// 1972 is an arbitrary start
    ///
    /// note that the order of arguments here is different than for the
    /// HashTable_hashbytes function; this is to make the order consistent
    /// with how the crc/adler hash's work in both zlib and lzo library, and
    /// the ordering for the md5, sha1, etc. hashs from openssl; the order for 
    /// the other function is retained as the common usage is to not specify a
    /// previous hash
    uint32_t bobJenkinsHash(const uint32_t prev_hash,
			    const void *bytes, const size_t size);

    inline uint32_t hashBytes(const void *bytes, const size_t size,
			      const uint32_t prev_hash = 1972) { 
	return bobJenkinsHash(prev_hash, bytes, size);
    }

    // TODO: update with the revised hash function at:
    // http://burtleburtle.net/bob/hash/doobs.html
    // or the alternative one at:
    // http://murmurhash.googlepages.com/
    
    static inline uint32_t BobJenkinsHashMix3(uint32_t a, uint32_t b, uint32_t c) {
	lintel_BobJenkinsHashMix(a,b,c);
	return c;
    }

    static inline uint32_t 
    BobJenkinsHashMixULL(uint64_t v, uint32_t partial = 1972) {
	uint32_t a = static_cast<uint32_t>(v & 0xFFFFFFFF);
	uint32_t b = static_cast<uint32_t>((v >> 32) & 0xFFFFFFFF);
	uint32_t c = partial;
	lintel_BobJenkinsHashMix(a,b,c);
	return c;
    }

    namespace detail {
	// Overview of magic in here:
	//
	// 1) determine whether a class has a uint32_t hash() const { } function.
	//    if so, use that function for the hash.  The template magic is from
	//    http://lists.boost.org/Archives/boost/2002/03/27229.php; cleaned up
	//    to match our coding conventions.
	// 2) determine whether the type is integral, and if so do a whole bunch
	//    of complicated per size stuff so that we can work properly on 
	//    different platforms.  The problem boils down to 32bit Debian etch
	//    things long long and int64_t are the same types, and so can't have
	//    hash functions declared for both.  Conversely, 64bit RHEL4 thinks
	//    they are different types, and hence requires a separate definition
	//    for hashing both of these.
	// 3) if neither of the above is true; then look for a uint32_t hash(const T &)
	//    function, and use that one.
	typedef char (&no_tag)[1]; 
	typedef char (&yes_tag)[2]; 
 
	template <typename T, uint32_t (T::*)() const> struct PtmfHashHelper { }; 
	template <typename T> no_tag HasMemberHashHelper(...); 
 
	template <typename T> 
	yes_tag HasMemberHashHelper(PtmfHashHelper<T, &T::hash>* p); 
 
	template <typename T> struct HasMemberHash { 
	    BOOST_STATIC_CONSTANT(bool, 
				  value = sizeof(HasMemberHashHelper<T>(0)) == sizeof(yes_tag) 
             ); 
	}; 
 
	template <size_t Size> struct HashIntegral {
	};

	template <> struct HashIntegral<1> {
	    uint32_t operator()(const uint8_t v) const {
		BOOST_STATIC_ASSERT(sizeof(uint8_t) == 1);
		return v;
	    }
	};

	template <> struct HashIntegral<2> {
	    uint32_t operator()(const uint16_t v) const {
		BOOST_STATIC_ASSERT(sizeof(uint16_t) == 2);
		return v;
	    }
	};

	template <> struct HashIntegral<4> {
	    uint32_t operator()(const uint32_t v) const {
		BOOST_STATIC_ASSERT(sizeof(uint32_t) == 4);
		return v;
	    }
	};

	template <> struct HashIntegral<8> {
	    uint32_t operator()(const uint64_t v) const {
		BOOST_STATIC_ASSERT(sizeof(uint64_t) == 8);
		return BobJenkinsHashMixULL(v);
	    }
	};

	template <typename T, bool IsIntegral, bool HasHash> struct HashDispatch {
	};
	
	template <typename T> struct HashDispatch<T, true, false> : HashIntegral<sizeof(T)> {
	};
	    
	template <typename T> struct HashDispatch<T, false, true> {
	    uint32_t operator()(const T &v) const {
		return v.hash();
	    }
	};

	template <typename T> struct HashDispatch<T, false, false> {
	    uint32_t operator()(const T &v) const {
		// I'd like to check boost::is_integral<hash(v)>, but
		// that just fails.  Using typeid doesn't work any
		// better.
		BOOST_STATIC_ASSERT(sizeof(hash(v)) == 4);
		return hashType(v);
	    }
	};
    }

    template <typename V> struct Hash 
    : detail::HashDispatch<V, boost::is_integral<V>::value, detail::HasMemberHash<V>::value> {
	// uint32_t operator()(const V &v) const;
    };

    template <typename T> inline uint32_t hash(const T &v) { 
	return Hash<T>()(v);
    }

    inline uint32_t hashType(const std::string &a) { 
	return hashBytes(a.data(), a.length());
    }
    
    inline uint32_t hashType(const char * const a) {
	return hashBytes(a, strlen(a));
    }

    /// Object comparison by pointer, useful for making a hash
    /// structure on objects where each object instance is separate.
    /// Not a default implementation for pointers because that isn't
    /// safe.  People could reasonably expect the comparison to be
    /// done as hash(*a) also.  Used by HashMap<K, V,
    /// lintel::PointerHash<K>, lintel::PointerEqual<K> > the last bit
    /// can be left out if there are no operator == (const K *, const
    /// K *) functions defined.
    template<typename T> struct PointerHash {
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
    template<typename T> struct PointerEqual {
	bool operator()(const T *a, const T *b) const {
	    return a == b;
	}
    };

    /// SharedPointerHashing, same as PointerHash
    template<typename T> struct SharedPointerHash {
	uint32_t operator()(const boost::shared_ptr<T> &a) const {
	    BOOST_STATIC_ASSERT(sizeof(a.get()) == 4 || sizeof(a.get()) == 8);
	    if (sizeof(a.get()) == 4) {
		// RHEL4 64bit requires two stage cast even though this
		// branch should never be executed.
		return static_cast<uint32_t>
		    (reinterpret_cast<size_t>(a.get()));
	    } else if (sizeof(a) == 8) {
		return BobJenkinsHashMixULL
		    (reinterpret_cast<uint64_t>(a.get()));
	    }
	}
    };
    
    /// SharedPointerEqual; may always be needed as opposed to PointerEqual
    template<typename T> struct SharedPointerEqual {
	bool operator()(const boost::shared_ptr<T> &a, 
			const boost::shared_ptr<T> &b) const {
	    return a.get() == b.get();
	}
    };

} 

#endif
