/* -*-C++-*- */
/*
   (c) Copyright 2006, Hewlett-Packard Development Company, LP

  Originally from http://rlove.org/log/2005102601
  then fixed up for C++, and based on comments in:
  http://gcc.gnu.org/onlinedocs/gcc-3.0.4/gcc_5.html
*/

/** @file
    compiler markup bits from gcc, with header to 
*/

#ifndef LINTEL_COMPILER_MARKUP_INCLUDE
#define LINTEL_COMPILER_MARKUP_INCLUDE

// Commented out for now most of the C++ useful things because we
// don't use them yet.  Will need to choose different names because
// some of the linux include files define these names.

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
// # define inline         inline __attribute__ ((always_inline))
// # define __pure         __attribute__ ((pure))
// # define __const        __attribute__ ((const))
# define FUNC_ATTR_NORETURN     __attribute__ ((noreturn))
// # define __malloc       __attribute__ ((malloc))
// # define __must_check   __attribute__ ((warn_unused_result))
// # define __deprecated   __attribute__ ((deprecated))
# define LIKELY(x)      __builtin_expect ((x), 1)
# define UNLIKELY(x)    __builtin_expect ((x), 0)
#else
// # define inline         /* no inline attribute support */
// # define __pure         /* no pure attribute support */
// # define __const        /* no const attribute support */
# define FUNC_ATTR_NORETURN     /* no noreturn attribute support */
// # define __malloc       /* no malloc attribute support */
// # define __must_check   /* no warn_unused_result attribute support */
// # define __deprecated   /* no deprecated attribute support */
# define LIKELY(x)      (x)
# define UNLIKELY(x)    (x)
#endif

#endif
