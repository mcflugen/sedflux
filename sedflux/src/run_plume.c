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

#define EH_LOG_DOMAIN PLUME_PROCESS_NAME_S

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "sed_sedflux.h"
#include "plume_types.h"
#include "plumeinput.h"
#include "my_processes.h"

#define LEFT 0
#define RIGHT 1

#undef DEBUG

gboolean plume3d( Plume_inputs *plume_const , Plume_river river       ,
                  int n_grains              , Plume_sediment *sedload ,
                  Eh_dbl_grid *deposit      , Plume_data *data );
gboolean compare_river(Plume_river *r1,Plume_river *r2,int n_grains);
int copy_river(Plume_river *r1,Plume_river *r2,int n_grains);

gboolean init_plume_data     ( Sed_process proc , Sed_cube prof , GError** error );
gboolean init_plume_hypo_data( Sed_process proc , Sed_cube prof , GError** error );

GQuark
plume_hydro_data_quark( void )
{
   return g_quark_from_string( "plume-hydro-data-quark" );
}

Sed_process_info
run_plume( Sed_process proc , Sed_cube p )
{
   Plume_t*         data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   if ( sed_process_run_count(proc)==0 )
      init_plume_data( proc , p , NULL );

   //---
   // Run the plume for each of the rivers.
   //---
   {
      Sed_process plume_hyper = data->plume_proc_hyper;
      Sed_process plume_hypo  = data->plume_proc_hypo;
      Sed_riv*    all_leaves;
      Sed_riv*    r;

      all_leaves = sed_cube_all_leaves( p );

      if ( all_leaves )
      {
         for ( r=all_leaves ; *r ; r++ )
         {
            eh_debug( "Running plume for river %s" , sed_river_name_loc( *r ) );

            if ( sed_river_is_hyperpycnal(*r) ) eh_debug( "Plume is hyperpycnal" );
            else                                eh_debug( "Plume is hypopycnal" );

            sed_process_provide( plume_hyper , PLUME_HYDRO_DATA , *r );
            sed_process_provide( plume_hypo  , PLUME_HYDRO_DATA , *r );

            if      ( plume_hyper && sed_river_is_hyperpycnal( *r ) )
               sed_process_run_now( plume_hyper , p );
            else if ( plume_hypo )
               sed_process_run_now( plume_hypo  , p );

            sed_process_withhold( plume_hyper , PLUME_HYDRO_DATA );
            sed_process_withhold( plume_hypo  , PLUME_HYDRO_DATA );
         }
      }
   }

   return info;
}

Sed_process_info
run_plume_hypo( Sed_process proc , Sed_cube prof )
{
   Plume_hypo_t*    data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   Plume_sediment*  sediment_data;
   gssize           n_grains;
   gssize           n_susp_grains;

   if ( sed_process_run_count(proc)==0 )
      init_plume_hypo_data( proc , prof , NULL );

   n_grains      = sed_sediment_env_size();
   n_susp_grains = sed_sediment_env_size()-1;

   {
      gssize  i;
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

   info.mass_lost = 0.;

   {
      gssize        i, j, n;
      Sed_hydro     hydro_data;
      Sed_riv       this_river;
      Plume_river   river_data;
      Plume_inputs  plume_const;
      Eh_dbl_grid*  plume_deposit_grid = eh_new( Eh_dbl_grid , n_susp_grains );
      Sed_cell_grid in_suspension;

      for ( n=0 ; n<n_susp_grains ; n++ )
      {
         eh_debug( "Creating grid for grain type %d" , n );

         plume_deposit_grid[n] = eh_grid_new( double      ,
                                              2*sed_cube_n_x(prof) ,
                                              2*sed_cube_n_y(prof) );

         eh_debug( "Setting x values" );

         if ( sed_mode_is_3d() )
            eh_grid_set_x_lin( plume_deposit_grid[n] ,
                               - sed_cube_n_x(prof)*sed_cube_x_res( prof )
                               + sed_cube_x_res( prof )*.5    ,
                               sed_cube_x_res(prof) );
         else
            eh_grid_set_x_lin( plume_deposit_grid[n] ,
                               -sed_cube_x_res(prof) ,
                               sed_cube_x_res(prof) );

         eh_debug( "Setting y values" );
         eh_grid_set_y_lin( plume_deposit_grid[n] ,
                            - sed_cube_n_y(prof)*sed_cube_y_res( prof )
                            + sed_cube_y_res( prof )*.5    ,
                            sed_cube_y_res(prof) );
      }

      this_river = sed_process_use( proc , PLUME_HYDRO_DATA );
      hydro_data = sed_river_hydro( this_river );
   
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
                                                  sed_river_hinge(this_river).i ,
                                                  sed_river_hinge(this_river).j );

      eh_note_block( "Setting river direction to zero" , TRUE )
      {
         river_data.rdirection = 0;
      }

      river_data.rma        = river_data.rdirection
                            - sed_river_angle( this_river );

      river_data.rdirection = sed_river_angle( this_river );
      river_data.rma = 0.;

      if ( sed_mode_is_2d() )
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

      if ( sed_mode_is_2d() )
         plume_const.current_velocity = 0.;

      in_suspension = sed_cube_in_suspension( prof , this_river );
   
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
         double*    deposit_rate;
         double**   plume_deposit;
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

      hydro_data = sed_hydro_destroy( hydro_data );
   }

   eh_free( sediment_data );

   return info;
}

#define PLUME_KEY_HYPO_MODEL      "Hypopycnal plume model"
#define PLUME_KEY_HYPER_MODEL     "Hyperpycnal plume model"

static gchar* plume_req_labels[] =
{
   PLUME_KEY_HYPO_MODEL  ,
   PLUME_KEY_HYPER_MODEL ,
   NULL
};

gboolean
init_plume( Sed_process p , Eh_symbol_table t , GError** error )
{
   Plume_t* data     = sed_process_new_user_data( p , Plume_t );
   GError*  tmp_err  = NULL;

   data->plume_proc_hyper = NULL;
   data->plume_proc_hypo  = NULL;

   if ( eh_symbol_table_require_labels( t , plume_req_labels , &tmp_err ) )
   {
      data->hyper_name   = eh_symbol_table_value ( t , PLUME_KEY_HYPER_MODEL );
      data->hypo_name    = eh_symbol_table_value ( t , PLUME_KEY_HYPO_MODEL  );
   }

   if ( tmp_err ) g_propagate_error( error , tmp_err );

   return TRUE;
}

gboolean
destroy_plume( Sed_process p )
{
   if ( p )
   {
      Plume_t* data = sed_process_user_data( p );

      if ( data )
      {
         eh_free( data->hypo_name  );
         eh_free( data->hyper_name );
         eh_free( data             );
      }
   }

   return TRUE;
}

gboolean
init_plume_data( Sed_process proc , Sed_cube prof , GError** error )
{
   Plume_t* data = sed_process_user_data( proc );

   if ( data )
   {
      data->plume_proc_hypo  = sed_process_child( proc , data->hypo_name  );
      data->plume_proc_hyper = sed_process_child( proc , data->hyper_name );
   }

   return TRUE;
}

#define HYPO_KEY_CONCENTRATION    "background ocean concentration"
#define HYPO_KEY_CURRENT_VEL      "velocity of coastal current"
#define HYPO_KEY_WIDTH            "maximum plume width"
#define HYPO_KEY_X_SHORE_NODES    "number of grid nodes in cross-shore"
#define HYPO_KEY_RIVER_NODES      "number of grid nodes in river mouth"

static gchar* hypo_3d_req_labels[] =
{
   HYPO_KEY_CONCENTRATION ,
   HYPO_KEY_CURRENT_VEL   ,
   HYPO_KEY_WIDTH         ,
   HYPO_KEY_X_SHORE_NODES ,
   HYPO_KEY_RIVER_NODES   ,
   NULL
};

static gchar* hypo_2d_req_labels[] =
{
   HYPO_KEY_CONCENTRATION ,
   HYPO_KEY_WIDTH         ,
   HYPO_KEY_X_SHORE_NODES ,
   HYPO_KEY_RIVER_NODES   ,
   NULL
};

gboolean
init_plume_hypo( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Plume_hypo_t* data     = sed_process_new_user_data( p , Plume_hypo_t );
   GError*       tmp_err  = NULL;
   gchar**       err_s    = NULL;
   gboolean      is_ok    = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->deposit_size       = 0;
   data->deposit            = NULL;
   data->last_deposit       = NULL;
   data->plume_deposit      = NULL;
   data->last_river_data.Cs = NULL;
   data->plume_data         = NULL;
   data->deposit_grid       = NULL;
   data->last_deposit_grid  = NULL;

   if ( sed_mode_is_3d() ) eh_symbol_table_require_labels( tab , hypo_3d_req_labels , &tmp_err );
   else                    eh_symbol_table_require_labels( tab , hypo_2d_req_labels , &tmp_err );

   if ( !tmp_err )
   {
      if ( sed_mode_is_3d() )
         data->current_velocity = eh_symbol_table_input_value( tab , HYPO_KEY_CURRENT_VEL , &tmp_err);
      else
         data->current_velocity = eh_input_val_set( "0.0" , NULL );

      data->ocean_concentration = eh_symbol_table_dbl_value( tab , HYPO_KEY_CONCENTRATION );
      data->plume_width         = eh_symbol_table_dbl_value( tab , HYPO_KEY_WIDTH         );
      data->ndx                 = eh_symbol_table_int_value( tab , HYPO_KEY_X_SHORE_NODES );
      data->ndy                 = eh_symbol_table_int_value( tab , HYPO_KEY_RIVER_NODES   );

      data->plume_width *= 1000.;

      eh_check_to_s( data->ocean_concentration>=0. , "Ocean concentration positive" , &err_s );
      eh_check_to_s( data->plume_width>=0.         , "Plume width positive"         , &err_s );
      eh_check_to_s( data->ndx>0.                  , "Plume ndx positive integer"   , &err_s );
      eh_check_to_s( data->ndy>0.                  , "Plume ndy positive integer"   , &err_s );

      if ( err_s ) eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_plume_hypo_data( Sed_process proc , Sed_cube prof , GError** error )
{
   Plume_hypo_t* data = sed_process_user_data( proc );

   if ( data )
   {
      data->last_deposit_grid = sed_cell_grid_new( 2*sed_cube_n_x(prof) , 2*sed_cube_n_y(prof) );
      data->deposit_grid      = sed_cell_grid_new( 2*sed_cube_n_x(prof) , 2*sed_cube_n_y(prof) );

      sed_cell_grid_init( data->deposit_grid      , sed_sediment_env_size() );
      sed_cell_grid_init( data->last_deposit_grid , sed_sediment_env_size() );

      memset( &data->last_river_data , 0 , sizeof(Plume_river) );
      data->last_river_data.Cs = eh_new0( double , sed_sediment_env_size()-1 );

      data->plume_data = eh_new( Plume_data , 1 );
      plume_data_init( data->plume_data );
   }

   return TRUE;
}

gboolean
destroy_plume_hypo( Sed_process p )
{
   if ( p )
   {
      Plume_hypo_t* data = sed_process_user_data( p );
      
      if ( data )
      {
         sed_cell_grid_free( data->deposit_grid      );
         sed_cell_grid_free( data->last_deposit_grid );

         eh_grid_destroy( data->deposit_grid      , TRUE );
         eh_grid_destroy( data->last_deposit_grid , TRUE );

         eh_free( data->last_river_data.Cs );
         destroy_plume_data( data->plume_data );

         eh_input_val_destroy( data->current_velocity );
         eh_free( data );
      }
   }

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


