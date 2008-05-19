/* -*-C++-*-
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    file decompression 
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <map>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Uncompress.hpp>
#include <Lintel/Posix.hpp>

class child_stuff {
public:
    pid_t pid;	// pid of decompression process
    int err_fd;	// stderr of that process
    child_stuff(pid_t p, int fd) :
	pid(p), 
	err_fd(fd) {
    };
    child_stuff() :	// needed for map
        pid(0),
        err_fd(0) {
    };
};

typedef enum {
    FT_Z,	// "regular" compress (.Z) files
    FT_GZ,	// gzip'd files
    FT_BZ2,	// bzip'd files
    FT_OTHER	// everything else
} FileType;

static std::map<int, child_stuff> open_pipes;

int openCompressed(const char *pathname) {
    // figure out file type
    FileType ftype = FT_OTHER;
    int fd = posix::open(pathname, O_RDONLY);
    uint8_t byte1, byte2, byte3, byte4;
    posix::read0(fd, &byte1, 1);
    posix::read0(fd, &byte2, 1);
    posix::read0(fd, &byte3, 1);
    posix::read0(fd, &byte4, 1);

    if (byte1 == 0x1f && byte2 == 0x9d && byte3 == 0x90 && byte4 == 0x24) {
	ftype = FT_Z;
    }
    else if (byte1 == 0x1f && byte2 == 0x8b && byte3 == 0x08 && (byte4 == 0x08 || byte4 == 0x00)){
	ftype = FT_GZ;
    }
    else if (byte1 == 0x42 && byte2 == 0x5a && byte3 == 0x68 && byte4 == 0x39){
	ftype = FT_BZ2;
    }

    if (ftype == FT_OTHER) {
	// "normal" file: seek to beginning and return already-open descriptor
	posix::lseek(fd, 0, SEEK_SET);
	open_pipes[fd] = child_stuff(0, 0);
	return fd;
    }

    // compressed files: use child process to do decompression
    posix::close(fd);
    int err_pipe[2];	// pipe to child's stderr
    posix::pipe(err_pipe);
    int file_pipe[2];	// pipe for childs stdin
    posix::pipe(file_pipe);

    pid_t pid = posix::fork();
    if (pid == 0) { // child
	posix::close(err_pipe[0]);
	posix::dup2(err_pipe[1], STDERR_FILENO);
	posix::close(err_pipe[1]);

	posix::close(file_pipe[0]);
	posix::dup2(file_pipe[1], STDOUT_FILENO);
	posix::close(file_pipe[1]);

	switch(ftype) {
	case FT_Z:
	    execlp("zcat", "zcat", pathname, NULL);
	    FATAL_ERROR(boost::format("zcat: %s") % strerror(errno));
	    break;
	case FT_GZ:
	    execlp("gunzip", "gunzip", "-c", pathname, NULL);
	    FATAL_ERROR(boost::format("gzcat: %s") % strerror(errno));
	    break;
	case FT_BZ2:
	    execlp("bunzip2", "bunzip2", "-c", pathname, NULL);
	    FATAL_ERROR(boost::format("bzcat: %s") % strerror(errno));
	    break;
	default:
	    FATAL_ERROR("unknown file type");
	    break;
	}
    }

    // parent
    posix::close(file_pipe[1]);
    posix::close(err_pipe[1]);
    
    INVARIANT(open_pipes.find(file_pipe[0]) == open_pipes.end(),
	      "duplicate file descriptor");
    open_pipes[file_pipe[0]] = child_stuff(pid, err_pipe[0]);

    return file_pipe[0];
}

ssize_t readCompressed(int fd, void *buf, size_t nbytes) {
    ssize_t status = posix::readN(fd, buf, nbytes);
    return status;
}

static char *compressChildError(int fd) {
    char *msg = new char[1024];
    memset(msg, 0, 1024);
    read(fd, msg, 1023);
    return msg;
}

void closeCompressed(int fd) {
    std::map<int, child_stuff>::iterator csi = open_pipes.find(fd);
    INVARIANT(csi != open_pipes.end(), 
	      "closeCompressed: unknown descriptor");
    child_stuff cs = csi->second;
    open_pipes.erase(fd);
    posix::close(fd);
    
    if (cs.pid) {
	int status;
	pid_t pid2 = posix::waitpid(cs.pid, &status, WNOHANG | WUNTRACED);
	if (pid2 == 0) {	// child not yet exited
	    posix::kill(cs.pid, SIGKILL);
	    posix::waitpid(cs.pid, NULL, 0);
	}
	else if (WIFEXITED(status)) {	// exited
	    if (WEXITSTATUS(status)) {	// with an error...
		// decompression failed, get error msg, assert out
		FATAL_ERROR(boost::format("decompression failed: %s") 
			    % compressChildError(cs.err_fd));
	    }
	}
	else {	// child exited abnormally
	    FATAL_ERROR(boost::format("abnormal exit: %s") 
			% compressChildError(cs.err_fd));
	}

	posix::close(cs.err_fd);
    }
}

