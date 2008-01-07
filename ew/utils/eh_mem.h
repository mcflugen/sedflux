#ifndef __EH_MEM_H__
#define __EH_MEM_H__

#include <glib.h>
#include <utils/eh_types.h>

#define EH_MEM_LEAK_START { glong __s = eh_mem_in_use(), __e;
#define EH_MEM_LEAK_END_WARN  __e = eh_mem_in_use(); if ( __e!=__s ) { eh_watch_lng( __s ); eh_watch_lng(__e);  } }
#define EH_MEM_LEAK_END_FATAL __e = eh_mem_in_use(); if ( __e!=__s ) { eh_watch_lng( __s ); eh_watch_lng(__e);  eh_exit(0); } }

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

#define USE_MY_VTABLE

gpointer API_ENTRY eh_malloc         ( gsize    w_size   , Class_Desc desc , const char* file    , int line_no );
gpointer API_ENTRY eh_realloc        ( gpointer old      , gsize w_size , const char *file , int line_no );
void     API_ENTRY eh_walk_heap      ( void );
gboolean API_ENTRY eh_is_ptr_ok      ( gpointer mem    );
gpointer API_ENTRY eh_malloc_c_style ( gsize    w_size );
gpointer API_ENTRY eh_calloc_c_style ( gsize    n_blocks , gsize n_block_bytes );
gpointer API_ENTRY eh_realloc_c_style( gpointer mem      , gsize w_size );

#if defined( USE_MY_VTABLE )
glong    API_ENTRY eh_mem_in_use( void );
void*    API_ENTRY eh_free_mem       ( gpointer    mem  );
void     API_ENTRY eh_heap_dump      ( const char* file );
void     API_ENTRY eh_free_c_style   ( gpointer    mem  );

#define eh_new( type , n ) ( (type*)eh_malloc( ((gsize)(sizeof(type)))*((gsize)(n)) , \
                                                G_STRINGIFY(type) , \
                                                __FILE__   , \
                                                __LINE__ ) )
#define eh_new0( type , n ) ( (type*)eh_malloc( ((gsize)(sizeof(type)))*((gsize)(n)) , \
                                                G_STRINGIFY(type) , \
                                                __FILE__ , \
                                                __LINE__ ) )
#define eh_renew( type , old , n ) ( (type*)eh_realloc( (old) , \
                                                        ((gsize)sizeof(type))*((gsize)(n)) , \
                                                        __FILE__ , \
                                                        __LINE__ ) )
#define eh_free( mem ) ( (mem) = eh_free_mem( mem ) )

#else

#define eh_new( type , n )         ( g_new0( type , n ) )
#define eh_new0( type , n )        ( g_new0( type , n ) )
#define eh_renew( type , old , n ) ( g_renew( type , old , n ) )
#define eh_free( mem )             ( g_free( mem ) )
#define eh_free_mem                ( g_free )
#define eh_free_c_style            ( g_free )
#define eh_heap_dump( file )       ( g_mem_profile() )

#endif

void** eh_alloc_2    ( gssize m , gssize n , gssize size );
void   eh_free_void_2( void **p );

#endif

