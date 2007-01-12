/* -*-C++-*-
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    some string utilities
*/

#include <stdlib.h>

#include <StringUtil.H>
#include <LintelAssert.H>

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

static char hextable[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

string
hexstring(const string &a)
{
    string ret;
    ret.resize(a.size()*2);
    for(unsigned int i=0;i<a.size();++i) {
        unsigned int c = (unsigned char)(a[i]);
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


string
ipv4tostring(unsigned long val)
{
    char buf[50];
    sprintf(buf,"%d.%d.%d.%d",
	    (unsigned int)(val >> 24) & 0xFF,
	    (unsigned int)(val >> 16) & 0xFF,
	    (unsigned int)(val >> 8) & 0xFF,
	    (unsigned int)(val >> 0) & 0xFF);
    return string(buf);
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
