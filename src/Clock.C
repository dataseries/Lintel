/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Cycle counter and other timer support
*/

#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

#include <algorithm>
#include <vector>

#include <LintelAssert.H>
#include <PThread.H>
#include <Stats.H>

#include <Clock.H>
#include <Double.H>

const int min_samples = 20;
double Clock::clock_rate = -1;
double Clock::inverse_clock_rate = -1;
Clock::Tll Clock::max_recalibrate_measure_time = 10000000000LL;
Stats Clock::calibrate;

class Clock_Tll_Order {
public:
    inline bool operator() (Clock::Tll a, Clock::Tll b) {
	return a < b;
    }
};

void
Clock::calibrateClock(bool print_calibration_information, double max_conf95_rel,
		      int print_calibration_warnings_after_tries)
{
    Tdbl clock_s, clock_e;
    Tll tick_s, tick_e;

    double tmp_clock_rate;
    if (clock_rate == -1) {
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
		    AssertAlways(clock_rate == -1,("whoa, two threads calibrating at the same time?!\n"));
		    clock_rate = tmp_clock_rate;
		    AssertAlways(inverse_clock_rate == -1,
				 ("error: two threads trying to calibrate at once ?!\n"));
		    inverse_clock_rate = 1.0 / clock_rate;
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
		    AssertAlways((int)elapsed.size() > off &&
				 elapsed[off] < 10 * clock_rate,("Internal Error\n"));

		    // 2 * gives a little leeway to when it's running in 
		    // practice
		    // changed to 5 because we were having too many bad 
		    // recalibates at high load.
		    max_recalibrate_measure_time = 5 * elapsed[off];
		    AssertAlways(max_recalibrate_measure_time < 20 * tmp_clock_rate,
				 ("Internal sanity check failed, tod() takes too long, %lld > %.2f\n",
				  max_recalibrate_measure_time, 20 * tmp_clock_rate));
		    break;
		}
	    }
	    if (tmp_clock_rate > 200) {
		break;
	    }
	}
	AssertAlways(tmp_clock_rate > 200,("Unable to calibrate clock rate to %.8g relative error\n",max_conf95_rel));
	unbindFromProcessor();
	if (print_calibration_information) {
	    printf("Final calibrated clock rate is %.8g, conf95 %.8g, %ld samples\n",tmp_clock_rate,
		   calibrate.conf95(),(long)calibrate.count());
	}
    }
}

Clock::Clock(bool allow_calibration)
    : nrecalibrate(0), nbadgettod(0),
      last_calibrate_tod(0), cycle_count_offset(0), recalibrate_interval(0),
      last_cc(0), last_recalibrate_cc(0), recalibrate_interval_cycles(0)
      //, badrecalibration_delta(0.0), badrecalibration_delta_count(0), ntodzerotime(0)
{
    if (clock_rate == -1) {
	AssertAlways(allow_calibration,("you failed to call Clock::calibrateClock()!\n"));
	calibrateClock();
	AssertAlways(clock_rate > 0,("internal error\n"));
    }
    setRecalibrateInterval(100);
}

void
Clock::bindToProcessor(unsigned proc)
{
#ifdef HPUX_ACC
    AssertAlways(proc < pthread_num_processors_np(), ("Invalid processor #"));
    pthread_spu_t answer;
    int ret = pthread_processor_id_np(PTHREAD_GETFIRSTSPU_NP,&answer,0);
    AssertAlways(ret==0,("Fatal Error"));
    for (unsigned i = 0; i < proc; i++) {
	ret = pthread_processor_id_np(PTHREAD_GETNEXTSPU_NP, &answer, answer);
	AssertAlways(ret==0,("Fatal Error"));
    }
    ret = pthread_processor_bind_np(PTHREAD_BIND_FORCED_NP, &answer,
				    answer, PTHREAD_SELFTID_NP);
    AssertAlways(ret==0,("Fatal Error"));
#endif
}

void
Clock::unbindFromProcessor()
{
#ifdef HPUX_ACC
    pthread_spu_t answer;
    int ret = pthread_processor_bind_np(PTHREAD_BIND_ADVISORY_NP, &answer,
					PTHREAD_SPUFLOAT_NP, PTHREAD_SELFTID_NP);
    AssertAlways(ret==0,("Fatal Error"));
#endif
}

Clock::Tdbl
Clock::todcc_recalibrate()
{
    AssertAlways(clock_rate > 0,
		 ("Doofus, you do not appear to have setup a Clock object\n"));
    Clock::Tll start_cycle = now();
    Clock::Tdbl cur_us = tod();
    Clock::Tll end_cycle = now();
    AssertAlways(cur_us >= last_calibrate_tod,
		 ("Whoa, tod() went backwards %.2f < %.2f\n",
		  cur_us,last_calibrate_tod));
    if ((end_cycle < start_cycle) ||
	(end_cycle - start_cycle) > max_recalibrate_measure_time) {
#if 0
	// fprintf(stderr,"Whoa, bad todcc_recalibrate getting %lld took %lld to %lld = %lld, or %.3g us\n",cur_us, start_cycle, end_cycle, end_cycle - start_cycle, est_todus);
	if (end_cycle == start_cycle) {
	    ntodzerotime++;
	}
	badrecalibration_delta.add(last_recalibrate_cc - start_cycle - max_recalibrate_measure_time);
#endif
	++nbadgettod;
	// next two lines guarentee failure next time assuming the cycle
	// counter isn't reset and booting takes a little time
	cycle_count_offset = -Double::Inf; 
	last_recalibrate_cc = 0; 
    } else {
	double est_us = end_cycle * inverse_clock_rate;
	cycle_count_offset = cur_us - est_us;
	last_calibrate_tod = cur_us;
	last_recalibrate_cc = end_cycle;
	++nrecalibrate;
    }
    return cur_us;
}

void
Clock::timingTest()
{
    Clock::Tdbl measurement_time = 1000000;
    Clock::calibrateClock(true,0.001);
    Clock myclock;
    myclock.setRecalibrateInterval(100);

    printf("timing method           reps    elapsed us    us/fn     cycles/fn  ratio to tod\n");
    Clock::Tdbl start = Clock::tod();
    int nreps = 0;
    while(1) {
	nreps += 1;
	if ((Clock::tod() - start) > measurement_time) {
	    break;
	}
    }
    Clock::Tdbl end = Clock::tod();
    int clock_reps = nreps;
    double us_per_tod = (end - start)/(double)clock_reps;
    printf("tod()                 %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	   clock_reps, end - start, us_per_tod, us_per_tod / inverse_clock_rate,1.0);

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
	AssertAlways(endll - startll >= measurement_time_ll,("bad"));
	double us_per_todll = (double)(endll - startll)/(double)nreps;
	printf("todll()               %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_todll, us_per_todll / inverse_clock_rate,
	       us_per_tod/us_per_todll);
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
	AssertAlways(end - start >= measurement_time,("bad"));
	todcc_reps = nreps;
	double us_per_todcc = (double)(end - start)/(double)nreps;
	printf("todcc_direct()        %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_todcc, us_per_todcc / inverse_clock_rate,
	       us_per_tod/us_per_todcc);
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
	AssertAlways(end - start >= measurement_time,("bad"));
	double us_per_todcc = (double)(end - start)/(double)nreps;
	printf("todcc_incremental()   %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_todcc, us_per_todcc / inverse_clock_rate,
	       us_per_tod/us_per_todcc);
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
	AssertAlways(endll - startll >= measurement_time_ll,("bad"));
	double us_per_todcc = (double)(endll - startll)/(double)nreps;
	printf("todcc_direct()->ll    %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_todcc, us_per_todcc / inverse_clock_rate,
	       us_per_tod/us_per_todcc);
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
	AssertAlways(end - start >= measurement_time,("bad"));
	double us_per_divide = (double)(end - start)/(double)nreps;
	printf("todcc_divide()        %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_divide, us_per_divide / inverse_clock_rate,
	       us_per_tod/us_per_divide);
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
	AssertAlways(end - start >= measurement_time,("bad"));
	double us_per_double_mul = (double)(end - start)/(double)nreps;
	printf("double()              %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_double_mul, us_per_double_mul / inverse_clock_rate,
	       us_per_tod/us_per_double_mul);
    }

    // Test calculating times by using differencing
    {
	AssertAlways((recalibrate_interval / inverse_clock_rate) < (double)(1 << 30),
		     ("bad recalibrate\n"));
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
	AssertAlways(endll - startll >= measurement_time_ll,("bad"));
	double us_per_incremental = (double)(endll - startll)/(double)nreps;
       	printf("cc_incremental()      %9d   %8lld   %7.4g   %7.4g     %5.3g\n",
	       nreps, endll - startll, us_per_incremental, us_per_incremental / inverse_clock_rate,
	       us_per_tod/us_per_incremental);
    }

    // Test calculating times by using differencing result in double -- should be identical to todcc_incremental()
    {
	nreps = 0;
	double last_now;
	last_now = start = myclock.todcc_recalibrate();
	Clock::Tll last_cc = Clock::now();
	AssertAlways((recalibrate_interval / inverse_clock_rate) < (double)(1 << 30),
		     ("bad recalibrate\n"));
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
	AssertAlways(end - start >= measurement_time,("bad"));
	double us_per_incremental = (double)(end - start)/(double)nreps;
       	printf("incr_double()         %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_incremental, us_per_incremental / inverse_clock_rate,
	       us_per_tod/us_per_incremental);
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
	AssertAlways(end - start >= measurement_time,("bad"));
	double us_per_todcc = (double)(end - start)/(double)nreps;
	printf("fastest-of-above?     %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_todcc, us_per_todcc / inverse_clock_rate,
	       us_per_tod/us_per_todcc);
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
	AssertAlways(end - start >= measurement_time*0.9,("bad"));
	double us_per_cyclecounter = (double)(end - start)/(double)nreps;
       	printf("pure_cycle_counter    %9d   %8.0f   %7.4g   %7.4g     %5.3g\n",
	       nreps, end - start, us_per_cyclecounter, us_per_cyclecounter / inverse_clock_rate,
	       us_per_tod/us_per_cyclecounter);
    }
    printf("\n");
    Stats calibrate_error;
    int count_under_10_us = 0;
    for(int i=0;i<clock_reps;i++) {
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

    calibrate_error.reset();
    count_under_10_us = 0;
    for(int i=0;i<todcc_reps;i++) {
	Clock::Tdbl now_cc1 = myclock.todcc();
	Clock::Tdbl now_cc2 = myclock.todcc();
	calibrate_error.add((double)(now_cc2 - now_cc1));
	if ((now_cc2 - now_cc1) < 10) {
	    ++count_under_10_us;
	}
    }
    printf("Error between Clock::todcc() and Clock::todcc():\n");
    printf("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n",
	   calibrate_error.min(),calibrate_error.max(),
	   calibrate_error.mean(), calibrate_error.stddev(),
	   calibrate_error.conf95());
    printf("  %d/%d, %.6f%% under 10 us\n",count_under_10_us,
	   todcc_reps,100*(double)count_under_10_us / todcc_reps);

    calibrate_error.reset();
    for(int i=0;i<clock_reps;i++) {
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
    for(int i=0;i<clock_reps;i++) {
	Clock::Tdbl now_tod = Clock::tod();
	Clock::Tdbl now_cc = myclock.todcc_incremental();
	calibrate_error.add((double)(now_cc - now_tod));
    }
    printf("Error between Clock::tod() and Clock::todcc_incremental():\n");
    printf("  [min..max] [%.5g .. %.5g] mean %.3g +- %.3g, conf95 %.4g\n",
	   calibrate_error.min(),calibrate_error.max(),
	   calibrate_error.mean(), calibrate_error.stddev(),
	   calibrate_error.conf95());

}

