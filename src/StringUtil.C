/* -*-C++-*-
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    some string utilities
*/

#define __STDC_LIMIT_MACROS

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/socket.h>

#include <boost/static_assert.hpp>

#include <Lintel/StringUtil.H>
#include <Lintel/LintelAssert.H>
#include <Lintel/AssertBoost.H>

#ifdef __HP_aCC
#include <inttypes.h>
#endif

#if defined(__HP_aCC) && __HP_aCC < 35000
#else
using namespace std;
#endif

void 
split(const string &instr, const string &splitstr, vector<string> &bits) 
{
    AssertAlways(splitstr.length() > 0,("bad"));
    int startbit = 0;
    while(true) {
	int endbit = instr.find(splitstr,startbit);
	if (endbit < 0) {
	    bits.push_back(instr.substr(startbit,instr.length()-startbit));
	    return;
	} else {
	    AssertAlways(endbit >= startbit,("bad %d %d",startbit,endbit));
	    bits.push_back(instr.substr(startbit,endbit-startbit));
	    startbit = endbit + splitstr.length();
	}
    }
}

string
join(const string &joinstr, const vector<string> &bits)
{
    string ret;

    unsigned size = 0;
    for(vector<string>::const_iterator i = bits.begin(); 
	i != bits.end(); ++i) {
	if (i != bits.begin())
	    size += joinstr.size();
	size += i->size();
    }

    ret.reserve(size);

    for(vector<string>::const_iterator i = bits.begin(); 
	i != bits.end(); ++i) {
	if (i != bits.begin())
	    ret.append(joinstr);
	ret.append(*i);
    }
    return ret;
}


static char hextable[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

string
hexstring(const void *_data, unsigned datasize)
{
    const unsigned char *data = reinterpret_cast<const unsigned char *>(_data);
    string ret;
    ret.resize(datasize*2);
    for(unsigned int i=0;i<datasize;++i) {
        unsigned int c = (unsigned char)(data[i]);
        ret[i*2] = hextable[(c >> 4) & 0xF];
        ret[i*2+1] = hextable[c & 0xF];
    }
    return ret;
}

string
maybehexstring(const string &a)
{
    for(unsigned int i=0;i<a.size();++i) {
	if (!isprint(a[i])) {
	    return hexstring(a);
	}
    }
    return a;
}

// writes a char string in CSV form (as it is accepted by Excel)
// if the string has commas then we need to use "" around the string
// if the string has " then we need to double the existent quotes 
string 
toCSVform(const string &s)
{
    string ret(s);
    int pos = -1; 

    if ((pos = ret.find(",",0)) >= 0){
	ret.insert(0,"\"");
	pos = 1;
	while ((pos = ret.find("\"",pos)) >= 0){
	    ret.insert(pos,"\"");
	    pos = pos + 2;
	}
	ret.append("\"");
    }

    return ret;
}

static int 
unpackhexchar(const char v)
{
    if (v >= '0' && v <= '9')
	return v - '0';
    if (v >= 'a' && v <= 'f')
	return 10 + v - 'a';
    AssertFatal(("bad %d\n",v));
    return -1;
}

static unsigned char
unpackhex(const char *v)
{
    int v1 = unpackhexchar(v[0]);
    int v2 = unpackhexchar(v[1]);
    return (unsigned char)((v1 << 4) | v2);
}

string
hex2raw(const string &in)
{
    AssertAlways((in.size() % 2) == 0,("bad\n"));
    string out;
    out.resize(in.size()/2);
    for(unsigned int i=0;i<out.size();++i) {
	out[i] = unpackhex(in.data()+2*i);
    }
    return out;
}

string
maybehex2raw(const string &in)
{
    if ((in.size() % 2) != 0) {
	return in;
    }
    for(unsigned int i=0;i<in.size();++i) {
	if (!isxdigit(in[i])) {
	    return in;
	}
    }
    return hex2raw(in);
}

bool
prefixequal(const string &str, const string &prefix)
{
    return str.compare(0,prefix.size(),prefix) == 0;
}

bool
suffixequal(const string &str, const string &prefix)
{
    if (str.size() < prefix.size())
	return false;
    return str.compare(str.size() - prefix.size(),prefix.size(),prefix) == 0;
}


string
ipv4tostring(uint32_t val)
{
    char buf[50];
    sprintf(buf,"%d.%d.%d.%d",
	    (unsigned int)(val >> 24) & 0xFF,
	    (unsigned int)(val >> 16) & 0xFF,
	    (unsigned int)(val >> 8) & 0xFF,
	    (unsigned int)(val >> 0) & 0xFF);
    return string(buf);
}

uint32_t
stringtoipv4(const string &val)
{
    struct in_addr addr;
    INVARIANT(inet_aton(val.c_str(), &addr) != 0,
	      boost::format("Unable to convert %s to inet address")
	      % val);
    return static_cast<uint32_t>(ntohl(addr.s_addr));
}

double
stringToDouble(const std::string &str)
{
    char *endptr = NULL;
    AssertAlways(str.size() > 0, 
		 ("zero length string not valid in conversion to double"));
    double ret = strtod(str.c_str(),&endptr);
    AssertAlways(*endptr == '\0',("didn't parse all of '%s' as a double",
				  str.c_str()));
    return ret;
}

// TODO: this seems to silently accept longs > 2^31-1 and truncates them to
// 2^31-1.

long
stringToLong(const std::string &str, int base)
{
    char *endptr = NULL;
    AssertAlways(str.size() > 0, 
		 ("zero length string not valid in conversion to double"));
    long ret = strtol(str.c_str(),&endptr, base);
    AssertAlways(*endptr == '\0',("didn't parse all of '%s' as a long long",
				  str.c_str()));
    return ret;
}

#ifdef __HP_aCC
#define strtoll __strtoll
#endif

long long
stringToLongLong(const std::string &str, int base)
{
    char *endptr = NULL;
    AssertAlways(str.size() > 0, 
		 ("zero length string not valid in conversion to double"));
    long long ret = strtoll(str.c_str(),&endptr, base);
    AssertAlways(*endptr == '\0',("didn't parse all of '%s' as a long long",
				  str.c_str()));
    return ret;
}

// FUTURE: Seems there should be some way to templatize the next 4 of
// these since they're all the same code except substitutions of
// [u]int{32,64}_t, and strto[u]l[l]

int32_t
stringToInt32(const std::string &str, int base)
{
    BOOST_STATIC_ASSERT(sizeof(long int) >= sizeof(int32_t));

    errno = 0;
    char *endptr = NULL;
    INVARIANT(str.size() > 0,
	      "string must not be size 0 for convertion to an int32");
    long int v = strtol(str.c_str(), &endptr, base);
    if (sizeof(long int) != sizeof(int32_t)) {
	if (v > INT32_MAX || v < INT32_MIN) {
	    errno = ERANGE;
	}
    }
    INVARIANT(errno == 0 && endptr == str.c_str() + str.size(), 
	      boost::format("error in conversion of '%s' base %d to int32: %s")
	      % str % base % strerror(errno));
    return v;
}

uint32_t
stringToUInt32(const std::string &str, int base)
{
    BOOST_STATIC_ASSERT(sizeof(unsigned long int) >= sizeof(uint32_t));

    errno = 0;
    char *endptr = NULL;
    INVARIANT(str.size() > 0,
	      "string must not be size 0 for convertion to an uint32");
    unsigned long int v = strtoul(str.c_str(), &endptr, base);
    if (sizeof(unsigned long int) != sizeof(uint32_t)) {
	if (v > UINT32_MAX) {
	    errno = ERANGE;
	}
    }
    INVARIANT(errno == 0 && endptr == str.c_str() + str.size(), 
	      boost::format("error in conversion of '%s' base %d to uint32: %s")
	      % str % base % strerror(errno));
    return v;
}

int64_t
stringToInt64(const std::string &str, int base)
{
    BOOST_STATIC_ASSERT(sizeof(long long int) >= sizeof(int64_t));

    errno = 0;
    char *endptr = NULL;
    INVARIANT(str.size() > 0,
	      "string must not be size 0 for convertion to an int64");
    long long int v = strtoll(str.c_str(), &endptr, base);

    if (sizeof(long long int) != sizeof(int64_t)) {
	if (v > INT64_MAX || v < INT64_MIN) {
	    errno = ERANGE;
	}
    }
    INVARIANT(errno == 0 && endptr == str.c_str() + str.size(), 
	      boost::format("error in conversion of '%s' base %d to int64: %s")
	      % str % base % strerror(errno));
    return v;
}

uint64_t
stringToUInt64(const std::string &str, int base)
{
    BOOST_STATIC_ASSERT(sizeof(unsigned long long int) >= sizeof(uint64_t));

    errno = 0;
    char *endptr = NULL;
    INVARIANT(str.size() > 0,
	      "string must not be size 0 for convertion to an uint64");
    unsigned long long int v = strtoull(str.c_str(), &endptr, base);

    if (sizeof(unsigned long long int) != sizeof(uint64_t)) {
	if (v > UINT64_MAX) {
	    errno = ERANGE;
	}
    }
    INVARIANT(errno == 0 && endptr == str.c_str() + str.size(), 
	      boost::format("error in conversion of '%s' base %d to uint64: %s")
	      % str % base % strerror(errno));
    return v;
}
	      

bool stringIsBlank(const std::string &str)
{
    size_t n = str.size();

    for (size_t i = 0; i < n; i++) {
	if (!isblank(str[i])) return false;
    }
    return true;
}

bool stringIsSpace(const std::string &str)
{
    size_t n = str.size();

    for (size_t i = 0; i < n; i++) {
	if (!isspace(str[i])) return false;
    }
    return true;
}


std::string getHostFQDN()
{
    char buf[1024];
    buf[sizeof(buf) - 1] = '\0';
    int rc = gethostname(buf, sizeof(buf) - 1);
    INVARIANT(rc == 0, "gethostname failed");
    string hostname(buf);

    // Try to look up the host via netdb.  This is especially is
    // important on any host running DHCP, where the name returned by
    // gethostname isn't a FQDN on most configurations and usually
    // maps to a loopback address via /etc/hosts.  What we want
    // is a real IP address, if it exists.

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;	// TODO: remove and test if IPv6 works
    hints.ai_flags = AI_CANONNAME;
    struct addrinfo *res = 0;
    int error = getaddrinfo(buf, NULL, &hints, &res);
    if (error == 0) {
	for (struct addrinfo *cur = res; cur != NULL; cur = cur->ai_next) {
	    if (cur->ai_canonname) {
		hostname = string(cur->ai_canonname);
		break;
	    }
	}
    }
    
    if (res) freeaddrinfo(res);

    return hostname;
}
