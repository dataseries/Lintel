/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Header file for tod clock/cycle counter interactions
*/

#ifndef __CLOCK_H
#define __CLOCK_H

#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <Lintel/LintelAssert.H>
#include <Lintel/Stats.H>

#if __HP_aCC
extern "C" {
    void readControlRegister16(long long *);
};
#endif

#if defined(__linux__) && (defined(__i386__) || defined(__x86_64__))
#include <asm/msr.h>
#endif

class Clock {
public:
    // Type split as of 2003-01-30, and renamed to guarantee
    // compilation problems if the code is used incorrectly; after
    // June if no problems have been found, we should re-introduce the
    // T type as a synonym for Tdbl; performance measurement (see
    // tests/clock.C) has indicated that calculating the time as a
    // double is faster than using a long long, and both have enough
    // precision
    // 
    // for both of the versions, the units are micro-seconds
    typedef long long Tll;
    typedef double Tdbl;

    // Clock cycle calibration occurs until the relative conf95 is
    // under the maximum allowed, the default of 0.01 allows for a 1%
    // error.  Calibration only happens for the first clock object
    // created.  You should call this single threaded before
    // initializing any Clock() objects (or initialize them with
    // allow_calibration = true).  The separation is necessary because
    // FAB doesn't allow us to have a pthread lock around calibration,
    // but having too many threads calibrating at once will cause
    // calibration to fail.
    static void calibrateClock(bool print_calibration_information = false,
			       double max_conf95_rel = 0.01,
			       int print_calibration_warnings_after_tries = 1);

    Clock(bool allow_calibration = false);
    static double clock_rate; // MhZ, assume SMP processors run at same rate.
    static Stats calibrate;
    static Tll now() {
	Tll ret = 0;
#if defined(__HP_aCC)
	readControlRegister16(&ret);
#elif defined(__linux__) && (defined(__i386__) || defined(__x86_64__))
	rdtscll(ret);
#else
#error "Do not know how to get the cycle counter on this platform"
#endif
	return ret;
    }
  
    // Might want to switch to using clock_gettime() instead of gettimeofday()
    // for more accuracy
    static Tll todll() {
	struct timeval t;
	t.tv_sec = -1;
	AssertAlways(gettimeofday(&t,NULL)==0,("Internal error\n"));
	return (Tll)t.tv_sec * (Tll)1000000 + (Tll)t.tv_usec;
    }

    static Tdbl tod() {
	struct timeval t;
	t.tv_sec = -1;
	AssertAlways(gettimeofday(&t,NULL)==0,("Internal error\n"));
	return (Tdbl)t.tv_sec * (Tdbl)1000000 + (Tdbl)t.tv_usec;
    }	

#if 0
    // Unused?
    static void future(struct timespec &ts, T us_future = 0) {
	T t = (tod() + us_future) * 1000;
	ts.tv_sec = (long)(t / 1000000000);
	ts.tv_nsec = (long)(t % 1000000000);
    }
    static double usDiff(T start, T end) {
	T diff = end - start;
	return (double)diff * inverse_clock_rate;
    }
#endif
    static void bindToProcessor(unsigned proc = 0); // Needed on SMP machines as cycle counters aren't the same.
    static void unbindFromProcessor();
    
    Tdbl todcc_recalibrate();

    // Not a good idea to use the todcc from multiple threads; have a separate
    // Clock object for each of them.

    inline Tdbl todcc_direct() {
	Tll cur_cc = now();
	if (cur_cc < last_cc || (cur_cc - last_recalibrate_cc) > recalibrate_interval_cycles) {
	    last_cc = cur_cc;
	    return todcc_recalibrate();
	} else {
	    last_cc = cur_cc;
	    return (double)cur_cc * inverse_clock_rate + cycle_count_offset;
	}
//
//	  if (cur_us < last_calibrate_tod || 
//	      (cur_us - last_calibrate_tod) > recalibrate_interval) {
//	      return todcc_recalibrate();
//	  }
//	  return cur_us;
    }

    inline Tdbl todcc_incremental() {
	Tll delta_cc = now() - last_recalibrate_cc;
	if (delta_cc < 0 || delta_cc > recalibrate_interval_cycles) {
	    return todcc_recalibrate();
	} else {
	    double delta_us = (int)delta_cc * inverse_clock_rate;
	    return last_calibrate_tod + delta_us;
	}
    }

    inline Tdbl todcc() {
	// PIII based machines (all I have to test) are faster in 
	// incremental mode.
	// HPPA 2 based machines are all faster in direct mode, 
	// probably because they can use the fused multiply-add
#if defined(__i386__) || defined(i386) || defined(__i386)
	return todcc_incremental();
#else
	return todcc_direct();
#endif
    }

    void setRecalibrateInterval(Tdbl interval_us) {
	last_cc = 0;
	recalibrate_interval = interval_us;
	// This check is needed so that the (int) cast in
	// todcc_incremental is correct; and the (int) cast
	// results in a reasonable speedup
	AssertAlways(recalibrate_interval * clock_rate < (double)(1 << 30),
		     ("using too large a recalibration interval on too fast a machine %.2f, %.2f\n",
		      recalibrate_interval, clock_rate));
	recalibrate_interval_cycles = (Tll)(recalibrate_interval  * clock_rate);
    }
    // nbadgettod counts the number of times we got the time of day, but
    // switched CPUs or took forever to do it, thereby invalidating the
    // effort to calibrate the cycle counter to the time of day.
    int nrecalibrate, nbadgettod;
#if 0
    int ntodzerotime;
    Stats badrecalibration_delta;
#endif
    static void timingTest();
public:
    static double inverse_clock_rate; // us/cycle
    static Tll max_recalibrate_measure_time; // see todcc_recalibrate()
    // Cycle counter calibration variables
    Tdbl last_calibrate_tod, cycle_count_offset, recalibrate_interval;
    Tll last_cc, last_recalibrate_cc, recalibrate_interval_cycles;
};

#endif
