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
#include <stdlib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "subside.h"

#include <math.h>

void subside_point_load( Eh_dbl_grid g , double load , double h , double E , int i_load , int j_load )
{
   double alpha;
   double x_0, y_0;
   double **z = eh_dbl_grid_data(g);

   alpha = get_flexure_parameter( h , E , (eh_grid_n_x(g)==1)?1:2 );

   x_0 = eh_grid_x(g)[i_load];
   y_0 = eh_grid_y(g)[j_load];

   if ( eh_grid_n_x(g) > 1 )
   {
      gssize i, j;
      double r;
      double c = load/(2.*M_PI*sed_rho_mantle()*sed_gravity()*pow(alpha,2.));

      for ( i=0 ; i<eh_grid_n_x(g) ; i++ )
         for ( j=0 ; j<eh_grid_n_y(g) ; j++ )
         {
            r              = sqrt( pow(eh_grid_x(g)[i]-x_0,2) + pow(eh_grid_y(g)[j]-y_0,2) )
                           / alpha;
            z[i][j] += - c * eh_kei_0( r );
         }
   }
   else
   {
      if ( fabs( load )>1e-5 )
      {
         gssize j;
         double r;
         double c = load/( 2.*alpha*sed_rho_mantle()*sed_gravity() );

         for ( j=0 ; j<eh_grid_n_y(g) ; j++ )
         {
            r = fabs(eh_grid_y(g)[j]-y_0)/alpha;
            z[0][j] += c * exp( -r ) * ( cos(r) + sin(r) );
         }
      }

   }

}

void subside_half_plane_load( Eh_dbl_grid g ,
                              double load    ,
                              double h       ,
                              double E )
{
   double alpha = get_flexure_parameter( h , E , (eh_grid_n_x(g)==1)?1:2 );

   //---
   // This half-plane solution is only valid for the 1D case.
   //---
   eh_require( eh_grid_n_x(g)==1 )
   {
      //---
      // Add half-plane load.
      //---
      if ( fabs(load)>1e-5 )
      {
         gssize i;
         double r;
         double y_l = 1.5*eh_grid_y(g)[eh_grid_n_y(g)-1]
                    -  .5*eh_grid_y(g)[eh_grid_n_y(g)-2];
         double c   = load/(2*sed_rho_mantle()*sed_gravity());
         double** z = eh_dbl_grid_data(g);

         for ( i=0 ; i<eh_grid_n_y(g) ; i++ )
         {
            r        = (y_l - eh_grid_y(g)[i])/alpha;
            z[0][i] += c * exp(-r)*cos(r);
         }
      }
   }
}

double get_flexure_parameter( double h , double E , gssize n_dim )
{
   double poisson = .25;
   double D       = E*pow(h,3)/12./(1-pow(poisson,2));
   double rho_m   = sed_rho_mantle();
   double alpha;

   eh_require( n_dim==1 || n_dim==2 );

   if ( n_dim > 1 )
      alpha = pow( D / (rho_m * sed_gravity()) , .25 );
   else
      alpha = pow( 4.*D/(rho_m * sed_gravity()) , .25 );

   return alpha;
}

