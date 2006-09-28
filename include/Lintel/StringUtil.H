/* -*-C++-*- */
/*
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Utilities for dealing with strings
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

/** determine if str starts with prefix */
bool prefixequal(const std::string &str, const std::string &prefix);

/** convert an ipv4 32 bit value into a string, first octet is first byte */
std::string ipv4tostring(unsigned long val);

/** same as strtod, but bails out if the string isn't valid */
double stringToDouble(const std::string &str);

/** same as strtol, but bails out if the string isn't valid */
long stringToLong(const std::string &str, int base = 10);

/** same as strtoll, but bails out if the string isn't valid */
long long stringToLongLong(const std::string &str, int base = 10);

#endif

