/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsSeriesGroup implementation
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Double.hpp>
#include <Lintel/StatsMaker.hpp>
#include <Lintel/StatsSeriesGroup.hpp>

StatsSeriesGroup::StatsSeriesGroup(const std::string &_myname, double _interval) :
    interval(_interval), 
    cur_interval_start(Double::NaN), 
    last_time(Double::NaN),
    first_time(Double::NaN),
    myname(_myname + "-")
{
    INVARIANT(last_time != last_time, "IEEE violation??");
}

void
StatsSeriesGroup::add(const double)
{
    FATAL_ERROR("StatsSeriesGroup::add shouldn't be called.");
}

void
StatsSeriesGroup::add(const Stats &)
{
    FATAL_ERROR("unimplemented");
}

void
StatsSeriesGroup::setSeriesStart(const double timeSeq)
{
    INVARIANT(last_time != last_time, "Duplicate setSeriesStart?!");
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
    INVARIANT(timeSeq >= last_time,
	      boost::format("Time series expects data in increasing order, %.8g < %.8g")
	      % timeSeq % last_time);
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
StatsSeriesGroup::printTabular(int , std::ostream &) const
{
    FATAL_ERROR("unimplemented");
}
