//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#ifndef UTILS_INCLUDED
# define UTILS_INCLUDED

#undef OLD_NDGRID

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "eh_mem.h"
#include "eh_rand.h"
#include "eh_logging.h"
#include "eh_scanner.h"
#include "eh_get_opt.h"
#include "eh_glib.h"
#include "complex.h"
#include "eh_num.h"
#include "eh_opt_context.h"
#include "eh_project.h"
#include "eh_grid.h"
#include "eh_ndgrid.h"
#include "eh_key_file.h"
#include "eh_symbol_table.h"
#include "eh_data_record.h"
#include "eh_input_val.h"
#include "eh_types.h"
#include "eh_dlm_file.h"

#define E_BADVAL (G_MAXFLOAT)
#define E_NOVAL  (G_MAXFLOAT)

#define S_LINEMAX     (2048)
#define S_NAMEMAX     (255)
#define S_MAXPATHNAME (1024)

#define E_MAJOR_VERSION (1)
#define E_MINOR_VERSION (1)
#define E_MICRO_VERSION (0)

#define E_CHECK_VERSION(major,minor,micro)    \
    (E_MAJOR_VERSION > (major) || \
     (E_MAJOR_VERSION == (major) && E_MINOR_VERSION > (minor)) || \
     (E_MAJOR_VERSION == (major) && E_MINOR_VERSION == (minor) && \
      E_MICRO_VERSION >= (micro)))

#if defined( DISABLE_WATCH_POINTS )

#define eh_watch_int( val ) {}
#define eh_watch_lng( val ) {}
#define eh_watch_ptr( val ) {}
#define eh_watch_dbl( val ) {}
#define eh_watch_exp( val ) {}
#define eh_watch_str( val ) {}
#define eh_watch_chr( val ) {}
#define eh_watch_dbl_vec( val , l , h ) {}

#else

#define eh_watch_int( val )  G_STMT_START { \
   fprintf(stderr,                          \
           "%s = %d (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_lng( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %ld (%s, line %d)\n",      \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);          } G_STMT_END

#define eh_watch_ptr( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %p (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);          } G_STMT_END

#define eh_watch_dbl( val ) G_STMT_START {  \
   if ( fabs(val) < 1e3 && fabs(val)>1e-3 ) \
      fprintf(stderr,                       \
              "%s = %f (%s, line %d)\n",    \
              #val,                         \
              val,                          \
              g_basename(__FILE__),         \
              __LINE__),                    \
      fflush(stderr);                       \
   else                                     \
      eh_watch_exp( val );       } G_STMT_END

#define eh_watch_exp( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %g (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_str( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %s (%s, line %d)\n",       \
           #val,                            \
           (val)?(val):"(null)" ,           \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_chr( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %f (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_dbl_vec( val , l , h ) G_STMT_START { \
   int i;                                              \
   for ( i=l ; i<=h ; i++ ) {                          \
      fprintf(stderr,                                  \
              "%s[%d] = %f (%s, line %d)\n",           \
              #val,                                    \
              i,                                       \
              val[i],                                  \
              g_basename(__FILE__),                    \
              __LINE__),                               \
      fflush(stderr);       }               } G_STMT_END

#endif

#if defined( DISABLE_CHECKS )

#define eh_require( expr ) { }
#define eh_require_msg( expr ) { }
#define eh_require_critical( expr ) { }
#define eh_require_msg_critical( expr ) { }
#define eh_require_not_reached( ) { }
#define eh_make_note( expr ) { (expr); }
#define eh_return_if_fail( expr ) { }
#define eh_return_val_if_fail( expr , val ) { }

#else

#define eh_require( expr )                                            \
   if ( !(expr) ) {                                                   \
      fprintf( stderr ,                                               \
               "%s : line %d : requirement failed : (%s)\n" ,         \
               g_basename(__FILE__),                                  \
               __LINE__,                                              \
               #expr ),                                               \
      fflush( stderr ); } else

#define eh_require_msg( expr , msg )                                  \
   if ( !(expr) ) {                                                   \
      fprintf( stderr ,                                               \
               "%s : line %d : requirement (%s) failed : (%s)\n" ,    \
               g_basename(__FILE__),                                  \
               __LINE__,                                              \
               (msg)?(msg):"NULL" ,                                   \
               #expr ),                                               \
      fflush( stderr ); } else
/*
#define eh_require( expr ) {                                          \
   if ( expr ) { } else {                                             \
      fprintf( stderr ,                                               \
               "%s : line %d : check failed : (%s)\n" ,               \
               g_basename(__FILE__),                                  \
               __LINE__,                                              \
               #expr ),                                               \
      fflush( stderr ); }  }
*/

#define eh_require_msg_critical( expr , msg )                         \
   if ( !(expr) ) { eh_require_msg( expr , msg ); eh_exit(-1); } else

#define eh_require_critical( expr )                                   \
   if ( !(expr) ) { eh_require( expr ); eh_exit(-1); } else

#define eh_require_not_reached( ) {                                   \
   fprintf( stderr ,                                                  \
            "%s : line %d : should not be reached\n" ,                \
            g_basename(__FILE__) ,                                    \
            __LINE__ ),                                               \
   fflush( stderr ); }

#define eh_make_note( expr ) {                                        \
   (expr),                                                            \
   fprintf( stderr ,                                                  \
            "%s : line %d : %s\n" ,                                   \
            g_basename(__FILE__) ,                                    \
            __LINE__ ,                                                \
            #expr ),                                                  \
   fflush( stderr ); }

#define eh_note_block( msg , expr )                                   \
   if ( (expr) )                                                      \
   {                                                                  \
      gchar* base_name = g_path_get_basename(__FILE__);               \
      eh_debug(                                                       \
               "%s : line %d : %s\n" ,                                \
               base_name ,                                            \
               __LINE__,                                              \
               (msg)?(msg):"NULL" ) ,                                 \
      eh_free( base_name );                                           \
      fflush( stderr );                                               \
   }                                                                  \
   if ( !(expr) ) {} else

#define eh_return_val_if_fail( expr , val ) if ( !(expr) ) { return (val); } else
#define eh_return_if_fail( expr ) if ( !(expr) ) { return; } else

#endif

#define eh_p_msg( msg_level , str ) \
   eh_print_msg( msg_level , __PRETTY_FUNCTION__ , str )

#define eh_lower_bound( val , low  ) ( val = ((val)<(low) )?(low ):(val) )
#define eh_upper_bound( val , high ) ( val = ((val)>(high))?(high):(val) )
#define eh_clamp( val , low , high ) \
   ( val = ((val)<(low))?(low):(((val)>(high))?(high):(val)) )
#define EH_SWAP_PTR( a , b ) { void* t=(b); (b)=(a); (a)=t; }
#define swap_int( a , b ) { int temp=b; b=a; a=temp; }
#define swap_dbl( a , b ) { double temp=b; b=a; a=temp; }
#define swap_dbl_vec( x , i , j ) { double temp=x[i]; x[i]=x[j]; x[j]=temp; }
#define eh_memswap( a , b , n ) { void* temp=g_memdup(a,n); g_memmove(a,b,n); g_memmove(b,temp,n); eh_free(temp); }
#define EH_STRUCT_MEMBER( type , st , mem ) ( ((type*)st)->mem )

#define eh_new_2( type , m , n ) ( (type**)eh_alloc_2( m , n , sizeof(type) ) )
#define eh_free_2( p ) ( eh_free_void_2( ((void**)(p)) ) )

#define EH_RADS_PER_DEGREE ( 0.01745329251994 )
#define EH_DEGREES_PER_RAD ( 57.29577951308232 )

#define EH_SQRT_PI ( 1.77245385090552 )

#define POLYGON_IN_CROSSINGS  ( 1<<0 )
#define POLYGON_OUT_CROSSINGS ( 1<<1 )

#if defined( OLD_NDGRID )
typedef gboolean (*Populate_func)( double , double , gpointer );
#endif
typedef gboolean (*Eh_test_func)( void );

/** A series of (x,y) pairs

\deprecated Don't use this anymore.  Try Eh_dbl_grid instead.
*/
typedef struct
{
   double *x;
   double *y;
   int size;
} pos_t;

#if defined( OLD_NDGRID )
/** A point defined by its indices, (i,j)
*/
typedef struct
{
/// i-index
   int i;
/// j-index
   int j;
}
Eh_ind_2;
#endif

/** A point defined by an x-y pair.
*/
typedef struct
{
/// The x-coordinate
   double x;
/// The y-coordinate
   double y;
}
Eh_pt_2;

typedef GList* Eh_polygon_2;

/*
typedef struct
{
   double *v;
   int size;
}
dvector;
*/

/** A sequence of data
*/
typedef struct
{
/// The 'time' value for each member of the sequence
   double *t;
/// A pointer to each member of the sequence
   gpointer *data;
/// The number of members in the sequence
   gssize len;
}
Eh_sequence;

#if defined( OLD_NDGRID )

typedef struct
{
   void **data;
   double *x;
   double *y;
   gssize low_x;
   gssize low_y;
   gssize n_x;
   gssize n_y;
   gssize el_size;
}
Eh_grid;

typedef struct
{
   double *data;
   gssize n_dim;
   gssize *size;
   gssize *low;
   double **x;
   gssize el_size;
}
Eh_ndgrid;

#endif

#if defined( OLD_NDGRID )
typedef gssize Eh_grid_id;
typedef struct
{
   double **data;
   double *x;
   double *y;
   gssize low_x;
   gssize low_y;
   gssize n_x;
   gssize n_y;
   gssize el_size;
}
Eh_dbl_grid;

typedef struct
{
   int **data;
   double *x;
   double *y;
   gssize low_x;
   gssize low_y;
   gssize n_x;
   gssize n_y;
   gssize el_size;
}
Eh_int_grid;
#endif

typedef struct
{
   double day, month, year;
}
Eh_date_t;

#define BIT_ON	00000001
#define BIT_OFF	00000000
#define NO  0
#define YES 1
#define PTR_SIZE 8	/* length of a pointer in bytes */
#define ALL -1

/**********
*
* **** Macro descriptions
*
*  max(a,b)
*    evaluates to the maximum of the numbers a and b.
*
*  min(a,b)
*    evaluates to the minimum of the numbers a and b.
*
*  get_bit(pt,n)
*    returns the value of the nth bit in the data pointed to by pt.  To work
*    properly pt must be converted to char *.
*
*  put_but(pt,n,bit)
*    Sets the value of the nth bit in the data pointed to by pt to value bit.
*    To work properly pt must be converted to char *. 
*
*  nbb(n)
*    evaluates to the minimum number of bytes able to contain n bits
*
**********/

#define eh_sign(a)         ( (a>=0)?1:-1 )
#define eh_max(a,b)    	   ( ( (a) > (b) ) ? (a) : (b) )
#define eh_set_max(a,b)    if ( (b)>(a) ) { (a) = (b); }
#define eh_set_min(a,b)    if ( (b)<(a) ) { (a) = (b); }
#define eh_dbl_set_max(a,b) { double __t=(b); if ( __t>(a) ) { (a)=__t; } }
#define eh_dbl_set_min(a,b) { double __t=(b); if ( __t<(a) ) { (a)=__t; } }
#define eh_min(a,b)    	   ( ( (a) < (b) ) ? (a) : (b) )
#define get_bit(pt,n)	   ( (pt)[(n)/8]&(0x80>>((n)%8)) )
#define turn_bit_on(pt,n)  (pt)[(n)/8] = (pt)[(n)/8] | (0x80>>((n)%8))
#define turn_bit_off(pt,n) (pt)[(n)/8] = (pt)[(n)/8] &~ (0x80>>((n)%8));
#define nbb(n)             ( ( (n) + 7 ) / 8 )
#define eh_sqr( x )        ( (x)*(x) )
#define eh_nrsign( a , b )   ( (b>0)?fabs(a):(-fabs(a)) )


/**********
*
* **** Macro descriptions
*
*  initarray(pt,len,c)
*    Initializes the array pointed to by pt of length len to value c.
*
*  count_pix(pt,len,val,c)
*    Counts the number of elements of value, val in the data pointed to by pt
*    of length len.  The count is stored in c.
*
*  rotate_array(ar_type,pt,len)
*    Rotates the values in the array pointed to by pt (of length, len and type,
*    ar_type).  The values are rotated such that pt[i]=pt[i+1].  Also, pt[0] is
*    stored in a temporary variable and copied to pt[len-1].
*
*  mmove(dst,src,len)
*    Copies the len characters pointed to by src into the object pointed to by
*    dst.  The characters are not copied to a temporary location before being 
*    copied.  Thus, for the data to be copied correctly, the source and
*    destination objects must not overlap.
*
**********/

#ifndef initarray
# define initarray(pt,len,c) { unsigned long i; for (i=0;i<len;(pt)[i++]=c); }
#endif
#define count_pix(pt,len,val,c) { unsigned long i; for (i=0;i<len;i++) if ((pt)[i]==val) c++; }
#define rotate_array(ar_type,pt,len) { unsigned long i;				      \
				       ar_type temp;					      \
				       for (i=0,temp=(pt)[0];i<len-1;(pt)[i]=(pt)[i+1],i++); \
				       (pt)[len-1]=temp;				      \
				     }
#define mmove(dst,src,len) { unsigned long j; 		        \
			     for (j=0;j<(len);j++) 		\
			              (dst)[j]=(src)[j];	\
			   }


/* Function declarations -- Functions defined in utils.c */

void eh_init_glib( void );
void eh_exit( int code );
void eh_exit_on_error( GError* error , const gchar* format , ... );

gint eh_fprint_version_info( FILE* fp , const gchar* prog , gint maj , gint min , gint micro );

gboolean is_in_domain( gssize n_i , gssize n_j , gssize i , gssize j );
#if defined( OLD_NDGRID )
Eh_grid_id eh_grid_sub_to_id( gssize n_i , gssize i , gssize j );
Eh_ind_2 eh_grid_id_to_sub( gssize n_i , Eh_grid_id id );
#endif

double weighted_avg(const double x[],const double f[],long len);
#ifndef log2
double log2(double);
#endif

#undef OLD_EH_NAN
#if defined( OLD_EH_NAN )

float     eh_nan    ( void   );
int       eh_isnan  ( float  );

#else

double    eh_nan    ( void   );
gboolean  eh_isnan  ( double );

#endif

//dpair_vector& interpolate(dpair_vector&,dpair_vector&);
void eh_rebin_dbl_array( double *x , double *y , gssize len ,
                         double *x_new , double *y_new , gssize len_new );
void eh_rebin_dbl_array_bad_val(
        double *x     , double *y     , gssize len     ,
        double *x_bin , double *y_bin , gssize len_bin ,
        double bad_val );
#if defined( OLD_NDGRID )
void eh_rebin_dbl_grid( Eh_dbl_grid *source , Eh_dbl_grid *dest );
void eh_rebin_dbl_grid_bad_val( Eh_dbl_grid *source , Eh_dbl_grid *dest ,
                                double bad_val );
#endif
#if defined(OLD_NDGRID)
void interpolate_2( Eh_dbl_grid *source , Eh_dbl_grid *dest );
void interpolate_2_bad_val( Eh_dbl_grid *source ,
                            Eh_dbl_grid *dest   ,
                            double bad_val );
#else
void interpolate_2( Eh_dbl_grid source , Eh_dbl_grid dest );
void interpolate_2_bad_val( Eh_dbl_grid source ,
                            Eh_dbl_grid dest   ,
                            double bad_val );
#endif
void interp(float[],float[],int,float[],double[],int);
void finterp(float[],float[],int,float[],float[],int);
void dinterp(double[],double[],int,double[],double[],int);

int listLength(FILE*);
int read_double_vector(FILE*,double*,int);
int read_time_vector( FILE *fp , double *val , int len );
int read_int_vector(FILE*,int*,int);
double read_double_value(FILE*,double*);
int read_int_value(FILE*,int*);
gboolean read_logical_value(FILE *fp);
gboolean strtobool(const char *);
char *read_string_value(FILE*,char*);
char **read_string_vector(FILE*,int);
int no_data_line(char *line);
char *trimleft(char*);
char *trimright(char*);
char *remove_white_space(char *str);
//string& remove_white_space(string& str);

FILE *openFileCat(const char*,const char *,const char *);
FILE *eh_fopen(const char *,const char *);
FILE *eh_open_file( const char * , const char * );
FILE *eh_open_temp_file( const char *template , char **name_used );
FILE *eh_fopen_error(const char *,const char *,GError**);
gchar* eh_render_file_error_str( gint err_no );
gchar* eh_render_error_str( GError* error , const gchar* err_str );
gboolean eh_is_readable_file( const char* );
gboolean eh_is_writable_file( const char* );
gboolean eh_try_open( const char* file );
gboolean eh_open_dir( const char* dir );
gboolean try_open(const char *,int);
gboolean try_dir(const char *);

void **eh_alloc_2( gssize m , gssize n , gssize size );
void eh_free_void_2( void **p );
//void *malloc1D(int);
//double **malloc2Ddouble(int,int);
//void free2D(double**);

double **read_matrix(FILE*,int,int*);

double convert_time_to_years(Eh_date_t *time);
double scan_time_in_years(FILE *fp);
double sscan_time_in_years(const char *s);
double strtotime(const char *);

pos_t *createPosVec(int);
void destroyPosVec(pos_t*);
double *derivative(pos_t);

#if defined( OLD_NDGRID)
Eh_ind_2 eh_create_ind_2( int i , int j );
#endif
Eh_pt_2 eh_create_pt_2( double x , double y );
gboolean eh_cmp_pt_2( Eh_pt_2 a , Eh_pt_2 b , double eps );
#if defined( OLD_NDGRID )
gboolean eh_cmp_ind_2( Eh_ind_2 a , Eh_ind_2 b );
Eh_ind_2 *eh_dup_ind_2( Eh_ind_2 *src , Eh_ind_2 *dest );
#endif

/*
dvector *vec_create_dvector(int n);
void vec_destroy_dvector(dvector *dvec);
int vec_get_dvector_size(dvector *dvec);
double vec_get_dvector_val(dvector *dvec,int index);
dvector *vec_set_dvector_val(dvector *dvec,int index,double val);
dvector *vec_resize_dvector(dvector *dvec,int n);
dvector *vec_push_dvector_val(dvector *dvec,double val);
double vec_pop_dvector_val(dvector *dvec);
int vec_write_dvector(dvector *dvec,FILE *fp);
int vec_read_dvector(dvector *dvec,FILE *fp);
*/

typedef struct tpool_work
{
   void (*routine)();
   void *arg;
   struct tpool_work *next;
} tpool_work_t;

typedef struct tpool
{
   int num_threads;
   int max_queue_size;

   int do_not_block_when_full;

   pthread_t *threads;
   int cur_queue_size;
   tpool_work_t *queue_head;
   tpool_work_t *queue_tail;
   pthread_mutex_t queue_lock;
   pthread_cond_t queue_not_empty;
   pthread_cond_t queue_not_full;
   pthread_cond_t queue_empty;
   int queue_closed;
   int shutdown;
} *tpool_t;

void tpool_init( tpool_t* , int , int , int );
int tpool_add_work( tpool_t , void* , void* );
int tpool_destroy( tpool_t , int );

typedef struct
{
   char *prefix;
   char *suffix;
   char *format;
   int count;
}
Eh_file_list;

Eh_file_list *eh_create_file_list( char *base_name );
char *eh_get_next_file( Eh_file_list *list );
void eh_destroy_file_list( Eh_file_list *list );

double    eh_get_fuzzy_dbl      ( double min  , double max );
double    eh_get_fuzzy_dbl_norm ( double mean , double std );
gint32    eh_get_fuzzy_int      ( gint32 min  , gint32 max );

double*   eh_linspace( double min , double max , gssize n );
gssize*   eh_id_array( gssize i_0 , gssize i_1 , gssize* n );
double*   eh_uniform_array( double x1 , double x2 , double dx , gssize* n );
double*   eh_dbl_array_linspace( double* x , gssize n_x , double x_0 , double dx );
gboolean  eh_dbl_array_is_monotonic( double* x , gssize len );
double*   eh_logspace( double d1  , double d2  , int n );

void eh_print_msg( int , char* , char* );

#if defined( OLD_NDGRID )

Eh_grid*     eh_malloc_grid         ( gssize n_x     , gssize n_y ,
                                      gssize size );
void         eh_free_grid_data      ( Eh_grid *g     , gboolean free_data );
Eh_grid*     eh_destroy_grid        ( Eh_grid *g     , gboolean free_data );
void         eh_dump_grid           ( FILE *fp       , Eh_grid *g );
Eh_grid*     eh_load_grid           ( FILE *fp );
gboolean     eh_cmp_grid            ( Eh_grid *g_1   , Eh_grid *g_2 );
gboolean     eh_cmp_grid_data       ( Eh_grid *g_1   , Eh_grid *g_2 );
gboolean     eh_cmp_grid_x_data     ( Eh_grid *g_1   , Eh_grid *g_2 );
gboolean     eh_cmp_grid_y_data     ( Eh_grid *g_1   , Eh_grid *g_2 );
Eh_grid*     eh_dup_grid            ( Eh_grid *g );
Eh_grid*     eh_copy_grid           ( Eh_grid *dest  , Eh_grid *src );
Eh_grid*     eh_copy_grid_data      ( Eh_grid *dest  , Eh_grid *src );
Eh_grid*     eh_reindex_grid        ( Eh_grid *g ,
                                      gssize low_x   , gssize low_y );
gboolean     is_in_grid_domain      ( Eh_grid *g ,
                                      gssize i , gssize j );
gboolean     eh_is_grid_same_size   ( Eh_grid *g_1   , Eh_grid *g_2 );
Eh_dbl_grid* eh_create_dbl_grid     ( gssize n_x     , gssize n_y );
gboolean     eh_cmp_dbl_grid        ( Eh_dbl_grid *g_1 , Eh_dbl_grid *g_2 ,
                                      double eps );
Eh_dbl_grid* eh_dup_dbl_grid        ( Eh_dbl_grid *g );
Eh_dbl_grid* eh_copy_dbl_grid       ( Eh_dbl_grid *dest , Eh_dbl_grid *src );
Eh_int_grid* eh_create_int_grid     ( gssize n_x     , gssize n_y );
int          eh_write_ndgrid        ( FILE *fp       , Eh_ndgrid *g );
int          eh_write_dbl_grid      ( FILE *fp       , Eh_dbl_grid *g );
double**     eh_dbl_grid_data       ( Eh_dbl_grid *g );
gboolean     is_compatible_grid     ( Eh_dbl_grid *g_1 ,
                                      Eh_dbl_grid *g_2 );
Eh_dbl_grid* eh_add_dbl_grid        ( Eh_dbl_grid* g_1 ,
                                      Eh_dbl_grid* g_2 );
double       eh_sum_dbl_grid        ( Eh_dbl_grid *g );
double       eh_sum_dbl_grid_bad_val( Eh_dbl_grid *g , double bad_val );
Eh_dbl_grid* eh_set_dbl_grid        ( Eh_dbl_grid *g , double val );
Eh_dbl_grid* eh_randomize_dbl_grid  ( Eh_dbl_grid *g );
void         eh_scalar_mult_dbl_grid( Eh_dbl_grid *g , double scalar );
void         eh_rotate_dbl_grid     ( Eh_dbl_grid *g , double angle ,
                                      gssize i_0     , gssize j_0 );
Eh_dbl_grid* eh_reduce_dbl_grid     ( Eh_dbl_grid *g ,
                                      gssize new_n_x , gssize new_n_y );
Eh_dbl_grid* eh_expand_dbl_grid     ( Eh_dbl_grid *g ,
                                      gssize new_n_x , gssize new_n_y );
Eh_dbl_grid* eh_remesh_dbl_grid     ( Eh_dbl_grid *g ,
                                      gssize new_n_x , gssize new_n_y );
Eh_grid*     sed_subgrid            ( Eh_grid *g     ,
                                      int i_0 , int j_0 ,
                                      int n_x , int n_y );
#endif

double sigma( double s , double t , double p );

#define eh_create_grid( type , n_x , n_y ) \
           ( eh_malloc_grid(n_x,n_y,sizeof(type) ) )
#define eh_index_grid( type , g , i , j ) ( ((type*)((Eh_grid*)(g))->data[i])[j] )
#if defined(OLD_NDGRID)
Eh_dbl_grid *eh_populate_domain( Eh_dbl_grid *g  ,
                                 Populate_func f ,
                                 gpointer user_data );
#endif

Eh_polygon_2 eh_get_rectangle_polygon( Eh_pt_2 center , double dx , double dy );
void eh_destroy_polygon( Eh_polygon_2 p );
GList *eh_find_polygon_crossings( Eh_pt_2 start ,
                                   double angle  ,
                                   Eh_polygon_2 area ,
                                   int in_or_out );
gboolean is_between_angles( double angle , double angle_1 , double angle_2 );
gboolean is_inside_area( Eh_pt_2 x , Eh_polygon_2 area );
Eh_pt_2 eh_get_unit_vector( double angle );
Eh_pt_2 eh_get_dir_vector( Eh_pt_2 point_1 , Eh_pt_2 point_2 );
Eh_pt_2 eh_get_norm_vector( Eh_pt_2 u );
double eh_get_vector_length( Eh_pt_2 u );
double eh_get_vector_angle( Eh_pt_2 u );
Eh_pt_2 eh_normalize_vector( Eh_pt_2 u );
double eh_dot_vectors( Eh_pt_2 u , Eh_pt_2 v );
double eh_get_angle_between_vectors( Eh_pt_2 u , Eh_pt_2 v );

double eh_erf_inv( double y );

void eh_test_function( const char *f_name , Eh_test_func f );

Eh_sequence *eh_create_sequence( void );
Eh_sequence *eh_add_to_sequence( Eh_sequence *s , double t , gpointer data );
void eh_destroy_sequence( Eh_sequence *s , gboolean free_mem );

#if defined( OLD_NDGRID )
Eh_ndgrid *eh_malloc_ndgrid( gssize n_dim , gssize el_size , ... );
void eh_free_ndgrid_data( Eh_ndgrid *g );
double eh_ndgrid_ind( Eh_ndgrid *g , ... );
Eh_ndgrid *eh_reshape_ndgrid( Eh_ndgrid *g , gssize *new_size , gssize new_n_dim );
gssize eh_ndgrid_sub_to_id( gssize *size , gssize *sub , gssize n_dim  );
gssize *eh_ndgrid_id_to_sub( gssize *size , gssize id , gssize n_dim );
void eh_destroy_ndgrid( Eh_ndgrid *g );
Eh_dbl_grid *eh_ndgrid_to_grid( Eh_ndgrid *g );
Eh_ndgrid *eh_grid_to_ndgrid( Eh_grid *g );
#endif

int eh_write_dbl_array( FILE *fp , double *x , gssize len );

char*    eh_input_str    ( char *msg , char*    default_str );
gboolean eh_input_boolean( char *msg , gboolean default_val );

gssize       eh_pointer_list_length ( gpointer* x );

#if G_BYTE_ORDER==G_LITTLE_ENDIAN

gssize eh_fwrite_int32_to_be ( const void *ptr , gssize size , gssize nitems , FILE* stream  );
gssize eh_fwrite_int64_to_be ( const void *ptr , gssize size , gssize nitems , FILE* stream  );
gssize eh_fread_int32_from_be( const void *ptr , gssize size , gssize nitems , FILE* stream  );
gssize eh_fread_int64_from_be( const void *ptr , gssize size , gssize nitems , FILE* stream  );

#define eh_fwrite_dbl_to_be    ( eh_fwrite_int64_to_be  )
#define eh_fwrite_flt_to_be    ( eh_fwrite_int32_to_be  )
#define eh_fread_dbl_from_be   ( eh_fread_int64_from_be )
#define eh_fread_flt_from_be   ( eh_fread_int32_from_be )

#define eh_fwrite_int32_swap   ( eh_fwrite_int32_to_be  )
#define eh_fwrite_int64_swap   ( eh_fwrite_int64_to_be  )
#define eh_fread_int32_swap    ( eh_fread_int32_from_be )
#define eh_fread_int64_swap    ( eh_fread_int64_from_be )

#define eh_fread_flt_swap      ( eh_fread_int32_from_be )
#define eh_fread_dbl_swap      ( eh_fread_int64_from_be )
#define eh_fwrite_flt_swap     ( eh_fwrite_int32_to_be )
#define eh_fwrite_dbl_swap     ( eh_fwrite_int64_to_be )

#else

gssize eh_fwrite_int32_to_le ( const void *ptr , gssize size , gssize nitems , FILE* stream  );
gssize eh_fwrite_int64_to_le ( const void *ptr , gssize size , gssize nitems , FILE* stream  );
gssize eh_fread_int32_from_le( const void *ptr , gssize size , gssize nitems , FILE* stream  );
gssize eh_fread_int64_from_le( const void *ptr , gssize size , gssize nitems , FILE* stream  );

#define eh_fwrite_dbl_to_le    ( eh_fwrite_int64_to_le  )
#define eh_fwrite_flt_to_le    ( eh_fwrite_int32_to_le  )
#define eh_fread_dbl_from_le   ( eh_fread_int64_from_le )
#define eh_fread_flt_from_le   ( eh_fread_int32_from_le )

#define eh_fwrite_int32_swap   ( eh_fwrite_int32_to_le  )
#define eh_fwrite_int64_swap   ( eh_fwrite_int64_to_le  )
#define eh_fread_int32_swap    ( eh_fread_int32_from_le )
#define eh_fread_int64_swap    ( eh_fread_int64_from_le )

#define eh_fread_flt_swap      ( eh_fread_int32_from_le )
#define eh_fread_dbl_swap      ( eh_fread_int64_from_le )
#define eh_fwrite_flt_swap     ( eh_fwrite_int32_to_le )
#define eh_fwrite_dbl_swap     ( eh_fwrite_int64_to_le )

#endif

typedef enum
{
   EH_STATUS_BAR_RUNNING ,
   EH_STATUS_BAR_PAUSED  ,
   EH_STATUS_BAR_STOPPED
}
Eh_status_bar_status;

//typedef struct _Eh_status_bar Eh_Status_bar;
typedef struct
{
   double*  cur;
   double*  end;
   GThread* t;
   GTimer*  timer;
   GMutex*  mutex;
   Eh_status_bar_status status;
}
Eh_status_bar;

Eh_status_bar* eh_status_bar_new       ( double* cur , double* end );
Eh_status_bar* eh_status_bar_stop      ( Eh_status_bar* b );
gboolean       eh_status_bar_is_stopped( Eh_status_bar* b );
Eh_status_bar* eh_status_bar_pause     ( Eh_status_bar* b );
Eh_status_bar* eh_status_bar_destroy   ( Eh_status_bar* b );

gchar* eh_render_time_str( double sec , gchar* str );

gchar*   eh_render_command_str( int argc , char* argv[] );
gboolean eh_check_to_s( gboolean assert , const gchar* str , gchar*** str_list );
gchar*   eh_get_input_val( FILE *fp , char *msg , char *default_str );

#endif /* utils.h is included */
