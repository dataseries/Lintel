/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsSequence class header
*/

#ifndef LINTEL_STATSSEQUENCE_HPP
#define LINTEL_STATSSEQUENCE_HPP

#include <vector>
#include <Lintel/Stats.hpp>

class StatsSequence : public Stats {
public:
    enum mode { MergeSum, MergeMean, MergeMax };
    StatsSequence(unsigned long _max_retain, mode _merge_mode = MergeSum);
    virtual ~StatsSequence();
    virtual void reset();
    virtual void add(const double value);
    virtual void add(const Stats &stat); // attempt to merge two stats
    
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
