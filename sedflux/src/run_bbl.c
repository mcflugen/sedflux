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

#define SED_BBL_PROC_NAME "bbl"
#define EH_LOG_DOMAIN SED_BBL_PROC_NAME

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "processes.h"
#include "bbl.h"

double add_sediment_from_external_source( Sed_cube p       ,
                                          Eh_sequence* seq ,
                                          double start     ,
                                          double finish );
int rain_sediment_3( Sed_cube p , int algorithm , int river_no );

Sed_process_info run_bbl(gpointer ptr, Sed_cube prof)
{
   Bbl_t *data=(Bbl_t*)ptr;
   int i;
   double total_mass, init_mass;
   int n_rivers;
   gboolean error=FALSE;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         gint i;
         for ( i=0 ; i<data->src_seq->len ; i++ )
            eh_grid_destroy( data->src_seq->data[i] , TRUE );
         eh_destroy_sequence( data->src_seq , FALSE );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      double *y = sed_cube_y( prof , NULL );

      if ( data->src_file )
      {
         GError* err = NULL;
         if ( is_sedflux_3d() )
            data->src_seq  = sed_get_floor_sequence_3( data->src_file         ,
                                                       sed_cube_x_res( prof ) ,
                                                       sed_cube_y_res( prof ) );
         else
            data->src_seq  = sed_get_floor_sequence_2( data->src_file ,
                                                       y              ,
                                                       sed_cube_n_y(prof) ,
                                                       &err );

         if ( err )
         {
            fprintf( stderr , "Unable to read subsidence sequence file: %s\n" , err->message );
            eh_exit(-1);
         }
      }
      else
         data->src_seq = NULL;

      eh_free( y );

      data->last_year = sed_cube_age_in_years(prof);
      data->initialized = TRUE;
   }

   if ( data->src_seq )
   {
      info.mass_added =
         add_sediment_from_external_source( prof            ,
                                            data->src_seq   ,
                                            data->last_year ,
                                            sed_cube_age_in_years(prof) );
      data->last_year = sed_cube_age_in_years( prof );
   }

   n_rivers = sed_cube_number_of_rivers( prof );

//   init_mass = sed_cube_mass( prof );

   info.mass_lost = 0.;
   if ( n_rivers>0 )
   {

      for ( i=0 ; i<n_rivers ; i++ )
         error |= rain_sediment_3( prof , data->algorithm , i );

      info.mass_lost += sed_cube_mass_in_suspension( prof );

      // remove any remaining suspended sediment from the model.
      for ( i=0 ; i<n_rivers ; i++ )
         sed_cell_grid_clear( sed_cube_in_suspension( prof , i ) );
   }

//   total_mass = sed_cube_mass( prof );

   eh_message( "time                          : %f" , sed_cube_age_in_years( prof ) );
//   eh_message( "sediment added (kg)           : %g" , total_mass - init_mass );
//   eh_message( "total mass of the profile (kg): %g" , total_mass );

   return info;
}

#define S_KEY_ALGORITHM     "algorithm"
#define S_KEY_SOURCE_FILE   "external sediment source file"

gboolean init_bbl(Eh_symbol_table symbol_table,gpointer ptr)
{
   Bbl_t *data=(Bbl_t*)ptr;

   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   eh_require( symbol_table )
   {
      gchar* src_file = eh_symbol_table_value( symbol_table ,
                                               S_KEY_SOURCE_FILE );

      if ( g_ascii_strcasecmp( src_file , "NONE" )==0 )
      {
         eh_free( src_file );
         src_file = NULL;
      }

      data->src_file = src_file;
   }

   eh_require( symbol_table )
   {
      char *key = eh_symbol_table_lookup( symbol_table , S_KEY_ALGORITHM );

      if ( strcasecmp( key , "MUDS" )==0 )
      {
         data->algorithm = BBL_ALGORITHM_MUDS;
         if ( is_sedflux_3d() )
         {
            eh_warning( "Sedflux3D requires bbl algorithm to be 'NONE'." );
            data->algorithm = BBL_ALGORITHM_NONE;
         }
      }
      else if ( strcasecmp( key , "NONE" )==0 )
         data->algorithm = BBL_ALGORITHM_NONE;
      else
      {
         eh_error( "Unknown key word: %s.\n" , key );
         return FALSE;
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

