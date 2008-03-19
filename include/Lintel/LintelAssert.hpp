/* -*-C++-*- */
/*
   (c) Copyright 1994-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    enhanced assert functionality
*/

/*
*******************************************************************************
* Usage:
*	Assert(level, claim);
*	AssertMsg(level, claim, (format, args...) );
*
* The effect is to cause an abort if (1) AssertLevel is >= level, and (2) if
* claim is false.  Note that claim is not evaluated unless (1) is passed,
* so **DO NOT PUT ANY MAINLINE CODE IN IT**!
* Along the way to causing an abort, any user-supplied debug functions
* are invoked.
* Note that the (parens) around the third argument to AssertMsg are REQUIRED.
*
* Assert and AssertMsg are not even included if the compile-time flag 
* NDEBUG is set.
*
* AssertFatal is always compiled in (even if NDEBUG is set), but does not
* check any conditions. Use like this: AssertFatal( (format, args...) );
*
* AssertAlways is always compiled in (even with NDEBUG), and is independent
* of DebugLevel. It is intended to be used for error checking which must
* always be present, for example after calling system routines.
*
* DebugMsg is like AssertMsg, except that the program continues on:
*       DebugMsg(level, (format, args...) );
*******************************************************************************
*/

#ifndef LINTEL_LINTEL_ASSERT_HPP
#define LINTEL_LINTEL_ASSERT_HPP

// needed in order to make DebugMsg work when called
// otherwise all users need to know to include stream.h and stdio.h
#include <iostream>
#include <fstream>
#include <stdio.h>


////////////////////////////////////////////////////////////////
// Global variables and space allocation
////////////////////////////////////////////////////////////////

extern unsigned AssertLevel;	// Higher means more asserts; default=1
extern unsigned DebugLevel;	// Higher means more debug info; default=1


////////////////////////////////////////////////////////////////
// The macros that make all this easy to use :-)
////////////////////////////////////////////////////////////////

#ifdef NDEBUG
// Debugging code IS NOT to be compiled in.

#define Assert(LeveL,ExpressioN)  ((void)0)
#define AssertMsg(LeveL,ExpressioN, MessagE)  ((void)0)
#define DebugMsg(LeveL, MessagE)  ((void)0)

#else
// Debugging code IS to be compiled in.

#define Assert(LeveL,ExpressioN) \
	( \
	 (AssertLevel < (LeveL)) ? (void)0 : \
	 (ExpressioN) ? (void)0 : \
	 AssertFail(#ExpressioN,__FILE__,__LINE__,"") \
	)

  // Note that MessagE is a (paren-enclosed) printf style arg list
#define AssertMsg(LeveL,ExpressioN, MessagE) \
	( \
	 (AssertLevel < (LeveL)) ? (void)0 : \
	 (ExpressioN) ? (void)0 : \
	 AssertFail(#ExpressioN, __FILE__, __LINE__, \
		    AssertMakeMessage MessagE ) \
	)

  // Note that MessagE is a (paren-enclosed) printf style arg list
#define DebugMsg(LeveL, MessagE) \
    { if (DebugLevel >= (LeveL)) { \
        DebugMakeMessage MessagE; \
    } }
#endif

////////////////////////////////////////////////////////////////
// The following macro allows you to assert conditions
// whether or not assert checks are compiled in - that is, it
// ignores the setting of NDEBUG and doesn't depend on DebugLevel
// Usage:
//	AssertAlways(claim, (format, args...) )
////////////////////////////////////////////////////////////////
#define AssertAlways(ExpressioN, MessagE) \
	( \
         (ExpressioN) ? (void)0 : \
	 AssertFail(#ExpressioN, __FILE__, __LINE__, \
		    AssertMakeMessage MessagE )\
	)
// Note that MessagE is a (paren-enclosed) printf style arg list


////////////////////////////////////////////////////////////////
// The following macro allows you to generate fatal errors,
// whether or not assert checks are compiled in - that is, it
// ignores the setting of NDEBUG.
// Usage:
//	AssertFatal( (printf-args...) );
////////////////////////////////////////////////////////////////

#define AssertFatal(MessagE) \
	AssertFail("Fatal error", __FILE__, __LINE__, \
		   AssertMakeMessage MessagE )



////////////////////////////////////////////////////////////////
// You can provide a function to be called on an assert failure.
// Here is its type signature, and the functions to be used to
// add it to the list of functions to be called, or take it off.
//
// A user-supplied parameter is remembered when the function is
// registered, and passed in to it when it is invoked later.
////////////////////////////////////////////////////////////////

typedef void AssertDebugFunc_t(const void *userdata);


extern void AssertRegisterDebugFunction	// Register a debug-function
	(AssertDebugFunc_t *function,
	 const void        *userdata);
extern void AssertUnRegisterDebugFunction // De-register a debug-function
	(AssertDebugFunc_t *function,
	 const void        *userdata);


// Once the debug functions are registered, you can call them. Here's how.

extern void AssertRunDebugFunctions();	// Run the registered debug functions
void Abort();			// Runs debug-funcs, causes a core dump






////////////////////////////////////////////////////////////////
// The following are two helper functions for Asserts.
// * AssertFail emits messages, calls user debug functions, and
//   then aborts.
// * AssertMakeMessage is like an sprintf() that allocates its
//   output string: it returns the newly-formatted string.
//   This is put into a private static area, so there's no need
//   to free it - besides, we are well on the way to an abort ...
////////////////////////////////////////////////////////////////
//
extern void AssertFail(char const *string,
		       char const *filename,
		       const int   line_number,
		       char const *message);
#if defined(__GNUC__)
#define PRINTF_ATTR __attribute__ ((format (printf,1,2)))
#else
#define PRINTF_ATTR
#endif
extern char *AssertMakeMessage(const char *format, ...) PRINTF_ATTR;
extern void DebugMakeMessage(const char *format, ...) PRINTF_ATTR;


//////////////////////////////////////////////////////////////////////////////
// Some conditions are simply fatal, no matter when they happen.
//////////////////////////////////////////////////////////////////////////////

#define FatalCondition(StrinG) \
    fflush(stdout); \
    fprintf(stderr, "%s:%d: fatal error: %s", __FILE__, __LINE__, StrinG); \
    Abort();

const int AssertInfoListSize(); // used for regression tests
#endif
