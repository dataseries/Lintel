/* -*-C++-*-
*******************************************************************************
*
* File:         Double.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/Double.H,v 1.6 2003/07/30 00:17:16 anderse Exp $
* Description:  Some simple functions for handling doubles
* Author:       Eric Anderson
* Created:      Sat Jun  2 13:36:26 2001
* Modified:     Sat Mar 30 14:29:34 2002 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __DOUBLE_H
#define __DOUBLE_H

#include <algorithm>
#include <math.h>

#undef abs
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
};

#endif
