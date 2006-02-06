/* -*-C++-*-
/*
   (c) Copyright 2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Lintel version #
*/

// need the extern "C" to make the autoconf checks work easier; which
// is really why this file exists at all.
extern "C" {
    char *lintelVersion();
    char *libtoolLibLintelVersion();
}
