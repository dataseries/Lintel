/* -*-C++-*- */
/*
   (c) Copyright 1996-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief Collect statistics on a time series
*/

#ifndef LINTEL_STATSSERIES_HPP
#define LINTEL_STATSSERIES_HPP

#include <Lintel/Stats.hpp>


//////////////////////////////////////////////////////////////////////////////
// StatsSeries proper
//////////////////////////////////////////////////////////////////////////////

/// \brief Series statistics, specifying a maximum number of values and windowing the values
class StatsSeries : public Stats {
protected:
    unsigned max_lag;
    double *samples;
    double *product;
    double *firstsamples;
    unsigned sampleno;
  
public:
    StatsSeries(unsigned maxlag_in); // Create a new one
    virtual ~StatsSeries();		// Delete one
  
    virtual void add(const double value);	// Stick in a new value
    virtual void add(const Stats &stat); // attempt to merge two stats
    virtual void reset();
  
    //----Query functions
  
    virtual double autocovariance(unsigned lag) const;
    virtual double autocorrelation(unsigned lag) const {
        return autocovariance(lag)/autocovariance(0);
    }
    virtual bool iscorrelated95(unsigned lag) const;
    virtual unsigned maxlag() const { return max_lag; }
  
    //----Printing functions
  
    virtual std::string debugString() const;
    virtual Stats *another_new() const;
};

#endif /* _LINTEL_STATSSERIES_H_INCLUDED */
