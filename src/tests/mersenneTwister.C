/* -*-C++-*-
*******************************************************************************
*
* File:         mersenneTwister.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/tests/mersenneTwister.C,v 1.1 2002/11/17 01:03:03 anderse Exp $
* Description:  Regression test for the MersenneTwisterRandom class
* Author:       Eric Anderson
* Created:      Sat Aug 31 19:43:26 2002
* Modified:     Wed Nov 13 17:49:34 2002 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2002, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <stdio.h>

#include <MersenneTwisterRandom.H>

int 
main()
{
    MersenneTwisterRandom::selfTest();
    return 0;
}

