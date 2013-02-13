#include "plumevars.h"
#include "plumeinput.h"
#include "plume_local.h"
#include "plume_bmi.h"

#include "glib.h"
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

   eh_init_glib();

   eh_set_verbosity_level (0);

   { /* Parse command line options */
      GOptionContext* context = g_option_context_new( "Run hypopycnal flow model." );
   
      g_option_context_add_main_entries( context , entries , NULL );
   
      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );
   }

   if ( _version )
   { /* Print version and exit */
      eh_fprint_version_info( stdout , "plume" , PLUME_MAJOR_VERSION , PLUME_MINOR_VERSION , PLUME_MICRO_VERSION );
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
     BMI_Model * model = NULL;
     int err;

     fprintf (stderr, "Initializing... ");
     err = BMI_PLUME_Initialize ("plume_config.txt", &model);
     if (err) {
       fprintf (stderr, "FAIL\n");
       fprintf (stderr, "Error: %d: Unable to initialize\n", err);
       return EXIT_FAILURE;
     }
     fprintf (stderr, "PASS\n");

     fprintf (stderr, "Updating... ");
     err = BMI_PLUME_Update (model);
     if (err) {
       fprintf (stderr, "FAIL\n");
       fprintf (stderr, "Error: %d: Unable to update\n", err);
       return EXIT_FAILURE;
     }
     fprintf (stderr, "PASS\n");

     fprintf (stderr, "Getting deposit... ");
     {
       double *z;
       double *row;
       int size;
       int i, j, n;
       int shape[2];
       int n_grains;
       char * var_name = NULL;

       BMI_PLUME_Get_value (model, "grain_class__count", &n_grains);

       for (n=0; n<n_grains; n++) {
         var_name = g_strdup_printf ("grain_class_%d__deposition_rate", n);

         BMI_PLUME_Get_var_point_count (model, var_name, &size);
         z = g_new (double, size);

         BMI_PLUME_Get_double (model, var_name, z);

         BMI_PLUME_Get_grid_shape (model, var_name, shape);

         row = z;
         for (i=0; i<shape[0]; i++) {
           for (j=0; j<shape[1]; j++)
             fprintf (stdout,"%g ", row[j]);
           fprintf (stdout, "\n");
           row += shape[1];
         }

         g_free (z);
         g_free (var_name);
       }
     }

     BMI_PLUME_Finalize (model);
   }

   return EXIT_SUCCESS;

   if ( !error )
   { /* Run the model */
      Sed_hydro*      flood_arr = NULL;
      Sed_hydro*      r         = NULL;
      Plume_param_st* param     = NULL;
      Eh_dbl_grid*    dep_grid  = NULL;
      gint            len       = 0;
      gint            n_grains  = 0;

      if ( !error ) param     = plume_scan_parameter_file( _in_file    , &error );
      if ( !error ) flood_arr = sed_hydro_scan           ( _flood_file , &error );

      if ( !error )
      {
         param->n_dim  = _n_dim;
         param->rotate = _rotate;

         for ( r=flood_arr ; *r ; r++ )
         { /* Run each of the flood events */
            if ( _verbose ) sed_hydro_fprint( stderr , *r );

            dep_grid = plume_wrapper( *r , param , &len , &n_grains );

            eh_message( "Write output grid: %s" , _data_file );
            if ( _data_file ) plume_print_data( _data_file , dep_grid , len , n_grains );

            eh_message( "Finished.  Cleaning up." );
            {
               Eh_dbl_grid* d;

               for ( d=dep_grid ; *d ; d++ )
                  eh_grid_destroy( *d , TRUE );

               eh_free( dep_grid );
            }
         }
      }

      sed_hydro_array_destroy( flood_arr );

      eh_exit_on_error( error , "plume" );
   }

   return EXIT_SUCCESS;
}

