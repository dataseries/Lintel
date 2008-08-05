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

extern unsigned AssertLevel; // from LintelAssert.H, but want to deprecate it

class AssertBoostException : public std::exception {
public:
    const std::string expression;
    const std::string filename;
    unsigned line;
    const std::string msg;

    virtual const char *what();
    const std::string summary();
    AssertBoostException(const char *a, const char *b, unsigned c,
			 const std::string &d) 
	: expression(a), filename(b), line(c), msg(d) { }
    virtual ~AssertBoostException() throw ();
};


// Arguments are expression, file, line, message
typedef boost::function<void (const char *, const char *, unsigned, 
			      const std::string &)> assert_hook_fn;
/// Call this hook before printing out the message
void AssertBoostFnBefore(const assert_hook_fn &fn);
/// Call this hook after printing out the message
void AssertBoostFnAfter(void (*fn)()); // TODO: deprecate
void AssertBoostFnAfter(const assert_hook_fn &fn);
/// Clear out any of the before and after functions
void AssertBoostClearFns();

/// This function will create and throw an AssertBoostException; available
/// as a function so a wrapper could choose to only throw exceptions from
/// certain files.
void AssertBoostThrowExceptionFn(const char *expression, const char *filename, 
				 unsigned line, const std::string &message);

void AssertBoostFail(const char *expression, const char *file, int line,
		     boost::format &format) FUNC_ATTR_NORETURN;

void AssertBoostFail(const char *expression, const char *file, int line,
		     const std::string &format) FUNC_ATTR_NORETURN;

// TODO: Need to wrap try blocks in these expressions because 
// the boost format library can throw the exception during initial parsing
// which is before we call into AssertBoostFail, and so the exception
// checks that are in there don't apply.

// Use LIKELY to optimize for expected direction of tests
#define INVARIANT(ExpressioN, MessagE) \
	( \
         LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)

#define FATAL_ERROR(MessagE) \
	AssertBoostFail("Fatal error", __FILE__, __LINE__, \
		        MessagE)
        
// Checked is for things that will have side effects, some people
// don't like this to be called INVARIANT on the assumption that
// invariants could be compiled out.  We choose to only compile out
// debug invariants, but the naming change could be valuable.
#define CHECKED(ExpressioN, MessagE) INVARIANT(ExpressioN, MessagE)

// Simple Invariant, lets us stop writing additional bits that mean nothing
extern std::string global_assertboost_no_details;
#define SINVARIANT(ExpressioN) INVARIANT(ExpressioN, global_assertboost_no_details);

#ifdef DEBUG
#  if DEBUG
#    define LINTEL_ASSERT_BOOST_DEBUG
#  endif
#endif

#ifdef LINTEL_ASSERT_BOOST_DEBUG
#  define DEBUG_INVARIANT(ExpressioN, MessagE) \
	( \
         LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)

#  define DEBUG_SINVARIANT(ExpressioN) DEBUG_INVARIANT(ExpressioN, global_assertboost_no_details);
#  define DEBUG_CHECK(LeveL, ExpressioN, MessagE) \
	( \
	 LIKELY(AssertLevel < (LeveL)) ? (void)0 : \
	 LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)

#  undef LINTEL_ASSERT_BOOST_DEBUG
#else
#  define DEBUG_INVARIANT(ExpressioN, MessagE) do { } while(0)
#  define DEBUG_SINVARIANT(ExpressioN) do { } while(0)
#  define DEBUG_CHECK(LeveL, ExpressioN, MessagE) do { } while(0)
#endif

#endif
