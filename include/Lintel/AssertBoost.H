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

void AssertBoostFail(const char *expression, const char *file, int line,
		     boost::format &format) FUNC_ATTR_NORETURN;

void AssertBoostFail(const char *expression, const char *file, int line,
		     const std::string &format) FUNC_ATTR_NORETURN;

// Need to wrap try blocks in these expressions because 
// the boost format library can throw the exception during initial parsing
// which is before we call into AssertBoostFail, and so the exception
// checks that are in there don't apply.

// Optimize for the case where the debugging checks are off.
#define DEBUG_CHECK(LeveL,ExpressioN, MessagE) \
	( \
	 LIKELY(AssertLevel < (LeveL)) ? (void)0 : \
	 LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)


#define INVARIANT(ExpressioN, MessagE) \
	( \
         LIKELY(ExpressioN) ? (void)0 : \
	 AssertBoostFail(#ExpressioN, __FILE__, __LINE__, \
		         MessagE) \
	)

#define FATAL_ERROR(MessagE) \
	AssertBoostFail("Fatal error", __FILE__, __LINE__, \
		        MessagE)
        

#endif

