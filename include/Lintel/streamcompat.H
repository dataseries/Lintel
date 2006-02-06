/* -*-C++-*-
/*
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    ensure streams compatibility between compilers
*/

#if defined(__HP_aCC) && __HP_aCC < 35000
#include <stream.h>
#include <fstream.h>
#else
#include <iostream> 
#include <fstream>
#endif
