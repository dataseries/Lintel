#                                               -*- Autoconf -*-
#
#  (c) Copyright 2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Process this file with autoconf to produce a configure script.

AC_INIT(Lintel, 1.0, [software@cello.hpl.hp.com])

AC_PREREQ(2.57)

AC_REVISION($Revision: 1.4 $)
AC_CANONICAL_SYSTEM
AM_SANITY_CHECK
AM_INIT_AUTOMAKE

COM_HP_HPL_LINTEL_OPTMODE()

AM_MAINTAINER_MODE
COM_HP_HPL_ACINCLUDE(lintel)

# programs
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LD
AC_SUBST(LD)

HPL_PERL_SUBSTS

# library bits

case $host in
  hppa2.0w-hp-hpux11*) 
     if test "${enable_shared+set}" = set; then
	if test "${enable_shared}" = "yes"; then
	   AC_MSG_WARN(***WARNING: 2005-01-10: shared libraries for C++ were not working on HPUX)
	fi
     else
        AC_MSG_WARN(WARNING: disabling shared libraries on HP-UX, on 2005-01-10, they were not working)
	enable_shared=no
     fi
     ;;
esac

AC_ENABLE_SHARED
AC_ENABLE_STATIC
AC_LIBTOOL_SETUP
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
HPL_WITHLIB_TCL()
HPL_WITHLIB_GC()

# flags
AC_SYS_LARGEFILE

## following is things we build...

#
# Libtool library revision control info
# See the libtool documentation under the heading "Libtool's versioning
# system" in order to understand the meaning of these fields
#
# Here are a set of rules to help you update your library version
# information:
#
#  1. Start with version information of `0:0:0' for each libtool library.
#  2. Update the version information only immediately before a public
#     release of your software. More frequent updates are unnecessary, and
#     only guarantee that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update,
#     then increment revision (`c:r:a' becomes `c:r+1:a').
#  4. If any interfaces have been added, removed, or changed since the last
#     update, increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then
#     increment age.
#  6. If any interfaces have been removed since the last public release,
#     then set age to 0.

# Shared libraries have link dependencies for all files in the shared library.
# this means that if we link with extra libraries, e.g. pthread, gc, etc, even
# if an application won't use those libraries, it still has to provide the 
# dependencies.  After a bit of discussion, it was decided that we should
# pull out the source files that use extra libraries since linking with
# one of the extra libraries will automatically pull in the lintel library.
# This solution is not optimal, but is the best one we've been able to 
# discover to date.  Pulling out the gc dependent libraries hasn't happened
# yet.

LIBLINTEL_VERSION=0:0:0
LIBLINTELPTHREAD_VERSION=0:0:0
LIBNOGC_VERSION=0:0:0

AC_SUBST(LIBLINTEL_VERSION)
AC_SUBST(LIBLINTELPTHREAD_VERSION)
AC_SUBST(LIBNOGC_VERSION)

### HP-UX specific memory header replacement to make garbage collection work
### well on that system. (See header for comments)

case $host in
  *hpux*) WITH_HPUX_REPLACE_MEMORY=yes ;;
  *) WITH_HPUX_REPLACE_MEMORY=no ;;
esac

AM_CONDITIONAL(WITH_HPUX_REPLACE_MEMORY, test $WITH_HPUX_REPLACE_MEMORY = yes)

### Old Make.{common,world,cvs} bits that mighe be useful to someone

AC_ARG_WITH(makebits,
  [  --without-makebits     disable installation of Make.{common,world,cvs}],
  [with_makebits=$withval],
  [with_makebits='yes'])

AM_CONDITIONAL(WITH_MAKEBITS, test $with_makebits = yes)

### The Random regression tests generates different output on different 
### machines (and possibly compilers) -- pick the appropriate file.

case $host in 
  *hpux*) RANDOM_COMPARETO=tests/Random.hpux.regression ;; 
  *linux*) RANDOM_COMPARETO=tests/Random.linux.regression ;;
  *) echo "********************************************************";
     echo "*** WARNING: don't know appropriate file to use for";
     echo "*** WARNING: Random regression. (hosttype=$host)";
     echo "*** WARNING: choosing linux one as default";
     echo "********************************************************";
     RANDOM_COMPARETO=tests/Random.linux.regression ;;
esac

AC_SUBST(RANDOM_COMPARETO)

AC_CONFIG_FILES([Makefile src/lintel-config src/lintel-acinclude include/Makefile src/Makefile src/batch-parallel])

expanded_datadir=`eval echo $datadir`
AC_SUBST(expanded_datadir)

AC_OUTPUT

COM_HP_HPL_LINTEL_OPTMODE_MESSAGES
