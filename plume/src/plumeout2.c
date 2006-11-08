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

int plumeout2( Plume_enviro *env , Plume_grid *grid , double dx, double **deposit, int deposit_len, int n_grains, double basin_width)
{
   double *deposit_thickness, *deposit_int;
   double *deposit_x;
   double mass_in, mass_out;
   int   ii, jj, nn;
   Plume_river river = *(env->river);
   Plume_sediment *sedload = env->sed;

   deposit_thickness = eh_new( double , grid->lx );
   deposit_int       = eh_new( double , deposit_len );
   deposit_x         = eh_new( double , deposit_len );

   //---
   // initialize the x-coordinate for each deposit location.  this is where
   // where we want to calculate the sedimentation rates.  we will interpolate
   // plume's grid to this one.
   //---
   for ( ii=0 ; ii<deposit_len ; ii++ )
      deposit_x[ii] = ii*dx;
   
   //---
   // here we average the 2d plume grid row by row so that we obtain an
   // average sedimentation rate that we use as the centerline average.
   // we then interpolate this 1d array to the points that are specified
   // as input (a constant spacing of dx).
   //
   // we also calculate the sediment mass that was input (for each grain
   // size) and the mass the is output.  if there is a difference, we scale
   // the output mass to assure that mass is balanced.  we assume that any
   // discrepancy is a result of small numerical errors.
   //---
   for ( nn=0 ; nn<env->n_grains ; nn++ )
   {
      mass_in = river.Cs[nn]*river.Q*dTOs;

      // Determine the centerline average for each grain size
      for( ii=0 ; ii<grid->lx ; ii++ ) 
      {
         deposit_thickness[ii] = 0.0;
         for( jj=0 ; jj<grid->ly ; jj++ )
            deposit_thickness[ii] += grid->deps[ii][jj][nn];
         deposit_thickness[ii] /= grid->ly;
      }

      // Interpolate to the requested grid.
      interpolate( grid->xval ,
                   deposit_thickness ,
                   grid->lx ,
                   deposit_x ,
                   deposit_int ,
                   deposit_len );

      for( ii=0, mass_out=0 ; ii<deposit_len ; ii++ ) 
         if ( !isnan(deposit_int[ii]) )
            mass_out += deposit_int[ii]*sedload[nn].rho;
         else
            deposit_int[ii] = 0.;
      mass_out *= basin_width*dx;

      // Need to swap these arrays for sedflux.  While we are it, scale the
      // interpolated deposit for mass balance.
      if ( mass_out > 0 )
         for (ii=0;ii<deposit_len;ii++)
            deposit[ii][nn] = deposit_int[ii]*(mass_in/mass_out);
      else
         eh_require_not_reached();
   }

   eh_free( deposit_x );
   eh_free( deposit_thickness );
   eh_free( deposit_int );

   return 0;
}   // end of PlumeOut2

