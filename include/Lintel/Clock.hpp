/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Header file for tod clock/cycle counter interactions
*/

#ifndef LINTEL_CLOCK_HPP
#define LINTEL_CLOCK_HPP

#include <math.h>
#include <time.h>
#if defined(SYS_POSIX) || defined(__unix) || defined(__linux)
#include <sys/time.h>
#include <pthread.h>
#endif

#include <stdint.h>
#include <limits>

#include <boost/config.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Stats.hpp>

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
// cannot count on msr.h to define this in recent
// kernels.  this was ripped verbatim from RHEL4's
// /usr/include/asm-x86_64/msr.h
#define rdtscll(val) do { \
     unsigned int a,d; \
     asm volatile("rdtsc" : "=a" (a), "=d" (d)); \
     (val) = ((unsigned long)a) | (((unsigned long)d)<<32); \
} while(0)
#define HAVE_RDTSCLL 1
#endif

//TODO: how to determine processor architecture?
#if defined(BOOST_MSVC)
namespace {
uint64_t rdtsc(void) {
    __asm {
	rdtsc
        ret
    }
}

void rdtscll(uint64_t &val) {
    val = rdtsc();
}
}
#define HAVE_RDTSCLL 1
#endif

/// \class Clock
/// \brief Class for dealing with the clock in various ways.
///
/// The Clock class has four classes of functions: 1) functions for
/// getting the clock, either directly from the system, or using the
/// cycle counter.  2) functions for converting between various clock
/// representations.  3) functions for configuring how it operates. 4)
/// utility functions either for external or internal use.
///
/// TODO: deprecate Tdbl; it's just not safe, there is just barely enough
/// precision to represent us with doubles, and when you subtract them, you
/// get things like .934us for the minimum separation.  Probably switch
/// entirely to the use of Tfrac, change it to an int64 so substraction works
/// sanely, and rename it T.

class Clock {
public:
    // for both of the versions, the units are micro-seconds
    typedef long long Tll; ///< units of micro-seconds
    typedef double Tdbl; ///< units of micro-seconds

    /// Tfrac units are seconds * 2^-32, such that Tfrac >> 32 == time
    /// in seconds, and (Tfrac & 0xFFFFFFFF) / 4294967296.0 = fractional
    /// seconds
    typedef uint64_t Tfrac;

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

    static uint64_t getCycleCounter() {
	uint64_t ret = 0;
#if defined(__HP_aCC)
	readControlRegister16(&ret);
#define LINTEL_CLOCK_CYCLECOUNTER 1
#elif defined(HAVE_RDTSCLL)
	rdtscll(ret);
#define LINTEL_CLOCK_CYCLECOUNTER 1
#else
#warning "Do not know how to get the cycle counter on this platform, clock functions may be slow"
#define LINTEL_CLOCK_CYCLECOUNTER 0
	FATAL_ERROR("do not know how to get cycle counter on this platform");
#endif
	return ret;
    }
    static uint64_t now() {
	// TODO: deprecate this
	return getCycleCounter();
    }
  
    /////////////////////////////////////////
    // Time of day routines...

    static Tll todll() {
	struct timeval t;
	CHECKED(gettimeofday(&t,NULL)==0, "how did gettimeofday fail?");
	return (Tll)t.tv_sec * (Tll)1000000 + (Tll)t.tv_usec;
    }

    // Might want to switch to using clock_gettime() instead of
    // gettimeofday() for more accuracy; measurements indicate there
    // is no more precision the clock_gettime at least on linux
    static Tdbl tod() {
	return tod_epoch_internal(0);
    }	

    Tdbl tod_epoch() {
	return tod_epoch_internal(epoch_offset);
    }	

    Tdbl todcc_recalibrate();

    // Not a good idea to use the todcc from multiple threads; have a separate
    // Clock object for each of them, for example via Clock::perThread()

    inline Tdbl todcc_direct() {
	uint64_t cur_cc = getCycleCounter();
	uint64_t delta_cc = cur_cc - last_recalibrate_cc;
	if (cur_cc <= last_cc || delta_cc > recalibrate_interval_cycles) {
	    return todcc_recalibrate();
	} else {
	    last_cc = cur_cc;
	    return (double)cur_cc * inverse_clock_rate + cycle_count_offset;
	}
    }

    inline Tdbl todcc_incremental() {
	uint64_t cur_cc = getCycleCounter();
	uint64_t delta_cc = cur_cc - last_recalibrate_cc;
	if (cur_cc <= last_cc || delta_cc > recalibrate_interval_cycles) {
	    return todcc_recalibrate();
	} else {
	    double delta_us = (uint32_t)delta_cc * inverse_clock_rate;
	    return last_calibrate_tod + delta_us;
	}
    }

    // TODO: add check for last_cc not going backwards which also indicates
    // a core switch
    inline Tfrac todccTfrac_incremental() {
	uint64_t cur_cc = getCycleCounter();
	uint64_t delta_cc = cur_cc - last_recalibrate_cc;
	if (cur_cc <= last_cc || delta_cc > recalibrate_interval_cycles) {
	    return secondsToTfrac(todcc_recalibrate()*1e-6);
	} else {
	    double delta_us = static_cast<uint32_t>(delta_cc) 
		* inverse_clock_rate_tfrac;
	    return last_calibrate_tod_tfrac + static_cast<Tfrac>(delta_us);
	}
    }
    
    inline Tdbl todcc() {
	// PIII based machines (all I have to test) are faster in 
	// incremental mode.  Tested an opteron 2.8GhZ, also faster
	// in incremental mode
	// HPPA 2 based machines are all faster in direct mode, 
	// probably because they can use the fused multiply-add
#if LINTEL_CLOCK_CYCLECOUNTER == 0
	return todcc_recalibrate(); // generates warning
#elif defined(__i386__) || defined(i386) || defined(__i386) || defined(__x86_64__)
	return todcc_incremental();
#else
	return todcc_direct();
#endif
    }
    
    inline Tfrac todccTfrac() {
#if LINTEL_CLOCK_CYCLECOUNTER == 0
	return secondsToTfrac(todcc_recalibrate()*1e-6);
#elif defined(__i386__) || defined(i386) || defined(__i386) || defined(__x86_64__)
	return todccTfrac_incremental();
#else
	FATAL_ERROR("unimplemented");
#endif
    }

    /// How many micro seconds should elapse between recalibrations;
    /// default is 500us.  TODO: measure and figure out a smarter way
    /// to set this.
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

    static Tfrac todTfrac() {
	struct timeval t;
	CHECKED(gettimeofday(&t,NULL)==0, "how did gettimeofday fail?");
	return secMicroToTfrac(t.tv_sec,t.tv_usec);
    }	

    //////////////////////////////////////////
    /// Tfrac conversion routines...

    static Tfrac secondsToTfrac(double seconds) {
	INVARIANT(seconds < std::numeric_limits<uint32_t>::max(), 
		  boost::format("seconds %g out of bounds for conversion")
		  % seconds);
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

    // double in seconds
    static double TfracToDouble(Tfrac in) {
	return TfracToSec(in) 
	    + static_cast<uint32_t>(in & 0xFFFFFFFFULL) / 4294967296.0;
    }

    // double in seconds
    static double int64TfracToDouble(int64_t in) {
	return in >= 0 ? TfracToDouble(in) : -TfracToDouble(-in);
    }

    static struct timeval TfracToStructTimeval(Tfrac in) {
	struct timeval ret;
	ret.tv_sec = TfracToSec(in);
	ret.tv_usec = TfracToMicroSec(in);
	return ret;
    }

    ////////////////////////////////
    // Configuration functions

    enum AllowUnsafeFreqScalingOpt { 
	AUFSO_Yes, ///< silently allow fast but wrong results
	AUFSO_WarnFast, ///< like Yes but print out a warning to cerr
	AUFSO_WarnSlow, ///< like Slow but print out a warning to cerr
	AUFSO_Slow, ///< substitute tod() when frequency scaling is possible
	AUFSO_No ///< fail when frequency scaling is possible
    };
    /// \brief How should we deal with frequency scaling?
    /// Frequency scaling means that the estimated clock could be wrong
    /// by recalibrate_interval * min_freq/max_freq; by default if we
    /// can't tell that the frequency would be constant or we know 
    /// it isn't, then we use allow to determine how to behave.
    static void allowUnsafeFrequencyScaling(AllowUnsafeFreqScalingOpt allow = AUFSO_WarnFast);

    ////////////////////////////////
    // Utility functions

    // To get the per-thread functions you need to link with libLintelPThread
    static void perThreadInit(); // should be called once, from main.  will call calibrateClock()
    static Clock &perThread(); // uses default calibration routines, slow on first call.

    static void selfCheck();
    // Needed on some SMP machines as cycle counters aren't the same; used during calibration
    static void bindToProcessor(unsigned proc = 0); 
    static void unbindFromProcessor();

    // nbadgettod counts the number of times we got the time of day, but
    // switched CPUs or took forever to do it, thereby invalidating the
    // effort to calibrate the cycle counter to the time of day.
    int nrecalibrate, nbadgettod;
    static void timingTest();
    
public:
    static double clock_rate; // MhZ, assume SMP processors run at same rate.
    static Stats calibrate;
    static double inverse_clock_rate; // us/cycle
    static double inverse_clock_rate_tfrac; // tfrac/cycle
    static Tll max_recalibrate_measure_time; // see todcc_recalibrate()
    // Cycle counter calibration variables
    Tdbl last_calibrate_tod, cycle_count_offset, recalibrate_interval;
    Tfrac last_calibrate_tod_tfrac;
    uint64_t last_cc, last_recalibrate_cc, recalibrate_interval_cycles;
private:
    static pthread_key_t per_thread_clock;

    unsigned epoch_offset;
    
    static Tdbl tod_epoch_internal(unsigned epoch) {
	struct timeval t;
	CHECKED(gettimeofday(&t,NULL)==0,"how did gettimeofday fail?");
	return (Tdbl)(t.tv_sec - epoch) * (Tdbl)1000000 + (Tdbl)t.tv_usec;
    }	
};

#endif
