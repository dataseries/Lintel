/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    test for hashmap
*/

#include <iostream>
#include <map>

#include <Lintel/HashMap.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>

using namespace std;
using boost::format;

// The next two bits verify if other implementations of hash_maps can
// handle a const key; As of 2008-06-14 on Debian stable (etch) with
// g++ 4.1.2 (debian 4.1.1-21), both get compile errors.  Note this
// does work for a std::map.

#if 0
#include <ext/hash_map>

void foo_gxx_hash_map() {
    __gnu_cxx::hash_map<const int32_t, int32_t> test;

    test[5] = 7;
}
#endif

#if 0
#include <tr1/unordered_map>

void foo_tr1_unordered_map() {
    tr1::unordered_map<const int32_t, int32_t> test;

    test[5] = 7;
}
#endif

MersenneTwisterRandom rng;

template<typename INT> uint32_t test_int_type(const string &type) {
    typedef INT test_int_t;
    HashMap<test_int_t, uint32_t> hm;

    typedef map<test_int_t, uint32_t> stdmap_t;
    typedef typename stdmap_t::iterator iterator_t;
    stdmap_t stdmap;

    static const unsigned max_ents = 1000;

    {
	for(unsigned i = 0; i < max_ents; ++i) {
	    test_int_t k = static_cast<test_int_t>(rng.randLongLong());
	    uint32_t v = rng.randInt();
	    hm[k] = v;
	    stdmap[k] = v;
	    SINVARIANT(hm.size() == stdmap.size());
	}
    }

    {
	// test overwrite;
	for(iterator_t i = stdmap.begin();
	    i != stdmap.end(); ++i) {
	    SINVARIANT(hm[i->first] == i->second);
	    hm[i->first] = i->second;
	}

	SINVARIANT(hm.size() == stdmap.size());
    }

    uint32_t ret = 0;
    {
	// test removal
	while(!stdmap.empty()) {
	    iterator_t i = stdmap.begin();
	    test_int_t k = i->first;
	    uint32_t v = i->second;

	    ret ^= k ^ v;
	    SINVARIANT(hm[k] == v);
	    hm.remove(k);
	    stdmap.erase(i);
	    SINVARIANT(hm.size() == stdmap.size());
	}
	SINVARIANT(hm.size() == 0);
    }
    cout << format("Finished testing int type %s hash-val %u\n")
	% type % ret;
    return ret;
}

void test_types() {
    // these runs test HashMap against std::map with random key-value pairs.
    test_int_type<int>("int");
    test_int_type<unsigned>("unsigned");
    test_int_type<long>("long");
    test_int_type<unsigned long>("unsigned long");
    test_int_type<long long>("long long");
    test_int_type<unsigned long long>("unsigned long long");

    
    test_int_type<int32_t>("int32_t");
    test_int_type<uint32_t>("uint32_t");
    test_int_type<int64_t>("int64_t");
    test_int_type<uint64_t>("uint64_t");

    // Verify these compile; TODO: test usage.
    HashMap<string, int> hm_string;
    HashMap<const string, int> hm_const_string;
    
    HashMap<char *, int> hm_charstar;
    HashMap<const char *, int> hm_constcharstar;

    // Test with a specific seed so every machine should get same results.
    rng.init(1776);

    SINVARIANT(test_int_type<int>("int") == 122073032);
}

int main() {
    test_types();
}
