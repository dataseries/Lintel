/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#ifndef LINTEL_TUPLES_HPP
#define LINTEL_TUPLES_HPP

#include <bitset>

#include <boost/tuple/tuple_comparison.hpp>

#include <Lintel/HashMap.hpp>

/** @file 

    Additional tuple bits to extend what is in the boost tuples.  We
    add hashing of tuples and partial tuples
*/

namespace lintel { namespace tuples {
    // these types are so integral to the implementation of
    // lintel::tuples that it seems reasonable to pull them into
    // lintel tuples namespace.  Originally this code was just written
    // in the boost::tuples namespace, but that seemed wrong.
    using boost::tuples::null_type;
    using boost::tuples::cons;

    inline uint32_t hash(const null_type &) { return 0; }
    template<typename T> inline uint32_t hash(const T &v) {
	HashMap_hash<T> tmp;
	return tmp(v);
    }

    template<class Head>
    inline uint32_t hash(const cons<Head, null_type> &v) {
	return hash(v.get_head());
    }

    // See http://burtleburtle.net/bob/c/lookup3.c for a discussion
    // about how we could have even fewer calls to the mix function.
    // Would want to upgrade to the newer hash function.
    template<class Head1, class Head2, class Tail>
    inline uint32_t hash(const cons<Head1, cons<Head2, Tail> > &v) {
	uint32_t a = hash(v.get_head());
	uint32_t b = hash(v.get_tail().get_head());
	uint32_t c = hash(v.get_tail().get_tail());
	return BobJenkinsHashMix3(a,b,c);
    }

    template<class BitSet>
    inline uint32_t partial_hash(const null_type &, const BitSet &, size_t) {
	return 0;
    }

    template<class Head, class BitSet>
    inline uint32_t partial_hash(const cons<Head, null_type> &v,
				 const BitSet &used, size_t cur_pos) {
	return used[cur_pos] ? hash(v.get_head()) : 0;
    }

    template<class Head1, class Head2, class Tail, class BitSet>
    inline uint32_t partial_hash(const cons<Head1, cons<Head2, Tail> > &v, 
				 const BitSet &used, size_t cur_pos) {
	uint32_t a = used[cur_pos] ? hash(v.get_head()) : 0;
	uint32_t b = used[cur_pos+1] ? hash(v.get_tail().get_head()) : 0;
	uint32_t c = partial_hash(v.get_tail().get_tail(), used, cur_pos + 2);
	return BobJenkinsHashMix3(a,b,c);
    }

    template<class BitSet>
    inline bool partial_equal(const null_type &lhs, const null_type &rhs,
			      const BitSet &used, size_t cur_pos) {
	return true;
    }

    template<class Head, class Tail, class BitSet>
    inline bool partial_equal(const cons<Head, Tail> &lhs, const cons<Head, Tail> &rhs,
			      const BitSet &used, size_t cur_pos) {
	if (used[cur_pos] && lhs.get_head() != rhs.get_head()) {
	    return false;
	}
	return partial_equal(lhs.get_tail(), rhs.get_tail(), used, cur_pos + 1);
    }

    template<class BitSet>
    inline bool partial_strict_less_than(const null_type &lhs, const null_type &rhs,
					 const BitSet &used_lhs, const BitSet &used_rhs,
					 size_t cur_pos) {
	return false;
    }

    template<class Head, class Tail, class BitSet>
    inline bool partial_strict_less_than(const cons<Head, Tail> &lhs, const cons<Head, Tail> &rhs,
					 const BitSet &used_lhs, const BitSet &used_rhs,
					 size_t cur_pos) {
	if (used_lhs[cur_pos] && used_rhs[cur_pos]) {
	    if (lhs.get_head() < rhs.get_head()) {
		return true;
	    } else if (lhs.get_head() > rhs.get_head()) {
		return false;
	    } else {
		// don't know, fall through to recursion.
	    }
	} else if (used_lhs[cur_pos] && !used_rhs[cur_pos]) {
	    return true; // used < *
	} else if (!used_lhs[cur_pos] && used_rhs[cur_pos]) {
	    return false; // * > used
	} 
	return partial_strict_less_than(lhs.get_tail(), rhs.get_tail(), 
					used_lhs, used_rhs, cur_pos + 1);
    }

    template<class Tuple> struct TupleHash {
	uint32_t operator()(const Tuple &a) const {
	    return lintel::tuples::hash(a);
	}
    };

    // Not a pair so we can have operators that behave differently, and
    // to improve clarity.
    template<typename T> struct AnyPair {
	bool any;
	T val;
	AnyPair() : any(true) { }
	AnyPair(const T &v) : any(false), val(v) { }
	void set(const T &v) { val = v; any = false; }
	
	bool operator ==(const AnyPair<T> &rhs) const {
	    if (any != rhs.any) {
		return false;
	    } else if (any) {
		return true;
	    } else {
		return val == rhs.val;
	    }
	}
	bool operator <(const AnyPair<T> &rhs) const {
	    if (any && rhs.any) {
		return false;
	    } else if (any && !rhs.any) {
		return false;
	    } else if (!any && rhs.any) {
		return true;
	    } else if (!any && !rhs.any) {
		return val < rhs.val;
	    }
	    FATAL_ERROR("?");
	}
    };

    template<class T0, class T1> struct ConsToAnyPairCons {
	typedef ConsToAnyPairCons<typename T1::head_type,
				  typename T1::tail_type> tail;
	typedef boost::tuples::cons<AnyPair<T0>, typename tail::type> type;
    };
    
    template<class T0> struct ConsToAnyPairCons<T0, null_type> {
	typedef boost::tuples::cons<AnyPair<T0>, null_type> type;
    };

    template<class T> struct TupleToAnyTuple 
	: ConsToAnyPairCons<typename T::head_type, typename T::tail_type> 
    { };

    template<class T> inline uint32_t hash(const AnyPair<T> &v) {
	if (v.any) {
	    return 0;
	} else {
	    return hash(v.val);
	}
    }

    template<class Tuple> struct PartialTuple {
	BOOST_STATIC_CONSTANT(uint32_t, length = boost::tuples::length<Tuple>::value);
	
	Tuple data;
	typedef std::bitset<boost::tuples::length<Tuple>::value> UsedT;
	UsedT used;
	
	PartialTuple() { }
	// defaults to all unused
	explicit PartialTuple(const Tuple &from) : data(from) { }
	
	bool operator==(const PartialTuple &rhs) const {
	    if (used != rhs.used) { 
		return false;
	    }
	    return lintel::tuples::partial_equal(data, rhs.data, used, 0);
	}
	bool operator<(const PartialTuple &rhs) const {
	    return lintel::tuples::partial_strict_less_than(data, rhs.data, used, rhs.used, 0);
							   
	}
    };

    template<class Tuple> struct PartialTupleHash {
	uint32_t operator()(const PartialTuple<Tuple> &v) const {
	    return lintel::tuples::partial_hash(v.data, v.used, 0);
	}
    };
} }

#endif
