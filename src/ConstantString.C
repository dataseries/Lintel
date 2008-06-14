/* -*-C++-*-
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Constant String implementation
*/

// TODO: write a regression test; make sure to test strings that are C
// strings and binary data that includes nulls; previous version of
// code collided strings that were identical up to the first null.

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <Lintel/AssertBoost.H>
#include <Lintel/ConstantString.H>

using namespace std;
using boost::format;

int ConstantString::nstrings = 0;
int ConstantString::string_bytes = 0;
ConstantString::bufvect *ConstantString::buffers;
ConstantString::CS_hashtable *ConstantString::hashtable;

static ConstantString *empty_string;
static char *init_buffer;
static uint32_t init_buffer_len;
static const bool enable_constant_folding = true;
void
ConstantString::init(const char *s, uint32_t slen)
{
    if (buffers == NULL) {
	buffers = new bufvect;
	hashtable = new CS_hashtable;
    }
    if (slen == 0 && empty_string != NULL) {
	myptr = empty_string->myptr;
	return;
    }
    if (enable_constant_folding) {
	// TODO: update the hashtable code so that we can specify a
	// key that is not in the stored data format; there was some
	// other case where this would be useful, probably in
	// accessing variable32 data in dataseries and wanting to do a
	// lookup based on that without having to convert to a string
	// first.
	if (init_buffer_len < 5 + slen) {
	    delete [] init_buffer;
	    init_buffer = new char[5 + slen];
	    init_buffer_len = 5 + slen;
	}
	*reinterpret_cast<uint32_t *>(init_buffer) = slen;
	memcpy(init_buffer + 4, s, slen);
	init_buffer[4+slen] = '\0';
	
	ConstantStringValue *tmp = reinterpret_cast<ConstantStringValue *>(init_buffer + 4);
	const ConstantStringValue *const *ptr = hashtable->lookup(tmp);
	if (ptr != NULL) {
	    myptr = *ptr;
	    return;
	}
    }
    slen += 1; // "null at end of string"
    ++nstrings;
    string_bytes += slen;
    int space_used = slen + ((4 - (slen % 4))%4) + sizeof(uint32_t);
    slen -= 1;

    for(unsigned int i = 0;i<buffers->size();i++) {
	if (((*buffers)[i].size - (*buffers)[i].amt_used) >= space_used) {
	    char *ptr = reinterpret_cast<char *>((*buffers)[i].data);
	    ptr += (*buffers)[i].amt_used;
	    *reinterpret_cast<uint32_t *>(ptr) = slen;
	    ptr += 4;
	    memcpy(ptr,s,slen);
	    ptr[slen] = '\0';
	    (*buffers)[i].amt_used += space_used;
	    myptr = reinterpret_cast<ConstantStringValue *>(ptr);

	    if (slen == 0) {
		empty_string = this;
	    }
	    if (enable_constant_folding) {
		SINVARIANT(c_str() == reinterpret_cast<const char *>(myptr));
		hashtable->add(myptr);
	    }
	    return;
	}
    }
    INVARIANT(space_used < buffer_size / 64,
	      "Reduce possible badness by limiting max string size");

    buffer new_buffer;
    new_buffer.data = reinterpret_cast<ConstantStringValue *>(new char[buffer_size]);
    new_buffer.size = buffer_size;
    new_buffer.amt_used = 0;
    buffers->push_back(new_buffer);
    init(s,slen); // simpler than re-writing the alloc stuff
}

void
ConstantString::dumpInfo()
{
    if (buffers == 0) {
	cout << "CSInfo: never used\n";
	return;
    }
    int total_data = 0;
    int total_used = 0;
    for(unsigned int i=0;i<buffers->size();i++) {
	total_data += (*buffers)[i].amt_used;
	total_used += (*buffers)[i].size;
    }
    cout << format("CSInfo: %d strings, bytes: %d string, %d used, %d alloced\n")
	% nstrings % string_bytes % total_data % total_used;
    for(unsigned int i=0;i<buffers->size();i++) {
	cout << format("CSInfo: buffer %d, %d/%d used\n")
	    % i % (*buffers)[i].amt_used % (*buffers)[i].size;
    }
}

