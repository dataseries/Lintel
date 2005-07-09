/* -*-C++-*-
*******************************************************************************
*
* File:         StatsSeriesGroup.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/StatsSeriesGroup.C,v 1.4 2003/07/30 00:17:18 anderse Exp $
* Description:  implementation
* Author:       Eric Anderson
* Created:      Sun Dec 16 08:21:45 2001
* Modified:     Thu Nov 21 18:09:23 2002 (Mahesh Kallahalla) maheshk@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <StatsSeriesGroup.H>
#include <Double.H>
#include <StatsMaker.H>

StatsSeriesGroup::StatsSeriesGroup(const std::string &_myname, double _interval) :
    interval(_interval), 
    cur_interval_start(Double::NaN), 
    last_time(Double::NaN),
    first_time(Double::NaN),
    myname(_myname + "-")
{
    AssertAlways(last_time != last_time,("IEEE violation??\n"));
}

void
StatsSeriesGroup::add(const double value)
{
    AssertFatal(("StatsSeriesGroup::add shouldn't be called.\n"));
}

void
StatsSeriesGroup::setSeriesStart(const double timeSeq)
{
    AssertAlways(last_time != last_time,
		 ("Duplicate setSeriesStart?!\n"));
    last_time = timeSeq;
    cur_interval_start = timeSeq;
    first_time = timeSeq;
    addStat();
}

void
StatsSeriesGroup::addTimeSeq(const double value, const double timeSeq)
{
    if (last_time != last_time) {
	setSeriesStart(timeSeq);
    }
    AssertAlways(timeSeq >= last_time,
		 ("Time series expects data in increasing order, %.8g < %.8g\n",
		  timeSeq,last_time));
    last_time = timeSeq;

    // >= makes them half open intervals
    if ((timeSeq - cur_interval_start)>=interval) {
	for(cur_interval_start += interval;
	    (timeSeq - cur_interval_start) >= interval;
	    cur_interval_start += interval) {
	    stats.push_back(NULL);
	}
	addStat();
    }
    stats[stats.size()-1]->addTimeSeq(value, timeSeq);
    Stats::add(value);
}

void
StatsSeriesGroup::reset()
{
    for(unsigned int i=0;i<stats.size();i++) {
	delete stats[i];
    }
    stats.resize(0);
    cur_interval_start = last_time = first_time = Double::NaN;
}

void
StatsSeriesGroup::addStat()
{
    std::string sname = myname;

    char buf[20];
    sprintf(buf,"%d",(int)stats.size());

    sname += buf;
    Stats *newstat = StatsMaker::make(sname);
    stats.push_back(newstat);
}

void
StatsSeriesGroup::printRome(int depth, std::ostream &out) const
{
    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    Stats::printRome(depth,out);
    out << spaces << "{ seriesInterval " << interval << " }\n";
    out << spaces << "{ seriesStart " << first_time << " }\n";
    out << spaces << "{ seriesEnd " << last_time << " }\n";
    out << spaces << "{ series (\n";
    for(unsigned int i=0;i<stats.size();i++) {
	    out << spaces << "  {\n";
	if (stats[i]) {
	    stats[i]->printRome(depth+4,out);
	} else {
	    out << spaces << "    { count 0 }\n";
	}
	out << spaces << "    { interval_start " << (i*interval + first_time) << " }\n";
	out << spaces << "  }\n";
    }
    out << spaces << ")}\n";
}

void
StatsSeriesGroup::printTabular(int depth, std::ostream &out) const
{
    AssertFatal(("unimplemented\n"));
}
