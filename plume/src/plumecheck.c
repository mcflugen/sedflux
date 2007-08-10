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
 * PlumeCheck	Checks the ranges of the input values of PLUME
 *
 *	Author:		M.D. Morehead
 *	Original:	April 1998
 *	Modified:	Sep 1998, MDM, Conversion for SEDFLUX3D
 *
 */
#include "plumeinput.h"
#include "plumevars.h"

//---
// outputs:
//    O1, O2, O3
// inputs:
//    river.Q
//    river.u0
//    river.b0
//    river.rma
//    river.Cs
//    n_grains
//    sedload.lambda
//    sedload.rho
//    ocean.Cw
//    ocean.v0
//    ocean.cc
//    kwf
//    lat
//    dy
//    dx
//    fjrd
//    strt
//    ymin
//    ymax
//    ndy
//    ndx
//    O1, O2, O3
//---
/*
static gchar* err_msg[] = {
   [PLUME_ERROR_LOW_Q]    = "River discharge is too low" ,
   [PLUME_ERROR_HIGH_Q]   = "River discharge is too high" ,
   [PLUME_ERROR_LOW_VEL]  = "River velocity is too low" ,
   [PLUME_ERROR_HIGH_VEL] = "River velocity is too high" ,
};
*/

int plumecheck( Plume_enviro *env , Plume_grid *grid , Plume_options *opt )
{
   int err, ii;
   double mncs, mxcs, mnlm, mxlm, mnrs, mxrs;
   Plume_river *river = env->river;
   Plume_ocean *ocean = env->ocean;
   Plume_sediment *sedload = env->sed;

   err = 0;
/*
   if ( river->Q<1.    ) error = PLUME_ERROR_LOW_Q;
   if ( river->Q>1e6   ) error = PLUME_ERROR_HIGH_Q;
   if ( river->u0<.01  ) error = PLUME_ERROR_LOW_VEL;
   if ( river->u0>10   ) error = PLUME_ERROR_HIGH_VEL;
   if ( river->b0<1    ) error = PLUME_ERROR_LOW_WIDTH;
   if ( river->b0>1e5  ) error = PLUME_ERROR_HIGH_WIDTH;
   if ( river->rma>80  ) error = PLUME_ERROR_LOW_ANGLE;
   if ( river->rma<-80 ) error = PLUME_ERROR_HIGH_ANGLE;
*/
   // Check river parameters
   if( river->Q < 1.0 || river->Q > 1e6 )
   {
      fprintf( stderr," \n ERROR in PLUME input parameters (plumecheck.c) \n");
      fprintf( stderr,"   The river discharge is out of range \n");
      fprintf( stderr,"   river.Q = %g, qmin = 1.0, qmax = 1e6 (m^3/s) \n" ,
               river->Q );
/*
      eh_strv_append( &err_s , g_strdup_printf( "Plume input parameter error: River discharge out of range." ) );
      eh_strv_append( &err_s , g_strdup_printf( "River discharge = %g ()." , river->Q ) );
*/
      err = 1;
   }
   
   if( river->u0 < 0.01 || river->u0 > 10 )
   {
      fprintf( stderr," \n ERROR in PLUME input parameters (plumecheck.c) \n");
      fprintf( stderr,"   The river velocity is out of range \n");
      fprintf( stderr,"   river.u0 = %g, umin = 0.01, umax = 10 (m/s) \n" ,
               river->u0 );
      err = 1;
   }
   
   if( river->b0 < 1.0 || river->b0 > 1e5 )
   {
      fprintf( stderr," \n ERROR in PLUME input parameters (plumecheck.c) \n");
      fprintf( stderr,"   The river width is out of range \n");
      fprintf( stderr,"   river.b0 = %g, bmin = 1.0, bmax = 1e5 (m) \n" ,
               river->b0);
      err = 1;
   }
   
   if( fabs(river->rma) > 80 )
   {
      fprintf( stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf( stderr,"   The river mouth angle is out of range \n");
      fprintf( stderr,"   rma = %g \n", river->rma);
      fprintf( stderr,"   Allowed range: -80 < rma < 80 (degrees) \n");
      err = 1;
   }

   // Sediment Load

   // find min's and max's
   mxcs = mncs = river->Cs[0];
   mxlm = mnlm = sedload[0].lambda*dTOs;
   mxrs = mnrs = sedload[0].rho;
   for( ii=0 ; ii<env->n_grains ; ii++ ) 
   {
      mncs = mn( mncs, river->Cs[ii]);
      mxcs = mx( mxcs, river->Cs[ii]);
      mnlm = mn( mnlm, sedload[ii].lambda*dTOs);
      mxlm = mx( mxlm, sedload[ii].lambda*dTOs);
      mnrs = mn( mnrs, sedload[ii].rho);
      mxrs = mx( mxrs, sedload[ii].rho);
   }   

/*
   if( (int)ngrains != ngrains )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The Number of Grain Sizes simulated must be an integer \n");
      fprintf(stderr,"   ngrains = %d \n", ngrains);
      err = 1;
   }
*/
   
   if( mncs < 0.001 || mxcs > 500. )
   {
   /*
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The river concentration is out of range \n");
      fprintf(stderr,"   min(Cs) = %g, max(Cs) = %g \n", mncs, mxcs);
      fprintf(stderr,"   Allowed Range: min(Cs) = 0.001, max(Cs) = 100 (kg/m^3) \n");
   */
      err = 1;
   }
   
   if( mnlm < 0.1 || mxlm > 40 ) 
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The removal rate coefficients are out of range \n");
      fprintf(stderr,"   min(lambda) = %g, max(lambda) = %g \n", mnlm, mxlm);
      fprintf(stderr,"   Allowed Range = 0.1 to 40 (1/day) \n");
      err = 1;
   }
   
   if( mnrs < 1100 || mxrs > 2600 ) 
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The sediment densities are out of range \n");
      fprintf(stderr,"   min(rho) = %g, max(rho) = %g \n", mnrs, mxrs);
      fprintf(stderr,"   Allowed Range = 1100 to 2600 (kg/m^3) \n");
      err = 1;
   }

   // Check Ocean parameters

   if( ocean->Cw != 0 )
   {
      fprintf(stderr," \n WARNING in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   Plume edge effects may occur if ocean.Cw != 0, \n");
      fprintf(stderr,"   especially for low Cs, high lambda grain sizes. \n");
      fprintf(stderr,"   Cw = %g \n ", ocean->Cw);
      fprintf(stderr,"   Allowed Range: min(Cw) = 0, max(Cw) = min(Cs) = %g (kg/m^3) \n", mncs);
   }
   
   if( ocean->Cw < 0 || ocean->Cw > mncs )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The ocean concentration is out of range \n");
      fprintf(stderr,"   Cw = %g \n ", ocean->Cw);
      fprintf(stderr,"   Allowed Range: min(Cw) = 0, max(Cw) = min(Cs) = %g (kg/m^3) \n", mncs);
      err = 1;
   }
   
   if( fabs(ocean->vo) > 3.0 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The alongshore current is out of range \n");
      fprintf(stderr,"   vo = %g \n", ocean->vo);
      fprintf(stderr,"   Allowed range: -3.0 < vo < 3.0 (m/s) \n");
      err = 1;
   }
   
   if( 0.1 > ocean->cc || ocean->cc > 1 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The coastal current width multiplier is out of range \n");
      fprintf(stderr,"   ocean.cc = %g \n", ocean->cc);
      fprintf(stderr,"   Allowed range: 0.1 < cc < 1 (percent of Inertial Length Scale) \n");
      err = 1;
   }
   
   if( opt->kwf != 1 && opt->kwf != 0 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The Kelvin Wave Flag is out of range \n");
      fprintf(stderr,"   kwf = %d \n", opt->kwf);
      fprintf(stderr,"   kwf must equal 0 or 1");
      err = 1;
   }
   
   if( 0 > env->lat || env->lat > 90 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The latitude is out of range \n");
      fprintf(stderr,"   lat = %d \n", env->lat );
      fprintf(stderr,"   Allowed range: 0 < lat < 90 (degrees) \n");
      fprintf(stderr,"   Not tested for Southern Hemisphere yet \n");
      err = 1;
   }

   /* 
    *	Check Conservative Concentration Parameters
    *		no clear checks to be made
    *		we want this to be general
    *		ie.	Salinity (ppt): So = 0, Sw = 32
    *			deltaO18:	???
    */

   #if !defined(SEDFLUX3D) && !defined(SEDFLUX2D)
   // fprintf(stderr," PlumeCheck Warning: No checks made on So or Sw at present time \n");
   #endif

//   if ( finite(So) == 0 || finite(Sw) == 0 )
//   {
//      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
//      fprintf(stderr,"   The conservative concentration values are not finite \n");
//      fprintf(stderr,"   So and Sw must be present in the input file. \n");
//      err = 1;
//   }

   // Check grid parameters
   if( fmod(river->b0,grid->dy) || fmod(river->b0/grid->dy,2) == 0. )
   {
      fprintf( stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf( stderr,"The river width (bo) should be an odd integer multiple of the grid size (dy) \n");
      fprintf( stderr ,
               "bo = %g, dy = %f, ndy = %d \n" ,
               river->b0 ,
               grid->dy ,
               grid->ndy );
      err = 1;
   }
   
   if( (opt->fjrd || opt->strt) && grid->ymin > -2*river->b0 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The ymin grid point is too close to the river mouth \n");
      fprintf(stderr,"   ymin = %f, bo = %g \n", grid->ymin, river->b0);
      fprintf(stderr,"   Allowed range: ymin < -2*bo (m) \n");
      err = 1;
   }
   
   if( (opt->fjrd || opt->strt) && grid->ymax < 2*river->b0 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The ymax grid point is too close to the river mouth \n");
      fprintf(stderr,"   ymax = %f, bo = %g \n", grid->ymax, river->b0);
      fprintf(stderr,"   Allowed range: ymax > 2*bo (m) \n");
      err = 1;
   }
   
   if(    grid->ndy < 3    || grid->ndy > 21
       || grid->ndy%2 == 0 || grid->ndy%1 != 0 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The number of y grid points is incorrect \n");
      fprintf(stderr,"   ndy = %d \n", grid->ndy);
      fprintf(stderr,"   Allowed range: ndy must be an odd integer and 0 < ndy < 22 \n");
      err = 1;
   }
   
   if( !(opt->fjrd || opt->strt) && grid->ndx != 1)
   {
/*
      fprintf(stderr," \n PlumeCheck: Warning \n");
      fprintf(stderr,"  Curved centerlines must have ndx == 1 \n");
      fprintf(stderr,"  ndx reset to 1 \n");
*/
      grid->ndx = 1;
      grid->dx = grid->dy;
   }
   
   if( grid->dx <= 0 || grid->ndx%1 != 0 )
   {
      fprintf(stderr," \n ERROR in PLUME input parameters (PlumeCheck.c) \n");
      fprintf(stderr,"   The x grid spacing (dx) is out of range \n");
      fprintf(stderr,"   dx = %f, ndx = %d \n", grid->dx, grid->ndx);
      fprintf(stderr,"   Allowed Range: ndx must be an integer and dx > 0 \n");
      err = 1;
   }
   
   if( opt->o1 != 1 && opt->o1 != 0 )
   {
      fprintf(stderr," \n PlumeCheck.c: Warning  \n");
      fprintf(stderr,"   The standalone output Flag is out of range \n");
      fprintf(stderr,"   o1 = %d \n", opt->o1);
      fprintf(stderr,"   o1 must equal 0 or 1 \n");
      fprintf(stderr,"   defaulting to 1 \n");
      opt->o1 = 1;
   }
   
   if( opt->o2 != 1 && opt->o2 != 0 )
   {
      fprintf(stderr," \n PlumeCheck.c: Warning  \n");
      fprintf(stderr,"   The SEDFLUX-2D output Flag is out of range \n");
      fprintf(stderr,"   o2 = %d \n", opt->o2);
      fprintf(stderr,"   o2 must equal 0 or 1 \n");
      fprintf(stderr,"   defaulting to 0 \n");
      opt->o2 = 0;
   }
   
   if( opt->o3 != 1 && opt->o3 != 0 )
   {
      fprintf(stderr," \n PlumeCheck.c: Warning  \n");
      fprintf(stderr,"   The SEDFLUX-3D output Flag is out of range \n");
      fprintf(stderr,"   o3 = %d \n", opt->o3);
      fprintf(stderr,"   o3 must equal 0 or 1 \n");
      fprintf(stderr,"   defaulting to 0 \n");
      opt->o3 = 0;
   }
   
   if( opt->o1 == 0 && opt->o2 == 0 && opt->o3 == 0)
   {
      fprintf(stderr," \n PlumeCheck.c: Warning  \n");
      fprintf(stderr,"   All of the output flags are set to 0 (ie no output) \n");
      fprintf(stderr,"   defaulting standalone output (o1) to 1 \n");
      opt->o1 = 1;
   }

   return(err);
   
} // end of PlumeCheck.c

