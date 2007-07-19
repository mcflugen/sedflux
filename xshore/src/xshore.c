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

/** \file xshore.c

   Cross-shore transport

   \defgroup xshore_group Along/Cross Shore Transport

   Model the transport of sediment over a 1D profile.

@{
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "xshore.h"

double get_closure_depth( Sed_cube p , Sed_wave wave );
double get_h_c( Sed_wave wave );
gssize get_zone_indices( Sed_cube p , double z_0 , double z_1 , gssize i_0 , Sed_grid_func get_val , gssize* ind );

typedef struct
{
   double u_0;   ///< Cross-shore current
   Sed_wave w;   ///< Incoming deep-water wave
   Sed_cube p; 
   gssize* ind;
   gssize ind_len;

   double x_0;   ///< Position of shoreline
   double x_b;   ///< Position of breaker zone
   double h_b;   ///< Depth of bruun zone
   double dz_dx; ///< Slope at bruun zone
   double *k;    ///< Sediment flux at bruun zone
}
Bruun_data G_GNUC_INTERNAL;

double get_total_flux( double z     ,
                       double dz_dx ,
                       Sed_wave w   ,
                       double u_0   ,
                       double w_s   ,
                       double breaker_depth ,
                       double h_b   ,
                       double x     ,
                       double x_b   ,
                       double max_qx ) G_GNUC_INTERNAL;
Sed_cell   move_sediment                 ( Sed_cube p            ,
                                           double** du           ,
                                           double* erosion_limit ,
                                           double z_0            ,
                                           double dt             ,
                                           Bruun_data* data      ,
                                           Sed_cell lost         ,
                                           Sed_cell added        ,
                                           Sed_cell in_suspension ) G_GNUC_INTERNAL;

void update_bruun_zone_data( Bruun_data* data )
{
   if ( data && data->ind_len>2 )
   {
      gssize n;
      double *w_s, u_om, d_max;
      double q_left, q_right;
      Sed_cube p = data->p;
      double breaker_depth = get_breaking_wave_depth( data->w );
      double h_b_left, h_b_right, x_b_left;
      gssize i_0 = data->ind[0];
      gssize i_b = data->ind[data->ind_len-1];
      gssize n_grains = sed_sediment_env_size();
      Sed_wave this_wave;

      eh_require( i_0 >= 0 );
      eh_require( i_b >= 0 );
      eh_require( i_b-1 >= 0 );

      data->x_0   = sed_cube_col_y( p,i_0 );
      data->x_b   = sed_cube_col_y( p,i_b );
      //data->h_b   = get_h_c( data->w );
      data->h_b   = sed_cube_water_depth(p,0,i_b);
      data->dz_dx = sed_cube_y_slope( p , 0 , i_b );
      h_b_left  = sed_cube_water_depth( p,0,i_b-1 );
      h_b_right = sed_cube_water_depth( p,0,i_b+1 );
      x_b_left  = sed_cube_col_y( p,i_b-1 );
      this_wave = sed_gravity_wave_new( data->w , data->h_b , NULL );

      w_s = sed_sediment_property( NULL , &sed_type_settling_velocity );

      for ( n=0 ; n<n_grains ; n++ )
      {
         w_s[n] /= S_SECONDS_PER_DAY;
         u_om  = get_near_bed_velocity( data->h_b ,
                                        data->w   ,
                                        breaker_depth );
         d_max = get_grain_size_threshold( u_om , sed_wave_period( data->w ) );
/*
      if ( sed_grain_size_in_meters(p->sed,n) > d_max )
{
         data->k[n] = 0;
eh_watch_int( n );
}
      else
*/
         data->k[n] = get_diffusion_constant( data->h_b ,
                                              this_wave ,
                                              w_s[n] ,
                                              breaker_depth )
                    * ( h_b_right - data->h_b )/( data->h_b - h_b_left )
                    * pow( (data->x_b-data->x_0) / (x_b_left-data->x_0) , 1.-XSHORE_BRUUN_M );

q_left = get_total_flux( h_b_left ,
                         sed_cube_y_slope(p,0,i_b-1) ,
                         this_wave ,
                         data->u_0 ,
                         w_s[n] ,
                         breaker_depth ,
                         data->h_b ,
                         x_b_left-data->x_0 ,
                         data->x_b-data->x_0 ,
                         data->k[n] );
q_right = get_total_flux( data->h_b ,
                          data->dz_dx ,
                          this_wave ,
                          data->u_0 ,
                          w_s[n] ,
                          breaker_depth ,
                          data->h_b ,
                          data->x_b-data->x_0 ,
                          data->x_b-data->x_0 ,   
                          data->k[n] );

      }

      sed_wave_destroy( this_wave );
   }
}

void       diffuse_cols                  ( Sed_cube p              ,
                                           Sed_wave deep_wave      ,
                                           double u_0               ,
                                           double* erosion_limit    ,
                                           Sed_column* source_col    ,
                                           double* bruun_depth        ,
                                           Sed_cell along_shore_cell ,
                                           Bruun_data* data  ,
                                           double dt                ,
                                           double t_total           ,
                                           Sed_cell added          ,
                                           Sed_cell in             ,
                                           Sed_cell out  ) G_GNUC_INTERNAL;
double**   get_sediment_flux             ( Sed_cube p ,
                                           Sed_wave deep_wave ,
                                           double u_0 ,
                                           Bruun_data *data ,
                                           Sed_cell in );
double     get_time_step                 ( Sed_cube p              ,
                                           Sed_wave deep_wave      ,
                                           double u_0               ,
                                           Bruun_data* data ) G_GNUC_INTERNAL;

/** Erode/deposit sediment over a 1D profile.

\param p                A Sed_cube (<em> must be 1 dimensional </em>)
\param along_shore_cell A cell of the type of sediment to be introduced to
                        the profile by long-shore transport
\param xshore_current   Magnitude of any offshore current (m/s)
\param storm            The ocean storm responsible for the cross-shore
                        transport of sediment

\return A Sed_cell list of the amount of sediment lost through the model
        boundaries
*/

Xshore_info xshore( Sed_cube p               ,
                    Sed_cell along_shore_cell ,
                    double xshore_current     ,
                    Sed_ocean_storm storm )
{
   double      z_0;
   double*     zone_dt;
   double**    bruun_depth;
   double**    max_erode_depth;
   Sed_wave    deep_water_wave;
   Sed_cube*   shelf_zone;
   Sed_column* source_col;
   gssize      n_zones;
   Xshore_info info;
   Bruun_data  bruun_zone_data;

   info.added = NULL;
   info.lost  = NULL;
   info.dt    = NULL;

   eh_require( sed_cube_is_1d(p) );
   eh_require( along_shore_cell );
   eh_require( storm            );

   eh_return_val_if_fail( sed_cube_is_1d(p)   , info );
   eh_return_val_if_fail( sed_cube_n_y(p) > 3 , info );

   deep_water_wave = sed_wave_new( sed_ocean_storm_wave_height( storm ) ,
                                   sed_ocean_storm_wave_number( storm ) ,
                                   sed_ocean_storm_wave_freq  ( storm ) );

   eh_return_val_if_fail( !sed_wave_is_bad(deep_water_wave) , info );

   eh_debug( "Is there a Bruun zone?" );
   {
      gboolean error = FALSE;
      gssize i;
//      gssize*   bruun_ind   = eh_new( gssize , sed_cube_n_y(p) );
      Sed_cube* bruun_zones = get_shelf_zones( p , get_h_c( deep_water_wave ) , NULL );
/*
      gssize    ind_len     = get_zone_indices( p ,
                                                0 ,
                                                get_h_c(deep_water_wave) ,
                                                0 ,
                                                S_WATER_DEPTH_FUNC ,
                                                bruun_ind );
*/

      if ( bruun_zones[0]==NULL || sed_cube_n_y(bruun_zones[0])<=3 )
      {
         // No bruun zone, exit
         error = TRUE;
         sed_wave_destroy( deep_water_wave );
      }

      for ( i=0 ; bruun_zones[i] ; i++ )
         sed_cube_free( bruun_zones[i] , FALSE );

      if ( error )
         return info;
   }

   eh_debug( "Calculate the water depth of the inner shelf" );
   {
      z_0 = get_closure_depth( p , deep_water_wave );   
      z_0 = 100.;
      info.z_0 = z_0;
   }

   eh_debug( "Divide the profile into smaller regions" );
   {
      shelf_zone = get_shelf_zones( p , z_0 , NULL );
      for ( n_zones=0 ; shelf_zone[n_zones] ; n_zones++ );
      info.n_zones = n_zones;

      eh_require( n_zones>0 );
   }

   eh_debug( "Calculate the Bruun profile" );
   if ( n_zones>0 )
   {
      gssize i;
      double* y;
//      gssize*   bruun_ind   = eh_new( gssize , sed_cube_n_y(p) );
      gssize**  bruun_ind   = eh_new( gssize* , 2 );
      Sed_cube* bruun_zones = get_shelf_zones( p , get_h_c( deep_water_wave ) , bruun_ind );
/*
      gssize    ind_len     = get_zone_indices( p ,
                                                0 ,
                                                get_h_c(deep_water_wave) ,
                                                0 ,
                                                S_WATER_DEPTH_FUNC ,
                                                bruun_ind );
*/

      bruun_depth = eh_new0( double* , n_zones );

      if ( bruun_zones[0] && sed_cube_n_y(bruun_zones[0])>3 )
      {
         double bruun_m = XSHORE_BRUUN_M;
         double bruun_a = get_bruun_a  ( bruun_zones[0] , bruun_m );
         double y_0     = get_bruun_y_0( bruun_zones[0] );
         double y_b     = get_bruun_y_b( bruun_zones[0] );

         for ( i=0 ; i<1; i++ )
         {
            y = sed_cube_y( shelf_zone[i] , NULL );
         
            bruun_depth[i] = get_bruun_profile( y ,
                                               sed_cube_n_y(shelf_zone[i]) ,
                                               bruun_a ,
                                               bruun_m ,
                                               y_0 ,
                                               y_b );

            eh_free( y );
         }

         bruun_zone_data.p        = p;
         bruun_zone_data.ind_len  = sed_cube_n_y(bruun_zones[0]);
         bruun_zone_data.ind      = g_memdup( bruun_ind[0] , sizeof(gssize)*bruun_zone_data.ind_len );
         bruun_zone_data.w        = deep_water_wave;
         bruun_zone_data.u_0      = xshore_current;
         bruun_zone_data.k        = g_new( double , sed_sediment_env_size() );

         update_bruun_zone_data( &bruun_zone_data );

         info.bruun_a   = bruun_a;
         info.bruun_m   = bruun_m;
         info.bruun_y_0 = y_0;
         info.bruun_y_b = y_b;
         info.bruun_h_b = sed_cube_water_depth( bruun_zones[0] ,
                                                0              ,
                                                sed_cube_n_y(bruun_zones[0])-1 );
      }

      for ( i=0 ; i<n_zones ; i++ )
      {
         sed_cube_free( bruun_zones[i] , FALSE );
         eh_free( bruun_ind[i] );
      }
      eh_free( bruun_zones  );
      eh_free( bruun_ind );

   }

   eh_debug( "Find the column where suspended sediment is added" );
   {
      gssize i;
      Sed_cube* zone = get_shelf_zones( p , 30. , NULL );

      if ( zone[1] )
      {
         source_col    = g_new0( Sed_column , 2 );
         source_col[0] = sed_cube_col(zone[1],0);
      }
      else
         source_col = NULL;

      for ( i=0 ; zone[i] ; i++ )
         sed_cube_free( zone[i] , FALSE );
      eh_free( zone  );
   }

   eh_debug( "Calculate the maximum erosion depth for this storm" );
   if ( n_zones>0 )
   {
      gssize i;

      max_erode_depth = eh_new0( double* , n_zones );
      for ( i=0 ; i<1 ; i++ )
         max_erode_depth[i] = get_max_erosion_profile( shelf_zone[i] , deep_water_wave );
   }

   eh_debug( "Calculate the time step for each region" );
   if ( n_zones>0 )
   {
      gssize i;
      double t_total = sed_ocean_storm_duration(storm);

      zone_dt = eh_new( double , n_zones+1 );
      for ( i=0 ; i<1 ; i++ )
      {
         zone_dt[i] = get_time_step( shelf_zone[i]   ,
                                     deep_water_wave ,
                                     xshore_current  ,
                                     &bruun_zone_data );
         if ( zone_dt[i] > t_total )
            zone_dt[i] = t_total;
      }
      zone_dt[n_zones] = zone_dt[n_zones-1];

      info.dt = g_memdup( zone_dt , sizeof(double)*n_zones );
   }


   eh_debug( "Diffuse each region" );
   if ( n_zones>0 )
   {
      double m_0 = sed_cube_mass( p );
      double m_added, m_lost, m_1;
      gssize i;
      double t;
      double       t_total = sed_ocean_storm_duration(storm);
      gssize      n_grains = sed_sediment_env_size();
      Sed_cell  total_lost = sed_cell_new( n_grains );
      Sed_cell total_added = sed_cell_new( n_grains );
      Sed_cell          in = sed_cell_new( n_grains );
      Sed_cell         out = sed_cell_new( n_grains );
      Sed_cell       added = sed_cell_new( n_grains );

      eh_require( t_total>0 );

      for ( t=0 ; t<t_total ; )
      {
         sed_cell_resize( in  , 0. );
         for ( i=0 ; i<1 ; i++ )
         {
            diffuse_cols( shelf_zone[i]   ,
                          deep_water_wave , xshore_current ,
                          max_erode_depth[i] , source_col ,
                          bruun_depth[i]  , along_shore_cell ,
                          &bruun_zone_data ,
                          zone_dt[0] , t_total , added ,
                          NULL              , out );
         }

         t += t_total;

         sed_cell_add( total_lost , in );
      }
m_1 = sed_cube_mass(p);

      sed_cell_add( total_added , out );

m_added = sed_cell_mass(added)*sed_cube_x_res(p)*sed_cube_y_res(p);
m_lost  = sed_cell_mass(out)*sed_cube_x_res(p)*sed_cube_y_res(p);
if ( fabs((m_0+m_added-m_lost-m_1)/m_1) > .01 )
{
   eh_watch_dbl( m_0 );
   eh_watch_dbl( m_1 );
   eh_watch_dbl( m_added );
   eh_watch_dbl( m_lost );
   exit(0);
}

      sed_cell_destroy( in   );
      sed_cell_destroy( total_added );
      sed_cell_destroy( total_lost );

      info.lost  = out;
      info.added = added;
   }

   eh_debug( "Free memory" );
   {
      gssize i;

      eh_free( zone_dt );
      sed_wave_destroy( deep_water_wave );
      for ( i=0 ; i<n_zones ; i++ )
      {
         sed_cube_free( shelf_zone[i] , FALSE );
         eh_free( bruun_depth[i] );
         eh_free( max_erode_depth[i] );
      }
      eh_free( shelf_zone  );
      eh_free( bruun_depth );
      eh_free( max_erode_depth );
      eh_free( source_col );

      eh_free( bruun_zone_data.ind );
      eh_free( bruun_zone_data.k   );
   }
   eh_debug( "Done xshore" );

   return info;
}

//---
// get the water depths at each point along the profile.
// also calculate the breaking wave depth.  the wave will break if
// its height is greater than .78 times the water depth.
//---

double wave_break_helper( double z , gpointer user_data ) G_GNUC_INTERNAL;

double get_breaking_wave_depth( Sed_wave deep_water )
{
   Sed_wave waves[2];
   Sed_wave this_wave = sed_wave_new( 0,0,0 );
   double min_h, max_h;

   eh_require( deep_water );
   eh_require( !sed_wave_is_bad(deep_water) );

   waves[0] = deep_water;
   waves[1] = this_wave;

   min_h = sed_wave_height( deep_water )/2.;
   max_h = sed_wave_height( deep_water )*2.;
//   min_h = .01;
//   max_h = 100.;

   return eh_bisection( &wave_break_helper , min_h , max_h , .1 , waves );
}

double wave_break_helper( double z , gpointer user_data )
{
   double ans;
   Sed_wave deep_wave = ((Sed_wave*)(user_data))[0];
   Sed_wave this_wave = ((Sed_wave*)(user_data))[1];
   sed_gravity_wave_new( deep_wave , z , this_wave );
   ans = sed_wave_height(this_wave)/z - .78;

   return ans;
}
      
/** Find the indices for the inner and outer shelf

@param[in]      p              A Sed_cube
@param[in]      z_0            Water depth limit of the inner shelf
@param[in,out]  shelf_ind      Array of arrays of indices that make up each zone

\return An array of Sed_cube's.  One for each zone.
*/
Sed_cube* get_shelf_zones( Sed_cube p , double z_0 , gssize** shelf_ind )
{
   const gssize n_zones = 2;
   double boundary[3];
   Sed_cube* shelf_zones, *all_zones;
   gssize** zone_ind;

   eh_require( p );

   eh_clamp( z_0 , .1 , G_MAXDOUBLE );

   boundary[0] = .1;
   boundary[1] = z_0;
   boundary[2] = G_MAXDOUBLE;

   if ( shelf_ind )
      memset( shelf_ind , 0 , n_zones*sizeof(gssize*) );

   zone_ind  = eh_new( gssize* , n_zones );
   all_zones = get_zones( p , boundary , n_zones , S_WATER_DEPTH_FUNC , zone_ind );

   eh_debug( "Get rid of NULL zones" );
   {
      gssize i;
//      gssize n = 0;
      shelf_zones = eh_new0( Sed_cube , n_zones+1 );
/*
      for ( i=0 ; i<n_zones ; i++ )
         if ( all_zones[i] )
         {
            shelf_zones[n] = all_zones[i];
            shelf_ind[n]   = zone_ind[i];
            n = n+1;
         }
      shelf_zones[n] = NULL;
*/
      memcpy( shelf_zones , all_zones , sizeof(Sed_cube)*n_zones );
      shelf_zones[n_zones] = NULL;

      if ( shelf_ind )
         memcpy( shelf_ind , zone_ind , sizeof(gssize*)*n_zones );
      else
      {
         for ( i=0 ; i<n_zones ; i++ )
            eh_free( zone_ind[i] );
      }
   }

   eh_free( all_zones );
   eh_free( zone_ind  );

   return shelf_zones;
}

Sed_cube* get_bruun_zones( Sed_cube p , double y_0 )
{
   Sed_cube* shelf_zones;
   Sed_cube* all_zones;
   const gssize n_zones = 2;
   gssize i_rm;

   eh_require( p );

   eh_debug( "Find the start of the Bruun profile" );
   {
      double sea_level = sed_cube_sea_level( p );

      sed_cube_set_sea_level( p , sea_level );

      i_rm = sed_cube_river_mouth_1d( p ) - 1;
      eh_lower_bound( i_rm , 0 );

      sed_cube_set_sea_level( p , sea_level );
   }

   eh_debug( "Get the zones of the Bruun profile" );
   {
      double boundary[3];
      double y_rm = sed_cube_col_y( p , i_rm );

      boundary[0] = 0   + y_rm;
      boundary[1] = y_0 + y_rm;
      boundary[2] = G_MAXDOUBLE;

      all_zones = get_zones( p , boundary , n_zones , S_Y_FUNC , NULL );
   }

   eh_debug( "Get rid of NULL zones" );
   {
      gssize i;
      gssize n = 0;
      shelf_zones = eh_new0( Sed_cube , n_zones+1 );

      for ( i=0 ; i<n_zones ; i++ )
         if ( all_zones[i] )
         {
            shelf_zones[n] = all_zones[i];
            n = n+1;
         }
      shelf_zones[n] = NULL;
   }

   eh_free( all_zones );

   return shelf_zones;
}


/** Find indices of depth-defined zones of a Sed_cube

\note The length of the array \a z will be one greater than the number
of zones, \a n_zones since it defines the lower and upper boundaries
of each zone.

\param p         A Sed_cube
\param z         Zone depth-boundaries
\param n_zones   The number of zones
\param f         Function that get a parameter from a Sed_cube
\param ind       Array of arrays for the columns in each zone

\return          A array of Sed_cube's.  One for each zone.
*/
Sed_cube* get_zones( Sed_cube p , double* z , gssize n_zones , Sed_grid_func f , gssize** ind )
{
   Sed_cube* sub_cube;
   gssize **zones;
   gssize n;

   eh_require( p                 );
   eh_require( z                 );
   eh_require( sed_cube_is_1d(p) );
   eh_require( n_zones>0         );

   eh_require( eh_dbl_array_is_monotonic( z , n_zones+1 ) );

   zones = eh_new( gssize* , n_zones );
   for ( n=0 ; n<n_zones ; n++ )
      zones[n] = eh_new( gssize , sed_cube_n_y(p) );

   eh_require( zones )
   {
      gssize len = get_zone_indices( p , z[0] , z[1] , 0 , f , zones[0] );
      for ( n=1 ; n<n_zones ; n++ )
         len = get_zone_indices( p , z[n] , z[n+1] , zones[n-1][len-1] , f , zones[n] );
   }

   sub_cube = eh_new0( Sed_cube , n_zones+1 );
   for ( n=0 ; n<n_zones ; n++ )
   {
      if ( zones[n][0]>0 )
         sub_cube[n] = sed_cube_cols( p , zones[n] );
      else
         sub_cube[n] = NULL;
   }

   if ( ind )
      memcpy( ind , zones , sizeof(gssize*)*n_zones );
   else
      for ( n=0 ; n<n_zones ; n++ )
         eh_free(zones[n]);

   eh_free( zones );

   return sub_cube;
}

gssize get_zone_indices( Sed_cube p ,
                         double z_0 ,
                         double z_1 ,
                         gssize i_0 ,
                         Sed_grid_func get_val ,
                         gssize* ind )
{
   gssize max_i;
   gssize n = 0;

   eh_require( p       );
   eh_require( z_1>z_0 );
   eh_require( i_0>=0  );
   eh_require( ind     );

   max_i = sed_cube_n_y(p)-1;

   // Check if the right edge of the Sed_cube is in this zone.
   if ( get_val( p,0,max_i)>z_0 )
   {
      gssize i;

      // Find the index of right boundary
      for (  ; get_val(p,0,i_0)<z_0 ; i_0++ );

      // Check if the right boundary is within the profile.  If so, we
      // don't need to check for exceeding the array limits.
      if ( get_val(p,0,max_i)>z_1 )
         for ( i=i_0,n=0 ; get_val(p,0,i)<z_1 ; i++ )
         {
            ind[n] = i;
            n = n+1;
         }
      else
         for ( i=i_0,n=0 ; i<sed_cube_n_y(p) && get_val(p,0,i)<z_1 ; i++ )
         {
            ind[n] = i;
            n = n+1;
         }
      ind[n] = -1;
   }
   else
      ind[0] = -1;

   return n;
}

/** Find the time step necessary for stability within a zone

The equation we are looking to solve can be written as,

\f[
   {\partial \eta \over \partial t} =
        c v_0 {\partial \over \partial x } \left( U_{om}^3 \right)
      + c     {\partial \over \partial x } \left( U_{om}^5 {\partial \eta \over \partial x}\right)
\f]

For the diffusive term to be stable, we require,

\f[
   { 2 \left| c U_{om}^5 \right| \Delta t \over \left( \Delta x \right)^2} \le 1
\f]

The advective term can be rewritten as,

\f[
   c v_0 {\partial \over \partial x } \left( U_{om}^3 \right)
   = c v_0 3 U_{om}^2 { \partial U_{om} \over \partial \eta }
                      { \partial \eta \over \partial x }
\f]

Thus, for stability we require

\f[
   {\Delta t\over \Delta x }
      \left| 3 c v_0 U_{om}^2 {\partial U_{om} \over \partial \eta} \right|
   \le 1
\f]

\param p         A Sed_cube
\param deep_wave The incoming deep-water wave
\param u_0       Cross-shore current
\param data      Date that describe the Bruun zone

\return The time step in days
*/
double get_time_step( Sed_cube p , Sed_wave deep_wave , double u_0 , Bruun_data* data )
{
   double dt;
   double breaker_depth;
   double k_max = 0;

   eh_require( p                           );
   eh_require( deep_wave                   );
   eh_require( !sed_wave_is_bad(deep_wave) );

   breaker_depth = get_breaking_wave_depth( deep_wave );
/*
   {
      double* w_s = sed_sediment_property( NULL , &sed_type_settling_velocity );
      Sed_wave this_wave = sed_gravity_wave_new( deep_wave , breaker_depth , NULL );

      k_max     = get_diffusion_constant( breaker_depth     ,
                                          this_wave ,
                                          w_s[2] ,
                                          breaker_depth )
                * S_SECONDS_PER_DAY;

      eh_free( w_s );
      sed_wave_destroy( this_wave );
   }
*/
   {
      gssize n_grains = sed_sediment_env_size();
      double* w_s = sed_sediment_property( NULL , &sed_type_settling_velocity );
      Sed_wave this_wave = sed_gravity_wave_new( deep_wave , data->h_b , NULL );

      k_max     = get_diffusion_constant( data->h_b ,
                                          this_wave ,
                                          //w_s[4]    ,
                                          w_s[n_grains-1]    ,
                                          breaker_depth )
                * S_SECONDS_PER_DAY;

      eh_free( w_s );
      sed_wave_destroy( this_wave );
   }

   eh_debug( "Find the time step necessary for stability" );
   {
      double dy  = sed_cube_y_res(p);
      double fos = .9;

      dt = fos*dy*dy/(2*k_max);

      if ( dt*u_0*S_SECONDS_PER_DAY > dy )
         dt = fos*dy/(u_0*S_SECONDS_PER_DAY);
   }

   return dt;
}

double get_constant( double z , Sed_wave w , double w_s , double h_b )
{
   double rho    = sed_rho_sea_water();
   double rho_s  = sed_rho_quartz();
   double c_fs   = .01;
   double eps_ss = .01;
   double g      = sed_gravity();
   double u_om   = get_near_bed_velocity( z , w , h_b );
   double i_s    = 1.;

   return   16./(3*M_PI)
          * rho/(rho_s-rho)
          * c_fs*eps_ss/g
          * i_s*pow(u_om,3.)/w_s;
}

double get_diffusion_constant( double z   ,
                               Sed_wave w ,
                               double w_s ,
                               double h_b )
{
   double u_om = get_near_bed_velocity( z , w , h_b );
   return   get_constant( z , w , w_s , h_b )
          * pow(u_om,2.)/(5.*w_s);
}

/** Get the component of sediment flux due to diffusion

\param z     Water depth in meters
\param dz_dx Bathymetric slope
\param w     A Sed_wave
\param w_s   Settling velocity in meters per second
\param h_b   Depth of breaking waves in meters

\return Sediment flux in m^2/s
*/
double get_diffusion_flux( double z     ,
                           double dz_dx ,
                           Sed_wave w   ,
                           double w_s   ,
                           double h_b )
{
   return get_diffusion_constant( z , w , w_s , h_b )*dz_dx;
}

/** Get the component of sediment flux due to advection

\param z     Water depth in meters
\param w     A Sed_wave
\param u_0   Cross-shore current velocity in meters per second
\param w_s   Settling velocity in meters per second
\param h_b   Depth of breaking waves in meters

\return Sediment flux in m^2/s
*/
double get_advection_flux( double z   ,
                           Sed_wave w ,
                           double u_0 ,
                           double w_s ,
                           double h_b )
{
   return get_constant( z , w , w_s , h_b ) * u_0;
}

/** Get the total sediment flux due to both diffusion and advection

\f[
   q_s = {16\over 3\pi} {\rho\over\rho_s-\rho} { C_{fs}\epsilon_{ss}\over g} I_s {U_{om}^3\over W_s}
         \left( v_0 + {U_{om}^2 \over 5 W_s} {\partial h\over\partial x} \right) 
\f]

where \f$ rho \f$ is density of water, \f$ \rho_s \f$ is density of quartz, \f$ g \f$ is
acceleration due to gravity, \f$ I_s \f$ is an intermittancy constant, \f$ U_{om} \f$
is the near-bed wave orbital velocity, \f$ W_s \f$ is settling velocity, \f$ v_0 \f$
is cross-shore current velocity, \f$ h \f$ is bathymetric elevation, and \f$ x \f$ is
position.

\param z     Water depth in meters
\param dz_dx Bathymetric slope
\param w     A Sed_wave
\param u_0   Cross-shore current velocity in meters per second
\param w_s   Settling velocity in meters per second
\param h_b   Depth of breaking waves in meters

\return Sediment flux in m^2/s
*/
double
get_total_flux_outer_shelf( double z     ,
                            double dz_dx ,
                            Sed_wave w   ,
                            double u_0   ,
                            double w_s   ,
                            double h_b )
{
   double u_om = get_near_bed_velocity( z , w , h_b );
   return   get_constant( z , w , w_s , h_b )
          * ( u_0 + pow(u_om,2.)/(5.*w_s)*dz_dx );
}

/** Get the total flux at the Bruun zone

\param x        Position from shore
\param dz_dx    Bathymetric slope
\param x_0      Distance from shore to breaker zone
\param k_0      Diffusion coefficient at the edge of the Bruun zone
*/
double
get_total_flux_bruun_zone( double x , double dz_dx , double x_0 , double k_0 )
{
   eh_require( x>=0   );
   eh_require( x_0>0  );
   eh_require( x<=x_0 );
   eh_require( k_0>=0 );

   return k_0*pow(x/x_0,1.-XSHORE_BRUUN_M)*dz_dx;
}

double get_total_flux( double z     ,
                       double dz_dx ,
                       Sed_wave w   ,
                       double u_0   ,
                       double w_s   ,
                       double breaker_depth ,
                       double h_b   ,
                       double x     ,
                       double x_b   ,
                       double max_k )
{
      return get_total_flux_outer_shelf( z , dz_dx , w , u_0 , w_s , breaker_depth );
}

/** Diffuse sediment of Sed_cube due to ocean waves.

\note The thickness of \a in and \a out represent the \e flux of sediment into
and out of the Sed_cube in meters per second.

\param p                A Sed_cube
\param deep_wave        A Sed_wave causing sediment movement
\param u_0              Cross-shore current
\param erosion_limit    Maximum depth of erosion along profile
\param source_col       Unused
\param bruun_depth      Unused
\param along_shore_cell Unused
\param data             Data that describe the Bruun zone
\param dt               Time step to use (in seconds)
\param t_total          The duration of the ocean storm (in seconds)
\param added            A Sed_cell to record sediment that is added to the profile
\param in               A Sed_cell containing sediment flux into to Sed_cube
\param out              A Sed_cell containing sediment flux out of the Sed_cube

\todo Tidy up function diffuse_cols.

*/
void diffuse_cols( Sed_cube p                ,
                   Sed_wave deep_wave        ,
                   double u_0                ,
                   double* erosion_limit     ,
                   Sed_column* source_col    ,
                   double* bruun_depth       ,
                   Sed_cell along_shore_cell ,
                   Bruun_data* data          ,
                   double dt                 ,
                   double t_total            ,
                   Sed_cell added            ,
                   Sed_cell in               ,
                   Sed_cell out  )
{
   eh_require( p         );
   eh_require( deep_wave );
   eh_require( !sed_wave_is_bad(deep_wave) );
   eh_require( dt>0      );
   eh_require( t_total>0 );

   if ( dt>t_total )
      dt = t_total;

   if ( sed_cube_n_y(p)>1 )
   {
      double t, **qy;
      Sed_cell suspended_cell = sed_cell_new_env( );
      double z_0 = get_closure_depth( p , deep_wave );
      double m_0 = sed_cube_mass(p), m_1;
      double m_added, m_lost;

      // Diffuse the sediment.  Calculate the fluxes, remove the sediment,
      // and move it to the next column.
      for ( t=dt ; t<t_total ; t+=dt )
      {
         qy = get_sediment_flux( p , deep_wave , u_0 , data , in );
         move_sediment( p , qy , erosion_limit , z_0 , dt , data , out , added , suspended_cell );
m_added = sed_cell_mass( added )*sed_cube_x_res(p)*sed_cube_y_res(p);
m_lost = sed_cell_mass( out )*sed_cube_x_res(p)*sed_cube_y_res(p);
m_1=sed_cube_mass( p );
//eh_watch_dbl( m_0 );
//eh_watch_dbl( m_added );
//eh_watch_dbl( m_lost );
//eh_watch_dbl( m_1 );
if ( fabs((m_0+m_added-m_lost-m_1)/m_1) > .01 )
{
   eh_watch_dbl( sed_cell_mass( added ) );
   eh_watch_dbl( dt );
   eh_watch_dbl( m_0 );
   eh_watch_dbl( m_1 );
   eh_watch_dbl( m_added );
   eh_watch_dbl( m_lost );
   exit(0);
}
         update_bruun_zone_data( data );
         sed_cell_resize( suspended_cell , 0. );
         eh_free_2( qy );
      }

      // Do the last partial time step, if need be.
      if ( t>=t_total )
      {
         dt = t_total-(t-dt);
         qy = get_sediment_flux( p , deep_wave , u_0 , data , in );
         move_sediment( p , qy , erosion_limit , z_0 , dt , data , out , added , suspended_cell );
m_added = sed_cell_mass( added )*sed_cube_x_res(p)*sed_cube_y_res(p);
m_lost = sed_cell_mass( out )*sed_cube_x_res(p)*sed_cube_y_res(p);
m_1=sed_cube_mass( p );
//eh_watch_dbl( m_0 );
//eh_watch_dbl( m_added );
//eh_watch_dbl( m_lost );
//eh_watch_dbl( m_1=sed_cube_mass( p ) );
if ( fabs((m_0+m_added-m_lost-m_1)/m_1) > .01 )
{
   eh_watch_dbl( sed_cell_mass( added ) );
   eh_watch_dbl( dt );
   eh_watch_dbl( m_0 );
   eh_watch_dbl( m_1 );
   eh_watch_dbl( m_added );
   eh_watch_dbl( m_lost );
   exit(0);
}
         update_bruun_zone_data( data );
         sed_cell_resize( suspended_cell , 0. );
         eh_free_2( qy );
      }
eh_debug( "DONE" );

      sed_cell_destroy( suspended_cell );
   }

   return;
}

double** get_sediment_flux( Sed_cube p , Sed_wave deep_wave , double u_0 , Bruun_data *data , Sed_cell in )
{
   double** du;

   eh_require( p                  );
   eh_require( deep_wave          );
   eh_require( u_0>=0             );

   du = eh_new_2( double , sed_cube_n_y(p) , sed_sediment_env_size() );

   eh_require( du )
   {
      gssize i, n;
      double u_om, d_max, qy;
      gssize     n_grains  = sed_sediment_env_size();
      Sed_wave this_wave   = sed_wave_new( 0 , 0 , 0 );
      double  wave_period  = sed_wave_period( deep_wave );
      double breaker_depth = get_breaking_wave_depth( deep_wave );
      double            dy = sed_cube_y_res( p );
      Eh_dbl_grid   z_grid = sed_cube_water_depth_grid( p , NULL );
      Eh_dbl_grid dz_dy_grid = sed_cube_y_slope_grid( p , NULL );
      double*            z = eh_grid_data_start( z_grid );
      double*        dz_dy = eh_grid_data_start( dz_dy_grid );
      double depth, y_b, y, *k_b;
      double* gz  = sed_sediment_property( NULL , &sed_type_grain_size_in_meters );
      double* w_s = sed_sediment_property( NULL , &sed_type_settling_velocity );

      for ( n=0 ; n<n_grains ; n++ )
         w_s[n] /= S_SECONDS_PER_DAY;

      eh_dbl_array_set( du[0] , n_grains*sed_cube_n_y(p) , 0. );

      y_b = data->x_b - data->x_0;
      k_b = data->k;

      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      {
         if ( i!=sed_cube_n_y(p)-1 )
            depth = (z[i]+z[i+1])*.5;
         else
            depth = z[i];

         y = sed_cube_col_y( p , i ) - data->x_0;

         if ( depth>.01 )
         {

            this_wave = sed_gravity_wave_new( deep_wave , depth , this_wave );

            eh_require( !sed_wave_is_bad( this_wave ) );

            u_om  = get_near_bed_velocity( depth     ,
                                           this_wave ,
                                           breaker_depth );
            d_max = get_grain_size_threshold( u_om , wave_period );

//d_max = G_MAXDOUBLE;

            for ( n=0 ; n<n_grains ; n++ )
            {
               if ( gz[n] > d_max )
                  qy = 0;
               else
               {
                  qy  = get_total_flux( depth      ,
                                        dz_dy[i]   ,
                                        this_wave  ,
                                        u_0        ,
                                        w_s[n]     ,
                                        breaker_depth ,
                                        data->h_b  ,
                                        y          ,
                                        y_b        ,
                                        k_b[n] )
                      * S_SECONDS_PER_DAY
                      / dy
                      / (double)n_grains;
               }

               du[i][n] = qy;
            }

         }

      }

      eh_free( gz  );
      eh_free( w_s );
      eh_grid_destroy( dz_dy_grid , TRUE );
      eh_grid_destroy( z_grid , TRUE );
   }

   {
      gssize i, n;
      gssize n_grains  = sed_sediment_env_size();
      gssize* bruun_ind = data->ind;
      gssize  ind_len = data->ind_len;
/*
      gssize* bruun_ind = eh_new( gssize , sed_cube_n_y(p)+1 );
      gssize ind_len = get_zone_indices( p ,
                                         0 ,
                                         get_h_c( deep_wave ) ,
                                         0 ,
                                         S_WATER_DEPTH_FUNC ,
                                         bruun_ind );
*/
      double y_b, y_0, y;
      double* k_max = eh_new( double , sed_sediment_env_size() );

      if ( ind_len>0 )
      {
         gssize i_b = bruun_ind[ind_len-1] - bruun_ind[0];
         // Get Bruun k for each grain so that the fluxes match.
         i = bruun_ind[ind_len-1] + 1;
         for ( n=0 ; n<n_grains ; n++ )
            k_max[n] = du[i_b+1][n]/sed_cube_y_slope( p , 0 , i_b );
//            k_max[n] = du[i_b][n]/sed_cube_y_slope( p , 0 , bruun_ind[ind_len-1] );

         // calculate the fluxes within the Bruun region
//         y_0 = sed_cube_col_y( p , bruun_ind[0]         ) - sed_cube_y_res(p);
//         y_b = sed_cube_col_y( p , bruun_ind[ind_len-1] ) - y_0;
         y_0 = sed_cube_col_y( p , 0   ) - sed_cube_y_res(p);
         y_b = sed_cube_col_y( p , i_b ) - y_0;
//         for ( i=bruun_ind[0] ; i<=bruun_ind[ind_len-1] ; i++ )
         for ( i=0 ; i<=i_b ; i++ )
         {
            y = sed_cube_col_y( p , i ) - y_0;
            for ( n=0 ; n<n_grains ; n++ )
{
               du[i][n] = k_max[n]*pow( y / y_b , 1.-XSHORE_BRUUN_M )*sed_cube_y_slope(p,0,i);
//fprintf( stderr , "du[%d][%d] = %f , kmax = %f\n" , i , n , du[i][n] , k_max[n] );
}
         }
      }

/*
      eh_free( bruun_ind );
      eh_free( k_max );
*/
   }

   if ( in )
   {
      gssize n;
      gssize n_grains = sed_sediment_env_size();
      double t        = sed_cell_size(in);

      for ( n=0 ; n<n_grains ; n++ )
         du[0][n] += t*sed_cell_nth_fraction(in,n);
   }

   return du;
}

Sed_cell move_sediment( Sed_cube p            ,
                        double** du           ,
                        double* erosion_limit ,
                        double z_0            ,
                        double dt             ,
                        Bruun_data* data      ,
                        Sed_cell lost         ,
                        Sed_cell added        ,
                        Sed_cell in_suspension )
{
   gssize n_grains;
   Sed_cell* rem_cell;

   eh_require( p    );
   eh_require( du   );
   eh_require( lost );

   n_grains = sed_sediment_env_size();

   // Convert the fluxes to amounts.  Create a temporary array to hold
   // the removed sediment.
   {
      gssize i, n;
      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
         for ( n=0 ; n<n_grains ; n++ )
{
            du[i][n] *= dt;
if ( du[i][n] > 10 )
{
   eh_watch_int( i );
   eh_watch_int( n );
   eh_watch_int( sed_cube_n_y(p) );
   eh_watch_dbl( du[i][n] );
   eh_watch_dbl( sed_cube_y_slope(p,0,i) );
   eh_watch_dbl( data->x_0 );
   eh_watch_dbl( data->x_b );
   eh_watch_dbl( data->h_b );
   eh_watch_int( data->ind_len );
   eh_watch_int( data->ind[0] );
   eh_watch_int( data->ind[data->ind_len-1] );
   exit(0);
}
}
      rem_cell = eh_new( Sed_cell , sed_cube_n_y(p) );
      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
         rem_cell[i] = sed_cell_new_env( );
   }

   // Determine which column the sediment will be removed from and added to.
   // Remove the sediment and add it to the appropriate index of the temporary
   // array.
   {
      gssize i, add_index, remove_index;
      double du_tot;
      gssize top_i = sed_cube_n_y(p)-1;
      Sed_cell add_cell  = sed_cell_new_env();
      Sed_cell fill_cell;
      double*     dh_max = eh_new( double , sed_cube_n_y(p) );
//      gssize* bruun_ind = eh_new( gssize , sed_cube_n_y(p)+1 );
//      gssize ind_len = get_zone_indices( p , 0 , data->h_b , 0 , S_WATER_DEPTH_FUNC , bruun_ind );
      double total = 0;
      gssize    ind_len = data->ind_len;

      {
         double* f = eh_new0( double , n_grains );

         f[0] = 1.;

         fill_cell = sed_cell_new_sized( n_grains , G_MINDOUBLE , f );

         sed_cell_destroy( fill_cell );
         fill_cell = NULL;

         eh_free( f );
      }

      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
{
         dh_max[i] = erosion_limit[i]-sed_cube_water_depth(p,0,i);
if ( dh_max[i]<0 )
   dh_max[i] = 0;
         total += dh_max[i];
}
      for ( i=0 ; i<top_i ; i++ )
      {
         du_tot = eh_dbl_array_sum( du[i] , n_grains );
         eh_dbl_array_fabs( du[i] , n_grains );

         remove_index = (du_tot>0)?(i):(i+1);
         add_index    = (du_tot>0)?(i+1):(i);

         if ( fabs(du_tot) > 0 )
         {
            double m_0 = sed_column_mass( sed_cube_col(p,remove_index) );
            double m_1, dm;
if ( fabs(du_tot)>dh_max[i] )
{
   du_tot = dh_max[i];
}
//eh_watch_dbl( sed_cell_mass( add_cell ) );
//eh_watch_dbl( sed_column_thickness( sed_cube_col(p,remove_index) ) );
            sed_column_separate_top_amounts_fill(
                                             sed_cube_col(p,remove_index) ,
                                             fabs(du_tot) ,
                                             du[i] ,
                                             fill_cell ,
                                             add_cell );
m_1 = sed_column_mass( sed_cube_col(p,remove_index) );
dm  = sed_cell_mass( add_cell );

if ( fabs(m_1+dm-m_0) > 1e-6 )
{
   eh_watch_int( i );
   eh_watch_int( remove_index );
   eh_watch_dbl( sed_column_thickness( sed_cube_col(p,remove_index) ) );
   eh_watch_dbl( du_tot );
   eh_watch_dbl( m_0 );
   eh_watch_dbl( m_1 );
   eh_watch_dbl( dm );
   exit( 0 );
}
            sed_cell_add( rem_cell[add_index] , add_cell );
            sed_cell_add( added , fill_cell );
         }
      }
   
      {
         if ( ind_len > 0 )
         {
            //gssize i_0 = bruun_ind[0];
            gssize i_0 = 0;

            // Add whatever we removed from the first cell back.
            sed_cell_copy( rem_cell[i_0] , rem_cell[i_0+1] );
//            sed_cell_add ( lost          , rem_cell[i_0]   );

// NOTE: This sediment should be removed from the next river event.
            sed_cell_add ( added          , rem_cell[i_0]   );
         }
      }

//      eh_free( bruun_ind );

      du_tot = eh_dbl_array_sum( du[0] , n_grains );
      eh_dbl_array_fabs( du[0] , n_grains );
      if ( du_tot < 0 )
      {
         du_tot = -dh_max[0];
         sed_column_separate_top_amounts_fill( sed_cube_col(p,0) ,
                                          -du_tot ,
                                          du[0] ,
                                          fill_cell ,
                                          add_cell );
         sed_cell_add( added , fill_cell );
      }

      du_tot = eh_dbl_array_sum( du[top_i] , n_grains );
      eh_dbl_array_fabs( du[top_i] , n_grains );
      if ( du_tot > 0 )
      {
         if ( du_tot>dh_max[top_i] )
            du_tot = dh_max[top_i];

         sed_column_separate_top_amounts_fill( sed_cube_col(p,top_i) ,
                                          du_tot ,
                                          du[top_i],
                                          fill_cell ,
                                          add_cell );
         sed_cell_add( added , fill_cell );
         sed_cell_add( lost  , add_cell  );
      }
      else 
         sed_cell_copy( rem_cell[top_i] , rem_cell[top_i-1] );

      sed_cell_destroy( fill_cell );
      sed_cell_destroy( add_cell  );
      eh_free( dh_max );
   }

   // Set the facies type, and age of the sediment.  Add the removed sediment
   // to the profile.
   {
      gssize i;
//      Sed_cell clay_cell = sed_cell_new_env();
//      double* just_clay = eh_new0( double , n_grains );
//      double z_0 = 23;

//      just_clay[n_grains-1] = 1;
//      just_clay[n_grains-2] = 1;

      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      {
         sed_cell_set_facies( rem_cell[i] , S_FACIES_WAVE );
         sed_cell_set_age( rem_cell[i] , sed_cube_age_in_years(p) );

         sed_column_add_cell( sed_cube_col(p,i) , rem_cell[i] );
      }

//      sed_cell_add( in_suspension , clay_cell );

//      sed_cell_destroy( clay_cell );
   }

   // Free memory.
   {
      gssize i;
      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
         sed_cell_destroy( rem_cell[i] );
      eh_free( rem_cell );
   }

   return lost;
}

/** Add any sediment left in suspension

\param col  A NULL-terminated list of Sed_column s
\param cell A Sed_cell
*/
void add_suspended_sediment( Sed_column* col , Sed_cell cell )
{
   if ( col )
   {
      gssize i;

      eh_require( cell );
      for ( i=0 ; col[i] ; i++ )
         sed_column_add_cell( col[i] , cell );
   }
   return;
}

/** Get the depth of the Bruun profile at some position

The Bruun profile is described by the equation,
\f[
   \eta = a \left( y-y_0 \right)^m
\f]
where \f$ \eta \f$, is water depth, \f$ y \f$ is horizontal position, \f$ y_0 \f$
is the position of the shore, and \f$ a \f$ and \f$ m \f$ are constants that
define the shape of the profile.

\param y Horizontal position
\param y_0 Horizontal position of the shore
\param bruun_a Coefficient in Bruun profile
\param bruun_m Exponent in Bruun profile

\return The depth of the Bruun profile at position, \a y
*/
double get_bruun_depth( double y , double y_0 , double bruun_a , double bruun_m)
{
   return bruun_a*pow( fabs(y-y_0) , bruun_m );
}

/** Get the coefficent for a Bruun profile

\param p A Sed_cube that defines the inner shelf
\param bruun_m The exponent for the Bruun profile

\return The Bruun coefficient, \f$ a \f$
*/
double get_bruun_a( Sed_cube p , double bruun_m )
{
   double bruun_a;

   eh_require( p );

   if ( p )
   {
      double h_b = sed_cube_water_depth( p , 0 , sed_cube_n_y(p)-1 );
      double y_0 = sed_cube_col_y( p , 0        ) - sed_cube_y_res(p);
      double y_b = sed_cube_col_y( p , sed_cube_n_y(p)-1 );

      bruun_a = h_b / pow(y_b-y_0,bruun_m);
   }
   else
      bruun_a = eh_nan();

   return bruun_a;
}

double get_bruun_y_0( Sed_cube p )
{
   return sed_cube_col_y( p , 0 ) - sed_cube_y_res(p);
}

double get_bruun_y_b( Sed_cube p )
{
   return sed_cube_col_y( p , sed_cube_n_y(p)-1 );
}

double* get_bruun_profile( double* y      , gssize len     ,
                           double bruun_a , double bruun_m ,
                           double y_0     , double y_b )
{
   double* h = NULL;

   eh_require( y     );
   eh_require( len>0 );
   eh_require( y_b>y_0 );

   eh_debug( "Is there an inner shelf?" );
   if ( y_b>y_0 )
   {
      h = eh_new( double , len );

      eh_debug( "Calculate the depths of the Bruun profile" );
      {
         gssize i;
         for ( i=0 ; i<len ; i++ )
         {
            if ( y[i] < y_0 || y[i]>y_b )
               h[i] = eh_nan();
            else
               h[i] = get_bruun_depth( y[i] , y_0 , bruun_a , bruun_m );
         }
      }

      eh_debug( "Check if all points are outside of inner shelf" );
      {
         gssize i;
         for ( i=0 ; i<len && eh_isnan(h[i]) ; i++ );
         if ( i==len )
         {
            eh_free( h );
            h = NULL;
         }
      }
   }

   eh_debug( "Done." );

   return h;
}

/** Fill the columns of a Sed_cube to given depths

The columns of \a p are filled to the water depths provided in the array \a h.
\a fill_cell provides the sediment that will be added to the columns.  The added
sediment is given the current age of \a p, and the facies is set to S_FACIES_ALONG_SHORE.
If \p h is NULL, nothing is done.  If any depths are NaN, the column is skipped
over.

\note The size of \a fill_cell is adjusted to reflect the amount of sediment that was 
added to the Sed_cube.

\param[in,out]   p            A Sed_cube
\param[in]       h            Water depths to fill columns to
\param[in,out]   fill_cell    A Sed_cell that contains the sediment to fill with

*/
void fill_to_bruun( Sed_cube p ,
                    double* h   ,
                    Sed_cell fill_cell )
{
   double total = 0;

   eh_require( p );
   eh_require( fill_cell );

   if ( h )
   {
      gssize i;
      double dh;

      sed_cell_set_age   ( fill_cell , sed_cube_age_in_years(p) );
      sed_cell_set_facies( fill_cell , S_FACIES_ALONG_SHORE         );

      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      {
         if ( !eh_isnan(h[i]) )
         {
            dh = sed_cube_water_depth( p , 0 , i ) - h[i];

            if ( dh > 0 )
            {
               sed_cell_resize( fill_cell , dh );

               sed_column_add_cell( sed_cube_col(p,i) , fill_cell );

               total += dh;
            }
         }
      }
   }

   sed_cell_resize( fill_cell , total );

   return;
}

void fill_to_bruun_profile( Sed_cube p             ,
                            Sed_wave deep_wave     ,
                            double bruun_m         ,
                            Sed_cell fill_cell     ,
                            Sed_cell added_fill_cell )
{
   eh_require( p );

   sed_cell_set_facies( fill_cell , S_FACIES_BEDLOAD             );
   sed_cell_set_age   ( fill_cell , sed_cube_age_in_years(p) );

   if ( sed_cube_n_y(p)>2 )
   {
      double bruun_a, h_b;
      double y_0, y_b;

      h_b = sed_cube_water_depth( p , 0 , sed_cube_n_y(p)-1 );
      y_0 = sed_cube_col_y( p , 0        );
      y_b = sed_cube_col_y( p , sed_cube_n_y(p)-1 );

      //---
      // Define the coefficient in the Bruun equation so that the curve
      // joins the current profile.
      //---
      bruun_a = h_b / pow(y_b-y_0,bruun_m);

      //---
      // If a column has been eroded to below the Bruun profile, fill it up to
      // the profile using the fill sediment.
      //---
      {
         gssize i;
         double h, dh, h_total = 0.;

         for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
         {
            h = bruun_a * pow( sed_cube_col_y(p,i) - y_0 , bruun_m );

            dh = sed_cube_water_depth( p , 0 , i ) - h;

            if ( dh > 0 )
            {
               sed_cell_resize( fill_cell , dh );

               sed_column_add_cell( sed_cube_col(p,i) , fill_cell );

               h_total += dh;
            }
         }

         sed_cell_resize( fill_cell , h_total );
      }

      if ( added_fill_cell )
         sed_cell_add( added_fill_cell , fill_cell );
   }

   return;
}

/** Get the closure depth of a profile with given ocean wave conditions.

\param p    A Sed_cube
\param wave A Sed_wave

\return The closure depth in meters.
*/
double get_closure_depth( Sed_cube p , Sed_wave wave )
{
//   return 25;
   return 3.*sed_wave_break_depth( wave );
//   return sed_wave_length( wave );
//   return 15;
}

double get_h_c( Sed_wave w )
{
   double g = sed_gravity();
   double h = 1.25*sed_wave_height( w );
   double t = sed_wave_period( w );
   return 2.28*h - 6.85*( h*h / ( g * t*t ) );
}

/** Get the depth of erosion for some near-bed velocity

\param u Near-bed current velocity in m/s

\return The erosion depth (in meters) for one day.
*/
double get_erosion_depth( double u )
{
   double sua = 400;  // In Pa
   double sub = 0;    // In Pa/m
   double C_d = .004;

   eh_require( u>=0 );

   return (C_d*sed_rho_sea_water()*u*u-sub)/sua;
}

double* get_max_erosion_profile( Sed_cube p , Sed_wave w )
{
   double* z_max = NULL;

   eh_require( p );
   eh_require( sed_cube_n_y(p)>0 );
   eh_require( w );

   z_max = eh_new( double , sed_cube_n_y(p) );

   if ( p && w )
   {
      gssize i;
      double z;
      double u;
      double h_b = sed_wave_break_depth( w );
//      double h_b = get_h_c( w );

      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      {
         z = sed_cube_water_depth( p , 0 , i );
         if ( z>.1 )
         {
            u = get_near_bed_velocity( z , w , h_b );
            z_max[i] = get_erosion_depth( u ) + z;
         }
         else
            z_max[i] = z;
      }
   }
      
   return z_max;
}

/** Calculate the near bed velocity of a given wave

If the water depth is less than the breaker depth, the orbital velocity is taken
to fall off linearly from it breaker-depth value.

\param water_depth   Water depth in meters
\param w             A Sed_wave
\param breaker_depth The depth (in meters) of breaking waves

\return The near bed orbital velocity in meters per second.
*/
double get_near_bed_velocity( double water_depth , Sed_wave w , double breaker_depth )
{
   double u;

   // Assume that the near-bed velocity falls of linearly within the
   // breaker zone.
   if ( water_depth < breaker_depth )
      u = near_bed_velocity_func_mean( breaker_depth , w , breaker_depth )
        * pow( water_depth / breaker_depth , 1. ) ;
   else
      u = near_bed_velocity_func_mean( water_depth , w , breaker_depth );

   return u;
}

double near_bed_velocity_func_mean( double water_depth , Sed_wave w , double breaker_depth )
{
//   double alpha = (-atan(50*( w->h/water_depth-.5 ) ) + G_PI_2 )/G_PI;
   double alpha = (-atan(5*( sed_wave_height(w)/water_depth-.5 ) ) + G_PI_2 )/G_PI;

   alpha = 0.;

   return   (1.-alpha) * near_bed_velocity_func      ( water_depth , w , breaker_depth )
          +  alpha     * near_bed_velocity_func_komar( water_depth , w , breaker_depth );
}

/** The near-bed orbital velocity for shoaling waves

\f[
   u = { \gamma_b \over 2 } \sqrt{ g h_b }\left( {h\over h_b} \right)^{-.75}
\f]

where \f$ h \f$ is water depth, and \f$ \gamma_b \f$ is a breaker index that relates breaker
depth (\f$ h_b \f$) to the height of breaking waves.

\param water_depth   Water depth in meters
\param w             A Sed_wave
\param breaker_depth Breaking wave depth in meters

return The near-bed orbital velocity in meters per second
*/
double near_bed_velocity_func( double water_depth , Sed_wave w , double breaker_depth )
{
   double gamma_b = .6; // breaker index varies from .4 - .8
//   double breaker_depth = sed_wave_break_depth( w );
   return   .5*gamma_b
          * sqrt(sed_gravity()*breaker_depth)
          * pow(water_depth/breaker_depth,-.75);
}

/** Calculate the near bed orbital velocity using Komar's formula

\f[
   u = { \omega H \over 2 \sinh \left( \kappa h \right) }
\f]
where \f$ u \f$ is orbital velocity, \f$ \omega \f$ is wave frequency,
\f$ \kappa \f$ is wave number, and \f$ h \f$ is water depth.

\param water_depth   Water depth in meters
\param w             A Sed_wave
\param breaker_depth Breaking wave depth in meters

return The near-bed orbital velocity in meters per second
*/
double near_bed_velocity_func_komar( double water_depth ,
                                     Sed_wave w         ,
                                     double breaker_depth )
{
   return sed_wave_frequency(w)*sed_wave_height(w) / ( 2.*sinh( sed_wave_number(w)*water_depth ) );
}

/** Calculate the near bed orbital velocity for Stokes' waves

\f[
   u = {c\over 2} \left( {\kappa H \over 2 \sinh \left( \kappa h \right) }\right)^2
\f]
where \f$ u \f$ is orbital velocity, \f$ c \f$ is phase velocity,
\f$ \kappa \f$ is wave number, and \f$ h \f$ is water depth.

\param water_depth Water depth in meters
\param w           A Sed_wave
\param breaker_depth Breaking wave depth in meters

return The near-bed orbital velocity in meters per second;
*/
double near_bed_velocity_func_stokes( double water_depth ,
                                      Sed_wave w         ,
                                      double breaker_depth )
{
   return .5*pow( sed_wave_number(w)*sed_wave_height(w)*.5/sinh(sed_wave_number(w)*water_depth) , 2. )
          * sed_wave_phase_velocity( w );
}

/** Get the grain-size threshold for movement.

\f[
   D = \left( {\rho \over .21 g \left( \rho_s - \rho \right) } \right)^2 { u^4 \over d_0 }
\f]
where \f$ D \f$ is the grain size threshold, \f$ \rho \f$ is the density of sea water,
\f$ \rho_s \f$ is the desity of quartz, \f$ g \f$ is acceleration due to gravity, 
\f$ u \f$ is orbital velocity, and \f$ d_0 \f$ is orbital diameter.  Orbital diameter is
calculated by,
\f[
   d_0 =  u { T \over \pi }
\f]
where \f$ T \f$ is wave period.

\param orbital_velocity Wave orbital velocity in meters per second
\param wave_period      Wave period in seconds

\return The maximum grain diameter (in meters) that can be mobilized by the orbital velocity
*/
double get_grain_size_threshold( double orbital_velocity , double wave_period )
{
   double rho   = sed_rho_sea_water();
   double rho_s = sed_rho_quartz();
   double g     = sed_gravity();
   double c_sq  = pow( rho/(.21*g*(rho_s-rho) ),2.);
   double orbital_diameter = orbital_velocity*wave_period/G_PI;

   return c_sq * pow(orbital_velocity,4.) / orbital_diameter;
}

/* @} */

