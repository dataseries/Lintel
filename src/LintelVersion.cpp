/* -*-C++-*-
   (c) Copyright 2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Lintel version implementation; mostly for configure checking
*/

#include <Lintel/LintelVersion.hpp>

#ifndef VERSION
#define VERSION LIBLINTEL_VERSION
#endif
static char lintelVersionString[] = VERSION;
static char libtoolLibLintelVersionString[] = LIBLINTEL_VERSION;

const char *lintelVersion()
{
    return lintelVersionString;
}

const char *libtoolLibLintelVersion()
{
    return libtoolLibLintelVersionString;
}

