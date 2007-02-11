#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "sedflux.h"

Eh_project fill_sedflux_info_file             ( Eh_project p         ,
                                                const gchar* cmd_str ,
                                                const gchar* desc    );
gchar*     sedflux_get_file_name_interactively( gchar **working_dir ,
                                                gchar **in_file     );

/* Command line options */
static gboolean mode_3d      = FALSE;
static gboolean mode_2d      = FALSE;
static gchar*   init_file    = NULL;
static gchar*   out_file     = NULL; 
static gchar*   working_dir  = NULL;
static gchar*   run_desc     = NULL;
static gboolean just_plume   = FALSE;
static gboolean just_rng     = FALSE;
static gboolean summary      = FALSE;
static gboolean warn         = FALSE;
static gint     verbosity    = -1;
static gboolean verbose      = FALSE;
static gboolean version      = FALSE;
static char**   active_procs = NULL;

/* Define the command line options */
static GOptionEntry command_line_entries[] =
{
   { "3-dim"       , '3' , 0 , G_OPTION_ARG_NONE         , &mode_3d     , "Run in 3D mode"             , NULL     } ,
   { "2-dim"       , '2' , 0 , G_OPTION_ARG_NONE         , &mode_2d     , "Run in 2D mode"             , NULL     } ,
   { "init-file"   , 'i' , 0 , G_OPTION_ARG_FILENAME     , &init_file   , "Initialization file"        , "<file>" } ,
   { "out-file"    , 'o' , 0 , G_OPTION_ARG_FILENAME     , &out_file    , "Output file"                , "<file>" } ,
   { "working-dir" , 'd' , 0 , G_OPTION_ARG_FILENAME     , &working_dir , "Working directory"          , "<dir>"  } ,
   { "msg"         , 'm' , 0 , G_OPTION_ARG_STRING       , &run_desc    , "Run description"            , "<msg>"  } ,
   { "active-proc" , 'a' , 0 , G_OPTION_ARG_STRING_ARRAY , &active_procs, "Specify active process"     , "<name>" } ,
   { "just-plume"  , 'p' , 0 , G_OPTION_ARG_NONE         , &just_plume  , "Run just the plume"         , NULL     } ,
   { "just-rng"    , 'r' , 0 , G_OPTION_ARG_NONE         , &just_rng    , "Run just the rng processes" , NULL     } ,
   { "summary"     , 's' , 0 , G_OPTION_ARG_NONE         , &summary     , "Print a summary and quit"   , NULL     } ,
   { "warn"        , 'w' , 0 , G_OPTION_ARG_NONE         , &warn        , "Print warnings"             , NULL     } ,
   { "verbosity"   , 'l' , 0 , G_OPTION_ARG_INT          , &verbosity   , "Verbosity level"            , "n"      } ,
   { "verbose"     , 'V' , 0 , G_OPTION_ARG_NONE         , &verbose     , "Be verbose"                 , NULL     } ,
   { "version"     , 'v' , 0 , G_OPTION_ARG_NONE         , &version     , "Version number"             , NULL     } ,
   { NULL }
};

static gchar* just_plume_procs[] = { "plume"      , "river"  ,  "bbl" , NULL }; //< Processes to run with just-plume option
static gchar* just_rng_procs[]   = { "earthquake" , "storms" , NULL };          //< Process to run with just-rng option

Sedflux_param_st*
sedflux_parse_command_line( int argc , char *argv[] , GError** error )
{
   Sedflux_param_st* p = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );
   eh_require( argv );

   {
      GError*         tmp_err = NULL;
      GOptionContext* context = g_option_context_new( "Run basin filling model sedflux-2.0" );

      g_option_context_add_main_entries( context , command_line_entries , NULL );

      g_option_context_parse( context , &argc , &argv , &tmp_err );

      if ( (mode_3d && mode_2d) && !tmp_err )
         g_set_error( &tmp_err ,
                      SEDFLUX_ERROR ,
                      SEDFLUX_ERROR_MULTIPLE_MODES ,
                      "Mode must be either 2D or 3D" );

      if ( version )
      {
         eh_fprint_version_info( stdout          ,
                                 PROGRAM_NAME    ,
                                 S_MAJOR_VERSION ,
                                 S_MINOR_VERSION ,
                                 S_MICRO_VERSION );
         eh_exit( EXIT_SUCCESS );
      }

      if ( !tmp_err )
      {
         if ( mode_3d )
            sed_mode_set( SEDFLUX_MODE_3D );
         else
            sed_mode_set( SEDFLUX_MODE_2D );

         if      ( verbosity >= 0 )
            eh_set_verbosity_level( verbosity );
         else if ( verbose )
            eh_set_verbosity_level( 4 );
         else
            eh_set_verbosity_level( 0 );

         if ( !active_procs )
         {
            if ( just_plume )
               active_procs = just_plume_procs;
            if ( just_rng )
               active_procs = just_rng_procs;
         }

         g_option_context_free( context );

         p = eh_new( Sedflux_param_st , 1 );

         p->init_file    = init_file;
         p->out_file     = out_file;
         p->working_dir  = working_dir;
         p->run_desc     = run_desc;
         p->just_plume   = just_plume;
         p->just_rng     = just_rng;
         p->summary      = summary;
         p->warn         = warn;
         p->verbosity    = verbosity;
         p->verbose      = verbose;
         p->version      = version;
         p->active_procs = active_procs;
      }
      else
         g_propagate_error( error , tmp_err );
   }

   return p;
}

GQuark sedflux_error_quark( void )
{
   return g_quark_from_static_string( "sedflux-error-quark" );
}

gboolean
sedflux_setup_project_dir( gchar** init_file , gchar** working_dir , GError** error )
{
   gboolean rtn_val = TRUE;
   GError*  tmp_err = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   /* If not given, prompt the user to supply the initialization file
      and working directory. */
   if ( !(*init_file) )
      sedflux_get_file_name_interactively( working_dir , init_file );

   /* Create the working directory */
   if ( g_mkdir_with_parents( *working_dir , 0 ) == -1 )
      g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_DIR , strerror(errno) );

   /* Make sure the working directory is writable */
   if ( !tmp_err && g_access( *working_dir , R_OK|W_OK|X_OK ) == -1 )
      g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_DIR , strerror(errno) );

   /* Move to the working directory */
   if ( !tmp_err && g_chdir( *working_dir )!=0 )
      g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_DIR , strerror(errno) );

   /* Make sure the initialization file is readable */
   if ( !tmp_err && g_access( *init_file , R_OK ) == -1 )
      g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_INIT_FILE , strerror(errno) );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      rtn_val = FALSE;
   }

   return rtn_val;
}

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

gchar*
sedflux_get_file_name_interactively( gchar **working_dir , gchar **in_file )
{
   char *dir       = eh_new( char , 2048 );
   char *cur_dir   = eh_new( char , 2048 );
   char *init_file = g_strdup( "basin.init" );

   eh_require( working_dir );
   eh_require( in_file     );

   eh_print_message( stderr , copyleft_msg );

   fprintf( stderr , "---\n" );
   fprintf( stderr , "--- This is %s" , PROGRAM_NAME );

   if ( sed_mode_is_3d() )
      fprintf( stderr , "3d" );

   eh_fprint_version_info( stderr          ,
                           PROGRAM_NAME    ,
                           S_MAJOR_VERSION ,
                           S_MINOR_VERSION ,
                           S_MICRO_VERSION );
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

gint
sedflux_print_info_file( const gchar* init_file , const gchar* wd , const gchar* cmd_str , const gchar* desc )
{
   gint n = 0;

   {
      Eh_project proj = eh_create_project( "sedflux" );

      eh_set_project_dir    ( proj , wd );
      fill_sedflux_info_file( proj , cmd_str , desc );

      n = eh_write_project_info_file( proj );

      eh_destroy_project    ( proj        );
   }
   return n;
}

Eh_project
fill_sedflux_info_file( Eh_project p , const gchar* cmd_str , const gchar* desc )
{
   gchar* command = NULL;
   gchar* options = NULL;

   eh_require( p       );
   eh_require( cmd_str );

   {
      gchar** str_array = NULL;

      str_array = g_strsplit( cmd_str , " " , 2 );

      command = str_array[0];
      options = str_array[1];

      eh_free( str_array );
   }

   if ( p )
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
   eh_require( command )
   {
      struct stat stat_buf;
      char* mod_str = eh_new( gchar , S_LINEMAX );
      GDate *today = g_date_new();

      g_stat( command , &stat_buf );
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
   eh_require( options )
   {
      int i;

      eh_require( command );
      eh_require( options );

      eh_project_add_info_val( p , "PROGRAM" , command );
      eh_project_add_info_val( p , "COMMAND-LINE OPTIONS" , options );
   }

   eh_free( command );
   eh_free( options );

   return p;
}

