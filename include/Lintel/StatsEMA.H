/* -*-C++-*-
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Stats class that does an exponential moving average.
*/

#ifndef LINTEL_STATSEMA_H
#define LINTEL_STATSEMA_H

#include <Stats.H>
#include <vector>

/** \brief A simple class that calculates multiple exponential moving averages

 * This class calculates multiple exponential moving averages over
 * it's inputs.  The decay rate [0..1] specifies how quickly old
 * values are aged out, so a decay rate of 0 is somewhat uninteresting as
 * the EMA will simply be the initial value.  Similarly, a decay rate
 * of 1 is also uninteresting as the EMA will always be the last input value.
 * Reasonable values are probably in the range [0.01..0.5].  As a special
 * case, if the number of input values is <= 1.0/decay_rate then we just set
 * the EMA equal to the mean.  This makes the EMA behave more sanely as the
 * first few values are added in.
 */

class StatsEMA : public Stats {
public:
    StatsEMA(double decay_rate);
    StatsEMA(const std::vector<double> &decay_rates);

    virtual ~StatsEMA();
    virtual void reset();
    virtual void add(const double value);
    
    /** ema_num matches the different decay rates specified in the vector constructor */
    double getEMA(unsigned ema_num = 0);
    double minEMA();
    double maxEMA();
private:
    void init(const std::vector<double> &decay_rates);
    void updateEMA(unsigned ema_num, const double value) {
	ema_values[ema_num] = 
	    (1.0-decay_rates[ema_num]) * ema_values[ema_num] +
	    decay_rates[ema_num] * value;
    }
    std::vector<double> ema_values, decay_rates;
    std::vector<bool> count_low;
};

#endif
