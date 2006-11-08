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
#include "utils.h"
#include "xpat.h"

//double *tridiag( double* , double* , double* , double* , double* , int );

double *pat_solve_concentration( double *c , double *z , double *k , int n , double w_s , double dc_dz , double dt )
{
   int i;
   int n_elements;
   Advec_element *e;
   Advec_const a_const;
   double alpha = 1.;
   double c_n = 0.;
   double t_end = dt*2;

   // the number of elements.
   n_elements = n-1;

   // allocate memory.
   e    = g_new ( Advec_element , n_elements );

   // define the element parameters.
   for ( i=0 ; i<n_elements ; i++ )
   {
      e[i].x_1 = z[i];
      e[i].x_2 = z[i+1];
      e[i].k   = k[i];
      e[i].v   = w_s;
   }

   // define the parameters needed to solve the advection diffusion equation.
   a_const.dt    = dt;
   a_const.t_end = t_end;
   a_const.alpha = alpha;

   a_const.bc_lower_type = BC_TYPE_NEUMAN;
   a_const.bc_upper_type = BC_TYPE_DIRICHLET;
   a_const.bc_lower_val  = dc_dz;
   a_const.bc_upper_val  = c_n;

   // solve the advection diffusion equation.
   advec( e , c , n , a_const );

   // free allocated memory.
   free( e );

   // return the new concentration profile.
   return c;
}

void get_element_matrix_a( Advec_element e , double a[2][2] );
void get_element_matrix_b( Advec_element e , double b[2][2] );
void get_global_matrix( double a_e[2][2] , int i , double *l , double *d , double *u );
void print_output( double t , double *c , int n );

double *advec( Advec_element *e , double *u , int n_nodes , Advec_const a_const )
{
   double dt, t_end;
   double alpha;
   int i;
   int n_elements;
   double a[2][2], b[2][2];
   double *A_l, *A_d, *A_u;
   double *B_l, *B_d, *B_u;
   double *G_l, *G_d, *G_u;
   double *P_l, *P_d, *P_u;
   double *rhs;
   double t=0;

   dt    = a_const.dt;
   t_end = a_const.t_end;
   alpha = a_const.alpha;

   // the number of elements is one less than the number of nodes.  note
   // that this is the length of the input element array.
   n_elements = n_nodes-1;

   // allocate memory.
   rhs  = g_new0( double , n_nodes );
   G_l  = g_new0( double , n_nodes );
   G_d  = g_new0( double , n_nodes );
   G_u  = g_new0( double , n_nodes );
   P_l  = g_new0( double , n_nodes );
   P_d  = g_new0( double , n_nodes );
   P_u  = g_new0( double , n_nodes );
   A_l  = g_new0( double , n_nodes );
   A_d  = g_new0( double , n_nodes );
   A_u  = g_new0( double , n_nodes );
   B_l  = g_new0( double , n_nodes );
   B_d  = g_new0( double , n_nodes );
   B_u  = g_new0( double , n_nodes );

   // initialize the diagonal entries to zero.  this must be done before
   // assembling the system matrix.
   for ( i=0 ; i<n_nodes ; i++ )
   {
      G_d[i] = 0;
      P_d[i] = 0;
   }

   // assemble the system matrix.
   for ( i=0 ; i<n_elements ; i++ )
   {
      // get the element matricies a and b.
      get_element_matrix_a( e[i] , a );
      get_element_matrix_b( e[i] , b );

      // put these element matrices into the system matrices.
      get_global_matrix( a , i , G_l , G_d , G_u );
      get_global_matrix( b , i , P_l , P_d , P_u );
   }

   for ( i=0 ; i<n_nodes ; i++ )
   {
      // create the left and right hand side matrices for our matrix 
      // equation.  B is the matrix that needs to be inverted.  A will
      // be multiplied by the current concentration values to form the
      // right hand side.
      A_l[i] = -(1-alpha)*G_l[i] - P_l[i]/dt;
      A_d[i] = -(1-alpha)*G_d[i] - P_d[i]/dt;
      A_u[i] = -(1-alpha)*G_u[i] - P_u[i]/dt;

      B_l[i] = alpha*G_l[i] - P_l[i]/dt;
      B_d[i] = alpha*G_d[i] - P_d[i]/dt;
      B_u[i] = alpha*G_u[i] - P_u[i]/dt;
   }

   // apply boundary condition at lower node.
   if ( a_const.bc_lower_type == BC_TYPE_NEUMAN )
   {
      B_d[0] = -1;
      B_u[0] = 1;
      rhs[0] = a_const.bc_lower_val*(e[0].x_2-e[0].x_1);
   }
   else
   {
      B_l[0] = 0;
      B_d[0] = 1;
      rhs[0] = a_const.bc_lower_val;
   }

   // apply boundary condition at upper node.
   if ( a_const.bc_upper_type == BC_TYPE_NEUMAN )
   {
      B_l[n_nodes-1] = -1;
      B_d[n_nodes-1] = 1;
      rhs[n_nodes-1] = a_const.bc_upper_val*(e[n_nodes-1].x_2-e[n_nodes-1].x_1);
   }
   else
   {
      B_l[n_nodes-1] = 0;
      B_d[n_nodes-1] = 1;
      rhs[n_nodes-1] = a_const.bc_upper_val;
   }

   // print the initial conditions.
   print_output( t , u , n_nodes );

   // solve for u at each time step.
   for ( t=dt ; t<t_end ; t+=dt )
   {
      // for the right hand side solumn vector.
      for ( i=1 ; i<n_nodes-1 ; i++ )
         rhs[i] = A_l[i]*u[i-1] + A_d[i]*u[i] + A_u[i]*u[i+1];

      // solve for the new concentrations.
      if ( !tridiag( B_l , B_d , B_u , rhs , u , n_nodes ) )
         perror( "tridiag" );

      // print the concentrations for this time step.
      print_output( t , u , n_nodes );
   }

   free( rhs );
   free( G_l );
   free( G_d );
   free( G_u );
   free( P_l );
   free( P_d );
   free( P_u );
   free( A_l );
   free( A_d );
   free( A_u );
   free( B_l );
   free( B_d );
   free( B_u );

   return u;
}

void get_element_matrix_a( Advec_element e , double a[2][2] )
{
   double l = e.x_2 - e.x_1;

   a[0][0] = -e.k/l + e.v/2;
   a[0][1] =  e.k/l + e.v/2;
   a[1][0] =  e.k/l - e.v/2;
   a[1][1] = -e.k/l - e.v/2;
}

void get_element_matrix_b( Advec_element e , double b[2][2] ) 
{
   double l = e.x_2 - e.x_1;

   b[0][0] = l/3;
   b[0][1] = l/6;
   b[1][0] = l/6;
   b[1][1] = l/3;
}

void get_global_matrix( double a_e[2][2] , int i , double *l , double *d , double *u )
{
   d[i]   += a_e[0][0];
   l[i+1]  = a_e[1][0];

   u[i]    = a_e[0][1];
   d[i+1] += a_e[1][1];
}

void print_output( double t , double *c , int n )
{
   int i;
   fprintf( stdout , "%f " , t );
   for ( i=0 ; i<n ; i++ )
      fprintf( stdout , ", %f" , c[i] );
   fprintf( stdout , "\n" );
}

