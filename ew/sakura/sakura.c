//--- // // This file is part of sedflux.
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
#include <utils/utils.h>
#include "sakura_local.h"
#include "sakura.h"

Sakura_sediment*
sakura_sediment_new( gint n_grains )
{
   Sakura_sediment* s = NULL;

   if ( n_grains>0 )
   {
      s = eh_new( Sakura_sediment , 1 );
      s->rho_grain  = eh_new0( double , n_grains );
      s->rho_dep    = eh_new0( double , n_grains );
      s->u_settling = eh_new0( double , n_grains );

      s->len = n_grains;
   }

   return s;
}

Sakura_sediment*
sakura_sediment_destroy( Sakura_sediment* s )
{
   if ( s )
   {
      eh_free( s->rho_grain  );
      eh_free( s->rho_dep    );
      eh_free( s->u_settling );
      eh_free( s );
   }
   return NULL;
}

Sakura_sediment*
sakura_sediment_set_rho_grain( Sakura_sediment* s , double* x )
{
   if ( s && x )
   {
      eh_dbl_array_copy( s->rho_grain , x , s->len );
   }
   return s;
}

Sakura_sediment*
sakura_sediment_set_rho_dep( Sakura_sediment* s , double* x )
{
   if ( s && x )
   {
      eh_dbl_array_copy( s->rho_dep , x , s->len );
   }
   return s;
}

Sakura_sediment*
sakura_sediment_set_u_settling( Sakura_sediment* s , double* x )
{
   if ( s && x )
   {
      eh_dbl_array_copy( s->u_settling , x , s->len );
   }
   return s;
}

Sakura_array*
sakura_array_new( gint len , gint n_grain )
{
   Sakura_array* a = NULL;

   if ( len>0 )
   {
      gint n_nodes = len + 4;
      gint i;

      a = eh_new( Sakura_array , 1 );

      a->x = eh_new0( double , n_nodes ) + 2;
      a->w = eh_new0( double , n_nodes ) + 2;
      a->h = eh_new0( double , n_nodes ) + 2;
      a->u = eh_new0( double , n_nodes ) + 2;
      a->c = eh_new0( double , n_nodes ) + 2;

      a->c_grain     = eh_new0( double* , n_nodes ) + 2;
      a->c_grain[-2] = eh_new0( double  , n_nodes*n_grain );
      for ( i=-1 ; i<len+2 ; i ++ )
         a->c_grain[i] = a->c_grain[i-1] + n_grain;

      a->d     = eh_new0( double* , n_nodes ) + 2;
      a->d[-2] = eh_new0( double  , n_nodes*n_grain );
      for ( i=-1 ; i<len+2 ; i ++ )
         a->d[i] = a->d[i-1] + n_grain;

      a->e     = eh_new0( double* , n_nodes ) + 2;
      a->e[-2] = eh_new0( double  , n_nodes*n_grain );
      for ( i=-1 ; i<len+2 ; i ++ )
         a->e[i] = a->e[i-1] + n_grain;

      a->len     = len;
      a->n_grain = n_grain;
   }

   return a;
}

Sakura_array*
sakura_array_destroy( Sakura_array* a )
{
   if ( a )
   {
      a->x -= 2;
      a->w -= 2;
      a->h -= 2;
      a->u -= 2;
      a->c -= 2;

      eh_free( a->x );
      eh_free( a->w );
      eh_free( a->h );
      eh_free( a->u );
      eh_free( a->c );

      a->c_grain -= 2;
      a->d       -= 2;
      a->e       -= 2;

      eh_free( a->c_grain[0] );
      eh_free( a->c_grain    );
      eh_free( a->d[0]       );
      eh_free( a->d          );
      eh_free( a->e[0]       );
      eh_free( a->e          );

      eh_free( a );
   }
   return NULL;
}

Sakura_array*
sakura_array_copy( Sakura_array* d , Sakura_array* s )
{
   if ( s )
   {
      if ( !d )
         d = sakura_array_new( s->len , s->n_grain );

      eh_dbl_array_copy( d->x-2 , s->x-2 , s->len+4 );
      eh_dbl_array_copy( d->w-2 , s->w-2 , s->len+4 );
      eh_dbl_array_copy( d->h-2 , s->h-2 , s->len+4 );
      eh_dbl_array_copy( d->u-2 , s->u-2 , s->len+4 );
      eh_dbl_array_copy( d->c-2 , s->c-2 , s->len+4 );

      eh_dbl_array_copy( d->c_grain[-2] , s->c_grain[-2] , (s->len+4)*s->n_grain );
      eh_dbl_array_copy( d->d[-2]       , s->d[-2]       , (s->len+4)*s->n_grain );
      eh_dbl_array_copy( d->e[-2]       , s->e[-2]       , (s->len+4)*s->n_grain );
   }

   return d;
}

Sakura_array*
sakura_array_set_x( Sakura_array* a , double* x )
{
   eh_require( a    );
   eh_require( a->x );
   eh_require( x    );

   if ( a && x )
   {
      double dx;

      eh_dbl_array_copy( a->x , x , a->len );

      dx = x[1] - x[0];

      eh_require( dx>0 );

      a->x[-1] = x[0] - dx;
      a->x[-2] = x[0] - dx*2.;

      dx = x[a->len-1] - x[a->len-2];

      eh_require( dx>0 );

      a->x[a->len  ] = x[a->len-1] + dx;
      a->x[a->len+1] = x[a->len-1] + dx*2.;
   }

   return a;
}

Sakura_array*
sakura_array_set_w( Sakura_array* a , double* w )
{
   eh_require( a    );
   eh_require( a->w );
   eh_require( w    );

   if ( a && w )
   {
      eh_dbl_array_copy( a->w , w , a->len );

      a->w[-1] = w[0];
      a->w[-2] = w[0];

      a->w[a->len  ] = w[a->len-1];
      a->w[a->len+1] = w[a->len-1];
   }

   return a;
}

Sakura_array*
sakura_array_set_bc( Sakura_array* a , Sakura_node* inflow , Sakura_node* outflow )
{
   if ( a )
   {
      const gint len = a->len;

      a->u[0]     = inflow->u;
      a->u[-1]    = inflow->u;
      a->u[-2]    = inflow->u;

      a->c[-1]    = inflow->c;
      a->c[-2]    = inflow->c;

      eh_dbl_array_copy( a->c_grain[-1] , inflow->c_grain , a->n_grain );
      eh_dbl_array_copy( a->c_grain[-2] , inflow->c_grain , a->n_grain );

      a->h[-1]    = inflow->h;
      a->h[-2]    = inflow->h;

      a->u[len]   = outflow->u;
      a->u[len+1] = outflow->u;

      a->c[len]   = outflow->c;
      a->c[len+1] = outflow->c;

      eh_dbl_array_copy( a->c_grain[len]   , outflow->c_grain , a->n_grain );
      eh_dbl_array_copy( a->c_grain[len+1] , outflow->c_grain , a->n_grain );

      a->h[len]   = outflow->h;
      a->h[len+1] = outflow->h;
   }
   return a;
}

double
sakura_array_mass_in_susp( Sakura_array* a , Sakura_sediment* s )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i;
      gint   n;
      double vol_w;

      for ( i=0 ; i<a->len+1 ; i++ )
      {
         vol_w = a->h[i]*a->w[i]*(a->x[i+1]-a->x[i]);
         for ( n=0 ; n<s->len ; n++ )
            mass += vol_w*a->c_grain[i][n]*s->rho_grain[n];
      }
   }

   return mass;
}

double
sakura_array_mass_lost( Sakura_array* a , Sakura_sediment* s , double dt )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i = a->len-2;
      gint   n;
      double flux_water = 0;
      double flux_sed   = 0;

      flux_water = a->h[i]*a->w[i]*a->u[i];
      for ( n=0 ; n<s->len ; n++ )
         flux_sed += flux_water*a->c_grain[i][n]*s->rho_grain[n];
      mass = flux_sed * dt;
   }

   return mass;
}

double
sakura_array_mass_eroded( Sakura_array* a , Sakura_sediment* s )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i;
      gint   n;

      for ( i=0 ; i<a->len ; i++ )
         for ( n=0 ; n<s->len ; n++ )
            mass += a->e[i][n]*s->rho_grain[n];
   }

   return mass;
}

double
sakura_array_mass_deposited( Sakura_array* a , Sakura_sediment* s )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i;
      gint   n;

      for ( i=0 ; i<a->len+1 ; i++ )
         for ( n=0 ; n<s->len ; n++ )
            mass += a->d[i][n]*s->rho_grain[n];
   }

   return mass;
}

gint
sakura_array_print_data( Sakura_array* a , Sakura_const_st* c )
{
   gint n = 0;

   if ( c->data_fp && c->data_id )
   {
      gint          i;
      Sakura_var*   id;
      double*       data = NULL;
      FILE*         fp   = c->data_fp;
      const gint    len  = a->len;
      
      for ( id=c->data_id ; *id>=0 ; id++ )
      {
         switch ( *id )
         {
            case SAKURA_VAR_VELOCITY: data = a->u; break;
            case SAKURA_VAR_DEPTH:    data = a->h; break;
            case SAKURA_VAR_WIDTH:    data = a->c; break;
            default: eh_require_not_reached();
         }

         n += fprintf( fp , "%f" , data[0] );
         for ( i=1 ; i<len ; i++ )
            n += fprintf( fp , "; %f" , data[i] );
         n += fprintf( fp , "\n" );
      }
   }

   return n;
}

Sakura_node*
sakura_node_new( double u , double c , double h , double* c_grain , gint len )
{
   return sakura_node_set( NULL , u , c , h , c_grain , len );
}

Sakura_node*
sakura_node_destroy( Sakura_node* x )
{
   if ( x )
   {
      eh_free( x->c_grain );
      eh_free( x );
   }
   return NULL;
}

Sakura_node*
sakura_node_set( Sakura_node* x , double u , double c , double h , double* c_grain , gint len )
{
   if ( !x ) x = eh_new( Sakura_node , 1 );

   if ( x )
   {
      x->u = u;
      x->c = c;
      x->h = h;

      if ( len!=x->n_grain )
      {
         eh_free( x->c_grain );
         x->c_grain = NULL;
      }

      if      ( c_grain    ) x->c_grain = eh_dbl_array_copy( x->c_grain , c_grain , len );
      else if ( x->c_grain ) x->c_grain = eh_dbl_array_set ( x->c_grain , len , 0. );
      else                   x->c_grain = eh_new0          ( double , len );

      x->n_grain = len;

   }

   return x;
}
      
gboolean
sakura_set_outflow( Sakura_node* out , Sakura_array* a , double x_head , double dt , double dx )
{
   gboolean success = TRUE;

   eh_require( out );
   eh_require( a   );

   if ( out && a )
   {
      gint   n_grains  = a->n_grain;
      gint   n_nodes   = a->len;
      double  u = 0.; // Free outflow at downstream end
      double  c = 0.;
      double  h = 0.;
      double* c_grain = eh_new0( double , n_grains );

      //if ( x_head > basin_len+dx )
      if ( x_head > a->x[a->len-1]+dx )
      {
         u = a->u[n_nodes-1];
         h = a->h[n_nodes-2];
         c = a->c[n_nodes-2];
         eh_dbl_array_copy( c_grain , a->c_grain[n_nodes-2] , n_grains );
      }
      //else if ( x_head > basin_len )
      else if ( x_head > a->x[a->len-1] )
      {
         u = 0; 
         h = out->h + a->h[n_nodes-2] * a->u[n_nodes-1] * dt/dx;
         c = a->c[n_nodes-2];
         eh_dbl_array_copy( out->c_grain , a->c_grain[n_nodes-2] , n_grains );
      }

      sakura_node_set( out , u , c , h , c_grain , n_grains );

      eh_free( c_grain );
   }

   return success;
}

double
sakura_get_sin_slope( Sakura_get_func f , gpointer data , Sakura_array* a , gint i )
{
   double s = 0;

   eh_require( f );
if ( i<1 ) i=1;

   if ( f )
   {
      double depth_0 = f( data , a->x[i-1] );
      double depth_1 = f( data , a->x[i]   );
      double dx      = a->x[i] - a->x[i-1];

      s = -sin( atan( (depth_1-depth_0)/dx ) );
   }
   return s;
}

// This is step 1
gboolean
calculate_mid_vel( Sakura_array* a_mid , Sakura_array* a , gint ind_head , Sakura_const_st* con )
{
   gboolean success = TRUE;

   eh_require( a_mid );
   eh_require( a     );
   eh_require( con   );

   if ( a_mid && a && con )
   {
      gint i;
      double dx = a->x[1] - a->x[0];
      //double u_star;
      //double u_head;
      double u_0, ul, ull, ur, urr;
      double cl, cm, cr, hl, hm, hr;
      double s;
      double du_dt;
      const double    dt             = con->dt;
      //Sakura_get_func get_depth_func = con->get_depth;
      //double *x     = a->x;
      double *u     = a->u;
      double *h     = a->h;
      double *c     = a->c;
      double *u_new = a_mid->u;

      // STEP 1: calculate tentative velocity at t + 0.5Dt
      // start from node =1 because velocity is given at upstream end (node=0)
      // calculate only within the flow (behind the head position)
      ind_head = eh_min( ind_head , a_mid->len-1);

      for ( i=1 ; i<=ind_head && success ; i++ )
      {
         u_0   = u[i];

         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
         s = sakura_get_sin_slope( con->get_depth , con->depth_data , a , i );
/*
         depth_0 = get_depth_func( Const->depth_data , x[i-1] );
         depth_1 = get_depth_func( Const->depth_data , x[i]   );
         s       = - sin( atan( (depth_1-depth_0)/dx ) );
*/

         ull = u[i-2];
         ul  = u[i-1]; 
         ur  = u[i+1];
         urr = u[i+2];

         cl  = c[i-1]; 
         cr  = c[i];

         hl  = h[i-1]; 
         hr  = h[i];

         // value at the node calculated from those at midpoints
         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
         if ( hm>0 && hm<HMIN )
         {
            eh_warning( "hm too small in STEP 1 at node = %d" , i );
            success = FALSE;
         }
         du_dt = dudt( u_0 , ul , ur     , ull , urr       , hl    ,
                       hr  , hm , cl     , cr  , cm        , -9999 ,
                       s   , -9 , -99999 , dx  , con->c_drag , con->mu_water );
         // tentative variables with dt = 0.5 Dt 
         u_new[i] = u_0 + du_dt * dt * .5;

         if ( u_new[i] < 0 )
         {
            eh_message( "calculate_mid_vel: Negative flow velocity (i=%d): %f" , i , u_new[i] );
            success = FALSE;
         }

      } // end of STEP1

   }
   else
      success = FALSE;

   return success;
}

// This is step 3
gboolean
calculate_next_vel( Sakura_array* a_last , Sakura_array* a_mid , Sakura_array* a_next , gint ind_head , Sakura_const_st* Const )
{
   gboolean success = TRUE;

   eh_require( a_last );
   eh_require( a_mid  );
   eh_require( a_next );

   if ( a_last && a_mid && a_next )
   {
      const double dx = a_last->x[1] - a_last->x[0];
      const double dt = Const->dt;
      gint i;
      //double u_star;
      double u_0, ul, ull, ur, urr;
      double cl, cm, cr, hl, hm, hr;
      double* u_mid  = a_mid->u;
      double* c_mid  = a_mid->c;
      double* h_mid  = a_mid->h;
      double* u_last = a_last->u;
      double* u_next = a_next->u;
      double s;

      ind_head = eh_min( ind_head , a_last->len-1 );

      /* STEP3: calculate new velocity*/
      /* trying to use variables at t + 0.5 Dt */
      for ( i=1 ; i<=ind_head ; i++ )
      {
         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
/*
         depth_0 = get_depth_func( Const->depth_data , a_last->x[i-1] );
         depth_1 = get_depth_func( Const->depth_data , a_last->x[i]   );
         s       = - sin( atan( (depth_1-depth_0)/dx ) );
*/

         s = sakura_get_sin_slope( Const->get_depth , Const->depth_data , a_last , i );

// u_temp is at t+.5dt
         ull = u_mid[i-2];
         ul  = u_mid[i-1]; 
         u_0 = u_mid[i];
         ur  = u_mid[i+1];
         urr = u_mid[i+2];

// c is at t?
// c_new is at t+dt?
         cl  = c_mid[i-1];
         cr  = c_mid[i];

         hl  = h_mid[i-1];
         hr  = h_mid[i];

         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
// U is at t?
         u_next[i] = u_last[i]
                   + dudt( u_0, ul, ur, ull, urr,
                           hl, hr, hm,
                           cl, cr, cm,
                           -999 , s, -999 , -999 , dx, Const->c_drag, Const->mu_water) * dt;

         eh_require( fabs(u_next[i])<=UPPERLIMIT );

         if (fabs(u_next[i]) > UPPERLIMIT)
         {
            eh_message( "calculate_next_vel: Extreme flow velocity (i=%d): %f" , i , u_next[i] );
            success = FALSE;
         }

         if ( u_next[i] < 0 )
         {
            eh_message( "calculate_next_vel: Negative flow velocity (i=%d): %f" , i , u_next[i] );
            success = FALSE;
         }
      }
/*
      if (x_head <= basin_len) u_new[n_nodes-1]  = 0;
      else                     u_new[n_nodes-1] -= dt/dx*u_new[n_nodes-2] * (u_new[n_nodes-1] - u_new[n_nodes-2]);

      // velocity at flow head boundary
      x_head   += u_head * dt;
      ind_head  = (int)floor(x_head/dx);
      if ( ind_head == n_nodes-1)
      {
         //fprintf(stderr,"flow reaches downstream end\n");
         stopnumber = 0;
      }
*/
   }
   else
      success = FALSE;

   return success;
}


double
sakura_erode_depth( double rho_f , double u , double dt , double sua , double sub , double c_drag )
{
   double e = 0;

   eh_require( rho_f>=0 );
//   eh_require( u>=0     );
   eh_require( dt>0     );

   if ( dt>0 )
   {
      // Amount of erosion in m
      e = ( c_drag * rho_f * u*u - sub ) / sua * ( dt * S_DAYS_PER_SECOND );

      if ( e<0 ) e = 0;
   }

   return e;
}

Sakura_array*
sakura_next_c_grain( Sakura_array* a_next , Sakura_array* a_last , double* u , gint i , double dt , Sakura_sediment* sed )
{
   eh_require( a_next );
   eh_require( a_last );
   eh_require( sed    );

   if ( a_next && a_last && sed )
   {
      gint n;
      double cll, cl, c_0, cr, crr;
      double ul, ur;
      double wl, wr;
      double hll, hl, h_0, hr, hrr;
      double small_h;
      double c_grain_new;
      double df_dt;
      const gint   n_grain = a_last->n_grain;
      const double dx      = a_last->x[i+1] - a_last->x[i];

      ul = u[i];
      ur = u[i+1];

      wl = a_last->w[i];
      wr = a_last->w[i+1]; 

      hll = a_last->h[i-2];
      hl  = a_last->h[i-1];
      h_0 = a_last->h[i  ];
      hr  = a_last->h[i+1];
      hrr = a_last->h[i+2];

      for ( n = 0 ; n<n_grain ; n++)
      {
         small_h = sed->u_settling[n] * dt * Ro;

         cll = a_last->c_grain[i-2][n];
         cl  = a_last->c_grain[i-1][n];
         c_0 = a_last->c_grain[i  ][n];
         cr  = a_last->c_grain[i+1][n];
         crr = a_last->c_grain[i+2][n];

         eh_require( a_next->h[i]>=HMIN );
            
         if ( a_next->h[i]<HMIN )
         {
               c_grain_new = 0;
               //stopnumber = 1;
         }
         else
         {
            df_dt = dfdt(ul, ur, wl, wr, hl*cl, hr*cr, hll*cll, hrr*crr, h_0*c_0, dx, 0);
            c_grain_new = (c_0 * h_0 + dt * df_dt )/a_next->h[i];

            if ( c_grain_new < -HMIN)
            {
               eh_warning("negative new CC: node= %d, i= %d", i, n);
               eh_warning("cnew= %f, cold=%f", c_grain_new, c_0);
               c_grain_new = 0;
            }
         } //cnewi is the new concentration due to sediment transport by the flow

         a_next->c_grain[i][n] = c_grain_new;
      }

      a_next->c[i] = eh_dbl_array_sum( a_next->c_grain[i] , n_grain );

   }
   else
      a_next = NULL;

   return a_next;
}

double
sakura_rho_flow( double* c_grain , double* rho_grain , gint n_grains , double rho_water )
{
   double rho_f = 0;

   {
      gint n;
      for ( n=0 ; n<n_grains ; n++ )
         rho_f += c_grain[n]*rho_grain[n];

      rho_f += rho_water;
   }

   return rho_f;
}

double
sakura_erode( Sakura_array* a , Sakura_sediment* sed , double* u , gint i , double dt , Sakura_const_st* c )
{
   double ero = 0;

   eh_require( a    );
   eh_require( sed  );
   eh_require( c    );
   eh_require( dt>0 );

   if ( a && sed && c && dt>0 )
   {
      gint           n;
      const gint     n_grains   = sed->len;
      const double   dx         = a->x[i+1] - a->x[i];
      double*        phe_bottom = eh_new( double , n_grains );
      const double   vol_w      = dx*a->w[i]*a->h[i];
      double         rho_f;
      double         e_tot;
      double         e_grain;
      double         p;
      Sakura_cell_st sediment;
      Sakura_phe_st  phe_data;

      // Density of the flow
      rho_f      = sakura_rho_flow( a->c_grain[i] , sed->rho_grain , n_grains , c->rho_sea_water );

      // Total erosion depth (in meters of sediment plus water) over the time step
      e_tot      = sakura_erode_depth( rho_f , .5*(u[i]+u[i+1]) , dt , c->sua , c->sub , c->c_drag );

      // Cubic meters of eroded sediment plus water
      phe_data.val      = e_tot*dx*a->w[i];
      phe_data.phe      = phe_bottom;
      phe_data.n_grains = n_grains;

      c->get_phe( c->get_phe_data , a->x[i] , &phe_data );

      // get_phe_func may have changed the erosion depth if there wasn't enough sediment.
      // meters of sediment plus water
      e_tot = phe_data.val/(dx*a->w[i]);

      for ( n=0 ; n<n_grains ; n++ )
      {
//         porosity = ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - rho_sea_water );
//         e_grain = e_tot*phe_bottom[n]*(1.-porosity);

         p = ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - c->rho_sea_water );

         // Meters of sediment plus water
         e_grain = e_tot*phe_bottom[n];

         // Cubic meters of sediment plus water
         sediment.id = n;
         sediment.t  = e_grain*dx*a->w[i];

         if ( e_grain > 0 ) e_grain = c->remove( c->remove_data , a->x[i] , &sediment );
         if ( e_grain > 0 )
         {
            // Cubic meters of sediment
            e_grain *= (1-p);

//         if (a->h[i] < HMIN) a->c_grain[i][n]  = 0;
//         else                a->c_grain[i][n] += e_grain * dt / a->h[i];

            if ( a->h[i]>=HMIN ) a->c_grain[i][n] += e_grain / vol_w;

            eh_require( a->c_grain[i][n]>=0 );

            if ( a->c_grain[i][n]<0 ) a->c_grain[i][n] = 0.;
         }
         else
            e_grain = 0;

         ero        += e_grain;
         a->e[i][n] += e_grain;
      }

      eh_free( phe_bottom );
   }
   return ero;
}

double
sakura_deposit( Sakura_array* a , Sakura_sediment* sed , gint i , double dt , Sakura_const_st* c )
{
   double dep = 0;

   eh_require( a    );
   eh_require( sed  );
   eh_require( dt>0 );

   if ( a && sed && dt>0 )
   {
      if ( a->x[i] > c->dep_start )
      {
         gint           n;
         const gint     n_grains = sed->len;
         const double   dx       = a->x[i+1] - a->x[i];
         const double   vol_w    = dx*a->w[i]*a->h[i];
         Sakura_cell_st sediment;
         double         d_grain;
         double         p;
         double         small_h;
         double         avail;

         for ( n=0 ; n<n_grains ; n++ )
         {
            small_h = sed->u_settling[n] * dt * Ro;

            // Meters of sediment
            if ( a->h[i] <= small_h ) d_grain = a->h[i]/dt           *a->c_grain[i][n];
            else                      d_grain = sed->u_settling[n]*Ro*a->c_grain[i][n];

            p = ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - c->rho_sea_water );

            // Meters of sediment plus water
            d_grain /= (1-p);

            // Cubic meters of sediment plus water
            sediment.id = n;
            sediment.t  = d_grain*dx*a->w[i]*dt;

            if ( d_grain > 0 ) d_grain = c->add( c->add_data , a->x[i] , &sediment );

            // Cubic meters of sediment
            d_grain *= (1-p);

//            if (a->h[i] < HMIN) a->c_grain[i][n]  = 0;
//            else                a->c_grain[i][n] -= d_grain * dt / (dx*a->w[i]*a->h[i]);

            avail = a->c_grain[i][n]*vol_w;

            if ( d_grain > avail ) d_grain = avail;

            a->c_grain[i][n] -= d_grain / vol_w;

            eh_require( a->c_grain[i][n]>=-1e-10 );

            if ( a->c_grain[i][n]<0 ) a->c_grain[i][n] = 0.;

            dep        += d_grain;
            a->d[i][n] += d_grain;
         }
      }
   }

   return dep;
}

double
sakura_deposit_all( Sakura_array* a , Sakura_sediment* sed , Sakura_const_st* c )
{
   double dep = 0;

   eh_require( a    );
   eh_require( sed  );

   if ( a && sed )
   {
      gint           i, n;
      const gint     n_grains   = sed->len;
      double*        f_sed      = eh_new( double , n_grains );
      double         vol_w;
      double         vol_grain;
      Sakura_cell_st sediment;

      /* 1 minus porosity (sediment volume over total volume) */
      for ( n=0 ; n<n_grains ; n++ )
         f_sed[n] = 1. - ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - c->rho_sea_water );

      for ( i=0 ; i<a->len ; i++ )
      {
         vol_w = a->h[i]*a->w[i]*(a->x[i+1]-a->x[i]);
         for ( n=0 ; n<n_grains ; n++ )
         {
            // Meters of sediment plus water
            vol_grain = vol_w*a->c_grain[i][n] / f_sed[n];

            // Cubic meters of sediment plus water
            sediment.id = n;
            sediment.t  = vol_grain;

            if ( vol_grain > 0 ) vol_grain = c->add( c->add_data , a->x[i] , &sediment );

            // Cubic meters of sediment
            dep       += vol_grain*f_sed[n];
         }
      }

      eh_free( f_sed );
   }

   return dep;
}

gboolean
compute_c_grain_new( Sakura_array* a , Sakura_array* a_last , double* u , gint i , double dt , Sakura_const_st* c , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   eh_require( a      );
   eh_require( a_last );
   eh_require( u      );
   eh_require( c      );
   eh_require( sed    );

   if ( dt>0 )
   {
      sakura_next_c_grain( a , a_last , u , i , dt , sed );
      sakura_erode       ( a , sed , u , i , dt , c );
      sakura_deposit     ( a , sed , i , dt , c );
   }
   return success;
}

// output c_new, sed_rate, CCMULTI_new
// input
gboolean
compute_c_grain( Sakura_array* a , Sakura_array* a_last , double* u , gint i , double dx , Sakura_const_st* Const , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   eh_require( a        );
   eh_require( a_last   );
   eh_require( i>=0     );
   eh_require( i<a->len );

   if ( a && a_last )
   {
      gint   n;
      double sed_rate;
      double c_new;
      double h_0;
      double cll, cl, c_0, cr, crr;
      double ul, um, ur;
      double wl, wr;
      double hll, hl, hr, hrr;
      double small_h;
      double dh;
      double c_grain_new;
      double erode_depth;
      double depth_node;
      double porosity;
      double flux_at_bed;
      double rho_avg;
      double rho_bottom;
      double df_dt;
      const double    init_h         = a_last->h[-1];
      const double    dt             = Const->dt;
      const gint      n_grain        = a->n_grain;
      double*         phe_bottom     = eh_new( double , n_grain );
      double*         erosion        = eh_new( double , n_grain );
      Sakura_phe_func get_phe_func   = Const->get_phe;
      Sakura_add_func add_func       = Const->add;
      Sakura_add_func remove_func    = Const->remove;
      Sakura_get_func get_depth_func = Const->get_depth;
      Sakura_phe_st   phe_data;
      Sakura_cell_st  sediment;


      ul = u[i];
      ur = u[i+1];

      wl = a_last->w[i];
      wr = a_last->w[i+1]; 

      um = 0.5 * ( ul + ur );

      hll = a_last->h[i-2];
      hl  = a_last->h[i-1];
      h_0 = a_last->h[i  ];
      hr  = a_last->h[i+1];
      hrr = a_last->h[i+2];

      // compute CCMULTI for each grain size fraction
      for ( n = 0, sed_rate = 0, c_new = 0; n<n_grain ; n++)
      {

         small_h = sed->u_settling[n] * dt * Ro;

         cll = a_last->c_grain[i-2][n];
         cl  = a_last->c_grain[i-1][n];
         c_0 = a_last->c_grain[i  ][n];
         cr  = a_last->c_grain[i+1][n];
         crr = a_last->c_grain[i+2][n];

         eh_require( a->h[i]>=HMIN );
            
         if ( a->h[i]<HMIN )
         {
            c_grain_new = 0;
            //stopnumber = 1;
         }
         else
         {
            df_dt = dfdt(ul, ur, wl, wr, hl*cl, hr*cr, hll*cll, hrr*crr, h_0*c_0, dx, 0);
            c_grain_new = (c_0 * h_0 + dt * df_dt )/a->h[i];

            if ( c_grain_new < -HMIN)
            {
               eh_warning("negative new CC: node= %d, i= %d", i, n);
               eh_warning("cnew= %f, cold=%f", c_grain_new, c_0);
               c_grain_new = 0;
            }
         } //cnewi is the new concentration due to sediment transport by the flow

         // here we get the PheBottom at the node location.
//         erode_depth     = ( Const->c_drag * (1+c_grain_new*R)*Const->rho_sea_water * um*um - Const->sub );

         // amount of erosion in a time step
         erode_depth     = ( Const->c_drag * (1+c_grain_new*R)*Const->rho_sea_water * um*um - Const->sub )
                         / Const->sua * ( dt * S_DAYS_PER_SECOND );

//         phe_data.val      = erode_depth*dx;
         phe_data.val      = erode_depth*dx*a->w[i]; // Eroded volume
         phe_data.phe      = phe_bottom;
         phe_data.n_grains = a->n_grain;

         get_phe_func( Const->get_phe_data , a->x[i] , &phe_data );

         // get_phe_func may have changed the erosion depth if there wasn't enough sediment.
//         erode_depth = phe_data.val/dx;
         erode_depth = phe_data.val/(dx*a->w[i]);

         // Update the shear strength at the surface
         //a->sub[i] += Const->sua*erode_depth;

         rho_avg    = eh_dbl_array_mean_weighted( sed->rho_grain , a->n_grain , phe_bottom );
         rho_bottom = eh_dbl_array_mean_weighted( sed->rho_dep   , a->n_grain , phe_bottom );

         eh_require( rho_avg   >0 );
         eh_require( rho_bottom>0 );

         porosity = 1.0 - rho_bottom/rho_avg;

//         erosion[n] = erode_depth
//                    * phe_bottom[n]*(1.-porosity)
//                    / (Const->sua*S_SECONDS_PER_DAY);

         erosion[n] = erode_depth*phe_bottom[n]*(1.-porosity);

         if ( a->h[i] <= small_h) flux_at_bed = a->h[i]/dt              * c_grain_new - eh_max( 0 , erosion[n] ); 
         else                     flux_at_bed = sed->u_settling[n] * Ro * c_grain_new - eh_max( 0 , erosion[n] );

         if ( a->x[i] < Const->dep_start ) flux_at_bed = 0.0;

         depth_node = get_depth_func( Const->depth_data , a->x[i] );

         if ( depth_node + flux_at_bed*dt/porosity/phe_bottom[n] > -init_h )
         //if ( depth_node + flux_at_bed*dt/porosity/phe_bottom[n] > 0 )
            flux_at_bed = 0.0;

         dh = flux_at_bed*dt/porosity;

         // Keep track of volume eroded/deposited at each node and for each grain size
         sediment.t  = dh*dx*a->w[i];
         sediment.id = n;
         if ( dh<0 ) a->e[i][n] += remove_func( Const->remove_data , a->x[i] , &sediment );
         else        a->d[i][n] += add_func   ( Const->add_data    , a->x[i] , &sediment );

//         SEDMULTI[i][n] += flux_at_bed * dt / porosity;

         if (a->h[i] < HMIN) a->c_grain[i][n] = 0;
         else                a->c_grain[i][n] = c_grain_new - flux_at_bed * dt/ a->h[i];

         eh_require( a->c_grain[i][n] >= -HMIN );

         if (a->c_grain[i][n] < -HMIN)
         {
            eh_warning("negative CCMULTInew: node=%d, grain=%d",i,n);
            eh_warning("cnew=%f, cold=%f, hnew=%f", a->c_grain[i][n], c_grain_new , a->h[i]);
         }
         else if ( a->c_grain[i][n] < 0)
            a->c_grain[i][n] = 0.0;

         a->c_grain[i][n] = eh_max( 0 , a->c_grain[i][n] );

         c_new    += a->c_grain[i][n];
         sed_rate += flux_at_bed;
      } //end of CCMULTI

      a->c[i]     = c_new;
//      a->s[i]    += dt*sed_rate;
//      a->r[i]    += sed_rate;

      eh_free( phe_bottom );
      eh_free( erosion    );
/*
      max_c       = eh_max( max_c , c_new );
      total_susp += c_new * a->h[i];

      if ( max_c<=HMIN || total_susp<=HMIN )
      {
         eh_warning("maxc=%f, totalsusp=%f", max_c, total_susp);
         eh_warning("ccmultinew=%f, hew=%f",a->c_grain[i-1][n-1], a->h[i-1]);
         eh_watch_int( node );
         eh_watch_dbl( HMIN );
         stopnumber = 1;
      }
*/
      
   }
   else
      success = FALSE;

   return success;
}

gboolean
compute_next_h( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* c )
{
   gboolean success = TRUE;

   if ( a_new && a_last )
   {
      const double dt    = c->dt;
      const gint   top_i = a_new->len-2;
      const double dx    = a_new->x[1] - a_new->x[0];
      gint i;
      double hll, hl, h_0, hr, hrr;
      double ul, um, ur;
      double wl, wr;
      double c_0;
      double Ew, Ri;
      double df_dt;

      ind_head = eh_min( ind_head , a_last->len-1 );

      // START STEP2: calculate new flow thickness and sediment concentration
      // calculations at node midpoint for HH, CC and SED
      //  uses Utemp to get HHnew and CCnew
      for ( i=0 ; i<=ind_head && i<=top_i && success ; i++ )
      {
         h_0 = a_last->h[i];
         c_0 = a_last->c[i];
         
         ul = u_temp[i];
         ur = u_temp[i+1];

         wl = a_last->w[i];
         wr = a_last->w[i+1]; 

         um = 0.5 * ( ul + ur );

         hll = a_last->h[i-2];
         hl  = a_last->h[i-1];
         hr  = a_last->h[i+1];
         hrr = a_last->h[i+2];
         
         // compute water entrainment 
         if ( eh_compare_dbl( um , 0.0 , 1e-12 ) )
         {
            Ew = 0.0;
            Ri = 0.0;
         }
         else
         //if (um != 0.0)
         {
            Ri = R * G * c_0 * h_0 / eh_sqr(um);
            Ew = c->e_a / (c->e_b + Ri);
         }

         df_dt = dfdt(ul, ur, wl, wr, hl, hr, hll, hrr, h_0, dx, Ew*fabs(um));
         
         // compute new HH
         a_new->h[i] = h_0 + dt * df_dt;

         eh_require( a_new->h[i]>=0 );

         if (a_new->h[i] < 0 || eh_isnan(a_new->h[i]) )
         {
            //eh_warning( "HHnew negative but cancelled at the %d-th node" ,i);
            //eh_warning( "ul:%f, ur:%f, hl:%f, h:%f, hr:%f", ul,ur,hl,h_0,hr);
            //a_new->h[i] = 0;
            eh_watch_dbl( ul );
            eh_watch_dbl( ur );
            eh_watch_dbl( wl );
            eh_watch_dbl( wr );
            eh_watch_dbl( hll );
            eh_watch_dbl( hl );
            eh_watch_dbl( h_0 );
            eh_watch_dbl( hr );
            eh_watch_dbl( hrr );
            eh_watch_dbl( dx );
            eh_watch_dbl( Ew );
            eh_watch_dbl( fabs(um) );

            eh_watch_int( i );
            eh_watch_int( a_last->len );
            eh_watch_dbl( a_last->w[i] );
            eh_watch_dbl( a_last->w[i+1] );

            eh_warning( "compute_next_h: Negative flow height (i=%d): %f", i , a_new->h[i] );
            success = FALSE;
         }
      }
   }

   return success;
}

gboolean
compute_next_c( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* c , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   if ( a_new && a_last )
   {
      gint         i;
      const gint   top_i = a_new->len-2;
      const double dt    = c->dt;

      ind_head = eh_min( ind_head , a_last->len-1 );

      for ( i=0 ; i<=ind_head && i<=top_i ; i++ )
      {
         sakura_next_c_grain( a_new , a_last , u_temp , i , dt , sed );
         sakura_erode       ( a_new , sed , u_temp  , i , dt , c );
         sakura_deposit     ( a_new , sed , i , dt , c );
      }
   }

   return success;
}

// output HH_new, CC_new, and SED_new at t+dt
// input HH, CC, Wx
gboolean
calculate_next_c_and_h( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* Const , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   if ( a_new && a_last )
   {
      const double dt    = Const->dt;
      const gint   top_i = a_new->len-2;
      const double dx    = a_new->x[1] - a_new->x[0];
      gint i;
      double hll, hl, h_0, hr, hrr;
      double ul, um, ur;
      double wl, wr;
      double c_0;
      double Ew, Ri;
      double df_dt;

      // START STEP2: calculate new flow thickness and sediment concentration
      // calculations at node midpoint for HH, CC and SED
      //  uses Utemp to get HHnew and CCnew
      for ( i=0 ; i<=ind_head && i<=top_i ; i++ )
      {
         h_0 = a_last->h[i];
         c_0 = a_last->c[i];
         
         ul = u_temp[i];
         ur = u_temp[i+1];

         wl = a_last->w[i];
         wr = a_last->w[i+1]; 

         um = 0.5 * ( ul + ur );

         hll = a_last->h[i-2];
         hl  = a_last->h[i-1];
         hr  = a_last->h[i+1];
         hrr = a_last->h[i+2];
         
         // compute water entrainment 
         if ( eh_compare_dbl( um , 0.0 , 1e-12 ) )
         {
            Ew = 0.0;
            Ri = 0.0;
         }
         else
         //if (um != 0.0)
         {
            Ri = R * G * c_0 * h_0 / eh_sqr(um);
            Ew = Const->e_a / (Const->e_b + Ri);
         }

         df_dt = dfdt(ul, ur, wl, wr, hl, hr, hll, hrr, h_0, dx, Ew*fabs(um));
         
         // compute new HH
         a_new->h[i] = h_0 + dt * df_dt;

         eh_require( a_new->h[i]>=0 );

         if (a_new->h[i] < 0 )
         {
            eh_warning( "HHnew negative but cancelled at the %d-th node" ,i);
/*
            eh_warning( "old=%f, new=%f, dfdt=%f, left=%f, right=%f"      ,h,HHnew[node],dfdt(ul,ur,wl,wr,hl,hr,hll,hrr,h,Dx,Ew*um),tvdleft(ul,h,hl,hr,hll,hrr),tvdright(ur,h,hl,hr,hll,hrr));
*/
            eh_warning( "ul:%f, ur:%f, hl:%f, h:%f, hr:%f", ul,ur,hl,h_0,hr);
            a_new->h[i] = 0;
         }

//                     if (HHnew[node] > LARGER(InitH, fabs(DEPTH[node])) )
 //                        HHnew[node] = -DEPTH[node];

         // compute CCMULTI for each grain size fraction
         compute_c_grain_new( a_new , a_last , u_temp , i , dx , Const , sed );

         //a->c[i]     = c_new;
         //a->s[i]    += dt*sed_rate;
         //a->r[i]    += sed_rate;

         //max_c       = eh_max( max_c , c_new );
         //total_susp += c_new * a->h[i];
      } /*end STEP2*/
   }
   else
      success = FALSE;

   return success;
}

gboolean
calculate_mid_c_and_h( Sakura_array* a_mid , Sakura_array* a_last , Sakura_array* a_next )
{
   gboolean success = TRUE;

   if ( a_mid && a_last && a_next )
   {
      gint i;
      gint top_i = a_mid->len+2;

      for ( i=-2 ; i<top_i ; i++ )
      {
         a_mid->c[i] = .5*( a_last->c[i] + a_next->c[i] );
         a_mid->h[i] = .5*( a_last->h[i] + a_next->h[i] );
      }
   }
   return success;
}

gint
calculate_head_index( Sakura_array* a , double* u , gint ind_head , double dx , double dt , double* x_head )
{
   gint new_ind = -1;

   eh_require( a );
   eh_require( u );

   if ( a && u )
   {
      eh_require( ind_head>=0     );
if ( ind_head<0 )
   eh_watch_int( ind_head );

      if ( ind_head<a->len )
      {
         double u_head = eh_max( u[ind_head] , u[ind_head-1] );

         if      ( ind_head<=0 ) u_head = u[0];
         else if ( u_head   >0 ) u_head = eh_min( u_head , 1.5*pow( G*R*a->c[ind_head-1]*a->h[ind_head-1] ,1./3.) );
         else                    u_head = eh_max( u[ind_head] , u[ind_head-1] );

         *x_head += u_head * dt;
         new_ind  = floor( (*x_head-a->x[0]) / dx );
      }
      else
         new_ind = ind_head;

      eh_require( new_ind>=0     );
if ( new_ind<0 )
{
   eh_watch_int( new_ind );
   eh_watch_int( ind_head );
   eh_watch_int( a->len );
   eh_watch_dbl( u[ind_head] );
   eh_watch_dbl( u[ind_head-1] );
   eh_exit(0);
}
   }

   return new_ind;
}

/** Run the sakura hyperpycnal flow model

\param dx        [UNUSED] Grid spacing (m)
\param dt        Time step (s)
\param basin_len [UNUSED]
\param n_nodes   Number of grid nodes
\param n_grains  Number of sediment types
\param Xx        Positions of grid nodes (m)
\param Zz        Elevations of grid nodes (m)
\param Wx        Channel width at grid nodes (m)
\param u_init    River velocity (m/s)
\param c_init    River volume concentration (m^3/m^3)
\param lambda    Removal rates for sediment types [UNUSED]
\param u_settling Settling velocitiesfor sediment types (m/s)
\param Rey        [UNUSED]
\param rho_grain Grain density of sediment types (kg/m^3)
\param h_init    River depth
\param supply_time Duration of flow (s)
\param DepositionStart [UNUSED]
\param fraction  Fraction of each grain type in river
\param bottom_f  [UNUSED]
\param rho_dep   Bulk density of each grain type when deposited on the sea floor (kg/m^3)
\param OutTime   [UNUSED]
\param c         Structure of constants used in model
\param Deposit   [UNUSED]
\param fp_data   [UNUSED]

\return TRUE on success of FALSE if an error occured
*/
/*
gboolean
sakura( double dx          , double dt              , double basin_len ,
        int n_nodes        , int n_grains           , double Xx[]      ,
        double Zz[]        , double Wx[]            , double u_init[]  ,
        double c_init[]    , double *Lambda         , double* u_settling ,
        double *Rey        , double *rho_grain      , double h_init    ,
        double supply_time , double DepositionStart , double *fraction ,
        double *bottom_f   , double* rho_dep        , double OutTime   ,
        Sakura_const_st* c , double **Deposit       , FILE *fp_data )
*/
/** Run the sakura hyperpycnal flow model

\param u_riv     River velocity (m/s)
\param c_riv     River concentration (kg/m^3)
\param h_riv     River depth (m)
\param f_riv     Fraction of each grain type in river
\param dt        Time step to use (s)
\param duration  Duration of flow (s)
\param x         Position of grid nodes (m)
\param z         Elevation of grid nodes (m)
\param w         Channel widht at grid nodes (m)
\param n_nodes   Number of grid nodes
\param rho_grain Grain density of sediment types (kg/m^3)
\param rho_dep   Bulk density of each grain type when deposited on the sea floor (kg/m^3)
\param u_fall    Settling velocitiesfor sediment types (m/s)
\param n_grains  Number of sediment types
\param c         Structure of constants used in model

\return TRUE on success of FALSE if an error occured
*/
double**
sakura( double  u_riv     , double  c_riv    , double  h_riv  , double* f_riv    ,
        double  dt        , double  duration ,
        double* x         , double* z        , double* w      , gint    n_nodes  ,
        double* rho_grain , double* rho_dep  , double* u_fall , gint    n_grains ,
        Sakura_const_st* c )
{
   double** deposit = NULL;
   gboolean success = TRUE;

   eh_require( u_riv>0    );
   eh_require( c_riv>0    );
   eh_require( h_riv>0    );
   eh_require( f_riv      );
   eh_require( dt>0       );
   eh_require( duration>0 );
   eh_require( x          );
   eh_require( z          );
   eh_require( w          );
   eh_require( n_nodes>0  );
   eh_require( rho_grain  );
   eh_require( rho_dep    );
   eh_require( u_fall     );
   eh_require( n_grains>0 );
   eh_require( c          );

   if ( TRUE || g_getenv( "SAKURA_DEBUG" ) )
   {
         double       mass_in        = 0;
         const double vol_w          = u_riv*h_riv*w[0]*duration;
         gint n;
eh_watch_dbl( duration );
eh_watch_dbl( u_riv );
eh_watch_dbl( c_riv );
eh_watch_dbl( h_riv );
         // c_riv is now volume concentration
         for ( n=0,mass_in=0 ; n<n_grains ; n++ )
         {
            mass_in += c_riv*f_riv[n]*vol_w;
eh_watch_dbl( f_riv[n] );
eh_watch_dbl( rho_dep[n] );
eh_watch_dbl( rho_grain[n] );
eh_watch_dbl( u_fall[n] );
         }

         for ( n=0 ; n<n_nodes ; n++ )
            fprintf( stderr , "%f ; %f ; %f\n" , x[n] , z[n] , w[n] );

eh_watch_dbl( c->dt );
eh_watch_dbl( c->e_a );
eh_watch_dbl( c->e_b );
eh_watch_dbl( c->sua );
eh_watch_dbl( c->sub );
eh_watch_dbl( c->c_drag );
eh_watch_dbl( c->rho_river_water );
eh_watch_dbl( c->rho_sea_water );
eh_watch_dbl( c->tan_phi );
eh_watch_dbl( c->mu_water );
eh_watch_dbl( c->channel_width );
eh_watch_dbl( c->channel_len );
eh_watch_dbl( c->dep_start );
   }

   if ( dt>0 )
   { // Run the model for positive time steps
      Sakura_array*    a_next  = sakura_array_new( n_nodes , n_grains );
      Sakura_array*    a_mid   = sakura_array_new( n_nodes , n_grains );
      Sakura_array*    a_last  = sakura_array_new( n_nodes , n_grains );
      Sakura_node*     inflow  = NULL;
      Sakura_node*     outflow = NULL;
      Sakura_sediment* sed     = sakura_sediment_new( n_grains );
      double           mass_lost = 0;

      { // Set the inflow and outflow conditions
         gint    n;
         double* c_grain = eh_dbl_array_dup( f_riv , n_grains );

         for ( n=0 ; n<n_grains ; n++ )
            c_grain[n] = (c_riv*f_riv[n])/rho_grain[n];

         // Convert river concentration to volume concentration
         c_riv = eh_dbl_array_sum( c_grain , n_grains );

         inflow  = sakura_node_new( u_riv , c_riv , h_riv , c_grain , n_grains );

         c_grain = eh_dbl_array_set( c_grain , n_grains , 0. );
         outflow = sakura_node_new( 0. , 0. , 0. , c_grain , n_grains );

         eh_free( c_grain );
      }

      { // Initialize arrays
         sakura_array_set_x( a_next , x );
         sakura_array_set_x( a_mid  , x );
         sakura_array_set_x( a_last , x );
         sakura_array_set_w( a_next , w );
         sakura_array_set_w( a_mid  , w );
         sakura_array_set_w( a_last , w );
      }

      { // Initialize sediment
         sakura_sediment_set_rho_dep   ( sed , rho_dep   );
         sakura_sediment_set_rho_grain ( sed , rho_grain );
         sakura_sediment_set_u_settling( sed , u_fall    );
      }

      c->sua       *= 1e3;
      c->sub       *= 1e3;
      c->dep_start += x[0];

      if ( TRUE || g_getenv( "SAKURA_DEBUG" ) )
      { // Print input variables for debugging
         gint n;

         eh_debug( "Supply time        : %f" , duration     );
         eh_debug( "Init velocity      : %f" , inflow->u    );
         eh_debug( "Init concentration : %f" , inflow->c    );
         eh_debug( "Init height        : %f" , inflow->h    );
         eh_debug( "Time step          : %f" , dt           );
         eh_debug( "Number of nodes    : %d" , n_nodes      );
         eh_debug( "Number of grains   : %d" , n_grains     );
   
         for ( n=0 ; n<n_grains ; n++ )
         {
            eh_debug( "Grain Type: %d" , n );
            eh_debug( "   Settling velocity (m/d)  : %f" , u_fall[n]*S_SECONDS_PER_DAY );
            eh_debug( "   Grain density (kg/m^3)   : %f" , rho_grain[n] );
            eh_debug( "   Deposit density (kg/m^3) : %f" , rho_dep[n] );
            eh_debug( "   Fraction                 : %f" , f_riv[n] );
         }
   
         eh_debug( "c->dt  : %f" , c->dt  );
         eh_debug( "c->sua : %f" , c->sua );
         eh_debug( "c->sub : %f" , c->sub );
         eh_debug( "c->e_a : %f" , c->e_a );
         eh_debug( "c->e_b : %f" , c->e_b );
         eh_debug( "c->c_drag : %f" , c->c_drag );
         eh_debug( "c->mu_water : %f" , c->mu_water );
         eh_debug( "c->rho_sea_water : %f" , c->rho_sea_water );
         eh_debug( "c->dep_start : %f" , c->dep_start );
      }
   
      { // Run the model
         const double dx       = a_last->x[1] - a_last->x[0];
         double       x_head   = HMIN + a_last->x[0];
         gint         ind_head = floor( (x_head-a_last->x[0])/dx );
         const double total_t = 2.*duration;
         double       t;
         gint         n;
   
         eh_require( x_head>0         );
         eh_require( ind_head>=0      );
         eh_require( ind_head<n_nodes );
   
         for ( t=0.,n=0 ; t<=total_t && success ; t+=dt,n++ )
         { // Run the flow for each time step
            fprintf( stdout , "SAKURA time: %f s (%f s)\r" , t , total_t );

            if ( t>duration )
            {
               inflow->u = 0;
               inflow->c = 0;
               inflow->h = 0;
            }
   
            sakura_set_outflow ( outflow , a_last , x_head  , dt , dx );
            sakura_array_set_bc( a_last  , inflow , outflow );
            sakura_array_set_bc( a_mid   , inflow , outflow );
   
            // Calculate u at t+dt/2.  The rest of a_mid is invalid.  a_mid->u is u_temp.
            // This also calculates u at the head of the flow
            if ( success ) success = calculate_mid_vel( a_mid  , a_last , ind_head , c );
   
            // Calculate c, and h at t+dt.  This is a_next.  a_next->u is not valid.
            if ( success ) success = compute_next_h( a_next , a_last , a_mid->u , ind_head , c );
            if ( success ) success = compute_next_c( a_next , a_last , a_mid->u , ind_head , c , sed );
   
            // Set new boundary conditions
            sakura_set_outflow ( outflow , a_last , x_head , dt , dx );
            sakura_array_set_bc( a_next  , inflow , outflow );

            // Calculate c, and h at t+dt/2.  This is in a_mid and an average of a_last and a_next
            if ( success ) success = calculate_mid_c_and_h( a_mid , a_last , a_next );
   
            // Calculate u at t+dt.  This is a_next->u.
            if ( success ) success = calculate_next_vel( a_last , a_mid , a_next , ind_head , c );
   
            ind_head = calculate_head_index( a_last , a_mid->u , ind_head , dx , dt , &x_head );
   
            // Update variables
            sakura_array_copy( a_last , a_next );

            if ( c->data_int>0 && n%c->data_int==0 ) sakura_array_print_data( a_last , c );

            mass_lost += sakura_array_mass_lost( a_last , sed , dt );
         }

         if ( !success )
         { /* If no success, deposit everything in suspension */
            if ( t<duration )
            { /* What hasn't left the river yet */
               eh_warning( "Time remaining (seconds): %f" , duration - t );
            }
            sakura_deposit_all( a_last , sed , c );
         }
      }

      if ( TRUE || g_getenv( "SAKURA_DEBUG" ) )
      { // Mass balance check
         gint         n;
         double       mass_in        = 0;
         //double       mass_out       = 0;
         double       mass_bal       = 0;
         const double mass_in_susp   = sakura_array_mass_in_susp  ( a_last , sed );
         const double mass_eroded    = sakura_array_mass_eroded   ( a_last , sed );
         const double mass_deposited = sakura_array_mass_deposited( a_last , sed );
         const double vol_w          = u_riv*h_riv*w[0]*duration;

         // c_riv is now volume concentration
         for ( n=0,mass_in=0 ; n<n_grains ; n++ )
            mass_in += c_riv*f_riv[n]*vol_w*rho_grain[n];

         mass_bal = mass_in + mass_eroded - mass_deposited - mass_in_susp - mass_lost;

         fprintf( stdout , "\n\n" );
         fprintf( stdout , "Mass in (kg)             : %g\n" , mass_in        );
         fprintf( stdout , "Mass eroded (kg)         : %g\n" , mass_eroded    );
         fprintf( stdout , "Mass deposited (kg)      : %g\n" , mass_deposited );
         fprintf( stdout , "Mass in suspension (kg)  : %g\n" , mass_in_susp   );
         fprintf( stdout , "Mass lost (kg)           : %g\n" , mass_lost      );
         fprintf( stdout , "----------------------------------------------\n" );
         fprintf( stdout , "Mass balance (kg)        : %g\n" , mass_bal       );
         fprintf( stdout , "\n\n" );

         if ( mass_bal > 0 ) eh_message( "Relative error (-)       : %g (lost)"   , mass_bal / mass_in );
         else                eh_message( "Relative error (-)       : %g (gained)" , mass_bal / mass_in );

         //mass_in  += sakura_array_mass_eroded( a_last , sed );
         //mass_out  = sakura_array_mass_in_susp  ( a_last , sed )
         //          + sakura_array_mass_deposited( a_last , sed )
         //          + mass_lost
         //          - mass_eroded;


         //if ( !eh_compare_dbl(mass_in,mass_out,.01) )
         if ( !eh_compare_dbl(mass_bal,0.,.01) )
            eh_warning( "Mass balance check failed" );
      }

      if ( TRUE )
      {
         gint i, n;
         deposit = eh_new_2( double , n_grains , n_nodes );

         for ( n=0 ; n<n_grains ; n++ )
            for ( i=0 ; i<n_nodes ; i++ )
               deposit[n][i] = a_last->d[i][n]*rho_grain[n]/rho_dep[n];
      }


      { // de-allocate memory
         sakura_array_destroy( a_next );
         sakura_array_destroy( a_mid  );
         sakura_array_destroy( a_last );
   
         sakura_sediment_destroy( sed );
   
         sakura_node_destroy( inflow  );
         sakura_node_destroy( outflow );
      }

      c->sua       /= 1e3;
      c->sub       /= 1e3;
      c->dep_start -= x[0];
   }

   return deposit;
}

static gchar* _default_config[] = {
"sua",
"sub",
"entrainment constant, ea",
"entrainment constant, eb",
"drag coefficient",
"internal friction angle",
"width of channel",
"length of channel",
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

