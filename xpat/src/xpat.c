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

#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "sed_const.h"
#include "xpat.h"

//BOP
//
// !ROUTINE: pat_get_ripple_height - get the height of ripples on the sea bed.
//
// !INTERFACE:
double pat_get_ripple_height( d0 , mean_grain_size )

// !INPUT PARAMETERS:
   double d0;                // Wave orbital diameter near the sea bed (cm)
   double mean_grain_size;    // Mean grain size of sediment bed (cm)

// !DESCRIPTION:
// Calculate the sea bed ripple height ($\eta_{rip}$) and ripple width ($\lambda_{rip}$).  This is
// taken from Equation (10) of Wiberg and Harris (2001).  The ripple height is given by,
// \begin{equation}
// {\eta_{rip} \over \lambda_{rip}} = \exp \left( -.095\left( \log {d_0\over \eta_{rip}} \right)^2 + .442 \log {d_0\over \eta_{rip} } - 2.28 \right)
// \end{equation}
// The ripple height can not be solved for explicitly and so we use Newton's method to iterate to
// a value for $\eta_{rip}$.  This equation assumes that we are not in anorbital conditions.  If
// we find that this is not the case, (that is, if ${\eta_{rip} \over \lambda_{rip}} < .01$) we specify
// $\eta_{rip}=0$.  Also, if the ripple height is less than three times the mean grain diameter we
// specify $\eta_{rip} = 0$.  Furthermore, the ripple steepness is limited to .17.  This is 
// accomplised by altering the $\eta_{rip}$ accordingly.
//
//
// !REVISION HISTORY:
// Feb 2002 Eric Hutton Initial version.
//
//EOP

{
   void f( double x , double *y , double *dy , double *data );
   double lambda_rip;
   double eta_rip;
   double data[2];

   //
   // assume anorbital conditions.
   //
   data[0] = d0;
   data[1] = lambda_rip;
   lambda_rip = 535.*mean_grain_size;
   eta_rip = rtsafe( &f , .01 , 100 , .01 , data );

   if ( eta_rip/lambda_rip > .17 )
      eta_rip = .17*lambda_rip;

   if ( eta_rip < 3*mean_grain_size )
      eta_rip = 0;

   // check if anorbital conditions are consistent.
   if ( eta_rip/lambda_rip<.01 )
   {
      // no. we have upper plane bed conditions.
      eta_rip = 0.;
   }

   return eta_rip;
}

void f( double x , double *y , double *dy , double *data )
{
   double g, dg;
   double d0 = data[0];
   double lambda_rip = data[1];

   g   = -0.095*pow(log(d0/x),2) + 0.442*log(d0/x) - 2.28;
   dg  = 2*.095*log(d0/x)/x - .442/x;

   *y  = exp( g ) - x / lambda_rip;
   *dy = exp( g ) * dg - 1./lambda_rip;
}

//BOP
//
// !ROUTINE: pat_get_vertical_diffusion - calculate the vertical diffusion coefficient.
//
// !INTERFACE:
double pat_get_vertical_diffusion( z , u_star_c , delta_c , u_star_w , delta_w )

// !INPUT PARAMETERS:
   double z;          // height above datum (cm)
   double u_star_c;   // shear velocity due to currents (cm/s)
   double delta_c;    // height of bottom boundary layer (cm)
   double u_star_w;   // shear velocity due to waves (cm/s)
   double delta_w;    // height of wave bottom boundary layer (cm)

// !RETURN VALUE:
// The calculated diffusion coefficient is returned.

// !DEFINED PARAMETERS:
#define VONKARMAN (0.408)

// !DESCRIPTION:
// Calculate the vertical diffusion coefficient, $K_z$.  Equation (7) of Wiberg and Harris (2001) 
// solve for $K_z$ as,
// \begin{equation}
// K_z = \kappa z \sqrt{ \left( u_{\star c} e^{-{z\over \delta_c}} \right)^2 + \left( u_{\star w} e^{-{z\over \delta_w}} \right)^2  }
// \end{equation}
// where $\kappa$ is von Karman's constant (.408), $u_{\star c}$ is shear velocity due to currents,
// $u_{\star w}$ is shear velocity due to waves, $\delta_c$ is thickness of the bottom boundary layer,
// $\delta_w$ is the height of the wave bottom boundary layer, and $z$ is height above datum.
//
// To match the code of Wiberg and Harris we add a couple of extras that are not in Harris and 
// Wiberg.  First, if desired, another formulation of $K_z$ can be used,
// \begin{equation}
// K_z = \kappa z {1\over 1+\beta\zeta} \sqrt{ \left( u_{\star c} e^{-{z\over \delta_c}} \right)^2 + \left( u_{\star w} e^{-{z\over \delta_w}} \right)^2  }
// \end{equation}
// Second, another diffusion coefficient, $K_z^\star$ is calculated.  For the first case, $K_z^\star = K_z$ but for the second case,
// \begin{equation}
// K_z^\star = \kappa z {1\over \gamma+\beta_r\zeta} \sqrt{ \left( u_{\star c} e^{-{z\over \delta_c}} \right)^2 + \left( u_{\star w} e^{-{z\over \delta_w}} \right)^2  }
// \end{equation}
// where $\beta=5.4$, $\beta_r=7.3$, and $\gamma=1.0$.  Third, each of the diffusion coefficients are
// limited to a minimum value of $\nu = 0.013$ (only if the elevation if greater than one meter above
// the sea bed).
//
// !REVISION HISTORY:
// Feb 2002 Eric Hutton Initial version.
//
//EOP
{
   double K_z;
   double k = VONKARMAN;

   K_z = k*z*sqrt( pow(u_star_c*exp(-z/delta_c),2) + pow(u_star_w*exp(-z/delta_w),2) );

   return K_z;
}

double *pat_get_diffusion_with_depth( double *k , double *z , double u_star_c , double u_star_w , double u_star_cw , double coriolis_freq , double wave_freq , int n )
{
   int i;
   double delta_c = u_star_c/6./coriolis_freq;
   double delta_w = u_star_cw/3./wave_freq;

   for ( i=0 ; i<n ; i++ )
      k[i] = pat_get_vertical_diffusion( z[i] , u_star_c , delta_c , u_star_w , delta_w );

   return k;
}

//BOP
// !ROUTINE: pat_get_coriolis_frequency - given a latitude, calculate the coriolis frequency

// !INTERFACE:
double pat_get_coriolis_frequency( lat )

// !INPUT PARAMETERS:
   double lat;  // latitude (in degrees) 

// !RETURN VALUE:
// Returns the calculated coriolis frequency at the given latitude.

// !DEFINED PARAMETERS:
#define OMEGA (7.292e-5)

// !REVISION HISTORY:
// Feb 2002 Eric Hutton Initial version.

//EOP
{
   return fabs(2.*OMEGA*sin(lat*M_PI/180.));
}

//BOP
// !ROUTINE: pat_solve_velocity_profile - calculate the velocity profile with depth.

// !INTERFACE:
Complex *pat_solve_velocity_profile( vel , k , z , coriolis , vel_geo , n_bot , vel_bottom , n_top , vel_top , n )

// !INPUT PARAMETERS:
   double *k;           // the vertical diffusion coefficient with depth.
   double *z;           // elevation of each of the layers.
   double coriolis;     // coriolis frequency.
   Complex vel_geo;     // geostrophic velocities.
   int n_bot;           // index to the lower boundary.
   Complex vel_bottom;  // velocity at the lower boundary.
   int n_top;           // index to the top boundary.
   Complex vel_top;     // velocity at the top boundary.
   int n;               // number of layers.  length of vel and k.

// !OUTPUT PARAMETERS:
   Complex *vel;        // the calculated velocity profile.

// !RETURN VALUE:
// Returns a pointer to the calculted velocity profile.

// !DESCRIPTION:
// Current velocity in the bottom layer is calculated using the momentum 
// equation for an unstratified, steady uniform planetary boundary layer 
// beneath an interior region in which the flow is in geostrophic balance.
// \begin{eqnarray}
// {\partial \over \partial z}\left( K_z {\partial u \over \partial z} \right) - f \left( v_{geo} - v \right) & = & 0\\
// {\partial \over \partial z}\left( K_z {\partial v \over \partial z} \right) + f \left( u_{geo} - u \right) & = & 0
// \end{eqnarray}
// where $u$ and $v$ are the $x$ and $y$ components of velocity, $K_z$ is the 
// vertical eddy viscosity, $f$ is the Coriolis frequency, and $u_{geo}$ and
// $v_{geo}$ are the velocity components at the top of the boundary layer,
// which is assumed to lie in the geostrophiclly balanced region.

// !REVISION HISTORY:
// Feb 2002 Eric Hutton Initial version.

//EOP
{
   int i;
   double dz_down, dz, dz_up;
   double dk;
   Complex *l = g_new( Complex , n );
   Complex *d = g_new( Complex , n );
   Complex *u = g_new( Complex , n );
   Complex *f = g_new( Complex , n );

   l[0] = c_complex(0,0);
   d[0] = c_complex( -(k[1]-k[0])/pow(z[1]-z[0],2) , -coriolis );
   u[0] = c_complex(  (k[1]-k[0])/pow(z[1]-z[0],2) , 0 );
   f[0] = c_complex( coriolis*vel_geo.i , - coriolis*vel_geo.r );
//   d[0] = c_complex(1,0);
//   u[0] = c_complex(0,0);
   for ( i=1 ; i<n-1 ; i++ )
   {
      dz_down = z[i]-z[i-1];
      dz_up   = z[i+1]-z[i];
      dz      = .5*(dz_down + dz_up);
      dk      = .5*( (k[i]-k[i-1])/dz_down + (k[i+1]-k[i])/dz_up );

      l[i] = c_complex( -dk/(2*dz_down) + k[i]/(dz_down*dz) , 0 );
      u[i] = c_complex(  dk/(2*dz_up)   + k[i]/(dz_up*dz)   , 0 );
      d[i] = c_complex(  dk*(1/(2*dz_down) - 1/(2*dz_up) ) - k[i]*(1/(dz_up*dz) + 1/(dz_down*dz)) , -coriolis);

      f[i] = c_complex( coriolis*vel_geo.i , -coriolis*vel_geo.r );
   }
//   l[n-1] = c_complex(0,0);
//   d[n-1] = c_complex(1,0);
   l[n-1] = c_complex(  (k[n-1]-k[n-2])/pow(z[n-1]-z[n-2],2) , 0 );
   d[n-1] = c_complex( -(k[n-1]-k[n-2])/pow(z[n-1]-z[n-2],2) , -coriolis );
   u[n-1] = c_complex(0,0);
   f[n-1] = c_complex( coriolis*vel_geo.i , - coriolis*vel_geo.r );

   // boundary conditions.
//   f[0]   = vel_bottom;
//   f[n-1] = vel_top;
   l[n_bot] = c_complex( 0 , 0 );
   d[n_bot] = c_complex( 1 , 0 );
   u[n_bot] = c_complex( 0 , 0 );
   f[n_bot] = vel_bottom;

   l[n_top] = c_complex( 0 , 0 );
   d[n_top] = c_complex( 1 , 0 );
   u[n_top] = c_complex( 0 , 0 );
   f[n_top] = vel_top;

   if ( !c_tridiag( l , d , u , f , vel , n ) )
      perror( "pat_solve_velocity_profile" );

   free( l   );
   free( d   );
   free( u   );
   free( f   );

   return vel;
}

#define DRAG_COEFFICIENT (.5)
//BOP
// !ROUTINE: pat_get_skin_friction_shear - calculate the skin friction shear stress.

// !INTERFACE:
double pat_get_skin_friction_shear( tau_total , roughness , ripple_height , ripple_length )

// !INPUT PARAMETERS:
   double tau_total;      // the total shear stress (due to currents and waves)
   double roughness;      // hydraulic roughness of the bed surface.
   double ripple_height;  // height of the sea floor ripples.
   double ripple_length;  // length of the sea floor ripples.

// !RETURN VALUE:
// Returns the skin friction shear stress.

// !DESCRIPTION:
// The presence of bedforms reduces the shear stress acting on the bed surface,
// termed the skin friction shear stress ($\tau_{sf}$), relative to the total
// shear stress, $\tau_{cw}$.  It is the skin friction component of shear 
// stress that is important for sediment transport calculations.  The total
// shear stress is divided into its bedform and skin friction components 
// using the estimated bedform dimensions,
// \begin{equation}
// \tau_{sf} = \tau_{cw}\left( 1 + {C_d\over 2}{\eta\over\lambda}{1\over\kappa^2}\left(\log{\eta\over {z_0}_{sf}}-1 \right)^2\right)^{-1}
// \end{equation}
// where $C_d$ is the bedform drag coefficient, ${z_0}_{sf}$ is the hydraulic
// roughness of the bed surface, $\eta$ is the bedform height, and $\lambda$
// is the bedform length.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton  Initial Version.
//EOP

{
   double k=VONKARMAN;
   double c_d = DRAG_COEFFICIENT;
   double tau_sf = 0.;

   if ( ripple_height > 0. )
      tau_sf = tau_total / (1.0 + .5*c_d*ripple_height/ripple_length/k/k*pow((log(ripple_height/roughness)-1),2) );

   if ( tau_sf > tau_total )
      tau_sf = tau_total;

   return tau_sf;
}

//BOP
// !ROUTINE: pat_get_tau_max - calculate the maximum shear stress at ripple crest.

// !INTERFACE:
double pat_get_tau_max( tau_sf , tau_total , ripple_height , ripple_length )

// !INPUT PARAMETERS:
   double tau_sf;         // the skin friction shear stress.
   double tau_total;      // the total shear stress due to waves and currents.
   double ripple_height;  // the height of the bedforms.
   double ripple_length;  // the length of the bedforms.

// !RETURN VALUE:
// The calculated skin friction shear stress near the ripple crest is returned.

// !DESCRIPTION:
// Shear stress varies spatially over a bedform, with the highest stresses
// near the crest.  As a result, it is possible for the spatially averaged
// shear stress over a ripple to be below the threshold of motion while the 
// shear stress near the crest exceeds the threshold.  To account for this,
// the spatially averaged skin friction shear stress ($\tau_{sf}$) is adjusted
// to provide an estimate of skin friction shear stress near the ripple crest,
// \begin{equation}
// \tau_{sfm} = \tau_{sf} \left( 1+8{\eta\over\lambda}\right)
// \end{equation}
// where $\tau_{sf}$ is the skin friction shear stress, $\eta$ is the bedform
// height and $\lambda$ is the bedform wavelength.  If $\tau_{sfm}$ is 
// found to be larger than the total shear stress due to currents and waves,
// then it is clamped to $\tau_{cw}$.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton  Initial Version.
//EOP
{
   double tau_sf_max;

   if ( ripple_height > 0 )
      tau_sf_max = (1 + 8.*ripple_height/ripple_length)*tau_sf;
   else
      tau_sf_max = tau_sf;

   if ( tau_sf_max > tau_total )
      tau_sf_max = tau_total;

   return tau_sf_max;
}

#define DENSITY_SEA_WATER (1.03)

//BOP
// !ROUTINE: pat_get_shear_stress_profile - calculate the shear stress profile.

// !INTERFACE:
double *pat_get_shear_stress_profile( tau , z , u , k , n )

// !INPUT PARAMETERS:
   double *z;    // the water depths to calculate the shear stress at.
   double *u;    // velocities at each depth.
   double *k;    // vertical diffusion coefficient at each depth.
   int n;        // length of arrays.

// !OUTPUT PARAMETERS:
   double *tau;  // the shear stress profile.

// !DESCRIPTION:
// Calculate the shear stress at the water depths given by $z$.  The shear
// stress $\tau$ is solved as,
// \begin{equation}
// \tau = \rho K_z {\partial u \over \partial z}
// \end{equation}
// where $\rho$ is the density of sea water, $K_z$ is the eddy viscosity
// (vertical diffusion coefficient), $u$ is the current velocity, and $z$ is
// elevation above the bed.  We solve this equation with an explicit finite 
// difference method that allows for an irregular spaced gird.
// \begin{equation}
// \tau_i = \rho {K_z}_i \left( a_1 u_{i-1} + (-a_1-a_2) u_i + a_2 u_{i+1}\right)
// \end{equation}
// where,
// \begin{eqnarray}
// a_1 & =  - {1 \over \nabla z\left( {\nabla z\over \Delta z} + 1\right)}\\
// a_2 & = \hphantom{-} {1 \over \Delta z\left( {\Delta z\over \nabla z} + 1\right)}
// \end{eqnarray}
// and
// \begin{eqnarray}
// \Delta z & = & z_{i+1} - z_i\\
// \nabla z & = & z_i - z_{i-1}
// \end{eqnarray}

// !REVISION HISTORY:
// Arp 2002  Eric Hutton  Initial version.

//EOP
{
   int i;
   double rho=DENSITY_SEA_WATER;
   double dz_up, dz_down, a_1, a_2;

   for ( i=1 ; i<n-1 ; i++ )
   {

      dz_up   = z[i+1]-z[i];
      dz_down = z[i]-z[i-1];

      a_1 = -1/(dz_down*(dz_down/dz_up+1));
      a_2 =  1/(dz_up*(dz_up/dz_down+1));

      tau[i] = rho*k[i]*( a_1*u[i-1] + (-a_1-a_2)*u[i] + a_2*u[i+1] );

   }
   tau[0]   = rho*k[0]*(u[1]-u[0])/(z[1]-z[0]);
   tau[n-1] = rho*k[n-1]*(u[n-1]-u[n-2])/(z[n-1]-z[n-2]);

   return tau;
}

double *pat_get_total_current_stress( tau_total , tau_x , tau_y , n )
   double *tau_total;
   double *tau_x;
   double *tau_y;
   int n;
{
   int i;
   for ( i=0 ; i<n ; i++ )
      tau_total[i] = sqrt( tau_x[i]*tau_x[i] + tau_y[i]*tau_y[i] );
   return tau_total;
}

//BOP
// !ROUTINE: pat_get_total_stress - calculate the total stress due to waves and currents

// !INTERFACE:

double *pat_get_total_stress( tau_total , tau_x , tau_y , tau_wave , wave_dir , n )

// !INPUT PARAMETERS:
   double *tau_x;      // x-component of current shear stress.
   double *tau_y;      // y-component of current shear stress.
   double *tau_wave;   // total wave component of shear stress.
   double wave_dir;    // direction of incoming waves.
   int n;              // length of shear stress arrays.

// !OUTPUT PARAMETERS:
   double *tau_total;  // the total shear stress

// !RETURN VALUE:
// A pointer to the total shear stress array is returned.

// !DESCRIPTION:
// Calculate the total shear stress due to waves and currents from the wave
// and current components.  The total shear stress $\tau$ is,
// \begin{equation}
// \tau^2 = ( {\tau_c}_x + \tau_w\cos\alpha )^2 + ({\tau_c}_y + \tau_w\sin\alpha)^2
// \end{equation}
// where ${\tau_c}_x$ and ${\tau_c}_y$ are the $x$ and $y$ components of the shear
// stress due to currents respectively, $\tau_w$ is the shear stress 
// associated with waves, and $\alpha$ is the incoming wave direction.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton  Initial Version.
//EOP

{
   int i;
   double tau_wave_x, tau_wave_y;
   for ( i=0 ; i<n ; i++ )
   {
      tau_wave_x   = fabs(tau_wave[i]*cos(wave_dir));
      tau_wave_y   = fabs(tau_wave[i]*sin(wave_dir));
      tau_total[i] = sqrt( pow(fabs(tau_x[i])+tau_wave_x,2) + pow(fabs(tau_y[i])+tau_wave_y,2) );
   }
   return tau_total;
}

double pat_get_u_star( double tau )
{
   double rho=DENSITY_SEA_WATER;
   return sqrt( tau / rho );
}

double pat_get_tau( double u_star )
{
   double rho=DENSITY_SEA_WATER;
   return pow( u_star , 2 )*rho;
}

#define MAX_ITER (150)
#include "xpat.h"

Complex *pat_iterate_velocity( Complex *vel_cur , double *k , double *z , int n , Complex vel_bbl_wave , Complex vel_bbl_cur , Complex vel_geo , double coriolis_freq , double wave_freq , double wave_dir , int n_bbl )
{
   double *u_wave, *u_cur, *v_cur;
   double *tau_wave, *tau_x , *tau_y;
   double *tau_c, *tau_tot;
   Complex *vel_wave;
//   double *k;
   double gamma = .1;
   double residual, residual_old;
   double u_star_c, u_star_w, u_star_cw, u_star_cw_old;
   double u_star_c_old, u_star_w_old;
   int converged=FALSE;
   int i,it=0, max_iter=MAX_ITER;

   vel_wave  = g_new( Complex , n );
   tau_wave  = g_new( double  , n );
   tau_x     = g_new( double  , n );
   tau_y     = g_new( double  , n );
   tau_c     = g_new( double  , n );
   tau_tot   = g_new( double  , n );
//   k         = g_new( double  , n );
   u_wave    = g_new( double  , n );
   u_cur     = g_new( double  , n );
   v_cur     = g_new( double  , n );

   // initial guess for shear velocities.
   u_star_c  = 1.;
   u_star_w  = 1.;
   u_star_cw = sqrt( pow(u_star_c,2) + pow(u_star_w,2) );

   do
   {

      // calculate the diffusion coefficient with depth.
      pat_get_diffusion_with_depth( k , z , u_star_c , u_star_w , u_star_cw , coriolis_freq , wave_freq , n ); 

      // calculate the velocity profiles for waves and currents.
      pat_solve_velocity_profile( vel_wave , k , z , coriolis_freq , vel_geo , 0 , c_complex(0.,0.) , n_bbl-1 , vel_bbl_wave , n_bbl );
      pat_solve_velocity_profile( vel_cur  , k , z , coriolis_freq , vel_geo , 0 , c_complex(0.,0.) , n_bbl-1 , vel_bbl_cur  , n_bbl );

      for ( i=0 ; i<n_bbl ; i++ )
      {
         u_wave[i] = vel_wave[i].r;
         u_cur[i]  = vel_cur[i].r;
         v_cur[i]  = vel_cur[i].i;
      }

      // calculate the shear stress profiles for waves and currents.
      pat_get_shear_stress_profile( tau_wave , z , u_wave , k , n_bbl );
      pat_get_shear_stress_profile( tau_x    , z , u_cur  , k , n_bbl );
      pat_get_shear_stress_profile( tau_y    , z , v_cur  , k , n_bbl );

      // find the total shear stresses.
      pat_get_total_current_stress( tau_c , tau_x , tau_y , n_bbl );
      pat_get_total_stress( tau_tot , tau_x , tau_y , tau_wave , wave_dir , n_bbl );

      // save the old u_star_cw.
      u_star_cw_old = u_star_cw;
      u_star_c_old = u_star_c;
      u_star_w_old = u_star_w;

      // calculate the new u_star's.
      u_star_c  = pat_get_u_star( tau_c[0]   );
      u_star_w  = pat_get_u_star( tau_wave[0]   );
      u_star_cw = sqrt( pow(u_star_c,2) + pow(u_star_w,2) );
//      u_star_cw = pat_get_u_star( tau_tot[0] );

      // calculate the residual using the l2 norm
      residual_old = residual;
      residual = pat_get_velocity_residual( z , vel_cur , k , n_bbl-1 , coriolis_freq , vel_geo );
fprintf( stderr , "%f\n" , residual );
      // compare the new residual with the old one.
      if ( fabs(residual-residual_old)/residual_old < .1 )
         converged = TRUE;

      // guess a new u_star_cw for the next iteration.  the constant gamma should be kept small
      // so that we are able to better zero in on the solution.
      u_star_c  = u_star_c - gamma*(u_star_c-u_star_c_old);
      u_star_w  = u_star_w - gamma*(u_star_w-u_star_w_old);
      u_star_cw = sqrt( pow(u_star_c,2) + pow(u_star_w,2) );

   }
   while ( !converged && (++it)<max_iter );

   // solve for the velocities above the bbl.
//   pat_solve_velocity_profile( vel_cur+n_bbl  , k+n_bbl , z+n_bbl , coriolis_freq , vel_geo , vel_bbl_cur , vel_geo , n-n_bbl );
   pat_solve_velocity_profile( vel_cur  , k , z , coriolis_freq , vel_geo , 0 , c_complex(0,0) , n_bbl-1 , vel_bbl_cur , n );

   if ( !converged )
      fprintf( stderr , "warning : pat_iterate_velocity : not converged after %d iterations.\n", max_iter );

   return vel_cur;
}

double pat_get_velocity_residual( double *z , Complex *vel , double *k , int n , double f , Complex vel_geo )
{
   int i;
   double dz_down, dz, dz_up;
   double res=0, res_u, res_v;
   for ( i=1 ; i<n-1 ; i++ )
   {
      dz_up   = z[i+1] - z[i];
      dz_down = z[i]   - z[i-1];
      dz      = (dz_up + dz_down )/2.;
      res_u = k[i]*(vel[i-1].r/(dz_down*dz) - 2*vel[i].r/(dz_up*dz_down) - vel[i+1].r/(dz_up*dz) ) + (k[i+1]-k[i])*(vel[i+1].r-vel[i].r)/pow(dz_up,2) -f*(vel_geo.i - vel[i].i);
      res_v = k[i]*(vel[i-1].i/(dz_down*dz) - 2*vel[i].i/(dz_up*dz_down) - vel[i+1].i/(dz_up*dz) ) + (k[i+1]-k[i])*(vel[i+1].i-vel[i].i)/pow(dz_up,2) -f*(vel_geo.r - vel[i].r);
      res += res_u*res_u + res_v*res_v;
   }
   return sqrt(res)/n;
}

#include "xpat.h"

//BOP
//  !ROUTINE: average_grains - compute average grain properties.
//
// !INTERFACE:

double average_grains( grain , amount , n_grains , avg )

// !INPUT PARAMETERS:
   Grain_class *grain; // an array of grain classes to average.
   double *amount;     // the amount of each grain class.
   int n_grains;       // the number of grain classes.

// !OUTPUT PARAMETERS:
   Grain_class *avg;   // a grain class containing the average.

// !RETURN VALUE:
// The fraction of sand grains that were used in the average.  If there is no
// sand in the input classes, the average is set to zero and zero is returned.

// !DEFINED PARAMETERS:
#define PHI_SAND (3.5) // minimum size of sand grains (phi)

// !DESCRIPTION:
// Average all of the grains larger than a certain size.  This size is
// set to average only the sand grains.  The average grain size is found by
// averaging the grain classes in phi units.  The corresponding grain 
// properties are found by linearly interpolating between then grain classes.

// !REMARKS:
// The interpolation is done slightly different that in the initial version.
// Need to find out why it was done that way.

// !REVISION HISTORY:
//           Patricia Wiberg  Initial Version.
// Apr 2002  Eric Hutton      Rewritten and Converted to C.

//EOP

{
   int i;
   int found;
   double phi_mean=0;
   double total=0, total_sand=0;
   double a;

   // calculate the average size and amount of sands.
   for ( i=0 ; i<n_grains ; i++ )
   {
      if ( grain[i].phi < PHI_SAND )
      {
         phi_mean   += grain[i].phi*amount[i];
         total_sand += amount[i];
      }
      total += amount[i];
   }
   if ( total_sand<=0 )
   {
      eh_message( "There is no sand." );
      set_grain_class( avg , 0 , 0 , 0 , 0 , 0 );
      return 0;
   }
   else
      avg->phi = phi_mean/total_sand;

   // find the grain class that contains the average grain size.
   for ( i=0,found=FALSE ; i<n_grains && !found ; i++ )
      if ( avg->phi>grain[i].phi_min && avg->phi<=grain[i].phi_max )
         found = TRUE;

   // interpolate to find grain properties tau_cr, and ws.  the critical
   // shear stress for resuspension and settling velocity, respectively.
   if ( found )
      if ( i>0 )
      {
         a = (grain[i].tau_cr-grain[i-1].tau_cr)/(grain[i].phi-grain[i-1].phi);
         avg->tau_cr = a*(avg->phi-grain[i-1].phi) + grain[i-1].tau_cr;

         a = (grain[i].ws-grain[i-1].ws)/(grain[i].phi-grain[i-1].phi);
         avg->ws     = a*(avg->phi-grain[i-1].phi) + grain[i-1].ws;
      }
      else
      {
         a = (grain[i+1].tau_cr-grain[i].tau_cr)/(grain[i+1].phi-grain[i].phi);
         avg->tau_cr = a*(avg->phi-grain[i].phi) + grain[i].tau_cr;

         a = (grain[i+1].ws-grain[i].ws)/(grain[i+1].phi-grain[i].phi);
         avg->ws     = a*(avg->phi-grain[i].phi) + grain[i].ws;
      }
   else
      eh_error( "Average is outside limits." );

   // return the total amount of sand.
   return total_sand/total;
}

void set_grain_class( Grain_class *g , double phi_min , double phi , double phi_max , double ws , double tau_cr )
{

   g->phi_min = phi_min;
   g->phi     = phi;
   g->phi_max = phi_max;
   g->ws      = ws;
   g->tau_cr  = tau_cr;

}

//BOP
// !ROUTINE: - get_hydraulic_roughness - calculate the hydraulic roughness.
//
// !INTERFACE:

double get_hydraulic_roughness( d_50 , tau_cr , tau_sf , ripple_height , ripple_length )

// !INPUT PARAMETERS:
   double d_50;            // mean grain size of sediment bed.
   double tau_cr;          // critical shear stress for initial motion.
   double tau_sf;          // skin friction shear stress.
   double ripple_height;   // bedform height.
   double ripple_length;   // bedform length.

// !RETURN VALUE:
// The hydraulic roughness is returned.
//
// !DEFINED PARAMETERS:

#define HR_METHOD_NIK      (0)
#define HR_METHOD_WIBERG   (1)
#define HR_METHOD_DIETRICH (2)

// !DESCRIPTION:
// Calculate the hydraulic roughness, $z_0$ based on the grain roughness,
// sediment transport roughness, and bedform roughness.  The grain roughness
// is calculated using the Nikuradse method (${z_0}_{gr}$), the sediment
// transport roughness (${z_0}_{st}$) is calculated using Dietrich method,
// and the bedform roughness is,
// \begin{equation}
// {z_0}_{bf} = {\eta^2 \over \lambda } 
// \end{equation}
// where $\eta$ is the bedform height, and $\lambda$ is the bedform length.
// The outer hydraulic roughness ${z_0}_{out}$ is,
// \begin{equation}
// {z_0}_{out} = \cases {
// {z_0}_{bf}, &${z_0}_{bf}>{z_0}_{st}$, and ${z_0}_{bf}>{z_0}_{gr}$\cr
// {z_0}_{st}, &$\tau_{sf} > \tau_{cr}$\cr
// {z_0}_{gr}, &otherwise }
// \end{equation}
// The inner hydraulic roughness ${z_0}_{in}$ is,
// \begin{equation}
// {z_0}_{in} = \cases {
// {z_0}_{st}, &$\tau_{sf} > \tau_{cr}$\cr
// {z_0}_{gr}, &otherwise }
// \end{equation}
//
// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
{
   double roughness_out;
   double roughness_in;
   double roughness_bf;      // bedform roughness.
   double roughness_grain;   // grain roughness.
   double roughness_st;      // sediment transport roughness.

   roughness_bf    = pow( ripple_height , 2 )/ripple_length;
   roughness_grain = get_roughness( d_50 , tau_cr , tau_sf , HR_METHOD_NIK );
   roughness_st    = get_roughness( d_50 , tau_cr , tau_sf , HR_METHOD_DIETRICH );

   if ( roughness_bf > roughness_st && roughness_bf > roughness_grain )
      roughness_out = roughness_bf;
   else if ( tau_sf > tau_cr )
      roughness_out = roughness_st;
   else
      roughness_out = roughness_grain;

   if ( tau_sf > tau_cr )
      roughness_in = roughness_st;
   else
      roughness_in = roughness_grain;

   return roughness_out;
}

//BOP
// !ROUTINE: get_roughness - calculate the bed roughness.
//
// !INTERFACE:

double get_roughness( d_50 , tau_cr , tau_sf , method )

// !INPUT PARAMETERS:
   double d_50;    // mean grain size of sediment bed.
   double tau_cr;  // critical shear stress for initial motion.
   double tau_sf;  // skin friction shear stress.
   int method;     // method used to calculate the hydraulic roughness.
                   // use: HR_METHOD_NIK       for nikuradse method.
                   //      HR_METHOD_WIBERG    for wiberg smith method.
                   //      HR_METHOD_DIETRICH  for dietrich method.
//
// !RETURN VALUE:
// The hydraulic roughness is returned.
//
// !DESCRIPTION:
//
// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
{
   double roughness;
   switch ( method )
   {
      case HR_METHOD_NIK:
         roughness = get_roughness_nik( d_50 , tau_sf );
         break;
      case HR_METHOD_WIBERG:
         roughness = get_roughness_wiberg( d_50 , tau_cr , tau_sf );
         break;
      case HR_METHOD_DIETRICH:
         roughness = get_roughness_dietrich( d_50 , tau_cr , tau_sf );
         break;
   }
   return roughness;
}

double get_roughness_nik( d_50 , tau_sf )
   double d_50;
   double tau_sf;
{
   double roughness;
   double rho_water = 1.02;
   double nu_water = .013;
   double r_star = sqrt( tau_sf*rho_water )*d_50/nu_water;
   double log_r_star = log( r_star );

   if ( r_star <= 3 )
      roughness = 1/(r_star*9);
   else if ( r_star <= 6 )
      roughness = - .00798*pow( log_r_star , 3 ) 
                  + .05428*pow( log_r_star , 2 )
                  - .12569*log_r_star + .11990;
   else if ( r_star <= 35 )
      roughness = - .00141*pow( log_r_star , 3 ) 
                  + .01543*pow( log_r_star , 2 )
                  - .04859*log_r_star + .06860;
   else if ( r_star <= 200 )
      roughness =   .00053*pow( log_r_star , 3 ) 
                  - .00930*pow( log_r_star , 2 )
                  + .05377*log_r_star - .06960;
   else
      roughness = 1/30.;

   return roughness*d_50*2;
}

double get_roughness_wiberg( d_50 , tau_cr , tau_sf )
   double d_50;
   double tau_cr;
   double tau_sf;
{
   double roughness;
   double a_2;
   double a_1   = .68;
   double alpha = .06;
   double t_star = tau_sf / tau_cr;

   a_2 = .0204*pow( log( d_50 ) , 2 ) + .0220*log( d_50 ) + .0709;

   roughness = alpha*d_50*a_1*t_star/(1+a_2*t_star);

   return roughness;
}

double get_roughness_dietrich( d_50 , tau_cr , tau_sf )
   double d_50;
   double tau_cr;
   double tau_sf;
{
   double roughness;
   double a_2   = .2;
   double a_1   = .68;
   double alpha = .06;
   double t_star = tau_sf / tau_cr;

   roughness = alpha*d_50*a_1*t_star/(1+a_2*t_star);

   return roughness;
}

//BOP
// !ROUTINE: pat_get_c_a - get the sediment concentation at the bed.

// !INTERFACE:
double pat_get_c_a( s_sfm , fr , c_b , w_s , u_star )

// !INPUT PARAMETERS:
   double s_sfm;   // excess skin friction shear stress
   double fr;      // volumetric fraction of size class in bed
   double c_b;     // bed concentration of sediment
   double w_s;     // settling velocity
   double u_star;  // 

// !RETURN VALUE:
// The concentration of sediment just above the bed is returned.

// !DEFINED PARAMETERS:
#define RESUSPENSION_COEF (0.002)

// !DESCRIPTION:
// Following the formulation of Parker (1978), the upward flux of sediment from the
// bed is defined to be the product of settling velocity and an entrainment function
// ($E$) that has units of concentration.  At steady state, the rate of entrainment
// of sediment, $Ew_s$, must equal the downward sediment flux near the bed, 
// $C_a w_s$, where $C_a$ is the volumetric equilibrium concentration at a reference
// level close to the bed.  Smith and McLean (1977) proposed an expression for $C_a$
// at the near-bed elevation, $z_a$,
// \begin{equation}
// C_a = \cases {
// {fr C_b \gamma_0 S_{sfm} \over 1 + \gamma_0 S_{sfm}}, &S_{sfm}>0$ and {w_s \over \kappa {u_\star}_{sfm}}<2.5\cr
// 0, &otherwise }
// \end{equation}
// where $S_{sfm} = {\tau_{sfm} - \tau_{cr} \over \tau_{cr}}$ is the excess skin friction 
// shear stress, $fr$ is the volumetric fraction of a size class in the bed, $C_b$ is the
// bed concentration of the sediment, and $\gamma_0$ is the resuspension coefficient.
// The conditions of the excess skin-friction shear stress and Rouse parameter 
// (${w_s \over \kappa {u_\star}_{sfm}}$) ensure that the value calculated for $C_a$ will
// be zero during times of low energy.  Values cited for $\gamma_0$ range from $10^{-3}$
// to $10^{-5}$ (Drake and Cacchione, 1989).  For this implementation, a value of
// $\gamma_0=.002$ was used.  this value is cmaller than that used in similar 
// one-dimensional calculations to offset the fact that our calculations neglect density
// stratification of the water column by suspended sediments.  We take $z_a$ ro be the top
// of the bedload layer, approximately $3D_{50}$.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
//BOC
{
   double c_a;
   double k=VONKARMAN;
   double gamma_0 = RESUSPENSION_COEF;
   double rouse = w_s/(k*u_star);

   if ( s_sfm>0 && rouse<2.5 )
      c_a = fr*c_b*gamma_0*s_sfm/(1+gamma_0*s_sfm);
   else
      c_a = 0;

   return c_a;
}
//EOC


//BOP
// !ROUTINE: pat_get_bed_resuspension - get concentration gradient at the bed.
//
// !INTERFACE:

double pat_get_bed_resuspension( c_a , w_s , k_bed )

// !INPUT PARAMETERS:
   double c_a;    // equilibrium concentration near the bed.
   double w_s;    // sediment settling velocity
   double k_bed;  // vertical eddy diffusivity near the bed.

// !RETURN VALUE:
// the change in suspended sediment concentration (with depth) near the bed is returned.

// !DESCRIPTION:
// To solve the advection-diffusion equation we impose a flux condition on the bottom boundary.
// To apply this boundary condition to unsteady conditions, we assume that the entrainment 
// function, $E$, equals the reference concentration, $C_a$, estimated by,
// \begin{equation}
// C_a = \cases {
// {fr C_b \gamma_0 SS_{sfm} \over 1 + \gamma_0 S_{sfm}}, &S_{sfm}>0$ and {w_s \over \kappa {u_\star}_{sfm}}<2.5\cr
// 0, &otherwise }
// \end{equation}
// where $S_{sfm} = {\tau_{sfm} - \tau_{cr} \over \tau_{cr}}$ is the excess skin friction 
// shear stress, $fr$ is the volumetric fraction of a size class in the bed, $C_b$ is the
// bed concentration of the sediment, and $\gamma_0$ is the resuspension coefficient.  We
// then equate entrainment with upward mixing due to turbulence at a lebel $z_a$,
// \begin{equation}
// -K_z \left. { \partial C_s \over \partial z } \right|_{z_a} = E w_s = C_a w_s
// \end{equation}
// This function solves for $\left. {\partial C_s \over \partial z} \right|{z_a}$ to give
// us the lower (Neuman) boundary condition.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
//BOC
{
   double dcs_dz;

   dcs_dz = -c_a*w_s/k_bed;

   return dcs_dz;
}
//EOC

//BOP
// !ROUTINE: pat_get_mixing_depth - calculate the mixing depth of a bed.

// !INTERFACE:

double pat_get_mixing_depth( g , amount , n_grains , tau_sfm , period , c_b , lambda_rip )

// !INPUT PARAMETERS:
   Grain_class *g;     // grain classes in the bed.
   double *amount;     // amount of each grain class in the bed.
   int n_grains;       // the number of grain in the bed.
   double tau_sfm;     // skin friction shear stress at the ripple crest (maximum).
   double period;      // wave-period.
   double c_b;         // bulk concentration of the bed sediment.
   double lambda_rip;  // wavelength of the the bedforms.

// !DESCRIPTION:
// For each size class, the volume per unit bed area of sediment removed from the bed during any
// time step is limited by the amount available in the active layer.  The thickness of the active
// layer is calculated differently for sandy and silty beds.  For locations that contain both sands
// and silts, the depth of the active layer is estimated using a weighted average of the two 
// estimates of the mixing depth based on the fraction of the bed that is sand.  That is, the 
// mixing depth for a mixed bed, $\delta_{mix}$, is,
// \begin{equation}
// \delta_{mix} = \delta_{mix}^{(rip)} f_{sand} + \delta_{mix}^{(silt)}(1-f_{sand})
// \end{equation}
// where $\delta_{mix}^{(rip)}$ is the mixing depth for a sandy bed, $\delta_{mix}^{(silt)}$ is
// the mixing depth for a bed that is silty, and $f_{sand}$ is the fraction of sand in the bed.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
//BOC
{
   int n;
   double mix_ripp;
   Grain_class d_50;
   double f_sand;
   double *frac, sum;
   double mix_silt, mix_tot;

   frac = g_new( double , n_grains );
   for ( n=0,sum=0 ; n<n_grains ; n++ )
      sum += amount[n];

   if ( sum>0 )
   {
      for ( n=0,sum=0 ; n<n_grains ; n++ )
         amount[n] /= sum;
      f_sand   = average_grains( g , amount , n_grains , &d_50 );
      mix_ripp = pat_get_mixing_depth_ripp( g , frac , n_grains , tau_sfm , period , c_b , lambda_rip , d_50.phi );
      mix_silt = pat_get_mixing_depth_silt( tau_sfm , d_50.tau_cr , d_50.phi );
      mix_tot  = mix_ripp*f_sand + mix_silt*(1.-f_sand);
   }

   g_free( frac );

   return mix_tot;
}
//EOC

//BOP
// !ROUTINE: pat_get_mixing_depth_silt - calculate the mixing depth of a silty bed.
//
// !INTERFACE:

double pat_get_mixing_depth_silt( tau_sfm , tau_cr , d_50 )

// !INPUT PARAMETERS:
   double tau_sfm; // skin friction shear stress at the ripple crest (maximum).
   double tau_cr;  // critical shear stress for motion of the median grain size.
   double d_50;    // diameter of the median grain size.

// !DESCRIPTION:
// For silty beds, the depth of the active layer is assumed to be to be proportional to excess
// shear stress relative to the critical shear stress of the median grain size,
// \begin{equation}
// \delta_{mix}^{(silt)} = 8( \tau_{sfm} - {\tau_{cr}}_{50}) + 6D_{50}
// \end{equation}
// where $\tau_{sfm}$ is the skin friction shear stress at the ripple crest, ${\tau_{cr}}_{50}$
// is the critical shear stress of the median grain size, and $D_{50}$ is the diameter of the
// median grain size.
//
// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
//BOC
{
   return 8.*(tau_sfm - tau_cr) + 6.*d_50;
}
//EOC

//BOP
// !ROUTINE: pat_get_mixing_depth_ripp - calculate the mixing depth of a sandy bed.

// !INTERFACE:
double pat_get_mixing_depth_ripp( g , frac , n_grains , tau_sfm , period , c_b , lambda_rip , d_50 )

// !INPUT PARAMETERS:
   Grain_class *g;     // grain classes in the bed.
   double *frac;       // fraction of each grain class in the bed.
   int n_grains;       // number of grain classes in the bed.
   double tau_sfm;     // maximum (the ripple crest) skin friction shear stress of the bed.
   double period;      // wave-period.
   double c_b;         // bulk concentration of the bed sediment.
   double lambda_rip;  // wavelength of the the bedforms.
   double d_50;        // diameter of the median grain size in the bed.

// !DESCRIPTION:
// For sandy beds, the depth of the active layer is related to the depth of the bed that is mixed by
// migrating ripples or sheet flow.  The depth of sediment transport on sandy beds is estimated 
// using an expression based on the migration of a bedform of a bedform over the time scale of a 
// half-wave period,
// \begin{equation}
// \delta_{mix}^{(rip)} = {Q_b T \over 2 C_b \lambda_{rip} } + 6D_{50}
// \end{equation}
// where $Q_b$ is the volumetric bedload transport rate, $T$ is the half-wave period, $C_b$ is the
// bed concentration, $\lambda_{rip}$ is the bedform wavelength, and $D_{50}$ is the diameter of the
// median grain size.

// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
//BOC
{
   int n;
   double q_b=0;
   for ( n=0 ; n<n_grains ; n++ )
//      q_b += frac[n]*pat_get_bedload_transport_rate( g[n].rho , sed_rho_sea_water() , tau_sfm , g[n].tau_cr );
      q_b += frac[n]*pat_get_bedload_transport_rate( sed_rho_quartz() , sed_rho_sea_water() , tau_sfm , g[n].tau_cr );
   return q_b*period/(2*c_b*lambda_rip) + 6.*d_50;
}
//EOC

//BOP
// !ROUTINE: pat_get_bedload_transport_rate - calculate the bedload transport rate for a grain class.
//
// !INTERFACE:

double pat_get_bedload_transport_rate( grain_density , water_density , tau_sfm , tau_cr )

// !INPUT PARAMETERS:
   double grain_density; // the grain density of the sediment.
   double water_density; // the density of the fluid.
   double tau_sfm;       // shear stress at the ripple crest.
   double tau_cr;        // critical shear stress for motion.

// !RETURN VALUE:
// the bedload transport rate.

// !DESCRIPTION:
// The volumetric bedload transport rate for a size class $l$ is estimated by,
// \begin{equation}
// {Q_b}_l = 8.0 { (\tau_{sfm}-\tau_{cr} )^{1.5} \over (\rho_s-\rho)g }
// \end{equation}
// where $\tau_{sfm}$ is the skin friction shear stress at the ripple crest, $tau_{cr}$ is the
// critical shear stress for motion, $\rho_s$ is the grain density, and $\rho$ is the density
// of sea water.  To calculate the bedload transport rate for a mixed bed we simply average over
// the seperate grain classes,
// \begin{equation}
// Q_b = \sum_l f_l {Q_b}_l
// \end{equation}
// where $f_l$ is the fraction of grain class $l$ in the bed.  This is taken from Wiberg and
// Harris.
//
// !REVISION HISTORY:
// Apr 2002  Eric Hutton      Initial version.
//EOP
//BOC
{
   return 8.*pow( tau_sfm - tau_cr , 1.5 ) / ((grain_density - water_density)*sed_gravity());
}
//EOC

