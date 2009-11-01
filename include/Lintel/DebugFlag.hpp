/* -*-C++-*- */
/*
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    simple debug flag LINTEL_DEBUG since DEBUG is used inconsistently.
*/

#ifndef LINTEL_DEBUGFLAG_HPP
#define LINTEL_DEBUGFLAG_HPP

// allow for -DDEBUG=0
#ifdef DEBUG
#    if DEBUG
#        define LINTEL_DEBUG 1
#    endif
#elif defined(NDEBUG)
#    if NDEBUG
#        define LINTEL_DEBUG 0
#    endif
#else
#    define LINTEL_DEBUG 0
#endif

#endif
