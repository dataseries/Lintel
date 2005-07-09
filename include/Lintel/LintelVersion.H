/* -*-C++-*-
*******************************************************************************
*
* File:         LintelVersion.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/LintelVersion.H,v 1.1 2005/02/14 04:39:59 anderse Exp $
* Description:  header
* Author:       Eric Anderson
* Created:      Wed Jan  5 08:55:06 2005
* Modified:     Wed Jan  5 09:01:15 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2005, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

// need the extern "C" to make the autoconf checks work easier; which
// is really why this file exists at all.
extern "C" {
    char *lintelVersion();
    char *libtoolLibLintelVersion();
}
