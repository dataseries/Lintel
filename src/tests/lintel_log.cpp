/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Testing for LintelLog.hpp
*/

#include <iostream>

#include <boost/bind.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/LintelLog.hpp>

using namespace std;
using boost::format;

void testBasic() {
    // mostly tested in class function order, setDebugLevel in wrong place
    LintelLog::setDebugLevel("test", 1);

    SINVARIANT(LintelLog::wouldDebug("test"));
    
    LintelLog::Category test("test");
    SINVARIANT(LintelLog::wouldDebug(test));
    SINVARIANT(!LintelLog::wouldDebug(test,2));

    LintelLog::setDebugLevel("test2", 3);
    SINVARIANT(LintelLog::wouldDebug("test2",3));

    LintelLog::report("report string\nsecond report string line");

    LintelLog::debug("debug string");
    LintelLog::info("info string");
    LintelLog::warn("warn string");
    LintelLog::error("error string");

    LintelLog::report(format("report %d\nline2 %s") % 5 % "happy");
    LintelLog::debug(format("%s format") % "debug");
    LintelLog::info(format("%s format") % "info");
    LintelLog::warn(format("%s format") % "warn");
    LintelLog::error(format("%s format") % "error");
}

void testComplexMatch() {
    LintelLog::setDebugLevel("complex*", 1);

    SINVARIANT(LintelLog::wouldDebug("complex-foo"));
    LintelLogDebug("complex-bar", "complex-match");
    LintelLogDebug("env-foo", "env-complex-match");
    LintelLog::info("complex-match-ok");
}

string last_message;
LintelLog::LogType last_logtype;

void saveAppender(const string &msg, const LintelLog::LogType logtype) {
    last_message = msg;
    last_logtype = logtype;
}

void testVariable(const string &cat) {
    LintelLog::Category test(cat);

    LintelLogDebugLevelVariable(test, 1, "test-variable");
}

void testHooked() {
    LintelLog::addAppender(boost::bind(saveAppender, _1, _2));

    LintelLog::info("test");
    SINVARIANT(last_message == "test" && last_logtype == LintelLog::Info);

    LintelLogDebug("test", format("macro"));
    SINVARIANT(last_message == "macro" && last_logtype == LintelLog::Debug);

    last_message.clear();
    LintelLogDebugLevel("test", 5, "foo");
    SINVARIANT(last_message.empty() && last_logtype == LintelLog::Debug);

    testVariable("test");
    SINVARIANT(last_message == "test-variable" 
	       && last_logtype == LintelLog::Debug);
    
    last_message.clear();
    testVariable("test3");
    SINVARIANT(last_message.empty() && last_logtype == LintelLog::Debug);
}

void testEnv() {
    LintelLog::parseEnv();

    LintelLogDebug("env", "env-test");
    INVARIANT(last_message == "env-test" && last_logtype == LintelLog::Debug,
	      "environment test failed; need LINTEL_LOG_DEBUG=env set");
}

void testDebugString() {
    LintelLog::parseDebugString("ds,dslevel=5");
    LintelLogDebug("ds", "ds-test");
    SINVARIANT(last_message == "ds-test" && last_logtype == LintelLog::Debug);
    
    last_message.clear();
    LintelLogDebugLevel("ds", 2, "ds-test");
    SINVARIANT(last_message.empty() && last_logtype == LintelLog::Debug);

    LintelLogDebugLevel("dslevel", 5, "dslevel-test");
    SINVARIANT(last_message == "dslevel-test" 
	       && last_logtype == LintelLog::Debug);
}

void testMisc() {
    vector<string> known_cats;
    known_cats.push_back("known");
    known_cats.push_back("two");
    LintelLog::setKnownCategories(known_cats);

    last_message.clear();
    LintelLog::debugMessagesInitial();
    SINVARIANT(last_message.empty());

    LintelLog::parseDebugString("help");
    LintelLog::debugMessagesInitial();
    INVARIANT(last_message == "known debugging options: complex-bar, complex-foo, ds, dslevel, env, env-foo, help, known, test, test2, test3, two", last_message);

    last_message.clear();
    LintelLog::debugMessagesFinal();
    SINVARIANT(last_message.empty());

    LintelLog::parseDebugString("LintelLog::stats");
    LintelLog::debugMessagesFinal();
    INVARIANT(last_message == "3 calls to slow wouldDebug path", last_message);
}

void testOverflow() {
    LintelLog::addAppender(boost::bind(LintelLog::consoleTimeAppender, 
				       _1, _2));

    LintelLog::reserveCategories(6000);
    
    for(unsigned i = 0; i < 5000; ++i) {
	string cat = (format("overflow%d") % i).str();
	LintelLog::setDebugLevel(cat);
    }
    SINVARIANT(last_message == "close to running out of reserved categories 4800/6000 used; recommend calling LintelLog::reserveCategories(9000) or greater"
	       && last_logtype == LintelLog::Warn);
}

int main() {
    testBasic();
    testComplexMatch();
    testHooked();
    testEnv();
    testDebugString();
    testMisc();
    testOverflow();

    cout << "LintelLog tests successful\n";
    return 0;
}
