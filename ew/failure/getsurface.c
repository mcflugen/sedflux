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

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "failure.h"

#ifndef SECONDS_PER_YEAR
# define SECONDS_PER_YEAR 31536000.
#endif

#define FAIL_LOCAL_MODEL
#undef FAIL_WITH_THREADS

Sed_cube get_failure_surface( const Sed_cube p , int start , int len )
{
   Sed_cube fail;
//    GStaticMutex fail_mutex=G_STATIC_MUTEX_INIT;
   double *ellipse;
   gssize *x, *y;
   int i;
   
/*
   g_mutex_lock( fail_mutex );
   if ( !fail )
   {
// create a profile that is large enough to hold any potential failure surface.
      fail = sed_create_cube( 1 , FAIL_MAX_FAILURE_LENGTH , p->sed );
   }
   g_mutex_unlock( fail_mutex );
*/

   // if there is no sediment in the first column, return null.
   if ( sed_column_thickness( sed_cube_col(p,start) ) < 1e-5 )
      return NULL;

   // if the end of the failure is higher than the start, return null.  
   // this should be changed to allow failures on slopes that are negative.
   // the main reason to turn this off is because the turbidity current 
   // model isn't set up to move flows from right to left.
   if (   sed_cube_top_height(p,0,start)
        < sed_cube_top_height(p,0,start+len-1) )
      return NULL;

   x = eh_new0( gssize , len );
   y = eh_new0( gssize , len );
   ellipse = eh_new0( double , len );

   // get the elevations of the failure surface.  the failure surface 
   // will either be a quarter ellipse (get_ellipse) or a semi circle 
   // (get_circle).
   if ( !get_circle( p , start , len , ellipse ) )
   {
      eh_free(x);
      eh_free(y);
      eh_free(ellipse);
      return NULL;
   }
   for (i=0;i<len;i++)
   {
      x[i] = 0;
      y[i] = i+start;
   }

   // remove the failure surface from the profile.
//   fail = sed_get_1d_cube_from_cube( p , x , ellipse , len , NULL );
   fail = sed_cube_copy_cols( p , x , y , ellipse , len );

   // check if the failure surface is ok.  if there are columns with 
   // no sediment, return null.
   for ( i=1 ; i<sed_cube_n_y(fail)-1 ; i++ )
      if ( sed_cube_thickness(fail,0,i) < 1e-5 )
      {
         sed_cube_destroy(fail);
         eh_free(x);
         eh_free(y);
         eh_free(ellipse);
         return NULL;
      }

   eh_free(x);
   eh_free(y);
   eh_free(ellipse);

   return fail;
}

gboolean fail_check_failure_plane_is_valid( const Sed_cube p , 
                                            int start        , 
                                            int len          , 
                                            const double *failure_plane )
{
   int i;

   eh_require( p!=NULL             );
   eh_require( sed_cube_is_1d(p)   );
   eh_require( failure_plane!=NULL );

   if ( start+len >= sed_cube_n_y(p) )
      return FALSE;

   if (   sed_cube_top_height( p,0,start       ) 
        < sed_cube_top_height( p,0,start+len-1 ) )
      return FALSE;

   for ( i=1 ; i<len-1 ; i++ )
   {
      if ( failure_plane[i] >= sed_cube_top_height( p,0,start+i ) )
         return FALSE;
      if ( failure_plane[i] < sed_cube_base_height( p,0,start+i ) )
         return FALSE;
   }

   return TRUE;
}

double *get_circle( const Sed_cube p  ,
                    int failure_start ,
                    int n_points      ,
                    double *circle)
{
   int i;
   double x0, y0;
   double r;
   double rise, run;
   double width;
   double *x;

   eh_require( p!=NULL           );
   eh_require( sed_cube_is_1d(p) );
   eh_require( circle!=NULL      );

   if ( failure_start+n_points >= sed_cube_n_y(p) )
      return NULL;

   width = sed_cube_y_res(p);

   rise = fabs( 
          sed_cube_top_height( p , 0 , failure_start ) 
        - sed_cube_top_height( p , 0 , failure_start+n_points-1) );
   run = (n_points-1)*width;

   if ( rise < 1e-3 )
      return NULL;

   r = (pow(rise,2) + pow(run,2))/2./rise;

   y0 = sed_cube_top_height(p,0,failure_start+n_points-1) + r;
   x0 = (n_points-1)*width;

   x = eh_new0(double,n_points);
   for (i=1;i<n_points;i++)
      x[i] = x[i-1] + width;

   for (i=0;i<n_points;i++)
      circle[i] = y0 - sqrt(pow(r,2) - pow(x[i]-x0,2));

   eh_free(x);

   if ( !fail_check_failure_plane_is_valid( p             ,
                                            failure_start ,
                                            n_points      ,
                                            circle ) )
      return NULL;

   return circle;
}

double *get_ellipse( const Sed_cube p ,
                     int failureStart ,
                     int nPoints ,
                     double *ellipse )
{
   int i;
   double x0, y0;
   double rx, ry;
   double width, *x;

   eh_require( p );
   eh_require( sed_cube_is_1d(p) );

   x = eh_new( double , nPoints );
   width = sed_cube_y_res(p);

   ry = fabs(   sed_cube_top_height(p,0,failureStart)
              - sed_cube_top_height(p,0,failureStart+nPoints-1));
   rx = (nPoints-1)*width;

   x0 = rx;
   y0 = MAX( sed_cube_top_height(p,0,failureStart) ,
             sed_cube_top_height(p,0,failureStart+nPoints-1) );


   if (   sed_cube_top_height(p,0,failureStart)
        > sed_cube_top_height(p,0,failureStart+nPoints-1) )
      x[0] = 0;
   else
      x[0] = rx;

   for (i=1;i<nPoints;i++)
      x[i] = x[i-1] + width;
   
   for (i=0;i<nPoints;i++)
      ellipse[i] = y0 - ry*sqrt(1-pow((x[i]-x0)/rx,2));

   eh_free(x);
   
   return ellipse;
}

double get_m(const Sed_column s,double depth, double consolidation)
{
   double delta_t, rate, t;
   double i1, i2;

   if (sed_column_thickness(s) > 0 )
   {

      // Get the index to the top bin.
      i2 = sed_column_index_depth(s,0);

      // Get the index to the bottom bin.
      i1 = sed_column_index_depth(s,depth);
      if ( i1 < 0 ) i1 = 0;

      // Time (in seconds) over which this thickness of sediment was 
      // deposited.
      delta_t = (   sed_cell_age( sed_column_nth_cell(s,(int)i2) )
                  - sed_cell_age( sed_column_nth_cell(s,(int)i1) ) )
                *S_SECONDS_PER_YEAR;

      // The average rate at which this sediment was deposited.
      if ( delta_t <= 0. )
         rate = 0.;
      else
         rate = depth/delta_t;

      // The time factor used in polynomial approximation for constant
      // relating excess pore pressure to gamma' and failure depth.
      t = rate*rate*delta_t/consolidation;

      // Return the approximated relation.
      if ( t > 16 )
         return 1.;
      else
         return 6.4*pow(1.-t/16.,17)+1.;
   }
   else
      return 1;
}

#define FAIL_MIN_COL_LEN (128)

Fail_column *fail_create_fail_column( int n , gboolean allocate )
{
   int i;
   Fail_column *f;

   if ( n<0 )
      n=0;

   f = eh_new( Fail_column , 1 );

   f->w      = eh_new( double , FAIL_MIN_COL_LEN );
   f->u      = eh_new( double , FAIL_MIN_COL_LEN );
   f->phi    = eh_new( double , FAIL_MIN_COL_LEN );
   f->height = eh_new( double , FAIL_MIN_COL_LEN );
   f->c      = eh_new( double , FAIL_MIN_COL_LEN );

   f->len    = FAIL_MIN_COL_LEN;
   f->size   = n;

   if ( allocate )
      fail_resize_fail_column( f , n );

   for ( i=0 ; i<MAX_FAILURE_LENGTH ; i++ )
      f->fs[i] = FAIL_FOS_NOT_VALID;

   f->need_update = TRUE;

   return f;
}

Fail_column *fail_reuse_fail_column( Fail_column *f )
{
   int i;
   for ( i=0 ; i<MAX_FAILURE_LENGTH ; i++ )
      f->fs[i] = 999;

   f->need_update = TRUE;

   return f;
}

Fail_column *fail_resize_fail_column( Fail_column *f , int n )
{
   if ( n>f->len )
   {
      f->w      = g_renew( double , f->w      , n );
      f->u      = g_renew( double , f->u      , n );
      f->phi    = g_renew( double , f->phi    , n );
      f->height = g_renew( double , f->height , n );
      f->c      = g_renew( double , f->c      , n );
      f->len    = n;
   }

   f->size = n;

   return f;
}

void fail_destroy_fail_column( Fail_column *f )
{

   if ( f!=NULL )
   {
      eh_free( f->w   );
      eh_free( f->u   );
      eh_free( f->c   );
      eh_free( f->phi );
      eh_free( f->height );

      eh_free( f );
   }

   return;
}

void fail_dump_fail_column( Fail_column *f , FILE *fp )
{
   fwrite( f         , sizeof(Fail_column) , 1 , fp );
   fwrite( f->w      , sizeof(double) , f->len , fp );
   fwrite( f->u      , sizeof(double) , f->len , fp );
   fwrite( f->phi    , sizeof(double) , f->len , fp );
   fwrite( f->height , sizeof(double) , f->len , fp );
   fwrite( f->c      , sizeof(double) , f->len , fp );
}

gboolean fail_load_fail_column( Fail_column *f , FILE *fp )
{
   fread( f , sizeof(Fail_column) , 1 , fp );

   f->w      = eh_new( double , f->len );
   f->u      = eh_new( double , f->len );
   f->phi    = eh_new( double , f->len );
   f->height = eh_new( double , f->len );
   f->c      = eh_new( double , f->len );

   fread( f->w      , sizeof(double) , f->len , fp );
   fread( f->u      , sizeof(double) , f->len , fp );
   fread( f->phi    , sizeof(double) , f->len , fp );
   fread( f->height , sizeof(double) , f->len , fp );
   fread( f->c      , sizeof(double) , f->len , fp );

   return TRUE;
}

typedef struct
{
   Sed_cube p;
   Failure_t fail_const;
   double *failure_line;
   Fail_column **slice;
}
Init_failure_t;

void init_column( gpointer data , gpointer user_data );

Fail_profile *fail_init_fail_profile( Sed_cube p ,
                                      Failure_t fail_const )
{
   return fail_reinit_fail_profile( NULL , p , fail_const );
}

#define FAIL_INIT_N_THREADS (10)

Fail_profile *fail_reinit_fail_profile( Fail_profile *f ,
                                        Sed_cube p      ,
                                        Failure_t fail_const )
{
   int i, n_cols;
#ifdef FAIL_WITH_THREADS
   GThreadPool *t_pool;
#endif
   double *failure_line;
   Init_failure_t data;
   int *queue;

   eh_require( p!=NULL );
   eh_require( sed_cube_is_1d(p) );

#ifdef FAIL_WITH_THREADS
   if ( !g_thread_supported() ) g_thread_init( NULL );
#endif

   n_cols = sed_cube_n_y(p);

   if ( f )
      f = fail_reuse_fail_profile( f );
   else
      f = fail_create_fail_profile( n_cols );

   f = fail_resize_fail_profile( f , n_cols );

   f->p          = p;
   f->fail_const = fail_const;

   failure_line = fail_get_failure_line( p );

   data.p            = p;
   data.fail_const   = fail_const;
   data.failure_line = failure_line;
   data.slice        = f->col;

#ifdef FAIL_WITH_THREADS
   t_pool = g_thread_pool_new( (GFunc)&init_column , 
                               &data , 
                               FAIL_INIT_N_THREADS , 
                               TRUE , 
                               NULL );
#endif

   queue = eh_new( int , sed_cube_n_y(p) );

   // allocate memory for the failure slices.
   for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
   {
      queue[i] = i;

#ifdef FAIL_WITH_THREADS
      g_thread_pool_push( t_pool , &(queue[i]) , NULL );
#else
      init_column( &(queue[i]) , &data );
#endif

   }

#ifdef FAIL_WITH_THREADS
   g_thread_pool_free( t_pool , FALSE , TRUE );
#endif

   eh_free( failure_line );
   eh_free( queue );

   return f;
}

Fail_profile *fail_reuse_fail_profile( Fail_profile *f )
{
   eh_require( f!=NULL );

   f->p            = NULL;

   f->fs_min_val   = 999;
   f->fs_min_start = -1;
   f->fs_min_len   = -1;

   f->count = 0;

   return f;
}

Fail_profile *fail_create_fail_profile( int n_cols )
{
   Fail_profile *f;

   f       = eh_new ( Fail_profile , 1      );
   f->col  = eh_new0( Fail_column* , n_cols );
   f->size = n_cols;
   f->len  = n_cols;

   f->count = 0;

   return f;
}

void fail_dump_fail_profile( Fail_profile *f , FILE *fp )
{
   int i;
   fwrite( f , sizeof(Fail_profile) , 1 , fp );
   for ( i=0 ; i<f->size ; i++ )
      fail_dump_fail_column( f->col[i] , fp );
   sed_cube_write( fp , f->p );
}

void fail_load_fail_profile( FILE *fp )
{
   int i;
   Fail_profile *f = eh_new( Fail_profile , 1 );
   
   fread( f , sizeof(Fail_profile) , 1 , fp );
   f->col = eh_new( Fail_column* , f->len );
   for ( i=0 ; i<f->size ; i++ )
      fail_load_fail_column( f->col[i] , fp );
   f->p = sed_cube_read( fp );
}

Fail_profile *fail_resize_fail_profile( Fail_profile *f , int n_cols )
{
   int i;
   if ( n_cols > f->len )
   {
      f->col = g_renew( Fail_column* , f->col , n_cols );
      for ( i=f->len ; i<n_cols ; i++ )
         f->col[i] = NULL;
      f->len = n_cols;
   }

   f->size = n_cols;

   return f;
}

void init_column( gpointer data , gpointer user_data )
{
   int i                = *((int*)data);
   Sed_cube p           = ((Init_failure_t*)user_data)->p;
   Failure_t fail_const = ((Init_failure_t*)user_data)->fail_const;
   double *failure_line = ((Init_failure_t*)user_data)->failure_line;
   Fail_column **slice  = ((Init_failure_t*)user_data)->slice;

   slice[i] = fail_reinit_fail_column( slice[i]          ,
                                       sed_cube_col(p,i) ,
                                       failure_line[i]   ,
                                       fail_const );

   return;
}

void fail_destroy_failure_profile( Fail_profile *f )
{
   int i;

   if ( f!=NULL )
   {
      for ( i=0 ; i<f->size ; i++ )
         fail_destroy_fail_column( f->col[i] );
      eh_free( f->col );
      eh_free( f      );
   }
   return;
}

#define FAIL_NO_FAIL_SURFACE (G_MAXDOUBLE)

double *fail_get_failure_line( Sed_cube p )
{
   int i, j, len;
   double *ellipse;
   double *failure_line;

   eh_require( p!=NULL );
   eh_require( sed_cube_is_1d(p) );

   ellipse      = eh_new( double , MAX_FAILURE_LENGTH );
   failure_line = eh_new( double , sed_cube_n_y(p)    );

   for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      failure_line[i] = FAIL_NO_FAIL_SURFACE;

   // draw potential failure planes to determine which cells need 
   // to be examined.
   for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
   {
      for ( len=MIN_FAILURE_LENGTH ; len<MAX_FAILURE_LENGTH ; len++ )
         if ( get_circle( p , i , len , ellipse ) )
         {
            for ( j=0 ; j<len && i+j<sed_cube_n_y(p) ; j++ )
               if ( ellipse[j]<failure_line[i+j] )
                  failure_line[i+j] = ellipse[j];
         }
   }

   eh_free( ellipse );

   return failure_line;
}

#define N_THREADS (4)

void fail_examine_fail_profile( Fail_profile *p )
{
   int i, j;
#ifdef FAIL_WITH_THREADS
   GThreadPool *fos_pool[N_THREADS];
#endif
   int river_mouth, block_size;
   int start;
   int **queue;

   eh_require( p!=NULL );

#ifdef FAIL_WITH_THREADS
   if ( !g_thread_supported() ) g_thread_init( NULL );

   for ( i=0 ; i<N_THREADS ; i++ )
      fos_pool[i] = g_thread_pool_new( (GFunc)&get_node_fos ,
                                       p ,
                                       1 ,
                                       TRUE ,
                                       NULL );
#endif

// divide the profile up into blocks of block_size columns each.  
// a single thread pool will work on each block.
   river_mouth = sed_cube_river_mouth_1d( p->p ) - 3;
   if ( river_mouth<0 )
      river_mouth = 0;
   block_size  = ( p->size-river_mouth ) / N_THREADS;
   queue       = eh_new( int* , N_THREADS );

   for ( i=0 ; i<N_THREADS ; i++ )
   {
      queue[i] = eh_new( int , block_size );
      start    = i*block_size + river_mouth;

      for ( j=0 ; j<block_size ; j++, start++ )
      {
         queue[i][j] = start;

#ifdef FAIL_WITH_THREADS
         g_thread_pool_push( fos_pool[i] , &(queue[i][j]) , NULL );
#else
         get_node_fos( &(queue[i][j]) , p );
#endif

      }
   }

#ifdef FAIL_WITH_THREADS
   for ( i=0 ; i<N_THREADS ; i++ )
      g_thread_pool_free( fos_pool[i] , FALSE , TRUE );
#endif

   for ( i=0 ; i<N_THREADS ; i++ )
      eh_free( queue[i] );
   eh_free( queue );

// mark each column as updated.
   for ( i=0 ; i<p->size ; i++ )
      p->col[i]->need_update = FALSE;

   return;
}

void fail_reset_fail_profile( Fail_profile *p )
{
   int start, len;

   p->fs_min_val   = 999;
   p->fs_min_start = -1;
   p->fs_min_len   = -1;

   for ( start=0 ; start<p->size ; start++ )
   {
      for ( len=0 ; len<MAX_FAILURE_LENGTH ; len++ )
         p->col[start]->fs[len] = FAIL_FOS_NOT_VALID;
      p->col[start]->need_update = TRUE;
   }
}

void fail_update_fail_profile( Fail_profile *p )
{
   int i;
   int start, len;
   Fail_column *f_col;
   Sed_column s_col;
   double *failure_line;
   double fs, fs_min_val;
   double fail_col_h, sed_col_h;

   eh_require( p!=NULL );

   // calculate the new elevations of the failure line.
   failure_line = fail_get_failure_line( p->p );

p->count = 0;

   // look for changes in the elevation of the failure plane between the 
   // sed_profile and the fail_profile.
   for ( i=0 ; i<p->size ; i++ )
   {

      f_col = p->col[i];
      s_col = sed_cube_col(p->p,i);

      if ( f_col->size>0 )
         fail_col_h = f_col->height[f_col->size-1];
      else
         fail_col_h = f_col->failure_line;
      sed_col_h  = sed_column_top_height( s_col );
/*
eh_watch_dbl( fabs(fail_col_h-sed_col_h) );
eh_watch_dbl( f_col->failure_line );
eh_watch_dbl( fail_col_h );
eh_watch_dbl( sed_col_h );
eh_watch_int( f_col->size-1 );
eh_watch_dbl_vec( f_col->height , 0 , f_col->size-1 );
*/

      // Assume the the factor of safety does not need to be recalculated if
      // the column did not change in thickness by some amount.  Say for now,
      // 10cm.
//      if ( fabs(fail_col_h-sed_col_h) > FAIL_MIN_DELTA_H )
      if ( fabs(fail_col_h-sed_col_h) > .1 )
      {

         p->col[i] = fail_reinit_fail_column( p->col[i] , 
                                              s_col , 
                                              failure_line[i] , 
                                              p->fail_const );
         p->col[i]->need_update = TRUE;

p->count++;

      }

   }

   eh_free( failure_line );

// mark all of the failure surfaces that need to be calculated again.
   for ( i=0 ; i<p->size ; i++ )
      for ( len=0 ; len<MAX_FAILURE_LENGTH ; len++ )
        if ( i+len<p->size && p->col[i+len]->need_update )
           p->col[i]->fs[len] = FAIL_FOS_NOT_VALID;

// reset the minimum factor of safety values for the profile.
   p->fs_min_val   = 999;
   p->fs_min_start = -1;
   p->fs_min_len   = -1;

// find the minimum factor of safety from the surfaces that have not changed.
   for ( start=0 , fs_min_val=999 ; start<p->size ; start++ )
   {
      for ( len=0 ; len<MAX_FAILURE_LENGTH ; len++ )
      {
         fs = p->col[start]->fs[len];
         if ( fail_fos_is_valid(fs) && fs<fs_min_val )
         {
            fs_min_val      = fs;
            p->fs_min_val   = fs_min_val;
            p->fs_min_start = start;
            p->fs_min_len   = len;
         }
      }
   }

   eh_debug( "%d of %d columns have changed since last examination" ,
            p->count ,
            p->size );

   return;
}

void get_node_fos( gpointer data , gpointer user_data )
{
#ifdef FAIL_WITH_THREADS
   static GStaticMutex fs_min_lock = G_STATIC_MUTEX_INIT;
#endif
   int fail_start = *((int*)data);
   Fail_profile *p = (Fail_profile*)user_data;
   double fs, fs_min_val = 999;
   int fs_min_len;
   int fail_len;

   for ( fail_len = MIN_FAILURE_LENGTH ;
         fail_len < MAX_FAILURE_LENGTH && fail_start+fail_len < p->size ;
         fail_len++ )
   {
      if ( !fail_get_ignore_surface( p , fail_start , fail_len ) )
      {
         fs = fail_get_fail_profile_fos( p , fail_start , fail_len );

// NOTE: Toggle between these statements to choose the surface to fail.
// Choose the surface with the lowest factor of safety or choose the
// largest surface that is less than the factor of safety needed for failure.
         if ( fail_fos_is_valid(fs) && fs<fs_min_val )
//         if (    fail_fos_is_valid(fs)
//              && fs<MIN_FACTOR_OF_SAFETY
//              && fail_len>fs_min_len )
         {
            fs_min_val = fs;
            fs_min_len = fail_len;
         }

         p->col[fail_start]->fs[fail_len] = fs;
      }
      else
         eh_debug( "ignoring surface at (%d,%d)" ,
                  fail_start ,
                  fail_len );

   }

#ifdef FAIL_WITH_THREADS
   g_static_mutex_lock( &fs_min_lock );
#endif

   if ( fail_fos_is_valid( fs_min_val ) && fs_min_val<p->fs_min_val )
   {
      p->fs_min_val   = fs_min_val;
      p->fs_min_start = fail_start;
      p->fs_min_len   = fs_min_len;
   }
p->count++;

#ifdef FAIL_WITH_THREADS
   g_static_mutex_unlock( &fs_min_lock );
#endif

   return;
}

gboolean fail_fos_is_valid( double fos )
{
   return (fos<0)?FALSE:TRUE;
}

gboolean fail_surface_is_valid( double h )
{
   return (h<FAIL_NO_FAIL_SURFACE)?TRUE:FALSE;
}

void fail_set_failure_surface_ignore( Fail_profile *f , int start , int len )
{
   f->col[start]->fs[len] = FAIL_IGNORE_SURFACE;
}

gboolean fail_get_ignore_surface( Fail_profile *f , int start , int len )
{
   return f->col[start]->fs[len]<=FAIL_IGNORE_SURFACE;
}

double fail_get_fail_profile_fos( Fail_profile *f , int start , int len )
{
   Fail_slice **s;
   double *ellipse;
   int i;
   double fs;
   gboolean need_update=FALSE;

   // first check if the fos has already been calculated.
   for ( i=0 ; i<len && need_update==FALSE ; i++ )
      if ( f->col[start+i]->need_update )
         need_update = TRUE;

   if ( need_update )
   {
      ellipse = eh_new( double , len );

      // get the elevations to the failure surface.
      if ( get_circle( f->p , start , len , ellipse ) )
      {

         s = fail_get_janbu_parameters( f , start , ellipse , len );
         if ( s )
         {

            fs = rtsafe_fos( &factor_of_safety , s , .005 , 500 , .01 );

            // s is null terminated.
            for ( i=0 ; s[i] ; i++ )
               eh_free( s[i] );
            eh_free( s );
         }
         else
            fs = FAIL_FOS_NOT_VALID;

      }
      else
         fs = FAIL_FOS_NOT_VALID;

      eh_free( ellipse );
   }
   else
      fs = f->col[start]->fs[len];

   return fs;
}

Fail_slice **fail_get_janbu_parameters( Fail_profile *f ,
                                        int start ,
                                        double *fail_height ,
                                        int fail_len )
{
   int i;
   Fail_slice *s;
   Fail_slice **s_vec;
   Fail_column *c;
   double *depth;
   double a_angle=M_PI/8.;
   int ind, n_bins;

   eh_require( f!=NULL           );
   eh_require( fail_height!=NULL );

   depth = eh_new( double , fail_len );

   for ( i=0 ; i<fail_len && start+i<f->size ; i++ )
   {
      c = f->col[start+i];
      n_bins = c->size;
      if ( f->col[start+i]->size==0 )
      {
         eh_free( depth );
         return NULL;
      }
      if ( fail_height[i] > sed_cube_top_height( f->p , 0 , start+i ) )
      {
         eh_free( depth );
         return NULL;
      }

      if ( fail_height[i] < sed_cube_base_height( f->p,0,start+i ) )
         depth[i] = sed_cube_base_height( f->p,0,start+i );
      else
         depth[i] = fail_height[i];
/*
      if ( depth[i] > c->height[n_bins-1] )
      {
         eh_free( depth );
         return NULL;
      }
*/
   }

   s_vec = eh_new( Fail_slice* , fail_len+1 );
   s_vec[fail_len] = NULL;

   for ( i=0 ; i<fail_len ; i++ )
   {
      c = f->col[start+i];

      n_bins = c->size;

      // determine the index to the cell at the failure surface.
      for ( ind=0 ; ind<n_bins && c->height[ind]<depth[i] ; ind++ );
      if ( ind>= n_bins )
         ind = n_bins - 1;

      s = eh_new( Fail_slice , 1 );

      s->a_vertical   = sed_cube_quake( f->p )*cos(a_angle);
      s->a_horizontal = sed_cube_quake( f->p )*sin(a_angle);
//      s->depth        = c->height[n_bins-1] - depth[i];
//      s->depth        = sed_get_floor_height( c ) - depth[i];

      // The thickness of sediment in the failure (s->depth) should be
      // measured from the sea floor to the failure plane.  Remember that
      // c->depth[] is the elevation measured to the bottom of each bin.
      s->depth        = sed_cube_top_height( f->p,0,start+i ) - depth[i];

      s->c            = c->c[ind];
      s->u            = c->u[ind];
      s->phi          = c->phi[ind];
//      s->b            = f->p->colWidth;
      s->b            = sed_cube_y_res(f->p);
      s->w            = c->w[ind]*s->b;

      if ( i==fail_len-1 )
         s->alpha     = fabs(atan( (depth[i]-depth[i-1])/s->b) );
      else
         s->alpha     = fabs(atan( (depth[i+1]-depth[i])/s->b) );

      if ( s->u > .9*s->w/s->b )
         s->u = .9*s->w/s->b;

      s_vec[i] = s;
/*
if ( start == 788 && fail_len == 22 )
{
   eh_watch_dbl( s->a_vertical );
   eh_watch_dbl( s->a_horizontal );
   eh_watch_dbl( s->depth );
   eh_watch_dbl( s->c );
   eh_watch_dbl( s->u );
   eh_watch_dbl( s->phi );
   eh_watch_dbl( s->b );
   eh_watch_dbl( s->w );
   eh_watch_dbl( s->alpha );
   eh_watch_dbl( s->u );
   eh_watch_int( n_bins );
}
*/

/*
if ( s->depth < 0 )
   eh_error( "DEPTH = %f" , s->depth );
*/

   }
/*
if ( start == 788 && fail_len == 22 )
   eh_exit(-1);
*/


   eh_free( depth );

   return s_vec;
}

Fail_column *fail_reinit_fail_column( Fail_column *f ,
                                      Sed_column c   ,
                                      double h       ,
                                      Failure_t fail_const )
{
   int i, i_bot;
   double depth;
   double gravity = sed_gravity();
   int n_bins;
   double hydro_static;
   Sed_property friction_angle = sed_property_new( "friction_angle" );
   Sed_property cohesion       = sed_property_new( "cohesion" );

   eh_require( c!=NULL );

   // check the number of cells in the new column and make sure that it
   // is less then the length of the recycled column.
   if ( fail_surface_is_valid( h ) )
      n_bins = sed_column_top_nbins( c , h );
   else
      n_bins = 0;

   if ( f )
      f = fail_reuse_fail_column( f );
   else
      f = fail_create_fail_column( n_bins , TRUE );

   f = fail_resize_fail_column( f , n_bins );

   if ( n_bins>0 )
   {

      i_bot = sed_column_index_at( c , h );

      hydro_static = sed_column_water_pressure( c );

#ifdef FAIL_LOCAL_MODEL   // the local model.
      f->phi  = sed_column_avg_property( friction_angle ,
                                         c ,
                                         i_bot ,
                                         n_bins ,
                                         f->phi );
      f->c    = sed_column_avg_property_with_load( cohesion ,
                                                   c ,
                                                   i_bot ,
                                                   n_bins ,
                                                   f->c );
#endif

      for ( i=0 ; i<n_bins ; i++ )
      {
         f->height[i] = sed_column_base_height( c ) 
                      + sed_cell_size( sed_column_nth_cell( c , i_bot+i-1 ) );

         // The height of the failure column (f->height) is measured to the
         // bottom of a bin and so if the failure plane (h) cuts through this
         // bin, it will be heigher than the elevation of the bin.
         if ( f->height[i] < h )
            f->height[i] = h;
         depth        = sed_column_top_height( c ) - f->height[i];

// the weight of the sediment.
//         f->w[i]      = sed_get_top_rho( c , depth );
//         f->w[i]      = (f->w[i]-fail_const.density_sea_water)*gravity*depth;
         f->w[i]      = ( sed_column_top_rho(c,depth)
                        - fail_const.density_sea_water)
                      * gravity*depth;

#ifdef FAIL_LOCAL_MODEL   // the local model.

//         f->phi[i]    = sed_get_top_property( S_FRICTION_ANGLE , c , depth )
         f->phi[i]   *= S_RADS_PER_DEGREE;
//         f->c[i]      = sed_get_top_property_with_load( S_COHESION ,
//                                                        c ,
//                                                        depth );
         f->u[i]      = sed_cell_excess_pressure( sed_column_nth_cell(c,i_bot+i) ,
                                                  hydro_static );

#else                     // the global model

         f->phi[i]    = fail_const.frictionAngle;
         f->c[i]      = fail_const.cohesion;
         f->u[i]      = f->w[i] / get_m( c , depth , fail_const.consolidation );

#endif

      }

   }

   f->failure_line = h;
/*
   if ( h>1e50 )
   {
      eh_message( "%f" , h );
      eh_message( "%f" , FAIL_NO_FAIL_SURFACE );
      eh_error( "FAILURE LINE : %f" , h );
   }
*/

   sed_property_destroy( friction_angle );
   sed_property_destroy( cohesion       );

   return f;
}


Fail_column *fail_init_fail_column( Sed_column c , double h , Failure_t fail_const )
{
   return fail_reinit_fail_column( NULL , c , h , fail_const );
}

