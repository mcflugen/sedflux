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
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"
#include "eh_project.h"
#include "sed_sedflux.h"
#include "processes.h"

gchar* copyleft_msg[] =
{
"                                                                             ",
" sedflux - A process based basin fill model.                                 ",
" Copyright (C) 2003 Eric Hutton.                                             ",
"                                                                             ",
" This is free software; you can redistribute it and/or modify it under the   ",
" terms of the GNU General Public License as published by the Free Software   ",
" Foundation; either version 2 of the License, or (at your option) any later  ",
" version.                                                                    ",
"                                                                             ",
" This program is distributed in the hope that it will be useful, but WITHOUT ",
" ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       ",
" FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   ",
" more details.                                                               ",
"                                                                             ",
" You should have received a copy of the GNU General Public License along with",
" this program; if not, write to the Free Software Foundation, Inc., 59 Temple",
" Place - Suite 330, Boston, MA 02111-1307, USA.                              ",
"                                                                             ",
NULL
};

gchar* brief_copyleft_msg[] =
{
"Copywrite (C) 2006 Eric Hutton." ,
"This is free software; see the source for copying conditions.  This is NO" ,
"warranty;  not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." ,
NULL
};

int __quit_signal = 0; //< signal to indicate that the user wishes to quit.
int __dump_signal = 0; //< signal to indicate that the user wishes to dump output.
int __cpr_signal  = 0; //< signal to indicate that the user wishes to create a checkpoint.

gchar*     get_file_name_interactively ( gchar**      , gchar** );
char*      eh_get_input_val            ( FILE* fp     , char *msg    , char *default_str );
void       print_version_info          ( void );
void       print_choices               ( int );
void       print_header                ( FILE* fp);
void       print_time                  ( FILE* fp     , int epoch_no , double year );
void       print_footer                ( FILE* fp );
Eh_project fill_sedflux_info_file      ( Eh_project p , int argc     , char* argv[] , gchar* desc );
int        run_sedflux                 ( int argc     , char *argv[] );
gboolean   check_process_files         ( Sed_epoch_queue e_list , gchar** active );

Sed_process_queue sedflux_create_process_queue( const gchar* file , gchar** );

int main( int argc , char *argv[] )
{
   char *program_name;

   eh_init_glib();

   program_name = g_path_get_basename( argv[0] );

   if ( strcasecmp( program_name , "sedflux3d" )==0 )
   {
      g_set_application_name( "sedflux3d" );
      run_sedflux( argc , argv );
   }
   else if (    strcasecmp( program_name , "sedflux2d" )==0 
             || strcasecmp( program_name , "sedflux"   )==0 )
   {
      g_set_application_name( "sedflux2d" );
      run_sedflux( argc , argv );
   }
   else
      eh_exit(-1);

   return 1;
}

/* Command line options */
static gchar*   init_file    = NULL;
static gchar*   out_file     = NULL; 
static gchar*   working_dir  = NULL;
static gchar*   run_desc     = NULL;
static gboolean just_plume   = FALSE;
static gboolean just_rng     = FALSE;
static gboolean summary      = FALSE;
static gboolean warn         = FALSE;
static gint     verbose      = 0;
static gboolean version      = FALSE;
static char**   active_procs = NULL;

/* Define the command line options */
static GOptionEntry entries[] =
{
   { "init-file"   , 'i' , 0 , G_OPTION_ARG_FILENAME     , &init_file   , "Initialization file"        , "<file>" } ,
   { "out-file"    , 'o' , 0 , G_OPTION_ARG_FILENAME     , &out_file    , "Output file"                , "<file>" } ,
   { "working-dir" , 'd' , 0 , G_OPTION_ARG_FILENAME     , &working_dir , "Working directory"          , "<dir>"  } ,
   { "msg"         , 'm' , 0 , G_OPTION_ARG_STRING       , &run_desc    , "Run description"            , "<msg>"  } ,
   { "active-proc" , 'a' , 0 , G_OPTION_ARG_STRING_ARRAY , &active_procs, "Specify active process"     , "<name>" } ,
   { "just-plume"  , 'p' , 0 , G_OPTION_ARG_NONE         , &just_plume  , "Run just the plume"         , NULL     } ,
   { "just-rng"    , 'r' , 0 , G_OPTION_ARG_NONE         , &just_rng    , "Run just the rng processes" , NULL     } ,
   { "summary"     , 's' , 0 , G_OPTION_ARG_NONE         , &summary     , "Print a summary and quit"   , NULL     } ,
   { "warn"        , 'w' , 0 , G_OPTION_ARG_NONE         , &warn        , "Print warnings"             , NULL     } ,
   { "verbose"     , 'V' , 0 , G_OPTION_ARG_INT          , &verbose     , "Verbosity level"            , "n"      } ,
   { "version"     , 'v' , 0 , G_OPTION_ARG_NONE         , &version     , "Version number"             , NULL     } ,
   { NULL }
};

static gchar* just_plume_procs[] = { "plume"      , "river"  ,  "bbl" , NULL }; //< Processes to run with just-plume option
static gchar* just_rng_procs[]   = { "earthquake" , "storms" , NULL };          //< Process to run with just-rng option

int run_sedflux(int argc, char *argv[])
{
   Sed_epoch_queue   list; //< List of all the epochs
   Sed_cube          prof; //< The cube 

   g_log_set_handler( NULL , G_LOG_LEVEL_MASK , &eh_logger , NULL );

   eh_require( argv )
   {
      Eh_project      proj    = eh_create_project( "sedflux" );
      GLogLevelFlags  ignore  = 0;
      GError*         error   = NULL;
      GOptionContext* context = g_option_context_new( "Run basin filling model sedflux-2.0" );

      g_option_context_add_main_entries( context , entries , NULL );

      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );

      if ( version )
      {
         print_version_info( );
         exit(0);
      }

      switch (verbose)
      {
         case 0:
            ignore |= G_LOG_LEVEL_ERROR;
         case 1:
            ignore |= G_LOG_LEVEL_CRITICAL;
         case 2:
            ignore |= G_LOG_LEVEL_WARNING;
         case 3:
            ignore |= G_LOG_LEVEL_MESSAGE;
         case 4:
            ignore |= G_LOG_LEVEL_INFO;
         case 5:
            ignore |= G_LOG_LEVEL_DEBUG;
      }

      eh_set_ignore_log_level( ignore );

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

      eh_set_project_dir        ( proj , working_dir );
      fill_sedflux_info_file    ( proj , argc , argv , run_desc );
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

   print_header( stdout );

   eh_debug( "Checking consistancy of processes in epoch file." );
   if ( !check_process_files( list , active_procs )!=0 && warn )
      eh_exit(-1);

   eh_debug( "Start the simulation" );
   {
      Sed_epoch         epoch;
      double            year;
      double            n_years;
      double            time_step;
      Sed_process_queue q;

      for ( epoch = sed_epoch_queue_pop( list ) ;
            epoch && !__quit_signal             ;
            epoch = sed_epoch_queue_pop( list ) )
      {
         eh_debug( "Read the process data" );
         q = sedflux_create_process_queue( sed_epoch_filename(epoch) , active_procs );

         time_step = sed_epoch_time_step( epoch );
         n_years   = sed_epoch_duration ( epoch );

         sed_cube_set_time_step( prof , time_step );

         for ( year  = sed_cube_time_step(prof)  ;
               year <= n_years && !__quit_signal ;
               year += sed_cube_time_step(prof) )
         {
            print_time( stdout , sed_epoch_number(epoch) , year );

            sed_process_queue_run( q , prof );

            if ( __dump_signal )
            {
               sed_process_queue_run_process_now( q , "data dump" , prof );

               __dump_signal = FALSE;
            }

            sed_cube_increment_age( prof );
         }

         sed_process_queue_run_at_end( q , prof );

         sed_process_queue_destroy( q     );
         sed_epoch_destroy        ( epoch );
         sed_cube_free_river      ( prof  );
      }

   }
   
   print_footer(stdout);
   
   sed_epoch_queue_destroy( list );

   sed_cube_destroy( prof );
   sed_sediment_unset_env( );

   if ( g_getenv("SED_MEM_CHECK") )
      eh_heap_dump( "heap_dump.txt" );

   /* Command line options */
   eh_free   ( init_file    );
   eh_free   ( out_file     );
   eh_free   ( working_dir  );
   eh_free   ( run_desc     );
   g_strfreev( active_procs );

   eh_exit(0);

   eh_require_not_reached();
   return -1;
}

void
print_version_info( void )
{
   g_print( "%s %d.%d.%d\n" , PROGRAM_NAME , 
                              S_MAJOR_VERSION ,
                              S_MINOR_VERSION ,
                              S_MICRO_VERSION );
   g_print( "Written by Eric Hutton <huttone@colorado.edu>.\n" );
   g_print( "\n" );
   eh_print_message( stdout , brief_copyleft_msg );
}

gchar *get_file_name_interactively( gchar **working_dir , gchar **in_file )
{
   char *dir       = eh_new( char , 2048 );
   char *cur_dir   = eh_new( char , 2048 );
   char *init_file = g_strdup( "basin.init" );

   eh_print_message( stderr , copyleft_msg );

   fprintf( stderr , "---\n" );
   fprintf( stderr , "--- This is %s" , PROGRAM_NAME );

   if ( is_sedflux_3d() )
      fprintf( stderr , "3d" );

   fprintf( stderr , " version %s.%s.%s \n" ,
            PROGRAM_MAJOR_VERSION ,
            PROGRAM_MINOR_VERSION ,
            PROGRAM_MICRO_VERSION );
   fprintf( stderr , "---\n" );

   cur_dir = g_get_current_dir();

   fprintf( stderr , "\n" );
   dir       = eh_get_input_val( stdin ,
                                 "Set the working directory" ,
                                 cur_dir );
   init_file = eh_get_input_val( stdin ,
                                 "Specify an initialization file" ,
                                 init_file );

   fprintf( stderr , "\n" );

   fprintf( stderr , "Working directory        : %s\n" , dir );
   fprintf( stderr , "Initialization file      : %s\n" , init_file );

   *working_dir = g_strdup( dir );

   eh_free( *in_file     );
   eh_free( *working_dir );

   *working_dir = dir;
   *in_file     = init_file;

   eh_free( cur_dir   );

   return *in_file;
}

char *eh_get_input_val( FILE *fp , char *msg , char *default_str )
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

void print_header(FILE *fp)
{
   time_t sedflux_start_time;
   sedflux_start_time = time(&sedflux_start_time);

   fprintf( fp ,
            "%s %s.%s.%s\n" ,
            PROGRAM_NAME ,
            PROGRAM_MAJOR_VERSION ,
            PROGRAM_MINOR_VERSION ,
            PROGRAM_MICRO_VERSION );
   fprintf( fp , "Start time is %s\n" , ctime(&sedflux_start_time) );

   return;
}

void print_time(FILE *fp, int epoch_no, double year)
{
   fprintf( fp , "%.2f years (epoch %2d)\r" , year , epoch_no );
   fflush( fp );

   return;
}

void print_footer(FILE *fp)
{
   time_t sedflux_end_time;
   sedflux_end_time = time(&sedflux_end_time);
   fprintf( fp , "End time is %s\n" , ctime(&sedflux_end_time) );
   fprintf( fp , "\n" );
   return;
}

void print_choices(int i)
{
   char ch;

   fprintf( stdout , "\n"                                                );
   fprintf( stdout , "-----------------------------------------------\n" );
   fprintf( stdout , "  (1) End run after this time step.\n"             );
   fprintf( stdout , "  (2) Dump results and continue.\n"                );
   fprintf( stdout , "  (3) Create a checkpoint.\n"                      );
   fprintf( stdout , "  (4) Continue.\n"                                 );
   fprintf( stdout , "  (5) Quit immediatly (without saving).\n"         );
   fprintf( stdout , "-----------------------------------------------\n" );
   fprintf( stdout , "   Your choice? "                                  );

   fscanf(stdin,"%s",&ch);

   if ( g_strcasecmp( &ch , "1" )==0 )
   {
      __quit_signal = 1;
      fprintf(stdout,"   You have opted to quit early...\n\n");
   }
   else if ( g_strcasecmp( &ch , "2" )==0 )
   {
      __dump_signal = 1;
      signal(2,&print_choices);
      fprintf( stdout ,
               "   Temporary output files will be dumped as soon as we can get to it...\n\n");
   }
   else if ( g_strcasecmp( &ch , "3" )==0 )
   {
      __cpr_signal = 1;
      signal(2,&print_choices);
      fprintf(stdout,"   Creating a checkpoint/reset file...\n\n");
   }
   else if ( g_strcasecmp( &ch , "4" )==0 )
   {
      signal(2,&print_choices);
      fprintf(stdout,"   Continuing...\n\n");
   }
   else
   {
      signal(2,&print_choices);
      fprintf(stdout,"   Terminating run without saving...\n\n" );
      eh_exit(0);
   }

   return;
}

#include <time.h>

Eh_project fill_sedflux_info_file( Eh_project p , int argc , char* argv[] , gchar* desc )
{
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
      char *version_str = g_strconcat( PROGRAM_MAJOR_VERSION , "." ,
                                       PROGRAM_MINOR_VERSION , "." ,
                                       PROGRAM_MICRO_VERSION , NULL );

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

      eh_project_add_info_val( p , "PROGRAM" , argv[0] );

      for ( i=1 ; i<argc ; i++ )
         eh_project_add_info_val( p , "COMMAND-LINE OPTIONS" , argv[i] );
   }

   return p;
}


