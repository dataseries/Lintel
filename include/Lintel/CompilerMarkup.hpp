/* -*-C++-*- */
/*
   (c) Copyright 2006-2008, Hewlett-Packard Development Company, LP

  Originally from http://rlove.org/log/2005102601
  then fixed up for C++, and based on comments in:
  http://gcc.gnu.org/onlinedocs/gcc-3.0.4/gcc_5.html

  Windows versions added 2008
*/

/** @file
    compiler markup operations (gcc and visual c++)

    Function attribute usage: To specify an attribute on a function,
    either noreturn or deprecated, you have to put something at the
    beginning and the end of the function.  For example:
    FUNC_DEPRECATED_PREFIX int foo() FUNC_DEPRECATED;

    Conditional direction compiler hint:
    \verbatim
    
    if (LIKELY(expr)) { 
        // usually true branch 
    } else if (UNLIKELY(expr2)) {
        // usually false branch
    }
    \endverbatim
*/

#ifndef LINTEL_COMPILER_MARKUP_HPP
#define LINTEL_COMPILER_MARKUP_HPP

// Commented out for now most of the C++ useful things because we
// don't use them yet.  Will need to choose different names because
// some of the linux include files define these names.

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
// # define ALWAYS_INLINE        inline __attribute__ ((always_inline))
// # define FUNC_PURE         __attribute__ ((pure))
// # define FUNC_CONST        __attribute__ ((const))
#    define FUNC_ATTR_NORETURN_PREFIX
#    define FUNC_ATTR_NORETURN     __attribute__ ((noreturn))
#    define FUNC_DEPRECATED_PREFIX
#    define FUNC_DEPRECATED __attribute__ ((deprecated))
#    define LIKELY(x)      __builtin_expect ((x), 1)
#    define UNLIKELY(x)    __builtin_expect ((x), 0)
// # define __malloc       __attribute__ ((malloc))
// # define __must_check   __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER)
#    define FUNC_ATTR_NORETURN_PREFIX __declspec(noreturn)
#    define FUNC_ATTR_NORETURN
#    define FUNC_DEPRECATED_PREFIX __declspec(deprecated)
#    define FUNC_DEPRECATED
#    define LIKELY(x) (x)
#    define UNLIKELY(x) (x)
#else
#    define FUNC_ATTR_NORETURN_PREFIX
#    define FUNC_ATTR_NORETURN     /* no noreturn attribute support */
#    define FUNC_DEPRECATED_PREFIX
#    define FUNC_DEPRECATED
#    define LIKELY(x)      (x)
#    define UNLIKELY(x)    (x)
#endif

#endif
