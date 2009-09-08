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

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_sedflux.h"

Sedflux_param_st* sedflux_setup        ( gchar* command_s );

Sedflux_param_st*
sedflux_setup( gchar* command_s )
{
   Sedflux_param_st* p           = NULL;
   GError*           error       = NULL;
   gchar*            command_str = NULL;
   int               argc;
   char**            argv;

   g_thread_init( NULL );
   eh_init_glib();
   g_log_set_handler( NULL , G_LOG_LEVEL_MASK , &eh_logger , NULL );

   argv = g_strsplit( command_s , " " , 0 );
   argc = g_strv_length( argv );

   command_str = eh_render_command_str( argc , argv );

   /* Parse command line arguments */
   p = sedflux_parse_command_line( argc , argv , &error );
   eh_exit_on_error( error , "Error parsing command line arguments" );

   /* Create the project directory and check permissions */
   sedflux_setup_project_dir( &p->init_file , &p->working_dir , &error );
   eh_exit_on_error( error , "Error setting up project directory" );

   sedflux_print_info_file( p->init_file , p->working_dir , command_str , p->run_desc );

   /* Setup the signal handling */
   sed_signal_set_action();

   return p;
}

/** Get variables at a time and place of a Sed_cube

\param   q       A Sed_epoch_queue
\param   p       A Sed_cube
\param   val_s   Measurement string
\param   here    -1-terminated array of Sed_cube ids (or NULL)
\param   now     Time in years to get values at

\return Newly-allocated array of the variable measured at the specified time and place(s).
*/
double*
sedflux_get_val_at( Sed_epoch_queue q , Sed_cube p , const gchar* val_s , gint* here , double now )
{
   double* data = NULL;

   eh_require( q    );
   eh_require( p    );

   if ( sedflux_run_until( q , p , now ) )
   {
      Sed_measurement m = sed_measurement_new( val_s );

      eh_require( m );

      if ( !here )
      { /* If here is NULL, get values at all grid cells */
         gint64   id;
         Eh_ind_2 sub;

         data = eh_new( double , sed_cube_size( p ) );
         for ( id=0 ; id<sed_cube_size(p) ; id++ )
         {
            sub = sed_cube_sub( p , id );
            data[id] = sed_measurement_make( m , p , sub.i , sub.j );
         }
      }
      else
      { /* Get values at specified grid cells */
         gint64   len;
         gint64   i;
         Eh_ind_2 sub;

         for ( len=0 ; here[len]>=0 ; len++ );
         data = eh_new( double , len );

         for ( i=0 ; i<len ; i++ )
         {
            if ( sed_cube_is_in_domain_id( p , here[i] ) )
               data[i] = eh_nan();
            else
            {
               sub     = sed_cube_sub( p , here[i] );
               data[i] = sed_measurement_make( m , p , sub.i , sub.j );
            }
         }
      }

      sed_measurement_destroy( m );

   }

   return data;
}

gboolean
sedflux_set_val_at( Sed_cube p , const gchar* val_s , gint* here, const double* val )
{
   gboolean success = FALSE;

   eh_require( p );

   if ( val && g_ascii_strcasecmp( val_s, "BASEMENT" )==0 )
   {
      eh_watch_str( val_s );
      eh_watch_ptr( here );
      if ( here )
      {
         gint i;
         gint* id = here;
         for ( ; *id!=-1; id++ )
            sed_column_set_base_height( sed_cube_col( p, *id ), val[i] );

      }
      else
      {
         const gint len = sed_cube_size(p);
         gint i;

         for ( i=0; i<len; i++ )
            sed_column_set_base_height( sed_cube_col( p, i ), val[i] );
      }
   }

   return success;
}

