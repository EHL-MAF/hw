#include "mm_alloc.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


struct s_block {

struct s_block *next;
struct s_block *prev;
bool free;
size_t size_of_block;
size_t size_of_data;

char data[];

};

typedef struct s_block *s_block_ptr;
s_block_ptr first_block = NULL;
s_block_ptr last_block = NULL;


#define SLACK 4


static void split_block(s_block_ptr b, size_t s);
static s_block_ptr fusion(s_block_ptr b);
static s_block_ptr get_block(void *p);
static s_block_ptr extend_heap( size_t s);


void *
mm_malloc (size_t size) {

 if (size == 0)
  return NULL;
  
  s_block_ptr cur = NULL;
  for( cur = first_block ; cur !=NULL ; cur = cur->next)
  if(cur->free && cur-> size_of_block >= size)
    break;
	fprintf( stderr , "OK\n");
	
	
	if( cur != NULL)
	{
	 split_block( cur , size);
	 cur->free = false;
	 return (void*) cur->data;
	}
	else {
	s_block_ptr new_block = extend_heap( size);
	if( ! new_block)
	return NULL;
	return (void*) new_block->data;
	}
}

void*
mm_realloc ( void* ptr , size_t size)
{
 if( !ptr)
 return mm_malloc(size);
 
 size_of_block block= get_block(ptr);
 void* new_ptr= mm_malloc( size);
 
 if( new_ptr ){
 memcpy( new_ptr , ptr , block-> size_of_data);
 mm_free(ptr);
 return new_ptr;
}
return NULL;
}

void*
mm_free( void* ptr)
{
    if( ptr)
	{
       size_of_block block = get_block( ptr) ;
       block = fusion( block);
       block-> free = true;	   
	}
}

static void
split_block(s_block_ptr b, size_t s) {
   size_t block_size = sizeof( struct s_block);
   if( b->size_of_block >= block_size +s + SLACK)
   {
   s_block_ptr new_block = (s_block_ptr) ( b->data +s);
   new_block->next = b->next;
   new_block->prev = b;
   new_block->free = true;
   new_block->size_of_block = b-> size_of_block -s - block_size;
   
   b->size_of_block = s;
    b->size_of_data = s;
	 b->next= new_block;
   }
}

static s_block_ptr
fusion( s_block_ptr b)
{
 size_t block_size = sizeof( struct s_block);
 s_block_ptr result = b;
 
 if( b-> prev && b->prev-> free)
 { 
  result = b->prev;
  result->size_of_block += block_size + b->size_of_block;
  b->next->prev =  result;
  result->next = b->next;
 }
    if( result->next && result->next->free)
	{
	result->size_of_block += block_size + result->next->size_of_block;
	if( result->next->next)
	   result->next->next->prev = result;
	   result->next = result->next->next;
	}
	return result;
}
static s_block_ptr
extent_heap ( size_t s){

void* cur_brk = sbrk(0);
size_t size= sizeof( struct s_block) +s ;
if( sbrk( size) == ((void*)-1))
 return NULL;
 
 memset ( cur_brk , 0 , size);
 
 s_block_ptr new_block= (s_block_ptr) cur_brk;
 new_block->next = NULL;
   new_block->prev = NULL;
   new_block->free = false;
   new_block->size_of_block = s;
   new_block->size_of_data = s;
   
   if ( !first_block)
   first_block = last_block = new_block;
   else{
      new_block->prev = last_block;
	  last_block->next = new_block;
	  last_block = new_block;
   }
 return new_block;

}






