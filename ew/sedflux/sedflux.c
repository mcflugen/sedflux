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

/** Global variable that lists all of the process that sedflux will run 

sedflux will cycle through the process in the order that they are listed here.
To add a new process to sedflux, you will need to add an entry to this list.
sedflux will will then automatically cycle through your process.
*/
Sed_process_init_t my_proc_defs[] =
{
   { "constants"         , init_constants     , run_constants   , destroy_constants } , 
   { "earthquake"        , init_quake         , run_quake       , destroy_quake     } , 
   { "tide"              , init_tide          , run_tide        , destroy_tide      } , 
   { "sea level"         , init_sea_level     , run_sea_level   , destroy_sea_level } ,
   { "storms"            , init_storm         , run_storm       , destroy_storm     } ,
   { "river"             , init_river         , run_river       , destroy_river     } ,
   { "erosion"           , init_erosion       , run_erosion     , destroy_erosion   } ,
   { "avulsion"          , init_avulsion      , run_avulsion    , destroy_avulsion  } ,

   /* A new process */
   { "new process"       , init_new_process   , run_new_process , destroy_new_process } ,

   /* The rest of the processes */
   { "bedload dumping"   , init_bedload       , run_bedload     , destroy_bedload     } ,
   { "plume"             , init_plume         , run_plume       , destroy_plume       } ,
   { "diffusion"         , init_diffusion     , run_diffusion   , destroy_diffusion   } ,
   { "xshore"            , init_xshore        , run_xshore      , destroy_xshore      } ,
   { "squall"            , init_squall        , run_squall      , destroy_squall      } ,
   { "bioturbation"      , bio_init           , bio_run         , bio_destroy } ,
   { "compaction"        , NULL               , run_compaction  , NULL                } ,
   { "flow"              , init_flow          , run_flow        , destroy_flow        } ,
   { "isostasy"          , init_isostasy      , run_isostasy    , destroy_isostasy    } ,
   { "subsidence"        , init_subsidence    , run_subsidence  , destroy_subsidence  } ,
   { "data dump"         , init_data_dump     , run_data_dump   , destroy_data_dump   } ,
   { "failure"           , init_failure       , run_failure     , destroy_failure     } ,
   { "measuring station" , init_met_station   , run_met_station , destroy_met_station } ,
   { "bbl"               , init_bbl           , run_bbl         , destroy_bbl         } ,
   { "cpr"               , init_cpr           , run_cpr         , destroy_cpr         } ,

   { "hypopycnal plume"  , init_plume_hypo    , run_plume_hypo  , destroy_plume_hypo  } ,
   { "inflow"            , init_inflow        , run_plume_hyper_inflow , destroy_inflow     } ,
   { "sakura"            , init_inflow        , run_plume_hyper_sakura , destroy_inflow     } ,

   { "debris flow"       , init_debris_flow   , run_debris_flow , destroy_debris_flow } ,
   { "slump"             , NULL               , run_slump       , NULL                } ,

   { NULL }
};

Sed_process_family my_proc_family[] =
{
   { "failure" , "turbidity current" } ,
   { "failure" , "debris flow"       } ,
   { "failure" , "slump"             } ,
   { "plume"   , "hypopycnal plume"  } ,
   { "plume"   , "inflow"            } ,
   { "plume"   , "sakura"            } ,
   { NULL }
};

Sed_process_check my_proc_checks[] =
{
   {"plume"           , NULL         , SED_PROC_UNIQUE|SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"bedload dumping" , NULL         , SED_PROC_UNIQUE|SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"bbl"             , NULL         , SED_PROC_UNIQUE|SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"river"           , NULL         ,                 SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"earthquake"      , NULL         , SED_PROC_UNIQUE } ,
   {"storms"          , NULL         , SED_PROC_UNIQUE } ,
   {"failure"         , "earthquake" , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   {"squall"          , "storms"     , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   {"xshore"          , "storms"     , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   {"diffusion"       , "storms"     , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   { NULL }
};


#define ESMF
#if defined( ESMF )

Sedflux_param_st* sedflux_setup        ( gchar* command_s );
gboolean sedflux_init         ( Sed_epoch_queue* q , Sed_cube* p , const gchar* init_file );
gboolean sedflux_run_time_step( Sed_epoch_queue  q , Sed_cube  p );
gboolean sedflux_run          ( Sed_epoch_queue  q , Sed_cube  p );
gboolean sedflux_finalize     ( Sed_epoch_queue  q , Sed_cube  p );

gboolean
sedflux( const gchar* init_file )
{
   gboolean          success = TRUE;
   Sed_epoch_queue   q       = NULL; //< List of all the epochs
   Sed_cube          p       = NULL; //< The cube 

   if (    sedflux_init    ( &q , &p , init_file )
        && sedflux_run     (  q ,  p )
        && sedflux_finalize(  q ,  p ) )
      success = TRUE;
   else
      success = FALSE;

   return success;
}

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

gboolean
sedflux_init( Sed_epoch_queue* q , Sed_cube* p , const gchar* init_file )
{
   gboolean success = TRUE;

   eh_require( init_file  );
   eh_require( q          );
   eh_require( p          );
   eh_require( (*q)==NULL );
   eh_require( (*p)==NULL );
   eh_require( sed_mode_is_2d() || sed_mode_is_3d() );

   eh_debug( "Scan the init file" );

   if ( init_file && p && q )
   {
      GError* error = NULL;

      (*p) = sed_cube_new_from_file( init_file , &error );
      eh_exit_on_error( error , "%s: Error reading initialization file" , init_file );

//      (*q) = sed_epoch_queue_new_full( init_file , my_proc_defs , my_proc_family , my_proc_checks , &error );
      (*q) = sed_epoch_queue_new_full( init_file , my_proc_defs , my_proc_family , NULL , &error );
      eh_exit_on_error( error , "%s: Error reading epoch file" , init_file );

   }
/*
   if ( flag & SEDFLUX_RUN_FLAG_SUMMARY )
   {
      sed_cube_fprint       ( stdout , p );
      sed_epoch_queue_fprint( stdout , q );
   }
*/

   eh_require( success==TRUE || success==FALSE );

   return success;
}

gboolean
sedflux_run_time_step( Sed_epoch_queue q , Sed_cube p )
{
   gboolean success = FALSE;

   eh_require( q );
   eh_require( p );

   if ( p && q )
   {
      sed_epoch_queue_tic( q , p );
      success = TRUE;
   }

   return success;
}

gboolean
sedflux_run( Sed_epoch_queue q , Sed_cube p )
{
   gboolean success = FALSE;

   eh_require( q );
   eh_require( p );

   if ( q && p )
   {
      sed_epoch_queue_run( q , p );
      success = TRUE;
   }

   return success;
}

gboolean
sedflux_run_until( Sed_epoch_queue q , Sed_cube p , double then )
{
   gboolean success = FALSE;

   eh_require( q );
   eh_require( p );

   if ( q && p )
   {
      if ( sed_epoch_queue_run_until( q , p , then ) )
         success = TRUE;
   }

   return success;
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
   eh_require( here );

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
sedflux_finalize( Sed_epoch_queue q , Sed_cube p )
{
   gboolean success = FALSE;

   eh_require( q );
   eh_require( p );

   if ( q && p )
   {
      sed_epoch_queue_destroy( q );

      sed_cube_destroy( p );

      sed_sediment_unset_env( );

      success = TRUE;
   }

   return success;
}

#else /* ESMF */

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
#endif /* NOT ESMF */

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

