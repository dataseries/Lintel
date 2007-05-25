/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsSequence implementation
*/

#include <math.h>

#include <Lintel/LintelAssert.H>
#include <Lintel/StatsSequence.H>

StatsSequence::StatsSequence(unsigned long _max_retain, mode _merge_mode)
    : Stats(), max_retain(_max_retain)
{
    AssertAlways(max_retain > 0,("Max Retain must be > 0\n"));
    AssertAlways((max_retain % 2) == 0,("Max Retain must be even!\n"));
    merge_mode = _merge_mode;
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
	    double x = values[2*i], y = values[2*i+1];
	    switch(merge_mode) 
		{
		case MergeSum: values[i] = x + y;
		    break;
		case MergeMean: values[i] = (x+y) / 2;
		    break;
		case MergeMax: values[i] = (x>y) ? x : y;
		    break;
		default:
		    AssertFatal(("I didn't get here.\n"));
		}
	}
	values.resize(max_retain/2);
	points_per_bucket *= 2;
	points_remain_new_bucket = points_per_bucket;
    }
    if (merge_mode == MergeMax) { new_value = (new_value > value) ? new_value : value; }
    else { new_value += value; }
    points_remain_new_bucket -= 1;
    if (points_remain_new_bucket == 0) {
	switch(merge_mode)
	    {
	    case MergeSum: 
		break;
	    case MergeMean: new_value = new_value / (double)points_per_bucket;
		break;
	    case MergeMax: 
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
    bool extra = (points_remain_new_bucket < points_per_bucket);
    out << spaces << "  { values (";
    for(unsigned int k=0;k<values.size();k++) {
	out << values[k] << " ";
    }
    if (extra) {
	switch (merge_mode) 
	    {
	    case MergeSum: out << new_value;
		break;
	    case MergeMean: out << new_value / (double)points_per_bucket;
		break;
	    case MergeMax: out << new_value;
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

int
StatsSequence::getPointsPerBucket() const
{
    return points_per_bucket;
}
