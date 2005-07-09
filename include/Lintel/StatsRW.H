/* -*-C++-*-
*******************************************************************************
*
* File:         StatsRW.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/StatsRW.H,v 1.8 2003/07/30 00:17:18 anderse Exp $
* Description:  
* Author:       Eric Anderson
* Created:      Wed Aug 22 12:59:22 2001
* Modified:     Thu Nov 21 18:09:49 2002 (Mahesh Kallahalla) maheshk@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __STATS_RW
#define __STATS_RW

#include <Stats.H>
#include <StatsSequence.H>

class StatsRW {
public:
    enum modeT { Read, Write, All };
    StatsRW(const std::string &name); // make stats objects based on StatsMaker
    StatsRW(Stats *read, Stats *write, Stats *all); // can have null entries.
  
    // Stats objects which can use the time-series information will get it;
    // others will ignore it
    void add(const double value, const modeT mode, 
	     const double timeSeq = 0);
    void setSequenceInfo(double interval_width, StatsSequence::mode mode); 
      // setSequenceInfo does something if a sub-object is a StatsSequence 
      // objects, otherwise is no-op for each object.
    virtual void printRome(int depth, std::ostream &out) const;
    virtual void printTabular(int depth, std::ostream& out) const;
private:
    void finish_init();
    Stats *read, *write, *all;
    bool autoAll; // will automatically add things into the all category
    // unless one is specifically added into there, in which case the
    // stats object is reset
    bool allStatsSeriesGroup;
};

#endif
