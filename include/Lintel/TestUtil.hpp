/* -*-C++-*- */
/*
   (c) Copyright 2008-2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief Some help with testing things that throw invariants
*/

#ifndef LINTEL_TESTUTIL_HPP
#define LINTEL_TESTUTIL_HPP

#include <vector>
#include <string>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StringUtil.hpp>

/// Macro to test whether a particular chunk of code has an invariant
/// with any of a vector of messages; Assumes that you have not set up any
/// AssertBoost functions
#define TEST_INVARIANT_MSGVEC(code, messages) \
    AssertBoostFnBefore(AssertBoostThrowExceptionFn); \
    try { \
        code; \
        FATAL_ERROR("no invariant happened"); \
    } catch (AssertBoostException &e) { \
        AssertBoostClearFns(); \
        bool found = false; \
        for (std::vector<std::string>::iterator i = messages.begin(); \
             i != messages.end(); ++i) { \
             if (e.msg == *i) { found = true; break; } \
        } \
        INVARIANT(found, boost::format("unexpected error message '%s'") % e.msg); \
    }

/// One possibility variant of TEST_INVARIANT_MSGVEC
#define TEST_INVARIANT_MSG1(code, message) { \
    std::vector<std::string> msgs; msgs.push_back(message); \
    TEST_INVARIANT_MSGVEC(code, msgs); \
}

// TODO: remove uses and purge
/// Deprecated, old version of MSG1
#define TEST_INVARIANTMSG(code, message) TEST_INVARIANT_MSG1(code, message)

/// Two possibility variant of TEST_INVARIANT_MSGVEC
#define TEST_INVARIANT_MSG2(code, msg1, msg2) {                    \
    std::vector<std::string> msgs; msgs.push_back(msg1); msgs.push_back(msg2); \
    TEST_INVARIANT_MSGVEC(code, msgs); \
}

/// Three possibility variant of TEST_INVARIANT_MSGVEC
#define TEST_INVARIANT_MSG3(code, msg1, msg2, msg3) {			\
    std::vector<std::string> msgs; msgs.push_back(msg1); msgs.push_back(msg2); msgs.push_back(msg3); \
    TEST_INVARIANT_MSGVEC(code, msgs); \
}

namespace lintel {
    struct DeptoolInfo {
        std::string os; // e.g. debian, ubuntu, centos, fedora, opensuse, scilinux
        std::string version; // e.g. 7.0, 11.10, 5.3, 16, 12.1, 6.2
        std::string arch; // e.g. i386, x86_64
        std::string osVersion() {
            return os + "-" + version;
        }
        std::string osVersionArch() {
            return os + "-" + version + "-" + arch;
        }
        bool haveAllInfo() {
            return !(os.empty() || version.empty() || arch.empty());
        }
    };

    // If $BUILD_OS/$UNAME_M are present in the environment, use those values and convert them
    // into os/version/arch.  Otherwise, run deptool to get those values and return them in the
    // structure.  If deptool is not present, a structure of all empty strings will be returned.
    // This function is useful for whitelisting tests that fail on particular combinations of
    // os/version/arch
    DeptoolInfo getDeptoolInfo();
}

#endif
