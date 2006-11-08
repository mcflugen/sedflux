#if !defined( EH_MEM_INCLUDED )
#define EH_MEM_INCLUDED

#include <glib.h>
#include "eh_types.h"

#define eh_compiler_require(exp) extern char _compiler_require[(exp)?1:-1]

#define API_ENTRY
#define LOCAL static

#define IS_POWER_2(x) ( !((x)&((x)-1)) )

#define USE_WIN_ASSERT static char source_file[] = __FILE__; \
   int static _do_win_assert( int line_no ) {         \
      report_win_assert( source_file , line_no );     \
      win_assert( line_no );                          \
      return( 0 );                                    \
   }
#define assert_error _do_win_assert( __LINE__ )
#define win_assert( exp ) if ( !(exp) ) { assert_error; } else
void API_ENTRY report_win_assert( char *file_name , int line_no );
/*
typedef struct
{
   char *var_name;
}
*Class_Desc;
*/
gpointer API_ENTRY eh_malloc   ( gsize w_size        , Class_Desc* desc ,
                                 const char* file    , int line_no );
gpointer API_ENTRY eh_realloc  ( gpointer old     , gsize w_size ,
                                 const char *file , int line_no );
void*    API_ENTRY eh_free_mem ( gpointer mem );

void     API_ENTRY eh_walk_heap( void );
void     API_ENTRY eh_heap_dump( const char *file );
gboolean API_ENTRY eh_is_ptr_ok( gpointer mem );

gpointer API_ENTRY eh_malloc_c_style ( gsize w_size );
gpointer API_ENTRY eh_calloc_c_style ( gsize n_blocks , gsize n_block_bytes );
void     API_ENTRY eh_free_c_style   ( gpointer mem );
gpointer API_ENTRY eh_realloc_c_style( gpointer mem , gsize w_size );

#define eh_new( type , n ) ( (type*)eh_malloc( ((gsize)(sizeof(type)))*((gsize)(n)) , NULL , __FILE__ , __LINE__ ) )
#define eh_new0( type , n ) ( (type*)eh_malloc( ((gsize)(sizeof(type)))*((gsize)(n)) , NULL , __FILE__ , __LINE__ ) )
#define eh_renew( type , old , n ) ( (type*)eh_realloc( (old) , ((gsize)sizeof(type))*((gsize)(n)) , __FILE__ , __LINE__ ) )
#define eh_free( mem ) ( (mem) = eh_free_mem( mem ) )

#endif

