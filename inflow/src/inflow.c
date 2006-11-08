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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "inflow.h"
#include "sed_sedflux.h"
#include "utils.h"

//#define RHO_RIVER_WATER (1000.)
#define RHO_RIVER_WATER sed_rho_fresh_water()

#if defined(INFLOW_STANDALONE) || defined(INFLOW_DEBUG)
FILE *fpout_;
#endif

/** @memo inflow turbidity current model.

@doc A steady state turbidity current model based on the Mulder model.

Mulder, T., Savoye, B., and Syvitski, J.P.M., 1997.  Numerical modelling of a
mid-sized gravity flow: the 1979 Nice turbidity current (dynamics, processes,
sediment budget and seafloor impact).  Sedimentology, v. 44, pp. 305-326.

@param day           duration of the flow (s).  Typically this is one day.
@param x             x coordinate of each node (m).
@param slope         sea-floor slope (rads).  a negative slope means downward
                     dipping.
@param width         width of the channel (m).
@param n_nodes       number of nodes in the domain.
@param dx            distance between nodes (m).
@param x_dep         distance (m) from the river mouth before which no
                     deposition is allowed.
@param river_width   width of the river mouth (m).
@param river_vel     velocity of the river at the river mouth (m/s).
@param river_depth   depth of the river at the river mouth (m).
@param dc            water discharge at the river mouth (m^3/s).
@param gzF0          fraction of each grain type in the flow.
@param grain_dia     grain diameter of each grain type in the flow (m).
@param lambda        removal rate for each grain type (1/s).
@param rho_sed       bulk density of each grain type (kg/m^3).
@param rho_grain     grain density of each grain type (kg/m^3).
@param n_grains      number of grain types.
@param rho_rw        density of the river water (kg/m^3).  This is the river
                     water without any sediment.
@param rho_flow      density of the flow (kg/m^3).  This is the river water
                     plus the sediment.
@param consts        some constants that are required by the turbidity current
                     model.
@param deposit       2d array for storing the deposition rates for each node.
                     The fast dimension is over grid node number and the slow
                     dimension is over grain type.

@return 0 on success and -1 if a problem is found.

*/

int inflow( double day , double x[] , double slope[] , double width[] ,
   int n_nodes , double dx , double x_dep , double river_width ,
   double river_vel , double river_depth , double dc , double *gzF0 ,
   double *grain_dia, double *lambda, double *rho_sed, double *rho_grain,
   int n_grains, double rho_rw, double rho_flow, Inflow_t consts, double **deposit , FILE *fpout )
{
   double **save_data;

   void inflow_get_phe ( gpointer query_data , gpointer profile_data );
   void sedflux_get_phe( gpointer query_data , gpointer profile_data );

   // Node parameters.
   double q, q0, J, J0, conc, conc0;
   double h, h0, u, u0;
   double rho, rho0, rhoF, rhoF0, rhoS, rhoS0;
   double *Cgrain, *Cgrain0, *Jgrain, *Jgrain0;
   double RI, g0, E, A, A1, A2, A3;
   // Constants.
   double Ea, Eb, sua, sub, Cd, tanPhi, mu, rhoSW;
   double gravity;
   // Variables.
   double mass=0,massin=0,massout,masseroded=0;
   double gamma, beta, sineBeta, cosBeta;
   double denominatorExponent;
   double max_grain_dia, max_ws;
   double *Cgrain_init, *gzF;
   double *phe, erosion, *fDep, *fEro, uCritical;
   double muEffective, *ws;
   double depth=0, max_depth=0;
   int i, n;
   gpointer profile_data;
   I_phe_query_t phe_query;
#if defined(INFLOW_STANDALONE)
   gboolean standalone=FALSE;
#endif

#if defined(INFLOW_STANDALONE)
   standalone = TRUE;
#endif
   
   Ea     = consts.Ea;
   Eb     = consts.Eb;
   sua    = consts.sua;
   sub    = consts.sub;
   Cd     = consts.Cd;
   tanPhi = consts.tanPhi;
   mu     = consts.mu;
   rhoSW  = consts.rhoSW;

#ifdef INFLOW_DEBUG
   fpout = stderr;
#endif

   if ( fpout )
      save_data = eh_new_2( double , 13 , n_nodes );

   Jgrain      = eh_new( double , n_grains );
   Jgrain0     = eh_new( double , n_grains );
   Cgrain      = eh_new( double , n_grains );
   Cgrain_init = eh_new( double , n_grains );
   Cgrain0     = eh_new( double , n_grains );
   ws          = eh_new( double , n_grains );
   fEro        = eh_new( double , n_grains );
   fDep        = eh_new( double , n_grains );
   phe         = eh_new( double , n_grains );
   gzF         = eh_new( double , n_grains );

   gravity = sed_gravity();
   denominatorExponent = exp(1.0)-1.;
   
   // Initial conditions.
   for (n=0;n<n_grains;n++)
      gzF[n] = gzF0[n];
//   h0 = river_depth*river_width/width[0];
   u0    = river_vel;
   q0    = dc;
   rho0  = rho_rw;
   rhoF0 = rho_flow;
   rhoS0 = weighted_avg(gzF,rho_grain,n_grains);
   conc0 = (rhoF0-rho0)/(rhoS0-rho0);
   J0    = q0*conc0;
   for (n=0;n<n_grains;n++)
   {
      Cgrain0[n] = conc0*gzF[n];
      Cgrain_init[n] = Cgrain0[n];
      Jgrain0[n] = Cgrain0[n]*q0;
   }

   for (n=0;n<n_grains;n++)
      massin = conc0*q0*rho_grain[n];

//   fprintf(stderr,"inflow : Mass of sediment entering the flow (kg) : %f\n",massin*day);
   
/*
   for (i=0;i<n_nodes && conc0 > 1e-6 ;i++)
*/
   for (i=0;i<n_nodes ;i++)
   {
      // Find slope.
      beta = -slope[i];

      if ( fabs(beta) > .2)
         beta = .2;

      sineBeta = sin(beta);
      cosBeta = cos(beta);
      
      // Richardson number of the flow.
      g0 = gravity*(rhoS0-rho0)/rho0;
      RI = g0*cosBeta*J0/(u0*u0*u0*width[i]);
      
      // Entrainment coefficient.
      if ( sineBeta > 0.01 )
         E = Ea / (Eb+RI);
      else if ( sineBeta < 0.01 && sineBeta > 0 )
         E = 0.072*sineBeta;
      else
         E = 0.;

      // Gravity.
      if ( sineBeta > 0 )
      {
         A1 = g0*J0*sineBeta/u0/q0;
/*
         Cd = 0.004;
*/
      }
      else
      {
         A1 = 0.;
         A1 = g0*J0*sineBeta/u0/q0;
/*
         Cd = 0.004;
*/
      }

      depth += sineBeta;
      if ( depth > max_depth )
         max_depth = depth;
      if ( depth < max_depth )
      {
         if ( sineBeta > 0 )
            A1 *= 0.05;
         else
            A1 *= 0.05;
      }
      
      // Friction.
      A2 = -(E+Cd)*u0*u0*width[i]/q0;
      
      // Internal friction.
      gamma = tanPhi*(exp(J0/q0)-1.)/denominatorExponent;
      A3 = -.1*g0*J0*cosBeta*gamma/u0/q0;

/*
      if ( A3 < -.02 ) A3 = -.02;
      if ( A2 < -.02 ) A2 = -.02;
*/

/*      
      Cd = 0.004;
*/
      
      A = A1 + A2 + A3;
      
      u = u0 + A*dx;

// Drift velocity      
      if ( u < 0.01 )
         u = 0.01;

      if ( u <= 0 )
      {
         fprintf(stderr,"inflow : Error.  Velocity dropped below zero m/s\n");
         if ( fpout )
         {
            for (n=0;n<13;n++)
               fwrite( save_data[n] , n_nodes , sizeof(double) , fpout );
            eh_free_2(save_data);
         }
         return -1;         
      }
/*
      q = q0;
      h = q0/width[i]/u;
*/

      // New volume discharge.
      q = q0 + E*u*width[i]*dx;
      
      // New thickness.
      h = q/width[i]/u;
      
      /* New fluid density.
      */
      rho = rho0+E*u*width[i]*(rhoSW-rho0)/q0*dx;
      
      // Depth of erosion for a day.
      erosion = (Cd*rhoF0*u0*u0-sub)/sua;

      if ( erosion < 0 ) erosion = 0.;

      // Need to determine what phe is at each bin.
      // For the standalone version it will simply be an input and 
      // will be constant over the run.  For the version in SEDFLUX,
      // we'll have to write a routine to average the top 'erosion'
      // bins.
      profile_data = consts.get_phe_data;

      EH_STRUCT_MEMBER( I_phe_query_t , &phe_query , x           ) = x[i];
      EH_STRUCT_MEMBER( I_phe_query_t , &phe_query , dx          ) = dx;
      EH_STRUCT_MEMBER( I_phe_query_t , &phe_query , erode_depth ) = erosion;
      EH_STRUCT_MEMBER( I_phe_query_t , &phe_query , phe         ) = phe;

      (*(consts.get_phe))( (gpointer)&phe_query , profile_data );

      erosion = EH_STRUCT_MEMBER( I_phe_query_t , &phe_query , erode_depth );

// Find the maximum grain diameter that has a concentration greater than .01.
      for (n=0,max_grain_dia=0.;n<n_grains;n++)
         if ( grain_dia[n] > max_grain_dia )
            max_grain_dia = grain_dia[n];

      // Settling velocities.
      muEffective = mu*(1.+2.5*conc0);
/*
       ws[n] = g0/18.*grain_dia[n]*grain_dia[n]/muEffective;
       ws[n] = (rhoS0-rhoF0)*gravity*grain_dia[n]*grain_dia[n]/18./(rhoF0*muEffective);
*/
         
      max_ws = g0/18.*max_grain_dia*max_grain_dia/muEffective;

      /* Critical velocity for deposition.
      uCritical = ws[n]/sqrt(.016/8.);
      uCritical = ws[n]/sqrt(Cd);
      */

      uCritical = max_ws/sqrt(Cd);

      for (n=0;n<n_grains;n++)
      {
         // Rate of erosion (m^2/s).
         fEro[n] = erosion*phe[n]/DAY*width[i];

         // Deposition rate (m^2/s).
         // NOTE: we have divided the deposition rates by 10 to account
         // for the lambda obtained from plume data.
         if ( u >= uCritical )
            fDep[n] = 0.;
         else
            fDep[n] = -lambda[n]*Jgrain0[n]/u*(1-pow(u/uCritical,2.))/10.;
/*
            fDep[n] = -lambda[n]*Jgrain0[n]/u*(1-pow(u/uCritical,1/(n+1)))/10.;
            fDep[n] = -lambda[n]*Jgrain0[n]/u*(1-sqrt(u/uCritical));
            fDep[n] = -lambda[n]*Jgrain0[n]/u*(1-sqrt(u*u/uCritical/uCritical));
            fDep[n] = -lambda[n]*Jgrain0[n]/u*(1-(u*u/uCritical/uCritical));
*/

/*         
         if ( (x[i]-x[0]) <= x_dep )
            fDep[n] = 0.;
*/
         
         Jgrain[n] = Jgrain0[n] + (fDep[n]+fEro[n])*dx;
         
         // Deposit thickness.  Convert the deposit sediment to the appropriate
         // porosity.  The erosion part is given in meters of bottom sediment
         // eroded and so is already at the correct porosity.
#if defined(INFLOW_STANDALONE)
         deposit[n][i] = -(fDep[n]*rho_grain[n]/rho_sed[n]+fEro[n])*day/width[i];
#else
         deposit[n][i] = -(fDep[n]*rho_grain[n]/rho_sed[n])*day/width[i];
#endif
         mass += -fDep[n]*rho_grain[n]*dx;
         masseroded += fEro[n]*rho_grain[n]*dx;
      }

      // Get new grain size fractions in the flow.
      for (n=0,J=0;n<n_grains;n++)
         J += Jgrain[n];
      for (n=0;n<n_grains;n++)
         gzF[n] = Jgrain[n]/J;
      
      rhoS = weighted_avg(gzF,rho_grain,n_grains);
      for (n=0,conc=0;n<n_grains;n++)
      {
         Cgrain[n] = Jgrain[n]/q;
         conc += Cgrain[n];
      }

      if ( fpout )
      {
         save_data[0][i] = conc0;
         save_data[1][i] = rhoF0;
         save_data[2][i] = rho0;
         save_data[3][i] = A/dx;
         save_data[4][i] = A1;
         save_data[5][i] = A2;
         save_data[6][i] = A3;
         save_data[7][i] = E;
         save_data[8][i] = h0;
         save_data[9][i] = u0;
         save_data[10][i] = RI;
         for (n=0;n<n_grains;n++)
         {
            save_data[11][i] += fEro[n];
            save_data[12][i] += fDep[n];
         }
      }

      rhoF = conc*(weighted_avg(gzF,rho_grain,n_grains)-rho)+rho;

      // Save the node values.
      rho0 = rho;
      rhoS0 = rhoS;
      rhoF0 = rhoF;
      h0 = h;
      u0 = u;
      q0 = q;
      J0 = J;
      conc0 = conc;
      for (n=0;n<n_grains;n++)
      {
         Jgrain0[n] = Jgrain[n];
         Cgrain0[n] = Cgrain[n];
      }
      
   }

   massout = mass;
//   fprintf(stderr,"inflow : Mass deposited by the flow (kg) : %f\n",mass*day);
//   fprintf(stderr,"inflow : Mass eroded by the flow (kg) : %f\n",masseroded*day);
//   fprintf(stderr,"inflow : Mass balance error of : %f\n",(massin+masseroded-massout)/massin);

/* Deposit the sediment with the appropriate porosity.
   for (i=0;i<n_nodes;i++)
      for (n=0;n<n_grains;n++)
         deposit[n][i] *= (rho_grain[n]/rho_sed[n]);
*/


   if ( fpout )
   {
      for ( n=0 ; n<13 ; n++ )
         fwrite( save_data[n] , n_nodes , sizeof(double) , fpout );
      eh_free_2( save_data );
   }
   
   eh_free( Jgrain      );
   eh_free( Jgrain0     );
   eh_free( Cgrain      );
   eh_free( Cgrain0     );
   eh_free( ws          );
   eh_free( fDep        );
   eh_free( fEro        );
   eh_free( phe         );
   eh_free( gzF         );
   eh_free( Cgrain_init );

   return 0;
}
 
