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

/*
 * PlumeOut2.c   Prints the data to the output files for Plume (standalone)
 *      using MATLAB Binary file format (readsf2d.m)
 *
 *   Author:      M.D. Morehead
 *   Original:   April-May 1998
 *   Modified:   Sep 1998, MDM, Conversion for SEDFLUX3D
 *         see plume.c for list of changes
 *
 *   A binary file is written for each of the centerline averaged variables:
 *   (where fname = the user input file name)
 *
 * filename   internal variable   description
 * --------   -----------------   -----------
 * fname.sf2d   cdps(ly,ngrains)   centerline deposit thickness for each grain size
 *       tdps(ly,1)      total centerline deposit thickness
 *
 *   Each file contains a header followed by the xval and yval arrays
 *   followed by the data arrays.
 *
 *   The header contains (one line for each variable):
 *      lx
 *      ngrains (or -ngrains for total thickness)
 *      total sediment discharged from river (kg)
 *      total deposit (kg)
 *      rho...
 *
 *   Followed by an arrays of:
 *      xval(1:lx)
 *
 *   Then data arrays of the form:
 *      cdps(1,1)  cdps(2,1)  ... cdps(lx,1)
 *      cdps(1,2)  cdps(2,2)  ... cdps(lx,2)
 *      ...
 *      cdps(1,ngrain)  cdps(2,ngrain)  ... cdps(lx,ngrain)
 *
 *      tdps(1) ... tdps(lx)
 *
 *   Author:      M.D. Morehead
 *   Original:   May 1998
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"

int plumeout3( Plume_enviro *env , Plume_grid *grid , Eh_dbl_grid *deposit_grid )
{
   int   ii, jj, nn;
   gsize i_0, j_0;
   double mass_in, mass_out;
   Plume_river river = *(env->river);
   Plume_sediment *sedload = env->sed;
   Eh_dbl_grid plume_grid;
   double shore_angle = eh_reduce_angle( river.rdirection );
   double deposit_dx, deposit_dy, plume_dx, plume_dy;

   i_0 = eh_grid_n_x(deposit_grid[0])/2;
   j_0 = eh_grid_n_y(deposit_grid[0])/2;

   //---
   // This grid will hold the deposit from the plume.  This grid is required
   // because the indices of the plume grid are:
   //   1. cross-shore
   //   2. along-shore
   //   3. grain size
   // However, the dimensions of the output grid is:
   //   1. along-shore
   //   2. cross-shore
   // Also, the interpolation function requires an Eh_dbl_grid as input.
   //---
   plume_grid = eh_grid_new( double , grid->ly , grid->lx );
   memcpy( eh_grid_x(plume_grid) , grid->yval , eh_grid_n_x(plume_grid)*sizeof(double) );
   memcpy( eh_grid_y(plume_grid) , grid->xval , eh_grid_n_y(plume_grid)*sizeof(double) );
   
   //---
   // we calculate the sediment mass that was input (for each grain
   // size) and the mass the is output.  if there is a difference, we scale
   // the output mass to assure that mass is balanced.  we assume that any
   // discrepancy is a result of small numerical errors.
   //---
   for ( nn=0 ; nn<env->n_grains ; nn++ )
   {

      if ( river.Cs[nn] > .001 )
      {
         mass_in = river.Cs[nn]*river.Q*dTOs;
   
         //---
         // Transfer the deposit data to a grid so that it can be interpolated
         // to the output grid.
         // For the plume grid, the second dimension is along shore and the first
         // dimension is cross shore.  The reverse is true for the output grid.
         //---
         for ( ii=0 ; ii<grid->lx ; ii++ )
            for ( jj=0 ; jj<grid->ly ; jj++ )
               eh_dbl_grid_data(plume_grid)[jj][ii] = grid->deps[ii][jj][nn];
   
         mass_out = eh_dbl_grid_sum( plume_grid )
                  * sedload[nn].rho
                  * (eh_grid_x(plume_grid)[1] - eh_grid_x(plume_grid)[0])
                  * (eh_grid_y(plume_grid)[1] - eh_grid_y(plume_grid)[0]);
   
   //      interpolate_2_bad_val( plume_grid , deposit_grid[nn] , 0 );
   
         eh_dbl_grid_rebin_bad_val( plume_grid , deposit_grid[nn] , 0 );
   /*
   eh_watch_dbl( eh_grid_x(deposit_grid[nn])[0] );
   eh_watch_dbl( eh_grid_x(deposit_grid[nn])[1] );
   eh_watch_dbl( eh_grid_y(deposit_grid[nn])[0] );
   eh_watch_dbl( eh_grid_y(deposit_grid[nn])[eh_grid_n_y(deposit_grid[nn])-1] );
   eh_watch_dbl( eh_grid_x(plume_grid)[0] );
   eh_watch_dbl( eh_grid_x(plume_grid)[eh_grid_n_x(plume_grid)-1] );
   eh_watch_dbl( eh_grid_y(plume_grid)[0] );
   eh_watch_dbl( eh_grid_y(plume_grid)[eh_grid_n_y(plume_grid)-1] );
   eh_watch_dbl( eh_dbl_grid_val( deposit_grid[nn] , 0 , eh_grid_n_y(deposit_grid[nn])/2. ) );
   eh_watch_dbl( eh_dbl_grid_val( deposit_grid[nn] , 0 , eh_grid_n_y(deposit_grid[nn])/2.+1 ) );
   eh_watch_dbl( eh_dbl_grid_val( deposit_grid[nn] , 1 , eh_grid_n_y(deposit_grid[nn])/2. ) );
   eh_watch_dbl( eh_dbl_grid_val( deposit_grid[nn] , 1 , eh_grid_n_y(deposit_grid[nn])/2.+1 ) );
   */
   
         deposit_dx = eh_grid_x(deposit_grid[nn])[1] - eh_grid_x(deposit_grid[nn])[0];
         deposit_dy = eh_grid_y(deposit_grid[nn])[1] - eh_grid_y(deposit_grid[nn])[0];
         plume_dx   = eh_grid_x(plume_grid)[1] - eh_grid_x(plume_grid)[0];
         plume_dy   = eh_grid_y(plume_grid)[1] - eh_grid_x(plume_grid)[0];
   
   //      eh_scalar_mult_dbl_grid( deposit_grid[nn] ,
   //                               plume_dx*plume_dy/(deposit_dx*deposit_dy) );
   
   /*
         for ( ii=0 ; ii<deposit_grid[nn]->n_x ; ii++ )
            for ( jj=0 ; jj<deposit_grid[nn]->n_y ; jj++ )
               if ( eh_isnan( deposit_grid[nn]->data[ii][jj]) )
                  deposit_grid[nn]->data[ii][jj] = 0;
   */
   
   
         //---
         // Calculate the output mass for this grain size.  If the input and output
         // masses differ, scale the output deposit to conserve mass.
         //---
         mass_out = eh_dbl_grid_sum( deposit_grid[nn] )
                  * sedload[nn].rho
                  * (eh_grid_x(deposit_grid[nn])[1] - eh_grid_x(deposit_grid[nn])[0])
                  * (eh_grid_y(deposit_grid[nn])[1] - eh_grid_y(deposit_grid[nn])[0]);


         //if ( mass_out>0 && fabs( mass_out-mass_in )>1e-5 )
         if ( mass_out>0 && !eh_compare_dbl( mass_in , mass_out , 1e-5 ) )
            eh_dbl_grid_scalar_mult( deposit_grid[nn] , mass_in/mass_out );
         else if ( mass_out<0 )
            eh_require_not_reached();

      }

   }

   {
      double mass_lost = 0;
      double total     = 0;
      for ( nn=0 ; nn<env->n_grains ; nn++ )
      {
         eh_dbl_grid_rotate( deposit_grid[nn] , shore_angle-M_PI_2 , i_0 , j_0 , &mass_lost );
         total += mass_lost;
      }
   }

   eh_grid_destroy( plume_grid , TRUE );

   return 0;
}   // end of PlumeOut3

