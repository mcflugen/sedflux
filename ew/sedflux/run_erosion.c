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

#define EH_LOG_DOMAIN EROSION_PROCESS_NAME_S

#include <stdio.h>
#include <string.h>

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <diffusion.h>
#include "my_processes.h"

//#include "erosion.h"

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

#define EROSION_ALGORITHM_DIFFUSION (1)
#define EROSION_ALGORITHM_SLOPE     (2)

Sed_cell erode_river_profile( Sed_cube p , Sed_riv r , double slope , gint method );

Sed_process_info
run_erosion( Sed_process proc , Sed_cube p )
{
   Erosion_t*       data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   Sed_riv* all;

//   for ( this_river=sed_cube_river_list(p) ; this_river ; this_river=this_river->next )
/*
   for ( n=0,this_river = sed_cube_nth_river(p,n) ;
         n<n_rivers ;
         n++,this_river = sed_cube_nth_river(p,n) )
*/

   all = sed_cube_all_leaves( p );

   if ( all )
   {
      Sed_riv* r;
      Sed_cell eroded_sed = NULL;

      for ( r=all ; *r ; r++ )
      {
         eh_debug( "Eroding along river %s" , sed_river_name_loc( *r ) );

         eroded_sed = erode_river_profile( p , *r , data->slope , data->method );
         sed_cell_destroy( eroded_sed );
/*
         river_path = sed_cube_river_path_id( p , *r , TRUE );
      
         river_profile = sed_cube_cols( p , river_path );

         if ( sed_mode_is_3d() )
         {
            sed_cube_set_x_res( river_profile , 1. );
            sed_cube_set_y_res( river_profile , .5*(sed_cube_x_res(p)+sed_cube_y_res(p)) );
         }
         else
         {
            sed_cube_set_x_res( river_profile , sed_cube_x_res(p) );
            sed_cube_set_y_res( river_profile , sed_cube_y_res(p) );
         }
   
         eh_free( river_path );
      
   //      river_data    = ((Sed_river*)(this_river->data))->data;
         river_data = sed_river_hydro( *r );
   
         if ( data->method == EROSION_ALGORITHM_DIFFUSION )
            eroded_sed = diffuse_profile( river_profile , river_data );
         else
            eroded_sed = erode_profile( river_profile , river_data );

         sed_hydro_add_cell( river_data , eroded_sed );

         sed_river_set_hydro( *r , river_data );
         sed_hydro_destroy  ( river_data );
   
         sed_cube_free( river_profile , FALSE );
*/

/*
         if ( data->method == EROSION_ALGORITHM_DIFFUSION )
         {

            dt            = sed_cube_time_step_in_days( river_profile );
   //         time_fraction = 1e-1;
            time_fraction = 1e-0;
            if ( sed_mode_is_3d() )
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
               volume_eroded = sed_cell_size(lost_cell[1])*dx*width;
      
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
         
            volume_eroded = sed_cell_size(total)*dx*width;
         
            sed_hydro_add_cell( river_data  , total , volume_eroded );
         
            eh_message( "eroded sediment (m^3): %.3g" , volume_eroded );
         
            sed_cell_destroy( eroded );
            sed_cell_destroy( total  );
            eh_free( eroded_fraction );
         }
   
         sed_river_set_hydro( *r , river_data );
         sed_hydro_destroy  ( river_data );
   
         sed_cube_free( river_profile , FALSE );
*/
      }

      eh_free( all );
   }

   return info;
}

#define EROSION_KEY_REACH           "reach of highest order stream"
#define EROSION_KEY_RELIEF          "relief of highest order stream"
#define EROSION_KEY_METHOD          "method"

static gchar* erosion_req_labels[] =
{
   EROSION_KEY_METHOD ,
   NULL
};

static gchar* erosion_slope_req_labels[] =
{
   EROSION_KEY_REACH  ,
   EROSION_KEY_RELIEF ,
   NULL
};

gboolean
init_erosion( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Erosion_t* data    = sed_process_new_user_data( p , Erosion_t );
   GError*    tmp_err = NULL;
   gchar**    err_s   = NULL;
   gboolean   is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( eh_symbol_table_require_labels( tab , erosion_req_labels , &tmp_err ) )
   {
      gchar*     key = eh_symbol_table_lookup( tab , EROSION_KEY_METHOD );

      if      ( strcasecmp( key , "DIFFUSION" )==0 ) data->method = EROSION_ALGORITHM_DIFFUSION;
      else if ( strcasecmp( key , "SLOPE"     )==0 ) data->method = EROSION_ALGORITHM_SLOPE;
      else
         g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_ALGORITHM ,
                      "Invalid erosion algorithm (diffusion or slope): %s" , key );

      if (    !tmp_err 
           && eh_symbol_table_require_labels( tab , erosion_slope_req_labels , &tmp_err ) )
      {
         data->stream_reach  = eh_symbol_table_dbl_value( tab , EROSION_KEY_REACH  );
         data->stream_relief = eh_symbol_table_dbl_value( tab , EROSION_KEY_RELIEF );

         eh_check_to_s( data->stream_reach>=0  , "Stream reach positive"  , &err_s );
         eh_check_to_s( data->stream_relief>=0 , "Stream relief positive" , &err_s );

         data->slope         = data->stream_relief / data->stream_reach;

         if ( !tmp_err && err_s )
            eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );
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
destroy_erosion( Sed_process p )
{
   if ( p )
   {
      Erosion_t* data = sed_process_user_data( p );

      if ( data ) eh_free( data );
   }

   return TRUE;
}

Sed_cell erode_profile  ( Sed_cube river_profile , double    slope      );
Sed_cell diffuse_profile( Sed_cube river_profile , Sed_hydro river_data );

Sed_cell
erode_river_profile( Sed_cube p , Sed_riv r , double slope , gint method )
{
   Sed_cell eroded_sed = NULL;

   if ( p && r )
   {
      gint*  river_path = sed_cube_river_path_id( p , r , TRUE );

      eh_require( river_path );

      if ( river_path )
      {
         Sed_cube  river_profile = sed_cube_cols( p , river_path );
         Sed_hydro river_data    = sed_river_hydro( r );

         eh_require( river_data    );
         eh_require( river_profile );

         if ( sed_mode_is_3d() )
         {
            sed_cube_set_x_res( river_profile , 1. );
            sed_cube_set_y_res( river_profile , .5*(sed_cube_x_res(p)+sed_cube_y_res(p)) );
         }
         else
         {
            sed_cube_set_x_res( river_profile , sed_cube_x_res(p) );
            sed_cube_set_y_res( river_profile , sed_cube_y_res(p) );
         }
   
         switch ( method )
         {
            case EROSION_ALGORITHM_DIFFUSION:
               eroded_sed  = diffuse_profile( river_profile , river_data ); break;
            case EROSION_ALGORITHM_SLOPE:
               eroded_sed  = erode_profile  ( river_profile , slope      ); break;
            default:
               eh_require_not_reached();
         }

         sed_hydro_add_cell( river_data , eroded_sed );

         sed_river_set_hydro( r , river_data );

         sed_hydro_destroy( river_data );
         eh_free          ( river_path );
         sed_cube_free    ( river_profile , FALSE );
      }
   }

   return eroded_sed;
}

double get_paola_diffusion( Sed_hydro river_data    ,
                            double    basin_width   ,
                            double    time_fraction ,
                            int       river_type );
Sed_cell
diffuse_profile( Sed_cube river_profile , Sed_hydro river_data )
{
   Sed_cell eroded_sed = NULL;

   eh_require( river_profile );

   if ( river_profile )
   {
      double    time_fraction = 1e-0;
      double    dt            = sed_cube_time_step_in_days( river_profile );
      double    width         = sed_cube_x_res( river_profile );
      Sed_cell* lost_cell     = NULL;
      double    k_land;
      
      k_land  = get_paola_diffusion( river_data , width , time_fraction , PAOLA_BRAIDED )
              * S_SECONDS_PER_DAY
              * sed_cube_storm( river_profile );
   //         k_land = 200;
   //         dt = 15;
/*
      eh_message( "diffusion coefficient : %.3e m^2/day" , k_land );
      eh_message( "time step : %.3e days"                , dt     );
*/
      lost_cell = diffuse_sediment( river_profile , k_land ,
                                    0             , dt     ,
                                    DIFFUSION_OPT_LAND|DIFFUSION_OPT_FILL );

      // convert time step to seconds.
   //         dt = sed_get_profile_time_step_in_seconds(p);
      if ( lost_cell )
      {
         double dx          = sed_cube_y_res ( river_profile );
         double river_width = sed_hydro_width( river_data    );
      
         /* The volume of sediment diffused through the right boundary.  This
            is the sediment that has made it to the ocean. */
//         volume_eroded = sed_cell_size(lost_cell[1])*dx*river_width;

         /* Bedload may have been added along the profile ([3]).  Remove this from the sediment
            that is diffused through the right boundary ([1]). */
         sed_cell_separate_cell( lost_cell[1] , lost_cell[3] );
      
//         dt *= S_SECONDS_PER_DAY;

         eroded_sed = sed_cell_new_env();

         sed_cell_add   ( eroded_sed , lost_cell[1] );
         sed_cell_add   ( eroded_sed , lost_cell[2] );
         sed_cell_resize( eroded_sed , sed_cell_size(eroded_sed)*dx*river_width );
/*
         // Add suspended sediment to the river
         sed_hydro_add_cell( river_data , lost_cell[2] , volume_eroded );

         // Add sediment diffused out of the right boundary to the river
         sed_hydro_add_cell( river_data , lost_cell[1] , volume_eroded );
*/
         sed_cell_destroy( lost_cell[0] );
         sed_cell_destroy( lost_cell[1] );
         sed_cell_destroy( lost_cell[2] );
         sed_cell_destroy( lost_cell[3] );

         eh_free( lost_cell );
      }
      
//      eh_message( "eroded sediment (m^3): %.3g" , volume_eroded );
   }

   return eroded_sed;
}
      
typedef struct
{
   double x0, y0;
   double H1, Rl, b;
} Erosion_log_st;

typedef struct
{
   double a, b;
} Erosion_lin_st;

typedef struct
{
   double a, b;
} Erosion_exp_st;

Erosion_lin_st erosion_get_linear_constants(double slope);
double         erosion_get_linear_height   (double x , Erosion_lin_st c);

Sed_cell
erode_profile( Sed_cube river_profile , double slope )
{
   Sed_cell eroded_sed = sed_cell_new_env();

   eh_require( river_profile );

   if (    river_profile 
        && sed_cube_river_mouth_1d( river_profile) > 0 )
   {
      Sed_cell eroded  = sed_cell_new_env( );
      gint     i_river = sed_cube_river_mouth_1d( river_profile ) - 1;
      double   dx      = sed_cube_y_res( river_profile );
      double   width   = sed_cube_x_res( river_profile );
      Erosion_lin_st linear_const = erosion_get_linear_constants( slope );
      gint i;
      double x;
      double height;
      double river_height;
      double erode_height;
         
      river_height = sed_cube_top_height( river_profile , 0 , i_river );
      
      for ( i=i_river-1 ; i>=0 ; i-- )
      {
         x      = (i-i_river)*dx;
         height = erosion_get_linear_height( x , linear_const ) + river_height;
         
         erode_height = sed_cube_top_height(river_profile,0,i)-height;
         if ( erode_height > 1e-12 )
         {
            sed_column_extract_top( sed_cube_col(river_profile,i) ,
                                    erode_height                 ,
                                    eroded);
            sed_cell_add( eroded_sed , eroded );
         }
      }
         
      // convert time step to seconds.
      //dt = sed_cube_time_step_in_seconds( river_profile );
         
            // add the eroded sediment to the river discharge.
   //         river_data = sed_cube_river_data( river_profile );
         
//      volume_eroded = sed_cell_size(eroded_sed)*dx*width;

      sed_cell_resize( eroded_sed , sed_cell_size(eroded_sed)*dx*width );
         
      //sed_hydro_add_cell( river_data  , eroded_sed );
         
//      eh_message( "eroded sediment (m^3): %.3g" , volume_eroded );
         
      sed_cell_destroy( eroded );
   }

   return eroded_sed;
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

   if      ( river_type==PAOLA_MEANDERING ) a = 1.;
   else if ( river_type==PAOLA_BRAIDED    ) a = pow( .4 / (1+.4) , 1.5 );
   else eh_require_not_reached();

   c_f = .01;
   s   = sed_rho_quartz()/sed_rho_sea_water();

   return 8*q*a*sqrt(c_f)/( c_0*(s-1) );
}

/* Various functions used to generate a longitudinal stream profile.
*/

#include <math.h>

/* A linear profile.
*/
Erosion_lin_st
erosion_get_linear_constants(double slope)
{
   Erosion_lin_st c;
   c.a = slope;
   return c;
}

double
erosion_get_linear_height(double x, Erosion_lin_st c)
{
   return -x*c.a;
}

/* An exponential profile.
*/
Erosion_exp_st
erosion_get_exp_constants(double a, double b)
{
   Erosion_exp_st c;
   c.a = a;
   c.b = b;
   return c;
}

double
erosion_get_exp_profile(double x, Erosion_exp_st c)
{
   return c.b*(exp(-c.a*x)-1);
}

/* A -log profile.
*/

/* Constants for a specific stream used to estimate it's 
   longitudinal profile.
*/

Erosion_log_st
erosion_get_log_constants(double omega,double h,double l)
{
   Erosion_log_st c;
   int i;

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

double
eroion_get_log_height( double x , Erosion_log_st c )
{
   double constant;
   constant = 1+(x-c.x0)/c.b;
   if ( constant < .0001 )
      constant = .0001;
   return c.y0-(c.H1/log(c.Rl))*log(constant);
}

