/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/


#include <boost/format.hpp>
#include <Lintel/AssertBoost.hpp>
#include <Lintel/Double.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/Stats.hpp>
#include <Lintel/ProgramOptions.hpp>

using boost::format;

lintel::ProgramOption<int> po_extra_reps
("extra-reps", 
 "number of extra repetitions of testStatsMerging() to call", 
 1);

void
checkEqual(Stats &a, Stats &b)
{
    INVARIANT(a.count() == b.count(), 
	      format("count mismatch %d!=%d") % a.count() % b.count());
    INVARIANT(a.min() == b.min(), 
	      format("min mismatch %f!=%f") % a.min() % b.min());
    INVARIANT(a.max() == b.max(), 
	      format("max mismatch %f!=%f") % a.max() % b.max());
    INVARIANT(Double::eq(a.mean(), b.mean()), 
	      format("mean mismatch %f!=%f") % a.mean() % b.mean());
    INVARIANT(Double::eq(a.variance(), b.variance()), 
	      format("variance mismatch %f!=%f, difference is %f")
	      % a.variance() % b.variance() % (a.variance() - b.variance()));
}

void
checkInRange(unsigned long count, double min, double max)
{
    INVARIANT(count >= min && count <= max, "range error");
}

void
testStatsMerging()
{
    Stats base_1, base_2, merge;

    uint32_t n_random_values = 1000000;

    MersenneTwisterRandom rand; // Let it seed differently each time

    for(uint32_t i=0; i < n_random_values; ++i) {
	double rv = rand.randDoubleOpen53();
	uint32_t which_base = rand.randInt(2);
	Stats test;
	merge.add(rv);
	if (which_base) {
	    base_1.add(rv);
	    test.add(base_1);
	    checkEqual(test, base_1);
	    test.add(base_2);
	    checkEqual(test, merge);
	} else {
	    base_2.add(rv);
	    test.add(base_2);
	    checkEqual(test, base_2);
	    test.add(base_1);
	    checkEqual(test, merge);
	}
    }

    // TODO: work out what proper statistics would be.
    checkInRange(base_1.count(), n_random_values * 0.4, n_random_values * 0.6);
    checkInRange(base_2.count(), n_random_values * 0.4, n_random_values * 0.6);
}

int
main(int argc, char *argv[])
{
    lintel::parseCommandLine(argc, argv);
    
    for(int i = 0; i<po_extra_reps.get(); ++i) {
	// TODO: more tests
	testStatsMerging();
    }
    return 0;
}
