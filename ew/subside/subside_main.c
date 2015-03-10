#include <glib.h>
#include <utils/utils.h>

#include "bmi_subside.h"
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
  BMI_Model * model = NULL;

  g_thread_init( NULL );
  eh_init_glib();

  model = g_new0(BMI_Model, 1);
  register_bmi_subside(model);
  model->initialize(NULL, &(model->self));

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
    int grid;
    int size;
    int shape[2];
    double * load = NULL;

    model->get_var_grid(model->self, "earth_material_load__pressure", &grid);
    model->get_grid_shape(model->self, grid, shape);
    model->get_grid_size(model->self, grid, &size);

    model->get_value_ptr(model->self, "earth_material_load__pressure", &load);

    load[size / 2] = _load;

    model->update(model->self);

    { /* Print deflections */
      GError *error = NULL;
      void * dz;

      model->get_value_ptr(model->self, "lithosphere__increment_of_elevation", &dz);

      {
         int i, j;
         double *row = (double*)dz;
         int grid;
         int shape[2];

         model->get_var_grid(model->self, "lithosphere__increment_of_elevation", &grid);
         model->get_grid_shape(model->self, grid, shape);

         for (i=0; i<shape[0]; i++) {
           for (j=0; j<shape[1]; j++)
             fprintf (stdout,"%.3g ", row[j]);
           fprintf (stdout, "\n");
           row += shape[1];
         }
      }

      if (error)
        eh_error( "Error writing output file: %s" , error->message );
    }

    model->finalize(model->self);
  }

   return EXIT_SUCCESS;
}
