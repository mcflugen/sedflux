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
 * PlumeArray	Creates various arrays for PLUME
 *
 *	Author:		M.D. Morehead
 *	Original:	April 1998
 *
 *
 *	Define:
 *		Li   = Inertial distance, u/f
 *		zend = distance along the centerline at which the inventory
 *			reaches pcrit for the smallest lambda
 *		yend = y position of the deformed plume when it reaches zend
 *		xend = x position of the deformed plume when it reaches zend
 *		(min) indicates in opposite direction of vo
 *		(max) indicates in the direction of vo
 *
 * Find optimal array size for the following cases
 *
 *	1) Fjord (fjrd) or non-deflected (strt) run,
 *		indicated by vo = 0
 *		xmin == 0
 *		xmax = 1.1*zend
 *		ymin, ymax  are user specified
 *	2) Ocean plume, with downwelling
 *		indicated by vo ~= 0, kwf = 1
 *		xmin == 0
 *		xmax = Li
 *		y(min) = -2*bo		
 *		y(max) = zend
 *	3) Ocean plume, with upwelling, strong vo
 *		indicated by vo ~= 0, kwf = 0, yend > 1.5*xend
 *		xmin == 0
 *		xmax = xend + Li
 *		y(min) = -4*bo
 *		y(max) = yend
 *	4) Ocean plume, with upwelling, weak vo
 *		indicated by vo ~= 0, kwf = 0, xend > 1.5*yend
 *		xmin == 0
 *		xmax = xend
 *		y(min) = -Li
 *		y(max) = yend
 *	5) Ocean plume, with upwelling, moderate vo
 *		indicated by vo ~= 0, kwf = 0, xend ~ yend
 *		xmin == 0
 *		xmax = xend
 *		y(min) = -li
 *		y(max) = yend
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <stdlib.h>
#include <utils/utils.h>

//---
// outputs:
//    xmin
//    xmax
//    ymin
//    ymax
// inputs:
//    river.Cs
//    river.b0
//    river.u0
//    sedload.lambda
//    n_grains
//    fjrd
//    strt
//    ocean.v0
//---

//---
// Start of PlumeArray
//---
int plumearray( Plume_grid *grid , Plume_enviro *env , Plume_options *opt )
{
   static int reuse_count = 0;
   static int new_count = 0;
   int	ii, jj, kk, l1, endi;
   int x_size, y_size;
   double aa, AA, avo, mxcs, mnlm, Li, nv, tst, xend, yend, zend;
   double *dcl, *xcl, *ycl;
   int n_grains = env->n_grains;
   Plume_river *river = env->river;
   Plume_ocean *ocean = env->ocean;
   Plume_sediment *sedload = env->sed;

   grid->xmin	= 0;

   // find min's and max's
   mxcs = river->Cs[0];
   mnlm = sedload[0].lambda;
   for( ii=0; ii<n_grains; ii++ )
   {
      mxcs = mx( mxcs, river->Cs[ii]);
      mnlm = mn( mnlm, sedload[ii].lambda);
   }   

//---
// Find the along centerline distance to the cutoff concentration (zend)
// use: I = Io sqrt(-lt)
// where:
//   I = pcrit*Io
//   l = min(lambda)
//   t = x/u
//   u = albertson = uo*sqrt(bo/(sqrt(pi)*C1*x))
//---
   zend = 1.2*pow((-(river->u0/mnlm)*sqrt(river->b0/(sqpi*C1))*log(pcrit) ),(2.0/3.0));

//---
// We don't need to track sediment if it leaves the model (travels a distance
// greater than max_len).
//---
   if ( zend > grid->max_len && grid->max_len != 0 )
      zend = grid->max_len;

   if ( !(opt->fjrd || opt->strt) )
      Li = river->u0/(2*omega*sin(env->lat*degTOr));

   if ( opt->fjrd || opt->strt )
   { /* (1) Fjord run or straight centerline */
      grid->xmax = rnd(1.1*zend);
   }
   else if( opt->kwf == 1 )
   { /* (2) Ocean plume, with downwelling */
      grid->xmax = rnd(Li);
      if( ocean->vo > 0 )
      {
         grid->ymin = floor(-2*river->b0);
         grid->ymax = ceil(zend);
      }
      else
      {
         grid->ymin = floor(-zend);
         grid->ymax = ceil(2*river->b0);
      }
   }
   else
   { /* (3) - (5) Upwelling Conditions */

      // Deflected Jet centerline constants
      avo = fabs(ocean->vo);
      aa  = river->u0/avo;		// a = Wmo/Ue
      nv  = 0.37;			// n = [ 0.375 0.380 0.370 0.385 ]
      AA  = 1.53 + 0.90*aa;

      // Allocate centerline arrays
      l1	= (int)rnd(1.1*zend/grid->dy);
      if( (xcl = (double *) calloc(l1,sizeof(double))) == NULL ||
          (ycl = (double *) calloc(l1,sizeof(double))) == NULL ||
          (dcl = (double *) calloc(l1,sizeof(double))) == NULL    )
      {
          fprintf(stderr," PlumeArray ERROR: memory allocation failed \n");
          fprintf(stderr,"                failed on xcl, ycl, dcl \n");
          return 1;
      }

      // Find the jet position and distance along the jet
      xcl[0] = ycl[0] = dcl[0] = 0.0;
      for ( jj = 1; jj<l1; jj++ )
      {
         ycl[jj] = grid->dy*(jj-1);
         xcl[jj] = river->b0*AA*pow((ycl[jj]/river->b0),nv);
         dcl[jj] = dcl[jj-1]
                 + sqrt(pow((ycl[jj]-ycl[jj-1]),2)+pow((xcl[jj]-xcl[jj-1]),2));
      }

      // find the x,y values of zend 
      // numerically minimize the quadratic equation z^2 - x^2 - y^2 = 0
      endi = 0;
      tst  = fabs(dcl[0]-zend);
      for( jj=1; jj<l1; jj++ )
      {
         if( fabs(dcl[jj]-zend) < tst )
         {
            endi = jj;
            tst = fabs(dcl[jj]-zend);
         }
      } 
      xend = xcl[endi]; yend = ycl[endi];

      // free up local allocated memory
      free(xcl);
      free(ycl);
      free(dcl);
    
      if( yend > 1.5*xend )
      { /* (3) Ocean plume, with upwelling, strong vo indicated by vo~=0, kwf=0, yend>1.5*xend */
         grid->xmax = rnd(xend+Li);
         if( ocean->vo > 0 )
         {
            grid->ymin = rnd(-4*river->b0);
            grid->ymax = rnd(yend);
         }
         else
         {
            grid->ymin = rnd(-yend);
            grid->ymax = rnd(4*river->b0);
         }
      }
      else if( xend > 1.5*yend )
      { /* (4) Ocean plume, with upwelling, weak vo indicated by vo~=0, kwf=0, xend>1.5*yend */
         grid->xmax = rnd(xend);
         if ( ocean->vo > 0 )
         {
            grid->ymin = rnd(-Li);
            grid->ymax = rnd(yend);
         }
          else
         {
            grid->ymin = rnd(-yend);
            grid->ymax = rnd(Li);
         }
      }
      else
      { /* (5) Ocean plume, with upwelling, moderate vo indicated by vo~=0, kwf=0, xend ~ yend */
         grid->xmax = xend;
         if ( ocean->vo > 0 )
         {
            grid->ymin = rnd(-Li);
            grid->ymax = rnd(yend);
         }
         else
         {
            grid->ymin = rnd(-yend);
            grid->ymax = rnd(Li);
         }
      } // end 3)
   } // end of Upwelling Conditons

   eh_make_note( "Increased grid size!" );
   if ( fabs(grid->ymin) > grid->ymax ) grid->ymax =  fabs(grid->ymin);
   if ( fabs(grid->ymin) < grid->ymax ) grid->ymin = -fabs(grid->ymax);

   //---
   // Create nicely centered X/Y arrays
   // insure that yvals=0 at an integer index
   //---
   grid->ymin = rnd(floor(grid->ymin/(int)(grid->dy))*(int)(grid->dy));
   grid->lx = (int)(((grid->xmax-grid->xmin)/(int)(grid->dx))+2);
   grid->ly = (int)(((grid->ymax-grid->ymin)/(int)(grid->dy))+2);
   grid->lz = n_grains;
/*
   if( (xval = (double *) calloc(grid->lx,sizeof(double))) == NULL ||
       (yval = (double *) calloc(grid->ly,sizeof(double))) == NULL )
   {
       fprintf(stderr," PlumeArray ERROR: memory allocation failed \n");
       fprintf(stderr,"                failed on xvals or yvals \n");
       eh_exit(1);
   }
   for( ii = 0 ; ii<grid->lx ; ii++ )
      xval[ii] = grid->xmin+ii*grid->dx;
   for( ii = 0 ; ii<grid->ly ; ii++ )
      yval[ii] = grid->ymin+ii*grid->dy;
*/
   // Recalculate array limits, sizes, and zero points
//   grid->xmin = xval[0];
//   grid->xmax = xval[grid->lx-1];
   grid->xmax = grid->xmin + (grid->lx-1)*grid->dx;
//   grid->ymax = yval[grid->ly-1];
   grid->ymax = grid->ymin + (grid->ly-1)*grid->dy;
   grid->zx   = 0;
   grid->zy   = (int)((grid->ly - grid->ymax/(int)(grid->dy))-1);
   grid->lpc  = (int)(2.4*(grid->lx+grid->ly));

// Increase lpc.  Sometimes it is not large enough to store the entrie grid.
   grid->lpc *= 10;

   if ( grid->lx>grid->x_len || grid->ly>grid->y_len || TRUE )
   {

new_count++;

      if ( grid->x_len!=0 || grid->y_len!=0 )
      {
         eh_free( grid->xval );
         eh_free( grid->yval );

         free_d3tensor( grid->ccnc  );
         free_d3tensor( grid->ncnc  );
         free_d3tensor( grid->deps  );
         free_d3tensor( grid->dist  );
         free_dmatrix ( grid->ualb  );
         free_dmatrix ( grid->pcent );
      }

      if ( grid->lx>grid->x_len )
         grid->x_len = grid->lx;
      if ( grid->ly>grid->y_len )
         grid->y_len = grid->ly;

      grid->xval = eh_new( double , grid->lx );
      grid->yval = eh_new( double , grid->ly );

      for ( ii = 0 ; ii<grid->lx ; ii++ )
         grid->xval[ii] = grid->xmin+ii*grid->dx;
      for ( ii = 0 ; ii<grid->ly ; ii++ )
         grid->yval[ii] = grid->ymin+ii*grid->dy;

      y_size = grid->ly;
      if ( y_size<grid->y_len )
         y_size = grid->y_len;
      x_size = grid->lx;
      if ( x_size<grid->x_len )
         x_size = grid->x_len;

      // Create remaining arrays (i,j = cross-shore, along-shore)
      // ccnc  : Conservative Cs (kg/m^3)
      // ncnc  : Non-conservative Cs (kg/m^3)
      // deps  : Deposit thickness (kg/m^3)
      // dist  : Information relating to closest centerline point
      // ualb  : Albertson velocities (m/s)
      // pcent : Deflected centerline information
      grid->ccnc  = new_d3tensor(x_size,y_size,n_grains);
      grid->ncnc  = new_d3tensor(x_size,y_size,n_grains);
      grid->deps  = new_d3tensor(x_size,y_size,n_grains);
      grid->dist  = new_d3tensor(x_size,y_size,5);
      grid->ualb  = new_dmatrix(x_size,y_size);
      grid->pcent = new_dmatrix(grid->lpc,4);

      if( opt->o1 )
      {
         // sln  : Conservative tracer concentration
         grid->sln = new_dmatrix(x_size,y_size);
      }
   }
   else
   {
      reuse_count++;
   }

   for ( ii=0 ; ii<grid->x_len ; ii++ )
      for ( jj=0 ; jj<grid->y_len ; jj++ )
      {
         for ( kk=0 ; kk<n_grains ; kk++ )
         {
            grid->ccnc[ii][jj][kk] = 0.;
            grid->ncnc[ii][jj][kk] = 0.;
            grid->deps[ii][jj][kk] = 0.;
         }
         for ( kk=0 ; kk<5 ; kk++ )
            grid->dist[ii][jj][kk] = 0.;
         grid->ualb[ii][jj] = 0.;
      }
   for ( ii=0 ; ii<grid->lpc ; ii++ )
      for ( jj=0 ; jj<4 ; jj++ )
         grid->pcent[ii][jj] = 0.;

   return 0; 
 
} // end of PlumeArray

