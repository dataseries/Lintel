/* -*-C++-*- */
/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief utility wrappers for POSIX functions

    This namespace contains wrappers for the POSIX functions to automatiacally check
    the return codes.  This simplifies writing programs because users can assume the
    calls were successful.
*/

#ifndef LINTEL_POSIX_HPP
#define LINTEL_POSIX_HPP

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/wait.h>

#include <Lintel/AssertBoost.hpp>

/// \brief posix namespace
namespace posix {

    inline void close(int fd) {
	int status = ::close(fd);
	INVARIANT(status == 0, boost::format("error on close(): %s") 
		  % strerror(errno));
    };

    inline void dup2(int oldfd, int newfd) {
	int status = ::dup2(oldfd, newfd);
	INVARIANT(status != -1, boost::format("can't dup fd %d: %s")
		  % newfd % strerror(errno));
    };

    inline void pipe(int *filedes) {
	int status = ::pipe(filedes);
	INVARIANT(status != -1, boost::format("error on pipe(): %s")
		  % strerror(errno));
    };

    inline int open(const char *pathname, int flags) {
	int status = ::open(pathname, flags);
	INVARIANT(status != -1, boost::format("error opening %s: %s")
		  % pathname % strerror(errno));
	return status;
    };

    inline ssize_t read(int fd, void *buf, size_t nbyte) {
	ssize_t status = ::read(fd, buf, nbyte);
	INVARIANT(status != -1, boost::format("error on read(%d): %s")
		  % fd % strerror(errno));
	return status;
    };

    inline void read0(int fd, void *buf, size_t nbyte) {
	ssize_t amt = ::read(fd, buf, nbyte);
	INVARIANT(amt == static_cast<ssize_t>(nbyte), 
		  boost::format("Read0(%d, %p, %d): only got %d bytes: %s")
		  % fd % buf % nbyte % strerror(errno));
    };

    inline ssize_t readN(int fd, void *buf, size_t nbyte) {
	uint8_t *buf2 = (uint8_t *)buf;
	ssize_t left = nbyte;
	do {
	    ssize_t nb = ::read(fd, buf2, nbyte);
	    INVARIANT(nb != -1, boost::format("readN(%d): %s") 
		      % fd % strerror(errno));
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
	INVARIANT(status != -1, boost::format("write(%d): %s")
		  % fd % strerror(errno));
	return status;
    }

    inline off_t lseek(int fd, off_t offset, int whence) {
	off_t status = ::lseek(fd, offset, whence);
	INVARIANT(status != -1, boost::format("lseek(%d): %s")
		  % fd % strerror(errno));
	return status;
    }

    inline pid_t waitpid(pid_t pid, int *stat_loc, int options) {
	pid_t pid2 = ::waitpid(pid, stat_loc, options);
	INVARIANT(pid2 != -1, boost::format("waitpid(%d): %s")
		  % pid % strerror(errno));
	return pid2;
    }

    inline pid_t fork() {
	pid_t pid = ::fork();
	INVARIANT(pid != -1, boost::format("fork: %s")
		  % strerror(errno));
	return pid;
    }

    inline int kill(pid_t pid, int sig) {
	int status = ::kill(pid, sig);
	INVARIANT(status != -1, boost::format("kill(%d): %s")
		  % pid % strerror(errno));
	return status;
    }

}

#endif
