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
#include "xpat.h"

#define N_DEFAULT        (128)
#define U_TOP_DEFAULT    (1.0)
#define V_TOP_DEFAULT    (1.0)
#define U_GEO_DEFAULT    (1.0)
#define V_GEO_DEFAULT    (1.0)
#define LATITUDE_DEFAULT (45.0)
#define K_DEFAULT        (1.0)
#define DEPTH_DEFAULT    (10000.)

static char *help_msg[] = {
" get_vel_profile - calculate a velocity profile.                     ",
"                                                                     ",
" options:                                                            ",
"   u_top    : x-component of velocity at top of water column. (cm/s) ",
"   v_top    : y-component of velocity at top of water column. (cm/s) ",
"   u_geo    : x-component of geostrophic velocity. (cm/s)            ",
"   v_geo    : y-component of geostrophic velocity. (cm/s)            ",
"   n        : number of z-layers                                     ",
"   lat      : latitude (deg)                                         ",
"   k        : vertical diffusion coefficient (cm^2/s)                ",
"   d        : depth of water column (cm)                             ",
"                                                                     ",
NULL };

int main( int argc , char *argv[] )
{
   int n;
   double *z, *k;
   double u_top, v_top;
   double u_geo, v_geo;
   double lat;
   Eh_args *args;
   int i;
   double coriolis;
   double k_val, depth;
   Complex *vel;
   Grain_class avg;
   Grain_class *grain = g_new( Grain_class , 3 );
   double frac[3] = { 1/3. , 1/3. , 1/3. };

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   u_top    = eh_get_opt_dbl( args , "u_top" , U_TOP_DEFAULT    );
   v_top    = eh_get_opt_dbl( args , "v_top" , V_TOP_DEFAULT    );
   u_geo    = eh_get_opt_dbl( args , "u_geo" , U_GEO_DEFAULT    );
   v_geo    = eh_get_opt_dbl( args , "v_geo" , V_GEO_DEFAULT    );
   n        = eh_get_opt_int( args , "n"     , N_DEFAULT        );
   lat      = eh_get_opt_int( args , "lat"   , LATITUDE_DEFAULT );
   k_val    = eh_get_opt_int( args , "k"     , K_DEFAULT        );
   depth    = eh_get_opt_int( args , "d"     , DEPTH_DEFAULT    );

   set_grain_class( &grain[0] , 1   , 2 , 2.5 , 10 , 100 );
   set_grain_class( &grain[1] , 2.5 , 3 , 3.5 , 20 , 200 );
   set_grain_class( &grain[2] , 3.5 , 6 , 7   , 30 , 300 );

   average_grains( grain , frac , 3 , &avg );

eh_watch_dbl( avg.phi );
eh_watch_dbl( avg.tau_cr );

   z      = eh_logspace( -2 , log10(depth) , n );
   k      = g_new( double  , n );
   vel    = g_new( Complex , n );

   for ( i=0 ; i<n ; i++ )
   {
      k[i] = k_val;
   }

   coriolis = pat_get_coriolis_frequency( lat );

   pat_solve_velocity_profile( vel , k , z , coriolis , c_complex(u_geo,v_geo) , 0 , c_complex(0,0) , n-1 , c_complex(u_top,v_top) , n );

   for ( i=0 ; i<n ; i++ )
   {
      fprintf( stdout , "%f , %f , %f , %f\n" , z[i] , k[i] , vel[i].r , vel[i].i );
   }

   free( z );

   return 0;
}

