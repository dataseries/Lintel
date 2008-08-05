/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/Clock.hpp>

int
main()
{
    Clock::allowUnsafeFrequencyScaling(Clock::AUFSO_WarnFast);
    Clock::selfCheck();
    Clock::timingTest();
    return 0;
}
