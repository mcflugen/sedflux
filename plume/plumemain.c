#include "plumevars.h"
#include "plumeinput.h"
#include "plume_local.h"

#include "glib.h"
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

static gchar*   in_file    = NULL;
static gchar*   out_file   = NULL;
static gchar*   flood_file = NULL;
static gchar*   data_file  = NULL;
static gboolean version    = FALSE;
static gboolean verbose    = FALSE;

static GOptionEntry entries[] =
{
   { "in-file"    , 'i' , 0 , G_OPTION_ARG_FILENAME , &in_file    , "Initialization file" , "<file>" } ,
   { "out-file"   , 'o' , 0 , G_OPTION_ARG_FILENAME , &out_file   , "Output file"         , "<file>" } ,
   { "flood-file" , 'f' , 0 , G_OPTION_ARG_FILENAME , &flood_file , "Flood file"          , "<file>" } ,
   { "data-file"  , 'd' , 0 , G_OPTION_ARG_FILENAME , &data_file  , "Data file"           , "<file>" } ,
   { "verbose"    , 'V' , 0 , G_OPTION_ARG_NONE     , &verbose    , "Be verbose"          , NULL     } ,
   { "version"    , 'v' , 0 , G_OPTION_ARG_NONE     , &version    , "Print version number and exit" , NULL     } ,
   { NULL }
};

gint
main(int argc, char *argv[])
{
   GError* error = NULL;

 //  g_thread_init( NULL );
   eh_init_glib();

   { /* Parse command line options */
      GOptionContext* context = g_option_context_new( "Run hypopycnal flow model." );
   
      g_option_context_add_main_entries( context , entries , NULL );
   
      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );
   }

   if ( version )
   { /* Print version and exit */
      eh_fprint_version_info( stdout , "plume" , PLUME_MAJOR_VERSION , PLUME_MINOR_VERSION , PLUME_MICRO_VERSION );
      eh_exit( 0 );
   }

   if ( !error )
   { /* Run the model */
      Sed_hydro*      flood_arr = NULL;
      Sed_hydro*      r         = NULL;
      Plume_param_st* param     = NULL;
      double**        deposit   = NULL;
      gint            len       = 0;
      gint            n_grains  = 0;

      if ( !error ) param     = plume_scan_parameter_file( in_file    , &error );
      if ( !error ) flood_arr = sed_hydro_scan           ( flood_file , &error );

      if ( !error )
         for ( r=flood_arr ; *r ; r++ )
         {
            if ( verbose ) sed_hydro_fprint( stderr , *r );

            deposit = plume_wrapper( *r , param , &len , &n_grains );
            if ( data_file ) plume_print_data( data_file , deposit , len , n_grains );

            eh_free_2( deposit );
         }

      sed_hydro_array_destroy( flood_arr );

      eh_exit_on_error( error , "sakura" );
   }

   return EXIT_SUCCESS;
}

