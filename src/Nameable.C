/* -*-C++-*-
*******************************************************************************
*
* File:         Nameable.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/Nameable.C,v 1.7 2003/07/30 00:17:17 anderse Exp $
* Description:  Functions for nameable objects
* Author:       John Wilkes
* Created:      Sun Apr  5 23:18:17 1998
* Modified:     Thu Aug 23 14:52:45 2001 (Alistair Veitch) aveitch@hpl.hp.com
* Language:     C++
* Package:      Lintel
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 1998, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include "LintelAssert.H"
#include "Nameable.H"

std::string
Named::debugString() const
{
    const unsigned LINELEN = 1024;
    char line[LINELEN];
    
    snprintf(line, LINELEN-1, 
	     "%s [no debugString() information available]", name());
    return std::string(line);
};
