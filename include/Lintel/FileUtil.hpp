/*
   (c) Copyright 2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    File utility functions
*/

#ifndef LINTEL_FILEUTIL_HPP
#define LINTEL_FILEUTIL_HPP

#include <errno.h>
#include <inttypes.h>

#include <sys/stat.h>

#include <Lintel/AssertBoost.hpp>

namespace lintel {

/** return the modify time in nano-seconds from a statbuf, or just the seconds times 1 billion if
    we don't know how to get the ns time on the current OS (in which case a compile warning will
    have been generated) */
int64_t modifyTimeNanoSec(const struct stat &statbuf) {
#if defined(__linux__)
    return static_cast<int64_t>(statbuf.st_mtime) * 1000 * 1000 * 1000
        + statbuf.st_mtim.tv_nsec;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    return static_cast<int64_t>(statbuf.st_mtime) * 1000 * 1000 * 1000
        + statbuf.st_mtimespec.tv_nsec;
#else
    // don't know how to get ns time on HPUX
#   warning "Don't know how to get ns time on this OS"
    return static_cast<int64_t>(statbuf.st_mtime) * 1000 * 1000 * 1000;
#endif
}

/** return the modify time in nano-seconds from statting a file, or just the seconds times 1
    billion if we don't know how to get the ns time on the current OS (in which case a compile
    warning will have been generated) */
int64_t modifyTimeNanoSec(const std::string &filename) {
    struct stat statbuf;
    CHECKED(stat(filename.c_str(), &statbuf) == 0, 
            boost::format("stat(%s) failed: %s") % filename % strerror(errno));

    return modifyTimeNanoSec(statbuf);
}

} // namespace lintel

#endif
