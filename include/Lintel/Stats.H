/* -*-C++-*-
*******************************************************************************
*
* File:         Stats.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/Stats.H,v 1.31 2005/02/14 04:39:59 anderse Exp $
* Description:  Simple statistics functions for single variables
* Author:       John Wilkes
* Created:      Fri Nov 12 16:57:10 1993
* Modified:     Sun Jan 30 14:40:17 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
*
* (C) Copyright 1995, Hewlett-Packard Laboratories, all rights reserved.
*
* Derived from an original version by Chris Ruemmler and Richard Golding.
*
*******************************************************************************
*/


#ifndef __LINTEL_STATISTICS_H_INCL__
#define __LINTEL_STATISTICS_H_INCL__

#include <math.h>
#include <string>
#include "streamcompat.H"

class StatsBase {
protected:
    unsigned      reset_count;	///< Number of time reset() called
    bool          is_assigned;	///< True iff structure is not deleted
    
public:
    StatsBase();
    virtual ~StatsBase();		
    virtual bool checkInvariants() const { return is_assigned; }
    virtual void reset();		///< Reset to original values

    //----Query functions
    const unsigned resets()      const { return reset_count; };
    virtual const double min()	 const = 0;
    virtual const double max()	 const = 0;
    virtual const double mean()     const = 0;
    virtual const double stddev()   const;
    virtual const double variance() const = 0;
    virtual const double conf95()   const = 0;
    virtual const double relconf95()   const;

    /// Emit data about this value.
    virtual std::string debugString() const = 0;
};


//////////////////////////////////////////////////////////////////////////////
// Stats proper
//////////////////////////////////////////////////////////////////////////////

class Stats : public StatsBase {
protected:
    unsigned long long number;	///< How many calls to add() have occurred
    double        sum;		///< Sum of values passed in
    double        sumsq;	///< Sum of the squares of the values passed in
    double        min_value;	///< Minimum seen, else undefined
    double        max_value;	///< Minimum seen, else undefined

public:
    Stats(); 
    virtual ~Stats();
    
    /// Stick in a new value
    virtual void add(const double value);

    /// default function just re-dispatches to add
    virtual void addTimeSeq(const double value, const double timeSeq); 

    /// Clear all samples and stats.
    virtual void reset();

    // ----Query functions (not inherited from StatsBase):
    const unsigned long count()  const { return (unsigned long)number; };
    const unsigned long long countll() const { return number; };
    const double total()  const { return sum; };
    const double total_sq()  const { return sumsq; };

    // ----Query functions (inherited from StatsBase):
    virtual const double min()	 const { return min_value; };
    virtual const double max()	 const { return max_value; };
    const unsigned resets()      const { return reset_count; };
    virtual const double mean()     const;
    virtual const double variance() const;
    virtual const double conf95()   const;

    //----Printing functions
    virtual std::string debugString() const;
    virtual void printRome(int depth, std::ostream &out) const;
    virtual void printTabular(int depth, std::ostream &out) const;
    virtual Stats *another_new() const;
};

#endif
