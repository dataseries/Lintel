/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/Clock.hpp>
#include <Lintel/LintelLog.hpp>

int main(int argc, char *argv[]) {
    LintelLog::parseEnv();

    Clock::allowUnsafeFrequencyScaling(Clock::AUFSO_WarnFast);
    if (LintelLog::wouldDebug("calibrate")) {
	Clock::calibrateClock(true);
	return 0;
    }
    Clock::selfCheck();
    if (LintelLog::wouldDebug("timing")) {
	Clock::timingTest();
    }
    return 0;
}
