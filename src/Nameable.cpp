/* -*-C++-*-
   (c) Copyright 1998-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Functions for nameable objects
*/

#include <Lintel/LintelAssert.hpp>
#include <Lintel/Nameable.hpp>

std::string
Named::debugString() const
{
    const unsigned LINELEN = 1024;
    char line[LINELEN];
    
    snprintf(line, LINELEN-1, 
	     "%s [no debugString() information available]", name());
    return std::string(line);
};
