/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/


#include <algorithm>
#if defined(__HP_aCC) && __HP_aCC < 35000
#include <strstream.h>
#else
#include <sstream>
#endif

#include <algorithm>
#include <ostream>

#include <boost/format.hpp>

#include <Lintel/Double.H>
#include <Lintel/LintelAssert.H>
#include <Lintel/MersenneTwisterRandom.H>
#include <Lintel/StatsQuantile.H>

using namespace std;

void
checkQuantiles(StatsQuantile &stats,
	       vector<double> &sorted_list,
	       Stats &exact_error,
	       double epsilon)
{
#if STATSQUANTILE_TIMING
    stats.accum_gq_all = 0;
    stats.accum_gq_init = 0;
    stats.accum_gq_search = 0;
    stats.accum_gq_inner = 0;
    stats.accum_gq_nelem = 0;
    Clock::T accum_error = 0;
#endif
    int position_error = (int)ceil(epsilon * stats.count());
    for(int i=0;i<(int)stats.count();i++) {
	if ((i % 5000) == 0) {
	    if ((i % 50000) == 0) {
		printf("!");
	    } else {
		printf("."); 
	    }
	    fflush(stdout);
	}
#if STATSQUANTILE_TIMING
	if ((i % 10000) == 0) {
	    printf(" nelem %d all %.6g init %.6g search %.6g inner %.6g; error %.6g\n",
		   stats.accum_gq_nelem / 10000,
		   stats.accum_gq_all / 10000.0,
		   stats.accum_gq_init / 10000.0,
		   stats.accum_gq_search / 10000.0,
		   stats.accum_gq_inner / 10000.0,
		   accum_error / 10000.0);
	    stats.accum_gq_nelem = 0;
	    stats.accum_gq_all = 0;
	    stats.accum_gq_init = 0;
	    stats.accum_gq_search = 0;
	    stats.accum_gq_inner = 0;
	    accum_error = 0;
	}
#endif
	double phi = (double)i / (double)stats.count();
	double v = stats.getQuantile(phi);
#if STATSQUANTILE_TIMING
	Clock::T clock_0 = Clock::now();
#endif
	int match_error,min,max;
	min = i - position_error; 
	if (min < 0) min = 0;
	max = i + position_error + 1;
	if (max > (int)stats.count()) max = (int)stats.count();
	for(match_error=0;match_error<=position_error;match_error++) {
	    if ((i-match_error)>=0 && sorted_list[i-match_error] == v) { 
		break;
	    }
	    if ((i+match_error)<(int)sorted_list.size() && 
		sorted_list[i+match_error] == v) {
		break;
	    }
	}
#if STATSQUANTILE_TIMING
	Clock::T clock_1 = Clock::now();
	accum_error += clock_1 - clock_0;
#endif
	AssertAlways(match_error <= position_error,
		     ("checkQuantiles error!\n"));
	exact_error.add(match_error);
    }
}

void 
test_fromrandom(MersenneTwisterRandom &mt, 
		double epsilon, int nelems, 
		Stats &exact_error)
{
    StatsQuantile test(epsilon,nelems);
    printf("adding-entries... ");fflush(stdout);
    vector<double> exact_list;
    exact_list.reserve(nelems);
    for(int i = 0;i<nelems;i++) {
	double v = mt.randDoubleOpen53();
	exact_list.push_back(v);
	test.add(v);
    }
    printf("sorting... ");fflush(stdout);
    sort(exact_list.begin(), exact_list.end());
	
    printf("checking...");fflush(stdout);
    checkQuantiles(test, exact_list, exact_error, epsilon);
    printf("done.\n");
}

void 
test_merge(MersenneTwisterRandom &mt, 
	   double epsilon, int nelems, uint32_t nreps,
	   Stats &exact_error)
{
    cout << "test merge: select random...\n"; cout.flush();
    vector<double> random_list;
    random_list.reserve(nelems);
    for(int i = 0;i<nelems;i++) {
	double v = mt.randDoubleOpen53();
	random_list.push_back(v);
    }
    vector<double> sorted_list = random_list;
    sort(sorted_list.begin(), sorted_list.end());

    for(uint32_t rep = 1; rep <= nreps; ++rep) {
	cout << boost::format("  test round %d: loading... ") % rep;
	
	StatsQuantile test1(epsilon, nelems), test2(epsilon, nelems);
	for(int i=0; i < nelems; ++i) {
	    if (mt.randInt(2)) {
		test1.add(random_list[i]);
	    } else {
		test2.add(random_list[i]);
	    }
	}

	cout << "merging... "; cout.flush();
	StatsQuantile merge;
	if (mt.randInt(2)) {
	    merge.add(test1);
	    // TODO: equality test here for merge and test1?
	    merge.add(test2);
	} else {
	    merge.add(test2);
	    merge.add(test1);
	}
	cout << "checking... "; cout.flush();

	Stats exact_error;
	checkQuantiles(merge, sorted_list, exact_error, epsilon);
	cout << "ok\n";
    }
}

double eNbk_table[][4] = {
    // Table entries from the paper:
    { 0.100, 1e5,    5,   55 },
    { 0.100, 1e6,    7,   54 },
    { 0.100, 1e7,   10,   60 },
    { 0.100, 1e8,   15,   51 },
    { 0.100, 1e9,   12,   77 },

    { 0.050, 1e5,    6,   78 },
    { 0.050, 1e6,    6,  117 },
    { 0.050, 1e7,    8,  129 },
    { 0.050, 1e8,    7,  211 },
    { 0.050, 1e9,    8,  235 },

    { 0.010, 1e5,    7,  217 },
    { 0.010, 1e6,   12,  229 },
    { 0.010, 1e7,    9,  412 },
    { 0.010, 1e8,   10,  596 },
    { 0.010, 1e9,   10,  765 },

    { 0.005, 1e5,    3,  953 },
    { 0.005, 1e6,    8,  583 },
    { 0.005, 1e7,    8,  875 },
    { 0.005, 1e8,    8, 1290 },
    { 0.005, 1e9,    7, 2106 },

    { 0.001, 1e5,    3, 2778 },
    { 0.001, 1e6,    5, 3031 },
    { 0.001, 1e7,    5, 5495 },
    { 0.001, 1e8,    9, 4114 },
    { 0.001, 1e9,   10, 5954 },

    // Extra Eric-calculated test cases for very high precision estimates,
    // mainly serve to cross-check different implementations
    { 0.0001, 1e5, 2, 16667 },
    { 0.0001, 1e6, 3, 27778 },
    { 0.0001, 1e7, 5, 30304 },
    { 0.0001, 1e8, 12, 22894 },
    { 0.0001, 1e9, 9, 41136 },

    { 0.00005, 1e5, 2, 25000 },
    { 0.00005, 1e6, 3, 47620 },
    { 0.00005, 1e7, 3, 95239 },
    { 0.00005, 1e8, 6, 77701 },
    { 0.00005, 1e9, 8, 87413 },

    { 0.0001, 1e8,  12, 22894 }, // 2.096 MiB
    { 0.0001, 1e9,   9, 41136 }, // 2.825 MiB
    { 0.0001, 1e10, 10, 59538 }, // 4.542 MiB
    { 0.0001, 1e11, 10, 76482 }, // 5.835 MiB
    { 0.0001, 1e12, 12, 76700 }, // 7.022 MiB
    { 0.0001, 1e13, 14, 83501 }, // 8.919 MiB

    { 0.001,  1e8,   9, 4114 }, // 0.282 MiB
    { 0.001,  1e9,  10, 5954 }, // 0.454 MiB
    { 0.001,  1e10, 15, 5099 }, // 0.584 MiB
    { 0.001,  1e11, 12, 7670 }, // 0.702 MiB
    { 0.001,  1e12, 17, 6877 }, // 0.892 MiB
    { 0.001,  1e13, 17, 8571 }, // 1.112 MiB

    { 0.01,   1e8,  10, 596 },  // 0.045 MiB
    { 0.01,   1e9,  10, 765 },  // 0.058 MiB
    { 0.01,   1e10, 12, 767 },  // 0.070 MiB
    { 0.01,   1e11, 17, 688 },  // 0.089 MiB
    { 0.01,   1e12, 17, 858 },  // 0.111 MiB
    { 0.01,   1e13, 16, 1068 }, // 0.130 MiB

    { 0.0, 0.0, 0.0, 0.0 }
};

void
checkRomePrint()
{
    StatsQuantile *s = new StatsQuantile(0.0001, 20000000, 10);

    double data[] = {
	10, 15, 20, 25, 10, 25, 10, 25, 30, 10,
    };
    int nr_data = 10;

    ostringstream buf1;
    for (int i = 0; i < nr_data; i++)
	s->add(data[i]);
    s->printRome(0, buf1);
    string str1 = buf1.str();
    
    ostringstream buf2;
    s->reset();
    for (int i = 0; i < nr_data; i++)
	s->add(data[i]);
    s->printRome(0, buf2);
    string str2 = buf2.str();
    if (str1 != str2) {
	cout << " output mismatch: str1=" << str1 << endl;
	cout << " str1=" << str2 << endl;
	AssertFatal(("abort"));
    }
}

int
main(int argc, char *argv[])
{
    Double::selfCheck(); // quantile was failing because of problems checked now in here
    if (argc == 2 && strcmp(argv[1],"long") == 0) {
	printf("Running long (10b entry) test (count to 100)...\n");
	fflush(stdout);
	StatsQuantile test(0.001,(long long)1.1e10);
	for(double outer = 0;outer <= 100;++outer) {
	    if (outer == 5) { // early intermediate check 
		printf("\nIntermediate check quantiles (count to 100):\n"); 
		double maxerror = 5*1e8*0.001;
		for(double q = 0;q < 100;++q) {
		    double v= test.getQuantile(q/100.0);
		    double expect = 1e10*5.0/100.0*q/100.0;
		    double error = fabs(v-expect);
		    printf("  %.0f: expect=%.0f got=%.0f error=%.0f\n",q,expect,v,error);
		    AssertAlways(error < maxerror,("maxerror exceeded.\n"));
		}
		printf("all ok, continuing..."); fflush(stdout);
	    }
	    if (outer == 30) { // intermediate check at 3billion to catch any integers that wrap
		printf("\nIntermediate check quantiles (count to 100):\n");
		double maxerror = 30*1e8*0.001;
		for(double q = 0;q <= 100;++q) {
		    double v= test.getQuantile(q/100.0);
		    double expect = 1e10*30.0/100.0*q/100.0;
		    double error = fabs(v-expect);
		    printf("  %.0f: expect=%.0f got=%.0f error=%.0f\n",q,expect,v,error);
		    AssertAlways(error < maxerror,("maxerror exceeded.\n"));
		}
		printf("all ok, continuing..."); fflush(stdout);
	    }
	    for(double i=outer*1e8;i<(outer+1)*1e8;++i) {
		test.add(i);
	    }
	    printf("%.0f,",outer);
	    fflush(stdout);
	}
	printf("\nCheck quantiles (count to 100):\n");
	double maxerror = 1e10*0.001;
	for(double q = 0;q <= 100;++q) {
	    double v= test.getQuantile(q/100.0);
	    double expect = 1e10*100.0/100.0*q/100.0;
	    double error = fabs(v-expect);
	    printf("  %.0f: expect=%.0f got=%.0f error=%.0f\n",q,expect,v,error);
	    AssertAlways(error < maxerror,("maxerror exceeded.\n"));
	}
	printf("**** LONG TEST SUCCESSFUL\n");
    }	

    if (true) {
	// Make sure that the stats object doesn't choke with no sample.
	StatsQuantile test("",3,10);
	AssertAlways(test.count() == 0, ("Count != 0"));
	AssertAlways(test.total() == 0, ("Total != 0"));
	AssertAlways(isnan(test.getQuantile(0.0)),
		     ("Failed to get quantile, got %.2f",
		      test.getQuantile(0.0)));
	AssertAlways(isnan(test.getQuantile(0.99)), 
		     ("Failed to get quantile, got %.2f",
		      test.getQuantile(0.99)));

	// How about one sample??
	test.add(99.9);
	AssertAlways(test.count() == 1, ("Count != 0"));
	AssertAlways(test.total() == 99.9, ("Total != 0"));
	AssertAlways(test.getQuantile(0.0) == 99.9, ("Failed to get quantile"));
	AssertAlways(test.getQuantile(0.99) == 99.9, ("Failed to get quantile"));
    }

    if (true) {
	for(int i=0;eNbk_table[i][0] > 0;i++) {
	    StatsQuantile foo(eNbk_table[i][0],(long long)(eNbk_table[i][1]));

	    if (eNbk_table[i][2] == 0) {
		printf("%.7g %.5g -> %d %d = %.3f MiB\n",
		       eNbk_table[i][0],eNbk_table[i][1],
		       foo.getNBuffers(),foo.getBufferSize(),
		       foo.getNBuffers() * foo.getBufferSize() * 8.0 / (1024.0*1024.0)
		       );
		continue;
	    }
	    AssertAlways(foo.getNBuffers() == eNbk_table[i][2],
			 ("mismatch on nbuffers for %.4g %.4g, %.4g != %.4g\n",
			  eNbk_table[i][0],eNbk_table[i][1],
			  (double)foo.getNBuffers(), eNbk_table[i][2]));
	    AssertAlways(foo.getBufferSize() == eNbk_table[i][3],
			 ("mismatch on buffer size for %.4g %.4g, %.4g != %.4g\n",
			  eNbk_table[i][0],eNbk_table[i][1],
			  (double)foo.getBufferSize(), eNbk_table[i][3]));
	}
	printf("Size checking passed.\n"); 
    }
    if (true) { 
	StatsQuantile test1("",8,1100); // 8 x 1290

	for(int i=1000;i<2000;i++) {
	    test1.add(i);
	}
	for(int i=0;i<1000;i++) {
	    double v = test1.getQuantile((double)i/1000.0);
	    AssertAlways(v == i + 1000,
			 ("mismatch %.4g %.4g\n",v,(double)i+1000.0));
	}
	printf("Simple-1 (add sequential) quantile checking passed.\n");
	
	for(int i=2000;i<3000;i++) {
	    test1.add(i);
	}
	for(int i=0;i<2000;i++) {
	    double v = test1.getQuantile((double)i/2000.0);
	    AssertAlways(v == i + 1000,
			 ("mismatch %.4g %.4g\n",v,(double)i+1000.0));
	}
	printf("Simple-2 (more sequential) quantile checking passed.\n");
	for(int i=4999;i>=3000;i--) {
	    test1.add(i);
	}
	for(int i=0;i<4000;i++) {
	    double v = test1.getQuantile((double)i/4000.0);
	    AssertAlways(v == i + 1000,
			 ("mismatch %.4g %.4g\n",v,(double)i+1000.0));
	}
	printf("Simple-3 (reverse sequential) quantile checking passed.\n");
	for(int i=5000;i<8000;i+=2) {
	    test1.add(i);
	}
	for(int i=5001;i<8000;i+=2) {
	    test1.add(i);
	}
	for(int i=0;i<7000;i++) {
	    double v = test1.getQuantile((double)i/7000.0);
	    AssertAlways(v == i + 1000,
			 ("mismatch %.4g %.4g\n",v,(double)i+1000.0));
	}
	printf("Simple-4 (skip sequential) quantile checking passed.\n");
    }
    if (true) {
	StatsQuantile test2("",3,10);
	for(int i=0;i<30;i++) {
	    test2.add(i);
	}
	for(int i=0;i<30;i++) {
	    double v = test2.getQuantile((double)i/30.0);
	    AssertAlways(v == i,("mismatch %.4g %.4g\n",v,(double)i));
	}
	// this add will cause a collapse to occur on the first three
	// buckets, the first buffer should now contain 
	// 1,4,7,10,13,16,19,22,25,28; and the second 30
	test2.add(30);
	for(int i=0;i<31;i++) {
	    double v = test2.getQuantile((double)i/31.0);
	    double expect_v = 3 * (i / 3) + 1;
	    if (i == 30) expect_v = 30;
	    AssertAlways(v == expect_v,("mismatch %.4g %.4g\n",v,expect_v));
	}
	for(int i=31;i<50;i++) {
	    test2.add(i);
	}
	// should have all three buckets full again; the first at weight 3
	// the second two at weight 1
	for(int i=0;i<50;i++) {
	    double v = test2.getQuantile((double)i/50.0);
	    double expect_v = 3 * (i / 3) + 1;
	    if (i >= 30) expect_v = i;
	    AssertAlways(v == expect_v,("mismatch %.4g %.4g\n",v,expect_v));
	}
	// now collapse the second two buckets
	test2.add(50);
	// the second bucket should now contain
	// 30,32,34,36,38,40,42,44,46,48 because we use collapse_even_low
	// to start, the third bucket has 50
	for(int i=0;i<51;i++) {
	    double v = test2.getQuantile((double)i/51.0);
	    double expect_v = 3 * (i / 3) + 1;
	    if (i >= 30) expect_v = 2 * (i / 2);
	    if (i >= 50) expect_v = i;
	    AssertAlways(v == expect_v,("mismatch %.4g %.4g\n",v,expect_v));
	}
	for(int i=51;i<60;i++) {
	    test2.add(i);
	}
	// all three buckets should be full again, at weights 3, 2, 1
	for(int i=0;i<60;i++) {
	    double v = test2.getQuantile((double)i/60.0);
	    double expect_v = 3 * (i / 3) + 1;
	    if (i >= 30) expect_v = 2 * (i / 2);
	    if (i >= 50) expect_v = i;
	    AssertAlways(v == expect_v,("mismatch %.4g %.4g\n",v,expect_v));
	}
	// now collapse all three buckets
	test2.add(60);
	// first bucket should now contain 4,10,16,22,28,32,38,44,51,57
	// at weight 6; second bucket should have 60; note that the first
	// eight entries in the first bucket are off by +-1, but some error 
	// is expected as these are approximate bounds
	for(int i=0;i<61;i++) {
	    double v = test2.getQuantile((double)i/61.0);
	    double expect_v = 6 * (i / 6) + 4;
	    if (i >= 30) expect_v = 6 * (i / 6) + 2;
	    if (i >= 48) expect_v = 6 * (i / 6) + 3;
	    if (i >= 60) expect_v = i;
	    AssertAlways(v == expect_v,("mismatch #%d %.4g %.4g\n",
					i,v,expect_v));
	}
	printf("Collapse-1 exact checking passed.\n");
    }
    if (true) {
	MersenneTwisterRandom mt; // actually want to seed randomly, so we
	// check things slightly differently each time!  What we're really
	// testing is that the estimates are within the bounds 
	printf("Large-Approx checking, 0.01 x 250000 pure-random:\n  ");
	Stats exact_error;
	test_fromrandom(mt,0.01,250000,exact_error);
    }
    if (true) {
	MersenneTwisterRandom mt(1776); // seed exact for precice check

	printf("Large-Approx checking, 0.05 x 150000 seeded-random:\n  ");
	Stats exact_error;
	test_fromrandom(mt,0.05,150000,exact_error);
	printf("exact error: %.4g +- %.4g, [%.4g .. %.4g]\n",
	       exact_error.mean(), exact_error.stddev(),
	       exact_error.min(), exact_error.max());
    }
    if (true) {
	MersenneTwisterRandom mt(2010); // seed exact for precice check

	printf("Large-Approx checking, 0.001 x 50000 seeded-random:\n  ");
	Stats exact_error;
	test_fromrandom(mt,0.001,50000,exact_error);
	printf("exact error: %.4g +- %.4g, [%.4g .. %.4g]\n",
	       exact_error.mean(), exact_error.stddev(),
	       exact_error.min(), exact_error.max());
    }
    if (true) {
	MersenneTwisterRandom mt(3838278); // seed exact for precice check
	
	printf("Large-Approx checking, 0.005 x 75000 random, sorted:\n  ");
	printf("generating-entries... ");fflush(stdout);
	vector<double> exact_list;
	exact_list.reserve(75000);
	for(int i = 0;i<75000;i++) {
	    double v = mt.randDoubleOpen53();
	    exact_list.push_back(v);
	}
	printf("sorting... ");fflush(stdout);
	sort(exact_list.begin(), exact_list.end());
	
	StatsQuantile test(0.005,75000);
	printf("adding entries... "); fflush(stdout);
	for(int i = 0;i<75000;i++) {
	    test.add(exact_list[i]);
	}
	printf("checking...");fflush(stdout);
	Stats exact_error;
	checkQuantiles(test,exact_list,exact_error,0.005);
	printf("done.\n");
	printf("exact error: %.4g +- %.4g, [%.4g .. %.4g]\n",
	       exact_error.mean(), exact_error.stddev(),
	       exact_error.min(), exact_error.max());
	test.dumpState();
    }	
    if (true) {
	MersenneTwisterRandom mt; // actually want to seed randomly, so we
	// check things slightly differently each time!  What we're really
	// testing is that the estimates are within the bounds 
	printf("Merge checking  checking, 0.01 x 250000 pure-random:\n  ");
	Stats exact_error;
	test_merge(mt,0.01,250000,5,exact_error);
    }
}
