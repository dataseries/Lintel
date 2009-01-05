/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Tests for StringUtil
*/

#include <locale.h>
 
#include <Lintel/AssertBoost.hpp>
#include <Lintel/StringUtil.hpp>
#include <Lintel/TestUtil.hpp>

using namespace std;

void test_splitjoin() {
    vector<string> out;
    string in("a-bcd--ef");
    
    split(in, "-", out);
    SINVARIANT(out.size() == 4 && out[0] == "a" && out[1] == "bcd" &&
	       out[2] == "" && out[3] == "ef");

    vector<string> out2 = split(in, "-");
    SINVARIANT(out == out2);

    string ojoin = join("/", out);
    SINVARIANT(ojoin == "a/bcd//ef");
}

// TODO: change this to test stringToInteger instead
void test_stringtoint32() {
    SINVARIANT(stringToInteger<int32_t>("77737373") == 77737373);
    SINVARIANT(stringToInteger<int32_t>("-2133324") == -2133324);
    SINVARIANT(stringToInteger<int32_t>("0x1234abcd", 16) == 305441741);
    SINVARIANT(stringToInteger<int64_t>("0x1234abcdef5678", 16) == 5124462079858296LL);

    TEST_INVARIANTMSG(SINVARIANT(stringToInteger<int32_t>("abcdef") == 5),
		      "error converting 'abcdef' to integer");
    TEST_INVARIANTMSG(SINVARIANT(stringToInteger<int32_t>("0xghi", 16) == 0),
		      "error converting '0xghi' to integer");
}

void test_caseconversion() {
    string s("Hello world!");
    string up(upcaseString(s));
    string down(downcaseString(s));
    
    SINVARIANT(downcaseString(up) == down);
    SINVARIANT(upcaseString(down) == up);

    string empty;
    SINVARIANT(upcaseString(empty) == empty);
    SINVARIANT(downcaseString(empty) == empty);
}

// TODO: see if we can do something to support wstring on cygwin
#ifndef __CYGWIN__
void test_string2wstring() {
    string  src( "abcdefghijklmnopqrstuvwxyz0123456789");
    wstring dst(L"abcdefghijklmnopqrstuvwxyz0123456789");
    
    SINVARIANT(string2wstring(src) == dst);
}

void test_wstring2string() {
    wstring src(L"abcdefghijklmnopqrstuvwxyz0123456789");
    string  dst( "abcdefghijklmnopqrstuvwxyz0123456789");
    
    SINVARIANT(wstring2string(src) == dst);
}
#endif

// TODO: test the remainder of things in StringUtil.hpp

int main(int argc, char *argv[]) {
    test_splitjoin();
    test_stringtoint32();
    test_caseconversion();
// TODO: see if we can do something to support wstring on cygwin
#ifndef __CYGWIN__
    test_string2wstring();
    test_wstring2string();
#endif
    return 0;
}
