/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsSequence class header
*/

#ifndef __LINTEL_STATSSEQUENCE_H
#define __LINTEL_STATSSEQUENCE_H

#include <vector>
#include "Stats.H"

class StatsSequence : public Stats {
public:
    enum mode { MergeSum, MergeMean, MergeMax };
    StatsSequence(unsigned long _max_retain, mode _merge_mode = MergeSum);
    virtual ~StatsSequence();
    virtual void reset();
    virtual void add(const double value);
    
    virtual void printRome(int depth, std::ostream &out) const;
    void setIntervalWidth(double width);
    void setMergeMode(mode merge_mode);
    double get(unsigned int seqNum);
    unsigned int maxSeqNum();
    int getPointsPerBucket() const;

private:
    unsigned long max_retain;
    mode merge_mode;
    std::vector<double> values;
    double intervalWidth;
    double new_value;
    int points_per_bucket, points_remain_new_bucket;
};

#endif
