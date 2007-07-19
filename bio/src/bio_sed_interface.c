#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "bio.h"

Sed_cell* bio_array_to_cell_array( Sed_cell* c_arr , double** data , gint n_grains , gint n_layers );
double**  sed_array_to_bio_array ( Sed_cell* c_arr , gint* n_grains , gint* n_layers );

void
sed_column_bioturbate( Sed_column c , double depth , double k , double duration )
{
   eh_require( c          );
   eh_require( k>0        );
   eh_require( duration>0 );

   if ( c && depth>=0 )
   {
      double    z   = sed_column_top_height(c) - depth;
      Sed_cell* top = sed_column_extract_cells_above( c , z );

      if ( top )
      {
         gint     n_grains;
         gint     n_layers;
         double   dz   = sed_column_z_res( c );
         double** data = sed_array_to_bio_array( top , &n_grains , &n_layers );

         if ( data && n_layers>1 )
         {
            gint      i;
            Sed_cell* cell;

            bioturbate( data , n_grains , n_layers , dz , k , duration );

            bio_array_to_cell_array( top , data , n_grains , n_layers );

            eh_free_2( data );
         }
      }

      sed_column_stack_cells_loc( c , top );

      eh_free( top );
   }
}

double**
sed_array_to_bio_array( Sed_cell* col , gint* n_grains , gint* n_layers )
{
   double** data = NULL;

   eh_require( col      );
   eh_require( n_grains );
   eh_require( n_layers );

   if ( col )
   {
      gint i, n;

      *n_grains = sed_sediment_env_n_types();
      *n_layers = g_strv_length( (gchar**)col );

      for ( n=0 ; n<*n_grains ; n++ )
         for ( i=0 ; i<*n_layers ; i++ )
            data[n][i] = sed_cell_nth_amount( col[i] , n );
   }

   return data;
}

Sed_cell*
bio_array_to_cell_array( Sed_cell* c_arr , double** data , gint n_grains , gint n_layers )
{
   if ( data && c_arr )
   {
      gint i, n;
      double* t = eh_new( double , n_grains );

      eh_require( n_layers==g_strv_length( (gchar**)c_arr )    );
      eh_require( n_grains==sed_sediment_env_n_types() );

      for ( i=0 ; i<n_layers ; i++ )
      {
         for ( n=0 ; n<n_grains ; n++ )
            t[n] = data[n][i];

         sed_cell_set_amount( c_arr[i] , t );
      }

      eh_free( t );
   }

   return c_arr;
}


