/* -*-C++-*-
   (c) Copyright 2006, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Implementation
*/

#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <cstdlib>

#include <AssertBoost.H>

void AssertBoostFail(const char *expression, const char *file, int line,
		     const std::string &msg)
{

    fflush(stderr);  std::cerr.flush(); // Belts ...
    fflush(stdout);  std::cout.flush(); // ... and braces


    std::cerr << std::endl 
	      << "**** Assertion failure in file " << file << ", line "
	      << line << std::endl 
	      << "**** Failed expression: " << expression << std::endl;
    
    if (msg.size() > 0) {
	std::cerr << "**** Details: " << msg << std::endl;
    }
    std::cerr.flush();
    abort();	 // try to die
    exit(173);   // try harder to die
    kill(getpid(), 9); // try hardest to die
    while(1) {
	std::cerr << "**** Help, I'm still not dead????" << std::endl;
	sleep(1);
    }
    /*NOTREACHED*/
}

void AssertBoostFail(const char *expression, const char *file, int line,
		     boost::format &format)
{
    std::string msg;
    try {
	msg = format.str();
    } catch (boost::io::too_many_args &e) {
	msg = "**** Programmer error, format given too many arguments";
    } catch (boost::io::too_few_args &e) {
	msg = "**** Programmer error, format given too few arguments";
    } catch(...) {
	msg = "**** Unknown error, exception thrown during format evaluation";
    }

    AssertBoostFail(expression, file, line, msg);
}
