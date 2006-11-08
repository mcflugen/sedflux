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

#define SED_PLUME_PROC_NAME "plume"
#define EH_LOG_DOMAIN SED_PLUME_PROC_NAME

#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <values.h>

#include "utils.h"
#include "sed_sedflux.h"
#include "run_plume.h"
#include "plume_types.h"
#include "plumeinput.h"
#include "processes.h"

#define LEFT 0
#define RIGHT 1

#undef DEBUG

gboolean plume3d( Plume_inputs *plume_const , Plume_river river       ,
                  int n_grains              , Plume_sediment *sedload ,
                  Eh_dbl_grid *deposit      , Plume_data *data );
gboolean compare_river(Plume_river *r1,Plume_river *r2,int n_grains);
int copy_river(Plume_river *r1,Plume_river *r2,int n_grains);

Sed_process_info run_plume( gpointer ptr , Sed_cube prof )
{
   Plume_t *data=(Plume_t*)ptr;
   Plume_sediment *sediment_data;
   Sed_hydro hydro_data;
   gssize i, j, n;
   gssize n_grains, n_susp_grains;
   gssize n_rivers, river_no;
   Plume_river river_data;
   Plume_inputs plume_const;
   Eh_dbl_grid *plume_deposit_grid;
   Sed_cell_grid in_suspension;
   Sed_river *this_river;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      // Clean up the static variables and return.
      if ( data->initialized )
      {
         sed_cell_grid_free( data->deposit_grid );
         sed_cell_grid_free( data->last_deposit_grid );
         eh_grid_destroy( data->deposit_grid , TRUE );
         eh_grid_destroy( data->last_deposit_grid , TRUE );
         eh_free( data->last_river_data.Cs );
         destroy_plume_data( data->plume_data );

         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->last_deposit_grid = sed_cell_grid_new( 2*sed_cube_n_x(prof) , 2*sed_cube_n_y(prof) );
      data->deposit_grid      = sed_cell_grid_new( 2*sed_cube_n_x(prof) , 2*sed_cube_n_y(prof) );

      sed_cell_grid_init( data->deposit_grid      , sed_sediment_env_size() );
      sed_cell_grid_init( data->last_deposit_grid , sed_sediment_env_size() );

      memset( &data->last_river_data , 0 , sizeof(Plume_river) );
      data->last_river_data.Cs = eh_new0( double , sed_sediment_env_size()-1 );

      data->plume_data = eh_new( Plume_data , 1 );
      init_plume_data( data->plume_data );

      data->initialized = TRUE;
   }

   n_grains      = sed_sediment_env_size();
   n_susp_grains = sed_sediment_env_size()-1;

   {
      double* lambda     = sed_sediment_property( NULL , &sed_type_lambda_in_per_seconds );
      double* rho_sat    = sed_sediment_property( NULL , &sed_type_rho_sat );
      double* grain_size = sed_sediment_property( NULL , &sed_type_grain_size );
      double* diff_coef  = sed_sediment_property( NULL , &sed_type_diff_coef );

      sediment_data = eh_new( Plume_sediment , n_susp_grains );

      for ( i=0 ; i<n_susp_grains ; i++ )
      {
         sediment_data[i].lambda    = lambda    [i+1];
         sediment_data[i].rho       = rho_sat   [i+1];
         sediment_data[i].grainsize = grain_size[i+1];
         sediment_data[i].diff_coef = diff_coef [i+1];
      }

      eh_free( lambda     );
      eh_free( rho_sat    );
      eh_free( grain_size );
      eh_free( diff_coef  );
   }

   n_rivers = sed_cube_number_of_rivers( prof );

   info.mass_lost = 0.;

   //---
   // Run the plume for each of the rivers.
   //---
   for ( river_no=0 ; river_no<n_rivers ; river_no++ )
   {

      eh_debug( "Running plume for river %d" , river_no );

      plume_deposit_grid = eh_new( Eh_dbl_grid , n_susp_grains );
      for ( n=0 ; n<n_susp_grains ; n++ )
      {
         eh_debug( "Creating grid for grain type %d" , n );

         plume_deposit_grid[n] = eh_grid_new( double      ,
                                              2*sed_cube_n_x(prof) ,
                                              2*sed_cube_n_y(prof) );

         eh_debug( "Setting x values" );
/*
         eh_grid_set_x_lin( plume_deposit_grid[n] ,
                            - sed_cube_n_x(prof)*sed_cube_x_res( prof )
                            + sed_cube_x_res( prof )*.5    ,
                            sed_cube_x_res(prof) );
*/
         eh_grid_set_x_lin( plume_deposit_grid[n] ,
                            -sed_cube_x_res(prof) ,
                            sed_cube_x_res(prof) );

         eh_debug( "Setting y values" );
         eh_grid_set_y_lin( plume_deposit_grid[n] ,
                            - sed_cube_n_y(prof)*sed_cube_y_res( prof )
                            + sed_cube_y_res( prof )*.5    ,
                            sed_cube_y_res(prof) );
      }

      this_river = sed_cube_river( prof , river_no );

      hydro_data = this_river->data;
   
      // copy the river discharge data.
      river_data.Cs = sed_hydro_copy_concentration( NULL , hydro_data );
      river_data.Q  = sed_hydro_water_flux( hydro_data );
      river_data.u0 = sed_hydro_velocity  ( hydro_data );
      river_data.b0 = sed_hydro_width     ( hydro_data );
      river_data.d0 = sed_hydro_depth     ( hydro_data );


      if ( eh_dbl_array_min( river_data.Cs , n_susp_grains ) < .001 )
         info.mass_lost += sed_hydro_suspended_load( hydro_data );

//      river_data.rdirection = sed_get_river_angle( this_river );
//      river_data.rma = 0.;

      river_data.rdirection = sed_cube_slope_dir( prof              ,
                                                  this_river->x_ind ,
                                                  this_river->y_ind );

      eh_note_block( "Setting river direction to zero" , TRUE )
      {
         river_data.rdirection = 0;
      }

      river_data.rma        = river_data.rdirection
                            - sed_get_river_angle( this_river );
      if ( !is_sedflux_3d() )
      {
         river_data.rdirection = M_PI_2;
         river_data.rma        = 0;
      }
/*
river_data.rdirection = sed_get_river_angle( this_river );
river_data.rma = 0.*S_RADS_PER_DEGREE;

river_data.rdirection = 0;
river_data.rma = 15.*S_RADS_PER_DEGREE;
*/
   
      eh_message( "shore normal          : %f" , river_data.rdirection );
      eh_message( "river angle           : %f" , river_data.rma );

      plume_const.current_velocity    = eh_input_val_eval(
                                           data->current_velocity ,
                                           sed_cube_age_in_years( prof ) );
      plume_const.ocean_concentration = data->ocean_concentration;
      plume_const.plume_width         = data->plume_width;
      plume_const.ndx                 = data->ndx;
      plume_const.ndy                 = data->ndy;

      if (    fabs( plume_const.current_velocity ) < .05
           && plume_const.current_velocity != 0 )
         plume_const.current_velocity = plume_const.current_velocity<0?-.05:.05;

      if ( !is_sedflux_3d() )
         plume_const.current_velocity = 0.;

      in_suspension = sed_cube_in_suspension( prof , river_no );
   
      // compare this river with the river at the last time step.  if they are
      // the same, then we don't have to run the plume again.
      if ( compare_river( &river_data ,
                          &data->last_river_data ,
                          n_susp_grains ) )
      {
         sed_cell_grid_copy_data( data->deposit_grid , data->last_deposit_grid );
      }
      else if ( plume3d( &plume_const          ,
                         river_data            ,
                         n_susp_grains         ,
                         sediment_data         , 
                         plume_deposit_grid    ,
                         data->plume_data ) )
      {
         double* deposit_rate;
         double** plume_deposit;
         Sed_cell** deposit = sed_cell_grid_data( data->deposit_grid );

         deposit_rate = eh_new( double , n_grains );

         for ( i=0 ; i<eh_grid_n_x(data->deposit_grid) ; i++ )
         {
            for ( j=0 ; j<eh_grid_n_y(data->deposit_grid) ; j++ )
            {
               sed_cell_clear     ( deposit[i][j] );
               sed_cell_set_facies( deposit[i][j] , S_FACIES_PLUME );
   
               for ( n=0 ; n<n_susp_grains ; n++ )
               {
                  plume_deposit     = eh_dbl_grid_data(plume_deposit_grid[n]);
                  deposit_rate[n+1] = plume_deposit[i][j]
                                  * sed_cube_time_step_in_days(prof);
               }
               deposit_rate[0] = 0.;
   
               sed_cell_add_amount( deposit[i][j] , deposit_rate );
            }
         }
   
         eh_free( deposit_rate );
      }
      else
      {
         g_warning( "Subroutine PLUME returned an error." );
         sed_cell_grid_clear( data->deposit_grid );
      }

      eh_debug( "Save the current plume" );
      sed_cell_grid_copy_data( data->last_deposit_grid ,
                               data->deposit_grid      );
      copy_river( &data->last_river_data ,
                  &river_data ,
                  n_susp_grains );

      info.mass_added = sed_hydro_suspended_load( hydro_data );
   
      // calculate the inital mass of sediment in suspension.
      eh_debug( "Calculate the input and output sediment" );
      {
         double init_mass = 0;
         double final_mass = 0;
         double input_mass;


         eh_debug( "Calculate the mass of sediment already in suspension." );
         init_mass = sed_cell_grid_mass( in_suspension )
                   * sed_cube_x_res( prof )
                   * sed_cube_y_res( prof );
   
         eh_debug( "Calculate the mass of sediment input by the river." );
         input_mass = sed_hydro_suspended_flux( hydro_data )
                    * sed_cube_time_step_in_seconds( prof );

         eh_debug( "Add the plume sediment to the sediment in suspension" );
         eh_grid_reindex  ( in_suspension , 0                   , 0                   );
         sed_cell_grid_add( in_suspension , data->deposit_grid );
         eh_grid_reindex  ( in_suspension , -sed_cube_n_x(prof) , -sed_cube_n_y(prof) );

         eh_debug( "Calculate the final mass of sediment in suspension." );
         final_mass = sed_cell_grid_mass( in_suspension )
                    * sed_cube_x_res( prof )
                    * sed_cube_y_res( prof );

         eh_message( "time                  : %f" , sed_cube_age(prof) );
         eh_message( "sediment input (kg)   : %g" , input_mass           );
         eh_message( "sediment added (kg)   : %g" , final_mass-init_mass );
         eh_message( "shore normal          : %f" , river_data.rdirection );
         eh_message( "river angle           : %f" , river_data.rma );
         eh_message( "current velocity (m/s): %f" , plume_const.current_velocity );
         eh_message( "river velocity (m/s)  : %f" , sed_hydro_velocity( hydro_data ) );
         eh_message( "river width (m)       : %f" , sed_hydro_width   ( hydro_data ) );
         eh_message( "river depth (m)       : %f" , sed_hydro_depth   ( hydro_data ) );
         for ( i=0 ; i<n_susp_grains ; i++ )
            eh_message( "river conc %d (kg/m^3): %f" , i , sed_hydro_nth_concentration(hydro_data,i) );
      }

      eh_debug( "Free temporary grids" );
      for ( n=0 ; n<n_susp_grains ; n++ )
         eh_grid_destroy( plume_deposit_grid[n] , TRUE );
      eh_free( plume_deposit_grid );

      eh_free( river_data.Cs );
   }

   eh_free( sediment_data );

   return info;
}

#define S_KEY_CONCENTRATION    "background ocean concentration"
#define S_KEY_CURRENT_VELOCITY "velocity of coastal current"
#define S_KEY_WIDTH            "maximum plume width"
#define S_KEY_X_SHORE_NODES    "number of grid nodes in cross-shore"
#define S_KEY_RIVER_NODES      "number of grid nodes in river mouth"

gboolean init_plume( Eh_symbol_table symbol_table,gpointer ptr)
{
   Plume_t *data=(Plume_t*)ptr;
   if ( symbol_table == NULL )
   {
      eh_input_val_destroy( data->current_velocity );
      data->initialized = FALSE;
      return TRUE;
   }

   if ( is_sedflux_3d() )
      data->current_velocity = eh_input_val_set( eh_symbol_table_lookup(
                                                    symbol_table ,
                                                    S_KEY_CURRENT_VELOCITY ) );
   else
      data->current_velocity = eh_input_val_set( "0.0" );

   data->ocean_concentration = eh_symbol_table_dbl_value( symbol_table , S_KEY_CONCENTRATION );
   data->plume_width         = eh_symbol_table_dbl_value( symbol_table , S_KEY_WIDTH         );
   data->ndx                 = eh_symbol_table_int_value( symbol_table , S_KEY_X_SHORE_NODES );
   data->ndy                 = eh_symbol_table_int_value( symbol_table , S_KEY_RIVER_NODES   );

   data->plume_width *= 1000.;

   return TRUE;
}

gboolean dump_plume_data( gpointer ptr , FILE *fp )
{
   Plume_t *data=(Plume_t*)ptr;

   fwrite( data , sizeof(Plume_t) , 1 , fp );
/*
   for ( i=0 ; i<data->deposit_size ; i++ )
      sed_dump_cell( data->deposit[i] , data->n_grains );

   for ( i=0 ; i<data->deposit_size ; i++ )
      sed_dump_cell( data->last_deposit[i] , data->n_gains );
*/

   return TRUE;
}

gboolean compare_river( Plume_river *r1 , Plume_river *r2 , int n_grains )
{
   int n;
   double load1=0., load2=0.;
   
   for (n=0;n<n_grains;n++)
   {
      load1 += r1->Cs[n];
      load2 += r2->Cs[n];
   }
   load1 *= r1->Q;
   load2 *= r2->Q;

   if ( fabs(load1-load2)/load1 < .1 )
      return TRUE;

   for (n=0;n<n_grains;n++)
      if ( r1->Cs[n] != r2->Cs[n] )
         return FALSE;

   if ( r1->Q == r2->Q &&
        r1->u0 == r2->u0 &&
        r1->b0 == r2->b0 &&
        r1->d0 == r2->d0 &&
        r1->rdirection == r2->rdirection &&
        r1->rma == r2->rma )
      return TRUE;

   return FALSE;
}

int copy_river(Plume_river *r1,Plume_river *r2,int n_grains)
{
   memcpy(r1->Cs,r2->Cs,sizeof(double)*n_grains);
   r1->Q = r2->Q;
   r1->u0 = r2->u0;
   r1->b0 = r2->b0;
   r1->d0 = r2->d0;
   r1->rdirection = r2->rdirection;
   r1->rma = r2->rma;
   return 0;
}


