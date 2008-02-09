#
#  (c) Copyright 2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
## Process this file with automake to produce Makefile.in

# avoid requiring all GNUisms
AUTOMAKE_OPTIONS = 1.7 foreign

SUBDIRS = include src

aclocaldir = $(datadir)/aclocal
aclocal_DATA = com_hp_hpl_lintel.m4

cmakemodulesdir = $(datadir)/cmake-modules
cmakemodules_DATA = \
	cmake-modules/FindLintel.cmake \
	cmake-modules/FindLibXmlpp.cmake

linteldocdir = $(datadir)/doc/Lintel
linteldoc_DATA = \
	COPYING 

# Magic to make autoreconf find com_hp_hpl_lintel.m4
# Sadly, we can't use this in other packages because the proper -I depends
# on where we are installing.
ACLOCAL_AUTORECONF_FLAGS = -I .

if HAS_CMAKE
all: all-recursive
	(cd doc; cmake -DCMAKE_INSTALL_PREFIX=$(prefix) $(top_srcdir)/doc)
	(cd doc; make all)

install: install-recursive
	(cd doc; make install)

endif
