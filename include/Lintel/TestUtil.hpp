/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Some help with testing things that throw invariants
*/

#ifndef LINTEL_TESTUTIL_HPP
#define LINTEL_TESTUTIL_HPP

#include <Lintel/AssertBoost.hpp>
/// Macro to test whether a particular chunk of code has an invariant
/// with a specified message; Assumes that you have not set up any
/// AssertBoost functions
#define TEST_INVARIANTMSG(code, message) \
    AssertBoostFnBefore(AssertBoostThrowExceptionFn); \
    try { \
        code; \
        FATAL_ERROR("?"); \
    } catch (AssertBoostException &e) { \
        AssertBoostClearFns(); \
        INVARIANT(e.msg == message, \
                  boost::format("unexpected error message '%s'") % e.msg); \
    }

#endif
