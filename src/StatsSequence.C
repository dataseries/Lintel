/* -*-C++-*-
*******************************************************************************
*
* File:         StatsSequence.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/StatsSequence.C,v 1.7 2003/07/30 00:17:18 anderse Exp $
* Description:  
* Author:       Eric Anderson
* Created:      Wed Aug 22 12:53:50 2001
* Modified:     Thu Nov 21 18:09:44 2002 (Mahesh Kallahalla) maheshk@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include "StatsSequence.H"

#include "LintelAssert.H"
#include <math.h>

StatsSequence::StatsSequence(unsigned long _max_retain)
    : Stats(), max_retain(_max_retain)
{
    AssertAlways(max_retain > 0,("Max Retain must be > 0\n"));
    AssertAlways((max_retain % 2) == 0,("Max Retain must be even!\n"));
    merge_mode = MergeSum;
    intervalWidth = 1;
    points_per_bucket = 1;
    reset();
}

StatsSequence::~StatsSequence()
{
}

void
StatsSequence::reset()
{
    Stats::reset();
    values.resize(0);
    new_value = 0;
    points_per_bucket = 1;
    points_remain_new_bucket = 1;
}

void
StatsSequence::add(const double value)
{
    Stats::add(value);
    if (values.size() == max_retain) {
	Assert(1,points_remain_new_bucket == points_per_bucket 
	         && new_value == 0);
	for(unsigned int i=0;i<max_retain/2;i++) {
	    switch(merge_mode) 
		{
		case MergeSum: values[i] = values[2*i] + values[2*i+1];
		    break;
		case MergeMean: values[i] = (values[2*i] + values[2*i+1])/2;
		    break;
		default:
		    AssertFatal(("I didn't get here.\n"));
		}
	}
	values.resize(max_retain/2);
	points_per_bucket *= 2;
	points_remain_new_bucket = points_per_bucket;
    }
    new_value += value;
    points_remain_new_bucket -= 1;
    if (points_remain_new_bucket == 0) {
	switch(merge_mode)
	    {
	    case MergeSum: 
		break;
	    case MergeMean: new_value = new_value / (double)points_per_bucket;
		break;
	    default:
		AssertFatal(("I didn't get here.\n"));
	    }
	values.push_back(new_value);
	Assert(1,values.size() <= max_retain);
	new_value = 0;
	points_remain_new_bucket = points_per_bucket;
    }
}

void
StatsSequence::printRome(int depth, std::ostream &out) const
{
    Stats::printRome(depth,out);

    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    out << spaces << "{ sequence {\n";
    out << spaces << "  { intervalWidth " << intervalWidth * points_per_bucket << " }\n";
    int numrescales,j;
    for(numrescales=0,j=1;j<points_per_bucket;j*=2) {
	numrescales += 1;
    }
    out << spaces << "  { numRescales " << numrescales << " }\n";
    out << spaces << "  { values (";
    for(unsigned int k=0;k<values.size();k++) {
	out << values[k] << " ";
    }
    if (points_remain_new_bucket < points_per_bucket) {
	switch (merge_mode) 
	    {
	    case MergeSum: out << new_value;
		break;
	    case MergeMean: out << new_value / (double)points_per_bucket;
		break;
	    default:
		AssertFatal(("I didn't get here.\n"));
	    }
    }
    out << ")}\n";
    out << spaces << "}}\n";
}

void
StatsSequence::setIntervalWidth(double _width)
{
    AssertAlways(values.size() == 0,("Can't set interval width after adding value.\n"));
    intervalWidth = _width;
}

void 
StatsSequence::setMergeMode(mode _merge_mode)
{
    AssertAlways(values.size() == 0,("Can't set merge mode after adding value.\n"));
    merge_mode = _merge_mode;
}

double
StatsSequence::get(unsigned int seqNum)
{
    // ***FIXME*** should handle getting the value for the "current" bucket.
    // ***FIXME*** then rewrite printRome, maxSeqNum as appropriate.
    AssertAlways(seqNum < values.size(),
		 ("seqNum %d >= %d\n",seqNum,values.size()));
    return values[seqNum];
}

unsigned int
StatsSequence::maxSeqNum()
{
    return values.size();
}



