/* -*-C++-*-
/*
   (c) Copyright 1996-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Special math functions code
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "LintelAssert.H"
#include "MathSpecialFunctions.H"


double inverseErf(double y)
{
  /*  Find  x such that  erf(x) == y ; use Newton's method */
  double x0, y0, upperBound, lowerBound;
  if (y >= 1.0) {
    // %%% FIXME: There is a #define for NaN in some header file.
    // %%% It is even an ANSI or POSIX standard. Use that instead.
    #ifdef HUGE_VAL
    return HUGE_VAL;
    #else
    return 1.0 / 0.0;	// +Infinity; should really return NaN for |y|>1 
    #endif
  }
  else if (y <= -1.0) {
    #ifdef HUGE_VAL
    return -HUGE_VAL;
    #else
    return -1.0 / 0.0; // -Infinity 
    #endif
  }

  /*An initial guess, based on a formula from Abramovitz & Stegun Ch 26. */
  x0=sqrt(-2.*log(0.5*(1 - y))); 
  x0=0.7071067811865476*(x0 - (2.515517 + x0*(0.802853 + x0*0.010328))/
        (1.0 + x0*(1.432788 + x0*(0.189269 + x0*0.001308))));

  upperBound=y*1.00000000000001;
  lowerBound=y*0.99999999999999;
  y0=erf(x0);
  do {
    x0 += (y - y0)*exp(x0*x0)*0.886226925452758; 
    y0 = erf(x0); 
  } while(y0>upperBound||y0<lowerBound);
  return x0;
}

//////////////////////////////////////////////////////////////////////////////
// Data type conversion utilities
//////////////////////////////////////////////////////////////////////////////

// Return the rounding error part of a double.  This is defined
// as the difference between the argument and the nearest
// integer.  It is guaranteed to be between -1/2 and 1/2 (inclusive).
// This is an internal utility routine, used only by isDoubleIntegral.
static double roundingErrorPart(double d) {
    // Get the fractional part using a library function:
    double dummy;
    double rounding_error_part = modf(d, &dummy);

    // If it is near +1 or -1, fix it:
    if (rounding_error_part > 0.5)
        rounding_error_part -= 1.0;
    if (rounding_error_part < -0.5)
        rounding_error_part += 1.0;

    return rounding_error_part;
}


// Verify that the double argument can be converted (without rounding error)
// to an integer data type.  You get to specify the size of the integer
// data type; only the size of a long or a long long is supported.
// "Without rounding error" means that the double is really close to 
// being an actual integer.  
bool isDoubleIntegral(double d, size_t target_size) {
    // Check whether the specified target size makes sense:
    static const size_t size_long = sizeof(long);
    static const size_t size_long_long = sizeof(long long);

    AssertAlways(target_size > 0 &&
                 ( target_size == size_long ||
                   target_size == size_long_long ),
                 ( "Size %d is not valid for a long integer, valid sizes "
                   "are %d for long and %d for long long.",
                   target_size, size_long, size_long_long));

    // Determine the largest possible long values.  We have values for
    // long, but sadly we can't rely on the compiler giving us a value
    // for long long.  If we don't get one, we hand-calculate it:
    static const double long_max = LONG_MAX;
    #if defined(LONG_LONG_MAX)
        static const double long_long_max = LONG_LONG_MAX;
    #else
	// Make sure we have 8-bit bytes (because otherwise the calculation
	// below will fail):
        #if (UCHAR_MAX != 255)
            #error This machine does not have 8-bit bytes!
        #endif
	// Measure how big a long long is:
	double temp = 0.5; // Save 1 bit for the sign bit
	for (unsigned int i=0; i<sizeof(long long); ++i)
	    temp *= 256.0;
	static const double long_long_max = temp;
    #endif

    // Check that the number we were given isn't too large for the desired
    // target type:
    if (fabs(d) >= (target_size==size_long_long ? long_long_max : long_max))
        return false;

    // We arbitrarily require that for a number to be considered "purely"
    // integral, it must have at least 4 bits safety margin (i.e., if the
    // number were written in fixed point binary, the first four digits
    // after the comma are zero):
    if (fabs(roundingErrorPart(d)) >= 1.0/16.0)
        return false;

    // But wait. The above test might have succeeded simply because the
    // number is so big that it doesn't have 4 significant bits after the
    // decimal point left (remember, a double has 53 significant bits
    // in its mantissa). So we modify the original number by both adding
    // and subtracting 1/16.  At least one of these has to fail the test,
    // otherwise we are out of the safe range:
    if (fabs(roundingErrorPart(d+1.0/16.0)) < 1.0/16.0 &&
        fabs(roundingErrorPart(d-1.0/16.0)) < 1.0/16.0)
        return false;

    // OK, the number passed all the tests.
    return true;

    // To convert it to an integral type, please don't forget to round it 
    // so you don't get the wrong answer because it is actually a tiny bit
    // smaller than an integer.  You can, for example, use oneof the
    // convertDouble{,Long}Long routines below.
}

// Returns closest rounded approximation of the integral part of
// this number. Always rounds up (i.e. -1.7 is returned as -2).
// Returns utter nonsense if the number is too large to be represented
// correctly (i.e., the absolute of the mantissa is larger than
// 63 or 31 bits)
long convertDoubleLong(double d) {
    double sign = d >= 0 ? 1.0 : -1.0;
    return (long) ( d + sign / 2.0);
}    

long long convertDoubleLongLong(double d) {
    double sign = d >= 0 ? 1.0 : -1.0;
    return (long long) ( d + sign / 2.0);
}    

