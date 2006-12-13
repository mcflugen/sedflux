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
 * PlumeConc     Calculate the plume concentrations for PLUME
 *
 *     Author:          M.D. Morehead
 *     Original:     May 1998
 *
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <math.h>
#include "utils.h"

int plumeconc(Plume_enviro *env , Plume_grid *grid , Plume_options *opt )
{
   int ii, jj, nn;
   double aa, bb, ucor, *ccor, plugwidth;
   double dl, utest, uu, XX, v1, v2;
   Plume_river river = *(env->river);
   Plume_ocean ocean = *(env->ocean);
   Plume_sediment *sedload = env->sed;
//   double erfc(double);

//---     
// Set the minimum velocity cutoff used in the following calculations
// The value chosen reduces mass balance errors at the edges of the plume
//---
   utest  = 0.05*river.u0;
   dl     = 0.5*(double)(grid->dx+grid->dy); // Average grid length

// Allocate memory for the concentration correction 
   if( (ccor = (double *) calloc(env->n_grains,sizeof(double))) == NULL )
   {
       fprintf(stderr," PlumeArray ERROR: memory allocation failed \n");
       fprintf(stderr,"                failed on xcl, ycl, dcl \n");
       eh_exit(1);
   }

   for ( nn=0 ; nn<env->n_grains ; nn++ )
   {

      if ( river.Cs[nn] > .001 )
      {

         for( ii=0 ; ii<grid->lx ; ii++ )
         {

#ifdef DBG
            fprintf( stderr ,
                     "\r  PlumeConc  ii = %d, lx = %d, ly = %d, nn = %d, ngrains = %d    " , 
                     ii+1 , grid->lx , grid->ly , nn+1 , env->n_grains );
#endif
   
            //---
            // Mass and Velocity Corrections for flow out of the edges
            // of a Fjord
            //---
            if( opt->fjrd && grid->xval[ii] > 0 )
            {
               aa = sqrt( river.b0/(sqpi*C1*grid->xval[ii]) );
               bb = 1/(sqtwo*C1*grid->xval[ii]);
   
               ucor = 2.*river.u0*aa*aa*erfc(bb*0.5*(grid->ymax-grid->ymin))/((grid->ymax-grid->ymin)*bb);
               ccor[nn] = 2.*river.Cs[nn]*aa*aa*erfc(bb*0.5*(grid->ymax-grid->ymin))/((grid->ymax-grid->ymin)*bb);
            }
            else
            {
               ucor     = 0;
               ccor[nn] = 0;
            }
   
            for ( jj=0 ; jj<grid->ly ; jj++ )
            {
               // 'zone of flow establishment'
               if( grid->dist[ii][jj][2] < plg*river.b0 )
               {
   
                  plugwidth  = -grid->dist[ii][jj][2]/(2.*plg) + river.b0/2.;
   
                  if( grid->dist[ii][jj][3] < plugwidth )
                  {
                     grid->ccnc[ii][jj][nn] = river.Cs[nn]+ccor[nn];
                     grid->ualb[ii][jj]     = river.u0+ucor;
                  }
                  else
                  {
                     v1 = grid->dist[ii][jj][3]
                        + 0.5*sqpi*C1*grid->dist[ii][jj][2]
                        - river.b0/2.;
                     v2 = mx( (sqtwo*C1*grid->dist[ii][jj][2]), 0.01);
   
                     grid->ccnc[ii][jj][nn] = river.Cs[nn]*exp(-sq(v1/v2))+ccor[nn];
                     grid->ualb[ii][jj]     = river.u0*exp(-sq(v1/v2))+ucor;
                  }
               }
               else // 'zone of established flow'
               {
                  v1 = river.b0/(sqpi*C1*grid->dist[ii][jj][2]);
                  v2 = grid->dist[ii][jj][3]/(sqtwo*C1*grid->dist[ii][jj][2]);
   
                  grid->ccnc[ii][jj][nn] = river.Cs[nn]*sqrt(v1)*exp(-sq(v2))
                                         + ccor[nn];
                  grid->ualb[ii][jj] = river.u0*sqrt(v1)*exp(-sq(v2))+ucor;
               }
   
               // scale surface concentration by: t = x/u
               if ( opt->fjrd )
                  uu = (river.u0 + grid->dist[ii][jj][4] + 7.*grid->ualb[ii][jj])/9.;
               else
                  uu = (river.u0 + grid->dist[ii][jj][4] + 3.*grid->ualb[ii][jj])/5.;
               XX = sqrt(sq(grid->dist[ii][jj][2]) + sq(grid->dist[ii][jj][3]));
   
               // Calculate Non-conservative Concentration
               if ( grid->ualb[ii][jj] > utest )
                  grid->ncnc[ii][jj][nn] = grid->ccnc[ii][jj][nn]
                                         * exp(-sedload[nn].lambda*XX/uu)
                                         + ocean.Cw;
               else
                  grid->ncnc[ii][jj][nn] = ocean.Cw;
   
               //---
               // Calculate Deposit Thickness, scale time by local u and local x
               //  C do 1/rho => kg/m^3 m m^3/kg => m (~/dt)
               //  Vol/Area = do
               //  dt/day => dTOs*u/l  => (#ofs/day)*1/(#ofs/dt) => dt/day
               //  m/dt * dt/day => m (~/day)
               //---
               if (    grid->ncnc[ii][jj][nn] > ocean.Cw
                    && grid->ualb[ii][jj] > utest )
               {
                  grid->deps[ii][jj][nn] = grid->ncnc[ii][jj][nn] 
                               *( exp(sedload[nn].lambda*dl/grid->ualb[ii][jj]) - 1. )
                               *( river.d0*dTOs*grid->ualb[ii][jj] )
                               /( sedload[nn].rho*dl );
               }
               else
                  grid->deps[ii][jj][nn] = 0.0;
   
               // Calculate the concentration of a conservative tracer (like Salinity)
               if( opt->o1 )
                  grid->sln[ii][jj] = (ocean.Sw-ocean.So)
                                    * (1-grid->ccnc[ii][jj][0]/(river.Cs[0]-ocean.Cw))
                                    + ocean.So;
            }
         }
      }
   }

   free(ccor);

#ifdef DBG
   fprintf(stderr,"\n");
#endif

   return 0;

} // end of PlumeConc

/* 
 * estimate sediment/velocity lost out the edges of fjord runs
 * 
 * let     x = local distance from the river mouth
 *      ym = 1/2 width of the fjord (assumes river is at the center)
 * 
 * define     a = sqrt(bo/(sqrt(pi)*C1*x))
 *      b = 1/(sqrt(2)*C1*x)
 * 
 * then     u = uc*a*(2/sqrt(pi))*exp(-(b*y)^2)
 *      
 * with     uc = uo*a*exp(-(b*0)^2)          local centerline velocity
 * 
 * with     erfc(ym) = 1-erf(ym) = 2/sqrt(pi) Integral(ym->inf)[*exp(-y^2)];
 * 
 * then     erfc(u(ym)) = (1/b)*uo*a^2*erfc(b*ym)
 * 
 * then multiply by 2 (account for +- sides) and scale to width and
 * grid spacing dy/(ymax-ymin)
 * 
 */
