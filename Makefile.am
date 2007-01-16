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

# Magic to make autoreconf find com_hp_hpl_lintel.m4
# Sadly, we can't use this in other packages because the proper -I depends
# on where we are installing.
ACLOCAL_AUTORECONF_FLAGS = -I .
