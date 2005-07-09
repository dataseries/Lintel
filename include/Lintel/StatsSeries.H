/* -*-C++-*-
*******************************************************************************
*
* File:         StatsSeries.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/StatsSeries.H,v 1.9 2003/07/30 00:17:18 anderse Exp $
* Description:  Collect statistics on a time series
* Author:       Richard Golding
* Created:      Mon Jun 10 16:16:21 1996
* Modified:     Thu Aug 23 14:27:39 2001 (Alistair Veitch) aveitch@hpl.hp.com
* Language:     C++
* Package:      Lintel
*
* (C) Copyright 1996, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef _LINTEL_STATSSERIES_H_INCLUDED
#define _LINTEL_STATSSERIES_H_INCLUDED

#include "Stats.H"


//////////////////////////////////////////////////////////////////////////////
// StatsSeries proper
//////////////////////////////////////////////////////////////////////////////

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
    virtual void reset();
  
    //----Query functions
  
    virtual double autocovariance(unsigned lag) const;
    virtual double autocorrelation(unsigned lag) const;
    virtual bool iscorrelated95(unsigned lag) const;
    virtual unsigned maxlag() const;
  
    //----Printing functions
  
    virtual std::string debugString() const;
    virtual Stats *another_new() const;
};




//----------------------------------------------------------------
// Inline functions
//----------------------------------------------------------------

inline unsigned 
StatsSeries::maxlag() const
{
    return max_lag;
}

inline double
StatsSeries::autocorrelation(unsigned lag) const
{
    return autocovariance(lag)/autocovariance(0);
}

#endif /* _LINTEL_STATSSERIES_H_INCLUDED */
