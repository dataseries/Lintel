/* -*-C++-*-
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    some string utilities
*/

#define __STDC_LIMIT_MACROS

#include <cctype>

// #include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef SYS_POSIX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#ifdef SYS_NT
#include <winsock2.h>
#endif

#include <boost/bind.hpp>
#include <boost/static_assert.hpp>
#include <boost/integer_traits.hpp>
#include <boost/format.hpp>

#include <Lintel/StringUtil.hpp>
#include <Lintel/AssertBoost.hpp>

using namespace std;
using boost::format;

void 
split(const string &instr, const string &splitstr, vector<string> &bits) 
{
    SINVARIANT(splitstr.length() > 0);
    int startbit = 0;
    while(true) {
	int endbit = instr.find(splitstr,startbit);
	if (endbit < 0) {
	    bits.push_back(instr.substr(startbit,instr.length()-startbit));
	    return;
	} else {
	    INVARIANT(endbit >= startbit,
		      boost::format("bad %d %d") % startbit % endbit);
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

// TODO-future: do an escape function such as 
// http://search.cpan.org/~you/Encode-Escape-0.14/lib/Encode/Escape.pm
// with 256 char lookup for speed.

static unsigned char mysql_escape_table[256] = 
    {//   0     1     2     3     4     5     6     7     8     9    
        '0',    0,    0,    0,    0,    0,    0,    0,  'b',  't', // 0
        'n',    0,    0,  'r',    0,    0,    0,    0,    0,    0, // 1
          0,    0,    0,    0,    0,    0,  'Z',    0,    0,    0, // 2
          0,    0,    0,    0, '\"',    0,    0,    0,    0, '\'', // 3
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 4
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 5
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 6
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 7
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 8
          0,    0, '\\',    0,    0,    0,    0,    0,    0,    0, // 9
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 10
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 11
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 12
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 13
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 14
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 15
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 16
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 17
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 18
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 19
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 20
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 21
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 22
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 23
          0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 24
          0,    0,    0,    0,    0,    0 } ;                      // 25

string 
mysqlEscape(const void * _data, unsigned datasize) {
    const unsigned char *data = reinterpret_cast<const unsigned char *>(_data);
    int to_escape = 0;
    for(unsigned int i=0; i<datasize; ++i) {
	if (mysql_escape_table[data[i]]!=0) {
	    ++to_escape;
	}
    }

    string ret;
    ret.reserve(datasize+to_escape);
    for(unsigned int i=0; i<datasize; ++i) {
	if (mysql_escape_table[data[i]]!=0) {
	    ret.push_back('\\');
	    ret.push_back(mysql_escape_table[data[i]]);
	} else {
	    ret.push_back(data[i]);
	}
    }
    return ret;
}

// TODO: try a hextable with 255 2 character entries, and see if using
// .append(pair, 2) runs faster than two .push_back's another option
// is to build an 8-16 character buffer and append the whole thing in
// one go.

static char hextable[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

string hexstring(const void *_data, unsigned datasize) {
    const unsigned char *data = reinterpret_cast<const unsigned char *>(_data);
    string ret;
    ret.reserve(datasize*2);
    for(unsigned int i=0;i<datasize;++i) {
        unsigned int c = (unsigned char)(data[i]);
        ret.push_back(hextable[(c >> 4) & 0xF]);
        ret.push_back(hextable[c & 0xF]);
    }
    return ret;
}

string maybehexstring(const string &a) {
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
    FATAL_ERROR(format("bad hex character %d\n") % static_cast<unsigned>(v));
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
    INVARIANT((in.size() % 2) == 0,
	      format("can't convert '%s' to hex, not an even number of digits")
	      % in);
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
    return boost::str(boost::format("%d.%d.%d.%d")
		      % ((unsigned int)(val >> 24) & 0xFF)
		      % ((unsigned int)(val >> 16) & 0xFF)
		      % ((unsigned int)(val >> 8) & 0xFF)
		      % ((unsigned int)(val >> 0) & 0xFF));
}

// TODO: in_addr_t and friends under windows needs to be figured out
#ifdef SYS_POSIX

uint32_t
stringtoipv4(const string &val)
{
    in_addr_t addr;
    INVARIANT((addr = inet_addr(val.c_str())) != (in_addr_t)-1,
	      boost::format("Unable to convert %s to inet address")
	      % val);
    return static_cast<uint32_t>(ntohl(addr));
}

#endif // SYS_POSIX

double
stringToDouble(const string &str)
{
    char *endptr = NULL;
    INVARIANT(str.size() > 0, 
	      "zero length string not valid in conversion to double");
    double ret = strtod(str.c_str(),&endptr);
    INVARIANT(*endptr == '\0',
	      boost::format("didn't parse all of '%s' as a double") % str);
    return ret;
}

#ifdef SYS_NT
namespace {
    // isblank was a later POSIX/C99 addition, never picked up by windows
    bool isblank(char c) {
	return (c == ' ' || c == '\t');
    }
}
#endif

bool stringIsBlank(const string &str)
{
    size_t n = str.size();

    for (size_t i = 0; i < n; i++) {
	if (!isblank(str[i])) return false;
    }
    return true;
}

bool stringIsSpace(const string &str)
{
    size_t n = str.size();

    for (size_t i = 0; i < n; i++) {
	if (!isspace(str[i])) return false;
    }
    return true;
}

// not working on windows or cygwin?
#ifdef SYS_POSIX
#ifndef __CYGWIN__
string getHostFQDN()
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

#endif
#endif

string stringError(int errnum)
{
    const size_t buflen = 256;
    char buf[buflen];

#ifdef SYS_POSIX
    // Note there are two strerror_r(3). See <string.h> for details.
    char *s = ::strerror_r(errnum, buf, buflen);
    if (s != NULL) {
	return string(s);
    } else {
	return "(NULL)";
    }
#endif
#ifdef SYS_NT
    if (strerror_s(buf, buflen, errnum)) {
	return "(NULL)";
    }
    else {
	return string(buf);
    }
#endif
}

string downcaseString(const string &s)
{
    string r(s);
    string::size_type len = r.length();
    
    for (string::size_type i = 0; i < len; i++) {
	r[i] = tolower(r[i]);
    }
    return r;
}

string upcaseString(const string &s)
{
    string r(s);
    string::size_type len = r.length();
    
    for (string::size_type i = 0; i < len; i++) {
	r[i] = toupper(r[i]);
    }
    return r;
}


// TODO: see if we need to do something to support wstring on cygwin
#ifndef __CYGWIN__
wstring string2wstring(const string &s, const locale &loc)
{
    wstring out;
    out.reserve(s.size());
    const ctype<wchar_t> &ct = use_facet<ctype<wchar_t> >(loc);
    transform(s.begin(), s.end(), back_inserter(out), 
	      boost::bind(&ctype<wchar_t>::widen, boost::ref(ct), _1));
    return out;
}

string wstring2string(const wstring &s, char dfault, const locale &loc)
{
    string out;
    out.reserve(s.size());
    const ctype<wchar_t> &ct = use_facet<ctype<wchar_t> >(loc);
    transform(s.begin(), s.end(), back_inserter(out),
	      boost::bind(&ctype<wchar_t>::narrow, boost::ref(ct), _1, dfault));
    return out;
}
#endif
