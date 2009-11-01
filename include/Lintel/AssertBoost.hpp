/* -*-C++-*- */
/*
   (c) Copyright 1994-2006, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    assertion functionality using the boost formatting library
*/

#ifndef LINTEL_ASSERT_BOOST_HPP
#define LINTEL_ASSERT_BOOST_HPP

#include <boost/format.hpp>
#include <boost/function.hpp>

#include <Lintel/CompilerMarkup.hpp>
#include <Lintel/DebugFlag.hpp>

class AssertBoostException : public std::exception {
public:
    const std::string expression;
    const std::string filename;
    unsigned line;
    const std::string msg;

    virtual const char *what() throw ();
    const std::string summary();
    AssertBoostException(const char *a, const char *b, unsigned c,
			 const std::string &d)
	: expression(a), filename(b), line(c), msg(d), save_what() { }
    virtual ~AssertBoostException() throw ();
private:
    std::string save_what;
};


// Arguments are expression, file, line, message
typedef boost::function<void (const char *, const char *, unsigned,
			      const std::string &)> assert_hook_fn;
/// Call this hook before printing out the message
void AssertBoostFnBefore(const assert_hook_fn &fn);
/// Call this hook after printing out the message
FUNC_DEPRECATED_PREFIX void AssertBoostFnAfter(void (*fn)()) FUNC_DEPRECATED;
void AssertBoostFnAfter(const assert_hook_fn &fn);
/// Clear out any of the before and after functions
void AssertBoostClearFns();

/// This function will create and throw an AssertBoostException; available
/// as a function so a wrapper could choose to only throw exceptions from
/// certain files.
void AssertBoostThrowExceptionFn(const char *expression, const char *filename,
				 unsigned line, const std::string &message);

FUNC_ATTR_NORETURN_PREFIX void AssertBoostFail(const char *expression, const char *file, int line,
					       const boost::format &format) FUNC_ATTR_NORETURN;

FUNC_ATTR_NORETURN_PREFIX void AssertBoostFail(const char *expression, const char *file, int line,
					       const std::string &format) FUNC_ATTR_NORETURN;

// Need to wrap try blocks in these expressions because the boost
// format library can throw the exception during initial parsing which
// is before we call into AssertBoostFail, and so the exception checks
// that are in there don't apply.

/// Unconditional fatal errors 
#define FATAL_ERROR(MessagE)   	       	       	       	       	       	          \
    do {								          \
        try {								          \
	    AssertBoostFail("Fatal error", __FILE__, __LINE__, MessagE);          \
        } catch (AssertBoostException &e) {                                       \
            throw e;                                                              \
        } catch (std::exception &e) {					          \
            AssertBoostFail("Fatal error", __FILE__, __LINE__,                    \
                            boost::format("Exception evaluating %s") % #MessagE); \
	}                                                                         \
   } while (0)


// Use LIKELY to optimize for expected direction of tests; would like
// to put the try block around the expression, but unless try blocks
// are "free" then we greatly increase the cost of an invariant.
/// Possible errors with a message.
#define INVARIANT(ExpressioN, MessagE)                                                \
    do {                                                                              \
        if (LIKELY(ExpressioN)) {                                                     \
        } else {                                                                      \
            try {                                                                     \
	        AssertBoostFail(#ExpressioN, __FILE__, __LINE__, MessagE);            \
            } catch (AssertBoostException &e) {                                       \
                throw e;                                                              \
            } catch (std::exception &e) {                                             \
                AssertBoostFail(#ExpressioN, __FILE__, __LINE__,                      \
                                boost::format("Exception evaluating %s") % #MessagE); \
            }                                                                         \
        }                                                                             \
   } while (0)

/// Checked is for things that will have side effects, some people
/// don't like this to be called INVARIANT on the assumption that
/// invariants could be compiled out.  We choose to only compile out
/// debug invariants, but the naming change could be valuable.
#define CHECKED(ExpressioN, MessagE) INVARIANT(ExpressioN, MessagE)

/// global constant to avoid having lots of duplicated strings
extern std::string global_assertboost_no_details;

/// Simple Invariant, lets us stop writing additional bits that mean nothing
#define SINVARIANT(ExpressioN) INVARIANT(ExpressioN, global_assertboost_no_details);

#if LINTEL_DEBUG
#    define DEBUG_INVARIANT(ExpressioN, MessagE) INVARIANT(ExpressioN, MessagE)
#    define DEBUG_SINVARIANT(ExpressioN) INVARIANT(ExpressioN, global_assertboost_no_details);
#else
#    define LINTEL_ASSERT_BOOST_DEBUG 0
#    define DEBUG_INVARIANT(ExpressioN, MessagE) do { } while(0)
#    define DEBUG_SINVARIANT(ExpressioN) do { } while(0)
#endif

#endif
