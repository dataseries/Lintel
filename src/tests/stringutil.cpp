/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Tests for StringUtil
*/
#include <iostream>
#include <locale.h>

#include <boost/format.hpp>
 
#include <Lintel/AssertBoost.hpp>
#include <Lintel/Clock.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
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

void test_stringtoint32() {
    SINVARIANT(stringToInteger<int32_t>("77737373") == 77737373);
    SINVARIANT(stringToInteger<int32_t>("-2133324") == -2133324);
    SINVARIANT(stringToInteger<int32_t>("0x1234abcd", 16) == 305441741);
    SINVARIANT(stringToInteger<int64_t>("0x1234abcdef5678", 16) == 5124462079858296LL);

    TEST_INVARIANTMSG(SINVARIANT(stringToInteger<int32_t>("abcdef") == 5),
		      "error converting 'abcdef' to integer, not an integer or outside of range [-2147483648 .. 2147483647]");
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

void test_mysqlEscape() {
    string pre("XY\"\'\t\b\n\r\\%_0bnrtZ");
    pre[0] = 26;
    pre[1] = 0;
    string post("\\Z\\0\\\"\\\'\\t\\b\\n\\r\\\\%_0bnrtZ");
    INVARIANT(mysqlEscape(pre) == post, "\n" + post + " isn't \n" + mysqlEscape(pre));
}

void test_escapestring() {
    string pre_a("This has a few % characters, e.g. %20 in html.");
    string post_a("This has a few %% characters, e.g. %%20 in html.");
    SINVARIANT(escapestring(pre_a) == post_a);

    string pre_b("This is entirely printable");
    string post_b("This is entirely printable");
    SINVARIANT(escapestring(pre_b) == post_b);

    string pre_c("XYasdXee");
    pre_c[0] = 0;
    pre_c[1] = 0xff;
    pre_c[5] = 0xfe;
    string post_c("%00%ffasd%feee");
    SINVARIANT(escapestring(pre_c) == post_c);


    string pre_d("XYasdXee%");
    pre_d[0] = 0;
    pre_d[1] = 0xff;
    pre_d[5] = 0xfe;
    string post_d("%00%ffasd%feee%%");
    SINVARIANT(escapestring(pre_d) == post_d);

    bool speed_test = false;
    if (speed_test) {   
        MersenneTwisterRandom rng;
        string in[100000];
        string out[100000];
        for (int i = 0; i<100000; ++i) {
            in[i] = string("This is a fairly long string, one of my favorite strings in the world, and it goes on for a while because it is a key");
            int limit = rng.randInt(5);
            for (int j = 0; j<limit; ++j) {
                in[i][rng.randInt(in[i].size())] = 128 + rng.randInt(127);
            }
        }
        Clock::Tfrac start = Clock::todTfrac();
        int64_t count = 0;
        for (int i = 0; i<100000; ++i) {
            out[i] = escapestring(in[i]);
            ++count;
        }
        for(unsigned int j = 0; j<10; ++j) {
            for (unsigned int i = 0; i<100000-j; ++i) {
                if (rng.randInt(100000) != i) {
                    out[i] = escapestring(in[i+j]);
                    ++count;
                }
            }
        }
        Clock::Tfrac stop = Clock::todTfrac();
        cout << boost::format("it took %f seconds to do %d escapestring calls, or %d nanos per\n") 
            % Clock::TfracToDouble(stop-start) % count 
            % (1000000000.0 * Clock::TfracToDouble(stop-start)/count);
        // Force the optimizer to actually do all the work.
        for (int i = 0; i<100000; ++i) {
            if (rng.randInt(1000000) == 0 &&
                rng.randInt(1000000) == 0) {
                INVARIANT(out[i].size() > 2, out[i]);
            } else {
                break;
            }
        }        
    }

}

void test_hexstring() {
    string decafbad("decafbad");
    string decafbadhex("6465636166626164");
    string decafbadraw("\xde\xca\xfb\xad");

    string decaf("decaf");
    string helloworld("Hello World!");
    string helloworldhex("48656c6c6f20576f726c6421");

    SINVARIANT(ishexstring(decafbad) && ishexstring(decafbadhex) && ishexstring(helloworldhex));
    SINVARIANT(!ishexstring(decaf) && !ishexstring(helloworld) && !ishexstring(decafbadraw));

    SINVARIANT(decafbad == hexstring(decafbadraw));
    SINVARIANT(hexstring(maybehexstring(maybehex2raw(decafbad))) == decafbadhex);
    
    SINVARIANT(hexstring(helloworld)==helloworldhex && hex2raw(helloworldhex)==helloworld);
}

void test_ucharstringadaptor() {
    string s("1234");
    const unsigned char *us = UCharStringAdaptor(s).udata();
    for (unsigned int i = 0; i < s.length(); i++) {
	SINVARIANT((int)s[i] == (int)us[i]);
    }
}

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
    test_mysqlEscape();
    test_hexstring();
    test_escapestring();
    test_ucharstringadaptor();

    return 0;
}
