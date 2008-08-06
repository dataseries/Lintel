/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Some simple functions for handling doubles
*/

#ifndef LINTEL_DOUBLE_HPP
#define LINTEL_DOUBLE_HPP

#include <algorithm>
#include <math.h>

#ifdef SYS_NT
inline bool isnan(double);
inline bool isinf(double);
#endif

#undef abs
// Performance notes for floating point: 
// on T2600:
//    NaN != 0 NaN >= 0, etc are really slow
//    v >= 0 for -inf is slightly faster than !isinf
//    isnan(NaN) and isinf(INF) are about the same speed.

class Double {
public:
    static double default_epsilon; // relative to larger of two values.
    // If handled by setEpsilonDigits/Bits, it means (approx) that the 
    // numbers agree to the first that many digits/bits
    static void setEpsilonDigits(unsigned int digits) {
	default_epsilon = 1;
	for(unsigned int i=0;i<digits;i++) {
	    default_epsilon /= 10.0;
	}
    }
    static void setEpsilonBits(unsigned int bits) {
	default_epsilon = 1;
	for(unsigned int i=0;i<bits;i++) {
	    default_epsilon /= 2.0;
	}
    }

    static double abs(double a) {
	return a<0 ? -a : a;
    }
    static bool eq(double a, double b, double epsilon) {
	double relto = std::min(abs(a),abs(b)); 
	double diff = abs(a-b);
	if (relto == 0) {
	    return diff < epsilon;
	} else {
	    return diff/relto < epsilon;
	}
    }
    static bool eq(double a, double b) {
	return eq(a,b,default_epsilon);
    }
    static bool leq(double a, double b) { 
	return a < b || eq(a,b); 
    }
    static bool lt(double a, double b) {
	return a < b && !eq(a,b);
    }
    static bool geq(double a, double b) {
	return a > b || eq(a,b);
    } 
    static bool gt(double a, double b) {
	return a > b && !eq(a,b);
    }
    static const double NaN;
    static const double Inf;
    // Following routines somewhat useful for getting Linux to give
    // the same results as other machines.  Discussion is found on:
    // http://www.srware.com/linux_numerics.txt Unfortunately, you
    // probably don't want to put linux in this mode most of the time;
    // supposedly libm on Linux depends on the extra precision
    // provided by 80 bit floats to calculate some of the complex math
    // functions.  On non-linux, these routines are no-ops.
    static void setFP64BitMode();
    static void resetFPMode();

    static void selfCheck();
};

#endif
