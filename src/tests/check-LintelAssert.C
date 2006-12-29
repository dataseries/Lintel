/* -*-C++-*-
/*
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    verify assertions are working correctly
*/

#include <stdlib.h>
#include <LintelAssert.H>


////////////////////////////////////////////////////////////////
// Regression tests
////////////////////////////////////////////////////////////////
//

//----------------------------------------------------------------
// Sample debug functions
//----------------------------------------------------------------
//
static void debugFunc1(const void *userdata)
{
  fprintf(stderr, "debugFunc1 invoked, with argument %d\n", (int)(long)userdata);
}

static void debugFunc2(const void *userdata)
{
  fprintf(stderr, "debugFunc2 invoked, with argument %d\n", (int)(long)userdata);
}

static void debugFuncRecurse(const void *userdata)
{
  fprintf(stderr, "debugFuncRecurse invoked, with argument %d\n",
	  (int)(long)userdata);
  AssertRunDebugFunctions();
}

//----------------------------------------------------------------
// The main program.  Usage:
//	Assert [-R] [assert-level]
//----------------------------------------------------------------
//
int
main(int argc, char* argv[])
{
  bool recurseTest = false;	// True if going to run regression tests
  bool fatalTest   = false;	// True if going to run fatal-error tests

  printf("** LintelAssert.C standalone test.\n");
  fflush(stdout);

  int offset = 1;
  while (argc > 1) {
    if (strcmp(argv[offset], "-R") == 0) {
      recurseTest = true;      // Run recursion test
    } else {
      if (strcmp(argv[offset], "-F") == 0) {
	fatalTest = true;      // Run fatal-error test
      } else {
	AssertLevel = atoi(argv[offset]);
      }
    }
    argc--;
    offset++;
  }

  printf("AssertLevel = %d, recurse = %d fatal = %d\n",
	 AssertLevel, recurseTest, fatalTest);
  fflush(stdout);

  DebugMsg(1, ("DebugMsg(1): DebugLevel=%d",DebugLevel) );
  DebugMsg(9, ("DebugMsg(9): DebugLevel=%d",DebugLevel) );

  AssertRegisterDebugFunction(debugFunc1, (void *)111);
  AssertRegisterDebugFunction(debugFunc2, (void *)222);
  if (recurseTest) {
    AssertRegisterDebugFunction(debugFuncRecurse, (void *)333);
  }
  printf("after registerDebugFunction, list length = %d\n",
	 AssertInfoListSize());
	 // AssertInfoList.length());
  fflush(stdout);

  if (fatalTest) {
    AssertFatal(("Force fatal error, because fatalTest = %d", fatalTest));
  }
  if (recurseTest) {
    Assert(0, "Force recursion test"==0);
  }
  AssertMsg(6, "Assert at level 6"==0, ("Should fire if AssertLevel >= %d",6));
  AssertMsg(0, "Assert at level 0"==0, ("Should always fire"));

  printf("after asserts .. something has gone wrong!\n");
  fflush(stdout);
  abort();
  /*NOTREACHED*/

  return 1;
}

