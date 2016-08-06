/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/


#include <limits>
#include <string>

#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/Stats.hpp>
#include <Lintel/StatsHistogram.hpp>
#include <Lintel/StatsQuantile.hpp>

extern "C" {
#include <EXTERN.h>

// Defined in perl.h and Lintel/CompilerMarkup.hpp
#undef LIKELY
#undef UNLIKELY
#include <perl.h>

#undef list
#include "XSUB.h"

// perl 5.18 with c++ compiler doesn't define Perl___notused in dNOOP, it
// defines it as (void)0.  This makes the later macro expansion of
// PERL_UNUSED_VAR(Perl___notused) fail because the variable is undefined.
// Since we're already getting lots of unused parameter warnings, just
// suppress this entirely by changing the macro definition.

#undef PERL_UNUSED_VAR
#define PERL_UNUSED_VAR(x)
}


MODULE = LintelPerl		PACKAGE = Lintel::Stats

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

double
Stats::conf95()

MODULE = LintelPerl		PACKAGE = Lintel::Histogram

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

MODULE = LintelPerl		PACKAGE = Lintel::Histogram::Uniform

StatsHistogramUniform *
StatsHistogramUniform::new(bins, low, high, is_scalable = 0)
	int bins
	double low
	double high
	int is_scalable

MODULE = LintelPerl		PACKAGE = Lintel::Histogram::Log

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
	
MODULE = LintelPerl             PACKAGE = Lintel::StatsQuantile

StatsQuantile *
StatsQuantile::new(quantile_error = 0.01, nbound = 1.0e9)
        double quantile_error
	double nbound				 

double
StatsQuantile::getQuantile(quantile)
       double quantile

MODULE = LintelPerl		PACKAGE = Lintel::Random::MersenneTwister

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
