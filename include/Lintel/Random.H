/* -*-C++-*- */
/*
   (c) Copyright 1998-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Random number generation
*/

#ifndef _LINTEL_RANDOM_H_INCLUDED
#define _LINTEL_RANDOM_H_INCLUDED

#include <math.h>
#include <string>
#include <vector>
#include <errno.h>
#include <assert.h>
#include <values.h>
#include "streamcompat.H"
#include "LintelAssert.H"
#include "TclInterface.H"

//----------------------------------------------------------------
// Global default seed value: a Rand will use this if no 
// explicit seed value (or other seeder) is supplied.
//----------------------------------------------------------------

class RandSeeder;			// opaque forward reference
extern RandSeeder *RandSeederDefault;	// default seed for random # generators


//////////////////////////////////////////////////////////////////////////////
// RandSeed - this stores the internal state of the 48-bit congruential
// random number generators used here.
//////////////////////////////////////////////////////////////////////////////


class RandSeed {
public:
    unsigned short seeds[3];	// 48 bits stored as 3 * 16 bits

    RandSeed()		   { reset(); };	// uses global Seeder value
    RandSeed(long s)	   { reset(s); };	// explicit seed passed in
    RandSeed(RandSeed& rs)   { reset(rs); };
    RandSeed(unsigned short s1, unsigned short s2, unsigned short s3) 
	{ reset(s1,s2,s3); };
    RandSeed(RandSeeder* sd) { reset(sd); };
    virtual ~RandSeed() {};

    void reset();			// reset the seed value
    void reset(long s);
    void reset(RandSeed& s);
    void reset(unsigned short s1, unsigned short s2, unsigned short s3);
    void reset(RandSeeder* sd);
  
protected:
    void use_time_of_day();	// calculates a seed from the time-of-day
};






//////////////////////////////////////////////////////////////////////////////
// RandSeeder
//
// This generates seed values on request using an internal Randint.
// It should be initialised once, at start of day, and then used for all
// subsequent seed settings.
//
//////////////////////////////////////////////////////////////////////////////

class RandintU;			// forward class reference

class RandSeeder {
private:
    RandintU *sr;			// source of new seed values

public:
    RandSeeder();		// initialize from time-of-day
    RandSeeder(long s);		// initialize to given value

    void reset();		// reset the seed value from time-of-day
    void reset(long s);		// reset it to a given value
    void reset(RandSeed& s);
    void reset(unsigned short s1, unsigned short s2, unsigned short s3);
    void reset(RandSeeder* sd);

    void next_seed(unsigned short* seed);	// generates a new seed value
};






//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Rand - the base class for all other random number classes returning doubles
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class Rand : virtual public TclInterface {
public:
    Rand() {};
    virtual ~Rand() {};
  
    //TIFmethod
    virtual double draw() = 0;	// Return a new random value
    //TIFmethod
    virtual double mean() = 0;	// Return the average value
};
//TIFclass abstract Rand
/* DO NOT REMOVE THIS SECTION - AUTOMATICALLY GENERATED CODE */

int Tcl_GetRandFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, Rand **ppt);
void Tcl_Declare_Rand(Tcl_Interp *, Rand *);

/* END OF AUTOMATICALLY GENERATED CODE */

//////////////////////////////////////////////////////////////////////////////
// RandC
//
// Constant distribution, which always returns the same value. It is
// provided for consistency with other distributions, and so that a "Random
// variable" can be set to a specific value.
// The default constant is 0.
//
//////////////////////////////////////////////////////////////////////////////

class RandC : public Rand { 
private:
    double constant;

public:
    RandC()         : Rand(), constant(0) {};
    //TIFmethod
    RandC(double c) : Rand(), constant(c) {};
    RandC(long c)   : Rand(), constant(c) {};
    virtual ~RandC() {};

    virtual double draw()	  { return constant; };
    virtual double mean()	  { return constant; };
};
//TIFclass RandC
/* DO NOT REMOVE THIS SECTION - AUTOMATICALLY GENERATED CODE */

int Tcl_GetRandCFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, RandC **ppt);
void Tcl_Declare_RandC(Tcl_Interp *, RandC *);

/* END OF AUTOMATICALLY GENERATED CODE */

//////////////////////////////////////////////////////////////////////////////
// RandE
//
// Exponential distribution random number generator.
// Default mean is 1.
//
//////////////////////////////////////////////////////////////////////////////

class RandE : public Rand {
private:
    RandSeed seed;
protected:
    double mean_value;		// stores the mean of the distribution

public:
    RandE()			: Rand(), seed(), mean_value(1)  {};
    RandE(double m)		: Rand(), seed(), mean_value(m)  {};
    RandE(double m, RandSeeder* s): Rand(), seed(s), mean_value(m) {};
    virtual ~RandE() {};

    virtual double draw();
    virtual double mean() { return  mean_value; };
};


//////////////////////////////////////////////////////////////////////////////
// RandG
//
// Gamma distribution.
// Default is mu = 1, sigma = 1
//
//////////////////////////////////////////////////////////////////////////////

class RandG : public Rand {
private: 
    RandSeed seed;		// internal state
    double mu;
    double sigma;
    double a,b,c,d,t,h1,h2, alpha, max_random, inv_max_random;

    void initialise();		// sets up internal state
    double gs();		// calculate result for alpha  < 0.25
    double gbh();		// calculate result for alpha >= 0.25


public:
    RandG();
    RandG(double mu_, double sigma_);
    RandG(double mu_, double sigma_, RandSeeder* sd);
    virtual ~RandG() {};

    virtual double draw();
    virtual double mean()		{ return mu; };
    virtual double variance()	{ return sigma*sigma; };
};

////////////////////////////////////////////////////////////////////
// RandGeom
//
// Geometric Random, with a default mean of 10.
//
////////////////////////////////////////////////////////////////////

class RandGeom : public Rand
{
private:
    RandSeed seed;
    double mean_value;

public:
    RandGeom()                        : Rand(), seed(), mean_value(10)  {};
    RandGeom(double m)                : Rand(), seed(), mean_value(m)  {};
    RandGeom(double m, RandSeeder* sd): Rand(), seed(sd), mean_value(m) {};
    virtual ~RandGeom() {};

    virtual double draw();
    virtual double mean()	      { return mean_value; };
};


//////////////////////////////////////////////////////////////////////////////
// RandH
//
// Histogram distribution.
// 
// Input is a cumulative density histogram, organized as two arrays of
// doubles:
//	x -> a value in the range [0,Xmax] (where Xmax>0).
//	y -> cumulative likelihood of a value <= x being seen
// where:
//      the sizes of the arrays must be identical;
//      the highest x-value must > 0;
//      the highest y-value must ~= 1.0;
//      the (0,0) point should not be supplied (it is implicit);
//      the (Xmax,1) point should be;
//
// That is:
//
//  cumulative
//  density (y) ^
//		|
//	    1.0	+-----------------> *
//		|               *   ^
//		|            *      .
//		|          *        .
//		|         *         .
//		|        *          .
//		|   *               .
//		| *                 .
//		0-+-+----+++-+--+---+----->
//				   Xmax   index (x)
//
// Calling draw() causes a uniform random number in the range [0,Xmax]
// to be drawn and traeted as a y-value.  To find the corresponding
// x-value, a binary search in the y-array is done to find the bin in
// which the associated x-value is to be found, and then linear
// interpolation used inside the bin.  The resulting x-value is
// returned as the result.  
//
// This gives us what we want, since by the nature of cumulative
// distributions, the likelihood of a given x-value is increased if
// the slope of the cumulative density function near it is high - ie,
// its associated density (likelihood) in is high.
//
// If you want a value in the range [0,1], just pick 1.0 for the
// largest x value.  
//
// If you want the lower end of the range to be different from 0, you
// must transform the input parameters and the result of draw()
// yourself.  We should eventually provide a separate Rand... class
// for that.  
//
// If you want to specify the density distribution (instead of the
// cumulative distribution), you have to convert it to the CDF
// yourself.  
//
// If you want distinct values (for example 1024, 2048 and 4096), you
// either have to round the values from draw() yourself, or you make
// a CDF which has a few very narrow regions.  This should eventually
// be automated.
//
//////////////////////////////////////////////////////////////////////////////
//The purpose of HistogramRandom and DiscreteRandom is to use lists of
//discrete values (with probabilities) or histograms (bin locations
//and probabilities) for distributions of Rome attributes, even though
//the Tcl-based parser can't handle them.

// The input format of the data is as follows:
// - First line: Number of X values
// - For each X value, one line with X value and probability.
//   These lines must be in strictly increasing order of X value.
//   None of the X values must be <0.  The difference between adjacent
//   X values must be >= 1.0.  The probability must be positive or zero.
// All values must be readable with the scanf %f descriptor.
// The probabilities will be automatically rescaled to a total of 1.
// So for example, for an IO size which shall be 10% 1024 bytes,
// 60% 2048 bytes, and 30% 8192 bytes, you can try either:
// 3                              3
// 1024 0.10                      1024 123
// 2048 0.60                      2048 738
// 8192 0.30                      8192 369

// If you want a set of discrete values, call DiscreteRandom or
// DiscreteTwoRandom.  In that case, the values in the input file are
// a list of discrete values, and the probability for each value.

// If you want a histogram, call HistogramRandom or
// HistogramTwoRandom.  In that case, the values in the input file are
// a list of the upper limits of each histogram bins.  The beginning
// of the first bin is at zero.  You can't change that; but if you
// dn't like it, add a fake first bin with a probability of zero.

// Be warned: If the generator that uses this data rounds the data,
// there may be funny interactions between the rounding, and the
// binning of the histogram.

// FIXME: These routines are temporary hacks.  The whole idea of
// storing the histogram data in an external file is distasteful.
// Also, the implementation of these functions is full of hacks.  For
// example, the value the random number generators are going to return
// is between x and x+0.1, so round appropriately.  Also, these
// functions are memory leaks: they allocate a new rand object
// everytime you call them, it is your responsability to delete it.
///////////////////////////////////////////////////////////////////////////


class RandU;			// forward reference

class RandH : public Rand {
public:
    RandH(unsigned array_size_in,	// size of the two arrays
	  const double *x_in,	        // x-axis
	  const double *y_in);	        // y-axis values (cumulative)

    RandH(unsigned array_size_in,
	  const double *x_in,
	  const double *y_in,
	  RandSeeder* seed_in);	        // source of seed


    RandH(const RandH& randH_in);
    RandH() { AssertFatal(("Someone called the RandH default constructor.")); }

    virtual ~RandH();

    virtual double draw();
    virtual double draw(double _low, double _high);
    virtual bool zeroRange() {return zero_range;}
    virtual double mean()  { return mean_value; };


  // Generate a random number generator whose distribution is in the file.
  // The content of the file is a list of discrete values.
  static RandH* DiscreteRandom(std::string filename);

  // Generate a random number generator whose distribution is in the file.
  // The content of the file is a list of histogram bins.
  static RandH* HistogramRandom(std::string filename);

protected:
    bool zero_range;
    double *x_table;		// passed-in x-values
    double *y_table;		// cumulative y-values
    unsigned table_size;	// size of the two tables
    double   mean_value;	// calculated at init time

    RandU *randForSearch;	// provides random x-value to retrieve; it
				// is a pointer rather than a local object to
				// allow forward reference to RandU class

    const static double eps; // Slop on floating-point equality tests.

    //--Helper functions
    void init(const unsigned array_size_in,	// set up tables, etc
	      const double *x_in,		// makes a copy of these arrays
	      const double *y_in);

    // Binary-search the x-array to find which bucket the
    // given value lies in; result is index of the containing bucket.
    unsigned find_region(unsigned  low_index,
			 unsigned  high_index,
			 double    value);
};
//TIFclass RandH
/* DO NOT REMOVE THIS SECTION - AUTOMATICALLY GENERATED CODE */

int Tcl_GetRandHFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, RandH **ppt);
void Tcl_Declare_RandH(Tcl_Interp *, RandH *);

/* END OF AUTOMATICALLY GENERATED CODE */

//////////////////////////////////////////////////////////////////////////////
// RandN
//
// Normal distribution.
// Default is mu = 0, sigma = 1
//
// Warning: The sigma you specify is the standard deviaton,
// not the variance (which is sigma^2)!  Remember, Rome 1 likes to use
// variances, so you have to take a sqrt.
//
//////////////////////////////////////////////////////////////////////////////

class RandN : public Rand {
private: 
    RandSeed seed;
    double mu;
    double sigma;

public:
    RandN();			// Default mean/variance
    RandN(double muu, double sigmaa);
    RandN(double muu, double sigmaa, RandSeeder* sd);
    virtual ~RandN() {};

    virtual double draw();
    virtual double mean()   { return mu; };
};


//////////////////////////////////////////////////////////////////////////////
// RandU
//
// Uniform continuous distribution in the interval [low,high),
// Default interval is 0,1 
//
//////////////////////////////////////////////////////////////////////////////

class RandU : public Rand {
private:
    RandSeed seed;		// internal seed value
    double low,high;		// records the range to return

public:
    RandU()			: Rand(), seed(), low(0), high(1) {};
    RandU(double l, double h)	: Rand(), seed(), low(l), high(h) {};
    RandU(double l, double h, RandSeeder *sd)
	: Rand(), seed(sd), low(l), high(h) {};
    virtual ~RandU() {};

    virtual double  draw();
    virtual double  mean()  { return (high - low)/2.0; };
};


inline double RandU::draw()
{
    return low + (high-low) * erand48(seed.seeds);
}; 

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Randint
//
// Base class of random number generators returning long ints
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class Randint {				// note: *not* a base class of Rand
public:
    Randint() {};
    virtual ~Randint() {};

    virtual unsigned long draw() = 0;
    virtual double        mean() = 0;
};






//////////////////////////////////////////////////////////////////////////////
// RandintU
//
// Provides integers with a uniform distribution over the
// interval [0,MAXINT_AS_FLOAT]
//
//////////////////////////////////////////////////////////////////////////////

class RandintU: public Randint {
public:
    RandSeed seed;		// internal congruential random number data

public:
    RandintU()		: Randint(), seed()  {};
    RandintU(long s)	: Randint(), seed(s) {};
    RandintU(RandSeeder* s):Randint(), seed(s) {};
    virtual ~RandintU() {};

    virtual unsigned long draw() { return nrand48(seed.seeds); }
    virtual double        mean() { return MAXINT/2.0; };
};

//////////////////////////////////////////////////////////////////////////////
// RandList
//
// This class does't generate random numbers, but generates a list of 
// numbers from the specified file.  Zack wrote this so that Pylon
// could "randomly" generate real workloads (or parts of real workloads).
//
//////////////////////////////////////////////////////////////////////////////


class RandList : public Rand {

protected:
  int current;
  std::vector<double> objects;

public:

  RandList() : Rand(), current(0) { objects.push_back(1.0);}

  RandList(const double* input, int size)  : 
      Rand(), current(0) { init(input, size);}
  RandList(const char* fileName)  : Rand(), current(0) { init(fileName);}
  RandList(const char* file_name, const char* suffix) : Rand(), current(0)
    {
      char* buffer = new char[strlen(file_name) + strlen(suffix) + 1];
      //AssertAlways(buffer, ("Could not new up a char buffer"));

      
      sprintf(buffer, "%s%s", file_name, suffix);
      init(buffer);
      delete [] buffer;
    }
    
    void init(const char* file_name);
    void init(const double* input, int size);
    virtual ~RandList() {};
    virtual double draw();
   
    virtual double mean();

};

#endif /* _LINTEL_RANDOM_H_INCLUDED */














