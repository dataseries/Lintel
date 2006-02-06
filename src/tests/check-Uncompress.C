/* -*-C++-*-
/*
   (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Regression tests for check-Uncompress.C
*/

#include <Uncompress.H>
#include <LintelAssert.H>
#include <Posix.H>

int main(int argc, char *argv[]) {
    AssertAlways(argc == 2,("invalid args"));

    int fd = openCompressed(argv[1]);
    char buf[80];
    ssize_t nread;
    while ((nread = readCompressed(fd, buf, 80)) != 0) {
	posix::write(STDOUT_FILENO, buf, nread);
    }
    closeCompressed(fd);
    return 0;
}

