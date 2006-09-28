/* -*-C++-*- */
/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    utility I/O - file decompression
*/


/*
 * generic replacements for open()/close() when all you want to do is
 * read a (possibly compressed) file.
 * 
 * int openCompressed(const char *pathname)
 *	use like open(pathname, O_RDONLY), but will automagically decompress
 *	file as it's being read. return value is descriptor, all error checking
 *	(file exists etc) is done internally.
 * ssize_t readCompressed(int fd, void *buf, size_t nbytes)
 *	use like read(), read data from file opened by openCompressed
 * void closeCompressed(int fd)
 *	use like close(fd), closes file opened by openCompressed()
 */

#ifndef _LINTEL_UNCOMPRESS_H
#define _LINTEL_UNCOMPRESS_H

#include <sys/types.h>

int openCompressed(const char *pathname);
ssize_t readCompressed(int fd, void *buf, size_t nbytes);
void closeCompressed(int fd);

#endif
