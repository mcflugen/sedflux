#include <glib.h>
#include <utils/utils.h>
#include "subside.h"
#include <sed/sed_sedflux.h>
#include <utils/utils.h>

// Command line arguments
/*
static gboolean _reset_bathy = FALSE;
static gboolean _version     = FALSE;
static gboolean _debug       = FALSE;
static gdouble  _day         = 1.;
static gdouble  _angle       = 14.;
static gchar*   _in_file     = NULL;
static gchar*   _out_file    = NULL;
static gchar*   _bathy_file  = NULL;
static gchar*   _flood_file  = NULL;
static gchar*   _data_file   = NULL;
static gint*    _data_id     = NULL;
static gint     _data_int    = 1;
*/

gboolean parse_data_list( const gchar* name , const gchar* value , gpointer data , GError** error );

static gint     _verbosity   = 0;
static gboolean _verbose     = FALSE;
static gboolean _version     = FALSE;
static double   _eet         = 5000.;
static double   _y           = 7.e10;
static double   _load        = 1000.;
static double   _slope       = .001;
static double   _rho_w       = 1030.;
static double   _rho_m       = 3300.;
static gint     _n_dim       = 1;

static GOptionEntry entries[] =
{
   { "eet"    , 'h' , 0 , G_OPTION_ARG_DOUBLE , &_eet   , "Effective elastic thickness" , "DVAL" } ,
   { "youngs" , 'y' , 0 , G_OPTION_ARG_DOUBLE , &_y     , "Young's modulus"             , "DVAL" } ,
   { "load"   , 'q' , 0 , G_OPTION_ARG_DOUBLE , &_load  , "Load"                        , "DVAL" } ,
   { "slope"  , 's' , 0 , G_OPTION_ARG_DOUBLE , &_slope , "Bathymetric slope"           , "DVAL" } ,
   { "rho-w"  , 'w' , 0 , G_OPTION_ARG_DOUBLE , &_rho_w , "Density of water (kg/m^3)"   , "DVAL" } ,
   { "rho-m"  , 'm' , 0 , G_OPTION_ARG_DOUBLE , &_rho_m , "Density of mantle (kg/m^3)"  , "DVAL" } ,
   { "ndim"   , 'D' , 0 , G_OPTION_ARG_INT    , &_n_dim , "Number of dimensions"        , "N"    } ,
   { "verbosity"   , 'l' , 0 , G_OPTION_ARG_INT          , &_verbosity   , "Verbosity level"            , "n"      } ,
   { "verbose"     , 'V' , 0 , G_OPTION_ARG_NONE         , &_verbose     , "Be verbose"                 , NULL     } ,
   { "version"     , 'v' , 0 , G_OPTION_ARG_NONE         , &_version     , "Version number"             , NULL     } ,
   { NULL }
};

gint
main( int argc , char *argv[] )
{
   GError*     error = NULL;
   Eh_dbl_grid z     = NULL;

   g_thread_init( NULL );
   eh_init_glib();

   { /* Parse command-line arguments */
      GOptionContext* context = g_option_context_new( "Run subsidence model." );

      g_option_context_add_main_entries( context , entries , NULL );

      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );
   }

   if ( _version )
   {
     gchar* prog_name = NULL;

     if      ( sizeof(void*)==8 )
       prog_name = g_strconcat( SUBSIDE_PROGRAM_NAME , " (64-bit)" , NULL );
     else if ( sizeof(void*)==4 )
        prog_name = g_strconcat( SUBSIDE_PROGRAM_NAME , " (32-bit)" , NULL );
     else
        eh_require_not_reached();

     eh_fprint_version_info( stdout          ,
                             prog_name       ,
                             SUBSIDE_MAJOR_VERSION ,
                             SUBSIDE_MINOR_VERSION ,
                             SUBSIDE_MICRO_VERSION );

     eh_free( prog_name );

     eh_exit( EXIT_SUCCESS );
   }

   { /* Set up the initial profile */
      gssize n_x = 1, n_y = 100;
      gssize dx  = 1, dy  = 10;

      z = eh_grid_new( double , n_x , n_y );

      eh_grid_set_x_lin( z , 0 , dx );
      eh_grid_set_y_lin( z , 0 , dy );

      eh_dbl_grid_set( z , 0. );
   }

   // Calculate deflection due to point load at origin
   subside_point_load( z , _load , _eet , _y , 0 , 0 );

   // Print the new elevations.
   {
      gint i, j;

      for ( i=0 ; i<eh_grid_n_x(z) ; i++ )
      {
         for ( j=0 ; j<eh_grid_n_y(z) ; j++ )
            fprintf( stdout , "%f " , eh_dbl_grid_val(z,i,j) );
         fprintf( stdout , "\n" );
      }

   }

   { // Free resources
      eh_grid_destroy( z , TRUE );
   }

   return EXIT_SUCCESS;
}

