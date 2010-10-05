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
#include <utility>

#include <boost/config.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/CompilerMarkup.hpp>
#include <Lintel/Stats.hpp>

// A ton of cycle counter reading code can be found at
// http://www.fftw.org/cycle.h

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

// TODO-windows: how to determine processor architecture on windows?
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

class Clock {
public:
    // TODO: consider making a structure version of Tfrac that has all the
    // operators and converters on it; same space/efficiency, easier to use.

    // for both of the versions, the units are micro-seconds
    typedef long long Tll; ///< units of micro-seconds

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

    /// True if calibrateClock has already been run; note clock_rate can be < 0 if we
    /// are running in AUFSO_UseTod, in which case we are just assumed to be calibrated.
    static bool isCalibrated();

    Clock(bool allow_calibration = false);

    static uint64_t cycleCounter() {
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

    /////////////////////////////////////////
    // Time of day routines...

    // TODO-2010-04-01: remove deprecated functions
    FUNC_DEPRECATED_PREFIX static double tod() FUNC_DEPRECATED {
	return TfracToDouble(todTfrac());
    }

    FUNC_DEPRECATED_PREFIX static Tll todll() FUNC_DEPRECATED {
	struct timeval t;
	CHECKED(gettimeofday(&t,NULL)==0, "how did gettimeofday fail?");
	return (Tll)t.tv_sec * (Tll)1000000 + (Tll)t.tv_usec;
    }

    static Tfrac todTfrac() {
	struct timeval t;
	CHECKED(gettimeofday(&t,NULL)==0, "how did gettimeofday fail?");
	return secMicroToTfrac(t.tv_sec,t.tv_usec);
    }	

    Tfrac todccTfrac_recalibrate();

    // Unfortunately, at least on 32bit core2 duo (T2600), this is
    // 1.25x slower than todcc_incremental, but that's life, doubles
    // don't have enough precision, and doing the epoch thing make it
    // hard to get the code right.
    inline Tfrac todccTfrac_incremental() {
	uint64_t cur_cc = cycleCounter();
	uint64_t delta_cc = cur_cc - last_recalibrate_cc;
	if (cur_cc <= last_cc || delta_cc > recalibrate_interval_cycles) {
	    return todccTfrac_recalibrate();
	} else {
	    last_cc = cur_cc;
	    double delta_tfrac = static_cast<uint32_t>(delta_cc) * inverse_clock_rate_tfrac;
	    return last_calibrate_tod_tfrac + static_cast<uint32_t>(delta_tfrac);
	}
    }
    
    /// Calculate the time in Tfrac units using the cycle counter so the
    /// calculation runs quickly. If you want to use this from multiple 
    /// threads, you should have a separate Clock object for each of them,
    /// for examply by using Clock::perThread()
    inline Tfrac todccTfrac() {
#if LINTEL_CLOCK_CYCLECOUNTER == 0
	return todTfrac();
#elif defined(__i386__) || defined(i386) || defined(__i386) || defined(__x86_64__)
	return todccTfrac_incremental();
#else
	FATAL_ERROR("unimplemented");
#endif
    }

    /// How many seconds should elapse between recalibrations?  Default is 1ms
    void setRecalibrateIntervalSeconds(double seconds = 1e-3); 

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

    /// Extracts only the seconds portion of in
    static uint32_t TfracToSec(Tfrac in) {
	return static_cast<uint32_t>(in >> 32);
    }

    /// Extracts only the microseconds portion of in
    static uint32_t TfracToMicroSec(Tfrac in) {
	in = in & 0xFFFFFFFFULL;
	// Have to convert through a double because the conversion is
	// inexact, e.g.  1 us -> 4294 Tfrac units -> 0.99977 us
	double tmp = static_cast<double>(in) * 1.0e6 / 4294967296.0;
	return static_cast<uint32_t>(round(tmp));
    }

    /// Extracts only the nanoseconds portion of in
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

    // TODO-joe: Can the whole function be shortened as the following?
    // return (in + ((offset<0)? -1:1) * secondsToTfrac(fabs(offset)));
    static Tfrac TfracOffsetDouble(Tfrac in, double offset) {
	bool negate = (offset < 0);
	if (negate) {
	    offset = -offset;
	}
	// TODO-joe: Shouldn't the type of cooked_offset be Tfrac? It
	// will prevent the "cooked_offset = -cooked_offset;" though,
	// but I will still prefer
	// Tfrac cooked_offset = ...
	// if (negate) return in-cooked_offset;
	// else return in+cooked_offset;
	int64_t cooked_offset = secondsToTfrac(offset);
	if (negate) {
	    cooked_offset = -cooked_offset;
	}
	return in + cooked_offset;
    }

    ////////////////////////////////
    // Configuration functions

    enum AllowUnsafeFreqScalingOpt { 
	AUFSO_Yes, ///< silently allow fast but wrong results
	AUFSO_WarnFast, ///< like Yes but print out a warning to cerr
	AUFSO_WarnSlow, ///< like Slow but print out a warning to cerr
	AUFSO_Slow, ///< substitute todTfrac() when frequency scaling is possible
	AUFSO_No, ///< fail when frequency scaling is possible
	AUFSO_UseTod, ///< always use todTfrac()
    };
    /// \brief How should we deal with frequency scaling?
    /// Frequency scaling means that the estimated clock could be wrong
    /// by recalibrate_interval * min_freq/max_freq; by default if we
    /// can't tell that the frequency would be constant or we know 
    /// it isn't, then we use allow to determine how to behave.
    static void allowUnsafeFrequencyScaling(AllowUnsafeFreqScalingOpt allow = AUFSO_WarnFast);

    /// return current frequency scaling mode and whether we might have frequency scaling
    static std::pair<AllowUnsafeFreqScalingOpt, bool> getFrequencyScalingInfo();

    /// Return time in seconds for estimated latency of calling todTfrac()
    /// Useful for deciding if you will set frequency scaling mode to
    /// AUFSO_UseTod, if for example, the tod latency is << 1us.
    static double estimateTodTfracLatency();

    ////////////////////////////////
    // Utility functions

    // To get the per-thread functions you need to link with libLintelPThread
    static void perThreadInit(); // should be called once, from main.  will call calibrateClock()
    static Clock &perThread(); // uses default calibration routines, slow on first call.

    static void selfCheck();
    // Needed on some SMP machines as cycle counters aren't the same; used during calibration
    static void bindToProcessor(unsigned proc = 0); 
    static void unbindFromProcessor();

    /// how many times have we recalibrated?
    size_t nrecalibrate;
    
    /// How many cycles did it take when we got the tod, but found
    /// that either the cycle counter had gone backwards, or that it
    /// took excessively long to get the current tod.  This variable
    /// will be initialized to a Stats object on the first use, but is
    /// a pointer so that it could be replaced by a StatsQuantile if
    /// the caller is willing to tolerate occasionally much longer
    /// time spent updating the stats in exchange for the full
    /// distribution of the delay times.
    Stats *bad_get_tod_cycle_gap;
    static void timingTest();
    
public:
    static double clock_rate; // MhZ, assume SMP processors run at same rate.
    static double inverse_clock_rate_tfrac; // tfrac/cycle

    // Set in initialMeasurements()
    static uint64_t max_recalibrate_measure_time; 
    static Tfrac estimated_todtfrac_quanta;
    static uint64_t estimated_cycles_per_quanta;

    // Cycle counter calibration variables
    Tfrac last_calibrate_tod_tfrac;
    uint64_t last_cc, last_recalibrate_cc, recalibrate_interval_cycles;
private:
    static void initialMeasurements();
    static void setClockRate(double estimated_mhz);

    static pthread_key_t per_thread_clock;
};

#endif
