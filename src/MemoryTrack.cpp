/* -*-C++-*-
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple replacement for malloc which lets us try to figure out
    where memory is going
*/

#include <malloc.h>
#include <LintelAssert.hpp>

static const int bufsize = 256*1024*1024;
static unsigned char bigbuf[bufsize];
static unsigned char *curptr = bigbuf;
static unsigned char *endptr = bigbuf + bufsize;
static int totalalloc = 0;
static int malloccount = 0;

static const int nbins = 1024;
static const int binsize = 128;
static int bincount[nbins];
static int binalloc[nbins];

void *malloc(size_t size)
{
    ++malloccount;
    AssertAlways(size >= 0,("Bad size\n"));
    void *ret = curptr;
    curptr += size + (8-(size%8))%8;
    AssertAlways(curptr <= endptr,("Out of memory\n"));
    totalalloc += size;
    if (size > (nbins * binsize)) {
	bincount[nbins-1] += 1;
	binalloc[nbins-1] += size;
    } else {
	bincount[size/binsize] += 1;
	binalloc[size/binsize] += size;
    }
    return ret;
}

void free(void *ptr)
{
}

void dumpMemoryTrack()
{
    printf("MT: Overall: %d bytes allocated, %d bytes used, %d allocations\n",
	   totalalloc,curptr - bigbuf,malloccount);
    for(int i=0;i<nbins;i++) {
	if (bincount[i] > 0) {
	    printf("MT:   [%d .. %d]: %d allocations, %d bytes\n",
		   i*binsize,(i+1)*binsize - 1,bincount[i],
		   binalloc[i]);
	}
    }
}   

