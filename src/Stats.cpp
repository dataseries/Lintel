/* -*-C++-*-
   (c) Copyright 1993-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple statistics functions for single variables
*/

#include <stdlib.h>
#include <cstdio>
#include <string>
#include <float.h>
#include <string.h>

#include <boost/format.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Stats.hpp>
#include <Lintel/Double.hpp>

///////////////////////////////////////////////////////////////////////////
// Functions specific to the Statistics base class
///////////////////////////////////////////////////////////////////////////

StatsBase::StatsBase()
      : reset_count(0),		// make sure reset has a valid initial value
	is_assigned(true)	// "successfully initialized"
{
    reset();			// Do most of the work here
    reset_count = 0;		//  -- since this really is the first time
};


StatsBase::~StatsBase()
{
    DEBUG_SINVARIANT(checkInvariants());
    is_assigned = false;
};

bool StatsBase::checkInvariants() const
{ 
    return is_assigned; 
}

void
StatsBase::reset()
{
    DEBUG_SINVARIANT(checkInvariants());
    reset_count++;
}

double
StatsBase::stddev() const
{
    DEBUG_SINVARIANT(checkInvariants());
    double sigsq = variance();

    if (sigsq <= 0.0)
	return 0.0;

    DEBUG_SINVARIANT(sigsq > 0.0);
    return sqrt(sigsq);
}

double 
StatsBase::relconf95() const
{
    DEBUG_SINVARIANT(checkInvariants());
    return conf95()/mean();
}


///////////////////////////////////////////////////////////////////////////
// Functions related to the Stats singe-variable statistic class
///////////////////////////////////////////////////////////////////////////

// Create a new one
//
Stats::Stats()
      : StatsBase()
{
  reset();
};


Stats::~Stats()	// Delete a value
{};



void Stats::reset()
{
    DEBUG_SINVARIANT(checkInvariants());
    StatsBase::reset();
    number = 0;
    sum = 0.0;
    sumsq = 0.0;
    min_value = Double::Inf;
    max_value = -Double::Inf;
}



void Stats::add(const double value)
{
    // TODO: make this a DEBUG_INVARIANT so it goes away if debugging
    // invariants are disabled.
    INVARIANT(value == value,
	      "You tried to add a NaN to the stats object."); 
    ++number;
    sum += value;
    sumsq += value*value;

    if (value < min_value)
	min_value = value;
    if (value > max_value)
	max_value = value;
}

void
Stats::add(const Stats &stat)
{
    number += stat.number;
    sum += stat.sum;
    sumsq += stat.sumsq;

    if (stat.min_value < min_value)
	min_value = stat.min_value;
    if (stat.max_value > max_value)
	max_value = stat.max_value;
}

void
Stats::addTimeSeq(const double value, const double timeSeq)
{
    this->add(value);
}

// Accessor functions

double Stats::mean() const
{
    DEBUG_SINVARIANT(checkInvariants());
    if (number == 0)
	return 0.0;
    else
	return double(sum)/double(number);
};


double Stats::variance() const
{
    DEBUG_SINVARIANT(checkInvariants());
    if (number == 0) return 0.0;
    double m = double(mean());
    return double(sumsq)/double(number) - m*m;
}



double Stats::conf95() const
{
    DEBUG_SINVARIANT(checkInvariants());
    if (number == 0) return DBL_MAX; // **** Should really be NaN if count==0
    DEBUG_SINVARIANT(number > 0);
    return 1.96*stddev()/sqrt((double)number);
}



// Summarize contents as a string

std::string Stats::debugString() const
{
    // XXX rewrite using std::ostrstream.
    DEBUG_SINVARIANT(checkInvariants());

    char line[1024];
    if (count() == 0)
	sprintf(line, "count 0");
    else
	sprintf(line,
		"count %ld mean %G stddev %G var %G"
		" 95%%conf %G rel95%%conf %G min %G max %G",
		count(), mean(), stddev(), variance(),
		conf95(), relconf95(), min(), max());
    DEBUG_SINVARIANT(strlen(line) < 1024);
    return std::string(line);
};


void
Stats::printRome(int depth, std::ostream &out) const
{
    DEBUG_SINVARIANT(checkInvariants());

    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    out << spaces << "{ count " << countll() << " }\n";
    if (count() > 0) {
	out << spaces << "{ min " << min() << " }\n";
	out << spaces << "{ max " << max() << " }\n";
	out << spaces << "{ mean " << mean() << " }\n";
	out << spaces << "{ stddev " << stddev() << " }\n";
	out << spaces << "{ variance " << variance() << " }\n";
	out << spaces << "{ conf95 " << conf95() << " }\n";
	out << spaces << "{ total " << total() << " }\n";
	out << spaces << "{ total_sq " << total_sq() << " }\n";
    }
}

void
Stats::printTabular(int depth, std::ostream &out) const
{
  DEBUG_SINVARIANT(checkInvariants());

  std::string spaces;
  for(int i = 0; i < depth; i++) {
    spaces += " ";
  }

  out << spaces << "count " << countll() << "\n";
  if (count() > 0) {
    out << spaces << "min " << min() << "\n";
    out << spaces << "max " << max() << "\n";
    out << spaces << "mean " << mean() << "\n";
    out << spaces << "stddev " << stddev() << "\n";
    out << spaces << "variance " << variance() << "\n";
    out << spaces << "conf95 " << conf95() << "\n";
    out << spaces << "total " << total() << "\n";
    out << spaces << "total_sq " << total_sq() << "\n";
  }
  // This is kind of a hack, but I had problems when the printout of 
  // Stats changed for an empty list.
  else
    {
      out << spaces << "min " << min() << "\n";
      out << spaces << "max " << max() << "\n";
      out << spaces << "mean " << 0 << "\n";
      out << spaces << "stddev " << 0 << "\n";
      out << spaces << "variance " << 0 << "\n";
      out << spaces << "conf95 " << 0 << "\n";
      out << spaces << "total " << total() << "\n";
      out << spaces << "total_sq " << total_sq() << "\n";
    }
}
  
void Stats::printText(std::ostream &out) const
{
    out << boost::format("count=%lld, mean=%.8g, stddev=%.8g, min=%.8g, max=%.8g")
	% countll() % mean() % stddev() % min() % max();
}


Stats *
Stats::another_new() const
{
    return new Stats();
}


