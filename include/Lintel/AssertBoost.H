/* -*-C++-*- */
/*
   (c) Copyright 1994-2006, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    assertion functionality using the boost formatting library
*/

#ifndef INCLUDED_ASSERT_BOOST_H
#define INCLUDED_ASSERT_BOOST_H

#include <boost/format.hpp>

#include <Lintel/CompilerMarkup.H>

extern unsigned AssertLevel; // from LintelAssert.H, but want to deprecate it

// TODO: figure out what AssertBoostFnAfter interface ought to be; no
// arguments is probably wrong.
void AssertBoostFnAfter(void (*fn)()); 

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
        
#if defined(COMPILE_DEBUG) || defined(DEBUG)
#define DEBUG_INVARIANT(ExpressioN, MessagE) \
	( \
         LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)

#define DEBUG_CHECK(LeveL, ExpressioN, MessagE) \
	( \
	 LIKELY(AssertLevel < (LeveL)) ? (void)0 : \
	 LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)


#else
#define DEBUG_INVARIANT(ExpressioN, MessagE) do { } while(0)
#define DEBUG_CHECK(LeveL, ExpressioN, MessagE) do { } while(0)
#endif

#endif
