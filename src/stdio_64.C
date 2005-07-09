/*
*******************************************************************************
* 
* File:         stdio_64.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/stdio_64.C,v 1.6 2005/02/14 23:24:03 ysaito Exp $
* Description:  64 versions of stdio routines.
* Author:       Eric Anderson,SSP 1U-13,857-3990,H=wilkes/S=07-1999/E=02-2000/M=eanders@cs.berkeley.edu/T=(510)642-9669
* Created:      Wed Jul 19 08:51:23 2000
* Modified:     Sun Jan  9 21:26:14 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
* 
* (C) Copyright 2000, Hewlett-Packard Laboratories, all rights reserved.
*******************************************************************************
*/
#include <stdio_64.H>

#ifdef __LP64__
#define O_LARGEFILE 0
#endif

#if defined(__HP_aCC) && _FILE_OFFSET_BITS == 64
#define lseek64 lseek
#endif

int fseek64(FILE *stream, off64_t offset, int whence) 
{
    fflush(stream);
    if (lseek64(fileno(stream),offset,whence) == offset)
	return 0;
    else
	return -1;
}

off64_t ftell64(FILE *stream)
{
  fflush(stream);
  return lseek64(fileno(stream),0,SEEK_CUR);
}

FILE *
fopen64 (const char *pathname, const char *mode)
{
    FILE *ret = NULL;

    int fd = -1;
    if (mode[1] != '\0' && mode[2] != '\0') {
	fprintf(stderr,"Internal error, expected mode[1] or mode[2] to be '\\0'\n");
	exit(253);
    }
    if (mode[1] != 'b' && mode[1] != '\0') {
	fprintf(stderr,"Internal error, expected mode[1] to be 'b' or '\\0'\n");
	exit(253);
    }
    if (mode[0] == 'r') {
	fd = open(pathname,O_RDONLY|O_LARGEFILE);
    } else if (mode[0] == 'w') {
	fd = open(pathname,O_WRONLY|O_LARGEFILE|O_CREAT|O_TRUNC,0666);
    } else {
	fprintf(stderr,"Internal error, expected mode[0] to be 'r' or 'w'\n");
	exit(253);
    }
    if (fd >= 0) {
	ret = fdopen(fd,mode);
    }
    return ret;
}

FILE *
freopen64(const char *filename, char *mode, FILE *file)
{
    FILE *ret = fopen64(filename,mode);
    if (ret == NULL)
	return ret;
    fflush(file);
    (void)(clearerr)(file);
    int dupret = dup2(fileno(ret),fileno(file));
    if (dupret != fileno(file))
	return NULL;
    return file;
}

#if defined(__GNUC__) && __GNUC__ >= 3

// 2004-09-26 can't figure out how to make this work for g++ 3.4 :(
#include <ext/stdio_filebuf.h>

class __Lintel_fdstream: public std::istream {
protected:
    __gnu_cxx::stdio_filebuf<char> *buf;
public:
    __Lintel_fdstream(__gnu_cxx::stdio_filebuf<char> *buf_in)
	: std::istream(buf_in), buf(buf_in) {}
    ~__Lintel_fdstream() {
	delete buf;
    }
};

std::istream *open_fdstream(int fd)
{
#if __GNUC__ == 3 && __GNUC_MINOR__ <= 3
    return new __Lintel_fdstream(new __gnu_cxx::stdio_filebuf<char>(fd, std::ostream::in, true, BUFSIZ));
#else
    FILE *fp = fdopen(fd, "r");
    return new __Lintel_fdstream(new __gnu_cxx::stdio_filebuf<char>(fp, std::ostream::in, BUFSIZ));
#endif
}

#elif HPUX_ACC

std::istream *open_fdstream(int fd)
{
    return new std::ifstream(fd);
}

#endif

std::istream *open_ifstream64(const char *filename)
{
    int fd = open(filename,O_RDONLY|O_LARGEFILE);
    return open_fdstream(fd);
}

