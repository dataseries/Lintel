/* -*-C++-*-
/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Constant String implementation
*/

#include <ConstantString.H>
#include <LintelAssert.H>

int ConstantString::nstrings = 0;
int ConstantString::string_bytes = 0;
ConstantString::bufvect *ConstantString::buffers;
ConstantString::CS_hashtable ConstantString::hashtable;

ConstantString::ConstantString(const std::string &str)
{
    init(str.c_str(),str.size());
}

ConstantString::ConstantString(const gcstring &str)
{
    init(str.c_str(),str.size());
}

ConstantString::ConstantString(const char *s)
{
    init(s,strlen(s));
}

static ConstantString *empty_string;

static const bool enable_constant_folding = true;
void
ConstantString::init(const char *s,uint32 slen)
{
    if (buffers == NULL) {
	buffers = new bufvect;
    }
    if (slen == 0 && empty_string != NULL) {
	myptr = empty_string->myptr;
	return;
    }
    if (enable_constant_folding) {
	const char *const *ptr = hashtable.lookup(s);
	if (ptr != NULL) {
	    myptr = (uint32 *)(*ptr);
	    return;
	}
    }
    slen += 1;
    ++nstrings;
    string_bytes += slen;
    int space_used = slen + ((4 - (slen % 4))%4) + sizeof(uint32);
    slen -= 1;

    for(unsigned int i = 0;i<buffers->size();i++) {
	if (((*buffers)[i].size - (*buffers)[i].amt_used) >= space_used) {
	    uint32 *ptr = (uint32 *)((*buffers)[i].data + (*buffers)[i].amt_used);
	    *ptr = slen;
	    ptr += 1;
	    memcpy(ptr,s,slen+1);
	    (*buffers)[i].amt_used += space_used;
	    myptr = ptr;

	    if (slen == 0) {
		empty_string = this;
	    }
	    if (enable_constant_folding) {
		AssertAlways(c_str() == (const char *)myptr,
			     ("?! internal error ?!\n"));
		hashtable.add(c_str());
	    }
	    return;
	}
    }
    AssertAlways(space_used < buffer_size / 64,
		 ("Reduce possible badness by limiting max string size\n"));

    buffer new_buffer;
    new_buffer.data = (char *)GC_malloc_atomic_ignore_off_page(buffer_size);
    new_buffer.size = buffer_size;
    new_buffer.amt_used = 0;
    buffers->push_back(new_buffer);
    init(s,slen); // simpler than re-writing the alloc stuff
}

void
ConstantString::dumpInfo()
{
    int total_data = 0;
    int total_used = 0;
    for(unsigned int i=0;i<buffers->size();i++) {
	total_data += (*buffers)[i].amt_used;
	total_used += (*buffers)[i].size;
    }
    printf("CSInfo: %d strings, bytes: %d string, %d used, %d alloced\n",
	   nstrings, string_bytes, total_data, total_used);
    for(unsigned int i=0;i<buffers->size();i++) {
	printf("CSInfo: buffer %d, %d/%d used\n",i,(*buffers)[i].amt_used,
	       (*buffers)[i].size);
    }
}

