/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Cycle counter and other timer support
*/

#include <iostream>

using namespace std;

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>

#include <algorithm>
#include <vector>

#include <Lintel/Clock.hpp>
#include <Lintel/PThread.hpp>
#include <Lintel/Stats.hpp>
#include <Lintel/Double.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>

using boost::format;

const int min_samples = 20;
// Use -Double::Inf so things blow up suitably if these get used.
double Clock::clock_rate = -Double::Inf;
double Clock::inverse_clock_rate = -Double::Inf;
double Clock::inverse_clock_rate_tfrac = -Double::Inf;
Clock::Tll Clock::max_recalibrate_measure_time = 10000000000LL;
Stats Clock::calibrate;

static Clock::AllowUnsafeFreqScalingOpt allow_unsafe_frequency_scaling 
    = Clock::AUFSO_No;
static bool may_have_frequency_scaling = true; 
class Clock_Tll_Order {
public:
    inline bool operator() (Clock::Tll a, Clock::Tll b) {
	return a < b;
    }
};

static string readFileLine(const string &filename)
{
    // After much searching on the web, I can't establish that I can
    // get errno after opening an ifstream.  Most examples just assume
    // opens succeed, a few check file.good() and just say they can't
    // open the file.

    FILE *f = fopen(filename.c_str(), "r");
    if (f == NULL) {
	return "";
    }
    static const unsigned bufsize = 80; // long enough for how we are using it.
    char buf[bufsize]; 
    fgets(buf, bufsize, f);
    unsigned len = strlen(buf);
    SINVARIANT(len > 0 && buf[len-1] == '\n');
    buf[len-1] = '\0';
    return string(buf);
}

static void checkForPossibleFrequencyScaling()
{
#ifdef __linux__
	const string cpu_dir("/sys/devices/system/cpu");
	DIR *dir = opendir(cpu_dir.c_str());
	INVARIANT(dir != NULL, 
		  format("can't opendir %s(%s), so can't check for frequency scaling")
		  % cpu_dir % strerror(errno));
	bool all_ok = true;
	unsigned cpu_count = 0;
	struct dirent *ent;
	while((ent = readdir(dir)) != NULL) {
	    if (strncmp(ent->d_name, "cpu", 3) == 0 
		&& isdigit(ent->d_name[3])) {
		++cpu_count;
		string dir = cpu_dir + "/" + ent->d_name + "/cpufreq";
		string min_freq = readFileLine(dir + "/scaling_min_freq");
		string max_freq = readFileLine(dir + "/scaling_max_freq");
		string governor = readFileLine(dir + "/scaling_governor");
		if (governor == "performance" 
		    || (!min_freq.empty() && min_freq == max_freq)) {
		    // ok
		} else {
		    all_ok = false;
		    break;
		}
	    }
	}
	INVARIANT(cpu_count > 0, "can't check frequency scaling, no cpus found?");
	if (all_ok) {
	    may_have_frequency_scaling = false;
	}
#else
	may_have_frequency_scaling = true;
#endif

    if (!may_have_frequency_scaling) {
	// great.
    } else if (allow_unsafe_frequency_scaling == Clock::AUFSO_WarnFast ||
	       allow_unsafe_frequency_scaling == Clock::AUFSO_WarnSlow) {
	cerr << "Warning, frequency scaling may be enabled on at least one cpu.\n"
	     << "This program is using Clock::todcc*, and may therefore get\n";
	if (allow_unsafe_frequency_scaling == Clock::AUFSO_WarnFast) {
	    cerr << "incorrect times back." << endl;
	} else {
	    cerr << "slower than expected time operations." << endl;
	}
    } else if (allow_unsafe_frequency_scaling == Clock::AUFSO_No) {
	FATAL_ERROR("frequency scaling enabled on at least one cpu; configuration disallows use of Clock::todcc()");
    }
}

void Clock::allowUnsafeFrequencyScaling(AllowUnsafeFreqScalingOpt allow)
{
    allow_unsafe_frequency_scaling = allow;
}

void
Clock::calibrateClock(bool print_calibration_information, 
		      double max_conf95_rel,
		      int print_calibration_warnings_after_tries)
{
    Tdbl clock_s, clock_e;
    Tll tick_s, tick_e;

    if (allow_unsafe_frequency_scaling != AUFSO_Yes) {
	checkForPossibleFrequencyScaling();
    }

    double tmp_clock_rate;
    if (clock_rate == -Double::Inf) {
	bindToProcessor();
	for(int tries=0;tries<10;++tries) {
	    calibrate.reset();
	    tmp_clock_rate = -1;
	    for(int ndelay=1;true;ndelay+=1) {
		// De-schedule ourselves for a bit, to lower the
		// chance of getting pre-empted while running the
		// calibration
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		select(0,NULL,NULL,NULL,&timeout);

		clock_s = Clock::tod();
		tick_s = now();
		for(volatile int d=0;d<ndelay*10000;d++) {
		    /* empty */
		}
		clock_e = Clock::tod();
		tick_e = now();

		Tll tick_d = tick_e - tick_s;
		Tdbl us_d = clock_e - clock_s;

		if (us_d < 500) {
		    ndelay += 20;
		    continue;
		}
		if (us_d > 100000) {
		    if (tries >= print_calibration_warnings_after_tries) {
			fprintf(stderr,"WHOA, calibration didn't succeed after %ld samples; restarting\n",
				calibrate.count());
		    }
		    tmp_clock_rate = -1;
		    break;
		}

		double t = (double)tick_d / us_d;
		calibrate.add(t);
		if ((int)calibrate.count() < min_samples)
		    continue;
		if (print_calibration_information || tries >= print_calibration_warnings_after_tries) {
		    fprintf(stderr, "Calibrating clock rate, try %d: ndelay=%ld us=%.5g ticks=%lld clock est %.6g;; clock mean %.10g, conf95 %.10g\n",
			    tries, (long)ndelay, us_d, tick_d, t,calibrate.mean(),calibrate.conf95());
		}
		if (calibrate.conf95() < calibrate.mean() * max_conf95_rel) {
		    tmp_clock_rate = calibrate.mean();
		    INVARIANT(clock_rate == -Double::Inf,
			      format("whoa, two threads calibrating at the same time?! %g != %g")
			      % clock_rate % (-Double::Inf));
		    clock_rate = tmp_clock_rate;
		    INVARIANT(inverse_clock_rate == -Double::Inf,
			      "error: two threads trying to calibrate at once ?!");
		    inverse_clock_rate = 1.0 / clock_rate;

		    // cycle_counter * icr = time in us * 1/1e6 = time
		    // in seconds * 2**32 = time in Tfrac
		    inverse_clock_rate_tfrac = inverse_clock_rate * (4294967296.0/1000000.0);
		    std::vector<Tll> elapsed;
		    const int nelapsed = 100;
		    for(int i=0;i<nelapsed;i++) {
			Tll start_cycle = now();
			tod();
			Tll end_cycle = now();
			Tll delta = end_cycle - start_cycle;
			elapsed.push_back(delta);
		    }
		    std::sort(elapsed.begin(),elapsed.end(),Clock_Tll_Order());
		    int off = nelapsed/2; // median
		    SINVARIANT((int)elapsed.size() > off &&
			      elapsed[off] < 10 * clock_rate);

		    // 2 * gives a little leeway to when it's running in 
		    // practice
		    // changed to 5 because we were having too many bad 
		    // recalibates at high load.
		    max_recalibrate_measure_time = 5 * elapsed[off];
		    // increased the bound to 30us
		    // (30*tmp_clock_rate); under load, this invariant
		    // could fire; might want to warn, on a really
		    // slow recalibrate, we could introduce a lot of
		    // error.
		    INVARIANT(max_recalibrate_measure_time < 30 * tmp_clock_rate,
			      format("Internal sanity check failed, tod() takes too long, %d > %.2f\n")
			      % max_recalibrate_measure_time
			      % (20 * tmp_clock_rate));
		    break;
		}
	    }
	    if (tmp_clock_rate > 200) {
		break;
	    }
	}
	INVARIANT(tmp_clock_rate > 200, format("Unable to calibrate clock rate to %.8g relative error") % max_conf95_rel);
	unbindFromProcessor();
	if (print_calibration_information) {
	    cout << format("Final calibrated clock rate is %.8g, conf95 %.8g, %ld samples\n") % tmp_clock_rate % calibrate.conf95() % calibrate.count();
	}
    }
}

Clock::Clock(bool allow_calibration)
    : nrecalibrate(0), nbadgettod(0),
      last_calibrate_tod(0), cycle_count_offset(0), recalibrate_interval(0),
      last_calibrate_tod_tfrac(0),
      last_cc(0), last_recalibrate_cc(0), recalibrate_interval_cycles(0),
      epoch_offset(0)
{
    if (clock_rate == -1) {
	INVARIANT(allow_calibration,
		  "you failed to call Clock::calibrateClock()!");
	calibrateClock();
	SINVARIANT(clock_rate > 0);
    }
    setRecalibrateInterval(500);
}

void
Clock::bindToProcessor(unsigned proc)
{
#ifdef HPUX_ACC
    IVNARIANT(proc < pthread_num_processors_np(), "Invalid processor #");
    pthread_spu_t answer;
    int ret = pthread_processor_id_np(PTHREAD_GETFIRSTSPU_NP,&answer,0);
    SINVARIANT(ret==0);
    for (unsigned i = 0; i < proc; i++) {
	ret = pthread_processor_id_np(PTHREAD_GETNEXTSPU_NP, &answer, answer);
	SINVARIANT(ret==0);
    }
    ret = pthread_processor_bind_np(PTHREAD_BIND_FORCED_NP, &answer,
				    answer, PTHREAD_SELFTID_NP);
    SINVARIANT(ret==0);
#endif
}

void
Clock::unbindFromProcessor()
{
#ifdef HPUX_ACC
    pthread_spu_t answer;
    int ret = pthread_processor_bind_np(PTHREAD_BIND_ADVISORY_NP, &answer,
					PTHREAD_SPUFLOAT_NP, PTHREAD_SELFTID_NP);
    SINVARIANT(ret==0);
#endif
}

Clock::Tdbl
Clock::todcc_recalibrate()
{
    if (may_have_frequency_scaling) {
	if (allow_unsafe_frequency_scaling == AUFSO_WarnSlow ||
	    allow_unsafe_frequency_scaling == AUFSO_Slow) {
	    return tod();
	} else {
	    SINVARIANT(allow_unsafe_frequency_scaling == AUFSO_Yes ||
		       allow_unsafe_frequency_scaling == AUFSO_WarnFast);
	}
    }
    INVARIANT(clock_rate > 0,
	      "You do not appear to have setup a Clock object; how did you get here?");

    // TODO: consider measuring how long a tod_epoch usually takes and
    // then moving us forward until we cross a boundary, without that
    // we get a weird wobble every recalibration interval --
    // alternately could check to see if the time is still consistent
    // with the last one, and if so, just adjust the offsets,
    // e.g. estimate where in the cur_us we actually are.

    // TODO: also consider setting for sooner re-calibration if we are
    // above some low water mark for how long the gettod takes, for
    // example, above the 10% rate from calibrateClock()

    uint64_t start_cycle = now();
    Clock::Tdbl cur_us = tod_epoch();
    uint64_t end_cycle = now();
    INVARIANT(cur_us >= last_calibrate_tod,
	      format("Whoa, tod_epoch() went backwards %.2f < %.2f")
	      % cur_us % last_calibrate_tod);
    if ((end_cycle < start_cycle) || (end_cycle - start_cycle) 
	> static_cast<uint64_t>(max_recalibrate_measure_time)) {
#if 0
	fprintf(stderr,"Whoa, bad todcc_recalibrate getting %lld took %lld to %lld = %lld, or %.3g us\n",cur_us, start_cycle, end_cycle, end_cycle - start_cycle, est_todus);
	if (end_cycle == start_cycle) {
	    ntodzerotime++;
	}
	badrecalibration_delta.add(last_recalibrate_cc - start_cycle - max_recalibrate_measure_time);
#endif
	++nbadgettod;
	// next two lines guarantee failure next time assuming the cycle
	// counter isn't reset and booting takes a little time
	cycle_count_offset = -Double::Inf; 
	last_recalibrate_cc = 0; 
    } else {
	double est_us = end_cycle * inverse_clock_rate;
	cycle_count_offset = cur_us - est_us;
	last_calibrate_tod = cur_us;
	last_calibrate_tod_tfrac = secondsToTfrac(last_calibrate_tod*1e-6);
	last_recalibrate_cc = end_cycle;
	++nrecalibrate;
    }
    
    return cur_us;
}

void
Clock::setRecalibrateInterval(Tdbl interval_us)
{
    last_cc = 0;
    recalibrate_interval = interval_us;
    // This check is needed so that the (int) cast in
    // todcc_incremental is correct; and the (int) cast
    // results in a reasonable speedup 1<< 30 is conservative.
    INVARIANT(recalibrate_interval * clock_rate < (double)(1 << 30),
	      format("using too large a recalibration interval on too fast a machine %.2f, %.2f")
	      % recalibrate_interval % clock_rate);
    recalibrate_interval_cycles = (Tll)(recalibrate_interval  * clock_rate);
}

void 
Clock::setTodccEpoch(unsigned seconds)
{
    epoch_offset = seconds;
    
    INVARIANT(Clock::tod() > static_cast<Tdbl>(epoch_offset) * 1.0e6,
	      "Specified epoch is in the future; todcc_recalibrate will fail");

    last_calibrate_tod = 0; // time needs to go forward
    todcc_recalibrate();
}

void
Clock::timingTest()
{
    Clock::Tdbl measurement_time = 1000000;
    Clock::calibrateClock(true,0.001);
    Clock myclock;
    myclock.setRecalibrateInterval(100);

    printf("timing method           reps    elapsed us    ns/fn     cycles/fn  ratio to tod\n");
    Clock::Tdbl start = Clock::tod();
    unsigned nreps = 0;
    while(1) {
	nreps += 1;
	if ((Clock::tod() - start) > measurement_time) {
	    break;
	}
    }
    Clock::Tdbl end = Clock::tod();
    unsigned clock_reps = nreps;
    double ns_per_tod = 1000.0*(end - start)/(double)clock_reps;
    printf("tod()                 %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	   clock_reps, end - start, ns_per_tod, ns_per_tod / (1000.0*inverse_clock_rate),1.0);

    // Test Clock::todll()
    {
	Clock::Tll measurement_time_ll = (Clock::Tll)measurement_time;
	nreps = 0;
	Clock::Tll startll = Clock::todll();
	while(1) {
	    nreps += 1;
	    if ((Clock::todll() - startll) > measurement_time_ll) {
		break;
	    }
	}
	Clock::Tll endll = Clock::todll();
	SINVARIANT(endll - startll >= measurement_time_ll);
	double ns_per_todll = 1000.0*(double)(endll - startll)/(double)nreps;
	printf("todll()               %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_todll, ns_per_todll / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_todll);
    }

    // Test Clock::todcc_direct()
    int todcc_reps;
    {
	nreps = 0;
	start = myclock.todcc_recalibrate();
	while(1) {
	    nreps += 1;
	    if ((myclock.todcc_direct() - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	todcc_reps = nreps;
	double ns_per_todcc = 1000.0*(double)(end - start)/(double)nreps;
	printf("todcc_direct()        %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_todcc, ns_per_todcc / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_todcc);
    }

    // Test Clock::todcc_incremental()
    {
	nreps = 0;
	start = myclock.todcc_recalibrate();
	while(1) {
	    nreps += 1;
	    if ((myclock.todcc_incremental() - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	double ns_per_todcc = 1000.0*(double)(end - start)/(double)nreps;
	printf("todcc_incremental()   %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_todcc, ns_per_todcc / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_todcc);
    }

    // Test Clock::todcc_direct() with long long cast
    // this was how the original todcc(), which was just todcc_direct() worked.
    {
	nreps = 0;
	Clock::Tll measurement_time_ll = (Clock::Tll)measurement_time;
       	Clock::Tll startll = (Clock::Tll)myclock.todll();
	while(1) {
	    nreps += 1;
	    if (((Clock::Tll)myclock.todcc_direct() - startll) > measurement_time_ll) {
		break;
	    }
	}
	Clock::Tll endll = myclock.todll();
	SINVARIANT(endll - startll >= measurement_time_ll);
	double ns_per_todcc = 1000.0*(double)(endll - startll)/(double)nreps;
	printf("todcc_direct()->ll    %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_todcc, ns_per_todcc / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_todcc);
    }

    Clock::Tdbl recalibrate_interval = myclock.recalibrate_interval_cycles * myclock.inverse_clock_rate;
    // Test calculating times by using division
    {
	double clock_rate = 1.0/myclock.inverse_clock_rate;
	double offset;
	{
	    double cur_us = Clock::tod();
	    double est_us = Clock::now()/clock_rate;
	    offset = cur_us - est_us;
	}

	nreps = 0;
	start = myclock.todcc_recalibrate();
	while(1) {
	    nreps += 1;
	    Clock::Tdbl cur_us = Clock::now()/clock_rate + offset;
	    if (cur_us < myclock.last_calibrate_tod ||
		(cur_us - myclock.last_calibrate_tod) > recalibrate_interval) {
		//		printf("recalibrate %.2f // %.2f",cur_us, cur_us - myclock.last_calibrate_tod);
		cur_us = myclock.todcc_recalibrate();
		//		printf(" to %.0f\n",cur_us);
		offset = cur_us - Clock::now()/clock_rate;
		myclock.last_calibrate_tod = cur_us;
	    }
	    if ((cur_us - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	double ns_per_divide = 1000.0*(double)(end - start)/(double)nreps;
	printf("todcc_divide()        %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_divide, ns_per_divide / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_divide);
    }

    // Test calculating times using doubles -- should be identical to todcc_direct()
    {
	nreps = 0;
	start = myclock.todcc_recalibrate();
	double start_double = start;
	while(1) {
	    nreps += 1;
	    double cur_us = (double)Clock::now() * inverse_clock_rate + myclock.cycle_count_offset;
	    if (cur_us < myclock.last_calibrate_tod ||
		(cur_us - myclock.last_calibrate_tod) > recalibrate_interval) {
		cur_us = myclock.todcc_recalibrate();
	    }
	    if ((cur_us - start_double) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	double ns_per_double_mul = 1000.0*(double)(end - start)/(double)nreps;
	printf("double()              %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_double_mul, ns_per_double_mul / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_double_mul);
    }

    // Test calculating times by using differencing
    {
	INVARIANT((recalibrate_interval / inverse_clock_rate) < (double)(1 << 30),
		  "bad recalibrate");
	unsigned int recalibrate_cycles = (int)(recalibrate_interval / inverse_clock_rate);
	nreps = 0;
	Clock::Tll measurement_time_ll = (Clock::Tll)measurement_time;
	Clock::Tll last_now;
	Clock::Tll startll = Clock::todll();
	last_now = startll;
	Clock::Tll last_cc = Clock::now();
	while(1) {
	    Clock::Tll now;
	    nreps += 1;
	    Clock::Tll cur_cc = Clock::now();
	    Clock::Tll delta_cc = cur_cc - last_cc;
	    if (delta_cc < 0 || delta_cc > recalibrate_cycles) {
		last_now = now = myclock.todll();
		last_cc = Clock::now();
	    } else {
		int delta_us = (int)((int)delta_cc * inverse_clock_rate);
		now = last_now + delta_us;
	    }
	    if ((now - startll) > measurement_time_ll) {
		break;
	    }
	}
	Clock::Tll endll = myclock.todll();
	SINVARIANT(endll - startll >= measurement_time_ll);
	double ns_per_incremental = 1000.0*(double)(endll - startll)/(double)nreps;
       	printf("cc_incremental()      %9d   %8lld   %7.4g   %7.4g     %5.3g\n",
	       nreps, endll - startll, ns_per_incremental, ns_per_incremental / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_incremental);
    }

    // Test calculating times by using differencing result in double -- should be identical to todcc_incremental()
    {
	nreps = 0;
	double last_now;
	last_now = start = myclock.todcc_recalibrate();
	Clock::Tll last_cc = Clock::now();
	INVARIANT((recalibrate_interval / inverse_clock_rate) < (double)(1 << 30),
		  "bad recalibrate");
	unsigned int recalibrate_cycles = (int)(recalibrate_interval / inverse_clock_rate);
	while(1) {
	    nreps += 1;
	    Tdbl now;
	    Clock::Tll delta_cc = Clock::now() - last_cc;
	    if (delta_cc < 0 || delta_cc > recalibrate_cycles) {
		last_now = now = myclock.todcc_recalibrate();
		last_cc = Clock::now();
	    } else {
		double delta_us = (int)delta_cc * inverse_clock_rate;
		now = last_now + delta_us;
	    }
	    if ((now - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	double ns_per_incremental = 1000.0*(double)(end - start)/(double)nreps;
       	printf("incr_double()         %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_incremental, ns_per_incremental / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_incremental);
    }

    // Test Clock::todcc()
    {
	nreps = 0;
	start = myclock.todcc_recalibrate();
	while(1) {
	    nreps += 1;
	    if ((myclock.todcc() - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	double ns_per_todcc = 1000.0*(double)(end - start)/(double)nreps;
	printf("fastest-of-above?     %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_todcc, ns_per_todcc / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_todcc);
    }

    // Test if we could just magically use the cycle counter
    {
	nreps = 0;
	Clock::Tll elapsed_cycles = (Clock::Tll)(measurement_time / inverse_clock_rate);
	start = myclock.tod();
	Clock::Tll start_cc = Clock::now();
	while(1) {
	    nreps += 1;
	    Clock::Tll now_cc = Clock::now();
	    if ((now_cc - start_cc) > elapsed_cycles) {
		break;
	    }
	}
	end = myclock.tod();
	SINVARIANT(end - start >= measurement_time*0.9);
	double ns_per_cyclecounter = 1000.0*(double)(end - start)/(double)nreps;
       	printf("pure_cycle_counter    %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, ns_per_cyclecounter, ns_per_cyclecounter / (1000.0*inverse_clock_rate),
	       ns_per_tod/ns_per_cyclecounter);
    }
    printf("\n");
    Stats calibrate_error;

    {
	int count_under_10_us = 0;
	for(unsigned i=0;i<clock_reps;i++) {
	    Clock::Tdbl now_tod = Clock::tod();
	    Clock::Tdbl now_tod2 = Clock::tod();
	    calibrate_error.add((double)(now_tod2 - now_tod));
	    if ((now_tod2 - now_tod) < 10) {
		++count_under_10_us;
	    }
	}
	printf("Error between Clock::tod() and Clock::tod():\n");
	printf("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n",
	       calibrate_error.min(),calibrate_error.max(),
	       calibrate_error.mean(), calibrate_error.stddev(),
	       calibrate_error.conf95());
	printf("  %d/%d, %.6f%% under 10 us\n",count_under_10_us,
	       clock_reps, 100*(double)count_under_10_us / clock_reps);
    }

    calibrate_error.reset();
    unsigned count_under_1_us = 0;
    for(int i=0;i<todcc_reps;i++) {
	Clock::Tdbl now_cc1 = myclock.todcc();
	Clock::Tdbl now_cc2 = myclock.todcc();
	calibrate_error.add((double)(now_cc2 - now_cc1));
	if ((now_cc2 - now_cc1) < 1) {
	    ++count_under_1_us;
	}
    }
    printf("Error between Clock::todcc() and Clock::todcc():\n");
    printf("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n",
	   calibrate_error.min(),calibrate_error.max(),
	   calibrate_error.mean(), calibrate_error.stddev(),
	   calibrate_error.conf95());
    printf("  %d/%d, %.6f%% under 1 us\n",count_under_1_us,
	   todcc_reps,100*(double)count_under_1_us / todcc_reps);

    calibrate_error.reset();
    for(unsigned i=0;i<clock_reps;i++) {
	Clock::Tdbl now_tod = Clock::tod();
	Clock::Tdbl now_cc = myclock.todcc_direct();
	calibrate_error.add((double)(now_cc - now_tod));
    }
    printf("Error between Clock::tod() and Clock::todcc_direct():\n");
    printf("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n",
	   calibrate_error.min(),calibrate_error.max(),
	   calibrate_error.mean(), calibrate_error.stddev(),
	   calibrate_error.conf95());

    calibrate_error.reset();
    for(unsigned i=0;i<clock_reps;i++) {
	Clock::Tdbl now_tod = Clock::tod();
	Clock::Tdbl now_cc = myclock.todcc_incremental();
	calibrate_error.add((double)(now_cc - now_tod));
    }
    printf("Error between Clock::tod() and Clock::todcc_incremental():\n");
    printf("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n",
	   calibrate_error.min(),calibrate_error.max(),
	   calibrate_error.mean(), calibrate_error.stddev(),
	   calibrate_error.conf95());

    {
	unsigned nzeros = 0;
	nreps = 0;
	start = myclock.todcc_recalibrate();
	Clock::Tdbl last_todcc = start;
	while(1) {
	    ++nreps;
	    Clock::Tdbl now = myclock.todcc();
	    if (now == last_todcc) {
		++nzeros;
	    }
	    last_todcc = now;
	    if ((now - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	printf("%u/%u/%u zero_advance/nonzero_advance/total_reps; %.2f%%\n",
	       nzeros, nreps-nzeros, nreps, 100.0*nzeros/nreps);

	myclock.setTodccEpoch(static_cast<unsigned>((Clock::tod()/1000000.0)-60));

	myclock.nbadgettod = 0;
	nzeros = 0;
	nreps = 0;
	start = myclock.todcc_recalibrate();
	last_todcc = start;
	Clock::Tdbl example_zeros[100];
	while(1) {
	    ++nreps;
	    Clock::Tdbl now = myclock.todcc();
	    if (now == last_todcc) {
		if (nzeros < 100) {
		    example_zeros[nzeros] = now;
		}
		++nzeros;
	    }
	    last_todcc = now;
	    if ((now - start) > measurement_time) {
		break;
	    }
	}
	end = myclock.todcc_recalibrate();
	SINVARIANT(end - start >= measurement_time);
	printf("%u/%u/%u zero_advance/nonzero_advance/total_reps with recent epoch; %.2f%%\n",
	       nzeros, nreps-nzeros, nreps, 100.0*nzeros/nreps);
	if (nzeros > 0) {
	    printf("nbadgettod: %d\n", myclock.nbadgettod);
	    for(unsigned i = 0; i < 100 && i < nzeros; ++i) {
		printf("Example zero happened at %.30g\n", example_zeros[i]);
	    }
	}

	nzeros = 0;
	Clock::Tll last_now = Clock::now();
	for(unsigned i = 0; i < nreps; ++i) {
	    Clock::Tll cur_now = Clock::now();
	    if (cur_now == last_now) {
		++nzeros;
	    }
	    last_now = cur_now;
	}
	printf("%u/%u/%u zero_advance/nonzero_advance/total_reps with Clock::now(); %.2f%%\n",
	       nzeros, nreps-nzeros, nreps, 100.0*nzeros/nreps);
    }
    
}

void
Clock::selfCheck()
{
    cout << "Checking conversion from s.us <-> Tfrac" << endl;
    
    MersenneTwisterRandom rand;

    for(uint32_t us = 0; us < 1000000; ++us) { // test all possible us values.
	uint32_t sec = rand.randInt();
	Clock::Tfrac tfrac = secMicroToTfrac(sec, us);
	uint32_t sec_test = TfracToSec(tfrac);
	uint32_t us_test = TfracToMicroSec(tfrac);
	INVARIANT(sec_test == sec && us_test == us,
		  format("s.us <-> Tfrac failed: %d.%d -> %lld (lowbits %lld) -> %d.%d")
		  % sec % us % tfrac % (tfrac & 0xFFFFFFFFULL) % sec_test % us_test);
    }
    
    cout << "Checking conversion from s.ns <-> Tfrac" << endl;
    for(uint32_t testnum = 0; testnum < 20000000; ++testnum) { // sample test 2% of possible values
	uint32_t sec = rand.randInt();
	uint32_t ns = rand.randInt(1000000000);
	Clock::Tfrac tfrac = secNanoToTfrac(sec, ns);
	uint32_t sec_test = TfracToSec(tfrac);
	uint32_t ns_test = TfracToNanoSec(tfrac);
	INVARIANT(sec_test == sec && ns_test == ns,
		  format("s.ns <-> Tfrac failed: %d.%d -> %lld (lowbits %lld) -> %d.%d")
		  % sec % ns % tfrac % (tfrac & 0xFFFFFFFFULL) % sec_test % ns_test);
    }
}

