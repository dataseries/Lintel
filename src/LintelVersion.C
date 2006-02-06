/* -*-C++-*-
/*
   (c) Copyright 2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Lintel version implementation; mostly for configure checking
*/

#include <LintelVersion.H>

static char *lintelVersionString = VERSION;
static char *libtoolLibLintelVersionString = LIBLINTEL_VERSION;

char *lintelVersion()
{
    return lintelVersionString;
}

char *libtoolLibLintelVersion()
{
    return libtoolLibLintelVersionString;
}

