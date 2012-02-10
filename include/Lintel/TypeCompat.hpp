/*
   (c) Copyright 2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Type compatibility definitions.
*/

#ifndef LINTEL_TYPECOMPAT_HPP
#define LINTEL_TYPECOMPAT_HPP

#ifdef __CYGWIN__
    typedef _off64_t off64_t;
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
    typedef off_t off64_t;
#   define O_LARGEFILE 0
#endif

#endif
