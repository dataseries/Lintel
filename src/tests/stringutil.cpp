/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Tests for StringUtil
*/
#include <locale.h>

#include <boost/format.hpp>
 
#include <Lintel/AssertBoost.hpp>
#include <Lintel/Clock.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/ProgramOptions.hpp>
#include <Lintel/StringUtil.hpp>
#include <Lintel/TestUtil.hpp>

using namespace std;
using boost::format;
using lintel::ProgramOption;

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


string htmlEscapeUnprintable_slow(const string &a) {
    static char hextable[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

    string ret;
    for(size_t i=0; i<a.size(); ++i) {
        uint32_t c = static_cast<unsigned char>(a[i]);
	if (!isprint(c) || c=='%') {
            if (c=='%') {
                ret.append("%%");
            } else {
                ret.push_back('%');
                ret.push_back(hextable[(c >> 4) & 0xF]);
                ret.push_back(hextable[c & 0xF]);
            }
	} else {
            ret.push_back(c);
        }
    }
    return ret;
}

ProgramOption<uint32_t> po_heu_speed_test_iterations
  ("heu-speed-test-iterations", "Number of iterations for the html escape unprintable speed test");

ProgramOption<uint32_t> po_heu_random_seed
  ("heu-random-seed", "Specify the random seed for the html escape unprintable test");

void test_htmlEscapeUnprintable() {
    string pre_a("This has a few % characters, e.g. %20 in html.");
    string post_a("This has a few %% characters, e.g. %%20 in html.");
    SINVARIANT(htmlEscapeUnprintable(pre_a) == post_a);

    string pre_b("This is entirely printable");
    string post_b("This is entirely printable");
    SINVARIANT(htmlEscapeUnprintable(pre_b) == post_b);

    string pre_c("XYasdXee");
    pre_c[0] = 0;
    pre_c[1] = 0xff;
    pre_c[5] = 0xfe;
    string post_c("%00%ffasd%feee");
    SINVARIANT(htmlEscapeUnprintable(pre_c) == post_c);

    string pre_d("XYasdXee%");
    pre_d[0] = 0;
    pre_d[1] = 0xff;
    pre_d[5] = 0xfe;
    string post_d("%00%ffasd%feee%%");
    SINVARIANT(htmlEscapeUnprintable(pre_d) == post_d);

    MersenneTwisterRandom rng;

    if (po_heu_random_seed.used()) {
        rng.init(po_heu_random_seed.get());
    }
    LintelLog::info(format("Using --heu-random-seed=%d") % rng.seed_used);

    for (uint32_t i = 0; i < 500; ++i) {
        string in;
        for (uint32_t j = 0; j < 100; ++j) {
            in.push_back(rng.randInt(256));
        }
        SINVARIANT(htmlEscapeUnprintable_slow(in) == htmlEscapeUnprintable(in));
    }

    if (po_heu_speed_test_iterations.used()) {
        uint32_t iters = po_heu_speed_test_iterations.get();
        vector<string> in, out;
        in.resize(iters);
        out.resize(iters);

        for (uint32_t i = 0; i < iters; ++i) {
            in[i] = string("This is a fairly long string, one of my favorite strings in the world, and it goes on for a while because it is a key");
            in[i].resize(rng.randInt(in[i].size()) + 1);
            int limit = rng.randInt(5);
            for (int j = 0; j<limit; ++j) {
                in[i][rng.randInt(in[i].size())] = 128 + rng.randInt(127);
            }
        }
        Clock::Tfrac start = Clock::todTfrac();
        int64_t count = 0;
        for (uint32_t i = 0; i<iters; ++i) {
            out[i] = htmlEscapeUnprintable(in[i]);
            ++count;
        }
        Clock::Tfrac stop = Clock::todTfrac();

        // Make sure that the optimizer doesn't declare the loop above dead
        uint32_t checksum = 0;
        for (uint32_t i = 0; i < iters; ++i) {
            checksum += out[i][0];
        }

        double elapsed = Clock::TfracToDouble(stop-start);
        cout << format("heu: %f seconds, %d calls, %d nanos per call, checksum=%d\n") 
            % elapsed % count % (1.0e9 * elapsed/count) % checksum;
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
    lintel::parseCommandLine(argc, argv);

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
    test_htmlEscapeUnprintable();
    test_ucharstringadaptor();

    return 0;
}
