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
static const char *default_what = 
   "AssertBoostException: unable to use summary() to get full error -- out of memory?";

const char * AssertBoostException::what() throw () {
    const char *ret = default_what;
    try { 
	if (save_what.empty()) {
	    save_what = summary();
	}
	ret = save_what.c_str();
    } catch (...) {
	ret = default_what;
    }
    return default_what;
}

const std::string AssertBoostException::summary() {
    return (boost::format("AssertBoostException(%s,%s,%d,%s)")
	    % expression % filename % line % msg).str();
}

AssertBoostException::~AssertBoostException() throw ()
{ }

void AssertBoostThrowExceptionFn(const char *a, const char *b, unsigned c,
				 const std::string &d) {
    throw AssertBoostException(a,b,c,d);
}

static void noArgHook(void (*fn)(), const char *, const char *, unsigned,
		      const string &) {
    fn();
}

void AssertBoostFnBefore(const assert_hook_fn &fn) {
    pre_msg_fns.push_back(fn);
}

void AssertBoostFnAfter(void (*fn)()) {
    AssertBoostFnAfter(boost::bind(noArgHook, fn, _1, _2, _3, _4));
}

void AssertBoostFnAfter(const assert_hook_fn &fn) {
    post_msg_fns.push_back(fn);
}

void AssertBoostClearFns() {
    pre_msg_fns.clear();
    post_msg_fns.clear();
}

static bool assert_boost_fail_recurse;

static void AssertBoostFailOutput(const char *expression, const char *file, 
				  int line, const string &msg) {
    fflush(stderr);  cerr.flush(); // Belts ...
    fflush(stdout);  cout.flush(); // ... and braces
	
    cerr << boost::format("\n**** Assertion failure in file %s, line %d\n"
			  "**** Failed expression: %s\n")
	% file % line % expression;
	
    if (msg.size() > 0) {
	cerr << "**** Details: " << msg << "\n";
    }
    cerr.flush();
}

static void AssertBoostFailDie() FUNC_ATTR_NORETURN;

void AssertBoostFailDie() {
    abort();	 // try to die
    exit(173);   // try harder to die
    kill(getpid(), 9); // try hardest to die
    while(1) {
	cerr << "**** Help, I'm still not dead????" << endl;
	sleep(1);
    }
    /*NOTREACHED*/
}

// Doing things this way means a throw inside an assertion cleans up
// the recursion flag.  Can't use SINVARIANT inside of this, so expand
// it out and manually invert the test 
class AssertBoostCleanupRecurse {
public:
    AssertBoostCleanupRecurse() {
	if (!(assert_boost_fail_recurse == false)) {
	    AssertBoostFailOutput("assert_boost_fail_recurse == false", 
				  __FILE__, __LINE__, "unexpected value for recurse flag; multiple threads dying at the same time?");
	    AssertBoostFailDie();
	}
	assert_boost_fail_recurse = true;
    }
    ~AssertBoostCleanupRecurse() {
	if (!(assert_boost_fail_recurse == true)) {
	    AssertBoostFailOutput("assert_boost_fail_recurse == true", 
				  __FILE__, __LINE__, "unexpected value for recurse flag; multiple threads dying at the same time?");
	    AssertBoostFailDie();
	}
	assert_boost_fail_recurse = false;
    }
};

void AssertBoostFail(const char *expression, const char *file, int line,
		     const string &msg) {
    fflush(stderr);  cerr.flush(); // Belts ...
    fflush(stdout);  cout.flush(); // ... and braces

    if (assert_boost_fail_recurse) {
	cerr << "**** Recursing on assertions, you have a bad hook" << endl;
    } else {
	for(vector<assert_hook_fn>::iterator i = pre_msg_fns.begin();
	    i != pre_msg_fns.end(); ++i) {
	    AssertBoostCleanupRecurse cleanup;
	    (*i)(expression, file, line, msg);
	}
    }	

    AssertBoostFailOutput(expression, file, line, msg);
    if (!assert_boost_fail_recurse) {
	for(vector<assert_hook_fn>::iterator i = post_msg_fns.begin();
	    i != post_msg_fns.end(); ++i) {
	    AssertBoostCleanupRecurse cleanup;
	    (*i)(expression, file, line, msg);
	}
    }

    AssertBoostFailDie();
    /*NOTREACHED*/
}

void AssertBoostFail(const char *expression, const char *file, int line,
		     boost::format &format) {
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
