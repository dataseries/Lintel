/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Double.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/Stats.hpp>
#include <Lintel/StringUtil.hpp>

using namespace std;
using boost::format;

void checkEqual(Stats &a, Stats &b, uint32_t iters) {
    INVARIANT(a.count() == b.count(), "count mismatch");
    INVARIANT(a.min() == b.min(), "min mismatch");
    INVARIANT(a.max() == b.max(), "max mismatch");
    INVARIANT(Double::eq(a.mean(), b.mean()), "mean mismatch");

    // With seed 2069933494 on an Opteron 8220 SE with Debian etch
    // 32bit, the following check fails after one iteration.  The
    // error appears to be down in the last fp bit since the error is
    // almost exactly 2^-53, and it just gets magnified up, so we fail
    // to be equal to the default epsilon.
    double var_a = a.total_sq()/static_cast<double>(a.count()) - a.mean() * a.mean();
    double var_b = b.total_sq()/static_cast<double>(b.count()) - b.mean() * b.mean();

    INVARIANT(Double::eq(a.variance(), b.variance(), 5.0e-12), 
	      format("variance mismatch %.20g - %.20g = %.20g; %d iters\n"
		     "(n,mean^2,sumsq): a=(%d,%.20g,%.20g); b=(%d,%.20g,%.20g)\n"
		     "a-b=(%d,%.20g,%.20g) ;; %.20g - %.20g = %.20g") 
	      % a.variance() % b.variance() % (a.variance() - b.variance()) % iters 
	      % a.count() % (a.mean() * a.mean()) % a.total_sq()
	      % b.count() % (b.mean() * b.mean()) % b.total_sq()
	      % (a.count() - b.count()) % (a.mean() * a.mean() - b.mean() * b.mean())
	      % (a.total_sq() - b.total_sq())
	      % var_a % var_b % (var_a - var_b));
}

void checkInRange(unsigned long count, double min, double max) {
    INVARIANT(count >= min && count <= max, "range error");
}

void testStatsMerging() {
    Stats base_1, base_2, merge;

    uint32_t n_random_values = 1000000;

    MersenneTwisterRandom rand; // Let it seed differently each time

    if (getenv("STATS_SEED") != NULL) {
	rand.init(stringToInteger<uint32_t>(getenv("STATS_SEED")));
    }

    cout << format("Seeded merge test with %d\n") % rand.seedUsed();

    for(uint32_t i=0; i < n_random_values; ++i) {
	double rv = rand.randDoubleOpen53();
	uint32_t which_base = rand.randInt(2);
	Stats test;
	merge.add(rv);
	if (which_base) {
	    base_1.add(rv);
	    test.add(base_1);
	    checkEqual(test, base_1, i);
	    test.add(base_2);
	    checkEqual(test, merge, i);
	} else {
	    base_2.add(rv);
	    test.add(base_2);
	    checkEqual(test, base_2, i);
	    test.add(base_1);
	    checkEqual(test, merge, i);
	}
    }

    // TODO: work out what proper statistics would be.
    checkInRange(base_1.count(), n_random_values * 0.4, n_random_values * 0.6);
    checkInRange(base_2.count(), n_random_values * 0.4, n_random_values * 0.6);
}

int main(int, char **) {
    // TODO: more tests
    testStatsMerging();
    return 0;
}
