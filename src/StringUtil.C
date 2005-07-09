/* -*-C++-*-
*******************************************************************************
*
* File:         StringUtil.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/StringUtil.C,v 1.5 2005/02/14 04:36:52 anderse Exp $
* Description:  implementation
* Author:       Eric Anderson
* Created:      Tue Apr 27 22:55:24 2004
* Modified:     Thu Jan 20 13:09:47 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2004, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <StringUtil.H>
#include <LintelAssert.H>

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

