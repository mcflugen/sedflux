#include "eh_mem.h"
#include "utils.h"
#include <stdio.h>

USE_WIN_ASSERT

#define EH_MEM_TABLE_SIZE (4096)
static glong profile_data_malloc[EH_MEM_TABLE_SIZE];
static glong profile_data_realloc[EH_MEM_TABLE_SIZE];
static glong profile_data_free[EH_MEM_TABLE_SIZE];
static glong  total_alloc   = -1;
static glong  total_realloc = 0;
static glong  total_free    = 0;

typedef enum
{
   EH_MEM_PROFILE_MALLOC ,
   EH_MEM_PROFILE_REALLOC ,
   EH_MEM_PROFILE_FREE
}
Eh_mem_job;

#include <stdlib.h>

void eh_mem_profile_log( Eh_mem_job job , gulong size )
{
   if ( size>0 )
   {
      gulong ind = size-1;

//      if ( profile_data_malloc==NULL )
      if ( total_alloc<0 )
      {
/*
         profile_data_malloc  = (glong*)calloc( EH_MEM_TABLE_SIZE , sizeof(glong) );
         profile_data_realloc = (glong*)calloc( EH_MEM_TABLE_SIZE , sizeof(glong) );
         profile_data_free    = (glong*)calloc( EH_MEM_TABLE_SIZE , sizeof(glong) );
*/
         total_alloc = 0;

         memset( profile_data_malloc  , 0 , EH_MEM_TABLE_SIZE );
         memset( profile_data_realloc , 0 , EH_MEM_TABLE_SIZE );
         memset( profile_data_free    , 0 , EH_MEM_TABLE_SIZE );
      }


      if ( ind>EH_MEM_TABLE_SIZE-1 )
         ind = EH_MEM_TABLE_SIZE-1;

      if      ( job == EH_MEM_PROFILE_MALLOC )
      {
         profile_data_malloc[ind]  += 1;
         total_alloc               += size;
      }
      else if ( job == EH_MEM_PROFILE_REALLOC )
      {
         profile_data_realloc[ind] += 1;
         total_realloc             += size;
      }
      else if ( job == EH_MEM_PROFILE_FREE )
      {
         profile_data_free[ind]    += 1;
         total_free                += size;
      }
   }
}

void eh_mem_profile_fprint( FILE* fp )
{
   glong i;
   glong t_alloc, t_realloc, t_free, t_left;
   glong total;

   fprintf( fp , " Block Size | N Mallocs | N Reallocs | N Frees    | Remaining\n" );
   fprintf( fp , " (in bytes) |           |            |            | (in bytes)\n" );

   for ( i=0 ; i<EH_MEM_TABLE_SIZE ; i++ )
   {
      t_alloc   = profile_data_malloc[i];
      t_realloc = profile_data_realloc[i];
      t_free    = profile_data_free[i];
      t_left    = (i+1)*(t_alloc + t_realloc - t_free);

      if ( t_alloc!=0 || t_realloc!=0 || t_free!=0 )
         fprintf( fp , "%11ld | %9ld | %10ld | %10ld | %10ld\n" ,
                  i+1 , t_alloc , t_realloc , t_free , t_left );

      total_alloc += (i+1)*(t_alloc+t_realloc);
      total_free  += (i+1)*t_free;
   }

   total = total_alloc+total_realloc;

   fprintf( fp , "Bytes allocated    : %12ld\n" , total_alloc );
   fprintf( fp , "Bytes re-allocated : %12ld\n" , total_realloc );
   fprintf( fp , "Bytes freed        : %12ld (%f%%)\n" , total_free , total_free*100./total );
   fprintf( fp , "Bytes in use       : %12ld (%f%%)\n" , total-total_free , (total-total_free)*100./total );
}

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
   Class_Desc class_desc;
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
                              Class_Desc desc ,
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

      /* Update profile data */
      eh_mem_profile_log( EH_MEM_PROFILE_MALLOC , w_size );

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

      mem = eh_malloc( n_blocks*n_block_bytes , NULL , __FILE__ , __LINE__ );

      if ( mem )
         return mem;

      eh_error( "Could not allocate %d bytes." , n_blocks*n_block_bytes );
   }

   return NULL;
}

#if defined( USE_MY_VTABLE )

void* API_ENTRY eh_free_mem( gpointer mem )
{
   if ( !mem )
      return NULL;

   if ( verify_heap_pointer(mem) )
   {
      Heap_Prefix prefix = (Heap_Prefix)mem-1;
      size_t w_size = (size_t)(prefix->postfix) - (size_t)prefix->mem;
      remove_from_linked_list( prefix );
      memset( prefix , 0 , w_size );
      free( prefix );

      eh_mem_profile_log( EH_MEM_PROFILE_FREE , w_size );
   }

   return NULL;
}

void API_ENTRY eh_free_c_style( gpointer mem )
{
   if ( mem )
      eh_free_mem( mem );
}

#endif

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
         size_t old_bytes = (size_t)(prefix->postfix) - (size_t)prefix->mem;

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

         /* Update profile data */
         eh_mem_profile_log( EH_MEM_PROFILE_FREE    , old_bytes );
         eh_mem_profile_log( EH_MEM_PROFILE_REALLOC , w_size    );
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

#if defined( USE_MY_VTABLE )

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

         if ( strlen(buffer)>0 )
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

      eh_mem_profile_fprint( fp );
   }
   else
      fprintf( fp , "Heap is empty\n" );

   fclose( fp );

}
#endif

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

      if ( prefix->class_desc )
      {

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
                     "of type %s" , prefix->class_desc );
         }
         else
            sprintf( buffer+strlen(buffer) , "(unknown type)" );
      }
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



