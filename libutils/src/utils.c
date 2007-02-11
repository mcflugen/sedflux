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

#include <math.h>
#include <stdlib.h>
#include "utils.h"

void eh_init_glib( void )
{
#if defined( USE_MY_VTABLE )
   static GMemVTable my_vtable;

   my_vtable.malloc      = &eh_malloc_c_style;
   my_vtable.realloc     = &eh_realloc_c_style;
   my_vtable.free        = &eh_free_c_style;
   my_vtable.calloc      = &eh_calloc_c_style;
   my_vtable.try_malloc  = &eh_malloc_c_style;
   my_vtable.try_realloc = &eh_realloc_c_style;

   g_mem_set_vtable( &my_vtable );
#else
//   g_mem_set_vtable( glib_mem_profiler_table );
#endif
}

void
eh_exit( int code )
{
   fprintf( stderr , "Exiting program.  " );

//   fprintf( stderr , "Looking for memory leaks...\n" );
//   eh_walk_heap( );

#ifdef G_PLATFORM_WIN32
   fprintf( stderr , "Hit enter to continue..." );

   fflush( stdin );
   while ( '\n' != getchar() )
   {
   }
#endif
   fprintf( stderr , "\n" );

   exit( code );
}

gchar* brief_copyleft_msg[] =
{
"Copywrite (C) 2006 Eric Hutton." ,
"This is free software; see the source for copying conditions.  This is NO" ,
"warranty;  not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." ,
NULL
};

gint eh_fprint_version_info( FILE* fp          ,
                             const gchar* prog ,
                             gint maj          ,
                             gint min          ,
                             gint micro )
{
   fprintf( fp , "%s %d.%d.%d\n" , prog , maj , min , micro );

   fprintf( fp , "Written by Eric Hutton <eric.hutton@colorado.edu>.\n" );
   fprintf( fp , "\n" );

   eh_print_message( fp , brief_copyleft_msg );
}

/** Test if the index to a cell is within a square domain.

@param n_i The number of i elements in the domain.
@param n_j The number of j elements in the domain.
@param i   An i coordinate to test.
@param j   An j coordinate to test.

@return TRUE if the coordinate is within the domain.
*/
gboolean is_in_domain( gssize n_i , gssize n_j , gssize i , gssize j )
{
   return i>=0 && j>=0 && i<n_i && j<n_j;
}

/* Compute a weighted average of the data in vector x, given the weights in vector f.
   Vectors are of length len.
*/
double weighted_avg(const double x[],const double f[],long len)
{
   long i;
   double sum;
   
   for (i=0,sum=0;i<len;i++)
      sum += f[i]*x[i];
   
   return sum;
}

#ifndef M_LN2
# define M_LN2 0.69314718055994530942
#endif

#ifndef log2
double log2(double x)
{
   return log(x)/M_LN2;
}
#endif

#undef M_LN2

#if defined( OLD_EH_NAN )

float eh_nan(void)
{
   gint32 a=0x7FF00000L;
   return *((float*)(&a));
}

int eh_isnan(float x)
{
   return (    ((*(gint32*)&(x) & 0x7F800000L) == 0x7F800000L)
            && ((*(gint32*)&(x) & 0x007FFFFFL) != 0x00000000L) );
}
#else

double eh_nan( void )
{
//   return sqrt(-1);
   return strtod( "NAN" , NULL );
}

gboolean eh_isnan( double x )
{
   return isnan(x);
}
#endif

#if defined(USING_C_PLUS_PLUS)

vector<point<double,double> >& interpolate(vector<point<double,double> >& A, vector<point<double,double> >& B )
{
   int i, j;
   double m, b, x0;

   /* Initialize output with NaN's.
   */
   for (j=0;j<B.size();j++);
      B[j].second = eh_nan();

   /* Set j to the first index inside of the given data.
   */
   for (j=0;B[j].first<A[0].first;j++);

   /* Interpolate linearly between points.
   */
   for (i=0;i<A.size()-1;i++)
   {
      m = (A[i+1].second-A[i].second)/(A[i+1].first-A[i].first);
      b = A[i].second;
      x0 = A[i].first;
      while ( j<B.size() && B[j].first <= A[i+1].first )
      {
         B[j].second = m*(B[j].first-x0)+b;
         j++;
      }
   }

   return B;
}

#endif

void eh_rebin_dbl_array( double *x     , double *y     , gssize len ,
                         double *x_bin , double *y_bin , gssize len_bin )
{
   eh_rebin_dbl_array_bad_val( x     , y     , len     ,
                               x_bin , y_bin , len_bin ,
                               eh_nan() );
}

void eh_rebin_dbl_array_bad_val(
        double *x     , double *y     , gssize len     ,
        double *x_bin , double *y_bin , gssize len_bin ,
        double bad_val )
{
   gssize i, j;
   gssize top_i, top_j, lower_j, upper_j;
//   gssize lower_edge, upper_edge;
//   double left, right;
//   gssize n;
   double left_bin, right_bin, lower, upper, upper_bin, lower_bin;
   double sum;
   double *x_edge;

   // initialize y_bin with NaN's.
   for ( i=0 ; i<len_bin ; y_bin[i]=bad_val , i++ );

   top_i = len_bin-1;
   top_j = len-1;
   lower_bin = x_bin[0]     - ( x_bin[1]     - x_bin[0]       ) * .5;
   upper_bin = x_bin[top_i] + ( x_bin[top_i] - x_bin[top_i-1] ) * .5;
   lower = x[0]     - ( x[1]     - x[0]       ) * .5;
   upper = x[top_j] + ( x[top_j] - x[top_j-1] ) * .5;

   // define the edges of the data.
   x_edge      = eh_new( double , len+1 );
   x_edge[0]   = x[0] - ( x[1] - x[0] ) * .5;
   x_edge[len] = x[top_j] + ( x[top_j] - x[top_j-1] ) * .5;
   for ( j=1 ; j<len ; j++ )
      x_edge[j] = ( x[j-1] + x[j] ) * .5;
/*
   // set j to the index of the first data point that is inside a bin.
   for ( j=0 ; j<len && !(x[j]>=lower && x[j]<upper) ; j++ );
   if ( j==len )
      return;

   // set i to the first bin that contains data.
   for ( i=0 ; i<len_bin && !(x_bin[i]>=x[0] && x_bin[i]<x[top_j]) ; i++ );
   if ( i==len_bin )
      return;
*/
   upper_j = 0;
   for ( i=0 ; i<len_bin && j<=len ; i++ )
   {

      // Integrate the data in y over the entire bin.
      left_bin  = (i!=0)    ?( x_bin[i]   + x_bin[i-1] ) * .5 : lower_bin;
      right_bin = (i!=top_i)?( x_bin[i+1] + x_bin[i]   ) * .5 : upper_bin;

      // Find the lower j.
      for ( j=upper_j ; j<=len && x_edge[j]<=left_bin ; j++ );
      lower_j = j-1;
      eh_clamp( lower_j , 0 , len );

      // Find the upper j.
      for ( j=lower_j ; j<=len && x_edge[j]<=right_bin ; j++ );
/*
if ( len_bin == 2 )
{
   eh_watch_int( lower_j );
   eh_watch_int( j );
   eh_watch_int( len );
   eh_watch_dbl( left_bin );
   eh_watch_dbl( right_bin );
   eh_watch_dbl( x_edge[0] );
   eh_watch_dbl( x_edge[1] );
}
*/
      upper_j = j;
      eh_clamp( upper_j , 0 , len );
//eh_watch_int( upper_j );
//eh_watch_int( lower_j );
//eh_watch_dbl( x_edge[lower_j] );
//eh_watch_dbl( x_edge[upper_j] );
//eh_watch_dbl( left_bin );
//eh_watch_dbl( right_bin );

      // Integrate the data over these j.
//      sum  = y[lower_j]*(left_bin-x_edge[lower_j])/(right_bin-left_bin);
//      sum += y[upper_j]*(right_bin-x_edge[upper_j])/(right_bin-left_bin);

      sum  = 0;
      if ( left_bin > x_edge[upper_j] || right_bin < x_edge[lower_j] )
         sum = bad_val;
      else if ( upper_j-lower_j>1 )
      {
//eh_watch_int( i );
         if ( left_bin >= x_edge[lower_j] )
            sum += y[lower_j]
                 * (x_edge[lower_j+1]-left_bin);
         else
            sum += y[lower_j]
                 * (x_edge[lower_j+1]-x_edge[lower_j]);
//              / (x_edge[lower_j+1]-x_edge[lower_j])
//              * (x_edge[lower_j+1]-x_edge[lower_j])
//              / (x_edge[lower_j+1]-left_bin);
//eh_watch_dbl( sum );
         if ( right_bin <= x_edge[upper_j] )
            sum += y[upper_j-1] 
                 * (right_bin-x_edge[upper_j-1]);
         else
            sum += y[upper_j-1] 
                 * (x_edge[upper_j]-x_edge[upper_j-1]);
//              / (x_edge[upper_j]-x_edge[upper_j-1])
//              * (x_edge[upper_j]-x_edge[upper_j-1])
//              / (right_bin-x_edge[lower_j+1]);
//eh_watch_dbl( sum );
//eh_watch_dbl_vec( y , lower_j+1, upper_j-2 );
         for ( j=lower_j+1 ; j<upper_j-1 ; j++ )
            sum += y[j]*(x_edge[j+1]-x_edge[j]);
//eh_watch_dbl( sum );
         sum /= (right_bin-left_bin);
if ( sum<0 )
{
   eh_watch_int( i );
   eh_watch_int( lower_j );
   eh_watch_int( upper_j );
   eh_watch_dbl( left_bin );
   eh_watch_dbl( right_bin );
   eh_watch_dbl( sum );
   eh_watch_dbl( x_edge[lower_j] );
   eh_watch_dbl( x_edge[upper_j] );
}
//eh_watch_dbl( sum );
      }
      else
         for ( j=lower_j ; j<upper_j ; j++ )
            sum += y[j];
//            sum += y[j]*(right_bin-left_bin)/(x_edge[j+1]-x_edge[j]);


//eh_watch_dbl( sum );

      y_bin[i] = sum;
/*

      // This is the left and right edge of the current datum.
      left  = (j!=0)    ?( x[j]   + x[j-1] ) * .5 : lower;
      right = (j!=top_j)?( x[j+1] + x[j]   ) * .5 : upper;

      // This datum is entirely in the bin.
      if ( left>=left_bin && right<right_bin )
         sum += y[j];
      // This datum is 
      else ( left>left_bin && right<right_bin )
      else ( left>=left_bin && right>right_bin )

      for ( sum=0 ; j<len && left>=left_bin && right<right_bin; j++,n++ )



      for ( n=0,sum=0 ; j<len && x[j]>=left_bin && x[j]<right_bin; j++,n++ )
         sum += y[j];

      if ( n==0 )
      {
         if ( j>=0 )
         {
            y_bin[i] = ( y[j] - y[j-1] ) / ( x[j] - x[j-1] ) * (x_bin[i]-x[j-1])
                     + y[j-1];
      y_bin[i] *= ( x_bin[i] - x_bin[i-1] ) / ( x[j] - x[j-1] );
         }
      }
      else
         y_bin[i] = sum;
//         y_bin[i] = sum/(double)n;
*/
   }

   eh_free( x_edge );

   return;
}

#if defined( OLD_NDGRID )

void eh_rebin_dbl_grid( Eh_dbl_grid *source , Eh_dbl_grid *dest )
{
   eh_rebin_dbl_grid_bad_val( source , dest , eh_nan() );
}

void eh_rebin_dbl_grid_bad_val( Eh_dbl_grid *source , Eh_dbl_grid *dest ,
                                double bad_val )
{
   gssize i,j;
   double **temp, *temp_source, *temp_dest;
   gssize src_low_x=source->low_x, dest_low_x=dest->low_x;
   gssize src_low_y=source->low_y, dest_low_y=dest->low_y;

   eh_reindex_grid( (Eh_grid*)source , 0 , 0 );
   eh_reindex_grid( (Eh_grid*)dest   , 0 , 0 );

   temp    = eh_new( double* , source->n_x );
   temp[0] = eh_new( double  , source->n_x*dest->n_y );
   for ( i=1 ; i<source->n_x ; i++ )
      temp[i] = temp[i-1] + dest->n_y;
   temp_source = eh_new( double , source->n_x );
   temp_dest   = eh_new( double , dest->n_x   );

   for ( i=0 ; i<source->n_x ; i++ )
      eh_rebin_dbl_array_bad_val( source->y , source->data[i] , source->n_y ,
                                  dest->y   , temp[i]         , dest->n_y   ,
                                  bad_val );

   for ( j=0 ; j<dest->n_y ; j++ )
   {
      for ( i=0 ; i<source->n_x ; i++ )
         temp_source[i] = temp[i][j];

      eh_rebin_dbl_array_bad_val( source->x , temp_source , source->n_x ,
                                  dest->x   , temp_dest   , dest->n_x   ,
                                  bad_val );

      for ( i=0 ; i<dest->n_x ; i++ )
         dest->data[i][j] = temp_dest[i];
   }

   eh_free( temp_dest   );
   eh_free( temp_source );
   eh_free( temp[0]     );
   eh_free( temp        );

   eh_reindex_grid( (Eh_grid*)source , src_low_x  , src_low_y  );
   eh_reindex_grid( (Eh_grid*)dest   , dest_low_x , dest_low_y );

   return;
}

#endif

#if defined( OLD_NDGRID )
void interpolate_2( Eh_dbl_grid *source , Eh_dbl_grid *dest )
{
   interpolate_2_bad_val( source , dest , eh_nan() );
}

void interpolate_2_bad_val( Eh_dbl_grid *source , Eh_dbl_grid *dest ,
                            double bad_val )
{
   gssize i,j;
   double **temp, *temp_source, *temp_dest;
   gssize src_low_x=source->low_x, dest_low_x=dest->low_x;
   gssize src_low_y=source->low_y, dest_low_y=dest->low_y;

   eh_reindex_grid( (Eh_grid*)source , 0 , 0 );
   eh_reindex_grid( (Eh_grid*)dest   , 0 , 0 );

   temp    = eh_new( double* , source->n_x );
   temp[0] = eh_new( double  , source->n_x*dest->n_y );
   for ( i=1 ; i<source->n_x ; i++ )
      temp[i] = temp[i-1] + dest->n_y;
   temp_source = eh_new( double , source->n_x );
   temp_dest   = eh_new( double , dest->n_x   );

   for ( i=0 ; i<source->n_x ; i++ )
      interpolate_bad_val( source->y , source->data[i] , source->n_y ,
                           dest->y   , temp[i]         , dest->n_y   ,
                           bad_val );

   for ( j=0 ; j<dest->n_y ; j++ )
   {
      for ( i=0 ; i<source->n_x ; i++ )
         temp_source[i] = temp[i][j];

      interpolate_bad_val( source->x , temp_source , source->n_x ,
                           dest->x   , temp_dest   , dest->n_x   ,
                           bad_val );

      for ( i=0 ; i<dest->n_x ; i++ )
         dest->data[i][j] = temp_dest[i];
   }

   eh_free( temp_dest   );
   eh_free( temp_source );
   eh_free( temp[0]     );
   eh_free( temp        );

   eh_reindex_grid( (Eh_grid*)source , src_low_x  , src_low_y  );
   eh_reindex_grid( (Eh_grid*)dest   , dest_low_x , dest_low_y );

   return;
}
#else
void interpolate_2( Eh_dbl_grid source , Eh_dbl_grid dest )
{
   interpolate_2_bad_val( source , dest , eh_nan() );
}

void interpolate_2_bad_val( Eh_dbl_grid source , Eh_dbl_grid dest ,
                            double bad_val )
{
   gssize i,j;
   Eh_dbl_grid temp;
   double *temp_source, *temp_dest;
   gssize src_low_x=eh_grid_low_x(source), dest_low_x=eh_grid_low_x(dest);
   gssize src_low_y=eh_grid_low_y(source), dest_low_y=eh_grid_low_y(dest);

   eh_grid_reindex( source , 0 , 0 );
   eh_grid_reindex( dest   , 0 , 0 );

   temp        = eh_grid_new( double , eh_grid_n_x(source) , eh_grid_n_y(dest) );
   temp_source = eh_new( double , eh_grid_n_x(source) );
   temp_dest   = eh_new( double , eh_grid_n_x(dest)   );

   for ( i=0 ; i<eh_grid_n_x(source) ; i++ )
      interpolate_bad_val( eh_grid_y(source) , eh_grid_row(source,i) , eh_grid_n_y(source) ,
                           eh_grid_y(dest)   , eh_grid_row(temp,i)   , eh_grid_n_y(dest)   ,
                           bad_val );

   for ( j=0 ; j<eh_grid_n_y(dest) ; j++ )
   {
      for ( i=0 ; i<eh_grid_n_x(source) ; i++ )
         temp_source[i] = eh_dbl_grid_val(temp,i,j);

      interpolate_bad_val( eh_grid_x(source) , temp_source , eh_grid_n_x(source) ,
                           eh_grid_x(dest)   , temp_dest   , eh_grid_n_x(dest)   ,
                           bad_val );

      for ( i=0 ; i<eh_grid_n_x(dest) ; i++ )
         eh_dbl_grid_set_val( dest , i , j , temp_dest[i] );
//         dest->data[i][j] = temp_dest[i];
   }

   eh_free( temp_dest   );
   eh_free( temp_source );
   eh_grid_destroy( temp , TRUE );

   eh_grid_reindex( source , src_low_x  , src_low_y  );
   eh_grid_reindex( dest   , dest_low_x , dest_low_y );

   return;
}

#endif

#include <string.h>

FILE *eh_fopen(const char *filename, const char *type)
{
   FILE *fp;

   eh_require( filename!=NULL );

   fp = fopen(filename,type);
   if ( !fp )
   {
      g_error( "eh_fopen: could not open file: %s" , filename );
      fp = NULL;
   }

   return fp;
}

#include <errno.h>

FILE* eh_fopen_error( const char* file , const char* type , GError** error )
{
   FILE* fp = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   fp = fopen( file , type );
   if ( !fp && error )
   {
      GFileError file_error = g_file_error_from_errno( errno );
      gchar*     err_str    = NULL;

      switch ( file_error )
      {
         case G_FILE_ERROR_EXIST:
            err_str = g_strdup( "Operation not permitted" ); break;
         case G_FILE_ERROR_ISDIR:
            err_str = g_strdup( "File is a directory" ); break;
         case G_FILE_ERROR_ACCES:
            err_str = g_strdup( "Permission denied" ); break;
         case G_FILE_ERROR_NAMETOOLONG:
            err_str = g_strdup( "Filename too long" ); break;
         case G_FILE_ERROR_NOENT:
            err_str = g_strdup( "No such file or directory" ); break;
         case G_FILE_ERROR_NOTDIR:
            err_str = g_strdup( "Not a directory" ); break;
         case G_FILE_ERROR_NXIO:
            err_str = g_strdup( "No such device or address" ); break;
         case G_FILE_ERROR_NODEV:
            err_str = g_strdup( "No such device" ); break;
         case G_FILE_ERROR_ROFS:
            err_str = g_strdup( "Read-only file system" ); break;
         case G_FILE_ERROR_TXTBSY:
            err_str = g_strdup( "Text file busy" ); break;
         case G_FILE_ERROR_FAULT:
            err_str = g_strdup( "Pointer to bad memory" ); break;
         case G_FILE_ERROR_LOOP:
            err_str = g_strdup( "Too many levels of symbolic links" ); break;
         case G_FILE_ERROR_NOSPC:
            err_str = g_strdup( "No space left on device" ); break;
         case G_FILE_ERROR_NOMEM:
            err_str = g_strdup( "No memory available" ); break;
         case G_FILE_ERROR_MFILE:
            err_str = g_strdup( "The current process has too many open files" ); break;
         case G_FILE_ERROR_NFILE:
            err_str = g_strdup( "The entire system has too many open files" ); break;
         case G_FILE_ERROR_BADF:
            err_str = g_strdup( "Bad file descriptor" ); break;
         case G_FILE_ERROR_INVAL:
            err_str = g_strdup( "Invalid argument" ); break;
         case G_FILE_ERROR_PIPE:
            err_str = g_strdup( "Broken pipe" ); break;
         case G_FILE_ERROR_AGAIN:
            err_str = g_strdup( "Resource temporarily unavailable" ); break;
         case G_FILE_ERROR_INTR:
            err_str = g_strdup( "Interrupted function call" ); break;
         case G_FILE_ERROR_IO:
            err_str = g_strdup( "Input/output error" ); break;
         case G_FILE_ERROR_PERM:
            err_str = g_strdup( "Output not permitted" ); break;
         case G_FILE_ERROR_NOSYS:
            err_str = g_strdup( "Function not implemented" ); break;
         case G_FILE_ERROR_FAILED:
            err_str = g_strdup( "Error unknown" ); break;
      }

      g_set_error( error , G_FILE_ERROR , file_error , "%s: %s" , file , err_str );

      eh_free( err_str );
   }
   return fp;
}

FILE* eh_open_file( const char *filename , const char *type )
{
   FILE *fp;
   if ( filename )
      fp = eh_fopen( filename , type );
   else
      fp = stdin;

   return fp;
}

FILE *eh_open_temp_file( const char *template , char **name_used )
{
   GError *error;
   int fd = g_file_open_tmp( template , name_used , &error );
   return fdopen( fd , "w+" );
}

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

gboolean eh_is_readable_file( const char* filename )
{
   FILE *fp = fopen( filename , "r" );
   if ( fp )
   {
      fclose(fp);
      return TRUE;
   }
   else
      return FALSE;
}

gboolean eh_is_writable_file( const char* filename )
{
   FILE *fp = fopen( filename , "a" );
   if ( fp )
   {
      fclose(fp);
      return TRUE;
   }
   else
      return FALSE;
}

#include <glib/gstdio.h>

gboolean eh_try_open( const char* file )
{
   gboolean open_ok = FALSE;

   if ( file )
   {
      if ( g_file_test( file , G_FILE_TEST_EXISTS ) )
         open_ok = TRUE;
      else
      {
         char* dir = g_path_get_dirname( file );

         open_ok = eh_open_dir( dir );

         eh_free( dir );
      }
   }

   return open_ok;
}

gboolean eh_open_dir( const char* dir )
{
   gboolean open_ok;

   eh_require( dir );

   //---
   // If the directory exists, great.  Otherwise try to create it (and its
   // parents, if necessary.
   //---
   if ( !g_file_test( dir , G_FILE_TEST_EXISTS|G_FILE_TEST_IS_DIR ) )
   {
      gboolean parent_exists;
      char* parent_dir = g_path_get_dirname( dir );

      //---
      // Create the parent directory if it doesn't exist.
      //---
      if ( g_file_test( parent_dir , G_FILE_TEST_EXISTS|G_FILE_TEST_IS_DIR ) )
         parent_exists = TRUE;
      else
         parent_exists = eh_open_dir( parent_dir );

      //---
      // Now that the parent has been created (hopefully), create the new
      // directory.
      //---
      if ( parent_exists )
      {
         char* msg = g_strdup_printf( "Ok to create directory %s" , dir );

         if (    eh_input_boolean( msg , TRUE )
              && g_mkdir( dir , S_ISUID|S_ISGID|S_IRWXU|S_IRWXG ) == 0 )
            open_ok = TRUE;
         else
            open_ok = FALSE;

         eh_free( msg );
      }
      else
         open_ok = FALSE;

      eh_free( parent_dir );
   }
   else
      open_ok = TRUE;

   return open_ok;
}

gboolean try_open(const char *filename, int flags)
{
   int fd = open(filename,flags,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
   if ( fd < 0 )
   {
      fprintf(stderr,"warning : try_open : %s : %s\n",strerror(errno),filename);
      return FALSE;
   }
   else
   {
      close(fd);
      return TRUE;
   }
}

#include <dirent.h>

gboolean try_dir(const char *filename)
{
   int fd = open(filename,O_RDONLY);
   if ( fd < 0 )
   {
      fprintf(stderr,"warning : try_dir : %s : %s\n",strerror(errno),filename);
      return FALSE;
   }
   else
   {
      close(fd);
      return TRUE;
   }
}

void **eh_alloc_2( gssize m , gssize n , gssize size )
{
   gssize i;
   void **p=NULL;

   if ( m>0 && n>0 && size>0 )
   {
//      p = (void**)malloc( m*sizeof(void*) );
      p = eh_new( void* , m );
      if ( !p )
         eh_error( "Failed to allocate %d bytes" , n*sizeof(void*) );
//      p[0] = (void*)malloc( m*n*size );
//      p[0] = eh_malloc( m*n*size , NULL , __FILE__ , __LINE__ );
      p[0] = eh_new( gchar ,  m*n*size );
      if ( !p )
         eh_error( "Failed to allocate %d bytes" , n*m*size );
      for ( i=1 ; i<m ; i++ )
         p[i] = (gint8*)(p[i-1])+size*n;
   }

   return p;
}

void eh_free_void_2( void **p )
{
   if ( p )
   {
      if ( p[0] )
         eh_free( p[0] );
      eh_free( p );
   }
}

/*
void *malloc1D(int n)
{
   void *ptr;
   ptr = malloc(n);
   if ( !ptr )
   {
      fprintf(stderr,"malloc1d : failed to allocate %d bytes\n",n);
      eh_exit(-1);
   }
   return ptr;
}
*/
/*
double **malloc2Ddouble(int m,int n)
{
   double **ptr;
   int i;
   
   ptr = (double**)malloc1D(sizeof(double*)*m);
   ptr[0] = (double*)malloc1D(sizeof(double)*m*n);

   for (i=1;i<m;i++)
      ptr[i] = ptr[i-1] + n;
   return ptr;
}

void free2D(double **ptr)
{
   eh_free(ptr[0]);
   eh_free(ptr);
   return;
}
*/

#include <stdio.h>
//#include <values.h>
#include <string.h>

#ifndef DATA_DELIMETER
# define DATA_DELIMETER ":"
#endif

#ifndef TOKEN_DELIMETER 
# define TOKEN_DELIMETER ","
#endif

int peek_list_length(FILE *fp)
{
   long i, start, finish;
   char **svec;
   start = finish = 0;
   start = ftell(fp);

   svec = read_string_vector(fp,-1);
   for (i=0;svec[i]!=NULL;i++);
   
   finish = ftell(fp);
   fseek(fp,start-finish,SEEK_CUR);
   return i;
}

int read_double_vector(FILE *fp,double *val,int len)
{
   char **str_vec=NULL;
   int i=0;
   str_vec=read_string_vector(fp,len);
   if ( str_vec )
      for (i=0;str_vec[i]!=NULL;i++)
         sscanf(str_vec[i],"%lf",&val[i]);
   g_strfreev( str_vec );
   return i;
}

int read_time_vector( FILE *fp , double *val , int len )
{
   char **str_vec=NULL;
   int i=0;
   str_vec=read_string_vector(fp,len);
   if ( str_vec )
      for ( i=0 ; str_vec[i]!=NULL ; i++ )
         val[i] = strtotime( str_vec[i] );
   g_strfreev( str_vec );
   return i;
}

int read_int_vector(FILE *fp,int *val,int len)
{
   char **str_vec=NULL;
   int i=0;
   str_vec=read_string_vector(fp,len);
   if ( str_vec )
      for (i=0;str_vec[i]!=NULL;i++)
         sscanf(str_vec[i],"%d",&(val[i]));
   g_strfreev( str_vec );
   return i;
}

int read_int_value(FILE *fp,int *val)
{
   read_int_vector(fp,val,1);
   return val[0];
}

double read_double_value(FILE *fp,double *val)
{
   read_double_vector(fp,val,1);
   return val[0];
}

#include <string.h>
// NOTE: it doesn't look like i free str.  i don't see that there is
// any reason not to.
char *read_string_value(FILE *fp,char *val)
{
   char **str;

   str = read_string_vector(fp,1);
   if ( str )
      strcpy(val,str[0]);
   else
      val = NULL;

   return val;
}

//#include <values.h>
#include <stdlib.h>

char **read_string_vector(FILE *fp,int len)
{
   char line[S_LINEMAX], *str;
   char **val=NULL;
   int i;

   // set to an empty string in case there is no more to read in the file.
   line[0] = '\0';

   // advance to the next line containing a label-data entry.
   do
   {
      fgets(line,S_LINEMAX,fp);
   }
   while ( !feof(fp) && no_data_line(line) );
   if ( strstr(line,DATA_DELIMETER) )
   {
      strtok(line,DATA_DELIMETER);
      if ( len==-1 )
      {
         len = G_MAXINT;
         val = NULL;
      }
      str=strtok(NULL,TOKEN_DELIMETER);
      for (i=0; str!=NULL && i<len ;i++)
      {
//         val = (char**)realloc(val,(i+2)*sizeof(char*));
//         val[i] = (char*)malloc1D(sizeof(char)*(strlen(str)+1));
         val    = eh_renew( char* , val , (i+2) );
         val[i] = eh_new( char , strlen(str)+1 );
         strcpy(val[i],str);
         str=strtok(NULL,TOKEN_DELIMETER);
      }
      if ( val )
         val[i] = NULL;
   }
   return val;
}

#define COMMENT_CHAR ';'

int no_data_line(char *line)
{
   if ( line[0] == COMMENT_CHAR || !strstr(line,DATA_DELIMETER) )
      return 1;
   else
      return 0;
}

#include <string.h>

#define WHITE_SPACE " \t\n"

char *remove_white_space(char *str)
{
   int i, j;
   char *str_temp;
// Add one for the terminating null character.
   str_temp = eh_new( char , strlen(str)+1 );
   strcpy(str_temp,str);
   for (i=0,j=0;i<strlen(str_temp);i++)
      if ( strchr(WHITE_SPACE,str_temp[i]) == NULL )
      {
         str[j] = str_temp[i];
         j++;
      }
   str[j] = '\0';
   eh_free(str_temp);
   return str;
}

char *trimleft(char *str)
{
   char *ptr;

//   ptr = (char*)malloc1D(sizeof(char)*(strlen(str)+1));
   ptr = eh_new( char , strlen(str)+1 );
   strcpy(ptr,str);
   while ( strchr(WHITE_SPACE,ptr[0])!=0 && ptr!='\0' )
      ptr++;
   strcpy(str,ptr);
   return str;
}

char *trimright(char *str)
{
   char *ptr;
   int len;

//   ptr = (char*)malloc1D(sizeof(char)*(strlen(str)+1));
   ptr = eh_new( char , strlen(str)+1 );
   strcpy(ptr,str);
   len = strlen(ptr);
   while ( strchr(WHITE_SPACE,ptr[len-1])!=0 && len>0 )
   {
      ptr[len-1]='\0';
      len--;
   }
   strcpy(str,ptr);
   return str;
}

#undef DATA_DELIMETER

double **read_matrix(FILE *fp,int nColumns,int *nRows)
{
   double **matrix=NULL;
   double *row;
   int i, j;

//   row = (double*)malloc1D(sizeof(double)*nColumns);
   row = eh_new( double , nColumns );

   /* See how many rows there will be. */
   for (i=0; read_double_vector(fp,row,nColumns) ;i++);
   rewind(fp);
   *nRows = i;

//   matrix = (double**)malloc2Ddouble(nColumns,*nRows);
   matrix = eh_new_2( double , nColumns , *nRows );
   for (i=0; read_double_vector(fp,row,nColumns) ;i++)
      for (j=0;j<nColumns;j++)
         matrix[j][i] = row[j];

   return matrix;
}

pos_t *createPosVec(int size)
{
   pos_t *v;
//   v = (pos_t*)malloc1D(sizeof(pos_t));
//   v->x = (double*)malloc1D(sizeof(double)*size);
//   v->y = (double*)malloc1D(sizeof(double)*size);
   v    = eh_new( pos_t  , 1    );
   v->x = eh_new( double , size );
   v->y = eh_new( double , size );
   v->size = size;
   return v;
}

void destroyPosVec(pos_t *v)
{
   eh_free(v->x);
   eh_free(v->y);
   eh_free(v);
   return;
}

#if defined( OLD_NDGRID )
Eh_ind_2 eh_create_ind_2( int i , int j )
{
   Eh_ind_2 ind;
   ind.i = i;
   ind.j = j;
   return ind;
}
#endif

Eh_pt_2 eh_create_pt_2( double x , double y )
{
   Eh_pt_2 pt;
   pt.x = x;
   pt.y = y;
   return pt;
}

gboolean eh_cmp_pt_2( Eh_pt_2 a , Eh_pt_2 b , double eps )
{
   return fabs( a.x-b.x ) < eps && fabs( a.y-b.y ) < eps;
}

#if defined( OLD_NDGRID )
gboolean eh_cmp_ind_2( Eh_ind_2 a , Eh_ind_2 b )
{
   return a.i==b.i && a.j==b.j;
}

Eh_ind_2 *eh_dup_ind_2( Eh_ind_2 *src , Eh_ind_2 *dest )
{
   if ( !dest )
      dest = eh_new( Eh_ind_2 , 1 );
   dest->i = src->i;
   dest->j = src->j;
   return dest;
}
#endif

#define DERIVATIVE_FORWARD_DIFFERENCE

double *derivative(pos_t v)
{
   int i;
   double *slope;
//   slope = (double*)malloc1D(sizeof(double)*v.size);
   slope = eh_new( double , v.size );
#if defined(DERIVATIVE_FORWARD_DIFFERENCE)
   for (i=0;i<v.size-1;i++)
      slope[i] = (v.y[i+1]-v.y[i])/(v.x[i+1]-v.x[i]);
   slope[v.size-1] = slope[v.size-2];
#elif defined(DERIVATIVE_BACKWARD_DIFFERENCE)
   for (i=1;i<v.size;i++)
      slope[i] = (v.y[i]-v.y[i-1])/(v.x[i]-v.x[i-1]);
   slope[0] = slope[1];
#elif defined(DERIVATIVE_CENTERED_DIFFERENCE)
   for (i=1;i<v.size-1;i++)
      slope[i] = (v.y[i+1]-v.y[i-1])/(v.x[i+1]-v.x[i-1]);
   slope[v.size-1] = slope[v.size-2];
   slope[0] = slope[1];
#endif
   return slope;
}

#include <string.h>

gboolean read_logical_value(FILE *fp)
{
   char str[S_LINEMAX];
   read_string_value(fp,str);
   trimleft(trimright(str));
   if (    g_ascii_strcasecmp( str , "ON"  ) == 0
        || g_ascii_strcasecmp( str , "YES" ) == 0 )
      return TRUE;
   if (    g_ascii_strcasecmp( str , "OFF" ) == 0
        || g_ascii_strcasecmp( str , "NO"  ) == 0 )
      return FALSE;
   else
   {
      eh_info( "read_logical_value: unknown flag -- %s\n" , str );
      eh_info( "read_logical_value: valid options are: 'ON', or 'OFF'.\n");
      eh_exit(-1);
   }

   eh_require_not_reached();
   return FALSE;

}

gboolean strtobool(const char *str)
{
   if ( g_ascii_strcasecmp(str,"YES"  ) == 0 ||
        g_ascii_strcasecmp(str,"ON"   ) == 0 ||
        g_ascii_strcasecmp(str,"TRUE" ) == 0 ||
        g_ascii_strcasecmp(str,"OUI"  ) == 0 )
      return TRUE;
   else if (
        g_ascii_strcasecmp(str,"NO"   ) == 0 ||
        g_ascii_strcasecmp(str,"OFF"  ) == 0 ||
        g_ascii_strcasecmp(str,"FALSE") == 0 ||
        g_ascii_strcasecmp(str,"NON"  ) == 0 )
      return FALSE;
   else
   {
      eh_info( "strtobool : unknown boolean value-- %s\n" , str );
      eh_exit(-1);
   }
   eh_require_not_reached();
   return FALSE;
}

double convert_time_to_years(Eh_date_t *time)
{
   return time->year+time->month/12.+time->day/365.;
}

/* Convert the time given in str to time in years.
#include <string>

double scan_time_in_years(FILE *fp)
{
   char *str;
   double time;
   str = scan_string_value(fp);
   time = sscan_time_in_years(str);
   return time;
}
*/

double strtotime(const char *str)
{
   return sscan_time_in_years(str);
}

double sscan_time_in_years(const char *s)
{
   char unit=0;
//   string str(s);
   GString *str=g_string_new(s);
   int n;
   double val;
   Eh_date_t time={0,0,0};

   eh_string_remove_white_space(str);

   while ( (n=sscanf(eh_string_c_str(str),"%lf%c",&val,&unit)) > 0 )
   {
      if ( n == 1 )
      {
         fprintf(stderr,"missing time unit -- %s\n",eh_string_c_str(str));
         eh_exit(-1);
      }
 
      switch ( unit )
      {
         case 'd':
            time.day = val;
            break;
         case 'm':
            time.month = val;
            break;
         case 'y':
            time.year = val;
            break;
         default:
            eh_info( "invalid string -- %c\n" , unit );
            eh_exit(-1);
      }
      g_string_erase(str,0,eh_string_find_first_of(str,unit)+1);
   }

   return convert_time_to_years(&time);
}

/*
dvector *vec_create_dvector(int n)
{
   dvector *dvec;
//   dvec = (dvector*)malloc1D(sizeof(dvector));
//   dvec->v = (double*)malloc1D(sizeof(double)*n);
   dvec    = eh_new( dvector , 1 );
   dvec->v = eh_new( double  , n );
   dvec->size = n;
   return dvec;
}

void vec_destroy_dvector(dvector *dvec)
{
   eh_free(dvec->v);
   eh_free(dvec);
   return;
}

dvector *vec_resize_dvector(dvector *dvec,int n)
{
//   dvec->v = (double*)realloc(dvec->v,n);
   dvec->v = eh_renew( double , dvec->v , n );
   dvec->size = n;
   return dvec;
}

int vec_get_dvector_size(dvector *dvec)
{
   return dvec->size;
}

double vec_get_dvector_val(dvector *dvec,int index)
{
   return dvec->v[index];
}

dvector *vec_set_dvector_val(dvector *dvec,int index, double val)
{
   dvec->v[index] = val;
   return dvec;
}

dvector *vec_push_dvector_val(dvector *dvec,double val)
{
   dvec = vec_resize_dvector(dvec,dvec->size+1);
   dvec->v[dvec->size-1] = val;
   return dvec;
}

double vec_pop_dvector_val(dvector *dvec)
{
   double val;
   if ( dvec->size == 1 )
      val = dvec->v[0];
   else
   {
      val = dvec->v[dvec->size-1];
      dvec = vec_resize_dvector(dvec,dvec->size-1);
   }
   return val;
}
//
int vec_write_dvector(dvector *dvec,FILE *fp)
{
   fwrite(dvec->v,sizeof(double),dvec->size,fp);
   return 0;
}
//
int vec_read_dvector(dvector *dvec,FILE *fp)
{
   int n_items;
   double *list;

   n_items = peek_list_length(fp);
//   list = (double*)malloc1D(n_items*sizeof(double));
   list = eh_new( double , n_items );
   read_double_vector(fp,list,n_items);

   dvec = vec_resize_dvector(dvec,0);
   dvec->v = list;
   dvec->size = n_items;

   return dvec->size;
}
*/

/* Thread pools */
#include <pthread.h>

void tpool_thread( tpool_t tpool );

void tpool_init(tpool_t *tpoolp, int num_worker_threads, int max_queue_size, int do_not_block_when_full)
{
   int i, rtn;
   tpool_t tpool;
   pthread_attr_t attr;

   pthread_attr_init( &attr );
   pthread_attr_setscope( &attr , PTHREAD_SCOPE_SYSTEM );

//   if ( (tpool = (tpool_t)malloc(sizeof(struct tpool))) == NULL )
//      perror("malloc"), eh_exit(-1);
//   tpool = (tpool_t)eh_malloc( sizeof(tpool_t) , NULL , __FILE__ , __LINE__ );
   tpool = eh_new( struct tpool , 1 );

   tpool->num_threads = num_worker_threads;
   tpool->max_queue_size = max_queue_size;
   tpool->do_not_block_when_full = do_not_block_when_full;
//   if ( (tpool->threads = (pthread_t*)malloc(sizeof(pthread_t)*num_worker_threads)) == NULL )
//      perror("malloc"), eh_exit(-1);
   tpool->threads = eh_new( pthread_t , num_worker_threads );
   
   tpool->cur_queue_size = 0;
   tpool->queue_head = NULL;
   tpool->queue_tail = NULL;
   tpool->queue_closed = 0;
   tpool->shutdown = 0;
   if ( (rtn=pthread_mutex_init(&(tpool->queue_lock),NULL)) != 0 )
      fprintf(stderr,"pthread_mutex_init %s",strerror(rtn)), eh_exit(-1);
   if ( (rtn=pthread_cond_init(&(tpool->queue_not_empty),NULL)) != 0 )
      fprintf(stderr,"pthread_cond_init %s",strerror(rtn)), eh_exit(-1);
   if ( (rtn=pthread_cond_init(&(tpool->queue_not_full),NULL)) != 0 )
      fprintf(stderr,"pthread_cond_init %s",strerror(rtn)), eh_exit(-1);
   if ( (rtn=pthread_cond_init(&(tpool->queue_empty),NULL)) != 0 )
      fprintf(stderr,"pthread_cond_init %s",strerror(rtn)), eh_exit(-1);

   for ( i=0 ; i!=num_worker_threads ; i++ )
      if ( (rtn=pthread_create( &(tpool->threads[i]) , &attr , (void *(*)(void*))tpool_thread , (void*)tpool )) != 0 )
         fprintf(stderr,"pthread_create %d",rtn), eh_exit(-1);

   pthread_attr_destroy(&attr);

   *tpoolp = tpool;
}

void tpool_thread( tpool_t tpool )
{
   tpool_work_t *my_workp;

   for (;;)
   {
      pthread_mutex_lock( &(tpool->queue_lock) );
      while ( (tpool->cur_queue_size==0) && (!tpool->shutdown) )
      {
         pthread_cond_wait( &(tpool->queue_not_empty) , &(tpool->queue_lock) );
      }

      if ( tpool->shutdown )
      {
         pthread_mutex_unlock( &(tpool->queue_lock) );
         pthread_exit( NULL );
      }

      my_workp = tpool->queue_head;
      tpool->cur_queue_size--;
      if ( tpool->cur_queue_size == 0 )
         tpool->queue_head = tpool->queue_tail = NULL;
      else
         tpool->queue_head = my_workp->next;

      if ( (!tpool->do_not_block_when_full) && (tpool->cur_queue_size==(tpool->max_queue_size-1)) )
         pthread_cond_broadcast( &(tpool->queue_not_full) );

      if ( tpool->cur_queue_size == 0 )
         pthread_cond_signal( &(tpool->queue_empty) );

      pthread_mutex_unlock( &(tpool->queue_lock) );
      (*(my_workp->routine))(my_workp->arg);
      eh_free(my_workp);
   }
}

int tpool_add_work( tpool_t tpool , void *routine , void *arg )
{
   tpool_work_t *workp;
   pthread_mutex_lock( &(tpool->queue_lock) );

   if (    ( tpool->cur_queue_size==tpool->max_queue_size )
        && tpool->do_not_block_when_full )
   {
      pthread_mutex_unlock( &(tpool->queue_lock) );
      return -1;
   }

   while (    ( tpool->cur_queue_size==tpool->max_queue_size )
           && ( !(tpool->shutdown || tpool->queue_closed) ) )
   {
      pthread_cond_wait( &(tpool->queue_not_full) , &(tpool->queue_lock) );
   }

   if ( tpool->shutdown || tpool->queue_closed )
   {
      pthread_mutex_unlock( &(tpool->queue_lock) );
      return -1;
   }

//   workp = (tpool_work_t*)malloc(sizeof(tpool_work_t));
   workp = eh_new( tpool_work_t , 1 );
   workp->routine = (void(*)())routine;
   workp->arg = arg;
   workp->next = NULL;
   if ( tpool->cur_queue_size == 0 )
   {
      tpool->queue_tail = tpool->queue_head = workp;
      pthread_cond_broadcast( &(tpool->queue_not_empty) );
   }
   else
   {
      (tpool->queue_tail)->next = workp;
      tpool->queue_tail = workp;
   }
   tpool->cur_queue_size++;

   pthread_mutex_unlock( &(tpool->queue_lock) );
   return 1;
}

int tpool_destroy( tpool_t tpool , int finish )
{
   int i, rtn;
   tpool_work_t *cur_nodep;

   if ( (rtn=pthread_mutex_lock( &(tpool->queue_lock) )) != 0 )
      fprintf(stderr,"pthread_mutex_lock %d",rtn), eh_exit(-1);

   if ( tpool->queue_closed || tpool->shutdown )
   {
      if ( (rtn=pthread_mutex_unlock( &(tpool->queue_lock) )) != 0 )
         fprintf(stderr,"pthread_mutex_unlock %d",rtn), eh_exit(-1);
      return 0;
   }

   tpool->queue_closed = 1;

   if ( finish==1 )
   {
      while ( tpool->cur_queue_size!=0 )
         if ( (rtn=pthread_cond_wait( &(tpool->queue_empty) , &(tpool->queue_lock) )) != 0 )
            fprintf(stderr,"pthread_cond_wait %d",rtn), eh_exit(-1);
   }

   tpool->shutdown = 1;

   if ( (rtn=pthread_mutex_unlock( &(tpool->queue_lock) ))!=0 )
      fprintf(stderr,"pthread_mutex_unlock %d",rtn), eh_exit(-1);

   if ( (rtn=pthread_cond_broadcast( &(tpool->queue_not_empty) ))!=0 )
      fprintf(stderr,"pthread_cond_broadcast %d",rtn), eh_exit(-1);
   if ( (rtn=pthread_cond_broadcast( &(tpool->queue_not_full) ))!=0 )
     fprintf(stderr,"pthread_cond_broadcast %d",rtn), eh_exit(-1);

   for ( i=0 ; i<tpool->num_threads ; i++ )
   {
      if ( (rtn=pthread_join( tpool->threads[i] , NULL ))!=0 )
         fprintf(stderr,"pthread_join %d",rtn), eh_exit(-1);
   }

   eh_free(tpool->threads);
   while ( tpool->queue_head != NULL )
   {
      cur_nodep = tpool->queue_head->next;
      tpool->queue_head = tpool->queue_head->next;
      eh_free(cur_nodep);
   }
   eh_free(tpool);
   return 0 ;
}

/** Create a list of successive file name.

Create a list of files that all have the same form but contain a numerical
portion that is incremented.  For instance, a list of files might be something
like: file0001.txt, file0002.txt, etc.

The input is a template from which the file of the list will be made.  The
template must contain one '#' which indicates the position of the increment
within the file names.  The '#' will be replaced by a number with a format
of "%04d".

Use eh_get_next_file to obtain the next file name in the list.

@param base_name The form for the files in the list.

@return A pointer to an Eh_file_list.

@see eh_get_next_file , eh_destroy_file_list .
*/
Eh_file_list *eh_create_file_list( char *base_name )
{
   char **str_array;
   Eh_file_list *list;

   list = eh_new( Eh_file_list , 1 );

   str_array = g_strsplit( base_name , "#" , -1 );
   list->prefix = str_array[0];
   list->suffix = str_array[1];
   list->format = g_strdup("%04d");
   list->count  = 0;

   eh_free( str_array );

   return list;
}

/** Get the next file name from an Eh_file_list.

@param list A pointer to an Eh_file_list.

@return A pointer to string containing the next file name in the list.

@see eh_create_file_list , eh_destroy_file_list .
*/
char *eh_get_next_file( Eh_file_list *list )
{
   char *new_file;
   char *count;

   count = g_strdup_printf( list->format , ++(list->count) );
   new_file = g_strconcat( list->prefix , count , list->suffix , NULL );
   eh_free( count );
   return new_file;
}

/** Destroy an Eh_file_list.

@param list A pointer to an Eh_file_list.

@see eh_create_file_list , eh_get_next_file .
*/
void eh_destroy_file_list( Eh_file_list *list )
{
   eh_free( list->prefix );
   eh_free( list->suffix );
   eh_free( list->format );
   eh_free( list );
}

double eh_get_fuzzy_dbl( double min , double max )
{
   return g_random_double_range( min , max );
}

double eh_get_fuzzy_dbl_norm( double mean , double std )
{
   return eh_rand_normal( NULL , mean , std );
}

double eh_get_fuzzy_dbl_log_norm( double mean , double std )
{
   return eh_log_normal( NULL , mean , std );
}

gint32 eh_get_fuzzy_int( gint32 min , gint32 max )
{
   return g_random_int_range( min , max );
}

/** Generate an array of equally spaced numbers.

Generate an array of \a n linearly equally spaced points between \a x1 and
\a x2.  returns a pointer to a newly allocated array.  should be freed
using eh_free.

\param x1 Lower bound
\param x2 Lower bound
\param n  Number of points

\return An array of equally spaced points

\see eh_logspace

*/
double *eh_linspace( double x1 , double x2 , gssize n )
{
   double *x = NULL;

   eh_require( !eh_compare_dbl(x1,x2,1e-12) );
   eh_require( n>1 );

   {
      gssize i;
      double dx   = fabs(x2-x1)/(n-1.);
      gssize sign = (x2>x1)?1:-1;

      x = eh_new( double , n );
   
      for ( i=1 , x[0]=x1 ; i<n ; i++ )
         x[i] = x[i-1] + sign*dx;
   }

   return x;
}

gssize*
eh_id_array( gssize i_0 , gssize i_1 , gssize* n )
{
   gssize* id = NULL;

   eh_require( i_1>=i_0 );

   if ( i_1>=i_0 )
   {
      gint i;
      gint len = i_1 - i_0 + 1;

      id = eh_new( gssize , len+1 );

      for ( i=0 ; i<len ; i++ )
         id[i] = i_0+i;
      id[len] = -1;

      if ( n )
         *n = len;
   }
   else
   {
      if ( n )
         *n = 0;
   }

   return id;
}

double *eh_uniform_array( double x1 , double x2 , double dx , gssize* n )
{
   double* x = NULL;

   eh_require( n     );
   eh_require( x2>x1 );
   eh_require( dx>0  );

   *n = ( x2-x1 )/dx;

   if ( *n>0 )
   {
      gssize i, len=*n;

      x = eh_new( double , len );
      for ( i=1,x[0]=x1 ; i<len ; i++ )
         x[i] = x[i-1] + dx;

   }
   else
      *n = 0;

   return x;
}

double* eh_dbl_array_linspace( double* x , gssize n_x ,  double x_0 , double dx )
{
   eh_require( x      );
   eh_require( n_x>=0 );

   eh_return_val_if_fail( x && n_x>=0 , NULL )
   {
      gssize i;
      for ( i=0 ; i<n_x ; i++ )
         x[i] = i*dx + x_0;
      return x;
   }
}

/** Check if an array is monotonic (either increasing of decreasing)

\param x A pointer to an array of doubles
\param len The length of the array

\return TRUE if the array is either monotonically increasing or decreasing.
*/
gboolean eh_dbl_array_is_monotonic( double* x , gssize len )
{
   gssize i;

   eh_require( x );
   eh_require( len>0 );

   if ( len==1 )
      return TRUE;
   else if ( x[1]>x[0] )
   {
      for ( i=2 ; i<len ; i++ )
         if ( x[i]<=x[i-1]  )
            return FALSE;
   }
   else if ( x[1]<x[0] )
   {
      for ( i=2 ; i<len ; i++ )
         if ( x[i]<=x[i-1]  )
            return FALSE;
   }
   else
      return FALSE;

   return TRUE;
}

/** Generate an array of equally spaced numbers.

Generate a row vector of n logarithmically equally spaced points between
decades 10^d1 and 10^d2.  returns a pointer to a newly allocated array.
should be freed using eh_free.

\param d1 Lower bound
\param d2 Lower bound
\param n  Number of points

\return An array of equally spaced points

\see eh_linspace
*/
double *eh_logspace( double d1 , double d2 , int n )
{
   int i;
   double *x = eh_linspace( d1 , d2 , n );
   for ( i=0 ; i<n ; i++ )
      x[i] = pow( 10. , x[i] );
   return x;
}

void eh_print_msg( int msg_level , char *function_name , char *msg )
{

   g_strchomp( msg );

   switch ( msg_level )
   {
      case 0:
         break;
      case 1:
         fprintf( stderr , "%s: %s: %s\n" , "warning" , function_name , msg );
         break;
      case 2:
         fprintf( stderr , "%s: %s: %s\n" , "error" , function_name , msg );
         fprintf( stderr , "quitting...\n" );
         eh_exit(-1);
   }

}

#if defined( OLD_NDGRID )

Eh_ndgrid *eh_malloc_ndgrid( gssize n_dim , gssize el_size , ... )
{
   gssize n;
   gssize n_el=1;
   Eh_ndgrid *g;
   va_list args;

   g       = eh_new( Eh_ndgrid , 1 );
   g->size = eh_new( gssize  , n_dim );
   g->low  = eh_new( gssize  , n_dim );
   g->x    = eh_new( double* , n_dim );

   g->n_dim = n_dim;

   va_start( args , el_size );
   for ( n=0 ; n<n_dim ; n++ )
   {
      g->size[n] = va_arg( args , gssize );
      g->low[n]  = 0;
      g->x[n]    = eh_new( double , g->size[n] );
      n_el      *= g->size[n];
   }
   va_end( args );

//   g->data = (double*)g_malloc0( n_el*el_size );
//   g->data = (double*)eh_malloc( n_el*el_size , NULL , __FILE__ , __LINE__ );
   g->data = (double*)eh_new( gchar , n_el*el_size );

   return g;
}

void eh_free_ndgrid_data( Eh_ndgrid *g )
{
   gssize n;
   if ( g )
   {
      for ( n=0 ; n<g->n_dim ; n++ )
         eh_free( g->x[n] );
      eh_free( g->x    );
      eh_free( g->low  );
      eh_free( g->size );
      eh_free( g->data );
   }
}

double eh_ndgrid_ind( Eh_ndgrid *g , ... )
{
   gssize n;
   gssize id;
   gssize *sub;
   va_list args;

   sub = eh_new( gssize , g->n_dim );

   va_start( args , g );
   for ( n=0 ; n<g->n_dim ; n++ )
      sub[n] = va_arg( args , gssize );
   va_end( args );

   id = eh_ndgrid_sub_to_id( g->size , sub , g->n_dim );

   eh_free( sub );

   return g->data[id];
}

Eh_ndgrid *eh_reshape_ndgrid( Eh_ndgrid *g , gssize *new_size , gssize new_n_dim )
{
   gssize k_old, k_new;
   gssize n;

   for ( n=0 ; n<g->n_dim ; n++ )
   {
      k_old *= g->size[n];
      k_new *= new_size[n];
   }
   eh_require( k_old == k_new );

   if ( new_n_dim > g->n_dim )
   {
      g->x = g_renew( double* , g->x , new_n_dim );
      for ( n=0 ; n<g->n_dim ; n++ )
         g->x[n] = g_renew( double , g->x[n] , new_size[n] );
      for ( n=g->n_dim ; n<new_n_dim ; n++ )
         g->x[n] = eh_new( double , new_size[n] );
   }
   else
   {
      for ( n=0 ; n<new_n_dim ; n++ )
         g->x[n] = g_renew( double , g->x[n] , new_size[n] );
      for ( n=new_n_dim ; n<g->n_dim ; n++ )
         eh_free( g->x[n] );
      g->x = g_renew( double* , g->x , new_n_dim );
   }
   
   g->n_dim = new_n_dim;
   g->size  = g_renew( gssize , g->size  , new_n_dim );
   for ( n=0 ; n<g->n_dim ; n++ )
      g->size[n] = new_size[n];

   return g;
}

gssize eh_ndgrid_sub_to_id( gssize *size , gssize *sub , gssize n_dim  )
{
   gssize n;
   gssize *k;
   gssize id;

   k = eh_new( gssize , n_dim );
   for ( n=1,k[0]=1 ; n<n_dim ; n++ )
      k[n] = k[n-1]*size[n-1];

   for ( n=0 ; n<n_dim ; n++ )
      id += sub[n]*k[n];

   eh_free( k );

   return id;
}

gssize *eh_ndgrid_id_to_sub( gssize *size , gssize id , gssize n_dim )
{
   gssize n;
   gssize *k;
   gssize *sub;

   sub = eh_new( gssize , n_dim );
   k   = eh_new( gssize , n_dim );

   for ( n=1,k[0]=1 ; n<n_dim ; n++ )
      k[n] = k[n-1]*size[n-1];

   for ( n=n_dim-1 ; n>=0 ; n-- )
   {
      sub[n] = id/k[n];
      id     = id%k[n];
   }

   eh_free( k );

   return sub;
}

void eh_destroy_ndgrid( Eh_ndgrid *g )
{
   eh_free_ndgrid_data( g );
   eh_free( g );
}

Eh_dbl_grid *eh_ndgrid_to_grid( Eh_ndgrid *g )
{
   Eh_dbl_grid *dest;
   gssize i, n, n_x, n_y;

   // find the first non-singleton dimension
   for ( n=0 ; n<g->n_dim && g->size[n]==1 ; n++ );

   if ( n!=g->n_dim )
   {
//      n_x = g->size[n];
//      for ( n+=1,n_y=1 ; n<g->n_dim ; n++ )
//         n_y *= g->size[n];

      n_y = g->size[g->n_dim-1];
      for ( n_x=1 ; n<g->n_dim-1 ; n++ )
         n_x *= g->size[n];
   }
   else
   {
      n_x = 1;
      n_y = 1;
   }

//   dest = eh_create_grid( n_x , n_y , g->el_size );

   dest          = eh_new( Eh_dbl_grid , 1 );
   dest->x       = eh_new( double , n_x );
   dest->y       = eh_new( double , n_y );
   dest->el_size = g->el_size;
   dest->low_x   = 0;
   dest->low_y   = 0;
   dest->n_x     = n_x;
   dest->n_y     = n_y;

   dest->data    = eh_new( double* , n_x );
   dest->data[0] = g->data;
   for ( i=1 ; i<n_x ; i++ )
      dest->data[i] = dest->data[i-1]+n_y;

   memcpy( dest->x , g->x[0] , sizeof(double)*g->size[0] );

   return dest;
}

Eh_ndgrid *eh_grid_to_ndgrid( Eh_grid *g )
{
   Eh_ndgrid *dest;

   dest = eh_new( Eh_ndgrid , 1 );

   dest->n_dim = 2;

   dest->x    = eh_new( double* , 2 );
   dest->x[0] = eh_new( double , g->n_x );
   dest->x[1] = eh_new( double , g->n_y );
   memcpy( dest->x[0] , g->x , sizeof(double)*g->n_x );
   memcpy( dest->x[1] , g->y , sizeof(double)*g->n_y );

   dest->data = g->data[0];

   dest->size    = eh_new( gssize , 2 );
   dest->size[0] = g->n_x;
   dest->size[1] = g->n_y;
   
   return dest;
}

Eh_grid *eh_malloc_grid( gssize n_x , gssize n_y , gssize size )
{
   gssize i;
   Eh_grid *g = eh_new( Eh_grid , 1 );
   
   eh_return_if_fail( n_x>0  , NULL );
   eh_return_if_fail( n_y>0  , NULL );
   eh_return_if_fail( size>0 , NULL );

   g->data    = eh_new( void* , n_x );
//   g->data[0] = eh_malloc( n_x*n_y*size , NULL , __FILE__ , __LINE__ );
   g->data[0] = eh_new( gchar , n_x*n_y*size );

   g->x = eh_new( double , n_x );
   g->y = eh_new( double , n_y );

   for ( i=1 ; i<n_x ; i++ )
     g->data[i] = (gint8*)(g->data[i-1]) + n_y*size;

   g->low_x = 0;
   g->low_y = 0;

   g->n_x = n_x;
   g->n_y = n_y;

   g->el_size = size;

   return g;
}

void eh_free_grid_data( Eh_grid *g , gboolean free_data )
{
   if ( g )
   {
      if ( free_data )
      {
         eh_reindex_grid( g , 0 , 0 );
         eh_free( g->data[0] );
      }
      eh_free( g->data    );
      eh_free( g->x       );
      eh_free( g->y       );
   }
}

Eh_grid *eh_destroy_grid( Eh_grid *g , gboolean free_data )
{
   eh_free_grid_data( g , free_data );
   eh_free( g );
   return NULL;
}

void eh_dump_grid( FILE *fp , Eh_grid *g )
{
   fwrite( &(g->n_x)     , sizeof(gssize) , 1             , fp );
   fwrite( &(g->n_y)     , sizeof(gssize) , 1             , fp );
   fwrite( &(g->el_size) , sizeof(gssize) , 1             , fp );
   fwrite( &(g->low_x)   , sizeof(gssize) , 1             , fp );
   fwrite( &(g->low_y)   , sizeof(gssize) , 1             , fp );
   fwrite( g->x          , sizeof(double) , g->n_x        , fp );
   fwrite( g->y          , sizeof(double) , g->n_y        , fp );
   fwrite( g->data[0]    , g->el_size     , g->n_x*g->n_y , fp );
}

Eh_grid *eh_load_grid( FILE *fp )
{
   gssize n_x, n_y, el_size;
   gssize low_x, low_y;
   Eh_grid *g;

   fread( &n_x       , sizeof(gssize) , 1       , fp );
   fread( &n_y       , sizeof(gssize) , 1       , fp );
   fread( &el_size   , sizeof(gssize) , 1       , fp );

   g = eh_malloc_grid( n_x , n_y , el_size );

   fread( &low_x     , sizeof(gssize) , 1       , fp );
   fread( &low_y     , sizeof(gssize) , 1       , fp );

   fread( g->x       , el_size        , n_x     , fp );
   fread( g->y       , el_size        , n_y     , fp );
   fread( g->data[0] , el_size        , n_x*n_y , fp );

   eh_reindex_grid( g , low_x , low_y );

   return g;
}

gboolean eh_cmp_grid_data( Eh_grid *g_1 , Eh_grid *g_2 )
{
   gboolean is_same = FALSE;

   if ( !eh_is_grid_same_size( g_1 , g_2 ) )
      is_same = FALSE;
   else
   {
      gssize n_bytes = g_1->n_x*g_1->n_y*g_1->el_size;
      is_same = (memcmp( g_1->data[0] , g_2->data[0] , n_bytes )==0)?TRUE:FALSE;
   }

   return is_same;
}

gboolean eh_cmp_grid_x_data( Eh_grid *g_1 , Eh_grid *g_2 )
{
   gboolean is_same = FALSE;

   if ( g_1->n_x != g_2->n_x )
      is_same = FALSE;
   else
      is_same = (memcmp( g_1->x , g_2->x , g_1->n_x )==0)?TRUE:FALSE;

   return is_same;
}

gboolean eh_cmp_grid_y_data( Eh_grid *g_1 , Eh_grid *g_2 )
{
   gboolean is_same = FALSE;

   if ( g_1->n_y != g_2->n_y )
      is_same = FALSE;
   else
      is_same = (memcmp( g_1->y , g_2->y , g_1->n_y )==0)?TRUE:FALSE;

   return is_same;
}

gboolean eh_cmp_grid( Eh_grid *g_1 , Eh_grid *g_2 )
{
   gboolean is_same = FALSE;

   if ( !eh_is_grid_same_size( g_1 , g_2 ) )
      is_same = FALSE;
   else
   {
      is_same =    eh_cmp_grid_data( g_1 , g_2 )
                && eh_cmp_grid_x_data( g_1 , g_2 )
                && eh_cmp_grid_y_data( g_1 , g_2 );
   }

   return is_same;
}

Eh_grid *eh_dup_grid( Eh_grid *g )
{
   Eh_grid *new_grid = eh_malloc_grid( g->n_x , g->n_y , g->el_size );
   eh_copy_grid( new_grid , g );

   return new_grid;
}

Eh_grid *eh_copy_grid( Eh_grid *dest , Eh_grid *src )
{
   gssize low_x = src->low_x;
   gssize low_y = src->low_y;

   eh_require( src->n_x==dest->n_x         );
   eh_require( src->n_y==dest->n_y         );
   eh_require( src->el_size==dest->el_size );

   eh_reindex_grid( src , 0 , 0 );

   memcpy( dest->data[0] , src->data[0] , src->n_x*src->n_y*src->el_size );
   memcpy( dest->x       , src->x       , src->n_x*sizeof(double)        );
   memcpy( dest->y       , src->y       , src->n_y*sizeof(double)        );

   eh_reindex_grid( src  , low_x , low_y );
   eh_reindex_grid( dest , low_x , low_y );

   return dest;
}

Eh_grid *eh_copy_grid_data( Eh_grid *dest , Eh_grid *src )
{
   gssize low_x = src->low_x;
   gssize low_y = src->low_y;

   eh_require( src->n_x==dest->n_x         );
   eh_require( src->n_y==dest->n_y         );
   eh_require( src->el_size==dest->el_size );

   eh_reindex_grid( src , 0 , 0 );

   memcpy( dest->data[0] , src->data[0] , src->n_x*src->n_y*src->el_size );

   eh_reindex_grid( src  , low_x , low_y );
   eh_reindex_grid( dest , low_x , low_y );

   return dest;
}

Eh_grid *eh_reindex_grid( Eh_grid *g , gssize low_x , gssize low_y )
{
   gssize i;
   gssize change_low_x = low_x - g->low_x;
   gssize change_low_y = low_y - g->low_y;

   if ( change_low_x == 0 && change_low_y == 0 )
      return g;

   for ( i=g->low_x ; i<g->n_x+g->low_x ; i++ )
      g->data[i] = (gint8*)(g->data[i]) - change_low_y*g->el_size;
   g->data -= change_low_x;

   g->x -= change_low_x;
   g->y -= change_low_y;

   g->low_x = low_x;
   g->low_y = low_y;

   return g;
}

gboolean is_in_grid_domain( Eh_grid *g , gssize i , gssize j )
{
   return i>=g->low_x && j>=g->low_y && i<g->n_x+g->low_x && j<g->n_y+g->low_y;
}

gboolean eh_is_grid_same_size( Eh_grid *g_1 , Eh_grid *g_2 )
{
   return    g_1->n_x     == g_2->n_x
          && g_1->n_y     == g_2->n_y
          && g_1->el_size == g_2->el_size;
}

Eh_grid_id eh_grid_sub_to_id( gssize n_j , gssize i , gssize j )
{
   return i*n_j + j;
}

Eh_ind_2 eh_grid_id_to_sub( gssize n_i , Eh_grid_id id )
{
   Eh_ind_2 sub;

   sub.i = id/n_i;
   sub.j = id%n_i;

   return sub;
}

Eh_dbl_grid *eh_create_dbl_grid( gssize n_x , gssize n_y )
{
   Eh_dbl_grid *g = (Eh_dbl_grid*)eh_create_grid( double , n_x , n_y );
   return g;
}

gboolean eh_cmp_dbl_grid( Eh_dbl_grid *g_1 , Eh_dbl_grid *g_2 , double eps )
{
   gboolean is_same;

   if ( !eh_is_grid_same_size( (Eh_grid*)g_1 , (Eh_grid*)g_2 ) )
      is_same = FALSE;
   else if ( eps<0 )
      is_same = eh_cmp_grid( (Eh_grid*)g_1 , (Eh_grid*)g_2 );
   else
   {
      gssize i, n_elem = g_1->n_x*g_1->n_y;
      for ( i=0,is_same=TRUE ; i<n_elem && is_same ; i++ )
         if ( fabs( g_1->data[0][i] - g_2->data[0][i] ) > eps )
            is_same = FALSE;
   }
   return is_same;
}

Eh_dbl_grid *eh_dup_dbl_grid( Eh_dbl_grid *g )
{
   return (Eh_dbl_grid*)eh_dup_grid( (Eh_grid*)g );
}

Eh_dbl_grid *eh_copy_dbl_grid( Eh_dbl_grid *dest , Eh_dbl_grid *src )
{
   return (Eh_dbl_grid*)eh_copy_grid( (Eh_grid*)dest , (Eh_grid*)src );
}

Eh_int_grid *eh_create_int_grid( gssize n_x , gssize n_y )
{
   Eh_int_grid *g = (Eh_int_grid*)eh_create_grid( int , n_x , n_y );
   return g;
}

int eh_write_ndgrid( FILE *fp , Eh_ndgrid *g )
{
   gssize n, n_x;
//   for ( n=0,n_x=0 ; n<g->n_dim ; n++ )
//      n_x += g->size[n];
   for ( n=0,n_x=1 ; n<g->n_dim ; n++ )
      n_x *= g->size[n];
   return eh_write_dbl_array( fp , g->data , n_x );
}

#endif /* OLD_NDGRID */

int eh_write_dbl_array( FILE *fp , double *x , gssize len )
{
   size_t s=0;
   int n_i = len;
   int n, i, i_0;
   int el_size = sizeof(double);
   int one = 1, size;
   double this_val;

   for ( i_0=0 ; i_0<n_i ; i_0+=n )
   {
      if ( i_0==n_i-1 || x[i_0] == x[i_0+1] )
      {
         this_val = x[i_0];

         for ( i=i_0,n=0 ;
               i<n_i && x[i]==this_val ;
               i++,n++ );

         s += fwrite( &el_size  , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &n        , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &this_val , el_size     , 1 , fp )*el_size;
      }
      else
      {
         for ( i=i_0+1,n=1 ;
               i<n_i && x[i-1]!=x[i] ;
               i++,n++ );

         if ( i<n_i )
            n--;

         size = n*el_size;

         s += fwrite( &size      , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &one       , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &(x[i_0])  , size        , 1 , fp )*size;
      }
   }

   return s;
}

#if defined( OLD_NDGRID )

int eh_write_dbl_grid( FILE *fp , Eh_dbl_grid *g )
{
   size_t s=0;
   int n_i = g->n_x*g->n_y;
   int n, i, i_0;
   int el_size = g->el_size;
   int one = 1, size;
   double this_val;

   for ( i_0=0 ; i_0<n_i ; i_0+=n )
   {
      if ( i_0==n_i-1 || g->data[0][i_0] == g->data[0][i_0+1] )
      {
         this_val = g->data[0][i_0];

         for ( i=i_0,n=0 ; i<n_i && g->data[0][i]==this_val ; i++,n++ );

         s += fwrite( &el_size  , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &n        , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &this_val , el_size     , 1 , fp )*el_size;
      }
      else
      {
         for ( i=i_0+1,n=1 ; i<n_i && g->data[0][i-1]!=g->data[0][i] ; i++,n++ );

         if ( i<n_i )
            n--;

         size = n*el_size;

         s += fwrite( &size               , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &one                , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &(g->data[0][i_0])  , size        , 1 , fp )*size;
      }
   }

   return s;
}

double **eh_dbl_grid_data( Eh_dbl_grid *g )
{
   return g->data;
}

Eh_dbl_grid *eh_set_dbl_grid( Eh_dbl_grid *g , double val )
{
   int i, j;
   for ( i=g->low_x ; i<g->n_x+g->low_x ; i++ )
      for ( j=g->low_y ; j<g->n_y+g->low_y ; j++ )
         g->data[i][j] = val;
   return g;
}

Eh_dbl_grid *eh_randomize_dbl_grid( Eh_dbl_grid *g )
{
   int i, j;
   for ( i=g->low_x ; i<g->n_x+g->low_x ; i++ )
      for ( j=g->low_y ; j<g->n_y+g->low_y ; j++ )
         g->data[i][j] = g_random_double();
   return g;
}

gboolean is_compatible_grid( Eh_dbl_grid *g_1 , Eh_dbl_grid *g_2 )
{
   gboolean ans=FALSE;

   if (    g_1
        && g_2
        && g_1->n_x == g_2->n_x
        && g_1->n_y == g_2->n_y )
      ans = TRUE;

   return ans;
}

Eh_dbl_grid* eh_add_dbl_grid( Eh_dbl_grid *g_1 , Eh_dbl_grid *g_2 )
{
   gssize i, j;
   gssize low_i = g_1->low_x;
   gssize low_j = g_1->low_y;
   gssize up_i  = g_1->n_x + g_1->low_x;
   gssize up_j  = g_1->n_y + g_1->low_y;

   eh_require( is_compatible_grid( g_1 , g_2 ) );

   for ( i=low_i ; i<up_i ; i++ )
      for ( j=low_j ; j<up_j ; j++ )
         g_1->data[i][j] += g_2->data[i][j];

   return g_1;
}

double eh_sum_dbl_grid( Eh_dbl_grid *g )
{
   return eh_sum_dbl_grid_bad_val( g , eh_nan() );
}

double eh_sum_dbl_grid_bad_val( Eh_dbl_grid *g , double bad_val )
{
   gssize i, j;
   double sum=0;
   gssize low_x = g->low_x;
   gssize low_y = g->low_y;
   gssize high_x = g->low_x + g->n_x;
   gssize high_y = g->low_y + g->n_y;

   eh_require( g!=NULL );

   if ( eh_isnan( bad_val ) )
   {
      for ( i=low_x ; i<high_x ; i++ )
         for ( j=low_y ; j<high_y ; j++ )
            if ( !eh_isnan( g->data[i][j] ) )
               sum += g->data[i][j];
   }
   else
   {
      for ( i=low_x ; i<high_x ; i++ )
         for ( j=low_y ; j<high_y ; j++ )
            if ( fabs( g->data[i][j] - bad_val )>1e-12 )
               sum += g->data[i][j];
   }

   return sum;
}

void eh_scalar_mult_dbl_grid( Eh_dbl_grid *g , double scalar )
{
   gssize i, j;

   eh_require( g!=NULL );

   for ( i=g->low_x ; i<g->n_x+g->low_x ; i++ )
      for ( j=g->low_y ; j<g->n_y+g->low_y ; j++ )
         g->data[i][j] *= scalar;
}

void eh_rotate_dbl_grid( Eh_dbl_grid *g , double angle , gssize i_0 , gssize j_0 )
{
   gssize i, j;
   gssize i_rotate, j_rotate;
   double r, alpha, new_angle;
   double d_i, d_j;
   Eh_dbl_grid *temp = eh_create_dbl_grid( g->n_x , g->n_y );

   eh_reindex_grid( (Eh_grid*)temp , g->low_x , g->low_y );

   if ( angle==0 )
   {
      eh_destroy_grid( (Eh_grid*)temp , TRUE );
      return;
   }

   for ( i=g->low_x ; i<g->n_x+g->low_x ; i++ )
      for ( j=g->low_y ; j<g->n_y+g->low_y ; j++ )
      {
         if ( g->data[i][j] != 0 )
         {
            d_i       = i-i_0;
            d_j       = j-j_0;

            r         = sqrt( pow(d_i,2.) + pow(d_j,2.) );
            alpha     = atan2( d_j , d_i );
            new_angle = alpha + angle;

            i_rotate = eh_round( r*cos( new_angle ) , 1 )+i_0;
            j_rotate = eh_round( r*sin( new_angle ) , 1 )+j_0;

//            if ( is_in_domain( temp->n_x , temp->n_y , i_rotate , j_rotate ) )
            if ( is_in_grid_domain( (Eh_grid*)temp , i_rotate , j_rotate ) )
               temp->data[i_rotate][j_rotate] = g->data[i][j];
         }
      }

   eh_copy_grid_data( (Eh_grid*)g , (Eh_grid*)temp );
   eh_destroy_grid( (Eh_grid*)temp , TRUE );
}

Eh_dbl_grid *eh_reduce_dbl_grid( Eh_dbl_grid *g ,
                                 gssize new_n_x , gssize new_n_y )
{
   eh_require( new_n_x<=g->n_x );
   eh_require( new_n_y<=g->n_y );

   return eh_remesh_dbl_grid( g , new_n_x , new_n_y );
}

Eh_dbl_grid *eh_expand_dbl_grid( Eh_dbl_grid *g ,
                                 gssize new_n_x , gssize new_n_y )
{
   eh_require( new_n_x>=g->n_x );
   eh_require( new_n_y>=g->n_y );

   return eh_remesh_dbl_grid( g , new_n_x , new_n_y );
}

Eh_dbl_grid *eh_remesh_dbl_grid( Eh_dbl_grid *g ,
                                 gssize new_n_x , gssize new_n_y )
{
   gssize i, j;
   gssize cur_n_x, cur_n_y;
   Eh_dbl_grid *new_grid;
   double *x_ind, *new_x_ind;
   double *y_ind, *new_y_ind;
   double *orig_x, *orig_y;
   double dx, dy;

   eh_require( g           );

   if ( new_n_x == g->n_x && new_n_y == g->n_y )
      return eh_dup_dbl_grid( g );

   eh_require( g->n_x>=2   );
   eh_require( g->n_y>=2   );
   eh_require( new_n_x > 1 );
   eh_require( new_n_y > 1 );

   cur_n_x = g->n_x;
   cur_n_y = g->n_y;

   if ( new_n_x == cur_n_x && new_n_y == cur_n_y )
      return eh_dup_dbl_grid( g );

   new_grid = eh_create_dbl_grid( new_n_x , new_n_y );
   eh_reindex_grid( (Eh_grid*)new_grid , g->low_x , g->low_y );

   dx = (cur_n_x-1.) / (double)(new_n_x-1.);
   dy = (cur_n_y-1.) / (double)(new_n_y-1.);

   x_ind     = eh_new( double , g->n_x        );
   new_x_ind = eh_new( double , new_grid->n_x );
   y_ind     = eh_new( double , g->n_y        );
   new_y_ind = eh_new( double , new_grid->n_y );

   for ( i=0 ; i<g->n_x          ; i++ ) x_ind[i]     = i+g->low_x;
   for ( j=0 ; j<g->n_y          ; j++ ) y_ind[j]     = j+g->low_y;

   for ( i=0 ; i<new_grid->n_x ; i++ ) new_x_ind[i] = i*dx+new_grid->low_x;
   for ( j=0 ; j<new_grid->n_y ; j++ ) new_y_ind[j] = j*dy+new_grid->low_y;

   if ( new_x_ind[new_grid->n_x-1] > x_ind[g->n_x-1] )
      new_x_ind[new_grid->n_x-1] = (1.-1e-6)*x_ind[g->n_x-1];
   if ( new_y_ind[new_grid->n_y-1] > y_ind[g->n_y-1] )
      new_y_ind[new_grid->n_y-1] = (1.-1e-6)*y_ind[g->n_y-1];

   interpolate( x_ind         , x_ind       , g->n_x ,
                new_x_ind     , new_grid->x , new_grid->n_x );
   interpolate( y_ind         , y_ind       , g->n_y ,
                new_y_ind     , new_grid->y , new_grid->n_y );

   orig_x = g->x;
   orig_y = g->y;

   g->x = x_ind;
   g->y = y_ind;

   interpolate_2( g , new_grid );

   g->x = orig_x;
   g->y = orig_y;

   eh_free( x_ind     );
   eh_free( y_ind     );
   eh_free( new_x_ind );
   eh_free( new_y_ind );

   return new_grid;
}

Eh_grid *sed_subgrid( Eh_grid *g , int i_0 , int j_0 , int n_x , int n_y )
{
   gssize i;
   Eh_grid *sub = eh_new( Eh_grid , 1 );

   if ( i_0+n_x > g->n_x )
      n_x = g->n_x-i_0;
   if ( j_0+n_y > g->n_y )
      n_y = g->n_y-j_0;

   sub->n_x = n_x;
   sub->n_y = n_y;
   sub->low_x = 0;
   sub->low_y = 0;

   sub->data = eh_new( void* , sub->n_x );

   for ( i=0 ; i<sub->n_x ; i++ )
      sub->data[i] = (gint8*)(g->data[i_0])   + j_0*g->el_size;
//      sub->data[i] = (gint8*)(g->data[i_0+1]) + j_0*g->el_size;

   if ( g->x )
      sub->x = g->x+i_0;
   else
      sub->x = NULL;

   if ( g->y )
      sub->y = g->y+j_0;
   else
      sub->y = NULL;

   return sub;
}

#endif /* OLD_NDGRID */

/** Calculate the density of sea water

Calculate the density of sea water for a given salinity, temperature, and
pressure.  Use the international unesco equation of state 1980.

@param p Water pressure in bars.  This is water depth (in meters) times .1.
@param s Salinity of water in promille.
@param t Water temperature in degrees centigrade.

@return Water density in kg/m^3.

*/

double sigma( double s , double t , double p )
{
   double ans;
   double t_2 = pow( t , 2 );
   double t_3 = t_2*t;
   double t_4 = t_3*t;
   double t_5 = t_4*t;
   double s_2 = pow( s , 2 );
   double s_3 = s_2*s;
   double s_1 = sqrt( s_3 );
   double p_2 = pow( p , 2 );
   double rho, sbmk;

   rho = 6.793952e-2*t   - 9.095290e-3*t_2 + 1.001685e-4*t_3
       - 1.120083e-6*t_4 + 6.536332e-9*t_5
       + s   * (   8.24493e-1  - 4.0899e-3*t + 7.6438e-5*t_2
                 - 8.2467e-7*t_3  + 5.3875e-9*t_4 )
       + s_1 * ( - 5.72466e-3  + 1.0227e-4*t - 1.6546e-6*t_2 )
       + 4.8314e-4*s_2;

   sbmk =  19652.21
        + 148.4206*t - 2.327105*t_2   + 1.360477e-2*t_3 - 5.155288e-5*t_4
        + p     * (   3.239908    + 1.43713e-3*t  + 1.16092e-4*t_2
                    - 5.77905e-7*t_3)
        + p_2   * ( 8.50935e-5 - 6.12293e-6*t + 5.2787e-8*t_2 )
        + s     * ( 54.6746    - 0.603459*t   + 1.09987e-2*t_2 - 6.1670e-5*t_3 )
        + s_1   * ( 7.944e-2   + 1.6483e-2*t  - 5.3009e-4*t_2  + 1.91075e-4*p )
        + p*s   * ( 2.2838e-3  - 1.0981e-5*t  - 1.6078e-6*t_2 )
        + p_2*s * ( -9.9348e-7 + 2.0816e-8*t  + 9.1697e-10*t_2 );

   ans = 1.e-3*( rho + 999.842594*p/sbmk )/( 1.0-p/sbmk );

   return ans*1e3+1000;
}

#if defined( OLD_NDGRID )
Eh_dbl_grid *eh_populate_domain( Eh_dbl_grid *g  ,
                                 Populate_func f ,
                                 gpointer user_data )
{
   gssize n;
   gssize population_size=100000;
   gssize low_x  = g->low_x;
   gssize low_y  = g->low_y;
   gssize high_x = g->low_x+g->n_x;
   gssize high_y = g->low_y+g->n_y;
   double x, y;

   for ( n=0 ; n<population_size ; n++ )
   {
      x = eh_get_fuzzy_dbl( low_x , high_x );
      y = eh_get_fuzzy_dbl( low_y , high_y );
      if ( (*f)( x , y , user_data ) )
         g->data[(gssize)floor(x)][(gssize)floor(y)]++;
   }

   eh_scalar_mult_dbl_grid( g , ((double)(g->n_x*g->n_y))/population_size );

   return g;
}
#endif

Eh_polygon_2 eh_get_rectangle_polygon( Eh_pt_2 center , double dx , double dy )
{
   int n;
   Eh_polygon_2 poly=NULL;
   Eh_pt_2 *this_corner;
   gssize x_offset[4] = { -1 , +1 , +1 , -1 };
   gssize y_offset[4] = { -1 , -1 , +1 , +1 };

   for ( n=0 ; n<4 ; n++ )
   {
      this_corner = eh_new( Eh_pt_2 , 1 );

      this_corner->x = center.x + x_offset[n]*dx/2.;
      this_corner->y = center.y + y_offset[n]*dy/2.;

      poly = g_list_append( poly , this_corner );
   }
   return poly;
}

#if defined( OLD_NDGRID )
Eh_polygon_2 eh_get_polygon_from_grid( Eh_grid *g , gssize i , gssize j )
{
   int n;
   Eh_polygon_2 poly=NULL;
   Eh_pt_2 *this_corner;
   gssize x_offset[4] = { -1 , -1 , +1 , +1 };
   gssize y_offset[4] = { -1 , +1 , +1 , -1 };

   for ( n=0 ; n<4 ; n++ )
   {
      this_corner = eh_new( Eh_pt_2 , 1 );

      this_corner->x = ( g->x[x_offset[n]] + g->x[i] ) / 2.;
      this_corner->y = ( g->y[y_offset[n]] + g->y[j] ) / 2.;

      poly = g_list_append( poly , this_corner );
   }
   return poly;
}
#else
Eh_polygon_2 eh_get_polygon_from_grid( Eh_grid g , gssize i , gssize j )
{
   int n;
   Eh_polygon_2 poly=NULL;
   Eh_pt_2 *this_corner;
   gssize x_offset[4] = { -1 , -1 , +1 , +1 };
   gssize y_offset[4] = { -1 , +1 , +1 , -1 };

   for ( n=0 ; n<4 ; n++ )
   {
      this_corner = eh_new( Eh_pt_2 , 1 );

      this_corner->x = ( eh_grid_x(g)[x_offset[n]] + eh_grid_x(g)[i] )/ 2.;
      this_corner->y = ( eh_grid_y(g)[y_offset[n]] + eh_grid_y(g)[j] )/ 2.;

      poly = g_list_append( poly , this_corner );
   }
   return poly;
}
#endif

void eh_destroy_polygon( Eh_polygon_2 p )
{
   GList *this_link;
   for ( this_link=p ; this_link ; this_link=this_link->next )
      eh_free( this_link->data );
   g_list_free( p );
}

GList *eh_find_polygon_crossings( Eh_pt_2 start ,
                                  double angle  ,
                                  Eh_polygon_2 area ,
                                  int in_or_out )
{
   GList *crossing=NULL, *in=NULL, *out=NULL;
   GList *this_link;
   Eh_pt_2 this_corner;
   Eh_pt_2 next_corner;
   Eh_pt_2 u, v, n;
   Eh_pt_2 *intercept;
   double angle_to_this_corner, angle_to_next_corner;
   double theta;
   double m_0, b_0, m_1, b_1;
   gboolean is_entering;

   m_0 = tan( angle );
   b_0 = start.y - start.x*m_0;

   for ( this_link=area ; this_link ; this_link = this_link->next )
   {
      this_corner = *((Eh_pt_2*)(this_link->data));
      if ( this_link->next )
         next_corner = *((Eh_pt_2*)(this_link->next->data));
      else
         next_corner = *((Eh_pt_2*)(area->data));

      angle_to_this_corner = atan2( this_corner.y - start.y ,
                                    this_corner.x - start.x );
      angle_to_next_corner = atan2(  next_corner.y - start.y ,
                                     next_corner.x - start.x );

      //---
      // u is the vector pointing from this corner to the next. n is its
      // normal.
      //---
      u = eh_get_dir_vector( this_corner , next_corner );
      n = eh_get_norm_vector( u );
      v = eh_get_dir_vector( start , this_corner );

      //---
      // If v is the same direction as the normal vector of this side,
      // then the line is exiting the polygon, otherwise if is entering.
      //---
      theta = eh_get_angle_between_vectors( v , n );
      if ( theta > -M_PI_2 && theta < M_PI_2 )
         is_entering = FALSE;
      else
         is_entering = TRUE;

      if ( is_entering )
         swap_dbl( angle_to_this_corner , angle_to_next_corner );

      //---
      // The ray intercepts this side.
      //---
      if (  (    (  is_entering && (in_or_out & POLYGON_IN_CROSSINGS ) )
              || ( !is_entering && (in_or_out & POLYGON_OUT_CROSSINGS) ) )
           && is_between_angles( angle                ,
                              angle_to_this_corner ,
                              angle_to_next_corner ) )
      {

         intercept = eh_new( Eh_pt_2 , 1 );

         if ( u.x == 0 )
         {
            intercept->x = this_corner.x;
            intercept->y = m_0*intercept->x + b_0;
         }
         else
         {
            //---
            // Calculate where the ray crosses this side.
            //---
            m_1 = ( next_corner.y - this_corner.y )
                / ( next_corner.x - this_corner.x );
            b_1 = this_corner.y - ( this_corner.x * m_1 );

            intercept->x = ( b_1 - b_0 ) / ( m_0 - m_1 );
            intercept->y = m_0*intercept->x + b_0;
         }

         if ( is_entering )
            in  = g_list_append( in  , intercept );
         else
            out = g_list_append( out , intercept );

      }

   }

   if ( in_or_out & POLYGON_IN_CROSSINGS )
      crossing = g_list_concat( crossing , in  );
   if ( in_or_out & POLYGON_OUT_CROSSINGS )
      crossing = g_list_concat( crossing , out );

   return crossing;
}

gboolean is_between_angles( double angle , double angle_1 , double angle_2 )
{
   angle_1 = eh_reduce_angle( angle_1 );
   angle_2 = eh_reduce_angle( angle_2 );

   //---
   // The first angle will be greater than the second angle if the angle between
   // them crosses the negative y-axis.
   //---
   if ( angle_1 > angle_2 )
   {
      if ( angle < angle_2 )
         angle += 2.*M_PI;
      angle_2 += 2.*M_PI;
   }
   if ( angle > angle_1 && angle < angle_2 )
      return TRUE;
   else
      return FALSE;
}

gboolean is_inside_area( Eh_pt_2 x , Eh_polygon_2 area )
{
   GList *crossings;
   GList *this_link;
   Eh_pt_2 *this_corner, *next_corner;
   gboolean is_inside = TRUE;
   guint number_of_crossings;
   Eh_pt_2 u;
   double angle;

   for ( this_link=area ; this_link && is_inside ; this_link = this_link->next )
   {
      this_corner = (Eh_pt_2*)(this_link->data);
      if ( this_link->next )
         next_corner = (Eh_pt_2*)(this_link->next->data);
      else
         next_corner = (Eh_pt_2*)(area->data);

      u = eh_get_dir_vector( *this_corner , *next_corner );

      angle = eh_get_vector_angle( u );
      crossings = eh_find_polygon_crossings( x , angle , area ,
                                               POLYGON_IN_CROSSINGS
                                             | POLYGON_OUT_CROSSINGS );

      number_of_crossings = g_list_length( crossings );

      //---
      // If the number of crossings is even, the point is outside of the
      // polygon.
      //---
      if ( number_of_crossings%2 == 0 )
         is_inside = FALSE;
   }

   return is_inside;
}

Eh_pt_2 eh_get_unit_vector( double angle )
{
   Eh_pt_2 u;
   u.x = cos( angle );
   u.y = sin( angle );
   return u;
}

Eh_pt_2 eh_get_dir_vector( Eh_pt_2 point_1 , Eh_pt_2 point_2 )
{
   Eh_pt_2 u;

   u.x = point_2.x - point_1.x;
   u.y = point_2.y - point_1.y;

   return eh_normalize_vector( u );
}

Eh_pt_2 eh_get_norm_vector( Eh_pt_2 u )
{
   return eh_get_unit_vector( eh_get_vector_angle( u )-M_PI_2 );
}

double eh_get_vector_length( Eh_pt_2 u )
{
   return sqrt( pow( u.x , 2. ) + pow( u.y , 2. ) );
}

double eh_get_vector_angle( Eh_pt_2 u )
{
   return atan2( u.y , u.x );
}

Eh_pt_2 eh_normalize_vector( Eh_pt_2 u )
{
   double r = eh_get_vector_length( u );
   u.x /= r;
   u.y /= r;
   return u;
}

double eh_dot_vectors( Eh_pt_2 u , Eh_pt_2 v )
{
   return u.x*v.x + u.y*v.y;
}

double eh_get_angle_between_vectors( Eh_pt_2 u , Eh_pt_2 v )
{
   return acos(   eh_dot_vectors( u , v )
                / eh_get_vector_length( u )
                / eh_get_vector_length( v ) );
}

void eh_test_function( const char *func_name , Eh_test_func f )
{
   gboolean result;

   fprintf( stdout , "Testing %s ... " , func_name );
   fflush( stdout );

   result = (*f)();

   fprintf( stdout, "\nDONE.\n" );
   if ( result == TRUE )
      fprintf( stdout, "ok.\n" );
   else
      fprintf( stdout , "FAIL.\n" );
   fflush( stdout );

   return;
}

Eh_sequence *eh_create_sequence( void )
{
   Eh_sequence *s = eh_new( Eh_sequence , 1 );
   s->len  = 0;
   s->t    = NULL;
   s->data = NULL;

   return s;
}

Eh_sequence *eh_add_to_sequence( Eh_sequence *s , double t , gpointer data )
{
   eh_require( s!=NULL );

   s->len++;

   s->t    = g_renew( double   , s->t    , s->len );
   s->data = g_renew( gpointer , s->data , s->len );

   s->t[s->len-1] = t;
   s->data[s->len-1] = data;

   return s;
}

void eh_destroy_sequence( Eh_sequence *s , gboolean free_mem )
{
   gssize i;
   if ( s )
   {
      if ( free_mem )
         for ( i=0 ; i<s->len ; i++ )
            eh_free( s->data[i] );
      eh_free( s->data );
      eh_free( s->t    );
      eh_free( s       );
   }
}

char *eh_input_str( char *msg , char *default_str )
{
   char *str = eh_new( char , S_LINEMAX );

   //---
   // Print the message followed by the default string in [].
   //---
   fprintf( stderr , "%s " , msg );
   if ( default_str )
      fprintf( stderr , "[%s] " , default_str );
   fprintf( stderr , ": " );

   //---   
   // Read a line of input.  If the line is blank, use the default string.
   // Remove leading and trailing whitespace from the string.
   //---
   fgets( str , S_LINEMAX , stdin );
   if ( default_str && strncmp( str , "\n" , 1 )==0 )
      strcpy( str , default_str );
   g_strstrip( str );

   return str;
}

gboolean eh_input_boolean( char *msg , gboolean default_val )
{
   char *str = eh_new( char , S_LINEMAX );
   gboolean ans, valid_ans = FALSE;

   while ( !valid_ans )
   {
      fprintf( stderr , "%s [%s]: " , msg , (default_val)?"yes":"no" );
   
      fgets( str , S_LINEMAX , stdin );

      if ( g_ascii_strncasecmp( str , "\n" , 1 ) == 0 )
      {
         ans = default_val;
         valid_ans = TRUE;
      }
      else if (    g_ascii_strncasecmp( str , "YES" , 2 )==0
                || g_ascii_strncasecmp( str , "Y"   , 1 )==0 )
      {
         ans = TRUE;
         valid_ans = TRUE;
      }
      else if (    g_ascii_strncasecmp( str , "NO" , 2 )==0 
                || g_ascii_strncasecmp( str , "N"  , 1 )==0 )
      {  
         ans = FALSE;
         valid_ans = TRUE;
      }
   }

   eh_free( str );

   return ans;
}


gssize eh_pointer_list_length( gpointer* x )
{
   gssize len=0;
   if ( x )
   {
      gssize i;
      for ( len=0 ; x[i] ; len++ );
   }
   return len;
}

#if G_BYTE_ORDER==G_LITTLE_ENDIAN

gssize eh_fread_int32_from_be( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint32) );

   if ( ptr && stream )
   {
      gssize i;
      gint32 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         n += fread( &i_val , sizeof(gint32) , 1 , stream );
         ((gint32*)ptr)[i] = GINT32_TO_BE( i_val );
      }
   }

   return n;
}

gssize eh_fread_int64_from_be( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint64) );

   if ( ptr && stream )
   {
      gssize i;
      gint64 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         n += fread( &i_val , sizeof(gint64) , 1 , stream );
         ((gint64*)ptr)[i] = GINT64_TO_BE( i_val );
      }
   }

   return n;
}

gssize eh_fwrite_int32_to_be( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint32) );

   if ( ptr && stream )
   {
      gssize i;
      gint32 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         i_val = GINT32_TO_BE( ((gint32*)(ptr))[i] );
         n += fwrite( &i_val , sizeof(gint32) , 1 , stream );
      }
   }

   return n;
}

gssize eh_fwrite_int64_to_be( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint64) );

   if ( ptr && stream )
   {
      gssize i;
      gint64 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         i_val = GINT64_TO_BE( ((gint64*)(ptr))[i] );
         n += fwrite( &i_val , sizeof(gint64) , 1 , stream );
      }
   }

   return n;
}

#else

gssize eh_fread_int32_from_le( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint32) );

   if ( ptr && stream )
   {
      gssize i;
      gint32 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         n += fread( &i_val , sizeof(gint32) , 1 , stream );
         ((gint32*)ptr)[i] = GINT32_TO_LE( i_val );
      }
   }

   return n;
}

gssize eh_fread_int64_from_le( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint64) );

   if ( ptr && stream )
   {
      gssize i;
      gint64 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         n += fread( &i_val , sizeof(gint64) , 1 , stream );
         ((gint64*)ptr)[i] = GINT64_TO_LE( i_val );
      }
   }

   return n;
}

gssize eh_fwrite_int32_to_le( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint32) );

   if ( ptr && stream )
   {
      gssize i;
      gint32 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         i_val = GINT32_TO_LE( ((gint32*)(ptr))[i] );
         n += fwrite( &i_val , sizeof(gint32) , 1 , stream );
      }
   }

   return n;
}

gssize eh_fwrite_int64_to_le( const void *ptr , gssize size , gssize nitems , FILE* stream  )
{
   gssize n = 0;

   eh_require( size==sizeof(gint64) );

   if ( ptr && stream )
   {
      gssize i;
      gint64 i_val;

      for ( i=0 ; i<nitems ; i++ )
      {
         i_val = GINT64_TO_LE( ((gint64*)(ptr))[i] );
         n += fwrite( &i_val , sizeof(gint64) , 1 , stream );
      }
   }

   return n;
}

#endif

gpointer print_status( gpointer data );

Eh_status_bar* eh_status_bar_new( double* cur , double* end )
{
   Eh_status_bar* status_bar = eh_new( Eh_status_bar , 1 );

   status_bar->cur    = cur;
   status_bar->end    = end;
   status_bar->status = EH_STATUS_BAR_RUNNING;
   status_bar->timer  = g_timer_new();
   status_bar->mutex  = g_mutex_new();

   status_bar->t      = g_thread_create( print_status , status_bar , TRUE , NULL );

   return status_bar;
}

Eh_status_bar* eh_status_bar_stop( Eh_status_bar* b )
{
   g_mutex_lock( b->mutex );
   b->status = EH_STATUS_BAR_STOPPED;
   g_mutex_unlock( b->mutex );

   return b;
}

Eh_status_bar* eh_status_bar_pause( Eh_status_bar* b )
{
   g_mutex_lock( b->mutex );
   b->status = EH_STATUS_BAR_PAUSED;
   g_mutex_unlock( b->mutex );
   return b;
}

gboolean eh_status_bar_is_stopped( Eh_status_bar* b )
{
   gboolean is_stopped;

   g_mutex_lock( b->mutex );
   is_stopped = (b->status == EH_STATUS_BAR_STOPPED);
   g_mutex_unlock( b->mutex );

   return is_stopped;
}

Eh_status_bar* eh_status_bar_destroy( Eh_status_bar* b )
{
   eh_status_bar_stop( b );

   g_thread_join( b->t );

   g_timer_destroy( b->timer );
   g_mutex_free   ( b->mutex );
   eh_free( b );

   return NULL;
}

gpointer print_status( gpointer data )
{
   Eh_status_bar* b = (Eh_status_bar*)data;
   double t, eta;
   gchar t_str[2048], eta_str[2048];
   gchar* status_bar[] = { "." , "o" , "0" , "O" , NULL };
   gchar** p = status_bar;

   fprintf( stderr , "\n" );
   fprintf( stderr , " Current        |   Elapsed   |     ETA     \n" );

   for ( ; *(b->cur)<=0 && !eh_status_bar_is_stopped(b) ; );

   for ( ; !eh_status_bar_is_stopped(b) ; )
   {
      t   = g_timer_elapsed(b->timer,NULL);
      eta = t / *(b->cur) * ( *(b->end) - *(b->cur) );

      if ( *p==NULL )
         p = status_bar;

      if ( b->status==EH_STATUS_BAR_RUNNING )
      {
         eh_render_time_str( t   , t_str   );
         eh_render_time_str( eta , eta_str );

         fprintf( stderr , " %7g (%3.0f%%) | %s | %s" ,
                  *(b->cur) , *(b->cur) / *(b->end)*100. , t_str , eta_str );

         fprintf( stderr , "   (%s)" , *p );
         fprintf( stderr , "          \r" );
      }

      p++;

      g_usleep( 100000 );
   }

   fprintf( stderr , "\n" );

   fprintf( stderr , "Elapsed time: %s\n" ,
            eh_render_time_str(g_timer_elapsed(b->timer,NULL),t_str) );

   return data;
}

#define EH_SECONDS_PER_DAY    ( 86400. )
#define EH_SECONDS_PER_HOUR   ( 3600. )
#define EH_SECONDS_PER_MINUTE ( 60. )

gchar* eh_render_time_str( double sec , gchar* str )
{
   gint d = sec / (gint)EH_SECONDS_PER_DAY;
   gint h = sec / (gint)EH_SECONDS_PER_HOUR;
   gint m = sec / (gint)EH_SECONDS_PER_MINUTE;
   gint s = fmod( sec , 60. );

   sprintf( str , "%02d:%02d:%02d:%02d\0" , d , h , m , s );

   return str;
}

gboolean
eh_check_to_s( gboolean assert , const gchar* str , gchar*** str_list )
{
   if ( !assert )
   {
      if ( str_list )
         eh_strv_append( str_list , g_strconcat( "FAILED: " , str , NULL ) );
   }

   return assert;
}

gchar*
eh_render_command_str( int argc , char* argv[] )
{
   gchar* str = NULL;

   eh_require( argv!=NULL );

   if ( argv )
   {
      gint    i;
      gchar** str_array = NULL;

      for ( i=0 ; i<argc ; i++ )
         eh_strv_append( &str_array , argv[i] );

      str = g_strjoinv( " " , str_array );
      
      eh_free( str_array );
   }

   return str;
}

gchar*
eh_get_input_val( FILE *fp , char *msg , char *default_str )
{
   char *str = eh_new( char , S_LINEMAX );

   fprintf( stderr , "%s " , msg );
   if ( default_str )
      fprintf( stderr , "[%s] " , default_str );
   fprintf( stderr , ": " );

   fgets( str , S_LINEMAX , fp );
   if ( default_str && strncmp( str , "\n" , 1 )==0 )
      strcpy( str , default_str );
   g_strstrip( str );
   return str;
}

