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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "subside.h"

#include <math.h>
#ifdef HAVE_OMP_H
# include <omp.h>
#endif

#define N_THREADS 5

typedef struct
{
   Eh_dbl_grid w;
   double      v_0;
   double      eet;
   double      y;
   gint        id;
}
Subside_data;

void subside_helper( gpointer data , gpointer v_0 );
void subside_parallel_row( double* w, const double* load, const gint len, const double dy, const double dx, const double alpha, const double* r );

/* Calculate a deflection grid

\param w   Grid of deflections.
\param v_0 Grid of loads.
\param eet Effective elastic thickness.
\parma y   Young's modulus

*/
void
subside_grid_load( Eh_dbl_grid w , Eh_dbl_grid v_0 , double eet , double y )
{
   eh_require( w   );
   eh_require( v_0 );

   if ( !g_thread_supported() ) g_thread_init(NULL);
#undef WITH_THREADS
#ifndef WITH_THREADS
   if ( w && v_0 )
   {
      gint i, j;
      const gint n_x = eh_grid_n_x( v_0 );
      const gint n_y = eh_grid_n_y( v_0 );

      if ( TRUE )
      { /* Grid is not equally spaced. */
         double load;
         for ( i=0 ; i<n_x ; i++ )
         {
            //load = eh_grid_row( v_0, i );
            for ( j=0 ; j<n_y ; j++ )
            {
               load = eh_dbl_grid_val( v_0 , i , j );
               //if ( fabs(load[j]) > 1e-3 )
               if ( fabs(load) > 1e-10 )
                  subside_point_load( w , load , eet , y , i , j );
                  //subside_point_load( w , load[j] , eet , y , i , j );
            }
         }
      }
      else
      { /* Grid is equally spaced. */
         const double alpha     = get_flexure_parameter( eet , y , (eh_grid_n_x(w)==1)?1:2 );
         const double inv_alpha = 1./alpha;
         const double dx        = eh_grid_x(w)[1] - eh_grid_x(w)[0];
         const double dy        = eh_grid_y(w)[1] - eh_grid_y(w)[0];
         double** r             = eh_new_2( double, n_x, n_y );
         gint d_row;
         double dx_2, dy_2;

         for ( d_row=0; d_row<n_x; d_row++ )
         {
            dx_2 = (d_row*dx)*(d_row*dx);
            for ( j=0; j<n_y; j++ )
            {
               dy_2 = (j*dy)*(j*dy);
               r[d_row][j] = eh_kei_0( sqrt( dx_2 + dy_2 )*inv_alpha );
            }
         }

#define USE_OMP
#ifdef USE_OMP
//#pragma omp parallel num_threads(4)
         {
/*
            int id        = omp_get_thread_num();
            int n_threads = omp_get_num_threads();
            int j_start   = id*n_x/n_threads;
            int j_end     = ( id+1 )*n_x/n_threads;

            if ( id==n_threads-1 )
               j_end = n_x;
            for ( j=j_start ; j<j_end ; j++ )
*/

#pragma omp parallel for num_threads(4)
            for ( j=0 ; j<n_x ; j++ )
            {
               double* w_row = eh_grid_row(w,j);
               gint i;
               gint d_row;
               for ( i=0 ; i<n_x ; i++ )
               { /* For each row of loads */
                  d_row = abs(j-i);
                  subside_parallel_row( w_row,
                                        eh_grid_row(v_0,i),
                                        n_y,
                                        dy,
                                        d_row*dx,
                                        alpha, r[d_row] );//r[d_row] );
               }
               //eh_message( "Done (row=%d).", j );
            }
            
         }
#else

//         for ( j=0 ; j<n_x ; j++ )
//         { /* For each row of locations */
//            w_row = eh_grid_row(w,j);
//            for ( i=0 ; i<n_x ; i++ )
//            { /* For each row of loads */
//               d_row = abs(j-i);
//               subside_parallel_row( w_row, eh_grid_row(v_0,i), n_y, dy, d_row*dx, alpha, NULL );
//            }
//         }

         for ( i=0 ; i<n_x ; i++ )
         { /* For each row of loads. */
            load = eh_grid_row( v_0, i );
            for ( j=0 ; j<n_x ; j++ )
            { /* For each row of locations */
               d_row = abs(j-i);
               //subside_parallel_row( eh_grid_row(w,j), load, n_y, dy, d_row*dx, alpha, r[d_row] );
               subside_parallel_row( eh_grid_row(w,j), load, n_y, dy, d_row*dx, alpha, NULL );
            }
         }
#endif

         eh_free_2( r );
      }
   }
#else
   if ( w && v_0 )
   {
      GThreadPool* pool;

      pool = g_thread_pool_new( subside_helper , NULL , N_THREADS , TRUE , NULL );

      eh_require( pool );
      {
         Subside_data* queue = NULL;
         gssize len = eh_grid_n_el( v_0 );
         gssize id, n, n_jobs;
         double load;

         queue = eh_new( Subside_data , len );
         for ( id=0,n=0 ; id<len ; id++ )
         {
            load = eh_dbl_grid_data( v_0 )[0][id];
            if ( fabs(load) > 1e-3 )
            {
               queue[n].w   = eh_grid_dup( w );
               eh_dbl_grid_set( queue[n].w , 0. );

               queue[n].v_0 = load;
               queue[n].id  = id;
               queue[n].eet = eet;
               queue[n].y   = y;

               g_thread_pool_push( pool , &(queue[n]) , NULL );

               n++;
            }
         }
         g_thread_pool_free( pool , FALSE , TRUE );
         n_jobs = n;

         for ( n=0 ; n<n_jobs ; n++ )
         {
            eh_dbl_grid_add( w , queue[n].w );
            eh_grid_destroy( queue[n].w , TRUE );
         }
         eh_free( queue );
      }
   }
#endif

   return;
}

void subside_helper( gpointer d , gpointer g )
{
   Subside_data* data = (Subside_data*)d;
   Eh_ind_2      sub  = eh_grid_id_to_sub( eh_grid_n_y(data->w) , data->id );
   double        load = data->v_0;

   subside_point_load( data->w , load , data->eet , data->y , sub.i , sub.j );

   return;
}

/** Subside a grid of elevations due to a point load

\param g      A Eh_dbl_grid of elevations (in meters)
\param load   Applied load
\param h      EET of crust
\param E      Young's modulus
\param i_load i-subscript where load is applied
\param j_load j-subscript where load is applied

*/
void
subside_point_load( Eh_dbl_grid g , double load , double h , double E , int i_load , int j_load )
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
      double* x = eh_grid_x(g);
      double* y = eh_grid_y(g);
      const double inv_alpha = 1./alpha;
      double dx_2, dy_2;

      for ( i=0 ; i<eh_grid_n_x(g) ; i++ )
      {
         dx_2 = (x[i]-x_0)*(x[i]-x_0);
         for ( j=0 ; j<eh_grid_n_y(g) ; j++ )
         {
            dy_2     = (y[j]-y_0)*(y[j]-y_0);
            r        = sqrt( dx_2 + dy_2 ) * inv_alpha;
            z[i][j] += - c * eh_kei_0( r );
         }
      }
   }
   else
   {
      //if ( fabs( load )>1e-5 )
      if ( fabs( load )>1e-10 )
      {
         gint j;
         const gint len = eh_grid_n_y(g);
         double r;
         double* y = eh_grid_y(g);
         const double c = load/( 2.*alpha*sed_rho_mantle()*sed_gravity() );
         const double inv_alpha = 1./alpha;

         for ( j=0 ; j<len ; j++ )
         {
            r = fabs(y[j]-y_0)*inv_alpha;
            z[0][j] += c * exp( -r ) * ( cos(r) + sin(r) );
         }
      }

   }

}

void
subside_parallel_row( double* w, const double* load, const gint len, const double dy, const double dx, const double alpha, const double* r )
{
   if ( w && load )
   {
      gint i, j;
      double c;
      const double inv_c     = 1./(2.*M_PI*sed_rho_mantle()*sed_gravity()*alpha*alpha);
      const double inv_alpha = 1./alpha;
      const double dx_2      = dx*dx;
      double dy_2;
      double* kei = NULL;
      gboolean free_kei = FALSE;

      if ( !r )
      {
         kei = eh_new( double, len );
         for ( i=0; i<len; i++ )
         {
            dy_2 = (i*dy)*(i*dy);
            kei[i] = eh_kei_0( sqrt( dx_2 + dy_2 )*inv_alpha );
         }
         free_kei = TRUE;
      }
      else
         kei = (double*)r;

      for ( i=0; i<len; i++ )
      { /* For each load. */
         c   = load[i]*inv_c;
         for ( j=0; j<len; j++ )
         { /* For each location. */
            //w[j] += -c * eh_kei_0(r[abs(j-i)]);
            w[j] += -c * kei[abs(j-i)];
         }
      }

      if ( free_kei )
         eh_free( kei );
   }
   return;
}

void
subside_point_load_1d( double* z , double* y , gint len , double load , double y_0 , double alpha )
{
   if ( z )
   {
      gint         j;
      double       r;
      const double c = load/( 2.*alpha*sed_rho_mantle()*sed_gravity() );
      const double inv_alpha = 1./alpha;

      for ( j=0 ; j<len ; j++ )
      {
         r     = fabs(y[j]-y_0)*inv_alpha;
         z[j] += c * exp( -r ) * ( cos(r) + sin(r) );
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
         gint   i;
         double r;
         const gint len = eh_grid_n_y(g);
         double*  y = eh_grid_y(g);
         double** z = eh_dbl_grid_data(g);
         const double y_l       = 1.5*y[len-1] - .5*y[len-2];
         const double c         = load/(2.*sed_rho_mantle()*sed_gravity());
         const double inv_alpha = 1./alpha;

         for ( i=0 ; i<len ; i++ )
         {
            r        = (y_l - y[i])*inv_alpha;
            z[0][i] += c * exp(-r)*cos(r);
         }
      }
   }
}

/** Calculate the flexure parameter, alpha

\param h       Effective elastic thickness of crust
\param E       Young's modulus
\param n_dim   Number of dimensions (1 or 2)

\return The flexure parameter in meters
*/
double
get_flexure_parameter( double h , double E , gssize n_dim )
{
   const double poisson = .25;
   const double D       = E*pow(h,3)/12./(1-pow(poisson,2));
   const double rho_m   = sed_rho_mantle();
   double alpha;

   eh_require( n_dim==1 || n_dim==2 );

   if ( n_dim > 1 ) alpha = pow(    D / (rho_m * sed_gravity()) , .25 );
   else             alpha = pow( 4.*D / (rho_m * sed_gravity()) , .25 );

   return alpha;
}

static gchar* _default_config[] = {
"effective elastic thickness",
"Youngs modulus",
"relaxation time",
NULL
};

gchar*
get_config_text (const gchar* file)
{
  if (g_ascii_strcasecmp (file, "config")==0)
    return g_strjoinv ("\n", _default_config);
  else
    return NULL;
}

