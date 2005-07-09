/* -*-C++-*-
*******************************************************************************
*
* File:         check-Uncompress.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/tests/check-Uncompress.C,v 1.1 2005/02/14 04:36:54 anderse Exp $
* Description:  Regression tests for check-Uncompress.C
* Author:       Eric Anderson
* Created:      Sun Dec 26 11:15:45 2004
* Modified:     Mon Dec 27 11:21:00 2004 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2004, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
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

