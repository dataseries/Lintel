#
#  (c) Copyright 2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

AC_DEFUN([EXPAND_VARIABLE],
[
# recent versions of autoconf (2.60) seem to store some variables needing
# multiple rounds of expansion to get to the fully expanded variable.
expand_variable() {
    ret="$[1]"
    ev_keepgoing=true
    while $ev_keepgoing; do
	case "$ret" in
	    *\$\{*) ret=`eval echo $ret` ;;
	    *) ev_keepgoing=false
	esac
    done
    echo $ret
}
])

AC_DEFUN([COM_HP_HPL_ACINCLUDE],
[
EXPAND_VARIABLE
ACINCLUDE_DEPENDENCIES=
if test "$USE_MAINTAINER_MODE" = yes; then
    if test "$1" = ""; then
	AC_MSG_ERROR(misuse of COM_HP_HPL_""ACINCLUDE)
	exit 1
    fi
    AC_MSG_CHECKING(if acinclude.m4 needs to be rebuilt)
    expanded_datadir=`expand_variable $datadir`
    test -f $srcdir/acinclude.m4.new && rm $srcdir/acinclude.m4.new
    touch $srcdir/acinclude.m4.new
    for i in $1; do
	if test -f $top_srcdir/com_hp_hpl_$i.m4; then
	    cat $top_srcdir/com_hp_hpl_$i.m4 >>$srcdir/acinclude.m4.new
	    ACINCLUDE_DEPENDENCIES="$ACINCLUDE_DEPENDENCIES \$(top_srcdir)/com_hp_hpl_$i.m4"
	elif test -f $srcdir/com_hp_hpl_$i.m4; then
	    cat $srcdir/com_hp_hpl_$i.m4 >>$srcdir/acinclude.m4.new
	    # Use of top_srcdir here is intentional, some versions of 
	    # configure don't seem to define that in the configure run, but
            # we absolutely need this in the makefiles because srcdir is 
   	    # in subdirectories will point to the subdirectory.
	    ACINCLUDE_DEPENDENCIES="$ACINCLUDE_DEPENDENCIES \$(top_srcdir)/com_hp_hpl_$i.m4"
	elif test -f $expanded_datadir/aclocal/com_hp_hpl_$i.m4; then
	    cat $expanded_datadir/aclocal/com_hp_hpl_$i.m4 >>$srcdir/acinclude.m4.new
	    ACINCLUDE_DEPENDENCIES="$ACINCLUDE_DEPENDENCIES $expanded_datadir/aclocal/com_hp_hpl_$i.m4"
	else
	    AC_MSG_NOTICE([unable to find com_hp_hpl_$i.m4 (looked in $top_srcdir, $srcdir, $expanded_datadir/aclocal), needed for building acinclude.m4])
	    AC_MSG_NOTICE([did you run make install for $i?])
	    AC_MSG_ERROR([which is needed for maintainer mode, aborting])
	    exit 1
	fi
    done
    if cmp $srcdir/acinclude.m4 $srcdir/acinclude.m4.new >/dev/null 2>&1; then
	AC_MSG_RESULT(no)
	rm $srcdir/acinclude.m4.new
    else
	AC_MSG_RESULT(yes)
        test -f $srcdir/acinclude.m4 && rm $srcdir/acinclude.m4
	mv $srcdir/acinclude.m4.new $srcdir/acinclude.m4
    fi
fi
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],$ACINCLUDE_DEPENDENCIES)
])
	
# have to use m4_define here; tried to use AC_DEFUN, but got really weird
# error message.  Second use of m4_define eliminates a warning from aclocal
# about not having used HPL_$1

m4_define([HPL_DEFINE_REQUIRELIB],
[
    m4_define(m4_format([HPL_REQUIRELIB_%s], translit($1, [a-z], [A-Z])),
m4_format([[HPL_WITHLIB_%s]], translit($1, [a-z], [A-Z]))
if test m4_format(["$with_%s"], translit($1, [A-Z], [a-z])) = yes; then
	AC_MSG_NOTICE([Found required library $1])
else
	AC_MSG_ERROR([Could not find required library $1])
	exit 1
fi
)
])

# using m4_define here, per comment on HPL_DEFINE_REQUIRELIB
# This macro (HPL_DEFINE_SIMPLE_WITHLIB) is still under development, 
# it is the intended way to make the various WITHLIB's once we get this
# working.

# usage: 
#   HPL_DEFINE_SIMPLE_WITHLIB(withname, description, headername, fn_in_lib)
# assumes that -l<withname> would link with the library.

m4_define([HPL_DEFINE_SIMPLE_WITHLIB],
[
    m4_define(m4_format([HPL_WITHLIB_%s], translit($1, [a-z], [A-Z])),
AC_ARG_WITH($1,
  m4_format(["--without-%15s disable %s support"], $1, $2),
  m4_format(["with_%s=$withval"], $1),
  m4_format(["with_%s=yes"], $1))

m4_format([[HPL_WITHLIB_%s]], translit($1, [a-z], [A-Z]))
if test m4_format(["$with_%s"], translit($1, [A-Z], [a-z])) = yes; then
	AC_MSG_NOTICE([Found required library $1])
else
	AC_MSG_ERROR([Could not find required library $1])
fi
)
])

AC_DEFUN([HPL_WITHLIB_TCL],
[
AC_ARG_WITH(tcl,
  [  --without-tcl           disable TCL support],
  [with_tcl=$withval],
  [with_tcl='yes'])

# check for tcl...; like to have a better method for doing this bit of goo
have_tcl_hdr='no'
have_tcl_lib='no'
TCL_CFLAGS=''
TCL_LIBS=''
if test ! "$with_tcl" = 'no'; then
	# keep the version specific checks in the same order
	AC_CHECK_HEADER(tcl.h,have_tcl_hdr=yes,)
	if test $have_tcl_hdr = no; then
		AC_CHECK_HEADER(/usr/include/tcl8.4/tcl.h,have_tcl_hdr=8.4;TCL_CFLAGS="-I/usr/include/tcl8.4",)
	fi
	if test $have_tcl_hdr = no; then
		AC_CHECK_HEADER(/usr/include/tcl8.3/tcl.h,have_tcl_hdr=8.3;TCL_CFLAGS="-I/usr/include/tcl8.3",)
	fi
	if test $have_tcl_hdr = no; then
		AC_CHECK_HEADER(/usr/local/include/tcl.h,have_tcl_hdr=local;TCL_CFLAGS="-I/usr/local/include",)
	fi

	AC_CHECK_LIB(tcl,Tcl_Alloc,have_tcl_lib=yes;TCL_LIBS=-ltcl,)
	if test $have_tcl_lib = no; then
		AC_CHECK_LIB(tcl8.4,Tcl_Alloc,have_tcl_lib=8.4;TCL_LIBS=-ltcl8.4,)
	fi
	if test $have_tcl_lib = no; then
		AC_CHECK_LIB(tcl8.3,Tcl_Alloc,have_tcl_lib=8.3;TCL_LIBS=-ltcl8.3,)
	fi
	if test $have_tcl_lib = no; then
		save_tcl_ldflags=$LDFLAGS
		LDFLAGS="-L/usr/local/lib $LDFLAGS"
		AC_CHECK_LIB(tcl,Tcl_Free,have_tcl_lib=local;TCL_LIBS="-L/usr/local/lib -ltcl",)
		LDFLAGS=$save_tcl_ldflags
	fi
fi
AC_MSG_CHECKING(for consistent TCL support)
if test $have_tcl_hdr = no -o $have_tcl_lib = no; then
	AC_MSG_RESULT(missing either header file or library)
	with_tcl=no
elif test $have_tcl_hdr != $have_tcl_lib; then
	AC_MSG_RESULT(inconsistent header file/library $have_tcl_hdr != $have_tcl_lib)
	TCL_LIBS=''
	TCL_CFLAGS=''
	with_tcl=no
else
	AC_MSG_RESULT(success -- extra include dir '$TCL_CFLAGS' -- link argument $TCL_LIBS)
	with_tcl=yes
fi
AC_SUBST(TCL_CFLAGS)
AC_SUBST(TCL_LIBS)
AM_CONDITIONAL(WITH_TCL, test $with_tcl = yes)
])



AC_DEFUN([HPL_WITHLIB_GC],
[
AC_ARG_WITH(gc,
  [  --without-gc           disable garbage collection support],
  [with_gc=$withval],
  [with_gc='yes'])

have_gc_hdr='no'
have_gc_lib='no'
GC_LIBS=''
GC_CFLAGS=''
if test ! "$with_gc" = 'no'; then
        AC_CHECK_HEADER(gc.h,have_gc_hdr=yes,)
	if test $have_gc_hdr = no; then
	        AC_CHECK_HEADER(/usr/include/gc/gc.h,have_gc_hdr=yes;GC_CFLAGS="-I/usr/include/gc",)
	fi

	AC_CHECK_LIB(gc,GC_malloc,have_gc_lib=yes;GC_LIBS=-lgc,)
fi
AC_MSG_CHECKING(for GC support)
if test $have_gc_hdr = yes -a $have_gc_lib = yes; then
         AC_MSG_RESULT(success -- extra include dir '$GC_CFLAGS' -- link argument $GC_LIBS)
else
	AC_MSG_RESULT(failed: missing header or library or configured without gc)
	with_gc=no
fi
AC_SUBST(GC_CFLAGS)
AC_SUBST(GC_LIBS)
AM_CONDITIONAL(WITH_GC, test $with_gc = yes)
])


AC_DEFUN([HPL_REQUIRELIB_LINTEL],
[
AC_LANG_ASSERT(C++)
AC_MSG_CHECKING(for lintel-config)
LINTEL_CFLAGS=
LINTEL_LIBS=
LINTEL_LIBTOOL=
LINTEL_ANYGCLIBS=
LINTEL_REQUIREGCLIBS=
LINTEL_NOGCLIBS=
have_lintel=no
# this ordering of tests causes the build to prefer the version of Lintel
# installed into the prefix, rather than the one in the user's path
save_exec_prefix=$exec_prefix
if test $exec_prefix = NONE; then
    exec_prefix=$prefix
fi
LINTEL_CONFIG=`eval echo $bindir`/lintel-config
LINTEL_CONFIG_TRY1=$LINTEL_CONFIG
exec_prefix=$save_exec_prefix
LINTEL_VERSION=`$LINTEL_CONFIG --version 2>/dev/null`
if test "$LINTEL_VERSION" = ""; then
    LINTEL_CONFIG=lintel-config
fi
LINTEL_VERSION=`$LINTEL_CONFIG --version 2>/dev/null`

if test "$LINTEL_VERSION" = ""; then
    AC_MSG_RESULT(failed -- tried $LINTEL_CONFIG and $LINTEL_CONFIG_TRY1)
else
    AC_MSG_RESULT(success -- using $LINTEL_CONFIG)
    LINTEL_CFLAGS=`$LINTEL_CONFIG --cflags || exit 1`
    LINTEL_LIBTOOL=`$LINTEL_CONFIG --libtool-libs || exit 1`
    LINTELPTHREAD_LIBTOOL=`$LINTEL_CONFIG --libtool-lintelpthread || exit 1`
    LINTEL_LIBS=`$LINTEL_CONFIG --libs || exit 1`
    LINTEL_MUSTGCLIBS=`$LINTEL_CONFIG --mustgclibs || exit 1`
    LINTEL_NOGCLIBS=`$LINTEL_CONFIG --nogclibs || exit 1`
    save_lintel_cppflags=$CPPFLAGS
    CPPFLAGS="$CPPFLAGS $LINTEL_CFLAGS"
    AC_CHECK_HEADER(LintelAssert.H,have_lintel=yes,have_lintel=no)
    CPPFLAGS=$save_lintel_cppflags
    if test $have_lintel = yes; then
        have_lintel=no
	save_lintel_ldflags=$LDFLAGS
	LDFLAGS="$LDFLAGS $LINTEL_LIBS"
        AC_CHECK_LIB(Lintel,lintelVersion,have_lintel=yes,have_lintel=no,)
	LDFLAGS=$save_lintel_ldflags
    fi
    if test "$LINTEL_CFLAGS" = ""; then
	have_lintel=no
    fi
    if test "$LINTEL_LIBTOOL" = ""; then
	have_lintel=no
    fi
    if test "$LINTEL_LIBS" = ""; then
	have_lintel=no
    fi
    AC_MSG_CHECKING(for Lintel)
    if test $have_lintel = no; then
        AC_MSG_RESULT(failed)
        AC_MSG_NOTICE(found lintel-config as $LINTEL_CONFIG.)
        AC_MSG_NOTICE(cflags: $LINTEL_CFLAGS)
        AC_MSG_NOTICE(lib: $LINTEL_LIBS)
        AC_MSG_FAILURE(but couldn't get compilation to work; this is broken)
        exit 1
    fi
    AC_MSG_RESULT(success -- version $LINTEL_VERSION)
fi

if test "$have_lintel" = 'yes'; then
    :
else
    AC_MSG_FAILURE(Couldn't find a working version of Lintel, aborting)
    exit 1
fi

AC_SUBST(LINTEL_CFLAGS)
AC_SUBST(LINTEL_LIBTOOL)
AC_SUBST(LINTELPTHREAD_LIBTOOL)
AC_SUBST(LINTEL_LIBS)
AC_SUBST(LINTEL_MUSTGCLIBS)
AC_SUBST(LINTEL_NOGCLIBS)
])


AC_DEFUN([HPL_REQUIRELIB_XML2],
[
XML2_CFLAGS=`xml2-config --cflags`
XML2_LIBTOOL=`xml2-config --libtool-libs`
XML2_LIBS=`xml2-config --libs`
if test `echo $XML2_LIBTOOL | grep xml2.la | wc -l` = 1; then
    :
else
    AC_MSG_WARN([You have a pretty old version of libxml2, libxml2-config does not support the --libtool-libs option])
    XML2_LIBTOOL=$XML2_LIBS
fi
have_xml2=no
save_xml2_cppflags=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $XML2_CFLAGS"
AC_CHECK_HEADER(libxml/parser.h,have_xml2=yes,have_xml2=no)
CPPFLAGS=$save_xml2_cppflags
if test $have_xml2 = yes; then
    have_xml2=no
    save_xml2_ldflags=$LDFLAGS
    LDFLAGS="$LDFLAGS $XML2_LIBS"
    AC_CHECK_LIB(xml2,xmlParseMemory,have_xml2=yes,have_xml2=no,$LDFLAGS)
    LDFLAGS=$save_xml2_ldflags
fi
if test "$have_xml2" = 'yes'; then
    :
else
    AC_MSG_FAILURE(Could not find a working version of libxml2, aborting)
    exit 1
fi
AC_SUBST(XML2_CFLAGS)
AC_SUBST(XML2_LIBTOOL)
AC_SUBST(XML2_LIBS)
])

AC_DEFUN([HPL_WITHLIB_LZF],
[
AC_ARG_WITH(lzf,
  [  --without-lzf           disable LZF compression support],
  [with_lzf=$withval],
  [with_lzf='yes'])

LZF_LIBS=''
if test "$with_lzf" = yes; then
    have_lzf_hdr=no
    have_lzf_lib=no
    AC_LANG_ASSERT(C)
    AC_CHECK_HEADER(lzf.h,have_lzf_hdr=yes,)
    AC_CHECK_LIB(lzf,lzf_compress,have_lzf_lib=yes;LZF_LIBS=-llzf)
    
    if test $have_lzf_hdr = yes -a $have_lzf_lib = yes; then
    	with_lzf=yes
    else
        with_lzf=no
    	LZF_LIBS=''
    fi
fi
AC_SUBST(LZF_LIBS)
AM_CONDITIONAL(WITH_LZF, test $with_lzf = yes)
])

HPL_DEFINE_REQUIRELIB(lzf)

AC_DEFUN([HPL_WITHLIB_LZO],
[
AC_ARG_WITH(lzo,
  [  --without-lzo           disable LZO compression support],
  [with_lzo=$withval],
  [with_lzo='yes'])

LZO_LIBS=''
if test "$with_lzo" = yes; then
    have_lzo_hdr=no
    have_lzo_lib=no
    AC_LANG_ASSERT(C)
    AC_CHECK_HEADER(lzo1x.h,have_lzo_hdr=yes,)
    AC_CHECK_LIB(lzo,lzo1x_999_compress_level,have_lzo_lib=yes;LZO_LIBS=-llzo)

    if test $have_lzo_hdr = yes -a $have_lzo_lib = yes; then
    	with_lzo=yes
    else
        with_lzo=no
    	LZO_LIBS=''
    fi
fi
AC_SUBST(LZO_LIBS)
AM_CONDITIONAL(WITH_LZO, test $with_lzo = yes)
])


AC_DEFUN([HPL_WITHLIB_ZLIB],
[
AC_ARG_WITH(zlib,
  [  --without-zlib           disable ZLIB compression support],
  [with_zlib=$withval],
  [with_zlib='yes'])

ZLIB_LIBS=''
if test "$with_zlib" = yes; then
    have_zlib_hdr=no
    have_zlib_lib=no
    AC_LANG_ASSERT(C)
    AC_CHECK_HEADER(zlib.h,have_zlib_hdr=yes,)
    AC_CHECK_LIB(z,compress2,have_zlib_lib=yes;ZLIB_LIBS=-lz)
    
    if test $have_zlib_hdr = yes -a $have_zlib_lib = yes; then
    	with_zlib=yes
    else
    	with_zlib=no
    	ZLIB_LIBS=''
    fi
fi

AC_SUBST(ZLIB_LIBS)
AM_CONDITIONAL(WITH_ZLIB, test $with_zlib = yes)
])

HPL_DEFINE_REQUIRELIB(zlib)

AC_DEFUN([HPL_WITHLIB_BZ2],
[
AC_ARG_WITH(bz2,
  [  --without-bz2           disable BZ2 compression support],
  [with_bz2=$withval],
  [with_bz2='yes'])

BZ2_LIBS=''
if test "$with_bz2" = yes; then
    have_bz2_hdr=no
    have_bz2_lib=no
    AC_LANG_ASSERT(C)
    AC_CHECK_HEADER(bzlib.h,have_bz2_hdr=yes,)
    AC_CHECK_LIB(bz2,BZ2_bzBuffToBuffCompress,have_bz2_lib=yes;BZ2_LIBS=-lbz2)
    
    if test $have_bz2_hdr = yes -a $have_bz2_lib = yes; then
    	with_bz2=yes
    else
        with_bz2=no
	BZ2_LIBS=''
    fi
fi

AC_SUBST(BZ2_LIBS)
AM_CONDITIONAL(WITH_BZ2, test $with_bz2 = yes)
])

HPL_DEFINE_REQUIRELIB(bz2)

AC_DEFUN([HPL_WITHLIB_PCRE],
[
AC_ARG_WITH(pcre,
  [  --without-pcre           disable perl compatible regular expression support],
  [with_pcre=$withval],
  [with_pcre='yes'])

PCRE_LIBS=''
if test "$with_pcre" = yes; then
   have_pcre_hdr=no
   have_pcre_lib=no
   AC_LANG_ASSERT(C)
   AC_CHECK_HEADER(pcre.h,have_pcre_hdr=yes,)
   AC_CHECK_LIB(pcre,pcre_get_substring,have_pcre_lib=yes;PCRE_LIBS=-lpcre)

   if test $have_pcre_hdr = yes -a $have_pcre_lib = yes; then
      with_pcre=yes
   else
      with_pcre=no
      PCRE_LIBS='' 
   fi
else
   with_pcre=no
fi

AC_SUBST(PCRE_LIBS)
AM_CONDITIONAL(WITH_PCRE, test $with_pcre = yes)
])

AC_DEFUN([HPL_WITHLIB_CRYPTO],
[
AC_ARG_WITH(crypto,
  [  --without-crypto           disable OpenSSL cryptographic library support],
  [with_crypto=$withval],
  [with_crypto='yes'])

CRYPTO_LIBS=''
if test "$with_crypto" = yes; then
   have_crypto_hdr=no
   have_crypto_lib=no
   AC_LANG_ASSERT(C)
   AC_CHECK_HEADER(openssl/sha.h,have_crypto_hdr=yes,)
   AC_CHECK_LIB(crypto,SHA1,have_crypto_lib=yes;CRYPTO_LIBS=-lcrypto)

   if test $have_crypto_hdr = yes -a $have_crypto_lib = yes; then
      with_crypto=yes
   else
      with_crypto=no
      CRYPTO_LIBS='' 
   fi
else
   with_crypto=no
fi

AC_SUBST(CRYPTO_LIBS)
AM_CONDITIONAL(WITH_CRYPTO, test $with_crypto = yes)
])

HPL_DEFINE_REQUIRELIB(crypto)

AC_DEFUN([HPL_WITHLIB_PCAP],
[
AC_ARG_WITH(pcap,
  [  --without-pcap           disable libpcap support],
  [with_pcap=$withval],
  [with_pcap='yes'])

PCAP_LIBS=''
if test "$with_pcap" = yes; then
   have_pcap_hdr=no
   have_pcap_lib=no
   AC_LANG_ASSERT(C)
   AC_CHECK_HEADER(pcap.h,have_pcap_hdr=yes,)
   AC_CHECK_LIB(pcap,pcap_loop,have_pcap_lib=yes;PCAP_LIBS=-lpcap)

   if test $have_pcap_hdr = yes -a $have_pcap_lib = yes; then
      with_pcap=yes
   else
      with_pcap=no
      PCAP_LIBS='' 
   fi
else
   with_pcap=no
fi

AC_SUBST(PCAP_LIBS)
AM_CONDITIONAL(WITH_PCAP, test $with_pcap = yes)
])

AC_DEFUN([HPL_WITH_BOOST_INCLUDES],
[
AC_ARG_WITH(boost-includes,
  [  --without-boost-includes           disable boost support],
  [with_boost_includes=$withval],
  [with_boost_includes='yes'])

if test "$with_boost_includes" = yes; then
   AC_LANG_ASSERT(C++)
   have_boost_format_hdr=no
   AC_CHECK_HEADER(boost/format.hpp,have_boost_format_hdr=yes,)
   have_boost_cstdint_hdr=no
   AC_CHECK_HEADER(boost/cstdint.hpp,have_boost_cstdint_hdr=yes,)

   if test $have_boost_format_hdr = yes -a $have_boost_cstdint_hdr = yes; then
      with_boost_includes=yes
   else
      with_boost_includes=no
   fi
else
   with_boost_includes=no
fi

AM_CONDITIONAL(WITH_BOOST_INCLUDES, test $with_boost_includes = yes)
])

AC_DEFUN([COM_HP_HPL_LINTEL_OPTMODE],
[
# make sure that any call to AC_PROG_CC/CXX occur after this macro so
# that the optmode macro can go hunting around for the "preferred"
# compiler if it's not specified and an optmode is specified

AC_ARG_ENABLE(optmode,
  [  --enable-optmode={debug,profile,optimize}     select a default optimization mode based on compilation host],
  [enable_optmode=$enableval],
  [enable_optmode=no])

# default compiler setting stuff...
LINTEL_OPTMODE_COMPILER=unknown
OPTMODE_LDFLAGS=
if test "$enable_optmode" = "no"; then
    :
else
    case $host in
	hppa2.0w-hp-hpux11.11)
	    optmode_fixa=no
	    optmode_fixb=no
  	    if test "$ac_test_CC" != set; then
		CC="cc -Ae"
		AC_MSG_NOTICE(Setting default C compiler to $CC)
		optmode_fixa=yes
	    fi
# What the aCC flags mean:
#       -Aa   -> turn on all ANSI features
#       -AA   -> turn on newer C++ features
#       +p    -> disallow anachronistic constructs
#       -z    -> trap null pointer dereferences
#       +w    -> warn about questionable constructs
#       +W361 -> disable "function might not return a value" warning.
#       +W392 -> disable "conversion unnecessary" warning.
#       +W655 -> disable "does not have any non-inline, non-pure virtual 
#                   member functions" warning.
#       +W469 -> disable "useless typedef" warning.
#       +W495 -> disable "linkage directive ignored for static obj/fn" 
#                   warning.
#       +W684 -> disable "integer constant -1 ... out of range"
#       +W818 -> disable type truncation warning, very common in the aCC 
#                include files
# The +W disable arguments are necessary as they are trigged by the system
# include files (i.e. we have no control over them). Hopefully someone on
# the aCC team will manage to fix the system files sometime, and these
# will go away...
	# for hpux 11.00 we found aCC -Aa -Dstd=
	    if test "$ac_test_CXX" != set; then
		CXX="aCC -AA +p -z +w +W361,392,431,469,495,655,684,818 -D__HPACC_STRICTER_ANSI__ -D_INCLUDE_LONGLONG"
		AC_MSG_NOTICE(Setting default C++ compiler to $CXX)
		optmode_fixb=yes
		OPTMODE_CXX="$CXX"
	    fi
	    if test $optmode_fixa = yes -a $optmode_fixb = yes; then
		LINTEL_OPTMODE_COMPILER=HPUX11
		AC_MSG_NOTICE([Automatically adding -Xlinker +vnocompatwarnings to OPTMODE_LDFLAGS])
		OPTMODE_LDFLAGS="$LDFLAGS -Xlinker +vnocompatwarnings"
	    fi
	;;
    esac
fi

# this has to occur before we do the optmode stuff, but after much
# experimentation, it was discovered that having the AC_PROG_CC macro
# in both sides of the if test below resulted in very strange behavior
# on debian testing as of 2005-01-01; this is probably a bug, but the
# following works around the bug while still allowing the optmode
# macro to select the default compiler

AC_PROG_CC
AC_PROG_CXX

if test "$enable_optmode" = "no"; then
    AC_MSG_NOTICE(no --enable-optmode specified)
else
    if test "$ac_test_CFLAGS" = set; then
	AC_MSG_FAILURE(Can not use --enable-optmode and also set CFLAGS, did you want to set CPPFLAGS?!)
	exit 1
    fi
    if test "$ac_test_CXXFLAGS" = set; then
	AC_MSG_FAILURE(Can not use --enable-optmode and also set CXXFLAGS, did you want to set CPPFLAGS?!)
	exit 1
    fi

    ICC_help=`$CC --help 2>&1 | grep 'Intel.*Compiler' | wc -l`
    if test $ICC_help -gt 0; then
	# brackets get removed in the following two lines so these expand out the way we expect
	ICC_VERSION=`$CC -dumpversion | awk '{print $[1]}'`
	ICC_BUILD=`$CC -V 2>&1 | grep Build | sed 's/.*Build //' | awk '{print $[1]}'`
	 LINTEL_OPTMODE_COMPILER=icc-$ICC_VERSION-$ICC_BUILD
	 if test $GNU_dumpspecs -gt 1; then
	     AC_MSG_ERROR([Whoa, looks like intel compiler, but has GNU dumpspecs])
	 fi
	 if test "`$CXX -dumpversion`" != "`$CC -dumpversion`"; then
	     AC_MSG_ERROR(version mismatch between $CXX and $CC??)
	 fi
	 AC_MSG_NOTICE([Adding -lstdc++ to LDFLAGS.  icc seems to need this to correctly link files with only .o files on the link line, do not know why])
	 LDFLAGS="$LDFLAGS -lstdc++"
    elif test "$ac_compiler_gnu" = "yes"; then
	if test "$CC" = ""; then
	    AC_MSG_FAILURE(CC variable is empty?!)
	fi
	LINTEL_OPTMODE_COMPILER=gcc-`$CC -dumpversion`
	if test "`$CXX -dumpversion`" != "`$CC -dumpversion`"; then
	    AC_MSG_FAILURE(version mismatch between $CXX and $CC??)
	    exit 1
	fi
    fi

    # three parts to the selection of optimization options
    # (gnu-host-type)::(processor-subtype)::(compiler+version)
    case $host in 
	*-linux-gnu)
	  PROCMOD=`grep 'model name' /proc/cpuinfo | head -1 | sed 's/model name.: //' | sed 's/(R)//g' | sed 's/ //g'`
	  ;;
	hppa2.0w-hp-hpux11*)
	  PROCMOD=PA2.0W
	  ;;
	
	*)
	  PROCMOD=unknown
	  ;;
    esac

    PROFILEFLAG=-pg
    # choices for optimization determined using doc/Performance, please 
    # update that file as appropriate.	
    # cases are sorted alphabetically, please preserve this
    case $host::$PROCMOD::$LINTEL_OPTMODE_COMPILER in
        hppa2.0w-hp-hpux11*::PA2.0W::HPUX11)
	   OPTFLAGS="+O3 +Onolimit +DA2.0 +Oaggressive"
	   PROFILEFLAG="-G"
	   ;;
	i686-*::*PentiumIII*::gcc-3.[34]*)
	   OPTFLAGS="-Wall -O3 -march=pentium3 -msse2 -D__pentiumpro__ -g"
  	   ;;
	i686-*::*IntelPentium4CPU*::gcc-3.[34]*)
	   OPTFLAGS="-Wall -O3 -march=pentium4 -D__pentiumpro__ -g"
	   ;;
	i686-*::*IntelXeon*::gcc-3.3*)
	   OPTFLAGS="-Wall -O3 -march=pentium4 -D__pentiumpro__ -g"
	   ;;
	i686-*::*IntelXeon*::gcc-3.4*)
	   OPTFLAGS="-Wall -O3 -march=pentium4 -D__pentium4__ -g"
	   ;;
	i686-*::*PentiumM*::gcc-2.95*)
	   OPTFLAGS="-Wall -O3 -march=i686 -msse2 -D__pentiumpro__ -g"
	   ;;
	i686-*::*PentiumM*::gcc-3.3*) 
	   OPTFLAGS="-Wall -O3 -march=pentium4 -msse2 -D__pentiumpro__ -g"
	   ;;
	i686-*::*PentiumM*::gcc-3.4*) 
	   OPTFLAGS="-Wall -O3 -march=pentium-m -D__pentiumpro__ -g"
	   ;;
	i686-*::*PentiumM*::gcc-4.*) 
	   OPTFLAGS="-Wall -O3 -march=pentium-m -D__pentiumpro__ -g"
	   ;;
	i686-*::*IntelCPUT2600*::gcc-4.*)
	   OPTFLAGS="-Wall -O3 -march=pentium-m -D__pentiumpro__ -g"
	   ;;
	i686-*::*PentiumM*::icc-8.1*)
	   OPTFLAGS="-O3 -xN -ipo -D__pentiumpro__" # can't use -xB as isnan(NAN) == false
  	   ;;
	x86_64-*::*AMDOpteron*::gcc-3.*)
	   OPTFLAGS="-Wall -O3 -march=opteron -D__pentiumpro__ -g"
	   ;;
	x86_64-*::*AMDOpteron*::gcc-4.*)
	   OPTFLAGS="-Wall -O3 -march=opteron -D__pentiumpro__ -g"
	   ;;
	i686-*::*AMDOpteron*::gcc-3.*)
	   OPTFLAGS="-Wall -O3 -march=opteron -D__pentiumpro__ -g"
	   ;;
	i686-*::*AMDOpteron*::gcc-4.*)
	   OPTFLAGS="-Wall -O3 -march=opteron -D__pentiumpro__ -g"
	   ;;
	*) AC_MSG_NOTICE(**************************************************)
	   AC_MSG_NOTICE(Unknown host ($host) processor model ($PROCMOD) compiler ($LINTEL_OPTMODE_COMPILER) combination)
	   AC_MSG_NOTICE(Leaving optimization option alone as '$CXXFLAGS')
	   AC_MSG_NOTICE(See doc/building.html under 'Add optimization flags' for how to fix)
	   AC_MSG_NOTICE([this warning, but you can safely ignore it.])
	   AC_MSG_NOTICE(sleeping 10 seconds to make this message more obvious)
	   AC_MSG_NOTICE(**************************************************)
	   OPTFLAGS="$CFLAGS"
	   PROFILEFLAGS="$CFLAGS"
	   sleep 10
	   ;;
    esac

    if test "$enable_optmode" = "debug"; then
	CFLAGS="-g -DCOMPILE_DEBUG"
	CXXFLAGS="-g -DCOMPILE_DEBUG"
	case $LINTEL_OPTMODE_COMPILER in
	    *gcc*) 
		CFLAGS="-Wall $CFLAGS"
		CXXFLAGS="-Wall $CXXFLAGS"
	        ;;
        esac
    elif test "$enable_optmode" = "optimize"; then
	CFLAGS="$OPTFLAGS -DCOMPILE_OPTIMIZE"
	CXXFLAGS="$OPTFLAGS -DCOMPILE_OPTIMIZE"
    elif test "$enable_optmode" = "profile"; then
	CFLAGS="$OPTFLAGS $PROFILEFLAG -DCOMPILE_PROFILE"
	CXXFLAGS="$OPTFLAGS $PROFILEFLAG -DCOMPILE_PROFILE"
    else
	echo "unknown enable-optmode '$enable_optmode'"
	exit 1
    fi
    OPTMODE_FLAGS="$CXXFLAGS"
    echo "setting CFLAGS to $CFLAGS"
    if test "$prefix" = "NONE"; then
	if test "$srcdir" = "."; then
		prefix="`pwd`/.."
	else
		prefix="$HOME/build/$enable_optmode"
	fi
	echo "setting prefix to $prefix"
	OPTMODE_PREFIX="$prefix"
    fi
fi
AC_SUBST(OPTMODE_LDFLAGS)
])

AC_DEFUN([COM_HP_HPL_LINTEL_OPTMODE_MESSAGES],
[
    if test "$OPTMODE_CXX" = ""; then
	:
    else
	AC_MSG_NOTICE(--enable-optmode set CXX to '$OPTMODE_CXX')
    fi
    if test "$OPTMODE_FLAGS" = ""; then
	:
    else
	AC_MSG_NOTICE(--enable-optmode set compile FLAGS to '$OPTMODE_FLAGS')
    fi
    if test "$OPTMODE_LDFLAGS" = ""; then
	:
    else
	AC_MSG_NOTICE(--enable-optmode set link OPTMODE_LDFLAGS to '$OPTMODE_LDFLAGS')
    fi
    if test "$OPTMODE_PREFIX" = ""; then
	:
    else
	AC_MSG_NOTICE(--enable-optmode set prefix to '$OPTMODE_PREFIX')
    fi
])

AC_DEFUN([HPL_PERL_SUBSTS],
[
    EXPAND_VARIABLE
    AC_PATH_PROG([PERL_BINARY], [perl], [/no/such/file], [$PATH:/usr/bin:/usr/local/bin])
    if test "$PERL_BINARY" = "/no/such/file"; then
	AC_MSG_FAILURE([can not find perl binary])
    fi
    AC_SUBST(PERL_BINARY)
    expanded_datadir=`expand_variable $datadir`
    # next line has [] around it to protect against m4 macro expansion
    [PERL_SHAREDIR=`perl -e 'foreach $i (@INC) { if ($i =~ /^$ARGV[0]\b/) { print $i;exit(0); }}; print "$ARGV[0]/perl5";' $expanded_datadir`]
    AC_SUBST(PERL_SHAREDIR)
    PERL_MODULES_INC_UNSHIFT="BEGIN { unshift(@INC,'$PERL_SHAREDIR') }"
    # next line has [] around it to protect against m4 macro expansion
    [if perl -e 'foreach $i (@INC) { exit(0) if $i eq $ARGV[0]; }; exit(1);' $PERL_SHAREDIR; then ]
	PERL_MODULES_INC_UNSHIFT="# in-searchpath $PERL_MODULES_INC_UNSHIFT";
    fi
    AC_SUBST(PERL_MODULES_INC_UNSHIFT)
    AC_MSG_NOTICE([Using $PERL_SHAREDIR for installation of perl shared libraries])
])
