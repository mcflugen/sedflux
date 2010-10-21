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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "failure.h"

#include <math.h>

#define FACTOR_OF_SAFETY_ERROR -2
Fail_slice** get_janbu_parameters( const Sed_cube p   ,
                                   Fail_slice **slice ,
                                   const Failure_t* failure_const );
double rtsafe_fos( void (*funcd)(double, gconstpointer, double *, double *) ,
                   gconstpointer data                                       ,
                   double x1                                                ,
                   double x2                                                ,
                   double xacc );
void factor_of_safety(double x,gconstpointer,double *fn, double *dfn);
double get_m(const Sed_column,double,double);

double get_factor_of_safety( const Sed_cube p ,
                             const Failure_t* failure_const )
{
   int i;
   double fs;
   Fail_slice **slice;
   Fail_slice **cur_vars;

   eh_require( sed_cube_is_1d(p) );

   slice    = eh_new( Fail_slice* , sed_cube_n_y(p) );
   slice[0] = eh_new( Fail_slice  , sed_cube_n_y(p) );
   for ( i=1 ; i<sed_cube_n_y(p) ; i++ )
      slice[i] = slice[i-1] + 1;

   if ( !get_janbu_parameters( p , slice , failure_const ) )
   {
      eh_free(slice[0]);
      eh_free(slice);
      return FACTOR_OF_SAFETY_ERROR;
   }

   // don't do the first slice and last slice.  their depths are zero which
   // causes problems.
   cur_vars = slice+1;
   cur_vars[sed_cube_n_y(p)-2] = NULL;

//   fsPoints_ = p->size-2;
//   fs  = rtsafe(&factor_of_safety,0.0001,2000.,.01);
   fs  = rtsafe_fos(&factor_of_safety,(gconstpointer)cur_vars,0.005,200.,.01);

   eh_free(slice[0]);
   eh_free(slice);

   return fs;
}

Fail_slice **get_janbu_parameters( const Sed_cube p   ,
                                   Fail_slice **slice ,
                                   const Failure_t *failure_const )
{
   double volume=0.;
   double width, m;
   double rhoSolid;
   double gravity;
   double a_angle;
   double hydro_static;
   int i, len;
   Sed_column this_col;

   gravity = 9.81;

   len = sed_cube_n_y(p);

   a_angle = M_PI/64.;

   for ( i=0 ; i<len ; i++ )
   {
      slice[i]->a_vertical   = sed_cube_quake(p)*cos(a_angle);
      slice[i]->a_horizontal = sed_cube_quake(p)*sin(a_angle);
   }

// NOTE: 'm' is taken from D.A.Sangrey in Eleventh Annual Offshore Technology 
// Conference -- 1979 Proceedings Volume 1.  It's the slope relating (H/h) and
// (delta u / gamma' h).  For large values of the quantity (m^2 t / Cv) and for
// values of (H/h) < 0.2 it can be approximated as 1.
   m = 1.;

   width = sed_cube_y_res(p);

   // Depth (m) of sediment in this slice of the failure.
   for ( i=0 ; i<len ; i++ )
      slice[i]->depth = sed_cube_thickness(p,0,i);

   // Get slopes at the failure surface.
   for ( i=0 ; i<len-1 ; i++ )
      slice[i]->alpha = fabs( atan( (   sed_cube_base_height(p,0,i+1)
                                      - sed_cube_base_height(p,0,i) )
                                    / width ) );
//      slice[i].alpha = fabs(atan((slice[i+1].depth-slice[i].depth)/width));
   slice[len-1]->alpha = slice[len-2]->alpha;
   
   for (i=0,volume=0.;i<len;i++)
   {

      this_col = sed_cube_col(p,i);

if ( sed_column_len( this_col ) > 0 )
{
      hydro_static = sed_column_water_pressure( this_col );

      volume += slice[i]->depth;

      rhoSolid = sed_column_top_rho( this_col , slice[i]->depth );

// the global model.
      m = get_m( this_col , slice[i]->depth , failure_const->consolidation );
      slice[i]->u   = (rhoSolid-failure_const->density_sea_water)
                    * gravity*slice[i]->depth
                    / m;
      slice[i]->c   = failure_const->cohesion;
      slice[i]->phi = failure_const->frictionAngle;

// the local model.
      slice[i]->u   = sed_cell_pressure( sed_column_nth_cell( this_col , 0 ) )
                    - hydro_static;
      if ( slice[i]->u < 0 )
         slice[i]->u = 0;

//      slice[i]->c   = sed_get_column_property_with_load( S_COHESION , p->col[0][i] );
//      slice[i]->phi = sed_get_column_property( S_FRICTION_ANGLE , p->col[0][i] )*S_RADS_PER_DEGREE;

      slice[i]->b   = width;
      slice[i]->w   = (rhoSolid-failure_const->density_sea_water)
                    * gravity*slice[i]->b*slice[i]->depth;

      // If the excess pore pressure exceeds the weight of the sediment,
      // reduce the excess pore pressure to match the sediment weight.
      if ( slice[i]->u > slice[i]->w/slice[i]->b )
         slice[i]->u = slice[i]->w/slice[i]->b;
}
   }
   
   if ( volume == 0 )
      return NULL;
   else
      return slice;
}

void factor_of_safety( double x , gconstpointer data , double *fn , double *dfn )
{
   int i;
   double c1[MAX_FAILURE_LENGTH], c2[MAX_FAILURE_LENGTH];
   double numerator, denominator;
   double f=1.; /* this is the shape factor.  We just set it to 1 now. */
   double effective_weight, horizontal_weight;
   Fail_slice **col = (Fail_slice**)data;

   for ( i=0,numerator=0.,denominator=0. ; col[i] ; i++ )
   {

      // The effective weight is the normal weight minus the weight
      // relieved by the vertical accelerations of an earthquake.  
      // The horizontal weight is just the weight of the sediment 
      // adjusted to the horizontal accelerations of the earthquake.
      effective_weight  = col[i]->w
                        - col[i]->a_vertical/sed_gravity()*col[i]->w;
      horizontal_weight = col[i]->a_horizontal/sed_gravity()*col[i]->w;

      denominator += effective_weight*sin(col[i]->alpha) 
                   + horizontal_weight*cos(col[i]->alpha);

      c1[i] = col[i]->b
            * (    col[i]->c
                +   (effective_weight/col[i]->b-col[i]->u-horizontal_weight*sin(col[i]->alpha) )
                  * tan(col[i]->phi) )
            / cos(col[i]->alpha);
      c2[i] = tan(col[i]->alpha)*tan(col[i]->phi);
      numerator += c1[i]/(1.+c2[i]/x);

   }
   *fn = f*numerator/denominator - x;

   for ( i=0,numerator=0. ; col[i] ; i++ )
      numerator += c1[i]*c2[i]/pow(x+c2[i]/x,2);

   *dfn = f*numerator/denominator - 1.;

   return;
}

#define MAXIT 1000

double rtsafe_fos(void (*funcd)(double, gconstpointer, double *, double *), gconstpointer data, double x1, double x2, double xacc)
{
   int j;
   double df,dx,dxold,f,fh,fl;
   double temp,xh,xl,rts;

   (*funcd)(x1,data,&fl,&df);
   (*funcd)(x2,data,&fh,&df);
   if ((fl > 0.0 && fh > 0.0) || (fl < 0.0 && fh < 0.0))
        {
//           eh_message("Root must be bracketed in rtsafe");
           return -1;
        }
   if (fl == 0.0) return x1;
   if (fh == 0.0) return x2;
   if (fl < 0.0) {
      xl=x1;
      xh=x2;
   } else {
      xh=x1;
      xl=x2;
   }
   rts=0.5*(x1+x2);
   dxold=fabs(x2-x1);
   dx=dxold;
   (*funcd)(rts,data,&f,&df);
   for (j=1;j<=MAXIT;j++) {
      if ((((rts-xh)*df-f)*((rts-xl)*df-f) >= 0.0)
         || (fabs(2.0*f) > fabs(dxold*df))) {
         dxold=dx;
         dx=0.5*(xh-xl);
         rts=xl+dx;
         if (xl == rts) return rts;
      } else {
         dxold=dx;
         dx=f/df;
         temp=rts;
         rts -= dx;
         if (temp == rts) return rts;
      }
      if (fabs(dx) < xacc) return rts;
      (*funcd)(rts,data,&f,&df);
      if (f < 0.0)
         xl=rts;
      else
         xh=rts;
   }

//   eh_message("Maximum number of iterations exceeded in rtsafe");
//   return 0.0;

   return -1;
}
#undef MAXIT

static const gchar* _default_config[] = {
"coefficient of consolidation",
"cohesion of sediments",
"apparent coulomb friction angle",
"fraction of clay for debris flow",
NULL
};

gchar*
get_config_text (const gchar* file)
{
  if (g_ascii_strcasecmp (file, "config")==0)
    return g_strjoinv ("\n", (gchar**)_default_config);
  else
    return NULL;
}
