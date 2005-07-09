/* -*-C++-*-
*******************************************************************************
*
* File:         LintelVersion.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/LintelVersion.C,v 1.1 2005/02/14 04:36:52 anderse Exp $
* Description:  Lintel version implementation; mostly for configure checking
* Author:       Eric Anderson
* Created:      Wed Jan  5 08:53:49 2005
* Modified:     Wed Jan  5 09:01:28 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2005, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <LintelVersion.H>

static char *lintelVersionString = VERSION;
static char *libtoolLibLintelVersionString = LIBLINTEL_VERSION;

char *lintelVersion()
{
    return lintelVersionString;
}

char *libtoolLibLintelVersion()
{
    return libtoolLibLintelVersionString;
}

