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

// Self Documentation
char *help_msg[] =
{
"                                                                             ",
" sedflux [options] [parameters]  [filein]                                    ",
"  run the sedflux basin filling model.                                       ",
"                                                                             ",
" Options                                                                     ",
"  -v          - be verbose. [off]                                            ",
"  -h          - print this help message.                                     ",
"                                                                             ",
" Parameters                                                                  ",
"  -pdt=value  - Use a time step of value seconds. [ .01 ]                    ",
"  -pnu=value  - Use a viscosity value of value m^2/s for the flow. [ .00083 ]",
"  -pnuart=value - Use a numerical viscosity value of value m^2/s for the     ",
"                  flow. [ .01 ]                                              ",
"  -pyield=value - Use a yield strength of value N/m^2. [ 100. ]              ",
"  -prho=value - Use a flow density of value kg/m^3. [ 1500. ]                ",
"  -pend=value - Stop the simulation after value minutes. [ 60. ]             ",
"  -pint=value - Write to output file every value seconds [ 120 ]             ",
"                                                                             ",
" Files                                                                       ",
"  Input File                                                                 ",
"   filein      -- Read bathymetry data from filein. [ stdin ]                ",
"    Bathymetry is defined by x-y pairs with each point given on a new line.  ",
"    Values are in meters from an arbitrary origin.  Comment lines are allowed",
"    and are defined as any lines not beginning with a number.                ",
"                                                                             ",
"  Output Files:                                                              ",
"                                                                             ",
NULL
};

char *copyleft_msg[] =
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


#define DEFAULT_VERBOSE      0
#define DEFAULT_IN_FILE      stdin
#define DEFAULT_IN_FILE_NAME "stdin"
#define DEFAULT_TIME_STEP    1

// signal to indicate that the user wishes to quit.
int __quit_signal = 0;
// signal to indicate that the user wishes to dump output.
int __dump_signal = 0;
// signal to indicate that the user wishes to create a checkpoint.
int __cpr_signal  = 0;

char *get_file_name_interactively( char** , char** );
char *eh_get_input_val( FILE *fp , char *msg , char *default_str );
void print_choices(int);
//Sed_cube *init_cube( GSList* , char* );
//GSList *init_epoch_list( GSList*symbol_table_list );
Sed_cube init_cube( Eh_key_file , char* );
GSList *init_epoch_list( Eh_key_file );
void destroy_epoch_list( GSList *epoch_list );
int print_header(FILE *fp);
int print_time(FILE *fp, int epoch_no, double year);
int print_footer(FILE *fp);
gboolean check_process_files( GSList *e_list );
Sed_process_list *init_process_list( Sed_process_list *pl , Epoch *cur_epoch );
Sed_process_list *create_process_list( void );
void destroy_process_list( Sed_process_list *pl );
Eh_project fill_sedflux_info_file( Eh_project p , int argc , char* argv[] );

void print_run_info_file( int argc , char *argv[] );

int run_sedflux(int argc, char *argv[])
{
   Sed_process_list *pl;
   GSList *e_list;
   Epoch *cur_epoch;
   Sed_cube prof;
   double n_years, year, time_step;
   int epoch_no, n_epochs;
   gboolean warn;
   char *in_file, *cpr_file;

   g_log_set_handler( NULL , G_LOG_LEVEL_MASK , &eh_logger , NULL );

   eh_require( argv )
   {
      Eh_args *args   = eh_opts_init( argc , argv );
      Eh_project proj = eh_create_project( "sedflux" );
      GLogLevelFlags ignore=0;
      char* working_dir = NULL;
      gboolean verbose;

      if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
         eh_exit(-1);

      warn        = eh_get_opt_bool( args , g_strdup("warn") , FALSE );
      in_file     = eh_get_opt_str ( args , g_strdup("in")   , NULL  );
      cpr_file    = eh_get_opt_str ( args , g_strdup("cpr")  , NULL  );
      working_dir = eh_get_opt_str ( args , g_strdup("wd")   , NULL  );
      verbose     = eh_get_opt_int ( args , g_strdup("v")    , 5     );

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

      if ( !in_file )
         in_file = get_file_name_interactively( &working_dir , &in_file );

      if ( working_dir )
      {
         if ( g_chdir( working_dir )!=0 )
            perror( working_dir );
      }
      else
         working_dir = g_get_current_dir();

      eh_set_project_dir( proj , working_dir );

      fill_sedflux_info_file( proj , argc , argv );
      eh_write_project_info_file( proj );

      eh_free           ( working_dir );
      eh_destroy_project( proj        );
      eh_destroy_args   ( args        );
   }

   signal(2,&print_choices);

   eh_debug( "Scan the init file" );
   {
      Eh_key_file init_file = eh_key_file_scan( in_file );

      eh_debug( "Initializing cube." );
      prof         = init_cube( init_file , cpr_file );

      eh_debug( "Reading epoch file." );
      e_list       = init_epoch_list( init_file );

      eh_key_file_destroy( init_file );
   }
/*
   if ( FALSE )
   {
   GHashTable *symbol_table = eh_scan_record_file( in_file );

   eh_debug( "Initializing cube." );
   prof         = init_cube( 
                     (GSList*)g_hash_table_lookup( symbol_table , "global" ) ,
                     cpr_file );


   eh_debug( "Reading epoch file." );
   e_list       = init_epoch_list( 
                     (GSList*)g_hash_table_lookup( symbol_table , "epoch"  ) );

   }
*/
   n_epochs = g_slist_length(e_list);

   print_header( stdout );

   eh_debug( "Checking consistancy of processes in epoch file." );
   if ( check_process_files( e_list )!=0 && warn )
      eh_exit(-1);

   eh_debug( "The main loop." );
   for ( epoch_no=0 ; epoch_no < n_epochs && !__quit_signal ; epoch_no++ )
   {
      // Create the processes.
      pl = create_process_list( );

      cur_epoch = (Epoch*)g_slist_nth_data(e_list,epoch_no);

      // Scan the process constants from a file.
      init_process_list( pl , cur_epoch );

      time_step = epoch_get_epoch_time_step(cur_epoch);
      n_years   = epoch_get_epoch_duration(cur_epoch);
      sed_cube_set_time_step( prof , time_step );

      for ( year=time_step ; year<=n_years && !__quit_signal ; year+=time_step )
      {

         print_time(stdout,epoch_no,year);

         // Set the constants for this time step.
         eh_debug( "Constants" );
         g_slist_foreach( pl->constants_l   , (GFunc)&sed_process_run , prof );

         // Set the weather.
         eh_debug( "River" );
         g_slist_foreach( pl->river_l       , (GFunc)&sed_process_run , prof );
         eh_debug( "Quake" );
         g_slist_foreach( pl->quake_l       , (GFunc)&sed_process_run , prof );
         eh_debug( "Tide" );
         g_slist_foreach( pl->tide_l        , (GFunc)&sed_process_run , prof );
         eh_debug( "Sea level" );
         g_slist_foreach( pl->sea_level_l   , (GFunc)&sed_process_run , prof );
         eh_debug( "Storm" );
         g_slist_foreach( pl->storm_l       , (GFunc)&sed_process_run , prof );
         eh_debug( "Avulsion" );
         g_slist_foreach( pl->avulsion_l    , (GFunc)&sed_process_run , prof );
         eh_debug( "Erosion" );
         g_slist_foreach( pl->erosion_l     , (GFunc)&sed_process_run , prof );

         // Add sediment to the system from each source.
         eh_debug( "Plume" );
         g_slist_foreach( pl->plume_l       , (GFunc)&sed_process_run , prof );
         eh_debug( "Bed load" );
         g_slist_foreach( pl->bedload_l     , (GFunc)&sed_process_run , prof );

         // Redistribute sediment.
         eh_debug( "Bottom boundary layer" );
         g_slist_foreach( pl->bbl_l         , (GFunc)&sed_process_run , prof );
         eh_debug( "Diffusion" );
         g_slist_foreach( pl->diffusion_l   , (GFunc)&sed_process_run , prof );
         eh_debug( "Xshore" );
         g_slist_foreach( pl->xshore_l      , (GFunc)&sed_process_run , prof );
         eh_debug( "Squall" );
         g_slist_foreach( pl->squall_l      , (GFunc)&sed_process_run , prof );
         eh_debug( "Failure" );
         g_slist_foreach( pl->failure_l     , (GFunc)&sed_process_run , prof );
         eh_debug( "Fluid flow" );
         g_slist_foreach( pl->flow_l        , (GFunc)&sed_process_run , prof );
         eh_debug( "Compaction" );
         g_slist_foreach( pl->compaction_l  , (GFunc)&sed_process_run , prof );
         eh_debug( "Isostasy" );
         g_slist_foreach( pl->isostasy_l    , (GFunc)&sed_process_run , prof );
         eh_debug( "Subsidence" );
         g_slist_foreach( pl->subsidence_l  , (GFunc)&sed_process_run , prof );
         eh_debug( "Tripod" );
         g_slist_foreach( pl->met_station_l , (GFunc)&sed_process_run , prof );
         eh_debug( "Data dump" );
         g_slist_foreach( pl->data_dump_l   , (GFunc)&sed_process_run , prof );
         eh_debug( "CPR" );
         g_slist_foreach( pl->cpr_l         , (GFunc)&sed_process_run , prof );

         if ( __dump_signal )
         {
            g_slist_foreach( pl->final_dump_l            ,
                             (GFunc)&sed_process_run_now ,
                             prof );
            __dump_signal = 0;
         }
         if ( __cpr_signal )
         {
            g_slist_foreach( pl->cpr_l                   ,
                             (GFunc)&sed_process_run_now ,
                             prof );
            __cpr_signal = 0;
         }

         sed_cube_set_age( prof ,   sed_cube_age(prof)
                                  + sed_cube_time_step(prof) );
         
         time_step = sed_cube_time_step_in_years( prof );
      }

      g_slist_foreach( pl->final_dump_l , (GFunc)&sed_process_run_now , prof );

      // Destroy the processes.
      destroy_process_list( pl );

      sed_cube_free_river( prof );
   }
   
   print_footer(stdout);
   
   destroy_epoch_list( e_list );

   sed_cube_destroy( prof );
   eh_heap_dump( "heap_dump.txt" );

   eh_exit(0);

   eh_require_not_reached();
   return -1;
}

char *get_file_name_interactively( char **working_dir , char **in_file )
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
   *in_file     = g_strdup( init_file );

   eh_free( cur_dir   );

   return init_file;
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

int print_header(FILE *fp)
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

   return 0;
}

int print_time(FILE *fp, int epoch_no, double year)
{
   fprintf( fp , "%.2f years (epoch %2d)\r" , year , epoch_no+1 );
   fflush( fp );
   return 0;
}

int print_footer(FILE *fp)
{
   time_t sedflux_end_time;
   sedflux_end_time = time(&sedflux_end_time);
   fprintf( fp , "End time is %s\n" , ctime(&sedflux_end_time) );
   fprintf( fp , "\n" );
   return 0;
}

void print_choices(int i)
{
   char ch;

   fprintf( stdout , "\n"                                          );
   fprintf( stdout , "(((\n"                                         );
   fprintf( stdout , "((( You have some choices.  They are these:\n" );
   fprintf( stdout , "(((  1) End run after this time step.\n"       );
   fprintf( stdout , "(((  2) Dump results and continue.\n"          );
   fprintf( stdout , "(((  3) Create a checkpoint.\n"                );
   fprintf( stdout , "(((  4) Continue.\n"                           );
   fprintf( stdout , "(((  5) Quit immediatly (without saving).\n"   );
   fprintf( stdout , "((( My choice is this: "                       );

   fscanf(stdin,"%s",&ch);

   if ( g_strcasecmp( &ch , "1" )==0 )
   {
      __quit_signal = 1;
      fprintf(stdout,"((( You have opted to quit early...\n\n");
   }
   else if ( g_strcasecmp( &ch , "2" )==0 )
   {
      __dump_signal = 1;
      signal(2,&print_choices);
      fprintf( stdout ,
               "((( Temporary output files will be dumped as soon as we can get to it...\n\n");
   }
   else if ( g_strcasecmp( &ch , "3" )==0 )
   {
      __cpr_signal = 1;
      signal(2,&print_choices);
      fprintf(stdout,"((( Creating a checkpoint/reset file...\n\n");
   }
   else if ( g_strcasecmp( &ch , "4" )==0 )
   {
      signal(2,&print_choices);
      fprintf(stdout,"((( Continuing...\n\n");
   }
   else
   {
      signal(2,&print_choices);
      fprintf(stdout,"((( Terminating run without saving...\n\n" );
      eh_exit(0);
   }
}

#include <time.h>

Eh_project fill_sedflux_info_file( Eh_project p , int argc , char* argv[] )
{
   //---
   // Define the description of the model run.
   //---
   eh_require( p )
   {
      char* default_str;
      char* desc_str;
      Eh_project temp = eh_create_project( eh_project_name(p) );

      eh_set_project_dir( temp , eh_project_dir_name(p) );
      eh_read_project_info_file( temp );

      default_str = eh_project_get_info_val( temp , "RUN DESCRIPTION" );
      desc_str    = eh_input_str( "RUN DESCRIPTION" , default_str );

      eh_project_add_info_val( p , "RUN DESCRIPTION" , desc_str );

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
   //---
   eh_require( argv[0] )
   {
      struct stat stat_buf;
      char mod_str[S_LINEMAX];
      GDate *today = g_date_new();

      g_stat( argv[0] , &stat_buf );
      g_date_set_time( today , time(&(stat_buf.st_mtime)) );
      g_date_strftime( mod_str , S_LINEMAX , "%d/%m/%Y" , today );

      eh_project_add_info_val( p , "CREATED" , mod_str );

      g_date_free( today );
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


