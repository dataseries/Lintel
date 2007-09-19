/* -*-C++-*- */
/*
   (c) Copyright 1993-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple statistics functions for single variables
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
    // TODO: decide whether this function should print trailing newline,
    // and if so, how it should deal with formats that are naturally multi-line
    // also whether indentation, etc should be in the interface.
    virtual void printText(std::ostream &out) const;
    friend std::ostream &operator<<(std::ostream &to, const Stats &s) {
	s.printText(to);
	return to;
    }

    virtual Stats *another_new() const;
};


#endif
