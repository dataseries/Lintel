/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    create stats objects
*/

#ifndef LINTEL_STATSMAKER_HPP
#define LINTEL_STATSMAKER_HPP

#include <list>
#include <map>

#include <Lintel/Stats.hpp>
#include <Lintel/StatsHistogram.hpp>
#include <Lintel/StatsRW.hpp>
#include <Lintel/StatsSequence.hpp>

// make(RW) will create a stats object of the appropriate type based
// on the name provided; It will check to see if the name was
// registered (by the setMode* functions), and will then repeatedly
// strip the rightmost group of -'s and everything after it from the
// name and try the lookup again.  if it runs out of -'s to remove, it
// will look up the name "default" to specify the default type of
// object to create

class StatsMaker {
public:
    static void setModeSimple(const std::string &statname);

    static void setModeHistogram(const std::string &statname,
				 StatsHistogram::HistMode histmode,
				 const int bins,
				 const double low,
				 const double high,
				 const bool scalable = false,
				 const bool growable = false);

    static void setModeHistogramGroup(const std::string &statname,
				      StatsHistogram::HistMode histmode,
				      const int bins,
				      std::vector<double> &ranges);

    static void setModeSequence(const std::string &statname,
				const int max_retain);

    // Eventually add in something to control reporting modes ...

    static Stats *make(const std::string &name);
    static StatsRW *makeRW(const std::string &name);

    // Doesn't seem like these should have to be public, but aCC claims so.
    struct statDefT;
    typedef Stats * (*makeStatsFnType)(statDefT &def);
    typedef StatsRW * (*makeStatsRWFnType)(const std::string &name,statDefT &def);
    struct statDefT {
	StatsHistogram::HistMode histmode;
	int bins;
	double low, high;
	bool scalable, growable;
	std::vector<double> ranges;
 	makeStatsFnType makeStatsFn;
	makeStatsRWFnType makeStatsRWFn;
	statDefT() { makeStatsFn = makeSimple; makeStatsRWFn = makeRWStats; }
    };
    // g++ forces us to make these public also :(
    static Stats *makeSimple(statDefT &def);
    static StatsRW *makeRWStats(const std::string &name,statDefT &def);
private:
    static Stats *makeHistogram(statDefT &def);
    static Stats *makeHistogramGroup(statDefT &def);
    static Stats *makeSequence(statDefT &def);

    static std::map<std::string,statDefT *> statDefs;

    static statDefT &getCreateStatDef(const std::string &statname);
    static statDefT &getDefaultStatDef(const std::string &statname);
};

//TIFfunction
void setModeSimple(const std::string &statname);

// histogram_type is a string for the TIF methods.

//TIFfunction
void setModeHistogram(const std::string &statname,
		      const std::string &histogram_type,
		      const int bins,
		      const double low,
		      const double high,
		      const bool scalable = false,
		      const bool growable = false);

// ranges is a list because that's what the TIF methods support
//TIFfunction
void setModeHistogramGroup(const std::string &statname,
			   const std::string &histogram_type,
			   const int bins,
			   std::list<double> &ranges);

//TIFfunction
void setModeSequence(const std::string &statname,
		     const int max_retain);

#endif
  
