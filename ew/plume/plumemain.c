#include "plumevars.h"
#include "plumeinput.h"
#include "plume_local.h"
#include "bmi_plume.h"

#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

static gchar*   _in_file    = NULL;
//static gchar*   _out_file   = NULL;
static gchar*   _flood_file = NULL;
static gchar*   _data_file  = NULL;
static gint     _n_dim      = 1;
static gboolean _rotate     = TRUE;
static gboolean _version    = FALSE;
static gboolean _verbose    = FALSE;

static GOptionEntry entries[] =
{
   { "in-file"    , 'i' , 0 , G_OPTION_ARG_FILENAME , &_in_file    , "Initialization file" , "<file>" } ,
//   { "out-file"   , 'o' , 0 , G_OPTION_ARG_FILENAME , &_out_file   , "Output file"         , "<file>" } ,
   { "flood-file" , 'f' , 0 , G_OPTION_ARG_FILENAME , &_flood_file , "Flood file"          , "<file>" } ,
   { "data-file"  , 'd' , 0 , G_OPTION_ARG_FILENAME , &_data_file  , "Data file"           , "<file>" } ,
   { "n-dim"      , 'D' , 0 , G_OPTION_ARG_INT      , &_n_dim      , "Number of dimensions" , "[1|2]" } ,
   { "no-rotate"  ,  0  , G_OPTION_FLAG_REVERSE , G_OPTION_ARG_NONE     , &_rotate     , "Do not rotate output" , NULL     } ,
   { "verbose"    , 'V' , 0 , G_OPTION_ARG_NONE     , &_verbose    , "Be verbose"          , NULL     } ,
   { "version"    , 'v' , 0 , G_OPTION_ARG_NONE     , &_version    , "Print version number and exit" , NULL     } ,
   { NULL }
};

gint
main(int argc, char *argv[])
{
   GError* error = NULL;

   //eh_init_glib();

   eh_set_verbosity_level (0);

   { /* Parse command line options */
      GOptionContext* context = g_option_context_new( "Run hypopycnal flow model." );
   
      g_option_context_add_main_entries( context , entries , NULL );
   
      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );
   }

   if ( _version )
   { /* Print version and exit */
      eh_fprint_version_info( stdout , "plume" , PLUME_MAJOR_VERSION ,
          PLUME_MINOR_VERSION , PLUME_MICRO_VERSION );
      eh_exit( 0 );
   }

   {
     FILE * fp = fopen ("plume_config.txt", "w");
     if (fp) {
       fprintf (fp, "%s %s", _in_file, _flood_file);
       fclose (fp);
     }
   }

   if (!error) {
     BMI_Model * model = g_new0(BMI_Model, 1);
     int err;

     register_bmi_plume(model);
      
     fprintf (stderr, "Initializing... ");
     err = model->initialize("plume_config.txt", &(model->self));
     if (err) {
       fprintf (stderr, "FAIL\n");
       fprintf (stderr, "Error: %d: Unable to initialize\n", err);
       return EXIT_FAILURE;
     }
     fprintf (stderr, "PASS\n");

     fprintf (stderr, "Updating... ");
     err = model->update(model->self);
     if (err) {
       fprintf (stderr, "FAIL\n");
       fprintf (stderr, "Error: %d: Unable to update\n", err);
       return EXIT_FAILURE;
     }
     fprintf (stderr, "PASS\n");

     fprintf (stderr, "Getting deposit... ");
     {
       double *z;
       int *shape;

       {
         int grid, size, rank;

         model->get_var_grid(model->self,
             "sea_bottom_sediment__deposition_rate", &grid);
         model->get_grid_size(model->self, grid, &size);
         z = g_new (double, size);

         model->get_grid_rank(model->self, grid, &rank);
         shape = g_new(int, rank);

         model->get_grid_shape(model->self, grid, shape);
       }

       model->get_value(model->self, "sea_bottom_sediment__deposition_rate", z);

       {
         int i, j;
         double *row = z;
         for (i=0; i<shape[0]; i++) {
           for (j=0; j<shape[1]; j++)
             fprintf (stdout,"%g ", row[j]);
           fprintf (stdout, "\n");
           row += shape[1];
         }
       }

       g_free (z);
       g_free (shape);
     }

     model->finalize(model->self);
   }

   return EXIT_SUCCESS;
}
