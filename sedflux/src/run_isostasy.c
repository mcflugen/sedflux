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

#define SED_ISOSTASY_PROC_NAME "isostasy"
#define EH_LOG_DOMAIN SED_ISOSTASY_PROC_NAME

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "isostasy.h"
#include "processes.h"

void subside_point_load      ( Eh_dbl_grid g , double load , double h ,
                               double E       , int i_load  , int j_load );
void subside_half_plane_load ( Eh_dbl_grid g , double load , double h , double E );
double get_flexure_parameter ( double h       , double E    , gssize n_dim );

Sed_process_info run_isostasy( gpointer ptr , Sed_cube prof )
{
   Isostasy_t *data=(Isostasy_t*)ptr;
   double full_n_x, full_n_y;
   int small_n_x, small_n_y;
   double x_reduction, y_reduction;
   double C;
   double time, time_step;
   Eh_dbl_grid dw_iso;
   double total_dw = 0;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         eh_grid_destroy( data->last_dw_iso , TRUE );
         eh_grid_destroy( data->last_load   , TRUE );

         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( sed_mode_is_3d() )
   {
      x_reduction = .2;
      y_reduction = .2;
   }
   else
   {
      x_reduction = 1;
      y_reduction = .2;
   }

   if ( sed_mode_is_3d() )
      C = sed_cube_x_res( prof )
        * sed_cube_y_res( prof )
        / x_reduction
        / y_reduction;
   else
      C = sed_cube_y_res( prof )
        / x_reduction
        / y_reduction;

   if ( !data->initialized )
   {
      data->last_dw_iso = eh_grid_new( double , sed_cube_n_x(prof) , sed_cube_n_y(prof) );
      data->last_load   = sed_cube_load_grid( prof , NULL );
      eh_dbl_grid_scalar_mult( data->last_load , C );
      data->last_half_load = sed_cube_water_pressure( prof , 0 , sed_cube_n_y(prof)-1 );

      data->last_time     = sed_cube_age_in_years(prof);
      data->initialized   = TRUE;
   }

   full_n_x = sed_cube_n_x(prof);
   full_n_y = sed_cube_n_y(prof);
   small_n_x = ceil( full_n_x * x_reduction );
   small_n_y = ceil( full_n_y * y_reduction );

   //---
   // Save the current time, and the time since the last subsidence
   //---
   time            = sed_cube_age_in_years(prof);
   time_step       = time - data->last_time;
   data->last_time = time;

   //---
   // First we calculate the total deflection to equilibrium.  Keep updating
   // the sed_cube but save the deflection so we can add it back later.
   //---
   {
      gssize iter = 0;
      double last_dw;
      double this_half_load;
      Eh_dbl_grid this_dw_small;
      Eh_dbl_grid this_dw_full;
      Eh_dbl_grid last_load_small;
      Eh_dbl_grid last_load_full;
      Eh_dbl_grid this_load_small;
      Eh_dbl_grid this_load_full;

      //---
      // Create a grid to hold the calculated deflections.
      //---
      eh_debug( "Create a grid to hold the calculated deflections" );
      {
         this_dw_small = eh_grid_new( double , small_n_x , small_n_y );
         eh_grid_set_x_lin( this_dw_small , 0 , sed_cube_x_res(prof)/x_reduction );
         eh_grid_set_y_lin( this_dw_small , 0 , sed_cube_y_res(prof)/y_reduction );
      }

      last_load_full = eh_grid_dup( data->last_load );

      dw_iso = eh_grid_new( double , full_n_x , full_n_y );
      eh_debug( "Subside the basin" );
      do
      {
         eh_debug( "Clear the deflection grid for each iteration" );
         eh_dbl_grid_scalar_mult( this_dw_small , 0. );

         //---
         // Calculate the loads due to each column in a sed_cube.  This is the
         // total load, which includes all of the sediment and water in each
         // column.
         //
         // Multiply by a constant that takes into account the width and length
         // of each column.
         //---
         eh_debug( "Get the new load grid" );
         this_load_full = sed_cube_load_grid( prof , NULL );
         eh_dbl_grid_scalar_mult( this_load_full , C );
         this_half_load = sed_cube_water_pressure( prof , 0 , sed_cube_n_y(prof)-1 );
   
         //---
         // Remesh the old and new load grids, to a coarser mesh.  We do this to
         // improve run-time for 2D simulations.
         //---
         eh_debug( "Remesh the load grids" );
         last_load_small = eh_dbl_grid_remesh( last_load_full ,
                                               small_n_x      ,
                                               small_n_y );
         this_load_small = eh_dbl_grid_remesh( this_load_full ,
                                               small_n_x      ,
                                               small_n_y );

         //---
         // Calculate the isostatic subsidence for the newly added sediment.
         // All of the grids used in this step are reduced from the original
         // size.  Unfortunately, this method is order n^3 and so we do the
         // calculations on a small grid to save time.  The result is then
         // interpolated to a full size grid.
         //
         // Skip the subsidence if there is no load change.  However, the load
         // can now be less than zero.
         //---
         {
            Eh_dbl_grid v_0 = eh_grid_dup( this_load_small );
            double eet = data->eet;
            double y   = data->youngs_modulus;

            eh_dbl_grid_subtract( v_0 , last_load_small );

            subside_grid_load( this_dw_small , v_0 , eet , y );

            if ( sed_mode_is_2d() )
            {
               double half_load = this_half_load - data->last_half_load;
               subside_half_plane_load( this_dw_small , half_load , eet , y );
            }
            eh_grid_destroy( v_0 , TRUE );
         }
/*
         eh_debug( "Calculate the isostatic subsidence" );
         {
            gssize i, j;
            double v_0;
            double eet = data->eet;
            double y   = data->youngs_modulus;
   
            for ( i=0 ; i<small_n_x ; i++ )
               for ( j=0 ; j<small_n_y ; j++ )
               {
                  v_0 = eh_dbl_grid_val(this_load_small,i,j)
                      - eh_dbl_grid_val(last_load_small,i,j);
                  if ( fabs(v_0) > 1e-3 )
                     subside_point_load( this_dw_small , v_0 , eet , y , i , j );
               }

            if ( !is_sedflux_3d() )
            {
               v_0 = this_half_load - data->last_half_load;
               subside_half_plane_load( this_dw_small , v_0 , eet , y );
            }

         }
*/
         //---
         // Save the current load.
         //---
         eh_debug( "Save the current load" );
         eh_grid_copy( last_load_full , this_load_full );
         data->last_half_load = this_half_load;
   
         //---
         // Expand the deflection grid back to full resolution so that the
         // sed_cube can be deflected.
         //---
         eh_debug( "Expand the grid to full resolution" );
         this_dw_full = eh_dbl_grid_expand( this_dw_small ,
                                            full_n_x      ,
                                            full_n_y );
   
         //---
         // Subside the sed_cube.
         // Save the total defelction.
         //---
         eh_debug( "Subside the basin" );
         {
            gssize i, len = sed_cube_size(prof);
   
            for ( i=0 ; i<len ; i++ )
               sed_cube_adjust_base_height( prof , 0 , i , -eh_dbl_grid_val(this_dw_full,0,i) );
         }
   
         eh_dbl_grid_add( dw_iso , this_dw_full );
   
         //---
         // Free the grids.
         //---
         eh_grid_destroy( this_dw_full    , TRUE );
         eh_grid_destroy( last_load_small , TRUE );
         eh_grid_destroy( this_load_small , TRUE );
         eh_grid_destroy( this_load_full  , TRUE );
   
         last_dw  = total_dw;
         total_dw = eh_dbl_grid_sum( dw_iso );
iter++;
      }
//      while ( iter<50 );
      while ( fabs(total_dw)>0 && fabs((total_dw-last_dw)/total_dw )>.01 );

      eh_grid_destroy( this_dw_small  , TRUE );
      eh_grid_destroy( last_load_full , TRUE );

   }

//---
// After we know the total deflection, add it back to the sed_cube.  Now we
// again deflect the sed_cube but this time we add the relaxation-time
// term.
//---
   {
      gssize i, j;
      double dt = time_step;
      double k  = data->relaxation_time;
      double **last_dw_iso = eh_dbl_grid_data(data->last_dw_iso);
      double **this_dw_iso = eh_dbl_grid_data(dw_iso);
      double f = (k<1e-6)?0:exp(-dt/k);

      for ( i=0 ; i<sed_cube_n_x(prof) ; i++ )
         for ( j=0 ; j<sed_cube_n_y(prof) ; j++ )
         {
            sed_cube_adjust_base_height( prof , i , j , this_dw_iso[i][j] );
            sed_cube_adjust_base_height( prof , i , j ,
                                         -   (1.-f)
                                           * (   this_dw_iso[i][j]
                                               + last_dw_iso[i][j] ) );

            //---
            // Save the distance from isostatic equilibrium.
            //---
            last_dw_iso[i][j] = f*(this_dw_iso[i][j]+last_dw_iso[i][j]);
         }
   }

   //---
   // Save the current load.
   //---
   eh_grid_destroy( data->last_load , TRUE );
   data->last_load = sed_cube_load_grid( prof , NULL );
   eh_dbl_grid_scalar_mult( data->last_load , C );

   eh_grid_destroy( dw_iso , TRUE );

   eh_message( "time             : %f" , sed_cube_age_in_years(prof) );
   eh_message( "Youngs modulus   : %f" , data->youngs_modulus        );
   eh_message( "EET              : %f" , data->eet                   );
   eh_message( "sea level        : %f" , sed_cube_sea_level(prof)    );
   eh_message( "total downward deflection : %f" , total_dw           );

   return info;
}

#define S_KEY_D               "flexural rigity of the crust"
#define S_KEY_EET             "effective elastic thickness"
#define S_KEY_YOUNGS_MODULUS  "Youngs modulus"
#define S_KEY_RELAXATION_TIME "relaxation time"

gboolean init_isostasy( Eh_symbol_table symbol_table,gpointer ptr)
{
   Isostasy_t *data=(Isostasy_t*)ptr;

   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }
/*
   data->D               = g_strtod( eh_symbol_table_lookup( symbol_table ,
                                                             S_KEY_D ) , 
                                     NULL);
*/
   data->relaxation_time = eh_symbol_table_dbl_value( symbol_table , S_KEY_RELAXATION_TIME );
   data->eet             = eh_symbol_table_dbl_value( symbol_table , S_KEY_EET             );
   data->youngs_modulus  = eh_symbol_table_dbl_value( symbol_table , S_KEY_YOUNGS_MODULUS  );

   return TRUE;
}

gboolean dump_isostasy_data( gpointer ptr , FILE *fp )
{
   Isostasy_t *data = (Isostasy_t*)ptr;

   fwrite( data , sizeof(Isostasy_t) , 1 , fp );
//   fwrite( data->old_thickness , sizeof(double) , data->len , fp );
//   fwrite( data->old_height    , sizeof(double) , data->len , fp );

   return TRUE;
}

gboolean load_isostasy_data( gpointer ptr , FILE *fp )
{
   Isostasy_t *data = (Isostasy_t*)ptr;

   fread( data , sizeof(Isostasy_t) , 1 , fp );

//   data->old_thickness = eh_new( double , data->len );
//   data->old_height    = eh_new( double , data->len );

//   fread( data->old_thickness , sizeof(double) , data->len , fp );
//   fread( data->old_height    , sizeof(double) , data->len , fp );

   return TRUE;
}
