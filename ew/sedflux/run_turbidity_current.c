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

#define SED_TURBIDITY_CURRENT_PROC_NAME "turbidity current"
#define EH_LOG_DOMAIN SED_TURBIDITY_CURRENT_PROC_NAME

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <inflow.h>
#include "my_processes.h"

#define WITH_SAKURA
#ifdef WITH_SAKURA
#include "sakura.h"
//#include "sakura_turbidity_current.h"
#endif


#ifndef VISCOSITY_OF_WATER
# define VISCOSITY_OF_WATER 1.3e-6
#endif
#ifndef DENSITY_OF_WATER
# define DENSITY_OF_WATER 1000.
#endif
#ifndef DENSITY_OF_SEA_WATER
# define DENSITY_OF_SEA_WATER 1028.
#endif
#ifndef DENSITY_OF_SEDIMENT_GRAINS
# define DENSITY_OF_SEDIMENT_GRAINS 2650.
#endif

#ifndef FLOW_CONCENTRATION 
# define FLOW_CONCENTRATION 0.08
#endif
#ifndef FLOW_DURATION 
# define FLOW_DURATION 7200.
#endif

#define TURBIDITY_CURRENT_GRID_SPACING          (100.0)
#define TURBIDITY_CURRENT_TIME_INTERVAL         (3.0)
#define TURBIDITY_CURRENT_INITIAL_VELOCITY      (1.0)
#define TURBIDITY_CURRENT_VELOCITY_RANGE        (3.0)
#define TURBIDITY_CURRENT_INITIAL_WIDTH         (1000.0)
#define TURBIDITY_CURRENT_INITIAL_HEIGHT        (6.0)
#define TURBIDITY_CURRENT_INITIAL_CONCENTRATION (0.01)
#define TURBIDITY_CURRENT_CONCENTRATION_RANGE   (0.04)
#define TURBIDITY_CURRENT_GRAIN_DENSITY         (2650.0)
#define TURBIDITY_CURRENT_SPREADING_ANGLE       (14.0)
#define TURBIDITY_CURRENT_NO_DEPOSIT_LENGTH     (0.)
#define TURBIDITY_CURRENT_INITIAL_FLUID_DENSITY (DENSITY_OF_SEA_WATER)
#define TURBIDITY_CURRENT_ALGORITHM_INFLOW      (0)
#define TURBIDITY_CURRENT_ALGORITHM_SAKURA      (1)

#define DAY_IN_SECONDS (86400.0)

#define DEFAULT_SUPPLY_TIME (1000)
#define DEFAULT_OUT_TIME    (30)
#define DEFAULT_PHEBOTTOM   (0.2) 

typedef struct
{
   double  dx;
   double  x;
   double  erode_depth;
   double* phe;
}
Sed_phe_query_t;

typedef struct
{
   double dh;
   int    i;
}
Sed_remove_query_t;

typedef struct
{
   int     i;
   double  dh;
   double* phe;
   int     n_grains;
}
Sed_add_query_t;

typedef struct
{
   int    i;
   double depth;
}
Sed_depth_query_t;

//void sed_get_phe   ( gpointer user_data , gpointer bathy_data );

Sed_process_info
run_turbidity_inflow( Sed_process proc , Sed_cube p )
{
   Inflow_t*        data = (Inflow_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   Sed_cube         fail;
   Inflow_const_st  inflow_const;
   gssize           ind_start;
   Sed_cell         flow_cell;
   Sed_hydro        flow;

   //---
   // This module can only be run on a 1D profile.
   //---
   if ( sed_mode_is_3d() )
      return info;

   eh_message( "time             : %f" , sed_cube_age_in_years(p) );

   fail = (Sed_cube)sed_process_use( proc , FAILURE_PROFILE_DATA );

   eh_require( fail );

   // Transfer over the (inflow) turbidity current flow constants.
   inflow_const.e_a             = data->E_a;
   inflow_const.e_b             = data->E_b;
   inflow_const.sua             = data->sua;
   inflow_const.sub             = data->sub;
   inflow_const.c_drag          = data->C_d;
   inflow_const.tan_phi         = data->tan_phi;
   inflow_const.mu_water        = data->mu;
   inflow_const.rho_sea_water   = data->rhoSW;
   inflow_const.rho_river_water = data->rhoSW;
   inflow_const.channel_len     = data->channel_length;
   inflow_const.channel_width   = data->channel_width;

   // Start the flow at the end of the failure.
   ind_start    = (int)( sed_cube_col_y( fail,sed_cube_n_y(fail)-1 ) );

   // Average the failure into one cell.
   flow_cell = sed_cube_to_cell( fail , NULL );

   // initial flow conditions.
   flow = inflow_flood_from_cell( flow_cell , sed_cube_x_res(p)*sed_cube_y_res(p) );

   sed_inflow( p , flow , ind_start , TURBIDITY_CURRENT_GRID_SPACING , &inflow_const );

   sed_cell_destroy( flow_cell );

   return info;
}

Sed_process_info
run_turbidity_sakura( Sed_process proc , Sed_cube p )
{
   Inflow_t*        data = (Inflow_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   Sed_cube         fail;
   Sakura_const_st  sakura_const;
   gssize           ind_start;
   Sed_cell         flow_cell;
   Sed_hydro        flow;

   //---
   // This module can only be run on a 1D profile.
   //---
   if ( sed_mode_is_3d() )
      return info;

   eh_message( "time             : %f" , sed_cube_age_in_years(p) );

   fail = (Sed_cube)sed_process_use( proc , FAILURE_PROFILE_DATA );
   //fail = data->failure;

   // Transfer over the (inflow) turbidity current flow constants.
   sakura_const.e_a             = data->E_a;
   sakura_const.e_b             = data->E_b;
   sakura_const.sua             = data->sua;
   sakura_const.sub             = data->sub;
   sakura_const.c_drag          = data->C_d;
   sakura_const.tan_phi         = data->tan_phi;
   sakura_const.mu_water        = data->mu;
   sakura_const.rho_sea_water   = data->rhoSW;
   sakura_const.rho_river_water = data->rhoSW;
   sakura_const.channel_len     = data->channel_length;
   sakura_const.channel_width   = data->channel_width;

   // Start the flow at the end of the failure.
   ind_start    = (int)( sed_cube_col_y( fail,sed_cube_n_y(fail)-1 ) );

   // Average the failure into one cell.
   flow_cell = sed_cube_to_cell( fail , NULL );

   // initial flow conditions.
   flow = sakura_flood_from_cell( flow_cell , sed_cube_x_res(p)*sed_cube_y_res(p) );

   sed_sakura( p , flow , ind_start , TURBIDITY_CURRENT_GRID_SPACING , &sakura_const );

   sed_cell_destroy( flow_cell );

   return info;
}

Sed_process_info
run_plume_hyper_inflow( Sed_process proc , Sed_cube p )
{
   Inflow_t*        data = (Inflow_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   //---
   // This module can only be run on a 1D profile.
   //---
   if ( sed_mode_is_2d() )
   {
      Inflow_const_st  inflow_const;
      Sed_hydro        flow;
      Sed_riv          this_river;
      gint             ind_start;

      // Transfer over the (inflow) turbidity current flow constants.
      inflow_const.e_a             = data->E_a;
      inflow_const.e_b             = data->E_b;
      inflow_const.sua             = data->sua;
      inflow_const.sub             = data->sub;
      inflow_const.c_drag          = data->C_d;
      inflow_const.tan_phi         = data->tan_phi;
      inflow_const.mu_water        = data->mu;
      inflow_const.rho_sea_water   = data->rhoSW;
      inflow_const.rho_river_water = sed_rho_fresh_water();
      inflow_const.channel_len     = data->channel_length;
      inflow_const.channel_width   = data->channel_width;
      inflow_const.dep_start       = TURBIDITY_CURRENT_NO_DEPOSIT_LENGTH;

      // Start the flow at the river mouth
      ind_start  = sed_cube_river_mouth_1d( p );

      // initial flow conditions.
      this_river = (Sed_riv)sed_process_use( proc , PLUME_HYDRO_DATA );
      flow       = sed_river_hydro( this_river );

      eh_require( this_river                            );
      eh_require( flow                                  );
      eh_require( sed_cube_is_in_domain_id(p,ind_start) );

      info.mass_added = sed_hydro_suspended_load( flow );

      sed_inflow( p , flow , ind_start , TURBIDITY_CURRENT_GRID_SPACING , &inflow_const );

      flow = sed_hydro_destroy( flow );
   }

   return info;
}

Sed_process_info
run_plume_hyper_sakura( Sed_process proc , Sed_cube p )
{
   Inflow_t*        data = (Inflow_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   //---
   // This module can only be run on a 1D profile.
   //---
   if ( sed_mode_is_2d() )
   {
      Sakura_const_st  sakura_const;
      Sed_hydro        flow;
      Sed_riv          this_river;
      gint             ind_start;

      // Transfer over the (inflow) turbidity current flow constants.
      sakura_const.e_a             = data->E_a;
      sakura_const.e_b             = data->E_b;
      sakura_const.sua             = data->sua;
      sakura_const.sub             = data->sub;
      sakura_const.c_drag          = data->C_d;
      sakura_const.tan_phi         = data->tan_phi;
      sakura_const.mu_water        = data->mu;
      sakura_const.rho_sea_water   = data->rhoSW;
      sakura_const.rho_river_water = sed_rho_fresh_water();
      sakura_const.channel_len     = data->channel_length;
      sakura_const.channel_width   = data->channel_width;
      sakura_const.dep_start       = TURBIDITY_CURRENT_NO_DEPOSIT_LENGTH;
      sakura_const.dt              = TURBIDITY_CURRENT_TIME_INTERVAL;

      // Start the flow at the river mouth
      ind_start  = sed_cube_river_mouth_1d( p );

      // initial flow conditions.
      this_river = (Sed_riv)sed_process_use( proc , PLUME_HYDRO_DATA );
      flow       = sed_river_hydro( this_river );

      eh_require( this_river                            );
      eh_require( flow                                  );
      eh_require( sed_cube_is_in_domain_id(p,ind_start) );

      info.mass_added = sed_hydro_suspended_load( flow );

      sed_sakura( p , flow , ind_start , TURBIDITY_CURRENT_GRID_SPACING , &sakura_const );

      flow = sed_hydro_destroy( flow );
   }

   return info;
}

#undef HYPERPYCNAL
#if defined( HYPERPYCNAL )
Sed_process_info
run_hyperpycnal( gpointer ptr , Sed_cube p )
{
   Turbidity_t *data=(Turbidity_t*)ptr; 
   Sed_process_info info = SED_EMPTY_INFO;
   double get_equivalent_diameter( double real_diameter );
   Sed_cell flow, deposit_cell;
   double init_h, init_u, init_c, init_q, init_w;
   double *fraction, *bulk_density, *grain_size, *lambda, *grain_density;
   double rho_fluid, rho_flow;
   double flow_age, volume_of_sediment, day=DAY_IN_SECONDS, flow_duration;
   double *deposit_at_x, *slope;
   int i, ind, n_nodes, n, n_grains;
   int n_nodes0, start;
   long seed;
   pos_t *bathy, *bathy0;
   double *width, dx;
   double n_days;
   Sed_cube fail;
   gboolean ok;
   
   // specific to sakura
   double dt;
   double basin_len;
   double Dstar, Wstar, Rden;
   double *stv, *rey;
   double *phe_bot;
   double out_time;
#ifdef WITH_SAKURA
   Sakura_t sakura_const;
#endif

   // specific to inflow
   double x_dep;
   Inflow_t inflow_const;

   // Clean up process.
   if ( p == NULL )
   {
      if ( data->initialized )
      {
         eh_free_2( data->deposit );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   // Initialize process.
   if ( !data->initialized )
   {
      int max_n_nodes = (sed_cube_n_y(p)+1)
                      * sed_cube_y_res(p)
                      / TURBIDITY_CURRENT_GRID_SPACING;
      gssize n_grains = sed_sediment_env_n_types();
      data->n_x = n_grains;
      data->n_y = max_n_nodes;
      data->deposit = eh_new_2( double , n_grains , max_n_nodes );
      data->initialized = TRUE;
   }
   
   //---
   // This module can only be run on a 1D profile.
   //---
   if ( sed_mode_is_3d() )
      return info;

   // Check which river are hyperpycnal.
   {
      gssize i, n;

      for ( i=0,n=0 ; i<n_rivers ; i++ )
      {
         if ( sed_hydro_is_hyperpycnal( sed_cube_river( prof , i ) ) )
         {
            river_no[n++] = i;
         }
      }
   }

   fail = data->failure;

   // Transfer over the (inflow) turbidity current flow constants.
   inflow_const.Ea            = data->E_a;
   inflow_const.Eb            = data->E_b;
   inflow_const.sua           = data->sua;
   inflow_const.sub           = data->sub;
   inflow_const.Cd            = data->C_d;
   inflow_const.tanPhi        = data->tan_phi;
   inflow_const.mu            = data->mu;
   inflow_const.rhoSW         = data->rhoSW;
   inflow_const.channelLength = data->channel_length;
   inflow_const.channelWidth  = data->channel_width;

   // Transfer over the (sakura) turbidity current flow constants.
#ifdef WITH_SAKURA
   sakura_const.Ea            = data->E_a;
   sakura_const.Eb            = data->E_b;
   sakura_const.sua           = data->sua;
   sakura_const.sub           = data->sub;
   sakura_const.Cd            = data->C_d;
   sakura_const.tanPhi        = data->tan_phi;
   sakura_const.mu            = data->mu;
   sakura_const.rhoSW         = data->rhoSW;
   sakura_const.channelLength = data->channel_length;
   sakura_const.channelWidth  = data->channel_width;
   sakura_const.rhoRW         = DENSITY_OF_WATER;
#endif

   dx       = TURBIDITY_CURRENT_GRID_SPACING;
   dt       = TURBIDITY_CURRENT_TIME_INTERVAL;
   n_grains = sed_sediment_env_n_types();

   // Start the flow at the river mouth.
   {
      start    = (int)( sed_cube_col_y( fail,sed_cube_n_y(fail)-1 ) );
      n_nodes0 = sed_cube_n_y(p) - start;
      n_nodes  = (int)((sed_cube_n_y(p)-(start+1))*sed_cube_y_res(p)/dx);
      basin_len = n_nodes * dx;
   }

   // Average the failure into one cell.
   {
      Sed_cell top  = sed_cell_new_env( );

      flow = sed_cell_new_env( );
      for ( i=0 ; i<sed_cube_n_y(fail) ; i++ )
      {
         sed_column_top( sed_cube_col(fail,i) , sed_cube_thickness(fail,0,i) , top );
         sed_cell_add( flow , top );
      }
      flow_age = sed_cell_age( flow );

      sed_cell_destroy( top );
   }

   // initial flow conditions.
   init_u = TURBIDITY_CURRENT_INITIAL_VELOCITY;
   init_h = TURBIDITY_CURRENT_INITIAL_HEIGHT;
   init_w = TURBIDITY_CURRENT_INITIAL_WIDTH;
   init_c = TURBIDITY_CURRENT_INITIAL_CONCENTRATION;
   init_q = init_u*init_h*init_w;

   fraction      = sed_cell_copy_fraction( NULL     , flow );
   bulk_density  = sed_sediment_property ( NULL     , &sed_type_rho_sat );
   lambda        = sed_sediment_property ( NULL     , &sed_type_lambda_in_per_seconds );
   grain_size    = sed_sediment_property ( NULL     , &sed_type_grain_size_in_meters );
   grain_density = eh_dbl_array_new_set  ( n_grains , TURBIDITY_CURRENT_GRAIN_DENSITY );

   for ( n=0 ; n<n_grains ; n++ )
      grain_size[n] = get_equivalent_diameter( grain_size[n] );

   rho_fluid = TURBIDITY_CURRENT_INITIAL_FLUID_DENSITY;
   rho_flow  = init_c*(TURBIDITY_CURRENT_GRAIN_DENSITY-rho_fluid)+rho_fluid;
   volume_of_sediment  = sed_cell_size_0( flow )
                       * sed_cube_y_res(p)
                       * sed_cube_x_res(p);
   volume_of_sediment *= sed_cell_density( flow )
                       / TURBIDITY_CURRENT_GRAIN_DENSITY;

   // the number of days the flow will last
   n_days = volume_of_sediment/( init_c*init_q )/day;

   // bathymetry.
   bathy  = createPosVec( n_nodes );
   bathy0 = createPosVec( n_nodes0 );
   width  = eh_new( double , n_nodes );
   for ( i=0 ; i<n_nodes0 ; i++ )
   {
      bathy0->x[i] = (i+start)*sed_cube_y_res(p);
      bathy0->y[i] = -sed_cube_water_depth(p,0,i+start);
   }
   bathy->x[0] = bathy0->x[0] + 0.5*dx;
   for ( i=1 ; i<bathy->size ; i++ )
      bathy->x[i] = bathy->x[i-1] + TURBIDITY_CURRENT_GRID_SPACING;
   interpolate( bathy0->x , bathy0->y , n_nodes0 ,
                bathy->x  , bathy->y  , n_nodes );

#ifdef WITH_SAKURA
   width[0] = sakura_const.channelWidth;
#endif
   for ( i=1 ; i<n_nodes ; i++ )
   {
      if ( bathy->x[i]-bathy->x[0] < inflow_const.channelLength )
         width[i] = inflow_const.channelWidth;
      else
      {
         width[i] = width[i-1]
                  + tan( TURBIDITY_CURRENT_SPREADING_ANGLE*M_PI/180. )
                    * ( bathy->x[i]-bathy->x[i-1] );
         if ( width[i] > sed_cube_x_res(p) )
            width[i] = sed_cube_x_res(p);
      }
   }

   x_dep        = TURBIDITY_CURRENT_NO_DEPOSIT_LENGTH;
   deposit_at_x = eh_new( double , n_grains );

   // INITIALIZE settling velocity
#ifdef WITH_SAKURA
   stv = eh_new0( double , n_grains );
   rey = eh_new0( double , n_grains );
   for ( i=0; i<n_grains ; i++ )
   {
        Rden  = grain_density[i] / sakura_const.rhoRW;
        Dstar = G * Rden * pow( grain_size[i], 3.0 )/ pow(sakura_const.mu,2.0);
        Wstar = - 3.76715 + (1.92944 * log10(Dstar)) - 0.09815 * pow(log10(Dstar), 2.0)
                - 0.00575 * pow(log10(Dstar), 3.0) +0.00056 * pow(log10(Dstar), 4.0);
        Wstar = pow(10.0, Wstar);
        stv[i] = pow(Rden * G * sakura_const.mu * Wstar, 0.33333);
        rey[i] = sqrt(Rden * G * pow(grain_size[i], 3.0))/sakura_const.mu;
   } 
#endif

   // set up the data for the get_phe function.
//   get_phe_data.prof    = p;
//   get_phe_data.dx      = dx;
   inflow_const.get_phe_data   = (gpointer)p;
   inflow_const.get_depth_data = (gpointer)p;
   inflow_const.remove_data    = (gpointer)p;
   inflow_const.add_data       = (gpointer)p;
   inflow_const.get_phe        = (Sed_query_func)&sed_get_phe;
   inflow_const.get_depth      = (Sed_query_func)&sed_get_depth;
   inflow_const.remove         = (Sed_query_func)&sed_remove;
   inflow_const.add            = (Sed_query_func)&sed_add;

   eh_message( "time                 : %f" , sed_cube_age_in_years(p) );
   eh_message( "flow_duration (days) : %f" , n_days                   );
   eh_message( "mass                 : %f" , sed_cube_mass(p)         );

   while ( volume_of_sediment > 0 )
   {
      // determine the initial flow parameters.
      init_h   = TURBIDITY_CURRENT_INITIAL_HEIGHT;
      init_w   = TURBIDITY_CURRENT_INITIAL_WIDTH;
      init_u   = TURBIDITY_CURRENT_INITIAL_VELOCITY
               + TURBIDITY_CURRENT_VELOCITY_RANGE*eh_ran2(&seed);
      init_c   = TURBIDITY_CURRENT_INITIAL_CONCENTRATION
               + TURBIDITY_CURRENT_CONCENTRATION_RANGE*eh_ran2(&seed);
      init_q   = init_u*init_h*init_w;
      rho_flow = init_c*(TURBIDITY_CURRENT_GRAIN_DENSITY-rho_fluid)+rho_fluid;

#ifndef WITH_SAKURA
data->algorithm = TURBIDITY_CURRENT_ALGORITHM_INFLOW;
#endif
      if ( data->algorithm == TURBIDITY_CURRENT_ALGORITHM_INFLOW )
      {
         if ( volume_of_sediment < init_q*init_c*day )
            flow_duration = volume_of_sediment/init_q/init_c;
         else
            flow_duration = day;
         volume_of_sediment -= init_q*init_c*day;
      }
      else
         volume_of_sediment = 0;

      // Update the bathymetry.
      for (i=0;i<n_nodes0;i++)
         bathy0->y[i] = -sed_cube_water_depth(p,0,i+start);
      interpolate( bathy0->x , bathy0->y , n_nodes0 ,
                   bathy->x  , bathy->y  , n_nodes );
      slope = derivative( *bathy );

      // Run the flow.
#ifdef WITH_SAKURA
      if ( data->algorithm == TURBIDITY_CURRENT_ALGORITHM_SAKURA )
         ok = sakura( dx            , dt            , basin_len     ,
                      n_nodes       , n_grains      , bathy->x      ,
                      bathy->y      , width         , &init_u       ,
                      &init_c       , lambda        , stv           ,
                      rey           , grain_density , init_h        ,
                      flow_duration , x_dep         , fraction      ,
                      phe_bot       , bulk_density  , out_time      ,
                      sakura_const  , data->deposit , NULL );
      else
#endif
         ok = inflow( flow_duration , bathy->x  , slope        , width         ,
                      n_nodes       , dx        , x_dep        , init_w        ,
                      init_u        , init_h    , init_q       , fraction      ,
                      grain_size    , lambda    , bulk_density , grain_density ,
                      n_grains      , rho_fluid , rho_flow     , inflow_const  ,
                      data->deposit , NULL );

      if ( ok )
      {
         // Spread the deposit from just over the channel width to the entire
         // basin width.
         for ( n=0 ; n<n_grains ; n++ )
            for ( i=0 ; i<n_nodes ; i++ )
               data->deposit[n][i] *= width[i]
                                    / sed_cube_x_res(p)
                                    * TURBIDITY_CURRENT_GRID_SPACING
                                    / sed_cube_y_res(p);

         // Add the sediment to the profile.
         deposit_cell = sed_cell_new_env( );
         for ( i=0 ; i<n_nodes ; i++ )
         {
            for ( n=0 ; n<n_grains ; n++ )
               deposit_at_x[n] = data->deposit[n][i];
            ind = (int)(bathy->x[i]/sed_cube_y_res(p));
            if ( ind < sed_cube_n_y(p) )
            {
               sed_cell_clear( deposit_cell );
               sed_cell_set_age( deposit_cell , flow_age );
               sed_cell_set_facies( deposit_cell , S_FACIES_TURBIDITE );
               sed_cell_add_amount( deposit_cell , deposit_at_x );
               sed_column_add_cell( sed_cube_col(p,ind) , deposit_cell );
            }
         }
         sed_cell_destroy( deposit_cell );
      }
      else
      {
         // Pretend like nothing happened.  Put the failed sediment where
         // it began.
         sed_cube_add(p,fail);
      }

      eh_free( slope );

   }

   eh_free( stv           );
   eh_free( rey           );
   eh_free( deposit_at_x  );
   eh_free( fraction      );
   eh_free( bulk_density  );
   eh_free( grain_density );
   eh_free( grain_size    );
   eh_free( lambda        );
   eh_free( width         );

   sed_cell_destroy( flow );
   destroyPosVec( bathy0 );
   destroyPosVec( bathy  );

   return info;
}
#endif

#define S_KEY_SUA            "sua"
#define S_KEY_SUB            "sub"
#define S_KEY_E_A            "entrainment constant, ea"
#define S_KEY_E_B            "entrainment constant, eb"
#define S_KEY_C_D            "drag coefficient"
#define S_KEY_TAN_PHI        "internal friction angle"
#define S_KEY_CHANNEL_WIDTH  "width of channel"
#define S_KEY_CHANNEL_LENGTH "length of channel"

static const gchar* inflow_labels[] =
{
   S_KEY_SUA            ,
   S_KEY_SUB            ,
   S_KEY_E_A            ,
   S_KEY_E_B            ,
   S_KEY_C_D            ,
   S_KEY_TAN_PHI        ,
   S_KEY_CHANNEL_WIDTH  ,
   S_KEY_CHANNEL_LENGTH ,
   NULL
};

gboolean
init_inflow( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Inflow_t*    data    = sed_process_new_user_data( p , Inflow_t );
   GError*      tmp_err = NULL;
   gchar**      err_s   = NULL;
   gboolean     is_ok   = TRUE;
   //gchar*       key;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   eh_symbol_table_require_labels( tab , inflow_labels , &tmp_err );

   if ( !tmp_err )
   {
      data->sua            = eh_symbol_table_dbl_value( tab , S_KEY_SUA            );
      data->sub            = eh_symbol_table_dbl_value( tab , S_KEY_SUB            );
      data->E_a            = eh_symbol_table_dbl_value( tab , S_KEY_E_A            );
      data->E_b            = eh_symbol_table_dbl_value( tab , S_KEY_E_B            );
      data->C_d            = eh_symbol_table_dbl_value( tab , S_KEY_C_D            );
      data->tan_phi        = eh_symbol_table_dbl_value( tab , S_KEY_TAN_PHI        );
      data->channel_width  = eh_symbol_table_dbl_value( tab , S_KEY_CHANNEL_WIDTH  );
      data->channel_length = eh_symbol_table_dbl_value( tab , S_KEY_CHANNEL_LENGTH );
/*
      key                  = eh_symbol_table_lookup( tab , S_KEY_ALGORITHM );
      if      ( g_ascii_strcasecmp( key , "INFLOW" ) == 0 ) data->algorithm = TURBIDITY_CURRENT_ALGORITHM_INFLOW;
      else if ( g_ascii_strcasecmp( key , "SAKURA" ) == 0 ) data->algorithm = TURBIDITY_CURRENT_ALGORITHM_SAKURA;
      else
         g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM ,
                      "Invalid turbidity current model (inflow or sakura): %s" , key );
*/

      data->tan_phi         = tan(data->tan_phi*S_RADS_PER_DEGREE);
      data->channel_length *= 1000.;
      data->mu              = 1.3e-6;
      data->rhoSW           = 1028.;

      eh_check_to_s( data->sua>=0            , "Bottom sediment shear strength positive"          , &err_s );
      eh_check_to_s( data->sub>=0            , "Bottom sediment shear strength gradient positive" , &err_s );
      eh_check_to_s( data->E_a>=0            , "Entrainment constant E_a positive"                , &err_s );
      eh_check_to_s( data->E_b>=0            , "Entrainment constant E_b positive"                , &err_s );
      eh_check_to_s( data->C_d>=0            , "Drag coefficient positive"                        , &err_s );
      eh_check_to_s( data->tan_phi>=0        , "Sediment friction angle positive"                 , &err_s );
      eh_check_to_s( data->channel_width>=0  , "Channel width positive"                           , &err_s );
      eh_check_to_s( data->channel_length>=0 , "Channel length positive"                          , &err_s );

      if ( !tmp_err && err_s )
         eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );

   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_inflow( Sed_process p )
{
   if ( p )
   {
      Inflow_t* data = (Inflow_t*)sed_process_user_data( p );
      
      if ( data ) eh_free( data );
   }

   return TRUE;
}

gboolean dump_turbidity_current_data( gpointer ptr , FILE *fp )
{
   Inflow_t *data = (Inflow_t*)ptr;

   fwrite( data , sizeof(Inflow_t) , 1 , fp );
   sed_cube_write( fp , data->failure );

//   for ( i=0 ; i<data->n_x ; i++ )
//      fwrite( data->deposit[i] , sizeof(double) , data->n_y , fp );

   return TRUE;
}

gboolean load_turbidity_current_data( gpointer ptr , FILE *fp )
{
   Inflow_t* data = (Inflow_t*)ptr;

   fread( data , sizeof(Inflow_t) , 1 , fp );
   data->failure = sed_cube_read( fp );

//   data->deposit = eh_new( double* , data->n_x );
//   for ( i=0 ; i<data->n_x ; i++ )
//   {
//      data->deposit[i] = eh_new( double , data->n_y );
//      fread( data->deposit[i] , sizeof(double) , data->n_y , fp );
//   }

   return TRUE;
}

/***********************************************************************
*                                                                      *
* Function :                                                           *
*                                                                      *
*  GETPHE                                                              *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Description :                                                        *
*                                                                      *
*  Used by the turbidity current model to talk to the SEDFLUX model.   *
*  This function removes sediment from our basin and returns the       *
*  the amount that was eroded (in meters) and sets the fractions of    *
*  each grain type that has been eroded.                               *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Formal parameters :                                                  *
*                                                                      *
*  phe   - The fractions of each grain type that were removed.         *
*  pos   - The position (in meters) within the basin that sediment is  *
*          to be removed.                                              *
*  depth - The depth (in meters) that is to be eroded.                 *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Global Variables :                                                   *
*                                                                      *
*  prof_ - A pointer to the basin that sediment is to be eroded from.  *
*  dx_   - The horizontal spacing of the sediment columns in our model *
*          basin.                                                      *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Local Variables :                                                    *
*                                                                      *
*  i       - Loop counter.                                             *
*  n       - Loop counter.                                             *
*  n_grains - The number of grains types in our basin.                  *
*  avg     - A cell of sediment that contains the eroded sediment.     *
*                                                                      *
***********************************************************************/
 
#ifdef OLDWAY

#include <string.h>
#include "sed_sedflux.h"

/* Sediment profile for inflow to remove sediment from.
*/
extern Sed_cube prof_;

/* Grid spacing used by inflow.
*/
extern double dx_;

double getPhe(double *phe,double pos,double depth)
{
   Sed_cell avg;
   double volume;
   int i, n, n_grains;
   
   n_grains = sed_size(prof_->sed);
   avg = sed_create_cell(n_grains);

   if ( depth > 0 )
   {
      /* Determine which column of the profile to remove sediment from.
      */
      i = (int)(pos/sed_get_profile_spacing(prof_));
      if ( i<0 ) i=0;
      if ( i>prof_->size-1 ) i=prof_->size-1;

      /* The grid used by inflow will be smaller than that used by sedflux.
         As such we reduce the erosion depth but remove from the entire width
         of the cell in such a way that mass is conserved.
      */
      depth *= dx_/sed_get_profile_spacing(prof_);

      if ( depth > sed_get_column_thickness(prof_->col[i]) )
         depth = sed_get_column_thickness(prof_->col[i]);
   
      /* Remove different grain sizes equally.  We can change this so that sands 
         are more easily eroded than clays -- or whatever we want, really.
      */
      sed_extract_top_from_column(prof_->col[i],depth,avg);

#ifdef ______NOTHING
      /* Remove 'depth' sediment from the column.
      */
      avg->thickness = -depth;
      avg->uncompacted_thickness = avg->thickness;
   
      /* Remove different grain sizes equally.  We can change this so that sands 
         are more easily eroded than clays -- or whatever we want, really.
      */
      for (n=0;n<n_grains;n++)
         avg->fraction[n] = 1.;

      /* Remove the sediment.
      */
      sedAddCell(prof_->col[i],avg);
#endif

      for (n=0;n<n_grains;n++)
         phe[n] = avg->fraction[n];

      /* We want to return the amount of sediment that was removed.  That is,
         sediment - JUST SEDIMENT, DRY SEDIMENT - not sediment plus water.
      */
      for (n=0,volume=0.;n<n_grains;n++)
         volume += depth*phe[n]*sed_rho_sat(prof_->sed,n)/DENSITY_OF_SEDIMENT_GRAINS;
   }
   else
   {
      for (n=0;n<n_grains;n++)
         phe[n] = 0.;
      volume = 0.;
   }

   sed_destroy_cell(avg);

   /* This is how much was actually removed.
   */
   return volume;
}

#endif

/** Get the grain size distribution of bottom sediments.

Get the fractions of each grain type of bottom sediments from a
Sed_cube.  This function is intended to be used within another program that
needs to communicate with the sedflux architecture but is otherwise separate
from sedflux.

Note that the member, eroded_depth may be changed to reflect the actual amount
of bottom sediment available to be eroded.  That is, we may be trying to erode
more sediment than is actually present.

\param data    A structure that contains the necessary data for the function to
               retreive the grain type fracitons.
\param p       A Sed_cube to query

\return       A pointer to the array of grain type fractions.

*/
/*
void sed_get_phe( gpointer data , gpointer p )
{
   Sed_cube prof  = (Sed_cube)p;
   double dx      = EH_STRUCT_MEMBER( Sed_phe_query_t , data , dx          );
   double x       = EH_STRUCT_MEMBER( Sed_phe_query_t , data , x           );
   double depth   = EH_STRUCT_MEMBER( Sed_phe_query_t , data , erode_depth );
   double *phe    = EH_STRUCT_MEMBER( Sed_phe_query_t , data , phe         );
   Sed_cell avg;
   double volume;
   int i, n, n_grains;
   
   n_grains = sed_sediment_env_n_types();
   avg      = sed_cell_new_env( );

   if ( depth > 0 )
   {
      // Determine which column of the profile to remove sediment from.
      i = (int)(x/sed_cube_y_res(prof));
      if ( i<0 ) i=0;
      eh_lower_bound( i , 0 );
      eh_upper_bound( i , sed_cube_n_y(prof)-1 );

      // The grid used by inflow will be smaller than that used by sedflux.
      // As such, we reduce the erosion depth but remove from the entire width
      // of the cell in such a way that mass is conserved.
      depth *= dx/sed_cube_y_res( prof );

      eh_upper_bound( depth , sed_cube_thickness(prof,0,i) );
   
      // Remove different grain sizes equally.  We can change this so that
      // sands are more easily eroded than clays -- or whatever we want,
      // really.
      sed_column_extract_top( sed_cube_col(prof,i) , depth , avg );

      for ( n=0 ; n<n_grains ; n++ )
         phe[n] = sed_cell_nth_fraction( avg , n );

      // We want to return the amount of sediment that was removed.  That is,
      // sediment - JUST SEDIMENT, DRY SEDIMENT - not sediment plus water.
      for ( n=0 , volume=0. ; n<n_grains ; n++ )
         volume += depth
                 * phe[n]
                 * sed_type_rho_sat( sed_sediment_type( NULL , n ) )
                 / DENSITY_OF_SEDIMENT_GRAINS;
   }
   else
   {
      for ( n=0 ; n<n_grains ; n++ )
         phe[n] = 0.;
      volume = 0.;
   }

   sed_cell_destroy( avg );

   // save the volume that was actually eroded.
   EH_STRUCT_MEMBER( Sed_phe_query_t , data , erode_depth ) = volume;

   return;
}

void sed_remove( gpointer remove_query , gpointer data )
{
   double remove = EH_STRUCT_MEMBER( Sed_remove_query_t , remove_query , dh );
   int    i      = EH_STRUCT_MEMBER( Sed_remove_query_t , remove_query , i  );
   Sed_cube prof = (Sed_cube)data;
   Sed_cell cell;
   
   cell = sed_column_top( sed_cube_col(prof,i) , remove , NULL );
   EH_STRUCT_MEMBER( Sed_remove_query_t , remove_query , dh ) = sed_cell_size( cell );
   sed_cell_destroy( cell );

   return;
}

void sed_add( gpointer add_query , gpointer data )
{
   double add   = EH_STRUCT_MEMBER( Sed_add_query_t , add_query , dh       );
   double *phe  = EH_STRUCT_MEMBER( Sed_add_query_t , add_query , phe      );
   int    i     = EH_STRUCT_MEMBER( Sed_add_query_t , add_query , i        );
   int n_grains = EH_STRUCT_MEMBER( Sed_add_query_t , add_query , n_grains );
   Sed_cube prof = (Sed_cube)data;
   double *amount;
   int n;

   amount = eh_new( double , n_grains );

   for ( n=0 ; n<n_grains ; n++ )
      amount[n] = phe[n]*add;

   sed_column_add_vec( sed_cube_col(prof,i) , amount );

   eh_free( amount );

   return;
}

void sed_get_depth( gpointer depth_query , gpointer data )
{
   int i   = EH_STRUCT_MEMBER( Sed_depth_query_t , depth_query , i );
   Sed_cube prof = (Sed_cube)data;

   EH_STRUCT_MEMBER( Sed_depth_query_t , depth_query , depth ) =
      sed_cube_water_depth( prof , 0 , i );

   return;
}
*/
