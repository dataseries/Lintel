/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsMaker implementation
*/

#include <Lintel/StatsMaker.hpp>

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
	SINVARIANT(tmp[i] == '-');
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
StatsMaker::makeSimple(statDefT &)
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
	FATAL_ERROR("This didn't happen.");
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
StatsMaker::makeRWStats(const std::string &name, statDefT &)
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
	FATAL_ERROR(boost::format("Unknown histogram mode %s")
		    % histogram_type.c_str());
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


