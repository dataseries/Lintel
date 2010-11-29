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
#include <Lintel/HashUnique.hpp>

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
	    if (!hm.exists(k + max_ents)) {
		// had a bug where removal of non-existent things gave
		// incorrect size count.  However, removal of
		// something that actually exists (low probability,
		// but possible) gives an error on hm vs stdmap size.
		hm.remove(k + max_ents, false);
	    }
	    stdmap.erase(i);
	    INVARIANT(hm.size() == stdmap.size(), format("%d != %d") % hm.size() % stdmap.size());
	}
	SINVARIANT(hm.size() == 0);
    }
    cout << format("Finished testing int type %s hash-val %u\n")
	% type % ret;
    return ret;
}

void testTypes() {
    cout << format("Using random seed %d\n") % rng.seed_used;
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

void testForeach() {
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
	uint32_t partial_hash = lintel::BobJenkinsHashMix3(a.x, a.y, 2001);
	return lintel::hashBytes(a.str.data(), a.str.size(), partial_hash);
    }
};


void testStruct() {
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
		SINVARIANT(test_map.find(tmp) == test_map.end());
	    } else {
		SINVARIANT(j < 0 && i >= 0 && *v == i + j);
		HashMap<foo, int>::iterator k = test_map.find(tmp);
		SINVARIANT(k != test_map.end() && k->first == tmp
			   && k->second == *v);
	    }
	}
    }
}

// Following just verifies that it compiles; somehow doing this in a
// class differs from doing it in a function and requires an operator=
// in the iterator to work.

class Test {
public:
    typedef HashMap<int, int> Map;
    typedef Map::iterator iterator;
    
    Test() : foo(foomap.begin()) { }
	
    void test();

    private:
	Map foomap;
	iterator foo;
};

void Test::test() {
    foo = foomap.begin();
}

void testIterator() {
    HashMap<int, int> test_map;

    test_map[3] = 3;
    
    SINVARIANT(test_map.begin() != test_map.end());
    SINVARIANT(test_map.begin()->first == 3);
    
    HashMap<int, int>::iterator a = test_map.begin();
    HashMap<int, int>::iterator b = a;
    HashMap<int, int>::iterator c(a);
	    
    ++a;
    SINVARIANT(a != b && b == c);
    ++b;
    SINVARIANT(a == b && b != c);
    ++c;
    SINVARIANT(a == b && b == c);
}
    
void testPointermap() {
    {
	HashMap<Test *, int, PointerHashMapHash<Test>, PointerHashMapEqual<Test> > test_map;
    
	Test *a = new Test();
	Test *b = new Test();

	SINVARIANT(!test_map.exists(a));
	test_map[a] = 1;
	SINVARIANT(test_map.exists(a));
	SINVARIANT(test_map[a] == 1);

	SINVARIANT(!test_map.exists(b));
	test_map[b] = 2;
	SINVARIANT(test_map.exists(b));
	SINVARIANT(test_map[a] == 1 && test_map[b] == 2);
    }
    { 
	HashMap<const Test *, int, PointerHashMapHash<const Test>, PointerHashMapEqual<const Test> > test_map;
    
	const Test *a = new Test();
	const Test *b = new Test();
	
	SINVARIANT(!test_map.exists(a));
	test_map[a] = 1;
	SINVARIANT(test_map.exists(a));
	SINVARIANT(test_map[a] == 1);
	
	SINVARIANT(!test_map.exists(b));
	test_map[b] = 2;
	SINVARIANT(test_map.exists(b));
	SINVARIANT(test_map[a] == 1 && test_map[b] == 2);
    }
}
    
void testConstA(const HashMap<int, int> &test_map, const HashMap<int, int> &test_map2) {
    HashMap<int, int>::const_iterator i = test_map.find(5);
    SINVARIANT(i != test_map.end() && i->second == 5);
    i = test_map.find(6);
    SINVARIANT(i == test_map.end());

    i = test_map2.find(5);
    SINVARIANT(i == test_map2.end());
    i = test_map2.find(6);
    SINVARIANT(i != test_map2.end() && i->second == 6);

    SINVARIANT(test_map.cGet(5) == 5);
    SINVARIANT(test_map2.cGet(6) == 6);
    SINVARIANT(test_map.dGet(33) == 0);

    SINVARIANT(test_map.lookup(5) != NULL && *test_map.lookup(5) == 5);
    SINVARIANT(test_map.lookup(6) == NULL);
}

void testConstHashUnique(const HashUnique<int> &unique) {
    SINVARIANT(unique.exists(3));
    SINVARIANT(!unique.exists(0));
}

void testConst() {
    HashMap<int, int> test_map;
    HashMap<int, int> test_map2;

    test_map[5] = 5;
    test_map2[6] = 6;
    testConstA(test_map, test_map2);
    SINVARIANT(test_map[5] == 5 && test_map2[6] == 6 &&
	       test_map.size() == 1 && test_map2.size() == 1);

    HashUnique<int> unique;
    unique.add(3);
    testConstHashUnique(unique);
    cout << "Const tests pass\n";
}

void testKeys() {
    HashMap<int, int> test_map;

    for(int i = 0; i < 10; ++i) {
	test_map[i] = i;
    }
    vector<int> keys = test_map.keys();
    sort(keys.begin(), keys.end());
    SINVARIANT(keys.size() == 10);
    for(int i = 0; i < 10; ++i) {
	SINVARIANT(keys[i] == i);
    }
}

void testErase() {
    MersenneTwisterRandom rng;

    cout << format("erase test using seed %d\n") % rng.seed_used;
    vector<uint32_t> ents;
    HashMap<uint32_t, uint32_t> table;

    static const uint32_t nents = 1000;

    for (uint32_t i = 0; i < nents; ++i) {
	uint32_t v = rng.randInt();
	ents.push_back(v);
	table[v] = v;
    }

    MT_random_shuffle(ents.begin(), ents.end(), rng);
    
    for (uint32_t i = 0; i < nents; ++i) {
	SINVARIANT(table.size() == nents - i);
	HashMap<uint32_t, uint32_t>::iterator e = table.find(ents[i]);
	SINVARIANT(e != table.end());
	table.erase(e);
	for(uint32_t j = 0; j <= i; ++j) {
	    INVARIANT(table.lookup(ents[j]) == NULL, format("%d %d") % i % j);
	}
	for(uint32_t j = i + 1; j < nents; ++j) {
	    INVARIANT(table.lookup(ents[j]) != NULL, format("%d %d") % i % j);
	}
    }
    SINVARIANT(table.empty() && table.size() == 0);
    cout << "erase test passed.\n";
}

void testAddUnlessExist() {
    HashMap<int32_t, string> hm;
    hm[0] = "zero";
    hm[1] = "one";
    SINVARIANT(hm.size() == 2 && hm[0] == "zero" && hm[1] == "one");
    hm.addUnlessExist(0); // no effect
    SINVARIANT(hm.size() == 2 && hm[0] == "zero"); 
    SINVARIANT(!hm.exists(2));
    hm.addUnlessExist(2);
    SINVARIANT(hm.exists(2));
    SINVARIANT(hm.size() == 3 && hm[2] == ""); // default value of string is ""

    // Same test, but with base type to verify defaulting works for base types.
    HashMap<int32_t, int> hm2; 
    hm2[0] = 1;
    hm2[1] = 2;
    SINVARIANT(hm2.size() == 2 && hm2[0] == 1 && hm2[1] == 2);
    hm2.addUnlessExist(0); // no effect
    SINVARIANT(hm2.size() == 2 && hm2[0] == 1); 
    SINVARIANT(!hm2.exists(2));
    hm2.addUnlessExist(2);
    SINVARIANT(hm2.exists(2));
    SINVARIANT(hm2.size() == 3 && hm2[0] == 1 && hm2[1] == 2 && hm2[2] == 0);
}

int main() {
    testTypes();
    testForeach();
    testStruct();
    testIterator();
    testPointermap();
    testConst();
    testKeys();
    testErase();
    testAddUnlessExist();
}
