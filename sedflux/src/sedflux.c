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

#include "utils.h"
#include "sed_sedflux.h"
#include "processes.h"
#include "sedflux.h"

Sed_process_queue sedflux_create_process_queue( const gchar* file , gchar** user_data , GError** error );

gboolean
sedflux( const gchar* init_file , Sedflux_run_flag flag )
{
   gboolean          success = TRUE;
   Sed_epoch_queue   list; //< List of all the epochs
   Sed_cube          prof; //< The cube 

   eh_require( init_file );
   eh_require( sed_mode_is_2d() || sed_mode_is_3d() );

   eh_debug( "Scan the init file" );
   {
      GError* error = NULL;

      prof = sed_cube_new_from_file( init_file , &error );
      eh_exit_on_error( error , "%s: Error reading initialization file" , init_file );

      list = sed_epoch_queue_new   ( init_file , &error );
      eh_exit_on_error( error , "%s: Error reading epoch file" , init_file );
   }

   if ( flag & SEDFLUX_RUN_FLAG_SUMMARY )
   {
      sed_cube_fprint       ( stdout , prof );
      sed_epoch_queue_fprint( stdout , list );
   }

   eh_debug( "Checking consistancy of processes in epoch file." );
   if ( !check_process_files( list , NULL )!=0 && (flag & SEDFLUX_RUN_FLAG_WARN) )
      success = FALSE;

   eh_debug( "Start the simulation" );
   if ( success )
   {
      Sed_epoch         epoch;
      double            year;
      double            n_years;
      double            time_step;
      Sed_process_queue q;
      Eh_status_bar*    bar;
      GError*           error = NULL;

      bar = eh_status_bar_new( &year , &n_years );

      for ( epoch = sed_epoch_queue_pop( list ) ;
            epoch && !sedflux_signal_is_pending(SEDFLUX_SIG_QUIT) ;
            epoch = sed_epoch_queue_pop( list ) )
      {
         eh_debug( "Read the process data" );
         q = sedflux_create_process_queue( sed_epoch_filename(epoch) , NULL , &error );
         eh_exit_on_error( error , "Error creating process queue" );

         eh_debug( "Set the time step and duration for this epoch" );
         time_step = sed_epoch_time_step( epoch );
         n_years   = sed_epoch_duration ( epoch );

         sed_cube_set_time_step( prof , time_step );

         eh_debug( "Run this epoch" );
         for ( year  = sed_cube_time_step(prof)  ;
               year <= n_years && !sedflux_signal_is_pending(SEDFLUX_SIG_QUIT) ;
               year += sed_cube_time_step(prof) )
         {
            sed_process_queue_run( q , prof );

            if ( sedflux_signal_is_pending(SEDFLUX_SIG_DUMP) )
            {
               sed_process_queue_run_process_now( q , "data dump" , prof );
               sedflux_signal_reset(SEDFLUX_SIG_DUMP);
            }

            sed_cube_increment_age( prof );
         }

         eh_debug( "Run at-end processes" );
         sed_process_queue_run_at_end( q , prof );

         eh_debug( "Write process summary" );
         sed_process_queue_summary( stdout , q );

         eh_debug( "Destroy process queue" );
         sed_process_queue_destroy( q     );
         eh_debug( "Destroy epoch" );
         sed_epoch_destroy        ( epoch );
         eh_debug( "Free river data" );
         sed_cube_free_river      ( prof  );
      }

      eh_debug( "Destroy status bar" );
      eh_status_bar_destroy( bar );
   }
   
   eh_debug( "Destroy epoch queue" );
   sed_epoch_queue_destroy( list );

   eh_debug( "Destroy cube" );
   sed_cube_destroy( prof );

   eh_debug( "Destroy sediment environment" );
   sed_sediment_unset_env( );

   if ( g_getenv("SED_MEM_CHECK") )
      eh_heap_dump( "heap_dump.txt" );

   return success;
}

#if defined(IGNORE)
int run_sedflux(int argc, char *argv[])
{
   Sed_epoch_queue   list; //< List of all the epochs
   Sed_cube          prof; //< The cube 

   g_log_set_handler( NULL , G_LOG_LEVEL_MASK , &eh_logger , NULL );

   eh_require( argv )
   {
      Eh_project      proj    = eh_create_project( "sedflux" );
      GError*         error   = NULL;
      GOptionContext* context = g_option_context_new( "Run basin filling model sedflux-2.0" );

      g_option_context_add_main_entries( context , entries , NULL );

      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );

      if ( version )
      {
         eh_fprint_version_info( stdout          ,
                                 PROGRAM_NAME    ,
                                 S_MAJOR_VERSION ,
                                 S_MINOR_VERSION ,
                                 S_MICRO_VERSION );
         exit(0);
      }

      eh_set_verbosity_level( verbose );

      if ( !init_file )
         get_file_name_interactively( &working_dir , &init_file );

      if ( working_dir && g_chdir( working_dir )!=0 )
         perror( working_dir );

      if ( !active_procs )
      {
         if ( just_plume )
            active_procs = just_plume_procs;
         if ( just_rng )
            active_procs = just_rng_procs;
      }

      eh_debug( "Set project directory" );
      eh_set_project_dir        ( proj , working_dir );
      eh_debug( "Fill sedflux info file" );
      fill_sedflux_info_file    ( proj , argc , argv , run_desc );
      eh_debug( "Write sedflux info file" );
      eh_write_project_info_file( proj );

      eh_free              ( working_dir );
      eh_destroy_project   ( proj        );
      g_option_context_free( context     );
   }

   signal(2,&print_choices);

   eh_debug( "Scan the init file" );
   {
      prof = sed_cube_new_from_file( init_file );
      list = sed_epoch_queue_new   ( init_file );
   }

   if ( summary )
   {
      sed_cube_fprint       ( stdout , prof );
      sed_epoch_queue_fprint( stdout , list );
   }

   eh_debug( "Checking consistancy of processes in epoch file." );
   if ( !check_process_files( list , active_procs )!=0 && warn )
      eh_exit( EXIT_FAILURE );

   eh_debug( "Start the simulation" );
   {
      Sed_epoch         epoch;
      double            year;
      double            n_years;
      double            time_step;
      Sed_process_queue q;
      Eh_status_bar*    bar;

      bar = eh_status_bar_new( &year , &n_years );

      for ( epoch = sed_epoch_queue_pop( list ) ;
            epoch && !sedflux_signal_is_pending(SEDFLUX_SIG_QUIT) ;
            epoch = sed_epoch_queue_pop( list ) )
      {
         eh_debug( "Read the process data" );
         q = sedflux_create_process_queue( sed_epoch_filename(epoch) , active_procs );

         eh_debug( "Set the time step and duration for this epoch" );
         time_step = sed_epoch_time_step( epoch );
         n_years   = sed_epoch_duration ( epoch );

         sed_cube_set_time_step( prof , time_step );

         eh_debug( "Run this epoch" );
         for ( year  = sed_cube_time_step(prof)  ;
               year <= n_years && !sedflux_signal_is_pending(SEDFLUX_SIG_QUIT) ;
               year += sed_cube_time_step(prof) )
         {
            sed_process_queue_run( q , prof );

            if ( sedflux_signal_is_pending(SEDFLUX_SIG_DUMP) )
            {
               sed_process_queue_run_process_now( q , "data dump" , prof );
               sedflux_signal_reset(SEDFLUX_SIG_DUMP);
            }

            sed_cube_increment_age( prof );
         }

         eh_debug( "Run at-end processes" );
         sed_process_queue_run_at_end( q , prof );

         eh_debug( "Write process summary" );
         sed_process_queue_summary( stdout , q );

         eh_debug( "Destroy process queue" );
         sed_process_queue_destroy( q     );
         eh_debug( "Destroy epoch" );
         sed_epoch_destroy        ( epoch );
         eh_debug( "Free river data" );
         sed_cube_free_river      ( prof  );
      }

      eh_debug( "Destroy status bar" );
      eh_status_bar_destroy( bar );
   }
   
   eh_debug( "Destroy epoch queue" );
   sed_epoch_queue_destroy( list );

   eh_debug( "Destroy cube" );
   sed_cube_destroy( prof );
   eh_debug( "Destroy sediment environment" );
   sed_sediment_unset_env( );

   if ( g_getenv("SED_MEM_CHECK") )
      eh_heap_dump( "heap_dump.txt" );

   /* Command line options */
   eh_free   ( init_file    );
   eh_free   ( out_file     );
   eh_free   ( working_dir  );
   eh_free   ( run_desc     );
   g_strfreev( active_procs );

   return 1;
}


#include <time.h>

Eh_project
fill_sedflux_info_file( Eh_project p , int argc , char* argv[] , gchar* desc )

   //---
   // Define the description of the model run.
   //---
   eh_require( p )
   {
      char* default_str = NULL;
      char* desc_str    = NULL;
      Eh_project temp = eh_create_project( eh_project_name(p) );

      eh_set_project_dir( temp , eh_project_dir_name(p) );
      eh_read_project_info_file( temp );

      if ( !desc )
      {
         default_str = eh_project_get_info_val( temp , "RUN DESCRIPTION" );
         desc_str    = eh_input_str( "RUN DESCRIPTION" , default_str );
         eh_require( desc_str );
         eh_project_add_info_val( p , "RUN DESCRIPTION" , desc_str );
      }
      else
         eh_project_add_info_val( p , "RUN DESCRIPTION" , desc );

      eh_destroy_project( temp );
      eh_free( default_str );
      eh_free( desc_str );
   }

   //---
   // Define the sedflux version.
   //---
   eh_require( p )
   {
      gchar* version_str = g_strdup( SED_VERSION_S );

      eh_require( version_str );
      eh_project_add_info_val( p , "VERSION" , version_str );

      eh_free( version_str );
   }

   //---
   // Define the modification time of the executable.
   //--
   eh_require( argv[0] )
   {
      struct stat stat_buf;
      char* mod_str = eh_new( gchar , S_LINEMAX );
      GDate *today = g_date_new();

      g_stat( argv[0] , &stat_buf );
      g_date_set_time( today , time(&(stat_buf.st_mtime)) );
      g_date_strftime( mod_str , S_LINEMAX , "%d/%m/%Y" , today );

      eh_require( mod_str );
      eh_project_add_info_val( p , "CREATED" , mod_str );

      g_date_free( today );
      eh_free( mod_str );
   }


   //---
   // Define the name of the executable file and the command-line arguments.
   //---
   eh_require( argv[0] )
   {
      int i;

      eh_require( argv[0] );
      eh_project_add_info_val( p , "PROGRAM" , argv[0] );

      for ( i=1 ; i<argc ; i++ )
{
      eh_require( argv[i] );
         eh_project_add_info_val( p , "COMMAND-LINE OPTIONS" , argv[i] );
}
   }

   return p;
}
#endif /* IGNORE */

