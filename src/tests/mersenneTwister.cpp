/* -*-C++-*-
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Regression test for the MersenneTwisterRandom class
*/


#include <stdio.h>

#include <Lintel/MersenneTwisterRandom.hpp>

int 
main()
{
    MersenneTwisterRandom::selfTest();
    return 0;
}

