/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/
#include <Lintel/HashTupleStats.hpp>

using namespace std;

typedef boost::tuple<int32_t, bool, string> IBSTuple;


static bool vw_seen[10];

void verifyWalk(const IBSTuple &tuple, Stats &stat) {
    int32_t v = tuple.get<0>();
    SINVARIANT(v >= 0 && v < 10);
    SINVARIANT(!vw_seen[v]);
    SINVARIANT(tuple.get<1>() == ((v % 2) == 0));
    SINVARIANT(tuple.get<2>() == str(boost::format("%d") % (v * 10)));
    SINVARIANT(stat.count() == 1);
    SINVARIANT(stat.mean() == v*5);
    vw_seen[v] = true;
}

static int32_t prev = -1;
void verifySortedWalk(const IBSTuple &tuple, Stats &stat) {
    ++prev;
    int32_t v = tuple.get<0>();
    SINVARIANT(v == prev);
    verifyWalk(tuple, stat);
}

void verifyZeroWalk(const IBSTuple &tuple, Stats &stat) {
    if (stat.count() == 1) {
	verifyWalk(tuple, stat);
    } else {
	SINVARIANT(stat.count() == 0);
	int32_t v = tuple.get<0>();
	SINVARIANT(v >= 0 && v < 10);
	SINVARIANT(tuple.get<1>() != ((v % 2) == 0) ||
		   tuple.get<2>() != str(boost::format("%d") % (v * 10)));
    }
}

bool pruneGreaterEqual5(const IBSTuple &tuple) {
    return tuple.get<0>() >= 5;
}

int main() {
    IBSTuple tmp(1,true,"a");

    lintel::HashTupleStats<IBSTuple> hts;

    for(int32_t i = 0; i < 10; ++i) {
	hts.add(IBSTuple(i,(i % 2) == 0, str(boost::format("%d") % (i * 10))), i*5);
    }

    hts.walk(boost::bind(&verifyWalk, _1, _2));
    for(unsigned i = 0; i < 10; ++i) {
	SINVARIANT(vw_seen[i]);
	vw_seen[i] = false;
    }

    hts.walkOrdered(boost::bind(&verifySortedWalk, _1, _2));

    for(unsigned i = 0; i < 10; ++i) {
	SINVARIANT(vw_seen[i]);
	vw_seen[i] = false;
    }

    hts.walkZeros(boost::bind(&verifyZeroWalk, _1, _2));

    for(unsigned i = 0; i < 10; ++i) {
	SINVARIANT(vw_seen[i]);
	vw_seen[i] = false;
    }

    lintel::HashTupleStats<IBSTuple>::HashUniqueTuple hut;

    hts.fillHashUniqueTuple(hut);

    SINVARIANT(hut.get<0>().size() == 10);
    SINVARIANT(hut.get<1>().size() == 2);
    SINVARIANT(hut.get<2>().size() == 10);

    for(unsigned i = 0; i < 10; ++i) {
	SINVARIANT(hut.get<0>().exists(i));
	SINVARIANT(hut.get<2>().exists(str(boost::format("%d") % (i * 10))));
    }

    hts.prune(boost::bind(&pruneGreaterEqual5, _1));
    
    hut.get<0>().clear();
    hut.get<1>().clear();
    hut.get<2>().clear();
    hts.fillHashUniqueTuple(hut);

    SINVARIANT(hut.get<0>().size() == 5);
    SINVARIANT(hut.get<1>().size() == 2);
    SINVARIANT(hut.get<2>().size() == 5);

    prev = -1;
    hts.walkOrdered(boost::bind(&verifySortedWalk, _1, _2));
    SINVARIANT(prev == 4);
   
    prev = -1;
    hts.clear();
    hts.walkOrdered(boost::bind(&verifySortedWalk, _1, _2));
    SINVARIANT(prev == -1);

    return 0;
}
