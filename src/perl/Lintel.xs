#undef bool

#ifdef __hpux
#define __HPACC_USING_MULTIPLIES_IN_FUNCTIONAL
#include <functional>
#undef __HPACC_USING_MULTIPLIES_IN_FUNCTIONAL

#include <iostream.h>
#include <limits>
#include <string>
#endif

#include <Stats.H>
#include <StatsHistogram.H>
#include <MersenneTwisterRandom.H>

// Conflicts between perl and tcl
#undef STRINGIFY
extern "C" {
#include <EXTERN.h>
#include <perl.h>
#undef list
#include "XSUB.h"
}


MODULE = Lintel		PACKAGE = Lintel::Stats

PROTOTYPES: ENABLE

Stats *
Stats::new()

unsigned long
Stats::count()

double
Stats::total()

double
Stats::total_sq()

void
Stats::add(value)
	double value

double 
Stats::min()

double
Stats::max()

double
Stats::mean()

double
Stats::variance()

double
Stats::stddev()
	CODE:
	RETVAL = THIS->stddev();
	OUTPUT:
	RETVAL

double
Stats::conf95()

MODULE = Lintel		PACKAGE = Lintel::Histogram

long
StatsHistogram::binCount(bin)
	int bin
	CODE:
	RETVAL = (*THIS)[bin];
	OUTPUT:
	RETVAL

long
StatsHistogram::numBins()

double
StatsHistogram::binlow(bin)
	int bin

double
StatsHistogram::bincenter(bin)
	int bin

double
StatsHistogram::binhigh(bin)
	int bin

MODULE = Lintel		PACKAGE = Lintel::Histogram::Uniform

StatsHistogramUniform *
StatsHistogramUniform::new(bins, low, high, is_scalable = 0)
	int bins
	double low
	double high
	int is_scalable

MODULE = Lintel		PACKAGE = Lintel::Histogram::Log

StatsHistogramLogAccum *
StatsHistogramLogAccum::new(bins, low, high)
	int bins
	double low
	double high

double
StatsHistogramLogAccum::binMean(bin)
	int bin
	CODE:
	{
	   double binCount = (*THIS)[bin];
	   if (binCount == 0) 
	      RETVAL = 0;
	   else
	      RETVAL = THIS->value(bin) / binCount;
	}
	OUTPUT:
	RETVAL
	
MODULE = Lintel		PACKAGE = Lintel::Random::MersenneTwister

MersenneTwisterRandom *
MersenneTwisterRandom::new(seed = 0)
	unsigned int seed

void
MersenneTwisterRandom::init(seed)
	unsigned int seed

unsigned int
MersenneTwisterRandom::randInt()

double
MersenneTwisterRandom::randDouble()
