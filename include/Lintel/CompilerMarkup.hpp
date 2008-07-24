/* -*-C++-*- */
/*
   (c) Copyright 2006-2008, Hewlett-Packard Development Company, LP

  Originally from http://rlove.org/log/2005102601
  then fixed up for C++, and based on comments in:
  http://gcc.gnu.org/onlinedocs/gcc-3.0.4/gcc_5.html

  Windows versions added 2008
*/

/** @file
    compiler markup operations (gcc and visual c++), with header to 
*/

#ifndef LINTEL_COMPILER_MARKUP_HPP
#define LINTEL_COMPILER_MARKUP_HPP

// Commented out for now most of the C++ useful things because we
// don't use them yet.  Will need to choose different names because
// some of the linux include files define these names.

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
// # define inline         inline __attribute__ ((always_inline))
// # define __pure         __attribute__ ((pure))
// # define __const        __attribute__ ((const))
#define FUNC_ATTR_NORETURN     __attribute__ ((noreturn))
#define FUNC_DEPRECATED __attribute__ ((deprecated))
// # define __malloc       __attribute__ ((malloc))
// # define __must_check   __attribute__ ((warn_unused_result))
# define LIKELY(x)      __builtin_expect ((x), 1)
# define UNLIKELY(x)    __builtin_expect ((x), 0)
#else

/* this is a start at a visual c++ version. Unfortunately, it
   will not work as written, as the specs have to go in a different
   spot than gcc. e.g.:

   int f(int) __attribute__((deprecated));   	[spec last for gcc]
   __declspec(deprecated) int f(int);		[spec first for MSC]

   I'm sure there is some macro/metaprogramming magic that can be done
   to make this portable, but I don't know what it is :-(

#ifdef _MSC_VER
#define FUNC_ATTR_NORETURN __declspec(noreturn)
#define FUNC_DEPRECATED __declspec(deprecated)
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#else

*/


// # define inline         /* no inline attribute support */
// # define __pure         /* no pure attribute support */
// # define __const        /* no const attribute support */
#define FUNC_ATTR_NORETURN     /* no noreturn attribute support */
#define FUNC_DEPRECATED
// # define __malloc       /* no malloc attribute support */
// # define __must_check   /* no warn_unused_result attribute support */
// # define __deprecated   /* no deprecated attribute support */
# define LIKELY(x)      (x)
# define UNLIKELY(x)    (x)
#endif

#endif
