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
#include <stdint.h>

#include <limits>

#include <Lintel/AssertBoost.H>
#include <Lintel/LintelAssert.H>
#include <Lintel/Stats.H>

#if __HP_aCC
extern "C" {
    void readControlRegister16(long long *);
};
#endif

#if defined(__linux__) && defined(__i386__)
/* don't include <asm/msr.h>, as it fails on some platforms */
#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))
#define HAVE_RDTSCLL 1
#endif

#if defined(__linux__) && defined(__x86_64__)
#include <asm/msr.h>
#define HAVE_RDTSCLL 1
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

    // units for this is seconds * 2^-32, such that Tfrac >> 32 == time in seconds
    typedef unsigned long long Tfrac;

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
#define LINTEL_CLOCK_CYCLECOUNTER 1
#elif defined(HAVE_RDTSCLL)
	rdtscll(ret);
#define LINTEL_CLOCK_CYCLECOUNTER 1
#else
#warning "Do not know how to get the cycle counter on this platform, clock functions may be slow"
#define LINTEL_CLOCK_CYCLECOUNTER 0
	AssertFatal(("do not know how to get cycle counter on this platform"));
#endif
	return ret;
    }
  
    static Tll todll() {
	struct timeval t;
	AssertAlways(gettimeofday(&t,NULL)==0,("Internal error\n"));
	return (Tll)t.tv_sec * (Tll)1000000 + (Tll)t.tv_usec;
    }

    // Might want to switch to using clock_gettime() instead of gettimeofday()
    // for more accuracy
    static Tdbl tod() {
	return tod_epoch_internal(0);
    }	

    static void bindToProcessor(unsigned proc = 0); // Needed on SMP machines as cycle counters aren't the same.
    static void unbindFromProcessor();
    
    Tdbl tod_epoch() {
	return tod_epoch_internal(epoch_offset);
    }	

    Tdbl todcc_recalibrate();

    // Not a good idea to use the todcc from multiple threads; have a separate
    // Clock object for each of them.

    inline Tdbl todcc_direct() {
	Tll cur_cc = now();
	if (cur_cc <= last_cc || (cur_cc - last_recalibrate_cc) > recalibrate_interval_cycles) {
	    last_cc = cur_cc;
	    return todcc_recalibrate();
	} else {
	    last_cc = cur_cc;
	    return (double)cur_cc * inverse_clock_rate + cycle_count_offset;
	}
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
	// incremental mode.  Tested an opteron 2.8GhZ, also faster
	// in incremental mode
	// HPPA 2 based machines are all faster in direct mode, 
	// probably because they can use the fused multiply-add
#if LINTEL_CLOCK_CYCLECOUNTER == 0
	return tod();
#elif defined(__i386__) || defined(i386) || defined(__i386) || defined(__x86_64__)
	return todcc_incremental();
#else
	return todcc_direct();
#endif
    }

    /// How many micro seconds should elapse between recalibrations
    void setRecalibrateInterval(Tdbl interval_us);

    /// What epoch (in seconds since 1970) should be used for the
    /// return value from todcc{,_direct,_incremental}(); this is
    /// needed to get precision back in Tdbl as on modern (e.g. 2.8GhZ
    /// opteron) the time to call todcc() is less than the precision
    /// available in a Tdbl.
    void setTodccEpoch(unsigned seconds);

    void setTodccEpoch() {
	// An hour ago
	setTodccEpoch(static_cast<unsigned>(Clock::tod() * 1e-6) - 3600);
    }

    Tdbl getTodccEpochTdbl() { 
	return static_cast<Tdbl>(epoch_offset) * 1.0e6;
    }

    // nbadgettod counts the number of times we got the time of day, but
    // switched CPUs or took forever to do it, thereby invalidating the
    // effort to calibrate the cycle counter to the time of day.
    int nrecalibrate, nbadgettod;
    static void timingTest();

    static Tfrac secondsToTfrac(double seconds) {
	INVARIANT(seconds < std::numeric_limits<uint32_t>::max(), 
		  "seconds out of bounds for conversion");
	uint32_t sec_only = static_cast<uint32_t>(floor(seconds));
	// lower = now - secs
	// lower_ns = round(lower * 1e9)
	// lower_tfrac = lower_ns << 32 / 1e9
	//             = round(lower * 1e9) * 2^32 / 1e9
	//             ~= round(lower * 1e9 * 2^32 / 1e9)
	//             = round(lower * 2^32)
	//             = round((now - secs) * 2^32) = tmp
	double tmp = round((seconds - sec_only) * 4294967296.0);
	uint32_t tfrac_lower = static_cast<uint32_t>(tmp);
	return (static_cast<uint64_t>(sec_only) << 32) + tfrac_lower;	
    }
    static Tfrac secMicroToTfrac(uint32_t seconds, uint32_t micro_seconds) {
	Tfrac ret = static_cast<Tfrac>(seconds) << 32;
	ret += (static_cast<Tfrac>(micro_seconds) << 32) / 1000000;
	return ret;
    }

    static Tfrac secNanoToTfrac(uint32_t seconds, uint32_t nano_seconds) {
	Tfrac ret = static_cast<Tfrac>(seconds) << 32;
	ret += (static_cast<Tfrac>(nano_seconds) << 32) / 1000000000;
	return ret;
    }

    static uint32_t TfracToSec(Tfrac in) {
	return static_cast<uint32_t>(in >> 32);
    }

    static uint32_t TfracToMicroSec(Tfrac in) {
	in = in & 0xFFFFFFFFULL;
	// Have to convert through a double because the conversion is
	// inexact, e.g.  1 us -> 4294 Tfrac units -> 0.99977 us
	double tmp = static_cast<double>(in) * 1.0e6 / 4294967296.0;
	return static_cast<uint32_t>(round(tmp));
    }

    static uint32_t TfracToNanoSec(Tfrac in) {
	in = in & 0xFFFFFFFFULL;
	// Have to convert through a double because the conversion is
	// inexact
	double tmp = static_cast<double>(in) * 1.0e9 / 4294967296.0;
	return static_cast<uint32_t>(round(tmp));
    }

    static Tll TfracToTll(Tfrac in) {
	return static_cast<Tll>(TfracToSec(in))*1000000 + TfracToMicroSec(in);
    }

    static Tfrac todTfrac() {
	struct timeval t;
	AssertAlways(gettimeofday(&t,NULL)==0,("Internal error\n"));
	return secMicroToTfrac(t.tv_sec,t.tv_usec);
    }	

    static struct timeval TfracToStructTimeval(Tfrac in) {
	struct timeval ret;
	ret.tv_sec = TfracToSec(in);
	ret.tv_usec = TfracToMicroSec(in);
	return ret;
    }

    static void selfCheck();
public:
    static double inverse_clock_rate; // us/cycle
    static Tll max_recalibrate_measure_time; // see todcc_recalibrate()
    // Cycle counter calibration variables
    Tdbl last_calibrate_tod, cycle_count_offset, recalibrate_interval;
    Tll last_cc, last_recalibrate_cc, recalibrate_interval_cycles;
private:
    unsigned epoch_offset;
    
    static Tdbl tod_epoch_internal(unsigned epoch) {
	// No point of this on linux, timer there is only doing us precision and
	// clock_gettime() requires us to list with -lrt
#if 0 && defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
	struct timespec t;
	AssertAlways(clock_gettime(CLOCK_REALoTIME,&t)==0,("Internal error\n"));
	return (Tdbl)(t.tv_sec - epoch) * (Tdbl)1000000 + (Tdbl)t.tv_nsec * 0.001;
#else
	struct timeval t;
	AssertAlways(gettimeofday(&t,NULL)==0,("Internal error\n"));
	return (Tdbl)(t.tv_sec - epoch) * (Tdbl)1000000 + (Tdbl)t.tv_usec;
#endif
    }	
};

#endif
