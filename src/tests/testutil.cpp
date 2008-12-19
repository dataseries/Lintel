/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Tests for TestUtil
*/

#include <Lintel/TestUtil.hpp>

int main(int argc, char *argv[]) {
    TEST_INVARIANTMSG(FATAL_ERROR("die now"), "die now");
    TEST_INVARIANTMSG(INVARIANT(false, boost::format("foo %s") % "bar"), "foo bar");
    TEST_INVARIANTMSG(SINVARIANT(false), "No additional details provided");
    TEST_INVARIANTMSG(FATAL_ERROR(boost::format("%1% %d")), 
		      "Exception evaluating boost::format(\"%1% %d\")");
    TEST_INVARIANTMSG(INVARIANT(1 == 0, "expected to fail"), "wrong test msg");
    
    FATAL_ERROR("should not get here");
    return 0;
}
