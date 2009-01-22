/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/
#include <iostream>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/HashFns.hpp>

#include <Lintel/HashUnique.hpp>

using lintel::Hash;
using boost::format;
using namespace std;

template<typename T> void check(const T &v, uint32_t hv) {
    INVARIANT(lintel::hash(v) == hv, 
	      format("bad hash(%s): %d != %d") % v % lintel::hash(v) % hv);

    Hash<T> tmp;
    INVARIANT(tmp(v) == hv, format("bad Hash(%s): %d != %d") % v % lintel::hash(v) % hv);
}

namespace foo {
    struct TestNoHash {
	TestNoHash() : x(0), y(0) { }
	TestNoHash(int a, int b) : x(a), y(b) { }
	int x, y;
    };

    ostream &operator <<(ostream &to, const foo::TestNoHash &a) {
	to << format("tnh(%d,%d)") % a.x % a.y;
	return to;
    }

    uint32_t hashType(const foo::TestNoHash &a) { // this can also be in namespace lintel
	return a.x ^ a.y;
    }
}

double xhash(const foo::TestNoHash &a) {
    return a.x ^ a.y;
}

struct TestHash {
    TestHash() : x(0), y(0) { }
    TestHash(int a, int b) : x(a), y(b) { }
    int x, y;

    uint32_t hash() const { 
	return x ^ y;
    }
};

ostream &operator <<(ostream &to, const TestHash &a) {
    to << format("th(%d,%d)") % a.x % a.y;
    return to;
}

int main() {
    bool a = true;
    char b = 'a';
    unsigned char c = 'x';
    int16_t d = 1234;
    uint16_t e = 53312;
    int32_t f = 777;
    uint32_t g = 3838383;
    int64_t h = 13567876319LL;
    uint64_t i = 45678976511ULL;
    short j = -32156;
    int k = 4111;
    long l = 313515111;
    long long m = 151351351351515ULL;
    const string n("foo");
    foo::TestNoHash o(15,127);
    TestHash p(2048,1);

    check(a, 1);
    check(b, 97);
    check(c, 120);
    check(d, 1234);
    check(e, 53312);
    check(f, 777);
    check(g, 3838383);
    check(h, 3487493002UL);
    check(i, 1129882560UL);
    check(j, 33380);
    check(k, 4111);
    if (sizeof(long) == 4) {
	check(l, 313515111);
    } else {
	check(l, 3341577989U);
    }
    check(m, 4217888175U);
    check(n, 2039183822U);
    check(o, 15 ^ 127);
    check(p, 2049);

    SINVARIANT(!lintel::detail::HasMemberHash<foo::TestNoHash>::value); 
    SINVARIANT(lintel::detail::HasMemberHash<TestHash>::value);

    BOOST_STATIC_ASSERT(sizeof(xhash(o)) != 4);
    cout << "passed hashfns tests\n";

    return 0;
}

