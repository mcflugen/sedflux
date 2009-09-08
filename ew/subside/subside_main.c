#include <glib.h>
#include <utils/utils.h>
#include "subside_api.h"
#include <sed/sed_sedflux.h>
#include <utils/utils.h>

#define SUBSIDE_PROGRAM_NAME "subside"
#define SUBSIDE_MAJOR_VERSION 0
#define SUBSIDE_MINOR_VERSION 1
#define SUBSIDE_MICRO_VERSION 0

// Command line arguments
static double _eet   = 5000.;
static double _y     = 7.e10;
static double _load  = 1000.;
static double _width_x  = 100000.;
static double _width_y  = 100000.;
static double _slope = .001;
static double _rho_w = 1030.;
static double _rho_m = 3300.;
static int    _nx = 200;
static int    _ny = 200;
static gint   _n_dim = 1;

static gboolean _version = FALSE;

static GOptionEntry entries[] =
{
   { "eet"    , 'h' , 0 , G_OPTION_ARG_DOUBLE , &_eet   , "Effective elastic thickness" , "DVAL" } ,
   { "youngs" , 'y' , 0 , G_OPTION_ARG_DOUBLE , &_y     , "Young's modulus"             , "DVAL" } ,
   { "load"   , 'q' , 0 , G_OPTION_ARG_DOUBLE , &_load  , "Load"                        , "DVAL" } ,
   { "width-x"   , 'x' , 0 , G_OPTION_ARG_DOUBLE , &_width_x  , "X extent (m)"                        , "WIDTH" } ,
   { "width-y"   , 'y' , 0 , G_OPTION_ARG_DOUBLE , &_width_y  , "Y extent (m)"                        , "WIDTH" } ,
   { "slope"  , 's' , 0 , G_OPTION_ARG_DOUBLE , &_slope , "Bathymetric slope"           , "DVAL" } ,
   { "nx"   , 0 , 0 , G_OPTION_ARG_INT    , &_nx , "Number of nodes in x direction"        , "N"    } ,
   { "ny"   , 0 , 0 , G_OPTION_ARG_INT    , &_ny , "Number of nodes in y direction"        , "N"    } ,
   { "rho-w"  , 'w' , 0 , G_OPTION_ARG_DOUBLE , &_rho_w , "Density of water (kg/m^3)"   , "DVAL" } ,
   { "rho-m"  , 'm' , 0 , G_OPTION_ARG_DOUBLE , &_rho_m , "Density of mantle (kg/m^3)"  , "DVAL" } ,
   { "ndim"   , 'D' , 0 , G_OPTION_ARG_INT    , &_n_dim , "Number of dimensions"        , "N"    } ,
   { "version"   , 'v' , 0 , G_OPTION_ARG_NONE    , &_version , "Print version number and exit"        , NULL    } ,
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
      eh_error( "Error parsing command line arguments: %s" ,
                error->message );
  }

  if ( _version )
  {
    gchar* prog_name = NULL;

    prog_name = g_strdup_printf ("%s (%d-bit)",
                                 SUBSIDE_PROGRAM_NAME,
                                 sizeof(void*)*8);

    eh_fprint_version_info( stdout          ,
                            prog_name       ,
                            SUBSIDE_MAJOR_VERSION ,
                            SUBSIDE_MINOR_VERSION ,
                            SUBSIDE_MICRO_VERSION );

    eh_free( prog_name );

    eh_exit (EXIT_SUCCESS);
  }

  { /* Run the model */
    Subside_state* state = NULL;
    const int n_x = _nx;
    const int n_y = _ny;
    const double dx = _width_x / n_x;
    const double dy = _width_y / n_y;

    state = sub_init (n_x, n_y, dx, dy);

    sub_set_eet (state, _eet);
    sub_set_youngs (state, _y);
    sub_set_load_at (state, _load, n_x/2, n_y/2);

    sub_run (state, 1e6);

    { /* Print deflections */
      GError *error = NULL;
      int len[3] = {1, n_x, n_y};
      double size[3] = {1, dx, dy};
      const double* dz = sub_get_deflection (state);
      const double* y = sub_get_y (state);
     
      eh_bov_print ("deflections", dz, "Deflection (m)", len, size, &error);
      //eh_curve2d_print ("deflections.csv", y, dz, "Deflection (m)", len, &error);

      if (error)
        eh_error( "Error writing output file: %s" , error->message );
    }

    sub_destroy (state);
  }

   return EXIT_SUCCESS;
}

