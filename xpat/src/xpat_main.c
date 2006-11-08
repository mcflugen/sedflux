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

#define N_DEFAULT         (128)
#define X_0_DEFAULT       (.01)
#define X_N_DEFAULT       (10000.)
#define LATITUDE_DEFAULT  (45.0)
#define WAVE_DIR_DEFAULT  (0.0)
#define WAVE_FREQ_DEFAULT (1e-1)
#define U_WAVE_DEFAULT    ( 5. )
#define V_WAVE_DEFAULT    ( 0. )
#define U_BBL_DEFAULT     ( 5. )
#define V_BBL_DEFAULT     ( -5. )
#define U_GEO_DEFAULT     ( 4. )
#define V_GEO_DEFAULT     ( 5. )
#define W_S_DEFAULT       (1.)
#define EROSION_DEFAULT   (-.1)
#define DT_DEFAULT        (10.)
#define VERBOSE_DEFAULT   (FALSE)

static char *help_msg[] = {
" get_vel_profile - calculate a velocity profile.                     ",
"                                                                     ",
" inputs:                                                             ",
"   uw    : x-component of wave velocity. (cm/s)                      ",
"   vw    : y-component of wave velocity. (cm/s)                      ",
"   ub    : x-component of current velocity at the top of the bottom  ",
"           boundary layer. (cm/s)                                    ",
"   vb    : y-component of current velocity at the top of the bottom  ",
"           boundary layer. (cm/s)                                    ",
"   ug    : x-component of the geostrophic current velocity. (cm/s)   ",
"   vg    : y-component of the geostrophic current velocity. (cm/s)   ",
"   w_dir : direction of incoming waves. (deg)                        ",
"   w_f   : frequency of the waves. (1/s)                             ",
"   x0    : elevation of the bottom node.                             ",
"   xn    : elevation of the top node.                                ",
"   ws    : settling velocity of the suspended sediment. (cm/s)       ",
"   ero   : erosion rate at the base of the model. (cm/s)             ",
"   dt    : time step. (s)                                            ",
"   n     : number of z-layers                                        ",
"   lat   : latitude (deg)                                            ",
"                                                                     ",
NULL };

void print_output( double , double* , int );

int main( int argc , char *argv[] )
{
   int n;
   double *z, x_0, x_n;
   double lat;
   double wave_freq, wave_dir;
   double w_s, dc_dz, dt;
   gboolean verbose;
   Eh_args *args;
   int i;
   double coriolis_freq;
   double u_wave, v_wave, u_bbl, v_bbl, u_geo, v_geo;
   double *k;
   double *c;
   Complex *vel;
   Complex vel_wave;
   Complex vel_bbl;
   Complex vel_geo;

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   u_wave    = eh_get_opt_dbl ( args , "uw"    , U_WAVE_DEFAULT    );
   v_wave    = eh_get_opt_dbl ( args , "vw"    , V_WAVE_DEFAULT    );
   u_bbl     = eh_get_opt_dbl ( args , "ub"    , U_BBL_DEFAULT     );
   v_bbl     = eh_get_opt_dbl ( args , "vb"    , V_BBL_DEFAULT     );
   u_geo     = eh_get_opt_dbl ( args , "ug"    , U_GEO_DEFAULT     );
   v_geo     = eh_get_opt_dbl ( args , "vg"    , V_GEO_DEFAULT     );
   wave_dir  = eh_get_opt_dbl ( args , "w_dir" , WAVE_DIR_DEFAULT  );
   wave_freq = eh_get_opt_dbl ( args , "w_f"   , WAVE_FREQ_DEFAULT );
   x_0       = eh_get_opt_dbl ( args , "x0"    , X_0_DEFAULT       );
   x_n       = eh_get_opt_dbl ( args , "xn"    , X_N_DEFAULT       );
   w_s       = eh_get_opt_dbl ( args , "ws"    , W_S_DEFAULT       );
   dc_dz     = eh_get_opt_dbl ( args , "ero"   , EROSION_DEFAULT   );
   dt        = eh_get_opt_dbl ( args , "dt"    , DT_DEFAULT        );
   n         = eh_get_opt_int ( args , "n"     , N_DEFAULT         );
   lat       = eh_get_opt_int ( args , "lat"   , LATITUDE_DEFAULT  );
   verbose   = eh_get_opt_bool( args , "v"     , VERBOSE_DEFAULT   );

   if ( verbose )
   {
      eh_print_opt( args , "uw"    );
      eh_print_opt( args , "vw"    );
      eh_print_opt( args , "ub"    );
      eh_print_opt( args , "vb"    );
      eh_print_opt( args , "ug"    );
      eh_print_opt( args , "vg"    );
      eh_print_opt( args , "w_dir" );
      eh_print_opt( args , "w_f"   );
      eh_print_opt( args , "x0"    );
      eh_print_opt( args , "xn"    );
      eh_print_opt( args , "ws"    );
      eh_print_opt( args , "ero"   );
      eh_print_opt( args , "dt"    );
      eh_print_opt( args , "n"     );
      eh_print_opt( args , "lat"   );
   }

   wave_dir *= M_PI/180.;
   z         = eh_logspace( log10(x_0) , log10(x_n) , n );
//   z         = eh_linspace( x_0 , x_n , n );
   k         = g_new( double  , n );
   c         = g_new( double  , n );
   vel       = g_new( Complex , n );

   vel_wave = c_complex( u_wave , v_wave );
   vel_bbl  = c_complex( u_bbl  , v_bbl  );
   vel_geo  = c_complex( u_geo  , v_geo  );

   coriolis_freq = pat_get_coriolis_frequency( lat );

// calculate the velocity profile and the vertical diffusion coefficent.
   pat_iterate_velocity( vel , k , z , n , vel_wave , vel_bbl , vel_geo , coriolis_freq , wave_freq , wave_dir , 3*n/4 );

   print_output( 0. , z , n );
   print_output( 0. , k , n );

// set up the initial conditions.
   for ( i=0 ; i<n ; i++ )
      c[i] = 0.;
   for ( i=n/4 ; i<n/2 ; i++ )
      c[i] = 1.;

// solve for the concentrations at each time step.
   for ( i=0 ; i<10 ; i++ )
      pat_solve_concentration( c , z , k , n , w_s , dc_dz , dt );

   free( z   );
   free( k   );
   free( vel );

   return 0;
}

