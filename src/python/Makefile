#
#  (c) Copyright 2002, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#


CPPFLAGS=-I/usr/include/python1.5 -I..

PYTHON=python2.2
all: 
	python setup.py build
	python setup.py install
foo:
	c++ -E $(CPPFLAGS) Lintel.C
