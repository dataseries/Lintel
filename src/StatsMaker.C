/* -*-C++-*-
*******************************************************************************
*
* File:         StatsMaker.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/StatsMaker.C,v 1.12 2005/01/20 22:56:29 wilkes Exp $
* Description:  
* Author:       Eric Anderson
* Created:      Wed Aug 22 14:10:24 2001
* Modified:     Fri Nov 28 20:41:14 2003 (John Wilkes) wilkes@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <StatsMaker.H>

std::map<std::string,StatsMaker::statDefT *> StatsMaker::statDefs;

StatsMaker::statDefT &
StatsMaker::getCreateStatDef(const std::string &statname)
{
  statDefT *ret = statDefs[statname];
  if (ret == NULL) {
    ret = new statDefT();
    statDefs[statname] = ret;
  }
  return *ret;
}
    
StatsMaker::statDefT &
StatsMaker::getDefaultStatDef(const std::string &statname)
{
    statDefT *ret = statDefs[statname];
    if (ret != NULL)
	return *ret;

    std::string tmp = statname;
    while(1) {
	int i = tmp.rfind("-");
	if (i < 0 || i >= (int)tmp.length())
	    break;
	AssertAlways(tmp[i] == '-',("?? internal error\n"));
	while(i>=0 && tmp[i] == '-') {
	    i--;
	}
	std::string tmp2 = tmp.substr(0,i+1);
	tmp = tmp2;
	if (tmp.length() > 0) {
	    ret = statDefs[tmp];
	    if (ret != NULL)
		return *ret;
	}
    }
    return getCreateStatDef("default");
}
    
void 
StatsMaker::setModeSimple(const std::string &statname)
{
  statDefT &ref = getCreateStatDef(statname);
  ref.makeStatsFn = makeSimple;
}

void
StatsMaker::setModeHistogram(const std::string &statname,
			     StatsHistogram::HistMode _histmode,
			     const int _bins,
			     const double _low,
			     const double _high,
			     const bool _scalable,
			     const bool _growable)
{
  statDefT &ref = getCreateStatDef(statname);
  ref.histmode = _histmode;
  ref.bins = _bins;
  ref.low = _low;
  ref.high = _high;
  ref.scalable = _scalable;
  ref.growable = _growable;
  ref.makeStatsFn = makeHistogram;
}

void 
StatsMaker::setModeHistogramGroup(const std::string &statname,
				  StatsHistogram::HistMode _histmode,
				  const int _bins,
				  std::vector<double> &_ranges)
{
  statDefT &ref = getCreateStatDef(statname);
  ref.histmode = _histmode;
  ref.bins = _bins;
  ref.ranges = _ranges;
  ref.makeStatsFn = makeHistogramGroup;
}

void
StatsMaker::setModeSequence(const std::string &statname,
			    const int max_retain)
{
    statDefT &ref = getCreateStatDef(statname);
    ref.bins = max_retain;
    ref.makeStatsFn = makeSequence;
}

Stats *
StatsMaker::make(const std::string &name)
{
  statDefT &ref = getDefaultStatDef(name);
  return ref.makeStatsFn(ref);
}

StatsRW *
StatsMaker::makeRW(const std::string &name)
{
  statDefT &ref = getDefaultStatDef(name);
  return ref.makeStatsRWFn(name,ref);
}

Stats *
StatsMaker::makeSimple(statDefT &def)
{
  return new Stats();
}

Stats *
StatsMaker::makeHistogram(statDefT &def)
{
  switch (def.histmode) 
    {
    case StatsHistogram::Uniform:
      return new StatsHistogramUniform(def.bins, def.low, def.high,
				       def.scalable, def.growable);
      break;
    case StatsHistogram::Log:
      return new StatsHistogramLog(def.bins, def.low, def.high, def.scalable);
      break;
    case StatsHistogram::UniformAccum:
      return new StatsHistogramUniformAccum(def.bins, def.low, def.high,
					    def.scalable, def.growable);
      break;
    case StatsHistogram::LogAccum:
      return new StatsHistogramLogAccum(def.bins, def.low, def.high);
      break;
    default:
      AssertFatal(("This didn't happen.\n"));
    }
  return NULL;
}

Stats *
StatsMaker::makeHistogramGroup(statDefT &def)
{
  return new StatsHistogramGroup(def.histmode,def.bins, def.ranges);
}

Stats *
StatsMaker::makeSequence(statDefT &def)
{
    return new StatsSequence(def.bins);
}

StatsRW *
StatsMaker::makeRWStats(const std::string &name, statDefT &def)
{
  return new StatsRW(name);
}


static StatsHistogram::HistMode
getHistMode(const std::string &histogram_type)
{
  if (histogram_type == "Uniform") {
    return StatsHistogram::Uniform;
  } else if (histogram_type == "Log") {
    return StatsHistogram::Log;
  } else if (histogram_type == "UniformAccum") {
    return StatsHistogram::UniformAccum;
  } else if (histogram_type == "LogAccum") {
    return StatsHistogram::LogAccum;
  } else {
    AssertFatal(("Unknown histogram mode %s\n",histogram_type.c_str()));
  }
  return StatsHistogram::Uniform;
}

void
setModeSimple(const std::string &statname)
{
  StatsMaker::setModeSimple(statname);
}

void 
setModeHistogram(const std::string &statname,
		 const std::string &histogram_type,
		 const int bins,
		 const double low,
		 const double high,
		 const bool scalable,
		 const bool growable)
{
  StatsMaker::setModeHistogram(statname,getHistMode(histogram_type),bins,
			       low,high,scalable,growable);
}


void 
setModeHistogramGroup(const std::string &statname,
		      const std::string &histogram_type,
		      const int bins,
		      std::list<double> &_ranges)
{
  std::vector<double> ranges(_ranges.size());
  copy(_ranges.begin(),_ranges.end(),ranges.begin());

  StatsMaker::setModeHistogramGroup(statname,getHistMode(histogram_type),
				    bins,ranges);
}

void
setModeSequence(const std::string &statname,
		const int max_retain)
{
    StatsMaker::setModeSequence(statname,max_retain);
}


