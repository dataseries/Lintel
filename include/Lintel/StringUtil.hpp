/* -*-C++-*- */
/*
   (c) Copyright 2004-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Utilities for dealing with strings
*/

#ifndef LINTEL_STRINGUTIL_HPP
#define LINTEL_STRINGUTIL_HPP

#include <string>
#include <vector>
#include <locale>
#include <stdint.h>

#include <boost/lexical_cast.hpp>

#include <Lintel/AssertBoost.hpp>

/** split instr into all of the separate strings separated by splitstr, and
    put the results into bits; alternately, and probably better, use
    boost::split from boost/algorithm/string.hpp */
void split(const std::string &instr, const std::string &splitstr, 
	   std::vector<std::string> &bits);

/** more normal usage variant of split, but probably slower (unchecked) */
inline std::vector<std::string>
split(const std::string &instr, const std::string &splitstr) {
    std::vector<std::string> ret;
    split(instr, splitstr, ret);
    return ret;
}

/** join parts, each entry separated by joinstr */
std::string join(const std::string &joinstr, const std::vector<std::string> &parts);

/** escapes data for passing into MySQL; doesn't escape for string
    comparison functions (e.g. the LIKE operator).  Produces \verbatim \0 \' \"
    \b \n \r \t \Z (ASCII 26) and \\  \endverbatim

    See http://dev.mysql.com/doc/refman/5.0/en/string-syntax.html for
    further details on MySQL's string escaping requirements.
*/
std::string mysqlEscape(const void * data, unsigned datasize);

inline std::string mysqlEscape(const std::string &instr) {
    return mysqlEscape(instr.data(), instr.size());
}

/** convert data for size bytes into a hex string */
std::string hexstring(const void *data, unsigned datasize);

/** convert instr into a hex string */
inline std::string hexstring(const std::string &instr) {
    return hexstring(instr.data(), instr.size());
}

/** even though it is a void *, maybe it is completely printable... */
std::string maybehexstring(const void *data, unsigned datasize);

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

/** convert a string into the given integer type. Asserts out if given a
    bad string or value is too large to fit in given type */
template<typename T>
T stringToInteger(const std::string &str, int base = 10) {
    BOOST_STATIC_ASSERT(boost::is_integral<T>::value);
    T ret = 0;
    if (base == 10) {
	try {
	    ret = boost::lexical_cast<T>(str);
	}
	catch (boost::bad_lexical_cast &) {
	    FATAL_ERROR(boost::format("error converting '%s' to integer") % str);
	}
    }
    else {
	std::istringstream iss(str);
	switch(base) {
	case 16:
	    iss >> std::hex >> ret;
	    break;
	case 8:
	    iss >> std::oct >> ret;
	    break;
	default:
	    FATAL_ERROR(boost::format("base %d unsupported: 8, 10 or 16 only") % base);
	    break;
	}
	if (iss.fail()) {
	    FATAL_ERROR(boost::format("error converting '%s' to integer") % str);
	}
    }
    return ret;
}

/** returns true if the string is all blanks according to isblank(3) */
bool stringIsBlank(const std::string &);

/** returns true if the string is all spaces according to isspace(3) */
bool stringIsSpace(const std::string &);

/** returns the current host's hostname.  the function tries to return
 * a FQDN, but this is not guaranteed -- if the effort fails, it simply
 * returns the result of gethostname(2) as a std::string. */
std::string getHostFQDN();

/** This is a form of strerror_r(3) that is convenient to use. */
std::string stringError(int errnum);

/** Applies tolower to each character in the supplied string s
    and returns the result. */
std::string downcaseString(const std::string &s);

/** Applies toupper to each character in the supplied string s
    and returns the result. */
std::string upcaseString(const std::string &s);

// TODO: see if we can do something to support wstring on cygwin
#ifndef __CYGWIN__
/** Converts a string to a wstring, using the supplied locale, which
    defaults to the current locale. */
std::wstring string2wstring(const std::string &s, 
			    const std::locale &loc = std::locale());

/** Converts a wstring to a string, using the supplied locale, which
    defaults to the current locale.  The dfault char parameter supples
    the characeter to be substituted wherever narrowing is not
    possible. */
std::string wstring2string(const std::wstring &s, char dfault = 0,
			   const std::locale &loc = std::locale());

#endif
#endif
