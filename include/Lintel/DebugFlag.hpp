/* -*-C++-*- */
/*
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief simple debug flag LINTEL_DEBUG since DEBUG is used inconsistently.
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

// TODO: is there a better way to do this?  we don't have standard variable macro arguments
// and trying to do '#define IF_LINTEL_DEBUG //' doesn't work.
#if LINTEL_DEBUG
#define IF_LINTEL_DEBUG(x) x
#define IF_LINTEL_DEBUG2(x,y) x,y
#define IF_LINTEL_DEBUG3(x,y,z) x,y,z
#else
#define IF_LINTEL_DEBUG(x)
#define IF_LINTEL_DEBUG2(x,y)
#define IF_LINTEL_DEBUG3(x,y,z)
#endif

#endif
