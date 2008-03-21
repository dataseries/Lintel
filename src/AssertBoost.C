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

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <Lintel/AssertBoost.hpp>

using namespace std;

static vector<assert_hook_fn> pre_msg_fns, post_msg_fns;
string global_assertboost_no_details("No additional details provided");

const char * AssertBoostException::what() 
{
    // can't manufacture a string here or else we space leak.  Grr, why
    // did the committee not use std::string?
    return "AssertBoostException, use summary() to get full error";
}

const std::string AssertBoostException::summary()
{
    return (boost::format("AssertBoostException(%s,%s,%d,%s)")
	    % expression % filename % line % msg).str();
}

AssertBoostException::~AssertBoostException() throw ()
{
}

void AssertBoostThrowExceptionFn(const char *a, const char *b, unsigned c,
				 const std::string &d)
{
    throw AssertBoostException(a,b,c,d);
}

static void noArgHook(void (*fn)(), const char *, const char *, unsigned,
		      const string &)
{
    fn();
}

void AssertBoostFnBefore(const assert_hook_fn &fn)
{
    pre_msg_fns.push_back(fn);
}

void AssertBoostFnAfter(void (*fn)())
{
    AssertBoostFnAfter(boost::bind(noArgHook, fn, _1, _2, _3, _4));
}

void AssertBoostFnAfter(const assert_hook_fn &fn)
{
    post_msg_fns.push_back(fn);
}

void AssertBoostClearFns()
{
    pre_msg_fns.clear();
    post_msg_fns.clear();
}

void AssertBoostFail(const char *expression, const char *file, int line,
		     const string &msg)
{
    fflush(stderr);  cerr.flush(); // Belts ...
    fflush(stdout);  cout.flush(); // ... and braces

    for(vector<assert_hook_fn>::iterator i = pre_msg_fns.begin();
	i != pre_msg_fns.end(); ++i) {
      (*i)(expression, file, line, msg);
    }

    fflush(stderr);  cerr.flush(); // Belts ...
    fflush(stdout);  cout.flush(); // ... and braces

    cerr << boost::format("\n**** Assertion failure in file %s, line %d\n"
			  "**** Failed expression: %s\n")
	% file % line % expression;
    
    if (msg.size() > 0) {
	cerr << "**** Details: " << msg << "\n";
    }
    cerr.flush();
    for(vector<assert_hook_fn>::iterator i = post_msg_fns.begin();
	i != post_msg_fns.end(); ++i) {
      (*i)(expression, file, line, msg);
    }
    abort();	 // try to die
    exit(173);   // try harder to die
    kill(getpid(), 9); // try hardest to die
    while(1) {
	cerr << "**** Help, I'm still not dead????" << endl;
	sleep(1);
    }
    /*NOTREACHED*/
}

void AssertBoostFail(const char *expression, const char *file, int line,
		     boost::format &format)
{
    string msg;
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
