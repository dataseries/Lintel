/* -*-C++-*-
*******************************************************************************
*
* File:         StatsSeriesGroup.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/StatsSeriesGroup.H,v 1.4 2003/07/30 00:17:18 anderse Exp $
* Description:  A group of stats objects ordered by time
* Author:       Eric Anderson
* Created:      Sun Dec 16 08:24:37 2001
* Modified:     Thu Nov 21 18:09:10 2002 (Mahesh Kallahalla) maheshk@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __LINTEL_STATSSERIESGROUP_H

#include <vector>
#include <Stats.H>
#include <LintelAssert.H>

// Each of the stats objects in the series will be created using
// StatsMaker.  Each object will have a name of myname-<statnum>

class StatsSeriesGroup : public Stats {
public:
    StatsSeriesGroup(const std::string &_myname, double interval);
    virtual void add(const double value); // asserts out, shouldn't be called
    virtual void addTimeSeq(const double value, const double timeSeq);

    virtual void reset();

    const int getSeriesLength() { return stats.size(); }
    const Stats &getSeriesStat(const int statnum) {
	AssertAlways(statnum >= 0 && statnum < (int)stats.size(),
		     ("statnum %d is out of bounds\n",statnum));
	return *(stats[statnum]);
    }

    virtual void printRome(int depth, std::ostream &out) const;
    virtual void printTabular(int depth, std::ostream &out) const; 
    void setSeriesStart(const double timeSeq);
private:
    void addStat();
    std::vector<Stats *> stats;
    double interval;
    double cur_interval_start, last_time, first_time;
    const std::string myname;
};

#endif
