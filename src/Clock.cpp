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
#ifdef SYS_POSIX
#include <sys/time.h>
#include <dirent.h>
#endif
#include <sys/types.h>

#include <algorithm>
#include <vector>

#include <Lintel/Clock.hpp>
#include <Lintel/Double.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/PThread.hpp>
#include <Lintel/SimpleMutex.hpp>
#include <Lintel/Stats.hpp>

using namespace std;
using boost::format;

// Use -Double::Inf so things blow up suitably if these get used.
double Clock::clock_rate = -Double::Inf;
double Clock::inverse_clock_rate_tfrac = -Double::Inf;
uint64_t Clock::max_recalibrate_measure_time = 0; // forces recalibration always unless initialized
Clock::Tfrac Clock::estimated_todtfrac_quanta = 0;
uint64_t Clock::estimated_cycles_per_quanta = 0;

static Clock::AllowUnsafeFreqScalingOpt allow_unsafe_frequency_scaling = Clock::AUFSO_No;
static bool may_have_frequency_scaling = true; 

namespace {
    class Clock_Tll_Order {
    public:
	inline bool operator() (Clock::Tll a, Clock::Tll b) const {
	    return a < b;
	}
    };

    string readFileLine(const string &filename) {
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

    void checkForPossibleFrequencyScaling() {
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
}

void Clock::allowUnsafeFrequencyScaling(AllowUnsafeFreqScalingOpt allow) {
    INVARIANT(clock_rate == -Double::Inf || allow == allow_unsafe_frequency_scaling, 
	      "Already calibrated the clock, so can't change the Clock::allowUnsafeFrequencyScaling mode");
    allow_unsafe_frequency_scaling = allow;
}

void Clock::initialMeasurements() {
    {
	vector<Tll> elapsed;
	vector<Tfrac> tod_delta;
	const uint32_t nelapsed = 500;
	elapsed.reserve(nelapsed);
	tod_delta.reserve(nelapsed);
	Tfrac prev_tod = todTfrac();
	for(uint32_t i=0; i < nelapsed || tod_delta.size() < 20; i++) {
	    Tll start_cycle = cycleCounter();
	    Tfrac cur_tod = todTfrac();
	    Tll end_cycle = cycleCounter();
	    elapsed.push_back(end_cycle - start_cycle);
	    if (cur_tod - prev_tod > 0) {
		tod_delta.push_back(cur_tod - prev_tod);
		prev_tod = cur_tod;
	    }
	}
	sort(elapsed.begin(),elapsed.end(),Clock_Tll_Order());
	sort(tod_delta.begin(), tod_delta.end());
	uint32_t off = static_cast<uint32_t>(elapsed.size() * 0.25); // 25th percentile

	// If we're over 2x the estimated time to call tod, then we
	// really know nothing about how to calibrate so have to rely
	// on the kernel timekeeping.
	max_recalibrate_measure_time = 2 * elapsed[off];
	
	off = static_cast<uint32_t>(tod_delta.size() * 0.25); // 25th percentile
	estimated_todtfrac_quanta = tod_delta[off];
    }

    {
	// Estimate min # cycles to pass a quanta
	vector<uint64_t> cycles;
	// This many measurements will be somewhat slow on windows
	// ~0.45s since we have to pass through 3 tod changes, and the
	// windows quanta is 15ms.
	const uint32_t nmeasurements = 10;
	cycles.reserve(nmeasurements);
	Tfrac prev_tod, cur_tod, next_tod;
	uint64_t cycle_a, cycle_b;
	while(cycles.size() < nmeasurements) {
	    cycle_a = cycleCounter();
	    prev_tod = todTfrac();
	    do { // Wait for a quanta shift
		cycle_a = cycleCounter();
		cur_tod = todTfrac();
	    } while(cur_tod == prev_tod); 
	    do {
		next_tod = todTfrac();
		cycle_b = cycleCounter();
	    } while (next_tod == cur_tod);
	    if (cycle_b > cycle_a) { 
		// upper bound on cycles to move through a quanta
		cycles.push_back(cycle_b - cycle_a);
	    } else {
		// We definately switched cores, so this measurement is bogus.
	    }
	}
	Stats truncated_mean;
	for(uint32_t i = static_cast<uint32_t>(nmeasurements * 0.25); 
	    i < static_cast<uint32_t>(nmeasurements * 0.75);
	    ++i) {
	    truncated_mean.add(cycles[i]);
	}
	estimated_cycles_per_quanta = static_cast<uint64_t>(round(truncated_mean.mean()));
    }
}

namespace {
    SIMPLE_MUTEX_SINGLETON(clockRateMutex);
}

void Clock::setClockRate(double estimated_mhz) {
    clockRateMutex().lock();
    INVARIANT(clock_rate == -Double::Inf,
	      format("whoa, two threads calibrating at the same time?! %g != %g")
	      % clock_rate % (-Double::Inf));
    clock_rate = estimated_mhz;

    double inverse_clock_rate = 1.0 / estimated_mhz;

    // cycle_counter * icr = time in us * 1/1e6 = time
    // in seconds * 2**32 = time in Tfrac
    inverse_clock_rate_tfrac = inverse_clock_rate * (4294967296.0/1000000.0);
    // increased the bound to 30us
    // (30*tmp_clock_rate); under load, this invariant
    // could fire; might want to warn, on a really
    // slow recalibrate, we could introduce a lot of
    // error.
    INVARIANT(max_recalibrate_measure_time < 30 * clock_rate,
	      format("Internal sanity check failed, tod() takes too long, %d > %.2f\n")
	      % max_recalibrate_measure_time % (30 * clock_rate));
    clockRateMutex().unlock();
}

void Clock::calibrateClock(bool print_calibration_information, 
			   double max_conf95_rel,
			   int print_calibration_warnings_after_tries) {
    if (clock_rate != -Double::Inf) {
	return; // already calibrated; don't handle changes.
    }

    Tfrac calibrate_start = todTfrac();

    if (allow_unsafe_frequency_scaling != AUFSO_Yes) {
	checkForPossibleFrequencyScaling();
    }

    string errors;

    LintelLogDebug("lintel::Clock", "starting time calibration");

    bindToProcessor();

    initialMeasurements();

    uint64_t target_cycles = estimated_cycles_per_quanta * 25; // 25us on unix

    const uint32_t min_samples = 40; // 1ms total on unix
    const uint32_t max_samples = 400;
    vector<double> measurements;
    Stats truncated_mean;
    for(int tries=0;tries<10;++tries) {
	measurements.clear();
	measurements.reserve(min_samples);
	uint32_t fail_count = 0;
	while (true) {
	    uint64_t cycles_a = cycleCounter();
	    Tfrac tod_start = todTfrac();
	    uint64_t cycles_b = cycleCounter();
	    uint64_t cycles_end = cycles_a + target_cycles;
	    uint64_t cycles_c = cycles_b;
	    while(cycles_c > cycles_a && cycles_c < cycles_end) {
		cycles_c = cycleCounter();
	    }
	    Tfrac tod_end = todTfrac();
	    uint64_t cycles_d = cycleCounter();

	    if (cycles_c <= cycles_a || (cycles_c - cycles_end) > 10000) {
		// useless, cycles went backwards, or went forward by
		// a lot, so we must have either switched cores or
		// been descheduled
		++fail_count;
		if (fail_count > 128) {
		    errors.append(str(format("%d failures taking timing measurements, only %d successful\n") % fail_count % measurements.size()));
		    break;
		}
		if ((fail_count % 16) == 0) {
		    // we're getting pre-empted or re-scheduled a lot.
		    // de-schedule in the hopes that when we come back 
		    struct timeval timeout;
		    timeout.tv_sec = 0;
		    // 128 / 16 * 11000 = 88,000us or ~0.1s
		    timeout.tv_usec = (fail_count / 16) * 11000;
		    select(0, NULL, NULL, NULL, &timeout);
		}
	    } else {
		uint64_t cycles_start = (cycles_a + cycles_b) / 2;
		uint64_t cycles_end = (cycles_c + cycles_d) / 2;
		double elapsed_cycles = cycles_end - cycles_start;
		double elapsed_seconds = TfracToDouble(tod_end - tod_start);
		
		measurements.push_back(elapsed_cycles / (1.0e6 * elapsed_seconds));
	    }
	    if (measurements.size() >= min_samples) {
		sort(measurements.begin(), measurements.end());
		truncated_mean.reset();
		for(uint32_t i = static_cast<uint32_t>(measurements.size() * 0.25); 
		    i < static_cast<uint32_t>(measurements.size() * 0.75);
		    ++i) {
		    truncated_mean.add(measurements[i]);
		}
		if (truncated_mean.conf95() < truncated_mean.mean() * max_conf95_rel) {
		    break;
		}
	    }
	    if (measurements.size() >= max_samples) {
		errors.append("too many measurements taken, trying again\n");
	    }
	}

	if (measurements.size() < min_samples) {
	    continue;
	}

	if (print_calibration_information || tries >= print_calibration_warnings_after_tries) {
	    cerr << format("Calibrating clock rate, try %d: est mean %.10g, conf95 %.10g\n")
		% tries % truncated_mean.mean() % truncated_mean.conf95();
	}
	if (truncated_mean.conf95() < truncated_mean.mean() * max_conf95_rel) {
	    setClockRate(truncated_mean.mean());
	    break;
	}
    }

    INVARIANT(clock_rate > 200, 
	      format("Unable to calibrate clock rate to %.8g relative error.  Errors:%s\n") 
	      % max_conf95_rel % errors);
    unbindFromProcessor();
    if (print_calibration_information) {
	Tfrac calibrate_end = todTfrac();
	double calibrate_ms = 1000 * TfracToDouble(calibrate_end - calibrate_start);
	cout << format("Final calibrated clock rate is %.8g, conf95 %.8g, %d samples; %.3fms to calibrate\n") 
	    % clock_rate % truncated_mean.conf95() % truncated_mean.count() % calibrate_ms;
    }
}

Clock::Clock(bool allow_calibration)
    : nrecalibrate(0), nbadgettod(0), bad_get_tod_cycle_gap(NULL), 
      last_calibrate_tod_tfrac(0),
      last_cc(0), last_recalibrate_cc(0), recalibrate_interval_cycles(0)
{
    if (clock_rate <= 0) {
	INVARIANT(allow_calibration,
		  "you failed to call Clock::calibrateClock()!");
	calibrateClock();
    }
    SINVARIANT(clock_rate > 0);
    setRecalibrateIntervalSeconds();
}

void Clock::bindToProcessor(unsigned proc) {
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

void Clock::unbindFromProcessor() {
#ifdef HPUX_ACC
    pthread_spu_t answer;
    int ret = pthread_processor_bind_np(PTHREAD_BIND_ADVISORY_NP, &answer,
					PTHREAD_SPUFLOAT_NP, PTHREAD_SELFTID_NP);
    SINVARIANT(ret==0);
#endif
}

Clock::Tfrac Clock::todccTfrac_recalibrate() {
    if (may_have_frequency_scaling) {
	if (allow_unsafe_frequency_scaling == AUFSO_WarnSlow ||
	    allow_unsafe_frequency_scaling == AUFSO_Slow) {
	    return todTfrac();
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

    uint64_t start_cycle = cycleCounter();
    Clock::Tfrac cur_time = todTfrac();
    uint64_t end_cycle = cycleCounter();
    INVARIANT(cur_time >= last_calibrate_tod_tfrac,
	      format("Whoa, todTfrac() went backwards %d.%09d < %d.%09d")
	      % TfracToSec(cur_time) % TfracToNanoSec(cur_time)
	      % TfracToSec(last_calibrate_tod_tfrac) % TfracToNanoSec(last_calibrate_tod_tfrac));
    if ((end_cycle <= start_cycle) || (end_cycle - start_cycle) >= max_recalibrate_measure_time) {
	LintelLogDebug("lintel::Clock", format("Whoa, bad todcc_recalibrate getting %d took %d to %d = %d, or %.3g us\n")
		       % cur_time % start_cycle % end_cycle % (end_cycle - start_cycle)
		       % (static_cast<double>(end_cycle - start_cycle)/clock_rate));
	if (bad_get_tod_cycle_gap == NULL) {
	    bad_get_tod_cycle_gap = new Stats;
	}
	bad_get_tod_cycle_gap->add(end_cycle - start_cycle);
	++nbadgettod;
	// next line guarantees recalibration next time assuming the
	// cycle counter isn't reset and booting takes a little time
	last_recalibrate_cc = 0; 
    } else {
	uint64_t mid_cycle = (start_cycle + end_cycle)/2;
	last_calibrate_tod_tfrac = cur_time;

	last_recalibrate_cc = last_cc = mid_cycle;
	
	++nrecalibrate;
    }
    
    return cur_time;
}

void Clock::setRecalibrateIntervalSeconds(double interval) {
    last_recalibrate_cc = 0;
    double tmp_recalibrate_cycles = interval * 1.0e6 * clock_rate;
    double extra_multiplier = inverse_clock_rate_tfrac < 1 ? 1 : inverse_clock_rate_tfrac;
    // Make sure both the # cycles in a recalibrate interval fits in a
    // uint32, and that the # of Tfracs in a recalibrate interval fits
    // in a uint32.  This check makes sure that the cast to int32_t
    // in todcc_incremental, and the two casts to uint32_t in
    // todccTfrac_incremental are both safe (no wrapping).
    // worth a 10% speedup on 32 bit, 1<<30 is conservative
    INVARIANT(tmp_recalibrate_cycles * extra_multiplier < (1 << 30),
	      format("using too large a recalibration interval on too fast a machine:\n"
		     "  %.2f * 10^6 * %.2f * %.2f= %.2f > %d")
	      % interval % clock_rate % tmp_recalibrate_cycles % extra_multiplier % (1 << 30));
    recalibrate_interval_cycles = static_cast<uint64_t>(interval * 1.0e6 * clock_rate);
}

void Clock::timingTest() {
    // TODO: add some invariants in to this so that we can verify it's
    // doing the right thing.  With CMake hiding the output, there's
    // no point in doing this in the regression test size it doesn't
    // verify anything.  Fix the #if 0 down below at the same time;
    // it's using todcc() and it's not worth fixing until we have real
    // regression stuff on this.

    Clock::Tfrac measurement_time_tfrac = Clock::secNanoToTfrac(1,0);
    Clock::calibrateClock(true,0.001);
    Clock myclock;
    myclock.setRecalibrateIntervalSeconds();

    cout << "timing method           reps    elapsed us    ns/fn     cycles/fn ratio to todtfrac\n";

    uint32_t nreps = 0;
    double ns_per_todtfrac;

    {
	Clock::Tfrac start = myclock.todTfrac();
	Clock::Tfrac end_target = start + measurement_time_tfrac;
	do {
	    nreps += 1;
	} while (myclock.todTfrac() < end_target);
	Clock::Tfrac end = myclock.todTfrac();
	SINVARIANT(end > start && end - start >= measurement_time_tfrac);
	double elapsed_ns = Clock::TfracToDouble(end - start) * 1.0e9;
	ns_per_todtfrac = elapsed_ns / static_cast<double>(nreps);
	cout << format("todTfrac              %9d   %8.0f   %7.4g   %7.4g     %5.3g\n")
	    % nreps % (elapsed_ns * 1e-3) % ns_per_todtfrac
	    % (ns_per_todtfrac * clock_rate / 1000.0) % (ns_per_todtfrac/ns_per_todtfrac);
    }

    unsigned clock_reps = nreps;

    // Test as if we could just magically use the cycle counter
    {
	nreps = 0;
	uint64_t elapsed_cycles = clock_rate * 1000 * 1000;
	Tfrac start = myclock.todTfrac();
	uint64_t start_cc = Clock::cycleCounter();
	while(1) {
	    nreps += 1;
	    uint64_t now_cc = Clock::cycleCounter();
	    if ((now_cc - start_cc) > elapsed_cycles) {
		break;
	    }
	}
	Tfrac end = myclock.todTfrac();
	double elapsed = TfracToDouble(end - start);
	SINVARIANT(elapsed >= 0.9);
	double ns_per_cyclecounter = (1.0e9 * elapsed)/nreps;
       	cout << format("pure_cycle_counter    %9d   %8.0f   %7.4g   %7.4g     %5.3g\n")
	    % nreps % (end - start) % ns_per_cyclecounter
	    % (ns_per_cyclecounter * clock_rate / 1000.0) % (ns_per_todtfrac/ns_per_cyclecounter);
    }

    cout << "\n";

    Stats calibrate_error;

    {
	Tfrac ten_us = secMicroToTfrac(0, 10);
	int count_under_10_us = 0;
	for(unsigned i=0;i<clock_reps;i++) {
	    Clock::Tfrac now_tod = Clock::todTfrac();
	    Clock::Tfrac now_tod2 = Clock::todTfrac();
	    calibrate_error.add((double)(now_tod2 - now_tod));
	    if ((now_tod2 - now_tod) < ten_us) {
		++count_under_10_us;
	    }
	}
	cout << "Error between Clock::todTfrac() and Clock::todTfrac():\n";
	cout << format("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n")
	    % calibrate_error.min() % calibrate_error.max() 
	    % calibrate_error.mean() % calibrate_error.stddev() % calibrate_error.conf95();
	cout << format("  %d/%d, %.6f%% under 10 us\n") % count_under_10_us
	    % clock_reps % (100.0 * count_under_10_us / clock_reps);
    }

    // TODO: add in a timing test for todccTfrac and alternatives, potentially
    // lift some of the removed code that did the calculation with doubles
    // (faster? but wrong)
}

void Clock::selfCheck() {
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

