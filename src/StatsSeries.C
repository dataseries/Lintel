/* -*-C++-*-
/*
   (c) Copyright 1996-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Collect statistics on a time series
*/

#include <stdlib.h>
#include <stdio.h>

#include "LintelAssert.H"
#include "StatsSeries.H"

// The code for this time series analysis was derived from _Time
// Series Analysis: Forecasting and Control_ (3rd ed.), Box, Jenkins, and
// Reinsel, Prentice-Hall, 1994, section 2.1.5 (Estimation of
// autocovariance and autocorrelation functions).
// The code was validated against their example in Table 2.1.  The sequence
// for that is:

// 47 64 23 71 38 64 55 41 59 48 71 35 57 40 58 44 80 55 37 74 51 57 50
// 60 45 57 50 45 25 59 50 71 56 74 50 58 45 54 36 54 48 55 45 57 50 62
// 44 64 43 52 38 59 55 41 53 49 34 35 54 45 68 38 50 60 39 59 40 57 54
// 23

// and the resulting autocorrelations match theirs (theirs are
// reported to two decimal places:

// autocorr(1): -0.389878
// autocorr(2): 0.304394
// autocorr(3): -0.165555
// autocorr(4): 0.070719
// autocorr(5): -0.097039
// autocorr(6): -0.047058
// autocorr(7): 0.035373
// autocorr(8): -0.043458
// autocorr(9): -0.004796
// autocorr(10): 0.014393
// autocorr(11): 0.109917
// autocorr(12): -0.068778
// autocorr(13): 0.148034
// autocorr(14): 0.035769
// autocorr(15): -0.006678


StatsSeries::StatsSeries(unsigned maxlag_in)
: Stats(),
  max_lag(maxlag_in),
  samples(NULL),
  product(NULL)
{
  Assert(1, max_lag > 0);
  samples = new double[max_lag+1];
  Assert(1, samples != NULL);
  product = new double[max_lag+1];
  Assert(1, product != NULL);
  firstsamples = new double[max_lag+1];
  Assert(1, firstsamples != NULL);
  reset();
}

StatsSeries::~StatsSeries()
{
  delete samples;
  samples = NULL;
  delete product;
  product = NULL;
  delete firstsamples;
  firstsamples = NULL;
  max_lag = 0;
}

void
StatsSeries::add(const double value)
{
  // record the general statistics
  Stats::add(value);

  // insert this value into the list of samples
  for (unsigned i=max_lag; i>0; i--) {
    samples[i] = samples[i-1];
  }
  samples[0] = value;

  // compute products for autocovariance
  if (sampleno < max_lag) {
    unsigned lag;
    for (lag=0; lag<=sampleno; lag++) {
      product[lag] += value*samples[lag];
    }
    for (lag = sampleno+1; lag <= max_lag; lag++) {
      firstsamples[lag] += value;
    }
  } else {
    for (unsigned lag=0; lag<=max_lag; lag++) {
      product[lag] += value*samples[lag];
    }
  }
  
  // get ready for the next sample
  sampleno++;
}

void
StatsSeries::reset()
{
  Stats::reset();
  for (unsigned i=0; i<=max_lag; i++) {
    samples[i] = 0;
    product[i] = 0;
    firstsamples[i] = 0;
  }
  sampleno = 0;
}

double
StatsSeries::autocovariance(unsigned lag) const
{
  Assert(1, lag>=0);
  Assert(1, lag<=max_lag);
  
  double lastsamples;
  lastsamples = 0.0;
  for (unsigned i=0; i<lag; i++) {
    lastsamples += samples[i];
  }

  return (product[lag] - mean()*(sum + sum - lastsamples - firstsamples[lag]) + mean()*mean()*(sampleno - lag - 1))/(1.0*sampleno) + mean()*mean()/(double)sampleno;
}


// Hypothesis test:  are the two random variables measured by the
// Correlation in fact correlated?  (Note that this assumes both variables
// are samples of random variable; if one is a controlled variable, then
// this hypothesis test doesn't apply.)  Specifically, the null hypothesis
// is that (the correlation coefficient of the actual process) rho = 0;
// the alternative is rho != 0, at a 95% confidence level.
// Reference: John E. Freund, Mathematical Statistics, Prentice-Hall, 1962,
// pp 308-12.

bool
StatsSeries::iscorrelated95(unsigned lag) const
{
  Assert(1,checkInvariants());
  
  double z;			// confidence level for rejecting 
				// linear correlation hypothesis
  double r;			// autocorrelation coeff estimate

  // cannot determine a correlation with less than four samples
  if (count() <= 3) {
    return false;
  }
  
  r = autocorrelation(lag);
  z = (sqrt(count() - 3.0)/2.0)*log((1.0+r)/(1.0-r));
  return z >= 1.96;
}

// Summarize contents as a string

std::string
StatsSeries::debugString() const
{
    Assert(1,checkInvariants());

    char line[1024];
    if (count() == 0)
	snprintf(line, sizeof line-1, "count 0");
    else
	snprintf(line, sizeof line-1, 
		 "count %ld mean %G stddev %G var %G"
		 " 95%%conf %G rel95%%conf %G min %G max %G samples %d maxlag %d",
		 count(), mean(), stddev(), variance(),
		 conf95(), relconf95(), min(), max(), sampleno, maxlag());
    return std::string(line);
};


Stats *
StatsSeries::another_new() const
{
    return new StatsSeries(max_lag); 
}

