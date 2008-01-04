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
#include <inttypes.h>

/** split instr into all of the separate strings separated by splitstr, and
    put the results into bits; alternately, and probably better, use
    boost::split from boost/algorithm/string.hpp */
void split(const std::string &instr, const std::string &splitstr, 
	   std::vector<std::string> &bits);

/** join parts, each entry separated by joinstr */
std::string join(const std::string &joinstr, const std::vector<std::string> &parts);

/** convert data for size bytes into a hex string */
std::string hexstring(const void *data, unsigned datasize);

/** convert instr into a hex string */
inline std::string hexstring(const std::string &instr) {
    return hexstring(static_cast<const void *>(instr.data()), instr.size());
}

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

/** determine if str ends with suffix */
bool suffixequal(const std::string &str, const std::string &suffix);

/** convert an ipv4 32 bit value into a string, first octet is first byte */
std::string ipv4tostring(uint32_t val);

/** convert a string #.#.#.# to an ipv4 with the first octet as the
    first byte; uses inet_aton, so 0## is octal and 0x## is hex */
uint32_t stringtoipv4(const std::string &str);

/** same as strtod, but bails out if the string isn't valid */
double stringToDouble(const std::string &str);

// TODO: deprecate stringToLong, stringToLongLong in favor of [u]int{32,64}
/** same as strtol, but bails out if the string isn't valid */
long stringToLong(const std::string &str, int base = 10);

/** same as strtoll, but bails out if the string isn't valid */
long long stringToLongLong(const std::string &str, int base = 10);

/// Converts a string to an int32, INVARIANT if not a valid string
int32_t stringToInt32(const std::string &str, int base = 10);

/// Converts a string to an uint32, INVARIANT if not a valid string
uint32_t stringToUInt32(const std::string &str, int base = 10);

/// Converts a string to an int32, INVARIANT if not a valid string
int64_t stringToInt64(const std::string &str, int base = 10);

/// Converts a string to an uint32, INVARIANT if not a valid string
uint64_t stringToUInt64(const std::string &str, int base = 10);

/** returns true if the string is all blanks according to isblank(3) */
bool stringIsBlank(const std::string &);

/** returns true if the string is all spaces according to isspace(3) */
bool stringIsSpace(const std::string &);

/** returns the current host's hostname.  the function tries to return
 * a FQDN, but this is not guaranteed -- if the effort fails, it simply
 * returns the result of gethostname(2) as a std::string. */
std::string getHostFQDN();

#endif

