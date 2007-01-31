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

#define SED_EROSION_PROC_NAME "erosion"
#define EH_LOG_DOMAIN SED_EROSION_PROC_NAME

#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "sed_sedflux.h"
#include "processes.h"

#include "erosion.h"
#include "diffusion.h"

#define RIVER_SLOPE 0.001

/* These are values that were measured for the Homathko.
*/
#define A_DELTA 6.1
#define B_DELTA .000059
/* These are values that were measured for the Kliniklini.
#define A_DELTA 5.7
#define B_DELTA .000093
*/
#define OMEGA_DELTA 6
#define H_DELTA 10
#define L_DELTA 10000

#define PAOLA_MEANDERING (1)
#define PAOLA_BRAIDED    (2)

typedef struct
{
   double x0, y0;
   double H1, Rl, b;
} logProf_t;

typedef struct
{
   double a, b;
} linearProf_t;

Sed_process_info run_erosion( gpointer ptr , Sed_cube p )
{
   Erosion_t *data=(Erosion_t*)ptr;
   logProf_t getLogConstants(double,double,double);
   linearProf_t get_linear_constants(double);
   double getLogHeight(double,logProf_t);
   double get_linear_height(double,linearProf_t);
   double get_paola_diffusion( Sed_hydro river_data , double basin_width ,
                               double time_fraction     , int river_type );
   gint n_grains, n_rivers;
   Sed_riv this_river;
   int i, i_river, n;
   double k_land;
   double x, dx, width, dt;
   double river_height, height, eroded_height;
   double volume_eroded=0;
   double *eroded_fraction;
   double time_fraction, basin_width;
   Sed_cell total, eroded;
   Sed_cell *lost_cell;
   linearProf_t linear_const;
   Sed_hydro river_data;
   Sed_cube river_profile;
   gssize *river_path;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( p == NULL )
   {
      if ( data->initialized )
      {
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->initialized = TRUE;
   }

   n_grains = sed_sediment_env_size();
   n_rivers = sed_cube_number_of_rivers( p );

//   for ( this_river=sed_cube_river_list(p) ; this_river ; this_river=this_river->next )

   for ( n=0,this_river = sed_cube_nth_river(p,n) ;
         n<n_rivers ;
         n++,this_river = sed_cube_nth_river(p,n) )
   {
      river_path = sed_cube_river_path_id( p , this_river , TRUE );
   
      river_profile = sed_cube_cols( p , river_path );
      if ( is_sedflux_3d() )
      {
         sed_cube_set_x_res( river_profile , 1. );
         sed_cube_set_y_res( river_profile ,
                          .5*(sed_cube_x_res(p)+sed_cube_y_res(p)) );
      }

      eh_free( river_path );
   
//      river_data    = ((Sed_river*)(this_river->data))->data;
      river_data = sed_river_hydro( this_river );

      if ( data->method == EROSION_DIFFUSION )
      {
         dt            = sed_cube_time_step_in_days( river_profile );
//         time_fraction = 1e-1;
         time_fraction = 1e-0;
         if ( is_sedflux_3d() )
            basin_width   = sed_cube_x_res( river_profile );
         else
            basin_width   = sed_cube_x_res( p );
   
         k_land  = get_paola_diffusion( river_data    , basin_width ,
                                        time_fraction , PAOLA_BRAIDED )
                 * S_SECONDS_PER_DAY;
         k_land *= sed_cube_storm( river_profile );
   
//         k_land = 200;
//         dt = 15;

         eh_message( "diffusion coefficient : %.3e m^2/day" , k_land );
         eh_message( "time step : %.3e days"                , dt     );

         lost_cell = diffuse_sediment( river_profile , k_land ,
                                       0             , dt     ,
                                       DIFFUSION_OPT_LAND|DIFFUSION_OPT_FILL );
          
         // convert time step to seconds.
//         dt = sed_get_profile_time_step_in_seconds(p);
         if ( lost_cell )
         {
            dx    = sed_cube_y_res( river_profile );
            width = sed_hydro_width( river_data );
   
            // add the eroded sediment to the river discharge.
            volume_eroded = sed_cell_thickness(lost_cell[1])*dx*width;
   
            sed_cell_separate_cell( lost_cell[1] , lost_cell[3] );
   
            dt *= S_SECONDS_PER_DAY;
            sed_hydro_add_cell( river_data , lost_cell[2] , volume_eroded );
            sed_hydro_add_cell( river_data , lost_cell[1] , volume_eroded );
   
            sed_cell_destroy( lost_cell[0] );
            sed_cell_destroy( lost_cell[1] );
            sed_cell_destroy( lost_cell[2] );
            sed_cell_destroy( lost_cell[3] );
            eh_free( lost_cell );
         }
   
         eh_message( "eroded sediment (m^3): %.3g" , volume_eroded );
   
      }
      else
      {
         eroded_fraction = eh_new( double , n_grains );
      
         for (i=0;i<n_grains;i++)
            eroded_fraction[i] = 1.;
      
         total  = sed_cell_new( n_grains );
         eroded = sed_cell_new( n_grains );
      
         if ( sed_cube_river_mouth_1d( river_profile )<0 )
            return info;
      
         dx    = sed_cube_y_res( river_profile );
         width = sed_cube_x_res( river_profile );
      
         i_river = sed_cube_river_mouth_1d( river_profile ) - 1;
         if ( i_river < 0 )
            return info;
      
         river_height = sed_cube_top_height(river_profile,0,i_river);
//         log_const = getLogConstants( OMEGA_DELTA         ,
//                                      data->stream_relief ,
//                                      data->stream_reach);
         linear_const = get_linear_constants(   data->stream_relief
                                              / data->stream_reach );
   
         for (i=i_river-1;i>=0;i--)
         {
            x = (i-i_river)*dx;
//            height = getLogHeight(x,log_const) + river_height;
            height = get_linear_height( x , linear_const ) + river_height;
      
            eroded_height = sed_cube_top_height(river_profile,0,i)-height;
            if ( eroded_height > 1e-12 )
            {
               sed_column_extract_top( sed_cube_col(river_profile,i) ,
                                       eroded_height                 ,
                                       eroded);
               sed_cell_add( total , eroded );
            }
         }
      
         if ( sed_cube_river_mouth_1d( river_profile )<0 )
            return info;
      
         // convert time step to seconds.
         dt = sed_cube_time_step_in_seconds( river_profile );
      
         // add the eroded sediment to the river discharge.
//         river_data = sed_cube_river_data( river_profile );
      
         volume_eroded = sed_cell_thickness(total)*dx*width;
      
         sed_hydro_add_cell( river_data  , total , volume_eroded );
      
         eh_message( "eroded sediment (m^3): %.3g" , volume_eroded );
      
         sed_cell_destroy( eroded );
         sed_cell_destroy( total  );
         eh_free( eroded_fraction );
      }

      sed_river_set_hydro( this_river , river_data );
      sed_hydro_destroy  ( river_data );

      sed_cube_free( river_profile , FALSE );
   }

   return info;
}

#define S_KEY_REACH  "reach of highest order stream"
#define S_KEY_RELIEF "relief of highest order stream"
#define S_KEY_METHOD "method"

gboolean init_erosion( Eh_symbol_table symbol_table,gpointer ptr)
{
   Erosion_t *data=(Erosion_t*)ptr;
   char *key;

   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   data->stream_reach  = eh_symbol_table_dbl_value( symbol_table , S_KEY_REACH  );
   data->stream_relief = eh_symbol_table_dbl_value( symbol_table , S_KEY_RELIEF );
   key                 = eh_symbol_table_lookup   ( symbol_table , S_KEY_METHOD );

   if ( strcasecmp( key , "DIFFUSION" )==0 )
      data->method = EROSION_DIFFUSION;
   else if ( strcasecmp( key , "SLOPE" )==0 )
      data->method = EROSION_SLOPE;
   else
   {
      fprintf( stderr , "error : unknown keyword, %s\n" , key );
      return FALSE;
   }

   return TRUE;
}

double get_paola_diffusion( Sed_hydro river_data ,
                            double basin_width   ,
                            double time_fraction ,
                            int river_type )
{
   double q, c_0, a, c_f, s;

   eh_require( river_data!=NULL );
   q = time_fraction
     * sed_hydro_water_flux( river_data )
     / sed_hydro_width( river_data )
     / basin_width;
   c_0 = sed_hydro_suspended_concentration( river_data );
   if ( river_type==PAOLA_MEANDERING )
      a = 1.;
   else if ( river_type==PAOLA_BRAIDED )
      a = pow( .4 / (1+.4) , 1.5 );
   else
      eh_require_not_reached();

   c_f = .01;
   s   = sed_rho_quartz()/sed_rho_sea_water();

   return 8*q*a*sqrt(c_f)/( c_0*(s-1) );
}

/* Various functions used to generate a longitudinal stream profile.
*/

#include <math.h>

/* A linear profile.
*/

linearProf_t get_linear_constants(double slope)
{
   linearProf_t c;
   c.a = slope;
   return c;
}

double get_linear_height(double x, linearProf_t c)
{
   return -x*c.a;
}

/* An exponential profile.
*/

typedef struct
{
   double a, b;
} expProf_t;

expProf_t getExpConstants(double a, double b)
{
   expProf_t c;
   c.a = a;
   c.b = b;
   return c;
}

double getExpProfile(double x, expProf_t c)
{
   return c.b*(exp(-c.a*x)-1);
}

/* A -log profile.
*/

/* Constants for a specific stream used to estimate it's 
   longitudinal profile.
*/

logProf_t getLogConstants(double omega,double h,double l)
{
   int i;
   logProf_t c;

   /* x0,y0 will be the location of the order 1 stream from the 
      river mouth.
   */
   c.y0 = omega*h;
   for (i=1,c.x0=0;i<=omega;i++)
      c.x0 -= l/pow(2,i-1);
   c.H1 = h;
   c.Rl = 2;
   c.b  = l/pow(2,omega-1)/(2-1.);

   return c;
}

double getLogHeight(double x,logProf_t c)
{
   double constant;
   constant = 1+(x-c.x0)/c.b;
   if ( constant < .0001 )
      constant = .0001;
   return c.y0-(c.H1/log(c.Rl))*log(constant);
}

