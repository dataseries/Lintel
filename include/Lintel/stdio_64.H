/*
*******************************************************************************
* 
* File:         stdio_64.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/stdio_64.H,v 1.4 2005/02/14 04:39:59 anderse Exp $
* Description:  stdio routines for 64 bit files.
* Author:       Eric Anderson
* Created:      Wed Jul 19 08:50:55 2000
* Modified:     Fri Jan  7 23:58:40 2005 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
* 
* (C) Copyright 2000, Hewlett-Packard Laboratories, all rights reserved.
*******************************************************************************
*/
#ifndef __STDIO_64_H
#define __STDIO_64_H

// I think that the autoconfing obsoletes this.
// #ifndef _LARGEFILE64_SOURCE
// #error "Must compile with -D_LARGEFILE64_SOURCE to use stdio_64.H"
// #endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "streamcompat.H"
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
