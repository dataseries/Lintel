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
#include "boost_foreach.hpp"

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

void test_foreach() {
    map<uint32_t, uint32_t> test_map;
    typedef map<uint32_t, uint32_t>::value_type test_map_vt;
    HashMap<uint32_t, uint32_t> test_hm;
    typedef HashMap<uint32_t, uint32_t>::value_type test_hm_vt;

    for(unsigned i = 0; i < 100; ++i) {
	uint32_t v = rng.randInt();
	test_map[v] = ~v;
	test_hm[v] = ~v;
    }
    
    BOOST_FOREACH(const test_map_vt &i, test_map) {
	SINVARIANT(i.first == ~i.second);
	SINVARIANT(test_hm[i.first] == i.second);
    }

    BOOST_FOREACH(const test_hm_vt &i, test_hm) {
	SINVARIANT(i.first == ~i.second);
	SINVARIANT(test_map[i.first] == i.second);
    }
    cout << "Finished testing with foreach.\n";
}

struct foo {
    int32_t x, y;
    string str;
    bool operator==(const foo &right) const {
	return x == right.x && y == right.y && str == right.str;
    }
    foo() : x(0), y(0) { }

    foo(int32_t a, int32_t b, const string &c) : x(a), y(b), str(c) { }
};

template <> struct HashMap_hash<const foo> {
    uint32_t operator()(const foo &a) const {
	// 2001 is an arbitrary constant; could also use the return
	// from hashbytes, which will make up a start hash if one
	// isn't provided.
	uint32_t partial_hash = BobJenkinsHashMix3(a.x, a.y, 2001);
	return HashTable_hashbytes(a.str.data(), a.str.size(), partial_hash);
    }
};


void test_struct() {
    HashMap<foo, int> test_map;

    for(int i=0; i < 10; ++i) {
	for(int j=-10; j < 0; ++j) {
	    test_map[foo(i, j, (format("%d,%d") % i % j).str())] = i + j;
	}
    }

    for(int j=-10; j < 10; ++j) {
	for(int i=-10; i < 10; ++i) {
	    foo tmp(i, j, (format("%d,%d") % i % j).str());
	    int *v = test_map.lookup(tmp);
	    if (v == NULL) {
		SINVARIANT(j >= 0 || i < 0);
	    } else {
		SINVARIANT(j < 0 && i >= 0 && *v == i + j);
	    }
	}
    }
}

int main() {
    test_types();
    test_foreach();
    test_struct();
}
