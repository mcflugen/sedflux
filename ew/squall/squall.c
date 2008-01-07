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

//#define DISABLE_WATCH_POINTS
//#define DISABLE_CHECKS

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "squall.h"

typedef struct
{
   gsize count;
   double lower_edge;
   double upper_edge;
   gpointer data;
}
Eh_histogram;

typedef struct
{
   double f;
   gssize ind;
}
Hist_data;

Eh_histogram **eh_create_histogram( double dx , gssize n );
void eh_destroy_histogram( Eh_histogram **h );
Eh_histogram **make_depth_histogram( double *x , int n_x , double dz , gssize n_z );

double get_weibull_deposition_rate( double x , double alpha , double beta );
double get_threshold_depth( double wave_length , double wave_height ,
                            double wave_period , double grain_size );

/** Calculate erosion/deposition of a Sed_cube due to waves.

Run the Storms 2003 wave model.

@param p A pointer to a Sed_cube.
@param time_step_in_years The length of time (in years) to run the model for.

@return TRUE is there are no problems. FALSE otherwise.
*/
gboolean squall( Sed_cube p , double time_step_in_years )
{
   int i, j, k, n, n_grains = sed_sediment_env_n_types();
   int *zone;
   int i_w, i_s, i_c, i_l, i_m, i_0;
   double h_w, h_c;
   double c_v = SQUALL_DEFAULT_C_V;
   double *e       = eh_new0( double , sed_cube_n_y(p) );
   double *g       = eh_new0( double , sed_cube_n_y(p) );
   double *f       = eh_new0( double , n_grains );
   double *is_moveable = eh_new0( double , n_grains );
   double depth, h, dep, dep_fraction;
   double distance_to_h_w, total;
   double *fraction_total;
   double *threshold_depth = eh_new( double , n_grains );
   double **dep_thickness  = eh_new_2( double , sed_cube_n_y(p) , n_grains );
   double *profile_depth;
   double *total_erosion   = eh_new( double , n_grains );
   double dz;
   int n_z_bins;
   gssize dep_column;
   Eh_histogram **hist;
   gboolean back_barrier_is_on = FALSE;
   Sed_cell add_cell;
   Sed_cell top_cell           = sed_cell_new( n_grains );
   Sed_cell lag_cell           = sed_cell_new( n_grains );
   Sed_cell removed_cell       = sed_cell_new( n_grains );
   Sed_cell erosion_cell       = sed_cell_new( n_grains );
   Sed_cell* dep_cell = sed_cell_list_new( sed_cube_n_y(p) , n_grains );
   Sed_cell bb_cell, sf_cell;
   double mass_in, mass_out;

   h_c = sed_cube_wave_height( p );
   h_w = get_deep_water_wave_base( h_c );

//eh_make_note( h_w=50 );

h_c = 5;

   //---
   // calculate the locations of the various domains for erosion and
   // deposition.  there will be a maximum of 6 zones:
   //  (1) continental
   //  (2) lagoon
   //  (3) barrier island
   //  (4) shoreface
   //  (5) breaking
   //  (6) deep water
   // however, zones (2) and (3) will only be present if a barrier island
   // has formed.  this is not always the case.  thus, there may only be
   // zones (1), (4), (5), and (6).
   //---
   zone = get_zone_boundaries( p , h_w , h_c );

   i_m = zone[0];
   i_l = zone[1];
   i_c = zone[2];
   i_s = zone[3];
   i_w = zone[4];

//   eh_make_note( h_w = h_shelf );
//   eh_make_note( i_w = zone[5] );

   distance_to_h_w = ( i_w - i_s )*sed_cube_y_res( p );

/*
   h_c = -sed_get_depth_from_profile( p , i_c );
   h_w =  sed_get_depth_from_profile( p , i_w );
*/
/*
   for ( i=i_c ; i<i_w ; i++ )
   {
      if ( sed_get_depth_from_profile( p , i )>h_w )
      {
         eh_watch_int( i );
         eh_watch_int( i_w );
         eh_watch_dbl( h_w );
         eh_watch_int( sed_get_profile_river_mouth( p ) );
         eh_watch_dbl( sed_get_depth_from_profile( p , i ) );
      }
      if ( -sed_get_depth_from_profile( p , i )>h_c )
      {
         eh_watch_int( i );
         eh_watch_int( i_c );
         eh_watch_dbl( h_c );
         eh_watch_int( sed_get_profile_river_mouth( p ) );
         eh_watch_dbl( sed_get_depth_from_profile( p , i ) );
      }
   }
*/
   eh_free( zone );

   //---
   // phase 1:
   //
   // calculate erosion rates.
   //---
   if ( back_barrier_is_on )
      i = i_c;
   else
      i = i_s;
   for ( ; i<i_w ; i++ )
   {
//      e[i] = get_erosion_rate_from_profile( p , i , i_c , i_w )
//      e[i] = get_erosion_rate_from_profile( p , i , h_c , h_w )
//           * time_step_in_years;
      h    = sed_cube_water_depth( p , 0 , i );
      g[i] = pow( (h-h_w) / (-h_c-h_w) , 3. );
/*
eh_require( g[i]>=0 );
eh_require( g[i]<=1 );
if ( g[i]<0 || g[i]>1 )
{
   eh_watch_dbl(  h   );
   eh_watch_dbl(  h_w );
   eh_watch_dbl( -h_c );
}
*/

      e[i] = (h_w/10.)*SQUALL_DEFAULT_C_E*g[i]*time_step_in_years;
//      e[i] = SQUALL_DEFAULT_C_E*g[i]*time_step_in_years;
      if ( e[i]<0 )
         e[i] = 0;
   }
/*
   if ( back_barrier_is_on )
      i = i_c;
   else
      i = i_s;
   for ( ; i<i_shelf ; i++ )
   {
      e_shelf[i] = ( pow((h-h_shelf)/(-h_c-h_shelf),3.) - g[i] )
                 * SQUALL_DEFAULT_C_E*time_step_in_years;
      e_shelf[i] = 0.;
   }
eh_message( "e_shelf set to zero." );
*/

   //---
   // remove the eroded sediment and mix it together.
   //---
   for ( i=i_c ; i<i_w ; i++ )
   {
      get_moveable_grains( sed_cube_water_depth( p , 0 , i ) ,
                           sed_cube_wave_height( p )        ,
                           sed_cube_wave_period( p )        ,
                           NULL                                 ,
                           is_moveable );

      sed_column_extract_top( sed_cube_col(p,i) , e[i] , lag_cell );

      sed_cell_separate_fraction( lag_cell    ,
                                  is_moveable ,
                                  removed_cell );

      sed_column_add_cell( sed_cube_col(p,i) , lag_cell );
      sed_cell_add( erosion_cell , removed_cell );
   }
   sed_cell_set_facies( erosion_cell , S_FACIES_WAVE );

   //---
   // deposit sediment in the backbarrier (phase 2) and shoreface (phase 3).
   // the total eroded sediment is divided between the backbarrier and the
   // shoreface according to c_v.
   //---
   if ( i_m<=0 || !back_barrier_is_on )
      c_v = 0;
   bb_cell = sed_cell_dup( erosion_cell );
   sf_cell = sed_cell_dup( erosion_cell );
   sed_cell_resize( bb_cell ,      c_v*sed_cell_size(erosion_cell) );
   sed_cell_resize( sf_cell , (1.-c_v)*sed_cell_size(erosion_cell) );

   //---
   // phase 2:
   //
   // deposit sediment in the backbarrier.  begin on the barrier island, x_c
   // and more landward to x_m.
   //----

   if ( i_m > 0 && back_barrier_is_on )
   {
      double* grain_size = sed_sediment_property( NULL , &sed_type_grain_size_in_meters );
//      for ( i=i_c ; i>i_m && sed_get_cell_thickness(bb_cell)>1e-5 ; i-- )
      for ( i=i_c-1 ; i>i_m && sed_cell_size(bb_cell)>1e-5 ; i-- )
      {
         depth = sed_cube_water_depth( p , 0 , i );
   
         for ( n=0 ; n<n_grains ; n++ )
         {
            memset(f,0,sizeof(double)*n_grains);
            f[n] = 1;
   
            h = get_travel_dist( grain_size[n] ,
                                 depth         ,
                                 sed_cube_y_res(p) );
            dep = sed_cell_size( bb_cell )
                * sed_cell_fraction( bb_cell , n )
                * sed_cube_y_res( p )
                / h;

            sed_cell_move( bb_cell , dep_cell[i] , f , dep );
         }
      }
      eh_free( grain_size );
   }

   //---
   // phase 3:
   // 
   // deposit sediment on the shoreface.  any sediment that was able to be
   // deposited in the backbarrier during phase 2 is added to the 
   // shoreface sediment.
   //---
   sed_cell_add( sf_cell , bb_cell );

//   for ( i=i_c+1 ; i<p->size && sed_get_cell_thickness(sf_cell)>1e-5 ; i++ )
//   for ( i=i_c ; i<p->size && sed_get_cell_thickness(sf_cell)>1e-5 ; i++ )
   if ( back_barrier_is_on )
      i_0 = i_c;
   else
      i_0 = i_s;

   for ( mass_in=0,n=0 ; n<n_grains ; n++ )
   {
      total_erosion[n] = sed_cell_size( sf_cell )
                       * sed_cell_fraction( sf_cell , n );
      mass_in += total_erosion[n];
   }
   mass_out = 0;

{
   double* grain_size = sed_sediment_property( NULL , &sed_type_grain_size_in_meters );
   for ( k=0 ; k<1 ; k++ )
   {

      //---
      // divide the profile up into depth bins.  the histogram includes an 
      // index to the column at that depth, and the fraction of the bin that
      // the column occupies.
      //---
      dz = sed_cube_z_res(p);
      profile_depth = eh_new( double , sed_cube_n_y(p) );
      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
         profile_depth[i] = sed_cube_water_depth(p,0,i);
      n_z_bins = (int)(profile_depth[sed_cube_n_y(p)-1]/dz);
      hist = make_depth_histogram( profile_depth , sed_cube_n_y(p) , dz , n_z_bins );
      eh_free( profile_depth );
   
      for ( n=0 ; n<n_grains ; n++ )
      {
         threshold_depth[n] = get_threshold_depth(
                                 sed_cube_wave_length(p) ,
                                 sed_cube_wave_height(p) ,
                                 sed_cube_wave_period(p) ,
                                 grain_size[n] );
      }
   
      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
         for ( n=0 ; n<n_grains ; n++ )
            dep_thickness[i][n] = 0;
   
      fraction_total = eh_new( double , n_grains );
      for ( n=0 ; n<n_grains ; n++ )
         fraction_total[n] = 0.;
   
      for ( i=0 ; i<n_z_bins ; i++ )
         for ( n=0 ; n<n_grains ; n++ )
            if ( hist[i]->count > 0 )
               fraction_total[n] += get_weibull_deposition_rate(
                                       i*dz ,
                                       2 ,
                                       threshold_depth[n] )*dz;
   
      for ( i=0 ; i<n_z_bins && sed_cell_size(sf_cell)>1e-5 ; i++ )
      {
         depth = i*dz;
   
         for ( n=0 ; n<n_grains ; n++ )
         {
            memset(f,0,sizeof(double)*n_grains);
            f[n] = 1;
   
            if ( depth>0 && hist[i]->count>0 )
            {
               dep_fraction = -exp( -pow((depth+dz)/threshold_depth[n],2) )
                            +  exp( -pow(depth/threshold_depth[n],2) );
   /*
               dep_fraction = get_weibull_deposition_rate(
                                 depth ,
                                 (n==0)?1:2 ,
                                 threshold_depth[n] )*dz;
   */
            }
            else
               dep_fraction = 0;
   
   //         if ( dep > depth+h_c )
   //            dep = depth+h_c;
   
            for ( total=0,j=0 ; j<hist[i]->count ; j++ )
               total += ((Hist_data*)(hist[i]->data))[j].f;
   
            for ( j=0 ; j<hist[i]->count ; j++ )
            {
   //eh_require( ((Hist_data*)(hist[i]->data))[j].f<=1.0001 );
   //eh_require( ((Hist_data*)(hist[i]->data))[j].f>-0.0001 );
               dep_column  = ((Hist_data*)(hist[i]->data))[j].ind;
               dep        = dep_fraction
                          / fraction_total[n]
                          * total_erosion[n]
                          * ((Hist_data*)(hist[i]->data))[j].f
                          / total
                          / 1.;
   
   //            sed_move_sediment_to_cell( sf_cell , dep_cell[dep_column] , f ,
   //                                       dep     , n_grains );
   
               dep_thickness[dep_column][n] += dep;
               mass_out += dep;
            }
         }
      }
   
      eh_destroy_histogram( hist );
   
      //---
      // add the deposited sediment back to the profile.
      //---
      add_cell = sed_cell_new( n_grains );
      sed_cell_set_age( add_cell , sed_cube_age_in_years(p) );
      sed_cell_set_facies( add_cell , S_FACIES_WAVE );
      mass_out = 0;
   //   for ( i=0 ; i<p->size ; i++ )
      for ( i=1 ; i<sed_cube_n_y(p) ; i++ )
      {
   if ( sed_cube_water_depth( p , 0 , i ) > 0 )
   {
         sed_cell_resize( add_cell , 0. );
//         sed_add_vector_to_cell( add_cell , dep_thickness[i] , n_grains );
         sed_cell_add_amount( add_cell , dep_thickness[i] );
         if ( sed_cell_size(add_cell) > sed_cube_water_depth(p,0,i) )
            sed_cell_resize( add_cell , sed_cube_water_depth(p,0,i) );
   if (   sed_cube_water_depth(p,0,i)
        - sed_cell_size(add_cell)
        - sed_cube_water_depth(p,0,i-1) < 0. )
      sed_cell_resize( add_cell ,
                         sed_cube_water_depth(p,0,i)
                       - sed_cube_water_depth(p,0,i-1)
                       - .0 );
         if ( sed_cell_size( add_cell ) > 0 )
         {
            mass_out += sed_cell_size( add_cell );
            sed_column_add_cell( sed_cube_col(p,i) , add_cell );
         }
   }
      }
   
      sed_cell_destroy( add_cell );
      eh_free( fraction_total );
   
   }

   eh_free( grain_size );
}

   eh_free_2( dep_thickness );

//eh_watch_dbl( mass_in );
//eh_watch_dbl( mass_out );

   sed_cell_destroy( sf_cell            );
   sed_cell_destroy( bb_cell            );
   sed_cell_destroy( top_cell           );
   sed_cell_destroy( lag_cell           );
   sed_cell_destroy( removed_cell       );
   sed_cell_destroy( erosion_cell       );

   sed_cell_list_destroy( dep_cell );

   eh_free( is_moveable );
   eh_free( total_erosion );
   eh_free( e       );
   eh_free( g       );
   eh_free( f );
   eh_free( threshold_depth );

   return TRUE;
}

Eh_histogram **eh_create_histogram( double dx , gssize n )
{
   gsize i;
   Eh_histogram **h;

   h = eh_new( Eh_histogram* , n+1 );
   for ( i=0 ; i<n ; i++ )
   {
      h[i] = eh_new( Eh_histogram , 1 );
      h[i]->count = 0;
      h[i]->lower_edge = i*dx;
      h[i]->upper_edge = (i+1)*dx;
      h[i]->data = NULL;
   }
   h[n] = NULL;

   return h;
}

void eh_destroy_histogram( Eh_histogram **h )
{
   gssize i;

   for ( i=0 ; h[i] ; i++ )
   {
      eh_free( h[i]->data );
      eh_free( h[i] );
   }
   eh_free( h );
}

void add_to_hist( Eh_histogram **hist , gssize bin , double f , gssize ind );

Eh_histogram **make_depth_histogram( double *x , int n_x , double dz , gssize n_z )
{
   double dx = 50;
   gssize i, j;
   gssize lower_z_bin, upper_z_bin;
   double lower_edge, upper_edge;
   double lower_x, upper_x;
   double f;
   double slope;
   Eh_histogram **hist;

   eh_require( n_z>0 );

   hist = eh_create_histogram( dz , n_z );

   for ( i=1 ; i<n_x-1 ; i++ )
   {

      for ( j=0 ; j<1 ; j++ )
      {

         lower_x = x[i];
         upper_x = x[i+1];

         if ( j==0 )
         {
            lower_x = .5*(x[i] + x[i-1]);
            upper_x = x[i];
         }
         else
         {
            lower_x = x[i];
            upper_x = .5*(x[i+1] + x[i]);
         }

         lower_x = .5*(x[i] + x[i-1]);
         upper_x = .5*(x[i+1] + x[i]);

         if ( upper_x<lower_x )
            swap_dbl( upper_x , lower_x );

         lower_z_bin = floor(lower_x/dz);
         upper_z_bin = floor(upper_x/dz);

         if ( fabs(upper_x-upper_z_bin*dz) < 1e-5 )
            upper_z_bin--;
         if ( upper_z_bin<lower_z_bin )
            swap_int( upper_z_bin , lower_z_bin );

         if ( upper_z_bin<n_z && lower_z_bin>=0 )
         {

            lower_edge = lower_z_bin*dz;
            upper_edge = (upper_z_bin+1)*dz;

            if ( lower_z_bin == upper_z_bin )
            {
               f = sqrt( dx*dx + pow(upper_x-lower_x,2.) );
               f = dx;
               f = 1;
               f = (upper_x-lower_x)/dz;
               add_to_hist( hist , lower_z_bin , fabs(f) , i );
            }
            else
            {
               slope = atan(dx/(upper_x-lower_x));

               f = (lower_edge+dz-lower_x)/sin(slope);
               f = (lower_edge+dz-lower_x)/tan(slope);
               f = 1;
               f = ((lower_edge+dz)-lower_x)/dz;

               add_to_hist( hist , lower_z_bin , fabs(f) , i );
               for ( j=lower_z_bin+1 ; j<upper_z_bin ; j++ )
               {
                  f = dz/sin(slope);
                  f = dz/tan(slope);
                  f = 1;
                  add_to_hist( hist , j , f , i );
               }
               f = (upper_x-(upper_edge-dz))/sin(slope);
               f = (upper_x-(upper_edge-dz))/tan(slope);
               f = 1;
               f = (upper_x - (upper_edge-dz))/dz;
               add_to_hist( hist , upper_z_bin , fabs(f) , i );
//               add_to_hist( hist , upper_z_bin , f/2. , i+1 );
            }
         }
      }

   }

   return hist;
}

void add_to_hist( Eh_histogram **hist , gssize bin , double f , gssize ind )
{
   Eh_histogram *this_bin = hist[bin];
   
if ( this_bin->count > 1000 )
{
   eh_watch_int( bin );
   eh_watch_int( this_bin->count );
   eh_watch_dbl( this_bin->lower_edge );
   eh_watch_dbl( this_bin->upper_edge );
}
   this_bin->count = this_bin->count + 1;
if ( this_bin->count > 1000 )
{
   eh_watch_int( bin );
   eh_watch_int( this_bin->count );
   eh_watch_dbl( this_bin->lower_edge );
   eh_watch_dbl( this_bin->upper_edge );
}
   this_bin->data  = g_renew( Hist_data , this_bin->data , this_bin->count );
   ((Hist_data*)(this_bin->data))[this_bin->count-1].f   = f;
   ((Hist_data*)(this_bin->data))[this_bin->count-1].ind = ind;
}


double get_weibull_deposition_rate( double x , double alpha , double beta )
{
   if ( fabs(alpha-1.) < 1e-5 )
      return exp( -x/beta )/beta;
   else
      return alpha*pow(x,alpha-1)/pow(beta,alpha)*exp(-pow(x/beta,alpha));
}

/** Calculate the erosion rate from a Sed_cube.

@param p      A pointer to a Sed_cube.
@param i      Index to a column of a Sed_cube.
\param h_c    Maximum coastal elevation that is acted on by waves
\param h_w    Wave height

@return       The erosion rate (m/year)
*/
double get_erosion_rate_from_profile( Sed_cube p , int i ,
                                      double h_c  , double h_w )
{
//   double alpha_sf    = get_shoreface_slope( p , i_s+1 , i_w );
   double alpha_sf    = 1;
   double h           = sed_cube_water_depth( p , 0 , i   );
   double h_wave      = sed_cube_wave_height( p );
   double h_wave_fair = h_wave;

   return get_erosion_rate( h , h_w , h_c , alpha_sf , h_wave , h_wave_fair );
}

/** Calculate the slope of the shoreface.

@param p A pointer to a Sed_cube.
@param i_c Index to the maximum landward extent of the waves.
@param i_w Index to the location of the wave base.

@return The slope of the shoreface (grads).
*/
double get_shoreface_slope( Sed_cube p , int i_c , int i_w )
{
   double rise = sed_cube_water_depth( p , 0 , i_w )
               - sed_cube_water_depth( p , 0 , i_c );
   double run  = sed_cube_col_y( p,i_w ) - sed_cube_col_y( p,i_c );

   return rise/run;
}

/** Calculate the rate of erosion.

@param h           Water depth.
@param h_w         Water depth of the wave base.
@param h_c         Maximum coastal elevation that is acted on by waves.
@param alpha_sf    The slope of the shoreface.
@param h_wave      The current wave height of incoming waves.
@param h_wave_fair The wave height of waves during calm conditions.

@return The erosion rate (m/year).
*/
double get_erosion_rate( double h        , double h_w    , double h_c ,
                         double alpha_sf , double h_wave , double h_wave_fair )
{
   double c_e = SQUALL_DEFAULT_C_E;
   double c_d = get_coastal_dissipation( alpha_sf );
   double g   = get_erosion_efficiency( h , h_w , h_c );
c_d = 1;
/*
eh_watch_dbl( c_e );
eh_watch_dbl( c_d );
eh_watch_dbl( c_w );
eh_watch_dbl( g   );
*/
//   return c_e*c_d*c_w*g;
   return c_e*g;
}

/** Get the erosion efficiency.

@param h   Water depth.
@param h_w Water depth of the wave base.
@param h_c Maximum coastal elevation that is acted on by waves.

@return The erosion efficiency (dimensionless).
*/
double get_erosion_efficiency( double h , double h_w , double h_c )
{
   double m = SQUALL_DEFAULT_M;
   double g = pow( (h-h_w)/(h_c-h_w) , m );
//   double g = pow( (h_w-h)/(h_w-h_c) , m );

   eh_require( g>=0 );
   eh_require( g<=1 );

   if ( g<0 || g>1 )
   {
      eh_watch_dbl( g   );
      eh_watch_dbl( h   );
      eh_watch_dbl( h_w );
      eh_watch_dbl( h_c );
   }

   return g;
}

/** Get the coastal dissipation.

@param alpha_sf The slope of the shoreface.

@return The coastal dissipation factor (dimensionless).
*/

double get_coastal_dissipation( double alpha_sf )
{
   double alpha_ref = SQUALL_DEFAULT_ALPHA_REF;
   return 1+(alpha_sf-alpha_ref)/alpha_ref;
}

/** Get the coastal wave energy.

@param wave_height_actual The current wave height of incoming waves.
@param wave_height_fair   The wave height of waves during calm conditions.

@return The Coastal wave enerrgy (dimensionless).
*/
double get_coastal_wave_energy( double wave_height_actual ,
                                double wave_height_fair )
{
   return wave_height_actual/wave_height_fair;
}

/** Get the depth of the wave base.

As per usual, the wave base is taken to be half of the wavelength of a wave.

@param wave_length The wavelength of incoming waves.

@return The wave base.
*/
double get_wave_base( double wave_length )
{
   return wave_length/2.;
}

/** Get the depth of the wave base for a breaking wave.

As per usual, the wave base is taken to be half of the wavelength of a wave.
The wave length is assumed to be 7 times its height.

@param wave_height The height of incoming waves.

@return The wave base.
*/
double get_breaking_wave_base( double wave_height )
{
   return get_wave_base( wave_height*7. );
}

/** Get the depth of the wave base for a deep water wave.

As per usual, the wave base is taken to be half of the wavelength of a wave.
The wave length is assumed to be 25 times the wave height.

\param wave_height The height of incoming waves.

\return The wave base.
*/
double get_deep_water_wave_base( double wave_height )
{
   return get_wave_base(wave_height*25.);
}

/** Get the indices to the zone boundaries from a Sed_cube.

Given a Sed_cube, calculate the indices that mark the boundaries of the
six zones in the squall model.  The five boundaries are,
(1) h_w - Wave base.
(2) h_s - Sea level.
(3) h_c - Maximum landward extent of wave action.
(4) h_l - Seaward extend of lagoon.
(5) h_m - Landward extent of lagoon.
If a barrier does not exist, then there will be no lagoon and boundaries (4)
and (5) will not exist.  In such a case, their indices will be zero.

@param p A pointer to a Sed_cube.
@param h_w The water depth of the wave base.
@param h_c The maximum elevation a wave can act on.

@return A newly-allocated array of the five indices.
*/
int *get_zone_boundaries( Sed_cube p , double h_w , double h_c )
{
   int i;
   int *zone = eh_new( int , 5 );

   for ( i=sed_cube_n_y(p)-1 ; i>0 && sed_cube_water_depth(p,0,i)>h_w ; i-- );
   zone[4] = i;

   for ( i=zone[4] ; i>0 && sed_cube_water_depth(p,0,i)>0 ; i-- );
   zone[3] = i;

   for ( i=zone[3] ;
            i>0
         && - sed_cube_water_depth(p,0,i)<h_c
         && (  sed_cube_col_y(p,zone[3])
             - sed_cube_col_y(p,i) < 1500. ) ;
         i-- );
   zone[2] = i;

   for ( i=zone[2] ; i>0 && sed_cube_water_depth(p,0,i)<0 ; i-- );
   zone[1] = i;

   for ( i=zone[1] ; i>0 && sed_cube_water_depth(p,0,i)>0 ; i-- );
   zone[0] = i;

   return zone;
}

/** Get the travel distance for a grain.

The travel distance is clalculated using the equations of Storms 2003.  The
equation used is dependant on the grid resolution that is used and so a
correction is used if the grid spacing is anything other than 50 meters.

@param grain_size_in_m Grain diameter in meters.
@param depth           Water depth in meters.
@param dx              Grid spacing in meters.

@return The travel distance in meters.
*/
double get_travel_dist( double grain_size_in_m , double depth , double dx )
{
   double a   = SQUALL_DEFAULT_A;
   double z_l = SQUALL_DEFAULT_Z_L;
   double h_star = get_non_dim_travel_dist( grain_size_in_m );
   double z = -depth / z_l;
   double h;

   if ( dx!=50 )
      h_star = dx / ( 1 - pow( 1-50/h_star , dx/50 ) );

   h = h_star*( 1.+exp(a*z) );

   return h;
}

/** Get the non-dimensional travel distance for a grain size.

Calculate the non-dimensional travel distance for a grain size using the 
formula of Storms 2003.  

@param grain_size_in_m Grain diameter in units of meters.

@return The non-dimensional travel distance.
*/
double get_non_dim_travel_dist( double grain_size_in_m )
{
   double grain_size_ref = .125e-3;
   double h_star;
   double c_h = SQUALL_DEFAULT_C_H;

   if ( grain_size_in_m>grain_size_ref )
      h_star = c_h*(110. + 590.*pow( grain_size_ref/grain_size_in_m , 2.5 ) );
   else
      h_star = c_h*(500. + 200.*pow( grain_size_ref/grain_size_in_m , 0.6 ) );

   return h_star;
}

/** Determine which grain types are able to move under given ocean conditions.

Use relations of orbital velocity to grain diameter to determine which grain
types are able to move at some depth and wave conditions.  If a grain type
can be moved, its corresponding element in the output array is set to 1, 
otherwise 0.  Currently, only zeros and ones are written to the output array.

This is taken from,

Komar, P.D., 1976.  Beach Processes and Sedimentation.  Prentice Hall Inc, NJ.

@param water_depth Water depth in meters.
@param wave_height Wave height in meters.
@param wave_period Wave period in seconds.
@param sed         Type of sediment of the sea bed.
@param is_moveable Array to store the fraction of each grain type that is able
                   to be moved.  If NULL, a newly allocated array is used.

@return A pointer to an array of doubles that indicate the moveability of each
        grain type.

*/
double *get_moveable_grains( double water_depth ,
                             double wave_height ,
                             double wave_period ,
                             Sed_sediment sed   ,
                             double *is_moveable )
{
   double wave_length = 25.*wave_height;
   double u;
   double d;
   gssize n_grains;

   if ( sed )
      n_grains = sed_sediment_n_types( sed );
   else
      n_grains = sed_sediment_env_n_types( );

//eh_make_note( water_depth*=5 );
/*
   eh_require( wave_height>0 );
   eh_require( wave_period>0 );
   eh_require( water_depth>0 );
*/

   if ( !is_moveable )
      is_moveable = eh_new0( double , n_grains );

   if ( wave_height<= 0 || wave_period <= 0 || water_depth <= 0 )
   {
      gssize n;
      for ( n=0 ; n<n_grains ; n++ )
         is_moveable[n] = FALSE;
      return is_moveable;
   }

   u = M_PI*wave_height/(wave_period*sinh( 2*M_PI*water_depth/wave_length ));

   d = pow(   sed_rho_sea_water()
            / ((sed_rho_quartz()-sed_rho_sea_water())
            * sed_gravity()*.21) ,
            2. )
     * pow( u , 3 )
     * M_PI / wave_period;

   {
      double* grain_size = sed_sediment_property( NULL , &sed_type_grain_size_in_meters );
      gssize n;

      for ( n=0 ; n<n_grains ; n++ )
      {
         if ( grain_size[n] < d )
            is_moveable[n] = TRUE;
         else
            is_moveable[n] = FALSE;
//if ( n!=0 )
//   is_moveable[n] = 1;
      }

      eh_free( grain_size );
   }
   
   return is_moveable;
}

#include <math.h>

double get_threshold_depth( double wave_length , double wave_height ,
                            double wave_period , double grain_size )
{
   double rho   = sed_rho_sea_water();
   double rho_s = sed_rho_quartz();
   double g     = sed_gravity();
   double c_sq  = pow( rho/( .21*g*(rho_s-rho) ), 2.);

   return   wave_length/(2*M_PI)
          * asinh(   M_PI*wave_height/wave_period
                   * pow( M_PI*c_sq/(wave_period*grain_size) , 1./3 ) );
}

