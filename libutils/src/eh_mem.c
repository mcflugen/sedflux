#include "eh_mem.h"
#include <utils.h>
#include <stdio.h>

USE_WIN_ASSERT

//#define ALIGNMENT (sizeof(size_t))
#define ALIGNMENT (8)
#define DO_ALIGN( x ) (((x)+ALIGNMENT-1)&~(ALIGNMENT-1))

eh_compiler_require( IS_POWER_2(ALIGNMENT) );

new_handle( Heap_Prefix );
new_handle( Heap_Postfix );

typedef struct tag_Heap_Prefix
{
   char pad[4];
   Heap_Prefix prev;
   Heap_Prefix next;
   Heap_Postfix postfix;
   char *file_name;
   gint32 line_no;
   void *mem;
   Class_Desc* class_desc;
}
Prefix;

typedef struct tag_Heap_Postfix
{
   Heap_Prefix prefix;
}
Postfix;

eh_compiler_require( !(sizeof(Prefix)%ALIGNMENT) );

static Heap_Prefix heap_head = NULL;

void     LOCAL add_to_linked_list      ( Heap_Prefix );
void     LOCAL remove_from_linked_list ( Heap_Prefix );
gboolean LOCAL verify_heap_pointer     ( void* );
gsize    LOCAL render_desc             ( Heap_Prefix , char* );

#include <stdlib.h>


gpointer API_ENTRY eh_malloc( gsize w_size     ,
                              Class_Desc* desc ,
                              const char* file ,
                              int line_no )
{
   Heap_Prefix prefix;

   eh_require( w_size>=0 );

   if ( w_size <= 0 )
      return NULL;

   w_size = DO_ALIGN( w_size );
   prefix = (Heap_Prefix)malloc( sizeof(Prefix)+w_size+sizeof(Postfix) );

   if ( prefix )
   {
      add_to_linked_list( prefix );

      prefix->postfix         = (Heap_Postfix)( (char*)(prefix+1)+w_size );
      prefix->postfix->prefix = prefix;
      prefix->file_name       = file;
      prefix->line_no         = line_no;
      prefix->mem             = prefix+1;
      prefix->class_desc      = desc;
      memset( prefix->mem , 0 , w_size );
//fprintf( stderr , "Allocated %d bytes at %p (%p)\n" , w_size , prefix , prefix->mem );
   }
   else
   {
      assert_error;
   }

   return ( prefix ? prefix+1 : NULL );
}

gpointer API_ENTRY eh_malloc_c_style( gsize w_size )
{
   if ( w_size )
   {
      gpointer mem;

//fprintf( stderr , "eh_malloc:" );
      mem = eh_malloc( w_size , NULL , __FILE__ , __LINE__ );

      if ( mem )
         return mem;

      eh_error( "Could not allocate %d bytes." , w_size );
   }
   return NULL;
}

gpointer API_ENTRY eh_calloc_c_style( gsize n_blocks , gsize n_block_bytes )
{
   if ( n_blocks*n_block_bytes )
   {
      gpointer mem;

//fprintf( stderr , "eh_calloc:" );
      mem = eh_malloc( n_blocks*n_block_bytes , NULL , __FILE__ , __LINE__ );

      if ( mem )
         return mem;

      eh_error( "Could not allocate %d bytes." , n_blocks*n_block_bytes );
   }

   return NULL;
}

void* API_ENTRY eh_free_mem( gpointer mem )
{
   if ( !mem )
      return NULL;

   if ( verify_heap_pointer(mem) )
   {
      Heap_Prefix prefix = (Heap_Prefix)mem-1;
      size_t w_size = (char*)(prefix->postfix+1) - (char*)prefix;
//fprintf( stderr , "Freed %d bytes at %p (%p)\n" , w_size , prefix , prefix->mem );
      remove_from_linked_list( prefix );
      memset( prefix , 0 , w_size );
      free( prefix );
   }

   return NULL;
}


void API_ENTRY eh_free_c_style( gpointer mem )
{
   if ( mem )
      eh_free_mem( mem );
}

gpointer API_ENTRY eh_realloc( gpointer old     , gsize w_size ,
                               const char *file , int line_no )
{
   void *new = NULL;

   if ( old )
   {
      if ( verify_heap_pointer( old ) )
      {
         Heap_Prefix prefix = (Heap_Prefix)old - 1;
         Heap_Prefix new_prefix;
         Heap_Prefix pre;

         remove_from_linked_list( prefix );
         memset( prefix->postfix , 0 , sizeof( Postfix ) );

         w_size = DO_ALIGN( w_size );
         new_prefix = (Heap_Prefix)realloc( prefix ,
                                              sizeof(Prefix)
                                            + w_size
                                            + sizeof(Postfix) );

         pre = (new_prefix?new_prefix:prefix);
         add_to_linked_list( pre );
         pre->postfix         = (Heap_Postfix)((char*)(pre+1)+w_size);
         pre->postfix->prefix = pre;
         pre->mem             = pre+1;

         new = ( new_prefix?&new_prefix[1]:NULL );
         if ( !new )
         {
            assert_error;
         }
      }
   }
   else
   {
      new = eh_malloc( w_size , NULL , file , line_no );
   }

   return new;
}

gpointer API_ENTRY eh_realloc_c_style( gpointer old , gsize w_size )
{
   if ( w_size )
   {
      gpointer mem;

      mem = eh_realloc( old , w_size , __FILE__ , __LINE__ );

      if ( mem )
         return mem;

      eh_error( "Could not allocate %d bytes." , w_size );
   }

   if ( old )
      eh_free_c_style( old );

   return NULL;
}

void API_ENTRY eh_walk_heap( void )
{
   if ( heap_head )
   {
      Heap_Prefix cur = heap_head;
      while ( verify_heap_pointer( &cur[1] ) )
      {
         char buffer[1000];
         render_desc( cur , buffer );
         printf( "walk: %s\n" , buffer );
         cur = cur->next;
         if ( cur == heap_head )
         {
            break;
         }
      }
   }
}

void API_ENTRY eh_heap_dump( const char *file )
{
   FILE *fp;

   if ( !file )
      fp = stderr;
   else
   {
      fp = fopen( file , "w" );
      if ( !fp )
         fp = stderr;
   }

   fprintf( fp , "Heap dump:\n" );

   if ( heap_head )
   {
      Heap_Prefix cur = heap_head;
      gulong total_bytes = 0;
      while ( verify_heap_pointer( &cur[1] ) )
      {
         char buffer[2048];
         total_bytes += render_desc( cur , buffer );

         fprintf( fp , "%s\n" , buffer );
         cur = cur->next;
         if ( cur == heap_head )
         {
            break;
         }
      }
      fprintf( fp ,
               "Total number of allocated bytes in heap: %ld\n" ,
               total_bytes );
   }
   else
      fprintf( fp , "Heap is empty\n" );

   fclose( fp );

}

void LOCAL add_to_linked_list( Heap_Prefix add )
{
   if ( heap_head )
   {
      add->prev = heap_head->prev;
      (add->prev)->next = add;
      add->next = heap_head;
      (add->next)->prev = add;
   }
   else
   {
      add->prev = add;
      add->next = add;
   }

   heap_head = add;
}

void LOCAL remove_from_linked_list( Heap_Prefix remove )
{
   (remove->prev)->next = remove->next;
   (remove->next)->prev = remove->prev;

   if ( remove == heap_head )
   {
      heap_head = ((remove->next==remove) ? NULL : remove->next );
   }
}

gboolean LOCAL verify_heap_pointer( void *mem )
{
   gboolean ok = FALSE;

   if ( mem )
   {
      eh_require_msg( eh_is_ptr_ok( mem ) , "Pointer is aligned" )
      {
         Heap_Prefix prefix = (Heap_Prefix)mem - 1;
         eh_require_msg( prefix->mem == mem , "No underwrite" )
         {
            eh_require_msg(
               prefix->postfix->prefix == prefix , "No overwrite" )
            {
               ok = TRUE;
            }
         }
      }
   }

   return ok;
}

gboolean API_ENTRY eh_is_ptr_ok( gpointer mem )
{
   return ((mem) && (!((size_t)mem&(ALIGNMENT-1))));
}

gsize LOCAL render_desc( Heap_Prefix prefix , char* buffer )
{
   gulong bytes = 0;

   if ( prefix->mem == &prefix[1] )
   {
      bytes = (size_t)(prefix->postfix) - (size_t)prefix->mem;

      sprintf( buffer , "%08lx " , prefix->mem );
      if ( prefix->file_name )
      {
         sprintf( buffer+strlen(buffer) ,
                  "%s: line %d: %ld bytes allocated " ,
                  prefix->file_name ,
                  prefix->line_no   ,
                  bytes );
      }
      else
         sprintf( buffer+strlen(buffer) ,
                  "%s: line %d: %ld bytes allocated but not yet freed." ,
                  "(unknown file)" ,
                  prefix->line_no   ,
                  bytes );

      if ( prefix->class_desc )
      {
         sprintf( buffer+strlen(buffer) ,
                  "of type %s" , prefix->class_desc->var_name );
      }
      else
         sprintf( buffer+strlen(buffer) , "(unknown type)" );
   }
   else
   {
      strcpy( buffer , "(bad)" );
   }

   return bytes;
}

void API_ENTRY report_win_assert( char *file_name , int line_no )
{
   printf( "win_assert: %s-%d (Press Enter) ", file_name , line_no );
   while ( '\n' != getchar() )
   {
   }
}



