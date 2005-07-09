/* -*-C++-*-
*******************************************************************************
*
* File:         streamcompat.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/streamcompat.H,v 1.3 2005/02/14 04:39:59 anderse Exp $
* Description:  ensure streams compatibility between compilers
* Author:       Alistair Veitch
* Created:      Mon Jun 30 09:33:55 2003
* Modified:     Thu Jan 20 13:08:49 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2003, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#if defined(__HP_aCC) && __HP_aCC < 35000
#include <stream.h>
#include <fstream.h>
#else
#include <iostream> 
#include <fstream>
#endif
