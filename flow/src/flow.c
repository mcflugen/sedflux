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

#include "flow.h"

double *solve_excess_pore_pressure( double *psi , double *k , double *c , int n , double dz , double dt , double psi_top , double sed_rate )
{
   double *l = eh_new( double , n );
   double *d = eh_new( double , n );
   double *u = eh_new( double , n );
   double *b = eh_new( double , n );
   int i;

   // given these new k and c, calculate the new entries for our
   // matrix equation that solves for psi.  here we must use psi
   // at the previous time step.
   get_matrix_coefficients( psi+1 , k+1     , c+1 , sed_rate , dz ,
                            dt    , psi_top , n-2 , 1.       , l  ,
                            d     , u       , b );

   // solve the tridiagonal system for the new psi.
   if ( !tridiag( l , d , u , b , psi+1 , n-2 ) )
   {
      eh_watch_dbl( d[0] );
      for ( i=0 ; i<n ; i++ )
      {
         eh_watch_dbl( psi[i] );
         psi[i] = psi_top;
      }
      perror( "tridiag" );
   }

   psi[0]   = psi[2];
   psi[n-1] = psi_top;

   eh_free( l );
   eh_free( d );
   eh_free( u );
   eh_free( b );

   return psi;
}

//BOP
//
// !ROUTINE: get_matrix_coefficients - calculate matrix coefficents for the unsaturated flow problem.
//
// !INTERFACE:
//
// void get_matrix_coefficients( double *psi , double *k , double *c ,
//                               double r , double dz , double dt ,
//                               int n , double f , double *l ,
//                               double *d , double *u , double *b )
//
// !INPUT PARAMETERS:
//
// psi    the hydraulic head at the previous time step (cm).
// k      the hydraulic conductivity at the next time step (cm/min).
// c      the specific moisture capacity at the next time step (1/cm).
// r      rain rate at the top of the sediment column (cm/min).
// dz     the vertical node spacing (cm).
// dt     the time step (min)
// n      the lengh of the input/output vectors.
// f      fraction between 0 and 1 to indicate the numerical method used.
//        use f=0 for fully explicit, f=.5 for crank-nicholson, and f=1
//        for fully implicit.
// l      the lower diagonal entries of the system matrix (output).
// d      the diagonal entries of the system matrix (output).
// u      the upper diagonal entries of the system matrix (output).
// b      the right hand side of the matrix equation (output).
//
// !DESCRIPTION:
// form the matrices in our matrix equation to solve for the hydraulic
// head distribution.  our matrix equation is,
// $$ [ \rm A ] \{ \psi \} = \{ \rm b \} $$
// the matrix $\rm A$ is the tridiagonal matrix,
// $$ \left[ \matrix{
// 1   & 0 \cr
// l_2 & d_2    & u_2 \cr
//    & \ddots & \ddots  & \ddots \cr
//    &        & l_{n-1} & d_{n-1} & u_{n-1} \cr
//    &        &         & -1      & 1
// } \right] $$
// where,
// $$ \matrix{
// l_i &= -\alpha \left( K_{i-1} + K_i \right) \cr
// d_i &= \alpha \left( K_{i-1} + 2K_i + K_{i+1} \right) + 2 \gamma C_i\cr
// u_i &= -\alpha \left( K_i + K_{i+1} \right)
// }$$
// the column vector, $\rm b$ is,
// $$ \{ \rm b \} = \left\{ \matrix{ 
// \psi_0 \cr
// b_2\cr
// \vdots \cr
// b_{n-1} \cr 
// \Delta z \left( {R\over K_n} - 1 \right)
// }\right\} $$
// where,
// $$ b_i = \beta \left( K_{i-1} + K_i \right) \psi_{i-1}^k - \left( \beta \left( K_{i-1} + 2K_i + K_{i+1} \right) - 2\gamma C_i  \right) \psi_i^k + \beta \left( K_i + K_{i+1} \right) \psi_{i+1}^k + \Delta z \left( K_{i+1} - K_{i-1} \right) $$
// where $\beta = 1-\alpha$, and $\gamma = { (\Delta z)^2 \over \Delta t}$.
//
// !REVISION HISTORY:
// feb 2002 eric hutton initial version.
//
//EOP
//BOC
void get_matrix_coefficients( double *psi , double *k , double *c , double ds , double dz , double dt , double psi_top , int n , double f ,  double *l , double *d , double *u , double *b )
{
   int i;

   d[0] = -f*2*k[0]/(dz*dz) - c[0]/dt;
   u[0] =  f*2*k[0]/(dz*dz);
   for ( i=1 ; i<n-1 ; i++ )
   {
      l[i] =  f/(dz*dz)*(.5*k[i-1]+k[i]-.5*k[i+1]);
      d[i] = -f/(dz*dz)*2*k[i] - c[i]/dt;
      u[i] =  f/(dz*dz)*(-.5*k[i-1]+k[i]+.5*k[i+1]);
   }
   l[n-1] =  f/(dz*dz)*(k[n-1]-.5*k[n]+.5*k[n-2]);
   d[n-1] = -f/(dz*dz)*2*k[n-1] - c[n-1]/dt;

   b[0] = -(1.-f)/(dz*dz)
        * (   psi[1]*(.5*k[-1]+k[0]-.5*k[1])
            - psi[0]  *(2*k[0])
            + psi[1]*(-.5*k[-1]+k[0]+.5*k[1]) ) 
        - psi[0]*c[0]/dt;
   for ( i=1 ; i<n-1 ; i++ )
   {
      b[i] = -(1.-f)/(dz*dz)
           * (   psi[i-1]*(.5*k[i-1]+k[i]-.5*k[i+1])
               - psi[i]  *(2*k[i])
               + psi[i+1]*(-.5*k[i-1] + k[i] + .5*k[i+1]) ) 
           - psi[i]*c[i]/dt;
   }
   b[n-1] = -(1.-f)/(dz*dz)
           * (   psi[n-2]*(.5*k[n-2]+k[n-1]-.5*k[n])
               - psi[n-1]  *(2*k[n-1])
               + psi_top*(-.5*k[n-2] + k[n-1] + .5*k[n]) ) 
           - psi[n-1]*c[n-1]/dt
           - f*psi_top/(dz*dz)*(k[n-1]+.5*k[n]-.5*k[n-2]);

}
//EOC

double *solve_excess_pore_pressure_mg( double *psi , double *k , double *c , int n , double dz , double dt , double sed_rate )
{
   int i;
   double *f = eh_new( double , n );
   double depth = dz*(n-1);

   for ( i=0 ; i<n ; i++ )
      f[i] = (-sed_rate/dt - psi[i]/dt);

   for ( i=0 ; i<2 ; i++ )
      fmg_1d( psi , k , f , n , depth , dt );

   eh_free( f );

   return psi;
}

double **solve_excess_pore_pressure_mg_2d( double **psi , double **kx , double **kz , double **c , int n , double dx , double dz , double dt , double *sed_rate )
{
   int i, j;
   double **f = allocate_2d( n );
   double depth = dz*(n-1);
   double width = dx*(n-1);

   for ( i=0 ; i<n ; i++ )
      for ( j=0 ; j<n ; j++ )
         f[i][j] = (-sed_rate[i]/dt - psi[i][j]/dt);

   for ( i=0 ; i<5 ; i++ )
      fmg_2d( psi , kx , kz , f , n , width , depth , dt );

   eh_free( f[0] );
   eh_free( f    );

   return psi;
}

double ***solve_excess_pore_pressure_mg_3d( double ***psi , double ***kx , double ***kz , double ***c , int n , double dx , double dz , double dt , double **sed_rate )
{
   int i, j, k;
   double ***f = allocate_3d( n );
   double depth = dz*(n-1);
   double width = dx*(n-1);

   for ( i=0 ; i<n ; i++ )
      for ( j=0 ; j<n ; j++ )
         for ( k=0 ; k<n ; k++ )
            f[i][j][k] = (-sed_rate[i][j]/dt - psi[i][j][k]/dt);

   for ( i=0 ; i<5 ; i++ )
      fmg_3d( psi , kx , kz , f , n , width , depth , dt );

   free_3d( f );

   return psi;
}


double *restrict_1d( double *u_2h , double *u_h , int n_h )
{
   int i, i_2h;
   int n_2h = (n_h+1)/2.;

   for ( i_2h=1,i=2 ; i_2h<n_2h-1 ; i_2h++,i+=2 )
      u_2h[i_2h] = .5*u_h[i] + .25*(u_h[i-1] + u_h[i+1]);

   u_2h[0]      = u_h[0];
   u_2h[n_2h-1] = u_h[n_h-1];

   return u_2h;
}

double **restrict_2d( double **u_2h , double **u_h , int n_h )
{
   int i, i_2h;
   int j, j_2h;
   int n_2h = (n_h+1)/2.;

   for ( i_2h=1,i=2 ; i_2h<n_2h-1 ; i_2h++,i+=2 )
      for ( j_2h=1,j=2 ; j_2h<n_2h-1 ; j_2h++,j+=2 )
         u_2h[i_2h][j_2h] = .5*u_h[i][j] + .125*(u_h[i-1][j] + u_h[i+1][j] + u_h[i][j-1] + u_h[i][j+1]);

   for ( j_2h=0,j=0 ; j_2h<n_2h ; j_2h++,j+=2 )
   {
      u_2h[0][j_2h]      = u_h[0][j];
      u_2h[n_2h-1][j_2h] = u_h[n_h-1][j];
   }

   for ( i_2h=0,i=0 ; i_2h<n_2h ; i_2h++,i+=2 )
   {
      u_2h[i_2h][0]      = u_h[i][0];
      u_2h[i_2h][n_2h-1] = u_h[i][n_h-1];
   }

   return u_2h;
}

double ***restrict_3d( double ***u_2h , double ***u_h , int n_h )
{
   int i, i_2h;
   int j, j_2h;
   int k, k_2h;
   int n_2h = (n_h+1)/2.;

   for ( i_2h=1,i=2 ; i_2h<n_2h-1 ; i_2h++,i+=2 )
      for ( j_2h=1,j=2 ; j_2h<n_2h-1 ; j_2h++,j+=2 )
         for ( k_2h=1,k=2 ; k_2h<n_2h-1 ; k_2h++,k+=2 )
            u_2h[i_2h][j_2h][k_2h] = u_h[i][j][k]/2 
                                      + (   u_h[i-1][j][k] + u_h[i+1][j][k] 
                                          + u_h[i][j-1][k] + u_h[i][j+1][k] 
                                          + u_h[i][j][k-1] + u_h[i][j][k+1] )/12;

   for ( i_2h=0,i=0 ; i_2h<n_2h ; i_2h++,i+=2 )
   {
      for ( j_2h=0,j=0 ; j_2h<n_2h ; j_2h++,j+=2 )
      {
         u_2h[i_2h][j_2h][0]      = u_h[i][j][0];
         u_2h[i_2h][j_2h][n_2h-1] = u_h[i][j][n_h-1];

         u_2h[i_2h][n_2h-1][j_2h] = u_h[i][n_h-1][j];
         u_2h[i_2h][0][j_2h]      = u_h[i][0][j];

         u_2h[n_2h-1][i_2h][j_2h] = u_h[n_h-1][i][j];
         u_2h[0][i_2h][j_2h]      = u_h[0][i][j];
      }
   }

   return u_2h;
}

double *inter_1d( double *u_h , double *u_2h , int n_2h )
{
   int i, i_h;
   int n_h=n_2h*2-1;

   for ( i_h=0,i=0 ; i_h<n_h ; i_h+=2,i++ )
      u_h[i_h] = u_2h[i];

   for ( i_h=1,i=0 ; i_h<n_h ; i_h+=2,i++ )
      u_h[i_h] = .5*(u_2h[i] + u_2h[i+1]);

   return u_h;
}

double **inter_2d( double **u_h , double **u_2h , int n_2h )
{
   int i, i_h;
   int j, j_h;
   int n_h=n_2h*2-1;

   for ( i_h=0,i=0 ; i_h<n_h ; i_h+=2,i++ )
      for ( j_h=0,j=0 ; j_h<n_h ; j_h+=2,j++ )
         u_h[i_h][j_h] = u_2h[i][j];

   for ( i_h=1,i=0 ; i_h<n_h ; i_h+=2,i++ )
      for ( j_h=0,j=0 ; j_h<n_h ; j_h+=2,j++ )
         u_h[i_h][j_h] = .5*(u_2h[i][j] + u_2h[i+1][j]);

   for ( i=0 ; i<n_h ; i++ )
      for ( j=1 ; j<n_h ; j+=2 )
         u_h[i][j] = .5*(u_h[i][j-1] + u_h[i][j+1]);

   return u_h;
}

void print_matrix( double **a , int n );
void print_matrix_3d( double ***a , int n );

double ***inter_3d( double ***u_h , double ***u_2h , int n_2h )
{
   int i, i_h;
   int j, j_h;
   int k, k_h;
   int n_h=n_2h*2-1;

   for ( i=0 ; i<n_h ; i++ )
      for ( j=0 ; j<n_h ; j++ )
         for ( k=0 ; k<n_h ; k++ )
            u_h[i][j][k] = -1;

   for ( i_h=0,i=0 ; i_h<n_h ; i_h+=2,i++ )
      for ( j_h=0,j=0 ; j_h<n_h ; j_h+=2,j++ )
         for ( k_h=0,k=0 ; k_h<n_h ; k_h+=2,k++ )
            u_h[i_h][j_h][k_h] = u_2h[i][j][k];

   for ( k_h=0,k=0 ; k_h<n_h ; k_h+=2,k++ )
   {

      for ( i_h=1,i=0 ; i_h<n_h ; i_h+=2,i++ )
         for ( j_h=0,j=0 ; j_h<n_h ; j_h+=2,j++ )
            u_h[i_h][j_h][k_h] = .5*(u_2h[i][j][k] + u_2h[i+1][j][k]);

      for ( i=0 ; i<n_h ; i++ )
         for ( j=1 ; j<n_h ; j+=2 )
            u_h[i][j][k_h] = .5*(u_h[i][j-1][k_h] + u_h[i][j+1][k_h]);

   }

   for ( i=0 ; i<n_h ; i++ )
      for ( j=0 ; j<n_h ; j++ )
         for ( k=1 ; k<n_h ; k+=2 )
            u_h[i][j][k] = .5*(u_h[i][j][k-1] + u_h[i][j][k+1]);

   return u_h;
}

double *relax_1d( double *u , double *k , double *f , int n , double dz , double dt )
{
   int i;
   double h=1./(n-1)*dz;
   double h_2 = h*h;
   double u0 = u[n-1];

   for ( i=1 ; i<n-1 ; i++ )
      u[i] = ( (1/h_2)*( u[i-1]*k[i] + u[i+1]*k[i+1] ) - f[i] )*h_2/( k[i+1] + k[i] + h_2/dt );

   u[0]   = u[1];
   u[n-1] = u0;

   return u;
}

double **relax_2d( double **u , double **kx , double **kz , double **f , int n , double dx , double dz , double dt )
{
   int i, j;
   double hx=1./(n-1)*dx;
   double hz=1./(n-1)*dz;
   double hx_2 = hx*hx;
   double hz_2 = hz*hz;

   for ( i=1 ; i<n-1 ; i++ )
      for ( j=1 ; j<n-1 ; j++ )
         u[i][j] = (  ( u[i-1][j]*kx[i][j] + u[i+1][j]*kx[i+1][j] )/hx_2
                    + ( u[i][j-1]*kz[i][j] + u[i][j+1]*kz[i][j+1] )/hz_2
                    - f[i][j] )
                  /( (kx[i][j]+kx[i+1][j])/hx_2 + (kz[i][j]+kz[i][j+1])/hz_2 + 1/dt);


   for ( i=0 ; i<n ; i++ )
   {
      u[0][i]   = u[1][i];
      u[n-1][i] = u[n-2][i];
      u[i][0]   = u[i][1];
//      u[i][n-1] = u[i][n-2];
   }

   return u;
}

double ***relax_3d( double ***u , double ***kx , double ***kz , double ***f , int n , double dx , double dz , double dt )
{
   int i, j, k;
   double hx=1./(n-1)*dx;
   double hz=1./(n-1)*dz;
   double hx_2 = hx*hx;
   double hz_2 = hz*hz;

   for ( i=1 ; i<n-1 ; i++ )
      for ( j=1 ; j<n-1 ; j++ )
         for ( k=1 ; k<n-1 ; k++ )
            u[i][j][k] = (  ( u[i-1][j][k]*kx[i][j][k] + u[i+1][j][k]*kx[i+1][j][k] )/hx_2
                          + ( u[i][j-1][k]*kx[i][j][k] + u[i][j+1][k]*kx[i][j+1][k] )/hx_2
                          + ( u[i][j][k-1]*kz[i][j][k] + u[i][j][k+1]*kz[i][j][k+1] )/hz_2
                          - f[i][j][k] )
                        /( (kx[i][j][k]+kx[i+1][j][k])/hx_2 + (kx[i][j][k]+kx[i][j+1][k])/hx_2 + (kz[i][j][k]+kz[i][j][k+1])/hz_2 + 1/dt);

   for ( i=0 ; i<n ; i++ )
      for ( j=0 ; j<n ; j++ )
      {
         u[0][i][j]   = u[1][i][j];
         u[n-1][i][j] = u[n-2][i][j];

         u[i][0][j]   = u[i][1][j];
         u[i][n-1][j] = u[i][n-2][j];

         u[i][j][0]   = u[i][j][1];
//         u[i][j][n-1] = u[i][j][n-2];
      }

   return u;
}

double *residual_1d( double *res , double *u , double *k , double *f , int n , double dz , double dt )
{
   int i;
   double h=1./(n-1)*dz;
   double h_2 = h*h;

   for ( i=1 ; i<n-1 ; i++ )
      res[i] = -( u[i-1]*k[i] + u[i+1]*k[i+1] - u[i]*(k[i+1] + k[i] + h_2/dt) )/h_2 + f[i];

   res[0]   = f[0];
   res[n-1] = 0;

   return res;
}

double **residual_2d( double **res , double **u , double **kx , double **kz , double **f , int n , double dx , double dz , double dt )
{
   int i, j;
   double hx=1./(n-1)*dx;
   double hz=1./(n-1)*dz;
   double hx_2 = hx*hx;
   double hz_2 = hz*hz;

   for ( i=1 ; i<n-1 ; i++ )
      for ( j=1 ; j<n-1 ; j++ )
         res[i][j] = - ( u[i-1][j]*kx[i][j] + u[i+1][j]*kx[i+1][j] )/hx_2
                     - ( u[i][j-1]*kz[i][j] + u[i][j+1]*kz[i][j+1] )/hz_2
                     + u[i][j]*( kx[i][j]+kx[i+1][j] )/hx_2
                     + u[i][j]*( kz[i][j]+kz[i][j+1] )/hz_2
                     + u[i][j]/dt
                     + f[i][j];

   for ( i=0 ; i<n ; i++ )
   {
      res[0][i]   = f[0][i];
      res[n-1][i] = f[n-1][i];
      res[i][0]   = f[i][0];
      res[i][n-1] = 0;
   }

   return res;
}

double ***residual_3d( double ***res , double ***u , double ***kx , double ***kz , double ***f , int n , double dx , double dz , double dt )
{
   int i, j, k;
   double hx=1./(n-1)*dx;
   double hz=1./(n-1)*dz;
   double hx_2 = hx*hx;
   double hz_2 = hz*hz;

   for ( i=1 ; i<n-1 ; i++ )
      for ( j=1 ; j<n-1 ; j++ )
         for ( j=1 ; j<n-1 ; j++ )
            res[i][j][k] = - ( u[i-1][j][k]*kx[i][j][k] + u[i+1][j][k]*kx[i+1][j][k] )/hx_2
                           - ( u[i][j-1][k]*kx[i][j][k] + u[i][j+1][k]*kx[i][j+1][k] )/hx_2
                           - ( u[i][j][k-1]*kz[i][j][k] + u[i][j][k+1]*kz[i][j][k+1] )/hz_2
                           + u[i][j][k]*( kx[i][j][k]+kx[i+1][j][k] )/hx_2
                           + u[i][j][k]*( kx[i][j][k]+kx[i][j+1][k] )/hx_2
                           + u[i][j][k]*( kz[i][j][k]+kz[i][j][k+1] )/hz_2
                           + u[i][j][k]/dt
                           + f[i][j][k];

   for ( i=0 ; i<n ; i++ )
      for ( j=0 ; j<n ; j++ )
      {
         res[0][i][j]   = f[0][i][j];
         res[n-1][i][j] = f[n-1][i][j];

         res[i][0][j]   = f[i][0][j];
         res[i][n-1][j] = f[i][n-1][j];

         res[i][j][0]   = f[i][j][0];
         res[i][j][n-1] = 0;
      }

   return res;
}

double *solve_1d( double *u , double *k , double *f , double dz , double dt )
{
   double h=1./(3-1)*dz;
   double h_2 = h*h;

   u[1] = ( (1/h_2)*( u[0]*k[0] + u[2]*k[2] ) - f[1] )*h_2/( k[2] + k[1] + h_2/dt );

   return u;
}

double **solve_2d( double **u , double **kx , double **kz , double **f , double dx , double dz , double dt )
{
   double hx=1./(3-1)*dx;
   double hz=1./(3-1)*dz;
   double hx_2 = hx*hx;
   double hz_2 = hz*hz;
   int i, j;

   i = 1;
   j = 1;
   u[i][j] = (  ( u[i-1][j]*kx[i][j] + u[i+1][j]*kx[i+1][j] )/hx_2
              + ( u[i][j-1]*kz[i][j] + u[i][j+1]*kz[i][j+1] )/hz_2
              - f[i][j] )
            /( (kx[i][j]+kx[i+1][j])/hx_2 + (kz[i][j]+kz[i][j+1])/hz_2 + 1/dt);
   return u;
}

double ***solve_3d( double ***u , double ***kx , double ***kz , double ***f , double dx , double dz , double dt )
{
   double hx=1./(3-1)*dx;
   double hz=1./(3-1)*dz;
   double hx_2 = hx*hx;
   double hz_2 = hz*hz;
   int i, j, k;

   i = 1;
   j = 1;
   k = 1;
   u[i][j][k] = (  ( u[i-1][j][k]*kx[i][j][k] + u[i+1][j][k]*kx[i+1][j][k] )/hx_2
                 + ( u[i][j-1][k]*kx[i][j][k] + u[i][j+1][k]*kx[i][j+1][k] )/hx_2
                 + ( u[i][j][k-1]*kz[i][j][k] + u[i][j][k+1]*kz[i][j][k+1] )/hz_2
                 - f[i][j][k] )
               /( (kx[i][j][k]+kx[i+1][j][k])/hx_2 + (kx[i][j][k]+kx[i][j+1][k])/hx_2 + (kz[i][j][k]+kz[i][j][k+1])/hz_2 + 1/dt);
   return u;
}

double *add_inter_1d( double *u_h , double *u_2h , int n_2h )
{
   int i, n_h=2*n_2h-1;
   double *r_h = eh_new( double , n_h );

   inter_1d( r_h , u_2h , n_2h );

   for ( i=0 ; i<n_h ; i++ )
      u_h[i] = u_h[i] + r_h[i];

   eh_free( r_h );

   return u_h;
}

double **add_inter_2d( double **u_h , double **u_2h , int n_2h )
{
   int i, j, n_h=2*n_2h-1;
   double **r_h;

   r_h = allocate_2d( n_h );

   inter_2d( r_h , u_2h , n_2h );

   for ( i=0 ; i<n_h ; i++ )
      for ( j=0 ; j<n_h ; j++ )
         u_h[i][j] = u_h[i][j] + r_h[i][j];

   eh_free( r_h[0] );
   eh_free( r_h    );

   return u_h;
}

double ***add_inter_3d( double ***u_h , double ***u_2h , int n_2h )
{
   int i, j, k, n_h=2*n_2h-1;
   double ***r_h;

   r_h = allocate_3d( n_h );

   inter_3d( r_h , u_2h , n_2h );

   for ( i=0 ; i<n_h ; i++ )
      for ( j=0 ; j<n_h ; j++ )
         for ( k=0 ; k<n_h ; k++ )
            u_h[i][j][k] = u_h[i][j][k] + r_h[i][j][k];

   free_3d( r_h );

   return u_h;
}

double *mgm_1d( double *u_h , double *k_h , double *f_h , int n_h , double dz , double dt )
{
   int n_2h = (n_h+1)/2.;
   double *u_2h, *k_2h, *f_2h, *r_h;

   u_2h = eh_new0( double , n_2h );
   k_2h = eh_new ( double , n_2h );
   f_2h = eh_new ( double , n_2h );
   r_h  = eh_new ( double , n_h  );

   relax_1d( u_h , k_h , f_h , n_h , dz , dt );
   residual_1d( r_h , u_h , k_h , f_h , n_h , dz , dt );

   restrict_1d( f_2h , r_h , n_h );
   restrict_1d( k_2h , k_h , n_h );

   if ( n_2h > 3 )
      mgm_1d( u_2h , k_2h , f_2h , n_2h , dz , dt );
   else
      solve_1d( u_2h , k_2h , f_2h , dz , dt );

   add_inter_1d( u_h , u_2h , n_2h );

   relax_1d( u_h , k_h , f_h , n_h , dz , dt );

   eh_free( u_2h );
   eh_free( k_2h );
   eh_free( f_2h ); 
   eh_free( r_h  ); 

   return u_h;
}

#include "utils.h"

double **mgm_2d( double **u_h , double **kx_h , double **kz_h , double **f_h , int n_h , double dx , double dz , double dt )
{
   int i, j, n_2h = (n_h+1)/2.;
   double **u_2h, **kx_2h, **kz_2h , **f_2h, **r_h;

   u_2h  = allocate_2d( n_2h );
   kx_2h = allocate_2d( n_2h );
   kz_2h = allocate_2d( n_2h );
   f_2h  = allocate_2d( n_2h );
   r_h   = allocate_2d( n_h  );

   for ( i=0 ; i<n_2h ; i++ )
      for ( j=0 ; j<n_2h ; j++ )
         u_2h[i][j] = 0;

   relax_2d( u_h , kx_h , kz_h , f_h , n_h , dx , dz , dt );
   residual_2d( r_h , u_h , kx_h , kz_h , f_h , n_h , dx , dz , dt );

   restrict_2d( f_2h , r_h , n_h );
   restrict_2d( kx_2h , kx_h , n_h );
   restrict_2d( kz_2h , kz_h , n_h );

   if ( n_2h > 3 )
      mgm_2d( u_2h , kx_2h , kz_2h , f_2h , n_2h , dx , dz , dt );
   else
      solve_2d( u_2h , kx_2h , kz_2h , f_2h , dx , dz , dt );

   add_inter_2d( u_h , u_2h , n_2h );

   relax_2d( u_h , kx_h , kz_h , f_h , n_h , dx , dz , dt );

   eh_free( u_2h[0]  );
   eh_free( kx_2h[0] );
   eh_free( kz_2h[0] );
   eh_free( f_2h[0]  ); 
   eh_free( r_h[0]   ); 

   eh_free( u_2h );
   eh_free( kx_2h );
   eh_free( kz_2h );
   eh_free( f_2h ); 
   eh_free( r_h  ); 

   return u_h;
}

double ***mgm_3d( double ***u_h , double ***kx_h , double ***kz_h , double ***f_h , int n_h , double dx , double dz , double dt )
{
   int i, j, k, n_2h = (n_h+1)/2.;
   double ***u_2h, ***kx_2h, ***kz_2h , ***f_2h, ***r_h;

   u_2h  = allocate_3d( n_2h );
   kx_2h = allocate_3d( n_2h );
   kz_2h = allocate_3d( n_2h );
   f_2h  = allocate_3d( n_2h );
   r_h   = allocate_3d( n_h  );

   for ( i=0 ; i<n_2h ; i++ )
      for ( j=0 ; j<n_2h ; j++ )
         for ( k=0 ; k<n_2h ; k++ )
            u_2h[i][j][k] = 0;

   relax_3d( u_h , kx_h , kz_h , f_h , n_h , dx , dz , dt );
   residual_3d( r_h , u_h , kx_h , kz_h , f_h , n_h , dx , dz , dt );

   restrict_3d( f_2h , r_h , n_h );
   restrict_3d( kx_2h , kx_h , n_h );
   restrict_3d( kz_2h , kz_h , n_h );

   if ( n_2h > 3 )
      mgm_3d( u_2h , kx_2h , kz_2h , f_2h , n_2h , dx , dz , dt );
   else
      solve_3d( u_2h , kx_2h , kz_2h , f_2h , dx , dz , dt );

   add_inter_3d( u_h , u_2h , n_2h );

   relax_3d( u_h , kx_h , kz_h , f_h , n_h , dx , dz , dt );

   free_3d( u_2h );
   free_3d( kx_2h );
   free_3d( kz_2h );
   free_3d( f_2h ); 
   free_3d( r_h  ); 

   return u_h;
}

#include "utils.h"

double *fmg_1d( double *u_h , double *k_h , double *f_h , int n_h , double dz , double dt )
{
   int i, n, n_grid = log2( n_h-1 );
   double **u = eh_new( double* , n_grid );
   double **k = eh_new( double* , n_grid );
   double **f = eh_new( double* , n_grid );
   double *u_2h = eh_new( double , n_h );

   u[n_grid-1] = u_h;
   k[n_grid-1] = k_h;
   f[n_grid-1] = f_h;

   n = n_h;

   for ( i=n_grid-2 ; i>=0 ; i-- )
   {
      n = (n+1)/2;

      u[i] = eh_new( double , n );
      k[i] = eh_new( double , n );
      f[i] = eh_new( double , n );

      restrict_1d( u[i] , u[i+1] , n*2-1 );
      restrict_1d( k[i] , k[i+1] , n*2-1 );
      restrict_1d( f[i] , f[i+1] , n*2-1 );
   }

   memcpy( u_2h , u[0] , sizeof(double)*n );

   for ( i=1 ; i<n_grid ; i++ )
   {

      inter_1d( u_h , u_2h , n );
      n = n*2-1;

      u_h[0]   = u_h[1];
      u_h[n-1] = u[i][n-1];

      mgm_1d( u_h , k[i] , f[i] , n , dz , dt );

      memcpy( u_2h , u_h , sizeof(double)*n );

   }

   return u_h;
}


double **fmg_2d( double **u_h , double **kx_h , double **kz_h , double **f_h , int n_h , double dx , double dz , double dt )
{
   int i, j, l, n, n_grid = log2( n_h-1 );
   double ***u  = eh_new( double** , n_grid );
   double ***kx = eh_new( double** , n_grid );
   double ***kz = eh_new( double** , n_grid );
   double ***f  = eh_new( double** , n_grid );
   double **u_2h = allocate_2d( n_h );

   u[n_grid-1] = allocate_2d( n_h );
   memcpy( u[n_grid-1][0] , u_h[0] , sizeof(double)*n_h*n_h );
   kx[n_grid-1] = allocate_2d( n_h );
   memcpy( kx[n_grid-1][0] , kx_h[0] , sizeof(double)*n_h*n_h );
   kz[n_grid-1] = allocate_2d( n_h );
   memcpy( kz[n_grid-1][0] , kz_h[0] , sizeof(double)*n_h*n_h );
   f[n_grid-1] = allocate_2d( n_h );
   memcpy( f[n_grid-1][0] , f_h[0] , sizeof(double)*n_h*n_h );

   n = n_h;

   for ( i=n_grid-2 ; i>=0 ; i-- )
   {
      n = (n+1)/2;

      u[i]  = allocate_2d( n );
      kx[i] = allocate_2d( n );
      kz[i] = allocate_2d( n );
      f[i]  = allocate_2d( n );

      restrict_2d( u[i]  , u[i+1]  , n*2-1 );
      restrict_2d( kx[i] , kx[i+1] , n*2-1 );
      restrict_2d( kz[i] , kz[i+1] , n*2-1 );
      restrict_2d( f[i]  , f[i+1]  , n*2-1 );
   }

   for (j=0;j<n;j++)
      for (l=0;l<n;l++)
         u_2h[j][l] = u[0][j][l];

   for ( i=1 ; i<n_grid ; i++ )
   {

      inter_2d( u_h , u_2h , n );
      n = n*2-1;

      for (j=0;j<n;j++)
      {
         u_h[j][n-1] = u[i][j][n-1];

         u_h[0][j]   = u_h[1][j];
         u_h[n-1][j] = u_h[n-2][j];
         u_h[j][0]   = u_h[j][1];
      }

      mgm_2d( u_h , kx[i] , kz[i] , f[i] , n , dx , dz , dt );

      for (j=0;j<n;j++)
         for (l=0;l<n;l++)
            u_2h[j][l] = u_h[j][l];

   }

   for ( i=0 ; i<n_grid ; i++ )
   {
      free_2d( u[i]  );
      free_2d( kx[i] );
      free_2d( kz[i] );
      free_2d( f[i]  );
   }

   return u_h;
}

void print_matrix_3d( double ***a , int n );

double ***fmg_3d( double ***u_h , double ***kx_h , double ***kz_h , double ***f_h , int n_h , double dx , double dz , double dt )
{
   int i, j, k, l, m, n, n_grid = log2( n_h-1 );
   double ****u  = eh_new( double*** , n_grid );
   double ****kx = eh_new( double*** , n_grid );
   double ****kz = eh_new( double*** , n_grid );
   double ****f  = eh_new( double*** , n_grid );
   double ***u_2h = allocate_3d( n_h );

   u [n_grid-1] = allocate_3d( n_h );
   kx[n_grid-1] = allocate_3d( n_h );
   kz[n_grid-1] = allocate_3d( n_h );
   f [n_grid-1] = allocate_3d( n_h );

   for ( i=0 ; i<n_h ; i++ )
   {
      for ( j=0 ; j<n_h ; j++ )
      {
         for ( k=0 ; k<n_h ; k++ )
         {
            u [n_grid-1][i][j][k] = u_h [i][j][k];
            kx[n_grid-1][i][j][k] = kx_h[i][j][k];
            kz[n_grid-1][i][j][k] = kz_h[i][j][k];
            f [n_grid-1][i][j][k] = f_h [i][j][k];
         }
      }
   }

   n = n_h;

   for ( i=n_grid-2 ; i>=0 ; i-- )
   {
      n = (n+1)/2;

      u[i]  = allocate_3d( n );
      kx[i] = allocate_3d( n );
      kz[i] = allocate_3d( n );
      f[i]  = allocate_3d( n );

      restrict_3d( u[i]  , u[i+1]  , n*2-1 );
      restrict_3d( kx[i] , kx[i+1] , n*2-1 );
      restrict_3d( kz[i] , kz[i+1] , n*2-1 );
      restrict_3d( f[i]  , f[i+1]  , n*2-1 );

//print_matrix_3d( u[i] , n );
   }

   for (j=0;j<n;j++)
      for (l=0;l<n;l++)
         for (m=0;m<n;m++)
            u_2h[j][l][m] = u[0][j][l][m];

   for ( i=1 ; i<n_grid ; i++ )
   {

//print_matrix_3d( u_2h , n );
      inter_3d( u_h , u_2h , n );
      n = n*2-1;
//print_matrix_3d( u_h , n );
      for ( l=0 ; l<n ; l++ )
         for ( m=0 ; m<n ; m++ )
         {
            u_h[0][l][m]   = u_h[1][l][m];
            u_h[n-1][l][m] = u_h[n-2][l][m];

            u_h[l][0][m]   = u_h[l][1][m];
            u_h[l][n-1][m] = u_h[l][n-2][m];

            u_h[l][m][0]   = u_h[l][m][1];
            u_h[l][m][n-1] = u[i][l][m][n-1];
         }


      mgm_3d( u_h , kx[i] , kz[i] , f[i] , n , dx , dz , dt );

      for (j=0;j<n;j++)
         for (l=0;l<n;l++)
            for (m=0;m<n;m++)
               u_2h[j][l][m] = u_h[j][l][m];

   }

   for ( i=0 ; i<n_grid ; i++ )
   {
      free_3d( u[i]  );
      free_3d( kx[i] );
      free_3d( kz[i] );
      free_3d( f[i]  );
   }
   free_3d( u_2h );
   eh_free( u  );
   eh_free( kx );
   eh_free( kz );
   eh_free( f  );

   return u_h;
}

double *allocate_1d( int n )
{
   return eh_new( double , n );
}

double **allocate_2d( int n )
{
   int i;
   double **ans;
   ans = eh_new( double* , n );
   ans[0] = eh_new( double , n*n );
   for ( i=1 ; i<n ; i++ )
      ans[i] = ans[i-1] + n;
   return ans;
}

double ***allocate_3d( int n )
{
   int i, j;
   double ***ans;

   ans = eh_new( double** , n );
   ans[0] = eh_new( double* , n*n );
   ans[0][0] = eh_new( double , n*n*n );

   for ( i=1 ; i<n ; i++ )
   {
      ans[i] = ans[i-1] + n;
      ans[i][0] = ans[i-1][0] + n*n;
   }

   for ( i=0 ; i<n ; i++ )
      for ( j=1 ; j<n ; j++ )
         ans[i][j] = ans[i][j-1]+n;
  
/*
   ans = eh_new( double** , n ); 
   for ( i=0 ; i<n ; i++ )
      ans[i] = eh_new( double* , n );
   for ( i=0 ; i<n ; i++ )
      for ( j=0 ; j<n ; j++ )
         ans[i][j] = eh_new( double , n );
*/

   return ans;
}

void free_2d( double **a )
{
   eh_free( a[0] );
   eh_free( a );
}

void free_3d( double ***a )
{
   eh_free( a[0][0] );
   eh_free( a[0] );
   eh_free( a );
}

void print_matrix( double **a , int n )
{
   int i, j;
   for ( i=0 ; i<n ; i++ )
   {
      for ( j=0 ; j<n ; j++ )
         fprintf( stderr , "%f " , a[i][j] );
      fprintf( stderr , "\n" );
   }
}

void print_matrix_3d( double ***a , int n )
{
   int i, j, k;
   for ( k=0 ; k<n ; k++ )
   {
      fprintf( stderr , "k=%d\n" , k );
      for ( j=0 ; j<n ; j++ )
      {
         for ( i=0 ; i<n ; i++ )
            fprintf( stderr , "%.2f " , a[i][j][k] );
         fprintf( stderr , "\n" );
      }
      fprintf( stderr , "\n\n");
   }

   fprintf( stderr , "\n\n" );
}



