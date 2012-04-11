/* -*-C++-*- */
/*
   (c) Copyright 2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/TestUtil.hpp>

int main(int argc, char *argv[]) {
    SINVARIANT(argc == 4);
    lintel::DeptoolInfo info = lintel::getDeptoolInfo();
    SINVARIANT(info.os == argv[1]);
    SINVARIANT(info.version == argv[2]);
    INVARIANT(info.arch == argv[3], boost::format("%s != %s") % info.arch % argv[3]);
    return 0;
}
