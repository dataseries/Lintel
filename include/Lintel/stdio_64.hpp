/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief stdio routines for 64 bit files.
*/

#ifndef LINTEL_STDIO_64_HPP
#define LINTEL_STDIO_64_HPP

// I think that the autoconfing obsoletes this.
// #ifndef _LARGEFILE64_SOURCE
// #error "Must compile with -D_LARGEFILE64_SOURCE to use stdio_64.H"
// #endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>

// The first half of this is just C, the second half is for C++

int fseek64(FILE *stream, off64_t offset, int whence);
off64_t ftell64(FILE *stream);
FILE *fopen64 (const char *pathname, const char *mode);
FILE *freopen64(const char *filename, char *mode, FILE *file);

std::istream *open_ifstream64(const char *filename);

/**
   Open an istream from a UNIX file descriptor. 
   An older C++ had a ifstream constructor that does this, but
   gcc >= 3 doesn't. So here's the one that absorbes this difference.
*/
std::istream *open_fdstream(int fd);

#endif
