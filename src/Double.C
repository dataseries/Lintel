/* -*-C++-*-
*******************************************************************************
*
* File:         Double.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/Double.C,v 1.7 2005/02/14 04:36:52 anderse Exp $
* Description:  C++ file for the static variable
* Author:       Eric Anderson
* Created:      Sat Jun  2 16:58:22 2001
* Modified:     Fri Jan  7 23:56:34 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#if __GNUC__ == 2
#define _ISOC9X_SOURCE 1
#endif

#ifdef __linux__
#include <fpu_control.h>
#endif

#include <math.h>
#include <Double.H>
#include <LintelAssert.H>

double Double::default_epsilon = 1e-12;

const double Double::NaN = NAN;
const double Double::Inf = INFINITY;

/*
 * this is how these used to be defined, before C9X was usable. Kept in case
 * we port to a system on which it isn't available.
const double Double::NaN = 0.0/0.0;
const double Double::Inf = 1.0/0.0;
*/

void
Double::setFP64BitMode()
{
#ifdef __linux__
    fpu_control_t cw;
    _FPU_GETCW(cw);
    AssertAlways(cw == _FPU_IEEE,
		 ("Whoa, exiting mode for fpu not IEEE?! %d != %d\n",
		  cw, _FPU_IEEE));
    cw &= ~_FPU_EXTENDED;
    cw |= _FPU_DOUBLE;
    _FPU_SETCW(cw);
#endif
}

void
Double::resetFPMode()
{
#ifdef __linux__
    fpu_control_t cw = _FPU_IEEE;
    _FPU_SETCW(cw);
#endif
}


