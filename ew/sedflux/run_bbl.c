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

#define EH_LOG_DOMAIN BBL_PROCESS_NAME_S

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"

#include "sedflux.h"

double   add_sediment_from_external_source( Sed_cube     p      ,
                                            Eh_sequence* seq    ,
                                            double       start  ,
                                            double       finish );
int      rain_sediment_3                  ( Sed_cube p    , int      algorithm , Sed_riv  r     );
gboolean init_bbl_data                    ( Sed_process p , Sed_cube prof      , GError** error );

Sed_process_info
run_bbl( Sed_process p , Sed_cube prof )
{
   Bbl_t*           data = sed_process_user_data(p);
   Sed_process_info info = SED_EMPTY_INFO;
   gint             n_rivers;

   if ( sed_process_run_count(p)==0 )
      init_bbl_data( p , prof , NULL );

   if ( data->src_seq )
   {
      info.mass_added =
         add_sediment_from_external_source( prof            ,
                                            data->src_seq   ,
                                            data->last_year ,
                                            sed_cube_age_in_years(prof) );
      data->last_year = sed_cube_age_in_years( prof );
   }

   n_rivers = sed_cube_n_rivers( prof );

   info.mass_lost = 0.;
   if ( n_rivers>0 )
   {
      Sed_riv* all = sed_cube_all_branches( prof );
      Sed_riv* r;

      if ( all )
      {
         for ( r=all ; *r ; r++ )
         {
            eh_debug( "Depositing sediment for river: %s" , sed_river_name_loc(*r) );
            rain_sediment_3( prof , data->algorithm , *r );
         }

         info.mass_lost += sed_cube_mass_in_suspension( prof );

         // remove any remaining suspended sediment from the model.
         for ( r=all ; *r ; r++ )
            sed_cell_grid_clear( sed_cube_in_suspension( prof , *r ) );

         eh_free( all );
      }
   }

   eh_message( "time : %f" , sed_cube_age_in_years( prof ) );

   return info;
}

#define BBL_ALGORITHM_NONE    (0)
#define BBL_ALGORITHM_MUDS    (1)

#define BBL_KEY_ALGORITHM     "algorithm"
#define BBL_KEY_SOURCE_FILE   "external sediment source file"

static gchar* bbl_req_labels[] =
{
   BBL_KEY_ALGORITHM   ,
   BBL_KEY_SOURCE_FILE ,
   NULL
};

gboolean
init_bbl( Sed_process p , Eh_symbol_table t , GError** error )
{
   Bbl_t*   data    = sed_process_new_user_data( p , Bbl_t );
   GError*  tmp_err = NULL;
   gboolean is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );
   eh_require( t );

   data->src_seq   = NULL;
   data->last_year = 0.;

   if ( eh_symbol_table_require_labels( t , bbl_req_labels , &tmp_err ) )
   {
      gchar* src_file = eh_symbol_table_value ( t , BBL_KEY_SOURCE_FILE );
      gchar* key      = eh_symbol_table_lookup( t , BBL_KEY_ALGORITHM   );

      if ( g_ascii_strcasecmp( src_file , "NONE" )==0 )
      {
         eh_free( src_file );
         src_file = NULL;
      }

      data->src_file = src_file;

      if      ( g_ascii_strcasecmp( key , "MUDS" )==0 ) data->algorithm = BBL_ALGORITHM_MUDS;
      else if ( g_ascii_strcasecmp( key , "NONE" )==0 ) data->algorithm = BBL_ALGORITHM_NONE;
      else
         g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_ALGORITHM ,
                      "Invalid bbl algorithm (muds or none): %s" , key );

      if ( data->algorithm==BBL_ALGORITHM_MUDS && sed_mode_is_3d() )
      {
         eh_warning( "Sedflux3D requires bbl algorithm to be 'NONE'." );
         data->algorithm = BBL_ALGORITHM_NONE;
      }
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_bbl_data( Sed_process p , Sed_cube prof , GError** error )
{
   gboolean is_ok = TRUE;
   Bbl_t*   data  = sed_process_user_data( p );

   if ( data )
   {
      GError* tmp_err = NULL;
      double* y       = sed_cube_y( prof , NULL );

      if ( data->src_file )
      {
         if ( sed_mode_is_3d() )
            data->src_seq  = sed_get_floor_sequence_3( data->src_file         ,
                                                       sed_cube_x_res( prof ) ,
                                                       sed_cube_y_res( prof ) , 
                                                       &tmp_err );
         else
            data->src_seq  = sed_get_floor_sequence_2( data->src_file ,
                                                       y              ,
                                                       sed_cube_n_y(prof) ,
                                                       &tmp_err );
      }
      else
         data->src_seq = NULL;

      data->last_year = sed_cube_age_in_years(prof);

      eh_free( y );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         is_ok = FALSE;
      }
   }

   return is_ok;
}

gboolean
destroy_bbl( Sed_process p )
{
   if ( p )
   {
      Bbl_t* data = sed_process_user_data( p );

      if ( data )
      {
         if ( data->src_seq )
         {
            gint i;
            for ( i=0 ; i<data->src_seq->len ; i++ )
               eh_grid_destroy( data->src_seq->data[i] , TRUE );

            eh_destroy_sequence( data->src_seq , FALSE );
         }

         eh_free( data );
      }
   }
   return TRUE;
}

double add_sediment_from_external_source( Sed_cube p       ,
                                          Eh_sequence* seq ,
                                          double start     ,
                                          double finish )
{
   double mass_added = 0;
   Sed_cell deposit_cell;
   double time_step;

   eh_require( p   );

   eh_require( seq && seq->len>0 )
   {
      Eh_dbl_grid g;
      gssize i;
      for ( i=0 ; i<seq->len ; i++ )
      {
         g = seq->data[i];

         eh_require( sed_cube_n_x(p)==eh_grid_n_x(g) );
         eh_require( sed_cube_n_y(p)==eh_grid_n_y(g) );
      }
   }

   eh_require( start<=finish );

   time_step = finish-start;

   deposit_cell = sed_cell_new_env( );
   sed_cell_set_equal_fraction( deposit_cell );

   //---
   // Add sediment to the basin.  If there is only one record, assume that 
   // sedimentation is constant with time.  If there are multiple records
   // integrate the sedimentation over the time step.
   //
   // Note that the time step will be 0 at the beginning of an epoch, in
   // this case, don't do anything.
   //---
   if ( seq->len == 1 && time_step>1e-6 )
   {
      double **dzdt = eh_dbl_grid_data( seq->data[0] );
      double h;
      gssize i, j;

      for ( i=0 ; i<sed_cube_n_x(p) ; i++ )
         for ( j=0 ; j<sed_cube_n_y(p) ; j++ )
         {
            h = eh_min( dzdt[i][j]*time_step ,
                        sed_cube_water_depth(p,i,j) );

            if ( h>0 )
            {
               sed_cell_resize( deposit_cell , h );

               mass_added += sed_cell_mass(deposit_cell);

               sed_column_add_cell( sed_cube_col_ij(p,i,j) , deposit_cell );
            }
         }
   }
   else if ( time_step>1e-6 )
   {
      double **dzdt = eh_dbl_grid_data( seq->data[0] );
      gssize i, j, n;
      double h, total_time, lower_edge, upper_edge;

      for ( n=0,total_time=0 ; n<seq->len ; n++ )
      {
         lower_edge = seq->t[n];
         if ( n<seq->len-1 )
            upper_edge = seq->t[n+1];
         else
            upper_edge = G_MAXDOUBLE;

         if ( start >= upper_edge )
            time_step = -1;
         else if ( start >= lower_edge && finish <  upper_edge )
            time_step = finish - start;
         else if ( start < lower_edge && finish >= upper_edge )
            time_step = upper_edge - lower_edge;
         else if ( start >= lower_edge && finish >= upper_edge )
            time_step = upper_edge - start;
         else if ( start <  lower_edge && finish <  upper_edge )
            time_step = finish - lower_edge;
         else
            time_step = -1;

         if ( time_step > 0 )
         {
            total_time += time_step;
            dzdt        = eh_dbl_grid_data( seq->data[n] );
            for ( i=0 ; i<sed_cube_n_x(p) ; i++ )
               for ( j=0 ; j<sed_cube_n_y(p) ; j++ )
               {
                  h = eh_min( dzdt[i][j]*time_step ,
                              sed_cube_water_depth(p,i,j) );

                  if ( h>0 )
                  {
                     sed_cell_resize( deposit_cell , h );

                     mass_added += sed_cell_mass(deposit_cell);

                     sed_column_add_cell( sed_cube_col_ij(p,i,j) , deposit_cell );
                  }

               }
         }
      }

      if ( fabs( total_time - (finish-start) ) > 1e-5 )
      {
         eh_warning( "The current time interval is not completely contained "
                     "within the sequence." );
         eh_warning( "Start of this time interval: %f" , start );
         eh_warning( "End of this time interval: %f" , finish );
         eh_warning( "Total time: %f" , total_time );
      }
   }

   mass_added *= sed_cube_x_res(p)*sed_cube_y_res(p);

   sed_cell_destroy( deposit_cell );

   return mass_added;
}

