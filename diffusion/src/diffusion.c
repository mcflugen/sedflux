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

/** \file diffusion.c

   Submarine diffusion.

   \defgroup diffusion_group Submarine Diffusion

   \callgraph
*/

/*@{*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "diffusion.h"

// this defines the percent of maximum that the diffusion coefficient will be
// at the supplied skin depth.
#define SKIN_DEPTH_PERCENT (1.)

#define DIFFUSION_OPT_FILL  (1<<0)
#define DIFFUSION_OPT_LAND  (1<<1)
#define DIFFUSION_OPT_WATER (1<<2)

/** 1D-diffusion of seafloor elevations.

Diffuse sediment using the diffusion equation:

   \f[
      { \partial u \over \partial t } = {\partial q(x) \over \partial x}
   \f]
where,
   \f[
      q(x) = k(x,D,H){ \partial u(x) \over \partial x }
   \f]

where, \f$ u \f$ is bathymetric elevation, \f$ t \f$ is time, \f$ q \f$ is sediment
flux, \f$ x \f$ is position, \f$ k \f$ is the diffusion coefficient, \f$ D \f$
is grain size, and \f$ H \f$ is wave height.

the diffusion coefficient (\f$ k \f$) will also fall off exponentially with
depth.   for each sediment column of the profile, we calculate the 
maximum amount of sediment that can be moved from that cell using
the above diffusion equation.  this amount is then adjusted for
grain type.  every grain type is given an alpha value between zero
and one that characterizes its ability to be moved.  zero indicates
that it is difficult to move, one that it is easy to move.

\param prof       A Sed_cube to diffuse.
\param k_max      The maximum value for the diffusion coefficent
\param skin_depth The depth at which \f$ k \f$ reached 1% of it's maximum
\param dt         The time step of the diffusion
\param options    Options that control the method of diffusion

\param A Sed_cell array of sediment that was lost
*/

Sed_cell *diffuse_sediment( Sed_cube prof     , double k_max , 
                            double skin_depth , double dt    ,
                            int options )
{
   Sed_cell *rem_cell, add_cell;
   Sed_cell *lost_cell;
   Sed_cell lost_left;
   Sed_cell lost_right;
   Sed_cell in_susp;
   Sed_cell added_fill;
   Sed_cell bedload_cell;
   double *just_bedload_fraction, *just_suspended_fraction;
   double *alpha_grain;
   double a, *k;
   double qx, *z, *u, *du, *dudx, *u_init;
   double dt_new;
   double depth;
   double dx;
   double u_0;
   long i, n, iter, n_iter=1;
   long remove_index, add_index;
   int n_grains, n_cols;
   Sed_facies facies;
   gssize *cols;

   eh_require( sed_cube_is_1d(prof) );

   sed_cube_find_all_river_mouths( prof );

   if ( options&DIFFUSION_OPT_LAND )
   {
      cols = sed_cube_find_column_above( prof , sed_cube_sea_level(prof) );
      if ( cols )
      {
         for ( n_cols=0 ;
               cols[n_cols]>=0 && cols[n_cols]+1==cols[n_cols+1] ;
               n_cols++ );
         if ( cols[n_cols]>=0 )
            n_cols++;
         eh_free( cols );
      }
      else
         n_cols = 0;

//      n_cols = sed_find_all_river_mouths( prof );

      facies = S_FACIES_RIVER;
   }
   else
   {
      n_cols = sed_cube_n_y(prof);
      facies = S_FACIES_DIFFUSED;
   }

   if ( n_cols<3 )
      return NULL;

   //---
   // this will be the height to sea level.  the diffusion coefficient falls off
   // exponentially below this height and is constant above this height.
   //---
//   u_0 = sed_get_floor_from_cube( prof , 0 , j_river );
   u_0 = sed_cube_sea_level( prof );

   n_grains = sed_sediment_env_size( );
   dx       = sed_cube_y_res( prof );
   
   //---
   // alpha_grain defines the ease at which different grain types can be moved.
   // values near zero are hard to move, values near one are easily moved.
   //---
   alpha_grain = sed_sediment_property( NULL , &sed_type_diff_coef );

   just_bedload_fraction   = eh_new0( double , n_grains );
   just_suspended_fraction = eh_new0( double , n_grains );
   for ( n=1 ; n<n_grains ; n++ )
   {
      just_suspended_fraction[n] = 1.;
   }
   just_suspended_fraction[0] = 0.;
   just_bedload_fraction[0]   = 1.;

   //---
   // If necessary, adjust the time step for stability.
   //---
   if ( k_max*dt/(dx*dx) > .5 )
   {
      dt_new=(dx*dx)/2./k_max;
      if ( options&DIFFUSION_OPT_LAND )
         dt_new /= 2.;
      if ( dt_new < dt )
      {
         n_iter=(int)(dt/dt_new)+1;
         dt=dt/n_iter;
      }
   }

   //---
   // a defines how quickly the diffusion coefficient falls off with depth.
   //---
   if ( skin_depth != 0 )
      a=(1./skin_depth)*log(SKIN_DEPTH_PERCENT/100.);
   else
      a=0;


   u_init   = eh_new( double   , n_cols );
   z        = eh_new( double   , n_cols );
   u        = eh_new( double   , n_cols );
   du       = eh_new( double   , n_cols );
   dudx     = eh_new( double   , n_cols );
   k        = eh_new( double   , n_cols );
   rem_cell = eh_new( Sed_cell , n_cols );
   for ( i=0 ; i<n_cols ; i++ )
      rem_cell[i] = sed_cell_new( n_grains );
   add_cell         = sed_cell_new( n_grains );
   bedload_cell     = sed_cell_new( n_grains );
   sed_cell_set_fraction( bedload_cell ,
                          just_bedload_fraction );
   in_susp    = sed_cell_new( n_grains );
   added_fill = sed_cell_new( n_grains );
   lost_left  = sed_cell_new( n_grains );
   lost_right = sed_cell_new( n_grains );
   lost_cell  = eh_new( Sed_cell , 4 );
   
   for ( iter=0 ; iter<n_iter ; iter++ )
   {
      //---
      // initialize these variables for each iteration.
      //---
      for ( i=0 ; i<n_cols ; i++ )
         sed_cell_clear( rem_cell[i] );
      sed_cell_clear( add_cell );

      //---
      // get the water depths at each point along the profile.
      //---
      for ( i=0 ; i<n_cols ; i++ )
         z[i] = sed_cube_water_depth( prof , 0 , i );
      
      //---
      // get diffusion constants.  the diffusion coefficient will fall
      // exponentially with depth below u_0 (the surface).
      //---
      if ( options&DIFFUSION_OPT_LAND )
         for ( i=0 ; i<n_cols ; i++ )
            k[i] = k_max;
      else
         for ( i=0 ; i<n_cols ; i++ )
         {
            depth = u_0 - sed_cube_top_height(prof,0,i);
            k[i] = k_max*exp(a*depth); 
            if ( k[i]>k_max ) k[i] = k_max;
            if ( depth < 0 ) k[i] = 0.;
         }
      
      //---
      // Get slopes.  Forward difference to find slope
      //---
      for ( i=0 ; i<n_cols-1 ; i++ )
         dudx[i] = -sed_cube_y_slope(prof,0,i);
      dudx[n_cols-1] = dudx[n_cols-2];
      dudx[0] = dudx[1];
      
      //---
      // Determine sediment fluxes between cells.
      // '+' means move to the right, '-' to the left find in meters
      //---
      for ( i=0 ; i<n_cols ; i++ )
      {
         qx = -k[i]*dudx[i];
         du[i] = qx*dt/dx;
      }

      //---
      // Determine the new elevations.
      //---
      if ( options&DIFFUSION_OPT_FILL )
      {
         for ( i=0 ; i<n_cols ; i++ )
            u_init[i] = sed_cube_top_height(prof,0,i);
         for ( i=1 ; i<n_cols-1 ; i++ )
            u[i] = u_init[i]
                 + ( k[i]*(u_init[i+1]-2*u_init[i]+u_init[i-1])/dx/dx
                   + (k[i+1]-k[i])/dx*(u_init[i+1]-u_init[i])/dx )*dt;
         u[0]            = u_init[0];
         u[n_cols-1] = u_init[n_cols-1];
      }
      
      //---
      // Remove the sediment that was diffused and move it to appropriate bins.
      // the amount of sediment to be moved (du[i]) is adjusted for grain size
      // by alpha_grain.  for easily moved grains alpha_grain will be near one,
      // for hard to move grains, alpha_grain will be near zero.
      //---
      for ( i=0 ; i<n_cols-1 ; i++ )
      {
         remove_index = (du[i]>0)?(i):(i+1);
         add_index    = (du[i]>0)?(i+1):(i);

         if ( fabs(du[i]) > 0 )
         {
            sed_column_separate_top( sed_cube_col(prof,remove_index) ,
                                          fabs(du[i]) ,
                                          alpha_grain ,
                                          add_cell );
            if ( options&DIFFUSION_OPT_LAND )
               sed_cell_move_fraction( add_cell ,
                                       in_susp ,
                                       just_suspended_fraction );
            sed_cell_add( rem_cell[add_index] , add_cell );
         }


      }
      
      //---
      // assume the the flux in (or out) of the first cell is the same as the
      // flux in (or out) of the second cell.
      //---
      if ( du[0] < 0 )
      {
         sed_column_separate_top( sed_cube_col(prof,0) ,
                                       -du[0] ,
                                       alpha_grain ,
                                       add_cell );
         sed_cell_add( lost_left , add_cell );
      }
      else
         sed_cell_copy( rem_cell[0] , rem_cell[1] );
            
      //---
      // do the same for the last cell.
      //---
      if ( du[n_cols-1] > 0 )
      {
         sed_column_separate_top( sed_cube_col(prof,n_cols-1) ,
                                       du[n_cols-1] ,
                                       alpha_grain ,
                                       add_cell );
         sed_cell_add( lost_right , add_cell );
      }
      else 
         sed_cell_copy( rem_cell[n_cols-1] , rem_cell[n_cols-2] );
            
      //---
      // Add the sediment that was diffused.
      //---
      for (i=0;i<n_cols;i++)
      {
         sed_cell_set_facies( rem_cell[i] , facies );
         sed_column_add_cell( sed_cube_col(prof,i) ,rem_cell[i]);
      }

      if ( options&DIFFUSION_OPT_FILL )
      {
         for ( i=0 ; i<n_cols ; i++ )
         {
            if ( u[i]-sed_cube_top_height(prof,0,i) > 1e-5 )
            {
               sed_cell_set_facies( bedload_cell , facies );
               sed_cell_set_age( bedload_cell ,
                                 sed_cube_age_in_years(prof) );
               sed_cell_resize( bedload_cell ,
                                u[i] - sed_cube_top_height(prof,0,i) );
               sed_column_add_cell( sed_cube_col(prof,i) , bedload_cell );
               sed_cell_add( added_fill , bedload_cell );
            }
         }
      }

   }

   lost_cell[0] = lost_left;
   lost_cell[1] = lost_right;
   lost_cell[2] = in_susp;
   lost_cell[3] = added_fill;

   sed_cell_destroy( add_cell );
   sed_cell_destroy( bedload_cell );
   for ( i=0 ; i<n_cols ; i++ )
      sed_cell_destroy( rem_cell[i] );
   eh_free( rem_cell );
   eh_free( z    );
   eh_free( u    );
   eh_free( du   );
   eh_free( dudx );
   eh_free( k    );
   eh_free( u_init );
   eh_free( alpha_grain             );
   eh_free( just_bedload_fraction   );
   eh_free( just_suspended_fraction );
   
   return lost_cell;
}


void get_diffusion_components( Eh_dbl_grid slope_dir ,
                               Eh_dbl_grid k_long    ,
                               Eh_dbl_grid k_cross   ,
                               Eh_dbl_grid k_x       ,
                               Eh_dbl_grid k_y );
Eh_dbl_grid diffuse_grid( Eh_dbl_grid g , double **k_x , double **k_y ,
                          double dt     , double dx    , double dy );

/** 2D-diffusion of seafloor elevations.

Diffuse sediment using the 2D-diffusion equation:

   \f[
      { \partial u \over \partial t } = {\partial q_x(x,y) \over \partial x}
                                      + {\partial q_y(x,y) \over \partial y}
   \f]
where,
   \f[
      q_x(x,y) = k_x(x,y,D,H){ \partial u(x,y) \over \partial x }
   \f]
and
   \f[
      q_y(x,y) = k_y(x,y,D,H){ \partial u(x,y) \over \partial y }
   \f]

where, \f$ u \f$ is bathymetric elevation, \f$ t \f$ is time, \f$ q \f$ is sediment
flux, \f$ x \f$ and \f$ y \f$ are position, \f$ k \f$ is the diffusion coefficient, \f$ D \f$
is grain size, and \f$ H \f$ is wave height.

the diffusion coefficient (\f$ k \f$) will also fall off exponentially with
depth.   for each sediment column of the profile, we calculate the 
maximum amount of sediment that can be moved from that cell using
the above diffusion equation.  this amount is then adjusted for
grain type.  every grain type is given an alpha value between zero
and one that characterizes its ability to be moved.  zero indicates
that it is difficult to move, one that it is easy to move.

\param prof        A Sed_cube to diffuse.
\param k_long_max  The maximum long-shore diffusion coefficent
\param k_cross_max The maximum cross-shore diffusion coefficent
\param skin_depth The depth at which \f$ k \f$ reached 1% of it's maximum
\param dt         The time step of the diffusion
\param options    Options that control the method of diffusion

\param A Sed_cell array of sediment that was lost
*/

Sed_cell *diffuse_sediment_2( Sed_cube prof      , double k_long_max , 
                              double k_cross_max , double skin_depth ,
                              double dt          , int options )
{
   Sed_cell **rem_cell_x, **rem_cell_y, **rem_cell, add_cell;
   Sed_cell_grid rem_cell_x_grid, rem_cell_y_grid, rem_cell_grid;
   Sed_cell *lost_cell;
   Sed_cell lost_left;
   Sed_cell lost_right;
   Sed_cell in_susp;
   Sed_cell added_fill;
   Sed_cell bedload_cell;
   Eh_dbl_grid u;
   Eh_dbl_grid dudx;
   Eh_dbl_grid dudy;
   Eh_dbl_grid slope_dir;
   Eh_dbl_grid k_long, k_cross;
   Eh_dbl_grid x_current, y_current;
   Eh_dbl_grid k_x, k_y;
   Eh_dbl_grid qx_grid, qy_grid;
   double **qx, **qy;
   double *just_bedload_fraction, *just_suspended_fraction;
   double *alpha_grain;
   double a, k_max, dt_new, depth;
   double dx, dy;
   double water_depth;
   gssize i, j, n, iter, n_iter=1;
   gssize remove_index, add_index;
   gssize n_x, n_y;
   gsize n_grains;
   Sed_facies facies;

   if ( options&DIFFUSION_OPT_LAND )
      facies = S_FACIES_RIVER;
   else
      facies = S_FACIES_DIFFUSED;
   
   n_grains = sed_sediment_env_size( );
   dx       = sed_cube_x_res( prof );
   dy       = sed_cube_y_res( prof );

   //---
   // alpha_grain defines the ease at which different grain types can be moved.
   // values near zero are hard to move, values near one are easily moved.
   //---
   alpha_grain = sed_sediment_property( NULL , &sed_type_diff_coef );

   just_bedload_fraction   = eh_new( double , n_grains );
   just_suspended_fraction = eh_new( double , n_grains );
   for ( n=1 ; n<n_grains ; n++ )
   {
      just_bedload_fraction[n]   = 0.;
      just_suspended_fraction[n] = 1.;
   }
   just_bedload_fraction[0]   = 1.;
   just_suspended_fraction[0] = 0.;

   //---
   // If necessary, adjust the time step for stability.
   //---
   k_max = sqrt( pow(k_long_max,2) + pow(k_cross_max,2) );
   if ( k_max*dt*(1./dx/dx+1./dy/dy) > .25 )
   {
      dt_new=.25/k_max/(1/dx/dx+1./dy/dy);
      dt_new /= 2;
      if ( options&DIFFUSION_OPT_LAND )
         dt_new /= 2.;
      if ( dt_new < dt )
      {
         n_iter=(int)(dt/dt_new)+1;
         dt=dt/n_iter;
      }
   }

   //---
   // a defines how quickly the diffusion coefficient falls off with depth.
   //---
   if ( skin_depth != 0 )
      a=(1./skin_depth)*log(SKIN_DEPTH_PERCENT/100.);
   else
      a=0;

   n_x = sed_cube_n_x( prof );
   n_y = sed_cube_n_y( prof );

   rem_cell_x_grid = sed_cell_grid_new( n_x , n_y );
   rem_cell_y_grid = sed_cell_grid_new( n_x , n_y );
   rem_cell_grid   = sed_cell_grid_new( n_x , n_y );
   sed_cell_grid_init( rem_cell_x_grid , n_grains );
   sed_cell_grid_init( rem_cell_y_grid , n_grains );
   sed_cell_grid_init( rem_cell_grid   , n_grains );
   k_x           = eh_grid_new( double , n_x , n_y );
   k_y           = eh_grid_new( double , n_x , n_y );
   k_long        = eh_grid_new( double , n_x , n_y );
   k_cross       = eh_grid_new( double , n_x , n_y );
   x_current     = eh_grid_new( double , n_x , n_y );
   y_current     = eh_grid_new( double , n_x , n_y );
/*
   qx_grid       = eh_create_dbl_grid( prof->n_x+2 , prof->n_y+2 );
   qy_grid       = eh_create_dbl_grid( prof->n_x+2 , prof->n_y+2 );
*/
   qx_grid       = eh_grid_new( double , n_x , n_y );
   qy_grid       = eh_grid_new( double , n_x , n_y );
//   eh_reindex_grid( qx_grid , -1 , -1 );
//   eh_reindex_grid( qy_grid , -1 , -1 );
   add_cell      = sed_cell_new( n_grains );
   bedload_cell  = sed_cell_new( n_grains );
   sed_cell_set_fraction( bedload_cell ,
                          just_bedload_fraction );
   in_susp    = sed_cell_new( n_grains );
   added_fill = sed_cell_new( n_grains );
   lost_left  = sed_cell_new( n_grains );
   lost_right = sed_cell_new( n_grains );
   lost_cell  = eh_new( Sed_cell , 4 );

   rem_cell_x = sed_cell_grid_data( rem_cell_x_grid );
   rem_cell_y = sed_cell_grid_data( rem_cell_y_grid );
   rem_cell   = sed_cell_grid_data( rem_cell_grid   );
   qx         = eh_dbl_grid_data( qx_grid );
   qy         = eh_dbl_grid_data( qy_grid );

   for ( i=0 ; i<eh_grid_n_x(x_current) ; i++ )
      for ( j=0 ; j<eh_grid_n_y(x_current) ; j++ )
      {
         eh_dbl_grid_set_val( x_current , i , j ,
                                 -2
                               * (eh_grid_n_x(x_current)-i)/(double)eh_grid_n_x(x_current)
                               * j/(double)eh_grid_n_y(x_current) );
         eh_dbl_grid_set_val( y_current , i , j , 0. );
      }
   
   for ( iter=0 ; iter<n_iter ; iter++ )
   {
eh_message( "initialize grids to zero" );
      //---
      // initialize these variables for each iteration.
      //---
      for ( i=0 ; i<n_x ; i++ )
         for ( j=0 ; j<n_y ; j++ )
         {
            sed_cell_clear( rem_cell_x[i][j] );
            sed_cell_clear( rem_cell_y[i][j] );
            sed_cell_clear( rem_cell[i][j]   );
         }
      sed_cell_clear( add_cell );
      
eh_message( "calculate diffusion coefficients" );
      //---
      // get diffusion constants.  the diffusion coefficient will fall
      // exponentially with depth below the water surface.
      //---
      if ( options&DIFFUSION_OPT_LAND )
         for ( i=0 ; i<n_x; i++ )
            for ( j=0 ; j<n_y; j++ )
            {
               eh_dbl_grid_set_val( k_long  , i , j , k_long_max  );
               eh_dbl_grid_set_val( k_cross , i , j , k_cross_max );
            }
      else
         for ( i=0 ; i<n_x; i++ )
            for ( j=0 ; j<n_y; j++ )
            {
               depth = sed_cube_water_depth(prof,i,j);
               if ( depth > 0 )
               {
                  eh_dbl_grid_set_val( k_long  , i , j , k_long_max *exp(a*depth) );
                  eh_dbl_grid_set_val( k_cross , i , j , k_cross_max*exp(a*depth) );
                  eh_clamp( eh_dbl_grid_data(k_long) [i][j] , 0 , k_long_max );
                  eh_clamp( eh_dbl_grid_data(k_cross)[i][j] , 0 , k_cross_max );
               }
               else
               {
                  eh_dbl_grid_set_val( k_long  , i , j , 0. );
                  eh_dbl_grid_set_val( k_cross , i , j , 0. );
               }
            }

      slope_dir = sed_cube_slope_dir_grid( prof , NULL );
      get_diffusion_components( slope_dir , k_long , k_cross , k_x , k_y );
      
eh_message( "calculate seafloor slopes" );
      //---
      // Get slopes.  Forward difference to find slope
      //---
      dudx = sed_cube_x_slope_grid( prof , NULL );
      dudy = sed_cube_y_slope_grid( prof , NULL );
      
eh_message( "calculate sediment fluxes" );
      //---
      // Determine sediment fluxes between cells.
      // '+' means move to the right, '-' to the left
      //---
      for ( i=0 ; i<n_x ; i++ )
      {
         for ( j=0 ; j<n_y ; j++ )
         {
            qy[i][j] = eh_dbl_grid_val(k_y,i,j)*eh_dbl_grid_val(dudy,i,j)*dt/dy;
            qx[i][j] = eh_dbl_grid_val(k_x,i,j)*eh_dbl_grid_val(dudx,i,j)*dt/dx;
/*
            qy[i][j] = k_y->data[i][j]*dudy->data[i][j]*dt/dy
                     + y_current->data[i][j]*dt/dy;
            qx[i][j] = k_x->data[i][j]*dudx->data[i][j]*dt/dx
                     + x_current->data[i][j]*dt/dx;
            long_shore = slope_dir->data[i][j] + M_PI_2;
            long_shore = 0;
            qy[i][j] = x_current->data[i][j]*dt/dy*sin( long_shore );
            qx[i][j] = x_current->data[i][j]*dt/dx*cos( long_shore );
*/
         }
      }

      eh_grid_destroy( slope_dir , TRUE );
/*
      for ( i=0 ; i<prof->n_x ; i++ )
      {
//         qy[i][0]        = qy[i][1];
         qy[i][-1]          = qy[i][0];
         qy[i][prof->n_y-1] = qy[i][prof->n_y-2];
//         qy[i][prof->n_x] = qy[i][prof->n_x-1];
//         qy[i][-1]        = 0;
//         qy[i][prof->n_x] = 0;
      }
      for ( j=0 ; j<prof->n_y ; j++ )
      {
//         qx[0][j]        = qx[1][j];
         qx[-1][j]          = qx[0][j];
         qx[prof->n_x-1][j] = qx[prof->n_x-2][j];
//         qx[prof->n_y][j] = qx[prof->n_y-1][j];
//         qx[-1][j]        = 0;
//         qx[prof->n_y][j] = 0;
      }
      qx[-1][-1] = 0;
      qy[-1][-1] = 0;
*/
      eh_grid_destroy( dudx , TRUE );
      eh_grid_destroy( dudy , TRUE );

      //---
      // Determine the new elevations.
      //---
      if ( options&DIFFUSION_OPT_FILL )
      {
eh_require_not_reached();
         u = sed_cube_water_depth_grid( prof , NULL );
         u = diffuse_grid( u , eh_dbl_grid_data(k_x) , eh_dbl_grid_data(k_y) , dt , dx , dy );
      }

/*
      for ( i=0 ; i<prof->n_x ; i++ )
         for ( j=0 ; j<prof->n_y ; j++ )
         {
sed_cell_clear( add_cell , n_grains );
sed_cell_clear( rem_cell[i][j], n_grains );
            q = fabs(qx[i][j]) + fabs(qy[i][j]);
            if ( q>0 )
            {
               sed_column_separate_top( prof->col[i][j] ,
                                             q               ,
                                             alpha_grain     ,
                                             add_cell );
               sed_add_cell_to_cell( rem_cell[i][j] ,
                                     add_cell       ,
                                     n_grains );
eh_watch_dbl( q );
eh_watch_int( i );
eh_watch_int( j );
eh_watch_dbl( add_cell->thickness );
eh_watch_dbl( rem_cell[i][j]->thickness );
            }
         }
            
      for ( i=0 ; i<prof->n_x ; i++ )
      {
         for ( j=-1 ; j<prof->n_y+1 ; j++ )
         {
            remove_index = j;
            add_index    = (qy[i][j]>0)?(j+1):(j-1);
            if ( fabs( qy[i][j] ) > 0 )
            {
sed_cell_clear( add_cell , n_grains );
f = fabs( qy[i][j] ) / ( fabs(qy[i][j] ) + fabs( qx[i][j] ) );
if ( f>1 )
   eh_watch_dbl( f );
               if ( is_in_domain( prof->n_x , prof->n_y , i , remove_index ) )
               {
                  sed_copy_cell( add_cell , rem_cell[i][j] , n_grains );
                  sed_cell_resize(
                     add_cell ,
                     add_cell->thickness
                     * fabs( qy[i][j] )/(fabs(qx[i][j])+fabs(qy[i][j]) ) );
eh_watch_dbl( qy[i][j] );
eh_watch_dbl( qx[i][j] );
eh_watch_dbl( rem_cell[i][j]->thickness );
eh_watch_dbl( add_cell->thickness );
               }
               else if ( is_in_domain( prof->n_x , prof->n_y , i , add_index ) )
               {
                  sed_copy_cell( add_cell , rem_cell[i][add_index] , n_grains );
                  sed_cell_resize(
                     add_cell ,
                     add_cell->thickness
                     * fabs( qy[i][j] )/(fabs(qx[i][j])+fabs(qy[i][j]) ) );
               }
               else
                  sed_cell_clear( add_cell , n_grains );

if ( add_cell->thickness > fabs( qy[i][j] ) )
   eh_watch_dbl( add_cell->thickness );

               if ( options&DIFFUSION_OPT_FILL )
                  sed_move_sediment_to_cell_by_fraction(
                     add_cell                ,
                     in_susp                 ,
                     just_suspended_fraction ,
                     n_grains );

               if ( is_in_domain( prof->n_x , prof->n_y , i , add_index ) )
{
                  sed_add_cell_to_cell( rem_cell_y[i][add_index] ,
                                        add_cell                 ,
                                        n_grains );
eh_watch_dbl( rem_cell_y[i][add_index]->thickness );
eh_watch_int( i );
eh_watch_int( add_index );
}
               else
                  sed_add_cell_to_cell( lost_left , add_cell , n_grains );
            }
         }
      }

      for ( i=-1 ; i<prof->n_x+1 ; i++ )
      {
         for ( j=0 ; j<prof->n_y ; j++ )
         {
            remove_index = i;
            add_index    = (qx[i][j]>0)?(i+1):(i-1);
            if ( fabs( qx[i][j] ) > 0 )
            {
sed_cell_clear( add_cell , n_grains );
f = fabs( qx[i][j] ) / ( fabs(qy[i][j] ) + fabs( qx[i][j] ) );
if ( f>1 )
   eh_watch_dbl( f );
               if ( is_in_domain( prof->n_x , prof->n_y , remove_index , j ) )
               {
                  sed_copy_cell( add_cell , rem_cell[i][j] , n_grains );
                  sed_cell_resize(
                     add_cell ,
                     add_cell->thickness
                     * fabs( qx[i][j] )/(fabs(qx[i][j])+fabs(qy[i][j]) ) );
               }
               else if ( is_in_domain( prof->n_x , prof->n_y , add_index , j ) )
               {
                  sed_copy_cell( add_cell , rem_cell[add_index][j] , n_grains );
                  sed_cell_resize(
                     add_cell ,
                     add_cell->thickness
                     * fabs( qx[i][j] )/(fabs(qx[i][j])+fabs(qy[i][j]) ) );
               }
               else
                  sed_cell_clear( add_cell , n_grains );

if ( add_cell->thickness > fabs( qx[i][j] ) )
   eh_watch_dbl( add_cell->thickness );

               if ( options&DIFFUSION_OPT_FILL )
                  sed_move_sediment_to_cell_by_fraction(
                     add_cell                ,
                     in_susp                 ,
                     just_suspended_fraction ,
                     n_grains );

               if ( is_in_domain( prof->n_x , prof->n_y , add_index , j ) )
                  sed_add_cell_to_cell( rem_cell_x[add_index][j] ,
                                        add_cell                 ,
                                        n_grains );
               else
                  sed_add_cell_to_cell( lost_left , add_cell , n_grains );
            }
         }
      }

*/

eh_message( "remove the diffused sediment" );
      //---
      // Remove the sediment that was diffused and move it to appropriate bins.
      // the amount of sediment to be moved (du[i]) is adjusted for grain size
      // by alpha_grain.  for easily moved grains alpha_grain will be near one,
      // for hard to move grains, alpha_grain will be near zero.
      //---
      for ( i=0 ; i<n_x ; i++ )
      {
//         for ( j=-1 ; j<n_y ; j++ )
         for ( j=0 ; j<n_y-1 ; j++ )
         {

            //---
            // Calculate the sediment flux in the y-direction.  For a positive
            // flux, move sediment from this cell to the next.  For a negative
            // flux, move sediment from the next cell to this one.  If the
            // flux moves the sediment out of the domain, remove the sediment
            // from this cell and add it to the accumulated lost sediment.
            // If the flux moves sediment into the domain, assume that the
            // flux into this cell is the same as that leaving this cell.
            // Thus, the final height of this cell will remain unchanged.
            //---
            remove_index = (qy[i][j]>0)?(j):(j+1);
            add_index    = (qy[i][j]>0)?(j+1):(j);
//            remove_index = j;
//            add_index    = (qy[i][j]>0)?(j+1):(j-1);
            if ( fabs( qy[i][j] ) > 0 )
            {
//               if ( is_in_domain( prof->n_x , prof->n_y , i , remove_index ) )
               if ( remove_index != 0 && remove_index != n_y-1 )
                  sed_column_separate_top( sed_cube_col_ij(prof,i,remove_index) ,
                                           fabs(qy[i][j])                       ,
                                           alpha_grain                          ,
                                           add_cell );
//               else if ( is_in_domain( prof->n_x , prof->n_y , i , add_index ) )
               else 
                  sed_column_top( sed_cube_col_ij(prof,i,remove_index) ,
                                  fabs(qy[i][j])             ,
                                  add_cell );
//               else
//                  sed_cell_clear( add_cell , n_grains );

               if ( options&DIFFUSION_OPT_FILL )
                  sed_cell_move_fraction( add_cell                ,
                                          in_susp                 ,
                                          just_suspended_fraction );

               if ( sed_cube_is_in_domain( prof , i , add_index ) )
                  sed_cell_add( rem_cell_y[i][add_index] , add_cell );
               else
                  sed_cell_add( lost_left , add_cell );
            }
         }
      }


      //---
      // Now do the fluxes in the x-direction.
      //---
//      for ( i=-1 ; i<prof->n_x ; i++ )
      for ( i=0 ; i<n_x-1 ; i++ )
      {
         for ( j=0 ; j<n_y ; j++ )
         {
            remove_index = (qx[i][j]>0)?(i):(i+1);
            add_index    = (qx[i][j]>0)?(i+1):(i);
//            remove_index = i;
//            add_index    = (qx[i][j]>0)?(i+1):(i-1);

            if ( fabs( qx[i][j] ) > 0 )
            {
//               if ( is_in_domain( prof->n_x , prof->n_y , remove_index , j ) )
               if ( remove_index != 0 && remove_index != n_x-1 )
                  sed_column_separate_top( sed_cube_col_ij(prof,remove_index,j) ,
                                           fabs(qx[i][j])             ,
                                           alpha_grain                ,
                                           add_cell );
//               else if ( is_in_domain( prof->n_x , prof->n_y , add_index , j ) )
               else
                  sed_column_top( sed_cube_col_ij(prof,remove_index,j) ,
                                  fabs(qx[i][j])             ,
                                  add_cell );
//               else
//                  sed_cell_clear( add_cell , n_grains );

               if ( options&DIFFUSION_OPT_FILL )
                  sed_cell_move_fraction( add_cell                ,
                                          in_susp                 ,
                                          just_suspended_fraction );

               if ( sed_cube_is_in_domain( prof , add_index , j ) )
                  sed_cell_add( rem_cell_x[add_index][j] , add_cell );
               else
                  sed_cell_add( lost_left , add_cell );
            }
         }
      }
      
eh_message( "add the diffused sediment" );
      //---
      // Add the sediment that was diffused.
      //---
      for ( i=0 ; i<n_x ; i++ )
         for ( j=0 ; j<n_y ; j++ )
         {
            sed_cell_copy( add_cell , rem_cell_x[i][j] );
            sed_cell_add( add_cell , rem_cell_y[i][j] );
            sed_cell_set_facies( add_cell , facies );

            water_depth = sed_cube_water_depth( prof , i , j );
            if ( sed_cell_thickness(add_cell) > water_depth )
               sed_cell_resize( add_cell , eh_max( water_depth , 0 ) );

            sed_column_add_cell( sed_cube_col_ij(prof,i,j) , add_cell );
         }

      for ( i=0 ; i<n_x ; i++ )
      {
         sed_cell_clear( add_cell );
         if ( qy[i][0] < 0 )
            sed_column_separate_top( sed_cube_col_ij(prof,i,0) ,
                                          fabs(qy[i][0])  ,
                                          alpha_grain     ,
                                          add_cell );
         else
         {
/*
            sed_get_top_from_column( prof->col[i][0] ,
                                     fabs(qy[i][0])  ,
                                     add_cell );
            sed_column_add_cell( prof->col[i][1] , add_cell );
*/
         }
         sed_cell_clear( add_cell );
         if ( qy[i][n_y-1] > 0 )
            sed_column_separate_top( sed_cube_col_ij(prof,i,n_y-1) ,
                                     fabs(qy[i][n_y-1])            ,
                                     alpha_grain                   ,
                                     add_cell );
         else
         {
/*
            sed_get_top_from_column( prof->col[i][prof->n_y-1] ,
                                     fabs(qy[i][prof->n_y-1])  ,
                                     add_cell );

            water_depth = sed_get_depth_from_cube( prof , i , prof->n_y-1 );
            if ( add_cell->thickness > water_depth )
               sed_cell_resize( add_cell , eh_max( water_depth , 0 ) );

            sed_column_add_cell( prof->col[i][prof->n_y-2] , add_cell );
*/
         }
      }

      for ( j=0 ; j<n_y ; j++ )
      {
         sed_cell_clear( add_cell );
         if ( qx[0][j] < 0 )
            sed_column_separate_top( sed_cube_col_ij(prof,0,j) ,
                                          fabs(qx[0][j])  ,
                                          alpha_grain     ,
                                          add_cell );
         else
         {
/*
            sed_get_top_from_column( prof->col[0][j] ,
                                     fabs(qx[0][j])  ,
                                     add_cell );
            sed_column_add_cell( prof->col[1][j] , add_cell );
*/
         }
         sed_cell_clear( add_cell );
         if ( qx[n_x-1][j] > 0 )
            sed_column_separate_top( sed_cube_col_ij(prof,n_x-1,j) ,
                                     fabs(qx[n_x-1][j])            ,
                                     alpha_grain                   ,
                                     add_cell );
         else
         {
/*
            sed_get_top_from_column( prof->col[prof->n_x-1][j] ,
                                     fabs(qx[prof->n_x-1][j])  ,
                                     add_cell );

            water_depth = sed_get_depth_from_cube( prof , prof->n_x-1 , j );
            if ( add_cell->thickness > water_depth )
               sed_cell_resize( add_cell , eh_max( water_depth , 0 ) );

            sed_column_add_cell( prof->col[prof->n_x-2][j] , add_cell );
*/
         }
      }

      if ( options&DIFFUSION_OPT_FILL )
      {
eh_require_not_reached();
         for ( i=0 ; i<n_x ; i++ )
            for ( j=0 ; j<n_y ; j++ )
            {
               if ( eh_dbl_grid_val(u,i,j)-sed_cube_water_depth(prof,i,j) > 1e-5 )
               {
                  sed_cell_set_facies( bedload_cell , facies );
                  sed_cell_resize( bedload_cell ,
                                     eh_dbl_grid_val(u,i,j)
                                   - sed_cube_water_depth(prof,i,j) );
                  sed_column_add_cell( sed_cube_col_ij(prof,i,j) , bedload_cell );
                  sed_cell_add( added_fill , bedload_cell );
               }
            }
      }

   }

   lost_cell[0] = lost_left;
   lost_cell[1] = lost_right;
   lost_cell[2] = in_susp;
   lost_cell[3] = added_fill;

   for ( i=0 ; i<4 ; i++ )
      sed_cell_destroy( lost_cell[i] );
   eh_free( lost_cell );
   lost_cell = NULL;

   sed_cell_destroy( add_cell     );
   sed_cell_destroy( bedload_cell );
   sed_cell_grid_free( rem_cell_x_grid );
   sed_cell_grid_free( rem_cell_y_grid );
   sed_cell_grid_free( rem_cell_grid   );

   eh_grid_destroy( k_long        , TRUE );
   eh_grid_destroy( k_cross       , TRUE );
   eh_grid_destroy( x_current     , TRUE );
   eh_grid_destroy( y_current     , TRUE );
   eh_grid_destroy( k_x           , TRUE );
   eh_grid_destroy( k_y           , TRUE );
   eh_grid_destroy( qx_grid       , TRUE );
   eh_grid_destroy( qy_grid       , TRUE );
   eh_grid_destroy( rem_cell_x_grid , TRUE );
   eh_grid_destroy( rem_cell_y_grid , TRUE );
   eh_grid_destroy( rem_cell_grid   , TRUE );

   if ( options&DIFFUSION_OPT_FILL )
      eh_grid_destroy( u , TRUE );
   eh_free(alpha_grain);
   eh_free(just_bedload_fraction);
   eh_free(just_suspended_fraction);
   
   return lost_cell;
}

void get_diffusion_components( Eh_dbl_grid slope_dir ,
                               Eh_dbl_grid k_long    ,
                               Eh_dbl_grid k_cross   ,
                               Eh_dbl_grid k_x       ,
                               Eh_dbl_grid k_y )
{
   gsize i, j;

   eh_require( slope_dir );
   eh_require( k_long    );
   eh_require( k_cross   );
   eh_require( k_x       );
   eh_require( k_y       );
   eh_require( eh_grid_is_compatible( slope_dir , k_long  ) );
   eh_require( eh_grid_is_compatible( slope_dir , k_cross ) );
   eh_require( eh_grid_is_compatible( slope_dir , k_x     ) );
   eh_require( eh_grid_is_compatible( slope_dir , k_y     ) );

   for ( i=0 ; i<eh_grid_n_x(slope_dir) ; i++ )
      for ( j=0 ; j<eh_grid_n_y(slope_dir) ; j++ )
      {
         eh_dbl_grid_set_val( k_x , i , j , fabs(   eh_dbl_grid_val(k_cross,i,j)
                                                  * cos( eh_dbl_grid_val(slope_dir,i,j) ) )
                                                  + fabs(   eh_dbl_grid_val(k_long,i,j)
                                                  * sin( eh_dbl_grid_val(slope_dir,i,j) ) ) );
         eh_dbl_grid_set_val( k_y , i , j , fabs(   eh_dbl_grid_val(k_cross,i,j)
                                                  * sin( eh_dbl_grid_val(slope_dir,i,j) ) )
                                                  + fabs(   eh_dbl_grid_val(k_long,i,j)
                                                  * cos( eh_dbl_grid_val(slope_dir,i,j) ) ) );
      }

   return;
}

/** Solve the diffusion equation on a grid

\param g A Eh_dbl_grid
\param k_x Diffusion coefficients in the x-direction
\param k_y Diffusion coefficients in the y-direction
\param dt  Time step
\param dx  Grid spacing in the x-direction
\param dy  Grid spacing in the y-direction

\return The input Eh_dbl_grid \a g that has been diffused
*/
Eh_dbl_grid diffuse_grid( Eh_dbl_grid g , double **k_x , double **k_y ,
                          double dt     , double dx    , double dy )
{
   gsize i, j;
   Eh_dbl_grid g_temp = eh_grid_new( double , eh_grid_n_x(g) , eh_grid_n_y(g) );
   double **u = eh_dbl_grid_data(g);
   gssize n_x = eh_grid_n_x( g );
   gssize n_y = eh_grid_n_y( g );

   for ( i=1 ; i<=n_x ; i++ )
      for ( j=1 ; j<=n_y ; j++ )
         eh_dbl_grid_set_val( g_temp , i , j ,
                              (   u[i-1][j] * k_x[i][j] 
                                - u[i][j]   * ( k_x[i][j] + k_x[i+1][j] )
                                + u[i+1][j] * k_x[i+1][j] ) * dt / (dx * dx)
                            + (   u[i][j-1] * k_y[i][j] 
                                - u[i][j]   * ( k_y[i][j] + k_y[i][j+1] )
                                + u[i][j+1] * k_y[i][j+1] ) * dt / (dy * dy) );
   for ( j=0 ; j<n_y ; j++ )
   {
      eh_dbl_grid_set_val( g_temp , 0     , j     , u[0][j]     );
      eh_dbl_grid_set_val( g_temp , n_x-1 , j     , u[n_x-1][j] );
   }
   for ( i=0 ; i<n_x ; i++ )
   {
      eh_dbl_grid_set_val( g_temp , i     , 0     , u[i][0]     );
      eh_dbl_grid_set_val( g_temp , i     , n_y-1 , u[i][n_y-1] );
   }

   eh_grid_set_data( g      , eh_grid_data(g_temp) );
   eh_grid_set_data( g_temp , (void**)u            );

   eh_grid_destroy( g_temp , TRUE );

   return g;
}

/*@}*/
