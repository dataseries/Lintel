/* -*-C++-*-
*******************************************************************************
*
* File:         StatsSequence.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/StatsSequence.H,v 1.5 2003/07/30 00:17:18 anderse Exp $
* Description:  
* Author:       Eric Anderson
* Created:      Wed Aug 22 12:59:52 2001
* Modified:     Thu Nov 21 18:09:40 2002 (Mahesh Kallahalla) maheshk@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __LINTEL_STATSSEQUENCE_H
#define __LINTEL_STATSSEQUENCE_H

#include <vector>
#include "Stats.H"

class StatsSequence : public Stats {
public:
    enum mode { MergeSum, MergeMean };
    StatsSequence(unsigned long max_retain);
    virtual ~StatsSequence();
    virtual void reset();
    virtual void add(const double value);
    
    virtual void printRome(int depth, std::ostream &out) const;
    void setIntervalWidth(double width);
    void setMergeMode(mode merge_mode);
    double get(unsigned int seqNum);
    unsigned int maxSeqNum();
private:
    const unsigned long max_retain;
    mode merge_mode;
    std::vector<double> values;
    double intervalWidth;
    double new_value;
    int points_per_bucket, points_remain_new_bucket;
};

#endif
