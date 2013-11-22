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

/***********************************************************************
*                                                                      *
* Author:                                                              *
*                                                                      *
*  Eric Hutton                                                         *
*  Institute of Arctic and Alpine Research                             *
*  University of Colorado at Boulder                                   *
*  Boulder, CO                                                         *
*  80309-0450                                                          *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Version :                                                            *
*                                                                      *
*  0.9 - April 1999                                                    *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Function :                                                           *
*                                                                      *
*  BING                                                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Description :                                                        *
*                                                                      *
*  Bingham-plastic debris flow model.                                  *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Formal parameters :                                                  *
*                                                                      *
*  bathy              - Bathymetry data (in meters) of the seafloor    *
*                       over which the debris flow is to be run.       *
*  n_nodes            - The number of nodes in the bathymetry data.    *
*  nFlowNodes         - The number of nodes in the debris flow.        *
*  dx                 - The horzontal spacing (in meters) of the nodes *
*                       given in the bathymetry.                       *
*  nodeX              - The positions (in meters) of each of the       *
*                       debris flow nodes.                             *
*  nodeH              - The heights (in meters) of each of the debris  *
*                       flow nodes.                                    *
*  yieldStrength      - The yield strength of the debris flow (in Pa). *
*  viscosity          - The kinematic viscosity of the debris flow     *
*                       in (m^2/s).                                    *
*  numericalViscosity - Pseudo-viscosity term added to help with       *
*                       numerical stability.                           *
*  flowDensity        - The density of the debris flow (in kg/m^3).    *
*  dt                 - The time step of the model (in seconds).       *
*  maxTime            - The maximum time (in minutes) the debris flow  *
*                       is allowed to run for.                         *
*  MC                 - Interval (in time steps) to write output.      *
*  deposit            - The final height of the debris flow deposit    *
*                       interpolated to the given bathymetry nodes.    *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Global Variables :                                                   *
*                                                                      *
*  NONE                                                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Local Variables :                                                    *
*                                                                      *
*  i          - Loop counter.                                          *
*  j          - Loop counter.                                          *
*  updateTime - The interval (in time steps) to write the updated      *
*               elapsed time of the debris flow.                       *
*  Mcount     - The number of times that data has been written to the  *
*               output file.                                           *
*  Xj         - Index to the node of the input bathymetry to which the *
*               current debris flow node lies nearest.                 *
*  Sj         - Index to the node of the slope data to which the       *
*               current debris flow node lies nearest.                 *
*  Xgrav      - Gravitational force term.                              *
*  Xpres      - Pressure term.                                         *
*  Xresi      - Friction term.                                         *
*  Xyield     - Yield strenght term.                                   *
*  Xmom       - Momentum term.                                         *
*  Xp1        - Plug flow pressure term.                               *
*  Xp2        - Plug flow yield strength term.                         *
*  dU         - Velocity change of a node over a single time step.     *
*  Dp         - Depth (in meters) of the plug flow layer.              *
*  Ds         - Depth (in meters) of the shear layer.                  *
*  art1       - Artificial viscosity term used for numerical           *
*               stability.                                             *
*  Ttime      - The total time (in minutes) the debris flow has been   *
*               running for.                                           *
*  Drho       - The submerged density (in kg/m^3) of the debris flow.  *
*  bathyX     - X-coordinates (in meters) to the bathymetry nodes.     *
*  area       - The area (in m^2) of each of the debris flow nodes.    *
*  slope      - The slope (in grads) at each of the bathymetry nodes.  *
*  finalX     - The final positions (in meters) of each of the debris  *
*               flow nodes.                                            *
*  finalH     - The final thicknesses (in meters) of each of the       *
*               debris flow nodes.                                     *
*  flow       - Structure containing values for each of the debris     *
*               flow nodes.                                            *
*                                                                      *
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glib.h>

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "bing.h"

#ifndef DENSITY_OF_SEA_WATER
# define DENSITY_OF_SEA_WATER sed_rho_sea_water()
#endif
#ifndef GRAVITY
# define GRAVITY sed_gravity()
#endif
#ifndef EPS
# define EPS .0001
#endif

double *bing(pos_t *bathy,pos_t *fail,bing_t consts,double *deposit)
{
#ifdef BING_STANDALONE
   extern FILE *bing_fpout_;
#endif
   int Mcount;
   int i, j, nFlowNodes, nNodes;
   int Xj;
   double yieldStrength, viscosity, numericalViscosity, flowDensity, dt, maxTime;
   int MC;
   double Sj;
   double Xgrav, Xpres, Xresi, Xyield, Xmom, Xp1, Xp2;
   double dU, Dp, Ds, art1;
   double Ttime, Drho;
   double *area, *slope;
   double *finalX, *finalH;
   double *initX, *initY, *finalY;
   node *flow;

   yieldStrength      = consts.yieldStrength;
   viscosity          = consts.viscosity;
   numericalViscosity = consts.numericalViscosity;
   flowDensity        = consts.flowDensity;
   dt                 = consts.dt;
   maxTime            = consts.maxTime;
   MC                 = consts.MC;
   nFlowNodes         = fail->size;
   nNodes             = bathy->size;
   
   //---
   // Allocate memory.
   //---
   flow = eh_new( node   , nFlowNodes );
   area = eh_new( double , nFlowNodes );

   finalX = eh_new( double , nFlowNodes );
   finalH = eh_new( double , nFlowNodes );
   finalY = eh_new( double , nFlowNodes );
   initX  = eh_new( double , nFlowNodes );
   initY  = eh_new( double , nFlowNodes );

   memcpy(initX,fail->x,nFlowNodes*sizeof(double));
   interpolate(bathy->x,bathy->y,bathy->size,fail->x,initY,fail->size);

   for (i=0;i<nFlowNodes;i++)
   {
      flow[i].D = fail->y[i];
      flow[i].X = fail->x[i];
   }

   slope = derivative(*bathy);

   for (i=0;i<nFlowNodes-1;i++)
      area[i] = flow[i].D*(flow[i+1].X-flow[i].X);
   area[nFlowNodes-1] = 0.;

   for (i=0;i<nFlowNodes;i++)
   {
      flow[i].Up    = EPS;
      flow[i].U     = .995*flow[i].Up;
      flow[i].Uold  = flow[i].U;
      flow[i].Upold = flow[i].Up;
   }

   //---
   // Calculation starts
   //---
   Ttime  = 0.0;
   Mcount = 0;
   Drho   = ( flowDensity - DENSITY_OF_SEA_WATER ) / flowDensity;
   while ( Ttime < maxTime && flow[nFlowNodes-1].X < bathy->x[bathy->size-1] ) 
   {
      
      for (i=0;i<nFlowNodes-1;i++) 
      {
         flow[i].Xbar  = ( flow[i+1].X + flow[i].X ) / 2.0;
         flow[i].Ubar  = ( flow[i+1].U + flow[i].U ) / 2.0;
         flow[i].Upbar = ( flow[i+1].Up + flow[i].Up ) / 2.0;
      }

      // Use a weighted average to calculate average height
      for (i=1;i<nFlowNodes-1;i++) 
          flow[i].Dbar = (   flow[i-1].D*(flow[i+1].X-flow[i].X)
                           + flow[i].D  *(flow[i].X-flow[i-1].X) )
                       / ( flow[i+1].X-flow[i-1].X );
      flow[0].Dbar            = 0;
      flow[nFlowNodes-1].Dbar = 0;
      
/**
*** Calculate the forces on the nodes.
*** 
***               1 d  2                      7
***   Xmom   =    - -( -( Up*Up*Dp )+ U*U*D - -( U*Up*D ) )
***               D dx 5                      5
***
***   Xgrav  =    g*Drho*S
***
***                 dD
***   Xpres  =    - -- g*Drho
***                 dx
***   
***                    mum*Up
***   Xresi  =    -2 -----------
***                   rhom*D*Ds
***   
***                  yieldStrength
***   Xyield =    - ---------------
***                     D*rhom
***                    
***                       dUp
***   Xp1    =    -(Up-U) ---
***                       dx
***   
***                  yieldStrength
***   Xp2    =    - ---------------
***                     rhom*Dp
***   
**/

      for (j=0;j<nFlowNodes;j++) 
      {
         // We'll assume an equally spaced grid for now
         double Dx = bathy->x[1]-bathy->x[0];

         //---
         // Find the nearest node in the bathymetry data to determine the
         // slope at a flow node.
         //---
         Xj = floor((flow[j].X-bathy->x[0])/Dx);

         if ( Xj >= nNodes || Xj < 0)
            Sj = 0;
         else
            Sj = slope[Xj];
         
         if ( Xj >= nNodes )
            flow[j].Y = bathy->y[nNodes-1];
         else if ( Xj < 0 )
            flow[j].Y = bathy->y[0];
         else
            flow[j].Y = bathy->y[Xj];
         
         //---
         // Calculate forces on the nodes.  The head and tail nodes are treated
         // differently.  The head and tail nodes use average heights,
         // velocities, etc. to calculate forces.
         //---
         Xgrav = Drho*GRAVITY*Sj;
         if ( j == 0 ) 
         {
            if ( abs(flow[j].Upbar) < EPS )
               Dp = flow[j].D*(3.0*.995-2.0);
            else
               Dp = flow[j].D*(3.*flow[j].Ubar/flow[j].Upbar-2.0);
            Ds = flow[j].D - Dp;
            
            Xyield  = -yieldStrength/flowDensity/flow[j].D*sign(flow[j].Uold);
            Xresi   = -2.0*viscosity*flow[j].Upbar/flow[j].D/Ds;
            Xp2     = -yieldStrength/flowDensity/Dp*sign(flow[j].Uold);

            Xpres   = -Drho*GRAVITY*(flow[j+1].Dbar) / (flow[j+1].X-flow[j].X);

            Xmom    = (   .4*sq(flow[j+1].Upold)*flow[j+1].Dbar
                        + sq(flow[j+1].Uold)*flow[j+1].Dbar
                        - 1.4*flow[j+1].Uold*flow[j+1].Upold*flow[j+1].Dbar )
                    / flow[j].D
                    / (flow[j+1].X-flow[j].X);
            Xp1     = -(flow[j].Upbar - flow[j].Ubar)
                    * (flow[j+1].Upold-flow[j].Upold)
                    / (flow[j+1].X-flow[j].X);
         } 
         else if ( j == nFlowNodes-1 ) 
         {
            if ( abs(flow[j-1].Upbar) < EPS )
               Dp = flow[j-1].D*(3.0*.995-2.0);
            else
               Dp = flow[j-1].D*(3.*flow[j-1].Ubar/flow[j-1].Upbar-2.0);
            Ds = flow[j-1].D - Dp;
            
            Xyield    = -yieldStrength
                      / flowDensity
                      / flow[j-1].D
                      * sign(flow[j].Uold);
            Xresi     = -2.0*viscosity*flow[j-1].Upbar/flow[j-1].D/Ds;
            Xp2       = -yieldStrength/flowDensity/Dp*sign(flow[j-1].Uold);
            Xpres     = -Drho*GRAVITY*(-flow[j-1].Dbar)
                      / (flow[j].X-flow[j-1].X);
            Xmom      = - ( .4*sq(flow[j-1].Upold)*flow[j-1].Dbar
                          + sq(flow[j-1].Uold)*flow[j-1].Dbar
                          - 1.4*flow[j-1].Uold*flow[j-1].Upold*flow[j-1].Dbar)
                      / flow[j-1].D
                      / (flow[j].X-flow[j-1].X);
            Xp1       = - (flow[j-1].Upbar - flow[j-1].Ubar)
                      * (flow[j].Upold-flow[j-1].Upold)
                      / (flow[j].X-flow[j-1].X);
         } 
         else 
         {
            if ( abs(flow[j].Up) < EPS )
               Dp = flow[j].Dbar*(3.0*.995-2.0);
            else
               Dp = flow[j].Dbar*(3.*flow[j].U/flow[j].Up-2.0);
            Ds = flow[j].Dbar - Dp;
            
            Xyield    = -yieldStrength
                      / flowDensity
                      / flow[j].Dbar
                      * sign(flow[j].Uold);
            Xresi     = -2.0*viscosity*flow[j].Upold/flow[j].Dbar/Ds;
            Xp2       = -yieldStrength/flowDensity/Dp*sign(flow[j].Uold);
            Xpres     = -Drho*GRAVITY*(flow[j+1].Dbar-flow[j-1].Dbar)
                      / (flow[j+1].X-flow[j-1].X);
            Xmom      = ( .4*sq(flow[j+1].Upold)*flow[j+1].Dbar
                        + sq(flow[j+1].Uold)*flow[j+1].Dbar
                        - 1.4*flow[j+1].Uold*flow[j+1].Upold*flow[j+1].Dbar)
                      / flow[j].Dbar
                      / (flow[j+1].X-flow[j-1].X)
                      - ( .4*sq(flow[j-1].Upold)*flow[j-1].Dbar
                        + sq(flow[j-1].Uold)*flow[j-1].Dbar
                        - 1.4*flow[j-1].Uold*flow[j-1].Upold*flow[j-1].Dbar)
                      / flow[j].Dbar
                      / (flow[j+1].X-flow[j-1].X);
            Xp1       = -(flow[j].Upold - flow[j].Uold)
                      * (flow[j+1].Upold-flow[j-1].Upold)
                      / (flow[j+1].X-flow[j-1].X);
         }
         
         // Sum forces to find new velocity.
         dU = (Xgrav+Xpres+Xresi+Xyield+Xmom)*dt;

         flow[j].U = flow[j].Uold + dU;
         
         flow[j].Up = flow[j].Upold + (Xgrav+Xp1+Xp2+Xpres)*dt;
         
         if ( j != 0 && j != nFlowNodes-1 )
         {
            art1 = numericalViscosity
                 * abs(flow[j+1].Dbar-2*flow[j].Dbar+flow[j-1].Dbar)
                 / ( abs(flow[j+1].Dbar)
                   + 2*abs(flow[j].Dbar)
                   + abs(flow[j-1].Dbar) );
            flow[j].U = flow[j].U
                      + art1*(flow[j+1].Uold-2*flow[j].Uold+flow[j-1].Uold);
            flow[j].Up = flow[j].Up
                       + art1*(flow[j+1].Upold-2*flow[j].Upold+flow[j-1].Upold);
         }

         if ( flow[j].U/flow[j].Up <= 2./3 ) 
            flow[j].Up=1.499*flow[j].U;
         if ( flow[j].U/flow[j].Up >= 1 ) 
            flow[j].Up = 1.001*flow[j].U;
      }   

#ifdef BING_STANDALONE
      if ( Mcount%updateTime == 0 )
      {
         fprintf(stderr,"\r\t\t\t\tTime = %.1f minutes",Mcount*dt/60.);
         fflush(stderr);
      }

      if ( Mcount%MC == 0 )
      {
         for (i=0;i<nFlowNodes;i++)
         {
            finalX[i] = flow[i].X;
            finalH[i] = flow[i].Dbar;
         }
         interpolate(finalX,finalH,nFlowNodes,bathy->x,deposit,bathy->size);
         fwrite(deposit,nNodes,sizeof(double),bing_fpout_);
      }
#endif
 
      
      // Update node positions
      for (i=0;i<nFlowNodes;i++) 
         flow[i].X = flow[i].X + dt*flow[i].U;

      // Update heights to conserve area
      for (i=0;i<nFlowNodes-1;i++)
      {
         flow[i].D = area[i]/(flow[i+1].X-flow[i].X);
         if ( flow[i].D < 0. ) 
         {
            for (i=0;i<nNodes;i++)
               deposit[i] = -1.;
            g_message( "there was a problem running the debris flow." );

            eh_free(flow);
            eh_free(area);
            eh_free(slope);
            eh_free(finalX);
            eh_free(finalY);
            eh_free(finalH);
            eh_free(initX);
            eh_free(initY);

            return NULL;
         }
      }
      flow[nFlowNodes-1].D = 0.0;

      // Save old values
      for (i=0;i<nFlowNodes;i++) 
      {
         flow[i].Uold  = flow[i].U;
         flow[i].Upold = flow[i].Up;
      }

      Ttime += dt/60.0;
      Mcount++;
   }  

   for (i=0;i<nFlowNodes;i++)
   {
      finalX[i] = flow[i].X;
      finalH[i] = flow[i].Dbar;
   }

   interpolate(bathy->x,bathy->y,bathy->size,finalX,finalY,fail->size);

//   fprintf(stderr,"bing : Debris flow number : %d\n",count++);
//   fprintf(stderr,"bing : Initial length     : %f\n",initX[nFlowNodes-1]-initX[0]);
//   fprintf(stderr,"bing : Final length       : %f\n",finalX[nFlowNodes-1]-finalX[0]);
//   fprintf(stderr,"bing : Run out length     : %f\n",finalX[nFlowNodes-1]-initX[nFlowNodes-1]);
//   fprintf(stderr,"bing : Drop               : %f\n",initY[nFlowNodes-1]-finalY[nFlowNodes-1]);


   interpolate_bad_val( finalX   , finalH , nFlowNodes  ,
                        bathy->x ,deposit , bathy->size , -99. );

   eh_free(flow);
   eh_free(area);
   eh_free(slope);
   eh_free(finalX);
   eh_free(finalY);
   eh_free(finalH);
   eh_free(initX);
   eh_free(initY);

   return deposit;
}

#ifdef WITH_THREADS

typedef struct
{
   node_t *flow;
   int     n_flow_nodes;
   pos_t  *bathy;
   int     n_nodes;
   double  dt;
}
force_t;

void calculate_forces( gpointer data , gpointer user_data )
{
   int j = (int)data;
   node_t *flow = 
   pos_t *bathy =
   int n_nodes = 
   double dt =
   double numerical_viscosity =
   double Dx, dU;
   double Xj, Sj;
   double Dp, Ds;
   double Xgrav, Xpres, Xresi, Xyield, Xmom, Xp1, Xp2;

   Dx = bathy->x[1]-bathy->x[0]; /* We'll assume an equally spaced grid for now */

/**
*** Find the nearest node in the bathymetry data to determine the slope at 
***  a flow node.
**/
   Xj = floor((flow[j].X-bathy->x[0])/Dx);

   if ( Xj >= n_nodes || Xj < 0)
      Sj = 0;
   else
      Sj = slope[Xj];
         
   if ( Xj >= n_nodes )
      flow[j].Y = bathy->y[n_nodes-1];
   else if ( Xj < 0 )
      flow[j].Y = bathy->y[0];
   else
      flow[j].Y = bathy->y[Xj];
         
/**
*** Calculate forces on the nodes.  The head and tail nodes are treated
***  differently.  The head and tail nodes use average heights, velocities,
***  etc. to calculate forces.
**/         
   Xgrav = Drho*GRAVITY*Sj;
   if ( j == 0 ) 
   {
      if ( abs(flow[j].Upbar) < EPS )
         Dp = flow[j].D*(3.0*.995-2.0);
      else
         Dp = flow[j].D*(3.*flow[j].Ubar/flow[j].Upbar-2.0);
      Ds = flow[j].D - Dp;
      
      Xyield  = -yieldStrength/flowDensity/flow[j].D*sign(flow[j].Uold);
      Xresi   = -2.0*viscosity*flow[j].Upbar/flow[j].D/Ds;
      Xp2     = -yieldStrength/flowDensity/Dp*sign(flow[j].Uold);

      Xpres   = -Drho*GRAVITY*(flow[j+1].Dbar) / (flow[j+1].X-flow[j].X);

      Xmom    = (.4*sq(flow[j+1].Upold)*flow[j+1].Dbar+sq(flow[j+1].Uold)*flow[j+1].Dbar
         -1.4*flow[j+1].Uold*flow[j+1].Upold*flow[j+1].Dbar)/flow[j].D/(flow[j+1].X-flow[j].X);
      Xp1     = -(flow[j].Upbar - flow[j].Ubar)*(flow[j+1].Upold-flow[j].Upold) / (flow[j+1].X-flow[j].X);
   } 
   else if ( j == nFlowNodes-1 ) 
   {
      if ( abs(flow[j-1].Upbar) < EPS )
         Dp = flow[j-1].D*(3.0*.995-2.0);
      else
         Dp = flow[j-1].D*(3.*flow[j-1].Ubar/flow[j-1].Upbar-2.0);
      Ds = flow[j-1].D - Dp;
      
      Xyield    = -yieldStrength/flowDensity/flow[j-1].D*sign(flow[j].Uold);
      Xresi     = -2.0*viscosity*flow[j-1].Upbar/flow[j-1].D/Ds;
      Xp2       = -yieldStrength/flowDensity/Dp*sign(flow[j-1].Uold);
      Xpres     = -Drho*GRAVITY*(-flow[j-1].Dbar) / (flow[j].X-flow[j-1].X);
      Xmom      = - (.4*sq(flow[j-1].Upold)*flow[j-1].Dbar+sq(flow[j-1].Uold)*flow[j-1].Dbar
         -1.4*flow[j-1].Uold*flow[j-1].Upold*flow[j-1].Dbar)/flow[j-1].D/(flow[j].X-flow[j-1].X);
      Xp1       = -(flow[j-1].Upbar - flow[j-1].Ubar)*(flow[j].Upold-flow[j-1].Upold) / (flow[j].X-flow[j-1].X);
   } 
   else 
   {
      if ( abs(flow[j].Up) < EPS )
         Dp = flow[j].Dbar*(3.0*.995-2.0);
      else
         Dp = flow[j].Dbar*(3.*flow[j].U/flow[j].Up-2.0);
      Ds = flow[j].Dbar - Dp;
      
      Xyield    = -yieldStrength/flowDensity/flow[j].Dbar*sign(flow[j].Uold);
      Xresi     = -2.0*viscosity*flow[j].Upold/flow[j].Dbar/Ds;
      Xp2       = -yieldStrength/flowDensity/Dp*sign(flow[j].Uold);
      Xpres     = -Drho*GRAVITY*(flow[j+1].Dbar-flow[j-1].Dbar) / (flow[j+1].X-flow[j-1].X);
      Xmom      = (.4*sq(flow[j+1].Upold)*flow[j+1].Dbar+sq(flow[j+1].Uold)*flow[j+1].Dbar
         -1.4*flow[j+1].Uold*flow[j+1].Upold*flow[j+1].Dbar)/flow[j].Dbar/(flow[j+1].X-flow[j-1].X)
         - (.4*sq(flow[j-1].Upold)*flow[j-1].Dbar+sq(flow[j-1].Uold)*flow[j-1].Dbar
         -1.4*flow[j-1].Uold*flow[j-1].Upold*flow[j-1].Dbar)/flow[j].Dbar/(flow[j+1].X-flow[j-1].X);
      Xp1       = -(flow[j].Upold - flow[j].Uold)*(flow[j+1].Upold-flow[j-1].Upold) / (flow[j+1].X-flow[j-1].X);
   }
   
/* Sum forces to find new velocity. */
   dU = (Xgrav+Xpres+Xresi+Xyield+Xmom)*dt;

   flow[j].U = flow[j].Uold + dU;
         
   flow[j].Up = flow[j].Upold + (Xgrav+Xp1+Xp2+Xpres)*dt;
   
   if ( j != 0 && j != n_flow_nodes-1 )
   {
      art1 = numerical_viscosity*abs(flow[j+1].Dbar-2*flow[j].Dbar+flow[j-1].Dbar) / 
         ( abs(flow[j+1].Dbar)+2*abs(flow[j].Dbar)+abs(flow[j-1].Dbar) );
      flow[j].U = flow[j].U + art1*(flow[j+1].Uold-2*flow[j].Uold+flow[j-1].Uold);
      flow[j].Up = flow[j].Up + art1*(flow[j+1].Upold-2*flow[j].Upold+flow[j-1].Upold);
   }

   if ( flow[j].U/flow[j].Up <= 2./3 ) 
      flow[j].Up=1.499*flow[j].U;
   if ( flow[j].U/flow[j].Up >= 1 ) 
      flow[j].Up = 1.001*flow[j].U;

   return;
}

#endif
