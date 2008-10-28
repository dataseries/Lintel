/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#ifndef LINTEL_HASH_TUPLE_STATS_HPP
#define LINTEL_HASH_TUPLE_STATS_HPP

#include <bitset>

#include <boost/bind.hpp>

#include <Lintel/HashTable.hpp>
#include <Lintel/HashUnique.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/Stats.hpp>
#include <Lintel/Tuples.hpp>

// TODO: consider renaming this to TupleHashMap or something like that
// once we support a copy constructor in StatsQuantile, and/or come up
// with some way to use a ptr argument and have a function that does
// the initialization.

namespace lintel {
    namespace detail {
	template<class T0, class T1> struct ConsToHashUniqueCons {
	    typedef ConsToHashUniqueCons<typename T1::head_type, typename T1::tail_type> tail;
	    typedef boost::tuples::cons<HashUnique<T0>, typename tail::type> type;
	};
	
	template<class T0> struct ConsToHashUniqueCons<T0, boost::tuples::null_type> {
	    typedef boost::tuples::cons<HashUnique<T0>, boost::tuples::null_type> type;
	};
	
	template<class T>
	struct TupleToHashUniqueTuple 
	    : ConsToHashUniqueCons<typename T::head_type, typename T::tail_type> {
	};

	void hashUniqueTupleAdd(boost::tuples::null_type, boost::tuples::null_type) {
	}

	template<class HUT, class T>
	void hashUniqueTupleAdd(HUT &hut, const T &v) {
	    hut.get_head().add(v.get_head());
	    hashUniqueTupleAdd(hut.get_tail(), v.get_tail());
	}

	double zeroCubeBaseCount(const boost::tuples::null_type) { 
	    return 1; 
	}

	template<class HUT>
	double zeroCubeBaseCount(const HUT &hut) {
	    return hut.get_head().size() * zeroCubeBaseCount(hut.get_tail());
	}

	template<class KeyBase, class BaseData, class Function>
	void zeroWalk(const boost::tuples::null_type, const boost::tuples::null_type, 
		      KeyBase &key_base, const BaseData &base_data, Stats &null_stat, 
		      const Function &fn) {
	    typedef typename BaseData::const_iterator iterator;
	    iterator i = base_data.find(key_base);
	    if (i == base_data.end()) {
		fn(key_base, null_stat); 
	    } else {
		fn(key_base, *(i->second)); 
	    }
	}
	
	template<class HUT, class KeyTail, class KeyBase, class BaseData, 
		 class Function>
	void zeroWalk(const HUT &hut, KeyTail &key_tail, KeyBase &key_base, 
		      const BaseData &base_data, Stats &null_stat, 
		      const Function &fn) {
	    typedef typename HUT::head_type::const_iterator const_iterator;
	    
	    for(const_iterator i = hut.get_head().begin(); 
		i != hut.get_head().end(); ++i) {
		key_tail.get_head() = *i;
		
		zeroWalk(hut.get_tail(), key_tail.get_tail(), key_base,
			 base_data, null_stat, fn);
	    }
	}

	template<class T> T *newT() {
	    return new T();
	}
    }

    /// A hash table from tuples to "stats" Includes a bunch of
    /// additional functions that help tie this together with the cube
    /// operation; in particular the various walk functions.
    template<class Tuple, class StatsT = Stats> class HashTupleStats {
    public:
	// base types
	typedef HashMap<Tuple, StatsT *, lintel::tuples::TupleHash<Tuple> > HTSMap;
	typedef typename HTSMap::iterator HTSiterator;
	typedef typename HTSMap::const_iterator HTSconst_iterator;
	typedef std::vector<typename HTSMap::value_type> HTSValueVector;
	typedef typename HTSValueVector::iterator HTSVViterator;

	// zero cubing types
	typedef detail::TupleToHashUniqueTuple<Tuple> HUTConvert;
	typedef typename HUTConvert::type HashUniqueTuple;

	// functions
	typedef boost::function<StatsT *()> StatsFactoryFn;
	typedef boost::function<void (const Tuple &key, StatsT &value)> WalkFn;
	typedef boost::function<bool (const Tuple &key)> PruneFn;

	explicit HashTupleStats(const StatsFactoryFn &fn1 = boost::bind(&detail::newT<StatsT>))
	    : stats_factory_fn(fn1)
	{ }

	~HashTupleStats() {
	    clear();
	}

	/// add a value into the stats based on the selected key.
	void add(const Tuple &key, double value) {
	    getHashEntry(key).add(value);
	}

	/// apply walk_fn(const Tuple &, StatsT &) to each entry in the
	/// hash table; entries are walked in a pseudo-random order.
	void walk(const WalkFn &walk_fn) const {
	    for(HTSconst_iterator i = data.begin(); i != data.end(); ++i) {
		walk_fn(i->first, *i->second);
	    }
	}

	/// apply walk_fn(const Tuple &, StatsT &) to each entry,
	/// sorted in tuple order.  If you can avoid using this
	/// function, do, it will use additional memory as a result of
	/// having to sort the tuples.
	void walkOrdered(const WalkFn &walk_fn) const {
	    HTSValueVector sorted;

	    sorted.reserve(data.size());
	    // TODO: figure out why the below doesn't work.
	    //	sorted.push_back(base_data.begin(), base_data.end());
	    for(HTSconst_iterator i = data.begin(); i != data.end(); ++i) {
		sorted.push_back(*i);
	    }
	    sort(sorted.begin(), sorted.end());
	    for(HTSVViterator i = sorted.begin(); i != sorted.end(); ++i) {
		walk_fn(i->first, *i->second);
	    }
	}

	/// apply walk_fn(const Tuple &, StatsT &) to each entry,
	/// including ones that would have been present if the full
	/// cross product of all tuple values were present.  For
	/// example, if there are two entries in the HTS ([0,1] -> v,
	/// [1,0] -> v') then by creating an empty StatsT z, walk_fn
	/// will be called four times, with ([0,0], z), ([0,1], v),
	/// ([1,0], v'), ([1,1], z).
	void walkZeros(const WalkFn &walk_fn) const {
	    HashUniqueTuple hut;

	    fillHashUniqueTuple(hut);

	    walkZeros(walk_fn, hut);
	}

	/// build a tuple of hash uniques out of the entries in the
	/// table, useful for determining all of the values that are
	/// used for any of the entries.
	void fillHashUniqueTuple(HashUniqueTuple &hut) const {
	    for(HTSconst_iterator i = data.begin(); i != data.end(); ++i) {
		hashUniqueTupleAdd(hut, i->first);
	    }
	}

	/// implementation of the zero walk function, separated out
	/// from the above because separately calculating the unique
	/// entries table would allow you to remove some entries out
	/// of the hash unique tuple and therefore avoid walking over
	/// some of the entries.
	void walkZeros(const WalkFn &walk_fn, const HashUniqueTuple &hut) const {
	    double expected_hut = zeroCubeBaseCount(hut);

	    uint32_t tuple_len = boost::tuples::length<Tuple>::value;

	    LintelLogDebug("HostInfo",
			   boost::format("Expecting to cube %.6g * 2^%d = %.0f")
			   % expected_hut % tuple_len 
			   % (expected_hut * pow(2.0, tuple_len)));

	    StatsT *zero = stats_factory_fn();

	    Tuple tmp_key;
	    detail::zeroWalk(hut, tmp_key, tmp_key, data, *zero, walk_fn);
	
	    delete zero;
	}

	/// Get the hash entry for a particular key, create it if it
	/// doesn't already exist.
	StatsT &getHashEntry(const Tuple &key) {
	    StatsT * &v = data[key];

	    if (v == NULL) {
		v = stats_factory_fn();
		SINVARIANT(v != NULL);
	    }
	    return *v;
	}

	/// operator version of getHashEntry()
	StatsT &operator[](const Tuple &key) {
	    return getHashEntry(key);
	}

	/// how big is the hash tuple stats
	size_t size() const {
	    return data.size();
	}

	/// remove a set of values from the hash tuple stats, removes
	/// all of the entries from the table that return true when
	/// prune is called with the tuple key.
	void prune(PruneFn fn) {
	    for(HTSiterator i = data.begin(); i != data.end(); ) {
		if (fn(i->first)) {
		    delete i->second;
		    data.remove(i->first);
		    i.partialReset();
		} else {
		    ++i;
		}
	    }
	}
    
	/// clear out all the values in the hash tuple stats.
	void clear() {
	    for(HTSiterator i = data.begin(); i != data.end(); ++i) {
		delete i->second;
	    }
	    data.clear();
	}

	/// how much memory is the hash tuple stats using; accurate
	/// only if StatsT does not have internal allocated memory.
	size_t memoryUsage() const {
	    return data.memoryUsage() + data.size() * sizeof(StatsT) + sizeof(*this);
	}

    private:
	HTSMap data;
	StatsFactoryFn stats_factory_fn;
    };
}

#endif
