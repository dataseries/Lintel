/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsEMA implementation
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StatsEMA.hpp>

using namespace std;

StatsEMA::StatsEMA(double decay_rate)
{
    vector<double> decay_rates;
    decay_rates.push_back(decay_rate);
    init(decay_rates);
}

StatsEMA::StatsEMA(const vector<double> &decay_rates)
{
    init(decay_rates);
}

StatsEMA::~StatsEMA()
{
}

void
StatsEMA::reset()
{
    FATAL_ERROR("unimplemented");
}

void
StatsEMA::add(const double value)
{
    Stats::add(value);
    for(unsigned i = 0;i<ema_values.size();++i) {
	if (count_low[i]) {
	    if ((double)countll() > 1.0 / decay_rates[i]) {
		count_low[i] = false;
		updateEMA(i,value);
	    } else {
		ema_values[i] = mean();
	    }
	} else {
	    updateEMA(i,value);
	}
    }
}

void
StatsEMA::add(const Stats &stat)
{
    FATAL_ERROR("unimplemented");
}

double
StatsEMA::getEMA(unsigned ema_num)
{
    INVARIANT(ema_num < ema_values.size(),
	      boost::format("invalid ema_num %d specified")
	      % ema_num);
    return ema_values[ema_num];
}

double 
StatsEMA::minEMA()
{
    double ret = ema_values[0];
    for(unsigned i = 1; i<ema_values.size(); ++i) {
	if (ema_values[i] < ret) 
	    ret = ema_values[i];
    }
    return ret;
}

double 
StatsEMA::maxEMA()
{
    double ret = ema_values[0];
    for(unsigned i = 1; i<ema_values.size(); ++i) {
	if (ema_values[i] > ret) 
	    ret = ema_values[i];
    }
    return ret;
}

void
StatsEMA::init(const std::vector<double> &_decay_rates)
{
    INVARIANT(_decay_rates.size() > 0,
	      "no decay rates specified?!");
    decay_rates = _decay_rates;
    for(unsigned i = 0; i<decay_rates.size(); ++i) {
	INVARIANT(decay_rates[i] > 0 && decay_rates[i] < 1,
		  boost::format("invalid decay rate %.9g specified")
		  % decay_rates[i]);
    }
    ema_values.resize(decay_rates.size(),0);
    count_low.resize(decay_rates.size(),true);
}
