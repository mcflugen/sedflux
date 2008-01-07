#include <glib.h>
#include <utils/utils.h>
#include "subside.h"

#define DEFAULT_RHO_W   (1030.)
#define DEFAULT_RHO_M   (3300.)
#define DEFAULT_EET     (5000.)
#define DEFAULT_Y       (7.e10)
#define DEFAULT_LOAD    (1000.)
#define DEFAULT_SLOPE   (.001)
#define DEFAULT_NDIM    (1)
#define DEFAULT_VERBOSE (FALSE)

static Eh_opt_entry entries[] =
{
   { "eet"    , 'h' , "Effective elastic thickness" , "VAL" , "5000" } ,
   { "youngs" , 'y' , "Young's modulus"             , "VAL" , "7e10" } ,
   { "load"   , 'q' , "Load"                        , "VAL" , "1e3"  } ,
   { "slope"  , 's' , "Bathymetric slope"           , "VAL" , ".001" } ,
   { "ndim"   , 'D' , "Number of dimensions"        , "VAL" , "1"    } ,
   { NULL }
};

int main( int argc , char *argv[] )
{
   Eh_opt_context opt;
   Eh_dbl_grid z;
   double eet, y, slope, load;
   gint n_dim;

   eh_init_glib();

   opt = eh_opt_create_context( "subside" ,
                                "flexural subsidence model" ,
                                "Show subsidence options" );
   opt = eh_opt_set_context( opt , entries );
   eh_opt_parse_context( opt , &argc , &argv , NULL );

   eh_opt_print_key_file( opt , stdout );

   eet    = eh_opt_dbl_value( opt , "eet" );
   y      = eh_opt_dbl_value( opt , "youngs" );
   slope  = eh_opt_dbl_value( opt , "slope" );
   load   = eh_opt_dbl_value( opt , "load" );
   n_dim  = eh_opt_int_value( opt , "ndim" );

/*
   verbose = DEFAULT_VERBOSE;
   opt_context = g_option_context_new( "flexural subsidence model" );
   g_option_context_add_main_entries( opt_context , entries , NULL );
   g_option_context_parse( opt_context , &argc , &argv , NULL );
*/

   // Set up the initial profile.
   {
      gssize n_x = 1, n_y = 100;
      gssize dx = 1, dy = 10;

      z = eh_grid_new( double , n_x , n_y );

      eh_grid_set_x_lin( z , 0 , dx );
      eh_grid_set_y_lin( z , 0 , dy );

      eh_dbl_grid_set( z , 0. );
   }

   // Subside due to external load.
   {
   }

   subside_point_load( z , load , eet , y , 0 , 0 );

   // Subside due to change in water load.

   // Print the new elevations.
   {
      gssize i, j;

      for ( i=0 ; i<eh_grid_n_x(z) ; i++ )
      {
         for ( j=0 ; j<eh_grid_n_y(z) ; j++ )
            fprintf( stdout , "%f " , eh_dbl_grid_val(z,i,j) );
         fprintf( stdout , "\n" );
      }

   }

   // Print the deflections.

   eh_destroy_context( opt );

   return 0;
}

