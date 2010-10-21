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

#include <eh_utils.h>
#include <math.h>
#include <stdlib.h>

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

void
eh_exit_on_error( GError* error , const gchar* format , ... )
{
   if ( error )
   {
      gchar* err_s;
      va_list ap;
      va_start( ap , format );

      err_s = g_strdup_vprintf( format , ap );
      eh_error( eh_render_error_str( error , err_s ) );
      eh_exit( EXIT_FAILURE );

      va_end(ap);
      eh_free( err_s );
   }
}

void
eh_print_on_error( GError* error , const gchar* format , ... )
{
   if ( error )
   {
      gchar* err_s;
      va_list ap;
      va_start( ap , format );

      err_s = g_strdup_vprintf( format , ap );
      eh_error( eh_render_error_str( error , err_s ) );

      va_end(ap);
      eh_free( err_s );
   }
}

const gchar* brief_copyleft_msg[] =
{
"Copywrite (C) 2006 Eric Hutton." ,
"This is free software; see the source for copying conditions.  This is NO" ,
"warranty;  not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." ,
NULL
};

gint
eh_fprint_version_info( FILE*        fp   ,
                        const gchar* prog ,
                        gint         maj  ,
                        gint         min  ,
                        gint         micro )
{
   gint n = 0;

   if ( fp )
   {
      n += fprintf( fp , "%s %d.%d.%d\n" , prog , maj , min , micro );

      n += fprintf( fp , "Written by Eric Hutton <eric.hutton@colorado.edu>.\n" );
      n += fprintf( fp , "\n" );

      n += eh_print_message( fp , brief_copyleft_msg );
   }
   return n;
}

/** Test if the index to a cell is within a square domain.

@param n_i The number of i elements in the domain.
@param n_j The number of j elements in the domain.
@param i   An i coordinate to test.
@param j   An j coordinate to test.

@return TRUE if the coordinate is within the domain.
*/
gboolean
eh_is_in_domain( gssize n_i , gssize n_j , gssize i , gssize j )
{
   return i>=0 && j>=0 && i<n_i && j<n_j;
}

gboolean
eh_is_boundary_id (gint nx, gint ny, gint id)
{
  if (id < ny || id >= ny*(nx-1) || id%ny == 0 || (id+1)%ny == 0 )
    return TRUE;
  else
    return FALSE;
}

double
eh_date_to_years( Eh_date_t* d )
{
   eh_require( d );
   return   d->year
          + d->month/12.
          + d->day/365.;
}

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

void
eh_test_function( const char *func_name , Eh_test_func f )
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

gboolean
eh_check_to_s( gboolean assert , const gchar* str , gchar*** str_list )
{
   if ( !assert && str_list ) eh_strv_append( str_list , g_strconcat( "FAILED: " , str , NULL ) );

   return assert;
}

void
eh_set_error_strv( GError** error , GQuark domain , gint code , gchar** err_s )
{
   if ( err_s && *err_s )
   {
      gchar* str = g_strjoinv( "\n" , err_s );

      g_set_error( error , domain , code , "%s" , str );

      eh_free( str );
   }

   return;
}

gchar*
eh_render_error_str( GError* error , const gchar* err_str )
{
   gchar* new_str = NULL;

   if ( error )
   {
      if ( err_str )
         new_str = g_strdup_printf( "Error %d: %s: %s: %s"        ,
                                    error->code                   ,
                                    g_quark_to_string(error->domain) ,
                                    err_str                       ,
                                    error->message );
      else
         new_str = g_strdup_printf( "Error %d: %s: %s"            ,
                                    error->code                   ,
                                    g_quark_to_string(error->domain) ,
                                    error->message );
   }

   return new_str;
}

gchar*
eh_render_command_str (const int argc, const char* argv[])
{
   gchar* str = NULL;

   eh_require( argv!=NULL );

   if ( argv )
   {
      gint    i;
      gchar** str_array = NULL;

      for ( i=0 ; i<argc ; i++ )
         eh_strv_append (&str_array, g_strdup (argv[i]));

      str = g_strjoinv( " " , str_array );
      
      g_strfreev (str_array);
   }

   return str;
}

