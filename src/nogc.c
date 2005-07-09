#include <stdio.h>
#include <stdlib.h>

void *GC_malloc_atomic_ignore_off_page(unsigned int sz)
{
    return malloc(sz);
}

void *GC_malloc_atomic(unsigned int sz)
{
    return malloc(sz);
}

void *GC_malloc(unsigned int sz)
{
    return malloc(sz);
}

void GC_free(void *p)
{
    free(p);
}
