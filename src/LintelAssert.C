/* -*-C++-*-
   (c) Copyright 1994-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Body of Assert functionality
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include <Lintel/LintelAssert.H>

// This variable is defined here so if you link against LintelAssert.o, you
// get a functional and self-contained package. 
unsigned AssertLevel = 1;		// Declare space and initialize
unsigned DebugLevel = 1;		// Declare space and initialize


////////////////////////////////////////////////////////////////
// The following structure and the list that points to instances
// of it are used to hold the list of user-supplied functions and
// the info that gets passed to them.  One object is created for
// each user-registered function.
////////////////////////////////////////////////////////////////
//
class AssertInfo_t
{
public:
    AssertInfo_t(AssertDebugFunc_t *function_in,
		 const void        *userdata_in)
      : function(function_in),
	userdata(userdata_in)
    {};
    AssertDebugFunc_t *function;
    const void              *userdata;
};

static std::vector<AssertInfo_t *> AssertInfoList;

const int
AssertInfoListSize()
{
    return AssertInfoList.size();
}
// static List AssertInfoList;		// Holds list of things to invoke




//////////////////////////////////////////////////////////////////////////////
// Setup functions to allow addition/removal of debug functions
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------
// Register a new user-level function to be called on an Assert failure.
//----------------------------------------------------------------
//
extern void AssertRegisterDebugFunction(AssertDebugFunc_t *function,
					const void        *userdata)
{
    AssertInfo_t *info = new AssertInfo_t(function, userdata);
    // AssertInfoList.AddAtTail(info);
    AssertInfoList.push_back(info);
};


//----------------------------------------------------------------
// Unregister a user-level function so that it will no longer be called
// on an Assert failure.
// It is a (fatal) error for it not to be present already.  Both the
// function pointer and the userdata value have to match.
//----------------------------------------------------------------
//
extern void AssertUnRegisterDebugFunction(AssertDebugFunc_t *function,
					  const void        *userdata)
{
    for (std::vector<AssertInfo_t *>::iterator i = AssertInfoList.begin();
	 i != AssertInfoList.end(); ++i) {
	// ListPos *pos = AssertInfoList.Head();
	// while (pos != NULL) {
	// AssertInfo_t *info = (AssertInfo_t *)pos->value();
	// Assert(1, info != NULL &&  info->function != NULL);
	AssertInfo_t *info = *i;
	if (info->function == function  && 
	    info->userdata == userdata) {
	    // AssertInfoList.RemoveLink(pos);
	    AssertInfoList.erase(i);
	    break;
	}
	// pos = AssertInfoList.NextPos(pos);
    }
};




//////////////////////////////////////////////////////////////////////////////
// Failure functions
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------
// User-invokable function to die with debug info
//----------------------------------------------------------------
//
extern void Abort()
{
    // Please don't use C++ iostreams here.  They are not thread-safe, and
    // Lintel is used by threaded programs.
    fflush(stderr);  std::cerr.flush(); // Belts ...
    fflush(stdout);  std::cout.flush(); // ... and braces

    fprintf(stderr, "\n**** Abort() has been invoked\n");
    fflush(stderr);

    // Call the registered debug functions.
    AssertRunDebugFunctions();

    // Take a core dump
    fprintf(stderr, "\n**** Abort(): about to take a core dump\n");
    abort();
    /*NOTREACHED*/
};



//----------------------------------------------------------------
// When an Assert fails, it calls this.
//----------------------------------------------------------------
//
extern void AssertFail(char  const  *string,
		       char  const  *filename,
		       const int     line_number,
		       char  const  *message)
{
    fflush(stderr);  std::cerr.flush(); // Belts ...
    fflush(stdout);  std::cout.flush(); // ... and braces

    fprintf(stderr, "\n**** Assert failure in %s, %d: %s\n",
                    filename, line_number, string);
    if ((message != (char const *)NULL) && (strlen(message) > 0)) {
        fprintf(stderr, "**** %s\n", message);
    }
    fflush(stderr);
    Abort();			// also runs the debug functions
    /*NOTREACHED*/
};



//----------------------------------------------------------------
// Helper function to do an sprintf into the internal static message area.
//----------------------------------------------------------------

#define ASSERT_MESSAGE_LENGTH 10000
static char assert_message[ASSERT_MESSAGE_LENGTH];

extern char *AssertMakeMessage(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  int result_len = vsprintf(assert_message, format, ap);
  if (result_len >= ASSERT_MESSAGE_LENGTH) {
    fflush(stderr);  std::cerr.flush(); // Belts ...
    fflush(stdout);  std::cout.flush(); // ... and braces
    fprintf(stderr, "\n**** AssertMakeMessage: warning: message too long\n");
    fflush(stderr);
    assert_message[ASSERT_MESSAGE_LENGTH] = '\0'; // force an end-of-string
  }

  va_end(ap);
  return assert_message;
}

void DebugMakeMessage(const char *format, ...)
{
  char buffer[1000];
  va_list ap;
  va_start(ap, format);

  // %%% Sadly, the very convenient vsnprintf function
  // %%% exists on HP-UX 11.0 and on Linux, but not on HP-UX 10.20.
  // %%% And I couldn't find any compiler define to distinguish
  // %%% 10.20 from 11.0.  So for now, both versions of HP-UX
  // %%% have to run without checking:
  #ifdef __linux
      int how_many_written = vsnprintf(buffer, 998, format, ap);
  #else
      int how_many_written = vsprintf(buffer, format, ap);
      AssertAlways(how_many_written<1000, 
                   ("Double fault: Debug message too long.  Message was\n%s",
                    buffer));
  #endif
  buffer[how_many_written] = '\n';
  buffer[how_many_written+1] = 0;

  fflush(stdout); fflush(stderr);
  fprintf(stderr, buffer);
  fflush(stdout); fflush(stderr);
  
  va_end(ap);
}




//----------------------------------------------------------------
// Execute the user-supplied debug functions
// It is an error to invoke this recursively ...
//----------------------------------------------------------------
//
void AssertRunDebugFunctions()
{
    static bool AssertRecursing = false;	// True if trying to recurse
    fflush(stderr);  std::cerr.flush(); // Belts ...
    fflush(stdout);  std::cout.flush(); // ... and braces

    if (AssertRecursing) {
	fprintf(stderr, "\n**** AssertFailure handler is recursing ... "
                        "aborting immediately\n");
	fflush(stderr);
	abort();
	/*NOTREACHED*/
    }
    AssertRecursing = true;	// Fault attempts to call Assert recursively

    for (unsigned i = 0; i < AssertInfoList.size(); i++) {
	// ListPos *pos = AssertInfoList.Head();
	// while (pos != NULL) {
	// AssertInfo_t *info = (AssertInfo_t *)pos->value();
	AssertInfo_t *info = AssertInfoList[i];
	if (info == NULL || info->function == NULL) {
	    fprintf(stderr, "\n**** Internal failure 1 in "
                            "AssertFailure code\n");
	    fflush(stderr);
	    abort();
	    /*NOTREACHED*/
	}
	(*info->function)(info->userdata);
	// pos = AssertInfoList.NextPos(pos);
	fflush(stderr);  std::cerr.flush(); // Belts ...
	fflush(stdout);  std::cout.flush(); // ... and braces
    }
    AssertRecursing = false;	// So we can be called again
};

