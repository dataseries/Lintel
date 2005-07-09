/* -*-C++-*-
*******************************************************************************
*
* File:         StringUtil.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/StringUtil.H,v 1.3 2004/07/08 20:30:46 popovici Exp $
* Description:  Utilities for dealing with strings
* Author:       Eric Anderson
* Created:      Tue Apr 27 22:54:29 2004
* Modified:     Wed Jul  7 19:38:02 2004 (Florentina Popovici) popovici@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2004, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __STRINGUTIL_H
#define __STRINGUTIL_H

#include <string>
#include <vector>

/** split instr into all of the separate strings separated by splitstr, and
    put the results into bits */
void split(const std::string &instr, const std::string &splitstr, 
	   std::vector<std::string> &bits);

/** convert instr into a hex string */
std::string hexstring(const std::string &instr);

/** convert instr into a hex string if it contains non-printing characters */
std::string maybehexstring(const std::string &instr); 

/** convert instr into a CSV form as accepted by Excel */
std::string toCSVform(const std::string &instr); 

/** convert instr into a raw string, abort if it contains non-hex characters */
std::string hex2raw(const std::string &instr);

/** convert instr into a raw string if it contains only hex characters */
std::string maybehex2raw(const std::string &instr);

#endif
