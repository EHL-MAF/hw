#ifndef _malloc_H_
#define _malloc_H_

#include <stdlib.h>

void* mm_malloc ( size_t size);
void* mm_realloc ( void* ptr , size_t size);
void mm_free( void* ptr);

#endif