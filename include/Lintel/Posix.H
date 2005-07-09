/* -*-C++-*-
*******************************************************************************
*
* File:         Posix.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/Posix.H,v 1.2 2005/02/14 04:39:59 anderse Exp $
* Description:  utility wrappers for POSIX functions
* Author:       Alistair Veitch
* Created:      Tue Jul 23 17:06:02 2002
* Modified:     Mon Dec 27 11:06:30 2004 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2002, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef _LINTEL_POSIX_H
#define _LINTEL_POSIX_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/wait.h>
#include "LintelAssert.H"

namespace posix {

    inline void close(int fd) {
	int status = ::close(fd);
	AssertAlways(status == 0, ("%s\n", strerror(errno)));
    };

    inline void dup2(int oldfd, int newfd) {
	int status = ::dup2(oldfd, newfd);
	AssertAlways(status != -1, ("can't dup fd %d: %s\n", newfd, 
				    strerror(errno)));
    };

    inline void pipe(int *filedes) {
	int status = ::pipe(filedes);
	AssertAlways(status != -1, ("%s\n", strerror(errno)));
    };

    inline int open(const char *pathname, int flags) {
	int status = ::open(pathname, flags);
	AssertAlways(status != -1, ("error opening %s: %s\n", pathname, 
				    strerror(errno)));
	return status;
    };

    inline ssize_t read(int fd, void *buf, size_t nbyte) {
	ssize_t status = ::read(fd, buf, nbyte);
	AssertAlways(status != -1, ("read: %s\n", strerror(errno)));
	return status;
    };

    inline void read0(int fd, void *buf, size_t nbyte) {
	AssertAlways(::read(fd, buf, nbyte) == (ssize_t)nbyte, 
		     ("Read0: premature EOF\n"));
    };

    inline ssize_t readN(int fd, void *buf, size_t nbyte) {
	uint8_t *buf2 = (uint8_t *)buf;
	ssize_t left = nbyte;
	do {
	    ssize_t nb = ::read(fd, buf2, nbyte);
	    AssertAlways(nb != -1, ("read: %s\n", strerror(errno)));
	    if (nb == 0) {
		return nbyte - left;
	    }
	    buf2 += nb;
	    left -= nb;
	} while (left != 0);
	return nbyte;
    }

    inline ssize_t write(int fd, const void *buf, size_t nbyte) {
	ssize_t status = ::write(fd, buf, nbyte);
	AssertAlways(status != -1, ("write: %s\n", strerror(errno)));
	return status;
    }

    inline off_t lseek(int fd, off_t offset, int whence) {
	off_t status = ::lseek(fd, offset, whence);
	AssertAlways(status != -1, ("lseek: %s\n", strerror(errno)));
	return status;
    }

    inline pid_t waitpid(pid_t pid, int *stat_loc, int options) {
	pid_t pid2 = ::waitpid(pid, stat_loc, options);
	AssertAlways(pid2 != -1, ("waitpid: %s\n", strerror(errno)));
	return pid2;
    }

    inline pid_t fork() {
	pid_t pid = ::fork();
	AssertAlways(pid != -1, ("fork: %s\n", strerror(errno)));
	return pid;
    }

    inline int kill(pid_t pid, int sig) {
	int status = ::kill(pid, sig);
	AssertAlways(status != -1, ("kill: %s\n", strerror(errno)));
	return status;
    }

}

#endif
