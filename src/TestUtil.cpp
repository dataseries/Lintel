/* -*-C++-*- */
/*
   (c) Copyright 2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Lintel/TestUtil.hpp>

using namespace std;

namespace {
    string getEnv(const char *name) {
        char *env = getenv(name);
        if (env != NULL) {
            return string(env);
        } else {
            return string();
        }
    }
    
    string extractEnv(const string from) {
        size_t first_quote = from.find('"');
        SINVARIANT(first_quote != string::npos);
        size_t last_quote = from.find('"', first_quote+1);
        SINVARIANT(last_quote != string::npos);
        SINVARIANT(from.size() == last_quote + 3); // ", ;, \n
        string ret = from.substr(first_quote + 1, last_quote - (first_quote+1));
        SINVARIANT(!ret.empty());
        return ret;
    }
}

namespace lintel {

DeptoolInfo getDeptoolInfo() {
    string build_os = getEnv("BUILD_OS");
    string arch = getEnv("UNAME_M");
    if (build_os.empty() || arch.empty()) {
        string deptool_env = getEnv("DEPTOOL");
        string deptool;
        if (deptool_env.empty()) {
            deptool = "deptool";// hope it's in the path.
        } else {
            deptool = string("perl ") + deptool_env; // allow it to be non-executable
        }
        deptool.append(" getenv for-sh");
        FILE *deptool_pipe = popen(deptool.c_str(), "r");
        if (deptool_pipe == NULL) {
            return DeptoolInfo();
        }
        CHECKED(deptool_pipe != NULL, boost::format("Unable to run '%s': %s")
                % deptool % strerror(errno));
        const size_t buf_size = 200;
        char buf[buf_size];
        while (!feof(deptool_pipe) && fgets(buf, buf_size, deptool_pipe) != NULL) {
            string str_buf(buf);
            SINVARIANT(!str_buf.empty() && str_buf[str_buf.size()-1] == '\n');
            if (prefixequal(str_buf, "export BUILD_OS=\"")) {
                build_os = extractEnv(str_buf);
            } else if (prefixequal(str_buf, "export UNAME_M=\"")) {
                arch = extractEnv(str_buf);
            } // else ignore, lots of bits we don't care about.
        }
        SINVARIANT(!build_os.empty() && !arch.empty());
    }

    size_t dash_pos = build_os.find('-');
    SINVARIANT(dash_pos != string::npos);
    DeptoolInfo ret;
    ret.os = build_os.substr(0, dash_pos);
    ret.version = build_os.substr(dash_pos + 1);
    ret.arch = arch;
    return ret;
}

} // namespace lintel

            
        
