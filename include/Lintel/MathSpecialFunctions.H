/* -*-C++-*-
*******************************************************************************
*
* File:         MathSpecialFunctions.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/MathSpecialFunctions.H,v 1.8 2001/08/30 16:29:52 aveitch Exp $
* Description:  A library of special mathematical functions not found in the standard math library.
* Author:       Arif Merchant
* Created:      Fri May 24 10:47:10 PDT 1996
* Modified:     Tue Aug 21 10:11:20 2001 (Alistair Veitch) aveitch@hpl.hp.com
* Language:     C++
* Package:      Lintel
*
* (C) Copyright 1995, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef _LINTEL_MATHSPECIALFUNCTIONS_HEADER_
#define _LINTEL_MATHSPECIALFUNCTIONS_HEADER_

#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////////////

// The inverse of the Erf function, defined over the domain -1 < y < 1.
double inverseErf(double y);

// The cumulative distribution function of the unit Normal RV U;
// unitNormalCDF(x) = Prob[U < x] = (1+erf(x/sqrt(2)))/2
inline double unitNormalCDF(double x) 
  {return 0.5*(1.0+ erf(0.70710678118654752440084436210485*x));};

// The probability that the absolute value of a measurement of a
// unit normal-distributed quantity is above X.  This is the same
// as the two-side folded cumulative distribution function of the
// unit normal RV U:
inline double probAbsNormal(double x)
{ return 1. - erf(0.70710678118654752440084436210485*fabs(x)); }

// The probability density function of the unit Normal RV U;
// unitNormalPDF(x) = unitNormalCDF'(x) = exp(-x*x/2)/sqrt(2 Pi)
inline double unitNormalPDF(double x) 
  {return 0.39894228040143267793994605993438 * exp(-0.5*x*x) ;};

// The inverse of the cumulative distribution function of the unit Normal 
// RV U, also known as inversePhi.
// inverseUnitNormalCDF(x) == y  <=>  x == unitNormalCDF(y)
// inverseUnitNormalCDF(x) = sqrt(2) * inverseErf(2*x -1)
inline double inverseUnitNormalCDF(double x) 
  {return 1.4142135623730950488016887242097*(inverseErf(2.0*x - 1.0));};


//////////////////////////////////////////////////////////////////////////////
// Data type conversion utilities
//////////////////////////////////////////////////////////////////////////////

// Verify that the double argument can be converted (without rounding error)
// to an integer data type.  You get to specify the size of the integer
// data type; only the size of a long or a long long is supported.
// "Without rounding error" means that the double is really close to 
// being an actual integer.  
extern bool isDoubleIntegral(double d, size_t target_size);

// Convert the double to an integer, of type long or long long
extern long convertDoubleLong(double d);
extern long long convertDoubleLongLong(double d);

#endif
