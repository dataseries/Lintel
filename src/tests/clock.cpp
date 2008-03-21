/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/Clock.H>

int
main()
{
    Clock::selfCheck();
    Clock::timingTest();
    return 0;
}
