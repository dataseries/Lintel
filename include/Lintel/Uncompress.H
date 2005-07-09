/* -*-C++-*-
*******************************************************************************
*
* File:         Uncompress.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/Uncompress.H,v 1.2 2005/02/14 04:39:59 anderse Exp $
* Description:  utility I/O - file decompression
* Author:       Alistair Veitch
* Created:      Tue Jul 23 11:01:51 2002
* Modified:     Mon Dec 27 11:04:07 2004 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2002, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
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
