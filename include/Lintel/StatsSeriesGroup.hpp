/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    A group of stats objects ordered by time
*/

#ifndef LINTEL_STATSSERIESGROUP_HPP

#include <vector>

#include <Lintel/LintelAssert.hpp>
#include <Lintel/Stats.hpp>

// Each of the stats objects in the series will be created using
// StatsMaker.  Each object will have a name of myname-<statnum>

class StatsSeriesGroup : public Stats {
public:
    StatsSeriesGroup(const std::string &_myname, double interval);
    virtual void add(const double value); // asserts out, shouldn't be called
    virtual void add(const Stats &stat); // attempt to merge two stats
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
